/***************************************************************************
    qgsdataprovider.cpp - DataProvider Interface
     --------------------------------------
    Date                 : May 2016
    Copyright            : (C) 2016 by Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdataprovider.h"


void QgsDataProvider::setProviderProperty( QgsDataProvider::ProviderProperty property, const QVariant& value )
{
  mProviderProperties.insert( property, value );
}

void QgsDataProvider::setProviderProperty( int property, const QVariant& value )
{
  mProviderProperties.insert( property, value );
}

QVariant QgsDataProvider::providerProperty( QgsDataProvider::ProviderProperty property, const QVariant& defaultValue ) const
{
  return mProviderProperties.value( property, defaultValue );
}

QVariant QgsDataProvider::providerProperty( int property, const QVariant& defaultValue = QVariant() ) const
{
  return mProviderProperties.value( property, defaultValue );
}

