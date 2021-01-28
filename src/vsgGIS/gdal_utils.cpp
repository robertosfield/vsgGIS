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

#include <vsgGIS/gdal_utils.h>

#include <cstring>

using namespace vsgGIS;

bool vsgGIS::compatibleDatasetProjections(const GDALDataset& lhs, const GDALDataset& rhs)
{
    if (&lhs == &rhs) return true;

    const auto lhs_projectionRef = const_cast<GDALDataset&>(lhs).GetProjectionRef();
    const auto rhs_projectionRef = const_cast<GDALDataset&>(lhs).GetProjectionRef();

    // if pointers are the same then they are compatible
    if (lhs_projectionRef == rhs_projectionRef) return true;

    // if one of the pointers is NULL then they are incompatible
    if (!lhs_projectionRef || !rhs_projectionRef) return false;

    // check in the OGRSpatialReference are the same
    return (std::strcmp(lhs_projectionRef, rhs_projectionRef) == 0);
}

bool vsgGIS::compatibleDatasetProjectionsTransformAndSizes(const GDALDataset& lhs, const GDALDataset& rhs)
{
    if (!compatibleDatasetProjections(lhs, rhs)) return false;

    auto& non_const_lhs = const_cast<GDALDataset&>(lhs);
    auto& non_const_rhs = const_cast<GDALDataset&>(rhs);

    if (non_const_lhs.GetRasterXSize() != non_const_rhs.GetRasterXSize() || non_const_lhs.GetRasterYSize() != non_const_rhs.GetRasterYSize())
    {
        return false;
    }

    double lhs_GeoTransform[6];
    double rhs_GeoTransform[6];

    int numberWithValidTransforms = 0;
    if (non_const_lhs.GetGeoTransform(lhs_GeoTransform) == CE_None) ++numberWithValidTransforms;
    if (non_const_rhs.GetGeoTransform(rhs_GeoTransform) == CE_None) ++numberWithValidTransforms;

    // if neither have transform mark as compatible
    if (numberWithValidTransforms == 0) return true;

    // only one has a transform so must be incompatible
    if (numberWithValidTransforms == 1) return false;

    for (int i = 0; i < 6; ++i)
    {
        if (lhs_GeoTransform[i] != rhs_GeoTransform[i])
        {
            return false;
        }
    }
    return true;
}
