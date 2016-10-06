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

#include <QString>
#include <QIcon>

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

    /**
     * Create a new QgsAction
     *
     * @param type          The type of this action
     * @param description   A human readable description string
     * @param action        The action text. Its interpretation depends on the type
     * @param capture       If this is set to true, the output will be captured when an action is run
     */
    QgsAction( ActionType type, const QString& description, const QString& action, bool capture )
        : mType( type )
        , mDescription( description )
        , mAction( action )
        , mCaptureOutput( capture )
        , mShowInAttributeTable( true )
    {}


    /**
     * Create a new QgsAction
     *
     * @param type          The type of this action
     * @param description   A human readable description string
     * @param action        The action text. Its interpretation depends on the type
     * @param icon          Path to an icon for this action
     * @param capture       If this is set to true, the output will be captured when an action is run
     * @param shortTitle    A short string used to label user interface elements like buttons
     */
    QgsAction( ActionType type, const QString& description, const QString& action, const QString& icon, bool capture, const QString& shortTitle = QString() )
        : mType( type )
        , mDescription( description )
        , mShortTitle( shortTitle )
        , mIcon( icon )
        , mAction( action )
        , mCaptureOutput( capture )
        , mShowInAttributeTable( true )
    {}

    /**
     * Create a new QgsAction
     *
     * @param type                 The type of this action
     * @param description          A human readable description string
     * @param action               The action text. Its interpretation depends on the type
     * @param icon                 Path to an icon for this action
     * @param capture              If this is set to true, the output will be captured when an action is run
     * @param showInAttributeTable If this is false, the action will be hidden on the attribute table action widget
     * @param shortTitle           A short string used to label user interface elements like buttons
     */
    QgsAction( ActionType type, const QString& description, const QString& action, const QString& icon, bool capture, bool showInAttributeTable, const QString& shortTitle = QString() )
        : mType( type )
        , mDescription( description )
        , mShortTitle( shortTitle )
        , mIcon( icon )
        , mAction( action )
        , mCaptureOutput( capture )
        , mShowInAttributeTable( showInAttributeTable )
    {}

    //! The name of the action. This may be a longer description.
    QString name() const { return mDescription; }

    //! The short title is used to label user interface elements like buttons
    QString shortTitle() const { return mShortTitle; }

    //! The path to the icon
    QString iconPath() const { return mIcon; }

    //! The icon
    QIcon icon() const { return QIcon( mIcon ); }

    //! The action
    QString action() const { return mAction; }

    //! The action type
    ActionType type() const { return mType; }

    //! Whether to capture output for display when this action is run
    bool capture() const { return mCaptureOutput; }

    //! Whether this action should be shown on the attribute table
    bool showInAttributeTable() const { return mShowInAttributeTable; }

    //! Checks if the action is runable on the current platform
    bool runable() const;

  private:
    ActionType mType;
    QString mDescription;
    QString mShortTitle;
    QString mIcon;
    QString mAction;
    bool mCaptureOutput;
    bool mShowInAttributeTable;
};

#endif // QGSACTION_H
