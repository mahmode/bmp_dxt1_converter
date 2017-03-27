/**
RangeEncoder.cpp
Purpose: Implementation of a color range fit algorithm for choosing c0 and c1 in a DX1 compressed block.
The idea is to calculate the principal axis in the block color space (using principal component analysis "PCA")
and choose the min and max points on the principal axis as c0 and c1. Implementation is not complete yet

@author Mahmoud Badri (mhdside@hotmail.com)
@version 1.2 12/02/2017
*/

#include "RangeEncoder.h"

RangeEncoder::RangeEncoder()
{
}


RangeEncoder::~RangeEncoder()
{
}

void RangeEncoder::getCovariance(const VecRGB* blockColors, float* coverianceMat)
{
	// calculate the mean color
	VecRGB meanColor;
	for (int i = 0; i < 16; ++i)
	{
		meanColor += blockColors[i];
	}
	
	meanColor.r /= 16;
	meanColor.g /= 16;
	meanColor.b /= 16;
	
	// calculate the covariance matrix
	for (int i = 0; i < 16; ++i)
	{
		VecRGB a = blockColors[i] - meanColor;

		coverianceMat[0] += a.r*a.r;
		coverianceMat[1] += a.r*a.g;
		coverianceMat[2] += a.r*a.b;
		coverianceMat[5] += a.g*a.g;
		coverianceMat[6] += a.g*a.b;
		coverianceMat[9] += a.b*a.b;
	}

	// from symmetry
	coverianceMat[4] = coverianceMat[1];
	coverianceMat[7] = coverianceMat[2];
	coverianceMat[8] = coverianceMat[5];
}

void RangeEncoder::calctPrincipleAxis(const VecRGB* blockColors, VecRGB* outputColors)
{
	// calculate coveriance
	float coverianceMat[9]; // symmetric 3x3 mat
	getCovariance(blockColors, coverianceMat);

	/* TODOs:
		1. calculate eigenvalues and eigenvectors (preferably using a vector math lib.)
		2. sort them and take the largest (principal axis)
		3. take the max and min point on the princial axies as c0 and c1
	*/
}