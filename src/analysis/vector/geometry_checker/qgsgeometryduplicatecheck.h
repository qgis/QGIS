/***************************************************************************
    qgsgeometryduplicatecheck.h
    ---------------------
    begin                : September 2015
    copyright            : (C) 2014 by Sandro Mani / Sourcepole AG
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

#ifndef QGS_GEOMETRY_DUPLICATE_CHECK_H
#define QGS_GEOMETRY_DUPLICATE_CHECK_H

#include "qgsgeometrycheck.h"

class ANALYSIS_EXPORT QgsGeometryDuplicateCheckError : public QgsGeometryCheckError
{
  public:
    QgsGeometryDuplicateCheckError( const QgsGeometryCheck *check,
                                    const QgsGeometryCheckerUtils::LayerFeature &layerFeature,
                                    const QgsPointXY &errorLocation,
                                    const QMap<QString, QList<QgsFeatureId>> &duplicates )
      : QgsGeometryCheckError( check, layerFeature, errorLocation, QgsVertexId(), duplicatesString( check->context()->featurePools, duplicates ) )
      , mDuplicates( duplicates )
    { }
    QMap<QString, QList<QgsFeatureId>> duplicates() const { return mDuplicates; }

    bool isEqual( QgsGeometryCheckError *other ) const override
    {
      return other->check() == check() &&
             other->layerId() == layerId() &&
             other->featureId() == featureId() &&
             // static_cast: since other->checker() == checker is only true if the types are actually the same
             static_cast<QgsGeometryDuplicateCheckError *>( other )->duplicates() == duplicates();
    }

  private:
    QMap<QString, QList<QgsFeatureId>> mDuplicates;

    static QString duplicatesString( const QMap<QString, QgsFeaturePool *> &featurePools, const QMap<QString, QList<QgsFeatureId>> &duplicates );
};

class ANALYSIS_EXPORT QgsGeometryDuplicateCheck : public QgsGeometryCheck
{
    Q_OBJECT

  public:
    explicit QgsGeometryDuplicateCheck( QgsGeometryCheckerContext *context )
      : QgsGeometryCheck( FeatureCheck, {QgsWkbTypes::PointGeometry, QgsWkbTypes::LineGeometry, QgsWkbTypes::PolygonGeometry}, context ) {}
    void collectErrors( QList<QgsGeometryCheckError *> &errors, QStringList &messages, QAtomicInt *progressCounter = nullptr, const QMap<QString, QgsFeatureIds> &ids = QMap<QString, QgsFeatureIds>() ) const override;
    void fixError( QgsGeometryCheckError *error, int method, const QMap<QString, int> &mergeAttributeIndices, Changes &changes ) const override;
    QStringList resolutionMethods() const override;
    QString errorDescription() const override { return tr( "Duplicate" ); }
    QString errorName() const override { return QStringLiteral( "QgsGeometryDuplicateCheck" ); }

    enum ResolutionMethod { NoChange, RemoveDuplicates };
};

#endif // QGS_GEOMETRY_DUPLICATE_CHECK_H
