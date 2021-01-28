#pragma once

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

#include <vsgGIS/Export.h>

#include "gdal_priv.h"
#include "ogr_spatialref.h"

#include <memory>

namespace vsgGIS
{

    /// Call GDALOpen(..) to open sepcified file returning a std::shared_ptr<GDALDataset> to reboustly manage the lifetime of the GDALDataSet, automatiically call GDALClose.
    inline std::shared_ptr<GDALDataset> openDataSet(const char* filename, GDALAccess access)
    {
        return std::shared_ptr<GDALDataset>(static_cast<GDALDataset*>(GDALOpen(filename, access)), [](GDALDataset* dataset) { GDALClose(dataset); });
    }

    /// Call GDALOpenShared(..) to open sepcified file returning a std::shared_ptr<GDALDataset> to reboustly manage the lifetime of the GDALDataSet, automatiically call GDALClose.
    inline std::shared_ptr<GDALDataset> openSharedDataSet(const char* filename, GDALAccess access)
    {
        return std::shared_ptr<GDALDataset>(static_cast<GDALDataset*>(GDALOpenShared(filename, access)), [](GDALDataset* dataset) { GDALClose(dataset); });
    }

    /// return true if two GDALDataset has the same projection referecne string/
    extern VSGGIS_DECLSPEC bool compatibleDatasetProjections(const GDALDataset& lhs, const GDALDataset& rhs);

    /// return true if two GDALDataset has the same projection, geo transform and dimensions indicating they are perfectly pixel aliged and matched in size.
    extern VSGGIS_DECLSPEC bool compatibleDatasetProjectionsTransformAndSizes(const GDALDataset& lhs, const GDALDataset& rhs);

    template<class Iterator, class BinaryPredicate>
    bool all_equal(Iterator first, Iterator last, BinaryPredicate compare)
    {
        if (first == last) return true;
        Iterator itr = first;
        ++itr;

        for (; itr != last; ++itr)
        {
            if (!compare(*first, *itr)) return false;
        }

        return true;
    }

    template<class Container, class BinaryPredicate>
    bool all_equal(Container& container, BinaryPredicate compare)
    {
        return all_equal(container.begin(), container.end(), compare);
    }
} // namespace vsgGIS
