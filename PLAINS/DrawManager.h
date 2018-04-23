#pragma once

#include "Hooks.h"

struct VMatrix
{
	float m[4][4];

	inline float* operator[](int i)
	{
		return m[i];
	}

	inline const float* operator[](int i) const
	{
		return m[i];
	}
};

class Fonts
{
public:
	DWORD menu;
	DWORD font_icons;
	DWORD menu_bold;
	DWORD esp;
	DWORD esp_extra;
	DWORD esp_small;
	DWORD esp_drops;
	DWORD esp_icons;
	DWORD indicator;
};

class DrawManager
{
public:
	Fonts fonts;

	bool screen_transform(const Vector& point, Vector& screen)
	{
		float w;
		const VMatrix& worldToScreen = (VMatrix&)m_pEngine->WorldToScreenMatrix();

		screen.x = worldToScreen[0][0] * point[0] + worldToScreen[0][1] * point[1] + worldToScreen[0][2] * point[2] + worldToScreen[0][3];
		screen.y = worldToScreen[1][0] * point[0] + worldToScreen[1][1] * point[1] + worldToScreen[1][2] * point[2] + worldToScreen[1][3];
		w = worldToScreen[3][0] * point[0] + worldToScreen[3][1] * point[1] + worldToScreen[3][2] * point[2] + worldToScreen[3][3];
		screen.z = 0.0f;

		bool behind = false;

		if(w < 0.001f)
		{
			behind = true;
			screen.x *= 100000;
			screen.y *= 100000;
		}
		else
		{
			behind = false;
			float invw = 1.0f / w;
			screen.x *= invw;
			screen.y *= invw;
		}

		return behind;
	}

	void rect(int x, int y, int w, int h, Color color)
	{
		m_pSurface->DrawSetColor(color);
		m_pSurface->DrawFilledRect(x, y, x + w, y + h);
	}

	void outlined_rect(int x, int y, int w, int h, Color color_out, Color color_in)
	{
		m_pSurface->DrawSetColor(color_in);
		m_pSurface->DrawFilledRect(x, y, x + w, y + h);

		m_pSurface->DrawSetColor(color_out);
		m_pSurface->DrawOutlinedRect(x, y, x + w, y + h);
	}

	void outline(int x, int y, int w, int h, Color color)
	{
		m_pSurface->DrawSetColor(color);
		m_pSurface->DrawOutlinedRect(x, y, x + w, y + h);
	}

	void line(int x, int y, int x2, int y2, Color color)
	{
		m_pSurface->DrawSetColor(color);
		m_pSurface->DrawLine(x, y, x2, y2);
	}

	void polyline(int *x, int *y, int count, Color color)
	{
		m_pSurface->DrawSetColor(color);
		m_pSurface->DrawPolyLine(x, y, count);
	}

	void polygon(int count, Vertex_t* Vertexs, Color color)
	{
		static int Texture = m_pSurface->CreateNewTextureID(true);
		unsigned char buffer[4] = { 255, 255, 255, 255 };

		m_pSurface->DrawSetTextureRGBA(Texture, buffer, 1, 1);
		m_pSurface->DrawSetColor(color);
		m_pSurface->DrawSetTexture(Texture);

		m_pSurface->DrawTexturedPolygon(count, Vertexs);
	}

	void polygon_outlined(int count, Vertex_t* Vertexs, Color color, Color colorLine)
	{
		static int x[128];
		static int y[128];

		polygon(count, Vertexs, color);

		for(int i = 0; i < count; i++)
		{
			x[i] = Vertexs[i].m_Position.x;
			y[i] = Vertexs[i].m_Position.y;
		}

		polyline(x, y, count, colorLine);
	}

	void gradient_verticle(int x, int y, int w, int h, Color c1, Color c2)
	{
		rect(x, y, w, h, c1);
		BYTE first = c2.r();
		BYTE second = c2.g();
		BYTE third = c2.b();
		for(int i = 0; i < h; i++)
		{
			float fi = i, fh = h;
			float a = fi / fh;
			DWORD ia = a * 255;
			rect(x, y + i, w, 1, Color(first, second, third, ia));
		}
	}

	void gradient_horizontal(int x, int y, int w, int h, Color c1, Color c2)
	{
		rect(x, y, w, h, c1);
		BYTE first = c2.r();
		BYTE second = c2.g();
		BYTE third = c2.b();
		for(int i = 0; i < w; i++)
		{
			float fi = i, fw = w;
			float a = fi / fw;
			DWORD ia = a * 255;
			rect(x + i, y, 1, h, Color(first, second, third, ia));
		}
	}
	void text(int x, int y, const char* _Input, int font, Color color)
	{
		int apple = 0;
		char Buffer[2048] = { '\0' };
		va_list Args;
		va_start(Args, _Input);
		vsprintf_s(Buffer, _Input, Args);
		va_end(Args);
		size_t Size = strlen(Buffer) + 1;
		wchar_t* WideBuffer = new wchar_t[Size];
		mbstowcs_s(0, WideBuffer, Size, Buffer, Size - 1);

		m_pSurface->DrawSetTextColor(color);
		m_pSurface->DrawSetTextFont(font);
		m_pSurface->DrawSetTextPos(x, y);
		m_pSurface->DrawPrintText(WideBuffer, wcslen(WideBuffer));
	}
	RECT get_text_size(const char* _Input, int font)
	{
		int apple = 0;
		char Buffer[2048] = { '\0' };
		va_list Args;
		va_start(Args, _Input);
		vsprintf_s(Buffer, _Input, Args);
		va_end(Args);
		size_t Size = strlen(Buffer) + 1;
		wchar_t* WideBuffer = new wchar_t[Size];
		mbstowcs_s(0, WideBuffer, Size, Buffer, Size - 1);
		int Width = 0, Height = 0;

		m_pSurface->GetTextSize(font, WideBuffer, Width, Height);

		RECT outcome = { 0, 0, Width, Height };
		return outcome;
	}

	void color_spectrum(int x, int y, int w, int h)
	{
		static int GradientTexture = 0;
		static std::unique_ptr<Color[]> Gradient = nullptr;
		if(!Gradient)
		{
			Gradient = std::make_unique<Color[]>(w * h);

			for(int i = 0; i < w; i++)
			{
				int div = w / 6;
				int phase = i / div;
				float t = (i % div) / (float)div;
				int r, g, b;

				switch(phase)
				{
				case(0):
					r = 255;
					g = 255 * t;
					b = 0;
					break;
				case(1):
					r = 255 * (1.f - t);
					g = 255;
					b = 0;
					break;
				case(2):
					r = 0;
					g = 255;
					b = 255 * t;
					break;
				case(3):
					r = 0;
					g = 255 * (1.f - t);
					b = 255;
					break;
				case(4):
					r = 255 * t;
					g = 0;
					b = 255;
					break;
				case(5):
					r = 255;
					g = 0;
					b = 255 * (1.f - t);
					break;
				}

				for(int k = 0; k < h; k++)
				{
					float sat = k / (float)h;
					int _r = r + sat * (255 - r);
					int _g = g + sat * (255 - g);
					int _b = b + sat * (255 - b);

					*reinterpret_cast<Color*>(Gradient.get() + i + k * w) = Color(_r, _g, _b);
				}
			}

			GradientTexture = m_pSurface->CreateNewTextureID(true);
			m_pSurface->DrawSetTextureRGBA(GradientTexture, (unsigned char*)Gradient.get(), w, h);
		}
		m_pSurface->DrawSetColor(Color(255, 255, 255, 255));
		m_pSurface->DrawSetTexture(GradientTexture);
		m_pSurface->DrawTexturedRect(x, y, x + w, y + h);
	}

	Color color_spectrum_pen(int x, int y, int w, int h, Vector stx)
	{
		int div = w / 6;
		int phase = stx.x / div;
		float t = ((int)stx.x % div) / (float)div;
		int r, g, b;

		switch(phase)
		{
		case(0):
			r = 255;
			g = 255 * t;
			b = 0;
			break;
		case(1):
			r = 255 * (1.f - t);
			g = 255;
			b = 0;
			break;
		case(2):
			r = 0;
			g = 255;
			b = 255 * t;
			break;
		case(3):
			r = 0;
			g = 255 * (1.f - t);
			b = 255;
			break;
		case(4):
			r = 255 * t;
			g = 0;
			b = 255;
			break;
		case(5):
			r = 255;
			g = 0;
			b = 255 * (1.f - t);
			break;
		}

		float sat = stx.y / h;
		return Color(r + sat * (255 - r), g + sat * (255 - g), b + sat * (255 - b), 255);
	}

	void filled_circle(int x, int y, float points, float radius, Color color)
	{
		std::vector<Vertex_t> vertices;
		float step = (float)M_PI * 2.0f / points;

		for(float a = 0; a < (M_PI * 2.0f); a += step)
			vertices.push_back(Vertex_t(Vector2D(radius * cosf(a) + x, radius * sinf(a) + y)));

		polygon((int)points, vertices.data(), color);
	}
};

extern DrawManager draw;