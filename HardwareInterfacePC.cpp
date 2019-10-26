#if defined __LINUX__ || defined WIN32 || defined WIN64
#define _USE_MATH_DEFINES
#include <chrono>
#include <thread>
#include <math.h>
#include <string>
#include <future>
#include <iostream>
#include <fstream>
#include "HardwareInterface.hpp"
#include <thread>
#include <filesystem>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <fstream>
#include <functional>

#define DEBUG_PRIORITY 0

#define rcast reinterpret_cast
#define DEBUG_PRIORITY 0

HI2::Color HI2::Color::Black{ 0,0,0,255 };
HI2::Color HI2::Color::White{ 255,255,255,255 };
HI2::Color HI2::Color::Red{ 255,0,0,255 };
HI2::Color HI2::Color::Green{ 0,255,0,255 };
HI2::Color HI2::Color::Blue{ 0,0,255,255 };

SDL_Window* window;
SDL_Renderer* renderer;

HI2::Color _bg;

std::ofstream _log;

int w, h;

void HI2::logWrite(std::string s) {
	_log << s << std::endl;
}

// System
void HI2::systemInit() {
	_log.open("/HI2.log");
	_bg = Color(255, 0, 0, 255);
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO) < 0) {
		_log << SDL_GetError() << std::endl;
		SDL_Log("SDL_Init: %s\n", SDL_GetError());
	}
	TTF_Init();
	IMG_Init(IMG_INIT_PNG | IMG_INIT_WEBP);
	// create an SDL window (OpenGL ES2 always enabled)
	// when SDL_FULLSCREEN flag is not set, viewport is automatically handled by SDL (use SDL_SetWindowSize to "change resolution")
	// available switch SDL2 video modes :
	// 1920 x 1080 @ 32 bpp (SDL_PIXELFORMAT_RGBA8888)
	// 1280 x 720 @ 32 bpp (SDL_PIXELFORMAT_RGBA8888)

	w = 1280;
	h = 720;
	window = SDL_CreateWindow("sdl2_gles2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, SDL_WINDOW_RESIZABLE);
	if (!window) {
		SDL_Log("SDL_CreateWindow: %s\n", SDL_GetError());
		//SDL_Quit();
	}

	// create a renderer (OpenGL ES2)
	renderer = SDL_CreateRenderer(window, 0, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!renderer) {
		SDL_Log("SDL_CreateRenderer: %s\n", SDL_GetError());
		//SDL_Quit();
	}
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	// open CONTROLLER_PLAYER_1 and CONTROLLER_PLAYER_2
	// when railed, both joycons are mapped to joystick #0,
	// else joycons are individually mapped to joystick #0, joystick #1, ...
	// https://github.com/devkitPro/SDL/blob/switch-sdl2/src/joystick/switch/SDL_sysjoystick.c#L45
	//for (int i = 0; i < 2; i++) {
	//	if (SDL_JoystickOpen(i) == NULL) {
	//		SDL_Log("SDL_JoystickOpen: %s\n", SDL_GetError());
	//		SDL_Quit();
	//	}
	//}
	Mix_Init(MIX_INIT_MP3 | MIX_INIT_OGG);
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 4096);
}
void HI2::systemFini() {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	Mix_CloseAudio();
	Mix_Quit();
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
	_log.close();
}

void HI2::startFrame() {
	SDL_SetRenderDrawColor(renderer, _bg.r, _bg.g, _bg.b, _bg.a);
	SDL_RenderClear(renderer);
}
void HI2::setBackgroundColor(Color color) { _bg = color; }

void HI2::playSound(HI2::Audio& audio, float volume) {
	Mix_PlayMusic(rcast<Mix_Music*>(audio._audio), audio._loop ? -1 : 0);
}

void HI2::drawText(Font& font, std::string text, point2D pos, int size, Color c) {
	SDL_Color color = { static_cast<Uint8>(c.r),static_cast<Uint8>(c.g),static_cast<Uint8>(c.b) };
	SDL_Surface* surface = TTF_RenderText_Solid(rcast<TTF_Font*>(font._font), text.c_str(), color);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

	int texW = 0;
	int texH = 0;
	SDL_QueryTexture(texture, NULL, NULL, &texW, &texH);
	SDL_Rect dstrect = { 0, 0, (double)texW / 10.0f * size, (double)texH / 10.0f * size };

	SDL_RenderCopy(renderer, texture, nullptr, &dstrect);

	SDL_DestroyTexture(texture);
	SDL_FreeSurface(surface);

}

void HI2::setTextureColorMod(Texture& texture, Color color)
{
	SDL_SetTextureColorMod(rcast<SDL_Texture*>(texture._texture), color.r, color.g, color.b);
}

void HI2::drawTexture(Texture& texture, int posX, int posY, double scale, double radians) {
	SDL_Rect texture_rect;
	texture_rect.x = posX;  //the x coordinate
	texture_rect.y = posY; // the y coordinate
	SDL_QueryTexture(rcast<SDL_Texture*>(texture._texture), NULL, NULL, &texture_rect.w, &texture_rect.h);
	texture_rect.w *= scale;
	texture_rect.h *= scale;
	texture_rect.w += 1;
	texture_rect.h += 1;


	// PI * rad = 180 * deg
	SDL_RenderCopyEx(renderer, rcast<SDL_Texture*>(texture._texture), NULL, &texture_rect, (radians * 180) / M_PI, nullptr, SDL_FLIP_NONE);

}//TODO

void HI2::drawTexturePart(Texture& texture, point2D texturePartStart, int sizeX, int sizeY, point2D displayPos) {}//TODO

HI2::Texture HI2::mergeTextures(Texture& originTexture, Texture& destinationTexture, point2D position) {
	return destinationTexture;//STUB
}

void HI2::drawRectangle(point2D pos, int width, int height, Color color) {
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
	SDL_Rect r = { pos.x, pos.y, width, height };
	SDL_RenderFillRect(renderer, &r);
}
void HI2::drawPixel(point2D pos, Color color) {
	HI2::drawRectangle(pos, 1, 1, color);
}

void HI2::endFrame() {
	SDL_RenderPresent(renderer);
}


//~~CLASSES~~

//SOUND
HI2::Audio::Audio() {}
HI2::Audio::Audio(std::filesystem::path path, bool loop, float volume) {
	_path = path;
	_loop = loop;
	_volume = volume;
	_audio = Mix_LoadMUS(path.string().c_str());
}
void HI2::Audio::clean() {
	if (_audio != nullptr) {
		Mix_FreeMusic(rcast<Mix_Music*>(_audio));
	}
	_audio = nullptr;
}

//FONT
HI2::Font::Font() {
	_font = nullptr;
	_path = std::filesystem::path();
}

HI2::Font::Font(std::filesystem::path path) {
	_path = path;
	_font = TTF_OpenFont(path.string().c_str(), 10);
	_name = path.filename().replace_extension("").string();
}
void HI2::Font::clean() {
	if (_font != nullptr) {
		TTF_CloseFont(rcast<TTF_Font*>(_font));
	}
	_font = nullptr;
}

//TEXTURE
HI2::Texture::Texture() {}
HI2::Texture::Texture(std::filesystem::path path) {
	_path = path;
	std::cout << "Loading tex:" << path.string().c_str() << std::endl;
	if (path.extension() == ".bmp") {
		std::cout << "BMP" << std::endl;
		SDL_Surface* temp = SDL_LoadBMP(path.string().c_str());
		_texture = SDL_CreateTextureFromSurface(renderer, temp);
		SDL_FreeSurface(temp);
	}
	else {
		std::cout << "Non-BMP" << std::endl;
		_texture = IMG_LoadTexture(renderer, path.string().c_str());
	}
	if (_texture == nullptr) {
		std::cout << "Error loading texture: " << SDL_GetError() << std::endl;
	}
}

HI2::Texture::Texture(std::vector<std::filesystem::path> paths, double step)
{
	_path = paths[0];
	_msPerFrame = step;
	_currentFrame = 0;
	for (auto path : paths)
	{
		std::cout << "Loading tex:" << path.string().c_str() << std::endl;
		if (path.extension() == ".bmp") {
			std::cout << "BMP" << std::endl;
			SDL_Surface* temp = SDL_LoadBMP(path.string().c_str());
			_texture = SDL_CreateTextureFromSurface(renderer, temp);
			SDL_FreeSurface(temp);
		}
		else {
			std::cout << "Non-BMP" << std::endl;
			_texture = IMG_LoadTexture(renderer, path.string().c_str());
		}
		_animationTextures.push_back(_texture);
	}
	_texture = _animationTextures[0];
}

void HI2::Texture::step(double ms)
{
	if (!_animationTextures.empty()) {
		_currentMs += ms;
		while (_currentMs > _msPerFrame)
		{
			_currentMs -= _msPerFrame;
			_currentFrame++;
			if (_currentFrame >= _animationTextures.size())
			{
				_currentFrame = 0;
			}
		}
		_texture = _animationTextures[_currentFrame];
	}
}

void HI2::Texture::clean() {
	if (_texture != nullptr) {
		SDL_DestroyTexture(rcast<SDL_Texture*>(_texture));
	}
	_texture = nullptr;
}

// filesystem
std::filesystem::path HI2::getDataPath() {
	return std::filesystem::path("data");
}

std::filesystem::path HI2::getSavesPath() {
	return std::filesystem::path("saves");
}

// HardwareInfo
int HI2::getScreenHeight() {
	return h;
}
int HI2::getScreenWidth() {
	return w;
}

HI2::PLATFORM HI2::getPlatform() {
	return HI2::PLATFORM_PC;
}

void HI2::consoleInit() {}
void HI2::consoleInit(std::filesystem::path path) {}
void HI2::consoleFini() {}
void HI2::consoleClear() {}
void HI2::sleepThread(unsigned long ns) {
	std::this_thread::sleep_for(std::chrono::nanoseconds(ns));
}

HI2::BUTTON translate(SDL_Keycode s) {
	switch (s) {
	case SDLK_BACKSPACE:
		return HI2::BUTTON::KEY_PLUS;
	case SDLK_s:
		return HI2::BUTTON::KEY_LSTICK_DOWN;
	case SDLK_w:
		return HI2::BUTTON::KEY_LSTICK_UP;
	case SDLK_a:
		return HI2::BUTTON::KEY_LSTICK_LEFT;
	case SDLK_d:
		return HI2::BUTTON::KEY_LSTICK_RIGHT;
	case SDLK_q:
		return HI2::BUTTON::KEY_L;
	case SDLK_e:
		return HI2::BUTTON::KEY_R;
	case SDLK_SPACE:
		return HI2::BUTTON::KEY_MINUS;
	case SDLK_f:
		return HI2::BUTTON::KEY_B;
	case SDLK_r:
		return HI2::BUTTON::KEY_A;
	case SDLK_PLUS:
		return HI2::BUTTON::KEY_ZR;
	case SDLK_MINUS:
		return HI2::BUTTON::KEY_ZL;
	case SDLK_DOWN:
		return HI2::BUTTON::KEY_DDOWN;
	case SDLK_UP:
		return HI2::BUTTON::KEY_DUP;
	case SDLK_LEFT:
		return HI2::BUTTON::KEY_DLEFT;
	case SDLK_RIGHT:
		return HI2::BUTTON::KEY_DRIGHT;
	case SDLK_INSERT:
		return HI2::BUTTON::KEY_X;
	case SDLK_DELETE:
		return HI2::BUTTON::KEY_Y;
	default:
		return HI2::BUTTON::KEY_TOUCH;
	}//TODO acabar aixo

}

unsigned long Down = 0;
unsigned long Held = 0;
unsigned long Up = 0;

bool HI2::aptMainLoop() {
	SDL_Event event;
	Down = 0;
	Up = 0;
	while (SDL_PollEvent(&event))
	{
		switch (event.type) {
		case SDL_QUIT:
		{
			return false;
		}
		case SDL_KEYDOWN:
			Held |= translate(event.key.keysym.sym);
			Down |= event.key.repeat ? 0 : translate(event.key.keysym.sym);
			break;

		case SDL_KEYUP:
			Up |= event.key.state == SDL_PRESSED ? 0 : translate(event.key.keysym.sym);
			Held &= ~(translate(event.key.keysym.sym));
			break;
		case SDL_WINDOWEVENT:
			if (event.window.event == SDL_WINDOWEVENT_RESIZED || event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
				SDL_GetWindowSize(window, &w, &h);
			}
			break;
		default:
			;
		}
	}
	return true;
}

unsigned long HI2::getKeysDown() {
	return Down;
}
unsigned long HI2::getKeysUp() {
	return Up;
}
unsigned long HI2::getKeysHeld() {
	return Held;
}
point2D HI2::getJoystickPos(HI2::JOYSTICK joystick) {
	//::JoystickPosition temp;
	//::hidJoystickRead(&temp,CONTROLLER_P1_AUTO,joystick==HI2::JOY_LEFT?::JOYSTICK_LEFT : (::JOYSTICK_RIGHT));
	point2D res;
	//res.x=temp.dx;
	//res.y=temp.dy;
	return res;
}

point2D HI2::getTouchPos() {
	//touchPosition temp;
	point2D res;
	//if(hidTouchCount()>0){
	//	hidTouchRead(&temp,0);
	//	res.x=temp.px;
	//	res.y=temp.py;
	//}
	return res;
}

#endif
