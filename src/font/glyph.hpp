#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

#include <cstdint>
#include <cmath>
#include <algorithm>

namespace game::font
{
	class bitmap
	{
		friend class textRender;

	public:

		operator bool()
		{
			return _pixels;
		}

		bitmap()
			: _width(0), _height(0), _pixels(nullptr)
		{
		}

		bitmap(const bitmap& rhs)
			: _width(rhs._width), _height(rhs._height)
		{
			_pixels = new uint8_t[_width*_height];
			std::copy(rhs._pixels, rhs._pixels+(_width*_height), _pixels);
		}

		bitmap(bitmap&& rhs) noexcept
			: _width(rhs._width), _height(rhs._height)
		{
			_pixels = rhs._pixels;
			rhs._pixels = nullptr;
			rhs._width = 0;
			rhs._height = 0;
		}

		~bitmap()
		{
			delete[] _pixels;
		}

		bitmap& operator=(const bitmap& rhs)
		{
			if (_width != rhs._width && _height != rhs._height) {
				delete[] _pixels;
				_width = rhs._width;
				_height = rhs._height;
				_pixels = new uint8_t[_width*_height];
			}
			std::copy(rhs._pixels, rhs._pixels+(_width*_height), _pixels);
			return *this;
		}

		bitmap& operator=(bitmap&& rhs) noexcept
		{
			delete[] _pixels;
			_width = rhs._width;
			_height = rhs._height;
			_pixels = rhs._pixels;
			rhs._pixels = nullptr;
			rhs._width = 0;
			rhs._height = 0;
			return *this;
		}

		int width() const
		{
			return _width;
		}

		int height() const
		{
			return _height;
		}

		const uint8_t * data() const
		{
			return _pixels;
		}

	protected:

		int _width, _height;

		uint8_t *_pixels;
	};

	class glyph : public bitmap
	{
		friend class textRender;

	public:

		glyph()
			: bitmap(), _xoffset(0), _advance(0)
		{
		}

		glyph(const FT_GlyphSlot& glyph, int boundLength, int boundHeight)
		{
			// Bitmap width and height
			const int& bh = glyph->bitmap.rows;
			const int& bw = glyph->bitmap.width;
			const int& bp = glyph->bitmap.pitch;

			// Bitmap offsets
			_xoffset = (glyph->metrics.horiBearingX/64);
			int yoffset = boundHeight - (bh + (glyph->metrics.horiBearingY-glyph->metrics.height)/64);
			_advance = (glyph->advance.x/64);

			// Initialize the output object
			_width = bw;
			_height = boundLength;
			_pixels = new uint8_t[_width*_height];

			// Copy bitmap data
			for (auto j = 0; j < _height; ++j) {
				for (auto i = 0; i < _width; ++i) {
					if (i < bw && (j >= yoffset && j < bh+yoffset)) 
						_pixels[j*_width + i] = glyph->bitmap.buffer[(j-yoffset)*bp + i*(bp/bw)];
					else
						_pixels[j*_width + i] = 0;
				}
			}
		}

		glyph(const glyph& rhs)
			: bitmap(rhs), _xoffset(rhs._xoffset), _advance(rhs._advance)
		{
		}

		glyph(glyph&& rhs) noexcept
			: bitmap(std::move(rhs)), _xoffset(rhs._xoffset), _advance(rhs._advance)
		{
		}

		~glyph()
		{
		}

		glyph& operator=(const glyph& rhs)
		{
			bitmap::operator=(rhs);
			_xoffset = rhs._xoffset;
			_advance = rhs._advance;
			return *this;
		}

		glyph& operator=(glyph&& rhs) noexcept
		{
			delete[] _pixels;
			_width = rhs._width;
			_height = rhs._height;
			_pixels = rhs._pixels;
			_xoffset = rhs._xoffset;
			_advance = rhs._advance;
			rhs._pixels = nullptr;
			rhs._width = 0;
			rhs._height = 0;
			rhs._xoffset = 0;
			rhs._advance = 0;
			return *this;
		}

	private:

		int _xoffset, _advance;
	};
}