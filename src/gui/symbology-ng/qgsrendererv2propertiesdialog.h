
#ifndef QGSRENDERERV2PROPERTIESDIALOG_H
#define QGSRENDERERV2PROPERTIESDIALOG_H

#include "ui_qgsrendererv2propsdialogbase.h"

class QgsVectorLayer;
class QgsStyleV2;
class QgsSymbolV2;

class QgsFeatureRendererV2;
class QgsSingleSymbolRendererV2;
class QgsCategorizedSymbolRendererV2;
class QgsGraduatedSymbolRendererV2;

class QgsRendererV2PropertiesDialog : public QDialog, private Ui::QgsRendererV2PropsDialogBase
{
  Q_OBJECT

public:
  QgsRendererV2PropertiesDialog(QgsVectorLayer* layer, QgsStyleV2* style, QWidget* parent = NULL, bool embedded = false);
  ~QgsRendererV2PropertiesDialog();
  
public slots:
  void changeSingleSymbol();
  void updateRenderer();
  
  void changeCategorizedSymbol();
  void categoryColumnChanged();
  void categoriesDoubleClicked(const QModelIndex & idx);
  void addCategories();
  void deleteCategory();
  void deleteAllCategories();

  void changeGraduatedSymbol();
  void classifyGraduated();
  void rangesDoubleClicked(const QModelIndex & idx);

  void symbolLevels();

  void onOK();
  void apply();

protected:

  //! Reimplements dialog keyPress event so we can ignore it
  void keyPressEvent( QKeyEvent * event );

  //! update UI to reflect changes in renderer
  void updateUiFromRenderer();
  
  void updateCategorizedSymbolIcon();
  void updateGraduatedSymbolIcon();
 
  //! populate categories view
  void populateCategories();
  
  //! populate column combos in categorized and graduated page
  void populateColumns();
  
  //! populate ranges of graduated symbol renderer
  void populateRanges();
  
  void populateColorRamps();

  //! return row index for the currently selected category (-1 if on no selection)
  int currentCategoryRow();
  
  //! return key for the currently selected category
  QVariant currentCategory();
  
  void changeCategorySymbol();

  void changeRangeSymbol(int rangeIdx);

  QgsSingleSymbolRendererV2* rendererSingle();
  QgsCategorizedSymbolRendererV2* rendererCategorized();
  QgsGraduatedSymbolRendererV2* rendererGraduated();
  
  //! temporary renderer in current dialog
  QgsFeatureRendererV2* mRenderer;

  QgsVectorLayer* mLayer;
  
  QgsStyleV2* mStyle;
  
  QgsSymbolV2* mSingleSymbol;
  QgsSymbolV2* mGraduatedSymbol;
  QgsSymbolV2* mCategorizedSymbol;
};

#endif
