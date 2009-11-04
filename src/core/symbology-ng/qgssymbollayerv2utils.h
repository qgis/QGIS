

#ifndef QGSSYMBOLLAYERV2UTILS_H
#define QGSSYMBOLLAYERV2UTILS_H

#include <QMap>
#include <Qt>

class QgsSymbolV2;
class QgsSymbolLayerV2;
class QgsVectorColorRampV2;

typedef QMap<QString, QString> QgsStringMap;
typedef QMap<QString, QgsSymbolV2* > QgsSymbolV2Map;

class QColor;
class QDomDocument;
class QDomElement;
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

  static QString encodePenJoinStyle(Qt::PenJoinStyle style);
  static Qt::PenJoinStyle decodePenJoinStyle(QString str);

  static QString encodePenCapStyle(Qt::PenCapStyle style);
  static Qt::PenCapStyle decodePenCapStyle(QString str);

  static QString encodeBrushStyle(Qt::BrushStyle style);
  static Qt::BrushStyle decodeBrushStyle(QString str);

  static QString encodePoint(QPointF point);
  static QPointF decodePoint(QString str);
  
  static QIcon symbolPreviewIcon(QgsSymbolV2* symbol, QSize size);
  static QIcon symbolLayerPreviewIcon(QgsSymbolLayerV2* layer, QSize size);
  static QIcon colorRampPreviewIcon(QgsVectorColorRampV2* ramp, QSize size);

  static QPixmap colorRampPreviewPixmap(QgsVectorColorRampV2* ramp, QSize size);

  static QgsSymbolV2* loadSymbol(QDomElement& element);
  static QgsSymbolLayerV2* loadSymbolLayer(QDomElement& element);
  static QDomElement saveSymbol(QString name, QgsSymbolV2* symbol, QDomDocument& doc, QgsSymbolV2Map* subSymbols = NULL);

  static QgsStringMap parseProperties(QDomElement& element);
  static void saveProperties(QgsStringMap props, QDomDocument& doc, QDomElement& element);

  static QgsSymbolV2Map loadSymbols(QDomElement& element);
  static QDomElement saveSymbols(QgsSymbolV2Map& symbols, QString tagName, QDomDocument& doc);

  static void clearSymbolMap(QgsSymbolV2Map& symbols);

  static QgsVectorColorRampV2* loadColorRamp(QDomElement& element);
  static QDomElement saveColorRamp(QString name, QgsVectorColorRampV2* ramp, QDomDocument& doc);
};

class QPolygonF;

//! calculate line shifted by a specified distance
QPolygonF offsetLine(QPolygonF polyline, double dist);


#endif
