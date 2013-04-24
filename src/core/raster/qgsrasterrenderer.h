/***************************************************************************
                         qgsrasterrenderer.h
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

#ifndef QGSRASTERRENDERER_H
#define QGSRASTERRENDERER_H

#include <QPair>

#include "qgsrasterdataprovider.h"
#include "qgsrasterinterface.h"

class QDomElement;

class QPainter;
class QgsRasterTransparency;

/** \ingroup core
  * Raster renderer pipe that applies colours to a raster.
  */
class CORE_EXPORT QgsRasterRenderer : public QgsRasterInterface
{
  public:
    // Origin of min / max values
    enum MinMaxOrigin
    {
      MinMaxUnknown         = 0,
      MinMaxUser            = 1, // entered by user
      // method
      MinMaxMinMax          = 1 << 1,
      MinMaxCumulativeCut   = 1 << 2,
      MinMaxStdDev          = 1 << 3,
      // Extent
      MinMaxFullExtent      = 1 << 4,
      MinMaxSubExtent       = 1 << 5,
      // Precision
      MinMaxEstimated       = 1 << 6,
      MinMaxExact           = 1 << 7
    };

    static const QRgb NODATA_COLOR;

    QgsRasterRenderer( QgsRasterInterface* input = 0, const QString& type = "" );
    virtual ~QgsRasterRenderer();

    QgsRasterInterface * clone() const = 0;

    virtual int bandCount() const;

    virtual QGis::DataType dataType( int bandNo ) const;

    virtual QString type() const { return mType; }

    virtual bool setInput( QgsRasterInterface* input );

    virtual QgsRasterBlock *block( int bandNo, const QgsRectangle &extent, int width, int height ) = 0;

    bool usesTransparency() const;

    void setOpacity( double opacity ) { mOpacity = opacity; }
    double opacity() const { return mOpacity; }

    void setRasterTransparency( QgsRasterTransparency* t );
    const QgsRasterTransparency* rasterTransparency() const { return mRasterTransparency; }

    void setAlphaBand( int band ) { mAlphaBand = band; }
    int alphaBand() const { return mAlphaBand; }

    /**Get symbology items if provided by renderer*/
    virtual void legendSymbologyItems( QList< QPair< QString, QColor > >& symbolItems ) const { Q_UNUSED( symbolItems ); }

    /**Sets base class members from xml. Usually called from create() methods of subclasses*/
    void readXML( const QDomElement& rendererElem );

    /**Returns a list of band numbers used by the renderer*/
    virtual QList<int> usesBands() const { return QList<int>(); }

    static QString minMaxOriginName( int theOrigin );
    static QString minMaxOriginLabel( int theOrigin );
    static int minMaxOriginFromName( QString theName );

  protected:

    /**Write upper class info into rasterrenderer element (called by writeXML method of subclasses)*/
    void _writeXML( QDomDocument& doc, QDomElement& rasterRendererElem ) const;

    QString mType;

    /**Global alpha value (0-1)*/
    double mOpacity;
    /**Raster transparency per color or value. Overwrites global alpha value*/
    QgsRasterTransparency* mRasterTransparency;
    /**Read alpha value from band. Is combined with value from raster transparency / global alpha value.
        Default: -1 (not set)*/
    int mAlphaBand;
};

#endif // QGSRASTERRENDERER_H
