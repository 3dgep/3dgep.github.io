#pragma once

#include <cstdint>

namespace rast
{

// A 32-bit color, packed as 0xAARRGGBB (SDL_PIXELFORMAT_ARGB8888).
//
//   bits 31..24  alpha
//   bits 23..16  red
//   bits 15..8   green
//   bits  7..0   blue
struct Color
{
    std::uint32_t argb = 0xFF000000; // Opaque black.

    constexpr Color() = default;

    constexpr explicit Color(std::uint32_t value)
        : argb{ value }
    {}

    constexpr Color(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a = 255)
        : argb{ static_cast<std::uint32_t>(a) << 24 |
                static_cast<std::uint32_t>(r) << 16 |
                static_cast<std::uint32_t>(g) << 8 |
                static_cast<std::uint32_t>(b) }
    {}

    constexpr std::uint8_t A() const { return static_cast<std::uint8_t>(argb >> 24); }
    constexpr std::uint8_t R() const { return static_cast<std::uint8_t>(argb >> 16); }
    constexpr std::uint8_t G() const { return static_cast<std::uint8_t>(argb >> 8); }
    constexpr std::uint8_t B() const { return static_cast<std::uint8_t>(argb); }

    constexpr bool operator==(const Color&) const = default;

    // Named colors: Transparent, and the 148 color keywords of CSS (like CornflowerBlue).
    // They are declared here and defined below the struct, because a struct
    // is incomplete until its closing brace, and a constexpr variable needs a complete type.
    static const Color Transparent;
    static const Color
        AliceBlue, AntiqueWhite, Aqua, Aquamarine, Azure, Beige, Bisque, Black, BlanchedAlmond,
        Blue, BlueViolet, Brown, BurlyWood, CadetBlue, Chartreuse, Chocolate, Coral, CornflowerBlue,
        Cornsilk, Crimson, Cyan, DarkBlue, DarkCyan, DarkGoldenrod, DarkGray, DarkGreen, DarkGrey,
        DarkKhaki, DarkMagenta, DarkOliveGreen, DarkOrange, DarkOrchid, DarkRed, DarkSalmon,
        DarkSeaGreen, DarkSlateBlue, DarkSlateGray, DarkSlateGrey, DarkTurquoise, DarkViolet,
        DeepPink, DeepSkyBlue, DimGray, DimGrey, DodgerBlue, FireBrick, FloralWhite, ForestGreen,
        Fuchsia, Gainsboro, GhostWhite, Gold, Goldenrod, Gray, Green, GreenYellow, Grey, Honeydew,
        HotPink, IndianRed, Indigo, Ivory, Khaki, Lavender, LavenderBlush, LawnGreen, LemonChiffon,
        LightBlue, LightCoral, LightCyan, LightGoldenrodYellow, LightGray, LightGreen, LightGrey,
        LightPink, LightSalmon, LightSeaGreen, LightSkyBlue, LightSlateGray, LightSlateGrey,
        LightSteelBlue, LightYellow, Lime, LimeGreen, Linen, Magenta, Maroon, MediumAquamarine,
        MediumBlue, MediumOrchid, MediumPurple, MediumSeaGreen, MediumSlateBlue, MediumSpringGreen,
        MediumTurquoise, MediumVioletRed, MidnightBlue, MintCream, MistyRose, Moccasin, NavajoWhite,
        Navy, OldLace, Olive, OliveDrab, Orange, OrangeRed, Orchid, PaleGoldenrod, PaleGreen,
        PaleTurquoise, PaleVioletRed, PapayaWhip, PeachPuff, Peru, Pink, Plum, PowderBlue, Purple,
        RebeccaPurple, Red, RosyBrown, RoyalBlue, SaddleBrown, Salmon, SandyBrown, SeaGreen,
        Seashell, Sienna, Silver, SkyBlue, SlateBlue, SlateGray, SlateGrey, Snow, SpringGreen,
        SteelBlue, Tan, Teal, Thistle, Tomato, Turquoise, Violet, Wheat, White, WhiteSmoke, Yellow,
        YellowGreen;
};

// A Color must be exactly one 32-bit pixel, so a row of Colors can be
// copied straight into an ARGB8888 texture.
static_assert(sizeof(Color) == 4);

static_assert(Color{ 0x12, 0x34, 0x56, 0x78 }.argb == 0x78123456);
static_assert(Color{ 0x78123456 }.R() == 0x12);

// The values of the named colors.
inline constexpr Color Color::Transparent         { 0x00000000 };
inline constexpr Color Color::AliceBlue           { 0xF0, 0xF8, 0xFF };
inline constexpr Color Color::AntiqueWhite        { 0xFA, 0xEB, 0xD7 };
inline constexpr Color Color::Aqua                { 0x00, 0xFF, 0xFF };
inline constexpr Color Color::Aquamarine          { 0x7F, 0xFF, 0xD4 };
inline constexpr Color Color::Azure               { 0xF0, 0xFF, 0xFF };
inline constexpr Color Color::Beige               { 0xF5, 0xF5, 0xDC };
inline constexpr Color Color::Bisque              { 0xFF, 0xE4, 0xC4 };
inline constexpr Color Color::Black               { 0x00, 0x00, 0x00 };
inline constexpr Color Color::BlanchedAlmond      { 0xFF, 0xEB, 0xCD };
inline constexpr Color Color::Blue                { 0x00, 0x00, 0xFF };
inline constexpr Color Color::BlueViolet          { 0x8A, 0x2B, 0xE2 };
inline constexpr Color Color::Brown               { 0xA5, 0x2A, 0x2A };
inline constexpr Color Color::BurlyWood           { 0xDE, 0xB8, 0x87 };
inline constexpr Color Color::CadetBlue           { 0x5F, 0x9E, 0xA0 };
inline constexpr Color Color::Chartreuse          { 0x7F, 0xFF, 0x00 };
inline constexpr Color Color::Chocolate           { 0xD2, 0x69, 0x1E };
inline constexpr Color Color::Coral               { 0xFF, 0x7F, 0x50 };
inline constexpr Color Color::CornflowerBlue      { 0x64, 0x95, 0xED };
inline constexpr Color Color::Cornsilk            { 0xFF, 0xF8, 0xDC };
inline constexpr Color Color::Crimson             { 0xDC, 0x14, 0x3C };
inline constexpr Color Color::Cyan                { 0x00, 0xFF, 0xFF };
inline constexpr Color Color::DarkBlue            { 0x00, 0x00, 0x8B };
inline constexpr Color Color::DarkCyan            { 0x00, 0x8B, 0x8B };
inline constexpr Color Color::DarkGoldenrod       { 0xB8, 0x86, 0x0B };
inline constexpr Color Color::DarkGray            { 0xA9, 0xA9, 0xA9 };
inline constexpr Color Color::DarkGreen           { 0x00, 0x64, 0x00 };
inline constexpr Color Color::DarkGrey            { 0xA9, 0xA9, 0xA9 };
inline constexpr Color Color::DarkKhaki           { 0xBD, 0xB7, 0x6B };
inline constexpr Color Color::DarkMagenta         { 0x8B, 0x00, 0x8B };
inline constexpr Color Color::DarkOliveGreen      { 0x55, 0x6B, 0x2F };
inline constexpr Color Color::DarkOrange          { 0xFF, 0x8C, 0x00 };
inline constexpr Color Color::DarkOrchid          { 0x99, 0x32, 0xCC };
inline constexpr Color Color::DarkRed             { 0x8B, 0x00, 0x00 };
inline constexpr Color Color::DarkSalmon          { 0xE9, 0x96, 0x7A };
inline constexpr Color Color::DarkSeaGreen        { 0x8F, 0xBC, 0x8F };
inline constexpr Color Color::DarkSlateBlue       { 0x48, 0x3D, 0x8B };
inline constexpr Color Color::DarkSlateGray       { 0x2F, 0x4F, 0x4F };
inline constexpr Color Color::DarkSlateGrey       { 0x2F, 0x4F, 0x4F };
inline constexpr Color Color::DarkTurquoise       { 0x00, 0xCE, 0xD1 };
inline constexpr Color Color::DarkViolet          { 0x94, 0x00, 0xD3 };
inline constexpr Color Color::DeepPink            { 0xFF, 0x14, 0x93 };
inline constexpr Color Color::DeepSkyBlue         { 0x00, 0xBF, 0xFF };
inline constexpr Color Color::DimGray             { 0x69, 0x69, 0x69 };
inline constexpr Color Color::DimGrey             { 0x69, 0x69, 0x69 };
inline constexpr Color Color::DodgerBlue          { 0x1E, 0x90, 0xFF };
inline constexpr Color Color::FireBrick           { 0xB2, 0x22, 0x22 };
inline constexpr Color Color::FloralWhite         { 0xFF, 0xFA, 0xF0 };
inline constexpr Color Color::ForestGreen         { 0x22, 0x8B, 0x22 };
inline constexpr Color Color::Fuchsia             { 0xFF, 0x00, 0xFF };
inline constexpr Color Color::Gainsboro           { 0xDC, 0xDC, 0xDC };
inline constexpr Color Color::GhostWhite          { 0xF8, 0xF8, 0xFF };
inline constexpr Color Color::Gold                { 0xFF, 0xD7, 0x00 };
inline constexpr Color Color::Goldenrod           { 0xDA, 0xA5, 0x20 };
inline constexpr Color Color::Gray                { 0x80, 0x80, 0x80 };
inline constexpr Color Color::Green               { 0x00, 0x80, 0x00 };
inline constexpr Color Color::GreenYellow         { 0xAD, 0xFF, 0x2F };
inline constexpr Color Color::Grey                { 0x80, 0x80, 0x80 };
inline constexpr Color Color::Honeydew            { 0xF0, 0xFF, 0xF0 };
inline constexpr Color Color::HotPink             { 0xFF, 0x69, 0xB4 };
inline constexpr Color Color::IndianRed           { 0xCD, 0x5C, 0x5C };
inline constexpr Color Color::Indigo              { 0x4B, 0x00, 0x82 };
inline constexpr Color Color::Ivory               { 0xFF, 0xFF, 0xF0 };
inline constexpr Color Color::Khaki               { 0xF0, 0xE6, 0x8C };
inline constexpr Color Color::Lavender            { 0xE6, 0xE6, 0xFA };
inline constexpr Color Color::LavenderBlush       { 0xFF, 0xF0, 0xF5 };
inline constexpr Color Color::LawnGreen           { 0x7C, 0xFC, 0x00 };
inline constexpr Color Color::LemonChiffon        { 0xFF, 0xFA, 0xCD };
inline constexpr Color Color::LightBlue           { 0xAD, 0xD8, 0xE6 };
inline constexpr Color Color::LightCoral          { 0xF0, 0x80, 0x80 };
inline constexpr Color Color::LightCyan           { 0xE0, 0xFF, 0xFF };
inline constexpr Color Color::LightGoldenrodYellow{ 0xFA, 0xFA, 0xD2 };
inline constexpr Color Color::LightGray           { 0xD3, 0xD3, 0xD3 };
inline constexpr Color Color::LightGreen          { 0x90, 0xEE, 0x90 };
inline constexpr Color Color::LightGrey           { 0xD3, 0xD3, 0xD3 };
inline constexpr Color Color::LightPink           { 0xFF, 0xB6, 0xC1 };
inline constexpr Color Color::LightSalmon         { 0xFF, 0xA0, 0x7A };
inline constexpr Color Color::LightSeaGreen       { 0x20, 0xB2, 0xAA };
inline constexpr Color Color::LightSkyBlue        { 0x87, 0xCE, 0xFA };
inline constexpr Color Color::LightSlateGray      { 0x77, 0x88, 0x99 };
inline constexpr Color Color::LightSlateGrey      { 0x77, 0x88, 0x99 };
inline constexpr Color Color::LightSteelBlue      { 0xB0, 0xC4, 0xDE };
inline constexpr Color Color::LightYellow         { 0xFF, 0xFF, 0xE0 };
inline constexpr Color Color::Lime                { 0x00, 0xFF, 0x00 };
inline constexpr Color Color::LimeGreen           { 0x32, 0xCD, 0x32 };
inline constexpr Color Color::Linen               { 0xFA, 0xF0, 0xE6 };
inline constexpr Color Color::Magenta             { 0xFF, 0x00, 0xFF };
inline constexpr Color Color::Maroon              { 0x80, 0x00, 0x00 };
inline constexpr Color Color::MediumAquamarine    { 0x66, 0xCD, 0xAA };
inline constexpr Color Color::MediumBlue          { 0x00, 0x00, 0xCD };
inline constexpr Color Color::MediumOrchid        { 0xBA, 0x55, 0xD3 };
inline constexpr Color Color::MediumPurple        { 0x93, 0x70, 0xDB };
inline constexpr Color Color::MediumSeaGreen      { 0x3C, 0xB3, 0x71 };
inline constexpr Color Color::MediumSlateBlue     { 0x7B, 0x68, 0xEE };
inline constexpr Color Color::MediumSpringGreen   { 0x00, 0xFA, 0x9A };
inline constexpr Color Color::MediumTurquoise     { 0x48, 0xD1, 0xCC };
inline constexpr Color Color::MediumVioletRed     { 0xC7, 0x15, 0x85 };
inline constexpr Color Color::MidnightBlue        { 0x19, 0x19, 0x70 };
inline constexpr Color Color::MintCream           { 0xF5, 0xFF, 0xFA };
inline constexpr Color Color::MistyRose           { 0xFF, 0xE4, 0xE1 };
inline constexpr Color Color::Moccasin            { 0xFF, 0xE4, 0xB5 };
inline constexpr Color Color::NavajoWhite         { 0xFF, 0xDE, 0xAD };
inline constexpr Color Color::Navy                { 0x00, 0x00, 0x80 };
inline constexpr Color Color::OldLace             { 0xFD, 0xF5, 0xE6 };
inline constexpr Color Color::Olive               { 0x80, 0x80, 0x00 };
inline constexpr Color Color::OliveDrab           { 0x6B, 0x8E, 0x23 };
inline constexpr Color Color::Orange              { 0xFF, 0xA5, 0x00 };
inline constexpr Color Color::OrangeRed           { 0xFF, 0x45, 0x00 };
inline constexpr Color Color::Orchid              { 0xDA, 0x70, 0xD6 };
inline constexpr Color Color::PaleGoldenrod       { 0xEE, 0xE8, 0xAA };
inline constexpr Color Color::PaleGreen           { 0x98, 0xFB, 0x98 };
inline constexpr Color Color::PaleTurquoise       { 0xAF, 0xEE, 0xEE };
inline constexpr Color Color::PaleVioletRed       { 0xDB, 0x70, 0x93 };
inline constexpr Color Color::PapayaWhip          { 0xFF, 0xEF, 0xD5 };
inline constexpr Color Color::PeachPuff           { 0xFF, 0xDA, 0xB9 };
inline constexpr Color Color::Peru                { 0xCD, 0x85, 0x3F };
inline constexpr Color Color::Pink                { 0xFF, 0xC0, 0xCB };
inline constexpr Color Color::Plum                { 0xDD, 0xA0, 0xDD };
inline constexpr Color Color::PowderBlue          { 0xB0, 0xE0, 0xE6 };
inline constexpr Color Color::Purple              { 0x80, 0x00, 0x80 };
inline constexpr Color Color::RebeccaPurple       { 0x66, 0x33, 0x99 };
inline constexpr Color Color::Red                 { 0xFF, 0x00, 0x00 };
inline constexpr Color Color::RosyBrown           { 0xBC, 0x8F, 0x8F };
inline constexpr Color Color::RoyalBlue           { 0x41, 0x69, 0xE1 };
inline constexpr Color Color::SaddleBrown         { 0x8B, 0x45, 0x13 };
inline constexpr Color Color::Salmon              { 0xFA, 0x80, 0x72 };
inline constexpr Color Color::SandyBrown          { 0xF4, 0xA4, 0x60 };
inline constexpr Color Color::SeaGreen            { 0x2E, 0x8B, 0x57 };
inline constexpr Color Color::Seashell            { 0xFF, 0xF5, 0xEE };
inline constexpr Color Color::Sienna              { 0xA0, 0x52, 0x2D };
inline constexpr Color Color::Silver              { 0xC0, 0xC0, 0xC0 };
inline constexpr Color Color::SkyBlue             { 0x87, 0xCE, 0xEB };
inline constexpr Color Color::SlateBlue           { 0x6A, 0x5A, 0xCD };
inline constexpr Color Color::SlateGray           { 0x70, 0x80, 0x90 };
inline constexpr Color Color::SlateGrey           { 0x70, 0x80, 0x90 };
inline constexpr Color Color::Snow                { 0xFF, 0xFA, 0xFA };
inline constexpr Color Color::SpringGreen         { 0x00, 0xFF, 0x7F };
inline constexpr Color Color::SteelBlue           { 0x46, 0x82, 0xB4 };
inline constexpr Color Color::Tan                 { 0xD2, 0xB4, 0x8C };
inline constexpr Color Color::Teal                { 0x00, 0x80, 0x80 };
inline constexpr Color Color::Thistle             { 0xD8, 0xBF, 0xD8 };
inline constexpr Color Color::Tomato              { 0xFF, 0x63, 0x47 };
inline constexpr Color Color::Turquoise           { 0x40, 0xE0, 0xD0 };
inline constexpr Color Color::Violet              { 0xEE, 0x82, 0xEE };
inline constexpr Color Color::Wheat               { 0xF5, 0xDE, 0xB3 };
inline constexpr Color Color::White               { 0xFF, 0xFF, 0xFF };
inline constexpr Color Color::WhiteSmoke          { 0xF5, 0xF5, 0xF5 };
inline constexpr Color Color::Yellow              { 0xFF, 0xFF, 0x00 };
inline constexpr Color Color::YellowGreen         { 0x9A, 0xCD, 0x32 };

static_assert(Color::CornflowerBlue == Color{ 0xFF6495ED });
static_assert(Color::Transparent.A() == 0);

} // namespace rast
