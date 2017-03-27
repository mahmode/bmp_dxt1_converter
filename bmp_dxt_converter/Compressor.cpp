/**
Compressor.cpp
Purpose: Converts from bmp to DXT1 dds and vice versa

@author Mahmoud Badri (mhdside@hotmail.com)
@version 1.2 12/02/2017
*/

#include <iostream>
#include <fstream>
#include <bitset>

#include <algorithm>    // std::max
#include "Compressor.h"


// convert a color to hex helper function
inline int toHex(RGBTriplet c)
{
	return c.r << 16 | c.g << 8 | c.b;
}

Compressor::Compressor(){}
Compressor::~Compressor(){}

void Compressor::compress(const string& filePath)
{
	ifstream bmpFile;
	bmpFile.open(filePath, ios::binary);

	if (!bmpFile.good())
	{
		cout << "- file not found." << endl;
		bmpFile.close();
		return;
	}
	
	// read the BMP file header (including info header)
	BMP_HEADER bmpHeader;
	bmpFile.seekg(0, ios::beg);
	bmpFile.read((char*)&bmpHeader, sizeof(bmpHeader));

	// make sure the BMP file is valid, uncompressed, 24bit, divisible by 4
	if (!isValidBMPFile(bmpHeader))
	{
		bmpFile.close();
		return;
	}
	
	int imgWidth = bmpHeader.imageWidth;
	int imgHeight = abs(bmpHeader.imageHeight);
	bool isBottomUp = bmpHeader.imageHeight > 0; // pixels stored from the bottom to top
	int nPixels = imgWidth * imgHeight; // number of image pixels
	int nBlocks = nPixels / 16; // number of blocks
	long nPixelBytes = nPixels * 3; // number of pixel bytes

	// print image header data
	//printBMPHeader(bmpHeader);
	//cout << "nPixels: " << nPixels << '\n';
	//cout << "nBlocks: " << nBlocks << '\n';
	
	// read BMP color data to a buffer
	RGBTriplet* bmpBuffer = new RGBTriplet[nPixels];
	bmpFile.seekg(bmpHeader.dataOffset, ios::beg);
	bmpFile.read((char*)bmpBuffer, nPixelBytes);

	// final compressed DXT1 blocks will be saved here
	Dxt1Block* blocks = new Dxt1Block[nBlocks];
	
	// compress the bmpBuffer into the blocks
	compressBMP(bmpBuffer, blocks, imgWidth, imgHeight, isBottomUp);

	// save the resulting DXT1 blocks to file
	saveDDS(blocks, nBlocks, imgWidth, imgHeight);
	
	cout << "- file converted and saved successfully to " << DDS_FILE_NAME << endl;

	// free memory
	delete[] bmpBuffer;
	delete[] blocks;
	bmpFile.close();
}

void Compressor::decompress(const string& filePath)
{
	ifstream ddsFile;
	ddsFile.open(filePath, ios::binary);

	if (!ddsFile.good())
	{
		cout << "- file not found." << endl;
		ddsFile.close();
		return;
	}

	// read DDS file header (including the magic number)
	DDS_HEADER ddsHeader;
	ddsFile.seekg(0, ios::beg);
	ddsFile.read((char*)&ddsHeader, sizeof(ddsHeader));

	// check valid DDS file, DXT1-compressed, divisible by 4
	if (!isValidDDSFile(ddsHeader))
	{
		ddsFile.close();
		return;
	}

	int imgWidth = ddsHeader.dwWidth;
	int imgHeight = ddsHeader.dwHeight;
	int nBlocks = (imgWidth * imgHeight) / 16; // number of blocks

	//printDdsHeader(ddsHeader);
	//cout << "nBlocks: " << nBlocks << endl;

	// read DDS DXT1 blocks to a buffer
	Dxt1Block* blocks = new Dxt1Block[nBlocks];
	ddsFile.seekg(ddsHeader.dwSize + 4, ios::beg); // 4b for the DDS magic number
	ddsFile.read((char*)blocks, nBlocks * 8); // each DXT1 block is 8b

	// final expanded BMP pixels colors will be saved here
	RGBTriplet* outputColors = new RGBTriplet[imgWidth * imgHeight];

	// decompress the DXT1 blocks and saved the generated pixel colors to outputColors
	decompressDDS(blocks, outputColors, nBlocks, imgWidth);

	// save the outputColors to a BMP file
	saveBMP(outputColors, imgWidth, imgHeight);

	// free memory
	delete[] outputColors;
	delete[] blocks;
	ddsFile.close();
}

void Compressor::compressBMP(const RGBTriplet* bmpBuffer, Dxt1Block* blocks, const int imgWidth, const int imgHeight, const bool isBottomUp)
{
	cout << "- converting..." << endl;

	// holds block colors to use to calculate DXT1 compressed colored c0 and c1 and pixel indices
	RGBTriplet blockColors[16];

	// h4/w4: iterates over blocks (h4 vertically, w4 horizontally), a block has 4x4 pixels
	// h/w are iterates over block pixels
	// pixelIdx: pixel index in the bmpBuffer matching a block pixel at a block coordinate: w,h,w4,h4
	// blockIdx: iterates over the blocks
	int h4, w4, h, w, pixelIdx, blockIdx = 0;
	for (h4 = 0; h4 < imgHeight; h4 += 4) // iterate blocks height-direction
	{
		for (w4 = 0; w4 < imgWidth; w4 += 4) // iterate blocks width-direction
		{
			for (h = 0; h < 4; ++h) // iterate block pixels height-direction
			{
				for (w = 0; w < 4; ++w) // // iterate block pixels width-direction
				{
					if (isBottomUp) // pixel data are stored from bottom left to top right
						pixelIdx = w4 + w + (imgHeight - h4 - h - 1) * imgWidth;
					else // TL to BR
						pixelIdx = w4 + w + (h4 + h) * imgWidth;
					
					// get and save the pixel color to the blockColors
					blockColors[w + h * 4] = bmpBuffer[pixelIdx];
				}
			}

			// compress a 4x4 block of 24bit colors (48b) to 8byte DXT1 block
			compressDxt1Block(blockColors, blocks[blockIdx]);

			++blockIdx;
		}
	}
}

void Compressor::decompressDDS(const Dxt1Block* blocks, RGBTriplet* outputColors, const int nBlocks, const int imgWidth)
{
	cout << "- converting..." << endl;

	// nBlocksPerRow: number of blocks in one row of the image
	// pixelIdx: pixel index in the outputColors space
	// cIdx: color index, each of the 16 block pixels will use to map it to one of the 4 block colors c0, c1, c2, c3
	// colors[4]: array of the 4 block colors c0, c1, c2, c3
	int nBlocksPerRow = imgWidth / 4;
	int pixelIdx, cIdx;
	RGBTriplet colors[4];

	for (int i = 0; i < nBlocks; ++i) // loop over blocks
	{
		// expand c0, c1 from RGB565 to RGB888, and calculate c2, c3
		// (RGB565 to RGB888 source: http://forum.arduino.cc/index.php?topic=285303.0#/?)
		colors[0].r = ((((blocks[i].c0 >> 11) & 0x1F) * 527) + 23) >> 6;
		colors[0].g = ((((blocks[i].c0 >> 5) & 0x3F) * 259) + 33) >> 6;
		colors[0].b = (((blocks[i].c0 & 0x1F) * 527) + 23) >> 6;

		colors[1].r = ((((blocks[i].c1 >> 11) & 0x1F) * 527) + 23) >> 6;
		colors[1].g = ((((blocks[i].c1 >> 5) & 0x3F) * 259) + 33) >> 6;
		colors[1].b = (((blocks[i].c1 & 0x1F) * 527) + 23) >> 6;

		colors[2].r = colors[0].r * (2.0f / 3.0f) + colors[1].r * (1.0f / 3.0f);
		colors[2].g = colors[0].g * (2.0f / 3.0f) + colors[1].g * (1.0f / 3.0f);
		colors[2].b = colors[0].b * (2.0f / 3.0f) + colors[1].b * (1.0f / 3.0f);

		colors[3].r = colors[0].r * (1.0f / 3.0f) + colors[1].r * (2.0f / 3.0f);
		colors[3].g = colors[0].g * (1.0f / 3.0f) + colors[1].g * (2.0f / 3.0f);
		colors[3].b = colors[0].b * (1.0f / 3.0f) + colors[1].b * (2.0f / 3.0f);
		
		// loop over and update block pixels based on indices
		for (int h = 0; h < 4; h++)
		{
			for (int w = 0; w < 4; w++)
			{
				// calc pixel index in the outputColors space
				pixelIdx = w + h * imgWidth + (i % nBlocksPerRow) * 4 + (i / nBlocksPerRow) * 4 * imgWidth;

				// calc the color index corresponding to c0, c1, c2 or c3
				cIdx = (blocks[i].indices[h] >> (w % 4) * 2) & 0x3;
				
				// update the pixel color
				outputColors[pixelIdx].r = colors[cIdx].r;
				outputColors[pixelIdx].g = colors[cIdx].g;
				outputColors[pixelIdx].b = colors[cIdx].b;
			}
		}
	}
}

void Compressor::compressDxt1Block(const RGBTriplet* blockColors, Dxt1Block& block)
{
	RGBTriplet colors[4]; // c0, c1, c2, c3
	int c1_intensity = 255, c0_intensity = 0;

	// calculate c0 and c1 based on colors intensity in the block, TODO: use range or cluster fit algorithms
	int intensity_i;
	for (int i = 0; i < 16; ++i)
	{
		// (intensity equation source: https://en.wikipedia.org/wiki/Relative_luminance)
		// the intensity equation, generates better images but also generates unexpected alpha spots
		intensity_i = (0.2125 * blockColors[i].r) + (0.7154 * blockColors[i].g) + (0.0721 * blockColors[i].b);
		
		//cout << hex << " ci:" << toHex(blockColors[i]) << ", intensity:" << intensity_i << endl;
		if (intensity_i < c1_intensity)
		{
			c1_intensity = intensity_i;
			colors[1] = blockColors[i];
		}

		if (intensity_i > c0_intensity)
		{
			c0_intensity = intensity_i;
			colors[0] = blockColors[i];
		}
	}

	// compress c0 and c1
	unsigned short c0 = ((colors[0].r >> 3) << 11) | ((colors[0].g >> 2) << 5) | colors[0].b >> 3;
	unsigned short c1 = ((colors[1].r >> 3) << 11) | ((colors[1].g >> 2) << 5) | colors[1].b >> 3;

	if (c0 == c1) // 1 color in the block
	{
		// all pixels take c0 color (index 0)
		for (int i = 0; i < 4; ++i)
			block.indices[i] = 0;
	}
	else
	{
		// make sure c0 is bigger than c1
		if (c0 < c1)
		{
			swap(colors[0], colors[1]);
			swap(c0, c1);
		}

		// calculating c2 and c3
		colors[2].r = colors[0].r * (2.0f / 3.0f) + colors[1].r * (1.0f / 3.0f);
		colors[2].g = colors[0].g * (2.0f / 3.0f) + colors[1].g * (1.0f / 3.0f);
		colors[2].b = colors[0].b * (2.0f / 3.0f) + colors[1].b * (1.0f / 3.0f);
		colors[3].r = colors[0].r * (1.0f / 3.0f) + colors[1].r * (2.0f / 3.0f);
		colors[3].g = colors[0].g * (1.0f / 3.0f) + colors[1].g * (2.0f / 3.0f);
		colors[3].b = colors[0].b * (1.0f / 3.0f) + colors[1].b * (2.0f / 3.0f);

		// calc pixels indices
		float min_dis_sq; // minimum suqare distance
		float curr_dis_sq; // current suqare distance
		int currIndex;	// current pixel index
		for (int i = 0; i < 16; ++i) // loop over block pixels
		{
			//cout << hex << "bc:" << toHex(blockColors[i]);

			min_dis_sq = FLT_MAX;
			currIndex = -1;
			for (int j = 0; j < 4; j++) // loop over the colors c0 to c3
			{
				// calc distance squared (didnt take square root to save some performance cycles)
				curr_dis_sq = pow(blockColors[i].r - colors[j].r, 2) + pow(blockColors[i].g - colors[j].g, 2) + pow(blockColors[i].b - colors[j].b, 2);

				//cout << ", dc" << j << ":" << curr_dis_sq;

				if (curr_dis_sq < min_dis_sq)
				{
					min_dis_sq = curr_dis_sq;
					currIndex = j;
				}
			}

			//cout << ", idx:" << currIndex << endl;

			// save index to the block
			block.indices[i / 4] |= currIndex << (i % 4) * 2;
		}
	}

	//cout << bitset<8>(block.indices[0]) << endl;
	//cout << bitset<8>(block.indices[1]) << endl;
	//cout << bitset<8>(block.indices[2]) << endl;
	//cout << bitset<8>(block.indices[3]) << endl << endl;

	// save c0 and c1 in RGB565 format (16bit) to the block
	block.c0 = c0;
	block.c1 = c1;
	
	//cout << hex << "c0:" << toHex(colors[0]) << ", c1:" << toHex(colors[1]) << endl;
	//cout << hex << "c0:" << block.c0 << ", c1:" << block.c1 << endl << endl;
}

void Compressor::saveDDS(const Dxt1Block* blocks, const int nBlocks, const int imageWidth, const int imageHeight)
{
	DDS_HEADER ddsHeader;
	ddsHeader.dwMagic = 0x20534444; // 'DDS '
	ddsHeader.dwSize = 124;
	ddsHeader.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
	ddsHeader.dwHeight = imageHeight;
	ddsHeader.dwWidth = imageWidth;
	ddsHeader.dwPitchOrLinearSize = max(1, ((imageWidth + 3) / 4)) * 8; // 8: block size in bytes for dxt1
	ddsHeader.dwDepth = 0; // unused
	ddsHeader.dwMipMapCount = 0; // unused

	for (int i = 0; i < 11; ++i)
		ddsHeader.dwReserved1[i] = 0; // unused

	// pixel format header
	ddsHeader.ddspf.dwSize = 32;
	ddsHeader.ddspf.dwFlags = DDPF_FOURCC; // compressed
	ddsHeader.ddspf.dwFourCC = DDPF_DXT1; // DXT1
	ddsHeader.ddspf.dwRGBBitCount = 16; // unused
	ddsHeader.ddspf.dwRBitMask = 0x0;  // unused
	ddsHeader.ddspf.dwGBitMask = 0x0;  // unused
	ddsHeader.ddspf.dwBBitMask = 0x0;  // unused
	ddsHeader.ddspf.dwABitMask = 0x0;  // unused

	ddsHeader.dwCaps = DDSCAPS_TEXTURE;
	ddsHeader.dwCaps2 = 0;  // unused
	ddsHeader.dwCaps3 = 0;  // unused
	ddsHeader.dwCaps4 = 0;  // unused
	ddsHeader.dwReserved2 = 0;  // unused
	
	// create output file
	ofstream ddsFile;
	ddsFile.open(DDS_FILE_NAME, ofstream::out | ofstream::binary);

	// write header data
	ddsFile.write((char*)&ddsHeader, sizeof(DDS_HEADER));

	// write DXT1 blocks data
	for (int i = 0; i < nBlocks; ++i)
	{
		ddsFile.write((char*)blocks, sizeof(Dxt1Block));
		blocks++;
	}
	
	ddsFile.close();
}

void Compressor::saveBMP(const RGBTriplet* pixelColors, const int imageWidth, const int imageHeight)
{
	BMP_HEADER bmpHeader;

	int pixelsSize = sizeof(RGBTriplet) * imageWidth * imageHeight;

	// file header
	bmpHeader.signature = 0x4d42; // 'BM'
	bmpHeader.fileSize = sizeof(BMP_HEADER) + pixelsSize;
	bmpHeader.reserved = 0;	 // joining reserve1 and 2 into 8-bytes
	bmpHeader.dataOffset = sizeof(BMP_HEADER);

	// info header
	bmpHeader.infoSize = 40;
	bmpHeader.imageWidth = imageWidth;
	bmpHeader.imageHeight = imageHeight * -1; // save pixels from top to bottom
	bmpHeader.colorPlanes = 1;
	bmpHeader.colorDepth = 24;
	bmpHeader.compression = 0;
	bmpHeader.imageSize = pixelsSize;
	bmpHeader.resX = 3;
	bmpHeader.resY = 3;
	bmpHeader.ncolours = 0;
	bmpHeader.importantcolours = 0;

	// create output file
	ofstream bmpFile;
	bmpFile.open(BMP_FILE_NAME, ofstream::out | ofstream::binary);

	// write header data
	bmpFile.write((char*)&bmpHeader, sizeof(BMP_HEADER));

	// write pixel data
	bmpFile.write((char*)pixelColors, pixelsSize);

	cout << "- file coverted and saved successfully to " << BMP_FILE_NAME << endl;

	bmpFile.close();
}

void Compressor::printBMPHeader(BMP_HEADER& header) const
{
	cout << "file header:" << endl;
	cout << "* Signature: " << header.signature << endl;
	cout << "* File Size: " << header.fileSize << " byte(s)" << endl;
	cout << "* Reserved: " << header.reserved << endl;
	cout << "* Data Offset: " << header.dataOffset << endl;

	cout << "\ninfo header" << endl;
	cout << "* infoSize: " << header.infoSize << endl;
	cout << "* width: " << header.imageWidth << endl;
	cout << "* height: " << header.imageHeight << endl;
	cout << "* colorPlanes: " << header.colorPlanes << endl;
	cout << "* colorDepth:" << header.colorDepth << endl;
	cout << "* compression:" << header.compression << endl;
	cout << "* imageSize:" << header.imageSize << endl;
	cout << "* ncolours:" << header.ncolours << endl;
	cout << "* importantcolours:" << header.importantcolours << endl;
}

void Compressor::printDDSHeader(DDS_HEADER& header) const
{
	cout << "file header:" << endl;
	cout << "* dwMagic: " << header.dwMagic << endl;
	cout << "* dwSize: " << header.dwSize << endl;
	cout << "* dwFlags: " << header.dwFlags << endl;
	cout << "* dwHeight: " << header.dwHeight << endl;
	cout << "* dwWidth: " << header.dwWidth << endl;
	cout << "* dwPitchOrLinearSize: " << header.dwPitchOrLinearSize << endl;
	cout << "* dwDepth: " << header.dwDepth << endl;
	cout << "* dwMipMapCount: " << header.dwMipMapCount << endl;
	cout << "* dwReserved1[11]: " << header.dwReserved1 << endl;
	cout << "* dwCaps: " << header.dwCaps << endl;
	cout << "* dwCaps2: " << header.dwCaps2 << endl;
	cout << "* dwCaps3: " << header.dwCaps3 << endl;
	cout << "* dwCaps4: " << header.dwCaps4 << endl;
	cout << "* dwReserved2: " << header.dwReserved2 << endl;

	cout << "\npixel format header:" << endl;
	cout << "* dwSize: " << header.ddspf.dwSize << endl;
	cout << "* dwFlags: " << header.ddspf.dwFlags << endl;
	cout << "* dwFourCC: " << header.ddspf.dwFourCC << endl;
	cout << "* dwRGBBitCount: " << header.ddspf.dwRGBBitCount << endl;
	cout << "* dwRBitMask: " << header.ddspf.dwRBitMask << endl;
	cout << "* dwGBitMask: " << header.ddspf.dwGBitMask << endl;
	cout << "* dwBBitMask: " << header.ddspf.dwBBitMask << endl;
	cout << "* dwABitMask: " << header.ddspf.dwABitMask << endl;
}

bool Compressor::isValidBMPFile(BMP_HEADER& header) const
{
	// check correct BMP signature
	if (header.signature != 'MB')
	{
		cout << "* file is not a BMP file." << endl;
		return false;
	}

	// check 24bit BMP
	if (header.colorDepth != 24)
	{
		cout << "* only 24bit BMP files supported." << '\n';
		return false;
	}

	// check not compressed
	if (header.compression != 0)
	{
		cout << "* only uncompressed BMP files supported." << '\n';
		return false;
	}

	// check dimensions are divisible by 4
	if (header.imageWidth % 4 != 0 ||
		header.imageHeight % 4 != 0)
	{
		cout << "* only BMP files with width/height divisible by 4 are supported." << '\n';
		return false;
	}

	return true;
}

bool Compressor::isValidDDSFile(DDS_HEADER& header) const
{
	if (header.dwMagic != 0x20534444 || header.dwSize != 124 ||
		!(header.dwFlags & DDSD_PIXELFORMAT) || !(header.dwFlags & DDSD_CAPS))
	{
		cout << "Invalid DDS file." << endl;
		return false;
	}

	if (header.ddspf.dwFlags != DDPF_FOURCC ||
		header.ddspf.dwFourCC != DDPF_DXT1) // check DXT1 compressed
	{
		cout << "Only DXT1 compressed file are supported." << endl;
		return false;
	}

	// check dimensions are divisible by 4
	if (header.dwWidth % 4 != 0 ||
		header.dwHeight % 4 != 0)
	{
		cout << "* only DDS files with width/height divisible by 4 are supported." << '\n';
		return false;
	}

	return true;
}