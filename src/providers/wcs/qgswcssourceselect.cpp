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
#include "qgsnumericsortlistviewitem.h"

#include <QWidget>

QgsWCSSourceSelect::QgsWCSSourceSelect( QWidget * parent, Qt::WFlags fl, bool managerMode, bool embeddedMode )
    : QgsOWSSourceSelect( "WCS", parent, fl, managerMode, embeddedMode )
{
  // Hide irrelevant widgets
  mWMSGroupBox->hide();
  mLayersTab->layout()->removeWidget( mWMSGroupBox );
  mTabWidget->removeTab( mTabWidget->indexOf( mLayerOrderTab ) );
  mTabWidget->removeTab( mTabWidget->indexOf( mTilesetsTab ) );
  mTabWidget->removeTab( mTabWidget->indexOf( mSearchTab ) );
  mAddDefaultButton->hide();

  mLayersTreeWidget->setSelectionMode( QAbstractItemView::SingleSelection );
}

QgsWCSSourceSelect::~QgsWCSSourceSelect()
{
}

void QgsWCSSourceSelect::populateLayerList( )
{
  QgsDebugMsg( "entered" );

  mLayersTreeWidget->clear();


  QgsDataSourceURI uri = mUri;
  QString cache = QgsNetworkAccessManager::cacheLoadControlName( selectedCacheLoadControl() );
  uri.setParam( "cache", cache );

  mCapabilities.setUri( uri );

  if ( !mCapabilities.lastError().isEmpty() )
  {
    showError( mCapabilities.lastErrorTitle(), mCapabilities.lastErrorFormat(), mCapabilities.lastError() );
    return;
  }

  QVector<QgsWcsCoverageSummary> coverages;
  if ( !mCapabilities.supportedCoverages( coverages ) )
    return;

  QMap<int, QgsNumericSortTreeWidgetItem *> items;
  QMap<int, int> coverageParents;
  QMap<int, QStringList> coverageParentNames;
  mCapabilities.coverageParents( coverageParents, coverageParentNames );

  mLayersTreeWidget->setSortingEnabled( true );

  int coverageAndStyleCount = -1;

  for ( QVector<QgsWcsCoverageSummary>::iterator coverage = coverages.begin();
        coverage != coverages.end();
        coverage++ )
  {
    QgsDebugMsg( QString( "coverage orderId = %1 identifier = %2" ).arg( coverage->orderId ).arg( coverage->identifier ) );

    QgsNumericSortTreeWidgetItem *lItem = createItem( coverage->orderId, QStringList() << coverage->identifier << coverage->title << coverage->abstract, items, coverageAndStyleCount, coverageParents, coverageParentNames );

    lItem->setData( 0, Qt::UserRole + 0, coverage->identifier );
    lItem->setData( 0, Qt::UserRole + 1, "" );

    // Make only leaves selectable
    if ( coverageParents.keys( coverage->orderId ).size() > 0 )
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
  QList<QTreeWidgetItem *> selectionList = mLayersTreeWidget->selectedItems();
  if ( selectionList.size() < 1 ) return QString(); // should not happen
  QString identifier = selectionList.value( 0 )->data( 0, Qt::UserRole + 0 ).toString();
  QgsDebugMsg( " identifier = " + identifier );
  return identifier;
}

void QgsWCSSourceSelect::addClicked( )
{
  QgsDebugMsg( "entered" );
  QgsDataSourceURI uri = mUri;

  QString identifier = selectedIdentifier();
  if ( identifier.isEmpty() ) { return; }

  uri.setParam( "identifier", identifier );

  // Set crs only if necessary (multiple offered), so that we can decide in the
  // provider if WCS 1.0 with RESPONSE_CRS has to be used.  Not perfect, they can
  // add more CRS in future and URI will be saved in project without any.
  // TODO: consider again, currently if crs in url is used to set WCS coverage CRS,
  //       without that param user is asked for CRS
  //if ( selectedLayersCRSs().size() > 1 )
  //{
  uri.setParam( "crs", selectedCRS() );
  //}

  QgsDebugMsg( "selectedFormat = " +  selectedFormat() );
  if ( !selectedFormat().isEmpty() )
  {
    uri.setParam( "format", selectedFormat() );
  }

  QgsDebugMsg( "selectedTime = " +  selectedTime() );
  if ( !selectedTime().isEmpty() )
  {
    uri.setParam( "time", selectedTime() );
  }

  QString cache;
  QgsDebugMsg( QString( "selectedCacheLoadControl = %1" ).arg( selectedCacheLoadControl() ) );
  cache = QgsNetworkAccessManager::cacheLoadControlName( selectedCacheLoadControl() );
  uri.setParam( "cache", cache );

  emit addRasterLayer( uri.encodedUri(), identifier, "wcs" );
}

void QgsWCSSourceSelect::on_mLayersTreeWidget_itemSelectionChanged()
{
  QgsDebugMsg( "entered" );

  QString identifier = selectedIdentifier();
  if ( identifier.isEmpty() ) { return; }

  mCapabilities.describeCoverage( identifier );

  populateTimes();

  populateFormats();

  populateCRS();

  updateButtons();

  mAddButton->setEnabled( true );
}

void QgsWCSSourceSelect::updateButtons()
{
  QgsDebugMsg( "entered" );

  if ( mLayersTreeWidget->selectedItems().isEmpty() )
  {
    showStatusMessage( tr( "Select a layer" ) );
  }
  else
  {
    if ( selectedCRS().isEmpty() )
    {
      showStatusMessage( tr( "No CRS selected" ) );
    }
  }

  mAddButton->setEnabled( !mLayersTreeWidget->selectedItems().isEmpty() && !selectedCRS().isEmpty() && !selectedFormat().isEmpty() );
}

QList<QgsWCSSourceSelect::SupportedFormat> QgsWCSSourceSelect::providerFormats()
{
  QgsDebugMsg( "entered" );
  QList<SupportedFormat> formats;

  QMap<QString, QString> mimes = QgsWcsProvider::supportedMimes();
  foreach ( QString mime, mimes.keys() )
  {
    SupportedFormat format = { mime, mimes.value( mime ) };

    // prefer tiff
    if ( mime == "image/tiff" )
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
  QgsDebugMsg( "entered" );

  QString identifier = selectedIdentifier();
  if ( identifier.isEmpty() ) { return QStringList(); }

  QgsWcsCoverageSummary c = mCapabilities.coverage( identifier );
  if ( !c.valid ) { return QStringList(); }

  QgsDebugMsg( "supportedFormat = " + c.supportedFormat.join( "," ) );
  return c.supportedFormat;
}

QStringList QgsWCSSourceSelect::selectedLayersCRSs()
{
  QgsDebugMsg( "entered" );

  QString identifier = selectedIdentifier();
  if ( identifier.isEmpty() ) { return QStringList(); }

  QgsWcsCoverageSummary c = mCapabilities.coverage( identifier );
  if ( !c.valid ) { return QStringList(); }

  return c.supportedCrs;
}

QStringList QgsWCSSourceSelect::selectedLayersTimes()
{
  QgsDebugMsg( "entered" );

  QString identifier = selectedIdentifier();
  if ( identifier.isEmpty() ) { return QStringList(); }

  QgsWcsCoverageSummary c = mCapabilities.coverage( identifier );
  if ( !c.valid ) { return QStringList(); }

  QgsDebugMsg( "times = " + c.times.join( "," ) );
  return c.times;
}

void QgsWCSSourceSelect::enableLayersForCrs( QTreeWidgetItem * )
{
  // TODO: I am not convinced to disable layers according to selected CRS
}
