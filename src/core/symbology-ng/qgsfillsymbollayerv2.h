
#ifndef QGSFILLSYMBOLLAYERV2_H
#define QGSFILLSYMBOLLAYERV2_H

#include "qgssymbollayerv2.h"

#define DEFAULT_SIMPLEFILL_COLOR        QColor(0,0,255)
#define DEFAULT_SIMPLEFILL_STYLE        Qt::SolidPattern
#define DEFAULT_SIMPLEFILL_BORDERCOLOR  QColor(0,0,0)

#include <QPen>
#include <QBrush>

class CORE_EXPORT QgsSimpleFillSymbolLayerV2 : public QgsFillSymbolLayerV2
{
  public:
    QgsSimpleFillSymbolLayerV2( QColor color = DEFAULT_SIMPLEFILL_COLOR,
                                QColor borderColor = DEFAULT_SIMPLEFILL_BORDERCOLOR,
                                Qt::BrushStyle style = DEFAULT_SIMPLEFILL_STYLE );

    // static stuff

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() );

    // implemented from base classes

    QString layerType() const;

    void startRender( QgsRenderContext& context );

    void stopRender( QgsRenderContext& context );

    void renderPolygon( const QPolygonF& points, QList<QPolygonF>* rings, QgsRenderContext& context );

    QgsStringMap properties() const;

    QgsSymbolLayerV2* clone() const;

    Qt::BrushStyle brushStyle() const { return mBrushStyle; }
    void setBrushStyle( Qt::BrushStyle style ) { mBrushStyle = style; }

    QColor borderColor() const { return mBorderColor; }
    void setBorderColor( QColor borderColor ) { mBorderColor = borderColor; }

  protected:
    QBrush mBrush;
    Qt::BrushStyle mBrushStyle;
    QColor mBorderColor;
    QPen mPen;
};

#endif
