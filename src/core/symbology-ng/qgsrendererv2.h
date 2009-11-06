
#ifndef QGSRENDERERV2_H
#define QGSRENDERERV2_H

#include "qgis.h"
#include "qgsfield.h" // for QgsFieldMap

#include <QList>
#include <QString>
#include <QVariant>
#include <QPair>
#include <QPixmap>

class QDomDocument;
class QDomElement;

class QgsSymbolV2;
class QgsRenderContext;
class QgsFeature;

typedef QList<QgsSymbolV2*> QgsSymbolV2List;
typedef QMap<QString, QgsSymbolV2* > QgsSymbolV2Map;

typedef QList< QPair<QString, QPixmap> > QgsLegendSymbologyList;

#define RENDERER_TAG_NAME   "renderer-v2"

////////
// symbol levels

class QgsSymbolV2LevelItem
{
public:
  QgsSymbolV2LevelItem( QgsSymbolV2* symbol, int layer ) : mSymbol(symbol), mLayer(layer) {}
  QgsSymbolV2* symbol() { return mSymbol; }
  int layer() { return mLayer; }
protected:
  QgsSymbolV2* mSymbol;
  int mLayer;
};

// every level has list of items: symbol + symbol layer num
typedef QList< QgsSymbolV2LevelItem > QgsSymbolV2Level;

// this is a list of levels
typedef QList< QgsSymbolV2Level > QgsSymbolV2LevelOrder;


//////////////
// renderers

class QgsFeatureRendererV2
{
public:
	// renderer takes ownership of its symbols!
  
  //! return a new renderer - used by default in vector layers
  static QgsFeatureRendererV2* defaultRenderer(QGis::GeometryType geomType);
  
  QString type() const { return mType; }
	
	// to be overridden
	virtual QgsSymbolV2* symbolForFeature(QgsFeature& feature)=0;
	
  virtual void startRender(QgsRenderContext& context, const QgsFieldMap& fields)=0;
	
	virtual void stopRender(QgsRenderContext& context)=0;
	
  virtual QList<QString> usedAttributes()=0;
	
	virtual ~QgsFeatureRendererV2() {}

  virtual QgsFeatureRendererV2* clone()=0;
	
  void renderFeature(QgsFeature& feature, QgsRenderContext& context, int layer = -1);

  //! for debugging
  virtual QString dump();
	
  //! for symbol levels
  virtual QgsSymbolV2List symbols()=0;

  bool usingSymbolLevels() const { return mUsingSymbolLevels; }
  void setUsingSymbolLevels(bool usingSymbolLevels) { mUsingSymbolLevels = usingSymbolLevels; }

  //! create a renderer from XML element
  static QgsFeatureRendererV2* load(QDomElement& symbologyElem);

  //! store renderer info to XML element
  virtual QDomElement save(QDomDocument& doc);

  //! return a list of symbology items for the legend
  virtual QgsLegendSymbologyList legendSymbologyItems(QSize iconSize);
  
  /** Returns the index of a field name or -1 if the field does not exist
    * copied from QgsVectorDataProvider... d'oh... probably should be elsewhere
    */
  static int fieldNameIndex( const QgsFieldMap& fields, const QString& fieldName );


protected:
  QgsFeatureRendererV2(QString type);

  QString mType;

  bool mUsingSymbolLevels;
};


#endif
