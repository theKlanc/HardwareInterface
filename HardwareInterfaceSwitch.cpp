#ifdef __SWITCH__
#define _USE_MATH_DEFINES
#include "HardwareInterface.hpp"
#include <switch.h>
#include <stack>
#include <filesystem>
#include <iostream>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <fstream>

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
HI2::Color HI2::Color::DarkestGrey{60,60,60,255};
HI2::Color HI2::Color::DarkGrey{100,100,100,255};
HI2::Color HI2::Color::Grey{150,150,150,255};
HI2::Color HI2::Color::LightGrey{200,200,200,255};
HI2::Color HI2::Color::LightestGrey{220,220,220,255};
HI2::Color HI2::Color::Transparent{255,255,255,0};

SDL_Window* window;
SDL_Renderer* renderer;

HI2::Color _bg;

std::ofstream _log;

void HI2::logWrite(std::string s){
	_log << s << std::endl;
}

point2D mousePosition;
point2D joystickPosition;

std::stack<SDL_Texture*> textTextures;

int w, h;

// System
void HI2::systemInit(){
	socketInitializeDefault();
	_log.open("/HI2.log");
	_bg = Color(255, 0, 0, 255);
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO) < 0) {
		_log << SDL_GetError()<<std::endl;
	}
	TTF_Init();
	IMG_Init(IMG_INIT_PNG | IMG_INIT_WEBP);
	// create an SDL window (OpenGL ES2 always enabled)
	// when SDL_FULLSCREEN flag is not set, viewport is automatically handled by SDL (use SDL_SetWindowSize to "change resolution")
	// available switch SDL2 video modes :
	// 1920 x 1080 @ 32 bpp (SDL_PIXELFORMAT_RGBA8888)
	// 1280 x 720 @ 32 bpp (SDL_PIXELFORMAT_RGBA8888)
	window = SDL_CreateWindow("sdl2_gles2", 0, 0, 1280,720, 0);
	if (!window) {
		_log << SDL_GetError()<<std::endl;
	}
	w = 1280;
	h = 720;
	// create a renderer (OpenGL ES2)
	renderer = SDL_CreateRenderer(window, 0, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
	if (!renderer) {
		_log << SDL_GetError()<<std::endl;
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
void HI2::systemFini(){
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	Mix_CloseAudio();
	Mix_Quit();
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
	_log.close();
	socketExit();
}

void HI2::startFrame(){
	SDL_SetRenderDrawColor(renderer, _bg.r, _bg.g, _bg.b, _bg.a);
	SDL_RenderClear(renderer);
}

void HI2::toggleFullscreen(){}

void HI2::setBackgroundColor(Color color){_bg=color;}

void HI2::playSound(HI2::Audio &audio,float volume){
	Mix_PlayMusic(rcast<Mix_Music*>(audio._audio),audio._loop?-1:0);
}

void HI2::drawText(Font& font, std::string text, point2D pos, int size, Color c) {
	SDL_Color color = { static_cast<Uint8>(c.r),static_cast<Uint8>(c.g),static_cast<Uint8>(c.b) };
	SDL_Surface* surface = TTF_RenderText_Solid(rcast<TTF_Font*>(font._font), text.c_str(), color);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

	int texW = 0;
	int texH = 0;
	SDL_QueryTexture(texture, nullptr, nullptr, &texW, &texH);
	SDL_Rect dstrect = { pos.x, pos.y, (double)texW / 10.0f * size, (double)texH / 10.0f * size };

	SDL_RenderCopyEx(renderer, texture, nullptr, &dstrect,0,nullptr,SDL_FLIP_NONE);

	textTextures.push(texture);
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
	SDL_QueryTexture(rcast<SDL_Texture*>(texture._texture), nullptr, nullptr, &texture_rect.w, &texture_rect.h);
	texture_rect.w *= scale;
	texture_rect.h *= scale;


	// PI * rad = 180 * deg
	SDL_RenderCopyEx(renderer, rcast<SDL_Texture*>(texture._texture), nullptr, &texture_rect, (radians * 180) / M_PI, nullptr, SDL_FLIP_NONE);

}//TODO

void HI2::drawTextureOverlap(Texture& texture, int posX, int posY, double scale, double radians) {
	SDL_Rect texture_rect;
	texture_rect.x = posX;  //the x coordinate
	texture_rect.y = posY; // the y coordinate
	SDL_QueryTexture(rcast<SDL_Texture*>(texture._texture), nullptr, nullptr, &texture_rect.w, &texture_rect.h);
	texture_rect.w *= scale;
	texture_rect.h *= scale;
	texture_rect.w += 1;
	texture_rect.h += 1;


	// PI * rad = 180 * deg
	SDL_RenderCopyEx(renderer, rcast<SDL_Texture*>(texture._texture), nullptr, &texture_rect, (radians * 180) / M_PI, nullptr, SDL_FLIP_NONE);

}//TODO

void HI2::drawTexturePart(Texture& texture, point2D texturePartStart, int sizeX, int sizeY, point2D displayPos){}//TODO

HI2::Texture HI2::mergeTextures(Texture& originTexture, Texture& destinationTexture,  point2D position){
	return destinationTexture;//STUB
}

void HI2::drawRectangle(point2D pos, int width, int height, Color color){
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
	SDL_Rect r = { pos.x, pos.y, width, height };
	SDL_RenderFillRect(renderer, &r);
}

void HI2::drawEmptyRectangle(point2D pos, int width, int height, Color color){
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
	SDL_Rect r = { pos.x, pos.y, width, height };
	SDL_RenderDrawRect(renderer, &r);
}

void HI2::drawPixel(point2D pos, Color color){
	HI2::drawRectangle(pos,1,1,color);
}

void HI2::endFrame(){
	SDL_RenderPresent(renderer);
	while(!textTextures.empty())
	{
		SDL_DestroyTexture(textTextures.top());
		textTextures.pop();
	}
}

void HI2::setCursorPos(point2D pos)
{
	SDL_WarpMouseInWindow( window,pos.x,pos.y);
}

//~~CLASSES~~

//SOUND
HI2::Audio::Audio(){}
HI2::Audio::Audio(std::filesystem::path path, bool loop, float volume){
	_path=path;
	_loop=loop;
	_volume=volume;
	_audio=Mix_LoadMUS(path.string().c_str());
}
void HI2::Audio::clean(){
	if(_audio!=nullptr){
		Mix_FreeMusic(rcast<Mix_Music *>(_audio));
	}
	_audio=nullptr;
}

//FONT
HI2::Font::Font(){
	_font=nullptr;
	_path=std::filesystem::path();
}

HI2::Font::Font(std::filesystem::path path){
	_path=path;
	_font=TTF_OpenFont(path.c_str(), 10);
	_name=path.filename().replace_extension("");
}
void HI2::Font::clean(){
	if(_font!=nullptr){
		TTF_CloseFont(rcast<TTF_Font*>(_font));
	}
	_font=nullptr;
}

//TEXTURE
HI2::Texture::Texture() {}
HI2::Texture::Texture(std::filesystem::path path) {
	_path = path;
	if (path.extension() == ".bmp") {
		SDL_Surface* temp = SDL_LoadBMP(path.string().c_str());
		_texture = SDL_CreateTextureFromSurface(renderer, temp);
		SDL_FreeSurface(temp);
	}
	else {
		_texture = IMG_LoadTexture(renderer, path.string().c_str());
	}
	if (_texture == nullptr) {
		std::cout << "Error loading texture: " << SDL_GetError() << std::endl;
	}
}

HI2::Texture::Texture(std::vector<std::filesystem::path> paths, double step)
{
	_path = paths[0];
	_sPerFrame = step;
	_currentFrame = 0;
	for (auto path : paths)
	{
		if (path.extension() == ".bmp") {
			SDL_Surface* temp = SDL_LoadBMP(path.string().c_str());
			_texture = SDL_CreateTextureFromSurface(renderer, temp);
			SDL_FreeSurface(temp);
		}
		else {
			_texture = IMG_LoadTexture(renderer, path.string().c_str());
		}
		_animationTextures.push_back(_texture);
	}
	_texture = _animationTextures[0];
}

HI2::Texture::Texture(point2D size)
{
	_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,size.x,size.y);
	SDL_SetTextureBlendMode(rcast<SDL_Texture*>(_texture), SDL_BLENDMODE_BLEND);
}

void HI2::Texture::step(double s)
{
	if (!_animationTextures.empty()) {
		_currentS += s;
		while (_currentS > _sPerFrame)
		{
			_currentS -= _sPerFrame;
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
std::filesystem::path HI2::getDataPath(){
	return std::filesystem::path("data");
}

std::filesystem::path HI2::getSavesPath(){
	return std::filesystem::path("saves");
}

// HardwareInfo
int HI2::getScreenHeight(){
	return ::appletGetOperationMode()==AppletOperationMode_Docked?1080:720;
}
int HI2::getScreenWidth(){
	return ::appletGetOperationMode()==AppletOperationMode_Docked?1920:1280;
}

HI2::PLATFORM HI2::getPlatform(){
	return HI2::PLATFORM_SWITCH;
}


void HI2::consoleInit(){
	::consoleInit(nullptr);
}
void HI2::consoleInit(std::filesystem::path path){
	::consoleInit(nullptr);
}
void HI2::consoleFini(){
	::consoleExit(nullptr);
}
void HI2::consoleClear(){
	::consoleClear();
}
void HI2::sleepThread(unsigned long ns){
	::svcSleepThread(ns);
}

std::bitset<HI2::BUTTON_SIZE> Down = 0;
std::bitset<HI2::BUTTON_SIZE> Held = 0;
std::bitset<HI2::BUTTON_SIZE> Up = 0;

bool HI2::aptMainLoop(){
	::hidScanInput();
	//READ BUTTONS
	consoleUpdate(nullptr);
	return ::appletMainLoop();
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

point2D HI2::getJoystickPos(HI2::JOYSTICK joystick){
	return joystickPosition;
}

point2D HI2::getTouchPos() {
	return mousePosition;
}

void HI2::setRenderTarget(HI2::Texture* t, bool clear){
	if(t == nullptr){
		SDL_SetRenderTarget(renderer, nullptr);
		SDL_SetRenderDrawColor(renderer,_bg.r,_bg.g,_bg.b,_bg.a);
	}
	else{
		SDL_SetRenderTarget(renderer,rcast<SDL_Texture*>(t->_texture));
		SDL_SetRenderDrawColor(renderer,0,0,0,0);
	}

	if(clear){
		SDL_RenderClear(renderer);
	}
}

#endif
