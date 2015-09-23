#include "qgsvectorlayerlabeling.h"

#include "qgspallabeling.h"
#include "qgsrulebasedlabeling.h"
#include "qgsvectorlayer.h"

QgsVectorLayerLabeling::QgsVectorLayerLabeling()
    : mMode( SimpleLabels )
    //, mSimpleLabeling( 0 )
    , mRuleBasedLabeling( 0 )
{

}

QgsVectorLayerLabeling::~QgsVectorLayerLabeling()
{
  //delete mSimpleLabeling;
  delete mRuleBasedLabeling;
}

//QgsPalLayerSettings QgsVectorLayerLabeling::simpleLabeling()
//{
//}

//void QgsVectorLayerLabeling::setSimpleLabeling(QgsPalLayerSettings* settings)
//{
//  delete mSimpleLabeling;
//  mSimpleLabeling = settings;
//}

void QgsVectorLayerLabeling::setRuleBasedLabeling( QgsRuleBasedLabeling* settings )
{
  delete mRuleBasedLabeling;
  mRuleBasedLabeling = settings;
}

QgsVectorLayerLabelProvider* QgsVectorLayerLabeling::provider( QgsVectorLayer* layer )
{
  if ( mMode == SimpleLabels )
  {
    if ( layer->customProperty( "labeling" ).toString() == QString( "pal" ) && layer->labelsEnabled() )
      return new QgsVectorLayerLabelProvider( layer, false );
  }
  else // rule-based
  {
    if ( mRuleBasedLabeling )
      return new QgsRuleBasedLabelProvider( *mRuleBasedLabeling, layer, false );
  }
  return 0;
}

/*
QgsAbstractLabelProvider* QgsVectorLayerLabeling::addProviderToEngine( QgsVectorLayer* layer, QgsLabelingEngineV2* engine, QgsRenderContext& context )
{
  if ( mMode == SimpleLabels )
  {
    QgsVectorLayerLabelProvider* provider = 0;
    if ( layer->labelsEnabled() )
    {
      provider = new QgsVectorLayerLabelProvider( layer, false );
      engine->addProvider( provider );
      if ( !provider->prepare( context, attributeNames ) )
      {
        engine->removeProvider( provider );
        provider = 0; // deleted by engine
      }
    }
  }
  else // rule-based
  {
    return 0; // TODO
  }

}
*/
