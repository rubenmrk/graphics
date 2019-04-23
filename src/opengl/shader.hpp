#pragma once

#include "glbase.hpp"
#include <string>
#include <fstream>
#include <iostream>

#ifdef WIN32
#define FORCEINLINE __forceinline
#else
#define FORCEINLINE __attribute__((always_inline))
#endif

namespace game::opengl
{
    template<bool debug = false>
    class shader
    {
    public:

        operator GLuint()
        {
            return _shader;
        }

        shader(std::string_view shader, GLenum type)
        {
            std::string buffer;

            auto readFile = [&](std::string_view name) {
                std::ifstream file(name.data(), std::ifstream::in | std::ifstream::ate);
                auto len = file.tellg();
                file.seekg(0, std::ifstream::beg);
                buffer.resize(len);
                file.read(buffer.data(), len);
                file.close();
                return len;
            };

            auto checkShaderError = [&]() {
                GLint status;
                glGetShaderiv(_shader, GL_COMPILE_STATUS, &status);
                if (status == GL_FALSE) {
                    if (debug) {
                        glGetShaderiv(_shader, GL_INFO_LOG_LENGTH, &status);
                        buffer.resize(status);
                        glGetShaderInfoLog(_shader, status, nullptr, buffer.data());
                        std::cerr << buffer;
                    }
                    throw exception(except_e::GRAPHICS_BASE, "glCompileShader");
                }
            };

            auto compileShader = [&](GLuint &binding, std::string_view name) {
                binding = glCreateShader(type); 
				if (binding == 0)
					throw exception(except_e::GRAPHICS_BASE, "glCreateShader");
                auto len = static_cast<GLint>(readFile(name));
                auto ptr = buffer.c_str();
                glShaderSource(binding, 1, &ptr, &len);
                glCompileShader(binding);
                checkShaderError();
            };

            constexpr auto folder = "./shaders/";
            compileShader(_shader, std::string(folder).append(shader));
        }

        shader(const shader& rhs) = delete;

        shader(shader&& rhs) noexcept
        {
            _shader = rhs._shader;
            rhs._shader = 0;
        }

        ~shader()
        {
            if (_shader)
                glDeleteShader(_shader);
        }

    private:

        GLuint _shader = 0;
    };

    template<class UBO, bool debug = false>
    class program
    {
    public:

        operator GLuint()
        {
            return _program;
        }

        program(std::string_view basename)
            : _vertex(std::string(basename).append("-vert.glsl"), GL_VERTEX_SHADER), 
            _fragment(std::string(basename).append("-frag.glsl"), GL_FRAGMENT_SHADER)
        {
            auto checkProgramError = [&]() {
                std::string buffer;
                GLint status;
                glGetProgramiv(_program, GL_LINK_STATUS, &status);
                if (status == GL_FALSE) {
                    if (debug) {
                        glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &status);
                        buffer.resize(status);
                        glGetProgramInfoLog(_program, status, nullptr, buffer.data());
                        std::cerr << buffer;
                    }
                    throw exception(except_e::GRAPHICS_BASE, "glLinkProgram");
                }
            };

            _program = glCreateProgram();
            if (!_program)
                throw exception(except_e::GRAPHICS_BASE, "glCreateProgram");

            glAttachShader(_program, _vertex);
            glAttachShader(_program, _fragment);

            glLinkProgram(_program);
            checkProgramError();
        }

        program(const program& rhs) = delete;

        program(program&& rhs) noexcept
			: _vertex(std::move(rhs._vertex)), 
            _fragment(std::move(rhs._fragment))
        {
        }

        ~program()
        {
            if (_program)
                glDeleteProgram(_program);
        }

		template<class... Args>
		void FORCEINLINE updateUbo(Args&&... args)
		{
			UBO::update(_program, std::forward<Args>(args)...);
		}

		template<class... Args>
		void FORCEINLINE updateUboIgnorant(Args&&... args)
		{
			UBO::updateIgnorant(_program, std::forward<Args>(args)...);
		}

    private:

        GLuint _program = 0;

        shader<debug> _vertex;

        shader<debug> _fragment;
    };
}