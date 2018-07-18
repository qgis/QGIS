/***************************************************************************
                         qgsprocessingoutputs.h
                         -------------------------
    begin                : Feb 2018
    copyright            : (C) 2018 by Arnaud Morvan
    email                : arnaud dot morvan at camptocamp dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORLAYERUPDATER_H
#define QGSVECTORLAYERUPDATER_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgsfeaturesink.h"
#include "qgsvectorlayer.h"

/**
 * \class QgsVectorLayerUpdater
 * \ingroup core
 *
 * Utility class to insert/update feature in an existing vector layer,
 * using on source and destination fields mapping and primary key handling.
 *
 * \since QGIS 3.2
 */

class CORE_EXPORT QgsVectorLayerUpdater : public QgsFeatureSink
{
  public:
    QgsVectorLayerUpdater(
      QgsVectorLayer &destinationLayer,
      const QMap<QString, QgsExpression> fieldsMap,
      QgsExpressionContext &context,
      const QList<QString> primaryKeys = QList<QString>(),
      const bool bypassEditBuffer = false
    );
    bool addFeature( QgsFeature &feature, QgsFeatureSink::Flags flags = 0 ) override;
    bool addFeatures( QgsFeatureIterator &features, QgsFeatureSink::Flags flags = 0 ) override;
    bool addFeatures( QgsFeatureList &features, QgsFeatureSink::Flags flags = 0 ) override;
  private:
    QgsVectorLayer &mLayer;
    const QMap<QString, QgsExpression> mFieldsMap;
    QgsExpressionContext &mContext;
    const QList<QString> mPrimaryKeys;
    const bool mBypassEditBuffer;
};

#endif // QGSVECTORLAYERUPDATER_H
