#ifndef QGSPOLYGON3DSYMBOLWIDGET_H
#define QGSPOLYGON3DSYMBOLWIDGET_H

#include <QWidget>

#include "ui_polygon3dsymbolwidget.h"

class QgsPolygon3DSymbol;

//! A widget for configuration of 3D symbol for polygons
class QgsPolygon3DSymbolWidget : public QWidget, private Ui::Polygon3DSymbolWidget
{
    Q_OBJECT
  public:
    explicit QgsPolygon3DSymbolWidget( QWidget *parent = nullptr );

    void setSymbol( const QgsPolygon3DSymbol &symbol );
    QgsPolygon3DSymbol symbol() const;

  signals:
    void changed();

  public slots:
};

#endif // QGSPOLYGON3DSYMBOLWIDGET_H
