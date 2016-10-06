/***************************************************************************
 qgsgeometrygeneratorsymbollayer.sip
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
class QgsGeometryGeneratorSymbolLayer : QgsSymbolLayer
{
%TypeHeaderCode
#include "qgsgeometrygeneratorsymbollayer.h"
%End
  public:
    static QgsSymbolLayer* create( const QgsStringMap& properties = QgsStringMap() );

    QString layerType() const;

    /**
     * Set the type of symbol which should be created.
     * Should match with the return type of the expression.
     *
     * @param symbolType The symbol type which shall be used below this symbol.
     */
    void setSymbolType( QgsSymbol::SymbolType symbolType );

    /**
     * Access the symbol type. This defines the type of geometry
     * that is created by this generator.
     *
     * @return Symbol type
     */
    QgsSymbol::SymbolType symbolType() const;

    void startRender( QgsSymbolRenderContext& context );

    void stopRender( QgsSymbolRenderContext& context );

    QgsSymbolLayer* clone() const;

    QgsStringMap properties() const;

    void drawPreviewIcon( QgsSymbolRenderContext& context, QSize size );

    /**
     * Set the expression to generate this geometry.
     */
    void setGeometryExpression( const QString& exp );

    /**
     * Get the expression to generate this geometry.
     */
    QString geometryExpression() const;

    virtual QgsSymbol* subSymbol();

    virtual bool setSubSymbol( QgsSymbol* symbol );

    virtual QSet<QString> usedAttributes() const;

    //! Will always return true.
    //! This is a hybrid layer, it constructs its own geometry so it does not
    //! care about the geometry of its parents.
    bool isCompatibleWithSymbol( QgsSymbol* symbol );

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
    virtual void render( QgsSymbolRenderContext& context );

    void setColor( const QColor& color );

  private:
    QgsGeometryGeneratorSymbolLayer( const QgsGeometryGeneratorSymbolLayer& copy );
};
