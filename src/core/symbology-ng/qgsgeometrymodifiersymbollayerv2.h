#ifndef QGSGEOMETRYMODIFIEDSYMBOLLAYERV2_H
#define QGSGEOMETRYMODIFIEDSYMBOLLAYERV2_H

#include "qgssymbollayerv2.h"

// template
class CORE_EXPORT QgsPolygonGeneratorSymbolLayer : public QgsFillSymbolLayerV2
{
  public:
    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() );

    QgsPolygonGeneratorSymbolLayer( QgsSymbolV2* symbol, const QgsStringMap& properties = QgsStringMap() );

    QString layerType() const;

    void startRender( QgsSymbolV2RenderContext& context );

    void stopRender( QgsSymbolV2RenderContext& context );

    QgsSymbolLayerV2* clone() const;

    QgsStringMap properties() const;

    void drawPreviewIcon( QgsSymbolV2RenderContext& context, QSize size );

    void setGeometryModifier( const QString& exp );

    QString geometryModifier() const { return mExpression->expression(); }

    QgsSymbolV2* symbol() const { return mSymbol; }

    QgsExpressionContext* expressionContext();

    virtual QSet<QString> usedAttributes() const;

    //! Will always return true.
    //! This is a hybrid layer, it constructs its own geometry so it does not
    //! care about the geometry of its parents.
    bool isCompatibleWithSymbol( QgsSymbolV2* symbol );

    void renderPolygon( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context );

  private:
    QScopedPointer<QgsExpression> mExpression;
    QgsSymbolV2* mSymbol;
};

#endif // QGSGEOMETRYMODIFIEDSYMBOLLAYERV2_H
