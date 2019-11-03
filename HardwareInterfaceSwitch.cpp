#ifdef __SWITCH__
#define _USE_MATH_DEFINES
#include "HardwareInterface.hpp"
#include <switch.h>
#include <filesystem>
#include <iostream>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <fstream>
#define rcast reinterpret_cast
#define DEBUG_PRIORITY 0

SDL_Window *window;
SDL_Renderer *renderer;

HI2::Color _bg;

std::ofstream _log;

void HI2::logWrite(std::string s){
	_log << s << std::endl;
}

int w, h;

// System
void HI2::systemInit(){
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
	renderer = SDL_CreateRenderer(window, 0, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
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
}

void HI2::startFrame(){
	SDL_SetRenderDrawColor(renderer, _bg.r, _bg.g, _bg.b, _bg.a);
	SDL_RenderClear(renderer);
}
void HI2::setBackgroundColor(Color color){_bg=color;}

void HI2::playSound(HI2::Audio &audio,float volume){
	Mix_PlayMusic(rcast<Mix_Music*>(audio._audio),audio._loop?-1:0);
}

void HI2::drawText(Font& font, std::string text, point2D pos, int size, Color c){
	SDL_Color color = { static_cast<Uint8>(c.r),static_cast<Uint8>(c.g),static_cast<Uint8>(c.b) };
	SDL_Surface* surface = TTF_RenderText_Solid(rcast<TTF_Font*>(font._font), text.c_str(), color);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

	SDL_RenderCopy(renderer, texture, nullptr, nullptr);

	SDL_DestroyTexture(texture);
	SDL_FreeSurface(surface);

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
	SDL_RenderCopyEx(renderer, rcast<SDL_Texture*>(texture._texture), NULL, &texture_rect, (radians * 180) / 3.1415926535f, nullptr, SDL_FLIP_NONE);

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
}


//~~CLASSES~~

//SOUND
HI2::Audio::Audio(){}
HI2::Audio::Audio(std::filesystem::path path, bool loop, float volume){
	_path=path;
	_loop=loop;
	_volume=volume;
	_audio=Mix_LoadMUS(path.c_str());
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

HI2::Font::Font(std::filesystem::path path,int size){
	_path=path;
	_font=TTF_OpenFont(path.c_str(),size);
	_name=path.filename().replace_extension("");
}
void HI2::Font::clean(){
	if(_font!=nullptr){
		TTF_CloseFont(rcast<TTF_Font*>(_font));
	}
	_font=nullptr;
}

//TEXTURE
HI2::Texture::Texture(){}
HI2::Texture::Texture(std::filesystem::path path){
	_path=path;
	_log << "Loading tex:"<<path.c_str()<<std::endl;
	if(path.extension()==".bmp"){
		_log << "BMP"<<std::endl;
		SDL_Surface* temp = SDL_LoadBMP(path.c_str());
		_texture=SDL_CreateTextureFromSurface(renderer,temp);
		SDL_FreeSurface(temp);
	}
	else{
		_log << "Non-BMP"<<std::endl;
		_texture=IMG_LoadTexture(renderer,path.c_str());
	}
	if(_texture==nullptr){
			_log << "Error loading texture: "<<SDL_GetError()<<std::endl;
	}
}
void HI2::Texture::clean(){
	if(_texture!=nullptr){
		SDL_DestroyTexture(rcast<SDL_Texture*>(_texture));
	}
	_texture=nullptr;
}
// filesystem
std::filesystem::path HI2::getDataPath(){
	return std::filesystem::path("data/");
}

std::filesystem::path HI2::getSavesPath(){
	return std::filesystem::path("saves/");
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
bool HI2::aptMainLoop(){
	::hidScanInput();
	return ::appletMainLoop();
	consoleUpdate(nullptr);
}

unsigned long HI2::getKeysDown(){
	return ::hidKeysDown(CONTROLLER_P1_AUTO);
}
unsigned long HI2::getKeysUp(){
	return ::hidKeysUp(CONTROLLER_P1_AUTO);
}
unsigned long HI2::getKeysHeld(){
	return ::hidKeysHeld(CONTROLLER_P1_AUTO);
}
point2D HI2::getJoystickPos(HI2::JOYSTICK joystick){
	::JoystickPosition temp;
	::hidJoystickRead(&temp,CONTROLLER_P1_AUTO,joystick==HI2::JOY_LEFT?::JOYSTICK_LEFT : (::JOYSTICK_RIGHT));
	point2D res;
	res.x=temp.dx;
	res.y=temp.dy;
	return res;
}

point2D HI2::getTouchPos(){
	touchPosition temp;
	point2D res;
	if(hidTouchCount()>0){
		hidTouchRead(&temp,0);
		res.x=temp.px;
		res.y=temp.py;
	}
	return res;
}

#endif
