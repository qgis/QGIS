#ifndef QGSLINE3DSYMBOLWIDGET_H
#define QGSLINE3DSYMBOLWIDGET_H

#include <QWidget>

#include "ui_line3dsymbolwidget.h"

class QgsLine3DSymbol;

//! A widget for configuration of 3D symbol for polygons
class QgsLine3DSymbolWidget : public QWidget, private Ui::Line3DSymbolWidget
{
    Q_OBJECT
  public:
    explicit QgsLine3DSymbolWidget( QWidget *parent = nullptr );

    void setSymbol( const QgsLine3DSymbol &symbol );
    QgsLine3DSymbol symbol() const;

  signals:
    void changed();

  public slots:
};

#endif // QGSLINE3DSYMBOLWIDGET_H
