#pragma once

#include "glbase.hpp"
#include <vector>

namespace game::opengl
{
    class buffer
    {
    public:

        GLuint getVertexInputBuffer()
        {
            return _buffer;
        }

        buffer(const void *data, GLsizeiptr size)
        {
            glCreateBuffers(1, &_buffer);
            if (!_buffer)
                throw exception(except_e::GRAPHICS_BASE, "glCreateBuffers");

            glNamedBufferData(_buffer, size, data, GL_STATIC_DRAW);
        }

        buffer(const buffer& rhs) = delete;

        buffer(buffer&& rhs) noexcept
        {
            _buffer = rhs._buffer;
            rhs._buffer = 0;
        }

        ~buffer()
        {
            if (_buffer)
                glDeleteBuffers(1, &_buffer);
        }

        static constexpr bool indexedTrait = false;

    private:

        GLuint _buffer = 0;
    };

    class indexedBuffer
    {
    public:

        GLuint getVertexInputBuffer()
        {
            return _buffer[0];
        }

        GLuint getVertexIndexBuffer()
        {
            return _buffer[1];
        }

        indexedBuffer(const void *data, GLsizeiptr dsize, const void *indices, GLsizeiptr isize)
        {
            glCreateBuffers(2, _buffer);
            if (!_buffer[1])
                throw exception(except_e::GRAPHICS_BASE, "glCreateBuffers");
            
            glNamedBufferData(_buffer[0], dsize, data, GL_STATIC_DRAW);
            glNamedBufferData(_buffer[1], isize, indices, GL_STATIC_DRAW);
        }

        ~indexedBuffer()
        {
            if (_buffer[0])
                glDeleteBuffers(2, _buffer);
        }

        static constexpr bool indexedTrait = true;

    private:

        GLuint _buffer[2] = {};
    };
}