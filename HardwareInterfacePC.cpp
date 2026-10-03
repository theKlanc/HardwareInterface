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

#include <fstream>
#include <functional>

#include "platform/platform.hpp"


#ifdef __EMSCRIPTEN__
#define SDL_RenderCopyExF SDL_RenderCopyEx
#define SDL_FRect SDL_Rect
#endif
#define DEBUG_PRIORITY 0

#define rcast reinterpret_cast

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
		currentRenderTarget ? (float)currentRenderTarget->w : (float)platform::screenSize().x,
		currentRenderTarget ? (float)currentRenderTarget->h : (float)platform::screenSize().y);
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

// Needs the GL context: called after platform::init(), and finiDrawing() before
// platform::shutdown().
void HI2::initDrawing() {
	TTF_Init();
	IMG_Init(IMG_INIT_PNG);
	initUiRenderer();
}

void HI2::finiDrawing() {
	finiUiRenderer();
	TTF_Quit();
	IMG_Quit();
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

//~~CLASSES~~

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

void HI2::setRenderTarget(HI2::Texture* t, bool clear) {
	if (t == nullptr) {
		currentRenderTarget = nullptr;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		const point2D screen = platform::screenSize();
		const Color background = platform::backgroundColor();
		glViewport(0, 0, screen.x, screen.y);
		glClearColor((double)background.r / 255.0, (double)background.g / 255.0, (double)background.b / 255.0, (double)background.a / 255.0);
	}
	else {
		currentRenderTarget = glTexture(*t);
		glBindFramebuffer(GL_FRAMEBUFFER, currentRenderTarget ? currentRenderTarget->fbo : 0);
		glViewport(0, 0, currentRenderTarget ? currentRenderTarget->w : platform::screenSize().x, currentRenderTarget ? currentRenderTarget->h : platform::screenSize().y);
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

#endif
