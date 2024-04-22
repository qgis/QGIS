/***************************************************************************
                         qgsrastersinglecolorrenderer.h
                         ---------------------------
    begin                : April 2024
    copyright            : (C) 2024 by Mathieu Pellerin
    email                : mathieu at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRASTERSINGLECOLORRENDERER_H
#define QGSRASTERSINGLECOLORRENDERER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsrasterrenderer.h"

#include <memory>

class QDomElement;

/**
 * \ingroup core
  * \brief Raster renderer which renders all data pixels using a single color.
  * \since QGIS 3.38
  */
class CORE_EXPORT QgsRasterSingleColorRenderer: public QgsRasterRenderer
{
  public:

    //! Creates a single \a color renderer
    QgsRasterSingleColorRenderer( QgsRasterInterface *input, int band, const QColor &color );

    //! QgsRasterSingleColorRenderer cannot be copied. Use clone() instead.
    QgsRasterSingleColorRenderer( const QgsRasterSingleColorRenderer & ) = delete;
    //! QgsRasterSingleColorRenderer cannot be copied. Use clone() instead.
    const QgsRasterSingleColorRenderer &operator=( const QgsRasterSingleColorRenderer & ) = delete;

    QgsRasterSingleColorRenderer *clone() const override SIP_FACTORY;
    Qgis::RasterRendererFlags flags() const override;

    //! Creates an instance of the renderer based on definition from XML (used by the renderer registry)
    static QgsRasterRenderer *create( const QDomElement &elem, QgsRasterInterface *input ) SIP_FACTORY;

    QgsRasterBlock *block( int bandNo, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback *feedback = nullptr ) override SIP_FACTORY;

    /**
     * Returns the single color used by the renderer.
     * \see setColor()
     */
    QColor color() const;

    /**
     * Sets the single \a color used by the renderer.
     * \see color()
     */
    void setColor( const QColor &color );

    void writeXml( QDomDocument &doc, QDomElement &parentElem ) const override;

    int inputBand() const override;
    bool setInputBand( int band ) override;
    QList<int> usesBands() const override;

  private:
#ifdef SIP_RUN
    QgsRasterSingleColorRenderer( const QgsRasterSingleColorRenderer & );
    const QgsRasterSingleColorRenderer &operator=( const QgsRasterSingleColorRenderer & );
#endif

    int mInputBand = -1;
    QColor mColor;
};

#endif // QGSRASTERSINGLECOLORRENDERER_H
