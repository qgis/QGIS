/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsgui.h                                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsGui
{
%Docstring
 QgsGui is a singleton class containing various registry and other global members
 related to GUI classes.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsgui.h"
%End
  public:



    static QgsGui *instance();
%Docstring
 Returns a pointer to the singleton instance.
 :rtype: QgsGui
%End


    static QgsEditorWidgetRegistry *editorWidgetRegistry();
%Docstring
 Returns the global editor widget registry, used for managing all known edit widget factories.
 :rtype: QgsEditorWidgetRegistry
%End

    static QgsSourceSelectProviderRegistry *sourceSelectProviderRegistry();
%Docstring
 Returns the global source select provider registry, used for managing all known source select widget factories.
 :rtype: QgsSourceSelectProviderRegistry
%End

    static QgsShortcutsManager *shortcutsManager();
%Docstring
 Returns the global shortcuts manager, used for managing a QAction and QShortcut sequences.
 :rtype: QgsShortcutsManager
%End

    static QgsLayerTreeEmbeddedWidgetRegistry *layerTreeEmbeddedWidgetRegistry();
%Docstring
 Returns the global layer tree embedded widget registry, used for registering widgets that may be embedded into layer tree view.
 :rtype: QgsLayerTreeEmbeddedWidgetRegistry
%End

    static QgsMapLayerActionRegistry *mapLayerActionRegistry();
%Docstring
 Returns the global map layer action registry, used for registering map layer actions.
 :rtype: QgsMapLayerActionRegistry
%End

    static QgsLayoutItemGuiRegistry *layoutItemGuiRegistry();
%Docstring
 Returns the global layout item GUI registry, used for registering the GUI behavior of layout items.
 :rtype: QgsLayoutItemGuiRegistry
%End

    ~QgsGui();

  private:
    QgsGui( const QgsGui &other );
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsgui.h                                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
