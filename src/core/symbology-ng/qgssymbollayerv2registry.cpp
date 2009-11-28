
#include "qgssymbollayerv2registry.h"

#include "qgsmarkersymbollayerv2.h"
#include "qgslinesymbollayerv2.h"
#include "qgsfillsymbollayerv2.h"

QgsSymbolLayerV2Registry* QgsSymbolLayerV2Registry::mInstance = NULL;

QgsSymbolLayerV2Registry::QgsSymbolLayerV2Registry()
{
  // init registry with known symbol layers
  addSymbolLayerType( QgsSymbolLayerV2Metadata( "SimpleLine", QgsSymbolV2::Line,
                      QgsSimpleLineSymbolLayerV2::create ) );
  addSymbolLayerType( QgsSymbolLayerV2Metadata( "MarkerLine", QgsSymbolV2::Line,
                      QgsMarkerLineSymbolLayerV2::create ) );
  addSymbolLayerType( QgsSymbolLayerV2Metadata( "LineDecoration", QgsSymbolV2::Line,
                      QgsLineDecorationSymbolLayerV2::create ) );

  addSymbolLayerType( QgsSymbolLayerV2Metadata( "SimpleMarker", QgsSymbolV2::Marker,
                      QgsSimpleMarkerSymbolLayerV2::create ) );
  addSymbolLayerType( QgsSymbolLayerV2Metadata( "SvgMarker", QgsSymbolV2::Marker,
                      QgsSvgMarkerSymbolLayerV2::create ) );

  addSymbolLayerType( QgsSymbolLayerV2Metadata( "SimpleFill", QgsSymbolV2::Fill,
                      QgsSimpleFillSymbolLayerV2::create ) );
}

void QgsSymbolLayerV2Registry::addSymbolLayerType( const QgsSymbolLayerV2Metadata& metadata )
{
  mMetadata[metadata.name()] = metadata;
}

bool QgsSymbolLayerV2Registry::setLayerTypeWidgetFunction( QString name, QgsSymbolLayerV2WidgetFunc f )
{
  if ( !mMetadata.contains( name ) )
    return false;
  mMetadata[name].setWidgetFunction( f );
  return true;
}

QgsSymbolLayerV2Metadata QgsSymbolLayerV2Registry::symbolLayerMetadata( QString name ) const
{
  if ( mMetadata.contains( name ) )
    return mMetadata.value( name );
  else
    return QgsSymbolLayerV2Metadata();
}

QgsSymbolLayerV2Registry* QgsSymbolLayerV2Registry::instance()
{
  if ( !mInstance )
    mInstance = new QgsSymbolLayerV2Registry();
  return mInstance;
}

QgsSymbolLayerV2* QgsSymbolLayerV2Registry::defaultSymbolLayer( QgsSymbolV2::SymbolType type )
{
  switch ( type )
  {
    case QgsSymbolV2::Marker:
      return QgsSimpleMarkerSymbolLayerV2::create();

    case QgsSymbolV2::Line:
      return QgsSimpleLineSymbolLayerV2::create();

    case QgsSymbolV2::Fill:
      return QgsSimpleFillSymbolLayerV2::create();
  }
  return NULL;
}


QgsSymbolLayerV2* QgsSymbolLayerV2Registry::createSymbolLayer( QString name, const QgsStringMap& properties ) const
{
  if ( !mMetadata.contains( name ) )
    return NULL;

  return mMetadata[name].createFunction()( properties );
}

QStringList QgsSymbolLayerV2Registry::symbolLayersForType( QgsSymbolV2::SymbolType type )
{
  QStringList lst;
  QMap<QString, QgsSymbolLayerV2Metadata>::ConstIterator it = mMetadata.begin();
  for ( ; it != mMetadata.end(); ++it )
  {
    if ( it->type() == type )
      lst.append( it.key() );
  }
  return lst;
}
