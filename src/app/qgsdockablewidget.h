/***************************************************************************
  qgsdockablewidget.h
  --------------------------------------
  Date                 : January 2022
  Copyright            : (C) 2022 by Belgacem Nedjima
  Email                : belgacem dot nedjima at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDOCKABLEWIDGET_H
#define QGSDOCKABLEWIDGET_H

#include "qgis_app.h"
#include "qgsdockwidget.h"

#include <QDialog>
#include <QVBoxLayout>

#define SIP_NO_FILE

class Qgs3DAnimationWidget;
class Qgs3DMapCanvas;
class Qgs3DMapSettings;
class Qgs3DMapToolMeasureLine;
class QgsMapCanvas;
class Qgs3DMapCanvasWidget;
class QgsDockWidget;

class APP_EXPORT QgsDockableWidget : public QWidget
{
    Q_OBJECT
  public:
    QgsDockableWidget( QWidget *parent = nullptr );

    void setWidget( QWidget *widget );

    QWidget *widget() { return mWidget; }

    QgsDockWidget *dockWidget() { return mDock; }

    QDialog *dialog() { return mDialog; }

    bool isDocked() const;

    void setWindowTitle( const QString &title );

  protected:
    void closeEvent( QCloseEvent * ) override;

  signals:
    void closed();

  public slots:
    void toggleDockMode( bool docked );

  private:
    bool mIsDocked = true;
    QWidget *mWidget = nullptr;

    QgsDockWidget *mDock = nullptr;

    QDialog *mDialog = nullptr;
};

#endif // QGSDOCKABLEWIDGET_H
