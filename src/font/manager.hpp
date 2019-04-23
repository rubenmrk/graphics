#pragma once

#include "textrender.hpp"
#include <list>

namespace game::opengl
{
	class text;
}

namespace game::font
{
	class manager
	{
		friend opengl::text;

	public:

		manager()
		{
			auto err = FT_Init_FreeType(&_lib);
			if (err)
				throw exception(except_e::FONT_BASE, "FT_Init_FreeType");
		}

		manager(const manager& rhs) = delete;

		manager(manager&& rhs) noexcept

		{
			_lib = rhs._lib;
			rhs._lib = nullptr;
			_fbs = std::move(rhs._fbs);
		}

		~manager()
		{
			_fbs.clear();
			if (_lib)
				FT_Done_FreeType(_lib);
		}

		void selectFont(std::string_view name, unsigned int size)
		{
			std::string path = std::string(dir).append(name);
			for (auto it = _fbs.begin(); it != _fbs.end(); ++it) {
				if (it->name() == path && it->size() == size) {
					_lastUsed = std::addressof(*it);
					break;
				}
			}
			loadFont(path, size);
		}

		void selectFont(std::string_view name)
		{
			std::string path = std::string(dir).append(name);
			for (auto it = _fbs.begin(); it != _fbs.end(); ++it) {
				if (it->name() == path) {
					_lastUsed = std::addressof(*it);
					break;
				}
			}
			loadFont(path, 12);
		}

		void selectFont(unsigned int size)
		{
			auto path = _lastUsed->name();

			for (auto it = _fbs.begin(); it != _fbs.end(); ++it) {
				if (it->name() == path && it->size() == size) {
					_lastUsed = std::addressof(*it);
					break;
				}
			}
			loadFont(path, size);
		}

		void loadFont(std::string_view name, unsigned int size)
		{
			_fbs.emplace_back(_lib, name, size);
			_lastUsed = &_fbs.back();
		}

		void unloadFont(std::string_view name, unsigned int size)
		{
			for (auto it = _fbs.begin(); it != _fbs.end(); ++it) {
				if (it->name() == name && it->size() == size) {
					_fbs.erase(it);
					break;
				}
			}
		}

		void unloadFont(std::string_view name)
		{
			for (auto it = _fbs.begin(); it != _fbs.end();) {
				if (it->name() == name)
					it = _fbs.erase(it);
				else
					++it;
			}
		}

	protected:

		bitmap renderString(std::string_view str, bool packed)
		{
			return _lastUsed->renderLine(str, packed);
		}

		bitmap renderString(std::string_view str, unsigned int maxWidth, bool packed)
		{
			return _lastUsed->renderLines(str, maxWidth, packed);
		}

		static constexpr std::string_view dir = "data/fonts/";

	private:

		FT_Library _lib = nullptr;

		std::vector<textRender> _fbs;

		textRender *_lastUsed = nullptr;
	};
}