/***************************************************************************
                         qgstemporalproperty.cpp
                         ---------------
    begin                : January 2020
    copyright            : (C) 2020 by Samweli Mwakisambwe
    email                : samweli at kartoza dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/


#include "qgstemporalproperty.h"

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
