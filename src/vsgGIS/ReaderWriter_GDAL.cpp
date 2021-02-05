/* <editor-fold desc="MIT License">

Copyright(c) 2021 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsgGIS/ReaderWriter_GDAL.h>
#include <vsgGIS/gdal_utils.h>

#include <cstring>
#include <iostream>

using namespace vsgGIS;

ReaderWriter_GDAL::ReaderWriter_GDAL()
{
}

vsg::ref_ptr<vsg::Object> ReaderWriter_GDAL::read(const vsg::Path& filename, vsg::ref_ptr<const vsg::Options> options) const
{
    vsg::Path filenameToUse = vsg::findFile(filename, options);
    if (filenameToUse.empty()) return {};

    vsgGIS::initGDAL();

    auto dataset = vsgGIS::openSharedDataSet(filenameToUse.c_str(), GA_ReadOnly);
    if (!dataset)
    {
        return {};
    }

    char ** metaData = dataset->GetMetadata();
    std::cout<<"ReaderWriter_GDAL::read("<<filename<<") metaData = "<<metaData<<std::endl;


    auto types = vsgGIS::dataTypes(*dataset);
    if (types.size() > 1)
    {
        std::cout << "ReaderWriter_GDAL::read("<<filename<<") multiple input data types not suported." << std::endl;
        for (auto& type : types)
        {
            std::cout << "   GDALDataType " << GDALGetDataTypeName(type) << std::endl;
        }
        return {};
    }
    GDALDataType dataType = *types.begin();

    std::vector<GDALRasterBand*> rasterBands;
    for(int i = 1; i <= dataset->GetRasterCount(); ++i)
    {
        GDALRasterBand* band = dataset->GetRasterBand(i);
        GDALColorInterp classification = band->GetColorInterpretation();

        if (classification != GCI_Undefined)
        {
            rasterBands.push_back(band);
        }
        else
        {
            std::cout<<"ReaderWriter_GDAL::read("<<filename<<") Undefined classification on raster band "<<i<<std::endl;
        }
    }

    int numComponents = rasterBands.size();
    if (numComponents==0)
    {
        std::cout<<"ReaderWriter_GDAL::read("<<filename<<") failed numComponents = "<<numComponents<<std::endl;
        return {};
    }

    if (numComponents==3) numComponents = 4;

    if (numComponents>4)
    {
        std::cout<<"ReaderWriter_GDAL::read("<<filename<<") Too many raster bands to merge into a single output, maximum of 4 raster bands supported."<<std::endl;
        return {};
    }

    int width = dataset->GetRasterXSize();
    int height =  dataset->GetRasterYSize();

    auto image = vsgGIS::createImage2D(width, height, numComponents, dataType, vsg::dvec4(0.0, 0.0, 0.0, 1.0));

    for(int component = 0; component < static_cast<int>(rasterBands.size()); ++component)
    {
        vsgGIS::copyRasterBandToImage(*rasterBands[component], *image, component);
    }

    vsgGIS::assignMetaData(*dataset, *image);

    if (dataset->GetProjectionRef() && std::strlen(dataset->GetProjectionRef())>0)
    {
        image->setValue("ProjectionRef", std::string(dataset->GetProjectionRef()));
    }

    auto transform = vsg::doubleArray::create(6);
    if (dataset->GetGeoTransform( transform->data() ) == CE_None)
    {
        image->setObject("GeoTransform", transform);
    }

    return image;
}
