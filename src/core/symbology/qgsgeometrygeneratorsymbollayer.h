/***************************************************************************
 qgsgeometrygeneratorsymbollayer.h
 ---------------------
 begin                : November 2015
 copyright            : (C) 2015 by Matthias Kuhn
 email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGEOMETRYGENERATORSYMBOLLAYER_H
#define QGSGEOMETRYGENERATORSYMBOLLAYER_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgssymbollayer.h"

class QgsFillSymbol;
class QgsLineSymbol;
class QgsMarkerSymbol;

/**
 * \ingroup core
 * \class QgsGeometryGeneratorSymbolLayer
 */
class CORE_EXPORT QgsGeometryGeneratorSymbolLayer : public QgsSymbolLayer
{
  public:
    ~QgsGeometryGeneratorSymbolLayer() override;

    //! Creates the symbol layer
    static QgsSymbolLayer *create( const QVariantMap &properties ) SIP_FACTORY;

    QString layerType() const override;

    /**
     * Set the type of symbol which should be created.
     * Should match with the return type of the expression.
     *
     * \param symbolType The symbol type which shall be used below this symbol.
     */
    void setSymbolType( Qgis::SymbolType symbolType );

    /**
     * Access the symbol type. This defines the type of geometry
     * that is created by this generator.
     *
     * \returns Symbol type
     */
    Qgis::SymbolType symbolType() const { return mSymbolType; }

    void startRender( QgsSymbolRenderContext &context ) override;
    void stopRender( QgsSymbolRenderContext &context ) override;
    void startFeatureRender( const QgsFeature &feature, QgsRenderContext &context ) override;
    void stopFeatureRender( const QgsFeature &feature, QgsRenderContext &context ) override;
    bool usesMapUnits() const override;
    QColor color() const override;
    QgsUnitTypes::RenderUnit outputUnit() const override;
    QgsMapUnitScale mapUnitScale() const override;

    QgsSymbolLayer *clone() const override SIP_FACTORY;

    QVariantMap properties() const override;

    void drawPreviewIcon( QgsSymbolRenderContext &context, QSize size ) override;

    /**
     * Set the expression to generate this geometry.
     */
    void setGeometryExpression( const QString &exp );

    /**
     * Gets the expression to generate this geometry.
     */
    QString geometryExpression() const { return mExpression->expression(); }

    /**
     * Returns the unit for the geometry expression.
     *
     * By default this is QgsUnitTypes::MapUnits, which means that the geometryExpression()
     * will return geometries in the associated layer's CRS.
     *
     * \see setUnits()
     * \since QGIS 3.22
     */
    QgsUnitTypes::RenderUnit units() const { return mUnits; }

    /**
     * Sets the \a units for the geometry expression.
     *
     * By default this is QgsUnitTypes::MapUnits, which means that the geometryExpression()
     * will return geometries in the associated layer's CRS.
     *
     * \see units()
     * \since QGIS 3.22
     */
    void setUnits( QgsUnitTypes::RenderUnit units ) { mUnits = units;}

    QgsSymbol *subSymbol() override { return mSymbol; }

    bool setSubSymbol( QgsSymbol *symbol SIP_TRANSFER ) override;

    QSet<QString> usedAttributes( const QgsRenderContext &context ) const override;
    bool hasDataDefinedProperties() const override;

    /**
     * Will always return TRUE.
     * This is a hybrid layer, it constructs its own geometry so it does not
     * care about the geometry of its parents.
     */
    bool isCompatibleWithSymbol( QgsSymbol *symbol ) const override;

    /**
     * Will render this symbol layer using the context.
     * In comparison to other symbols there is no geometry passed in, since
     * the geometry will be created based on information from the context
     * which contains a QgsRenderContext which in turn contains an expression
     * context which is available to the evaluated expression.
     *
     * \param context The rendering context which will be used to render and to construct a geometry.
     * \param geometryType type of original geometry being rendered by the parent symbol (since QGIS 3.22)
     * \param points optional list of original points which are being rendered by the parent symbol (since QGIS 3.22)
     * \param rings optional list of original rings which are being rendered by the parent symbol (since QGIS 3.22)
     */
    void render( QgsSymbolRenderContext &context, QgsWkbTypes::GeometryType geometryType = QgsWkbTypes::GeometryType::UnknownGeometry, const QPolygonF *points = nullptr, const QVector<QPolygonF> *rings = nullptr );

    void setColor( const QColor &color ) override;

  private:
    QgsGeometryGeneratorSymbolLayer( const QString &expression );

#ifdef SIP_RUN
    QgsGeometryGeneratorSymbolLayer( const QgsGeometryGeneratorSymbolLayer &copy );
#endif

    /**
     * Input geometry must be in painter units!
     */
    QgsGeometry evaluateGeometryInPainterUnits( const QgsGeometry &input, const QgsFeature &feature, const QgsRenderContext &renderContext, QgsExpressionContext &expressionContext ) const;

    /**
     * Tries to coerce the geometry output by the generator expression into
     * a type usable by the symbol.
     */
    QgsGeometry coerceToExpectedType( const QgsGeometry &geometry ) const;

    std::unique_ptr<QgsExpression> mExpression;
    std::unique_ptr<QgsFillSymbol> mFillSymbol;
    std::unique_ptr<QgsLineSymbol> mLineSymbol;
    std::unique_ptr<QgsMarkerSymbol> mMarkerSymbol;
    QgsSymbol *mSymbol = nullptr;

    /**
     * The type of the sub symbol.
     */
    Qgis::SymbolType mSymbolType;

    QgsUnitTypes::RenderUnit mUnits = QgsUnitTypes::RenderMapUnits;

    bool mRenderingFeature = false;
    bool mHasRenderedFeature = false;
};

#endif // QGSGEOMETRYGENERATORSYMBOLLAYER_H
