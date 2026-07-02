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
#include <vector>
#include "HI2.hpp"
#include <thread>
#include <filesystem>
#define GLEW_STATIC
#include <GL/glew.h>

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_mouse.h>

#include <fstream>
#include <functional>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"


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
SDL_GLContext context;

HI2::Color _bg;

std::ofstream _log;

bool fullscreen;

int w, h;
int oldW, oldH;
point2D mousePosition;
point2D mouseMotion;
bool mouseIsRelative = true;

point2D getDesktopResolution()
{
	SDL_DisplayMode mode;
	if (SDL_GetCurrentDisplayMode(0, &mode) == 0 && mode.w > 0 && mode.h > 0) {
		return {mode.w, mode.h};
	}

	SDL_Rect bounds;
	if (SDL_GetDisplayBounds(0, &bounds) == 0 && bounds.w > 0 && bounds.h > 0) {
		return {bounds.w, bounds.h};
	}

	return {1280, 720};
}

struct GLTexture {
	GLuint texture = 0;
	GLuint fbo = 0;
	int w = 0;
	int h = 0;
	bool renderTarget = false;
	HI2::Color colorMod = HI2::Color::White;
};

GLuint uiProgram = 0;
GLuint uiVao = 0;
GLuint uiVbo = 0;
GLuint whiteTexture = 0;
GLTexture* currentRenderTarget = nullptr;

GLuint compileUiShader(GLenum type, const char* source)
{
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, nullptr);
	glCompileShader(shader);

	GLint status = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		char log[1024];
		glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
		std::cout << "UI shader compile error: " << log << std::endl;
	}
	return shader;
}

void initUiRenderer()
{
	const char* vertexSource = R"(
		#version 330 core
		layout(location = 0) in vec2 a_Position;
		layout(location = 1) in vec2 a_TexCoord;
		out vec2 v_TexCoord;
		uniform vec2 u_SurfaceSize;
		void main()
		{
			vec2 zeroToOne = a_Position / u_SurfaceSize;
			vec2 clip = zeroToOne * 2.0 - 1.0;
			gl_Position = vec4(clip.x, -clip.y, 0.0, 1.0);
			v_TexCoord = a_TexCoord;
		}
	)";

	const char* fragmentSource = R"(
		#version 330 core
		in vec2 v_TexCoord;
		out vec4 color;
		uniform sampler2D u_Texture;
		uniform vec4 u_Color;
		uniform int u_UseTexture;
		void main()
		{
			vec4 tex = u_UseTexture == 1 ? texture(u_Texture, v_TexCoord) : vec4(1.0);
			color = tex * u_Color;
		}
	)";

	GLuint vertexShader = compileUiShader(GL_VERTEX_SHADER, vertexSource);
	GLuint fragmentShader = compileUiShader(GL_FRAGMENT_SHADER, fragmentSource);
	uiProgram = glCreateProgram();
	glAttachShader(uiProgram, vertexShader);
	glAttachShader(uiProgram, fragmentShader);
	glLinkProgram(uiProgram);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	glGenVertexArrays(1, &uiVao);
	glGenBuffers(1, &uiVbo);
	glBindVertexArray(uiVao);
	glBindBuffer(GL_ARRAY_BUFFER, uiVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24, nullptr, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (const void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (const void*)(sizeof(float) * 2));
	glBindVertexArray(0);

	const unsigned char white[] = {255, 255, 255, 255};
	glGenTextures(1, &whiteTexture);
	glBindTexture(GL_TEXTURE_2D, whiteTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
}

void finiUiRenderer()
{
	if (whiteTexture) glDeleteTextures(1, &whiteTexture);
	if (uiVbo) glDeleteBuffers(1, &uiVbo);
	if (uiVao) glDeleteVertexArrays(1, &uiVao);
	if (uiProgram) glDeleteProgram(uiProgram);
}

struct HI2PCTextureAccess {
	static GLTexture* get(const HI2::Texture& texture)
	{
		if (texture._texture == nullptr) {
			return nullptr;
		}
		return rcast<GLTexture*>(texture._texture.get()->get());
	}
	static void set(HI2::Texture& texture, GLTexture* value, bool owning)
	{
		if (owning) {
			texture._texture = std::make_shared<HI2::Texture::_internalTextureRAIIWrapper>(value);
		}
		else {
			texture._texture = std::make_shared<HI2::Texture::_internalWeakTextureRAIIWrapper>(value);
		}
	}
};

GLTexture* glTexture(const HI2::Texture& texture)
{
	return HI2PCTextureAccess::get(texture);
}

void setUiState()
{
	glUseProgram(uiProgram);
	glBindVertexArray(uiVao);
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(uiProgram, "u_Texture"), 0);
	glUniform2f(glGetUniformLocation(uiProgram, "u_SurfaceSize"),
		currentRenderTarget ? (float)currentRenderTarget->w : (float)w,
		currentRenderTarget ? (float)currentRenderTarget->h : (float)h);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void drawQuad(GLuint texture, float x, float y, float width, float height, float u0, float v0, float u1, float v1, HI2::Color color, double radians = 0.0)
{
	if (width <= 0 || height <= 0) {
		return;
	}

	const float centerX = x + width * 0.5f;
	const float centerY = y + height * 0.5f;
	const float c = std::cos(radians);
	const float s = std::sin(radians);
	auto rotateX = [&](float px, float py) {
		const float dx = px - centerX;
		const float dy = py - centerY;
		return centerX + dx * c - dy * s;
	};
	auto rotateY = [&](float px, float py) {
		const float dx = px - centerX;
		const float dy = py - centerY;
		return centerY + dx * s + dy * c;
	};

	const float x0 = x;
	const float y0 = y;
	const float x1 = x + width;
	const float y1 = y + height;
	const float vertices[] = {
		rotateX(x0, y0), rotateY(x0, y0), u0, v0,
		rotateX(x1, y0), rotateY(x1, y0), u1, v0,
		rotateX(x1, y1), rotateY(x1, y1), u1, v1,
		rotateX(x0, y0), rotateY(x0, y0), u0, v0,
		rotateX(x1, y1), rotateY(x1, y1), u1, v1,
		rotateX(x0, y1), rotateY(x0, y1), u0, v1,
	};

	setUiState();
	glBindTexture(GL_TEXTURE_2D, texture);
	glUniform4f(glGetUniformLocation(uiProgram, "u_Color"),
		(double)color.r / 255.0, (double)color.g / 255.0, (double)color.b / 255.0, (double)color.a / 255.0);
	glUniform1i(glGetUniformLocation(uiProgram, "u_UseTexture"), texture == whiteTexture ? 0 : 1);
	glBindBuffer(GL_ARRAY_BUFFER, uiVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

GLuint textureFromSurface(SDL_Surface* surface, int& width, int& height)
{
	SDL_Surface* converted = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
	if (converted == nullptr) {
		return 0;
	}

	width = converted->w;
	height = converted->h;
	GLuint texture = 0;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, converted->pitch / converted->format->BytesPerPixel);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, converted->w, converted->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, converted->pixels);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	SDL_FreeSurface(converted);
	return texture;
}

void HI2::logWrite(std::string s) {
	_log << s << std::endl;
}

void HI2::setMouseRelative(bool rel){
	SDL_SetRelativeMouseMode(rel ? SDL_TRUE:SDL_FALSE);
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

	const point2D desktopResolution = getDesktopResolution();
	w = desktopResolution.x;
	h = desktopResolution.y;
	oldW = w;
	oldH = h;
	fullscreen = true;
	window = SDL_CreateWindow("sdl2_gles2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN | SDL_WINDOW_MAXIMIZED);
	if (!window) {
		SDL_Log("SDL_CreateWindow: %s\n", SDL_GetError());
		//SDL_Quit();
	}
	else {
		SDL_GetWindowSize(window, &w, &h);
		oldW = w;
		oldH = h;
	}
	//SDL_GL_SetSwapInterval(1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
	SDL_GL_SetSwapInterval(0);
	context = SDL_GL_CreateContext(window);

	SDL_GL_SetSwapInterval(0);

	//auto status = glewInit();
	if(glewInit() != GLEW_OK)
		throw("glew not ok");
	std::cout << glGetString(GL_VERSION)<<std::endl;
	initUiRenderer();

	// Enable Unicode text input events (SDL_TEXTINPUT) so free-text UI such as the
	// in-game computer editor gets layout-correct characters. Mouse-look and gameplay
	// keybinds still come through the raw key events unaffected.
	SDL_StartTextInput();

	// create a renderer (OpenGL ES2)
	//SDL_SetHintWithPriority(SDL_HINT_RENDER_BATCHING,"1",SDL_HINT_OVERRIDE);
	////renderer = SDL_CreateRenderer(window, 0, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE | SDL_RENDERER_PRESENTVSYNC);
	////if (!renderer) {
	////	SDL_Log("SDL_CreateRenderer: %s\n", SDL_GetError());
	////	//SDL_Quit();
	////}
	////SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

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

	 IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    // window is the SDL_Window*
    // contex is the SDL_GLContext
    ImGui_ImplSDL2_InitForOpenGL(window, context);
    ImGui_ImplOpenGL3_Init();
}
void HI2::systemFini() {
	ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

	finiUiRenderer();
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	Mix_CloseAudio();
	Mix_Quit();
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
	_log.close();
}

void HI2::startFrame() {
	//SDL_SetRenderDrawColor(renderer, _bg.r, _bg.g, _bg.b, _bg.a);
	//SDL_RenderClear(renderer);
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(window);
	ImGui::NewFrame();
	currentRenderTarget = nullptr;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, w, h);
	glClearColor((double)_bg.r/255, (double)_bg.g/255, (double)_bg.b/255, (double)_bg.a/255);
	glClear(GL_COLOR_BUFFER_BIT);
	glClear(GL_DEPTH_BUFFER_BIT);
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
	SDL_Color color = { static_cast<Uint8>(c.r),static_cast<Uint8>(c.g),static_cast<Uint8>(c.b),static_cast<Uint8>(c.a) };
	SDL_Surface* surface = TTF_RenderText_Blended(rcast<TTF_Font*>(font._font), text.c_str(), color);
	if (surface == nullptr) {
		return;
	}

	int texW = 0, texH = 0;
	GLuint texture = textureFromSurface(surface, texW, texH);
	SDL_FreeSurface(surface);
	if (texture == 0) {
		return;
	}

	drawQuad(texture, pos.x, pos.y, (double)texW / 10.0f * size, (double)texH / 10.0f * size, 0, 0, 1, 1, HI2::Color::White);
	glDeleteTextures(1, &texture);
}

void HI2::setTextureColorMod(Texture& texture, Color color)
{
	GLTexture* glTex = glTexture(texture);
	if (glTex != nullptr) {
		glTex->colorMod = HI2::Color(color.r, color.g, color.b, 255);
	}
}

void HI2::drawTexture(const Texture& texture, int posX, int posY, double scale, double radians, HI2::FLIP flip) {
	GLTexture* glTex = glTexture(texture);
	if (glTex == nullptr) return;
	drawTexture(texture, posX, posY, {glTex->w, glTex->h}, {0, 0}, scale, radians, flip);
}
void HI2::drawTexture(const Texture& texture, int posX, int posY, point2D size, point2D startPos, double scale, double radians, HI2::FLIP flip) {
	GLTexture* glTex = glTexture(texture);
	if (glTex == nullptr) return;

	float u0 = (float)startPos.x / (float)glTex->w;
	float u1 = (float)(startPos.x + size.x) / (float)glTex->w;
	float v0 = (float)startPos.y / (float)glTex->h;
	float v1 = (float)(startPos.y + size.y) / (float)glTex->h;
	if (glTex->renderTarget) {
		v0 = 1.0f - (float)startPos.y / (float)glTex->h;
		v1 = 1.0f - (float)(startPos.y + size.y) / (float)glTex->h;
	}
	if (flip == HI2::FLIP::H) {
		std::swap(u0, u1);
	}
	else if (flip == HI2::FLIP::V) {
		std::swap(v0, v1);
	}

	drawQuad(glTex->texture, posX, posY, size.x * scale, size.y * scale, u0, v0, u1, v1, glTex->colorMod, radians);
}

void HI2::drawTextureOverlap(const Texture& texture, int posX, int posY, double scale, double radians, HI2::FLIP flip) {
	GLTexture* glTex = glTexture(texture);
	if (glTex == nullptr) return;
	drawTextureOverlap(texture, posX, posY, {glTex->w, glTex->h}, {0, 0}, scale, radians, flip);
}

void HI2::drawTextureOverlap(const Texture& texture, int posX, int posY, point2D size, point2D startPos, double scale, double radians, HI2::FLIP flip) {
	drawTexture(texture, posX, posY, size, startPos, scale, radians, flip);
}

void HI2::drawTextureF(const Texture& texture, float posX, float posY, double scale, double radians, HI2::FLIP flip) {
	drawTexture(texture, (int)posX, (int)posY, scale, radians, flip);
}

void HI2::drawTextureF(const Texture& texture, float posX, float posY, point2D size, point2D startPos, double scale, double radians, HI2::FLIP flip) {
	drawTexture(texture, (int)posX, (int)posY, size, startPos, scale, radians, flip);
}

HI2::Texture HI2::mergeTextures(Texture& originTexture, Texture& destinationTexture, point2D position) {
	return destinationTexture;//STUB
}

void HI2::drawRectangle(point2D pos, int width, int height, Color color) {
	drawQuad(whiteTexture, pos.x, pos.y, width, height, 0, 0, 1, 1, color);
}
void HI2::drawEmptyRectangle(point2D pos, int width, int height, Color color) {
	drawLine(pos, {pos.x + width, pos.y}, color);
	drawLine({pos.x + width, pos.y}, {pos.x + width, pos.y + height}, color);
	drawLine({pos.x + width, pos.y + height}, {pos.x, pos.y + height}, color);
	drawLine({pos.x, pos.y + height}, pos, color);
}
void HI2::drawEmptyRectangle(point2D pos, int width, int height, int strokewidth, Color color) {
	if(strokewidth == 1){
		drawEmptyRectangle(pos,width,height,color);
	}
	else{
		drawEmptyRectangle(pos,width,height,color);
		drawEmptyRectangle({pos.x+1,pos.y+1},width-2,height-2,strokewidth-1,color);
	}
}
void HI2::drawLine(point2D start, point2D end, Color color){
	const float vertices[] = {
		(float)start.x, (float)start.y, 0.0f, 0.0f,
		(float)end.x, (float)end.y, 0.0f, 0.0f,
	};
	setUiState();
	glBindTexture(GL_TEXTURE_2D, whiteTexture);
	glUniform4f(glGetUniformLocation(uiProgram, "u_Color"),
		(double)color.r / 255.0, (double)color.g / 255.0, (double)color.b / 255.0, (double)color.a / 255.0);
	glUniform1i(glGetUniformLocation(uiProgram, "u_UseTexture"), 0);
	glBindBuffer(GL_ARRAY_BUFFER, uiVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
	glDrawArrays(GL_LINES, 0, 2);
}
void HI2::drawLines(const std::vector<point2D>& points, Color color){
	if (points.empty()) {
		return;
	}
	std::vector<float> vertices;
	vertices.reserve(points.size() * 4);
	for (const point2D& point : points) {
		vertices.push_back(point.x);
		vertices.push_back(point.y);
		vertices.push_back(0.0f);
		vertices.push_back(0.0f);
	}
	setUiState();
	glBindTexture(GL_TEXTURE_2D, whiteTexture);
	glUniform4f(glGetUniformLocation(uiProgram, "u_Color"),
		(double)color.r / 255.0, (double)color.g / 255.0, (double)color.b / 255.0, (double)color.a / 255.0);
	glUniform1i(glGetUniformLocation(uiProgram, "u_UseTexture"), 0);
	glBindBuffer(GL_ARRAY_BUFFER, uiVbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
	glDrawArrays(GL_LINE_STRIP, 0, points.size());
}
void HI2::drawPixel(point2D pos, Color color) {
	HI2::drawRectangle(pos, 1, 1, color);
}

void HI2::endFrame() {
    //glClearColor((double)_bg.r/255, (double)_bg.g/255, (double)_bg.b/255, (double)_bg.a/255);
    //glClear(GL_COLOR_BUFFER_BIT);
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	SDL_GL_SwapWindow(window);

	//SDL_RenderPresent(renderer);

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
	SDL_Surface* surface = nullptr;
	if (path.extension() == ".bmp") {
		surface = SDL_LoadBMP(path.string().c_str());
	}
	else {
		surface = IMG_Load(path.string().c_str());
	}
	if (surface == nullptr) {
		std::cout << "Error loading texture: " << SDL_GetError() << std::endl;
		return;
	}

	GLTexture* glTex = new GLTexture();
	glTex->texture = textureFromSurface(surface, glTex->w, glTex->h);
	SDL_FreeSurface(surface);
	HI2PCTextureAccess::set(*this, glTex, true);
}

HI2::Texture::Texture(point2D size)
{
	GLTexture* glTex = new GLTexture();
	glTex->w = size.x;
	glTex->h = size.y;
	glTex->renderTarget = true;
	glGenTextures(1, &glTex->texture);
	glBindTexture(GL_TEXTURE_2D, glTex->texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	glGenFramebuffers(1, &glTex->fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, glTex->fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, glTex->texture, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, currentRenderTarget ? currentRenderTarget->fbo : 0);
	HI2PCTextureAccess::set(*this, glTex, true);
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
	case SDLK_GREATER:
		return HI2::BUTTON::KEY_PEIXMARTI_RIGHT;
	case SDLK_LESS:
		return HI2::BUTTON::KEY_PEIXMARTI_LEFT;
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
std::string TextInput; // UTF-8 text typed this frame (SDL_TEXTINPUT)

bool HI2::aptMainLoop() {
	SDL_Event event;
	Down.reset();
	Up.reset();
	TextInput.clear();
	while (SDL_PollEvent(&event))
	{
		if((SDL_GetWindowFlags(window) & SDL_WINDOW_INPUT_FOCUS) == 0){
			continue;
		}
		if(SDL_GetRelativeMouseMode() == SDL_FALSE){
			ImGui_ImplSDL2_ProcessEvent(&event);
		}

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
			mouseMotion.x += event.motion.xrel;
			mouseMotion.y += event.motion.yrel;
			break;
		case SDL_MOUSEWHEEL:
			Down[event.wheel.y > 0 ? HI2::BUTTON::KEY_MOUSEWHEEL_UP : HI2::BUTTON::KEY_MOUSEWHEEL_DOWN] = true;
			break;
		case SDL_TEXTINPUT:
			// Layout/IME-resolved characters (letters, digits, punctuation, symbols).
			TextInput += event.text.text;
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
const std::string& HI2::getTextInput() {
	return TextInput;
}
point2D HI2::getJoystickPos(HI2::JOYSTICK joystick) {
	point2D res;
	return res;
}

point2D HI2::getTouchPos() {
	return mousePosition;
}

point2D HI2::getRelativeMouseMovement() {
	point2D movement = SDL_GetRelativeMouseMode() ? mouseMotion : point2D(0, 0);
	mouseMotion = point2D(0,0);
	return movement;
}

void HI2::setRenderTarget(HI2::Texture* t, bool clear) {
	if (t == nullptr) {
		currentRenderTarget = nullptr;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, w, h);
		glClearColor((double)_bg.r / 255.0, (double)_bg.g / 255.0, (double)_bg.b / 255.0, (double)_bg.a / 255.0);
	}
	else {
		currentRenderTarget = glTexture(*t);
		glBindFramebuffer(GL_FRAMEBUFFER, currentRenderTarget ? currentRenderTarget->fbo : 0);
		glViewport(0, 0, currentRenderTarget ? currentRenderTarget->w : w, currentRenderTarget ? currentRenderTarget->h : h);
		glClearColor(0, 0, 0, 0);
	}

	if (clear) {
		glClear(GL_COLOR_BUFFER_BIT);
	}
}

HI2::Texture HI2::getRenderTarget(){
	Texture result;
	HI2PCTextureAccess::set(result, currentRenderTarget, false);
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
	GLTexture* glTex = glTexture(texture);
	if (glTex != nullptr) {
		result.x = glTex->w;
		result.y = glTex->h;
	}
	return result;
}

HI2::Texture::_internalWeakTextureRAIIWrapper::_internalWeakTextureRAIIWrapper(void *pointer)
{
	_texture=pointer;
}

HI2::Texture::_internalTextureRAIIWrapper::~_internalTextureRAIIWrapper()
{
	GLTexture* glTex = rcast<GLTexture*>(_texture);
	if (glTex != nullptr) {
		if (glTex->fbo) {
			glDeleteFramebuffers(1, &glTex->fbo);
		}
		if (glTex->texture) {
			glDeleteTextures(1, &glTex->texture);
		}
		delete glTex;
	}
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
