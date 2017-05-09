#ifndef QGSVECTORLAYERFEATURECOUNTER_H
#define QGSVECTORLAYERFEATURECOUNTER_H

#include "qgsvectorlayer.h"
#include "qgsvectorlayerfeatureiterator.h"
#include "qgsrenderer.h"
#include "qgstaskmanager.h"

/** \ingroup core
 *
 * Counts the features in a QgsVectorLayer in task.
 * You should most likely not use this directly and instead call
 * QgsVectorLayer::countSymbolFeatures() and connect to the signal
 * QgsVectorLayer::symbolFeatureCountMapChanged().
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsVectorLayerFeatureCounter : public QgsTask
{
    Q_OBJECT

  public:

    /**
     * Create a new feature counter for \a layer.
     */
    QgsVectorLayerFeatureCounter( QgsVectorLayer *layer );

    virtual bool run() override;

  signals:
    void symbolsCounted( const QHash<QString, long> &symbolFeatureCountMap );

  private:
    std::unique_ptr<QgsVectorLayerFeatureSource> mSource;
    std::unique_ptr<QgsFeatureRenderer> mRenderer;
    QList<QgsExpressionContextScope *> mExpressionContextScopes;
    QHash<QString, long> mSymbolFeatureCountMap;
    int mFeatureCount;

};

#endif // QGSVECTORLAYERFEATURECOUNTER_H
