/***************************************************************************
 *  qgsgeometrysegmentlengthcheck.h                                        *
 *  -------------------                                                    *
 *  copyright            : (C) 2014 by Sandro Mani / Sourcepole AG         *
 *  email                : smani@sourcepole.ch                             *
 ***************************************************************************/

#ifndef QGS_GEOMETRY_SEGMENTLENGTH_CHECK_H
#define QGS_GEOMETRY_SEGMENTLENGTH_CHECK_H

#include "qgsgeometrycheck.h"

class QgsGeometrySegmentLengthCheck : public QgsGeometryCheck
{
    Q_OBJECT

  public:
    QgsGeometrySegmentLengthCheck( QgsFeaturePool* featurePool, double minLength )
        : QgsGeometryCheck( FeatureNodeCheck, featurePool ), mMinLength( minLength ) {}
    void collectErrors( QList<QgsGeometryCheckError*>& errors, QStringList &messages, QAtomicInt* progressCounter = 0, const QgsFeatureIds& ids = QgsFeatureIds() ) const;
    void fixError( QgsGeometryCheckError* error, int method, int mergeAttributeIndex, Changes& changes ) const;
    const QStringList& getResolutionMethods() const;
    QString errorDescription() const { return tr( "Minimal segment length" ); }
    QString errorName() const { return "QgsGeometrySegmentLengthCheck"; }
  private:
    enum ResolutionMethod { NoChange };
    double mMinLength;
};

#endif // QGS_GEOMETRY_SEGMENTLENGTH_CHECK_H
