
#ifndef QGSRENDERERV2_H
#define QGSRENDERERV2_H

#include "qgis.h"
#include "qgsfield.h" // for QgsFieldMap

#include <QList>
#include <QHash>
#include <QString>
#include <QVariant>

class QDomDocument;
class QDomElement;

class QgsSymbolV2;
class QgsRenderContext;
class QgsFeature;

typedef QList<QgsSymbolV2*> QgsSymbolV2List;
typedef QMap<QString, QgsSymbolV2* > QgsSymbolV2Map;

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
  
  enum RendererType
  {
    RendererSingleSymbol,
    RendererCategorizedSymbol,
    RendererGraduatedSymbol
    // TODO: user type?
  };

  //! return a new renderer - used by default in vector layers
  static QgsFeatureRendererV2* defaultRenderer(QGis::GeometryType geomType);
  
  RendererType type() const { return mType; }
	
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
  
  /** Returns the index of a field name or -1 if the field does not exist
    * copied from QgsVectorDataProvider... d'oh... probably should be elsewhere
    */
  static int fieldNameIndex( const QgsFieldMap& fields, const QString& fieldName );


protected:
  QgsFeatureRendererV2(RendererType type);

  RendererType mType;

  bool mUsingSymbolLevels;
};

class QgsSingleSymbolRendererV2 : public QgsFeatureRendererV2
{
public:
	
	QgsSingleSymbolRendererV2(QgsSymbolV2* symbol);
	
	virtual ~QgsSingleSymbolRendererV2();
	
	virtual QgsSymbolV2* symbolForFeature(QgsFeature& feature);
	
  virtual void startRender(QgsRenderContext& context, const QgsFieldMap& fields);
	
	virtual void stopRender(QgsRenderContext& context);
	
  virtual QList<QString> usedAttributes();
  
  QgsSymbolV2* symbol() const;
  void setSymbol(QgsSymbolV2* s);

  virtual QString dump();

  virtual QgsFeatureRendererV2* clone();

  virtual QgsSymbolV2List symbols();

  //! create renderer from XML element
  static QgsFeatureRendererV2* create(QDomElement& element);

  //! store renderer info to XML element
  virtual QDomElement save(QDomDocument& doc);

protected:
	QgsSymbolV2* mSymbol;
};


class QgsRendererCategoryV2
{
public:
  
  //! takes ownership of symbol
  QgsRendererCategoryV2(QVariant value, QgsSymbolV2* symbol, QString label);
  
  //! copy constructor
  QgsRendererCategoryV2(const QgsRendererCategoryV2& cat);
  
  ~QgsRendererCategoryV2();
  
  QVariant value() const;
  QgsSymbolV2* symbol() const;
  QString label() const;
  
  void setSymbol(QgsSymbolV2* s);
  void setLabel(QString label);

  // debugging
  QString dump();
  
protected:
  QVariant mValue;
  QgsSymbolV2* mSymbol;
  QString mLabel;
};

typedef QList<QgsRendererCategoryV2> QgsCategoryList;

class QgsCategorizedSymbolRendererV2 : public QgsFeatureRendererV2
{
public:
	
  QgsCategorizedSymbolRendererV2(QString attrName = QString(), QgsCategoryList categories = QgsCategoryList());
	
	virtual ~QgsCategorizedSymbolRendererV2();
	
	virtual QgsSymbolV2* symbolForFeature(QgsFeature& feature);
	
  virtual void startRender(QgsRenderContext& context, const QgsFieldMap& fields);
	
	virtual void stopRender(QgsRenderContext& context);
	
  virtual QList<QString> usedAttributes();
  
  virtual QString dump();

  virtual QgsFeatureRendererV2* clone();

  virtual QgsSymbolV2List symbols();

  const QgsCategoryList& categories() { return mCategories; }
  
  //! return index of category with specified value (-1 if not found)
  int categoryIndexForValue(QVariant val);
  
  bool updateCategorySymbol(int catIndex, QgsSymbolV2* symbol);
  bool updateCategoryLabel(int catIndex, QString label);
  
  bool deleteCategory(int catIndex);
  void deleteAllCategories();
  
  QString classAttribute() const { return mAttrName; }
  void setClassAttribute(QString attr) { mAttrName = attr; }

  //! create renderer from XML element
  static QgsFeatureRendererV2* create(QDomElement& element);

  //! store renderer info to XML element
  virtual QDomElement save(QDomDocument& doc);

protected:
  QgsCategoryList mCategories;
  QString mAttrName;

  //! attribute index (derived from attribute name in startRender)
  int mAttrNum;

  //! hashtable for faster access to symbols
  QHash<QString, QgsSymbolV2*> mSymbolHash;

  void rebuildHash();

  QgsSymbolV2* symbolForValue(QVariant value);
};


//////////////////

class QgsRendererRangeV2
{
public:
  QgsRendererRangeV2(double lowerValue, double upperValue, QgsSymbolV2* symbol, QString label);
  QgsRendererRangeV2(const QgsRendererRangeV2& range);
  
  ~QgsRendererRangeV2();
  
  double lowerValue() const;
  double upperValue() const;
  
  QgsSymbolV2* symbol() const;
  QString label() const;
  
  void setSymbol(QgsSymbolV2* s);
  void setLabel(QString label);

  // debugging
  QString dump();

protected:
  double mLowerValue, mUpperValue;
  QgsSymbolV2* mSymbol;
  QString mLabel;
};

typedef QList<QgsRendererRangeV2> QgsRangeList;

class QgsVectorLayer;
class QgsVectorColorRampV2;

class QgsGraduatedSymbolRendererV2 : public QgsFeatureRendererV2
{
public:
  QgsGraduatedSymbolRendererV2(QString attrName = QString(), QgsRangeList ranges = QgsRangeList());
  
  virtual ~QgsGraduatedSymbolRendererV2();

  virtual QgsSymbolV2* symbolForFeature(QgsFeature& feature);
	
  virtual void startRender(QgsRenderContext& context, const QgsFieldMap& fields);
	
  virtual void stopRender(QgsRenderContext& context);
	
  virtual QList<QString> usedAttributes();

  virtual QString dump();

  virtual QgsFeatureRendererV2* clone();

  virtual QgsSymbolV2List symbols();

  QString classAttribute() const { return mAttrName; }
  void setClassAttribute(QString attr) { mAttrName = attr; }

  const QgsRangeList& ranges() { return mRanges; }
  
  bool updateRangeSymbol(int rangeIndex, QgsSymbolV2* symbol);
  bool updateRangeLabel(int rangeIndex, QString label);

  enum Mode
  {
    EqualInterval,
    Quantile,
    Custom
  };
  
  Mode mode() const { return mMode; }
  void setMode(Mode mode) { mMode = mode; }
  
  static QgsGraduatedSymbolRendererV2* createRenderer(
                  QgsVectorLayer* vlayer,
                  QString attrName,
                  int classes,
                  Mode mode,
                  QgsSymbolV2* symbol,
                  QgsVectorColorRampV2* ramp);

  //! create renderer from XML element
  static QgsFeatureRendererV2* create(QDomElement& element);

  //! store renderer info to XML element
  virtual QDomElement save(QDomDocument& doc);

protected:
  QgsRangeList mRanges;
  QString mAttrName;
  Mode mMode;

  //! attribute index (derived from attribute name in startRender)
  int mAttrNum;
  
  QgsSymbolV2* symbolForValue(double value);
};


#endif
