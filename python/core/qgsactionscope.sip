/******************************************************************
 * This file has been generated automatically by sipify.pl        *
 * Do not edit manually ! Edit header file and generate it again. *
 *****************************************************************/



class QgsActionScope
{

%TypeHeaderCode
#include "qgsactionscope.h"
%End
  public:

    explicit QgsActionScope();
%Docstring
 Creates a new invalid action scope.

 @note Added in QGSI 3.0
%End

    explicit QgsActionScope( const QString &id, const QString &title, const QString &description, const QgsExpressionContextScope &expressionContextScope = QgsExpressionContextScope() );
%Docstring
 Creates a new action scope.
 For details concerning the parameters check the documentation
 of the corresponding properties.
%End

    bool operator==( const QgsActionScope &other ) const;
%Docstring
 Compares two action scopes
%End

    QgsExpressionContextScope expressionContextScope() const;
%Docstring
 An expression scope may offer additional variables for an action scope.
 This can be an `field_name` for the attribute which was clicked or
 `clicked_x` and `clicked_y` for actions which are available as map canvas clicks.

 @note Added in QGIS 3.0
%End

    void setExpressionContextScope( const QgsExpressionContextScope &expressionContextScope );
%Docstring
 \copydoc expressionContextScope()
%End

    QString id() const;
%Docstring
 A unique identifier for this action scope.

 @note Added in QGIS 3.0
%End

    void setId( const QString &id );
%Docstring
\copydoc id()
%End

    QString title() const;
%Docstring
 The title is a human readable and translated string that will be
 presented to the user in the properties dialog.

 @note Added in QGIS 3.0
%End
    void setTitle( const QString &title );
%Docstring
\copydoc title()
%End

    QString description() const;
%Docstring
 The description should be a longer description of where actions in this scope
 are available. It is not necessary to list the available expression variables
 in here, they are extracted automatically from the expressionContextScope().

 @note Added in QGIS 3.0
%End
    void setDescription( const QString &description );
%Docstring
\copydoc description()
%End

    bool isValid() const;
%Docstring
 Returns if this scope is valid.

 @note Added in QGIS 3.0
%End

};


