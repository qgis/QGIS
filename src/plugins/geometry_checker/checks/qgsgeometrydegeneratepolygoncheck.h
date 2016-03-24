/***************************************************************************
 *  qgsgeometrydegeneratepolygoncheck.h                                    *
 *  -------------------                                                    *
 *  copyright            : (C) 2014 by Sandro Mani / Sourcepole AG         *
 *  email                : smani@sourcepole.ch                             *
 ***************************************************************************/

#ifndef QGS_GEOMETRY_DEGENERATEPOLYGON_CHECK_H
#define QGS_GEOMETRY_DEGENERATEPOLYGON_CHECK_H

#include "qgsgeometrycheck.h"

class QgsGeometryDegeneratePolygonCheck : public QgsGeometryCheck
{
    Q_OBJECT

  public:
    explicit QgsGeometryDegeneratePolygonCheck( QgsFeaturePool* featurePool )
        : QgsGeometryCheck( FeatureNodeCheck, featurePool ) {}
    void collectErrors( QList<QgsGeometryCheckError*>& errors, QStringList &messages, QAtomicInt* progressCounter = 0, const QgsFeatureIds& ids = QgsFeatureIds() ) const override;
    void fixError( QgsGeometryCheckError* error, int method, int mergeAttributeIndex, Changes& changes ) const override;
    const QStringList& getResolutionMethods() const override;
    QString errorDescription() const override { return tr( "Polygon with less than three nodes" ); }
    QString errorName() const override { return "QgsGeometryDegeneratePolygonCheck"; }

  private:
    enum ResolutionMethod { DeleteRing, NoChange };
};

#endif // QGS_GEOMETRY_DEGENERATEPOLYGON_CHECK_H
