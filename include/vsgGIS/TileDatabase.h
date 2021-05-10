#pragma once

#include <vsgGIS/Export.h>

#include <vsg/all.h>

namespace vsgGIS
{

extern VSGGIS_DECLSPEC bool init();

class VSGGIS_DECLSPEC TileDatabaseSettings : public vsg::Inherit<vsg::Object, TileDatabaseSettings>
{
public:
    // read/write of TileReader settings
    void read(vsg::Input& input) override;
    void write(vsg::Output& output) const override;

    // defaults for readymap
    vsg::dbox extents = {{-180.0, -90.0, 0.0}, {180.0, 90.0, 1.0}};
    uint32_t noX = 2;
    uint32_t noY = 1;
    uint32_t maxLevel = 22;
    bool originTopLeft = true;
    double lodTransitionScreenHeightRatio = 0.25;

    std::string projection;
    vsg::ref_ptr<vsg::EllipsoidModel> ellipsoidModel = vsg::EllipsoidModel::create();

    vsg::Path imageLayer;
    vsg::Path terrainLayer;
    uint32_t mipmapLevelsHint = 16;
};

class VSGGIS_DECLSPEC TileDatabase : public vsg::Inherit<vsg::Node, TileDatabase>
{
public:
    vsg::ref_ptr<TileDatabaseSettings> settings;
    vsg::ref_ptr<vsg::Node> child;

    template<class N, class V>
    static void t_traverse(N& node, V& visitor)
    {
        if (node.child) node.child->accept(visitor);
    }

    void traverse(vsg::Visitor& visitor) override { t_traverse(*this, visitor); }
    void traverse(vsg::ConstVisitor& visitor) const override { t_traverse(*this, visitor); }
    void traverse(vsg::RecordTraversal& visitor) const override { t_traverse(*this, visitor); }

    // read/write of TileReader settings
    void read(vsg::Input& input) override;
    void write(vsg::Output& output) const override;

    bool readDatabase(vsg::ref_ptr<const vsg::Options> options);
};

class VSGGIS_DECLSPEC TileReader : public vsg::Inherit<vsg::ReaderWriter, TileReader>
{
public:

    vsg::ref_ptr<TileDatabaseSettings> settings;

    // read/write of TileReader settings
    void read(vsg::Input& input) override;
    void write(vsg::Output& output) const override;

    // initialize data structures
    void init();

    // read the tile
    vsg::ref_ptr<vsg::Object> read(const vsg::Path& filename, vsg::ref_ptr<const vsg::Options> options = {}) const override;

    // timing stats
    mutable std::mutex statsMutex;
    mutable uint64_t numTilesRead{0};
    mutable double totalTimeReadingTiles{0.0};

protected:
    vsg::dvec3 computeLatitudeLongitudeAltitude(const vsg::dvec3& src) const;
    vsg::dbox computeTileExtents(uint32_t x, uint32_t y, uint32_t level) const;
    vsg::Path getTilePath(const vsg::Path& src, uint32_t x, uint32_t y, uint32_t level) const;

    vsg::ref_ptr<vsg::Object> read_root(vsg::ref_ptr<const vsg::Options> options = {}) const;
    vsg::ref_ptr<vsg::Object> read_subtile(uint32_t x, uint32_t y, uint32_t lod, vsg::ref_ptr<const vsg::Options> options = {}) const;

    vsg::ref_ptr<vsg::Node> createTile(const vsg::dbox& tile_extents, vsg::ref_ptr<vsg::Data> sourceData) const;
    vsg::ref_ptr<vsg::Node> createECEFTile(const vsg::dbox& tile_extents, vsg::ref_ptr<vsg::Data> sourceData) const;
    vsg::ref_ptr<vsg::Node> createTextureQuad(const vsg::dbox& tile_extents, vsg::ref_ptr<vsg::Data> sourceData) const;

    vsg::ref_ptr<vsg::StateGroup> createRoot() const;

    vsg::ref_ptr<vsg::DescriptorSetLayout> descriptorSetLayout;
    vsg::ref_ptr<vsg::PipelineLayout> pipelineLayout;
    vsg::ref_ptr<vsg::Sampler> sampler;
};

}

// Provide the means for the vsg::type_name<class> to get the human readable class name.
EVSG_type_name(vsgGIS::TileDatabaseSettings);
EVSG_type_name(vsgGIS::TileDatabase);
EVSG_type_name(vsgGIS::TileReader);
