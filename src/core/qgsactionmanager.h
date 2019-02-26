/***************************************************************************
                          qgsactionmanager.h

 These classes store and control the management and execution of actions
 associated with a particular Qgis layer. Actions are defined to be
 external programs that are run with user-specified inputs that can
 depend on the contents of layer attributes.

                             -------------------
    begin                : Oct 24 2004
    copyright            : (C) 2004 by Gavin Macaulay
    email                : gavin at macaulay dot co dot nz
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSACTIONMANAGER_H
#define QGSACTIONMANAGER_H

#include "qgis_core.h"
#include <QString>
#include <QIcon>
#include <QObject>

#include "qgsaction.h"
#include "qgsfeature.h"

class QDomNode;
class QDomDocument;
class QgsPythonUtils;
class QgsVectorLayer;
class QgsExpressionContextScope;
class QgsExpressionContext;

/**
 * \ingroup core
 * \class QgsActionManager
 * Storage and management of actions associated with a layer.
 *
 * Actions can trigger custom code or applications to be executed
 * based on attributes of a given feature.
 */

class CORE_EXPORT QgsActionManager: public QObject
{
    Q_OBJECT

  public:
    //! Constructor
    QgsActionManager( QgsVectorLayer *layer )
      : mLayer( layer )
    {}

    /**
     * Add an action with the given name and action details.
     * Will happily have duplicate names and actions. If
     * capture is TRUE, when running the action using doAction(),
     * any stdout from the process will be captured and displayed in a
     * dialog box.
     */
    QUuid addAction( QgsAction::ActionType type, const QString &name, const QString &command, bool capture = false );

    /**
     * Add an action with the given name and action details.
     * Will happily have duplicate names and actions. If
     * capture is TRUE, when running the action using doAction(),
     * any stdout from the process will be captured and displayed in a
     * dialog box.
     */
    QUuid addAction( QgsAction::ActionType type, const QString &name, const QString &command, const QString &icon, bool capture = false );

    /**
     * Add a new action to this list.
     */
    void addAction( const QgsAction &action );

    /**
     * Remove an action by its id.
     *
     * \since QGIS 3.0
     */
    void removeAction( QUuid actionId );

    /**
     * Does the given action.
     *
     * \param actionId action id
     * \param feature feature to run action for
     * \param defaultValueIndex index of the field to be used if the action has a $currfield placeholder.
     * \param scope expression context scope to add during expression evaluation
     *
     * \note available in Python bindings as doActionFeature
     */
    void doAction( QUuid actionId, const QgsFeature &feature, int defaultValueIndex = 0, const QgsExpressionContextScope &scope = QgsExpressionContextScope() ) SIP_PYNAME( doActionFeature );

    /**
     * Does the action using the expression engine to replace any embedded expressions
     * in the action definition.
     * \param actionId action id
     * \param feature feature to run action for
     * \param context expression context to evaluate expressions under
     */
    void doAction( QUuid actionId, const QgsFeature &feature, const QgsExpressionContext &context );

    //! Removes all actions
    void clearActions();

    /**
     * Returns a list of actions that are available in the given action scope.
     * If no action scope is provided, all actions will be returned.
     *
     * \since QGIS 3.0
     */
    QList<QgsAction> actions( const QString &actionScope = QString() ) const;

    //! Returns the layer
    QgsVectorLayer *layer() const { return mLayer; }

    //! Writes the actions out in XML format
    bool writeXml( QDomNode &layer_node ) const;

    //! Reads the actions in in XML format
    bool readXml( const QDomNode &layer_node );

    /**
     * Gets an action by its id.
     *
     * \since QGIS 3.0
     */
    QgsAction action( QUuid id );

    /**
     * Each scope can have a default action. This will be saved in the project
     * file.
     *
     * \since QGIS 3.0
     */
    void setDefaultAction( const QString &actionScope, QUuid actionId );

    /**
     * Each scope can have a default action. This will be saved in the project
     * file.
     *
     * \since QGIS 3.0
     */
    QgsAction defaultAction( const QString &actionScope );

  private:
    QList<QgsAction> mActions;
    QgsVectorLayer *mLayer = nullptr;
    static void ( *smPythonExecute )( const QString & );

    void runAction( const QgsAction &action );

    QMap<QString, QUuid> mDefaultActions;

    bool mOnNotifyConnected = false;

    QgsExpressionContext createExpressionContext() const;

  private slots:
    void onNotifyRunActions( const QString &message );
};

#endif
