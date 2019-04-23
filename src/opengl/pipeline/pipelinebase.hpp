#pragma once

#include "opengl/glbase.hpp"
#include "opengl/vertexinput.hpp"
#include "opengl/uniform.hpp"
#include <map>

namespace game::opengl
{
    class pipelineBase
    {
    public:

        virtual ~pipelineBase()
        {
        }

        virtual void render(const glm::mat4& proj, const glm::mat4& view) = 0;
    };

	template<class Model>
	class modelPipelineBase : pipelineBase
	{
	public:

		using idtype = uint32_t;

		modelPipelineBase()
		{
		}

		modelPipelineBase(const modelPipelineBase& rhs) = delete;

		modelPipelineBase(modelPipelineBase&& rhs) noexcept
			: _models(std::move(rhs._models)), _idgen(rhs._idgen)
		{
			rhs._idgen = 0;
		}

		~modelPipelineBase()
		{
		}

		idtype loadModel(std::string_view name)
		{
			auto id = _idgen++;
			auto modelObject = Model(name);

			_models.emplace(std::pair<idtype, Model>(id, std::move(modelObject)));
			return id;
		}

		void replaceModel(idtype id, std::string_view name)
		{
			auto modelObject = Model(name);
			auto it = _models.find(id);
			
			_models.erase(it);
			_models.emplace(std::pair<idtype, Model>(id, std::move(modelObject)));
		}

		void removeModel(idtype id)
		{
			auto it = _models.find(id);
			_models.erase(it);
		}

		Model * getInternalObjectPtr(idtype id)
		{
			auto it = _models.find(id);
			return std::addressof(it->second);
		}

		const Model * getInternalObjectPtr(idtype id) const
		{
			auto it = _models.find(id);
			return std::addressof(it->second);
		}

	protected:

		std::map<idtype, Model> _models;

		idtype _idgen = 0;
	};
}