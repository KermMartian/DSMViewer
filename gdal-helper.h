#pragma once

#include <gdal/gdal_priv.h>
 
class GDALHelper {
  public:
	GDALHelper(std::string filename);
	void printGDALInfo(void);
	void selectBand(int band);
  private:
	std::string filename_;
	GDALDataset* dataset_;

	GDALRasterBand* poBand_;
	int nBlockXSize_, nBlockYSize_;
	int bGotMin_, bGotMax_;
	double adfMinMax_[2];
};
