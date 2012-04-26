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
    : QgsOWSSourceSelect ( "WCS", parent, fl, managerMode, embeddedMode )
{
  // Hide irrelevant widgets 
  mWMSGroupBox->hide();
  mLayersTab->layout()->removeWidget ( mWMSGroupBox );
  mTabWidget->removeTab( mTabWidget->indexOf( mLayerOrderTab ) );
  mTabWidget->removeTab( mTabWidget->indexOf( mTilesetsTab ) );
  mTabWidget->removeTab( mTabWidget->indexOf( mSearchTab ) );
  mAddDefaultButton->hide();
  
  mLayersTreeWidget->setSelectionMode ( QAbstractItemView::SingleSelection );
}

QgsWCSSourceSelect::~QgsWCSSourceSelect()
{
}

void QgsWCSSourceSelect::populateLayerList(  )
{
  QgsDebugMsg( "entered" );

  mCapabilities.setUri ( mUri );

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

  mLayersTreeWidget->clear();
  mLayersTreeWidget->setSortingEnabled( true );

  int coverageAndStyleCount = -1;

  for ( QVector<QgsWcsCoverageSummary>::iterator coverage = coverages.begin();
        coverage != coverages.end();
        coverage++ )
  {
    QgsDebugMsg( QString( "coverage orderId = %1 identifier = %2").arg(coverage->orderId).arg(coverage->identifier) );

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

void QgsWCSSourceSelect::addClicked( )
{
  QgsDebugMsg ( "entered");
  QgsDataSourceURI uri = mUri; 
  
  QList<QTreeWidgetItem *> selectionList = mLayersTreeWidget->selectedItems();
  if ( selectionList.size() < 1 ) return; // should not happen
  QString identifier = selectionList.value(0)->data( 0, Qt::UserRole + 0 ).toString();
  QgsDebugMsg ( " identifier = " + identifier );

  uri.setParam( "identifier", identifier );

  uri.setParam( "crs", selectedCRS() );
  
  QgsDebugMsg ( "selectedFormat = " +  selectedFormat() );
  uri.setParam( "format", selectedFormat() ); 

  emit addRasterLayer( uri.encodedUri(), identifier, "gdal" );
}

void QgsWCSSourceSelect::on_mLayersTreeWidget_itemSelectionChanged()
{
  QgsDebugMsg ( "entered");
  populateFormats();

  populateCRS();

  mAddButton->setEnabled(true);
}

void QgsWCSSourceSelect::updateButtons()
{
  QgsDebugMsg ( "entered");

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
  QgsDebugMsg ( "entered");
  QList<QgsOWSSupportedFormat> formats;
  GDALAllRegister();

  QgsDebugMsg ( QString( "GDAL drivers cont %1").arg(GDALGetDriverCount()) );
  for ( int i = 0; i < GDALGetDriverCount(); ++i )
  {
    GDALDriverH driver = GDALGetDriver( i );
    Q_CHECK_PTR( driver );
    
    if ( !driver )
    {
      QgsLogger::warning( "unable to get driver " + QString::number( i ) );
      continue;
    }

    QString desc = GDALGetDescription( driver );

    QString mimeType = GDALGetMetadataItem ( driver, "DMD_MIMETYPE", "" );

    if ( mimeType.isEmpty() ) continue;

    desc = desc.isEmpty() ? mimeType : desc;

    QgsOWSSupportedFormat format = { mimeType, desc };

    QgsDebugMsg ( "add GDAL format " + mimeType + " " + desc );

    if ( mimeType == "image/tiff" )
    {
      formats.prepend ( format );
    }
    else
    {
      formats.append ( format );
    }
  }

  return formats;
}

QStringList QgsWCSSourceSelect::selectedLayersFormats()
{
  QgsDebugMsg ( "entered");

  QList<QTreeWidgetItem *> selectionList = mLayersTreeWidget->selectedItems();
  if ( selectionList.size() < 1 ) return QStringList();
  QString identifier = selectionList.value(0)->data( 0, Qt::UserRole + 0 ).toString();
  QgsDebugMsg ( " identifier = " + identifier );
  
  QgsWcsCoverageSummary c = mCapabilities.coverageSummary(identifier);
  return c.supportedFormat;  
}

QStringList QgsWCSSourceSelect::selectedLayersCRSs()
{
  QgsDebugMsg ( "entered");

  QList<QTreeWidgetItem *> selectionList = mLayersTreeWidget->selectedItems();
  if ( selectionList.size() < 1 ) return QStringList();
  QString identifier = selectionList.value(0)->data( 0, Qt::UserRole + 0 ).toString();
  QgsDebugMsg ( " identifier = " + identifier );

  QgsWcsCoverageSummary c = mCapabilities.coverageSummary(identifier);

  return c.supportedCrs;
}

void QgsWCSSourceSelect::enableLayersForCrs( QTreeWidgetItem *item )
{
  // TODO: I am not convinced to disable layers according to selected CRS
}
