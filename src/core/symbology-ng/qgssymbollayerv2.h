
#ifndef QGSSYMBOLLAYERV2_H
#define QGSSYMBOLLAYERV2_H

#include <QMap>

#include <QColor>

#include "qgssymbolv2.h"

#include "qgssymbollayerv2utils.h" // QgsStringMap

class QPainter;
class QSize;
class QPointF;
class QPolygonF;

class QgsRenderContext;
class QgsSymbolV2;



class QgsSymbolLayerV2
{
public:
	
  // not necessarily supported by all symbol layers...
	virtual void setColor(const QColor& color) { mColor = color; }
	virtual QColor color() const { return mColor; }
	
	virtual ~QgsSymbolLayerV2() {}
  
	virtual QString layerType() const = 0;
	
	virtual void startRender(QgsRenderContext& context) = 0;
	virtual void stopRender(QgsRenderContext& context) = 0;
	
	virtual QgsSymbolLayerV2* clone() const = 0;
	
	virtual QgsStringMap properties() const = 0;
	
	virtual void drawPreviewIcon(QPainter* painter, QSize size) = 0;
	
	virtual QgsSymbolV2* subSymbol() { return NULL; }
  // set layer's subsymbol. takes ownership of the passed symbol
	virtual bool setSubSymbol(QgsSymbolV2* symbol) { delete symbol; return false; }
	
  QgsSymbolV2::SymbolType type() const { return mType; }
	
  void setLocked(bool locked) { mLocked = locked; }
	bool isLocked() const { return mLocked; }
	
protected:
  QgsSymbolLayerV2(QgsSymbolV2::SymbolType type, bool locked = false) : mType(type), mLocked(locked) {}
	
  QgsSymbolV2::SymbolType mType;
	bool mLocked;
  QColor mColor;
};

//////////////////////

class QgsMarkerSymbolLayerV2 : public QgsSymbolLayerV2
{
public:
	virtual void renderPoint(const QPointF& point, QgsRenderContext& context) = 0;
	
	void drawPreviewIcon(QPainter* painter, QSize size);

  void setAngle(double angle) { mAngle = angle; }
  double angle() const { return mAngle; }
	
  void setSize(double size) { mSize = size; }
  double size() const { return mSize; } 
	
protected:
  QgsMarkerSymbolLayerV2(bool locked = false);
  
  double mAngle;
  double mSize;
};

class QgsLineSymbolLayerV2 : public QgsSymbolLayerV2
{
public:
	virtual void renderPolyline(const QPolygonF& points, QgsRenderContext& context) = 0;
	
  void setWidth(int width) { mWidth = width; }
  int width() const { return mWidth; }
		
  void drawPreviewIcon(QPainter* painter, QSize size);
  
protected:
  QgsLineSymbolLayerV2(bool locked = false);
  
  int mWidth;
};

class QgsFillSymbolLayerV2 : public QgsSymbolLayerV2
{
public:
	virtual void renderPolygon(const QPolygonF& points, QList<QPolygonF>* rings, QgsRenderContext& context) = 0;
	
	void drawPreviewIcon(QPainter* painter, QSize size);
  
protected:
  QgsFillSymbolLayerV2(bool locked = false);
};

class QgsSymbolLayerV2Widget;

#endif
