#ifndef MAP3D_H
#define MAP3D_H

#include "qgis_3d.h"

#include <memory>
#include <QColor>
#include <QMatrix4x4>

#include "qgscoordinatereferencesystem.h"
#include "qgsmaplayerref.h"

class QgsMapLayer;
class QgsRasterLayer;

class QgsAbstract3DRenderer;
class TerrainGenerator;


class QgsReadWriteContext;
class QgsProject;

class QDomElement;

//! Definition of the world
class _3D_EXPORT Map3D : public QObject
{
    Q_OBJECT
  public:
    Map3D();
    Map3D( const Map3D &other );
    ~Map3D();

    void readXml( const QDomElement &elem, const QgsReadWriteContext &context );

    QDomElement writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const;

    void resolveReferences( const QgsProject &project );

    double originX, originY, originZ;   //!< Coordinates in map CRS at which our 3D world has origin (0,0,0)
    QgsCoordinateReferenceSystem crs;   //!< Destination coordinate system of the world  (TODO: not needed? can be
    QColor backgroundColor;   //!< Background color of the scene

    //
    // terrain related config
    //

    void setTerrainVerticalScale( double zScale );
    double terrainVerticalScale() const;

    void setLayers( const QList<QgsMapLayer *> &layers );
    QList<QgsMapLayer *> layers() const;

    void setMapTileResolution( int res );
    int mapTileResolution() const;

    void setMaxTerrainScreenError( float error );
    float maxTerrainScreenError() const;

    void setMaxTerrainGroundError( float error );
    float maxTerrainGroundError() const;

    //! Takes ownership of the generator
    void setTerrainGenerator( TerrainGenerator *gen );
    TerrainGenerator *terrainGenerator() const { return mTerrainGenerator.get(); }

    //
    // 3D renderers
    //

    QList<QgsAbstract3DRenderer *> renderers;  //!< Stuff to render as 3D object

    bool skybox;  //!< Whether to render skybox
    QString skyboxFileBase;
    QString skyboxFileExtension;

    void setShowTerrainBoundingBoxes( bool enabled );
    bool showTerrainBoundingBoxes() const { return mShowTerrainBoundingBoxes; }
    void setShowTerrainTilesInfo( bool enabled );
    bool showTerrainTilesInfo() const { return mShowTerrainTileInfo; }

  signals:
    void layersChanged();
    void terrainGeneratorChanged();
    void terrainVerticalScaleChanged();
    void mapTileResolutionChanged();
    void maxTerrainScreenErrorChanged();
    void maxTerrainGroundErrorChanged();
    void showTerrainBoundingBoxesChanged();
    void showTerrainTilesInfoChanged();

  private:
    double mTerrainVerticalScale;   //!< Multiplier of terrain heights to make the terrain shape more pronounced
    std::unique_ptr<TerrainGenerator> mTerrainGenerator;  //!< Implementation of the terrain generation
    int mMapTileResolution;   //!< Size of map textures of tiles in pixels (width/height)
    float mMaxTerrainScreenError;   //!< Maximum allowed terrain error in pixels (determines when tiles are switched to more detailed ones)
    float mMaxTerrainGroundError;  //!< Maximum allowed horizontal map error in map units (determines how many zoom levels will be used)
    bool mShowTerrainBoundingBoxes;  //!< Whether to show bounding boxes of entities - useful for debugging
    bool mShowTerrainTileInfo;  //!< Whether to draw extra information about terrain tiles to the textures - useful for debugging
    QList<QgsMapLayerRef> mLayers;   //!< Layers to be rendered
};


#endif // MAP3D_H
