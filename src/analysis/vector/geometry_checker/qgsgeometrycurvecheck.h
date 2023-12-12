/***************************************************************************
    qgsgeometrycurvecheck.h
    ---------------------
    begin                : December 2023
    copyright            : (C) 2023 by Loïc Bartoletti (Oslandia)
    email                : loic dot bartoletti at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#define SIP_NO_FILE

#ifndef QGS_GEOMETRY_CURVE_CHECK_H
#define QGS_GEOMETRY_CURVE_CHECK_H

#include "qgsgeometrycheck.h"

/**
 * \ingroup analysis
 * \brief A curve check.
 *
 * \since QGIS 3.36
 */
class ANALYSIS_EXPORT QgsGeometryCurveCheck : public QgsGeometryCheck
{
    Q_DECLARE_TR_FUNCTIONS( QgsGeometryCurveCheck )
  public:

    /**
     * Creates a new has curve check with the provided \a context. No options are supported in \a configuration.
     */
    explicit QgsGeometryCurveCheck( QgsGeometryCheckContext *context, const QVariantMap &configuration )
      : QgsGeometryCheck( context, configuration ) {}
    QList<Qgis::GeometryType> compatibleGeometryTypes() const override { return factoryCompatibleGeometryTypes(); }
    void collectErrors( const QMap<QString, QgsFeaturePool *> &featurePools, QList<QgsGeometryCheckError *> &errors, QStringList &messages, QgsFeedback *feedback, const LayerFeatureIds &ids = LayerFeatureIds() ) const override;
    void fixError( const QMap<QString, QgsFeaturePool *> &featurePools, QgsGeometryCheckError *error, int method, const QMap<QString, int> &mergeAttributeIndices, Changes &changes ) const override;
    Q_DECL_DEPRECATED QStringList resolutionMethods() const override;
    QString description() const override { return factoryDescription(); }
    QString id() const override { return factoryId(); }
    QgsGeometryCheck::CheckType checkType() const override { return factoryCheckType(); }

///@cond private
    static bool factoryIsCompatible( QgsVectorLayer *layer ) SIP_SKIP { return factoryCompatibleGeometryTypes().contains( layer->geometryType() ); }
    static QList<Qgis::GeometryType> factoryCompatibleGeometryTypes() {return {Qgis::GeometryType::Polygon, Qgis::GeometryType::Line}; }
    static QString factoryDescription() { return tr( "Geometry with curve" ); }
    static QString factoryId() { return QStringLiteral( "QgsGeometryCurveCheck" ); }
    static QgsGeometryCheck::CheckType factoryCheckType() { return QgsGeometryCheck::FeatureCheck; }
///@endcond

    /**
     * The available resolutions for has curve check.
     */
    enum ResolutionMethod
    {
      StraightCurves, //!< Convert to straight segments
      Delete, //!< Delete feature
      NoChange //!< Do nothing
    };
    Q_ENUM( ResolutionMethod )
};

#endif // QGS_GEOMETRY_CURVE_CHECK_H

