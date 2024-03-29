#include <vsg/all.h>

#include <chrono>
#include <ostream>
#include <thread>

#include <vsgGIS/gdal_utils.h>
#include <vsgGIS/meta_utils.h>

int main(int argc, char** argv)
{
    vsgGIS::initGDAL();

    vsg::CommandLine arguments(&argc, argv);

    if (argc < 3)
    {
        vsg::info("usage:\n    vsggis input.tif [input.tif] [input.tif] [inputfile.tif] output.vsgt");
        return 1;
    }

    std::vector<std::shared_ptr<GDALDataset>> datasets;

    for (int ai = 1; ai < argc - 1; ++ai)
    {
        auto dataset = vsgGIS::openSharedDataSet(arguments[ai], GA_ReadOnly);
        if (dataset)
        {
            datasets.push_back(dataset);
        }
    }

    if (datasets.empty())
    {
        vsg::info("No datasets loaded.");
        return 1;
    }

    bool result = vsgGIS::all_equal(datasets.begin(), datasets.end(), vsgGIS::compatibleDatasetProjectionsTransformAndSizes);
    if (!result)
    {
        vsg::info("datasets are not compatible.");
        return 1;
    }

    auto types = vsgGIS::dataTypes(datasets.begin(), datasets.end());
    if (types.size() > 1)
    {
        vsg::info("multiple input data types not suported.");
        for (auto& type : types)
        {
            vsg::info("   GDALDataType ", GDALGetDataTypeName(type));
        }
        return 1;
    }

    vsg::info("datasets are compatible.");

    GDALDataType dataType = *types.begin();

    auto main_dataset = datasets.front();

    int width = main_dataset->GetRasterXSize();
    int height = main_dataset->GetRasterYSize();

    std::vector<GDALRasterBand*> rasterBands;
    for (auto& dataset : datasets)
    {
        for (int i = 1; i <= dataset->GetRasterCount(); ++i)
        {
            GDALRasterBand* band = dataset->GetRasterBand(i);
            GDALColorInterp classification = band->GetColorInterpretation();

            if (classification != GCI_Undefined)
            {
                rasterBands.push_back(band);
            }
            else
            {
                vsg::info("Undefined classification on raster band ", i);
            }
        }
    }

    vsg::info("rasterBands.size() = ", rasterBands.size());

    int numComponents = rasterBands.size();
    if (numComponents == 3) numComponents = 4;

    if (numComponents > 4)
    {
        vsg::info("Too many raster bands to merge into a single output, maximum of 4 raster bands supported.");
        return 1;
    }

    auto image = vsgGIS::createImage2D(width, height, numComponents, dataType, vsg::dvec4(0.0, 0.0, 0.0, 1.0));

    for (int component = 0; component < static_cast<int>(rasterBands.size()); ++component)
    {
        vsgGIS::copyRasterBandToImage(*rasterBands[component], *image, component);
    }

    if (main_dataset->GetProjectionRef())
    {
        image->setValue("ProjectionRef", std::string(main_dataset->GetProjectionRef()));
    }

    auto transform = vsg::doubleArray::create(6);
    if (main_dataset->GetGeoTransform(transform->data()) == CE_None)
    {
        image->setObject("GeoTransform", transform);
    }

    vsgGIS::assignMetaData(*main_dataset, *image);

    vsg::Path output_filename = arguments[argc - 1];
    vsg::write(image, output_filename);

    vsg::info("Written output to ", output_filename);

    return 0;
}
