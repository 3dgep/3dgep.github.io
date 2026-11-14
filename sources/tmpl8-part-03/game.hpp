#pragma once

#include "surface.hpp"

#include <SDL3/SDL_scancode.h>
#include <bitset>

namespace Tmpl8 {

class Game
{
public:
	explicit Game( Surface& screen ) : screen( screen ) {}
	void Init();
	void Shutdown();
	void DrawI( int x, int y ); // draws the letter I, x pixels to the right and y pixels down
	void Tick( float deltaTime );
	void MouseUp( int button ) { /* implement if you want to detect mouse button presses */ }
	void MouseDown( int button ) { /* implement if you want to detect mouse button presses */ }
	void MouseMove( int x, int y ) { /* implement if you want to detect mouse movement (x, y is the mouse position in the window) */ }

	// Keyboard: use the SDL_SCANCODE_ names for the keys, e.g. SDL_SCANCODE_A, SDL_SCANCODE_SPACE, SDL_SCANCODE_LEFT.
	bool GetKey( SDL_Scancode key ) const { return held.test( key ); }             // true while the key is held down
	bool GetKeyPressed( SDL_Scancode key ) const { return pressed.test( key ); }   // true in the first frame the key is down
	bool GetKeyReleased( SDL_Scancode key ) const { return released.test( key ); } // true in the frame the key is released

	// Called by the template, you don't need to call these yourself.
	void KeyDown( SDL_Scancode key ) { keys.set( key ); downEvents.set( key ); }
	void KeyUp( SDL_Scancode key ) { keys.reset( key ); upEvents.set( key ); }
	void ProcessInput() // once per frame, before Tick
	{
		// A key that is tapped quickly (down and up again before the next frame) still counts as pressed.
		pressed = downEvents;
		released = upEvents;
		held = keys | downEvents;
		downEvents.reset();
		upEvents.reset();
	}

private:
	Surface& screen; // the game window: draw on this

	// The example: a spinning gun turret.
	Sprite rotatingGun{ "assets/aagun.tga", 36 };
	int frame = 0;

	std::bitset<SDL_SCANCODE_COUNT> keys;       // keys that are down right now (updated by the template)
	std::bitset<SDL_SCANCODE_COUNT> downEvents; // keys that went down since the previous frame
	std::bitset<SDL_SCANCODE_COUNT> upEvents;   // keys that went up since the previous frame
	std::bitset<SDL_SCANCODE_COUNT> held;       // keys that are down in this frame
	std::bitset<SDL_SCANCODE_COUNT> pressed;    // keys that went down in this frame
	std::bitset<SDL_SCANCODE_COUNT> released;   // keys that went up in this frame
};

} // namespace Tmpl8
