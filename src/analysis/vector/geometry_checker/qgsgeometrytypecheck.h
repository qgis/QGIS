/***************************************************************************
    qgsgeometrytypecheck.h
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

#ifndef QGS_GEOMETRY_TYPE_CHECK_H
#define QGS_GEOMETRY_TYPE_CHECK_H

#include "qgsgeometrycheck.h"

class ANALYSIS_EXPORT QgsGeometryTypeCheckError : public QgsGeometryCheckError
{
  public:
    QgsGeometryTypeCheckError( const QgsGeometryCheck *check,
                               const QgsGeometryCheckerUtils::LayerFeature &layerFeature,
                               const QgsPointXY &errorLocation,
                               QgsWkbTypes::Type flatType )
      : QgsGeometryCheckError( check, layerFeature, errorLocation )
    {
      mTypeName = QgsWkbTypes::displayString( flatType );
    }

    bool isEqual( QgsGeometryCheckError *other ) const override
    {
      return QgsGeometryCheckError::isEqual( other ) &&
             mTypeName == static_cast<QgsGeometryTypeCheckError *>( other )->mTypeName;
    }

    QString description() const override { return QStringLiteral( "%1 (%2)" ).arg( mCheck->errorDescription(), mTypeName ); }

  private:
    QString mTypeName;
};

class ANALYSIS_EXPORT QgsGeometryTypeCheck : public QgsGeometryCheck
{
    Q_OBJECT

  public:
    QgsGeometryTypeCheck( QgsGeometryCheckerContext *context, int allowedTypes )
      : QgsGeometryCheck( FeatureCheck, {QgsWkbTypes::PointGeometry, QgsWkbTypes::LineGeometry, QgsWkbTypes::PolygonGeometry}, context )
    , mAllowedTypes( allowedTypes )
    {}
    void collectErrors( QList<QgsGeometryCheckError *> &errors, QStringList &messages, QAtomicInt *progressCounter = nullptr, const QMap<QString, QgsFeatureIds> &ids = QMap<QString, QgsFeatureIds>() ) const override;
    void fixError( QgsGeometryCheckError *error, int method, const QMap<QString, int> &mergeAttributeIndices, Changes &changes ) const override;
    QStringList resolutionMethods() const override;
    QString errorDescription() const override { return tr( "Geometry type" ); }
    QString errorName() const override { return QStringLiteral( "QgsGeometryTypeCheck" ); }
  private:
    enum ResolutionMethod { Convert, Delete, NoChange };
    int mAllowedTypes;
};

#endif // QGS_GEOMETRY_TYPE_CHECK_H
