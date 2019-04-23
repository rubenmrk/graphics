#pragma once

#include "base/exception.hpp"
#include "glyph.hpp"

#include <string>
#include <streambuf>
#include <istream>
#include <vector>
#include <map>

namespace game::font
{
	class textRender
	{
	public:

		textRender(FT_Library lib, std::string_view name, int size)
			: _lib(lib), _size(size), _name(name)
		{
			loadFace();
		}

		textRender(const textRender& rhs)
			: _lib(rhs._lib), _size(rhs._size), _spaceSize(rhs._spaceSize),
			_boundLength(rhs._boundLength), _boundHeight(rhs._boundHeight),
			_latinLength(rhs._latinLength), _glyphs(rhs._glyphs), 
			_name(rhs._name)
		{
			loadFace();
		}

		textRender(textRender&& rhs) noexcept
			: _lib(rhs._lib), _face(rhs._face), _size(rhs._size), 
			_spaceSize(rhs._spaceSize), _boundLength(rhs._boundLength), 
			_boundHeight(rhs._boundHeight), _latinLength(rhs._latinLength), 
			_glyphs(std::move(rhs._glyphs)), _name(std::move(rhs._name))
		{
			rhs._face = nullptr;
			rhs._lib = nullptr;
		}

		~textRender()
		{
			if (_face)
				FT_Done_Face(_face);
		}

		textRender& operator=(textRender&& rhs) noexcept
		{
			if (_face)
				FT_Done_Face(_face);
			_lib = rhs._lib;
			_face = rhs._face;
			_size = rhs._size;
			_spaceSize = rhs._spaceSize;
			_boundLength = rhs._boundLength;
			_boundHeight = rhs._boundHeight;
			_latinLength = rhs._latinLength;
			_glyphs = std::move(rhs._glyphs);
			_name = std::move(rhs._name);
			rhs._face = nullptr;
			rhs._lib = nullptr;
			return *this;
		}

		bitmap renderLine(std::string_view str, bool packed)
		{
			return std::move(stringToGlyph(str.begin(), str.end(), &textRender::loadCodePoint, !packed));
		}

		bitmap renderLines(std::string_view str, unsigned int maxWidth, bool packed)
		{
			return stringToBitmap(str.begin(), str.end(), &textRender::loadCodePoint, _latinLength - _boundLength, maxWidth, !packed);
		}

		textRender& size(int nsize)
		{
			if (_size != nsize) {
				_glyphs.clear();
				_size = nsize;
			}
			return *this;
		}

		int size() const
		{
			return _size;
		}

		textRender& name(std::string_view nname)
		{
			if (_name != nname) {
				_glyphs.clear();
				_name = nname;

				if (_face)
					FT_Done_Face(_face);
				loadFace();
			}
			return *this;
		}

		std::string_view name() const
		{
			return _name;
		}

	private:

		void loadFace()
		{
			// Load the face
			auto err = FT_New_Face(_lib, _name.c_str(), 0, &_face);
			if (err)
				throw exception(except_e::FONT_BASE, "FT_New_Face");
			err = FT_Set_Char_Size(_face, _size*64, _size*64, 144, 144);
			if (err)
				throw exception(except_e::FONT_BASE, "FT_Set_Char_Size");

			// Calculate the bounding box size
			auto emLenght = static_cast<double>(_size*144)/72.0;
			_boundLength = std::ceil(static_cast<double>((_face->bbox.yMax-_face->bbox.yMin)*emLenght) / _face->units_per_EM);
			_boundHeight = std::ceil(static_cast<double>(_face->bbox.yMax*emLenght) / _face->units_per_EM);

			// Load a latin character to calculate the latin bounding box size
			err = FT_Load_Char(_face, 'a', FT_LOAD_RENDER);
			if (err)
				throw exception(except_e::FONT_BASE, "FT_Load_Char");
			_glyphs['a'] = glyph(_face->glyph, _boundLength, _boundHeight);
			_latinLength = std::ceil(static_cast<double>(_face->glyph->metrics.vertAdvance)/64.0);

			// Grab the length of the space character
			err = FT_Load_Char(_face, ' ', FT_LOAD_ADVANCE_ONLY);
			if (err)
				throw exception(except_e::FONT_BASE, "FT_Load_Char");
			_spaceSize = (_face->glyph->advance.x/64);
		}

		glyph& loadCodePoint(uint32_t point)
		{
			if (_glyphs.count(point) == 0) {
				auto err = FT_Load_Char(_face, point, FT_LOAD_RENDER);
				if (err)
					throw exception(except_e::FONT_BASE, "FT_Load_Char");
				_glyphs[point] = glyph(_face->glyph, _boundLength, _boundHeight);
			}
			return _glyphs[point];
		}

		template<class Iter, class Fx>
		glyph stringToGlyph(Iter begin, Iter end, Fx loadFunc, bool encapsulate)
		{
			glyph ret;

			// Taking care of some edge cases
			auto len = std::distance(begin, end);
			if (len == 0) {
				return ret;
			}
			else if (len == 1 && !encapsulate) {
				if (*begin == ' ' || *begin == '\t') {
					ret._height = _boundHeight;
					ret._width = *begin == '\t' ? 4*_spaceSize : _spaceSize;
					ret._advance = _spaceSize;
					ret._pixels = new uint8_t[ret._width*ret._height];
					std::fill(ret._pixels, ret._pixels+(ret._width*ret._height), 0);
					return ret;
				}
				else {
					return (this->*loadFunc)(*begin);
				}
			}

			//
			// Calculate the string length, load all characters and store a reference to them
			//
			std::vector<std::pair<std::reference_wrapper<glyph>, int>> glyphs;
			int coffset = 0;

			// The first character is special, because it determines the offset of the entire line
			if (encapsulate) {
				coffset = ret._width = _spaceSize;
				ret._height = _boundLength;
			}
			else {
				if (*begin == ' ' || *begin == '\t') {
					coffset = ret._width = (*begin == ' ') ? _spaceSize : 4 * _spaceSize;
				}
				else {
					auto& img = (this->*loadFunc)(*begin);
					glyphs.emplace_back(std::make_pair(std::reference_wrapper<glyph>(img), coffset));
					ret._xoffset = img._xoffset;
					ret._width = img._advance - img._xoffset;
					coffset = img._advance - img._xoffset;
				}
				ret._height = _boundLength;
				++begin;
			}

			// The character between the first and last one are easier to deal with
			auto secondToLast = encapsulate ? end : std::prev(end);
			for (; begin != secondToLast; ++begin) {
				if (*begin == ' ' || *begin == '\t') {
					ret._width += (*begin == ' ') ? _spaceSize : 4*_spaceSize;
					coffset += (*begin == ' ') ? _spaceSize : 4*_spaceSize;
					continue;
				}
				
				auto& img = (this->*loadFunc)(*begin);
				glyphs.emplace_back(std::make_pair(std::reference_wrapper<glyph>(img), coffset+img._xoffset));
				ret._width += img._advance;
				coffset += img._advance;
			}

			// The final character is tricky again
			if (encapsulate) {
				ret._width += _spaceSize;
				ret._advance = coffset + _spaceSize;
			}
			else {
				if (*begin == ' ' || *begin == '\t') {
					ret._width += (*begin == ' ') ? _spaceSize : 4 * _spaceSize;
					ret._advance = (*begin == ' ') ? coffset + _spaceSize : coffset + 4 * _spaceSize;
				}
				else {
					auto& img = (this->*loadFunc)(*begin);
					glyphs.emplace_back(std::make_pair(std::reference_wrapper<glyph>(img), coffset + img._xoffset));
					ret._width += img._width + img._xoffset;
					ret._advance = coffset + img._advance;
				}
			}

			//
			// Generate the final bitmap
			//
			ret._pixels = new uint8_t[ret._width*ret._height];
            if (glyphs.size() > 0) {
                for (int j = 0; j < ret._height; ++j) {
                    // Fill the space before the first bitmap
                    std::fill(&ret._pixels[j*ret._width + 0], &ret._pixels[j*ret._width + glyphs[0].second], 0);

                    // Copy the single character bitmaps into the string
                    for (auto ptr = glyphs.begin();;) {
                        const auto& glyph = ptr->first.get();
                        auto off = ptr->second;

                        // Copy bitmap
                        std::copy(&glyph._pixels[j*glyph._width + 0], &glyph._pixels[j*glyph._width + glyph._width], &ret._pixels[j*ret._width + off]);

                        // Fill space between the next bitmap
                        if (++ptr != glyphs.end()) {
                            std::fill(&ret._pixels[j*ret._width + (off+glyph._width)], &ret._pixels[j*ret._width + ptr->second], 0);
                        }
                        else {
                            std::fill(&ret._pixels[j*ret._width + (off+glyph._width)], &ret._pixels[j*ret._width + ret._width], 0);
                            break;
                        }
                    }
                }
            }
            else {
                std::fill(&ret._pixels[0], &ret._pixels[ret._width*ret._height], 0);
            }

			return ret;
		}

		/*
		 * If you replace the iterators and the load function I don't see any reason why this function wouldn't work with a dictionary of words instead of a
		 * dictionary of letters, however I am not planning to do rendering of big texts so this is completely hypothethical and bugs will surely arrise.
		 * 
		 * Args:
		 * 	linedelta: 		The amount of extra spacing between lines.
		 * 	xmaxlen:		The maximum desired line length (result might be longer due to non encapsulating normalization or long words).
		 * 	encapsulate:	Whether to normalize the line length by prepending a space and appending a space.
		 */
		template<class Iter, class Fx>
        bitmap stringToBitmap(Iter begin, Iter end, Fx loadFunc, int linedelta, unsigned int xmaxlen, bool encapsulate)
        {
			bitmap ret;

			if (encapsulate)
				xmaxlen = (xmaxlen-2*_spaceSize > xmaxlen) ? 0 : xmaxlen-2*_spaceSize;

            // Taking care of some edge cases
			auto len = std::distance(begin, end);
			if (len == 0) {
				return ret;
			}
			else if (len == 1 && !encapsulate) {
				if (*begin == ' ' || *begin == '\t') {
					ret._height = _boundHeight;
					ret._width = *begin == '\t' ? 4*_spaceSize : _spaceSize;
					ret._pixels = new uint8_t[ret._width*ret._height];
					std::fill(ret._pixels, ret._pixels+(ret._width*ret._height), 0);
					return ret;
				}
				else {
					return (this->*loadFunc)(*begin);
				}
			}

			// Object to store lines and words in
			struct subGlyph
			{
				std::vector<std::pair<std::reference_wrapper<glyph>, int>> glyphs;
				int width = 0, xoffset = 0;
			};
			std::vector<subGlyph> lines;
			lines.emplace_back(subGlyph());
			
			//
			// Calculate the string length, load all characters and store a reference to them per line
			//
			int lastSpace = 0; // Strip empty spaces and tabs at the end of a line
			while (begin != end) {
				// Make sure we start on an actual word
				while (begin != end) {
					if (*begin == ' ') {
						if (lines.back().width + _spaceSize > xmaxlen && lines.back().width != 0)
							lines.emplace_back(subGlyph());
						else {
							lines.back().width += _spaceSize;
							lastSpace += _spaceSize;
						}
						++begin;
					}
					else if (*begin == '\t') {
						if (lines.back().width + 4*_spaceSize > xmaxlen && lines.back().width != 0)
							lines.emplace_back(subGlyph());
						else {
							lines.back().width += 4*_spaceSize;
							lastSpace += 4*_spaceSize;
						}
						++begin;
					}
					else if (*begin == '\n') {
						lines.emplace_back(subGlyph());
						++begin;
					}
					else if (*begin == '\r' || *begin == '\f') {
						++begin;
					}
					else {
						break;
					}
				}
				if (begin == end)
					break;


				subGlyph word;

				// Load the first glyph into the word
				auto& img = (this->*loadFunc)(*begin);
				word.glyphs.emplace_back(std::make_pair(std::reference_wrapper<glyph>(img), lines.back().width));
				word.xoffset = img._xoffset;
				word.width = img._advance - img._xoffset;
				++begin;

				// Load all other glyphs
				while (true) {
					if (begin == end || *begin == ' ' || *begin == '\t' || *begin == '\n' || *begin == '\r' || *begin == '\f') {
						if (lines.back().width == 0)
							lines.back().xoffset = word.xoffset;
						if (!encapsulate) {
							const auto& img = word.glyphs.back().first.get();
							word.width -= img._advance;
							word.width += img._width + img._xoffset;
						}
						lines.back().glyphs.insert(lines.back().glyphs.end(), word.glyphs.begin(), word.glyphs.end());
						lines.back().width += word.width;
						lastSpace = 0;
						break;
					}
					else {
						auto& img = (this->*loadFunc)(*begin);
						word.glyphs.emplace_back(std::make_pair(std::reference_wrapper<glyph>(img), lines.back().width+word.width+img._xoffset));
						word.width += img._advance;
						++begin;

						if (lines.back().width != 0 && lines.back().width + word.width > xmaxlen) {
							for (auto& glyph : word.glyphs) {
								glyph.second -= lines.back().width;
							}
							lines.back().width -= lastSpace;
							lines.emplace_back(subGlyph());
						}
					}
				}
			}

			//
			// Normalize the line offsets
			//
			int highestWidth = 0;
			if (encapsulate) {
				for (auto& line : lines) {
					for (auto& glyph : line.glyphs) {
						glyph.second += _spaceSize;
					}
					line.width += 2*_spaceSize;
					if (line.width > highestWidth) {
						highestWidth = line.width;
					}
				}
			}
			else {
				int lowestOffset = lines[0].xoffset;
				for (const auto& line : lines) {
					if (line.xoffset < lowestOffset)
						lowestOffset = line.xoffset;
				}
				for (auto& line : lines) {
					if (line.xoffset != lowestOffset) {
						auto delta = line.xoffset - lowestOffset;
						for (auto& glyph : line.glyphs) {
							glyph.second += delta;
						}
						if (line.width + delta > highestWidth)
							highestWidth = line.width + delta;
					}
					else if (line.width > highestWidth) {
						highestWidth = line.width;
					}
				}
			}

			//
			// Generate the final bitmap
			//
			ret._width = highestWidth;
			ret._height = _boundLength*lines.size() + linedelta*(lines.size()-1);
			if (ret._width == 0) { // The string consisted only out of \r \n \f
				return bitmap();
			}
			ret._pixels = new uint8_t[ret._width*ret._height];

			for (auto i = 0; i < lines.size(); ++i) { // For each line
				if (lines[i].width == 0) {
					std::fill(&ret._pixels[(i*(_boundLength+linedelta))*ret._width + 0], &ret._pixels[((i+1)*(_boundLength+linedelta))*ret._width + 0], 0);
				}
				else {
					for (auto j = 0; j < _boundLength; ++j) { // For each row in the current line
						// Fill the empty part before the first character (unlesss currently overlapping)
						if (linedelta > 0 || j >= -linedelta || i == 0)
							std::fill(&ret._pixels[(i*(_boundLength+linedelta)+j)*ret._width + 0], 
								&ret._pixels[(i*(_boundLength+linedelta)+j)*ret._width + lines[i].glyphs[0].second], 0);

						// Copy the single character bitmaps into the line
						for (auto ptr = lines[i].glyphs.begin();;) {
							const auto& glyph = ptr->first.get();
							auto off = ptr->second;

							// if currently overlapping
							if (linedelta < 0 && j < -linedelta && i != 0) {
								// Copy bitmap if it is higher than the previously painted value
								for (auto k = 0; k < glyph._width; ++k) { // For each pixel in the current row in the current line
									ret._pixels[(i*(_boundLength+linedelta)+j)*ret._width + (off+k)] = 
										std::max(ret._pixels[(i*(_boundLength+linedelta)+j)*ret._width + (off+k)], glyph._pixels[j*glyph._width + k]);
								}

								// Go to the next row
								if (++ptr == lines[i].glyphs.end())
									break;
							}
							else {
								// Copy bitmap
								std::copy(&glyph._pixels[j*glyph._width + 0], &glyph._pixels[j*glyph._width + glyph._width], 
									&ret._pixels[(i*(_boundLength+linedelta)+j)*ret._width + off]);

								// Fill space between the next bitmap or the end of the line
								if (++ptr != lines[i].glyphs.end()) {
									std::fill(&ret._pixels[(i*(_boundLength+linedelta)+j)*ret._width + (off+glyph._width)], 
										&ret._pixels[(i*(_boundLength+linedelta)+j)*ret._width + ptr->second], 0);
								}
								else {
									std::fill(&ret._pixels[(i*(_boundLength+linedelta)+j)*ret._width + (off+glyph._width)], 
										&ret._pixels[(i*(_boundLength+linedelta)+j)*ret._width + ret._width], 0);
									break;
								}
							}
						}
					}
				}

				// If the linewidth is positive, add empty lines
				if (linedelta > 0) {
					std::fill(&ret._pixels[(i*(_boundLength+linedelta))*ret._width + 0], &ret._pixels[(i*(_boundLength+linedelta)+linedelta)*ret._width + 0], 0);
				}
			}

			return ret;
		}

		FT_Library _lib;

		FT_Face _face;

		int _size, _spaceSize;

		int _boundLength, _boundHeight, _latinLength;

		std::map<uint32_t, glyph> _glyphs;

		std::string _name;
	};
}