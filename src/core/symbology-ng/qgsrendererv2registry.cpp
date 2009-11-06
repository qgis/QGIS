#include "qgsrendererv2registry.h"

// default renderers
#include "qgssinglesymbolrendererv2.h"
#include "qgscategorizedsymbolrendererv2.h"
#include "qgsgraduatedsymbolrendererv2.h"


QgsRendererV2Registry* QgsRendererV2Registry::mInstance = NULL;

QgsRendererV2Registry::QgsRendererV2Registry()
{
  // add default renderers
  addRenderer("singleSymbol", QgsSingleSymbolRendererV2::create);
  addRenderer("categorizedSymbol", QgsCategorizedSymbolRendererV2::create);
  addRenderer("graduatedSymbol", QgsGraduatedSymbolRendererV2::create);
}

QgsRendererV2Registry* QgsRendererV2Registry::instance()
{
  if (!mInstance)
    mInstance = new QgsRendererV2Registry();

  return mInstance;
}


bool QgsRendererV2Registry::addRenderer(QString rendererName, QgsRendererV2CreateFunc pfCreate)
{
  if (mRenderers.contains(rendererName))
    return false;
  mRenderers.insert(rendererName, pfCreate);
  return true;
}

bool QgsRendererV2Registry::removeRenderer(QString rendererName)
{
  if (!mRenderers.contains(rendererName))
    return false;
  mRenderers.remove(rendererName);
  return true;
}

QgsRendererV2CreateFunc QgsRendererV2Registry::rendererCreateFunction(QString rendererName)
{
  return mRenderers.value(rendererName, NULL);
}
