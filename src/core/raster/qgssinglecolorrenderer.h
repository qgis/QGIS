/***************************************************************************
                         qgssinglecolorrenderer.h
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

#ifndef QGSSINGLECOLORRENDERER_H
#define QGSSINGLECOLORRENDERER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsrasterrenderer.h"

#include <memory>

class QDomElement;

/**
 * \ingroup core
  * \brief Raster single color renderer pipe.
  * \since QGIS 3.38
  */
class CORE_EXPORT QgsSingleColorRenderer: public QgsRasterRenderer
{
  public:

    //! Creates a single \a color renderer
    QgsSingleColorRenderer( QgsRasterInterface *input, QColor color );

    //! QgsSingleColorRenderer cannot be copied. Use clone() instead.
    QgsSingleColorRenderer( const QgsSingleColorRenderer & ) = delete;
    //! QgsSingleColorRenderer cannot be copied. Use clone() instead.
    const QgsSingleColorRenderer &operator=( const QgsSingleColorRenderer & ) = delete;

    QgsSingleColorRenderer *clone() const override SIP_FACTORY;
    Qgis::RasterRendererFlags flags() const override;

    //! Creates an instance of the renderer based on definition from XML (used by the renderer registry)
    static QgsRasterRenderer *create( const QDomElement &elem, QgsRasterInterface *input ) SIP_FACTORY;

    QgsRasterBlock *block( int bandNo, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback *feedback = nullptr ) override SIP_FACTORY;

    /**
     * Returns the single color used by the renderer.
     */
    QColor color() const;

    /**
     * Sets the single color used by the renderer.
     */
    void setColor( QColor &color );

    void writeXml( QDomDocument &doc, QDomElement &parentElem ) const override;

    QList<int> usesBands() const override;

  private:
#ifdef SIP_RUN
    QgsSingleColorRenderer( const QgsSingleColorRenderer & );
    const QgsSingleColorRenderer &operator=( const QgsSingleColorRenderer & );
#endif

    QColor mColor;
};

#endif // QGSSINGLECOLORRENDERER_H
