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

#include "qgsrasterinterface.h"
#include "qgsrasterminmaxorigin.h"

class QDomElement;

class QPainter;
class QgsRasterTransparency;

/** \ingroup core
  * Raster renderer pipe that applies colors to a raster.
  */
class CORE_EXPORT QgsRasterRenderer : public QgsRasterInterface
{

    Q_DECLARE_TR_FUNCTIONS( QgsRasterRenderer );

  public:

    static const QRgb NODATA_COLOR;

    QgsRasterRenderer( QgsRasterInterface* input = nullptr, const QString& type = "" );
    virtual ~QgsRasterRenderer();

    QgsRasterRenderer( const QgsRasterRenderer& ) = delete;
    const QgsRasterRenderer& operator=( const QgsRasterRenderer& ) = delete;

    QgsRasterRenderer * clone() const override = 0;

    virtual int bandCount() const override;

    virtual Qgis::DataType dataType( int bandNo ) const override;

    virtual QString type() const { return mType; }

    virtual bool setInput( QgsRasterInterface* input ) override;

    virtual QgsRasterBlock *block( int bandNo, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback* feedback = nullptr ) override = 0;

    bool usesTransparency() const;

    void setOpacity( double opacity ) { mOpacity = opacity; }
    double opacity() const { return mOpacity; }

    void setRasterTransparency( QgsRasterTransparency* t );
    const QgsRasterTransparency* rasterTransparency() const { return mRasterTransparency; }

    void setAlphaBand( int band ) { mAlphaBand = band; }
    int alphaBand() const { return mAlphaBand; }

    //! Get symbology items if provided by renderer
    virtual void legendSymbologyItems( QList< QPair< QString, QColor > >& symbolItems ) const { Q_UNUSED( symbolItems ); }

    //! Sets base class members from xml. Usually called from create() methods of subclasses
    void readXml( const QDomElement& rendererElem ) override;

    /** Copies common properties like opacity / transparency data from other renderer.
     *  Useful when cloning renderers.
     *  @note added in 2.16  */
    void copyCommonProperties( const QgsRasterRenderer* other, bool copyMinMaxOrigin = true );

    //! Returns a list of band numbers used by the renderer
    virtual QList<int> usesBands() const { return QList<int>(); }

    //! Returns const reference to origin of min/max values
    const QgsRasterMinMaxOrigin& minMaxOrigin() const { return mMinMaxOrigin; }

    //! Sets origin of min/max values
    void setMinMaxOrigin( const QgsRasterMinMaxOrigin& theOrigin ) { mMinMaxOrigin = theOrigin; }

  protected:

    //! Write upper class info into rasterrenderer element (called by writeXml method of subclasses)
    void _writeXml( QDomDocument& doc, QDomElement& rasterRendererElem ) const;

    QString mType;

    //! Global alpha value (0-1)
    double mOpacity;
    //! Raster transparency per color or value. Overwrites global alpha value
    QgsRasterTransparency* mRasterTransparency;

    /** Read alpha value from band. Is combined with value from raster transparency / global alpha value.
        Default: -1 (not set)*/
    int mAlphaBand;

    //! Origin of min/max values
    QgsRasterMinMaxOrigin mMinMaxOrigin;

};

#endif // QGSRASTERRENDERER_H
