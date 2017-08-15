#include "qgs3dmapconfigwidget.h"

#include "qgs3dmapsettings.h"
#include "demterraingenerator.h"
#include "flatterraingenerator.h"
#include "utils.h"

#include "qgsmapcanvas.h"
#include "qgsrasterlayer.h"
//#include "qgsproject.h"

Qgs3DMapConfigWidget::Qgs3DMapConfigWidget( Qgs3DMapSettings *map, QgsMapCanvas *mainCanvas, QWidget *parent )
  : QWidget( parent )
  , mMap( map )
  , mMainCanvas( mainCanvas )
{
  setupUi( this );

  Q_ASSERT( map );
  Q_ASSERT( mainCanvas );

  cboTerrainLayer->setAllowEmptyLayer( true );
  cboTerrainLayer->setFilters( QgsMapLayerProxyModel::RasterLayer );

  TerrainGenerator *terrainGen = mMap->terrainGenerator();
  if ( terrainGen && terrainGen->type() == TerrainGenerator::Dem )
  {
    DemTerrainGenerator *demTerrainGen = static_cast<DemTerrainGenerator *>( terrainGen );
    spinTerrainResolution->setValue( demTerrainGen->resolution() );
    cboTerrainLayer->setLayer( demTerrainGen->layer() );
  }
  else
  {
    cboTerrainLayer->setLayer( nullptr );
    spinTerrainResolution->setEnabled( false );
    spinTerrainResolution->setValue( 16 );
  }

  spinTerrainScale->setValue( mMap->terrainVerticalScale() );
  spinMapResolution->setValue( mMap->mapTileResolution() );
  spinScreenError->setValue( mMap->maxTerrainScreenError() );
  spinGroundError->setValue( mMap->maxTerrainGroundError() );
  chkShowTileInfo->setChecked( mMap->showTerrainTilesInfo() );
  chkShowBoundingBoxes->setChecked( mMap->showTerrainBoundingBoxes() );

  connect( cboTerrainLayer, static_cast<void ( QComboBox::* )( int )>( &QgsMapLayerComboBox::currentIndexChanged ), this, &Qgs3DMapConfigWidget::onTerrainLayerChanged );
  connect( spinMapResolution, static_cast<void ( QSpinBox::* )( int )>( &QSpinBox::valueChanged ), this, &Qgs3DMapConfigWidget::updateMaxZoomLevel );
  connect( spinGroundError, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &Qgs3DMapConfigWidget::updateMaxZoomLevel );

  updateMaxZoomLevel();
}

Qgs3DMapConfigWidget::~Qgs3DMapConfigWidget()
{
}

void Qgs3DMapConfigWidget::apply()
{
  QgsRasterLayer *demLayer = qobject_cast<QgsRasterLayer *>( cboTerrainLayer->currentLayer() );

  // TODO: what if just changing generator's properties
  if ( demLayer && mMap->terrainGenerator()->type() != TerrainGenerator::Dem )
  {
    DemTerrainGenerator *demTerrainGen = new DemTerrainGenerator;
    demTerrainGen->setLayer( demLayer );
    demTerrainGen->setResolution( spinTerrainResolution->value() );
    mMap->setTerrainGenerator( demTerrainGen );
  }
  else if ( !demLayer && mMap->terrainGenerator()->type() != TerrainGenerator::Flat )
  {
    FlatTerrainGenerator *flatTerrainGen = new FlatTerrainGenerator;
    flatTerrainGen->setCrs( mMap->crs );
    flatTerrainGen->setExtent( mMainCanvas->fullExtent() );
    mMap->setTerrainGenerator( flatTerrainGen );
  }

  mMap->setTerrainVerticalScale( spinTerrainScale->value() );
  mMap->setMapTileResolution( spinMapResolution->value() );
  mMap->setMaxTerrainScreenError( spinScreenError->value() );
  mMap->setMaxTerrainGroundError( spinGroundError->value() );
  mMap->setShowTerrainTilesInfo( chkShowTileInfo->isChecked() );
  mMap->setShowTerrainBoundingBoxes( chkShowBoundingBoxes->isChecked() );
}

void Qgs3DMapConfigWidget::onTerrainLayerChanged()
{
  spinTerrainResolution->setEnabled( cboTerrainLayer->currentLayer() );
}

void Qgs3DMapConfigWidget::updateMaxZoomLevel()
{
  // TODO: tidy up, less duplication with apply()
  std::unique_ptr<TerrainGenerator> tGen;
  QgsRasterLayer *demLayer = qobject_cast<QgsRasterLayer *>( cboTerrainLayer->currentLayer() );
  if ( demLayer )
  {
    DemTerrainGenerator *demTerrainGen = new DemTerrainGenerator;
    demTerrainGen->setLayer( demLayer );
    demTerrainGen->setResolution( spinTerrainResolution->value() );
    tGen.reset( demTerrainGen );
  }
  else
  {
    FlatTerrainGenerator *flatTerrainGen = new FlatTerrainGenerator;
    flatTerrainGen->setCrs( mMap->crs );
    flatTerrainGen->setExtent( mMainCanvas->fullExtent() );
    tGen.reset( flatTerrainGen );
  }

  double tile0width = tGen->extent().width();
  int zoomLevel = Utils::maxZoomLevel( tile0width, spinMapResolution->value(), spinGroundError->value() );
  labelZoomLevels->setText( QString( "0 - %1" ).arg( zoomLevel ) );
}
