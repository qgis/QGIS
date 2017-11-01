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
#include "qgsfields.h"
#include "qgsfeature.h"
#include "qgsdatasourceuri.h"

/**
 * \brief This class holds data, shared between QgsAfsProvider and QgsAfsFeatureIterator
 **/
class QgsAfsSharedData : public QObject
{
    Q_OBJECT
  public:
    QgsAfsSharedData() = default;
    long featureCount() const { return mObjectIds.size(); }
    const QgsFields &fields() const { return mFields; }
    QgsRectangle extent() const { return mExtent; }
    QgsCoordinateReferenceSystem crs() const { return mSourceCRS; }

    bool getFeature( QgsFeatureId id, QgsFeature &f, bool fetchGeometry, const QList<int> &fetchAttributes, const QgsRectangle &filterRect = QgsRectangle() );

  private:
    friend class QgsAfsProvider;
    QgsDataSourceUri mDataSource;
    QgsRectangle mExtent;
    QgsWkbTypes::Type mGeometryType = QgsWkbTypes::Unknown;
    QgsFields mFields;
    QList<quint32> mObjectIds;
    QMap<QgsFeatureId, QgsFeature> mCache;
    QgsCoordinateReferenceSystem mSourceCRS;
};

#endif
