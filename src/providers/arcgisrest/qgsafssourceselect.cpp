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
  QVariantMap serviceInfoMap = QgsArcGisRestUtils::getServiceInfo( connection.uri().param( QStringLiteral( "url" ) ), authcfg, errorTitle, errorMessage );
  if ( serviceInfoMap.isEmpty() )
  {
    QMessageBox::warning( this, tr( "Error" ), tr( "Failed to retrieve service capabilities:\n%1: %2" ).arg( errorTitle, errorMessage ) );
    return false;
  }

  QStringList layerErrors;
  const QVariantList layers = serviceInfoMap.value( QStringLiteral( "layers" ) ).toList();
  for ( const QVariant &layerInfo : layers )
  {
    const QVariantMap layerInfoMap = layerInfo.toMap();
    if ( !layerInfoMap[QStringLiteral( "id" )].isValid() )
    {
      continue;
    }

    if ( !layerInfoMap.value( QStringLiteral( "subLayerIds" ) ).toList().empty() )
    {
      // group layer - do not show as it is not possible to load
      // TODO - turn model into a tree and show nested groups
      continue;
    }

    // Get layer info
    const QVariantMap layerData = QgsArcGisRestUtils::getLayerInfo( connection.uri().param( QStringLiteral( "url" ) ) + "/" + layerInfoMap[QStringLiteral( "id" )].toString(), authcfg, errorTitle, errorMessage );
    if ( layerData.isEmpty() )
    {
      layerErrors.append( tr( "Layer %1: %2 - %3" ).arg( layerInfoMap[QStringLiteral( "id" )].toString(), errorTitle, errorMessage ) );
      continue;
    }
    if ( !layerData.value( QStringLiteral( "capabilities" ) ).toString().contains( QStringLiteral( "query" ), Qt::CaseInsensitive ) )
    {
      QgsDebugMsg( QStringLiteral( "Layer %1 does not support query capabilities" ).arg( layerInfoMap[QStringLiteral( "id" )].toString() ) );
      continue;
    }
    // insert the typenames, titles and abstracts into the tree view
    QStandardItem *idItem = new QStandardItem( layerData[QStringLiteral( "id" )].toString() );
    bool ok = false;
    int idInt = layerData[QStringLiteral( "id" )].toInt( &ok );
    if ( ok )
    {
      // force display role to be int value, so that sorting works correctly
      idItem->setData( idInt, Qt::DisplayRole );
    }
    QStandardItem *nameItem = new QStandardItem( layerData[QStringLiteral( "name" )].toString() );
    QStandardItem *abstractItem = new QStandardItem( layerData[QStringLiteral( "description" )].toString() );
    abstractItem->setToolTip( layerData[QStringLiteral( "description" )].toString() );
    QStandardItem *cachedItem = new QStandardItem();
    QStandardItem *filterItem = new QStandardItem();
    cachedItem->setCheckable( true );
    cachedItem->setCheckState( Qt::Checked );

    QgsCoordinateReferenceSystem crs = QgsArcGisRestUtils::parseSpatialReference( serviceInfoMap[QStringLiteral( "spatialReference" )].toMap() );
    mAvailableCRS[layerData[QStringLiteral( "name" )].toString()] = QList<QString>()  << crs.authid();

    mModel->appendRow( QList<QStandardItem *>() << idItem << nameItem << abstractItem << cachedItem << filterItem );
  }
  if ( !layerErrors.isEmpty() )
  {
    QMessageBox::warning( this, tr( "Error" ), tr( "Failed to query some layers:\n%1" ).arg( layerErrors.join( QStringLiteral( "\n" ) ) ) );
  }
  return true;
}

void QgsAfsSourceSelect::buildQuery( const QgsOwsConnection &connection, const QModelIndex &index )
{
  if ( !index.isValid() )
  {
    return;
  }
  QModelIndex filterIndex = index.sibling( index.row(), 4 );
  QString id = index.sibling( index.row(), 0 ).data().toString();

  // Query available fields
  QgsDataSourceUri ds = connection.uri();
  QString url = ds.param( QStringLiteral( "url" ) ) + "/" + id;
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
  QString url = ds.param( QStringLiteral( "url" ) ) + "/" + layerTitle;
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
