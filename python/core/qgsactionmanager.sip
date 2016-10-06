/***************************************************************************
                          qgsactionmanager.sip

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

/** \class QgsActionManager
 * \brief Storage and management of actions associated with Qgis layer
 * attributes.
 */

class QgsActionManager
{
%TypeHeaderCode
#include <qgsactionmanager.h>
%End
  public:
    //! Constructor
    QgsActionManager( QgsVectorLayer *layer );

    //! Destructor
    virtual ~QgsActionManager();

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

    //! Remove an action at given index
    void removeAction( int index );

    /** Does the given values. defaultValueIndex is the index of the
     *  field to be used if the action has a $currfield placeholder.
     *  @note available in python bindings as doActionFeature
     */
    void doAction( int index,
                   const QgsFeature &feat,
                   int defaultValueIndex = 0 ) /PyName=doActionFeature/;

    /** Does the action using the expression engine to replace any embedded expressions
     * in the action definition.
     * @param index action index
     * @param feature feature to run action for
     * @param context expression context to evalute expressions under
     */
    void doAction( int index,
                   const QgsFeature& feature,
                   const QgsExpressionContext& context );

    //! Removes all actions
    void clearActions();

    //! List all actions
    QList<QgsAction> listActions() const;

    //! Return the layer
    QgsVectorLayer* layer() const;

    //! Writes the actions out in XML format
    bool writeXml( QDomNode& layer_node, QDomDocument& doc ) const;

    //! Reads the actions in in XML format
    bool readXml( const QDomNode& layer_node );

    int size() const;
    /**
     * Get the action at the specified index.
     */
    QgsAction at( int idx ) const /Factory/;
%MethodCode
  if ( a0 < 0 || a0 >= sipCpp->size() )
  {
    PyErr_SetString(PyExc_KeyError, QByteArray::number(a0));
    sipIsErr = 1;
  }
  else
  {
    sipRes = new QgsAction( sipCpp->at( a0 ) );
  }
%End

    /**
     * Get the action at the specified index.
     */
    QgsAction operator[]( int idx ) const;
%MethodCode
  if ( a0 < 0 || a0 >= sipCpp->size() )
  {
    PyErr_SetString(PyExc_KeyError, QByteArray::number(a0));
    sipIsErr = 1;
  }
  else
  {
    sipRes = new QgsAction( sipCpp->at( a0 ) );
  }
%End

    /**
     * Returns the index of the default action, or -1 if no default action is available.
     * @see setDefaultAction()
     */
    int defaultAction() const;

    /**
     * Set the index of the default action.
     * @param actionNumber index of action which should be made the default for the layer
     * @see defaultAction()
     */
    void setDefaultAction( int actionNumber );
};
