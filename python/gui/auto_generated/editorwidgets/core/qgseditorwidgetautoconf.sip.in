/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/editorwidgets/core/qgseditorwidgetautoconf.h                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsEditorWidgetAutoConfPlugin
{
%Docstring
Base class for plugins allowing to pick automatically a widget type for editing fields.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgseditorwidgetautoconf.h"
%End
  public:

    virtual ~QgsEditorWidgetAutoConfPlugin();

    virtual QgsEditorWidgetSetup editorWidgetSetup( const QgsVectorLayer *vl, const QString &fieldName, int &score /Out/ ) const = 0;
%Docstring
Typical scores are:
* 0: no matching type found.
* 10: a widget has been guessed from the type of field.
* 20: a widget has been determined from an external configuration (for example a database table)

:param vl: The vector layer for which this widget will be created
:param fieldName: The field name on the specified layer for which this widget will be created

:return: - and integer value rating how good is the setup provided by this plugin.
         - score: Where the score is returned (default to 0)
%End

};




/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/editorwidgets/core/qgseditorwidgetautoconf.h                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
