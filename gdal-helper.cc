// --------------------------------------------------------------------------
// DSM Viewer
// Copyright(C) 2015
// Christopher Mitchell
//                                                                            
// All rights reserved. See main.cc for license.                              
//                                                                          
// --------------------------------------------------------------------------
   
#include "gdal-helper.h"

GDALHelper::GDALHelper(std::string filename) :
	filename_(filename), po_band_(NULL)
{
	GDALAllRegister();
	dataset_ = (GDALDataset*)GDALOpen(filename.c_str(), GA_ReadOnly);
	if (dataset_ == NULL) {
		fprintf(stderr, "Failed to open GDAL dataset '%s'\n", filename.c_str());
	}
}

/* Display information about this GDAL file. */
void GDALHelper::printGDALInfo(void) {
	const char* projection = NULL;

	fprintf(stdout, "Driver: %s/%s\n",
		    dataset_->GetDriver()->GetDescription(), 
		    dataset_->GetDriver()->GetMetadataItem(GDAL_DMD_LONGNAME));
	fprintf(stdout, "Size is %dx%dx%d\n", 
		    dataset_->GetRasterXSize(), dataset_->GetRasterYSize(),
		    dataset_->GetRasterCount());
	if (NULL != (projection = dataset_->GetProjectionRef())) {
		fprintf(stdout, "Projection is `%s'\n", projection);
	}
	if (dataset_->GetGeoTransform(adf_geo_transform_) == CE_None) {
		fprintf(stdout, "Origin = (%.6f,%.6f)\n",
				 adf_geo_transform_[0], adf_geo_transform_[3]);
		fprintf(stdout, "Pixel Size = (%.6f,%.6f)\n",
				 adf_geo_transform_[1], adf_geo_transform_[5]);
	}
}

/* Get information about a single band in the GDAL file
   and load to local state. */
void GDALHelper::selectBand(const int band) {
	int b_got_min = false, b_got_max = false;

	po_band_ = dataset_->GetRasterBand(band);
	po_band_->GetBlockSize(&n_block_x_size_, &n_block_y_size_);
	printf("Block=%dx%d Type=%s, ColorInterp=%s\n",
	       n_block_x_size_, n_block_y_size_,
	       GDALGetDataTypeName(po_band_->GetRasterDataType()),
	       GDALGetColorInterpretationName(
	       po_band_->GetColorInterpretation()));

	// Try to get min and max encoded into band
	adf_min_max_[0] = po_band_->GetMinimum(&b_got_min);
	adf_min_max_[1] = po_band_->GetMaximum(&b_got_max);

	// If failed, do a more expensive computation to compute min and max
	if (!(b_got_min && b_got_max)) {
		GDALComputeRasterMinMax((GDALRasterBandH)po_band_, TRUE, adf_min_max_);
	}

	// Display (but do not use) information about an overview for this band
	printf( "Min=%.3fd, Max=%.3f\n", adf_min_max_[0], adf_min_max_[1]);
	if (po_band_->GetOverviewCount() > 0) {
		printf("Band has %d overviews.\n", po_band_->GetOverviewCount());
	}
	// Display (but do not use) information about a color table for this band
	if (po_band_->GetColorTable() != NULL) {
		printf("Band has a color table with %d entries.\n", 
		       po_band_->GetColorTable()->GetColorEntryCount());
	}
	return;
}

int GDALHelper::getBandSize(size_t& x, size_t& y) {
	if (!po_band_)
		return -1;

	x = (size_t)po_band_->GetXSize();
	y = (size_t)po_band_->GetYSize();

	return 0;
}

/* Takes 6-element array of floats, and returns
   [min_x, max_x, min_y, max_y, min_z, max_z] */
int GDALHelper::getBandExtents(float* points) {
	if (!po_band_)
		return -1;

	// Get candidate minimum and maximum X and Y. Because the
	// affine transformation can cause reflection, don't assume
	// that xa is xmin and ya is ymin.
	float xa, xb, ya, yb;
	xa = adf_geo_transform_[0];
	xb = xa + (po_band_->GetXSize() - 1) * adf_geo_transform_[1];
	ya = adf_geo_transform_[3];
	yb = ya + (po_band_->GetYSize() - 1) * adf_geo_transform_[5];

	// Now populate the array. The fmin/fmax on Z is probably unnecessary.
	points[0] = fmin(xa, xb);
	points[1] = fmax(xa, xb);
	points[2] = fmin(ya, yb);
	points[3] = fmax(ya, yb);
	points[4] = fmin(adf_min_max_[0], adf_min_max_[1]);
	points[5] = fmax(adf_min_max_[0], adf_min_max_[1]);
	
	return 0;
}

int GDALHelper::getBandScanline(float*& arr_x, float*& arr_y, float*& arr_z, const int y) {
	float *pafScanline;
	int nx_size = po_band_->GetXSize();

	// Reserve memory for X, Y, and Z arrays
	arr_x = (float*)malloc(sizeof(float) * nx_size);
	arr_y = (float*)malloc(sizeof(float) * nx_size);
	arr_z = (float*)malloc(sizeof(float) * nx_size);
	if (!arr_z || !arr_x || !arr_y) {
		fprintf(stderr, "Exhausted memory attempting to reserve memory for scanline\n");
	}

	// Get Z values
	po_band_->RasterIO(GF_Read, 0, y, nx_size, 1, 
					   arr_z, nx_size, 1, GDT_Float32, 
					   0, 0);

	// Compute X and Y values
	for(size_t i = 0; i < nx_size; i++) {
		arr_x[i] = adf_geo_transform_[0] + (i * adf_geo_transform_[1]) + (y * adf_geo_transform_[2]);
		arr_y[i] = adf_geo_transform_[3] + (i * adf_geo_transform_[4]) + (y * adf_geo_transform_[5]);
	}

	return 0;
}

int GDALHelper::freeBandArray(float* arr) {
	free((void*)arr);
	return 0;
}
