#include "qgsavoidintersectionsdialog.h"
#include "qgsvectorlayer.h"
#include "qgsmaplayerregistry.h"

QgsAvoidIntersectionsDialog::QgsAvoidIntersectionsDialog( QgsMapCanvas* canvas, const QSet<QString>& enabledLayers, QWidget * parent, Qt::WindowFlags f ):
    QDialog( parent, f ), mMapCanvas( canvas )
{
  setupUi( this );

  const QMap<QString, QgsMapLayer*> &mapLayers = QgsMapLayerRegistry::instance()->mapLayers();

  int i = 0;
  for ( QMap<QString, QgsMapLayer*>::const_iterator it = mapLayers.constBegin(); it != mapLayers.constEnd(); it++, i++ )
  {
    QgsVectorLayer* currentLayer = dynamic_cast<QgsVectorLayer*>( it.value() );
    if ( !currentLayer || currentLayer->geometryType() != QGis::Polygon )
      continue;

    QListWidgetItem *newItem = new QListWidgetItem( mLayersListWidget );
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

QgsAvoidIntersectionsDialog::~QgsAvoidIntersectionsDialog()
{

}

void QgsAvoidIntersectionsDialog::enabledLayers( QSet<QString>& enabledLayers )
{
  enabledLayers.clear();

  for ( int i = 0; i < mLayersListWidget->count(); ++i )
  {
    QListWidgetItem *currentItem = mLayersListWidget->item( i );
    if ( currentItem->checkState() == Qt::Checked )
    {
      enabledLayers.insert( currentItem->data( Qt::UserRole ).toString() );
    }
  }
}
