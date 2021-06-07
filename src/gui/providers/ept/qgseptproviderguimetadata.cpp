/***************************************************************************
                         qgseptproviderguimetadata.cpp
                         --------------------
    begin                : October 2020
    copyright            : (C) 2020 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
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
