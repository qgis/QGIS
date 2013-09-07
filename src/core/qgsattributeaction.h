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

#ifndef QGSATTRIBUTEACTION_H
#define QGSATTRIBUTEACTION_H

#include <QString>
#include <QObject>

#include <qgsfeature.h>

class QDomNode;
class QDomDocument;
class QgsPythonUtils;
class QgsVectorLayer;

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
      OpenUrl,
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
             mType == OpenUrl ||
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
    QgsAttributeAction( QgsVectorLayer *layer ) : mLayer( layer ) {}

    //! Destructor
    virtual ~QgsAttributeAction() {}

    //! Add an action with the given name and action details.
    // Will happily have duplicate names and actions. If
    // capture is true, when running the action using doAction(),
    // any stdout from the process will be captured and displayed in a
    // dialog box.
    void addAction( QgsAction::ActionType type, QString name, QString action, bool capture = false );

    //! Remove an action at given index
    void removeAction( int index );

    /*! Does the given values. defaultValueIndex is the index of the
     *  field to be used if the action has a $currfield placeholder.
     *  @note added in 1.9
     *  @note available in python bindings as doActionFeature
     */
    void doAction( int index,
                   QgsFeature &feat,
                   int defaultValueIndex = 0 );

    /*! Does the action using the expression builder to expand it
     *  and getting values from the passed feature attribute map.
     *  substitutionMap is used to pass custom substitutions, to replace
     *  each key in the map with the associated value
     *  @note added in 1.9
     *  @note available in python bindings as doActionFeatureWithSubstitution
     */
    void doAction( int index,
                   QgsFeature &feat,
                   const QMap<QString, QVariant> *substitutionMap = 0 );

    //! Removes all actions
    void clearActions() { mActions.clear(); }

    //! List all actions
    const QList<QgsAction>& listActions() { return mActions; }

    //! Return the layer
    QgsVectorLayer *layer() { return mLayer; }

    /*! Expands the given action, replacing all %'s with the value as
     *  given.
     */
    QString expandAction( QString action, const QgsAttributeMap &attributes, uint defaultValueIndex );

    /*! Expands the given action using the expression builder
     *  This function currently replaces each expression between [% and %]
     *  placeholders in the action with the result of its evaluation on
     *  the feature passed as argument.
     *
     *  Additional substitutions can be passed through the substitutionMap
     *  parameter
     *
     *  @note added in 1.9
     */
    QString expandAction( QString action,
                          QgsFeature &feat,
                          const QMap<QString, QVariant> *substitutionMap = 0 );


    //! Writes the actions out in XML format
    bool writeXML( QDomNode& layer_node, QDomDocument& doc ) const;

    //! Reads the actions in in XML format
    bool readXML( const QDomNode& layer_node );

    int size() const { return mActions.size(); }
    QgsAction &at( int idx ) { return mActions[idx]; }
    QgsAction &operator[]( int idx ) { return mActions[idx]; }

    //! @deprecated Initialize QgsPythonRunner instead
    static void setPythonExecute( void ( * )( const QString & ) );

    //! Whether the action is the default action
    int defaultAction() const { return mDefaultAction < 0 || mDefaultAction >= size() ? 0 : mDefaultAction; }
    void setDefaultAction( int actionNumber ) { mDefaultAction = actionNumber ; }

  private:
    QList<QgsAction> mActions;
    QgsVectorLayer *mLayer;
    static void ( *smPythonExecute )( const QString & );

    void runAction( const QgsAction &action,
                    void ( *executePython )( const QString & ) = 0 );

    int mDefaultAction;
};

#endif
