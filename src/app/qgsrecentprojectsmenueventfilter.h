/***************************************************************************
               qgsrecentprojectsmenueventfilter.h
               ----------------------------------------------------
    begin                : August 2023
    copyright            : (C) 2023 by Yoann Quenach de Quivillic
    email                : yoann dot quenach at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRECENTPROJECTSMENUEVENTFILTER_H
#define QGSRECENTPROJECTSMENUEVENTFILTER_H

#include <QObject>

class QgsWelcomePage;

class QgsRecentProjectsMenuEventFilter : public QObject
{
    Q_OBJECT

  public:
    QgsRecentProjectsMenuEventFilter( QgsWelcomePage *welcomePage = nullptr, QObject *parent = nullptr );

    bool eventFilter( QObject *obj, QEvent *event ) override;

  private:
    QgsWelcomePage *mWelcomePage;
};

#endif // QGSRECENTPROJECTSMENUEVENTFILTER_H
