/***************************************************************************
                              qgstransaction.cpp
                              ------------------
  begin                : May 5, 2014
  copyright            : (C) 2014 by Marco Hugentobler
  email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstransaction.h"
#include "qgsmaplayerregistry.h"
#include "qgsproviderregistry.h"
#include "qgsvectorlayer.h"
#include "qgis.h"
#include <QLibrary>

typedef bool beginTransaction_t( const QString& id, const QString& connString, QString& error );

QgsTransaction::QgsTransaction( const QString& connString, const QString& providerKey ): mConnString( connString ), mProviderKey( providerKey )
{

}

QgsTransaction::~QgsTransaction()
{

}

bool QgsTransaction::addLayer( const QString& layerId )
{
    if( !QgsMapLayerRegistry::instance()->mapLayer( layerId ) )
    {
        return false;
    }

    mLayers.insert( layerId );
    return true;
}

bool QgsTransaction::begin( QString& errorMsg )
{
    //Set all layers to direct edit mode

    QLibrary* lib = QgsProviderRegistry::instance()->providerLibrary( mProviderKey );
    if( !lib )
    {
        return false;
    }

    beginTransaction_t* pBeginTransaction = ( beginTransaction_t* ) cast_to_fptr( lib->resolve( "beginTransaction" ) );
    delete lib;

    if( !pBeginTransaction )
    {
        return false;
    }

    if( !pBeginTransaction( mId.toString(), mConnString, errorMsg ) )
    {
        return false;
    }

    //set all layers to transaction edit mode
    QSet<QString>::iterator layerIt = mLayers.begin();
    for(; layerIt != mLayers.end(); ++layerIt )
    {
        QgsVectorLayer* vl = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( *layerIt ) );
        if( vl )
        {
            vl->setTransactionId( mId.toString() );
        }
    }
    return true;
}

bool QgsTransaction::commit( QString& errorMsg )
{
    //Provider::commitTransaction( id )

    Q_UNUSED( errorMsg );
    return false; //soon...
}

bool QgsTransaction::rollback( QString& errorMsg )
{
    //Provider::rollbackTransaction( id )
    Q_UNUSED( errorMsg );
    return false; //soon...
}

bool QgsTransaction::executeSql( QString& errorMsg )
{
    Q_UNUSED( errorMsg );
    //Provider::executeTransactionSql( id )
    return false; //soon...
}

bool QgsTransaction::disableLayerEditModes()
{
    //set layer edit mode to false / non-transactional
    return false;
}
