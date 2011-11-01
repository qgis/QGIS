#ifndef QGSVECTORFIELDSYMBOLLAYERWIDGET_H
#define QGSVECTORFIELDSYMBOLLAYERWIDGET_H

#include "qgssymbollayerv2widget.h"
#include "ui_widget_vectorfield.h"

class QgsVectorFieldSymbolLayer;

class GUI_EXPORT QgsVectorFieldSymbolLayerWidget: public QgsSymbolLayerV2Widget, private Ui::WidgetVectorFieldBase
{
    Q_OBJECT
  public:
    QgsVectorFieldSymbolLayerWidget( const QgsVectorLayer* vl, QWidget* parent = 0 );
    ~QgsVectorFieldSymbolLayerWidget();

    static QgsSymbolLayerV2Widget* create( const QgsVectorLayer* vl ) { return new QgsVectorFieldSymbolLayerWidget( vl ); }

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayerV2* layer );
    virtual QgsSymbolLayerV2* symbolLayer();

  protected:
    QgsVectorFieldSymbolLayer* mLayer;
    void updateMarkerIcon();

  private slots:
    void on_mScaleSpinBox_valueChanged( double d );
    void on_mXAttributeComboBox_currentIndexChanged( int index );
    void on_mYAttributeComboBox_currentIndexChanged( int index );
    void on_mLineStylePushButton_clicked();
    void on_mCartesianRadioButton_toggled( bool checked );
    void on_mPolarRadioButton_toggled( bool checked );
    void on_mHeightRadioButton_toggled( bool checked );
    void on_mDegreesRadioButton_toggled( bool checked );
    void on_mRadiansRadioButton_toggled( bool checked );
    void on_mClockwiseFromNorthRadioButton_toggled( bool checked );
    void on_mCounterclockwiseFromEastRadioButton_toggled( bool checked );
};

#endif // QGSVECTORFIELDSYMBOLLAYERWIDGET_H
