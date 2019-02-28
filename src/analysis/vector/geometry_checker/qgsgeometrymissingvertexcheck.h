/***************************************************************************
    qgsgeometrymissingvertexcheck.h
    ---------------------
    begin                : September 2018
    copyright            : (C) 2018 Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#define SIP_NO_FILE

#ifndef QGSGEOMETRYMISSINGVERTEXCHECK_H
#define QGSGEOMETRYMISSINGVERTEXCHECK_H

#include "qgsgeometrycheck.h"
#include "qgsgeometrycheckerror.h"

class QgsCurvePolygon;

/**
 * \ingroup analysis
 *
 * A geometry check error for a missing vertex.
 * Includes additional details about the bounding box of the error,
 * centered on the missing error location and scaled by taking neighbouring
 * vertices into account.
 *
 * \since QGIS 3.8
 */
class ANALYSIS_EXPORT QgsGeometryMissingVertexCheckError : public QgsGeometryCheckError
{
  public:

    /**
     * Create a new missing vertex check error.
     */
    QgsGeometryMissingVertexCheckError( const QgsGeometryCheck *check,
                                        const QgsGeometryCheckerUtils::LayerFeature &layerFeature,
                                        const QgsPointXY &errorLocation,
                                        QgsVertexId vidx = QgsVertexId(),
                                        const QVariant &value = QVariant(),
                                        ValueType valueType = ValueOther );

    QgsRectangle affectedAreaBBox() const override;

    /**
     * Set the bounding box of the affected area.
     *
     * \since QGIS 3.8
     */
    void setAffectedAreaBBox( const QgsRectangle &affectedAreaBBox );

    QMap<QString, QgsFeatureIds> involvedFeatures() const override;

    /**
     * The two involved features, that share a common boundary but not all common
     * vertices on this boundary.
     *
     * \since QGIS 3.8
     */
    void setInvolvedFeatures( const QMap<QString, QgsFeatureIds> &involvedFeatures );

    QIcon icon() const override;

  private:
    QgsRectangle mAffectedAreaBBox;
    QMap<QString, QgsFeatureIds> mInvolvedFeatures;
};

/**
 * \ingroup analysis
 *
 * A topology check for missing vertices.
 * Any vertex which is on the border of another polygon but no corresponding vertex
 * can be found on the other polygon will be reported as an error.
 *
 * \since QGIS 3.4
 */
class ANALYSIS_EXPORT QgsGeometryMissingVertexCheck : public QgsGeometryCheck
{
    Q_GADGET

  public:

    /**
     * The available resolutions for missing vertex check.
     */
    enum ResolutionMethod
    {
      NoChange, //!< Do nothing
      AddMissingVertex //!< Add the missing vertex
    };
    Q_ENUM( ResolutionMethod )

    /**
     * Creates a new missing vertex geometry check with \a context and the provided \a geometryCheckConfiguration.
     */
    explicit QgsGeometryMissingVertexCheck( const QgsGeometryCheckContext *context, const QVariantMap &geometryCheckConfiguration );
    void collectErrors( const QMap<QString, QgsFeaturePool *> &featurePools, QList<QgsGeometryCheckError *> &errors, QStringList &messages, QgsFeedback *feedback, const LayerFeatureIds &ids = LayerFeatureIds() ) const override;
    void fixError( const QMap<QString, QgsFeaturePool *> &featurePools, QgsGeometryCheckError *error, int method, const QMap<QString, int> &mergeAttributeIndices, Changes &changes ) const override;
    QStringList resolutionMethods() const override;

    QString description() const override;
    QString id() const override;
    QList<QgsWkbTypes::GeometryType> compatibleGeometryTypes() const override;
    QgsGeometryCheck::Flags flags() const override;
    QgsGeometryCheck::CheckType checkType() const override;

///@cond private
    static QList<QgsWkbTypes::GeometryType> factoryCompatibleGeometryTypes() SIP_SKIP;
    static bool factoryIsCompatible( QgsVectorLayer *layer ) SIP_SKIP;
    static QString factoryDescription() SIP_SKIP;
    static QString factoryId() SIP_SKIP;
    static QgsGeometryCheck::Flags factoryFlags() SIP_SKIP;
    static QgsGeometryCheck::CheckType factoryCheckType() SIP_SKIP;
///@endcond

  private:
    void processPolygon( const QgsCurvePolygon *polygon, QgsFeaturePool *featurePool, QList<QgsGeometryCheckError *> &errors, const QgsGeometryCheckerUtils::LayerFeature &layerFeature, QgsFeedback *feedback ) const;

    QgsRectangle contextBoundingBox( const QgsCurvePolygon *polygon, const QgsVertexId &vertexId, const QgsPoint &point ) const;
};



#endif // QGSGEOMETRYMISSINGVERTEXCHECK_H
