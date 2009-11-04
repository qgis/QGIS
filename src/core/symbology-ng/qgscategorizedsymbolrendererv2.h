#ifndef QGSCATEGORIZEDSYMBOLRENDERERV2_H
#define QGSCATEGORIZEDSYMBOLRENDERERV2_H

#include "qgsrendererv2.h"

#include <QHash>

class QgsVectorColorRampV2;

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

  QgsSymbolV2* sourceSymbol();
  void setSourceSymbol(QgsSymbolV2* sym);

  QgsVectorColorRampV2* sourceColorRamp();
  void setSourceColorRamp(QgsVectorColorRampV2* ramp);

protected:
  QgsCategoryList mCategories;
  QString mAttrName;
  QgsSymbolV2* mSourceSymbol;
  QgsVectorColorRampV2* mSourceColorRamp;

  //! attribute index (derived from attribute name in startRender)
  int mAttrNum;

  //! hashtable for faster access to symbols
  QHash<QString, QgsSymbolV2*> mSymbolHash;

  void rebuildHash();

  QgsSymbolV2* symbolForValue(QVariant value);
};


#endif // QGSCATEGORIZEDSYMBOLRENDERERV2_H
