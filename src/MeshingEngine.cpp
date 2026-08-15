#include "MeshingEngine.hpp"
#include <thread>

#include "Camera.hpp"

void MeshingEngine::Request(glm::ivec3 pos) {
    {
        std::lock_guard chunks_lock(m_chunks_mutex);
        if (m_chunks.find(pos) != m_chunks.end()) {
            {
                std::lock_guard requests_lock(m_requests_mutex);
                m_chunks[pos]->SetState(Chunk::State::GENERATED);
                m_requests.push({
                    Req::Type::MESH, pos
                });
            }
            m_requests_cv.notify_one();
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
                py = get_chunk(r.pos + glm::ivec3{0, 1, 0});
                ny = get_chunk(r.pos + glm::ivec3{0, -1, 0});
                px = get_chunk(r.pos + glm::ivec3{1, 0, 0});
                nx = get_chunk(r.pos + glm::ivec3{-1, 0, 0});
            }

            if (r.type == Req::Type::TERRAIN) {
                if (!c) continue;
                //assert(c->GetState() == Chunk::State::UNLOADED);
                if (c->GetState() != Chunk::State::UNLOADED) continue;
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

                    if (ny && ny->GetState() == Chunk::State::UPLOADED) {
                        ny->SetState(Chunk::State::GENERATED);
                        m_requests.push({Req::Type::MESH, ny->GetPos()});
                    }

                    if (py && py->GetState() == Chunk::State::UPLOADED) {
                        py->SetState(Chunk::State::GENERATED);
                        m_requests.push({Req::Type::MESH, py->GetPos()});
                    }

                }
            } else if (r.type == Req::Type::MESH) {
                if (!c) continue;
                if(c->GetState() != Chunk::State::GENERATED && c->GetState() != Chunk::State::UPLOADED) {
                    continue;
                }

                if ((pz && pz->GetState() == Chunk::State::UNLOADED) ||
                    (nz && nz->GetState() == Chunk::State::UNLOADED) ||
                    (px && px->GetState() == Chunk::State::UNLOADED) ||
                    (nx && nx->GetState() == Chunk::State::UNLOADED) ||
                    (ny && ny->GetState() == Chunk::State::UNLOADED) ||
                    (py && py->GetState() == Chunk::State::UNLOADED)) 
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

static constexpr int RAD = 8;

void MeshingEngine::UnloadFarChunks(const glm::ivec3 &cam_chunk) {
    std::lock_guard lock(m_chunks_mutex);

    for (auto it = m_chunks.begin(); it != m_chunks.end(); ) {
        const glm::ivec3 &coord = it->first;

        const int dx = coord.x - cam_chunk.x;
        int dy = coord.y - cam_chunk.y;
        const int dz = coord.z - cam_chunk.z;

        if (std::abs(dx) > RAD || std::abs(dz) > RAD || std::abs(dy) > RAD) {
            auto chunk = it->second;
            if (chunk->GetState() == Chunk::State::BUILDING) {
                continue;
            }

            glm::ivec3 pos = it->first;
            it = m_chunks.erase(it);

            glm::ivec3 offsets[6] = {{1,0,0}, {-1,0,0}, {0,0,1}, {0,0,-1}, {0,1,0}, {0,-1,0}};
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

                    m_verts.push_back({xf,     yf,     zf + 1, du, 0, 0.8});
                    m_verts.push_back({xf + 1, yf,     zf + 1, t+du, 0, 0.8});
                    m_verts.push_back({xf + 1, yf + 1, zf + 1, t+du, t, 0.8});
                    m_verts.push_back({xf,     yf + 1, zf + 1, du, t, 0.8});

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

                    m_verts.push_back({xf + 1, yf,     zf, du, 0, 0.8});
                    m_verts.push_back({xf,     yf,     zf, t+du, 0, 0.8});
                    m_verts.push_back({xf,     yf + 1, zf, t+du, t, 0.8});
                    m_verts.push_back({xf + 1, yf + 1, zf, du, t, 0.8});

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
                        m_verts.push_back({xf,     yf + 1, zf + 1, 0, 0.5f, 1});
                        m_verts.push_back({xf + 1, yf + 1, zf + 1, 0.5f, 0.5f, 1});
                        m_verts.push_back({xf + 1, yf + 1, zf,     0.5f, 1.f, 1});
                        m_verts.push_back({xf,     yf + 1, zf,     0, 1.f, 1});
                    } else {
                        m_verts.push_back({xf,     yf + 1, zf + 1, du, 0, 1});
                        m_verts.push_back({xf + 1, yf + 1, zf + 1, t+du, 0, 1});
                        m_verts.push_back({xf + 1, yf + 1, zf,     t+du, t, 1});
                        m_verts.push_back({xf,     yf + 1, zf,     du, t, 1});
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

                    m_verts.push_back({xf,     yf, zf,     du, 0, 0.4});
                    m_verts.push_back({xf + 1, yf, zf,     t+du, 0, 0.4});
                    m_verts.push_back({xf + 1, yf, zf + 1, t+du, t, 0.4});
                    m_verts.push_back({xf,     yf, zf + 1, du, t, 0.4});

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

                    m_verts.push_back({xf + 1, yf,     zf + 1, du, 0, 0.6});
                    m_verts.push_back({xf + 1, yf,     zf,     t+du, 0, 0.6});
                    m_verts.push_back({xf + 1, yf + 1, zf,     t+du, t, 0.6});
                    m_verts.push_back({xf + 1, yf + 1, zf + 1, du, t, 0.6});

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

                    m_verts.push_back({xf, yf,     zf,     du, 0, 0.6});
                    m_verts.push_back({xf, yf,     zf + 1, t+du, 0, 0.6});
                    m_verts.push_back({xf, yf + 1, zf + 1, t+du, t, 0.6});
                    m_verts.push_back({xf, yf + 1, zf,     du, t, 0.6});

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

bool MeshingEngine::CheckCollision(glm::vec3 pos, glm::vec3 size) {
    const auto minAABB = pos - glm::vec3{size.x / 2.f, 0, size.z / 2.f};
    const auto maxAABB = pos + glm::vec3{size.x / 2.f, size.y, size.z / 2.f};

    const int minX = static_cast<int>(std::floor(minAABB.x));
    const int maxX = static_cast<int>(std::floor(maxAABB.x));
    const int minY = static_cast<int>(std::floor(minAABB.y));
    const int maxY = static_cast<int>(std::floor(maxAABB.y));
    const int minZ = static_cast<int>(std::floor(minAABB.z));
    const int maxZ = static_cast<int>(std::floor(maxAABB.z));

    for (int x = minX; x <= maxX; ++x) {
        for (int y = minY; y <= maxY; ++y) {
            for (int z = minZ; z <= maxZ; ++z) {
                if (get_block(glm::vec3(x, y, z)) != Chunk::Block::AIR) {
                    return true;
                }
            }
        }
    }

    return false;
}

Chunk::Block MeshingEngine::get_block(glm::vec3 pos) {
    std::shared_ptr<Chunk> chunk;
    {
        std::lock_guard lock(m_chunks_mutex);
        chunk = get_chunk(glm::ivec3{
            static_cast<int>(std::floor(pos.x / CHUNK_WIDTH)),
            static_cast<int>(std::floor(pos.y / CHUNK_HEIGHT)),
            static_cast<int>(std::floor(pos.z / CHUNK_DEPTH))
        });
    }

    if (!chunk || chunk->GetState() == Chunk::State::UNLOADED) {
        return Chunk::Block::AIR;
    }

    glm::ivec3 local = {
        static_cast<int>(std::floor(pos.x)) % CHUNK_WIDTH,
        static_cast<int>(std::floor(pos.y)) % CHUNK_HEIGHT,
        static_cast<int>(std::floor(pos.z)) % CHUNK_DEPTH
    };

    if (local.x < 0) local.x += CHUNK_WIDTH;
    if (local.y < 0) local.y += CHUNK_HEIGHT;
    if (local.z < 0) local.z += CHUNK_DEPTH;

    return chunk->GetBlock(local);
}

void MeshingEngine::SetBlock(glm::vec3 pos, Chunk::Block block) {
    std::shared_ptr<Chunk> chunk;
    {
        std::lock_guard lock(m_chunks_mutex);
        chunk = get_chunk(glm::ivec3{
            static_cast<int>(std::floor(pos.x / CHUNK_WIDTH)),
            static_cast<int>(std::floor(pos.y / CHUNK_HEIGHT)),
            static_cast<int>(std::floor(pos.z / CHUNK_DEPTH))
        });
    }

    if (!chunk || chunk->GetState() == Chunk::State::UNLOADED) {
        return;
    }

    glm::ivec3 local = {
        static_cast<int>(std::floor(pos.x)) % CHUNK_WIDTH,
        static_cast<int>(std::floor(pos.y)) % CHUNK_HEIGHT,
        static_cast<int>(std::floor(pos.z)) % CHUNK_DEPTH
    };

    if (local.x < 0) local.x += CHUNK_WIDTH;
    if (local.y < 0) local.y += CHUNK_HEIGHT;
    if (local.z < 0) local.z += CHUNK_DEPTH;

    chunk->Place(local, block);
}


bool MeshingEngine::Raycast(glm::vec3 start, glm::vec3 dir, glm::vec3 &end, glm::vec3 &prev) {
    glm::ivec3 map_pos = glm::ivec3(std::floor(start.x), std::floor(start.y), std::floor(start.z));

    glm::vec3 t_delta = glm::abs(1.0f / dir);

    glm::ivec3 step;

    glm::vec3 t_max;

    for (int i = 0; i < 3; ++i) {
        if (dir[i] > 0) {
            step[i] = 1;
            t_max[i] = (std::floor(start[i]) + 1.0f - start[i]) * t_delta[i];
        } else if (dir[i] < 0) {
            step[i] = -1;
            t_max[i] = (start[i] - std::floor(start[i])) * t_delta[i];
        } else {
            step[i] = 0;
            t_max[i] = std::numeric_limits<float>::infinity();
        }
    }

    float dist = 0.0f;
    static constexpr float max_dist = 10;

    prev = map_pos;

    while (dist < max_dist) {
        if (get_block(glm::vec3(map_pos.x, map_pos.y, map_pos.z)) != Chunk::Block::AIR) {
            end = map_pos;
            return true;
        }

        prev = map_pos;

        if (t_max.x < t_max.y) {
            if (t_max.x < t_max.z) {
                map_pos.x += step.x;
                dist = t_max.x;
                t_max.x += t_delta.x;
            } else {
                map_pos.z += step.z;
                dist = t_max.z;
                t_max.z += t_delta.z;
            }
        } else {
            if (t_max.y < t_max.z) {
                map_pos.y += step.y;
                dist = t_max.y;
                t_max.y += t_delta.y;
            } else {
                map_pos.z += step.z;
                dist = t_max.z;
                t_max.z += t_delta.z;
            }
        }
    }

    return false;
}