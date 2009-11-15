#ifndef QGSRENDERERV2REGISTRY_H
#define QGSRENDERERV2REGISTRY_H

#include <QMap>
#include <QStringList>

class QgsFeatureRendererV2;
class QDomElement;
class QgsVectorLayer;
class QgsStyleV2;
class QgsRendererV2Widget;

typedef QgsFeatureRendererV2* (*QgsRendererV2CreateFunc)(QDomElement&);
typedef QgsRendererV2Widget* (*QgsRendererV2WidgetFunc)(QgsVectorLayer*, QgsStyleV2*, QgsFeatureRendererV2*);

class QgsRendererV2Metadata
{
public:
  /** construct invalid metadata */
  QgsRendererV2Metadata()
   : mName(), mVisibleName(), mCreateFunc(NULL), mIconName(), mWidgetFunc(NULL) {}

  /** construct metadata */
  QgsRendererV2Metadata(QString name,
                        QString visibleName,
                        QgsRendererV2CreateFunc pfCreate,
                        QString iconName = QString(),
                        QgsRendererV2WidgetFunc pfWidget = NULL)
   : mName(name), mVisibleName(visibleName), mCreateFunc(pfCreate), mIconName(iconName), mWidgetFunc(pfWidget) {}

  QString name() const { return mName; }
  QString visibleName() const { return mVisibleName; }
  QString iconName() const { return mIconName; }
  QgsRendererV2CreateFunc createFunction() const { return mCreateFunc; }
  QgsRendererV2WidgetFunc widgetFunction() const { return mWidgetFunc; }

  void setWidgetFunction(QgsRendererV2WidgetFunc f) { mWidgetFunc = f; }

protected:
  //! name used within QGIS for identification (the same what renderer's type() returns)
  QString mName;
  //! name visible for users (translatable)
  QString mVisibleName;
  //! pointer to function that creates an instance of the renderer when loading project / style
  QgsRendererV2CreateFunc mCreateFunc;
  //! icon to be shown in the renderer properties dialog
  QString mIconName;
  //! pointer to function that creates a widget for configuration of renderer's params
  QgsRendererV2WidgetFunc mWidgetFunc;
};

/**
  Registry of renderers.

  This is a singleton, renderers can be added / removed at any time
 */
class QgsRendererV2Registry
{
public:

  static QgsRendererV2Registry* instance();

  //! add a renderer to registry
  void addRenderer(const QgsRendererV2Metadata& metadata);

  //! remove renderer from registry
  bool removeRenderer(QString rendererName);

  //! get factory method for particular renderer
  QgsRendererV2Metadata rendererMetadata(QString rendererName);

  //! assign a widget factory to particular renderer
  bool setRendererWidgetFunction(QString name, QgsRendererV2WidgetFunc f);

  //! return a list of available renderers
  QStringList renderersList();

protected:
  //! protected constructor
  QgsRendererV2Registry();

  static QgsRendererV2Registry* mInstance;

  QMap<QString, QgsRendererV2Metadata> mRenderers;

  //! list to keep order in which renderers have been added
  QStringList mRenderersOrder;
};

#endif // QGSRENDERERV2REGISTRY_H
