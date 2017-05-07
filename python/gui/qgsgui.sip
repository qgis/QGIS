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

    static QgsShortcutsManager *shortcutsManager();
%Docstring
 Returns the global shortcuts manager, used for managing a QAction and QShortcut sequences.
 :rtype: QgsShortcutsManager
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
