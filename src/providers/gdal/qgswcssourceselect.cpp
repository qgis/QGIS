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

#include "qgsgdalprovider.h"
#include "qgswcssourceselect.h"
#include "qgswcscapabilities.h"
#include "qgsnumericsortlistviewitem.h"

#include <QWidget>

#define CPL_SUPRESS_CPLUSPLUS
#include <gdal.h>

#if defined(GDAL_VERSION_NUM) && GDAL_VERSION_NUM >= 1800
#define TO8F(x) (x).toUtf8().constData()
#define FROM8(x) QString::fromUtf8(x)
#else
#define TO8F(x) QFile::encodeName( x ).constData()
#define FROM8(x) QString::fromLocal8Bit(x)
#endif

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

  mCapabilities.setUri( mUri );

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
  if ( selectedLayersCRSs().size() > 1 )
  {
    uri.setParam( "crs", selectedCRS() );
  }

  QgsDebugMsg( "selectedFormat = " +  selectedFormat() );
  if ( !selectedFormat().isEmpty() )
  {
    uri.setParam( "format", selectedFormat() );
  }

  emit addRasterLayer( uri.encodedUri(), identifier, "gdal" );
}

void QgsWCSSourceSelect::on_mLayersTreeWidget_itemSelectionChanged()
{
  QgsDebugMsg( "entered" );

  QString identifier = selectedIdentifier();
  if ( identifier.isEmpty() ) { return; }

  mCapabilities.describeCoverage( identifier );  // 1.0 get additional info

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

QList<QgsOWSSupportedFormat> QgsWCSSourceSelect::providerFormats()
{
  QgsDebugMsg( "entered" );
  QList<QgsOWSSupportedFormat> formats;

  QMap<QString, QString> mimes = QgsGdalProvider::supportedMimes();
  foreach( QString mime, mimes.keys() )
  {
    QgsOWSSupportedFormat format = { mime, mimes.value( mime ) };

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

  QgsWcsCoverageSummary * c = mCapabilities.coverageSummary( identifier );
  if ( !c ) { return QStringList(); }

  QgsDebugMsg( "supportedFormat = " + c->supportedFormat.join( "," ) );
  return c->supportedFormat;
}

QStringList QgsWCSSourceSelect::selectedLayersCRSs()
{
  QgsDebugMsg( "entered" );

  QString identifier = selectedIdentifier();
  if ( identifier.isEmpty() ) { return QStringList(); }

  QgsWcsCoverageSummary * c = mCapabilities.coverageSummary( identifier );
  if ( !c ) { return QStringList(); }

  return c->supportedCrs;
}

void QgsWCSSourceSelect::enableLayersForCrs( QTreeWidgetItem * )
{
  // TODO: I am not convinced to disable layers according to selected CRS
}
