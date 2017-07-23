#include "qgs3dmapconfigwidget.h"

#include "map3d.h"
#include "demterraingenerator.h"

#include "qgsrasterlayer.h"
//#include "qgsproject.h"

Qgs3DMapConfigWidget::Qgs3DMapConfigWidget( const Map3D *map, QWidget *parent )
  : QWidget( parent )
  , mMap( nullptr )
{
  setupUi( this );

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
  }

  spinTerrainScale->setValue( mMap->zExaggeration );
  spinMapResolution->setValue( mMap->tileTextureSize );
  spinScreenError->setValue( mMap->maxTerrainError );
  chkShowTileInfo->setChecked( mMap->drawTerrainTileInfo );
  chkShowBoundingBoxes->setChecked( mMap->showBoundingBoxes );
}

Qgs3DMapConfigWidget::~Qgs3DMapConfigWidget()
{
  delete mMap;
}

Map3D *Qgs3DMapConfigWidget::map()
{
  // TODO: update terrain settings

  mMap->zExaggeration = spinTerrainScale->value();
  mMap->tileTextureSize = spinMapResolution->value();
  mMap->maxTerrainError = spinScreenError->value();
  mMap->drawTerrainTileInfo = chkShowTileInfo->isChecked();
  mMap->showBoundingBoxes = chkShowBoundingBoxes->isChecked();

  return mMap;
}
