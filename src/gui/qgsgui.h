/***************************************************************************
                         qgsgui.h
                         --------
    begin                : May 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGUI_H
#define QGSGUI_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include <QWidget>
#include <memory>

class QgsEditorWidgetRegistry;
class QgsShortcutsManager;
class QgsLayerTreeEmbeddedWidgetRegistry;
class QgsMapLayerActionRegistry;
class QgsSourceSelectProviderRegistry;
class QgsNative;
class QgsLayoutItemGuiRegistry;
class QgsWidgetStateHelper;
class QgsProcessingGuiRegistry;
class QgsProcessingRecentAlgorithmLog;
class QgsWindowManagerInterface;

/**
 * \ingroup gui
 * QgsGui is a singleton class containing various registry and other global members
 * related to GUI classes.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsGui
{
  public:

    //! QgsGui cannot be copied
    QgsGui( const QgsGui &other ) = delete;

    //! QgsGui cannot be copied
    QgsGui &operator=( const QgsGui &other ) = delete;

    /**
     * Returns a pointer to the singleton instance.
     */
    static QgsGui *instance();

    /**
     * Returns the global native interface, which offers abstraction to the host OS's underlying public
     * interfaces.
     * \note Not available in Python bindings
     */
    SIP_SKIP static QgsNative *nativePlatformInterface();

    /**
     * Returns the global editor widget registry, used for managing all known edit widget factories.
     */
    static QgsEditorWidgetRegistry *editorWidgetRegistry();

    /**
     * Returns the global source select provider registry, used for managing all known source select widget factories.
     */
    static QgsSourceSelectProviderRegistry *sourceSelectProviderRegistry();

    /**
     * Returns the global shortcuts manager, used for managing a QAction and QShortcut sequences.
     */
    static QgsShortcutsManager *shortcutsManager();

    /**
     * Returns the global layer tree embedded widget registry, used for registering widgets that may be embedded into layer tree view.
     */
    static QgsLayerTreeEmbeddedWidgetRegistry *layerTreeEmbeddedWidgetRegistry();

    /**
     * Returns the global map layer action registry, used for registering map layer actions.
     */
    static QgsMapLayerActionRegistry *mapLayerActionRegistry();

    /**
     * Returns the global layout item GUI registry, used for registering the GUI behavior of layout items.
     */
    static QgsLayoutItemGuiRegistry *layoutItemGuiRegistry();

    /**
     * Returns the global processing gui registry, used for registering the GUI behavior of processing algorithms.
     * \since QGIS 3.2
     */
    static QgsProcessingGuiRegistry *processingGuiRegistry();

    /**
     * Returns the global processing recent algorithm log, used for tracking recently used processing algorithms.
     * \since QGIS 3.4
     */
    static QgsProcessingRecentAlgorithmLog *processingRecentAlgorithmLog();

    /**
     * Register the widget to allow its position to be automatically saved and restored when open and closed.
     * Use this to avoid needing to call saveGeometry() and restoreGeometry() on your widget.
     */
    static void enableAutoGeometryRestore( QWidget *widget, const QString &key = QString() );

    /**
     * Returns the global window manager, if set.
     * \see setWindowManager()
     * \since QGIS 3.4
     */
    static QgsWindowManagerInterface *windowManager();

    /**
     * Sets the global window \a manager. Ownership is transferred to the QgsGui instance.
     * \see windowManager()
     * \since QGIS 3.4
     */
    static void setWindowManager( QgsWindowManagerInterface *manager SIP_TRANSFER );

    ~QgsGui();

  private:

    QgsGui();

    QgsWidgetStateHelper *mWidgetStateHelper = nullptr;
    QgsNative *mNative = nullptr;
    QgsEditorWidgetRegistry *mEditorWidgetRegistry = nullptr;
    QgsSourceSelectProviderRegistry *mSourceSelectProviderRegistry = nullptr;
    QgsShortcutsManager *mShortcutsManager = nullptr;
    QgsLayerTreeEmbeddedWidgetRegistry *mLayerTreeEmbeddedWidgetRegistry = nullptr;
    QgsMapLayerActionRegistry *mMapLayerActionRegistry = nullptr;
    QgsLayoutItemGuiRegistry *mLayoutItemGuiRegistry = nullptr;
    QgsProcessingGuiRegistry *mProcessingGuiRegistry = nullptr;
    QgsProcessingRecentAlgorithmLog *mProcessingRecentAlgorithmLog = nullptr;
    std::unique_ptr< QgsWindowManagerInterface > mWindowManager;

#ifdef SIP_RUN
    QgsGui( const QgsGui &other );
#endif

};

#endif // QGSGUI_H
