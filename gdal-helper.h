#pragma once

#include <gdal/gdal_priv.h>
 
class GDALHelper {
  public:
	GDALHelper(std::string filename);
	void printGDALInfo(void);
  private:
	std::string filename_;
	GDALDataset* dataset_;
};
