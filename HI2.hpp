#pragma once
#include <cmath>
#include <filesystem>
#include <vector>
#include <fstream>
#include <string>
#include <cstdlib>
#include <algorithm>
#include "platform/types.hpp"

struct HI2PCTextureAccess;

namespace HI2 {
	// New and more polished version of HI, breaks compat

	enum class FLIP
	{
		NONE = 0,
		H = 1,
		V = 2,
	};
	
	// The colour type moved to platform/types.hpp; this alias keeps HI2 itself compiling
	// until the rest of it goes (plans/UI_PLATFORM_PLAN.md).
	using Color = ::Color;


	class Font {
	public:
		Font();
		Font(std::filesystem::path path);
		void clean();

	private:
		void* _font = nullptr;
		std::string _name;
		std::filesystem::path _path;

		friend void drawText(Font& font, std::string text, point2D pos,
			int size, Color color);
	};

	class Texture {
	public:
		Texture();
		Texture(std::filesystem::path path);
		Texture(point2D size);
		void clean(){}

	private:
		class _internalWeakTextureRAIIWrapper{
			public:
			_internalWeakTextureRAIIWrapper(void* pointer);
			virtual ~_internalWeakTextureRAIIWrapper();
			void* get() const;
			protected:
			void* _texture = nullptr;
		};
		class _internalTextureRAIIWrapper : public _internalWeakTextureRAIIWrapper{
		public:
			_internalTextureRAIIWrapper(void* pointer): _internalWeakTextureRAIIWrapper(pointer){};
			virtual ~_internalTextureRAIIWrapper() override;
		};

		std::shared_ptr<_internalWeakTextureRAIIWrapper> _texture;
		std::filesystem::path _path;

		friend void drawTexture(const Texture& texture, int posX, int posY,
			double scale, double rotation, FLIP flip);
		friend void drawTextureOverlap(const Texture& texture, int posX, int posY,
			double scale, double rotation, FLIP flip);
		friend void drawTextureF(const Texture& texture, float posX, float posY,
			double scale, double rotation, FLIP flip);
		friend void drawTexture(const Texture& texture, int posX, int posY, point2D size, point2D startPos,
			double scale, double rotation, FLIP flip);
		friend void drawTextureOverlap(const Texture& texture, int posX, int posY, point2D size, point2D startPos,
			double scale, double rotation, FLIP flip);
		friend void drawTextureF(const Texture& texture, float posX, float posY, point2D size, point2D startPos,
			double scale, double rotation, FLIP flip);
		friend Texture mergeTextures(Texture& originTexture,
			Texture& destinationTexture,
			point2D position);
		friend void setTextureColorMod(Texture& texture, Color color);
		friend void setRenderTarget(Texture* t, bool b);
		friend Texture getRenderTarget();
		friend point2D getTextureSize(Texture& texture);
		friend struct ::HI2PCTextureAccess;
	};


	// 2D drawing: what is left of HI2 while its callers move to ImGui (tools) and
	// RmlUi (player-facing UI); the window, frame, input and audio are the game's
	// own platform layer now (plans/UI_PLATFORM_PLAN.md). initDrawing() needs the
	// GL context, so it runs after platform::init(), and finiDrawing() before
	// platform::shutdown().
	void initDrawing();
	void finiDrawing();

	void drawText(Font& font, std::string text, point2D pos, int size,
		Color color);
	void setTextureColorMod(Texture& texture, Color color);
	void drawTexture(const Texture& texture, int posX, int posY, double scale = 1, double radians = 0, HI2::FLIP flip = HI2::FLIP::NONE);
	void drawTextureOverlap(const Texture& texture, int posX, int posY, double scale = 1, double radians = 0, HI2::FLIP flip = HI2::FLIP::NONE);
	void drawTextureF(const Texture& texture, float posX, float posY, double scale = 1, double radians = 0, HI2::FLIP flip = HI2::FLIP::NONE);
	void drawTexture(const Texture& texture, int posX, int posY, point2D size, point2D startPos, double scale = 1, double radians = 0, HI2::FLIP flip = HI2::FLIP::NONE);
	void drawTextureOverlap(const Texture& texture, int posX, int posY, point2D size, point2D startPos, double scale = 1, double radians = 0, HI2::FLIP flip = HI2::FLIP::NONE);
	void drawTextureF(const Texture& texture, float posX, float posY, point2D size, point2D startPos, double scale = 1, double radians = 0, HI2::FLIP flip = HI2::FLIP::NONE);
	Texture mergeTextures(Texture& originTexture, Texture& destinationTexture,
		point2D position);
	void drawRectangle(point2D pos, int width, int height, Color color); // draw a filled rectangle
	void drawEmptyRectangle(point2D pos, int width, int height, Color color);
	void drawEmptyRectangle(point2D pos, int width, int height, int strokewidth, Color color);
	void drawLine(point2D start, point2D end, Color color);
	void drawLines(const std::vector<point2D>& points, Color color);

	void drawPixel(point2D pos, Color color);
	void setRenderTarget(Texture* t = nullptr, bool clear = false);
	Texture getRenderTarget();

	point2D getTextureSize(Texture& texture);

} // namespace HI2
