#include <stack>
#include <array>
#if defined __LINUX__ || defined WIN32 || defined WIN64
#define _USE_MATH_DEFINES
#include <chrono>
#include <thread>
#include <math.h>
#include <string>
#include <future>
#include <iostream>
#include <fstream>
#include "HI2.hpp"
#include <thread>
#include <filesystem>

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_mixer.h>

#include <fstream>
#include <functional>


#ifdef __EMSCRIPTEN__
#define SDL_RenderCopyExF SDL_RenderCopyEx
#define SDL_FRect SDL_Rect
#endif
#define DEBUG_PRIORITY 0

#define rcast reinterpret_cast
HI2::Color HI2::Color::Black{ 0,0,0,255 };
HI2::Color HI2::Color::White{ 255,255,255,255 };
HI2::Color HI2::Color::Red{ 255,0,0,255 };
HI2::Color HI2::Color::Green{ 0,255,0,255 };
HI2::Color HI2::Color::Blue{ 0,0,255,255 };
HI2::Color HI2::Color::Yellow{ 255,255,0,255 };
HI2::Color HI2::Color::Orange{ 255,127,0,255 };
HI2::Color HI2::Color::Pink{ 255,0,255,255 };
HI2::Color HI2::Color::DarkestGrey{ 60,60,60,255 };
HI2::Color HI2::Color::DarkGrey{ 100,100,100,255 };
HI2::Color HI2::Color::Grey{ 150,150,150,255 };
HI2::Color HI2::Color::LightGrey{ 200,200,200,255 };
HI2::Color HI2::Color::LightestGrey{ 220,220,220,255 };
HI2::Color HI2::Color::Transparent{ 255,255,255,0 };
HI2::Color HI2::Color::Brown{ 111,92,66,255 };

SDL_Window* window;
SDL_Renderer* renderer;

HI2::Color _bg;

std::ofstream _log;

bool fullscreen;

int w, h;
int oldW, oldH;
point2D mousePosition;

std::array<std::stack<SDL_Texture*>,3> textTextures;
int textureStackIndex=0;

void HI2::logWrite(std::string s) {
	_log << s << std::endl;
}

// System
void HI2::systemInit() {
	fullscreen = false;
	_log.open("/HI2.log");
	_bg = Color(255, 0, 0, 255);
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO) < 0) {
		_log << SDL_GetError() << std::endl;
		SDL_Log("SDL_Init: %s\n", SDL_GetError());
	}
	TTF_Init();
	IMG_Init(IMG_INIT_PNG);
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
	SDL_SetHintWithPriority(SDL_HINT_RENDER_BATCHING,"1",SDL_HINT_OVERRIDE);
	renderer = SDL_CreateRenderer(window, 0, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
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

void HI2::toggleFullscreen()
{
	fullscreen = !fullscreen;
	if (fullscreen)
	{
		SDL_Rect rect;
		oldW = w;
		oldH = h;
		int displayIndex = SDL_GetWindowDisplayIndex(window);
		SDL_GetDisplayBounds(displayIndex, &rect);
		w = rect.w;
		h = rect.h;
	}
	else
	{
		w = oldW;
		h = oldH;
	}

	SDL_SetWindowSize(window, w, h);
	SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
}

void HI2::setBackgroundColor(Color color) { _bg = color; }

void HI2::playSound(HI2::Audio& audio, float volume) {
	Mix_PlayMusic(rcast<Mix_Music*>(audio._audio), audio._loop ? -1 : 0);
}

void HI2::drawText(Font& font, std::string text, point2D pos, int size, Color c) {
	SDL_Color color = { static_cast<Uint8>(c.r),static_cast<Uint8>(c.g),static_cast<Uint8>(c.b) };
	SDL_Surface* surface = TTF_RenderText_Blended(rcast<TTF_Font*>(font._font), text.c_str(), color);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

	int texW = 0;
	int texH = 0;
	SDL_QueryTexture(texture, nullptr, nullptr, &texW, &texH);
	SDL_Rect dstrect = { pos.x, pos.y, int((double)texW / 10.0f * size), int((double)texH / 10.0f * size) };
	SDL_RenderCopyEx(renderer, texture, nullptr, &dstrect, 0, nullptr, SDL_FLIP_NONE);
	textTextures[textureStackIndex].push(texture);
	SDL_FreeSurface(surface);
}

void HI2::setTextureColorMod(Texture& texture, Color color)
{
	SDL_SetTextureColorMod(rcast<SDL_Texture*>(texture._texture.get()->get()), color.r, color.g, color.b);
}

void HI2::drawTexture(const Texture& texture, int posX, int posY, double scale, double radians, HI2::FLIP flip) {
	SDL_Rect destRect;
	destRect.x = posX;  //the x coordinate
	destRect.y = posY; // the y coordinate
	SDL_QueryTexture(rcast<SDL_Texture*>(texture._texture.get()->get()), nullptr, nullptr, &destRect.w, &destRect.h);
	destRect.w *= scale;
	destRect.h *= scale;

	// PI * rad = 180 * deg
	SDL_RenderCopyEx(renderer, rcast<SDL_Texture*>(texture._texture.get()->get()), nullptr, &destRect, (radians * 180) / M_PI, nullptr, (SDL_RendererFlip)flip);
}
void HI2::drawTexture(const Texture& texture, int posX, int posY, point2D size, point2D startPos, double scale, double radians, HI2::FLIP flip) {
	SDL_Rect srcRect;
	srcRect.x = startPos.x;
	srcRect.y = startPos.y;
	srcRect.w = size.x;
	srcRect.h = size.y;

	SDL_Rect destRect;
	destRect.x = posX;  //the x coordinate
	destRect.y = posY; // the y coordinate
	destRect.w = size.x * scale;
	destRect.h = size.y * scale;

	// PI * rad = 180 * deg
	SDL_RenderCopyEx(renderer, rcast<SDL_Texture*>(texture._texture.get()->get()), &srcRect, &destRect, (radians * 180) / M_PI, nullptr, (SDL_RendererFlip)flip);
}

void HI2::drawTextureOverlap(const Texture& texture, int posX, int posY, double scale, double radians, HI2::FLIP flip) {
	SDL_Rect destRect;
	destRect.x = posX;  //the x coordinate
	destRect.y = posY; // the y coordinate
	int w, h;
	SDL_QueryTexture(rcast<SDL_Texture*>(texture._texture.get()->get()), nullptr, nullptr, &w, &h);
	destRect.w = (w*scale)+1;
	destRect.h = (h*scale)+1;

	// PI * rad = 180 * deg
	SDL_RenderCopyEx(renderer, rcast<SDL_Texture*>(texture._texture.get()->get()), nullptr, &destRect, (radians * 180) / M_PI, nullptr, (SDL_RendererFlip)flip);
}

void HI2::drawTextureOverlap(const Texture& texture, int posX, int posY, point2D size, point2D startPos, double scale, double radians, HI2::FLIP flip) {
	SDL_Rect srcRect;
	srcRect.x = startPos.x;
	srcRect.y = startPos.y;
	srcRect.w = size.x;
	srcRect.h = size.y;

	SDL_Rect destRect;
	destRect.x = posX;  //the x coordinate
	destRect.y = posY; // the y coordinate
	destRect.w = (size.x * scale)+1;
	destRect.h = (size.y * scale)+1;
	auto asdf = std::make_shared<SDL_Texture*>(rcast<SDL_Texture*>((void*)(nullptr)));

	// PI * rad = 180 * deg
	SDL_RenderCopyEx(renderer, rcast<SDL_Texture*>(texture._texture.get()->get()), &srcRect, &destRect, (radians * 180) / M_PI, nullptr, (SDL_RendererFlip)flip);
}

void HI2::drawTextureF(const Texture& texture, float posX, float posY, double scale, double radians, HI2::FLIP flip) {
	SDL_FRect destRect;
	destRect.x = posX;  //the x coordinate
	destRect.y = posY; // the y coordinate
	int w, h;
	SDL_QueryTexture(rcast<SDL_Texture*>(texture._texture.get()->get()), nullptr, nullptr, &w, &h);
	destRect.w = w*scale;
	destRect.h = h*scale;

	// PI * rad = 180 * deg
	SDL_RenderCopyExF(renderer, rcast<SDL_Texture*>(texture._texture.get()->get()), nullptr, &destRect, (radians * 180) / M_PI, nullptr, (SDL_RendererFlip)flip);
}

void HI2::drawTextureF(const Texture& texture, float posX, float posY, point2D size, point2D startPos, double scale, double radians, HI2::FLIP flip) {
	SDL_Rect srcRect;
	srcRect.x = startPos.x;
	srcRect.y = startPos.y;
	srcRect.w = size.x;
	srcRect.h = size.y;

	SDL_FRect destRect;
	destRect.x = posX;  //the x coordinate
	destRect.y = posY; // the y coordinate
	destRect.w = size.x * scale;
	destRect.h = size.y * scale;
	auto asdf = std::make_shared<SDL_Texture*>(rcast<SDL_Texture*>((void*)(nullptr)));

	// PI * rad = 180 * deg
	SDL_RenderCopyExF(renderer, rcast<SDL_Texture*>(texture._texture.get()->get()), &srcRect, &destRect, (radians * 180) / M_PI, nullptr, (SDL_RendererFlip)flip);
}

HI2::Texture HI2::mergeTextures(Texture& originTexture, Texture& destinationTexture, point2D position) {
	return destinationTexture;//STUB
}

void HI2::drawRectangle(point2D pos, int width, int height, Color color) {
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
	SDL_Rect r = { pos.x, pos.y, width, height };
	SDL_RenderFillRect(renderer, &r);
}
void HI2::drawEmptyRectangle(point2D pos, int width, int height, Color color) {
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
	SDL_Rect r = { pos.x, pos.y, width, height };
	SDL_RenderDrawRect(renderer, &r);
}
void HI2::drawEmptyRectangle(point2D pos, int width, int height, int strokewidth, Color color) {
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
	SDL_Rect r = { pos.x, pos.y, width, height };
	if(strokewidth == 1){
		drawEmptyRectangle(pos,width,height,color);
	}
	else{
		drawEmptyRectangle(pos,width,height,color);
		drawEmptyRectangle({pos.x+1,pos.y+1},width-2,height-2,strokewidth-1,color);
	}
}
void HI2::drawLine(point2D start, point2D end, Color color){
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
	SDL_RenderDrawLine(renderer,start.x,start.y,end.x,end.y);
}
void HI2::drawLines(const std::vector<point2D>& points, Color color){
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
	SDL_Point* pointArray = new SDL_Point[points.size()];
	for(int i = 0; i < points.size(); ++i){
		pointArray[i].x = points[i].x;
		pointArray[i].y = points[i].y;
	}
	SDL_RenderDrawLines(renderer,pointArray,points.size());
	delete[] pointArray;
}
void HI2::drawPixel(point2D pos, Color color) {
	HI2::drawRectangle(pos, 1, 1, color);
}

void HI2::endFrame() {
	SDL_RenderPresent(renderer);

	textureStackIndex=(textureStackIndex+1)%textTextures.size();
	while (!textTextures[textureStackIndex].empty())
	{
		SDL_DestroyTexture(textTextures[textureStackIndex].top());
		textTextures[textureStackIndex].pop();
	}
}

void HI2::setCursorPos(point2D pos)
{
	SDL_WarpMouseInWindow(window, pos.x, pos.y);
}


//~~CLASSES~~

//COLOR
HI2::Color::Color(unsigned int b){
	a = (unsigned char)b;
	b = (unsigned char)b >> 8;
	g = (unsigned char)b >> 16;
	r = (unsigned char)b >> 24;
}

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
	if (path.extension() == ".bmp") {
		SDL_Surface* temp = SDL_LoadBMP(path.string().c_str());
		_texture = std::make_shared<_internalTextureRAIIWrapper>(SDL_CreateTextureFromSurface(renderer, temp));
		SDL_FreeSurface(temp);
	}
	else {
		_texture = std::make_shared<_internalTextureRAIIWrapper>(IMG_LoadTexture(renderer, path.string().c_str()));
	}
	if (_texture == nullptr) {
		std::cout << "Error loading texture: " << SDL_GetError() << std::endl;
	}
}

HI2::Texture::Texture(point2D size)
{
	_texture = std::make_shared<_internalTextureRAIIWrapper>(SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, size.x, size.y));
	SDL_SetTextureBlendMode(rcast<SDL_Texture*>(_texture.get()->get()), SDL_BLENDMODE_BLEND);
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

constexpr HI2::PLATFORM HI2::getPlatform() {
	return HI2::PLATFORM::PLATFORM_PC;
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
	case SDLK_MINUS:
		return HI2::BUTTON::KEY_DASH;
	case SDLK_DOWN:
		return HI2::BUTTON::KEY_DOWN;
	case SDLK_UP:
		return HI2::BUTTON::KEY_UP;
	case SDLK_LEFT:
		return HI2::BUTTON::KEY_LEFT;
	case SDLK_RIGHT:
		return HI2::BUTTON::KEY_RIGHT;
	case SDLK_a:
		return HI2::BUTTON::KEY_A;
	case SDLK_b:
		return HI2::BUTTON::KEY_B;
	case SDLK_c:
		return HI2::BUTTON::KEY_C;
	case SDLK_d:
		return HI2::BUTTON::KEY_D;
	case SDLK_e:
		return HI2::BUTTON::KEY_E;
	case SDLK_f:
		return HI2::BUTTON::KEY_F;
	case SDLK_g:
		return HI2::BUTTON::KEY_G;
	case SDLK_h:
		return HI2::BUTTON::KEY_H;
	case SDLK_i:
		return HI2::BUTTON::KEY_I;
	case SDLK_j:
		return HI2::BUTTON::KEY_J;
	case SDLK_k:
		return HI2::BUTTON::KEY_K;
	case SDLK_l:
		return HI2::BUTTON::KEY_L;
	case SDLK_m:
		return HI2::BUTTON::KEY_M;
	case SDLK_n:
		return HI2::BUTTON::KEY_N;
	case SDLK_o:
		return HI2::BUTTON::KEY_O;
	case SDLK_p:
		return HI2::BUTTON::KEY_P;
	case SDLK_q:
		return HI2::BUTTON::KEY_Q;
	case SDLK_r:
		return HI2::BUTTON::KEY_R;
	case SDLK_s:
		return HI2::BUTTON::KEY_S;
	case SDLK_t:
		return HI2::BUTTON::KEY_T;
	case SDLK_u:
		return HI2::BUTTON::KEY_U;
	case SDLK_v:
		return HI2::BUTTON::KEY_V;
	case SDLK_w:
		return HI2::BUTTON::KEY_W;
	case SDLK_x:
		return HI2::BUTTON::KEY_X;
	case SDLK_y:
		return HI2::BUTTON::KEY_Y;
	case SDLK_z:
		return HI2::BUTTON::KEY_Z;
	case SDLK_F11:
		return HI2::BUTTON::KEY_F11;
	case SDL_BUTTON_LEFT:
		return HI2::BUTTON::KEY_LEFTCLICK;
	case SDL_BUTTON_RIGHT:
		return HI2::BUTTON::KEY_RIGHTCLICK;
	case SDLK_RETURN:
		return HI2::BUTTON::KEY_ENTER;
	case SDLK_ESCAPE:
		return HI2::BUTTON::KEY_ESCAPE;
	case SDLK_BACKSPACE:
		return HI2::BUTTON::KEY_BACKSPACE;
	case SDLK_SPACE:
		return HI2::BUTTON::KEY_SPACE;
	case SDLK_LSHIFT:
	case SDLK_RSHIFT:
		return HI2::BUTTON::KEY_SHIFT;
	case SDLK_LCTRL:
	case SDLK_RCTRL:
		return HI2::BUTTON::KEY_CONTROL;
	case SDLK_0:
		return HI2::BUTTON::KEY_0;
	case SDLK_1:
		return HI2::BUTTON::KEY_1;
	case SDLK_2:
		return HI2::BUTTON::KEY_2;
	case SDLK_3:
		return HI2::BUTTON::KEY_3;
	case SDLK_4:
		return HI2::BUTTON::KEY_4;
	case SDLK_5:
		return HI2::BUTTON::KEY_5;
	case SDLK_6:
		return HI2::BUTTON::KEY_6;
	case SDLK_7:
		return HI2::BUTTON::KEY_7;
	case SDLK_8:
		return HI2::BUTTON::KEY_8;
	case SDLK_9:
		return HI2::BUTTON::KEY_9;
	case SDLK_PLUS:
		return HI2::BUTTON::KEY_PLUS;
	case SDLK_BACKQUOTE:
	case 186:
		return HI2::BUTTON::KEY_CONSOLE;
	default:
		return (HI2::BUTTON)(HI2::BUTTON_SIZE - 1);
	}//TODO acabar aixo

}

std::bitset<HI2::BUTTON_SIZE> Down = 0;
std::bitset<HI2::BUTTON_SIZE> Held = 0;
std::bitset<HI2::BUTTON_SIZE> Up = 0;

bool HI2::aptMainLoop() {
	SDL_Event event;
	Down.reset();
	Up.reset();
	while (SDL_PollEvent(&event))
	{
		switch (event.type) {
		case SDL_QUIT:
		{
			return false;
		}
		case SDL_KEYDOWN:
			Held[translate(event.key.keysym.sym)] = true;
			Down[translate(event.key.keysym.sym)] = !event.key.repeat;
			break;

		case SDL_KEYUP:
			if (event.key.state != SDL_PRESSED)
				Up[translate(event.key.keysym.sym)] = true;
			Held[translate(event.key.keysym.sym)] = false;
			break;
		case SDL_WINDOWEVENT:
			if (event.window.event == SDL_WINDOWEVENT_RESIZED || event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
				SDL_GetWindowSize(window, &w, &h);
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
			Held[translate(event.button.button)] = true;
			Down[translate(event.button.button)] = true;
			break;
		case SDL_MOUSEBUTTONUP:
			Up[translate(event.button.button)] = true;
			Held[translate(event.button.button)] = false;
			break;
		case SDL_MOUSEMOTION:
			mousePosition.x = event.motion.x;
			mousePosition.y = event.motion.y;
			break;
		case SDL_MOUSEWHEEL:
			Down[event.wheel.y > 0 ? HI2::BUTTON::KEY_MOUSEWHEEL_UP : HI2::BUTTON::KEY_MOUSEWHEEL_DOWN] = true;
			break;
		default:
			break;
		}
	}
	// Fullscreen
	if (Down[HI2::BUTTON::KEY_F11])
	{
		HI2::toggleFullscreen();
	}

	calculateAggregators(Down);
	calculateAggregators(Up);
	calculateAggregators(Held);

	return true;
}

const std::bitset<HI2::BUTTON_SIZE>& HI2::getKeysDown() {
	return Down;
}
const std::bitset<HI2::BUTTON_SIZE>& HI2::getKeysUp() {
	return Up;
}
const std::bitset<HI2::BUTTON_SIZE>& HI2::getKeysHeld() {
	return Held;
}
point2D HI2::getJoystickPos(HI2::JOYSTICK joystick) {
	point2D res;
	return res;
}

point2D HI2::getTouchPos() {
	return mousePosition;
}

void HI2::setRenderTarget(HI2::Texture* t, bool clear) {
	if (t == nullptr) {
		SDL_SetRenderTarget(renderer, nullptr);
		SDL_SetRenderDrawColor(renderer, _bg.r, _bg.g, _bg.b, _bg.a);
	}
	else {
		SDL_SetRenderTarget(renderer, rcast<SDL_Texture*>(t->_texture.get()->get()));
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	}

	if (clear) {
		SDL_RenderClear(renderer);
	}
}

HI2::Texture HI2::getRenderTarget(){
	Texture result;
	result._texture = std::make_shared<Texture::_internalWeakTextureRAIIWrapper>(SDL_GetRenderTarget(renderer));
	return result;
}

void HI2::createDirectories(std::filesystem::path p) {
	std::filesystem::create_directories(p);
}

void HI2::deleteDirectory(std::filesystem::path p) {
	std::filesystem::remove_all(p);
}

point2D HI2::getTextureSize(Texture& texture) {
	point2D result;
	SDL_QueryTexture(rcast<SDL_Texture*>(texture._texture.get()->get()), nullptr, nullptr, &result.x, &result.y);
	return result;
}

HI2::Texture::_internalWeakTextureRAIIWrapper::_internalWeakTextureRAIIWrapper(void *pointer)
{
	_texture=pointer;
}

HI2::Texture::_internalTextureRAIIWrapper::~_internalTextureRAIIWrapper()
{
	SDL_DestroyTexture(rcast<SDL_Texture*>(_texture));
}

void* HI2::Texture::_internalWeakTextureRAIIWrapper::get() const
{
	return rcast<void*>(_texture);
}



HI2::Texture::_internalWeakTextureRAIIWrapper::~_internalWeakTextureRAIIWrapper(){}

void HI2::setClipboard(std::string mucho_texto){
	SDL_SetClipboardText(mucho_texto.c_str());
}

std::string HI2::getClipboard(){
	if(!SDL_HasClipboardText())
		return std::string();
	std::string temp = SDL_GetClipboardText();
	return temp;
}

#endif