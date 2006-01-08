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
#include <qgsrasterviewport.h>


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

    QgsRasterDataProvider( QString const & uri );

    virtual ~QgsRasterDataProvider() {};

    /**
     * Add the list of WMS layer names to be rendered by this server
     */
    virtual void addLayers(QStringList const & layers,
                           QStringList  const & styles = QStringList()) = 0;

    //! get raster image encodings supported by (e.g.) the WMS Server, expressed as MIME types
    virtual QStringList supportedImageEncodings() = 0;

    /**
     * Set the image encoding (as a MIME type) used in the transfer from (e.g.) the WMS server
     */
    virtual void setImageEncoding(QString  const & mimeType) = 0;

    // TODO: Document this better.
    /** \brief   Renders the layer as an image
     */
    virtual QImage* draw(QgsRect  const & viewExtent, int pixelWidth, int pixelHeight) = 0;

    
    
        

    // TODO: Get the supported formats by this provider
    
    // TODO: Get the file masks supported by this provider, suitable for feeding into the file open dialog box
    

    /**
     * Get metadata in a format suitable for feeding directly
     * into a subset of the GUI raster properties "Metadata" tab.
     */
    virtual QString getMetadata() = 0;

        
protected:

};

#endif
