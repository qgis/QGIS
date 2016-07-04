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

#include "qgsrendererv2.h"
#include "qgsdatadefined.h"

class QgsOuterGlowEffect;

/** \ingroup core
 * \class Qgs25DRenderer
 */
class CORE_EXPORT Qgs25DRenderer : public QgsFeatureRendererV2
{
  public:
    Qgs25DRenderer();

    /**
     * Create a new 2.5D renderer from XML
     *
     * @param element XML information
     */
    static QgsFeatureRendererV2* create( QDomElement& element );
    QDomElement save( QDomDocument& doc ) override;

    void startRender( QgsRenderContext& context, const QgsFields& fields ) override;
    void stopRender( QgsRenderContext& context ) override;

    QList<QString> usedAttributes() override;
    QgsFeatureRendererV2* clone() const override;

    virtual QgsSymbolV2* symbolForFeature( QgsFeature& feature, QgsRenderContext& context ) override;
    virtual QgsSymbolV2List symbols( QgsRenderContext& context ) override;

    /**
     * Get the roof color
     */
    QColor roofColor() const;

    /**
     * Set the roof color
     */
    void setRoofColor( const QColor& roofColor );

    /**
     * Get the wall color
     */
    QColor wallColor() const;

    /**
     * Set the wall color
     */
    void setWallColor( const QColor& wallColor );

    /**
     * Set wall shading enabled
     */
    void setWallShadingEnabled( bool enabled );

    /**
     * Get wall shading enabled
     */
    bool wallShadingEnabled();

    /**
     * Get the shadow's color
     */
    QColor shadowColor() const;

    /**
     * Set the shadow's color
     */
    void setShadowColor( const QColor& shadowColor );

    /**
     * Get the shadow's spread distance in map units
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
    static Qgs25DRenderer* convertFromRenderer( QgsFeatureRendererV2* renderer );

    /**
     * Is the shadow enabled
     */
    bool shadowEnabled() const;
    /**
     * Enable or disable the shadow
     */
    void setShadowEnabled( bool value );

  private:

    QgsFillSymbolLayerV2* roofLayer() const;
    QgsFillSymbolLayerV2* wallLayer() const;
    QgsOuterGlowEffect* glowEffect() const;

    QScopedPointer<QgsSymbolV2> mSymbol;
};

#endif // QGS25DRENDERER_H
