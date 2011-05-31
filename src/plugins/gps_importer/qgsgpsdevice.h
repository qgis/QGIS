/***************************************************************************
                          qgsgpsdevice.h
 Functions:
                             -------------------
    begin                : Oct 05, 2004
    copyright            : (C) 2004 by Lars Luthman
    email                : larsl@users.sourceforge.net

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGPSDEVICE_H
#define QGSGPSDEVICE_H

#include <QString>
#include <QStringList>

#include <qgsbabelformat.h>


class QgsGPSDevice : public QgsBabelFormat
{
  public:
    QgsGPSDevice() { }

    QgsGPSDevice( const QString& wptDlCmd, const QString& wptUlCmd,
                  const QString& rteDlCmd, const QString& rteUlCmd,
                  const QString& trkDlCmd, const QString& trkUlCmd );

    QStringList importCommand( const QString& babel, const QString& type,
                               const QString& in, const QString& out ) const;
    QStringList exportCommand( const QString& babel, const QString& type,
                               const QString& in, const QString& out ) const;

  private:

    QStringList mWptDlCmd, mWptUlCmd, mRteDlCmd, mRteUlCmd,
    mTrkDlCmd, mTrkUlCmd;
};


#endif
