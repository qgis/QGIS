/******************************************************************
 * This file has been generated automatically by sipify.pl        *
 * Do not edit manually ! Edit header file and generate it again. *
 *****************************************************************/


class QgsActionScopeRegistry : QObject
{
%Docstring
 \ingroup core
 The action scope registry is an application wide registry that
 contains a list of available action scopes.
 Some scopes are available by default, additional ones can be registered
 at runtime by plugins or custom applications.

 To get access use the QgsApplication:

 ```
 QgsApplication::actionScopeRegistry()
 ```

 @note Added in QGIS 3.0
%End

%TypeHeaderCode
#include "qgsactionscoperegistry.h"
%End

  public:

    explicit QgsActionScopeRegistry( QObject *parent = 0 );
%Docstring
 Create a new QgsActionScopeRegistry.
 QGIS already creates a central registry. You will normally
 want to use QgsApplication::actionScopeRegistry() to get access
 to that one instead.

 @note Added in QGIS 3.0
%End

    QSet<QgsActionScope> actionScopes() const;
%Docstring
 Get all registered action scopes.

 @note Added in QGIS 3.0
%End

    void registerActionScope( const QgsActionScope &actionScope );
%Docstring
 Register an additional action scope.

 @note Added in QGIS 3.0
%End

    void unregisterActionScope( const QgsActionScope &actionScope );
%Docstring
 Unregister an additional action scope.

 @note Added in QGIS 3.0
%End

    QgsActionScope actionScope( const QString &id );
%Docstring
 Get an action scope by its id.

 @note Added in QGIS 3.0
%End

  signals:

    void actionScopesChanged();
%Docstring
 Emitted whenever a new action scope is registered or an action scope
 is unregistered.

 @note Added in QGIS 3.0
%End

};

