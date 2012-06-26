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

#include "qgsrasterinterface.h"
#include "qgsrasterdataprovider.h"
#include <QPair>

class QPainter;
class QgsMapToPixel;
class QgsRasterResampler;
class QgsRasterProjector;
class QgsRasterTransparency;
struct QgsRasterViewPort;

class QDomElement;

class CORE_EXPORT QgsRasterRenderer : public QgsRasterInterface
{
  public:
    QgsRasterRenderer( QgsRasterInterface* input, const QString& type );
    virtual ~QgsRasterRenderer();

    virtual QString type() const { return mType; }
    //virtual void draw( QPainter* p, QgsRasterViewPort* viewPort, const QgsMapToPixel* theQgsMapToPixel ) = 0;

    virtual void * readBlock( int bandNo, QgsRectangle  const & extent, int width, int height )
    {
      Q_UNUSED( bandNo ); Q_UNUSED( extent ); Q_UNUSED( width ); Q_UNUSED( height );
      return 0;
    }

    bool usesTransparency() const;

    void setOpacity( double opacity ) { mOpacity = opacity; }
    double opacity() const { return mOpacity; }

    void setRasterTransparency( QgsRasterTransparency* t );
    const QgsRasterTransparency* rasterTransparency() const { return mRasterTransparency; }

    void setAlphaBand( int band ) { mAlphaBand = band; }
    int alphaBand() const { return mAlphaBand; }

    void setInvertColor( bool invert ) { mInvertColor = invert; }
    bool invertColor() const { return mInvertColor; }

    /**Get symbology items if provided by renderer*/
    virtual void legendSymbologyItems( QList< QPair< QString, QColor > >& symbolItems ) const { Q_UNUSED( symbolItems ); }

    virtual void writeXML( QDomDocument& doc, QDomElement& parentElem ) const = 0;

    /**Sets base class members from xml. Usually called from create() methods of subclasses*/
    void readXML( const QDomElement& rendererElem );

  protected:
    inline double readValue( void *data, QgsRasterInterface::DataType type, int index );

    /**Write upper class info into rasterrenderer element (called by writeXML method of subclasses)*/
    void _writeXML( QDomDocument& doc, QDomElement& rasterRendererElem ) const;


    QgsRasterInterface* mProvider;
    QString mType;
    /**Resampler used if screen resolution is higher than raster resolution (zoomed in). 0 means no resampling (nearest neighbour)*/
    QgsRasterResampler* mZoomedInResampler;
    /**Resampler used if raster resolution is higher than raster resolution (zoomed out). 0 mean no resampling (nearest neighbour)*/
    QgsRasterResampler* mZoomedOutResampler;
    //QMap<int, RasterPartInfo> mRasterPartInfos;

    /**Global alpha value (0-1)*/
    double mOpacity;
    /**Raster transparency per color or value. Overwrites global alpha value*/
    QgsRasterTransparency* mRasterTransparency;
    /**Read alpha value from band. Is combined with value from raster transparency / global alpha value.
        Default: -1 (not set)*/
    int mAlphaBand;

    bool mInvertColor;

    /**Maximum boundary for oversampling (to avoid too much data traffic). Default: 2.0*/
    double mMaxOversampling;

  private:
    /**Remove part into and release memory*/
    void removePartInfo( int bandNumer );
    void projectImage( const QImage& srcImg, QImage& dstImage, QgsRasterProjector* prj ) const;
};

inline double QgsRasterRenderer::readValue( void *data, QgsRasterInterface::DataType type, int index )
{
  if ( !mInput )
  {
    return 0;
  }

  if ( !data )
  {
    // TODO
    //return mInput->noDataValue();
    return 0;
  }

  switch ( type )
  {
    case QgsRasterInterface::Byte:
      return ( double )(( GByte * )data )[index];
      break;
    case QgsRasterInterface::UInt16:
      return ( double )(( GUInt16 * )data )[index];
      break;
    case QgsRasterInterface::Int16:
      return ( double )(( GInt16 * )data )[index];
      break;
    case QgsRasterInterface::UInt32:
      return ( double )(( GUInt32 * )data )[index];
      break;
    case QgsRasterInterface::Int32:
      return ( double )(( GInt32 * )data )[index];
      break;
    case QgsRasterInterface::Float32:
      return ( double )(( float * )data )[index];
      break;
    case QgsRasterInterface::Float64:
      return ( double )(( double * )data )[index];
      break;
    default:
      //QgsMessageLog::logMessage( tr( "GDAL data type %1 is not supported" ).arg( type ), tr( "Raster" ) );
      break;
  }

  return mInput->noDataValue();
}

#endif // QGSRASTERRENDERER_H
