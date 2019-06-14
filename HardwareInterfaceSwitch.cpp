#ifdef __SWITCH__
#include "HardwareInterface.h"
#include <switch.h>
#include <filesystem>
#include <iostream>
#include "HardwareInterface.h"
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

// System
void HI2::systemInit(){
	_log.open("/HI2.log");
	_bg=RGBA8(255,0,0,255);
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO) < 0) {
		_log << SDL_GetError()<<std::endl;
		SDL_Log("SDL_Init: %s\n", SDL_GetError());
	}
	TTF_Init();
	IMG_Init(IMG_INIT_PNG);
	// create an SDL window (OpenGL ES2 always enabled)
	// when SDL_FULLSCREEN flag is not set, viewport is automatically handled by SDL (use SDL_SetWindowSize to "change resolution")
	// available switch SDL2 video modes :
	// 1920 x 1080 @ 32 bpp (SDL_PIXELFORMAT_RGBA8888)
	// 1280 x 720 @ 32 bpp (SDL_PIXELFORMAT_RGBA8888)
	window = SDL_CreateWindow("sdl2_gles2", 0, 0, 1280,720, 0);
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
    romfsInit();
}
void HI2::systemFini(){
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	romfsExit();
	Mix_CloseAudio();
	Mix_Quit();
	TTF_Quit();
	SDL_Quit();
	_log.close();
}

void HI2::startFrame(){
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	SDL_SetRenderDrawColor(renderer, HI2::getR(_bg), HI2::getG(_bg), HI2::getB(_bg), HI2::getA(_bg));
	SDL_Rect f = {0, 0, getScreenWidth(), getScreenHeight()};
	SDL_RenderFillRect(renderer, &f);
}
void HI2::setBackgroundColor(Color color){_bg=color;}

void HI2::playSound(HI2::Audio &audio,float volume){
	Mix_PlayMusic(rcast<Mix_Music*>(audio._audio),audio._loop?-1:0);
}

void HI2::drawText(Font& font, std::string text, point2D pos, int size, Color c){
	SDL_Color color = { static_cast<Uint8>(getR(c)),static_cast<Uint8>(getG(c)),static_cast<Uint8>(getB(c)) };
	SDL_Surface * surface = TTF_RenderText_Solid(rcast<TTF_Font*>(font._font),text.c_str(), color);
	SDL_Texture * texture = SDL_CreateTextureFromSurface(renderer, surface);

	SDL_RenderCopy(renderer, texture, nullptr, nullptr);

	SDL_DestroyTexture(texture);
	SDL_FreeSurface(surface);

}
void HI2::drawTexture(Texture& texture, int posX, int posY){
	SDL_Rect texture_rect;
	texture_rect.x = posX;  //the x coordinate
	texture_rect.y = posY; // the y coordinate
	texture_rect.w = 1000; //the width of the texture
	texture_rect.h = 500; //the height of the texture

	SDL_RenderCopy(renderer, rcast<SDL_Texture*>(texture._texture), NULL, &texture_rect);

}//TODO

void HI2::drawTexturePart(Texture& texture, point2D texturePartStart, int sizeX, int sizeY, point2D displayPos){}//TODO

HI2::Texture HI2::mergeTextures(Texture& originTexture, Texture& destinationTexture,  point2D position){
	return destinationTexture;//STUB
}

void HI2::drawRectangle(point2D pos, int width, int height, Color color){
	SDL_SetRenderDrawColor(renderer, getR(color), getG(color),getB(color),getA(color));
	SDL_Rect r = {pos.x, pos.y, width, height};
	SDL_RenderFillRect(renderer, &r);
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
	std::cout << "Loading tex:"<<path.c_str()<<std::endl;
	if(path.extension()==".bmp"){
		std::cout << "BMP"<<std::endl;
		SDL_Surface* temp = SDL_LoadBMP(path.c_str());
		_texture=SDL_CreateTextureFromSurface(renderer,temp);
		SDL_FreeSurface(temp);
	}
	else{
		std::cout << "Otherwise"<<std::endl;
		_texture=IMG_LoadTexture(renderer,path.c_str());
	}
	if(_texture==nullptr){
			std::cout << "Error al carregar bmp: "<<SDL_GetError()<<std::endl;
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

short HI2::getR(Color color){ return (short)color; }
short HI2::getG(Color color){ return (short)color >> 8; }
short HI2::getB(Color color){ return (short)color >> 16; }
short HI2::getA(Color color){ return (short)color >> 24; }

#endif
