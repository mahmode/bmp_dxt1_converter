/**
bmp_dxt1_converter.cpp
Purpose: main class. The app converts from bmp to DXT1 dds and vice versa

@author Mahmoud Badri (mhdside@hotmail.com)
@version 1.2 12/02/2017
*/

#include "stdafx.h"
#include <iostream>
#include <string>
#include "Compressor.h"
#include <bitset>

using namespace std;

int main()
{
	string filePath;
	bool quit = false;
	Compressor compressor;

	cout << "=======================================================================================" << endl;
	cout << "================================== DDS/BMP converter ==================================" << endl;
	cout << "=======================================================================================" << endl;

	while (!quit)
	{
		cout << endl << "Enter a .bmp/.dds file path (or Q to quit): ";
		cin >> filePath;
		cout << endl;

		string ext = filePath.substr(filePath.find_last_of(".") + 1);
		if (ext == "bmp" || ext == "BMP") // convert bmp to dxt1 dds
		{
			compressor.compress(filePath);
		}
		else if (ext == "dds" || ext == "DDS") // convert dxt1 dds to bmp
		{
			compressor.decompress(filePath);
		}
		else if (filePath == "q" || filePath == "Q") // quit
		{
			quit = true;
		}
		else
		{
			cout << "- Invalid file name or bad command, please try again." << endl;
		}
	}
	
    return 0;
}