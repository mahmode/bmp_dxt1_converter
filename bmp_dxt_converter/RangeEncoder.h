/**
RangeEncoder.h
Purpose: Implementation of a color range fit algorithm for choosing c0 and c1 in a DX1 compressed block.
The idea is to calculate the principal axis in the block color space (using principal component analysis "PCA")
and choose the min and max points on the principal axis as c0 and c1. Implementation is not complete yet

@author Mahmoud Badri (mhdside@hotmail.com)
@version 1.2 12/02/2017
*/
#pragma once

#include "bmp_dxt1_headers.h"

/**
A point in the color space
TODO: Move this struct to another header
*/
struct VecRGB
{
	float r, g, b;

	VecRGB(void) {}
	VecRGB(const float r, const float g, const float b) :r(r), g(g), b(b) {}

	VecRGB operator + (const VecRGB& v) const
	{
		return VecRGB(r + v.r, g + v.g, b + v.b);
	}

	VecRGB operator - (const VecRGB& v) const
	{
		return VecRGB(r - v.r, g - v.g, b - v.b);
	}

	void operator += (const VecRGB& v)
	{
		r += v.r;
		g += v.g;
		b += v.b;
	}

	void operator += (const float num)
	{
		r += num;
		g += num;
		b += num;
	}

	void operator /= (const VecRGB& v)
	{
		r /= v.r;
		g /= v.g;
		b /= v.b;
	}

	float dot(const VecRGB& v) const
	{
		return v.r*r + v.g*g + v.b*b;
	}
};

class RangeEncoder
{
private:
	/**
	Calculate the covariance in a block's pixels' color space. This is required to progress with doing
	principal component analysis calculations.

	@param blockColors array of pixels colors
	@param coverianceMat output 3x3 symmetric convariance mattrix (TODO: create a matrix class?)
	*/
	void getCovariance(const VecRGB* blockColors, float* coverianceMat);
public:
	RangeEncoder();
	~RangeEncoder();
	
	/**
	Calculate the principal axis using principal component analysis.

	@param blockColors array of pixels colors
	@param outputColors output array of the 4 chosen compressed block colors)
	*/
	void calctPrincipleAxis(const VecRGB* blockColors, VecRGB* outputColors);
};

