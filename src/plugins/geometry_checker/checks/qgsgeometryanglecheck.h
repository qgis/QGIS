/***************************************************************************
 *  qgsgeometryanglecheck.h                                                *
 *  -------------------                                                    *
 *  copyright            : (C) 2014 by Sandro Mani / Sourcepole AG         *
 *  email                : smani@sourcepole.ch                             *
 ***************************************************************************/

#ifndef QGS_GEOMETRY_ANGLE_CHECK_H
#define QGS_GEOMETRY_ANGLE_CHECK_H

#include "qgsgeometrycheck.h"

class QgsGeometryAngleCheck : public QgsGeometryCheck
{
    Q_OBJECT

  public:
    QgsGeometryAngleCheck( QgsFeaturePool* featurePool, double minAngle )
        : QgsGeometryCheck( FeatureNodeCheck, featurePool ), mMinAngle( minAngle ) {}
    void collectErrors( QList<QgsGeometryCheckError*>& errors, QStringList& messages, QAtomicInt* progressCounter = 0, const QgsFeatureIds& ids = QgsFeatureIds() ) const;
    void fixError( QgsGeometryCheckError* error, int method, int mergeAttributeIndex, Changes& changes ) const;
    const QStringList& getResolutionMethods() const;
    QString errorDescription() const { return tr( "Minimal angle" ); }
    QString errorName() const { return "QgsGeometryAngleCheck"; }
  private:
    enum ResolutionMethod { DeleteNode, NoChange };
    double mMinAngle;
};

#endif // QGS_GEOMETRY_ANGLE_CHECK_H
