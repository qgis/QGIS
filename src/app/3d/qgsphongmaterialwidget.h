#ifndef QGSPHONGMATERIALWIDGET_H
#define QGSPHONGMATERIALWIDGET_H

#include <QWidget>

#include <ui_phongmaterialwidget.h>

class QgsPhongMaterialSettings;


//! Widget for configuration of Phong material settings
class QgsPhongMaterialWidget : public QWidget, private Ui::PhongMaterialWidget
{
    Q_OBJECT
  public:
    explicit QgsPhongMaterialWidget( QWidget *parent = nullptr );

    void setMaterial( const QgsPhongMaterialSettings &material );
    QgsPhongMaterialSettings material() const;

  signals:
    void changed();

  public slots:
};

#endif // QGSPHONGMATERIALWIDGET_H
