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
#include <Qt3DRender/QCamera>

#include "qgscoordinatereferencesystem.h"
#include "qgsmaplayerref.h"
#include "qgsphongmaterialsettings.h"
#include "qgsterraingenerator.h"
#include "qgsvector3d.h"
#include "qgs3daxissettings.h"
#include "qgsskyboxsettings.h"
#include "qgsshadowsettings.h"
#include "qgstemporalrangeobject.h"
#include "qgsambientocclusionsettings.h"
#include "qgsabstractterrainsettings.h"

class QgsMapLayer;
class QgsRasterLayer;
class QgsLightSource;
class QgsAbstract3DRenderer;
class QgsReadWriteContext;
class QgsProject;

class QDomElement;

/**
 * \ingroup 3d
 * \brief Definition of the world.
 *
 * \warning Qgs3DMapSettings are a QObject subclass, and accordingly are not
 * safe for access across different threads. See Qgs3DRenderContext instead
 * for a safe snapshot of settings from Qgs3DMapSettings.
 */
class _3D_EXPORT Qgs3DMapSettings : public QObject, public QgsTemporalRangeObject
{
    Q_OBJECT
  public:
    Qgs3DMapSettings();
    Qgs3DMapSettings( const Qgs3DMapSettings &other );
    ~Qgs3DMapSettings() override;

    Qgs3DMapSettings &operator=( Qgs3DMapSettings const & ) = delete;

    //! Reads configuration from a DOM element previously written by writeXml()
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context );
    //! Writes configuration to a DOM element, to be used later with readXml()
    QDomElement writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const;
    //! Resolves references to other objects (map layers) after the call to readXml()
    void resolveReferences( const QgsProject &project );

    /**
     * Returns the 3D scene's 2D extent in the 3D scene's CRS
     *
     * \see crs()
     * \since QGIS 3.30
     */
    QgsRectangle extent() const;

    /**
     * Sets the 3D scene's 2D \a extent in the 3D scene's CRS, while also setting the scene's origin to the extent's center
     * This needs to be called during initialization, as terrain will only be generated
     * within this extent and layer 3D data will only be loaded within this extent too.
     *
     * \see setOrigin()
     * \see setCrs()
     * \since QGIS 3.30
     */
    void setExtent( const QgsRectangle &extent );

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
     *
     * \see origin()
     */
    void setOrigin( const QgsVector3D &origin );

    /**
     * Returns coordinates in map CRS at which 3D scene has origin (0,0,0).
     *
     * \see setOrigin()
     */
    QgsVector3D origin() const;

    //! Converts map coordinates to 3D world coordinates (applies offset and turns (x,y,z) into (x,-z,y))
    QgsVector3D mapToWorldCoordinates( const QgsVector3D &mapCoords ) const;
    //! Converts 3D world coordinates to map coordinates (applies offset and turns (x,y,z) into (x,-z,y))
    QgsVector3D worldToMapCoordinates( const QgsVector3D &worldCoords ) const;

    /**
     * Sets coordinate reference system used in the 3D scene.
     *
     * \see crs()
     */
    void setCrs( const QgsCoordinateReferenceSystem &crs );

    /**
     * Returns coordinate reference system used in the 3D scene.
     *
     * \see setCrs()
     */
    QgsCoordinateReferenceSystem crs() const;

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
     */
    const QgsPathResolver &pathResolver() const;

    /**
     * Sets the path \a resolver for conversion between relative and absolute paths
     * during rendering operations, e.g. for resolving relative symbol paths.
     *
     * \see pathResolver()
     */
    void setPathResolver( const QgsPathResolver &resolver );

    /**
     * Returns pointer to the collection of map themes. Normally this would be QgsProject::mapThemeCollection()
     * of the currently used project. Without a valid map theme collection object it is not possible
     * to resolve map themes from their names.
     * \since QGIS 3.6
     */
    QgsMapThemeCollection *mapThemeCollection() const;

    /**
     * Sets pointer to the collection of map themes.
     * \see mapThemeCollection()
     * \since QGIS 3.6
     */
    void setMapThemeCollection( QgsMapThemeCollection *mapThemes );

    //! Sets background color of the 3D map view
    void setBackgroundColor( const QColor &color );
    //! Returns background color of the 3D map view
    QColor backgroundColor() const;

    //! Sets color used for selected features
    void setSelectionColor( const QColor &color );
    //! Returns color used for selected features
    QColor selectionColor() const;

    /**
     * Sets the list of 3D map \a layers to be rendered in the scene.
     *
     * This setting dictates which layers are to be rendered using their 3D rendering configuration, if available.
     *
     * \see layers()
     * \see layersChanged()
     */
    void setLayers( const QList<QgsMapLayer *> &layers );

    /**
     * Returns the list of 3D map layers to be rendered in the scene.
     *
     * This setting dictates which layers are to be rendered using their 3D rendering configuration, if available.
     *
     * \see setLayers()
     * \see layersChanged()
     */
    QList<QgsMapLayer *> layers() const;

    //
    // terrain related config
    //

    /**
     * Configures the map's terrain settings directly from a project's elevation \a properties.
     *
     * \since QGIS 3.26
     */
    void configureTerrainFromProject( QgsProjectElevationProperties *properties, const QgsRectangle &fullExtent ) SIP_SKIP;

    /**
     * Returns the terrain settings.
     *
     * \warning Modifications should never be made to the returned object. Instead use setTerrainSettings(), so that
     * the corresponding changed signals are correctly emitted.
     *
     * \see setTerrainSettings()
     * \since QGIS 3.42
     */
    const QgsAbstractTerrainSettings *terrainSettings() const;

    /**
     * Sets the terrain settings.
     *
     * \see terrainSettings()
     * \since QGIS 3.42
     */
    void setTerrainSettings( QgsAbstractTerrainSettings *settings SIP_TRANSFER );

    /**
     * Sets vertical scale (exaggeration) of terrain
     * (1 = true scale, > 1 = hills get more pronounced)
     *
     * \deprecated QGIS 3.42. Use terrainSettings() instead.
     */
    Q_DECL_DEPRECATED void setTerrainVerticalScale( double zScale ) SIP_DEPRECATED;

    /**
     * Returns vertical scale (exaggeration) of terrain
     *
     * \deprecated QGIS 3.42. Use terrainSettings() instead.
     */
    Q_DECL_DEPRECATED double terrainVerticalScale() const SIP_DEPRECATED;

    /**
     * Sets resolution (in pixels) of the texture of a terrain tile
     * \see mapTileResolution()
     *
     * \deprecated QGIS 3.42. Use terrainSettings() instead.
     */
    Q_DECL_DEPRECATED void setMapTileResolution( int res ) SIP_DEPRECATED;

    /**
     * Returns resolution (in pixels) of the texture of a terrain tile. This parameter influences
     * how many zoom levels for terrain tiles there will be (together with maxTerrainGroundError())
     *
     * \deprecated QGIS 3.42. Use terrainSettings() instead.
     */
    Q_DECL_DEPRECATED int mapTileResolution() const SIP_DEPRECATED;

    /**
     * Sets maximum allowed screen error of terrain tiles in pixels.
     * \see maxTerrainScreenError()
     *
     * \deprecated QGIS 3.42. Use terrainSettings() instead.
     */
    Q_DECL_DEPRECATED void setMaxTerrainScreenError( double error ) SIP_DEPRECATED;

    /**
     * Returns maximum allowed screen error of terrain tiles in pixels. This parameter decides
     * how aggressively less detailed terrain tiles are swapped to more detailed ones as camera gets closer.
     * Each tile has its error defined in world units - this error gets projected to screen pixels
     * according to camera view and if the tile's error is greater than the allowed error, it will
     * be swapped by more detailed tiles with lower error.
     *
     * \deprecated QGIS 3.42. Use terrainSettings() instead.
     */
    Q_DECL_DEPRECATED double maxTerrainScreenError() const SIP_DEPRECATED;

    /**
     * Sets the maximum ground error of terrain tiles in world units.
     * \see maxTerrainGroundError()
     *
     * \deprecated QGIS 3.42. Use terrainSettings() instead.
     */
    Q_DECL_DEPRECATED void setMaxTerrainGroundError( double error ) SIP_DEPRECATED;

    /**
     * Returns maximum ground error of terrain tiles in world units. This parameter influences
     * how many zoom levels there will be (together with mapTileResolution()).
     * This value tells that when the given ground error is reached (e.g. 10 meters), it makes no sense
     * to further split terrain tiles into finer ones because they will not add extra details anymore.
     *
     * \deprecated QGIS 3.42. Use terrainSettings() instead.
     */
    Q_DECL_DEPRECATED double maxTerrainGroundError() const SIP_DEPRECATED;

    /**
     * Sets the terrain elevation offset (used to move the terrain up or down)
     * \see terrainElevationOffset()
     * \deprecated QGIS 3.42. Use terrainSettings() instead.
     */
    Q_DECL_DEPRECATED void setTerrainElevationOffset( double offset ) SIP_DEPRECATED;

    /**
     * Returns the elevation offset of the terrain (used to move the terrain up or down)
     *
     * \deprecated QGIS 3.42. Use terrainSettings() instead.
     */
    Q_DECL_DEPRECATED double terrainElevationOffset() const SIP_DEPRECATED;

    /**
     * Sets terrain generator and sets extent() as the generator's extent.
     *
     * It takes care of producing terrain tiles from the input data.
     * Takes ownership of the generator.
     *
     * \note Terrain generation will only occur if terrainRenderingEnabled() is TRUE.
     *
     * \see terrainGenerator()
     * \see setTerrainRenderingEnabled()
     * \see setExtent()
     */
    void setTerrainGenerator( QgsTerrainGenerator *gen SIP_TRANSFER ) SIP_SKIP;

    /**
     * Returns the terrain generator.
     *
     * It takes care of producing terrain tiles from the input data.
     *
     * \note Terrain generation will only occur if terrainRenderingEnabled() is TRUE.
     *
     * \see setTerrainGenerator()
     * \see terrainRenderingEnabled()
     */
    QgsTerrainGenerator *terrainGenerator() const SIP_SKIP;

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
    bool isTerrainShadingEnabled() const;

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
    QgsPhongMaterialSettings terrainShadingMaterial() const;

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
    QString terrainMapTheme() const;

    //
    // misc configuration
    //

    //! Sets whether to display bounding boxes of terrain tiles (for debugging)
    void setShowTerrainBoundingBoxes( bool enabled );
    //! Returns whether to display bounding boxes of terrain tiles (for debugging)
    bool showTerrainBoundingBoxes() const;
    //! Sets whether to display extra tile info on top of terrain tiles (for debugging)
    void setShowTerrainTilesInfo( bool enabled );
    //! Returns whether to display extra tile info on top of terrain tiles (for debugging)
    bool showTerrainTilesInfo() const;

    /**
     * Sets whether to show camera's view center as a sphere (for debugging)
     * \since QGIS 3.4
     */
    void setShowCameraViewCenter( bool enabled );

    /**
     * Returns whether to show camera's view center as a sphere (for debugging)
     * \since QGIS 3.4
     */
    bool showCameraViewCenter() const;

    /**
     * Sets whether to show camera's rotation center as a sphere (for debugging)
     * \since QGIS 3.24
     */
    void setShowCameraRotationCenter( bool enabled );

    /**
     * Returns whether to show camera's rotation center as a sphere (for debugging)
     * \since QGIS 3.24
     */
    bool showCameraRotationCenter() const;

    /**
     * Sets whether to show light source origins as a sphere (for debugging)
     * \since QGIS 3.16
     */
    void setShowLightSourceOrigins( bool enabled );

    /**
     * Returns whether to show light source origins as a sphere (for debugging)
     * \since QGIS 3.16
     */
    bool showLightSourceOrigins() const;

    //! Sets whether to display labels on terrain tiles
    void setShowLabels( bool enabled );
    //! Returns whether to display labels on terrain tiles
    bool showLabels() const;

    /**
    * Sets whether eye dome lighting will be used
    * \see eyeDomeLightingEnabled()
    * \since QGIS 3.18
    */
    void setEyeDomeLightingEnabled( bool enabled );
    //! Returns whether eye dome lighting is used
    bool eyeDomeLightingEnabled() const;

    /**
     * Sets the eye dome lighting strength value
     * \see eyeDomeLightingStrength()
     * \since QGIS 3.18
     */
    void setEyeDomeLightingStrength( double strength );
    //! Returns the eye dome lighting strength value
    double eyeDomeLightingStrength() const;

    /**
     * Sets the eye dome lighting distance value (contributes to the contrast of the image
     * \see eyeDomeLightingDistance()
     * \since QGIS 3.18
     */
    void setEyeDomeLightingDistance( int distance );
    //! Returns the eye dome lighting distance value (contributes to the contrast of the image)
    int eyeDomeLightingDistance() const;

    /**
    * Sets whether scene updates on camera movement should be enabled
    * \note By default, scene is updating on camera movement. Useful for debugging purposes.
    * \since QGIS 3.42
    */
    void setStopUpdates( bool enabled );

    /**
     * Returns whether the scene updates on camera movement
     * \since QGIS 3.42
     */
    bool stopUpdates() const;

    /**
     * Sets the debugging settings of the shadow map
     * \see debugShadowMapEnabled()
     * \see debugShadowMapCorner()
     * \see debugShadowMapSize()
     * \since QGIS 3.18
     */
    void setDebugShadowMapSettings( bool enabled, Qt::Corner corner, double size );
    //! Returns whether the shadow map debugging is enabled
    bool debugShadowMapEnabled() const;
    //! Returns the corner where the shadow map preview is displayed
    Qt::Corner debugShadowMapCorner() const;
    //! Returns the size of the shadow map preview
    double debugShadowMapSize() const;

    /**
     * Sets the debugging settings of the depth map
     * \see debugDepthMapEnabled()
     * \see debugDepthMapCorner()
     * \see debugDepthMapSize()
     * \since QGIS 3.18
     */
    void setDebugDepthMapSettings( bool enabled, Qt::Corner corner, double size );
    //! Returns whether the shadow map debugging is enabled
    bool debugDepthMapEnabled() const;
    //! Returns the corner where the shadow map preview is displayed
    Qt::Corner debugDepthMapCorner() const;
    //! Returns the size of the shadow map preview
    double debugDepthMapSize() const;

    /**
     * Returns list of directional light sources defined in the scene.
     * \see setLightSources()
     * \since QGIS 3.26
     */
    QList<QgsLightSource *> lightSources() const;

    /**
     * Sets the list of \a light sources defined in the scene.
     *
     * Ownership of the lights is transferred to the settings.
     *
     * \see lightSources()
     * \since QGIS 3.26
     */
    void setLightSources( const QList<QgsLightSource *> &lights SIP_TRANSFER );

    /**
     * Returns the camera lens' field of view
     * \since QGIS 3.8
     */
    float fieldOfView() const;

    /**
     * Sets the camera lens' field of view
     * \since QGIS 3.8
     */
    void setFieldOfView( const float fieldOfView );

    /**
     * Returns the camera lens' projection type
     * \since QGIS 3.18
     */
    Qt3DRender::QCameraLens::ProjectionType projectionType() const SIP_SKIP;

    /**
     * Sets the camera lens' projection type
     * \since QGIS 3.18
     */
    void setProjectionType( const Qt3DRender::QCameraLens::ProjectionType projectionType ) SIP_SKIP;

#ifndef SIP_RUN

    /**
     * Returns the navigation mode used by the camera
     * \since QGIS 3.18
     */
    Qgis::NavigationMode cameraNavigationMode() const;

    /**
     * Sets the navigation mode for the camera
     * \since QGIS 3.18
     */
    void setCameraNavigationMode( Qgis::NavigationMode navigationMode );
#endif

    /**
     * Returns the camera movement speed
     * \since QGIS 3.18
     */
    double cameraMovementSpeed() const;

    /**
     * Sets the camera movement speed
     * \since QGIS 3.18
     */
    void setCameraMovementSpeed( double movementSpeed );

    /**
     * Sets DPI used for conversion between real world units (e.g. mm) and pixels
     * \param dpi the number of dot per inch
     * \since QGIS 3.10
     */
    void setOutputDpi( const double dpi );

    /**
     * Returns DPI used for conversion between real world units (e.g. mm) and pixels
     * Default value is 96
     * \since QGIS 3.10
     */
    double outputDpi() const;

    /**
     * Returns the current configuration of the skybox
     * \since QGIS 3.16
     */
    QgsSkyboxSettings skyboxSettings() const SIP_SKIP;

    /**
     * Returns the current configuration of shadows
     * \return QGIS 3.16
     */
    QgsShadowSettings shadowSettings() const SIP_SKIP;

    /**
     * Returns the current configuration of screen space ambient occlusion
     * \since QGIS 3.28
     */
    QgsAmbientOcclusionSettings ambientOcclusionSettings() const SIP_SKIP;

    /**
     * Sets the current configuration of the skybox
     * \since QGIS 3.16
     */
    void setSkyboxSettings( const QgsSkyboxSettings &skyboxSettings ) SIP_SKIP;

    /**
     * Sets the current configuration of shadow rendering
     * \since QGIS 3.16
     */
    void setShadowSettings( const QgsShadowSettings &shadowSettings ) SIP_SKIP;

    /**
     * Sets the current configuration of screen space ambient occlusion
     * \since QGIS 3.28
     */
    void setAmbientOcclusionSettings( const QgsAmbientOcclusionSettings &ambientOcclusionSettings ) SIP_SKIP;

    /**
     * Returns whether the skybox is enabled.
     * \see setIsSkyboxEnabled()
     * \since QGIS 3.16
     */
    bool isSkyboxEnabled() const;

    /**
     * Sets whether the skybox is enabled.
     * \see isSkyboxEnabled()
     * \since QGIS 3.16
     */
    void setIsSkyboxEnabled( bool enabled );

    /**
     * Returns whether FPS counter label is enabled
     * \see setIsFpsCounterEnabled()
     * \since QGIS 3.18
     */
    bool isFpsCounterEnabled() const;

    /**
     * Sets whether FPS counter label is enabled
     * \see isFpsCounterEnabled()
     * \since QGIS 3.18
     */
    void setIsFpsCounterEnabled( bool fpsCounterEnabled );

    /**
     * Returns whether the 2D terrain surface will be rendered.
     * \see setTerrainRenderingEnabled()
     * \since QGIS 3.22
     */
    bool terrainRenderingEnabled() const;

    /**
     * Sets whether the 2D terrain surface will be rendered in.
     * \see terrainRenderingEnabled()
     * \since QGIS 3.22
     */
    void setTerrainRenderingEnabled( bool terrainRenderingEnabled );

    /**
     * Returns the renderer usage
     *
     * \see rendererUsage()
     * \since QGIS 3.24
     */
    Qgis::RendererUsage rendererUsage() const;

    /**
     * Sets the renderer usage
     *
     * \see rendererUsage()
     * \since QGIS 3.24
     */
    void setRendererUsage( Qgis::RendererUsage rendererUsage );

    /**
     * Returns the view sync mode (used to synchronize the 2D main map canvas and the 3D camera navigation)
     *
     * \since QGIS 3.26
     */
    Qgis::ViewSyncModeFlags viewSyncMode() const;

    /**
     * Sets the view sync mode (used to synchronize the 2D main map canvas and the 3D camera navigation)
     *
     * \since QGIS 3.26
     */
    void setViewSyncMode( Qgis::ViewSyncModeFlags mode );

    /**
     * Returns whether the camera's view frustum is visualized on the 2D map canvas
     *
     * \since QGIS 3.26
     */
    bool viewFrustumVisualizationEnabled() const;

    /**
     * Sets whether the camera's view frustum is visualized on the 2D map canvas
     *
     * \since QGIS 3.26
     */
    void setViewFrustumVisualizationEnabled( bool enabled );

    /**
     * Returns the current configuration of 3d axis
     * \return QGIS 3.26
     */
    Qgs3DAxisSettings get3DAxisSettings() const SIP_SKIP;

    /**
     * Sets the current configuration of 3d axis
     * \since QGIS 3.26
     */
    void set3DAxisSettings( const Qgs3DAxisSettings &axisSettings, bool force = false ) SIP_SKIP;

    /**
     * Returns whether debug overlay is enabled
     * \see setIsDebugOverlayEnabled()
     * \since QGIS 3.26
     */
    bool isDebugOverlayEnabled() const;

    /**
     * Sets whether debug overlay is enabled
     * The debug overlay displays some debugging and profiling information.
     * It has been introduced in Qt version 5.15.
     * This parameter is transient. It is not saved in the project parameters.
     * \see isDebugOverlayEnabled()
     * \since QGIS 3.26
     */
    void setIsDebugOverlayEnabled( bool debugOverlayEnabled );

    /**
     * Returns whether the extent is displayed on the main 2D map canvas
     * \see setShowExtentIn2DView()
     * \since QGIS 3.32
     */
    bool showExtentIn2DView() const;

    /**
     * Sets whether the extent is displayed on the main 2D map canvas
     * \since QGIS 3.32
     */
    void setShowExtentIn2DView( bool show );

    /**
     * Sets whether the debug side panel is shown
     * \since QGIS 3.42
     */
    void setShowDebugPanel( bool enabled );

    /**
     * Returns whether the debug side panel is shown
     * \since QGIS 3.42
     */
    bool showDebugPanel() const;

  signals:

    /**
     * Emitted when one of the configuration settings has changed
     *
     * \since QGIS 3.24
     */
    void settingsChanged();

    //! Emitted when the background color has changed
    void backgroundColorChanged();
    //! Emitted when the selection color has changed
    void selectionColorChanged();

    /**
     * Emitted when the list of map layers for 3d rendering has changed.
     *
     * \see setLayers()
     * \see layers()
     */
    void layersChanged();

    //! Emitted when the terrain generator has changed
    void terrainGeneratorChanged();

    /**
     * Emitted when the terrain settings are changed.
     *
     * \since QGIS 3.42
     */
    void terrainSettingsChanged();

    /**
     * Emitted when the vertical scale of the terrain has changed
     * \deprecated QGIS 3.42. Use terrainSettingsChanged() instead.
     */
    Q_DECL_DEPRECATED void terrainVerticalScaleChanged() SIP_DEPRECATED;

    /**
     * Emitted when the map tile resoulution has changed
     * \deprecated QGIS 3.42. Use terrainSettingsChanged() instead.
     */
    Q_DECL_DEPRECATED void mapTileResolutionChanged() SIP_DEPRECATED;

    /**
     * Emitted when the maximum terrain screen error has changed
     * \deprecated QGIS 3.42. Use terrainSettingsChanged() instead.
     */
    Q_DECL_DEPRECATED void maxTerrainScreenErrorChanged() SIP_DEPRECATED;

    /**
     * Emitted when the maximum terrain ground error has changed
     *
     * \deprecated QGIS 3.42. Use terrainSettingsChanged() instead.
     */
    Q_DECL_DEPRECATED void maxTerrainGroundErrorChanged() SIP_DEPRECATED;

    /**
     * Emitted when the terrain elevation offset is changed
     *
     * \deprecated QGIS 3.42. Use terrainSettingsChanged() instead.
     */
    Q_DECL_DEPRECATED void terrainElevationOffsetChanged( double newElevation ) SIP_DEPRECATED;

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

    /**
     * Emitted when the flag whether camera's rotation center is shown has changed
     * \since QGIS 3.24
     */
    void showCameraRotationCenterChanged();

    /**
     * Emitted when the flag whether light source origins are shown has changed.
     * \since QGIS 3.15
     */
    void showLightSourceOriginsChanged();

    //! Emitted when the flag whether labels are displayed on terrain tiles has changed
    void showLabelsChanged();

    /**
     * Emitted when the flag whether to keep updating scene has changed
     * \since QGIS 3.42
     */
    void stopUpdatesChanged();

    /**
     * Emitted when the flag whether eye dome lighting is used has changed
     * \since QGIS 3.18
     */
    void eyeDomeLightingEnabledChanged();

    /**
     * Emitted when the eye dome lighting strength has changed
     * \since QGIS 3.18
     */
    void eyeDomeLightingStrengthChanged();

    /**
     * Emitted when the eye dome lighting distance has changed
     * \since QGIS 3.18
     */
    void eyeDomeLightingDistanceChanged();

    /**
     * Emitted when shadow map debugging has changed
     * \since QGIS 3.18
     */
    void debugShadowMapSettingsChanged();

    /**
     * Emitted when depth map debugging has changed
     * \since QGIS 3.18
     */
    void debugDepthMapSettingsChanged();

    /**
     * Emitted when the list of point lights changes
     * \since QGIS 3.6
     */
    void pointLightsChanged();

    /**
     * Emitted when any of the light source settings in the map changes.
     * \since QGIS 3.26
     */
    void lightSourcesChanged();

    /**
     * Emitted when the list of directional lights changes
     * \since QGIS 3.16
     */
    void directionalLightsChanged();

    /**
     * Emitted when the camera lens field of view changes
     * \since QGIS 3.8
     */
    void fieldOfViewChanged();

    /**
     * Emitted when the camera lens projection type changes
     * \since QGIS 3.18
     */
    void projectionTypeChanged();

    /**
     * Emitted when the camera navigation mode was changed
     * \since QGIS 3.18
     */
    void cameraNavigationModeChanged();

    /**
     * Emitted when the camera movement speed was changed
     * \since QGIS 3.18
     */
    void cameraMovementSpeedChanged();

    /**
     * Emitted when skybox settings are changed
     * \since QGIS 3.16
     */
    void skyboxSettingsChanged();

    /**
     * Emitted when shadow rendering settings are changed
     * \since QGIS 3.16
     */
    void shadowSettingsChanged();


    /**
     * Emitted when ambient occlusion rendering settings are changed
     * \since QGIS 3.28
     */
    void ambientOcclusionSettingsChanged();

    /**
     * Emitted when the FPS counter is enabled or disabled
     * \since QGIS 3.18
     */
    void fpsCounterEnabledChanged( bool fpsCounterEnabled );

    /**
     * Emitted when the camera's view frustum visualization on the main 2D map canvas is enabled or disabled
     *
     * \since QGIS 3.26
     */
    void viewFrustumVisualizationEnabledChanged();

    /**
     * Emitted when 3d axis rendering settings are changed
     * \since QGIS 3.26
     */
    void axisSettingsChanged();

    /**
     * Emitted when the debug overaly is enabled or disabled
     * \since QGIS 3.26
     */
    void debugOverlayEnabledChanged( bool debugOverlayEnabled );

    /**
     * Emitted when the 3d view's 2d extent has changed
     * \see setExtent()
     * \since QGIS 3.30
     */
    void extentChanged();

    /**
     * Emitted when the parameter to display 3d view's extent in the 2D canvas has changed
     * \see setShowExtentIn2DView()
     * \since QGIS 3.32
     */
    void showExtentIn2DViewChanged();

    /**
     * Emitted when the Show debug panel checkbox changes value
     * \see setShowDebugPanel()
     * \since QGIS 3.42
     */
    void showDebugPanelChanged( bool shown );

    /**
     * Emitted when the world's origin point has been shifted
     * \see setOrigin()
     * \since QGIS 3.42
     */
    void originChanged();

  private:
#ifdef SIP_RUN
    Qgs3DMapSettings &operator=( const Qgs3DMapSettings & );
#endif

  private:
    //! Connects the various changed signals of this widget to the settingsChanged signal
    void connectChangedSignalsToSettingsChanged();

  private:
    //! Offset in map CRS coordinates at which our 3D world has origin (0,0,0)
    QgsVector3D mOrigin;
    QgsCoordinateReferenceSystem mCrs;                      //!< Destination coordinate system of the world
    QColor mBackgroundColor = Qt::black;                    //!< Background color of the scene
    QColor mSelectionColor;                                 //!< Color to be used for selected map features
    std::unique_ptr<QgsTerrainGenerator> mTerrainGenerator; //!< Implementation of the terrain generation
    std::unique_ptr<QgsAbstractTerrainSettings> mTerrainSettings;
    bool mTerrainShadingEnabled = false;                                                                      //!< Whether terrain should be shaded taking lights into account
    QgsPhongMaterialSettings mTerrainShadingMaterial;                                                         //!< Material to use for the terrain (if shading is enabled). Diffuse color is ignored.
    QString mTerrainMapTheme;                                                                                 //!< Name of map theme used for terrain's texture (empty means use the current map theme)
    bool mShowTerrainBoundingBoxes = false;                                                                   //!< Whether to show bounding boxes of entities - useful for debugging
    bool mShowTerrainTileInfo = false;                                                                        //!< Whether to draw extra information about terrain tiles to the textures - useful for debugging
    bool mShowCameraViewCenter = false;                                                                       //!< Whether to show camera view center as a sphere - useful for debugging
    bool mShowCameraRotationCenter = false;                                                                   //!< Whether to show camera rotation center as a sphere - useful for debugging
    bool mShowLightSources = false;                                                                           //!< Whether to show the origin of light sources
    bool mShowLabels = false;                                                                                 //!< Whether to display labels on terrain tiles
    bool mStopUpdates = false;                                                                                //!< Whether to stop updating scene on zoom
    bool mShowDebugPanel = false;                                                                             //!< Whether to show debug panel
    QList<QgsLightSource *> mLightSources;                                                                    //!< List of light sources in the scene (owned by the settings)
    float mFieldOfView = 45.0f;                                                                               //<! Camera lens field of view value
    Qt3DRender::QCameraLens::ProjectionType mProjectionType = Qt3DRender::QCameraLens::PerspectiveProjection; //<! Camera lens projection type
    Qgis::NavigationMode mCameraNavigationMode = Qgis::NavigationMode::TerrainBased;
    double mCameraMovementSpeed = 5.0;
    QList<QgsMapLayerRef> mLayers; //!< Layers to be rendered
    //! Coordinate transform context
    QgsCoordinateTransformContext mTransformContext;
    QgsPathResolver mPathResolver;
    QgsMapThemeCollection *mMapThemes = nullptr; //!< Pointer to map themes (e.g. from the current project) to resolve map theme content from the name
    double mDpi = 96;                            //!< Dot per inch value for the screen / painter
    bool mIsFpsCounterEnabled = false;

    bool mIsSkyboxEnabled = false;                         //!< Whether the skybox is enabled
    QgsSkyboxSettings mSkyboxSettings;                     //!< Skybox related configuration
    QgsShadowSettings mShadowSettings;                     //!< Shadow rendering related settings
    QgsAmbientOcclusionSettings mAmbientOcclusionSettings; //!< Screen Space Ambient Occlusion related settings

    bool mEyeDomeLightingEnabled = false;
    double mEyeDomeLightingStrength = 1000.0;
    int mEyeDomeLightingDistance = 1;

    Qgis::ViewSyncModeFlags mViewSyncMode;
    bool mVisualizeViewFrustum = false;

    bool mDebugShadowMapEnabled = false;
    Qt::Corner mDebugShadowMapCorner = Qt::Corner::TopLeftCorner;
    double mDebugShadowMapSize = 0.2;

    bool mDebugDepthMapEnabled = false;
    Qt::Corner mDebugDepthMapCorner = Qt::Corner::BottomLeftCorner;
    double mDebugDepthMapSize = 0.2;

    bool mTerrainRenderingEnabled = true;

    Qgis::RendererUsage mRendererUsage;

    Qgs3DAxisSettings m3dAxisSettings; //!< 3d axis related configuration

    bool mIsDebugOverlayEnabled = false;

    QgsRectangle mExtent; //!< 2d extent used to limit the 3d view

    bool mShowExtentIn2DView = false;
};


#endif // QGS3DMAPSETTINGS_H
