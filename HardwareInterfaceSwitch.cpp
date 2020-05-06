#ifdef __SWITCH__
#define _USE_MATH_DEFINES
#include "HI2.hpp"
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
HI2::Color HI2::Color::Brown{ 111,92,66,255 };

SDL_Window* window;
SDL_Renderer* renderer;

HI2::Color _bg;

std::ofstream _log;

void HI2::logWrite(std::string s){
	_log << s << std::endl;
}

point2D mousePosition;
point2D joystickPosition;

std::array<std::stack<SDL_Texture*>,3> textTextures;
int textureStackIndex=0;

int w, h;

// System
void HI2::systemInit(){
	romfsInit();
	_log.open("/HI2.log");
	_bg = Color(255, 0, 0, 255);
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
		_log << SDL_GetError()<<std::endl;
		SDL_Quit();
	}
	TTF_Init();
	IMG_Init(IMG_INIT_PNG);

	w = 1280;
	h = 720;
	window = SDL_CreateWindow("lmao", 0, 0, w,h, 0);
	if (!window) {
		_log << SDL_GetError()<<std::endl;
		SDL_Quit();
	}

	std::cout << SDL_GetError() << std::endl;
	// create a renderer (OpenGL ES2)
	renderer = SDL_CreateRenderer(window, 0, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!renderer) {
		_log << SDL_GetError()<<std::endl;
		SDL_Quit();
	}

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	Mix_Init(MIX_INIT_MP3 | MIX_INIT_OGG);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 4096);

	//AccountUid uid;
    //u64 appId = 0x0100F33DC0DEBABE;
    //accountInitialize(AccountServiceType_Application);
    //accountGetPreselectedUser(&uid);
    //accountExit();
    //fsdevMountSaveData("save", appId, uid);
}
void HI2::systemFini(){
	//fsdevCommitDevice("save");
	//fsdevUnmountDevice("save");
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	Mix_CloseAudio();
	Mix_Quit();
	IMG_Quit();
	TTF_Quit();
	SDL_Quit();
	_log.close();
	romfsExit();
}

void HI2::startFrame(){
	SDL_SetRenderDrawColor(renderer, _bg.r, _bg.g, _bg.b, _bg.a);
	SDL_RenderClear(renderer);

	SDL_SetRenderDrawColor(renderer, _bg.r, _bg.g, _bg.b, _bg.a);
	SDL_Rect f = {0, 0, getScreenWidth(), getScreenHeight()};
	SDL_RenderFillRect(renderer, &f);
}

void HI2::toggleFullscreen(){}

void HI2::setBackgroundColor(Color color){_bg=color;}

void HI2::playSound(HI2::Audio &audio,float volume){
	Mix_PlayMusic(rcast<Mix_Music*>(audio._audio),audio._loop?-1:0);
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
	SDL_WarpMouseInWindow( window,pos.x,pos.y);
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
std::filesystem::path HI2::getDataPath(){
	return std::filesystem::path("romfs:/");
}

std::filesystem::path HI2::getSavesPath(){
	//return std::filesystem::path("save:/");
	return std::filesystem::path("saves/");
}

// HardwareInfo
int HI2::getScreenHeight(){
	return ::appletGetOperationMode()==AppletOperationMode_Docked?1080:720;
}
int HI2::getScreenWidth(){
	return ::appletGetOperationMode()==AppletOperationMode_Docked?1920:1280;
}

constexpr HI2::PLATFORM HI2::getPlatform(){
	return HI2::PLATFORM::PLATFORM_SWITCH;
}


void HI2::consoleInit(){
}
void HI2::consoleInit(std::filesystem::path path){
}
void HI2::consoleFini(){
}
void HI2::consoleClear(){
}
void HI2::sleepThread(unsigned long ns){
	::svcSleepThread(ns);
}

std::bitset<HI2::BUTTON_SIZE> Down = 0;
std::bitset<HI2::BUTTON_SIZE> Held = 0;
std::bitset<HI2::BUTTON_SIZE> Up = 0;

void translateButtons(std::bitset<HI2::BUTTON_SIZE>& b, const u64& h){
	for(int i =0;i<64;i++){
		switch(BIT(i)){

		case(::KEY_A):{
			b[HI2::BUTTON::BUTTON_A] = h&::KEY_A;
			break;
		}
		case(::KEY_B):{
			b[HI2::BUTTON::BUTTON_B] = h&::KEY_B;
			break;
		}
		case(::KEY_X):{
			b[HI2::BUTTON::BUTTON_X] = h&::KEY_X;
			break;
		}
		case(::KEY_Y):{
			b[HI2::BUTTON::BUTTON_Y] = h&::KEY_Y;
			break;
		}
		case(::KEY_LSTICK):{
			b[HI2::BUTTON::BUTTON_LSTICK] = h&::KEY_LSTICK;
			break;
		}
		case(::KEY_RSTICK):{
			b[HI2::BUTTON::BUTTON_RSTICK] = h&::KEY_RSTICK;
			break;
		}
		case(::KEY_L):{
			b[HI2::BUTTON::BUTTON_L] = h&::KEY_L;
			break;
		}
		case(::KEY_R):{
			b[HI2::BUTTON::BUTTON_R] = h&::KEY_R;
			break;
		}
		case(::KEY_ZL):{
			b[HI2::BUTTON::BUTTON_ZL] = h&::KEY_ZL;
			break;
		}
		case(::KEY_ZR):{
			b[HI2::BUTTON::BUTTON_ZR] = h&::KEY_ZR;
			break;
		}
		case(::KEY_PLUS):{
			b[HI2::BUTTON::BUTTON_PLUS] = h&::KEY_PLUS;
			break;
		}
		case(::KEY_MINUS):{
			b[HI2::BUTTON::BUTTON_MINUS] = h&::KEY_MINUS;
			break;
		}
		case(::KEY_DLEFT):{
			b[HI2::BUTTON::BUTTON_DLEFT] = h&::KEY_DLEFT;
			break;
		}
		case(::KEY_DUP):{
			b[HI2::BUTTON::BUTTON_DUP] = h&::KEY_DUP;
			break;
		}
		case(::KEY_DRIGHT):{
			b[HI2::BUTTON::BUTTON_DRIGHT] = h&::KEY_DRIGHT;
			break;
		}
		case(::KEY_DDOWN):{
			b[HI2::BUTTON::BUTTON_DDOWN] = h&::KEY_DDOWN;
			break;
		}
		case(::KEY_LSTICK_LEFT):{
			b[HI2::BUTTON::BUTTON_LSTICK_LEFT] = h&::KEY_LSTICK_LEFT;
			break;
		}
		case(::KEY_LSTICK_RIGHT):{
			b[HI2::BUTTON::BUTTON_LSTICK_RIGHT] = h&::KEY_LSTICK_RIGHT;
			break;
		}
		case(::KEY_LSTICK_DOWN):{
			b[HI2::BUTTON::BUTTON_LSTICK_DOWN] = h&::KEY_LSTICK_DOWN;
			break;
		}
		case(::KEY_LSTICK_UP):{
			b[HI2::BUTTON::BUTTON_LSTICK_UP] = h&::KEY_LSTICK_UP;
			break;
		}
		case(::KEY_TOUCH):{
			b[HI2::BUTTON::TOUCHSCREEN] = h&::KEY_TOUCH;
			break;
		}
		default:
			break;
		}
	}
}
bool lastTouch = false;
bool HI2::aptMainLoop(){
	::hidScanInput();
	translateButtons(Down,::hidKeysDown(CONTROLLER_P1_AUTO));
	translateButtons(Held,::hidKeysHeld(CONTROLLER_P1_AUTO));
	translateButtons(Up,::hidKeysUp(CONTROLLER_P1_AUTO));
	Down[HI2::BUTTON::TOUCHSCREEN] = hidTouchCount()>0 && !lastTouch;
	Held[HI2::BUTTON::TOUCHSCREEN] = hidTouchCount()>0;
	Up[HI2::BUTTON::TOUCHSCREEN] = hidTouchCount()==0 && lastTouch;


	calculateAggregators(Down);
	calculateAggregators(Held);
	calculateAggregators(Up);

	if(hidTouchCount()>0){
		touchPosition touch;
		hidTouchRead(&touch,0);
		mousePosition.x=touch.px;
		mousePosition.y=touch.py;
		lastTouch=true;
	}
	else{
		lastTouch=false;
	}


	//consoleUpdate(nullptr);
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

void HI2::createDirectories(std::filesystem::path p){
	for(auto& dir : p){
		std::filesystem::create_directory(dir);
	}
}

void HI2::deleteDirectory(std::filesystem::path p){
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

#endif
