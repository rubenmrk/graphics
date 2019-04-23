#pragma once

#include <cassert>
#include <string>
#include <vector>
#include <fstream>
#include <type_traits>
#include <glm/glm.hpp>

namespace game
{
    struct meshfile
    {
        struct header
        {
            unsigned long long dataCount;

            unsigned long long indexCount;
        };

        struct vertexData
        {
            bool operator==(const vertexData& rhs) const
            {
                return position == rhs.position && texcoord == rhs.texcoord && normal == rhs.normal;
            }

            bool operator!=(const vertexData& rhs) const
            {
                return !(*this == rhs);
            }

            glm::vec3 position;

            glm::vec2 texcoord;

            glm::vec3 normal;
        };

        meshfile()
            : head({0, 0}), data(nullptr), indices(nullptr)
        {
        }

        meshfile(std::string_view path)
        {
            std::ifstream in(path.data(), std::ifstream::in | std::ifstream::binary);
            if (!in.is_open())
                throw std::runtime_error("unable to open model file");
            
            in.read(reinterpret_cast<char*>(&head), sizeof(head));
            data = new vertexData[head.dataCount];
            indices = new unsigned int[head.indexCount];

            in.read(reinterpret_cast<char*>(data), head.dataCount*sizeof(vertexData));
            in.read(reinterpret_cast<char*>(indices), head.indexCount*sizeof(unsigned int));

            in.close();
        }

        meshfile(const meshfile& rhs)
        {
            if (rhs.head.dataCount) {
                data = new vertexData[rhs.head.dataCount];
                indices = new unsigned int[rhs.head.indexCount];
                std::copy(rhs.data, rhs.data+rhs.head.dataCount, data);
                std::copy(rhs.indices, rhs.indices+rhs.head.indexCount, indices);
            }
            else {
                head = {0, 0};
                data = nullptr;
                indices = nullptr;
            }
        }

        meshfile(meshfile&& rhs) noexcept
        {
            head = rhs.head;
            rhs.head = {0, 0};
            data = rhs.data;
            rhs.data = nullptr;
            indices = rhs.indices;
            rhs.indices = nullptr;
        }

        ~meshfile()
        {
            delete[] data;
            delete[] indices;
        }

        meshfile& operator=(const meshfile& rhs)
        {
            if (head.dataCount != rhs.head.dataCount) {
                head.dataCount = rhs.head.dataCount;
                delete[] data;
                data = new vertexData[head.dataCount];
            }
            std::copy(rhs.data, rhs.data+head.dataCount, data);

            if (head.indexCount != rhs.head.indexCount) {
                head.indexCount = rhs.head.indexCount;
                delete[] indices;
                indices = new unsigned int[head.indexCount];
            }
            std::copy(rhs.indices, rhs.indices+head.indexCount, indices);
            return *this;
        }

        meshfile& operator=(meshfile&& rhs) noexcept
        {
            assert(this != &rhs);
            delete[] data;
            delete[] indices;

            head.dataCount = rhs.head.dataCount;
            head.indexCount = rhs.head.indexCount;
            data = rhs.data;
            indices = rhs.indices;
            
            rhs.head.dataCount = 0;
            rhs.head.indexCount = 0;
            rhs.data = nullptr;
            rhs.indices = nullptr;
            return *this;
        }

        void toFile(std::string_view path)
        {
            std::ofstream out(path.data(), std::ofstream::out | std::ofstream::trunc | std::ofstream::binary);
            
            out.write(reinterpret_cast<char*>(&head), sizeof(head));
            out.write(reinterpret_cast<char*>(data), head.dataCount * sizeof(vertexData));
            out.write(reinterpret_cast<char*>(indices), head.indexCount * sizeof(unsigned int));

            out.close();
        }

        template<class VIO>
        static constexpr bool sameVIO()
        {
            return (VIO::positionTrait && !VIO::colorTrait && VIO::texCoordTrait && VIO::normalTrait);
        }

        template<class VIO>
        void toVertexInputObject(std::vector<VIO>& out, bool indexed = true)
        {
            out.reserve(head.dataCount);

            if (indexed) {
                for (auto i = 0; i < head.dataCount; ++i) {
                    VIO vio;

                    if constexpr (VIO::positionTrait)
                        vio.inputPosition = data[i].position;
                    
                    if constexpr (VIO::colorTrait)
                        vio.inputColor = glm::vec3(0.0f, 1.0f, 0.0f);

                    if constexpr (VIO::texCoordTrait)
                        vio.inputTexCoord = data[i].texcoord;
                    
                    if constexpr (VIO::normalTrait)
                        vio.inputNormal = data[i].normal;

                    out.push_back(vio);
                }
            }
            else {
                for (auto i = 0; i < head.indexCount; ++i) {
                    VIO vio;

                    if constexpr (VIO::positionTrait)
                        vio.inputPosition = data[indices[i]].position;
                    
                    if constexpr (VIO::colorTrait)
                        vio.inputColor = glm::vec3(0.0f, 1.0f, 0.0f);

                    if constexpr (VIO::texCoordTrait)
                        vio.inputTexCoord = data[indices[i]].texcoord;
                    
                    if constexpr (VIO::normalTrait)
                        vio.inputNormal = data[indices[i]].normal;

                    out.push_back(vio);
                }
            }
        }

        template<typename VI>
        static constexpr bool sameVI()
        {
            return std::is_same_v<VI, unsigned int>;
        }

        template<typename VI>
        void toVertexIndex(std::vector<VI>& out)
        {
            out.reserve(head.indexCount);
            for (auto i = 0; i < head.indexCount; ++i) {
                out.push_back(indices[i]);
            }
        }

        header head;

        vertexData *data;

        unsigned int *indices;
    };
}