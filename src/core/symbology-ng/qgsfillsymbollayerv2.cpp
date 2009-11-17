
#include "qgsfillsymbollayerv2.h"
#include "qgssymbollayerv2utils.h"

#include "qgsrendercontext.h"

#include <QPainter>

QgsSimpleFillSymbolLayerV2::QgsSimpleFillSymbolLayerV2(QColor color, Qt::BrushStyle style, QColor borderColor, Qt::PenStyle borderStyle, double borderWidth)
  : mBrushStyle(style), mBorderColor(borderColor), mBorderStyle(borderStyle), mBorderWidth(borderWidth)
{
  mColor = color;
}


QgsSymbolLayerV2* QgsSimpleFillSymbolLayerV2::create(const QgsStringMap& props)
{
  QColor color = DEFAULT_SIMPLEFILL_COLOR;
  Qt::BrushStyle style = DEFAULT_SIMPLEFILL_STYLE;
  QColor borderColor = DEFAULT_SIMPLEFILL_BORDERCOLOR;
  Qt::PenStyle borderStyle = DEFAULT_SIMPLEFILL_BORDERSTYLE;
  double borderWidth = DEFAULT_SIMPLEFILL_BORDERWIDTH;

  if (props.contains("color"))
    color = QgsSymbolLayerV2Utils::decodeColor(props["color"]);
  if (props.contains("style"))
    style = QgsSymbolLayerV2Utils::decodeBrushStyle(props["style"]);
  if (props.contains("color_border"))
    borderColor = QgsSymbolLayerV2Utils::decodeColor(props["color_border"]);
  if (props.contains("style_border"))
    borderStyle = QgsSymbolLayerV2Utils::decodePenStyle(props["style_border"]);
  if (props.contains("width_border"))
    borderWidth = props["width_border"].toDouble();

  return new QgsSimpleFillSymbolLayerV2(color, style, borderColor, borderStyle, borderWidth);
}


QString QgsSimpleFillSymbolLayerV2::layerType() const
{
  return "SimpleFill";
}

void QgsSimpleFillSymbolLayerV2::startRender(QgsRenderContext& context)
{
  mBrush = QBrush(mColor, mBrushStyle);
  mPen = QPen(mBorderColor);
  mPen.setStyle(mBorderStyle);
  mPen.setWidthF(mBorderWidth);
}

void QgsSimpleFillSymbolLayerV2::stopRender(QgsRenderContext& context)
{
}

void QgsSimpleFillSymbolLayerV2::renderPolygon(const QPolygonF& points, QList<QPolygonF>* rings, QgsRenderContext& context)
{
  QPainter* p = context.painter();
  p->setBrush(mBrush);
  p->setPen(mPen);
  
  if (rings == NULL)
  {
    // simple polygon without holes
    p->drawPolygon(points);
  }
  else
  {
    // polygon with holes must be drawn using painter path
    QPainterPath path;
    path.addPolygon(points);
    QList<QPolygonF>::iterator it;
    for (it = rings->begin(); it != rings->end(); ++it)
      path.addPolygon(*it);
    
    p->drawPath(path);
  }
}

QgsStringMap QgsSimpleFillSymbolLayerV2::properties() const
{
  QgsStringMap map;
  map["color"] = QgsSymbolLayerV2Utils::encodeColor(mColor);
  map["style"] = QgsSymbolLayerV2Utils::encodeBrushStyle(mBrushStyle);
  map["color_border"] = QgsSymbolLayerV2Utils::encodeColor(mBorderColor);
  map["style_border"] = QgsSymbolLayerV2Utils::encodePenStyle(mBorderStyle);
  map["width_border"] = QString::number(mBorderWidth);
  return map;
}

QgsSymbolLayerV2* QgsSimpleFillSymbolLayerV2::clone() const
{
  return new QgsSimpleFillSymbolLayerV2(mColor, mBrushStyle, mBorderColor, mBorderStyle, mBorderWidth);
}
