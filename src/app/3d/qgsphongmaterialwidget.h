#ifndef QGSPHONGMATERIALWIDGET_H
#define QGSPHONGMATERIALWIDGET_H

#include <QWidget>

#include <ui_phongmaterialwidget.h>

class PhongMaterialSettings;


//! Widget for configuration of Phong material settings
class QgsPhongMaterialWidget : public QWidget, private Ui::PhongMaterialWidget
{
    Q_OBJECT
  public:
    explicit QgsPhongMaterialWidget( QWidget *parent = nullptr );

    void setMaterial( const PhongMaterialSettings &material );
    PhongMaterialSettings material() const;

  signals:
    void changed();

  public slots:
};

#endif // QGSPHONGMATERIALWIDGET_H
