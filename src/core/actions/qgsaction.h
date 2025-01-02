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
#include "qgis.h"
#include "qgsexpressioncontext.h"

#include <QSet>
#include <QString>
#include <QIcon>
#include <QUuid>

#include <memory>

class QgsExpressionContextScope;

/**
 * \ingroup core
 * \brief Utility class that encapsulates an action based on vector attributes.
 */
class CORE_EXPORT QgsAction
{
  public:

    QgsAction() = default;

    /**
     * Create a new QgsAction
     *
     * \param type          The type of this action
     * \param description   A human readable description string
     * \param command       The action text. Its interpretation depends on the type
     * \param capture       If this is set to TRUE, the output will be captured when an action is run
     * \param enabledOnlyWhenEditable if TRUE then action is only enable in editmode. Not available in Python bindings.
     */
    QgsAction( Qgis::AttributeActionType type, const QString &description, const QString &command, bool capture = false, bool enabledOnlyWhenEditable SIP_PYARGREMOVE = false )
      : mType( type )
      , mDescription( description )
      , mCommand( command )
      , mCaptureOutput( capture )
      , mId( QUuid::createUuid() )
      , mIsEnabledOnlyWhenEditable( enabledOnlyWhenEditable )
    {}

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
     * \param enabledOnlyWhenEditable if TRUE then action is only enable in editmode. Not available in Python bindings.
     */
    QgsAction( Qgis::AttributeActionType type, const QString &description, const QString &action, const QString &icon, bool capture, const QString &shortTitle = QString(), const QSet<QString> &actionScopes = QSet<QString>(), const QString &notificationMessage = QString(), bool enabledOnlyWhenEditable SIP_PYARGREMOVE = false )
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

    /**
     * Create a new QgsAction
     *
     * \param id                   The unique identifier of this action
     * \param type                 The type of this action
     * \param description          A human readable description string
     * \param action               The action text. Its interpretation depends on the type
     * \param icon                 Path to an icon for this action
     * \param capture              If this is set to TRUE, the output will be captured when an action is run
     * \param shortTitle           A short string used to label user interface elements like buttons
     * \param actionScopes         A set of scopes in which this action will be available
     * \param notificationMessage  A particular message which reception will trigger the action
     * \param enabledOnlyWhenEditable if TRUE then action is only enable in editmode. Not available in Python bindings.
     */
    QgsAction( const QUuid &id, Qgis::AttributeActionType type, const QString &description, const QString &action, const QString &icon, bool capture, const QString &shortTitle = QString(), const QSet<QString> &actionScopes = QSet<QString>(), const QString &notificationMessage = QString(), bool enabledOnlyWhenEditable SIP_PYARGREMOVE = false )
      : mType( type )
      , mDescription( description )
      , mShortTitle( shortTitle )
      , mIcon( icon )
      , mCommand( action )
      , mCaptureOutput( capture )
      , mActionScopes( actionScopes )
      , mNotificationMessage( notificationMessage )
      , mId( id )
      , mIsEnabledOnlyWhenEditable( enabledOnlyWhenEditable )
    {}


    //! The name of the action. This may be a longer description.
    QString name() const { return mDescription; }

    //! The short title is used to label user interface elements like buttons
    QString shortTitle() const { return mShortTitle; }

    /**
     * Returns a unique id for this action.
     *
     */
    QUuid id() const { return mId; }

    /**
     * Returns TRUE if this action was a default constructed one.
     *
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
     */
    QString command() const { return mCommand; }

    /**
     * Returns the notification message that triggers the action
     *
     */
    QString notificationMessage() const { return mNotificationMessage; }

    //! The action type
    Qgis::AttributeActionType type() const { return mType; }

    //! Whether to capture output for display when this action is run
    bool capture() const { return mCaptureOutput; }


    //! Returns whether only enabled in editable mode
    bool isEnabledOnlyWhenEditable() const { return mIsEnabledOnlyWhenEditable; }

    /**
     * Set whether the action is only enabled in editable mode
     *
     * \since QGIS 3.16
     */
    void setEnabledOnlyWhenEditable( bool enable ) { mIsEnabledOnlyWhenEditable = enable; };


    //! Checks if the action is runable on the current platform
    bool runable() const;

    /**
     * Run this action.
     *
     */
    void run( QgsVectorLayer *layer, const QgsFeature &feature, const QgsExpressionContext &expressionContext ) const;

    /**
     * Run this action.
     *
     */
    void run( const QgsExpressionContext &expressionContext ) const;

    /**
     * The action scopes define where an action will be available.
     * Action scopes may offer additional variables like the clicked
     * coordinate.
     *
     * \see QgsActionScope
     */
    QSet<QString> actionScopes() const;

    /**
     * The action scopes define where an action will be available.
     * Action scopes may offer additional variables like the clicked
     * coordinate.
     *
     */
    void setActionScopes( const QSet<QString> &actionScopes );

    /**
     * Reads an XML definition from actionNode
     * into this object.
     *
     */
    void readXml( const QDomNode &actionNode );

    /**
     * Appends an XML definition for this action as a new
     * child node to actionsNode.
     *
     */
    void writeXml( QDomNode &actionsNode ) const;

    /**
     * Sets an expression context scope to use for running the action.
     *
     */
    void setExpressionContextScope( const QgsExpressionContextScope &scope );

    /**
     * Returns an expression context scope used for running the action.
     *
     */
    QgsExpressionContextScope expressionContextScope() const;

    /**
     * Returns an HTML table with the basic information about this action.
     *
     * \since QGIS 3.24
     */
    QString html( ) const;

    /**
     * Sets the action \a command.
     * \since QGIS 3.26
     */
    void setCommand( const QString &newCommand );

  private:

    void handleFormSubmitAction( const QString &expandedAction ) const;
    Qgis::AttributeActionType mType = Qgis::AttributeActionType::Generic;
    QString mDescription;
    QString mShortTitle;
    QString mIcon;
    QString mCommand;
    bool mCaptureOutput = false;
    QSet<QString> mActionScopes;
    QString mNotificationMessage;
    QUuid mId;
    QgsExpressionContextScope mExpressionContextScope;
    bool mIsEnabledOnlyWhenEditable = false;
};

Q_DECLARE_METATYPE( QgsAction )

#endif // QGSACTION_H
