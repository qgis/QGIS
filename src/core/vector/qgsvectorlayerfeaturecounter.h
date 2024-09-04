/***************************************************************************
    qgsvectorlayerfeaturecounter.h
    ---------------------
    begin                : May 2017
    copyright            : (C) 2017 by Matthias Kuhn
    email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSVECTORLAYERFEATURECOUNTER_H
#define QGSVECTORLAYERFEATURECOUNTER_H

#include "qgsvectorlayerfeatureiterator.h"
#include "qgsrenderer.h"
#include "qgstaskmanager.h"
#include "qgsfeatureid.h"

/**
 * \ingroup core
 *
 * \brief Counts the features in a QgsVectorLayer in task.
 *
 * You should most likely not use this directly and instead call
 * QgsVectorLayer::countSymbolFeatures() and connect to the signal
 * QgsVectorLayer::symbolFeatureCountMapChanged().
 *
 */
class CORE_EXPORT QgsVectorLayerFeatureCounter : public QgsTask
{
    Q_OBJECT

  public:

    /**
     * Create a new feature counter for \a layer.
     * \param layer Target QgsVectorLayer to perform counting on.
     * \param context Specific QgsExpressionContext to use during the rendering step.
     * \param storeSymbolFids If TRUE will store the feature ids (fids), otherwise will only count the number of features per symbol. Default FALSE.
     */
    QgsVectorLayerFeatureCounter( QgsVectorLayer *layer, const QgsExpressionContext &context = QgsExpressionContext(), bool storeSymbolFids = false );
    ~QgsVectorLayerFeatureCounter() override;

    /**
     * Calculates the feature count and Ids per symbol
     */
    bool run() override;

    void cancel() override;

    /**
     * Returns the count for each symbol. Only valid after the symbolsCounted()
     * signal has been emitted.
     *
     * \note Not available in Python bindings.
     */
    QHash<QString, long long> symbolFeatureCountMap() const SIP_SKIP;

    /**
     * Returns the feature count for a particular \a legendKey.
     * If the key has not been found, -1 will be returned.
     */
    long long featureCount( const QString &legendKey ) const;

    /**
     * Returns the QgsFeatureIds for each symbol. Only valid after the symbolsCounted()
     * signal has been emitted.
     *
     * \see symbolFeatureCountMap
     * \note Not available in Python bindings.
     * \since QGIS 3.10
     */
    QHash<QString, QgsFeatureIds> symbolFeatureIdMap() const SIP_SKIP;

    /**
     * Returns the feature Ids for a particular \a legendKey.
     * If the key has not been found an empty QSet will be returned.
     *
     * \since QGIS 3.10
     */
    QgsFeatureIds featureIds( const QString &symbolkey ) const;

  signals:

    /**
     * Emitted when the symbols have been counted.
     */
    void symbolsCounted();

  private:
    std::unique_ptr<QgsVectorLayerFeatureSource> mSource;
    std::unique_ptr<QgsFeatureRenderer> mRenderer;
    QgsExpressionContext mExpressionContext;
    QHash<QString, long long> mSymbolFeatureCountMap;
    QHash<QString, QgsFeatureIds> mSymbolFeatureIdMap;
    std::unique_ptr< QgsFeedback > mFeedback;
    bool mWithFids = false;
    long mFeatureCount = 0;

};

#endif // QGSVECTORLAYERFEATURECOUNTER_H
