/***************************************************************************
                          qgsattributeaction.h

 These classes store and control the managment and execution of actions
 associated with particulay Qgis layers. Actions are defined to be
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
/* $Id$ */

#ifndef QGSATTRIBUTEACTION_H
#define QGSATTRIBUTEACTION_H

#include <QString>
#include <QObject>

#include <list>
#include <vector>
#include <utility>

class QDomNode;
class QDomDocument;


/** \ingroup core
 * Utility class that encapsulates an action based on vector attributes.
 */
class CORE_EXPORT QgsAction
{
  public:
    QgsAction( QString name, QString action, bool capture ) :
        mName( name ), mAction( action ), mCaptureOutput( capture ) {}

    //! The name of the action
    QString name() const { return mName; }

    //! The action
    QString action() const { return mAction; }

    //! Whether to capture output for display when this action is run
    bool capture() const { return mCaptureOutput; }

  private:
    QString mName;
    QString mAction;
    bool mCaptureOutput;
};

/*! \class QgsAttributeAction
 * \brief Storage and management of actions associated with Qgis layer
 * attributes.
 */

class  CORE_EXPORT QgsAttributeAction
{
  public:

    typedef std::list<QgsAction> AttributeActions;
    typedef AttributeActions::const_iterator aIter;

    //! Constructor
    QgsAttributeAction() {};

    //! Destructor
    virtual ~QgsAttributeAction() {};

    //! Add an action with the given name and action details.
    // Will happily have duplicate names and actions. If
    // capture is true, when running the action using doAction(),
    // any stdout from the process will be captured and displayed in a
    // dialog box.
    void addAction( QString name, QString action, bool capture = false );

    //! Does the action using the given values. defaultValueIndex is an
    // index into values which indicates which value in the values vector
    // is to be used if the action has a default placeholder.
    void doAction( unsigned int index, const std::vector< std::pair<QString, QString> > &values,
                   uint defaultValueIndex = 0 );

    //! Returns a const_iterator that points to the QgsAction at the
    // given position in the data collection. The insertion order is
    // preserved. The index starts at 0 and goes to one less than the
    // number of actions. An index outside that range will return an
    // a const_iterator equal to that returned by end().
    aIter retrieveAction( unsigned int index ) const;

    //! Removes all actions
    void clearActions() { mActions.clear(); }

    //! A const iterator to the start of the action pairs
    const AttributeActions::const_iterator begin() const
    { return mActions.begin(); }

    //! A const iterator to the one past the end of the action pairs
    const AttributeActions::const_iterator end() const
    { return mActions.end(); }

    //! Returns the number of stored actions
    int size() const { return mActions.size(); }

    //! Expands the given action, replacing all %'s with the value as
    // given.
    static QString expandAction( QString action, const std::vector< std::pair<QString, QString> > &values,
                                 uint defaultValueIndex );

    //! Writes the actions out in XML format
    bool writeXML( QDomNode& layer_node, QDomDocument& doc ) const;

    //! Reads the actions in in XML format
    bool readXML( const QDomNode& layer_node );

  private:

    // Stores the name/action pairs.
    AttributeActions mActions;
};

#endif
