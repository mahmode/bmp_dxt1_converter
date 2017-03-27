/**
bmp_dxt1_headers.h
Purpose: This header contains a list of bmp/dxt1 headers and constants used in the application

@author Mahmoud Badri (mhdside@hotmail.com)
@version 1.2 12/02/2017
*/

#pragma once

typedef unsigned char byte;

// <Windows.h> includes needed the BMP file and info headers tagBITMAPFILEHEADER and tagBITMAPINFOHEADER
// but I'm not using them for the sake of writing all the code by myself

// BMP file header
#pragma pack(2)	// prevent padding short to 4-bytes
struct BMP_HEADER
{
	// file header
	unsigned short signature;
	unsigned int fileSize;
	unsigned int reserved;			  // joining reserve1 and 2 into 8-bytes
	unsigned int dataOffset;

	// info header
	unsigned int   infoSize;           /* Size of info header */
	int			   imageWidth;		  /* Width of image */
	int			   imageHeight;		  /* Height of image */
	unsigned short colorPlanes;       /* Number of color planes */
	unsigned short colorDepth;        /* Number of bits per pixel */
	unsigned int   compression;		  /* Type of compression to use */
	unsigned int   imageSize;		  /* Size of image data */
	int            resX;			  /* X pixels per meter */
	int            resY;			  /* Y pixels per meter */
	unsigned int   ncolours;          /* Number of colors used */
	unsigned int   importantcolours;  /* Number of important colors */
};
#pragma pack()


struct RGBTriplet
{
	byte  b; // Blue
	byte  g; // Green
	byte  r; // Red

	RGBTriplet() :r(0), g(0), b(0) {};
	RGBTriplet(byte r, byte  g, byte  b):r(r), g(g), b(b) {}
};

struct Dxt1Block
{
	unsigned short  c0;        // color 0 (2 bytes)
	unsigned short  c1;        // color 1 (2 bytes)
	byte indices[4];   // indices (4 bytes) (TODO: use int instead of char[4])

	Dxt1Block()
	{
		indices[0] = 0;
		indices[1] = 0;
		indices[2] = 0;
		indices[3] = 0;
	}
};

// DDS header flags
#define DDSD_CAPS		 0x1
#define DDSD_HEIGHT		 0x2
#define DDSD_WIDTH		 0x4
#define DDSD_PITCH		 0x8
#define DDSD_PIXELFORMAT 0x1000
#define DDSD_MIPMAPCOUNT 0x20000
#define DDSD_LINEARSIZE	 0x80000
#define DDSD_DEPTH		 0x800000

// DDS pixel format flags
#define DDPF_ALPHAPIXELS        0x1
#define DDPF_ALPHA              0x2
#define DDPF_FOURCC             0x4 
#define DDPF_RGB                0x40
#define DDPF_YUV                0x200
#define DDPF_LUMINANCE          0x20000
#define DDPF_DXT1				0x31545844

// DDSCAPS flags
#define DDSCAPS_COMPLEX         0x8
#define DDSCAPS_MIPMAP          0x400000
#define DDSCAPS_TEXTURE         0x1000

// DDS file pixel format header
struct DDS_PIXELFORMAT
{
	unsigned int dwSize;
	unsigned int dwFlags;
	unsigned int dwFourCC;
	unsigned int dwRGBBitCount;
	unsigned int dwRBitMask;
	unsigned int dwGBitMask;
	unsigned int dwBBitMask;
	unsigned int dwABitMask;
};

//DDS file header (https://msdn.microsoft.com/en-us/library/windows/desktop/bb943982(v=vs.85).aspx)
struct DDS_HEADER
{
	// DDS magic number
	unsigned int    dwMagic;

	// header
	unsigned int    dwSize;
	unsigned int    dwFlags;
	unsigned int    dwHeight;
	unsigned int    dwWidth;
	unsigned int    dwPitchOrLinearSize;
	unsigned int    dwDepth;
	unsigned int    dwMipMapCount;
	unsigned int    dwReserved1[11];
	DDS_PIXELFORMAT ddspf;
	unsigned int    dwCaps;
	unsigned int    dwCaps2;
	unsigned int    dwCaps3;
	unsigned int    dwCaps4;
	unsigned int    dwReserved2;
};