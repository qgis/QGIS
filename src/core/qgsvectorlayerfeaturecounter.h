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

/**
 * \ingroup core
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
    QgsVectorLayerFeatureCounter( QgsVectorLayer *layer, const QgsExpressionContext &context = QgsExpressionContext() );

    bool run() override;

    /**
     * Gets the count for each symbol. Only valid after the symbolsCounted()
     * signal has been emitted.
     *
     * \note Not available in Python bindings.
     */
    QHash<QString, long> symbolFeatureCountMap() const SIP_SKIP;

    /**
     * Gets the feature count for a particular \a legendKey.
     * If the key has not been found, -1 will be returned.
     */
    long featureCount( const QString &legendKey ) const;

  signals:

    /**
     * Emitted when the symbols have been counted.
     */
    void symbolsCounted();

  private:
    std::unique_ptr<QgsVectorLayerFeatureSource> mSource;
    std::unique_ptr<QgsFeatureRenderer> mRenderer;
    QgsExpressionContext mExpressionContext;
    QHash<QString, long> mSymbolFeatureCountMap;
    int mFeatureCount;

};

#endif // QGSVECTORLAYERFEATURECOUNTER_H
