#pragma once

#include "application/mesh.hpp"
#include "opengl/glbase.hpp"
#include "opengl/buffer.hpp"
#include "opengl/texture.hpp"
#include "opengl/vertexinput.hpp"

namespace game::opengl
{
    template<class VIO, typename VI, bool indexed>
    class modelBase
    {
        using bufferType = std::conditional_t<indexed, indexedBuffer, buffer>;

    public:

        modelBase(std::string_view name)
            : _vao()
        {
            // Load the mesh 
            meshfile mesh(std::string(_dir).append(name).append(".msh"));

            // Convert the mesh into a suitable VIO (and VI)
            if constexpr (indexed) {
                std::vector<VIO> vio;
                std::vector<VI> vi;
                void *cvio, *cvi;
                GLsizeiptr svio, svi;

                if constexpr (meshfile::sameVIO<VIO>()) {
                    cvio = mesh.data;
                    svio = mesh.head.dataCount * sizeof(VIO);
                }
                else {
                    mesh.toVertexInputObject(vio, true);
                    cvio = vio.data();
                    svio = vio.size() * sizeof(VIO);
                }

                if constexpr(meshfile::sameVI<VI>()) {
                    cvi = mesh.indices;
                    svi = mesh.head.indexCount * sizeof(VI);
                }
                else {
                    mesh.toVertexIndex(vi);
                    cvi = vi.data();
                    svi = vi.size() * sizeof(VI);
                }

                _elements = mesh.head.indexCount;
                _buffer = new indexedBuffer(cvio, svio, cvi, svi);
            }
            else {
                std::vector<VIO> vio;
                mesh.toVertexInputObject(vio, false);
                _elements = mesh.head.indexCount;
                _buffer = new buffer(vio.data(), vio.size()*sizeof(VIO));
            }

            _vao.bind(*_buffer);
        }

        modelBase(void *cvio, VI viocount, void *cvi = nullptr, VI vicount = 0)
        {
            if constexpr (indexed) {
                _elements = vicount;
                _buffer = new indexedBuffer(cvio, viocount*sizeof(VIO), cvi, vicount*sizeof(VI));
            }
            else {
                _elements = viocount;
                _buffer = new buffer(cvio, viocount*sizeof(VIO));
            }
        }

        modelBase(const modelBase& rhs) = delete;

        modelBase(modelBase&& rhs) noexcept
            : _vao(std::move(rhs._vao))
        {
            _elements = rhs._elements;
            rhs._elements = 0;
            _buffer = rhs._buffer;
            rhs._buffer = nullptr;
        }

        ~modelBase()
        {
            delete _buffer;
        }

        void render()
        {
            glBindVertexArray(_vao);
            
            if constexpr (indexed) {
                if constexpr (std::is_same_v<VI, GLuint>)
                    glDrawElements(GL_TRIANGLES, _elements, GL_UNSIGNED_INT, nullptr);
                else if constexpr (std::is_same_v<VI, GLshort>)
                    glDrawElements(GL_TRIANGLES, _elements, GL_UNSIGNED_SHORT, nullptr);
                else if constexpr (std::is_same_v<VI, GLubyte>)
                    glDrawElements(GL_TRIANGLES, _elements, GL_UNSIGNED_BYTE, nullptr);
                
                static_assert(std::is_same_v<VI, GLuint> || std::is_same_v<VI, GLshort> || std::is_same_v<VI, GLubyte>, "Illegal index type");
            }
            else {
                glDrawArrays(GL_TRIANGLES, 0, _elements);
            }
        }

    protected:

        VI _elements;

        VAO<VIO> _vao;

        static constexpr std::string_view _dir = "data/models/";

    private:

        bufferType *_buffer = nullptr;
    };
}