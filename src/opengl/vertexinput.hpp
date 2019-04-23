#pragma once

#include "glbase.hpp"
#include <type_traits>
#include <vector>

namespace game::opengl
{
	namespace vioBlocks
	{
		struct position
		{
			glm::vec3 inputPosition;

			static constexpr bool positionTrait = true;
		};

		struct emptyPosition
		{
			static constexpr bool positionTrait = false;
		};

		struct color
		{
			glm::vec3 inputColor;

			static constexpr bool colorTrait = true;
		};

		struct emptyColor
		{
			static constexpr bool colorTrait = false;
		};

		struct texCoord
		{
			glm::vec2 inputTexCoord;

			static constexpr bool texCoordTrait = true;
		};

		struct emptyTexCoord
		{
			static constexpr bool texCoordTrait = false;
		};

		struct normal
		{
			glm::vec3 inputNormal;

			static constexpr bool normalTrait = true;
		};

		struct emptyNormal
		{
			static constexpr bool normalTrait = false;
		};
	}

    template<bool p, bool c, bool t, bool n>
    struct vertexInputObject :
        std::conditional_t<p, vioBlocks::position, vioBlocks::emptyPosition>,
        std::conditional_t<c, vioBlocks::color, vioBlocks::emptyColor>,
        std::conditional_t<t, vioBlocks::texCoord, vioBlocks::emptyTexCoord>,
        std::conditional_t<n, vioBlocks::normal, vioBlocks::emptyNormal>
    {
        vertexInputObject(
				const std::conditional_t<p, vioBlocks::position, vioBlocks::emptyPosition>& pb = {},
                const std::conditional_t<c, vioBlocks::color, vioBlocks::emptyColor>& cb = {},
                const std::conditional_t<t, vioBlocks::texCoord, vioBlocks::emptyTexCoord>& tb = {},
                const std::conditional_t<n, vioBlocks::normal, vioBlocks::emptyNormal>& nb = {}
            ) :
            std::conditional_t<p, vioBlocks::position, vioBlocks::emptyPosition>(pb),
            std::conditional_t<c, vioBlocks::color, vioBlocks::emptyColor>(cb),
            std::conditional_t<t, vioBlocks::texCoord, vioBlocks::emptyTexCoord>(tb),
            std::conditional_t<n, vioBlocks::normal, vioBlocks::emptyNormal>(nb)
        {
        }

        vertexInputObject(const vertexInputObject& rhs)
        {
            if constexpr (p)
                vioBlocks::position::inputPosition = rhs.inputPosition;
            if constexpr (c)
                vioBlocks::color::inputColor = rhs.inputColor;
            if constexpr (t)
                vioBlocks::texCoord::inputTexCoord = rhs.inputTexCoord;
            if constexpr (n)
                vioBlocks::normal::inputNormal = rhs.inputNormal;
        }

        vertexInputObject(vertexInputObject&& rhs) noexcept
        {
            if constexpr (p)
                vioBlocks::position::inputPosition = std::move(rhs.inputPosition);
            if constexpr (c)
                vioBlocks::color::inputColor = std::move(rhs.inputColor);
            if constexpr (t)
                vioBlocks::texCoord::inputTexCoord = std::move(rhs.inputTexCoord);
            if constexpr (n)
                vioBlocks::normal::inputNormal = std::move(rhs.inputNormal);
        }

        ~vertexInputObject()
        {
        }

		vertexInputObject& operator=(const vertexInputObject& rhs)
		{
			if constexpr (p)
				vioBlocks::position::inputPosition = rhs.inputPosition;
			if constexpr (c)
				vioBlocks::color::inputColor = rhs.inputColor;
			if constexpr (t)
				vioBlocks::texCoord::inputTexCoord = rhs.inputTexCoord;
			if constexpr (n)
				vioBlocks::normal::inputNormal = rhs.inputNormal;
			return *this;
		}

		vertexInputObject& operator=(vertexInputObject&& rhs) noexcept
		{
			if constexpr (p)
				vioBlocks::position::inputPosition = std::move(rhs.inputPosition);
			if constexpr (c)
				vioBlocks::color::inputColor = std::move(rhs.inputColor);
			if constexpr (t)
				vioBlocks::texCoord::inputTexCoord = std::move(rhs.inputTexCoord);
			if constexpr (n)
				vioBlocks::normal::inputNormal = std::move(rhs.inputNormal);
			return *this;
		}

        bool operator==(const vertexInputObject& rhs) const
        {
            if constexpr (p) {
                if (vioBlocks::position::inputPosition != rhs.inputPosition)
                    return false;
            }
            if constexpr (c) {
                if (vioBlocks::color::inputColor != rhs.inputColor)
                    return false;
            }
            if constexpr (t) {
                if (vioBlocks::texCoord::inputTexCoord != rhs.inputTexCoord)
                    return false;
            }
			if constexpr (n) {
				if (vioBlocks::normal::inputNormal != rhs.inputNormal)
					return false;
			}
            return true;
        }

		bool operator!=(const vertexInputObject& rhs) const
		{
			return !(*this == rhs);
		}
    };

    template<class VIO>
    class VAO
    {
    public:

        operator GLuint()
        {
            return _vao;
        }

        VAO()
        {
            glCreateVertexArrays(1, &_vao);
            if (!_vao)
                throw exception(except_e::GRAPHICS_BASE, "glCreateVertexArrays");

            if constexpr (VIO::positionTrait) {
                constexpr GLuint binding = 0;
                glEnableVertexArrayAttrib(_vao, binding);
                glVertexArrayAttribFormat(_vao, binding, 3, GL_FLOAT, GL_FALSE, 0);
                glVertexArrayAttribBinding(_vao, binding, binding);
            }
            if constexpr (VIO::colorTrait) {
                constexpr GLuint binding = VIO::positionTrait;
                glEnableVertexArrayAttrib(_vao, binding);
                glVertexArrayAttribFormat(_vao, binding, 3, GL_FLOAT, GL_FALSE, 0);
                glVertexArrayAttribBinding(_vao, binding, binding);
            }
            if constexpr (VIO::texCoordTrait) {
                constexpr GLuint binding = VIO::positionTrait + VIO::colorTrait;
                glEnableVertexArrayAttrib(_vao, binding);
                glVertexArrayAttribFormat(_vao, binding, 2, GL_FLOAT, GL_TRUE, 0);
                glVertexArrayAttribBinding(_vao, binding, binding);
            }
			if constexpr (VIO::normalTrait) {
				constexpr GLuint binding = VIO::positionTrait + VIO::colorTrait + VIO::texCoordTrait;
				glEnableVertexArrayAttrib(_vao, binding);
				glVertexArrayAttribFormat(_vao, binding, 3, GL_FLOAT, GL_TRUE, 0);
				glVertexArrayAttribBinding(_vao, binding, binding);
			}
        }

        VAO(const VAO& rhs) = delete;

        VAO(VAO&& rhs) noexcept
        {
            _vao = rhs._vao;
            rhs._vao = 0;
        }

        ~VAO()
        {
            if (_vao)
                glDeleteVertexArrays(1, &_vao);
        }

        template<class Buffer>
        void bind(Buffer& buffer)
        {
            constexpr GLsizei stride = VIO::positionTrait*sizeof(glm::vec3) + VIO::colorTrait*sizeof(glm::vec3) + VIO::texCoordTrait*sizeof(glm::vec2)
				+ VIO::normalTrait*sizeof(glm::vec3);

            if constexpr (VIO::positionTrait) {
                constexpr GLuint binding = 0;
                constexpr GLintptr offset = 0;
                glVertexArrayVertexBuffer(_vao, binding, buffer.getVertexInputBuffer(), offset, stride);
            }
            if constexpr (VIO::colorTrait) {
                constexpr GLuint binding = VIO::positionTrait;
                constexpr GLintptr offset = VIO::positionTrait*sizeof(glm::vec3);
                glVertexArrayVertexBuffer(_vao, binding, buffer.getVertexInputBuffer(), offset, stride);
            }
            if constexpr (VIO::texCoordTrait) {
                constexpr GLuint binding = VIO::positionTrait + VIO::colorTrait;
                constexpr GLintptr offset = VIO::positionTrait*sizeof(glm::vec3) + VIO::colorTrait*sizeof(glm::vec3);
                glVertexArrayVertexBuffer(_vao, binding, buffer.getVertexInputBuffer(), offset, stride);
            }
			if constexpr (VIO::normalTrait) {
				constexpr GLuint binding = VIO::positionTrait + VIO::colorTrait + VIO::texCoordTrait;
				constexpr GLintptr offset = VIO::positionTrait*sizeof(glm::vec3) + VIO::colorTrait*sizeof(glm::vec3) + VIO::texCoordTrait*sizeof(glm::vec2);
				glVertexArrayVertexBuffer(_vao, binding, buffer.getVertexInputBuffer(), offset, stride);
			}

            if constexpr (Buffer::indexedTrait) {
                glVertexArrayElementBuffer(_vao, buffer.getVertexIndexBuffer());
            }
        }

    private:

        GLuint _vao = 0;
    };
}