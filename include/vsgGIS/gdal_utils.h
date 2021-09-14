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

#include <vsg/core/Data.h>
#include <vsg/maths/vec4.h>

#include <memory>
#include <set>

namespace vsgGIS
{
    /// call GDALAllRegister() etc. if it hasn't already been called. Return true if this call to initGDAL() invoked GDAL setup, or false if it has previously been done.
    extern VSGGIS_DECLSPEC bool initGDAL();

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

    /// create a vsg::Image2D of the approrpiate type that maps to specified dimensions and GDALDataType
    extern VSGGIS_DECLSPEC vsg::ref_ptr<vsg::Data> createImage2D(int width, int height, int numComponents, GDALDataType dataType, vsg::dvec4 def = {0.0, 0.0, 0.0, 1.0});

    /// copy a RasterBand onto a target RGBA component of a vsg::Data.  Dimensions and datatypes must be compatble between RasterBand and vsg::Data. Return true on success, false on failure to copy.
    extern VSGGIS_DECLSPEC bool copyRasterBandToImage(GDALRasterBand& band, vsg::Data& image, int component);

    /// assign GDAL MetaData mapping the "key=value" entries to vsg::Object as setValue(key, std::string(value)).
    extern VSGGIS_DECLSPEC bool assignMetaData(GDALDataset& dataset, vsg::Object& object);

    /// call binary comparison opeators on dereferenced items in specified range.
    template<class Iterator, class BinaryPredicate>
    bool all_equal(Iterator first, Iterator last, BinaryPredicate compare)
    {
        if (first == last) return true;
        Iterator itr = first;
        ++itr;

        for (; itr != last; ++itr)
        {
            if (!compare(**first, **itr)) return false;
        }

        return true;
    }

    /// collect the set of GDALDataType of all the RansterBand is the specified GDALDataset
    inline std::set<GDALDataType> dataTypes(GDALDataset& dataset)
    {
        std::set<GDALDataType> types;
        for (int i = 1; i <= dataset.GetRasterCount(); ++i)
        {
            GDALRasterBand* band = dataset.GetRasterBand(i);
            types.insert(band->GetRasterDataType());
        }

        return types;
    }

    /// collect the set of GDALDataType of all the RansterBand is the specified range of GDALDataset
    template<class Iterator>
    std::set<GDALDataType> dataTypes(Iterator first, Iterator last)
    {
        std::set<GDALDataType> types;
        for (Iterator itr = first; itr != last; ++itr)
        {
            GDALDataset& dataset = **itr;
            for (int i = 1; i <= dataset.GetRasterCount(); ++i)
            {
                GDALRasterBand* band = dataset.GetRasterBand(i);
                types.insert(band->GetRasterDataType());
            }
        }
        return types;
    }

} // namespace vsgGIS
