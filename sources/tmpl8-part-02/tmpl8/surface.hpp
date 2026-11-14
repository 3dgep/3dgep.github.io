// Template, BUAS version https://www.buas.nl/games
// IGAD/BUAS(NHTV)/UU - Jacco Bikker - 2006-2020
// CMGT/BUAS - Jeremiah van Oosten - 2021-2026

#pragma once

#include <memory>
#include <string_view>
#include <vector>

namespace Tmpl8 {

using Pixel = unsigned int; // a 32-bit color: 0xAARRGGBB (the alpha byte is ignored on screen)

constexpr Pixel RedMask = 0xff0000;
constexpr Pixel GreenMask = 0x00ff00;
constexpr Pixel BlueMask = 0x0000ff;

// additive blending
inline Pixel AddBlend( Pixel a_Color1, Pixel a_Color2 )
{
	const Pixel r = (a_Color1 & RedMask) + (a_Color2 & RedMask);
	const Pixel g = (a_Color1 & GreenMask) + (a_Color2 & GreenMask);
	const Pixel b = (a_Color1 & BlueMask) + (a_Color2 & BlueMask);
	const Pixel r1 = (r & RedMask) | (RedMask * (r >> 24));
	const Pixel g1 = (g & GreenMask) | (GreenMask * (g >> 16));
	const Pixel b1 = (b & BlueMask) | (BlueMask * (b >> 8));
	return r1 + g1 + b1;
}

// subtractive blending
inline Pixel SubBlend( Pixel a_Color1, Pixel a_Color2 )
{
	int red = static_cast<int>(a_Color1 & RedMask) - static_cast<int>(a_Color2 & RedMask);
	int green = static_cast<int>(a_Color1 & GreenMask) - static_cast<int>(a_Color2 & GreenMask);
	int blue = static_cast<int>(a_Color1 & BlueMask) - static_cast<int>(a_Color2 & BlueMask);
	if (red < 0) red = 0;
	if (green < 0) green = 0;
	if (blue < 0) blue = 0;
	return static_cast<Pixel>(red + green + blue);
}

// An image: a block of pixels you can draw on. The screen is a Surface too.
class Surface
{
public:
	Surface( int a_Width, int a_Height );
	explicit Surface( std::string_view a_File ); // loads an image (PNG, TGA, JPEG, BMP or GIF)
	// A Surface owns its pixels: it can't be copied by accident (use a reference, or a std::shared_ptr<Surface>
	// to share it), but it can be moved.
	Surface( const Surface& ) = delete;
	Surface& operator=( const Surface& ) = delete;
	Surface( Surface&& ) = default;
	Surface& operator=( Surface&& ) = default;
	// member data access
	Pixel* GetBuffer() { return m_Pixels.data(); }
	const Pixel* GetBuffer() const { return m_Pixels.data(); }
	int GetWidth() const { return m_Width; }
	int GetHeight() const { return m_Height; }
	int GetPitch() const { return m_Pitch; } // the number of pixels from one line to the next
	// Special operations
	void Centre( std::string_view a_String, int y1, Pixel color ); // prints a text centered horizontally
	void Print( std::string_view a_String, int x1, int y1, Pixel color );
	void Clear( Pixel a_Color );
	void Line( float x1, float y1, float x2, float y2, Pixel color );
	void Plot( int x, int y, Pixel c );
	void LoadImage( std::string_view a_File );
	void CopyTo( Surface& a_Dst, int a_X, int a_Y ) const;
	void BlendCopyTo( Surface& a_Dst, int a_X, int a_Y ) const;
	void ScaleColor( unsigned int a_Scale );
	void Box( int x1, int y1, int x2, int y2, Pixel color );
	void Bar( int x1, int y1, int x2, int y2, Pixel color );
	void Resize( const Surface& a_Orig ); // fills this surface with a scaled copy of a_Orig
private:
	std::vector<Pixel> m_Pixels;
	int m_Width = 0, m_Height = 0;
	int m_Pitch = 0;
};

// A sprite: an image with one or more animation frames (a sprite sheet). The frames can be next to each other in
// one row, or in a grid of columns and rows. Black pixels (0x000000) are transparent.
//
//     Sprite gun( "assets/aagun.tga", 36 );            // 36 frames in one row
//     Sprite hero( "assets/hero.png", 8, 4 );          // 8 columns and 4 rows: 32 frames
//
// The frames are numbered row by row: in the 8x4 grid, frame 0 to 7 are the first row, frame 8 to 15 the second, ...
// SetFrame( column, row ) picks a frame by its place in the grid, e.g. a row per direction and a column per step.
//
// Several sprites can share the same image: copy a sprite, or give them the same std::shared_ptr<Surface>.
// Each sprite (and each copy) has its own current frame, so all enemies can use one sprite sheet:
//
//     Sprite enemy( "assets/ctankbase.tga", 16 );     // loads the image
//     std::vector<Sprite> enemies( 10, enemy );       // 10 sprites that share that image
//
//     auto sheet = std::make_shared<Surface>( "assets/ctankbase.tga" );
//     Sprite tank1( sheet, 16 ), tank2( sheet, 16 );  // two sprites that share the image
class Sprite
{
public:
	// Loads the image from a file.
	Sprite( std::string_view a_File, unsigned int a_Columns, unsigned int a_Rows = 1 );
	// Uses an image that can be shared with other sprites.
	Sprite( std::shared_ptr<Surface> a_Surface, unsigned int a_Columns, unsigned int a_Rows = 1 );

	// Methods
	void Draw( Surface& a_Target, int a_X, int a_Y ) const;
	void DrawScaled( int a_X, int a_Y, int a_Width, int a_Height, Surface& a_Target ) const;
	void SetFrame( unsigned int a_Index ) { m_CurrentFrame = a_Index % Frames(); } // wraps around after the last frame
	void SetFrame( unsigned int a_Column, unsigned int a_Row ) { SetFrame( (a_Row % m_Rows) * m_Columns + (a_Column % m_Columns) ); }
	unsigned int GetFrame() const { return m_CurrentFrame; }
	int GetWidth() const { return m_Width; }   // the width of one frame
	int GetHeight() const { return m_Height; } // the height of one frame
	unsigned int Frames() const { return m_Columns * m_Rows; }
	unsigned int Columns() const { return m_Columns; }
	unsigned int Rows() const { return m_Rows; }
	Pixel* GetBuffer() { return m_Surface->GetBuffer(); }
	const Pixel* GetBuffer() const { return static_cast<const Surface&>(*m_Surface).GetBuffer(); }
	Surface& GetSurface() { return *m_Surface; }
	const Surface& GetSurface() const { return *m_Surface; }
	const std::shared_ptr<Surface>& GetSharedSurface() const { return m_Surface; } // to share the image with another sprite
private:
	void InitializeStartData();
	const Pixel* FramePixels( unsigned int a_Frame ) const; // the top-left pixel of a frame in the sprite sheet
	std::shared_ptr<Surface> m_Surface;
	int m_Width = 0, m_Height = 0, m_Pitch = 0; // the size of one frame, and the pitch of the sprite sheet
	unsigned int m_Columns = 1, m_Rows = 1;
	unsigned int m_CurrentFrame = 0;
	std::vector<int> m_Start; // for each frame and line: the first pixel that isn't transparent
};

// A bitmap font: an image with the characters next to each other, separated by empty columns.
class Font
{
public:
	Font( std::string_view a_File, std::string_view a_Chars ); // a_Chars: the characters in the image, in order
	void Print( Surface& a_Target, std::string_view a_Text, int a_X, int a_Y, bool clip = false ) const;
	void Centre( Surface& a_Target, std::string_view a_Text, int a_Y ) const;
	int Width( std::string_view a_Text ) const;
	int Height() const { return m_Surface->GetHeight(); }
	void YClip( int y1, int y2 ) { m_CY1 = y1; m_CY2 = y2; }
private:
	std::unique_ptr<Surface> m_Surface;
	std::vector<int> m_Offset, m_Width, m_Trans;
	int m_Height = 0, m_CY1 = 0, m_CY2 = 1024;
};

} // namespace Tmpl8
