#pragma once

#include "glbase.hpp"
#include <type_traits>
#include <vector>

#ifdef WIN32
#define FORCEINLINE __forceinline
#else
#define FORCEINLINE __attribute__((always_inline))
#endif

template <typename T, bool value>
static typename std::enable_if<value, void>::type FunctionWithReadableErrorMessage()
{
}


namespace game::opengl
{
	namespace uboBlocks
	{
		template<class T>
		inline static void updateInternal(GLuint program, GLint location, const T& value)
		{
			if constexpr (std::is_same_v<T, glm::mat4>)
				glProgramUniformMatrix4fv(program, location, 1, GL_FALSE, glm::value_ptr(value));

			else if constexpr (std::is_same_v<T, glm::mat3>)
				glProgramUniformMatrix3fv(program, location, 1, GL_FALSE, glm::value_ptr(value));

			else if constexpr (std::is_same_v<T, glm::vec3>)
				glProgramUniform3fv(program, location, 1, glm::value_ptr(value));

			else if constexpr (std::is_same_v<T, GLint>)
				glProgramUniform1i(program, location, value);

			else if constexpr (std::is_same_v<T, GLfloat>)
				glProgramUniform1f(program, location, value);
			
			static_assert(std::is_same_v<T, glm::mat4> || std::is_same_v<T, glm::mat3> || std::is_same_v<T, glm::vec3> || std::is_same_v<T, GLint>
				|| std::is_same_v<T, GLfloat>, "Unkown type");
		}

		template<class Block>
		inline static void update(GLuint program, const typename Block::type& value)
		{
			auto location = glGetUniformLocation(program, Block::name);
			if (location == -1)
				throw exception(except_e::GRAPHICS_BASE, "glGetUniformLocation");
			
			updateInternal(program, location, value);
		}

		struct mvp
		{
			using type = glm::mat4;

			static constexpr auto mvpTrait = true;

			static constexpr auto name = "mvpMatrix";
		};
		struct emptyMvp
		{
			static constexpr bool mvpTrait = false;
		};

		struct viewSpace
		{
			using type = glm::mat4;

			static constexpr auto viewSpaceTrait = true;

			static constexpr auto name = "viewSpaceMatrix";
		};
		struct emptyViewSpace
		{
			static constexpr auto viewSpaceTrait = false;
		};

		struct normalViewSpace
		{
			using type = glm::mat3;

			static constexpr auto normalViewSpaceTrait = true;

			static constexpr auto name = "normalViewSpaceMatrix";
		};
		struct emptyNormalViewSpace
		{
			static constexpr auto normalViewSpaceTrait = false;
		};

		struct texture
		{
			using type = GLint;

			static constexpr auto textureTrait = true;

			static constexpr auto name = "textureColor";
		};
		struct emptyTexture
		{
			static constexpr auto textureTrait = false;
		};

		struct textColor
		{
			using type = glm::vec3;

			static constexpr auto textTrait = true;

			static constexpr auto name = "textColor";
		};
		struct emptyTextColor
		{
			static constexpr auto textTrait = false;
		};

		struct ambientColor
		{
			using type = glm::vec3;

			static constexpr auto ambientTrait = true;

			static constexpr auto name = "ambient";
		};
		struct emptyAmbientColor
		{
			static constexpr auto ambientTrait = false;
		};

		struct Material
		{
			struct diffuse
			{
				using type = GLint;

				static constexpr auto name = "material.diffuse";
			} dif;

			struct specular
			{
				using type = GLint;

				static constexpr auto name = "material.specular";
			} spec;

			struct shininess
			{
				using type = GLfloat;

				static constexpr auto name = "material.shininess";
			} shin;

			static constexpr auto MaterialTrait = true;

			template<class T>
			static void update(GLuint program, const T& value)
			{
				uboBlocks::update<diffuse>(program, value.diffuse);
				uboBlocks::update<specular>(program, value.specular);
				uboBlocks::update<shininess>(program, value.shininess);
			}
		};
		struct EmptyMaterial
		{
			static constexpr auto MaterialTrait = false;
		};

		struct Light
		{
			struct position
			{
				using type = glm::vec3;

				static constexpr auto name = "light.position";
			} pos;

			struct diffuse
			{
				using type = glm::vec3;

				static constexpr auto name = "light.diffuse";
			} dif;

			struct specular
			{
				using type = glm::vec3;

				static constexpr auto name = "light.specular";
			} spec;

			static constexpr auto LightTrait = true;

			template<class T>
			static void update(GLuint program, const T& value)
			{
				uboBlocks::update<position>(program, value.position);
				uboBlocks::update<diffuse>(program, value.diffuse);
				uboBlocks::update<specular>(program, value.specular);
			}
		};
		struct EmptyLight
		{
			static constexpr auto LightTrait = false;
		};
	}

	/*
	 * Object to update the uniform buffer
	*/
	
    template<bool mvpEnabled, bool vsEnabled, bool tcEnabled, bool txtcEnabled, bool ambientEnabled, bool materialEnabled, bool lightEnabled> 
	struct uniformBufferObject :
		// MVP = Projection * View * Model enabled
        std::conditional_t<mvpEnabled, uboBlocks::mvp, uboBlocks::emptyMvp>,						// index = 0

		// Viewspace = View * model, and the its normal enabled
		std::conditional_t<vsEnabled, uboBlocks::viewSpace, uboBlocks::emptyViewSpace>,				// index = 1
		std::conditional_t<vsEnabled, uboBlocks::normalViewSpace, uboBlocks::emptyNormalViewSpace>,	// index = 1

		// Texture(s) enabled
        std::conditional_t<tcEnabled, uboBlocks::texture, uboBlocks::emptyTexture>,					// index = 2

		// Text color enabled
		std::conditional_t<txtcEnabled, uboBlocks::textColor, uboBlocks::emptyTextColor>,			// index = 3

		// Ambient color enabled
		std::conditional_t<ambientEnabled, uboBlocks::ambientColor, uboBlocks::emptyAmbientColor>,	// index = 4

		// Material struct enabled
		std::conditional_t<materialEnabled, uboBlocks::Material, uboBlocks::EmptyMaterial>,			// index = 5

		// Light struct enabled
		std::conditional_t<lightEnabled, uboBlocks::Light, uboBlocks::EmptyLight>					// index = 6
	{
		template<class T, class... Args>
		static FORCEINLINE void update(GLuint program, const T& arg, Args... args)
		{
			updateInternal<0>(program, arg, args...);
		}

		template<class T>
		static FORCEINLINE void update(GLuint program, const T& arg)
		{
			updateInternal<0>(program, arg);
		}

		template<class T, class... Args>
		static FORCEINLINE void updateIgnorant(GLuint program, const T& arg, Args... args)
		{
			updateInternalIgnorant<0>(program, arg, args...);
		}

		template<class T>
		static FORCEINLINE void updateIgnorant(GLuint program, const T& arg)
		{
			updateInternalIgnorant<0>(program, arg);
		}

	private:

		static constexpr int calcDisabled(int i)
		{
			int result = 0, index = 0;
			
			// MVP
			if (!mvpEnabled) {
				++result;
			}
			else {
				++index;
				if (index == i+1)
					return result;
			}
			
			// Viewspace
			if (!vsEnabled) {
				++result;
			}
			else {
				++index;
				if (index == i+1)
					return result;
			}
			
			// Texture
			if (!tcEnabled) {
				++result;
			}
			else {
				++index;
				if (index == i+1)
					return result;
			}

			// Text color
			if (!txtcEnabled)
				++result;
			else {
				++index;
				if (index == i+1)
					return result;
			}

			// Ambient color
			if (!ambientEnabled)
				++result;
			else {
				++index;
				if (index == i+1)
					return result;
			}

			// Material
			if (!materialEnabled)
				++result;
			else {
				++index;
				if (index == i+1)
					return result;
			}

			// Light
			if (!lightEnabled)
				++result;
			else {
				++index;
				if (index == i+1)
					return result;
			}

			return result;
		}

		template<int I, class T>
		static FORCEINLINE void updateInternalValue(GLuint program, const T& arg)
		{
			static_assert(I < totalElements, "Index out of bounds");

			// MVP
			if constexpr (I == 0 && mvpEnabled)
				uboBlocks::update<uboBlocks::mvp>(program, arg);

			// Viewspace
			else if constexpr (I == 1 && vsEnabled) {
				uboBlocks::update<uboBlocks::viewSpace>(program, arg);
				uboBlocks::update<uboBlocks::normalViewSpace>(program, uboBlocks::normalViewSpace::type(glm::transpose(glm::inverse(arg))));
			}

			// Texture
			else if constexpr (I == 2 && tcEnabled)
				uboBlocks::update<uboBlocks::texture>(program, arg);

			// Text color
			else if constexpr (I == 3 && txtcEnabled)
				uboBlocks::update<uboBlocks::textColor>(program, arg);

			// Ambient color
			else if constexpr (I == 4 && ambientEnabled)
				uboBlocks::update<uboBlocks::ambientColor>(program, arg);
			
			// Material
			else if constexpr (I == 5 && materialEnabled)
				uboBlocks::Material::update(program, arg);

			// Light
			else if constexpr (I == 6 && lightEnabled)
				uboBlocks::Light::update(program, arg);
		}

		template<int I, class T>
		static FORCEINLINE void updateInternal(GLuint program, const T& arg)
		{
			constexpr int index = I + calcDisabled(I);
			updateInternalValue<index>(program, arg);
		}

		template<int I, class T, class... Args>
		static FORCEINLINE void updateInternal(GLuint program, const T& arg, Args... args)
		{
			updateInternal<I>(program, arg);
			updateInternal<I+1>(program, args...);
		}

		template<int I, class T>
		static FORCEINLINE void updateInternalIgnorant(GLuint program, const T& arg)
		{
			updateInternalValue<I>(program, arg);
		}

		template<int I, class T, class... Args>
		static FORCEINLINE void updateInternalIgnorant(GLuint program, const T& arg, Args... args)
		{
			updateInternalIgnorant<I>(program, arg);
			updateInternalIgnorant<I+1>(program, args...);
		}

		static constexpr int totalElements = 7;
    };
}