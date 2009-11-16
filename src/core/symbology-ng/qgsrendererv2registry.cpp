#include "qgsrendererv2registry.h"

// default renderers
#include "qgssinglesymbolrendererv2.h"
#include "qgscategorizedsymbolrendererv2.h"
#include "qgsgraduatedsymbolrendererv2.h"


QgsRendererV2Registry* QgsRendererV2Registry::mInstance = NULL;

QgsRendererV2Registry::QgsRendererV2Registry()
{
  // add default renderers
  addRenderer(QgsRendererV2Metadata("singleSymbol",
                                    QObject::tr("Single Symbol"),
                                    QgsSingleSymbolRendererV2::create,
                                    "rendererSingleSymbol.png"));
  addRenderer(QgsRendererV2Metadata("categorizedSymbol",
                                    QObject::tr("Categorized"),
                                    QgsCategorizedSymbolRendererV2::create,
                                    "rendererCategorizedSymbol.png"));
  addRenderer(QgsRendererV2Metadata("graduatedSymbol",
                                    QObject::tr("Graduated"),
                                    QgsGraduatedSymbolRendererV2::create,
                                    "rendererGraduatedSymbol.png"));
}

QgsRendererV2Registry* QgsRendererV2Registry::instance()
{
  if (!mInstance)
    mInstance = new QgsRendererV2Registry();

  return mInstance;
}


void QgsRendererV2Registry::addRenderer(const QgsRendererV2Metadata& metadata)
{
  mRenderers[metadata.name()] = metadata;
  mRenderersOrder << metadata.name();
}

bool QgsRendererV2Registry::removeRenderer(QString rendererName)
{
  if (!mRenderers.contains(rendererName))
    return false;
  mRenderers.remove(rendererName);
  mRenderersOrder.removeAll(rendererName);
  return true;
}

QgsRendererV2Metadata QgsRendererV2Registry::rendererMetadata(QString rendererName)
{
  return mRenderers.value(rendererName);
}

bool QgsRendererV2Registry::setRendererWidgetFunction(QString name, QgsRendererV2WidgetFunc f)
{
  if (!mRenderers.contains(name))
    return false;
  mRenderers[name].setWidgetFunction(f);
  return true;
}

QStringList QgsRendererV2Registry::renderersList()
{
  return mRenderersOrder;
}
