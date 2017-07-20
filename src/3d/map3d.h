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
class QgsVectorLayer;

class TerrainGenerator;


//! how to handle altitude of vector features
enum AltitudeClamping
{
  AltClampAbsolute,   //!< Z_final = z_geometry
  AltClampRelative,   //!< Z_final = z_terrain + z_geometry
  AltClampTerrain,    //!< Z_final = z_terrain
};

QString altClampingToString( AltitudeClamping altClamp );
AltitudeClamping altClampingFromString( const QString &str );

//! how to handle clamping of vertices of individual features
enum AltitudeBinding
{
  AltBindVertex,      //!< Clamp every vertex of feature
  AltBindCentroid,    //!< Clamp just centroid of feature
};

QString altBindingToString( AltitudeBinding altBind );
AltitudeBinding altBindingFromString( const QString &str );


//! Basic shading material used for rendering
class _3D_EXPORT PhongMaterialSettings
{
  public:
    PhongMaterialSettings()
      : mAmbient( QColor::fromRgbF( 0.05f, 0.05f, 0.05f, 1.0f ) )
      , mDiffuse( QColor::fromRgbF( 0.7f, 0.7f, 0.7f, 1.0f ) )
      , mSpecular( QColor::fromRgbF( 0.01f, 0.01f, 0.01f, 1.0f ) )
      , mShininess( 150.0f )
    {
    }

    QColor ambient() const { return mAmbient; }
    QColor diffuse() const { return mDiffuse; }
    QColor specular() const { return mSpecular; }
    float shininess() const { return mShininess; }

    void setAmbient( const QColor &ambient ) { mAmbient = ambient; }
    void setDiffuse( const QColor &diffuse ) { mDiffuse = diffuse; }
    void setSpecular( const QColor &specular ) { mSpecular = specular; }
    void setShininess( float shininess ) { mShininess = shininess; }

    void readXml( const QDomElement &elem );
    void writeXml( QDomElement &elem ) const;

  private:
    QColor mAmbient;
    QColor mDiffuse;
    QColor mSpecular;
    float mShininess;
};


class _3D_EXPORT PolygonRenderer
{
  public:
    PolygonRenderer();

    void setLayer( QgsVectorLayer *layer );
    QgsVectorLayer *layer() const;

    void writeXml( QDomElement &elem ) const;
    void readXml( const QDomElement &elem );
    void resolveReferences( const QgsProject &project );

    AltitudeClamping altClamping;  //! how to handle altitude of vector features
    AltitudeBinding altBinding;    //! how to handle clamping of vertices of individual features

    float height;           //!< Base height of polygons
    float extrusionHeight;  //!< How much to extrude (0 means no walls)
    PhongMaterialSettings material;  //!< Defines appearance of objects

  private:
    QgsMapLayerRef layerRef; //!< Layer used to extract polygons from
};

class _3D_EXPORT PointRenderer
{
  public:
    PointRenderer();

    void setLayer( QgsVectorLayer *layer );
    QgsVectorLayer *layer() const;

    void writeXml( QDomElement &elem ) const;
    void readXml( const QDomElement &elem );
    void resolveReferences( const QgsProject &project );

    float height;
    PhongMaterialSettings material;  //!< Defines appearance of objects
    QVariantMap shapeProperties;  //!< What kind of shape to use and what
    QMatrix4x4 transform;  //!< Transform of individual instanced models

  private:
    QgsMapLayerRef layerRef; //!< Layer used to extract points from
};

class _3D_EXPORT LineRenderer
{
  public:
    LineRenderer();

    void setLayer( QgsVectorLayer *layer );
    QgsVectorLayer *layer() const;

    void writeXml( QDomElement &elem ) const;
    void readXml( const QDomElement &elem );
    void resolveReferences( const QgsProject &project );

    AltitudeClamping altClamping;  //! how to handle altitude of vector features
    AltitudeBinding altBinding;    //! how to handle clamping of vertices of individual features

    float height;           //!< Base height of polygons
    float extrusionHeight;  //!< How much to extrude (0 means no walls)
    PhongMaterialSettings material;  //!< Defines appearance of objects

    float distance;  //!< Distance of buffer of lines

  private:
    QgsMapLayerRef layerRef; //!< Layer used to extract points from
};

class QgsReadWriteContext;
class QgsProject;

class QDomElement;

//! Definition of the world
class _3D_EXPORT Map3D
{
  public:
    Map3D();

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
    std::unique_ptr<TerrainGenerator> terrainGenerator;  //!< Implementation of the terrain generation

    //
    // 3D renderers
    //

    QList<PolygonRenderer> polygonRenderers;   //!< Stuff to render as polygons
    QList<PointRenderer> pointRenderers;   //!< Stuff to render as points
    QList<LineRenderer> lineRenderers;  //!< Stuff to render as lines

    bool skybox;  //!< Whether to render skybox
    QString skyboxFileBase;
    QString skyboxFileExtension;

    bool showBoundingBoxes;  //!< Whether to show bounding boxes of entities - useful for debugging
    bool drawTerrainTileInfo;  //!< Whether to draw extra information about terrain tiles to the textures - useful for debugging

  private:
    QList<QgsMapLayerRef> mLayers;   //!< Layers to be rendered
};


#endif // MAP3D_H
