#include "stdafx.h"

#include "ShaderFunctions.h"
#include "vec2.h"

#define ATL_NO_LEAN_AND_MEAN
#include <atlcomcli.h>
#include <atlsync.h>

#include <gdiplus.h>
#undef min
#undef max

#pragma comment(lib, "gdiplus.lib")

namespace {

inline int highbit(unsigned x)
{
	int n = -1;
	while (x) {
		x >>= 1;
		++n;
	}
	return n;
}

inline bool ispow2(unsigned x) { return !(x & (x - 1)); }
inline unsigned nextpow2(unsigned x) { return ispow2(x) ? x : (1 << (highbit(x)+1)); }
inline bool ispow2(vec2i v) { return ispow2(v.x) && ispow2(v.y); }
inline vec2i nextpow2(vec2i v) { return vec2i(nextpow2(v.x), nextpow2(v.y)); }

inline const std::wstring &strtow(const std::wstring &s) { return s; }
inline std::wstring strtow(const std::string &s) {
	const char *ca = s.c_str();
	CA2W w(ca);
	const wchar_t *cw = w;
	return cw;
}

void mapFormat(long srcf,
	long &destf, GLenum &glif, GLenum &glf,
	GLenum &glt, int &Bpp, bool usePacked)
{
	using namespace Gdiplus;

	destf = srcf;
	glt = GL_UNSIGNED_BYTE;
	switch (srcf) {
	// gray:
	case PixelFormat16bppGrayScale:
		glf = GL_LUMINANCE16;
		glif = GL_LUMINANCE;
		Bpp = 2;
		break;

	// rgb:
	case PixelFormat16bppRGB565:
		if (usePacked) {
			glt = GL_UNSIGNED_SHORT_5_6_5_REV;
			glf = GL_RGB;
			glif = GL_RGB;
			Bpp = 2;
			break;
		}
	case PixelFormat16bppRGB555:
		if (usePacked) {
			destf = PixelFormat16bppRGB565;
			glt = GL_UNSIGNED_SHORT_5_6_5_REV;
			glf = GL_RGB;
			glif = GL_RGB;
			Bpp = 2;
			break;
		}
	case PixelFormat24bppRGB:
	case PixelFormat32bppRGB:
	case PixelFormat48bppRGB:
		destf = PixelFormat24bppRGB;
		glf = GL_BGR;
		glif = GL_RGB;
		Bpp = 3;
		break;

	// rgba:
	case PixelFormat16bppARGB1555:
		if (usePacked) {
			glt = GL_UNSIGNED_SHORT_1_5_5_5_REV;
			glf = GL_RGBA;
			glif = GL_RGBA;
			Bpp = 2;
			break;
		}
	case PixelFormat32bppARGB:
	case PixelFormat32bppPARGB:
	case PixelFormat64bppARGB:
	case PixelFormat64bppPARGB:
		destf = PixelFormat32bppARGB;
		glf = GL_BGRA;
		glif = GL_RGBA;
		Bpp = 4;
		break;

	// catch-all:
	case PixelFormat1bppIndexed:
	case PixelFormat4bppIndexed:
	case PixelFormat8bppIndexed:
	default:
		destf = PixelFormat32bppARGB;
		glf = GL_BGRA;
		glif = GL_RGBA;
		Bpp = 4;
		break;
	}
}

inline vec2i size(const Gdiplus::Bitmap &bmp) {
	Gdiplus::Bitmap &bmp_ = const_cast<Gdiplus::Bitmap &>(bmp);
	return vec2i(bmp_.GetWidth(), bmp_.GetHeight());
}

class LockedBits : public Gdiplus::BitmapData {
	LockedBits(const LockedBits &);
	LockedBits &operator=(const LockedBits &);
public:
	const Gdiplus::Bitmap &bitmap;

	LockedBits(const Gdiplus::Bitmap &bmp, const Gdiplus::Rect &r, unsigned format)
		: bitmap(bmp) { lock(r, format); }
	~LockedBits() { unlock(); }

	void lock(const Gdiplus::Rect &r, unsigned format) {
		unlock();

		if (int err = const_cast<Gdiplus::Bitmap &>(bitmap).LockBits(&r,
			Gdiplus::ImageLockModeRead, format, this))
			throw std::runtime_error("could not lock bitmap");
	}
	void unlock() {
		if (Scan0)
			const_cast<Gdiplus::Bitmap &>(bitmap).UnlockBits(this);

		BitmapData bd = { 0 };
		BitmapData::operator=(bd);
	}
};

} // anonymous namespace

GLuint loadTexture(const std::string &filename)
{
	using namespace Gdiplus;

	static bool initialized = false;
	if (!initialized) {
		initialized = true;

		// initialize GDI+
		ULONG_PTR gdiplusToken;
		GdiplusStartupInput gdipsi;
		GdiplusStartupOutput gdipso;
		Status gdis = GdiplusStartup(&gdiplusToken, &gdipsi, &gdipso);
		if (gdis != Ok)
			throw std::runtime_error("could not initialize GDI+");
	}

	// load bitmap
	std::wstring wfile = strtow(filename);
	Bitmap bmp(wfile.c_str(), false);
	std::auto_ptr<Bitmap> potbmp;
	Bitmap *source = &bmp;
	if (bmp.GetLastStatus() != Ok)
		throw std::runtime_error("could not open bitmap " + filename);

	// flip y-axis
	if (bmp.RotateFlip(RotateNoneFlipY) != Ok)
		throw std::runtime_error("could not flip bitmap");

	vec2i npots(bmp.GetWidth(), bmp.GetHeight());
	vec2i pots = nextpow2(npots);
	vec2i csize = npots, tsize = npots;

	// handle NPOT images
	if (npots != pots && GLEW_ARB_texture_non_power_of_two) {
		potbmp.reset(new Bitmap(pots.x, pots.y, bmp.GetPixelFormat()));
		source = &*potbmp;
		Graphics g(source);
		g.DrawImage(&bmp, Gdiplus::Rect(0,0,pots.x,pots.y));
		tsize = pots;
		csize = pots;
	}

	// convert pixel format to gl texture format
	long destf, srcf = source->GetPixelFormat();
	GLenum glif, glf, glt;
	int Bpp;
	mapFormat(srcf, destf, glif, glf, glt, Bpp, true);

	// convert and lock pixels
	Gdiplus::Rect r(0,0, csize.x,csize.y);
	LockedBits bd(*source, r, destf);

	// create and bind texture object
	const GLenum target = GL_TEXTURE_2D;
	GLuint name = 0;
	glGenTextures(1, &name);
	glBindTexture(target, name);

	// set texture parameters
	glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(target, GL_GENERATE_MIPMAP, GL_TRUE);

	// copy image to Texture
	glTexImage2D(target, 0, glif, csize.x, csize.y, 0, glf, glt, bd.Scan0);

	// unlock pixels
	bd.unlock();

	if (GLenum err = glGetError())
		throw std::runtime_error("Could not create texture: "
			+ std::string(reinterpret_cast<const char *>(gluErrorString(err))));

	glBindTexture(target, 0);

	return name;
}
