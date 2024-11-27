/***************************************************************************
                         qgstemporalproperty.cpp
                         ---------------
    begin                : January 2020
    copyright            : (C) 2020 by Samweli Mwakisambwe
    email                : samweli at kartoza dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgstemporalproperty.h"
#include "moc_qgstemporalproperty.cpp"

QgsTemporalProperty::QgsTemporalProperty( QObject *parent, bool enabled )
  : QObject( parent )
  , mActive( enabled )
{
}

void QgsTemporalProperty::setIsActive( bool active )
{
  if ( mActive != active )
  {
    mActive = active;
    emit changed();
  }
}

bool QgsTemporalProperty::isActive() const
{
  return mActive;
}
