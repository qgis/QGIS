#ifndef QGSELLIPSESYMBOLLAYERV2WIDGET_H
#define QGSELLIPSESYMBOLLAYERV2WIDGET_H

#include "ui_widget_ellipse.h"
#include "qgssymbollayerv2widget.h"

class QgsEllipseSymbolLayerV2;

class GUI_EXPORT QgsEllipseSymbolLayerV2Widget: public QgsSymbolLayerV2Widget, private Ui::WidgetEllipseBase
{
    Q_OBJECT

  public:
    QgsEllipseSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent = 0 );

    static QgsSymbolLayerV2Widget* create( const QgsVectorLayer* vl ) { return new QgsEllipseSymbolLayerV2Widget( vl ); }

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayerV2* layer );
    virtual QgsSymbolLayerV2* symbolLayer();

  protected:
    QgsEllipseSymbolLayerV2* mLayer;

  private:
    void blockComboSignals( bool block );
    //insert available attributes for data defined symbolisation
    void fillDataDefinedComboBoxes();

  private slots:
    void on_mShapeListWidget_itemSelectionChanged();
    void on_mWidthSpinBox_valueChanged( double d );
    void on_mHeightSpinBox_valueChanged( double d );
    void on_mRotationSpinBox_valueChanged( double d );
    void on_mOutlineWidthSpinBox_valueChanged( double d );
    void on_btnChangeColorBorder_clicked();
    void on_btnChangeColorFill_clicked();

    void on_mDDSymbolWidthComboBox_currentIndexChanged( int idx );
    void on_mDDSymbolHeightComboBox_currentIndexChanged( int idx );
    void on_mDDRotationComboBox_currentIndexChanged( int idx );
    void on_mDDOutlineWidthComboBox_currentIndexChanged( int idx );
    void on_mDDFillColorComboBox_currentIndexChanged( int idx );
    void on_mDDOutlineColorComboBox_currentIndexChanged( int idx );
    void on_mDDShapeComboBox_currentIndexChanged( int idx );
};

#endif // QGSELLIPSESYMBOLLAYERV2WIDGET_H
