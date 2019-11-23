/***************************************************************************
  qgs3dmapsettings.h
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS3DMAPSETTINGS_H
#define QGS3DMAPSETTINGS_H

#include "qgis_3d.h"

#include <memory>
#include <QColor>
#include <QMatrix4x4>

#include "qgscoordinatereferencesystem.h"
#include "qgsmaplayerref.h"
#include "qgsphongmaterialsettings.h"
#include "qgspointlightsettings.h"
#include "qgsterraingenerator.h"
#include "qgsvector3d.h"

class QgsMapLayer;
class QgsRasterLayer;

class QgsAbstract3DRenderer;


class QgsReadWriteContext;
class QgsProject;

class QDomElement;


/**
 * \ingroup 3d
 * Definition of the world
 *
 * \since QGIS 3.0
 */
class _3D_EXPORT Qgs3DMapSettings : public QObject
{
    Q_OBJECT
  public:

    //! Constructor for Qgs3DMapSettings
    Qgs3DMapSettings() = default;
    //! Copy constructor
    Qgs3DMapSettings( const Qgs3DMapSettings &other );
    ~Qgs3DMapSettings() override;

    //! Reads configuration from a DOM element previously written by writeXml()
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context );
    //! Writes configuration to a DOM element, to be used later with readXml()
    QDomElement writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const;
    //! Resolves references to other objects (map layers) after the call to readXml()
    void resolveReferences( const QgsProject &project );

    /**
     * Sets coordinates in map CRS at which our 3D world has origin (0,0,0)
     *
     * We move the 3D world origin to the center of the extent of our terrain: this is done
     * to minimize the impact of numerical errors when operating with 32-bit floats.
     * Unfortunately this is not enough when working with a large area (still results in jitter
     * with scenes spanning hundreds of kilometers and zooming in a lot).
     *
     * Need to look into more advanced techniques like "relative to center" or "relative to eye"
     * to improve the precision.
     */
    void setOrigin( const QgsVector3D &origin ) { mOrigin = origin; }
    //! Returns coordinates in map CRS at which 3D scene has origin (0,0,0)
    QgsVector3D origin() const { return mOrigin; }

    //! Converts map coordinates to 3D world coordinates (applies offset and turns (x,y,z) into (x,-z,y))
    QgsVector3D mapToWorldCoordinates( const QgsVector3D &mapCoords ) const;
    //! Converts 3D world coordinates to map coordinates (applies offset and turns (x,y,z) into (x,-z,y))
    QgsVector3D worldToMapCoordinates( const QgsVector3D &worldCoords ) const;

    //! Sets coordinate reference system used in the 3D scene
    void setCrs( const QgsCoordinateReferenceSystem &crs );
    //! Returns coordinate reference system used in the 3D scene
    QgsCoordinateReferenceSystem crs() const { return mCrs; }

    /**
     * Returns the coordinate transform context, which stores various
     * information regarding which datum transforms should be used when transforming points
     * from a source to destination coordinate reference system.
     *
     * \see setTransformContext()
     */
    QgsCoordinateTransformContext transformContext() const;

    /**
     * Sets the coordinate transform \a context, which stores various
     * information regarding which datum transforms should be used when transforming points
     * from a source to destination coordinate reference system.
     *
     * \see transformContext()
     */
    void setTransformContext( const QgsCoordinateTransformContext &context );

    /**
     * Returns the path resolver for conversion between relative and absolute paths
     * during rendering operations, e.g. for resolving relative symbol paths.
     *
     * \see setPathResolver()
     * \since QGIS 3.0
     */
    const QgsPathResolver &pathResolver() const { return mPathResolver; }

    /**
     * Sets the path \a resolver for conversion between relative and absolute paths
     * during rendering operations, e.g. for resolving relative symbol paths.
     *
     * \see pathResolver()
     * \since QGIS 3.0
     */
    void setPathResolver( const QgsPathResolver &resolver ) { mPathResolver = resolver; }

    /**
     * Returns pointer to the collection of map themes. Normally this would be QgsProject::mapThemeCollection()
     * of the currently used project. Without a valid map theme collection object it is not possible
     * to resolve map themes from their names.
     * \since QGIS 3.6
     */
    QgsMapThemeCollection *mapThemeCollection() const { return mMapThemes; }

    /**
     * Sets pointer to the collection of map themes.
     * \see mapThemeCollection()
     * \since QGIS 3.6
     */
    void setMapThemeCollection( QgsMapThemeCollection *mapThemes ) { mMapThemes = mapThemes; }

    //! Sets background color of the 3D map view
    void setBackgroundColor( const QColor &color );
    //! Returns background color of the 3D map view
    QColor backgroundColor() const;

    //! Sets color used for selected features
    void setSelectionColor( const QColor &color );
    //! Returns color used for selected features
    QColor selectionColor() const;

    //
    // terrain related config
    //

    /**
     * Sets vertical scale (exaggeration) of terrain
     * (1 = true scale, > 1 = hills get more pronounced)
     */
    void setTerrainVerticalScale( double zScale );
    //! Returns vertical scale (exaggeration) of terrain
    double terrainVerticalScale() const;

    /**
     * Sets the list of map layers to be rendered as a texture of the terrain
     * \note If terrain map theme is set, it has a priority over the list of layers specified here.
     */
    void setLayers( const QList<QgsMapLayer *> &layers );

    /**
     * Returns the list of map layers to be rendered as a texture of the terrain
     * \note If terrain map theme is set, it has a priority over the list of layers specified here.
     */
    QList<QgsMapLayer *> layers() const;

    /**
     * Sets resolution (in pixels) of the texture of a terrain tile
     * \see mapTileResolution()
     */
    void setMapTileResolution( int res );

    /**
     * Returns resolution (in pixels) of the texture of a terrain tile. This parameter influences
     * how many zoom levels for terrain tiles there will be (together with maxTerrainGroundError())
     */
    int mapTileResolution() const;

    /**
     * Sets maximum allowed screen error of terrain tiles in pixels.
     * \see maxTerrainScreenError()
     */
    void setMaxTerrainScreenError( float error );

    /**
     * Returns maximum allowed screen error of terrain tiles in pixels. This parameter decides
     * how aggressively less detailed terrain tiles are swapped to more detailed ones as camera gets closer.
     * Each tile has its error defined in world units - this error gets projected to screen pixels
     * according to camera view and if the tile's error is greater than the allowed error, it will
     * be swapped by more detailed tiles with lower error.
     */
    float maxTerrainScreenError() const;

    /**
     * Returns maximum ground error of terrain tiles in world units.
     * \see maxTerrainGroundError()
     */
    void setMaxTerrainGroundError( float error );

    /**
     * Returns maximum ground error of terrain tiles in world units. This parameter influences
     * how many zoom levels there will be (together with mapTileResolution()).
     * This value tells that when the given ground error is reached (e.g. 10 meters), it makes no sense
     * to further split terrain tiles into finer ones because they will not add extra details anymore.
     */
    float maxTerrainGroundError() const;

    /**
     * Sets terrain generator. It takes care of producing terrain tiles from the input data.
     * Takes ownership of the generator
     */
    void setTerrainGenerator( QgsTerrainGenerator *gen SIP_TRANSFER );
    //! Returns terrain generator. It takes care of producing terrain tiles from the input data.
    QgsTerrainGenerator *terrainGenerator() const { return mTerrainGenerator.get(); }

    /**
     * Sets whether terrain shading is enabled.
     * \see isTerrainShadingEnabled()
     * \since QGIS 3.6
     */
    void setTerrainShadingEnabled( bool enabled );

    /**
     * Returns whether terrain shading is enabled. When enabled, in addition to the terrain texture
     * generated from the map, the terrain rendering will take into account position of the lights,
     * terrain normals and terrain shading material (ambient and specular colors, shininess).
     * \since QGIS 3.6
     */
    bool isTerrainShadingEnabled() const { return mTerrainShadingEnabled; }

    /**
     * Sets terrain shading material.
     * \see terrainShadingMaterial()
     * \since QGIS 3.6
     */
    void setTerrainShadingMaterial( const QgsPhongMaterialSettings &material );

    /**
     * Returns terrain shading material. Diffuse color component is ignored since the diffuse component
     * is provided by 2D rendered map texture. Only used when isTerrainShadingEnabled() is TRUE.
     * \since QGIS 3.6
     */
    QgsPhongMaterialSettings terrainShadingMaterial() const { return mTerrainShadingMaterial; }

    /**
     * Sets name of the map theme.
     * \see terrainMapTheme()
     * \since QGIS 3.6
     */
    void setTerrainMapTheme( const QString &theme );

    /**
     * Returns name of the map theme (from the active project) that will be used for terrain's texture.
     * Empty map theme name means that the map theme is not overridden and the current map theme will be used.
     * \note Support for map themes only works if mapThemeCollection() is a valid object (otherwise it is not possible to resolve map themes from names)
     * \since QGIS 3.6
     */
    QString terrainMapTheme() const { return mTerrainMapTheme; }

    //
    // misc configuration
    //

    //! Sets list of extra 3D renderers to use in the scene. Takes ownership of the objects.
    void setRenderers( const QList<QgsAbstract3DRenderer *> &renderers SIP_TRANSFER );
    //! Returns list of extra 3D renderers
    QList<QgsAbstract3DRenderer *> renderers() const { return mRenderers; }

    /**
     * Sets skybox configuration. When enabled, map scene will try to load six texture files
     * using the following syntax of filenames: "[base]_[side][extension]" where [side] is one
     * of the following: posx/posy/posz/negx/negy/negz and [base] and [extension] are the arguments
     * passed this method.
     */
    void setSkybox( bool enabled, const QString &fileBase = QString(), const QString &fileExtension = QString() );
    //! Returns whether skybox is enabled
    bool hasSkyboxEnabled() const { return mSkyboxEnabled; }
    //! Returns base part of filenames of skybox (see setSkybox())
    QString skyboxFileBase() const { return mSkyboxFileBase; }
    //! Returns extension part of filenames of skybox (see setSkybox())
    QString skyboxFileExtension() const { return mSkyboxFileExtension; }

    //! Sets whether to display bounding boxes of terrain tiles (for debugging)
    void setShowTerrainBoundingBoxes( bool enabled );
    //! Returns whether to display bounding boxes of terrain tiles (for debugging)
    bool showTerrainBoundingBoxes() const { return mShowTerrainBoundingBoxes; }
    //! Sets whether to display extra tile info on top of terrain tiles (for debugging)
    void setShowTerrainTilesInfo( bool enabled );
    //! Returns whether to display extra tile info on top of terrain tiles (for debugging)
    bool showTerrainTilesInfo() const { return mShowTerrainTileInfo; }

    /**
     * Sets whether to show camera's view center as a sphere (for debugging)
     * \since QGIS 3.4
     */
    void setShowCameraViewCenter( bool enabled );

    /**
     * Returns whether to show camera's view center as a sphere (for debugging)
     * \since QGIS 3.4
     */
    bool showCameraViewCenter() const { return mShowCameraViewCenter; }
    //! Sets whether to display labels on terrain tiles
    void setShowLabels( bool enabled );
    //! Returns whether to display labels on terrain tiles
    bool showLabels() const { return mShowLabels; }

    /**
     * Returns list of point lights defined in the scene
     * \since QGIS 3.6
     */
    QList<QgsPointLightSettings> pointLights() const { return mPointLights; }

    /**
     * Sets list of point lights defined in the scene
     * \since QGIS 3.6
     */
    void setPointLights( const QList<QgsPointLightSettings> &pointLights );

    /**
     * Returns the camera lens' field of view
     * \since QGIS 3.8
     */
    float fieldOfView() const { return mFieldOfView; }

    /**
     * Sets the camera lens' field of view
     * \since QGIS 3.8
     */
    void setFieldOfView( const float fieldOfView );

    /**
     * Sets DPI used for conversion between real world units (e.g. mm) and pixels
     * \param dpi the number of dot per inch
     * \since QGIS 3.10
     */
    void setOutputDpi( const double dpi ) {mDpi = dpi;}


    /**
     * Returns DPI used for conversion between real world units (e.g. mm) and pixels
     * Default value is 96
     * \since QGIS 3.10
     */
    double outputDpi() const { return mDpi; }

  signals:
    //! Emitted when the background color has changed
    void backgroundColorChanged();
    //! Emitted when the selection color has changed
    void selectionColorChanged();
    //! Emitted when the list of map layers for terrain texture has changed
    void layersChanged();
    //! Emitted when the terrain generator has changed
    void terrainGeneratorChanged();
    //! Emitted when the vertical scale of the terrain has changed
    void terrainVerticalScaleChanged();
    //! Emitted when the map tile resoulution has changed
    void mapTileResolutionChanged();
    //! Emitted when the maximum terrain screen error has changed
    void maxTerrainScreenErrorChanged();
    //! Emitted when the maximum terrain ground error has changed
    void maxTerrainGroundErrorChanged();

    /**
     * Emitted when terrain shading enabled flag or terrain shading material has changed
     * \since QGIS 3.6
     */
    void terrainShadingChanged();

    /**
     * Emitted when terrain's map theme has changed
     * \since QGIS 3.6
     */
    void terrainMapThemeChanged();

    /**
     * Emitted when the list of map's extra renderers have been modified
     * \since QGIS 3.10
     */
    void renderersChanged();

    //! Emitted when the flag whether terrain's bounding boxes are shown has changed
    void showTerrainBoundingBoxesChanged();
    //! Emitted when the flag whether terrain's tile info is shown has changed
    void showTerrainTilesInfoChanged();

    /**
     * Emitted when the flag whether camera's view center is shown has changed
     * \since QGIS 3.4
     */
    void showCameraViewCenterChanged();
    //! Emitted when the flag whether labels are displayed on terrain tiles has changed
    void showLabelsChanged();

    /**
     * Emitted when the list of point lights changes
     * \since QGIS 3.6
     */
    void pointLightsChanged();

    /**
     * Emitted when the camera lens field of view changes
     * \since QGIS 3.8
     */
    void fieldOfViewChanged();

  private:
    //! Offset in map CRS coordinates at which our 3D world has origin (0,0,0)
    QgsVector3D mOrigin;
    QgsCoordinateReferenceSystem mCrs;   //!< Destination coordinate system of the world
    QColor mBackgroundColor = Qt::black;   //!< Background color of the scene
    QColor mSelectionColor; //!< Color to be used for selected map features
    double mTerrainVerticalScale = 1;   //!< Multiplier of terrain heights to make the terrain shape more pronounced
    std::unique_ptr<QgsTerrainGenerator> mTerrainGenerator;  //!< Implementation of the terrain generation
    int mMapTileResolution = 512;   //!< Size of map textures of tiles in pixels (width/height)
    float mMaxTerrainScreenError = 3.f;   //!< Maximum allowed terrain error in pixels (determines when tiles are switched to more detailed ones)
    float mMaxTerrainGroundError = 1.f;  //!< Maximum allowed horizontal map error in map units (determines how many zoom levels will be used)
    bool mTerrainShadingEnabled = false;   //!< Whether terrain should be shaded taking lights into account
    QgsPhongMaterialSettings mTerrainShadingMaterial;  //!< Material to use for the terrain (if shading is enabled). Diffuse color is ignored.
    QString mTerrainMapTheme;  //!< Name of map theme used for terrain's texture (empty means use the current map theme)
    bool mShowTerrainBoundingBoxes = false;  //!< Whether to show bounding boxes of entities - useful for debugging
    bool mShowTerrainTileInfo = false;  //!< Whether to draw extra information about terrain tiles to the textures - useful for debugging
    bool mShowCameraViewCenter = false;  //!< Whether to show camera view center as a sphere - useful for debugging
    bool mShowLabels = false; //!< Whether to display labels on terrain tiles
    QList<QgsPointLightSettings> mPointLights;  //!< List of lights defined for the scene
    float mFieldOfView = 45.0f; //<! Camera lens field of view value
    QList<QgsMapLayerRef> mLayers;   //!< Layers to be rendered
    QList<QgsAbstract3DRenderer *> mRenderers;  //!< Extra stuff to render as 3D object
    bool mSkyboxEnabled = false;  //!< Whether to render skybox
    QString mSkyboxFileBase; //!< Base part of the files with skybox textures
    QString mSkyboxFileExtension; //!< Extension part of the files with skybox textures
    //! Coordinate transform context
    QgsCoordinateTransformContext mTransformContext;
    QgsPathResolver mPathResolver;
    QgsMapThemeCollection *mMapThemes = nullptr;   //!< Pointer to map themes (e.g. from the current project) to resolve map theme content from the name
    double mDpi = 96;  //!< Dot per inch value for the screen / painter
};


#endif // QGS3DMAPSETTINGS_H
