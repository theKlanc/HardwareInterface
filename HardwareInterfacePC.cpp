#if defined __LINUX__ || defined WIN32
#define _USE_MATH_DEFINES
#include <chrono>
#include <thread>
#include <math.h>
#include <string>
#include <future>
#include <iostream>
#include <fstream>
#include "HardwareInterface.h"
#include <thread>
#include <filesystem>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

#include <functional>

#define DEBUG_PRIORITY 0

SDL_Window *window;
SDL_Renderer *renderer;

HI2::Color _bg;

// filesystem
std::filesystem::path HI2::getDataPath(){
	return std::filesystem::path();
}

std::filesystem::path HI2::getSavesPath(){
	return std::filesystem::path();
}

// HardwareInfo
int HI2::getScreenHeight(){
	int w,h;
	SDL_GetWindowSize(window,&w,&h);
	return h;
}
int HI2::getScreenWidth(){
	int w,h;
	SDL_GetWindowSize(window,&w,&h);
	return w;
}

HI2::PLATFORM HI2::getPlatform(){
	return HI2::PLATFORM_SWITCH;
}

// System
void HI2::systemInit(){
	_bg=RGBA8(255,0,0,255);
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_Log("SDL_Init: %s\n", SDL_GetError());
	}
	TTF_Init();
	IMG_Init(IMG_INIT_PNG);
	// create an SDL window (OpenGL ES2 always enabled)
	// when SDL_FULLSCREEN flag is not set, viewport is automatically handled by SDL (use SDL_SetWindowSize to "change resolution")
	// available switch SDL2 video modes :
	// 1920 x 1080 @ 32 bpp (SDL_PIXELFORMAT_RGBA8888)
	// 1280 x 720 @ 32 bpp (SDL_PIXELFORMAT_RGBA8888)
	window = SDL_CreateWindow("sdl2_gles2", 0, 0, 1280,720, SDL_WINDOW_RESIZABLE | SDL_WINDOW_SKIP_TASKBAR);
	if (!window) {
		SDL_Log("SDL_CreateWindow: %s\n", SDL_GetError());
		SDL_Quit();
	}

	// create a renderer (OpenGL ES2)
	renderer = SDL_CreateRenderer(window, 0, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!renderer) {
		SDL_Log("SDL_CreateRenderer: %s\n", SDL_GetError());
		SDL_Quit();
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
}
void HI2::systemFini(){
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	TTF_Quit();
	SDL_Quit();
}
void HI2::consoleInit(){}
void HI2::consoleInit(std::filesystem::path path){}
void HI2::consoleFini(){}
void HI2::consoleClear(){}
void HI2::sleepThread(unsigned long ns){
	std::this_thread::sleep_for(std::chrono::nanoseconds(ns));
}

HI2::BUTTON translate(SDL_Keycode s){
	switch(s){
	case SDLK_BACKSPACE:
		return HI2::BUTTON::KEY_PLUS;
	case SDLK_DOWN:
		return HI2::BUTTON::KEY_DOWN;
	case SDLK_UP:
		return HI2::BUTTON::KEY_UP;
	case SDLK_LEFT:
		return HI2::BUTTON::KEY_LEFT;
	case SDLK_RIGHT:
		return HI2::BUTTON::KEY_RIGHT;
	case SDLK_SPACE:
		return HI2::BUTTON::KEY_MINUS;
	default:
		return HI2::BUTTON::KEY_TOUCH;
	}//TODO acabar aixo

}

unsigned long Down;
unsigned long Held;
unsigned long Up;

bool HI2::aptMainLoop(){
	SDL_Event event;
	Down=0;
	Up=0;
	Held=0;
	while(SDL_PollEvent(&event))
	{
		switch(event.type){
		case SDL_QUIT:
		{
			return false;
		}
		case SDL_KEYDOWN:
			Held|= translate(event.key.keysym.sym);
			Down|= event.key.repeat?0:translate(event.key.keysym.sym);
			break;

		case SDL_KEYUP:
			Up|= event.key.state==SDL_PRESSED?0:translate(event.key.keysym.sym);
			break;
		case SDL_WINDOWEVENT:
			if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
				std::cout<<"MESSAGE:Resizing window...\n";
			}
			break;
		}

	}
	return true;
}

unsigned long HI2::getKeysDown(){
	return Down;
}
unsigned long HI2::getKeysUp(){
	return Up;
}
unsigned long HI2::getKeysHeld(){
	return Held;
}
point2D HI2::getJoystickPos(HI2::JOYSTICK joystick){
	//::JoystickPosition temp;
	//::hidJoystickRead(&temp,CONTROLLER_P1_AUTO,joystick==HI2::JOY_LEFT?::JOYSTICK_LEFT : (::JOYSTICK_RIGHT));
	point2D res;
	//res.x=temp.dx;
	//res.y=temp.dy;
	return res;
}

point2D HI2::getTouchPos(){
	//touchPosition temp;
	point2D res;
	//if(hidTouchCount()>0){
	//	hidTouchRead(&temp,0);
	//	res.x=temp.px;
	//	res.y=temp.py;
	//}
	return res;
}

short HI2::getR(Color color){ return (short)color; }
short HI2::getG(Color color){ return (short)color >> 8; }
short HI2::getB(Color color){ return (short)color >> 16; }
short HI2::getA(Color color){ return (short)color >> 24; }


void HI2::startFrame(){
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	SDL_SetRenderDrawColor(renderer, HI2::getR(_bg), HI2::getG(_bg), HI2::getB(_bg), HI2::getA(_bg));
	SDL_Rect f = {0, 0, getScreenWidth(), getScreenHeight()};
	SDL_RenderFillRect(renderer, &f);
}
void HI2::setBackgroundColor(Color color){_bg=color;}

void HI2::drawText(Font& font, std::string text, point2D pos, int size, Color c){
	SDL_Color color = { static_cast<Uint8>(getR(c)),static_cast<Uint8>(getG(c)),static_cast<Uint8>(getB(c)) };
	SDL_Surface * surface = TTF_RenderText_Solid(reinterpret_cast<TTF_Font*>(font.get()),text.c_str(), color);
	SDL_Texture * texture = SDL_CreateTextureFromSurface(renderer, surface);

	SDL_RenderCopy(renderer, texture, nullptr, nullptr);

	SDL_DestroyTexture(texture);
	SDL_FreeSurface(surface);

}
void HI2::drawTexture(Texture &texture, int posX, int posY){
	SDL_Rect texture_rect;
	texture_rect.x = posX;  //the x coordinate
	texture_rect.y = posY; // the y coordinate
	texture_rect.w = 1000; //the width of the texture
	texture_rect.h = 500; //the height of the texture

	SDL_RenderCopy(renderer, reinterpret_cast<SDL_Texture*>(texture.get()), NULL, NULL);
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
HI2::Font::~Font(){
	if(_font!=nullptr){
		TTF_CloseFont(reinterpret_cast<TTF_Font*>(_font));
	}
}
HI2::Font& HI2::Font::operator=(HI2::Font other)
{
	_font = other._font;
	_path=other._path;
	_name=other._name;
	other._font = nullptr;
	return *this;
}
void* HI2::Font::get(){
	return _font;
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
HI2::Texture::~Texture(){
	if(_texture!=nullptr){
		SDL_DestroyTexture(reinterpret_cast<SDL_Texture*>(_texture));
	}
}
HI2::Texture& HI2::Texture::operator=(HI2::Texture other)
{
	_texture = other._texture;
	_path=other._path;
	other._texture = nullptr;
	return *this;
}

void* HI2::Texture::get(){return _texture;}

#endif
