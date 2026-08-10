#include "MeshingEngine.hpp"

void MeshingEngine::req_internal(glm::ivec3 pos) {
    {
        if (m_chunks.find(pos) == m_chunks.end()) {
            auto chunk = std::make_shared<Chunk>(pos);
            chunk->GenTerrain();
            m_chunks.emplace(pos, chunk);
        } else {
            if (!m_chunks[pos]->IsBuilt()) {
                return;
            }
        }

        {
            std::lock_guard<std::mutex> lock(m_requests_mutex);
            m_requests.push(pos);
        }
    }

    m_requests_cv.notify_one();
}

void MeshingEngine::Request(glm::ivec3 pos) {
    {
        std::lock_guard<std::mutex> lock(m_chunks_mutex);
        if (m_chunks.find(pos) != m_chunks.end()) {
            return;
        }

        auto chunk = std::make_shared<Chunk>(pos);
        chunk->GenTerrain();
        m_chunks.emplace(pos, chunk);

        {
            std::lock_guard<std::mutex> lock(m_requests_mutex);
            m_requests.push(pos);
        }
    }

    m_requests_cv.notify_one();
}

void MeshingEngine::worker() {
    while (true) {
        {
            glm::ivec3 pos;
            {
                std::unique_lock<std::mutex> lock(m_requests_mutex);
                if (m_requests.empty()) {
                    m_requests_cv.wait(lock, [this] { return !m_requests.empty(); });
                }
                pos = m_requests.front();
                m_requests.pop();
                std::cout << "Worker thread processing chunk at (" << pos.x << ", " << pos.y << ", " << pos.z << ")" << std::endl;
            }

            std::shared_ptr<Chunk> chunk;
            {
                std::lock_guard<std::mutex> lock(m_chunks_mutex);
                //m_chunks[pos]->BuildMesh();
                chunk = m_chunks[pos];
                //m_chunks[pos]->Upload();
            }
            build_mesh(chunk);

            {
                std::lock_guard<std::mutex> results_lock(m_results_mutex);
                m_results.push(m_chunks[pos]);
            }
        }
    }
}


void MeshingEngine::UnloadFarChunks(const glm::ivec3 &cam_chunk) {
    std::lock_guard<std::mutex> lock(m_chunks_mutex);

    for (auto it = m_chunks.begin(); it != m_chunks.end(); ) {
        const glm::ivec3& coord = it->first;

        int dx = coord.x - cam_chunk.x;
        int dy = coord.y - cam_chunk.y;
        int dz = coord.z - cam_chunk.z;

        bool tooFar =
            std::abs(dx) > Render::RADIUS ||
            std::abs(dz) > Render::RADIUS;
        if (tooFar) {
            glm::ivec3 pos = it->first;
            std::cout << "Unloading chunk at (" << pos.x << ", " << pos.y << ", " << pos.z << ")" << std::endl;
            it = m_chunks.erase(it);

            glm::ivec3 offsets[4] = {{1,0,0}, {-1,0,0}, {0,0,1}, {0,0,-1}};
            for (auto& offset : offsets) {
                /*
                auto neighbor = GetChunk(pos + offset);
                if (neighbor && neighbor->IsBuilt()) {
                    m_unbuilt_chunks.push(neighbor);
                }
               */
               std::cout << "Requesting chunk at (" << pos.x + offset.x << ", " << pos.y + offset.y << ", " << pos.z + offset.z << ")" << std::endl;
               req_internal(pos + offset);
            }
        } else {
            ++it;
        }
    }
}

void MeshingEngine::Draw(Shader &shader) {
    std::lock_guard<std::mutex> lock(m_chunks_mutex);

    for (auto &[coord, c] : m_chunks) {
        if (c->IsBuilt() && c->IsUploaded()) {
            c->Draw(shader);
        }
    }
}


void MeshingEngine::build_mesh(std::shared_ptr<Chunk> chunk) {
    auto &m_verts = chunk->GetVerts();
    auto &m_indices = chunk->GetIndices();

    m_verts.clear();
    m_indices.clear();

    for (int x = 0; x < CHUNK_WIDTH; x++) {
        for (int y = 0; y < CHUNK_HEIGHT; y++) {
            for (int z = 0; z < CHUNK_DEPTH; z++) {

                auto b = chunk->GetBlock(glm::ivec3{x, y, z});
                if (b == Block::AIR) 
                    continue;

                int xr = x+static_cast<int>(chunk->GetPos().x) * CHUNK_WIDTH;
                int yr = y+static_cast<int>(chunk->GetPos().y) * CHUNK_HEIGHT;
                int zr = z+static_cast<int>(chunk->GetPos().z) * CHUNK_DEPTH;

                float t = 0.5;
                float du = (b == Block::STONE) ? 0.5f : 0;

                bool draw_PZ, draw_NZ, draw_PY, draw_NY, draw_PX, draw_NX;
                if (z < CHUNK_DEPTH - 1) {
                    draw_PZ = (chunk->GetBlock(glm::ivec3{x, y, z + 1}) == Block::AIR);
                } else {
                    draw_PZ = (this->GetBlock(glm::ivec3{xr, yr, zr + 1}) == Block::AIR);
                }

                if (z > 0) {
                    draw_NZ = (chunk->GetBlock(glm::ivec3{x, y, z - 1}) == Block::AIR);
                } else {
                    draw_NZ = (this->GetBlock(glm::ivec3{xr, yr, zr - 1}) == Block::AIR);
                }

                if (y < CHUNK_HEIGHT - 1) {
                    draw_PY = (chunk->GetBlock(glm::ivec3{x, y + 1, z}) == Block::AIR);
                } else {
                    draw_PY = (this->GetBlock(glm::ivec3{xr, yr + 1, zr}) == Block::AIR);
                }

                if (y > 0) {
                    draw_NY = (chunk->GetBlock(glm::ivec3{x, y - 1, z}) == Block::AIR);
                } else {
                    draw_NY = (this->GetBlock(glm::ivec3{xr, yr - 1, zr}) == Block::AIR);
                }

                if (x < CHUNK_WIDTH - 1) {
                    draw_PX = (chunk->GetBlock(glm::ivec3{x + 1, y, z}) == Block::AIR);
                } else {
                    draw_PX = (this->GetBlock(glm::ivec3{xr + 1, yr, zr}) == Block::AIR);
                }

                if (x > 0) {
                    draw_NX = (chunk->GetBlock(glm::ivec3{x - 1, y, z}) == Block::AIR);
                } else {
                    draw_NX = (this->GetBlock(glm::ivec3{xr - 1, yr, zr}) == Block::AIR);
                }


                float xf = static_cast<GLfloat>(x);
                float yf = static_cast<GLfloat>(y);
                float zf = static_cast<GLfloat>(z);

                // Front (+Z)
                if (draw_PZ) {
                    GLuint base = static_cast<GLuint>(m_verts.size());

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
                    
                    GLuint base = static_cast<GLuint>(m_verts.size());

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
                    GLuint base = static_cast<GLuint>(m_verts.size());

                    if (b == Block::GRASS) {
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
                    GLuint base = static_cast<GLuint>(m_verts.size());

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
                    GLuint base = static_cast<GLuint>(m_verts.size());

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
                    GLuint base = static_cast<GLuint>(m_verts.size());

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

    chunk->BuildMesh();
}


// hmmmmmm
Block MeshingEngine::GetBlock(glm::ivec3 pos) {
    std::lock_guard<std::mutex> lock(m_chunks_mutex);

    auto int_floor_div = [](int num, int denom) {
        int res = num / denom;
        int rem = num % denom;
        if (rem != 0 && ((num < 0) ^ (denom < 0))) {
            res -= 1;
        }
        return res;
    };

    glm::ivec3 chunk_coord{
        int_floor_div(pos.x, CHUNK_WIDTH),
        int_floor_div(pos.y, CHUNK_HEIGHT),
        int_floor_div(pos.z, CHUNK_DEPTH)
    };

    auto it = m_chunks.find(chunk_coord);
    if (it == m_chunks.end()) {
        return Block::AIR;
    }

    auto wrapIndex = [](int i, int i_max) {
        return ((i % i_max) + i_max) % i_max;
    };

    glm::ivec3 local_pos{
        wrapIndex(pos.x, CHUNK_WIDTH),
        wrapIndex(pos.y, CHUNK_HEIGHT),
        wrapIndex(pos.z, CHUNK_DEPTH)
    };

    return it->second->GetBlock(local_pos);
}


void MeshingEngine::Upload() {
    std::lock_guard<std::mutex> results_lock(m_results_mutex);
    while (!m_results.empty()) {
        auto chunk = m_results.front();
        m_results.pop();
        chunk->Upload();
    }
}