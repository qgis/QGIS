/***************************************************************************
    qgsgeometrylinelayerintersectioncheck.h
    ---------------------
    begin                : June 2017
    copyright            : (C) 2017 by Sandro Mani / Sourcepole AG
    email                : smani at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#define SIP_NO_FILE

#ifndef QGSGEOMETRYLINELAYERINTERSECTIONCHECK_H
#define QGSGEOMETRYLINELAYERINTERSECTIONCHECK_H

#include "qgsgeometrycheck.h"

/**
 * \ingroup analysis
 * \brief A line layer intersection check.
 */
class ANALYSIS_EXPORT QgsGeometryLineLayerIntersectionCheck : public QgsGeometryCheck
{
    Q_DECLARE_TR_FUNCTIONS( QgsGeometryLineLayerIntersectionCheck )
  public:
    QgsGeometryLineLayerIntersectionCheck( QgsGeometryCheckContext *context, const QVariantMap &configuration )
      : QgsGeometryCheck( context, configuration )
      , mCheckLayer( configurationValue<QString>( "checkLayer" ) )
    {}
    static QList<Qgis::GeometryType> factoryCompatibleGeometryTypes() { return { Qgis::GeometryType::Line }; }
    static bool factoryIsCompatible( QgsVectorLayer *layer ) SIP_SKIP { return factoryCompatibleGeometryTypes().contains( layer->geometryType() ); }
    QList<Qgis::GeometryType> compatibleGeometryTypes() const override { return factoryCompatibleGeometryTypes(); }
    void collectErrors( const QMap<QString, QgsFeaturePool *> &featurePools, QList<QgsGeometryCheckError *> &errors, QStringList &messages, QgsFeedback *feedback, const LayerFeatureIds &ids = LayerFeatureIds() ) const override;
    void fixError( const QMap<QString, QgsFeaturePool *> &featurePools, QgsGeometryCheckError *error, int method, const QMap<QString, int> &mergeAttributeIndices, Changes &changes ) const override;
    Q_DECL_DEPRECATED QStringList resolutionMethods() const override;
    static QString factoryDescription() { return tr( "Intersection" ); }
    QString description() const override { return factoryDescription(); }
    static QString factoryId() { return QStringLiteral( "QgsGeometryLineLayerIntersectionCheck" ); }
    QString id() const override { return factoryId(); }
    QgsGeometryCheck::CheckType checkType() const override { return factoryCheckType(); }
    static QgsGeometryCheck::CheckType factoryCheckType() SIP_SKIP;

    enum ResolutionMethod
    {
      NoChange
    };

  private:
    QString mCheckLayer;
};

#endif // QGSGEOMETRYLINELAYERINTERSECTIONCHECK_H
