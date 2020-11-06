/***************************************************************************
                         qgseptproviderguimetadata.cpp
                         --------------------
    begin                : October 2020
    copyright            : (C) 2020 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsapplication.h"
#include "qgseptproviderguimetadata.h"
#include "qgseptdataitemguiprovider.h"

///@cond PRIVATE

QgsEptProviderGuiMetadata::QgsEptProviderGuiMetadata()
  : QgsProviderGuiMetadata( QStringLiteral( "ept" ) )
{
}

QList<QgsDataItemGuiProvider *> QgsEptProviderGuiMetadata::dataItemGuiProviders()
{
  return QList<QgsDataItemGuiProvider *>()
         << new QgsEptDataItemGuiProvider;
}

///@endcond
