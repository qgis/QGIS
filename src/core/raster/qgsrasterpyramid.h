/***************************************************************************
                            qgsrasterpyramid.h

                             -------------------
    begin                : 2007
    copyright            : (C) 2007 by Gary E. Sherman
    email                : sherman@mrcc.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSRASTERPYRAMID
#define QGSRASTERPYRAMID

#include "qgis_core.h"
#include "qgis_sip.h"

/**
 * \ingroup core
  * \brief This struct is used to store pyramid info for the raster layer.
  */
class CORE_EXPORT QgsRasterPyramid
{
  public:

    //TODO QGIS 4.0 - rename get* to remove get prefix, and remove
    //temporary SIP_PROPERTY definitions

    /**
     * Returns the pyramid level.
     *
     * Pyramid levels are as defined by GDAL, eg
     * level 2 is half the original raster size.
     *
     * \since QGIS 3.20
     */
    int getLevel() const { return mLevel; }

    /**
     * Sets the pyramid \a level.
     *
     * Pyramid levels are as defined by GDAL, eg
     * level 2 is half the original raster size.
     *
     * \since QGIS 3.20
     */
    void setLevel( int level ) { mLevel = level; }

#ifdef SIP_RUN
    SIP_PROPERTY( name = level, get = getLevel, set = setLevel )
#endif

    /**
     * Sets the x \a dimension for this pyramid layer.
     *
     * \since QGIS 3.20
     */
    void setXDim( int dimension ) { mXDim = dimension; }

    /**
     * Returns the x dimension for this pyramid layer.
     *
     * \since QGIS 3.20
     */
    int getXDim() const { return mXDim; }

#ifdef SIP_RUN
    SIP_PROPERTY( name = xDim, get = getXDim, set = setXDim )
#endif

    /**
     * Sets the y \a dimension for this pyramid layer.
     *
     * \since QGIS 3.20
     */
    void setYDim( int dimension ) { mYDim = dimension; }

    /**
     * Returns the y dimension for this pyramid layer.
     *
     * \since QGIS 3.20
     */
    int getYDim() const { return mYDim; }

#ifdef SIP_RUN
    SIP_PROPERTY( name = yDim, get = getYDim, set = setYDim )
#endif

    /**
     * Returns TRUE if the pyramid layer currently exists.
     *
     * \since QGIS 3.20
     */
    bool getExists() const { return mExists; }

    /**
     * Sets whether the pyramid layer currently exists.
     *
     * \since QGIS 3.20
     */
    void setExists( bool exists ) { mExists = exists; }

#ifdef SIP_RUN
    SIP_PROPERTY( name = exists, get = getExists, set = setExists )
#endif

    /**
     * Returns TRUE if the pyramid layer will be built.
     *
     * When used with QgsRasterDataProvider::buildPyramids() this flag controls
     * whether pyramids will be built for the layer.
     *
     * \since QGIS 3.20
     */
    bool getBuild() const { return mBuild; }

    /**
     * Sets whether the pyramid layer will be built.
     *
     * When used with QgsRasterDataProvider::buildPyramids() this flag controls
     * whether pyramids will be built for the layer. Set to TRUE to build
     * the pyramids.
     *
     * \since QGIS 3.20
     */
    void setBuild( bool build ) { mBuild = build; }

#ifdef SIP_RUN
    SIP_PROPERTY( name = build, get = getBuild, set = setBuild )
#endif

  private:

    //! \brief The pyramid level as implemented in gdal (level 2 is half original raster size etc)
    int mLevel = 0;
    //! \brief XDimension for this pyramid layer
    int mXDim = 0;
    //! \brief YDimension for this pyramid layer
    int mYDim = 0;
    //! \brief Whether the pyramid layer has been built yet
    bool mExists = false;
    //! \brief Whether the pyramid should be built
    bool mBuild = false;

};
#endif
