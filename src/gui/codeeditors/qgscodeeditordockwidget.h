/***************************************************************************
  qgscodeeditordockwidget.h
  --------------------------------------
  Date                 : March 2023
  Copyright            : (C) 2023 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCODEEDITORDOCKWIDGET_H
#define QGSCODEEDITORDOCKWIDGET_H

#include "qgis_gui.h"

#include <QWidget>

class QgsDockableWidgetHelper;
class QToolButton;


/**
 * A custom dock widget for code editors.
 *
 * \warning Exposed to Python for internal use only -- this class is not considered stable API.
 *
 * \ingroup gui
 * \since QGIS 3.32
 */
class GUI_EXPORT QgsCodeEditorDockWidget : public QWidget
{
    Q_OBJECT
  public:
    /**
     * Constructor for QgsCodeEditorDockWidget, with the specified window geometry settings key.
     *
     * If \a usePersistentWidget is TRUE then the widget (either as a dock or window) cannot be destroyed and must be hidden instead.
     */
    QgsCodeEditorDockWidget( const QString &windowGeometrySettingsKey = QString(), bool usePersistentWidget = false );
    ~QgsCodeEditorDockWidget() override;

    /**
     * Sets the title to use for the code editor dock widget or window.
     */
    void setTitle( const QString &title );

    /**
     * Returns the dock toggle button for the widget, which is used to toggle between dock or full window mode.
     */
    QToolButton *dockToggleButton();

    //! Sets the object name of the dock widget
    void setDockObjectName( const QString &name );

    /**
     * Returns TRUE if the widget is user visible.
     */
    bool isUserVisible() const;

  public slots:

    /**
     * Sets whether the editor is user visible.
     */
    void setUserVisible( bool visible );

  signals:

    /**
     * Emitted when the editor's visibility is changed.
     */
    void visibilityChanged( bool isVisible );

  private:
    QgsDockableWidgetHelper *mDockableWidgetHelper = nullptr;
    QToolButton *mDockToggleButton = nullptr;
};

#endif // QGSCODEEDITORDOCKWIDGET_H
