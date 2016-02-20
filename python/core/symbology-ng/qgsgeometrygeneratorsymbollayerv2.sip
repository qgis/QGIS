/***************************************************************************
 qgsgeometrygeneratorsymbollayerv2.sip
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
class QgsGeometryGeneratorSymbolLayerV2 : QgsSymbolLayerV2
{
%TypeHeaderCode
#include "qgsgeometrygeneratorsymbollayerv2.h"
%End
  public:
    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() );

    QString layerType() const;

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
    QgsSymbolV2::SymbolType symbolType() const;

    void startRender( QgsSymbolV2RenderContext& context );

    void stopRender( QgsSymbolV2RenderContext& context );

    QgsSymbolLayerV2* clone() const;

    QgsStringMap properties() const;

    void drawPreviewIcon( QgsSymbolV2RenderContext& context, QSize size );

    /**
     * Set the expression to generate this geometry.
     */
    void setGeometryExpression( const QString& exp );

    /**
     * Get the expression to generate this geometry.
     */
    QString geometryExpression() const;

    virtual QgsSymbolV2* subSymbol();

    virtual bool setSubSymbol( QgsSymbolV2* symbol );

    virtual QSet<QString> usedAttributes() const;

    //! Will always return true.
    //! This is a hybrid layer, it constructs its own geometry so it does not
    //! care about the geometry of its parents.
    bool isCompatibleWithSymbol( QgsSymbolV2* symbol );

    /**
     * Will render this symbol layer using the context.
     * In comparison to other symbols there is no geometry passed in, since
     * the geometry will be created based on information from the context
     * which contains a QgsRenderContext which in turn contains an expression
     * context which is available to the evaluated expression.
     *
     * @param context The rendering context which will be used to render and to
     *                construct a geometry.
     */
    virtual void render( QgsSymbolV2RenderContext& context );

    void setColor( const QColor& color );

  private:
    QgsGeometryGeneratorSymbolLayerV2( const QgsGeometryGeneratorSymbolLayerV2& copy );
};
