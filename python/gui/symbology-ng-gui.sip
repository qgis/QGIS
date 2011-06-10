

class QgsSymbolV2PropertiesDialog : QDialog //, private Ui::DlgSymbolV2Properties
{
%TypeHeaderCode
#include <qgssymbolv2propertiesdialog.h>
%End

public:
  QgsSymbolV2PropertiesDialog(QgsSymbolV2* symbol, const QgsVectorLayer*, QWidget* parent = NULL);
  
  
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
    QgsRendererV2PropertiesDialog(QgsVectorLayer* layer, QgsStyleV2* style, bool embedded = false);

public slots:
    //! called when user changes renderer type
    void rendererChanged();

    void apply();
    void onOK();

    void showSymbolLevels();

protected:

    //! Reimplements dialog keyPress event so we can ignore it
    void keyPressEvent( QKeyEvent * event );

};


class QgsStyleV2ManagerDialog : QDialog //, private Ui::QgsStyleV2ManagerDialogBase
{
%TypeHeaderCode
#include <qgsstylev2managerdialog.h>
%End
  
public:
  QgsStyleV2ManagerDialog(QgsStyleV2* style, QWidget* parent = NULL);
  
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
  QgsSymbolV2SelectorDialog(QgsSymbolV2* symbol, QgsStyleV2* style, const QgsVectorLayer*, QWidget* parent = NULL, bool embedded = false);
  
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

signals:
  void symbolModified();

};



class QgsSymbolLayerV2Widget : QWidget
{
%TypeHeaderCode
#include <qgssymbollayerv2widget.h>
%End

  public:
    QgsSymbolLayerV2Widget( const QgsVectorLayer* vl, Widget* parent );
    virtual ~QgsSymbolLayerV2Widget();

    virtual void setSymbolLayer( QgsSymbolLayerV2* layer ) = 0;
    virtual QgsSymbolLayerV2* symbolLayer() = 0;

  signals:
    void changed();
};

class QgsRendererV2Widget : QWidget
{
%TypeHeaderCode
#include <qgsrendererv2widget.h>
%End

  public:
    QgsRendererV2Widget( QgsVectorLayer* layer, QgsStyleV2* style );

    virtual ~QgsRendererV2Widget();

    //! return pointer to the renderer (no transfer of ownership)
    virtual QgsFeatureRendererV2* renderer() = 0;

};
