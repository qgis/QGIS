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

    /**
     * Get the count for each symbol. Only valid after the symbolsCounted()
     * signal has been emitted.
     *
     * \note Not available in Python bindings.
     */
    QHash<QString, long> symbolFeatureCountMap() const SIP_SKIP;

  signals:

    /**
     * Emitted when the symbols have been counted.
     */
    void symbolsCounted();

  private:
    std::unique_ptr<QgsVectorLayerFeatureSource> mSource;
    std::unique_ptr<QgsFeatureRenderer> mRenderer;
    QList<QgsExpressionContextScope *> mExpressionContextScopes;
    QHash<QString, long> mSymbolFeatureCountMap;
    int mFeatureCount;

};

#endif // QGSVECTORLAYERFEATURECOUNTER_H
