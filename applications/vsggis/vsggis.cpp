#include <vsg/all.h>

#include <chrono>
#include <iostream>
#include <ostream>
#include <thread>

#include <vsgGIS/gdal_utils.h>

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
        if (dataset) datasets.push_back(dataset);
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

    int width = datasets.front()->GetRasterXSize();
    int height =  datasets.front()->GetRasterYSize();

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
                std::cout<<"Undefined classifciotn on raster band "<<i<<std::endl;
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


    vsg::Path output_filename = arguments[argc-1];
    vsg::write(image, output_filename);

    std::cout<<"Written output to "<<output_filename<<std::endl;

    return 0;
}
