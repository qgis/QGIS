#ifndef QGSPOINT3DSYMBOLWIDGET_H
#define QGSPOINT3DSYMBOLWIDGET_H

#include <QWidget>

#include "ui_point3dsymbolwidget.h"

class Point3DSymbol;

//! A widget for configuration of 3D symbol for points
class QgsPoint3DSymbolWidget : public QWidget, private Ui::Point3DSymbolWidget
{
    Q_OBJECT
  public:
    explicit QgsPoint3DSymbolWidget( QWidget *parent = nullptr );

    void setSymbol( const Point3DSymbol &symbol );
    Point3DSymbol symbol() const;

  signals:
    void changed();

  private slots:
    void onShapeChanged();
    void onChooseModelClicked(bool checked=false);
    void onOverwriteMaterialChecked(int state);
};

#endif // QGSPOINT3DSYMBOLWIDGET_H
