/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgseditorwidgetsetup.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsEditorWidgetSetup
{
%Docstring
 Holder for the widget type and its configuration for a field.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgseditorwidgetsetup.h"
%End
  public:

    QgsEditorWidgetSetup( const QString &type, const QVariantMap &config );
%Docstring
 Constructor
%End

    QgsEditorWidgetSetup();
%Docstring
Constructor for QgsEditorWidgetSetup
%End

    QString type() const;
%Docstring
 :return: the widget type to use
 :rtype: str
%End

    QVariantMap config() const;
%Docstring
 :return: the widget configuration to used
 :rtype: QVariantMap
%End

    bool isNull() const;
%Docstring
 :return: true if there is no widget configured.
 :rtype: bool
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgseditorwidgetsetup.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
