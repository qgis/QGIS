#include "qgspendingchangesmodel.h"

#include "qgslogger.h"
#include "qgsfeature.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayereditbuffer.h"

#include <QStandardItem>

QgsPendingChangesModel::QgsPendingChangesModel( QgsProject *project, QObject *parent )
  : QStandardItemModel( parent )
{
  setColumnCount( 2 );
  setHorizontalHeaderLabels( QStringList() << "Item" << "Layer" );
  mAddedFeaturesRoot = new QStandardItem( tr( "Added features" ) );
  mChagnedFeaturesRoot = new QStandardItem( tr( "Modified features" ) );
  mDeletedFeatuersRoot = new QStandardItem( tr( "Deleted features" ) );

  invisibleRootItem()->appendRow( mAddedFeaturesRoot );
  invisibleRootItem()->appendRow( mChagnedFeaturesRoot );
  invisibleRootItem()->appendRow( mDeletedFeatuersRoot );

  connect( project, &QgsProject::layersAdded, this, &QgsPendingChangesModel::connectAddedLayers );
}

void QgsPendingChangesModel::connectAddedLayers( const QList<QgsMapLayer *> layers )
{
  Q_FOREACH ( QgsMapLayer *layer, layers )
  {
    if ( layer->type() == QgsMapLayer::VectorLayer )
    {
      QgsVectorLayer *vLayer = qobject_cast<QgsVectorLayer *>( layer );

      connect( vLayer, &QgsVectorLayer::featureAdded, this, [this, vLayer]( QgsFeatureId id )
      {
        featureAdded( id, vLayer );
      } );

      // TODO Swap this for featuresDeleted for better speed
      connect( vLayer, &QgsVectorLayer::featureDeleted, this, [this, vLayer]( QgsFeatureId id )
      {
        featureDeleted( id, vLayer );
      } );
      connect( vLayer, &QgsVectorLayer::attributeValueChanged, this, [this, vLayer]( QgsFeatureId id )
      {
        featureModified( id, vLayer );
      } );

      // TODO Remove old entries on layer remove or editing stopped.
    }
  }
}

void QgsPendingChangesModel::featureAdded( QgsFeatureId id, QgsVectorLayer *layer )
{
  if ( nodeContainsFeature( mAddedFeaturesRoot, id, layer ) )
    return;
  mAddedFeaturesRoot->appendRow( makeFeatureItem( id, layer ) );
}

void QgsPendingChangesModel::featureDeleted( QgsFeatureId id, QgsVectorLayer *layer )
{
  if ( nodeContainsFeature( mDeletedFeatuersRoot, id, layer ) )
    return;
  mDeletedFeatuersRoot->appendRow( makeFeatureItem( id, layer ) );
}

void QgsPendingChangesModel::featureModified( QgsFeatureId id, QgsVectorLayer *layer )
{
  if ( nodeContainsFeature( mChagnedFeaturesRoot, id, layer ) )
    return;
  mChagnedFeaturesRoot->appendRow( makeFeatureItem( id, layer ) );
}

bool QgsPendingChangesModel::nodeContainsFeature( QStandardItem *root, QgsFeatureId id, QgsVectorLayer *layer )
{
  // This is just a hack for now until I do something better.
  QString key = FID_TO_STRING( id ) + layer->id();
  for ( int row = 0; row < root->rowCount(); ++row )
  {
    QStandardItem *item = root->child( row );
    QString itemkey = item->data().toString();
    if ( itemkey == key )
    {
      return true;
    }
  }
  return false;
}

QList<QStandardItem *> QgsPendingChangesModel::makeFeatureItem( QgsFeatureId id, QgsVectorLayer *layer ) const
{
  QList<QStandardItem *> items;
  QStandardItem *featureitem = new QStandardItem( FID_TO_STRING( id ) );
  featureitem->setData( FID_TO_STRING( id ) + layer->id() );
  QStandardItem *layeritem = new QStandardItem( layer->name() );
  layeritem->setData( FID_TO_STRING( id ) + layer->id() );
  items << featureitem;
  items << layeritem;
  return items;
}
