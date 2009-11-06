#ifndef QGSRENDERERV2REGISTRY_H
#define QGSRENDERERV2REGISTRY_H

#include <QMap>

class QgsFeatureRendererV2;
class QDomElement;

typedef QgsFeatureRendererV2* (*QgsRendererV2CreateFunc)(QDomElement&);

/**
  Registry of renderers.

  This is a singleton, renderers can be added / removed at any time
 */
class QgsRendererV2Registry
{
public:

  static QgsRendererV2Registry* instance();

  bool addRenderer(QString rendererName, QgsRendererV2CreateFunc pfCreate);

  bool removeRenderer(QString rendererName);

  QgsRendererV2CreateFunc rendererCreateFunction(QString rendererName);

protected:
  //! protected constructor
  QgsRendererV2Registry();

  static QgsRendererV2Registry* mInstance;

  QMap<QString, QgsRendererV2CreateFunc> mRenderers;
};

#endif // QGSRENDERERV2REGISTRY_H
