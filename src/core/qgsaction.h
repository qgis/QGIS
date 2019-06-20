/***************************************************************************
  qgsaction.h - QgsAction

 ---------------------
 begin                : 18.4.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSACTION_H
#define QGSACTION_H

#include "qgis_core.h"
#include <QSet>
#include <QString>
#include <QIcon>
#include <QAction>
#include <QUuid>

#include "qgsexpressioncontext.h"
#include <memory>

class QgsExpressionContextScope;

/**
 * \ingroup core
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

    /**
     * Default constructor
     */
    QgsAction() = default;

    /**
     * Create a new QgsAction
     *
     * \param type          The type of this action
     * \param description   A human readable description string
     * \param command       The action text. Its interpretation depends on the type
     * \param capture       If this is set to TRUE, the output will be captured when an action is run
     * \param enabledOnlyWhenEditable if TRUE then action is only enable in editmode
     */
#ifndef SIP_RUN
    QgsAction( ActionType type, const QString &description, const QString &command, bool capture = false, bool enabledOnlyWhenEditable = false )
      : mType( type )
      , mDescription( description )
      , mCommand( command )
      , mCaptureOutput( capture )
      , mId( QUuid::createUuid() )
      , mIsEnabledOnlyWhenEditable( enabledOnlyWhenEditable )
    {}
#else
    QgsAction( ActionType type, const QString &description, const QString &command, bool capture = false )
      : mType( type )
      , mDescription( description )
      , mCommand( command )
      , mCaptureOutput( capture )
      , mId( QUuid::createUuid() )
      , mIsEnabledOnlyWhenEditable( enabledOnlyWhenEditable )
    {}
#endif

    /**
     * Create a new QgsAction
     *
     * \param type                 The type of this action
     * \param description          A human readable description string
     * \param action               The action text. Its interpretation depends on the type
     * \param icon                 Path to an icon for this action
     * \param capture              If this is set to TRUE, the output will be captured when an action is run
     * \param shortTitle           A short string used to label user interface elements like buttons
     * \param actionScopes         A set of scopes in which this action will be available
     * \param notificationMessage  A particular message which reception will trigger the action
     * \param enabledOnlyWhenEditable if TRUE then action is only enable in editmode
     */
#ifndef SIP_RUN
    QgsAction( ActionType type, const QString &description, const QString &action, const QString &icon, bool capture, const QString &shortTitle = QString(), const QSet<QString> &actionScopes = QSet<QString>(), const QString &notificationMessage = QString(), bool enabledOnlyWhenEditable = false )
      : mType( type )
      , mDescription( description )
      , mShortTitle( shortTitle )
      , mIcon( icon )
      , mCommand( action )
      , mCaptureOutput( capture )
      , mActionScopes( actionScopes )
      , mNotificationMessage( notificationMessage )
      , mId( QUuid::createUuid() )
      , mIsEnabledOnlyWhenEditable( enabledOnlyWhenEditable )
    {}
#else
    QgsAction( ActionType type, const QString &description, const QString &action, const QString &icon, bool capture, const QString &shortTitle = QString(), const QSet<QString> &actionScopes = QSet<QString>(), const QString &notificationMessage = QString() )
      : mType( type )
      , mDescription( description )
      , mShortTitle( shortTitle )
      , mIcon( icon )
      , mCommand( action )
      , mCaptureOutput( capture )
      , mActionScopes( actionScopes )
      , mNotificationMessage( notificationMessage )
      , mId( QUuid::createUuid() )
      , mIsEnabledOnlyWhenEditable( enabledOnlyWhenEditable )
    {}
#endif

    //! The name of the action. This may be a longer description.
    QString name() const { return mDescription; }

    //! The short title is used to label user interface elements like buttons
    QString shortTitle() const { return mShortTitle; }

    /**
     * Returns a unique id for this action.
     *
     * \since QGIS 3.0
     */
    QUuid id() const { return mId; }

    /**
     * Returns TRUE if this action was a default constructed one.
     *
     * \since QGIS 3.0
     */
    bool isValid() const { return !mId.isNull(); }

    //! The path to the icon
    QString iconPath() const { return mIcon; }

    //! The icon
    QIcon icon() const { return QIcon( mIcon ); }

    /**
     * Returns the command that is executed by this action.
     * How the content is interpreted depends on the type() and
     * the actionScope().
     *
     * \since QGIS 3.0
     */
    QString command() const { return mCommand; }

    /**
     * Returns the notification message that triggers the action
     *
     * \since QGIS 3.0
     */
    QString notificationMessage() const { return mNotificationMessage; }

    //! The action type
    ActionType type() const { return mType; }

    //! Whether to capture output for display when this action is run
    bool capture() const { return mCaptureOutput; }


    //! Returns whether only enabled in editable mode
    bool isEnabledOnlyWhenEditable() const { return mIsEnabledOnlyWhenEditable; }


    //! Checks if the action is runable on the current platform
    bool runable() const;

    /**
     * Run this action.
     *
     * \since QGIS 3.0
     */
    void run( QgsVectorLayer *layer, const QgsFeature &feature, const QgsExpressionContext &expressionContext ) const;

    /**
     * Run this action.
     *
     * \since QGIS 3.0
     */
    void run( const QgsExpressionContext &expressionContext ) const;

    /**
     * The action scopes define where an action will be available.
     * Action scopes may offer additional variables like the clicked
     * coordinate.
     *
     * \see QgsActionScope
     * \since QGIS 3.0
     */
    QSet<QString> actionScopes() const;

    /**
     * The action scopes define where an action will be available.
     * Action scopes may offer additional variables like the clicked
     * coordinate.
     *
     * \since QGIS 3.0
     */
    void setActionScopes( const QSet<QString> &actionScopes );

    /**
     * Reads an XML definition from actionNode
     * into this object.
     *
     * \since QGIS 3.0
     */
    void readXml( const QDomNode &actionNode );

    /**
     * Appends an XML definition for this action as a new
     * child node to actionsNode.
     *
     * \since QGIS 3.0
     */
    void writeXml( QDomNode &actionsNode ) const;

    /**
     * Sets an expression context scope to use for running the action.
     *
     * \since QGIS 3.0
     */
    void setExpressionContextScope( const QgsExpressionContextScope &scope );

    /**
     * Returns an expression context scope used for running the action.
     *
     * \since QGIS 3.0
     */
    QgsExpressionContextScope expressionContextScope() const;

  private:
    ActionType mType = Generic;
    QString mDescription;
    QString mShortTitle;
    QString mIcon;
    QString mCommand;
    bool mCaptureOutput = false;
    QSet<QString> mActionScopes;
    QString mNotificationMessage;
    mutable std::shared_ptr<QAction> mAction;
    QUuid mId;
    QgsExpressionContextScope mExpressionContextScope;
    bool mIsEnabledOnlyWhenEditable = false;
};

Q_DECLARE_METATYPE( QgsAction )

#endif // QGSACTION_H
