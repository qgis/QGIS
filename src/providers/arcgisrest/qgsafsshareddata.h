/***************************************************************************
    qgsafsshareddata.h
    ---------------------
    begin                : June 2017
    copyright            : (C) 2017 by Sandro Mani
    email                : manisandro at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSAFSSHAREDDATA_H
#define QGSAFSSHAREDDATA_H

#include <QObject>
#include <QMutex>
#include "qgsfields.h"
#include "qgsfeature.h"
#include "qgsdatasourceuri.h"
#include "qgscoordinatereferencesystem.h"

class QgsFeedback;

/**
 * \brief This class holds data, shared between QgsAfsProvider and QgsAfsFeatureIterator
 */
class QgsAfsSharedData : public QObject
{
    Q_OBJECT
  public:
    QgsAfsSharedData() = default;
    long long featureCount() const { return mObjectIds.size(); }
    const QgsFields &fields() const { return mFields; }
    QgsRectangle extent() const { return mExtent; }
    QgsCoordinateReferenceSystem crs() const { return mSourceCRS; }
    void clearCache();

    bool getFeature( QgsFeatureId id, QgsFeature &f, const QgsRectangle &filterRect = QgsRectangle(), QgsFeedback *feedback = nullptr );
    QgsFeatureIds getFeatureIdsInExtent( const QgsRectangle &extent, QgsFeedback *feedback );

    bool hasCachedAllFeatures() const;

  private:
    friend class QgsAfsProvider;
    QMutex mMutex;
    QgsDataSourceUri mDataSource;
    QgsRectangle mExtent;
    QgsWkbTypes::Type mGeometryType = QgsWkbTypes::Unknown;
    QgsFields mFields;
    QString mObjectIdFieldName;
    QList<quint32> mObjectIds;
    QMap<QgsFeatureId, QgsFeature> mCache;
    QgsCoordinateReferenceSystem mSourceCRS;
};

#endif
