#include "MeshingEngine.hpp"
#include <thread>

void MeshingEngine::Request(glm::ivec3 pos) {
    {
        std::lock_guard chunks_lock(m_chunks_mutex);
        if (m_chunks.find(pos) != m_chunks.end()) {
            return;
        }

        auto chunk = std::make_shared<Chunk>(pos);
        m_chunks.emplace(pos, chunk);

        {
            std::lock_guard requests_lock(m_requests_mutex);
            m_requests.push({
                Req::Type::TERRAIN, pos
            });
        }
    }

    m_requests_cv.notify_one();
}

void MeshingEngine::worker() {
    while (true) {
        {
            Req r;
            {
                std::unique_lock lock(m_requests_mutex);
                if (m_requests.empty()) {
                    m_requests_cv.wait(lock, [this] { return !m_requests.empty(); });
                }

                r = m_requests.front();
                m_requests.pop();
            }


            std::shared_ptr<Chunk> c, pz, nz, py, ny, px ,nx;

            {
                std::unique_lock lock(m_chunks_mutex);
                c  = get_chunk(r.pos);
                pz = get_chunk(r.pos + glm::ivec3{0, 0, 1});
                nz = get_chunk(r.pos + glm::ivec3{0, 0, -1});
                //py = get_chunk(r.pos + glm::ivec3{0, 1, 0});
                //ny = get_chunk(r.pos + glm::ivec3{0, -1, 0});
                px = get_chunk(r.pos + glm::ivec3{1, 0, 0});
                nx = get_chunk(r.pos + glm::ivec3{-1, 0, 0});
            }

            if (r.type == Req::Type::TERRAIN) {
                if (!c) continue;
                assert(c->GetState() == Chunk::State::UNLOADED);
                c->GenTerrain();
                c->SetState(Chunk::State::GENERATED);

                {
                    std::unique_lock lock(m_requests_mutex);

                    m_requests.push({Req::Type::MESH, r.pos});

                    if (pz && pz->GetState() == Chunk::State::UPLOADED) {
                        pz->SetState(Chunk::State::GENERATED);
                        m_requests.push({Req::Type::MESH, pz->GetPos()});
                    }
                    if (nz && nz->GetState() == Chunk::State::UPLOADED) {
                        nz->SetState(Chunk::State::GENERATED);
                        m_requests.push({Req::Type::MESH, nz->GetPos()});
                    }
                    if (px && px->GetState() == Chunk::State::UPLOADED) {
                        px->SetState(Chunk::State::GENERATED);
                        m_requests.push({Req::Type::MESH, px->GetPos()});
                    }
                    if (nx && nx->GetState() == Chunk::State::UPLOADED) {
                        nx->SetState(Chunk::State::GENERATED);
                        m_requests.push({Req::Type::MESH, nx->GetPos()});
                    }
                }
            } else if (r.type == Req::Type::MESH) {
                if (!c) continue;
                assert(c->GetState() == Chunk::State::GENERATED || c->GetState() == Chunk::State::UPLOADED);

                if ((pz && pz->GetState() == Chunk::State::UNLOADED) ||
                    (nz && nz->GetState() == Chunk::State::UNLOADED) ||
                    (px && px->GetState() == Chunk::State::UNLOADED) ||
                    (nx && nx->GetState() == Chunk::State::UNLOADED)) 
                {
                    {
                        std::lock_guard lock(m_requests_mutex);
                        m_requests.push(r);
                    }

                    std::this_thread::yield();
                    continue;
                }

                if (c->TryLock()) {
                    build_mesh(c);
                    c->SetState(Chunk::State::BUILT);
                    {
                        std::lock_guard lock(m_results_mutex);
                        m_results.push(c);
                    }
                }
            }
        }
    }
}

static constexpr int RAD = 16;

void MeshingEngine::UnloadFarChunks(const glm::ivec3 &cam_chunk) {
    std::lock_guard lock(m_chunks_mutex);

    for (auto it = m_chunks.begin(); it != m_chunks.end(); ) {
        const glm::ivec3 &coord = it->first;

        const int dx = coord.x - cam_chunk.x;
        /*int dy = coord.y - cam_chunk.y;*/
        const int dz = coord.z - cam_chunk.z;

        if (std::abs(dx) > RAD || std::abs(dz) > RAD) {
            auto chunk = it->second;
            if (chunk->GetState() == Chunk::State::BUILDING) {
                continue;
            }

            glm::ivec3 pos = it->first;
            it = m_chunks.erase(it);

            glm::ivec3 offsets[4] = {{1,0,0}, {-1,0,0}, {0,0,1}, {0,0,-1}};
            for (auto& offset : offsets) {
                glm::ivec3 neighbor = pos + offset;
                auto n_it = m_chunks.find(neighbor);
                if (n_it != m_chunks.end() && n_it->second->GetState() == Chunk::State::UPLOADED) {
                    n_it->second->SetState(Chunk::State::GENERATED);

                    {
                        std::lock_guard requests_lock(m_requests_mutex);
                        m_requests.push({Req::Type::MESH, neighbor});
                    }
                    m_requests_cv.notify_one();
                }
            }
        } else {
            ++it;
        }
    }
}

void MeshingEngine::Draw(std::shared_ptr<Shader> shader, std::shared_ptr<Camera> camera) {
    std::lock_guard lock(m_chunks_mutex);

    Camera::Frustum frustum = camera->GetFrustum();

    for (auto &[coord, c] : m_chunks) {
        if (c->GetState() == Chunk::State::UPLOADED || c->IsUploaded()) {
            glm::vec3 min = glm::vec3(c->GetPos()) * glm::vec3(CHUNK_WIDTH, CHUNK_HEIGHT, CHUNK_DEPTH);
            glm::vec3 max = min + glm::vec3(CHUNK_WIDTH, CHUNK_HEIGHT, CHUNK_DEPTH);
            if (frustum.IsBoxVisible(min, max)) {
                c->Draw(shader);
            }
        }
    }
}

std::shared_ptr<Chunk> MeshingEngine::get_chunk(glm::ivec3 pos) {
    const auto it = m_chunks.find(pos);
    return it != m_chunks.end() ? it->second : nullptr;
}

void MeshingEngine::build_mesh(std::shared_ptr<Chunk> chunk) {
    auto &m_verts = chunk->GetVerts();
    auto &m_indices = chunk->GetIndices();

    m_verts.clear();
    m_indices.clear();

    std::shared_ptr<Chunk> pz, nz, py, ny, px ,nx;

    {
        std::lock_guard lock(m_chunks_mutex);

        pz = get_chunk(chunk->GetPos() + glm::ivec3{0, 0, 1});
        nz = get_chunk(chunk->GetPos() + glm::ivec3{0, 0, -1});
        py = get_chunk(chunk->GetPos() + glm::ivec3{0, 1, 0});
        ny = get_chunk(chunk->GetPos() + glm::ivec3{0, -1, 0});
        px = get_chunk(chunk->GetPos() + glm::ivec3{1, 0, 0});
        nx = get_chunk(chunk->GetPos() + glm::ivec3{-1, 0, 0});
    }

    for (int x = 0; x < CHUNK_WIDTH; x++) {
        for (int y = 0; y < CHUNK_HEIGHT; y++) {
            for (int z = 0; z < CHUNK_DEPTH; z++) {

                auto b = chunk->GetBlock(glm::ivec3{x, y, z});
                if (b == Chunk::Block::AIR)
                    continue;

                float t = 0.5;
                float du = (b == Chunk::Block::STONE) ? 0.5f : 0;

                bool draw_PZ, draw_NZ, draw_PY, draw_NY, draw_PX, draw_NX;
                if (z < CHUNK_DEPTH - 1) {
                    draw_PZ = (chunk->GetBlock(glm::ivec3{x, y, z + 1}) == Chunk::Block::AIR);
                } else {
                    draw_PZ = !pz || pz->GetBlock(glm::ivec3{x, y, 0}) == Chunk::Block::AIR;
                }

                if (z > 0) {
                    draw_NZ = (chunk->GetBlock(glm::ivec3{x, y, z - 1}) == Chunk::Block::AIR);
                } else {
                    draw_NZ = !nz || nz->GetBlock(glm::ivec3{x, y, CHUNK_DEPTH - 1}) == Chunk::Block::AIR;
                }

                if (y < CHUNK_HEIGHT - 1) {
                    draw_PY = (chunk->GetBlock(glm::ivec3{x, y + 1, z}) == Chunk::Block::AIR);
                } else {
                    draw_PY = !py || py->GetBlock(glm::ivec3{x, 0, z}) == Chunk::Block::AIR;
                }

                if (y > 0) {
                    draw_NY = (chunk->GetBlock(glm::ivec3{x, y - 1, z}) == Chunk::Block::AIR);
                } else {
                    draw_NY = !ny || ny->GetBlock(glm::ivec3{x, CHUNK_HEIGHT - 1, z}) == Chunk::Block::AIR;
                }

                if (x < CHUNK_WIDTH - 1) {
                    draw_PX = (chunk->GetBlock(glm::ivec3{x + 1, y, z}) == Chunk::Block::AIR);
                } else {
                    draw_PX = !px || px->GetBlock(glm::ivec3{0, y, z}) == Chunk::Block::AIR;
                }

                if (x > 0) {
                    draw_NX = (chunk->GetBlock(glm::ivec3{x - 1, y, z}) == Chunk::Block::AIR);
                } else {
                    draw_NX = !nx || nx->GetBlock(glm::ivec3{CHUNK_WIDTH - 1, y, z}) == Chunk::Block::AIR;
                }


                const auto xf = static_cast<GLfloat>(x);
                const auto yf = static_cast<GLfloat>(y);
                const auto zf = static_cast<GLfloat>(z);

                // Front (+Z)
                if (draw_PZ) {
                    const auto base = static_cast<GLuint>(m_verts.size());

                    m_verts.push_back({xf,     yf,     zf + 1, du, 0});
                    m_verts.push_back({xf + 1, yf,     zf + 1, t+du, 0});
                    m_verts.push_back({xf + 1, yf + 1, zf + 1, t+du, t});
                    m_verts.push_back({xf,     yf + 1, zf + 1, du, t});

                    m_indices.push_back(base + 0);
                    m_indices.push_back(base + 1);
                    m_indices.push_back(base + 2);
                    m_indices.push_back(base + 0);
                    m_indices.push_back(base + 2);
                    m_indices.push_back(base + 3);
                }

                // Back (-Z)
                if (draw_NZ) {
                    
                    const auto base = static_cast<GLuint>(m_verts.size());

                    m_verts.push_back({xf + 1, yf,     zf, du, 0});
                    m_verts.push_back({xf,     yf,     zf, t+du, 0});
                    m_verts.push_back({xf,     yf + 1, zf, t+du, t});
                    m_verts.push_back({xf + 1, yf + 1, zf, du, t});

                    m_indices.push_back(base + 0);
                    m_indices.push_back(base + 1);
                    m_indices.push_back(base + 2);
                    m_indices.push_back(base + 0);
                    m_indices.push_back(base + 2);
                    m_indices.push_back(base + 3);
                }

                // Top (+Y)
                if (draw_PY) {
                    const auto base = static_cast<GLuint>(m_verts.size());

                    if (b == Chunk::Block::GRASS) {
                        m_verts.push_back({xf,     yf + 1, zf + 1, 0, 0.5f});
                        m_verts.push_back({xf + 1, yf + 1, zf + 1, 0.5f, 0.5f});
                        m_verts.push_back({xf + 1, yf + 1, zf,     0.5f, 1.f});
                        m_verts.push_back({xf,     yf + 1, zf,     0, 1.f});
                    } else {
                        m_verts.push_back({xf,     yf + 1, zf + 1, du, 0});
                        m_verts.push_back({xf + 1, yf + 1, zf + 1, t+du, 0});
                        m_verts.push_back({xf + 1, yf + 1, zf,     t+du, t});
                        m_verts.push_back({xf,     yf + 1, zf,     du, t});
                    }

                    m_indices.push_back(base + 0);
                    m_indices.push_back(base + 1);
                    m_indices.push_back(base + 2);
                    m_indices.push_back(base + 0);
                    m_indices.push_back(base + 2);
                    m_indices.push_back(base + 3);
                }

                // Bottom (-Y)
                if (draw_NY) {
                    const auto base = static_cast<GLuint>(m_verts.size());

                    m_verts.push_back({xf,     yf, zf,     du, 0});
                    m_verts.push_back({xf + 1, yf, zf,     t+du, 0});
                    m_verts.push_back({xf + 1, yf, zf + 1, t+du, t});
                    m_verts.push_back({xf,     yf, zf + 1, du, t});

                    m_indices.push_back(base + 0);
                    m_indices.push_back(base + 1);
                    m_indices.push_back(base + 2);
                    m_indices.push_back(base + 0);
                    m_indices.push_back(base + 2);
                    m_indices.push_back(base + 3);
                }

                // Right (+X)
                if (draw_PX) {
                    const auto base = static_cast<GLuint>(m_verts.size());

                    m_verts.push_back({xf + 1, yf,     zf + 1, du, 0});
                    m_verts.push_back({xf + 1, yf,     zf,     t+du, 0});
                    m_verts.push_back({xf + 1, yf + 1, zf,     t+du, t});
                    m_verts.push_back({xf + 1, yf + 1, zf + 1, du, t});

                    m_indices.push_back(base + 0);
                    m_indices.push_back(base + 1);
                    m_indices.push_back(base + 2);
                    m_indices.push_back(base + 0);
                    m_indices.push_back(base + 2);
                    m_indices.push_back(base + 3);
                }

                // Left (-X)
                if (draw_NX) {
                    const auto base = static_cast<GLuint>(m_verts.size());

                    m_verts.push_back({xf, yf,     zf,     du, 0});
                    m_verts.push_back({xf, yf,     zf + 1, t+du, 0});
                    m_verts.push_back({xf, yf + 1, zf + 1, t+du, t});
                    m_verts.push_back({xf, yf + 1, zf,     du, t});

                    m_indices.push_back(base + 0);
                    m_indices.push_back(base + 1);
                    m_indices.push_back(base + 2);
                    m_indices.push_back(base + 0);
                    m_indices.push_back(base + 2);
                    m_indices.push_back(base + 3);
                }
            }
        }
    }
}

void MeshingEngine::Upload() {
    static constexpr int UPLOADS_PER_FRAME = 16;
    std::lock_guard results_lock(m_results_mutex);
    int i = 0;
    while (!m_results.empty() && i++ < UPLOADS_PER_FRAME) {
        std::shared_ptr<Chunk> chunk = m_results.front();
        assert(chunk);
        assert(chunk->GetState() == Chunk::State::BUILT);

        m_results.pop();
        chunk->Upload();
        chunk->SetState(Chunk::State::UPLOADED);
    }
}