
#ifndef QGSFILLSYMBOLLAYERV2_H
#define QGSFILLSYMBOLLAYERV2_H

#include "qgssymbollayerv2.h"

#define DEFAULT_SIMPLEFILL_COLOR        QColor(0,0,255)
#define DEFAULT_SIMPLEFILL_STYLE        Qt::SolidPattern
#define DEFAULT_SIMPLEFILL_BORDERCOLOR  QColor(0,0,0)
#define DEFAULT_SIMPLEFILL_BORDERSTYLE  Qt::SolidLine
#define DEFAULT_SIMPLEFILL_BORDERWIDTH  DEFAULT_LINE_WIDTH

#include <QPen>
#include <QBrush>

class CORE_EXPORT QgsSimpleFillSymbolLayerV2 : public QgsFillSymbolLayerV2
{
  public:
    QgsSimpleFillSymbolLayerV2( QColor color = DEFAULT_SIMPLEFILL_COLOR,
                                Qt::BrushStyle style = DEFAULT_SIMPLEFILL_STYLE,
                                QColor borderColor = DEFAULT_SIMPLEFILL_BORDERCOLOR,
                                Qt::PenStyle borderStyle = DEFAULT_SIMPLEFILL_BORDERSTYLE,
                                double borderWidth = DEFAULT_SIMPLEFILL_BORDERWIDTH );

    // static stuff

    static QgsSymbolLayerV2* create( const QgsStringMap& properties = QgsStringMap() );

    // implemented from base classes

    QString layerType() const;

    void startRender( QgsSymbolV2RenderContext& context );

    void stopRender( QgsSymbolV2RenderContext& context );

    void renderPolygon( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context );

    QgsStringMap properties() const;

    QgsSymbolLayerV2* clone() const;

    Qt::BrushStyle brushStyle() const { return mBrushStyle; }
    void setBrushStyle( Qt::BrushStyle style ) { mBrushStyle = style; }

    QColor borderColor() const { return mBorderColor; }
    void setBorderColor( QColor borderColor ) { mBorderColor = borderColor; }

    Qt::PenStyle borderStyle() const { return mBorderStyle; }
    void setBorderStyle( Qt::PenStyle borderStyle ) { mBorderStyle = borderStyle; }

    double borderWidth() const { return mBorderWidth; }
    void setBorderWidth( double borderWidth ) { mBorderWidth = borderWidth; }

  protected:
    QBrush mBrush;
    Qt::BrushStyle mBrushStyle;
    QColor mBorderColor;
    Qt::PenStyle mBorderStyle;
    double mBorderWidth;
    QPen mPen;
};

#endif
