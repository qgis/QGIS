#include "qgsvectorlayerlabeling.h"

#include "qgspallabeling.h"
#include "qgsrulebasedlabeling.h"
#include "qgsvectorlayer.h"


QgsAbstractVectorLayerLabeling::~QgsAbstractVectorLayerLabeling()
{
}

QgsAbstractVectorLayerLabeling* QgsAbstractVectorLayerLabeling::create( const QDomElement& element )
{
  if ( element.attribute( "type" ) == "rule-based" )
  {
    return QgsRuleBasedLabeling::create( element );
  }
  else
  {
    // default
    return new QgsVectorLayerSimpleLabeling;
  }
}

QgsVectorLayerLabelProvider* QgsVectorLayerSimpleLabeling::provider( QgsVectorLayer* layer ) const
{
  if ( layer->customProperty( "labeling" ).toString() == QString( "pal" ) && layer->labelsEnabled() )
    return new QgsVectorLayerLabelProvider( layer, false );

  return 0;
}

QString QgsVectorLayerSimpleLabeling::type() const
{
  return "simple";
}

QDomElement QgsVectorLayerSimpleLabeling::save( QDomDocument& doc ) const
{
  // all configuration is kept in layer custom properties (for compatibility)
  QDomElement elem = doc.createElement( "labeling" );
  elem.setAttribute( "type", "simple" );
  return elem;
}
