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
#include "qgsterraingenerator.h"

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
    ~Qgs3DMapSettings();

    //! Reads configuration from a DOM element previously written by writeXml()
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context );
    //! Writes configuration to a DOM element, to be used later with readXml()
    QDomElement writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const;
    //! Resolves references to other objects (map layers) after the call to readXml()
    void resolveReferences( const QgsProject &project );

    //! Sets coordinates in map CRS at which our 3D world has origin (0,0,0)
    void setOrigin( double originX, double originY, double originZ );
    //! Returns X coordinate in map CRS at which 3D scene has origin (zero)
    double originX() const { return mOriginX; }
    //! Returns Y coordinate in map CRS at which 3D scene has origin (zero)
    double originY() const { return mOriginY; }
    //! Returns Z coordinate in map CRS at which 3D scene has origin (zero)
    double originZ() const { return mOriginZ; }

    //! Sets coordinate reference system used in the 3D scene
    void setCrs( const QgsCoordinateReferenceSystem &crs );
    //! Returns coordinate reference system used in the 3D scene
    QgsCoordinateReferenceSystem crs() const { return mCrs; }

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

    //! Sets the list of map layers to be rendered as a texture of the terrain
    void setLayers( const QList<QgsMapLayer *> &layers );
    //! Returns the list of map layers to be rendered as a texture of the terrain
    QList<QgsMapLayer *> layers() const;

    /**
     * Sets resolution (in pixels) of the texture of a terrain tile
     * \sa mapTileResolution()
     */
    void setMapTileResolution( int res );

    /**
     * Returns resolution (in pixels) of the texture of a terrain tile. This parameter influences
     * how many zoom levels for terrain tiles there will be (together with maxTerrainGroundError())
     */
    int mapTileResolution() const;

    /**
     * Sets maximum allowed screen error of terrain tiles in pixels.
     * \sa maxTerrainScreenError()
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
     * \sa maxTerrainGroundError()
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
    //! Sets whether to display labels on terrain tiles
    void setShowLabels( bool enabled );
    //! Returns whether to display labels on terrain tiles
    bool showLabels() const { return mShowLabels; }

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
    //! Emitted when the flag whether terrain's bounding boxes are shown has changed
    void showTerrainBoundingBoxesChanged();
    //! Emitted when the flag whether terrain's tile info is shown has changed
    void showTerrainTilesInfoChanged();
    //! Emitted when the flag whether labels are displayed on terrain tiles has changed
    void showLabelsChanged();

  private:
    //! X coordinate in map CRS at which our 3D world has origin (0,0,0)
    double mOriginX = 0;
    //! Y coordinate in map CRS at which our 3D world has origin (0,0,0)
    double mOriginY = 0;
    //! Z coordinate in map CRS at which our 3D world has origin (0,0,0)
    double mOriginZ = 0;
    QgsCoordinateReferenceSystem mCrs;   //!< Destination coordinate system of the world
    QColor mBackgroundColor = Qt::black;   //!< Background color of the scene
    QColor mSelectionColor; //!< Color to be used for selected map features
    double mTerrainVerticalScale = 1;   //!< Multiplier of terrain heights to make the terrain shape more pronounced
    std::unique_ptr<QgsTerrainGenerator> mTerrainGenerator;  //!< Implementation of the terrain generation
    int mMapTileResolution = 512;   //!< Size of map textures of tiles in pixels (width/height)
    float mMaxTerrainScreenError = 3.f;   //!< Maximum allowed terrain error in pixels (determines when tiles are switched to more detailed ones)
    float mMaxTerrainGroundError = 1.f;  //!< Maximum allowed horizontal map error in map units (determines how many zoom levels will be used)
    bool mShowTerrainBoundingBoxes = false;  //!< Whether to show bounding boxes of entities - useful for debugging
    bool mShowTerrainTileInfo = false;  //!< Whether to draw extra information about terrain tiles to the textures - useful for debugging
    bool mShowLabels = false; //!< Whether to display labels on terrain tiles
    QList<QgsMapLayerRef> mLayers;   //!< Layers to be rendered
    QList<QgsAbstract3DRenderer *> mRenderers;  //!< Extra stuff to render as 3D object
    bool mSkyboxEnabled = false;  //!< Whether to render skybox
    QString mSkyboxFileBase; //!< Base part of the files with skybox textures
    QString mSkyboxFileExtension; //!< Extension part of the files with skybox textures
};


#endif // QGS3DMAPSETTINGS_H
