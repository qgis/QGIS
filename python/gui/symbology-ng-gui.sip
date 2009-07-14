

class QgsSymbolV2PropertiesDialog : QDialog //, private Ui::DlgSymbolV2Properties
{
%TypeHeaderCode
#include <qgssymbolv2propertiesdialog.h>
%End

public:
  QgsSymbolV2PropertiesDialog(QgsSymbolV2* symbol, QWidget* parent = NULL);
  
  
public slots:
  void moveLayerDown();
  void moveLayerUp();

  void addLayer();
  void removeLayer();

  void lockLayer();

  void layerTypeChanged();
  
  void layerChanged();

};



class QgsRendererV2PropertiesDialog : QDialog //, private Ui::QgsRendererV2PropsDialogBase
{
%TypeHeaderCode
#include <qgsrendererv2propertiesdialog.h>
%End

public:
  QgsRendererV2PropertiesDialog(QgsVectorLayer* layer, QgsStyleV2* style, QWidget* parent = NULL);
  
public slots:
  void changeSingleSymbol();
  void updateRenderer();
  
  void categoryColumnChanged();
  void categoriesDoubleClicked(const QModelIndex & idx);
  void addCategories();
  void deleteCategory();
  void deleteAllCategories();
};



class QgsStyleV2ManagerDialog : QDialog //, private Ui::QgsStyleV2ManagerDialogBase
{
%TypeHeaderCode
#include <qgsstylev2managerdialog.h>
%End
  
public:
  QgsStyleV2ManagerDialog(QgsStyleV2* style, QString styleFilename, QWidget* parent = NULL);
  
public slots:
  void addItem();
  void editItem();
  void removeItem();
};


class QgsSymbolV2SelectorDialog : QDialog //, private Ui::QgsSymbolV2SelectorDialogBase
{
%TypeHeaderCode
#include <qgssymbolv2selectordialog.h>
%End

public:
  QgsSymbolV2SelectorDialog(QgsSymbolV2* symbol, QgsStyleV2* style, QWidget* parent = NULL);
  
protected:
  void populateSymbolView();
  void updateSymbolPreview();
  void updateSymbolColor();
  void updateSymbolInfo();
  

public slots:
  void changeSymbolProperties();
  void setSymbolFromStyle(const QModelIndex & index);
  void setSymbolColor();
  void setMarkerAngle(double angle);
  void setMarkerSize(double size);
  void setLineWidth(double width);

};
