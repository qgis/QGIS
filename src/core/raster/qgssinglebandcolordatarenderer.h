/***************************************************************************
                         qgssinglebandcolordatarenderer.h
                         --------------------------------
    begin                : January 2012
    copyright            : (C) 2012 by Marco Hugentobler
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

#ifndef QGSSINGLEBANDCOLORDATARENDERER_H
#define QGSSINGLEBANDCOLORDATARENDERER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsrasterrenderer.h"

class QDomElement;

/**
 * \ingroup core
  * Raster renderer pipe for single band color.
  */
class CORE_EXPORT QgsSingleBandColorDataRenderer: public QgsRasterRenderer
{
  public:
    QgsSingleBandColorDataRenderer( QgsRasterInterface *input, int band );

    //! QgsSingleBandColorDataRenderer cannot be copied. Use clone() instead.
    QgsSingleBandColorDataRenderer( const QgsSingleBandColorDataRenderer & ) = delete;
    //! QgsSingleBandColorDataRenderer cannot be copied. Use clone() instead.
    const QgsSingleBandColorDataRenderer &operator=( const QgsSingleBandColorDataRenderer & ) = delete;

    QgsSingleBandColorDataRenderer *clone() const override SIP_FACTORY;

    static QgsRasterRenderer *create( const QDomElement &elem, QgsRasterInterface *input ) SIP_FACTORY;

    bool setInput( QgsRasterInterface *input ) override;

    QgsRasterBlock *block( int bandNo, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback *feedback = nullptr ) override SIP_FACTORY;

    void writeXml( QDomDocument &doc, QDomElement &parentElem ) const override;

    QList<int> usesBands() const override;

  private:
#ifdef SIP_RUN
    QgsSingleBandColorDataRenderer( const QgsSingleBandColorDataRenderer & );
    const QgsSingleBandColorDataRenderer &operator=( const QgsSingleBandColorDataRenderer & );
#endif
    int mBand;

};

#endif // QGSSINGLEBANDCOLORDATARENDERER_H
