/***************************************************************************
                          qgszonalstatisticsdialog.h  -  description
                             -----------------------
    begin                : September 1st, 2009
    copyright            : (C) 2009 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgszonalstatisticsdialog.h"
#include "qgsmaplayerregistry.h"
#include "qgsrasterlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgisinterface.h"

#include <QSettings>
#include <QListWidgetItem>

QgsZonalStatisticsDialog::QgsZonalStatisticsDialog( QgisInterface* iface ): QDialog( iface->mainWindow() ), mIface( iface )
{
  setupUi( this );

  QListWidgetItem* countItem = new QListWidgetItem( tr( "Count" ), mStatsListWidget );
  countItem->setFlags( countItem->flags() | Qt::ItemIsUserCheckable );
  countItem->setCheckState( Qt::Checked );
  countItem->setData( Qt::UserRole, QgsZonalStatistics::Count );
  mStatsListWidget->addItem( countItem );
  QListWidgetItem* sumItem = new QListWidgetItem( tr( "Sum" ), mStatsListWidget );
  sumItem->setFlags( sumItem->flags() | Qt::ItemIsUserCheckable );
  sumItem->setCheckState( Qt::Checked );
  sumItem->setData( Qt::UserRole, QgsZonalStatistics::Sum );
  mStatsListWidget->addItem( sumItem );
  QListWidgetItem* meanItem = new QListWidgetItem( tr( "Mean" ), mStatsListWidget );
  meanItem->setFlags( meanItem->flags() | Qt::ItemIsUserCheckable );
  meanItem->setCheckState( Qt::Checked );
  meanItem->setData( Qt::UserRole, QgsZonalStatistics::Mean );
  mStatsListWidget->addItem( meanItem );
  QListWidgetItem* medianItem = new QListWidgetItem( tr( "Median" ), mStatsListWidget );
  medianItem->setFlags( medianItem->flags() | Qt::ItemIsUserCheckable );
  medianItem->setCheckState( Qt::Unchecked );
  medianItem->setData( Qt::UserRole, QgsZonalStatistics::Median );
  mStatsListWidget->addItem( medianItem );
  QListWidgetItem* stdevItem = new QListWidgetItem( tr( "Standard deviation" ), mStatsListWidget );
  stdevItem->setFlags( stdevItem->flags() | Qt::ItemIsUserCheckable );
  stdevItem->setCheckState( Qt::Unchecked );
  stdevItem->setData( Qt::UserRole, QgsZonalStatistics::StDev );
  mStatsListWidget->addItem( stdevItem );
  QListWidgetItem* minItem = new QListWidgetItem( tr( "Minimum" ), mStatsListWidget );
  minItem->setFlags( minItem->flags() | Qt::ItemIsUserCheckable );
  minItem->setCheckState( Qt::Checked );
  minItem->setData( Qt::UserRole, QgsZonalStatistics::Min );
  mStatsListWidget->addItem( minItem );
  QListWidgetItem* maxItem = new QListWidgetItem( tr( "Maximum" ), mStatsListWidget );
  maxItem->setFlags( maxItem->flags() | Qt::ItemIsUserCheckable );
  maxItem->setCheckState( Qt::Checked );
  maxItem->setData( Qt::UserRole, QgsZonalStatistics::Max );
  mStatsListWidget->addItem( maxItem );
  QListWidgetItem* rangeItem = new QListWidgetItem( tr( "Range" ), mStatsListWidget );
  rangeItem->setFlags( rangeItem->flags() | Qt::ItemIsUserCheckable );
  rangeItem->setCheckState( Qt::Unchecked );
  rangeItem->setData( Qt::UserRole, QgsZonalStatistics::Range );
  mStatsListWidget->addItem( rangeItem );
  QListWidgetItem* minorityItem = new QListWidgetItem( tr( "Minority" ), mStatsListWidget );
  minorityItem->setFlags( minorityItem->flags() | Qt::ItemIsUserCheckable );
  minorityItem->setCheckState( Qt::Unchecked );
  minorityItem->setData( Qt::UserRole, QgsZonalStatistics::Minority );
  mStatsListWidget->addItem( minorityItem );
  QListWidgetItem* majorityItem = new QListWidgetItem( tr( "Majority" ), mStatsListWidget );
  majorityItem->setFlags( majorityItem->flags() | Qt::ItemIsUserCheckable );
  majorityItem->setCheckState( Qt::Unchecked );
  majorityItem->setData( Qt::UserRole, QgsZonalStatistics::Majority );
  mStatsListWidget->addItem( majorityItem );
  QListWidgetItem* varietyItem = new QListWidgetItem( tr( "Variety" ), mStatsListWidget );
  varietyItem->setFlags( varietyItem->flags() | Qt::ItemIsUserCheckable );
  varietyItem->setCheckState( Qt::Unchecked );
  varietyItem->setData( Qt::UserRole, QgsZonalStatistics::Variety );
  mStatsListWidget->addItem( varietyItem );
  QSettings settings;
  restoreGeometry( settings.value( "Plugin-ZonalStatistics/geometry" ).toByteArray() );

  insertAvailableLayers();
  mColumnPrefixLineEdit->setText( proposeAttributePrefix() );
}

QgsZonalStatisticsDialog::QgsZonalStatisticsDialog(): QDialog( nullptr ), mIface( nullptr )
{
  setupUi( this );

  QSettings settings;
  restoreGeometry( settings.value( "Plugin-ZonalStatistics/geometry" ).toByteArray() );
}

QgsZonalStatisticsDialog::~QgsZonalStatisticsDialog()
{
  QSettings settings;
  settings.setValue( "Plugin-ZonalStatistics/geometry", saveGeometry() );
}

void QgsZonalStatisticsDialog::insertAvailableLayers()
{
  //insert available raster layers
  //enter available layers into the combo box
  QMap<QString, QgsMapLayer*> mapLayers = QgsMapLayerRegistry::instance()->mapLayers();
  QMap<QString, QgsMapLayer*>::iterator layer_it = mapLayers.begin();

  for ( ; layer_it != mapLayers.end(); ++layer_it )
  {
    QgsRasterLayer* rl = dynamic_cast<QgsRasterLayer*>( layer_it.value() );
    if ( rl )
    {
      QgsRasterDataProvider* rp = rl->dataProvider();
      if ( rp && rp->name() == "gdal" )
      {
        mRasterLayerComboBox->addItem( rl->name(), QVariant( rl->id() ) );
      }
    }
    else
    {
      QgsVectorLayer* vl = dynamic_cast<QgsVectorLayer*>( layer_it.value() );
      if ( vl && vl->geometryType() == QGis::Polygon )
      {
        QgsVectorDataProvider* provider  = vl->dataProvider();
        if ( provider->capabilities() & QgsVectorDataProvider::AddAttributes )
        {
          mPolygonLayerComboBox->addItem( vl->name(), QVariant( vl->id() ) );
        }
      }
    }
  }
}

QgsRasterLayer* QgsZonalStatisticsDialog::rasterLayer() const
{
  int index = mRasterLayerComboBox->currentIndex();
  if ( index == -1 )
  {
    return nullptr;
  }
  QString id = mRasterLayerComboBox->itemData( index ).toString();
  QgsRasterLayer* layer = dynamic_cast<QgsRasterLayer*>( QgsMapLayerRegistry::instance()->mapLayer( id ) );
  return layer;
}

QString QgsZonalStatisticsDialog::rasterFilePath() const
{
  QgsRasterLayer* layer = rasterLayer();
  return layer ? layer->source() : QString();
}

int QgsZonalStatisticsDialog::rasterBand() const
{
  return mBandComboBox->currentIndex() + 1;
}

QgsVectorLayer* QgsZonalStatisticsDialog::polygonLayer() const
{
  int index = mPolygonLayerComboBox->currentIndex();
  if ( index == -1 )
  {
    return nullptr;
  }
  return dynamic_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( mPolygonLayerComboBox->itemData( index ).toString() ) );
}

QString QgsZonalStatisticsDialog::attributePrefix() const
{
  return mColumnPrefixLineEdit->text();
}

QgsZonalStatistics::Statistics QgsZonalStatisticsDialog::selectedStats() const
{
  QgsZonalStatistics::Statistics stats = nullptr;
  for ( int i = 0; i < mStatsListWidget->count(); ++i )
  {
    QListWidgetItem* item = mStatsListWidget->item( i );
    if ( item->checkState() == Qt::Checked )
    {
      stats |= ( QgsZonalStatistics::Statistic )item->data( Qt::UserRole ).toInt();
    }
  }
  return stats;
}

QString QgsZonalStatisticsDialog::proposeAttributePrefix() const
{
  if ( !polygonLayer() )
  {
    return "";
  }

  QString proposedPrefix = "";
  while ( !prefixIsValid( proposedPrefix ) )
  {
    proposedPrefix.prepend( '_' );
  }
  return proposedPrefix;
}

bool QgsZonalStatisticsDialog::prefixIsValid( const QString& prefix ) const
{
  QgsVectorLayer* vl = polygonLayer();
  if ( !vl )
  {
    return false;
  }
  QgsVectorDataProvider* dp = vl->dataProvider();
  if ( !dp )
  {
    return false;
  }

  QString currentFieldName;

  Q_FOREACH ( const QgsField& field, dp->fields() )
  {
    currentFieldName = field.name();
    if ( currentFieldName == ( prefix + "mean" ) || currentFieldName == ( prefix + "sum" ) || currentFieldName == ( prefix + "count" ) )
    {
      return false;
    }
  }
  return true;
}

void QgsZonalStatisticsDialog::on_mRasterLayerComboBox_currentIndexChanged( int index )
{
  Q_UNUSED( index );

  QgsRasterLayer* layer = rasterLayer();
  if ( !layer )
  {
    mBandComboBox->setEnabled( false );
    return;
  }

  mBandComboBox->setEnabled( true );
  mBandComboBox->clear();

  int bandCountInt = layer->bandCount();
  for ( int i = 1; i <= bandCountInt; ++i )
  {
    mBandComboBox->addItem( layer->bandName( i ) );
  }
}
