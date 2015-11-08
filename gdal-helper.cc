#include "gdal-helper.h"

GDALHelper::GDALHelper(std::string filename) :
	filename_(filename)
{
	GDALAllRegister();
	dataset_ = (GDALDataset*)GDALOpen(filename.c_str(), GA_ReadOnly);
	if (dataset_ == NULL) {
		fprintf(stderr, "Failed to open GDAL dataset '%s'\n", filename.c_str());
	}
}

void GDALHelper::printGDALInfo(void) {
	double adfGeoTransform[6];
	printf("Driver: %s/%s\n",
		dataset_->GetDriver()->GetDescription(), 
		dataset_->GetDriver()->GetMetadataItem( GDAL_DMD_LONGNAME ) );
	printf("Size is %dx%dx%d\n", 
		dataset_->GetRasterXSize(), dataset_->GetRasterYSize(),
		dataset_->GetRasterCount() );
	if (dataset_->GetProjectionRef()  != NULL ) {
		printf( "Projection is `%s'\n", dataset_->GetProjectionRef() );
	}
	if (dataset_->GetGeoTransform( adfGeoTransform ) == CE_None ) {
		printf( "Origin = (%.6f,%.6f)\n",
				adfGeoTransform[0], adfGeoTransform[3] );
		printf( "Pixel Size = (%.6f,%.6f)\n",
				adfGeoTransform[1], adfGeoTransform[5] );
	}
}

void GDALHelper::selectBand(int band) {
	poBand_ = dataset_->GetRasterBand(band);
	poBand_->GetBlockSize(&nBlockXSize_, &nBlockYSize_);
	printf("Block=%dx%d Type=%s, ColorInterp=%s\n",
	       nBlockXSize_, nBlockYSize_,
	       GDALGetDataTypeName(poBand_->GetRasterDataType()),
	       GDALGetColorInterpretationName(
	       poBand_->GetColorInterpretation()));
	adfMinMax_[0] = poBand_->GetMinimum(&bGotMin_);
	adfMinMax_[1] = poBand_->GetMaximum(&bGotMax_);
	if (!(bGotMin_ && bGotMax_)) {
		GDALComputeRasterMinMax((GDALRasterBandH)poBand_, TRUE, adfMinMax_);
	}

	printf( "Min=%.3fd, Max=%.3f\n", adfMinMax_[0], adfMinMax_[1]);
	if (poBand_->GetOverviewCount() > 0) {
		printf("Band has %d overviews.\n", poBand_->GetOverviewCount());
	}
	if (poBand_->GetColorTable() != NULL) {
		printf("Band has a color table with %d entries.\n", 
		       poBand_->GetColorTable()->GetColorEntryCount());
	}
	return;
}
