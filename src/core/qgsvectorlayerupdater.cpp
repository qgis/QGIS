#include "qgsvectorlayerupdater.h"


QgsVectorLayerUpdater::QgsVectorLayerUpdater(
  QgsVectorLayer &destinationLayer,
  const QMap<QString, QgsExpression> fieldsMap,
  QgsExpressionContext &context,
  const QList<QString> primaryKeys,
  const bool bypassEditBuffer )
  : mLayer( destinationLayer )
  , mFieldsMap( fieldsMap )
  , mContext( context )
  , mPrimaryKeys( primaryKeys )
  , mBypassEditBuffer( bypassEditBuffer )
{

}

bool QgsVectorLayerUpdater::addFeature( QgsFeature &feature, QgsFeatureSink::Flags flags )
{
  Q_UNUSED( flags );

  mContext.setFeature( ( const QgsFeature & ) feature );

  QgsFeature f = QgsFeature();
  f.setGeometry( feature.geometry() );

  //QgsAttributes &values( mLayer.fields().count() );
  for ( int i = 0; i < mLayer.fields().count(); i++ )
  {
    QgsField field = mLayer.fields()[i];
    QgsExpression expr = mFieldsMap[field.name()];
    f.setAttribute( i, expr.evaluate( ( const QgsExpressionContext * ) &mContext ) );
  }

  bool updated = false;
  if ( !mPrimaryKeys.isEmpty() )
  {
    QgsFeatureRequest request = QgsFeatureRequest();
    Q_FOREACH ( const QString &pKey, mPrimaryKeys )
    {
      request.combineFilterExpression( QgsExpression( QStringLiteral( "%1=%2" )
                                       .arg( QgsExpression::quotedColumnRef( pKey )
                                             .arg( QgsExpression::quotedValue( f.attribute( mLayer.fields().indexOf( pKey ) ) ) ) ) ) );
    }
    QgsFeatureIterator candidates = mLayer.getFeatures( request );
    QgsFeature candidate;
    while ( candidates.nextFeature( candidate ) )
    {
      f.setId( candidate.id() );
      mLayer.updateFeature( f );
      updated = true;
    }
  }
  if ( !updated )
  {
    mLayer.addFeature( f );
  }

  return true;
}

bool QgsVectorLayerUpdater::addFeatures( QgsFeatureIterator &features, QgsFeatureSink::Flags flags )
{
  Q_UNUSED( flags );

  QgsFeature f;
  while ( features.nextFeature( f ) )
  {
    addFeature( f, flags );
  }

  return true;
}


bool QgsVectorLayerUpdater::addFeatures( QgsFeatureList &features, QgsFeatureSink::Flags flags )
{
  Q_UNUSED( flags );

  Q_FOREACH ( QgsFeature f, features )
  {
    addFeature( f, flags );
  }

  return true;
}
