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

/** \ingroup core
 * Utility class that encapsulates an action based on vector attributes.
 */
class QgsAction
{
%TypeHeaderCode
#include <qgsaction.h>
%End
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

    QgsAction( ActionType type, const QString& name, const QString& action, bool capture );

    QgsAction( ActionType type, const QString& name, const QString& action, const QString& icon, bool capture );

    //! The name of the action
    QString name() const;

    //! The path to the icon
    QString iconPath() const;

    //! The icon
    QIcon icon() const;

    //! The action
    QString action() const;

    //! The action type
    ActionType type() const;

    //! Whether to capture output for display when this action is run
    bool capture() const;

    //! Whether the action is runable on the current platform
    bool runable() const;
};
