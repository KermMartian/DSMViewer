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
	if(dataset_->GetProjectionRef()  != NULL ) {
		printf( "Projection is `%s'\n", dataset_->GetProjectionRef() );
	}
	if(dataset_->GetGeoTransform( adfGeoTransform ) == CE_None ) {
		printf( "Origin = (%.6f,%.6f)\n",
				adfGeoTransform[0], adfGeoTransform[3] );
		printf( "Pixel Size = (%.6f,%.6f)\n",
				adfGeoTransform[1], adfGeoTransform[5] );
	}
}

