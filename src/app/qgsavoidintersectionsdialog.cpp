#include "qgsavoidintersectionsdialog.h"
#include "qgsmapcanvas.h"
#include "qgsvectorlayer.h"

QgsAvoidIntersectionsDialog::QgsAvoidIntersectionsDialog( QgsMapCanvas* canvas, const QSet<QString>& enabledLayers, QWidget * parent, Qt::WindowFlags f ): \
    QDialog( parent, f ), mMapCanvas( canvas )
{
  setupUi( this );

  int nLayers = mMapCanvas->layerCount();
  QgsVectorLayer* currentLayer = 0;
  QListWidgetItem* newItem = 0;

  for ( int i = 0; i < nLayers; ++i )
  {
    currentLayer = dynamic_cast<QgsVectorLayer*>( mMapCanvas->layer( i ) );
    if ( currentLayer )
    {
      //only consider polygon or multipolygon layers
      if ( currentLayer->geometryType() == QGis::Polygon )
      {
        newItem = new QListWidgetItem( mLayersListWidget );
        newItem->setText( currentLayer->name() );
        newItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsUserCheckable );
        newItem->setData( Qt::UserRole, currentLayer->getLayerID() );
        if ( enabledLayers.contains( currentLayer->getLayerID() ) )
        {
          newItem->setCheckState( Qt::Checked );
        }
        else
        {
          newItem->setCheckState( Qt::Unchecked );
        }
      }
    }
  }
}

QgsAvoidIntersectionsDialog::~QgsAvoidIntersectionsDialog()
{

}

void QgsAvoidIntersectionsDialog::enabledLayers( QSet<QString>& enabledLayers )
{
  enabledLayers.clear();

  int itemCount = mLayersListWidget->count();
  QListWidgetItem* currentItem = 0;

  for ( int i = 0; i < itemCount; ++i )
  {
    currentItem = mLayersListWidget->item( i );
    if ( currentItem->checkState() == Qt::Checked )
    {
      enabledLayers.insert( currentItem->data( Qt::UserRole ).toString() );
    }
  }
}

