#include "qgs3dmapconfigwidget.h"

#include "map3d.h"
#include "demterraingenerator.h"
#include "flatterraingenerator.h"

#include "qgsmapcanvas.h"
#include "qgsrasterlayer.h"
//#include "qgsproject.h"

Qgs3DMapConfigWidget::Qgs3DMapConfigWidget( const Map3D *map, QgsMapCanvas *mainCanvas, QWidget *parent )
  : QWidget( parent )
  , mMap( nullptr )
  , mMainCanvas( mainCanvas )
{
  setupUi( this );

  Q_ASSERT( map );
  Q_ASSERT( mainCanvas );

  mMap = new Map3D( *map );

  cboTerrainLayer->setAllowEmptyLayer( true );
  cboTerrainLayer->setFilters( QgsMapLayerProxyModel::RasterLayer );

  TerrainGenerator *terrainGen = mMap->terrainGenerator.get();
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
  chkShowTileInfo->setChecked( mMap->drawTerrainTileInfo );
  chkShowBoundingBoxes->setChecked( mMap->showBoundingBoxes );

  connect( cboTerrainLayer, static_cast<void ( QComboBox::* )( int )>( &QgsMapLayerComboBox::currentIndexChanged ), this, &Qgs3DMapConfigWidget::onTerrainLayerChanged );
}

Qgs3DMapConfigWidget::~Qgs3DMapConfigWidget()
{
  delete mMap;
}

Map3D *Qgs3DMapConfigWidget::map()
{
  // update terrain settings
  if ( QgsRasterLayer *demLayer = qobject_cast<QgsRasterLayer *>( cboTerrainLayer->currentLayer() ) )
  {
    DemTerrainGenerator *demTerrainGen = new DemTerrainGenerator;
    demTerrainGen->setLayer( demLayer );
    demTerrainGen->setResolution( spinTerrainResolution->value() );
    mMap->terrainGenerator.reset( demTerrainGen );
  }
  else
  {
    FlatTerrainGenerator *flatTerrainGen = new FlatTerrainGenerator;
    flatTerrainGen->setCrs( mMap->crs );
    flatTerrainGen->setExtent( mMainCanvas->fullExtent() );
    mMap->terrainGenerator.reset( flatTerrainGen );
  }

  mMap->zExaggeration = spinTerrainScale->value();
  mMap->tileTextureSize = spinMapResolution->value();
  mMap->maxTerrainError = spinScreenError->value();
  mMap->drawTerrainTileInfo = chkShowTileInfo->isChecked();
  mMap->showBoundingBoxes = chkShowBoundingBoxes->isChecked();

  return mMap;
}

void Qgs3DMapConfigWidget::onTerrainLayerChanged()
{
  spinTerrainResolution->setEnabled( cboTerrainLayer->currentLayer() );
}
