#include <vsg/all.h>

#include <chrono>
#include <iostream>
#include <ostream>
#include <thread>

#include <vsgGIS/gdal_utils.h>
#include <vsgGIS/meta_utils.h>

void processMetaData(GDALDataset& dataset)
{
    // https://gdal.org/drivers/raster/jpeg.html
    char ** metaData = dataset.GetMetadata();
    if (!metaData) return;

    std::map<std::string, std::string> metaMap;

    for(char** ptr = metaData; *ptr != 0; ++ptr)
    {
        std::string line(*ptr);
        auto equal_pos = line.find('=');
        if (equal_pos==std::string::npos)
        {
            metaMap[line] = "";
        }
        else
        {
            metaMap[line.substr(0, equal_pos)] = line.substr(equal_pos+1, std::string::npos);
        }
    }

    for(auto& [key, value] : metaMap)
    {
        std::cout<<key<<" ["<<value<<"]"<<std::endl;
    }

    auto getAngle = [](const std::string& value) -> double
    {
        double degrees;
        std::stringstream str(value);
        str >> vsgGIS::dms_in_brackets(degrees);
        return degrees;
    };

    auto getDouble = [](const std::string& value) -> double
    {
        double v;
        std::stringstream str(value);
        str >> vsgGIS::in_brackets(v);
        return v;
    };

    if (auto itr = metaMap.find("EXIF_GPSLatitude"); itr != metaMap.end())
    {
        std::cout<<"latitude = "<<getAngle(itr->second)<<std::endl;
    }

    if (auto itr = metaMap.find("EXIF_GPSLongitude"); itr != metaMap.end())
    {
        std::cout<<"longitude = "<<getAngle(itr->second)<<std::endl;
    }
    if (auto itr = metaMap.find("EXIF_GPSAltitude"); itr != metaMap.end())
    {
        std::cout<<"altitude = "<<getDouble(itr->second)<<std::endl;
    }
}


int main(int argc, char** argv)
{
    vsg::CommandLine arguments(&argc, argv);

    GDALAllRegister();

    if (argc<3)
    {
        std::cout << "usage:\n    vsggis input.tif [input.tif] [input.tif] [inputfile.tif] output.vsgt" << std::endl;
        return 1;
    }

    std::vector<std::shared_ptr<GDALDataset>> datasets;

    for (int ai = 1; ai < argc-1; ++ai)
    {
        auto dataset = vsgGIS::openSharedDataSet(arguments[ai], GA_ReadOnly);
        if (dataset)
        {
            datasets.push_back(dataset);

            processMetaData(*dataset);
        }
    }

    if (datasets.empty())
    {
        std::cout<<"No datasets loaded."<<std::endl;
        return 1;
    }

    bool result = vsgGIS::all_equal(datasets.begin(), datasets.end(), vsgGIS::compatibleDatasetProjectionsTransformAndSizes);
    if (!result)
    {
        std::cout << "datasets are not compatible." << std::endl;
        return 1;
    }

    auto types = vsgGIS::dataTypes(datasets.begin(), datasets.end());
    if (types.size() > 1)
    {
        std::cout << "multiple input data types not suported." << std::endl;
        for (auto& type : types)
        {
            std::cout << "   GDALDataType " << GDALGetDataTypeName(type) << std::endl;
        }
        return 1;
    }

    std::cout << "datasets are compatible." << std::endl;

    GDALDataType dataType = *types.begin();

    auto main_dataset = datasets.front();

    int width = main_dataset->GetRasterXSize();
    int height =  main_dataset->GetRasterYSize();

    std::vector<GDALRasterBand*> rasterBands;
    for(auto& dataset : datasets)
    {
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
                std::cout<<"Undefined classification on raster band "<<i<<std::endl;
            }
        }
    }

    std::cout<<"rasterBands.size() = "<<rasterBands.size()<<std::endl;

    int numComponents = rasterBands.size();
    if (numComponents==3) numComponents = 4;

    if (numComponents>4)
    {
        std::cout<<"Too many raster bands to merge into a single output, maximum of 4 raster bands supported."<<std::endl;
        return 1;
    }

    auto image = vsgGIS::createImage2D(width, height, numComponents, dataType, vsg::dvec4(0.0, 0.0, 0.0, 1.0));

    for(int component = 0; component < static_cast<int>(rasterBands.size()); ++component)
    {
        vsgGIS::copyRasterBandToImage(*rasterBands[component], *image, component);
    }

    if (main_dataset->GetProjectionRef())
    {
        image->setValue("ProjectionRef", std::string(main_dataset->GetProjectionRef()));
    }

    auto transform = vsg::doubleArray::create(6);
    if (main_dataset->GetGeoTransform( transform->data() ) == CE_None)
    {
        image->setObject("GeoTransform", transform);
    }

    vsgGIS::assignMetaData(*main_dataset, *image);

    vsg::Path output_filename = arguments[argc-1];
    vsg::write(image, output_filename);

    std::cout<<"Written output to "<<output_filename<<std::endl;

    return 0;
}
