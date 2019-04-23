#pragma once

#include "vertexinput.hpp"
#include "uniform.hpp"
#include "text/text.hpp"
#include "model/basicmodel.hpp"
#include "model/complexmodel.hpp"
#include "pipeline/basicmodelpipeline.hpp"
#include "pipeline/complexmodelpipeline.hpp"
#include "pipeline/textpipeline.hpp"

namespace game::opengl
{
    /*
     * Possible shader types
     */

    enum class ShaderType { PLANE_TEXTURED, PHONG_TEXTURED, TEXT };

    /*
     * Shader definitions
     */

    template<ShaderType st>
    struct ShaderInfo
    {
        static_assert(true, "Undefined ShaderType");
    };

	template<>
	struct ShaderInfo<ShaderType::PLANE_TEXTURED>
	{
		using vio = vertexInputObject<true, false, true, false>;
		using ubo = uniformBufferObject<true, false, true, false, false, false, false>;
		using vi = GLuint;

        using model = basicModel<vio, vi, true>;
        using pipeline = basicModelPipeline<ubo, vio, vi, true>;

		static constexpr auto shaderName = "texture";
	};

	template<>
	struct ShaderInfo<ShaderType::PHONG_TEXTURED>
	{
		using vio = vertexInputObject<true, false, true, true>;
		using ubo = uniformBufferObject<true, true, false, false, true, true, true>;
		using vi = GLuint;

        using model = complexModel<vio, vi>;
        using pipeline = complexModelPipeline<ubo, vio, vi>;

		static constexpr auto shaderName = "phong";
	};

	template<>
	struct ShaderInfo<ShaderType::TEXT>
	{
		using vio = vertexInputObject<true, false, true, false>;
		using ubo = uniformBufferObject<false, false, true, true, false, false, false>;

        using text = text;
        using pipeline = textPipeline<ubo, vio>;

		static constexpr auto shaderName = "text";
	};
}