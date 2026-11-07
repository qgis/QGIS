/***************************************************************************
    qgswaylandpointerlocker.h
                             -------------------
    begin                : November 2026
    copyright            : (C) 2026 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSWAYLANDPOINTERLOCKER_H
#define QGSWAYLANDPOINTERLOCKER_H

#include "qgsnative.h"

#include <QObject>

class WaylandPointerConstraints;
class WaylandRelativePointerManager;
class WaylandRelativePointer;

struct zwp_locked_pointer_v1;

class NATIVE_EXPORT QgsWaylandPointerLocker : public QgsPointerLocker
{
    Q_OBJECT
  public:
    QgsWaylandPointerLocker( QWindow *window, QObject *parent );
    ~QgsWaylandPointerLocker() override;

    bool lock() override;
    bool unlock() override;

    void handleRelativeMotion( double dx, double dy );

  private:
    bool mIsLocked = false;

    WaylandPointerConstraints *mConstraints = nullptr;
    WaylandRelativePointerManager *mRelativePointerManager = nullptr;
    WaylandRelativePointer *mRelativePointer = nullptr;

    zwp_locked_pointer_v1 *mLockedPointer = nullptr;
};

#endif // QGSWAYLANDPOINTERLOCKER_H
