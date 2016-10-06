/***************************************************************************
    qgsgeometryanglecheck.h
    ---------------------
    begin                : September 2014
    copyright            : (C) 2015 by Sandro Mani / Sourcepole AG
    email                : smani at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS_GEOMETRY_ANGLE_CHECK_H
#define QGS_GEOMETRY_ANGLE_CHECK_H

#include "qgsgeometrycheck.h"

class QgsGeometryAngleCheck : public QgsGeometryCheck
{
    Q_OBJECT

  public:
    QgsGeometryAngleCheck( QgsFeaturePool* featurePool, double minAngle )
        : QgsGeometryCheck( FeatureNodeCheck, featurePool )
        , mMinAngle( minAngle )
    {}
    void collectErrors( QList<QgsGeometryCheckError*>& errors, QStringList& messages, QAtomicInt* progressCounter = nullptr, const QgsFeatureIds& ids = QgsFeatureIds() ) const override;
    void fixError( QgsGeometryCheckError* error, int method, int mergeAttributeIndex, Changes& changes ) const override;
    const QStringList& getResolutionMethods() const override;
    QString errorDescription() const override { return tr( "Minimal angle" ); }
    QString errorName() const override { return "QgsGeometryAngleCheck"; }
  private:
    enum ResolutionMethod { DeleteNode, NoChange };
    double mMinAngle;
};

#endif // QGS_GEOMETRY_ANGLE_CHECK_H
