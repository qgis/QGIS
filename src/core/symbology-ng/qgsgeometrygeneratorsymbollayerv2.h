/***************************************************************************
 qgsgeometrygeneratorsymbollayerv2.h
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

#ifndef QGSGEOMETRYGENERATORSYMBOLLAYERV2_H
#define QGSGEOMETRYGENERATORSYMBOLLAYERV2_H

#include "qgssymbollayerv2.h"

class CORE_EXPORT QgsGeometryGeneratorSymbolLayerV2 : public QgsSymbolLayerV2
{
  public:
    ~QgsGeometryGeneratorSymbolLayerV2();

    static QgsSymbolLayerV2* create( const QgsStringMap& properties );

    QString layerType() const override;

    /**
     * Set the type of symbol which should be created.
     * Should match with the return type of the expression.
     *
     * @param symbolType The symbol type which shall be used below this symbol.
     */
    void setSymbolType( QgsSymbolV2::SymbolType symbolType );

    /**
     * Access the symbol type. This defines the type of geometry
     * that is created by this generator.
     *
     * @return Symbol type
     */
    QgsSymbolV2::SymbolType symbolType() const { return mSymbolType; }

    void startRender( QgsSymbolV2RenderContext& context ) override;

    void stopRender( QgsSymbolV2RenderContext& context ) override;

    QgsSymbolLayerV2* clone() const override;

    QgsStringMap properties() const override;

    void drawPreviewIcon( QgsSymbolV2RenderContext& context, QSize size ) override;

    void setGeometryExpression( const QString& exp );

    QString geometryExpression() const { return mExpression->expression(); }

    virtual QgsSymbolV2* subSymbol() override { return mSymbol; }

    virtual bool setSubSymbol( QgsSymbolV2* symbol ) override;

    virtual QSet<QString> usedAttributes() const override;

    //! Will always return true.
    //! This is a hybrid layer, it constructs its own geometry so it does not
    //! care about the geometry of its parents.
    bool isCompatibleWithSymbol( QgsSymbolV2* symbol ) override;

    virtual void render( QgsSymbolV2RenderContext& context );

  private:
    QgsGeometryGeneratorSymbolLayerV2( const QString& expression );

    QScopedPointer<QgsExpression> mExpression;
    QgsFillSymbolV2* mFillSymbol;
    QgsLineSymbolV2* mLineSymbol;
    QgsMarkerSymbolV2* mMarkerSymbol;
    QgsSymbolV2* mSymbol;

    /**
     * The type of the sub symbol.
     */
    QgsSymbolV2::SymbolType mSymbolType;
};

#endif // QGSGEOMETRYGENERATORSYMBOLLAYERV2_H
