#pragma once

#include "opengl/text/text.hpp"

namespace game
{
	template<class Pipeline, class Model>
	class modelProxy
	{
	public:

		modelProxy(Pipeline *pl, typename Pipeline::idtype id)
			: _id(id), _pl(pl)
		{
		}

		modelProxy(const modelProxy& rhs) = delete;

		modelProxy(modelProxy&& rhs) noexcept
			: _id(std::move(rhs._id)), _pl(rhs._pl)
		{
			rhs._pl = nullptr;
		}

		~modelProxy()
		{
			if (_pl)
				_pl->removeModel(_id);
		}

		modelProxy& operator=(modelProxy&& rhs) noexcept
		{
			delete _pl;
			_id = std::move(rhs._id);
			_pl = rhs._pl;
			rhs._pl = nullptr;
			return *this;
		}

		void invalidate()
		{
			_pl = nullptr;
		}

		void reload(std::string_view model)
		{
			_pl->replaceModel(_id, model);
		}

		Model * operator->()
		{
			return _pl->getInternalObjectPtr(_id);
		}

		const Model * operator->() const
		{
			return _pl->getInternalObjectPtr(_id);
		}

	private:

		typename Pipeline::idtype _id;

		Pipeline *_pl;
	};

	template<class Pipeline, class Text>
	class textProxy
	{
	public:

		textProxy(Pipeline *pl, typename Pipeline::idtype id)
			: _id(id), _pl(pl)
		{
		}

		textProxy(const textProxy& rhs) = delete;

		textProxy(textProxy&& rhs) noexcept
			: _id(std::move(rhs._id)), _pl(rhs._pl)
		{
			rhs._pl = nullptr;
		}

		~textProxy()
		{
			if (_pl)
				_pl->removeText(_id);
		}

		textProxy& operator=(textProxy&& rhs) noexcept
		{
			delete _pl;
			_id = std::move(rhs._id);
			_pl = rhs._pl;
			rhs._pl = nullptr;
			return *this;
		}

		void invalidate()
		{
			_pl = nullptr;
		}

		void reload(std::string_view txt, float xpos, float ypos, opengl::text::anchor attachPos = opengl::text::anchor::bottomLeft)
		{
			_pl->replaceText(_id, txt, xpos, ypos, attachPos);
		}

		void shift(float deltax, float deltay)
		{
			_pl->shiftText(_id, deltax, deltay);
		}

		Text * operator->()
		{
			return _pl->getInternalObjectPtr(_id);
		}

		const Text * operator->() const
		{
			return _pl->getInternalObjectPtr(_id);
		}

	private:

		typename Pipeline::idtype _id;

		Pipeline *_pl;
	};
}