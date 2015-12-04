/***************************************************************************
 qgslinegeneratorsymbollayerv2.sip
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
class QgsLineGeneratorSymbolLayerV2 : QgsLineSymbolLayerV2
{
%TypeHeaderCode
#include "qgslinegeneratorsymbollayerv2.h"
%End
  public:
    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() );

    QgsLineGeneratorSymbolLayerV2( QgsLineSymbolV2* symbol, const QgsStringMap& properties = QgsStringMap() );

    QString layerType() const;

    void startRender( QgsSymbolV2RenderContext& context );

    void stopRender( QgsSymbolV2RenderContext& context );

    QgsSymbolLayerV2* clone() const;

    QgsStringMap properties() const;

    void drawPreviewIcon( QgsSymbolV2RenderContext& context, QSize size );

    void setGeometryExpression( const QString& exp );

    QString geometryExpression() const;

    virtual QgsSymbolV2* subSymbol();

    virtual bool setSubSymbol( QgsSymbolV2* symbol );

    virtual QSet<QString> usedAttributes() const;

    //! Will always return true.
    //! This is a hybrid layer, it constructs its own geometry so it does not
    //! care about the geometry of its parents.
    bool isCompatibleWithSymbol( QgsSymbolV2* symbol );

    virtual void renderPolyline( const QPolygonF& point, QgsSymbolV2RenderContext& context );

  private:
    QgsLineGeneratorSymbolLayerV2( const QgsLineGeneratorSymbolLayerV2& copy );
};
