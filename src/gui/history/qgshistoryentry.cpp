/***************************************************************************
                            qgshistoryentry.cpp
                            -------------------------
    begin                : April 2019
    copyright            : (C) 2019 by Nyall Dawson
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

#include "qgshistoryentry.h"

QgsHistoryEntry::QgsHistoryEntry( const QString &providerId, const QDateTime &timestamp, const QVariantMap &entry )
  : timestamp( timestamp )
  , providerId( providerId )
  , entry( entry )
{
}

QgsHistoryEntry::QgsHistoryEntry( const QVariantMap &entry )
  : timestamp( QDateTime::currentDateTime() )
  , entry( entry )
{
}

bool QgsHistoryEntry::isValid() const
{
  return !providerId.isEmpty() || !entry.isEmpty() || timestamp.isValid();
}
