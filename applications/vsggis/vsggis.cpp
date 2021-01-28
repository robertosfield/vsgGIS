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

    std::vector<std::shared_ptr<GDALDataset>> datasets;

    for (int ai = 1; ai < argc; ++ai)
    {
        std::string filename = arguments[ai];

        datasets.push_back(vsgGIS::openSharedDataSet(filename.c_str(), GA_ReadOnly));
    }

    bool result = vsgGIS::all_equal(datasets.begin(), datasets.end(), vsgGIS::compatibleDatasetProjectionsTransformAndSizes);

    if (result)
        std::cout << "datasets are compatible." << std::endl;
    else
        std::cout << "datasets are not compatible." << std::endl;

    return 0;
}
