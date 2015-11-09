// --------------------------------------------------------------------------
// DSM Viewer
// Copyright(C) 2015
// Christopher Mitchell
//                                                                            
// All rights reserved. See main.cc for license.                              
//                                                                          
// --------------------------------------------------------------------------
   
#pragma once

#include <gdal/gdal_priv.h>
#include <gdal/cpl_conv.h>            // for CPLMalloc()
 
class GDALHelper {
  public:
	GDALHelper(std::string filename);
	void printGDALInfo(void);
	void selectBand(const int band);
	int getBandSize(size_t& x, size_t& y);
	int getBandExtents(float* points);
	int getBandScanline(float*& arr_x, float*& arr_y, float*& arr_z, const int y);
	int freeBandArray(float* arr);

  private:
	std::string filename_;
	GDALDataset* dataset_;

	GDALRasterBand* po_band_;
	int n_block_x_size_, n_block_y_size_;
	double adf_geo_transform_[6];
	double adf_min_max_[2];
};
