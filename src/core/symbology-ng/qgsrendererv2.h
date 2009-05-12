
#ifndef QGSRENDERERV2_H
#define QGSRENDERERV2_H

#include <QList>
#include <QHash>
#include <QString>
#include <QVariant>

class QgsSymbolV2;
class QgsRenderContext;
class QgsFeature;

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
  
  RendererType type() const { return mType; }
	
	// to be overridden
	virtual QgsSymbolV2* symbolForFeature(QgsFeature& feature)=0;
	
	virtual void startRender(QgsRenderContext& context)=0;
	
	virtual void stopRender(QgsRenderContext& context)=0;
	
	virtual QList<int> usedAttributes()=0;
	
	virtual ~QgsFeatureRendererV2() {}
	
	void renderFeature(QgsFeature& feature, QgsRenderContext& context);
	
	//TODO: symbols() for symbol levels
  
protected:
  QgsFeatureRendererV2(RendererType type);
  
  RendererType mType;
};

class QgsSingleSymbolRendererV2 : public QgsFeatureRendererV2
{
public:
	
	QgsSingleSymbolRendererV2(QgsSymbolV2* symbol);
	
	virtual ~QgsSingleSymbolRendererV2();
	
	virtual QgsSymbolV2* symbolForFeature(QgsFeature& feature);
	
	virtual void startRender(QgsRenderContext& context);
	
	virtual void stopRender(QgsRenderContext& context);
	
	virtual QList<int> usedAttributes();
  
  QgsSymbolV2* symbol() const;
  void setSymbol(QgsSymbolV2* s);

protected:
	QgsSymbolV2* mSymbol;
};


class QgsRendererCategoryV2
{
public:
  
  //! takes ownership of symbol
  QgsRendererCategoryV2(QVariant value, QgsSymbolV2* symbol, QString label)
  : mValue(value), mSymbol(symbol), mLabel(label) { }
  
  //! copy constructor
  QgsRendererCategoryV2(const QgsRendererCategoryV2& cat);
  
  ~QgsRendererCategoryV2();
  
  QVariant value() const { return mValue; }
  QgsSymbolV2* symbol() const { return mSymbol; }
  QString label() const { return mLabel; }
  
  void setSymbol(QgsSymbolV2* s);
  void setLabel(QString label) { mLabel = label; }
  
protected:
  QVariant mValue;
  QgsSymbolV2* mSymbol;
  QString mLabel;
};

typedef QList<QgsRendererCategoryV2> QgsCategoryList;

class QgsCategorizedSymbolRendererV2 : public QgsFeatureRendererV2
{
public:
	
  QgsCategorizedSymbolRendererV2(int attrNum = -1, QgsCategoryList categories = QgsCategoryList());
	
	virtual ~QgsCategorizedSymbolRendererV2();
	
	virtual QgsSymbolV2* symbolForFeature(QgsFeature& feature);
	
	virtual void startRender(QgsRenderContext& context);
	
	virtual void stopRender(QgsRenderContext& context);
	
	virtual QList<int> usedAttributes();
  
  const QgsCategoryList& categories() { return mCategories; }
  
  //! return index of category with specified value (-1 if not found)
  int categoryIndexForValue(QVariant val);
  
  bool updateCategorySymbol(int catIndex, QgsSymbolV2* symbol);
  bool updateCategoryLabel(int catIndex, QString label);
  
  bool deleteCategory(int catIndex);
  void deleteAllCategories();
  
  int attributeIndex() const { return mAttrNum; }
  void setAttributeIndex(int attr) { mAttrNum = attr; }

protected:
  QgsCategoryList mCategories;
  int mAttrNum;
  
  QgsSymbolV2* symbolForValue(QVariant value);
};


//////////////////

class QgsRendererRangeV2
{
public:
  QgsRendererRangeV2(double lowerValue, double upperValue, QgsSymbolV2* symbol, QString label);
  QgsRendererRangeV2(const QgsRendererRangeV2& range);
  
  ~QgsRendererRangeV2();
  
  double lowerValue() const { return mLowerValue; }
  double upperValue() const { return mUpperValue; }
  
  QgsSymbolV2* symbol() const { return mSymbol; }
  QString label() const { return mLabel; }
  
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
  QgsGraduatedSymbolRendererV2(int attrNum = -1, QgsRangeList ranges = QgsRangeList());
  
  virtual ~QgsGraduatedSymbolRendererV2();

  virtual QgsSymbolV2* symbolForFeature(QgsFeature& feature);
	
  virtual void startRender(QgsRenderContext& context);
	
  virtual void stopRender(QgsRenderContext& context);
	
  virtual QList<int> usedAttributes();
  
  int attributeIndex() const { return mAttrNum; }
  void setAttributeIndex(int attr) { mAttrNum = attr; }
  
  const QgsRangeList& ranges() { return mRanges; }
  
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
                  int attrNum,
                  int classes,
                  Mode mode,
                  QgsSymbolV2* symbol,
                  QgsVectorColorRampV2* ramp);

protected:
  QgsRangeList mRanges;
  int mAttrNum;
  Mode mMode;
  
  QgsSymbolV2* symbolForValue(double value);
};


#endif
