#ifndef QGSCATEGORIZEDSYMBOLRENDERERV2WIDGET_H
#define QGSCATEGORIZEDSYMBOLRENDERERV2WIDGET_H

#include "qgsrendererv2widget.h"

class QgsCategorizedSymbolRendererV2;

#include "ui_qgscategorizedsymbolrendererv2widget.h"

class QgsCategorizedSymbolRendererV2Widget : public QgsRendererV2Widget, private Ui::QgsCategorizedSymbolRendererV2Widget
{
  Q_OBJECT

public:

  static QgsRendererV2Widget* create(QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer);

  QgsCategorizedSymbolRendererV2Widget(QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer);
  ~QgsCategorizedSymbolRendererV2Widget();

  virtual QgsFeatureRendererV2* renderer();

public slots:
  void changeCategorizedSymbol();
  void categoryColumnChanged();
  void categoriesDoubleClicked(const QModelIndex & idx);
  void addCategories();
  void deleteCategory();
  void deleteAllCategories();

protected:

  void updateUiFromRenderer();

  void updateCategorizedSymbolIcon();

  //! populate categories view
  void populateCategories();

  //! populate column combo
  void populateColumns();

  void populateColorRamps();

  //! return row index for the currently selected category (-1 if on no selection)
  int currentCategoryRow();

  //! return key for the currently selected category
  QVariant currentCategory();

  void changeCategorySymbol();

protected:
  QgsCategorizedSymbolRendererV2* mRenderer;

  QgsSymbolV2* mCategorizedSymbol;
};



#endif // QGSCATEGORIZEDSYMBOLRENDERERV2WIDGET_H
