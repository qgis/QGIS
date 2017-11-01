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
  : QgsArcGisServiceSourceSelect( QStringLiteral( "ArcGisMapServer" ), QgsArcGisServiceSourceSelect::MapService, parent, fl, widgetMode )
{

  // import/export of connections not supported yet
  btnLoad->hide();
  btnSave->hide();
}

bool QgsAmsSourceSelect::connectToService( const QgsOwsConnection &connection )
{
  QString errorTitle, errorMessage;
  QVariantMap serviceInfoMap = QgsArcGisRestUtils::getServiceInfo( connection.uri().param( QStringLiteral( "url" ) ), errorTitle, errorMessage );
  if ( serviceInfoMap.isEmpty() )
  {
    QMessageBox::warning( this, tr( "Error" ), tr( "Failed to retrieve service capabilities:\n%1: %2" ).arg( errorTitle, errorMessage ) );
    return false;
  }

  populateImageEncodings( serviceInfoMap[QStringLiteral( "supportedImageFormatTypes" )].toString().split( QStringLiteral( "," ) ) );

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
      layerErrors.append( QStringLiteral( "Layer %1: %2 - %3" ).arg( layerInfoMap[QStringLiteral( "id" )].toString(), errorTitle, errorMessage ) );
      continue;
    }
    // insert the typenames, titles and abstracts into the tree view
    QStandardItem *idItem = new QStandardItem( layerData[QStringLiteral( "id" )].toString() );
    QStandardItem *nameItem = new QStandardItem( layerData[QStringLiteral( "name" )].toString() );
    QStandardItem *abstractItem = new QStandardItem( layerData[QStringLiteral( "description" )].toString() );
    abstractItem->setToolTip( layerData[QStringLiteral( "description" )].toString() );

    QgsCoordinateReferenceSystem crs = QgsArcGisRestUtils::parseSpatialReference( serviceInfoMap[QStringLiteral( "spatialReference" )].toMap() );
    if ( !crs.isValid() )
    {
      layerErrors.append( tr( "Layer %1: unable to parse spatial reference" ).arg( layerInfoMap[QStringLiteral( "id" )].toString() ) );
      continue;
    }
    mAvailableCRS[layerData[QStringLiteral( "name" )].toString()] = QList<QString>()  << crs.authid();

    mModel->appendRow( QList<QStandardItem *>() << idItem << nameItem << abstractItem );
  }
  if ( !layerErrors.isEmpty() )
  {
    QMessageBox::warning( this, tr( "Error" ), tr( "Failed to query some layers:\n%1" ).arg( layerErrors.join( QStringLiteral( "\n" ) ) ) );
  }
  return true;
}

QString QgsAmsSourceSelect::getLayerURI( const QgsOwsConnection &connection,
    const QString &layerTitle, const QString & /*layerName*/,
    const QString &crs,
    const QString & /*filter*/,
    const QgsRectangle & /*bBox*/ ) const
{
  QgsDataSourceUri ds = connection.uri();
  ds.setParam( QStringLiteral( "layer" ), layerTitle );
  ds.setParam( QStringLiteral( "crs" ), crs );
  ds.setParam( QStringLiteral( "format" ), getSelectedImageEncoding() );
  return ds.uri();
}

void QgsAmsSourceSelect::addServiceLayer( QString uri, QString typeName )
{
  emit addRasterLayer( uri, typeName, QStringLiteral( "arcgismapserver" ) );
}
