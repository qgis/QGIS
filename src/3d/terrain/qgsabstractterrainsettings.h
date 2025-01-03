/***************************************************************************
  qgsabstractterrainsettings.h
  --------------------------------------
  Date                 : August 2024
  Copyright            : (C) 2024 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSABSTRACTTERRAINSETTINGS_H
#define QGSABSTRACTTERRAINSETTINGS_H

#include "qgis_3d.h"
#include "qgis_sip.h"
#include <QString>
#include <memory>

class QDomElement;
class QgsReadWriteContext;
class QgsProject;
class QgsTerrainGenerator;
class Qgs3DRenderContext;

/**
 * \ingroup 3d
 * \brief Base class for all terrain settings classes.
 *
 * QgsAbstractTerrainSettings subclasses are responsible for storing the configuration
 * of terrain generators.
 *
 * \warning This is not considered stable API, and may change in future QGIS releases. It is
 * exposed to the Python bindings as a tech preview only.
 *
 * \since QGIS 3.42
 */
class _3D_EXPORT QgsAbstractTerrainSettings SIP_ABSTRACT
{
#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( sipCpp->type() == "flat" )
      sipType = sipType_QgsFlatTerrainSettings;
    else if ( sipCpp->type() == "dem" )
      sipType = sipType_QgsDemTerrainSettings;
    else if ( sipCpp->type() == "online" )
      sipType = sipType_QgsOnlineDemTerrainSettings;
    else if ( sipCpp->type() == "mesh" )
      sipType = sipType_QgsMeshTerrainSettings;
    else if ( sipCpp->type() == "quantizedmesh" )
      sipType = sipType_QgsQuantizedMeshTerrainSettings;
    else
      sipType = 0;
    SIP_END
#endif

  public:
    virtual ~QgsAbstractTerrainSettings();

    /**
     * Returns a copy of the terrain settings.
     */
    virtual QgsAbstractTerrainSettings *clone() const = 0 SIP_FACTORY;

    /**
     * Returns the unique type name for the terrain generator.
     */
    virtual QString type() const = 0;

    /**
     * Returns TRUE if this settings is exactly equal to another \a other settings.
     */
    virtual bool equals( const QgsAbstractTerrainSettings *other ) const = 0;

    /**
     * Reads settings from a DOM \a element.
     *
     * Subclasses should take care to call readCommonProperties() to read common properties from the element.
     *
     * \see resolveReferences()
     * \see writeXml()
     */
    virtual void readXml( const QDomElement &element, const QgsReadWriteContext &context ) = 0;

    /**
     * Writes settings to a DOM \a element.
     *
     * Subclasses should take care to call writeCommonProperties() to write common properties to the element.
     *
     * \see readXml()
     */
    virtual void writeXml( QDomElement &element, const QgsReadWriteContext &context ) const = 0;

    /**
     * After reading settings from XML, resolves references to any layers in a \a project that have been read as layer IDs.
     *
     * \see readXml()
     */
    virtual void resolveReferences( const QgsProject *project );

    /**
     * Creates a new instance of a terrain generator which matches the terrain settings.
     *
     * \note Not available in Python bindings
     */
    virtual std::unique_ptr<QgsTerrainGenerator> createTerrainGenerator( const Qgs3DRenderContext &context ) const = 0 SIP_SKIP;

    // common settings

    /**
     * Sets the vertical \a scale (exaggeration) for terrain.
     *
     * (1 = true scale, > 1 = hills get more pronounced)
     *
     * \see verticalScale()
     */
    void setVerticalScale( double scale ) { mTerrainVerticalScale = scale; }

    /**
     * Returns the vertical scale (exaggeration) for terrain.
     *
     * (1 = true scale, > 1 = hills get more pronounced)
     *
     * \see setVerticalScale()
     */
    double verticalScale() const { return mTerrainVerticalScale; }

    /**
     * Sets the \a resolution (in pixels) of the texture of a terrain tile
     * \see mapTileResolution()
     */
    void setMapTileResolution( int resolution ) { mMapTileResolution = resolution; }

    /**
     * Returns the resolution (in pixels) of the texture of a terrain tile.
     *
     * This parameter influences how many zoom levels for terrain tiles there will be (together with maxTerrainGroundError())
     *
     * \see setMapTileResolution()
     */
    int mapTileResolution() const { return mMapTileResolution; }

    /**
     * Sets the maximum allowed screen \a error of terrain tiles in pixels.
     *
     * \see maximumScreenError()
     */
    void setMaximumScreenError( double error ) { mMaxTerrainScreenError = error; }

    /**
     * Returns the maximum allowed screen error of terrain tiles in pixels.
     *
     * This parameter decides how aggressively less detailed terrain tiles are swapped to more detailed ones as camera gets closer.
     * Each tile has its error defined in world units - this error gets projected to screen pixels
     * according to camera view and if the tile's error is greater than the allowed error, it will
     * be swapped by more detailed tiles with lower error.
     *
     * see setMaximumScreenError()
     */
    double maximumScreenError() const { return mMaxTerrainScreenError; }

    /**
     * Sets the maximum ground \a error of terrain tiles in world units.
     *
     * \see maximumGroundError()
     */
    void setMaximumGroundError( double error ) { mMaxTerrainGroundError = error; }

    /**
     * Returns the maximum ground error of terrain tiles in world units.
     *
     * This parameter influences how many zoom levels there will be (together with mapTileResolution()).
     * This value tells that when the given ground error is reached (e.g. 10 meters), it makes no sense
     * to further split terrain tiles into finer ones because they will not add extra details anymore.
     *
     * \see setMaximumGroundError()
     */
    double maximumGroundError() const { return mMaxTerrainGroundError; }

    /**
     * Sets the terrain elevation \a offset (used to move the terrain up or down).
     *
     * \see elevationOffset()
     */
    void setElevationOffset( double offset ) { mTerrainElevationOffset = offset; }

    /**
     * Returns the elevation offset of the terrain (used to move the terrain up or down).
     *
     * \see setElevationOffset()
     */
    double elevationOffset() const { return mTerrainElevationOffset; }

  protected:
    /**
     * Writes common properties from the base class into an XML \a element.
     *
     * \see writeXml()
     */
    void writeCommonProperties( QDomElement &element, const QgsReadWriteContext &context ) const;

    /**
     * Reads common properties from the base class from the given DOM \a element.
     *
     * \see readXml()
     */
    void readCommonProperties( const QDomElement &element, const QgsReadWriteContext &context );

    /**
     * Copies common properties from a \a source object.
     */
    void copyCommonProperties( const QgsAbstractTerrainSettings *source );

    /**
     * Returns TRUE if common base class settings from \a other match this object.
     */
    bool equalsCommon( const QgsAbstractTerrainSettings *other ) const;

  private:
    double mTerrainVerticalScale = 1;     //!< Multiplier of terrain heights to make the terrain shape more pronounced
    int mMapTileResolution = 512;         //!< Size of map textures of tiles in pixels (width/height)
    double mMaxTerrainScreenError = 3.0;  //!< Maximum allowed terrain error in pixels (determines when tiles are switched to more detailed ones)
    double mMaxTerrainGroundError = 1.0;  //!< Maximum allowed horizontal map error in map units (determines how many zoom levels will be used)
    double mTerrainElevationOffset = 0.0; //!< Terrain elevation offset (used to adjust the position of the terrain and move it up and down)
};


#endif // QGSABSTRACTTERRAINSETTINGS_H
