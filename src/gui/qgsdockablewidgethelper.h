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

#include "qgis_gui.h"

#include <QDialog>
#include <QToolButton>
#include <QMainWindow>
#include <QDomElement>
#include <QPointer>

#define SIP_NO_FILE

class QgsDockWidget;

///@cond PRIVATE

class GUI_EXPORT QgsNonRejectableDialog : public QDialog
{
    Q_OBJECT
  public:
    explicit QgsNonRejectableDialog( QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags() );
    void reject() override;
};

/**
 * This class is responible of displaying a QWidget inside a top level window or a dock widget (only one of the 2 is alive at most).
 * The widget is not owned by this class and its ownership is passed to the owner window before this class's object is deleted or
 * another widget is set using setWidget() function
 *
 * \note Not available from Python bindings
 *
 * \ingroup gui
 * \since QGIS 3.24
 */
class GUI_EXPORT QgsDockableWidgetHelper : public QObject
{
    Q_OBJECT
  public:
    /**
     * Constructs an object that is responsible of making a docked widget or a window titled \a windowTitle that holds the \a widget
     * The ownership of \a widget is returned to \a ownerWindow once the object is destroyed.
     *
     * If \a usePersistentWidget is TRUE then the \a widget (either as a dock or window) cannot be destroyed and must be hidden instead.
     */
    QgsDockableWidgetHelper( bool isDocked, const QString &windowTitle, QWidget *widget, QMainWindow *ownerWindow, Qt::DockWidgetArea defaultDockArea = Qt::NoDockWidgetArea, const QStringList &tabifyWith = QStringList(), bool raiseTab = false, const QString &windowGeometrySettingsKey = QString(), bool usePersistentWidget = false );
    ~QgsDockableWidgetHelper();

    //! Reads the dimensions of both the dock widget and the top level window
    void writeXml( QDomElement &viewDom );
    void readXml( const QDomElement &viewDom );

    //! Sets the widget placed inside the dock widget or the window
    void setWidget( QWidget *widget );
    //! Return the widget placed inside the dock widget or the window
    QWidget *widget() { return mWidget; }

    //! Returns the dock widget if we are in docking mode and nullptr otherwise.
    QgsDockWidget *dockWidget();
    //! Returns the dialog window if we are in top level window mode and nullptr otherwise.
    QDialog *dialog();

    //! Sets the displayed title of the dialog and the dock widget
    void setWindowTitle( const QString &title );
    //! Returns the displayed title of the dialog and the dock widget
    QString windowTitle() const { return mWindowTitle; }

    //! Sets the object name of the dock widget
    void setDockObjectName( const QString &name );
    //! Returns the object name of the dock widget
    QString dockObjectName() const;

    /**
     * Returns TRUE if the widget is a visible dialog or a user-visible
     * dock widget.
     */
    bool isUserVisible() const;

    /**
     * Create a tool button for docking/undocking the widget
     * \note The ownership of the tool button is managed by the caller
     */
    QToolButton *createDockUndockToolButton();

    /**
     * Create an action for docking/undocking the widget, with the specified \a parent widget.
     */
    QAction *createDockUndockAction( const QString &title, QWidget *parent );

    bool eventFilter( QObject *watched, QEvent *event ) override;

    static std::function<void( Qt::DockWidgetArea, QDockWidget *, const QStringList &, bool )> sAddTabifiedDockWidgetFunction;
    static std::function<QString()> sAppStylesheetFunction;

    static QMainWindow *sOwnerWindow;

  signals:
    void closed();

    void dockModeToggled( bool docked );

    void visibilityChanged( bool isVisible );

  public slots:
    void toggleDockMode( bool docked );

    void setUserVisible( bool visible );

  private:
    void setupDockWidget( const QStringList &tabSiblings = QStringList() );

    bool mIsDocked = true;
    QWidget *mWidget = nullptr;

    QPointer<QDialog> mDialog;
    QRect mDialogGeometry;

    QPointer<QgsDockWidget> mDock;
    QRect mDockGeometry;
    bool mIsDockFloating = true;
    Qt::DockWidgetArea mDockArea = Qt::RightDockWidgetArea;

    QString mWindowTitle;
    QString mObjectName;
    QMainWindow *mOwnerWindow = nullptr;

    QStringList mTabifyWith;
    bool mRaiseTab = false;

    QString mWindowGeometrySettingsKey;

    // Unique identifier of dock
    QString mUuid;

    bool mUsePersistentWidget = false;
};

///@endcond

#endif // QGSDOCKABLEWIDGETHELPER_H
