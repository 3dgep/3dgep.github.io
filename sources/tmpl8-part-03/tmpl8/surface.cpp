// Template, BUAS version https://www.buas.nl/games
// IGAD/BUAS(NHTV)/UU - Jacco Bikker - 2006-2020
// CMGT/BUAS - Jeremiah van Oosten - 2021-2026

#include "surface.hpp"
#include "template.hpp"

#include <SDL3/SDL_filesystem.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <string>

// stb_image loads the images (PNG, TGA, JPEG, BMP, GIF): https://github.com/nothings/stb
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_ONLY_TGA
#define STBI_ONLY_JPEG
#define STBI_ONLY_BMP
#define STBI_ONLY_GIF
#define STBI_WINDOWS_UTF8 // file names are UTF-8
#include "stb_image.h"

namespace Tmpl8 {

// -----------------------------------------------------------
// The built-in 5x5 font (used by Surface::Print)
// -----------------------------------------------------------

namespace {

constexpr std::string_view FontCharacters = "abcdefghijklmnopqrstuvwxyz0123456789!?:=,.-() #'*/";
constexpr int FontSpace = 45; // the index of ' ', also used for characters the font doesn't have

// One glyph per character in FontCharacters: 5 lines of 5 pixels ('o' is a pixel).
constexpr std::array<std::array<std::string_view, 5>, 50> FontGlyphs = { {
	{ ":ooo:", "o:::o", "ooooo", "o:::o", "o:::o" }, // a
	{ "oooo:", "o:::o", "oooo:", "o:::o", "oooo:" }, // b
	{ ":oooo", "o::::", "o::::", "o::::", ":oooo" }, // c
	{ "oooo:", "o:::o", "o:::o", "o:::o", "oooo:" }, // d
	{ "ooooo", "o::::", "oooo:", "o::::", "ooooo" }, // e
	{ "ooooo", "o::::", "ooo::", "o::::", "o::::" }, // f
	{ ":oooo", "o::::", "o:ooo", "o:::o", ":ooo:" }, // g
	{ "o:::o", "o:::o", "ooooo", "o:::o", "o:::o" }, // h
	{ "::o::", "::o::", "::o::", "::o::", "::o::" }, // i
	{ ":::o:", ":::o:", ":::o:", ":::o:", "ooo::" }, // j
	{ "o::o:", "o:o::", "oo:::", "o:o::", "o::o:" }, // k
	{ "o::::", "o::::", "o::::", "o::::", "ooooo" }, // l
	{ "oo:o:", "o:o:o", "o:o:o", "o:::o", "o:::o" }, // m
	{ "o:::o", "oo::o", "o:o:o", "o::oo", "o:::o" }, // n
	{ ":ooo:", "o:::o", "o:::o", "o:::o", ":ooo:" }, // o
	{ "oooo:", "o:::o", "oooo:", "o::::", "o::::" }, // p
	{ ":ooo:", "o:::o", "o:::o", "o::oo", ":oooo" }, // q
	{ "oooo:", "o:::o", "oooo:", "o:o::", "o::o:" }, // r
	{ ":oooo", "o::::", ":ooo:", "::::o", "oooo:" }, // s
	{ "ooooo", "::o::", "::o::", "::o::", "::o::" }, // t
	{ "o:::o", "o:::o", "o:::o", "o:::o", ":oooo" }, // u
	{ "o:::o", "o:::o", ":o:o:", ":o:o:", "::o::" }, // v
	{ "o:::o", "o:::o", "o:o:o", "o:o:o", ":o:o:" }, // w
	{ "o:::o", ":o:o:", "::o::", ":o:o:", "o:::o" }, // x
	{ "o:::o", "o:::o", ":oooo", "::::o", ":ooo:" }, // y
	{ "ooooo", ":::o:", "::o::", ":o:::", "ooooo" }, // z
	{ ":ooo:", "o::oo", "o:o:o", "oo::o", ":ooo:" }, // 0
	{ "::o::", ":oo::", "::o::", "::o::", ":ooo:" }, // 1
	{ ":ooo:", "o:::o", "::oo:", ":o:::", "ooooo" }, // 2
	{ "oooo:", "::::o", "::oo:", "::::o", "oooo:" }, // 3
	{ "o::::", "o::o:", "ooooo", ":::o:", ":::o:" }, // 4
	{ "ooooo", "o::::", "oooo:", "::::o", "oooo:" }, // 5
	{ ":oooo", "o::::", "oooo:", "o:::o", ":ooo:" }, // 6
	{ "ooooo", "::::o", ":::o:", "::o::", "::o::" }, // 7
	{ ":ooo:", "o:::o", ":ooo:", "o:::o", ":ooo:" }, // 8
	{ ":ooo:", "o:::o", ":oooo", "::::o", ":ooo:" }, // 9
	{ "::o::", "::o::", "::o::", ":::::", "::o::" }, // !
	{ ":ooo:", "::::o", ":::o:", ":::::", "::o::" }, // ?
	{ ":::::", ":::::", "::o::", ":::::", "::o::" }, // :
	{ ":::::", ":::::", ":ooo:", ":::::", ":ooo:" }, // =
	{ ":::::", ":::::", ":::::", ":::o:", "::o::" }, // ,
	{ ":::::", ":::::", ":::::", ":::::", "::o::" }, // .
	{ ":::::", ":::::", ":ooo:", ":::::", ":::::" }, // -
	{ ":::o:", "::o::", "::o::", "::o::", ":::o:" }, // (
	{ "::o::", ":::o:", ":::o:", ":::o:", "::o::" }, // )
	{ ":::::", ":::::", ":::::", ":::::", ":::::" }, // space
	{ "ooooo", "ooooo", "ooooo", "ooooo", "ooooo" }, // #
	{ "::o::", "::o::", ":::::", ":::::", ":::::" }, // ' (Tnx Ferry)
	{ "o:o:o", ":ooo:", "ooooo", ":ooo:", "o:o:o" }, // *
	{ "::::o", ":::o:", "::o::", ":o:::", "o::::" }, // /
} };

// The glyph for a character (upper case letters use the lower case glyphs).
const std::array<std::string_view, 5>& Glyph( char c )
{
	if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
	const size_t index = FontCharacters.find( c );
	return FontGlyphs[index == std::string_view::npos ? FontSpace : index];
}

// A path as UTF-8 text (SDL and stb_image use UTF-8 file names and messages).
std::string Utf8( const std::filesystem::path& path )
{
	const std::u8string text = path.u8string();
	return std::string( text.begin(), text.end() );
}

// A relative path ("assets/ball.png") is relative to the working directory. When the file isn't there, it is
// also looked for next to the executable (when the game is started from another folder, e.g. a CMake build).
std::filesystem::path FindFile( std::string_view a_File )
{
	const std::filesystem::path path( a_File );
	if (path.is_relative() && !std::filesystem::exists( path ))
	{
		if (const char* basePath = SDL_GetBasePath())
		{
			// SDL_GetBasePath returns UTF-8 text.
			const std::filesystem::path nextToExecutable = std::filesystem::path( reinterpret_cast<const char8_t*>(basePath) ) / path;
			if (std::filesystem::exists( nextToExecutable )) return nextToExecutable;
		}
	}
	return path;
}

int LineOutCode( float x, float y, float xMin, float xMax, float yMin, float yMax )
{
	return (x < xMin ? 1 : (x > xMax ? 2 : 0)) + (y < yMin ? 4 : (y > yMax ? 8 : 0));
}

} // namespace

// -----------------------------------------------------------
// True-color surface class implementation
// -----------------------------------------------------------

Surface::Surface( int a_Width, int a_Height ) :
	m_Pixels( static_cast<size_t>(a_Width) * a_Height ),
	m_Width( a_Width ),
	m_Height( a_Height ),
	m_Pitch( a_Width )
{
}

Surface::Surface( std::string_view a_File )
{
	LoadImage( a_File );
}

void Surface::LoadImage( std::string_view a_File )
{
	const std::filesystem::path path = FindFile( a_File );
	int width, height, channels;
	// UTF-8, so this also works for folder names with accents or other scripts. Always 4 bytes per pixel: R, G, B, A.
	unsigned char* data = stbi_load( Utf8( path ).c_str(), &width, &height, &channels, 4 );
	if (!data)
	{
		const std::string reason = std::filesystem::exists( path )
			? std::string( "not a supported image (" ) + stbi_failure_reason() + ")"
			: "file not found in " + Utf8( std::filesystem::absolute( path ).parent_path() );
		NotifyUser( "Could not load image \"" + std::string( a_File ) + "\": " + reason );
	}
	m_Width = m_Pitch = width;
	m_Height = height;
	m_Pixels.resize( static_cast<size_t>(width) * height );
	// Pixels are stored as 0xAARRGGBB.
	for (size_t i = 0; i < m_Pixels.size(); i++)
	{
		const unsigned char* p = data + i * 4;
		m_Pixels[i] = static_cast<Pixel>(p[3]) << 24 | static_cast<Pixel>(p[0]) << 16 | static_cast<Pixel>(p[1]) << 8 | static_cast<Pixel>(p[2]);
	}
	stbi_image_free( data );
}

void Surface::Clear( Pixel a_Color )
{
	for (int y = 0; y < m_Height; y++)
		std::fill_n( m_Pixels.data() + y * m_Pitch, m_Width, a_Color );
}

void Surface::Centre( std::string_view a_String, int y1, Pixel color )
{
	const int x = (m_Width - static_cast<int>(a_String.size()) * 6) / 2;
	Print( a_String, x, y1, color );
}

void Surface::Print( std::string_view a_String, int x1, int y1, Pixel color )
{
	// Each character is 5x5 pixels, with 1 pixel of space after it. Text that doesn't fit on the surface is cut off.
	int x = x1;
	for (const char character : a_String)
	{
		int y = y1;
		for (const std::string_view line : Glyph( character ))
		{
			for (int h = 0; h < 5; h++)
			{
				// the pixel, and a black shadow below it
				if (line[h] == 'o') Plot( x + h, y, color ), Plot( x + h, y + 1, 0 );
			}
			y++;
		}
		x += 6;
	}
}

void Surface::Resize( const Surface& a_Orig )
{
	const Pixel* src = a_Orig.GetBuffer();
	Pixel* dst = m_Pixels.data();
	const int owidth = a_Orig.GetWidth(), oheight = a_Orig.GetHeight();
	const int dx = (owidth << 10) / m_Width, dy = (oheight << 10) / m_Height;
	for (int v = 0; v < m_Height; v++)
	{
		for (int u = 0; u < m_Width; u++)
		{
			const int su = u * dx, sv = v * dy;
			const Pixel* s = src + (su >> 10) + (sv >> 10) * owidth;
			const unsigned int ufrac = su & 1023, vfrac = sv & 1023;
			const unsigned int w4 = (ufrac * vfrac) >> 12;
			const unsigned int w3 = ((1023 - ufrac) * vfrac) >> 12;
			const unsigned int w2 = (ufrac * (1023 - vfrac)) >> 12;
			const unsigned int w1 = ((1023 - ufrac) * (1023 - vfrac)) >> 12;
			const int x2 = ((su + dx) > ((owidth - 1) << 10)) ? 0 : 1;
			const int y2 = ((sv + dy) > ((oheight - 1) << 10)) ? 0 : 1;
			const Pixel p1 = *s, p2 = *(s + x2), p3 = *(s + owidth * y2), p4 = *(s + owidth * y2 + x2);
			const Pixel r = (((p1 & RedMask) * w1 + (p2 & RedMask) * w2 + (p3 & RedMask) * w3 + (p4 & RedMask) * w4) >> 8) & RedMask;
			const Pixel g = (((p1 & GreenMask) * w1 + (p2 & GreenMask) * w2 + (p3 & GreenMask) * w3 + (p4 & GreenMask) * w4) >> 8) & GreenMask;
			const Pixel b = (((p1 & BlueMask) * w1 + (p2 & BlueMask) * w2 + (p3 & BlueMask) * w3 + (p4 & BlueMask) * w4) >> 8) & BlueMask;
			*(dst + u + v * m_Pitch) = r + g + b;
		}
	}
}

void Surface::Line( float x1, float y1, float x2, float y2, Pixel c )
{
	// clip (Cohen-Sutherland, https://en.wikipedia.org/wiki/Cohen%E2%80%93Sutherland_algorithm)
	const float xmin = 0, ymin = 0, xmax = static_cast<float>(m_Width - 1), ymax = static_cast<float>(m_Height - 1);
	int c0 = LineOutCode( x1, y1, xmin, xmax, ymin, ymax ), c1 = LineOutCode( x2, y2, xmin, xmax, ymin, ymax );
	bool accept = false;
	while (true)
	{
		if (!(c0 | c1)) { accept = true; break; }
		else if (c0 & c1) break; else
		{
			float x = 1.0f, y = 1.0f;
			const int co = c0 ? c0 : c1;
			if (co & 8) x = x1 + (x2 - x1) * (ymax - y1) / (y2 - y1), y = ymax;
			else if (co & 4) x = x1 + (x2 - x1) * (ymin - y1) / (y2 - y1), y = ymin;
			else if (co & 2) y = y1 + (y2 - y1) * (xmax - x1) / (x2 - x1), x = xmax;
			else if (co & 1) y = y1 + (y2 - y1) * (xmin - x1) / (x2 - x1), x = xmin;
			if (co == c0) x1 = x, y1 = y, c0 = LineOutCode( x1, y1, xmin, xmax, ymin, ymax );
			else x2 = x, y2 = y, c1 = LineOutCode( x2, y2, xmin, xmax, ymin, ymax );
		}
	}
	if (!accept) return;
	const float b = x2 - x1;
	const float h = y2 - y1;
	const float l = std::max( std::abs( b ), std::abs( h ) );
	const int il = static_cast<int>(l);
	const float dx = b / l;
	const float dy = h / l;
	for (int i = 0; i <= il; i++)
	{
		*(m_Pixels.data() + static_cast<int>(x1) + static_cast<int>(y1) * m_Pitch) = c;
		x1 += dx, y1 += dy;
	}
}

void Surface::Plot( int x, int y, Pixel c )
{
	if ((x >= 0) && (y >= 0) && (x < m_Width) && (y < m_Height)) m_Pixels.data()[x + y * m_Pitch] = c;
}

void Surface::Line( int x1, int y1, int x2, int y2, Pixel c )
{
	Line( static_cast<float>(x1), static_cast<float>(y1), static_cast<float>(x2), static_cast<float>(y2), c );
}

void Surface::Box( int x1, int y1, int x2, int y2, Pixel c )
{
	Line( x1, y1, x2, y1, c );
	Line( x2, y1, x2, y2, c );
	Line( x1, y2, x2, y2, c );
	Line( x1, y1, x1, y2, c );
}

void Surface::Bar( int x1, int y1, int x2, int y2, Pixel c )
{
	Pixel* a = m_Pixels.data() + x1 + y1 * m_Pitch;
	for (int y = y1; y <= y2; y++)
	{
		std::fill_n( a, x2 - x1 + 1, c );
		a += m_Pitch;
	}
}

void Surface::CopyTo( Surface& a_Dst, int a_X, int a_Y ) const
{
	Pixel* dst = a_Dst.GetBuffer();
	const Pixel* src = m_Pixels.data();
	if (src && dst)
	{
		int srcwidth = m_Width;
		int srcheight = m_Height;
		const int srcpitch = m_Pitch;
		const int dstwidth = a_Dst.GetWidth();
		const int dstheight = a_Dst.GetHeight();
		const int dstpitch = a_Dst.GetPitch();
		if ((srcwidth + a_X) > dstwidth) srcwidth = dstwidth - a_X;
		if ((srcheight + a_Y) > dstheight) srcheight = dstheight - a_Y;
		if (a_X < 0) src -= a_X, srcwidth += a_X, a_X = 0;
		if (a_Y < 0) src -= a_Y * srcpitch, srcheight += a_Y, a_Y = 0;
		if ((srcwidth > 0) && (srcheight > 0))
		{
			dst += a_X + dstpitch * a_Y;
			for (int y = 0; y < srcheight; y++)
			{
				std::copy_n( src, srcwidth, dst );
				dst += dstpitch;
				src += srcpitch;
			}
		}
	}
}

void Surface::BlendCopyTo( Surface& a_Dst, int a_X, int a_Y ) const
{
	Pixel* dst = a_Dst.GetBuffer();
	const Pixel* src = m_Pixels.data();
	if (src && dst)
	{
		int srcwidth = m_Width;
		int srcheight = m_Height;
		const int srcpitch = m_Pitch;
		const int dstwidth = a_Dst.GetWidth();
		const int dstheight = a_Dst.GetHeight();
		const int dstpitch = a_Dst.GetPitch();
		if ((srcwidth + a_X) > dstwidth) srcwidth = dstwidth - a_X;
		if ((srcheight + a_Y) > dstheight) srcheight = dstheight - a_Y;
		if (a_X < 0) src -= a_X, srcwidth += a_X, a_X = 0;
		if (a_Y < 0) src -= a_Y * srcpitch, srcheight += a_Y, a_Y = 0;
		if ((srcwidth > 0) && (srcheight > 0))
		{
			dst += a_X + dstpitch * a_Y;
			for (int y = 0; y < srcheight; y++)
			{
				for (int x = 0; x < srcwidth; x++) dst[x] = AddBlend( dst[x], src[x] );
				dst += dstpitch;
				src += srcpitch;
			}
		}
	}
}

void Surface::ScaleColor( unsigned int a_Scale )
{
	for (int y = 0; y < m_Height; y++)
	{
		Pixel* line = m_Pixels.data() + y * m_Pitch;
		for (int x = 0; x < m_Width; x++)
		{
			const Pixel c = line[x];
			const Pixel rb = (((c & (RedMask | BlueMask)) * a_Scale) >> 5) & (RedMask | BlueMask);
			const Pixel g = (((c & GreenMask) * a_Scale) >> 5) & GreenMask;
			line[x] = rb + g;
		}
	}
}

// -----------------------------------------------------------
// Sprite class implementation
// -----------------------------------------------------------

Sprite::Sprite( std::string_view a_File, unsigned int a_Columns, unsigned int a_Rows ) :
	Sprite( std::make_shared<Surface>( a_File ), a_Columns, a_Rows )
{
}

Sprite::Sprite( std::shared_ptr<Surface> a_Surface, unsigned int a_Columns, unsigned int a_Rows ) :
	m_Surface( std::move( a_Surface ) ),
	m_Columns( a_Columns ),
	m_Rows( a_Rows )
{
	if (!m_Surface) NotifyUser( "Sprite: no surface (the image is a nullptr)." );
	if (m_Columns == 0 || m_Rows == 0) NotifyUser( "Sprite: the number of columns and rows must be at least 1." );
	m_Width = m_Surface->GetWidth() / static_cast<int>(m_Columns);
	m_Height = m_Surface->GetHeight() / static_cast<int>(m_Rows);
	m_Pitch = m_Surface->GetPitch();
	InitializeStartData();
}

const Pixel* Sprite::FramePixels( unsigned int a_Frame ) const
{
	const int column = static_cast<int>(a_Frame % m_Columns);
	const int row = static_cast<int>(a_Frame / m_Columns);
	return GetBuffer() + column * m_Width + row * m_Height * m_Pitch;
}

void Sprite::Draw( Surface& a_Target, int a_X, int a_Y ) const
{
	if ((a_X < -m_Width) || (a_X > (a_Target.GetWidth() + m_Width))) return;
	if ((a_Y < -m_Height) || (a_Y > (a_Target.GetHeight() + m_Height))) return;
	int x1 = a_X, x2 = a_X + m_Width;
	int y1 = a_Y, y2 = a_Y + m_Height;
	const Pixel* src = FramePixels( m_CurrentFrame );
	if (x1 < 0)
	{
		src += -x1;
		x1 = 0;
	}
	if (x2 > a_Target.GetWidth()) x2 = a_Target.GetWidth();
	if (y1 < 0)
	{
		src += -y1 * m_Pitch;
		y1 = 0;
	}
	if (y2 > a_Target.GetHeight()) y2 = a_Target.GetHeight();
	Pixel* dest = a_Target.GetBuffer();
	const int dpitch = a_Target.GetPitch();
	if ((x2 > x1) && (y2 > y1))
	{
		int addr = y1 * dpitch + x1;
		const int width = x2 - x1;
		const int height = y2 - y1;
		for (int y = 0; y < height; y++)
		{
			const int line = y + (y1 - a_Y);
			const int lsx = m_Start[m_CurrentFrame * m_Height + line] + a_X;
			const int xs = (lsx > x1) ? lsx - x1 : 0;
			for (int x = xs; x < width; x++)
			{
				const Pixel c1 = src[x];
				if (c1 & 0xffffff) dest[addr + x] = c1; // black is transparent
			}
			addr += dpitch;
			src += m_Pitch;
		}
	}
}

void Sprite::DrawScaled( int a_X, int a_Y, int a_Width, int a_Height, Surface& a_Target ) const
{
	if ((a_Width <= 0) || (a_Height <= 0)) return;
	const Pixel* frame = FramePixels( m_CurrentFrame );
	for (int x = 0; x < a_Width; x++) for (int y = 0; y < a_Height; y++)
	{
		const int tx = a_X + x, ty = a_Y + y;
		if (tx < 0 || ty < 0 || tx >= a_Target.GetWidth() || ty >= a_Target.GetHeight()) continue; // outside the target
		const int u = static_cast<int>(static_cast<float>(x) * (static_cast<float>(m_Width) / static_cast<float>(a_Width)));
		const int v = static_cast<int>(static_cast<float>(y) * (static_cast<float>(m_Height) / static_cast<float>(a_Height)));
		const Pixel color = frame[u + v * m_Pitch];
		if (color & 0xffffff) a_Target.GetBuffer()[tx + ty * a_Target.GetPitch()] = color;
	}
}

void Sprite::InitializeStartData()
{
	m_Start.assign( static_cast<size_t>(Frames()) * m_Height, m_Width );
	for (unsigned int f = 0; f < Frames(); ++f)
	{
		for (int y = 0; y < m_Height; ++y)
		{
			const Pixel* addr = FramePixels( f ) + y * m_Pitch;
			for (int x = 0; x < m_Width; ++x)
			{
				if (addr[x])
				{
					m_Start[f * m_Height + y] = x;
					break;
				}
			}
		}
	}
}

// -----------------------------------------------------------
// Font class implementation
// -----------------------------------------------------------

Font::Font( std::string_view a_File, std::string_view a_Chars ) :
	m_Surface( std::make_unique<Surface>( a_File ) ),
	m_Offset( a_Chars.size(), 0 ),
	m_Width( a_Chars.size(), 0 ),
	m_Trans( 256, 0 )
{
	const Pixel* b = static_cast<const Surface&>(*m_Surface).GetBuffer();
	const int w = m_Surface->GetWidth();
	const int h = m_Surface->GetHeight();
	for (size_t i = 0; i < a_Chars.size(); i++) m_Trans[static_cast<unsigned char>(a_Chars[i])] = static_cast<int>(i);
	m_Height = h;
	size_t charnr = 0;
	int start = 0;
	bool lastempty = true;
	for (int x = 0; x < w && charnr < a_Chars.size(); x++)
	{
		bool empty = true;
		for (int y = 0; y < h; y++) if (b[x + y * w] & 0xffffff)
		{
			if (lastempty) start = x;
			empty = false;
		}
		if (empty && !lastempty)
		{
			m_Width[charnr] = x - start;
			m_Offset[charnr] = start;
			charnr++;
		}
		lastempty = empty;
	}
}

int Font::Width( std::string_view a_Text ) const
{
	int w = 0;
	for (const char character : a_Text)
	{
		if (character == ' ') w += 4; else w += m_Width[m_Trans[static_cast<unsigned char>(character)]] + 2;
	}
	return w;
}

void Font::Centre( Surface& a_Target, std::string_view a_Text, int a_Y ) const
{
	const int x = (a_Target.GetPitch() - Width( a_Text )) / 2;
	Print( a_Target, a_Text, x, a_Y );
}

void Font::Print( Surface& a_Target, std::string_view a_Text, int a_X, int a_Y, bool clip ) const
{
	Pixel* b = a_Target.GetBuffer() + a_X + a_Y * a_Target.GetPitch();
	const Pixel* s = static_cast<const Surface&>(*m_Surface).GetBuffer();
	if (((a_Y + m_Height) < m_CY1) || (a_Y > m_CY2)) return;
	int cx = 0;
	for (const char character : a_Text)
	{
		if (character == ' ') { cx += 4; continue; }
		const int c = m_Trans[static_cast<unsigned char>(character)];
		const Pixel* t = s + m_Offset[c];
		Pixel* d = b + cx;
		for (int y = 0; y < m_Height; y++)
		{
			if (((a_Y + y) >= m_CY1) && ((a_Y + y) <= m_CY2))
			{
				for (int x = 0; x < m_Width[c]; x++)
				{
					if (t[x] && (!clip || (x + cx + a_X) < a_Target.GetPitch())) d[x] = AddBlend( t[x], d[x] );
				}
			}
			t += m_Surface->GetPitch(), d += a_Target.GetPitch();
		}
		cx += m_Width[c] + 2;
		if ((cx + a_X) >= a_Target.GetPitch()) break;
	}
}

} // namespace Tmpl8
