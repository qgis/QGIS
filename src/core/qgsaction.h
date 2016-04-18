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

    QgsAction( ActionType type, const QString& name, const QString& action, bool capture )
        : mType( type )
        , mName( name )
        , mAction( action )
        , mCaptureOutput( capture )
    {}

    QgsAction( ActionType type, const QString& name, const QString& action, const QString& icon, bool capture )
        : mType( type )
        , mName( name )
        , mIcon( icon )
        , mAction( action )
        , mCaptureOutput( capture )
    {}

    //! The name of the action
    QString name() const { return mName; }

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

    //! Whether the action is runable on the current platform
    bool runable() const;

  private:
    ActionType mType;
    QString mName;
    QString mIcon;
    QString mAction;
    bool mCaptureOutput;
};

#endif // QGSACTION_H
