#include "qgs3dmapconfigwidget.h"

#include "map3d.h"
#include "demterraingenerator.h"
#include "flatterraingenerator.h"

#include "qgsmapcanvas.h"
#include "qgsrasterlayer.h"
//#include "qgsproject.h"

Qgs3DMapConfigWidget::Qgs3DMapConfigWidget( Map3D *map, QgsMapCanvas *mainCanvas, QWidget *parent )
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

  spinTerrainScale->setValue( mMap->zExaggeration );
  spinMapResolution->setValue( mMap->tileTextureSize );
  spinScreenError->setValue( mMap->maxTerrainError );
  chkShowTileInfo->setChecked( mMap->showTerrainTilesInfo() );
  chkShowBoundingBoxes->setChecked( mMap->showTerrainBoundingBoxes() );

  connect( cboTerrainLayer, static_cast<void ( QComboBox::* )( int )>( &QgsMapLayerComboBox::currentIndexChanged ), this, &Qgs3DMapConfigWidget::onTerrainLayerChanged );
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

  mMap->zExaggeration = spinTerrainScale->value();
  mMap->tileTextureSize = spinMapResolution->value();
  mMap->maxTerrainError = spinScreenError->value();
  mMap->setShowTerrainTilesInfo( chkShowTileInfo->isChecked() );
  mMap->setShowTerrainBoundingBoxes( chkShowBoundingBoxes->isChecked() );
}

void Qgs3DMapConfigWidget::onTerrainLayerChanged()
{
  spinTerrainResolution->setEnabled( cboTerrainLayer->currentLayer() );
}
