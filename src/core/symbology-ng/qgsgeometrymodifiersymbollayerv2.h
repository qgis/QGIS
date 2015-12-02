#ifndef QGSGEOMETRYMODIFIEDSYMBOLLAYERV2_H
#define QGSGEOMETRYMODIFIEDSYMBOLLAYERV2_H

#include "qgssymbollayerv2.h"

// template
class CORE_EXPORT QgsPolygonGeneratorSymbolLayer : public QgsFillSymbolLayerV2
{
  public:
    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() );

    QgsPolygonGeneratorSymbolLayer( QgsFillSymbolV2* symbol, const QgsStringMap& properties = QgsStringMap() );

    QString layerType() const override;

    void startRender( QgsSymbolV2RenderContext& context ) override;

    void stopRender( QgsSymbolV2RenderContext& context ) override;

    QgsSymbolLayerV2* clone() const override;

    QgsStringMap properties() const override;

    void drawPreviewIcon( QgsSymbolV2RenderContext& context, QSize size ) override;

    void setGeometryModifier( const QString& exp );

    QString geometryModifier() const { return mExpression->expression(); }

    virtual QgsSymbolV2* subSymbol() const override { return mSymbol; }

    virtual bool setSubSymbol( QgsSymbolV2* symbol ) override;

    QgsExpressionContext* expressionContext();

    virtual QSet<QString> usedAttributes() const override;

    //! Will always return true.
    //! This is a hybrid layer, it constructs its own geometry so it does not
    //! care about the geometry of its parents.
    bool isCompatibleWithSymbol( QgsSymbolV2* symbol ) override;

    void renderPolygon( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context ) override;

  private:
    QScopedPointer<QgsExpression> mExpression;
    QgsFillSymbolV2* mSymbol;
};

#endif // QGSGEOMETRYMODIFIEDSYMBOLLAYERV2_H
