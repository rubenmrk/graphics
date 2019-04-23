#pragma once

#include "glbase.hpp"
#include <string>
#include <stb_image.h>

namespace game::opengl
{
    class texture
    {
    public:

        operator GLuint()
        {
            return _texture;
        }

        texture(std::string_view name)
        {
            int width, height, channels;

            stbi_uc *pixels = stbi_load(name.data(), &width, &height, &channels, STBI_rgb_alpha);
            if (!pixels)
                throw exception(except_e::GRAPHICS_BASE, "stbi_load");
            
            glCreateTextures(GL_TEXTURE_2D, 1, &_texture);
            if (!_texture)
                throw exception(except_e::GRAPHICS_BASE, "glCreateTextures");

            auto miplevels = static_cast<GLuint>(std::floor(std::log2(std::max(width, height)))) + 1;

            glTextureStorage2D(_texture, miplevels, GL_RGBA8, width, height);
            glTextureSubImage2D(_texture, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
            glTextureParameteri(_texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTextureParameteri(_texture, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTextureParameteri(_texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTextureParameteri(_texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glGenerateTextureMipmap(_texture);

            stbi_image_free(pixels);
        }

        // This function is only meant to be used for font rendering
        texture(const void *data, int width, int height)
        {
            glCreateTextures(GL_TEXTURE_2D, 1, &_texture);
            if (!_texture)
                throw exception(except_e::GRAPHICS_BASE, "glCreateTextures");
            
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glTextureStorage2D(_texture, 1, GL_R8, width, height);
            glTextureSubImage2D(_texture, 0, 0, 0, width, height, GL_RED, GL_UNSIGNED_BYTE, data);
            glTextureParameteri(_texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTextureParameteri(_texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            glTextureParameteri(_texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTextureParameteri(_texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        }

        texture(const texture& rhs) = delete;

        texture(texture&& rhs) noexcept
        {
            _texture = rhs._texture;
            rhs._texture = 0;
        }

        ~texture()
        {
            if (_texture)
                glDeleteTextures(1, &_texture);
        }

    private:

        GLuint _texture = 0;
    };
}