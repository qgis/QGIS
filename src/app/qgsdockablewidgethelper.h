/***************************************************************************
  qgsdockablewidgethelper.h
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

#ifndef QGSDOCKABLEWIDGETHELPER_H
#define QGSDOCKABLEWIDGETHELPER_H

#include "qgis_app.h"
#include "qgsdockwidget.h"

#include <QDialog>
#include <QVBoxLayout>

#define SIP_NO_FILE

class QgsDockWidget;

class APP_EXPORT QgsDockableWidgetHelper : public QObject
{
    Q_OBJECT
  public:
    QgsDockableWidgetHelper( bool isDocked, const QString &windowTitle, QWidget *widget );
    ~QgsDockableWidgetHelper();

    void setWidget( QWidget *widget );

    QWidget *widget() { return mWidget; }

    QgsDockWidget *dockWidget() { return mDock; }

    QDialog *dialog() { return mDialog; }

    bool isDocked() const;

    void setWindowTitle( const QString &title );
    QString windowTitle() const { return mWindowTitle; }

    void setDialogGeometry( const QRect &geom );
    void setDockGeometry( const QRect &geom, bool isFloating, Qt::DockWidgetArea area );

    QRect dialogGeometry() const;
    QRect dockGeometry() const;
    bool isDockFloating() const;
    Qt::DockWidgetArea dockFloatingArea() const;

  signals:
    void closed();

  public slots:
    void toggleDockMode( bool docked );

  private:
    void setupDockWidget();

    bool mIsDocked = true;
    QWidget *mWidget = nullptr;

    QDialog *mDialog = nullptr;
    QRect mDialogGeometry;

    QgsDockWidget *mDock = nullptr;
    QRect mDockGeometry;
    bool mIsDockFloating;
    Qt::DockWidgetArea mDockArea;

    QString mWindowTitle;
};

#endif // QGSDOCKABLEWIDGETHELPER_H
