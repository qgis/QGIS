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
  : QgsArcGisServiceSourceSelect( QStringLiteral( "ArcGisFeatureServer" ), QgsArcGisServiceSourceSelect::FeatureService, parent, fl, widgetMode )
{
  // import/export of connections not supported yet
  btnLoad->hide();
  btnSave->hide();
}

bool QgsAfsSourceSelect::connectToService( const QgsOwsConnection &connection )
{
  QString errorTitle, errorMessage;
  QVariantMap serviceInfoMap = QgsArcGisRestUtils::getServiceInfo( connection.uri().param( QStringLiteral( "url" ) ), errorTitle, errorMessage );
  if ( serviceInfoMap.isEmpty() )
  {
    QMessageBox::warning( this, tr( "Error" ), tr( "Failed to retrieve service capabilities:\n%1: %2" ).arg( errorTitle, errorMessage ) );
    return false;
  }

  QStringList layerErrors;
  foreach ( const QVariant &layerInfo, serviceInfoMap["layers"].toList() )
  {
    QVariantMap layerInfoMap = layerInfo.toMap();
    if ( !layerInfoMap[QStringLiteral( "id" )].isValid() )
    {
      continue;
    }

    // Get layer info
    QVariantMap layerData = QgsArcGisRestUtils::getLayerInfo( connection.uri().param( QStringLiteral( "url" ) ) + "/" + layerInfoMap[QStringLiteral( "id" )].toString(), errorTitle, errorMessage );
    if ( layerData.isEmpty() )
    {
      layerErrors.append( tr( "Layer %1: %2 - %3" ).arg( layerInfoMap[QStringLiteral( "id" )].toString(), errorTitle, errorMessage ) );
      continue;
    }
    // insert the typenames, titles and abstracts into the tree view
    QStandardItem *idItem = new QStandardItem( layerData[QStringLiteral( "id" )].toString() );
    QStandardItem *nameItem = new QStandardItem( layerData[QStringLiteral( "name" )].toString() );
    QStandardItem *abstractItem = new QStandardItem( layerData[QStringLiteral( "description" )].toString() );
    abstractItem->setToolTip( layerData[QStringLiteral( "description" )].toString() );
    QStandardItem *cachedItem = new QStandardItem();
    QStandardItem *filterItem = new QStandardItem();
    cachedItem->setCheckable( true );
    cachedItem->setCheckState( Qt::Checked );

    QgsCoordinateReferenceSystem crs = QgsArcGisRestUtils::parseSpatialReference( serviceInfoMap[QStringLiteral( "spatialReference" )].toMap() );
    if ( !crs.isValid() )
    {
      // If not spatial reference, just use WGS84
      crs.createFromString( QStringLiteral( "EPSG:4326" ) );
    }
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
  QgsAfsProvider provider( ds.uri() );
  if ( !provider.isValid() )
  {
    return;
  }

  //show expression builder
  QgsExpressionBuilderDialog d( 0, filterIndex.data().toString() );

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
