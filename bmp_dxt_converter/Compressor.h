/**
Compressor.h
Purpose: Converts from bmp to DXT1 dds and vice versa

@author Mahmoud Badri (mhdside@hotmail.com)
@version 1.2 12/02/2017
*/

#pragma once

#include <string>
#include "bmp_dxt1_headers.h"

using namespace std;

// generated dds, bmp file names(TODO: use input file name for output file instead of fixed output name)
#define	DDS_FILE_NAME	"dds_output.dds"	
#define	BMP_FILE_NAME	"bmp_output.bmp"

class Compressor
{
private:
	/**
	Compress bmp pixels colors into DXT1 blocks

	@param bmpBuffer source colors to compress
	@param blocks target blocks where the compressed colors and indices will be saved
	@param imgWidth image width
	@param imgHeight image height
	@param isBottomUp if true, the bmp pixels array "bmpBuffer" is stored from bottom to top
	*/
	void compressBMP(const RGBTriplet* bmpBuffer, Dxt1Block* blocks, const int imgWidth, const int imgHeight, const bool isBottomUp);

	/**
	Compress 16 pixel colors into 1 DXT1 block (2 RGB565 colors and 16 indices)
	
	@param blockColors source 16 pixel colors to compress
	@param block target block where the 2 colors and indices will be saved
	*/
	void compressDxt1Block(const RGBTriplet* blockColors, Dxt1Block& block);

	/**
	Decompress dds blocks into pixel colors.

	@param blocks source blocks containing compressed data
	@param outputColors target colors where expanded pixel colors will be saved
	@param nBlocks number of blocks
	@param imgWidth image width
	*/
	void Compressor::decompressDDS(const Dxt1Block* blocks, RGBTriplet* outputColors, const int nBlocks, const int imgWidth);
	
	/**
	Save DXT1 compressed blocks to a dds file

	@param blocks DXT1 blocks to be saved
	@param nBlocks number of blocks
	@param imgWidth image width
	@param imgHeight image height
	*/
	void saveDDS(const Dxt1Block* blocks, const int nBlocks, const int imageWidth, const int imageHeight);

	/**
	Save pixel colors to a bmp file

	@param pixelColors pixels colors to save
	@param imgWidth image width
	@param imgHeight image height
	*/
	void saveBMP(const RGBTriplet* pixelColors, const int imageWidth, const int imageHeight);

	/**
	Check that a BMP file is valid. A BMP file is valid if it is uncompressed 24bit, dimensions devisible by 4

	@param header the BMP file header (incliding the info header)
	*/
	bool isValidBMPFile(BMP_HEADER& header) const;

	/**
	Check that a DDS file is valid. A DDS file is valid if it is compressed using DXT1 format
	and dimensions devisible by 4

	@param header the DDS file header (including the info header)
	*/
	bool isValidDDSFile(DDS_HEADER& header) const;

	/**
	Print the header of a BMP file (for debug purpose)
	*/
	void printBMPHeader(BMP_HEADER& header) const;

	/**
	Print the header of a DDS file  (for debug purpose)

	@param header the DDS file header (including the DDS magic number)
	*/
	void printDDSHeader(DDS_HEADER& header) const;


public:
	Compressor();
	~Compressor();

	/**
	Load a BMP file and compress it using DDX1 and save the file as .dds
	BMP image must be uncompressed 24bit, dimensions devisible by 4

	@param filePath BMP file path
	*/
	void compress(const string& filePath);

	/**
	Load a DDS file and decompress it to BMP and save the file as .bmp
	DDS file must be compressed using DXT1 and dimentions divisible by 4

	@param filePath DDS file path
	*/
	void decompress(const string&  filePath);
};