/***************************************************************************
                          qgsactionmanager.h

 These classes store and control the managment and execution of actions
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

#include <QString>
#include <QIcon>

#include "qgsaction.h"
#include "qgsfeature.h"

class QDomNode;
class QDomDocument;
class QgsPythonUtils;
class QgsVectorLayer;
class QgsExpressionContextScope;
class QgsExpressionContext;

/** \ingroup core
 * \class QgsActionManager
 * Storage and management of actions associated with a layer.
 *
 * Actions can trigger custom code or applications to be executed
 * based on attributes of a given feature.
 */

class CORE_EXPORT QgsActionManager
{
  public:
    //! Constructor
    QgsActionManager( QgsVectorLayer *layer )
        : mLayer( layer )
    {}

    /** Add an action with the given name and action details.
     * Will happily have duplicate names and actions. If
     * capture is true, when running the action using doAction(),
     * any stdout from the process will be captured and displayed in a
     * dialog box.
     */
    void addAction( QgsAction::ActionType type, const QString& name, const QString& action, bool capture = false );

    /** Add an action with the given name and action details.
     * Will happily have duplicate names and actions. If
     * capture is true, when running the action using doAction(),
     * any stdout from the process will be captured and displayed in a
     * dialog box.
     */
    void addAction( QgsAction::ActionType type, const QString& name, const QString& action, const QString& icon, bool capture = false );

    /**
     * Add a new action to this list.
     */
    void addAction( const QgsAction& action );

    /** Does the given values. defaultValueIndex is the index of the
     *  field to be used if the action has a $currfield placeholder.
     *  @note available in python bindings as doActionFeature
     */
    void doAction( const QString& actionId,
                   const QgsFeature &feature,
                   int defaultValueIndex = 0 );

    /** Does the action using the expression engine to replace any embedded expressions
     * in the action definition.
     * @param index action index
     * @param feature feature to run action for
     * @param context expression context to evalute expressions under
     */
    void doAction( const QString& actionId,
                   const QgsFeature& feature,
                   const QgsExpressionContext& context );

    //! Removes all actions
    void clearActions();

    /**
     * Return a list of actions that are available in the given action scope.
     * If no action scope is provided, all actions will be returned.
     */
    QList<QgsAction> listActions( const QString& actionScope = QString() ) const;

    //! Return the layer
    QgsVectorLayer* layer() const { return mLayer; }

    //! Writes the actions out in XML format
    bool writeXml( QDomNode& layer_node, QDomDocument& doc ) const;

    //! Reads the actions in in XML format
    bool readXml( const QDomNode& layer_node );

    /**
     * Get an action by its id.
     *
     * @note Added in QGIS 3.0
     */
    QgsAction action( const QString& id );

    /**
     * Each scope can have a default action. This will be saved in the project
     * file.
     *
     * @note Added in QGIS 3.0
     */
    void setDefaultAction( const QString& actionScope, const QString& actionId );

    /**
     * Each scope can have a default action. This will be saved in the project
     * file.
     *
     * @note Added in QGIS 3.0
     */
    QgsAction defaultAction( const QString& actionScope );

  private:
    QList<QgsAction> mActions;
    QgsVectorLayer *mLayer;
    static void ( *smPythonExecute )( const QString & );

    void runAction( const QgsAction &action );

    QVariantMap mDefaultActions;

    QgsExpressionContext createExpressionContext() const;
};

#endif
