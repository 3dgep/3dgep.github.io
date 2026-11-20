#pragma once

#include "surface.hpp"
#include "template.hpp" // ScreenWidth/Height, Rand, vec2, fullscreen and VSync

#include <SDL3/SDL_keyboard.h> // SDL_Scancode, SDL_Keycode and SDL_GetScancodeFromKey
#include <bitset>

namespace Tmpl8 {

class Game
{
public:
	explicit Game( Surface& screen ) : screen( screen ) {}
	void Init();
	void Shutdown();
	void Tick( float deltaTime );
	void Update( float deltaTime ); // drives the tank
	void Edit( float deltaTime );   // the map editor
	// button 1 is the left mouse button, 3 the right one
	void MouseUp( int button ) { if (button == 1) mouseLeft = false; if (button == 3) mouseRight = false; }
	void MouseDown( int button ) { if (button == 1) mouseLeft = true; if (button == 3) mouseRight = true; }
	void MouseMove( int x, int y ) { mousex = x, mousey = y; }

	// Keyboard: use the SDL_SCANCODE_ names for the keys, e.g. SDL_SCANCODE_A, SDL_SCANCODE_SPACE, SDL_SCANCODE_LEFT.
	bool GetKey( SDL_Scancode key ) const { return held.test( key ); }             // true while the key is held down
	bool GetKeyPressed( SDL_Scancode key ) const { return pressed.test( key ); }   // true in the first frame the key is down
	bool GetKeyReleased( SDL_Scancode key ) const { return released.test( key ); } // true in the frame the key is released
	// KeyFor( SDLK_S ) is the key with an "s" on it, on any keyboard (for keys where the letter matters, like S for save)
	SDL_Scancode KeyFor( SDL_Keycode key ) const { return SDL_GetScancodeFromKey( key, nullptr ); }

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

	int mousex = 0, mousey = 0;                 // the position of the mouse
	bool mouseLeft = false, mouseRight = false; // true while the left or right mouse button is held down

	std::bitset<SDL_SCANCODE_COUNT> keys;       // keys that are down right now (updated by the template)
	std::bitset<SDL_SCANCODE_COUNT> downEvents; // keys that went down since the previous frame
	std::bitset<SDL_SCANCODE_COUNT> upEvents;   // keys that went up since the previous frame
	std::bitset<SDL_SCANCODE_COUNT> held;       // keys that are down in this frame
	std::bitset<SDL_SCANCODE_COUNT> pressed;    // keys that went down in this frame
	std::bitset<SDL_SCANCODE_COUNT> released;   // keys that went up in this frame
};

} // namespace Tmpl8
