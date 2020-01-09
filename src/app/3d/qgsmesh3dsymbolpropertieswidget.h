#ifndef QGSMESH3DSYMBOLPROPERTIESWIDGET_H
#define QGSMESH3DSYMBOLPROPERTIESWIDGET_H


#include "ui_qgsmesh3dsymbolproperties.h"

class QgsMesh3DSymbol;
class QgsMeshLayer;

//! A widget for configuration of 3D symbol for meshes
class QgsMesh3dSymbolPropertiesWidget: public QWidget, private Ui::QgsMesh3dSymbolProperties
{
    Q_OBJECT
  public:
    QgsMesh3dSymbolPropertiesWidget( QgsMeshLayer *meshLayer, QWidget *parent = nullptr );

    QgsMesh3DSymbol symbol() const;

    void setLayer( QgsMeshLayer *meshLayer, bool updateSymbol = true );
    void setSymbol( const QgsMesh3DSymbol &symbol );
    int rendererTypeComboBoxIndex() const;
    void setRendererTypeComboBoxIndex( int index );

  public slots:
    void reloadColorRampShaderMinMax();

  signals:
    void changed();

  private slots:

    void onColorRampShaderMinMaxChanged();
    void onColoringTypeChanged();

  private:
    double lineEditValue( const QLineEdit *lineEdit ) const;

    QgsMeshLayer *mLayer = nullptr;
};

#endif // QGSMESH3DSYMBOLPROPERTIESWIDGET_H
