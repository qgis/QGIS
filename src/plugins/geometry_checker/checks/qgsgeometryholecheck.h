/***************************************************************************
 *  qgsgeometryholecheck.h                                                 *
 *  -------------------                                                    *
 *  copyright            : (C) 2014 by Sandro Mani / Sourcepole AG         *
 *  email                : smani@sourcepole.ch                             *
 ***************************************************************************/

#ifndef QGS_GEOMETRY_HOLE_CHECK_H
#define QGS_GEOMETRY_HOLE_CHECK_H

#include "qgsgeometrycheck.h"

class QgsGeometryHoleCheck : public QgsGeometryCheck
{
    Q_OBJECT

  public:
    QgsGeometryHoleCheck( QgsFeaturePool* featurePool )
        : QgsGeometryCheck( FeatureCheck, featurePool ) {}
    void collectErrors( QList<QgsGeometryCheckError*>& errors, QStringList &messages, QAtomicInt* progressCounter = 0, const QgsFeatureIds& ids = QgsFeatureIds() ) const;
    void fixError( QgsGeometryCheckError* error, int method, int mergeAttributeIndex, Changes& changes ) const;
    const QStringList& getResolutionMethods() const;
    QString errorDescription() const { return tr( "Polygon with hole" ); }
    QString errorName() const { return "QgsGeometryHoleCheck"; }
  private:
    enum ResolutionMethod { RemoveHoles, NoChange };
};

#endif // QGS_GEOMETRY_HOLE_CHECK_H
