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

#include "qgis_core.h"
#include "qgis_sip.h"
#include <QPair>

#include "qgsrasterinterface.h"
#include "qgsrasterminmaxorigin.h"

class QDomElement;

class QPainter;
class QgsRasterTransparency;
class QgsStyleEntityVisitorInterface;
class QgsLayerTreeModelLegendNode;
class QgsLayerTreeLayer;

/**
 * \ingroup core
  * \brief Raster renderer pipe that applies colors to a raster.
  */
class CORE_EXPORT QgsRasterRenderer : public QgsRasterInterface
{

    Q_DECLARE_TR_FUNCTIONS( QgsRasterRenderer )

  public:

    static const QRgb NODATA_COLOR;

    /**
     * Constructor for QgsRasterRenderer.
     */
    QgsRasterRenderer( QgsRasterInterface *input = nullptr, const QString &type = QString() );
    ~QgsRasterRenderer() override;

    //! QgsRasterRenderer cannot be copied. Use clone() instead.
    QgsRasterRenderer( const QgsRasterRenderer & ) = delete;
    //! QgsRasterRenderer cannot be copied. Use clone() instead.
    const QgsRasterRenderer &operator=( const QgsRasterRenderer & ) = delete;

    QgsRasterRenderer *clone() const override = 0 SIP_FACTORY;

    int bandCount() const override;

    Qgis::DataType dataType( int bandNo ) const override;

    /**
     * Returns a unique string representation of the renderer type.
     */
    virtual QString type() const { return mType; }

    /**
     * Returns flags which dictate renderer behavior.
     *
     * \since QGIS 3.28
     */
    virtual Qgis::RasterRendererFlags flags() const;

    /**
     * Returns TRUE if the renderer is suitable for attribute table creation.
     * The default implementation returns FALSE.
     *
     *  \since QGIS 3.30
     */
    virtual bool canCreateRasterAttributeTable( ) const;

    bool setInput( QgsRasterInterface *input ) override;

    /**
     * Returns the input band for the renderer, or -1 if no input band is available.
     *
     * For renderers which utilize multiple input bands -1 will be returned. In these
     * cases usesBands() will return a list of all utilized bands (including alpha
     * bands).
     *
     * \see setInputBand()
     * \see usesBands()
     *
     * \since QGIS 3.38
     */
    virtual int inputBand() const;

    /**
     * Attempts to set the input \a band for the renderer.
     *
     * Returns TRUE if the band was successfully set, or FALSE if the band could not be set.
     *
     * \note Not all renderers support setting the input band.
     *
     * \see inputBand()
     * \see usesBands()
     *
     * \since QGIS 3.38
     */
    virtual bool setInputBand( int band );

    QgsRasterBlock *block( int bandNo,
                           const QgsRectangle &extent,
                           int width,
                           int height,
                           QgsRasterBlockFeedback *feedback = nullptr ) override = 0 SIP_FACTORY;

    bool usesTransparency() const;

    /**
     * Sets the \a opacity for the renderer, where \a opacity is a value between 0 (totally transparent)
     * and 1.0 (fully opaque).
     * \see opacity()
     */
    void setOpacity( double opacity ) { mOpacity = opacity; }

    /**
     * Returns the opacity for the renderer, where opacity is a value between 0 (totally transparent)
     * and 1.0 (fully opaque).
     * \see setOpacity()
     */
    double opacity() const { return mOpacity; }

    /**
     * Returns the color to use for shading nodata pixels.
     *
     * If the returned value is an invalid color then the default transparent rendering of
     * nodata values will be used.
     *
     * \see renderColorForNodataPixel()
     * \see setNodataColor()
     * \since QGIS 3.12
     */
    QColor nodataColor() const { return mNodataColor; }

    /**
     * Sets the \a color to use for shading nodata pixels.
     *
     * If \a color is an invalid color then the default transparent rendering of
     * nodata values will be used.
     *
     * \see nodataColor()
     * \since QGIS 3.12
     */
    void setNodataColor( const QColor &color ) { mNodataColor = color; }

    void setRasterTransparency( QgsRasterTransparency *t SIP_TRANSFER );
    const QgsRasterTransparency *rasterTransparency() const { return mRasterTransparency; }

    void setAlphaBand( int band ) { mAlphaBand = band; }
    int alphaBand() const { return mAlphaBand; }

    /**
     * Returns symbology items if provided by renderer.
     *
     * \see createLegendNodes()
     */
    virtual QList< QPair< QString, QColor > > legendSymbologyItems() const;

    /**
     * Creates a set of legend nodes representing the renderer.
     *
     * The default implementation calls legendSymbologyItems() and creates corresponding legend nodes for each returned
     * symbology item.
     *
     * Subclasses can override this to return more legend nodes which better represent the renderer.
     *
     * \since QGIS 3.18
     */
    virtual QList<QgsLayerTreeModelLegendNode *> createLegendNodes( QgsLayerTreeLayer *nodeLayer ) SIP_FACTORY;

    //! Sets base class members from xml. Usually called from create() methods of subclasses
    void readXml( const QDomElement &rendererElem ) override;

    /**
     * Copies common properties like opacity / transparency data from other renderer.
     * Useful when cloning renderers.
     */
    void copyCommonProperties( const QgsRasterRenderer *other, bool copyMinMaxOrigin = true );

    /**
     * Returns a list of band numbers used by the renderer.
     *
     * \see setInputBand()
     */
    virtual QList<int> usesBands() const { return QList<int>(); }

    //! Returns const reference to origin of min/max values
    const QgsRasterMinMaxOrigin &minMaxOrigin() const { return mMinMaxOrigin; }

    //! Sets origin of min/max values
    void setMinMaxOrigin( const QgsRasterMinMaxOrigin &origin ) { mMinMaxOrigin = origin; }

    /**
     * Used from subclasses to create SLD Rule elements following SLD v1.0 specs
     * \since QGIS 3.6
    */
    virtual void toSld( QDomDocument &doc, QDomElement &element, const QVariantMap &props = QVariantMap() ) const;

    /**
     * Accepts the specified symbology \a visitor, causing it to visit all symbols associated
     * with the renderer.
     *
     * Returns TRUE if the visitor should continue visiting other objects, or FALSE if visiting
     * should be canceled.
     *
     * \since QGIS 3.10
     */
    virtual bool accept( QgsStyleEntityVisitorInterface *visitor ) const;

  protected:

    //! Write upper class info into rasterrenderer element (called by writeXml method of subclasses)
    void _writeXml( QDomDocument &doc, QDomElement &rasterRendererElem ) const;

    QString mType;

    //! Global alpha value (0-1)
    double mOpacity = 1.0;
    //! Raster transparency per color or value. Overwrites global alpha value
    QgsRasterTransparency *mRasterTransparency = nullptr;

    /**
     * Read alpha value from band. Is combined with value from raster transparency / global alpha value.
     * Default: -1 (not set)
    */
    int mAlphaBand = -1;

    //! Origin of min/max values
    QgsRasterMinMaxOrigin mMinMaxOrigin;

    /**
     * Returns the color for the renderer to use to represent nodata pixels.
     *
     * Subclasses should use this rather then nodataColor() to determine the color to use for nodata pixels
     * during an actual rendering operation.
     *
     * \since QGIS 3.10
     */
    QRgb renderColorForNodataPixel() const;

  private:

    QColor mNodataColor;

#ifdef SIP_RUN
    QgsRasterRenderer( const QgsRasterRenderer & );
    const QgsRasterRenderer &operator=( const QgsRasterRenderer & );
#endif

};

#endif // QGSRASTERRENDERER_H
