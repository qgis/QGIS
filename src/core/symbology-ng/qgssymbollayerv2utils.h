

#ifndef QGSSYMBOLLAYERV2UTILS_H
#define QGSSYMBOLLAYERV2UTILS_H

#include <QMap>
#include <Qt>

typedef QMap<QString, QString> QgsStringMap;

class QgsSymbolV2;
class QgsSymbolLayerV2;
class QgsVectorColorRampV2;

class QColor;
class QIcon;
class QPixmap;
class QPointF;
class QSize;

class QgsSymbolLayerV2Utils
{
public:

  static QString encodeColor(QColor color);
  static QColor decodeColor(QString str);

  static QString encodePenStyle(Qt::PenStyle style);
  static Qt::PenStyle decodePenStyle(QString str);

  static QString encodeBrushStyle(Qt::BrushStyle style);
  static Qt::BrushStyle decodeBrushStyle(QString str);

  static QString encodePoint(QPointF point);
  static QPointF decodePoint(QString str);
  
  static QIcon symbolPreviewIcon(QgsSymbolV2* symbol, QSize size);
  static QIcon symbolLayerPreviewIcon(QgsSymbolLayerV2* layer, QSize size);
  static QIcon colorRampPreviewIcon(QgsVectorColorRampV2* ramp, QSize size);

  static QPixmap colorRampPreviewPixmap(QgsVectorColorRampV2* ramp, QSize size);
};

class QPolygonF;

//! calculate line shifted by a specified distance
QPolygonF offsetLine(QPolygonF polyline, double dist);


#endif
