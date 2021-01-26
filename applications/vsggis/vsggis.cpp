#include <vsg/all.h>

#include <chrono>
#include <iostream>
#include <ostream>
#include <thread>

#include "gdal_priv.h"

int main(int argc, char** argv)
{
    vsg::CommandLine arguments(&argc, argv);

    std::cout<<"vsggis"<<std::endl;

    if (argc <= 1) return 0;

    GDALAllRegister();


    for(int ai=1; ai<argc; ++ai)
    {
        std::string filename = arguments[ai];

        GDALDataset* poDataset = static_cast<GDALDataset*>(GDALOpen(filename.c_str(), GA_ReadOnly));
        if (poDataset)
        {
            std::cout<<"\nSuccessfully loaded "<<filename<<",  poDataset = "<<poDataset<<std::endl;
            double        adfGeoTransform[6];
            printf( "Driver: %s/%s\n",
                    poDataset->GetDriver()->GetDescription(),
                    poDataset->GetDriver()->GetMetadataItem( GDAL_DMD_LONGNAME ) );
            printf( "Size is %dx%dx%d\n",
                    poDataset->GetRasterXSize(), poDataset->GetRasterYSize(),
                    poDataset->GetRasterCount() );
            if( poDataset->GetProjectionRef()  != NULL )
                printf( "Projection is `%s'\n", poDataset->GetProjectionRef() );
            if( poDataset->GetGeoTransform( adfGeoTransform ) == CE_None )
            {
                printf( "Origin = (%.6f,%.6f)\n",
                        adfGeoTransform[0], adfGeoTransform[3] );
                printf( "Pixel Size = (%.6f,%.6f)\n",
                        adfGeoTransform[1], adfGeoTransform[5] );

                std::cout<<"adfGeoTransform "<<adfGeoTransform[0]<<"\t"<<adfGeoTransform[1]<<"\t"<<adfGeoTransform[2]<<std::endl;
                std::cout<<"                "<<adfGeoTransform[3]<<"\t"<<adfGeoTransform[4]<<"\t"<<adfGeoTransform[5]<<std::endl;
            }


            for(int i=1; i<=poDataset->GetRasterCount(); ++i)
            {
                int             nBlockXSize, nBlockYSize;
                GDALRasterBand  *poBand = poDataset->GetRasterBand( i );
                std::cout<<"GDALRasterBand["<<i<<")" <<poBand<<std::endl;

                poBand->GetBlockSize( &nBlockXSize, &nBlockYSize );
                printf( "    Block=%dx%d Type=%s, ColorInterp=%s\n",
                        nBlockXSize, nBlockYSize,
                        GDALGetDataTypeName(poBand->GetRasterDataType()),
                        GDALGetColorInterpretationName(
                            poBand->GetColorInterpretation()) );

                int             bGotMin, bGotMax;
                double          adfMinMax[2];
                adfMinMax[0] = poBand->GetMinimum( &bGotMin );
                adfMinMax[1] = poBand->GetMaximum( &bGotMax );
                if( ! (bGotMin && bGotMax) )
                    GDALComputeRasterMinMax((GDALRasterBandH)poBand, TRUE, adfMinMax);
                printf( "    Min=%.3f, Max=%.3f\n", adfMinMax[0], adfMinMax[1] );
                if( poBand->GetOverviewCount() > 0 )
                    printf( "    Band has %d overviews.\n", poBand->GetOverviewCount() );
                if( poBand->GetColorTable() != NULL )
                    printf( "    Band has a color table with %d entries.\n",
                            poBand->GetColorTable()->GetColorEntryCount() );
            }
            GDALClose(poDataset);
        }
    }


    return 0;
}
