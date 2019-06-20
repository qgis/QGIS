/***************************************************************************
      qgsamssourceselect.cpp
      ----------------------
    begin                : Nov 26, 2015
    copyright            : (C) 2015 by Sandro Mani
    email                : smani@sourcepole.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsamssourceselect.h"
#include "qgsarcgisrestutils.h"
#include "qgsamsprovider.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgsowsconnection.h"
#include "qgsnetworkaccessmanager.h"

#include <QMessageBox>


QgsAmsSourceSelect::QgsAmsSourceSelect( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode )
  : QgsArcGisServiceSourceSelect( QStringLiteral( "ARCGISMAPSERVER" ), QgsArcGisServiceSourceSelect::MapService, parent, fl, widgetMode )
{

  // import/export of connections not supported yet
  btnLoad->hide();
  btnSave->hide();
}

bool QgsAmsSourceSelect::connectToService( const QgsOwsConnection &connection )
{
  QString errorTitle, errorMessage;

  const QString authcfg = connection.uri().param( QStringLiteral( "authcfg" ) );
  const QString baseUrl = connection.uri().param( QStringLiteral( "url" ) );
  const QString referer = connection.uri().param( QStringLiteral( "referer" ) );
  QgsStringMap headers;
  if ( ! referer.isEmpty() )
    headers[ QStringLiteral( "Referer" )] = referer;

  bool hasPopulatedImageFormats = false;
  std::function< bool( const QString &, QStandardItem * )> visitItemsRecursive;
  visitItemsRecursive = [this, &hasPopulatedImageFormats, &visitItemsRecursive, baseUrl, authcfg, headers, &errorTitle, &errorMessage]( const QString & baseItemUrl, QStandardItem * parentItem ) -> bool
  {
    const QVariantMap serviceInfoMap = QgsArcGisRestUtils::getServiceInfo( baseItemUrl, authcfg, errorTitle, errorMessage, headers );

    if ( serviceInfoMap.isEmpty() )
    {
      return false;
    }

    if ( !hasPopulatedImageFormats )
    {
      const QString supportedFormats = serviceInfoMap[QStringLiteral( "supportedImageFormatTypes" )].toString();
      if ( !supportedFormats.isEmpty() )
      {
        populateImageEncodings( supportedFormats.split( ',' ) );
        hasPopulatedImageFormats = true;
      }
    }

    bool res = true;

    QgsArcGisRestUtils::visitFolderItems( [ =, &res ]( const QString & name, const QString & url )
    {
      QStandardItem *nameItem = new QStandardItem( name );
      nameItem->setToolTip( url );
      if ( parentItem )
        parentItem->appendRow( QList<QStandardItem *>() << nameItem );
      else
        mModel->appendRow( QList<QStandardItem *>() << nameItem );

      if ( !visitItemsRecursive( url, nameItem ) )
        res = false;
    }, serviceInfoMap, baseUrl );

    QgsArcGisRestUtils::visitServiceItems(
      [ =, &res]( const QString & name, const QString & url )
    {
      QStandardItem *nameItem = new QStandardItem( name );
      nameItem->setToolTip( url );
      if ( parentItem )
        parentItem->appendRow( QList<QStandardItem *>() << nameItem );
      else
        mModel->appendRow( QList<QStandardItem *>() << nameItem );

      if ( !visitItemsRecursive( url, nameItem ) )
        res = false;
    }, serviceInfoMap, baseUrl, QgsArcGisRestUtils::Raster );

    QMap< QString, QList<QStandardItem *> > layerItems;
    QMap< QString, QString > parents;

    QgsArcGisRestUtils::addLayerItems( [ =, &layerItems, &parents]( const QString & parentLayerId, const QString & layerId, const QString & name, const QString & description, const QString & url, bool, const QString & authid, const QString & )
    {
      if ( !parentLayerId.isEmpty() )
        parents.insert( layerId, parentLayerId );

      // insert the typenames, titles and abstracts into the tree view
      QStandardItem *idItem = new QStandardItem( layerId );
      bool ok = false;
      int idInt = layerId.toInt( &ok );
      if ( ok )
      {
        // force display role to be int value, so that sorting works correctly
        idItem->setData( idInt, Qt::DisplayRole );
      }
      idItem->setData( url, UrlRole );
      idItem->setData( layerId, IdRole );
      idItem->setData( true, IsLayerRole );
      QStandardItem *nameItem = new QStandardItem( name );
      QStandardItem *abstractItem = new QStandardItem( description );
      abstractItem->setToolTip( description );
      QStandardItem *filterItem = new QStandardItem();

      mAvailableCRS[name] = QList<QString>()  << authid;

      layerItems.insert( layerId, QList<QStandardItem *>() << idItem << nameItem << abstractItem << filterItem );
    }, serviceInfoMap, baseItemUrl, QgsArcGisRestUtils::Raster );

    // create layer groups
    for ( auto it = layerItems.constBegin(); it != layerItems.constEnd(); ++it )
    {
      const QString id = it.key();
      QList<QStandardItem *> row = it.value();
      const QString parentId = parents.value( id );
      QList<QStandardItem *> parentRow;
      if ( !parentId.isEmpty() )
        parentRow = layerItems.value( parentId );
      if ( !parentRow.isEmpty() )
      {
        parentRow.at( 0 )->appendRow( row );
      }
      else
      {
        if ( parentItem )
          parentItem->appendRow( row );
        else
          mModel->appendRow( row );
      }
    }

    return res;
  };

  if ( !visitItemsRecursive( baseUrl, nullptr ) )
    QMessageBox::warning( this, tr( "Error" ), tr( "Failed to retrieve service capabilities:\n%1: %2" ).arg( errorTitle, errorMessage ) );

  return true;
}

QString QgsAmsSourceSelect::getLayerURI( const QgsOwsConnection &connection,
    const QString &layerTitle, const QString & /*layerName*/,
    const QString &crs,
    const QString & /*filter*/,
    const QgsRectangle & /*bBox*/, const QString &layerId ) const
{
  QgsDataSourceUri ds = connection.uri();
  QString url = layerTitle;
  QString trimmedUrl = layerId.isEmpty() ? url : url.left( url.length() - 1 - layerId.length() ); // trim '/0' from end of url -- AMS provider requires this omitted
  ds.removeParam( QStringLiteral( "url" ) );
  ds.setParam( QStringLiteral( "url" ), trimmedUrl );
  ds.setParam( QStringLiteral( "layer" ), layerId );
  ds.setParam( QStringLiteral( "crs" ), crs );
  ds.setParam( QStringLiteral( "format" ), getSelectedImageEncoding() );
  return ds.uri();
}

void QgsAmsSourceSelect::addServiceLayer( QString uri, QString typeName )
{
  emit addRasterLayer( uri, typeName, QStringLiteral( "arcgismapserver" ) );
}
