/***************************************************************************
    qgsrasterdataprovider.h - DataProvider Interface for raster layers
     --------------------------------------
    Date                 : Mar 11, 2005
    Copyright            : (C) 2005 by Brendan Morley
    email                : morb at ozemail dot com dot au
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

/* Thank you to Marco Hugentobler for the original vector DataProvider */

#ifndef QGSRASTERDATAPROVIDER_H
#define QGSRASTERDATAPROVIDER_H

#include <set>
#include <map>
#include <qobject.h>
#include <qtextcodec.h>
#include <qimage.h>

#include "qgsdataprovider.h"
#include "qgspoint.h"


    //! Copied from QgsRasterProvider
    struct QgsRasterViewPort
{
    /** \brief  The offset from the left hand edge of the raster for the rectangle that will be drawn to screen.
     * TODO Check this explanation is correc!*/
    int   rectXOffsetInt;
    float rectXOffsetFloat;
    /** \brief  The offset from the bottom edge of the raster for the rectangle that will be drawn to screen.
     * TODO Check this explanation is correc!*/
    int   rectYOffsetInt;
    float rectYOffsetFloat;
    /** \brief Lower left X dimension of clipped raster image in raster pixel space.
     *  RasterIO will do the scaling for us, so for example, if the user is zoomed in a long way, there may only 
     *  be e.g. 5x5 pixels retrieved from the raw raster data, but rasterio will seamlessly scale the up to 
     *  whatever the screen coordinates are (e.g. a 600x800 display window) */
    double clippedXMinDouble;
    /** \brief Top Right X dimension of clipped raster image in raster pixel space.
     *  RasterIO will do the scaling for us, so for example, if the user is zoomed in a long way, there may only 
     *  be e.g. 5x5 pixels retrieved from the raw raster data, but rasterio will seamlessly scale the up to 
     *  whatever the screen coordinates are (e.g. a 600x800 display window) */
    double clippedXMaxDouble;
    /** \brief Lower left Y dimension of clipped raster image in raster pixel space.
     *  RasterIO will do the scaling for us, so for example, if the user is zoomed in a long way, there may only 
     *  be e.g. 5x5 pixels retrieved from the raw raster data, but rasterio will seamlessly scale the up to 
     *  whatever the screen coordinates are (e.g. a 600x800 display window) */
    double clippedYMinDouble;
    /** \brief Top Right X dimension of clipped raster image in raster pixel space.
     *  RasterIO will do the scaling for us, so for example, if the user is zoomed in a long way, there may only 
     *  be e.g. 5x5 pixels retrieved from the raw raster data, but rasterio will seamlessly scale the up to 
     *  whatever the screen coordinates are (e.g. a 600x800 display window) */
    double clippedYMaxDouble;
    /** \brief  Distance in pixels from clippedXMinDouble to clippedXMaxDouble. */
    int clippedWidthInt;
    /** \brief Distance in pixels from clippedYMinDouble to clippedYMaxDouble  */
    int clippedHeightInt;
    /** \brief Coordinate (in geographic coordinate system) of top left corner of the part of the raster that 
     * is to be rendered.*/
    QgsPoint topLeftPoint;
    /** \brief Coordinate (in geographic coordinate system) of bottom right corner of the part of the raster that 
     * is to be rendered.*/
    QgsPoint bottomRightPoint;
    /** \brief Distance in map units from left edge to right edge for the part of the raster that 
     * is to be rendered.*/
    int drawableAreaXDimInt;
    /** \brief Distance in map units from bottom edge to top edge for the part of the raster that 
     * is to be rendered.*/
    int drawableAreaYDimInt;
};



/** Base class for raster data providers
 *
 *  \note  This class has been copied and pasted from
 *         QgsVectorDataProvider, and does not yet make
 *         sense for Raster layers.
 */
 
class QgsRasterDataProvider : public QgsDataProvider
{
 
  Q_OBJECT
     
public:


    QgsRasterDataProvider();

    virtual ~QgsRasterDataProvider() {};

    /**
     * Add the list of WMS layer names to be rendered by this server
     */
    virtual void addLayers(QStringList layers) = 0;

    // TODO: Document this better.
    /** \brief   Renders the layer as an image
     */
    virtual QImage* draw(QgsRect viewExtent, int pixelWidth, int pixelHeight) = 0;

    
    
        

    // TODO: Get the supported formats by this provider
    
    // TODO: Get the file masks supported by this provider, suitable for feeding into the file open dialog box
    


    
        
protected:

};

#endif
