/***************************************************************************
    qgswcssourceselect.cpp  -  selector for WCS
                             -------------------
    begin                : 04 2012
    copyright            :
    original             : (C) 2012 Radim Blazek

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgis.h"
#include "qgslogger.h"

#include "qgsnetworkaccessmanager.h"
#include "qgswcsprovider.h"
#include "qgswcssourceselect.h"
#include "qgswcscapabilities.h"
#include "qgstreewidgetitem.h"

#include <QWidget>

QgsWCSSourceSelect::QgsWCSSourceSelect( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode )
  : QgsOWSSourceSelect( QStringLiteral( "WCS" ), parent, fl, widgetMode )
{

  // Hide irrelevant widgets
  mWMSGroupBox->hide();
  mLayersTab->layout()->removeWidget( mWMSGroupBox );
  mTabWidget->removeTab( mTabWidget->indexOf( mLayerOrderTab ) );
  mTabWidget->removeTab( mTabWidget->indexOf( mTilesetsTab ) );
  mAddDefaultButton->hide();

  mLayersTreeWidget->setSelectionMode( QAbstractItemView::SingleSelection );

  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsWCSSourceSelect::showHelp );
}

void QgsWCSSourceSelect::populateLayerList()
{

  mLayersTreeWidget->clear();


  QgsDataSourceUri uri = mUri;
  const QString cache = QgsNetworkAccessManager::cacheLoadControlName( selectedCacheLoadControl() );
  uri.setParam( QStringLiteral( "cache" ), cache );

  mCapabilities.setUri( uri );

  if ( !mCapabilities.lastError().isEmpty() )
  {
    showError( mCapabilities.lastErrorTitle(), mCapabilities.lastErrorFormat(), mCapabilities.lastError() );
    return;
  }

  QVector<QgsWcsCoverageSummary> coverages;
  if ( !mCapabilities.supportedCoverages( coverages ) )
    return;

  QMap<int, QgsTreeWidgetItem *> items;
  QMap<int, int> coverageParents;
  QMap<int, QStringList> coverageParentNames;
  mCapabilities.coverageParents( coverageParents, coverageParentNames );

  mLayersTreeWidget->setSortingEnabled( true );

  int coverageAndStyleCount = -1;

  for ( QVector<QgsWcsCoverageSummary>::iterator coverage = coverages.begin();
        coverage != coverages.end();
        ++coverage )
  {
    QgsDebugMsg( QStringLiteral( "coverage orderId = %1 identifier = %2" ).arg( coverage->orderId ).arg( coverage->identifier ) );

    QgsTreeWidgetItem *lItem = createItem( coverage->orderId, QStringList() << coverage->identifier << coverage->title << coverage->abstract, items, coverageAndStyleCount, coverageParents, coverageParentNames );

    lItem->setData( 0, Qt::UserRole + 0, coverage->identifier );
    lItem->setData( 0, Qt::UserRole + 1, "" );

    // Make only leaves selectable
    if ( coverageParents.contains( coverage->orderId ) )
    {
      lItem->setFlags( Qt::ItemIsEnabled );
    }
  }

  mLayersTreeWidget->sortByColumn( 0, Qt::AscendingOrder );

  // If we got some coverages, let the user add them to the map
  if ( mLayersTreeWidget->topLevelItemCount() == 1 )
  {
    mLayersTreeWidget->expandItem( mLayersTreeWidget->topLevelItem( 0 ) );
  }
}

QString QgsWCSSourceSelect::selectedIdentifier()
{
  const QList<QTreeWidgetItem *> selectionList = mLayersTreeWidget->selectedItems();
  if ( selectionList.size() < 1 ) return QString(); // should not happen
  QString identifier = selectionList.value( 0 )->data( 0, Qt::UserRole + 0 ).toString();
  QgsDebugMsg( " identifier = " + identifier );
  return identifier;
}

void QgsWCSSourceSelect::addButtonClicked()
{
  QgsDataSourceUri uri = mUri;

  const QString identifier = selectedIdentifier();
  if ( identifier.isEmpty() ) { return; }

  uri.setParam( QStringLiteral( "identifier" ), identifier );

  // Set crs only if necessary (multiple offered), so that we can decide in the
  // provider if WCS 1.0 with RESPONSE_CRS has to be used.  Not perfect, they can
  // add more CRS in future and URI will be saved in project without any.
  // TODO: consider again, currently if crs in url is used to set WCS coverage CRS,
  //       without that param user is asked for CRS
  //if ( selectedLayersCRSs().size() > 1 )
  //{
  uri.setParam( QStringLiteral( "crs" ), selectedCrs() );
  //}

  QgsDebugMsg( "selectedFormat = " +  selectedFormat() );
  if ( !selectedFormat().isEmpty() )
  {
    uri.setParam( QStringLiteral( "format" ), selectedFormat() );
  }

  QgsDebugMsg( "selectedTime = " +  selectedTime() );
  if ( !selectedTime().isEmpty() )
  {
    uri.setParam( QStringLiteral( "time" ), selectedTime() );
  }

  QString cache;
  QgsDebugMsg( QStringLiteral( "selectedCacheLoadControl = %1" ).arg( selectedCacheLoadControl() ) );
  cache = QgsNetworkAccessManager::cacheLoadControlName( selectedCacheLoadControl() );
  uri.setParam( QStringLiteral( "cache" ), cache );

  emit addRasterLayer( uri.encodedUri(), identifier, QStringLiteral( "wcs" ) );
}


void QgsWCSSourceSelect::mLayersTreeWidget_itemSelectionChanged()
{

  const QString identifier = selectedIdentifier();
  if ( identifier.isEmpty() ) { return; }

  mCapabilities.describeCoverage( identifier );

  populateTimes();

  populateFormats();

  populateCrs();

  updateButtons();

  emit enableButtons( true );
}

void QgsWCSSourceSelect::updateButtons()
{

  if ( mLayersTreeWidget->selectedItems().isEmpty() )
  {
    showStatusMessage( tr( "Select a layer" ) );
  }
  else
  {
    if ( selectedCrs().isEmpty() )
    {
      showStatusMessage( tr( "No CRS selected" ) );
    }
  }

  emit enableButtons( !mLayersTreeWidget->selectedItems().isEmpty() && !selectedCrs().isEmpty() && !selectedFormat().isEmpty() );
}

QList<QgsWCSSourceSelect::SupportedFormat> QgsWCSSourceSelect::providerFormats()
{
  QList<SupportedFormat> formats;

  const QMap<QString, QString> mimes = QgsWcsProvider::supportedMimes();
  for ( auto it = mimes.constBegin(); it != mimes.constEnd(); ++it )
  {
    const SupportedFormat format = { it.key(), it.value() };

    // prefer tiff
    if ( it.key() == QLatin1String( "image/tiff" ) )
    {
      formats.prepend( format );
    }
    else
    {
      formats.append( format );
    }
  }

  return formats;
}

QStringList QgsWCSSourceSelect::selectedLayersFormats()
{

  const QString identifier = selectedIdentifier();
  if ( identifier.isEmpty() ) { return QStringList(); }

  const QgsWcsCoverageSummary c = mCapabilities.coverage( identifier );
  if ( !c.valid ) { return QStringList(); }

  QgsDebugMsg( "supportedFormat = " + c.supportedFormat.join( "," ) );
  return c.supportedFormat;
}

QStringList QgsWCSSourceSelect::selectedLayersCrses()
{
  const QString identifier = selectedIdentifier();
  if ( identifier.isEmpty() ) { return QStringList(); }

  const QgsWcsCoverageSummary c = mCapabilities.coverage( identifier );
  if ( !c.valid ) { return QStringList(); }

  return c.supportedCrs;
}

QStringList QgsWCSSourceSelect::selectedLayersTimes()
{

  const QString identifier = selectedIdentifier();
  if ( identifier.isEmpty() ) { return QStringList(); }

  const QgsWcsCoverageSummary c = mCapabilities.coverage( identifier );
  if ( !c.valid ) { return QStringList(); }

  QgsDebugMsg( "times = " + c.times.join( "," ) );
  return c.times;
}

void QgsWCSSourceSelect::enableLayersForCrs( QTreeWidgetItem * )
{
  // TODO: I am not convinced to disable layers according to selected CRS
}


void QgsWCSSourceSelect::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "working_with_ogc/ogc_client_support.html" ) );
}
