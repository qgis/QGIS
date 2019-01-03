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
#include "qgssymbollayer.h"

/**
 * \ingroup core
 * \class QgsGeometryGeneratorSymbolLayer
 */
class CORE_EXPORT QgsGeometryGeneratorSymbolLayer : public QgsSymbolLayer
{
  public:
    ~QgsGeometryGeneratorSymbolLayer() override;

    static QgsSymbolLayer *create( const QgsStringMap &properties ) SIP_FACTORY;

    QString layerType() const override;

    /**
     * Set the type of symbol which should be created.
     * Should match with the return type of the expression.
     *
     * \param symbolType The symbol type which shall be used below this symbol.
     */
    void setSymbolType( QgsSymbol::SymbolType symbolType );

    /**
     * Access the symbol type. This defines the type of geometry
     * that is created by this generator.
     *
     * \returns Symbol type
     */
    QgsSymbol::SymbolType symbolType() const { return mSymbolType; }

    void startRender( QgsSymbolRenderContext &context ) override;

    void stopRender( QgsSymbolRenderContext &context ) override;

    QgsSymbolLayer *clone() const override SIP_FACTORY;

    QgsStringMap properties() const override;

    void drawPreviewIcon( QgsSymbolRenderContext &context, QSize size ) override;

    /**
     * Set the expression to generate this geometry.
     */
    void setGeometryExpression( const QString &exp );

    /**
     * Gets the expression to generate this geometry.
     */
    QString geometryExpression() const { return mExpression->expression(); }

    QgsSymbol *subSymbol() override { return mSymbol; }

    bool setSubSymbol( QgsSymbol *symbol SIP_TRANSFER ) override;

    QSet<QString> usedAttributes( const QgsRenderContext &context ) const override;

    /**
     * Will always return true.
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
     * \param context The rendering context which will be used to render and to
     *                construct a geometry.
     */
    virtual void render( QgsSymbolRenderContext &context );

    void setColor( const QColor &color ) override;

  private:
    QgsGeometryGeneratorSymbolLayer( const QString &expression );

#ifdef SIP_RUN
    QgsGeometryGeneratorSymbolLayer( const QgsGeometryGeneratorSymbolLayer &copy );
#endif

    std::unique_ptr<QgsExpression> mExpression;
    QgsFillSymbol *mFillSymbol = nullptr;
    QgsLineSymbol *mLineSymbol = nullptr;
    QgsMarkerSymbol *mMarkerSymbol = nullptr;
    QgsSymbol *mSymbol = nullptr;

    /**
     * The type of the sub symbol.
     */
    QgsSymbol::SymbolType mSymbolType;
};

#endif // QGSGEOMETRYGENERATORSYMBOLLAYER_H
