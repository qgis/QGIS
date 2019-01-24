/***************************************************************************
      qgsafssourceselect.cpp
      ----------------------
    begin                : Jun 02, 2015
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

#include "qgsafssourceselect.h"
#include "qgsarcgisrestutils.h"
#include "qgsafsprovider.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgsowsconnection.h"
#include "qgsnetworkaccessmanager.h"
#include "qgslogger.h"

#include <QMessageBox>


QgsAfsSourceSelect::QgsAfsSourceSelect( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode )
  : QgsArcGisServiceSourceSelect( QStringLiteral( "ARCGISFEATURESERVER" ), QgsArcGisServiceSourceSelect::FeatureService, parent, fl, widgetMode )
{
  // import/export of connections not supported yet
  btnLoad->hide();
  btnSave->hide();
}

bool QgsAfsSourceSelect::connectToService( const QgsOwsConnection &connection )
{
  QString errorTitle, errorMessage;

  const QString authcfg = connection.uri().param( QStringLiteral( "authcfg" ) );
  const QString baseUrl = connection.uri().param( QStringLiteral( "url" ) );
  const QString referer = connection.uri().param( QStringLiteral( "referer" ) );
  QgsStringMap headers;
  if ( ! referer.isEmpty() )
    headers[ QStringLiteral( "Referer" )] = referer;

  std::function< bool( const QString &, QStandardItem * )> visitItemsRecursive;
  visitItemsRecursive = [this, &visitItemsRecursive, baseUrl, authcfg, headers, &errorTitle, &errorMessage]( const QString & baseItemUrl, QStandardItem * parentItem ) -> bool
  {
    const QVariantMap serviceInfoMap = QgsArcGisRestUtils::getServiceInfo( baseItemUrl, authcfg, errorTitle, errorMessage, headers );

    if ( serviceInfoMap.isEmpty() )
    {
      return false;
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
    }, serviceInfoMap, baseUrl );

    QMap< QString, QList<QStandardItem *> > layerItems;
    QMap< QString, QString > parents;

    QgsArcGisRestUtils::addLayerItems( [ =, &layerItems, &parents]( const QString & parentLayerId, const QString & layerId, const QString & name, const QString & description, const QString & url, bool isParentLayer, const QString & authid )
    {
      if ( !parentLayerId.isEmpty() )
        parents.insert( layerId, parentLayerId );

      if ( isParentLayer )
      {
        QStandardItem *nameItem = new QStandardItem( name );
        nameItem->setToolTip( url );
        layerItems.insert( layerId, QList<QStandardItem *>() << nameItem );
      }
      else
      {
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
        idItem->setData( true, IsLayerRole );
        QStandardItem *nameItem = new QStandardItem( name );
        QStandardItem *abstractItem = new QStandardItem( description );
        abstractItem->setToolTip( description );
        QStandardItem *filterItem = new QStandardItem();

        mAvailableCRS[name] = QList<QString>()  << authid;

        layerItems.insert( layerId, QList<QStandardItem *>() << idItem << nameItem << abstractItem << filterItem );
      }
    }, serviceInfoMap, baseItemUrl );

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

void QgsAfsSourceSelect::buildQuery( const QgsOwsConnection &connection, const QModelIndex &index )
{
  if ( !index.isValid() )
  {
    return;
  }
  QModelIndex filterIndex = index.sibling( index.row(), 3 );
  const QString url = index.sibling( index.row(), 0 ).data( UrlRole ).toString();

  // Query available fields
  QgsDataSourceUri ds = connection.uri();
  ds.removeParam( QStringLiteral( "url" ) );
  ds.setParam( QStringLiteral( "url" ), url );
  QgsDataProvider::ProviderOptions providerOptions;
  QgsAfsProvider provider( ds.uri(), providerOptions );
  if ( !provider.isValid() )
  {
    return;
  }

  //show expression builder
  QgsExpressionBuilderDialog d( nullptr, filterIndex.data().toString() );

  //add available attributes to expression builder
  QgsExpressionBuilderWidget *w = d.expressionBuilder();
  w->loadFieldNames( provider.fields() );

  if ( d.exec() == QDialog::Accepted )
  {
    QgsDebugMsg( "Expression text = " + w->expressionText() );
    mModelProxy->setData( filterIndex, QVariant( w->expressionText() ) );
  }
}

QString QgsAfsSourceSelect::getLayerURI( const QgsOwsConnection &connection,
    const QString &layerTitle, const QString & /*layerName*/,
    const QString &crs,
    const QString &filter,
    const QgsRectangle &bBox ) const
{
  QgsDataSourceUri ds = connection.uri();
  QString url = layerTitle;
  ds.removeParam( QStringLiteral( "url" ) );
  ds.setParam( QStringLiteral( "url" ), url );
  ds.setParam( QStringLiteral( "filter" ), filter );
  ds.setParam( QStringLiteral( "crs" ), crs );
  if ( !bBox.isEmpty() )
  {
    ds.setParam( QStringLiteral( "bbox" ), QStringLiteral( "%1,%2,%3,%4" ).arg( bBox.xMinimum() ).arg( bBox.yMinimum() ).arg( bBox.xMaximum() ).arg( bBox.yMaximum() ) );
  }
  return ds.uri();
}


void QgsAfsSourceSelect::addServiceLayer( QString uri, QString typeName )
{
  emit addVectorLayer( uri, typeName );
}
