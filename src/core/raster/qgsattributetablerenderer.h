/***************************************************************************
  qgsattributetablerenderer.h - QgsAttributeTableRenderer

 ---------------------
 begin                : 6.12.2021
 copyright            : (C) 2021 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSATTRIBUTETABLERENDERER_H
#define QGSATTRIBUTETABLERENDERER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsrectangle.h"
#include "qgsrastershader.h"
#include "qgsrastershaderfunction.h"
#include "qgsrasterrenderer.h"

class QDomElement;
class QgsLayerTreeLayer;
class QgsStyleEntityVisitorInterface;
class QgsLayerTreeModelLegendNode;

class CORE_EXPORT QgsAttributeTableRenderer : public  QgsRasterRenderer
{
  public:
    QgsAttributeTableRenderer( QgsRasterInterface *input, int band = -1, QgsRasterShader *shader SIP_TRANSFER = nullptr );

    //! QgsAttributeTableRenderer cannot be copied. Use clone() instead.
    QgsAttributeTableRenderer( const QgsAttributeTableRenderer & ) = delete;
    //! QgsAttributeTableRenderer cannot be copied. Use clone() instead.
    const QgsAttributeTableRenderer &operator=( const QgsAttributeTableRenderer & ) = delete;

    QgsAttributeTableRenderer *clone() const override SIP_FACTORY;

    static QgsRasterRenderer *create( const QDomElement &elem, QgsRasterInterface *input ) SIP_FACTORY;

    QgsRasterBlock *block( int bandNo, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback *feedback = nullptr ) override SIP_FACTORY;

    void writeXml( QDomDocument &doc, QDomElement &parentElem ) const override;
    QList< QPair< QString, QColor > > legendSymbologyItems() const override;
    QList<QgsLayerTreeModelLegendNode *> createLegendNodes( QgsLayerTreeLayer *nodeLayer ) SIP_FACTORY override;
    QList<int> usesBands() const override;
    void toSld( QDomDocument &doc, QDomElement &element, const QVariantMap &props = QVariantMap() ) const override;
    bool accept( QgsStyleEntityVisitorInterface *visitor ) const override;

    /**
     * Returns the band used by the renderer
     */
    int band() const { return mBand; }

    /**
     * Sets the band used by the renderer.
     * \see band
     */
    void setBand( int bandNo );

  private:
#ifdef SIP_RUN
    QgsAttributeTableRenderer( const QgsAttributeTableRenderer & );
    const QgsAttributeTableRenderer &operator=( const QgsAttributeTableRenderer & );
#endif

    std::unique_ptr< QgsRasterShader > mShader;
    int mBand;

};

#endif // QGSATTRIBUTETABLERENDERER_H
