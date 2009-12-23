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
#include <QList>
#include <QPair>

class QDomNode;
class QDomDocument;

/** \ingroup core
 * Utility class that encapsulates an action based on vector attributes.
 */
class CORE_EXPORT QgsAction
{
  public:
    enum ActionType
    {
      Generic,
      GenericPython,
      Mac,
      Windows,
      Unix,
    };

    QgsAction( ActionType type, QString name, QString action, bool capture ) :
        mType( type ), mName( name ), mAction( action ), mCaptureOutput( capture ) {}

    //! The name of the action
    QString name() const { return mName; }

    //! The action
    QString action() const { return mAction; }

    //! The action type
    ActionType type() const { return mType; }

    //! Whether to capture output for display when this action is run
    bool capture() const { return mCaptureOutput; }

    //! Whether the action is runable on the current platform
    bool runable() const
    {
      return mType == Generic ||
             mType == GenericPython ||
#if defined(Q_OS_WIN)
             mType == Windows
#elif defined(Q_OS_MAC)
             mType == Mac
#else
             mType == Unix
#endif
             ;
    }

  private:
    ActionType mType;
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
    //! Constructor
    QgsAttributeAction() {}

    //! Destructor
    virtual ~QgsAttributeAction() {}

    //! Add an action with the given name and action details.
    // Will happily have duplicate names and actions. If
    // capture is true, when running the action using doAction(),
    // any stdout from the process will be captured and displayed in a
    // dialog box.
    void addAction( QgsAction::ActionType type, QString name, QString action, bool capture = false );

    //! Does the action using the given values. defaultValueIndex is an
    // index into values which indicates which value in the values vector
    // is to be used if the action has a default placeholder.
    void doAction( int index, const QList< QPair<QString, QString> > &values,
                   int defaultValueIndex = 0, void ( *executePython )( const QString & ) = 0 );

    //! Removes all actions
    void clearActions() { mActions.clear(); }

    //! Expands the given action, replacing all %'s with the value as
    // given.
    static QString expandAction( QString action, const QList< QPair<QString, QString> > &values,
                                 uint defaultValueIndex );

    //! Writes the actions out in XML format
    bool writeXML( QDomNode& layer_node, QDomDocument& doc ) const;

    //! Reads the actions in in XML format
    bool readXML( const QDomNode& layer_node );

    int size() const { return mActions.size(); }
    QgsAction &at( int idx ) { return mActions[idx]; }
    QgsAction &operator[]( int idx ) { return mActions[idx]; }

  private:
    QList<QgsAction> mActions;
};

#endif
