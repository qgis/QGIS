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

/**
 * \ingroup analysis
 * This represents an error reported by a geometry check.
 *
 * \note This class is a technology preview and unstable API.
 * \since QGIS 3.4
 */
class ANALYSIS_EXPORT QgsGeometryCheckError
{
  public:

    /**
     * The status of an error.
     */
    enum Status
    {
      StatusPending, //!< The error is detected and pending to be handled
      StatusFixFailed, //!< A fix has been tried on the error but failed
      StatusFixed, //!< The error is fixed
      StatusObsolete //!< The error is obsolete because of other modifications
    };

    /**
     * Describes the type of an error value.
     */
    enum ValueType
    {
      ValueLength, //!< The value is a length
      ValueArea, //!< The value is an area
      ValueOther //!< The value if of another type
    };

    /**
     * Create a new geometry check error with the parent \a check and for the
     * \a layerFeature pair at the \a errorLocation. Optionally the vertex can be
     * specified via \a vixd and a \a value with its \a value Type for
     * additional information.
     */
    QgsGeometryCheckError( const QgsGeometryCheck *check,
                           const QgsGeometryCheckerUtils::LayerFeature &layerFeature,
                           const QgsPointXY &errorLocation,
                           QgsVertexId vidx = QgsVertexId(),
                           const QVariant &value = QVariant(),
                           ValueType valueType = ValueOther );

    virtual ~QgsGeometryCheckError() = default;

    const QgsGeometryCheckError &operator=( const QgsGeometryCheckError & ) = delete;

    /**
     * The geometry check that created this error.
     */
    const QgsGeometryCheck *check() const { return mCheck; }

    /**
     * The id of the layer on which this error has been detected.
     */
    const QString &layerId() const { return mLayerId; }

    /**
     * The id of the feature on which this error has been detected.
     */
    QgsFeatureId featureId() const { return mFeatureId; }

    /**
     * The geometry of the error in map units.
     */
    QgsGeometry geometry() const;

    /**
     * The bounding box of the affected area of the error.
     */
    virtual QgsRectangle affectedAreaBBox() const;

    /**
     * The error description. By default the description of the parent check
     * will be returned.
     */
    virtual QString description() const { return mCheck->description(); }

    /**
     * The location of the error in map units.
     */
    const QgsPointXY &location() const { return mErrorLocation; }

    /**
     * An additional value for the error.
     * Lengths and areas are provided in map units.
     * \see valueType()
     */
    QVariant value() const { return mValue; }

    /**
     * The type of the value.
     * \see value()
     */
    ValueType valueType() const { return mValueType; }

    /**
     * The id of the affected vertex. May be valid or not, depending on the
     * check.
     */
    const QgsVertexId &vidx() const { return mVidx; }

    /**
     * The status of the error.
     */
    Status status() const { return mStatus; }

    /**
     * A message with details, how the error has been resolved.
     */
    QString resolutionMessage() const { return mResolutionMessage; }

    /**
     * Set the status to fixed and specify the \a method that has been used to
     * fix the error.
     */
    void setFixed( int method );

    /**
     * Set the error status to failed and specify the \a reason for failure.
     */
    void setFixFailed( const QString &reason );

    /**
     * Set the error status to obsolete.
     */
    void setObsolete() { mStatus = StatusObsolete; }

    /**
     * Check if this error is equal to \a other.
     * Is reimplemented by subclasses with additional information, comparison
     * of base information is done in parent class.
     */
    virtual bool isEqual( QgsGeometryCheckError *other ) const;

    /**
     * Check if this error is almost equal to \a other.
     * If this returns TRUE, it can be used to update existing errors after re-checking.
     */
    virtual bool closeMatch( QgsGeometryCheckError * /*other*/ ) const;

    /**
     * Update this error with the information from \a other.
     * Will be used to update existing errors whenever they are re-checked.
     */
    virtual void update( const QgsGeometryCheckError *other );

    /**
     * Apply a list of \a changes.
     */
    virtual bool handleChanges( const QgsGeometryCheck::Changes &changes ) SIP_SKIP;

    /**
     * Returns a list of involved features.
     * By default returns an empty map.
     * The map keys are layer ids, the map value is a set of feature ids.
     *
     * \since QGIS 3.8
     */
    virtual QMap<QString, QgsFeatureIds > involvedFeatures() const SIP_SKIP;

    /**
     * Returns an icon that should be shown for this kind of error.
     *
     * \since QGIS 3.8
     */
    virtual QIcon icon() const;
  protected:

    /**
     * Create a new geometry check error with the parent \a check and for the
     * layer with \a layerId and \a featureId.
     * The \a geometry of the error and the \a errorLocation need to be
     * specified in map coordinates.
     * Optionally the vertex can be specified via \a vixd and a \a value with
     * its \a value Type for additional information.
     */
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
