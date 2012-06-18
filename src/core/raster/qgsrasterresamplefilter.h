/***************************************************************************
                         qgsrasterresamplefilter.h
                         -------------------
    begin                : December 2011
    copyright            : (C) 2011 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRASTERRESAMPLEFILTER_H
#define QGSRASTERRESAMPLEFILTER_H

#include "qgsrasterdataprovider.h"
#include "qgsrasterface.h"

class QPainter;
class QgsMapToPixel;
class QgsRasterResampler;
class QgsRasterProjector;
class QgsRasterTransparency;
class QgsRasterViewPort;

class QDomElement;

class QgsRasterResampleFilter : public QgsRasterFace
{
  public:
    QgsRasterResampleFilter( QgsRasterFace* input = 0 );
    ~QgsRasterResampleFilter();

    void * readBlock( int bandNo, QgsRectangle  const & extent, int width, int height );

    /**Set resampler for zoomed in scales. Takes ownership of the object*/
    void setZoomedInResampler( QgsRasterResampler* r );
    const QgsRasterResampler* zoomedInResampler() const { return mZoomedInResampler; }

    /**Set resampler for zoomed out scales. Takes ownership of the object*/
    void setZoomedOutResampler( QgsRasterResampler* r );
    const QgsRasterResampler* zoomedOutResampler() const { return mZoomedOutResampler; }

    void setMaxOversampling( double os ) { mMaxOversampling = os; }
    double maxOversampling() const { return mMaxOversampling; }

    void writeXML( QDomDocument& doc, QDomElement& parentElem );

    /**Sets base class members from xml. Usually called from create() methods of subclasses*/
    void readXML( const QDomElement& resamplefilterElem );

  protected:

    /**Write upper class info into <rasterresamplefilter> element (called by writeXML method of subclasses)*/
    //void _writeXML( QDomDocument& doc, QDomElement& rasterRendererElem ) const;


    /**Resampler used if screen resolution is higher than raster resolution (zoomed in). 0 means no resampling (nearest neighbour)*/
    QgsRasterResampler* mZoomedInResampler;
    /**Resampler used if raster resolution is higher than raster resolution (zoomed out). 0 mean no resampling (nearest neighbour)*/
    QgsRasterResampler* mZoomedOutResampler;
    //QMap<int, RasterPartInfo> mRasterPartInfos;

    /**Maximum boundary for oversampling (to avoid too much data traffic). Default: 2.0*/
    double mMaxOversampling;

  private:
};

#endif // QGSRASTERRESAMPLEFILTER_H
