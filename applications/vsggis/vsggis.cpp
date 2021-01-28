#include <vsg/all.h>

#include <chrono>
#include <iostream>
#include <ostream>
#include <thread>

#include <vsgGIS/gdal_utils.h>

int main(int argc, char** argv)
{
    vsg::CommandLine arguments(&argc, argv);

    std::vector<std::shared_ptr<GDALDataset>> datasets;

    for (int ai = 1; ai < argc; ++ai)
    {
        std::string filename = arguments[ai];

        datasets.push_back(vsgGIS::openSharedDataSet(filename.c_str(), GA_ReadOnly));
    }

    bool result = vsgGIS::all_equal(datasets, [](const std::shared_ptr<GDALDataset>& lhs, const std::shared_ptr<GDALDataset>& rhs) {
        return vsgGIS::compatibleDatasetProjectionsTransformAndSizes(*lhs, *rhs);
    });

    if (result)
        std::cout << "datasets are compatible." << result << std::endl;
    else
        std::cout << "datasets are not compatible." << result << std::endl;

    return 0;
}
