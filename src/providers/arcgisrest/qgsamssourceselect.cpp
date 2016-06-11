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


QgsAmsSourceSelect::QgsAmsSourceSelect( QWidget* parent, Qt::WindowFlags fl, bool embeddedMode )
    : QgsSourceSelectDialog( "ArcGisMapServer", QgsSourceSelectDialog::MapService, parent, fl )
{
  if ( embeddedMode )
  {
    buttonBox->button( QDialogButtonBox::Close )->hide();
  }
}

bool QgsAmsSourceSelect::connectToService( const QgsOWSConnection &connection )
{
  QString errorTitle, errorMessage;
  QVariantMap serviceInfoMap = QgsArcGisRestUtils::getServiceInfo( connection.uri().param( "url" ), errorTitle, errorMessage );
  if ( serviceInfoMap.isEmpty() )
  {
    QMessageBox::warning( this, tr( "Error" ), tr( "Failed to retrieve service capabilities:\n%1: %2" ).arg( errorTitle ).arg( errorMessage ) );
    return false;
  }

  populateImageEncodings( serviceInfoMap["supportedImageFormatTypes"].toString().split( "," ) );

  QStringList layerErrors;
  foreach ( const QVariant& layerInfo, serviceInfoMap["layers"].toList() )
  {
    QVariantMap layerInfoMap = layerInfo.toMap();
    if ( !layerInfoMap["id"].isValid() )
    {
      continue;
    }

    // Get layer info
    QVariantMap layerData = QgsArcGisRestUtils::getLayerInfo( connection.uri().param( "url" ) + "/" + layerInfoMap["id"].toString(), errorTitle, errorMessage );
    if ( layerData.isEmpty() )
    {
      layerErrors.append( QString( "Layer %1: %2 - %3" ).arg( layerInfoMap["id"].toString() ).arg( errorTitle ).arg( errorMessage ) );
      continue;
    }
    // insert the typenames, titles and abstracts into the tree view
    QStandardItem* idItem = new QStandardItem( layerData["id"].toString() );
    QStandardItem* nameItem = new QStandardItem( layerData["name"].toString() );
    QStandardItem* abstractItem = new QStandardItem( layerData["description"].toString() );
    abstractItem->setToolTip( layerData["description"].toString() );

    QgsCoordinateReferenceSystem crs = QgsArcGisRestUtils::parseSpatialReference( serviceInfoMap["spatialReference"].toMap() );
    if ( !crs.isValid() )
    {
      layerErrors.append( tr( "Layer %1: unable to parse spatial reference" ).arg( layerInfoMap["id"].toString() ) );
      continue;
    }
    mAvailableCRS[layerData["name"].toString()] = QList<QString>()  << crs.authid();

    mModel->appendRow( QList<QStandardItem*>() << idItem << nameItem << abstractItem );
  }
  if ( !layerErrors.isEmpty() )
  {
    QMessageBox::warning( this, tr( "Error" ), tr( "Failed to query some layers:\n%1" ).arg( layerErrors.join( "\n" ) ) );
  }
  return true;
}

QString QgsAmsSourceSelect::getLayerURI( const QgsOWSConnection& connection,
    const QString& layerTitle, const QString& /*layerName*/,
    const QString& crs,
    const QString& /*filter*/,
    const QgsRectangle& /*bBox*/ ) const
{
  QgsDataSourceURI ds = connection.uri();
  ds.setParam( "layer", layerTitle );
  ds.setParam( "crs", crs );
  ds.setParam( "format", getSelectedImageEncoding() );
  return ds.uri();
}
