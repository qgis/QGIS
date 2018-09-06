/***************************************************************************
                         qgsgeometrycheckerror.h
                         --------
    begin                : September 2018
    copyright            : (C) 2018 by Denis Rouzaud
    email                : denis@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGEOMETRYCHECKERROR_H
#define QGSGEOMETRYCHECKERROR_H

#include "qgis_analysis.h"

#include "qgsgeometrycheck.h"
#include "qgsgeometrycheckerutils.h"

class QgsPointXY;


class ANALYSIS_EXPORT QgsGeometryCheckError
{
  public:
    enum Status { StatusPending, StatusFixFailed, StatusFixed, StatusObsolete };
    enum ValueType { ValueLength, ValueArea, ValueOther };

    QgsGeometryCheckError( const QgsGeometryCheck *check,
                           const QgsGeometryCheckerUtils::LayerFeature &layerFeature,
                           const QgsPointXY &errorLocation,
                           QgsVertexId vidx = QgsVertexId(),
                           const QVariant &value = QVariant(),
                           ValueType valueType = ValueOther );

    virtual ~QgsGeometryCheckError() = default;

    const QgsGeometryCheckError &operator=( const QgsGeometryCheckError & ) = delete;

    const QgsGeometryCheck *check() const { return mCheck; }
    const QString &layerId() const { return mLayerId; }
    QgsFeatureId featureId() const { return mFeatureId; }
    // In map units
    const QgsAbstractGeometry *geometry() const;
    // In map units
    virtual QgsRectangle affectedAreaBBox() const;
    virtual QString description() const { return mCheck->description(); }
    // In map units
    const QgsPointXY &location() const { return mErrorLocation; }
    // Lengths, areas in map units
    QVariant value() const { return mValue; }
    ValueType valueType() const { return mValueType; }
    const QgsVertexId &vidx() const { return mVidx; }
    Status status() const { return mStatus; }
    QString resolutionMessage() const { return mResolutionMessage; }
    void setFixed( int method );
    void setFixFailed( const QString &reason );
    void setObsolete() { mStatus = StatusObsolete; }

    /**
     * Check if this error is equal to \a other.
     * Is reimplemented by subclasses with additional information, comparison
     * of base information is done in parent class.
     */
    virtual bool isEqual( QgsGeometryCheckError *other ) const;

    /**
     * Check if this error is almost equal to \a other.
     * If this returns true, it can be used to update existing errors after re-checking.
     */
    virtual bool closeMatch( QgsGeometryCheckError * /*other*/ ) const;

    /**
     * Update this error with the information from \other.
     * Will be used to update existing errors whenever they are re-checked.
     */
    virtual void update( const QgsGeometryCheckError *other );

    /**
     * Apply a list of \a changes.
     */
    virtual bool handleChanges( const QgsGeometryCheck::Changes &changes ) SIP_SKIP;

  protected:
    // Users of this constructor must ensure geometry and errorLocation are in map coordinates
    QgsGeometryCheckError( const QgsGeometryCheck *check,
                           const QString &layerId,
                           QgsFeatureId featureId,
                           const QgsGeometry &geometry,
                           const QgsPointXY &errorLocation,
                           QgsVertexId vidx = QgsVertexId(),
                           const QVariant &value = QVariant(),
                           ValueType valueType = ValueOther );

    const QgsGeometryCheck *mCheck = nullptr;
    QString mLayerId;
    QgsFeatureId mFeatureId;
    QgsGeometry mGeometry;
    QgsPointXY mErrorLocation;
    QgsVertexId mVidx;
    QVariant mValue;
    ValueType mValueType;
    Status mStatus;
    QString mResolutionMessage;

};

Q_DECLARE_METATYPE( QgsGeometryCheckError * )

#endif // QGSGEOMETRYCHECKERROR_H
