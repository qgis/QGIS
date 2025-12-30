/***************************************************************************
                         qgsprocessingmodelgroupbox.cpp
                         --------------------------
    begin                : March 2020
    copyright            : (C) 2020 by Nyall Dawson
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

#include "qgsprocessingmodelgroupbox.h"

#include <QUuid>

///@cond NOT_STABLE

QgsProcessingModelGroupBox::QgsProcessingModelGroupBox( const QString &description )
  : QgsProcessingModelComponent( description )
  , mUuid( QUuid::createUuid().toString() )
{
  setSize( QSizeF( 400, 360 ) );
}

QgsProcessingModelGroupBox *QgsProcessingModelGroupBox::clone() const
{
  return new QgsProcessingModelGroupBox( *this );
}

QVariant QgsProcessingModelGroupBox::toVariant() const
{
  QVariantMap map;
  map.insert( u"uuid"_s, mUuid );
  saveCommonProperties( map );
  return map;
}

bool QgsProcessingModelGroupBox::loadVariant( const QVariantMap &map, bool ignoreUuid )
{
  restoreCommonProperties( map );
  if ( !ignoreUuid )
    mUuid = map.value( u"uuid"_s ).toString();
  return true;
}

QString QgsProcessingModelGroupBox::uuid() const
{
  return mUuid;
}


///@endcond
