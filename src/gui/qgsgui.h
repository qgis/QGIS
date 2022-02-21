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

class QgsSettingsRegistryGui;
class QgsEditorWidgetRegistry;
class QgsShortcutsManager;
class QgsLayerTreeEmbeddedWidgetRegistry;
class QgsMapLayerActionRegistry;
class QgsSourceSelectProviderRegistry;
class QgsNative;
class QgsLayoutItemGuiRegistry;
class QgsAnnotationItemGuiRegistry;
class QgsWidgetStateHelper;
class QgsProcessingGuiRegistry;
class QgsProcessingRecentAlgorithmLog;
class QgsWindowManagerInterface;
class QgsDataItemGuiProviderRegistry;
class QgsProviderGuiRegistry;
class QgsProjectStorageGuiRegistry;
class QgsNumericFormatGuiRegistry;
class QgsCodeEditorColorSchemeRegistry;
class QgsMessageBar;
class QgsSubsetStringEditorProviderRegistry;
class QgsProviderSourceWidgetProviderRegistry;
class QgsRelationWidgetRegistry;
class QgsMapToolShapeRegistry;
class QgsHistoryProviderRegistry;

/**
 * \ingroup gui
 * \brief QgsGui is a singleton class containing various registry and other global members
 * related to GUI classes.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsGui : public QObject
{
    Q_OBJECT

  public:

    /**
     * Defines the behavior to use when setting the CRS for a newly created project.
     */
    enum ProjectCrsBehavior
    {
      UseCrsOfFirstLayerAdded = 1, //!< Set the project CRS to the CRS of the first layer added to a new project
      UsePresetCrs = 2, //!< Always set new projects to use a preset default CRS
    };
    Q_ENUM( ProjectCrsBehavior )

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
     * Returns the gui's settings registry, used for managing gui settings.
     * \since QGIS 3.22
     */
    static QgsSettingsRegistryGui *settingsRegistryGui() SIP_KEEPREFERENCE;

    /**
     * Returns the global editor widget registry, used for managing all known edit widget factories.
     */
    static QgsEditorWidgetRegistry *editorWidgetRegistry() SIP_KEEPREFERENCE;

    /**
     * Returns the global source select provider registry, used for managing all known source select widget factories.
     */
    static QgsSourceSelectProviderRegistry *sourceSelectProviderRegistry() SIP_KEEPREFERENCE;

    /**
     * Returns the global shortcuts manager, used for managing a QAction and QShortcut sequences.
     */
    static QgsShortcutsManager *shortcutsManager();

    /**
     * Returns the global layer tree embedded widget registry, used for registering widgets that may be embedded into layer tree view.
     */
    static QgsLayerTreeEmbeddedWidgetRegistry *layerTreeEmbeddedWidgetRegistry() SIP_KEEPREFERENCE;

    /**
     * Returns the global map layer action registry, used for registering map layer actions.
     */
    static QgsMapLayerActionRegistry *mapLayerActionRegistry() SIP_KEEPREFERENCE;

    /**
     * Returns the global layout item GUI registry, used for registering the GUI behavior of layout items.
     */
    static QgsLayoutItemGuiRegistry *layoutItemGuiRegistry() SIP_KEEPREFERENCE;

    /**
     * Returns the global annotation item GUI registry, used for registering the GUI behavior of annotation items.
     *
     * \since QGIS 3.22
     */
    static QgsAnnotationItemGuiRegistry *annotationItemGuiRegistry() SIP_KEEPREFERENCE;

    /**
     * Returns the global processing gui registry, used for registering the GUI behavior of processing algorithms.
     * \since QGIS 3.2
     */
    static QgsProcessingGuiRegistry *processingGuiRegistry() SIP_KEEPREFERENCE;

    /**
     * Returns the global numeric format gui registry, used for registering the GUI widgets associated with QgsNumericFormats.
     * \since QGIS 3.12
     */
    static QgsNumericFormatGuiRegistry *numericFormatGuiRegistry() SIP_KEEPREFERENCE;

    /**
     * Returns the global code editor color scheme registry, used for registering the color schemes for QgsCodeEditor widgets.
     * \since QGIS 3.16
     */
    static QgsCodeEditorColorSchemeRegistry *codeEditorColorSchemeRegistry() SIP_KEEPREFERENCE;

    /**
     * Returns the global processing recent algorithm log, used for tracking recently used processing algorithms.
     * \since QGIS 3.4
     */
    static QgsProcessingRecentAlgorithmLog *processingRecentAlgorithmLog();

    /**
     * Returns the global data item GUI provider registry, used for tracking providers which affect the browser
     * GUI.
     * \since QGIS 3.6
     */
    static QgsDataItemGuiProviderRegistry *dataItemGuiProviderRegistry() SIP_KEEPREFERENCE;

    /**
     * Returns the global GUI-related project storage registry
     * \since QGIS 3.10
     */
    static QgsProjectStorageGuiRegistry *projectStorageGuiRegistry() SIP_KEEPREFERENCE;

    /**
     * Returns the registry of GUI-related components of data providers
     * \since QGIS 3.10
     */
    static QgsProviderGuiRegistry *providerGuiRegistry() SIP_KEEPREFERENCE;

    /**
     * Returns the registry of subset string editors of data providers
     * \since QGIS 3.18
     */
    static QgsSubsetStringEditorProviderRegistry *subsetStringEditorProviderRegistry() SIP_KEEPREFERENCE;

    /**
     * Returns the registry of provider source widget providers.
     * \since QGIS 3.18
     */
    static QgsProviderSourceWidgetProviderRegistry *sourceWidgetProviderRegistry() SIP_KEEPREFERENCE;

    /**
     * Returns the global relation widget registry, used for managing all known relation widget factories.
    * \since QGIS 3.18
     */
    static QgsRelationWidgetRegistry *relationWidgetRegistry() SIP_KEEPREFERENCE;

    /**
     * Returns the registry of shape map tools
     * \note Not available in Python bindings
    * \since QGIS 3.26
     */
    static QgsMapToolShapeRegistry *mapToolShapeRegistry() SIP_SKIP;

    /**
     * Returns the global history provider registry, used for tracking history providers.
     * \since QGIS 3.24
     */
    static QgsHistoryProviderRegistry *historyProviderRegistry() SIP_KEEPREFERENCE;

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

    /**
     * HIG flags, which indicate the Human Interface Guidelines for the current platform.
     * \since QGIS 3.4
    */
    enum HigFlag
    {
      HigMenuTextIsTitleCase = 1 << 0,       //!< Menu action texts should be title case
      HigDialogTitleIsTitleCase = 1 << 1     //!< Dialog titles should be title case
    };
    Q_DECLARE_FLAGS( HigFlags, HigFlag )

    /**
    * Returns the platform's HIG flags.
    * \since QGIS 3.4
    */
    static QgsGui::HigFlags higFlags();

    ~QgsGui();

    /**
     * Samples the color on screen at the specified global \a point (pixel).
     *
     * \since QGIS 3.10
     */
    static QColor sampleColor( QPoint point );

    /**
     * Returns the screen at the given global \a point (pixel).
     *
     * \since QGIS 3.10
     */
    static QScreen *findScreenAt( QPoint point );

    /**
     * Returns TRUE if python macros are currently allowed to be run
     * If the global option is to ask user, a modal dialog will be shown
     * \param lambda a pointer to a lambda method. If specified, the dialog is not modal,
     * a message is shown with a button to enable macro.
     * The lambda will be run either if macros are currently allowed or if the user accepts the message.
     * The \a messageBar must be given in such case.
     * \param messageBar the message bar must be provided if a lambda method is used.
     */
    static bool pythonMacroAllowed( void ( *lambda )() = nullptr, QgsMessageBar *messageBar = nullptr ) SIP_SKIP;

    ///@cond PRIVATE
    void emitOptionsChanged() SIP_SKIP;
    ///@endcond

  signals:

    /**
     * This signal is emitted whenever the application options have been changed.
     *
     * This signal is a "blanket" signal, and will be emitted whenever the options dialog
     * has been accepted regardless of whether or not individual settings are changed.
     * It is designed as a "last resort" fallback only, allowing widgets to respond
     * to possible settings changes.
     *
     * \since QGIS 3.16
     */
    void optionsChanged();

  private:

    QgsGui();

    QgsSettingsRegistryGui *mSettingsRegistryGui = nullptr;
    QgsProviderGuiRegistry *mProviderGuiRegistry = nullptr;
    QgsWidgetStateHelper *mWidgetStateHelper = nullptr;
    QgsNative *mNative = nullptr;
    QgsEditorWidgetRegistry *mEditorWidgetRegistry = nullptr;
    QgsSourceSelectProviderRegistry *mSourceSelectProviderRegistry = nullptr;
    QgsShortcutsManager *mShortcutsManager = nullptr;
    QgsLayerTreeEmbeddedWidgetRegistry *mLayerTreeEmbeddedWidgetRegistry = nullptr;
    QgsMapLayerActionRegistry *mMapLayerActionRegistry = nullptr;
    QgsLayoutItemGuiRegistry *mLayoutItemGuiRegistry = nullptr;
    QgsAnnotationItemGuiRegistry *mAnnotationItemGuiRegistry = nullptr;
    QgsProcessingGuiRegistry *mProcessingGuiRegistry = nullptr;
    QgsProcessingRecentAlgorithmLog *mProcessingRecentAlgorithmLog = nullptr;
    QgsNumericFormatGuiRegistry *mNumericFormatGuiRegistry = nullptr;
    QgsDataItemGuiProviderRegistry *mDataItemGuiProviderRegistry = nullptr;
    QgsCodeEditorColorSchemeRegistry *mCodeEditorColorSchemeRegistry = nullptr;
    QgsProjectStorageGuiRegistry *mProjectStorageGuiRegistry = nullptr;
    QgsSubsetStringEditorProviderRegistry *mSubsetStringEditorProviderRegistry = nullptr;
    QgsProviderSourceWidgetProviderRegistry *mProviderSourceWidgetProviderRegistry = nullptr;
    QgsRelationWidgetRegistry *mRelationEditorRegistry = nullptr;
    QgsMapToolShapeRegistry *mShapeMapToolRegistry = nullptr;
    QgsHistoryProviderRegistry *mHistoryProviderRegistry = nullptr;
    std::unique_ptr< QgsWindowManagerInterface > mWindowManager;

#ifdef SIP_RUN
    QgsGui( const QgsGui &other );
#endif

};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsGui::HigFlags )

#endif // QGSGUI_H
