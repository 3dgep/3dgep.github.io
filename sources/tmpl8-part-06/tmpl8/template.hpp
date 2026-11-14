// Template, BUAS version https://www.buas.nl/games
// IGAD/BUAS(NHTV)/UU - Jacco Bikker - 2006-2020
// CMGT/BUAS - Jeremiah van Oosten - 2021-2026

#pragma once

#include <cmath>
#include <string>

// Size of the game window, in pixels.
constexpr int ScreenWidth = 800;
constexpr int ScreenHeight = 512;

// Set to true to start in fullscreen (the 800x512 image is scaled up to fit the screen).
// Alt+Enter or F11 switches between fullscreen and a window while the game runs.
constexpr bool StartFullscreen = false;

// VSync: wait for the screen before showing the next frame, so the game runs at the refresh rate of the screen
// (e.g. 60 frames per second). Set to false to draw as many frames per second as the computer can.
constexpr bool StartVSync = true;

// Shown in the window title. Make sure you are using the most recent version.
constexpr const char* TemplateVersion = "Tmpl8 v2026-11";

namespace Tmpl8 {

// Random numbers.
float Rand( float range ); // a random float in [0, range)
int IRand( int range );    // a random int in [0, range - 1]

template <typename T>
constexpr T Min( T a, T b ) { return (a > b) ? b : a; }

template <typename T>
constexpr T Max( T a, T b ) { return (a > b) ? a : b; }

template <typename T>
constexpr T Clamp( T value, T min, T max ) { return Min( max, Max( value, min ) ); }

constexpr float PI = 3.14159265358979323846f;

// Shows a message box with an error message, then closes the application.
[[noreturn]] void NotifyUser( const std::string& message );

// Fullscreen (Alt+Enter or F11 toggles it too, also in a web browser).
// In a browser, fullscreen can only start in response to a key press or a mouse click.
void SetFullscreen( bool fullscreen );
bool IsFullscreen();
void ToggleFullscreen();

// VSync (see StartVSync). In a browser, the browser always waits for the screen: there, these do nothing,
// and IsVSync() is always true.
void SetVSync( bool vsync );
bool IsVSync();
void ToggleVSync();

// A 2D vector: a position, a direction or a velocity.
struct vec2
{
	float x = 0, y = 0;

	vec2() = default;
	vec2( float v ) : x( v ), y( v ) {}
	vec2( float x, float y ) : x( x ), y( y ) {}

	vec2 operator - () const { return { -x, -y }; }
	vec2 operator + ( const vec2& v ) const { return { x + v.x, y + v.y }; }
	vec2 operator - ( const vec2& v ) const { return { x - v.x, y - v.y }; }
	vec2 operator * ( const vec2& v ) const { return { x * v.x, y * v.y }; }
	vec2 operator * ( float s ) const { return { x * s, y * s }; }
	vec2 operator / ( float s ) const { return { x / s, y / s }; }
	vec2& operator += ( const vec2& v ) { x += v.x; y += v.y; return *this; }
	vec2& operator -= ( const vec2& v ) { x -= v.x; y -= v.y; return *this; }
	vec2& operator *= ( const vec2& v ) { x *= v.x; y *= v.y; return *this; }
	vec2& operator *= ( float s ) { x *= s; y *= s; return *this; }

	float length() const { return std::sqrt( x * x + y * y ); }
	float sqrLength() const { return x * x + y * y; }
	vec2 normalized() const { const float r = 1.0f / length(); return { x * r, y * r }; }
	void normalize() { *this = normalized(); }
	float dot( const vec2& v ) const { return x * v.x + y * v.y; }
};

inline vec2 operator * ( float s, const vec2& v ) { return v * s; }
inline float dot( const vec2& a, const vec2& b ) { return a.dot( b ); }
inline vec2 normalize( const vec2& v ) { return v.normalized(); }

// A 3D vector.
struct vec3
{
	float x = 0, y = 0, z = 0;

	vec3() = default;
	vec3( float v ) : x( v ), y( v ), z( v ) {}
	vec3( float x, float y, float z ) : x( x ), y( y ), z( z ) {}

	vec3 operator - () const { return { -x, -y, -z }; }
	vec3 operator + ( const vec3& v ) const { return { x + v.x, y + v.y, z + v.z }; }
	vec3 operator - ( const vec3& v ) const { return { x - v.x, y - v.y, z - v.z }; }
	vec3 operator * ( const vec3& v ) const { return { x * v.x, y * v.y, z * v.z }; }
	vec3 operator * ( float s ) const { return { x * s, y * s, z * s }; }
	vec3 operator / ( float s ) const { return { x / s, y / s, z / s }; }
	vec3& operator += ( const vec3& v ) { x += v.x; y += v.y; z += v.z; return *this; }
	vec3& operator -= ( const vec3& v ) { x -= v.x; y -= v.y; z -= v.z; return *this; }
	vec3& operator *= ( const vec3& v ) { x *= v.x; y *= v.y; z *= v.z; return *this; }
	vec3& operator *= ( float s ) { x *= s; y *= s; z *= s; return *this; }

	float length() const { return std::sqrt( x * x + y * y + z * z ); }
	float sqrLength() const { return x * x + y * y + z * z; }
	vec3 normalized() const { const float r = 1.0f / length(); return { x * r, y * r, z * r }; }
	void normalize() { *this = normalized(); }
	float dot( const vec3& v ) const { return x * v.x + y * v.y + z * v.z; }
	vec3 cross( const vec3& v ) const { return { y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x }; }
};

inline vec3 operator * ( float s, const vec3& v ) { return v * s; }
inline float dot( const vec3& a, const vec3& b ) { return a.dot( b ); }
inline vec3 cross( const vec3& a, const vec3& b ) { return a.cross( b ); }
inline vec3 normalize( const vec3& v ) { return v.normalized(); }

} // namespace Tmpl8
