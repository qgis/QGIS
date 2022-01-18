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

#include <QDialog>
#include <QToolButton>
#include <QMainWindow>
#include <QDomElement>

#define SIP_NO_FILE

class QgsDockWidget;

/**
 * This class is responible of displaying a QWidget inside a dialog or a dock widget (only one of the 2 is alive at most).
 * The widget is not owned by this class and its ownership is passed to the owner window before this class's object is deleted or
 * another widget is set using setWidget() function
 *
 * \since QGIS 3.24
 */
class APP_EXPORT QgsDockableWidgetHelper : public QObject
{
    Q_OBJECT
  public:
    QgsDockableWidgetHelper( bool isDocked, const QString &windowTitle, QWidget *widget, QMainWindow *ownerWindow );
    ~QgsDockableWidgetHelper();

    void writeXml( QDomElement &viewDom );
    void readXml( QDomElement &viewDom );

    void setWidget( QWidget *widget );

    QWidget *widget() { return mWidget; }

    QgsDockWidget *dockWidget() { return mDock; }

    QDialog *dialog() { return mDialog; }

    void setWindowTitle( const QString &title );
    QString windowTitle() const { return mWindowTitle; }

    /**
     * Create a tool button for docking/undocking the widget
     * \note The ownership of the tool button is managed by the caller
     */
    QToolButton *createDockUndockToolButton();

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
    QMainWindow *mOwnerWindow;
};

#endif // QGSDOCKABLEWIDGETHELPER_H
