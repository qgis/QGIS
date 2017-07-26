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

    double zExaggeration;   //!< Multiplier of terrain heights to make the terrain shape more pronounced

    void setLayers( const QList<QgsMapLayer *> &layers );
    QList<QgsMapLayer *> layers() const;

    int tileTextureSize;   //!< Size of map textures of tiles in pixels (width/height)
    int maxTerrainError;   //!< Maximum allowed terrain error in pixels

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
    void showTerrainBoundingBoxesChanged();
    void showTerrainTilesInfoChanged();

  private:
    std::unique_ptr<TerrainGenerator> mTerrainGenerator;  //!< Implementation of the terrain generation
    bool mShowTerrainBoundingBoxes;  //!< Whether to show bounding boxes of entities - useful for debugging
    bool mShowTerrainTileInfo;  //!< Whether to draw extra information about terrain tiles to the textures - useful for debugging
    QList<QgsMapLayerRef> mLayers;   //!< Layers to be rendered
};


#endif // MAP3D_H
