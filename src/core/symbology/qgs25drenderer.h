/***************************************************************************
  qgs25drenderer.h - Qgs25DRenderer
  ---------------------------------

 begin                : 14.1.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGS25DRENDERER_H
#define QGS25DRENDERER_H

#include "qgis_core.h"
#include "qgsrenderer.h"

class QgsOuterGlowEffect;

/**
 * \ingroup core
 * \class Qgs25DRenderer
 */
class CORE_EXPORT Qgs25DRenderer : public QgsFeatureRenderer
{
  public:
    Qgs25DRenderer();

    /**
     * Create a new 2.5D renderer from XML
     *
     * \param element XML information
     * \param context reading context
     */
    static QgsFeatureRenderer *create( QDomElement &element, const QgsReadWriteContext &context ) SIP_FACTORY;
    QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context ) override;

    void startRender( QgsRenderContext &context, const QgsFields &fields ) override;
    void stopRender( QgsRenderContext &context ) override;

    QSet<QString> usedAttributes( const QgsRenderContext &context ) const override;
    QgsFeatureRenderer *clone() const override SIP_FACTORY;

    QgsSymbol *symbolForFeature( const QgsFeature &feature, QgsRenderContext &context ) const override;
    QgsSymbolList symbols( QgsRenderContext &context ) const override;

    /**
     * Gets the roof color
     */
    QColor roofColor() const;

    /**
     * Set the roof color
     */
    void setRoofColor( const QColor &roofColor );

    /**
     * Gets the wall color
     */
    QColor wallColor() const;

    /**
     * Set the wall color
     */
    void setWallColor( const QColor &wallColor );

    /**
     * Set wall shading enabled
     */
    void setWallShadingEnabled( bool enabled );

    /**
     * Gets wall shading enabled
     */
    bool wallShadingEnabled() const;

    /**
     * Gets the shadow's color
     */
    QColor shadowColor() const;

    /**
     * Set the shadow's color
     */
    void setShadowColor( const QColor &shadowColor );

    /**
     * Gets the shadow's spread distance in map units
     */
    double shadowSpread() const;

    /**
     * Set the shadow's spread distance in map units
     */
    void setShadowSpread( double shadowSpread );

    /**
     * Try to convert from an existing renderer. If it is not of the same type
     * we assume that the internals are not compatible and create a new default
     * 2.5D renderer.
     */
    static Qgs25DRenderer *convertFromRenderer( QgsFeatureRenderer *renderer ) SIP_FACTORY;

    /**
     * Is the shadow enabled
     */
    bool shadowEnabled() const;

    /**
     * Enable or disable the shadow
     */
    void setShadowEnabled( bool value );

  private:

    QgsFillSymbolLayer *roofLayer() const;
    QgsFillSymbolLayer *wallLayer() const;
    QgsOuterGlowEffect *glowEffect() const;

    std::unique_ptr<QgsSymbol> mSymbol;
};

#endif // QGS25DRENDERER_H
