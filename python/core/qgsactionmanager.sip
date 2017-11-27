/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsactionmanager.h                                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/







class QgsActionManager: QObject
{
%Docstring
 Storage and management of actions associated with a layer.

 Actions can trigger custom code or applications to be executed
 based on attributes of a given feature.
%End

%TypeHeaderCode
#include "qgsactionmanager.h"
%End
  public:
    QgsActionManager( QgsVectorLayer *layer );
%Docstring
Constructor
%End

    QUuid addAction( QgsAction::ActionType type, const QString &name, const QString &command, bool capture = false );
%Docstring
 Add an action with the given name and action details.
 Will happily have duplicate names and actions. If
 capture is true, when running the action using doAction(),
 any stdout from the process will be captured and displayed in a
 dialog box.
 :rtype: QUuid
%End

    QUuid addAction( QgsAction::ActionType type, const QString &name, const QString &command, const QString &icon, bool capture = false );
%Docstring
 Add an action with the given name and action details.
 Will happily have duplicate names and actions. If
 capture is true, when running the action using doAction(),
 any stdout from the process will be captured and displayed in a
 dialog box.
 :rtype: QUuid
%End

    void addAction( const QgsAction &action );
%Docstring
 Add a new action to this list.
%End

    void removeAction( const QUuid &actionId );
%Docstring
 Remove an action by its id.

.. versionadded:: 3.0
%End

    void doAction( const QUuid &actionId, const QgsFeature &feature, int defaultValueIndex = 0, const QgsExpressionContextScope &scope = QgsExpressionContextScope() ) /PyName=doActionFeature/;
%Docstring
 Does the given action.

 \param actionId action id
 \param feature feature to run action for
 \param defaultValueIndex index of the field to be used if the action has a $currfield placeholder.
 \param scope expression context scope to add during expression evaluation

.. note::

   available in Python bindings as doActionFeature
%End

    void doAction( const QUuid &actionId, const QgsFeature &feature, const QgsExpressionContext &context );
%Docstring
 Does the action using the expression engine to replace any embedded expressions
 in the action definition.
 \param actionId action id
 \param feature feature to run action for
 \param context expression context to evaluate expressions under
%End

    void clearActions();
%Docstring
Removes all actions
%End

    QList<QgsAction> actions( const QString &actionScope = QString() ) const;
%Docstring
 Return a list of actions that are available in the given action scope.
 If no action scope is provided, all actions will be returned.

.. versionadded:: 3.0
 :rtype: list of QgsAction
%End

    QgsVectorLayer *layer() const;
%Docstring
Return the layer
 :rtype: QgsVectorLayer
%End

    bool writeXml( QDomNode &layer_node ) const;
%Docstring
Writes the actions out in XML format
 :rtype: bool
%End

    bool readXml( const QDomNode &layer_node );
%Docstring
Reads the actions in in XML format
 :rtype: bool
%End

    QgsAction action( const QUuid &id );
%Docstring
 Get an action by its id.

.. versionadded:: 3.0
 :rtype: QgsAction
%End

    void setDefaultAction( const QString &actionScope, const QUuid &actionId );
%Docstring
 Each scope can have a default action. This will be saved in the project
 file.

.. versionadded:: 3.0
%End

    QgsAction defaultAction( const QString &actionScope );
%Docstring
 Each scope can have a default action. This will be saved in the project
 file.

.. versionadded:: 3.0
 :rtype: QgsAction
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsactionmanager.h                                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
