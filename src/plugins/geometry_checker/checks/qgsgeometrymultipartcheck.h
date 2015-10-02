/***************************************************************************
 *  qgsgeometrymultipartcheck.h                                            *
 *  -------------------                                                    *
 *  copyright            : (C) 2014 by Sandro Mani / Sourcepole AG         *
 *  email                : smani@sourcepole.ch                             *
 ***************************************************************************/

#ifndef QGS_GEOMETRY_MULTIPART_CHECK_H
#define QGS_GEOMETRY_MULTIPART_CHECK_H

#include "qgsgeometrycheck.h"

class QgsGeometryMultipartCheck : public QgsGeometryCheck
{
    Q_OBJECT

  public:
    explicit QgsGeometryMultipartCheck( QgsFeaturePool* featurePool )
        : QgsGeometryCheck( FeatureCheck, featurePool ) {}
    void collectErrors( QList<QgsGeometryCheckError*>& errors, QStringList &messages, QAtomicInt* progressCounter = 0, const QgsFeatureIds& ids = QgsFeatureIds() ) const override;
    void fixError( QgsGeometryCheckError* error, int method, int mergeAttributeIndex, Changes& changes ) const override;
    const QStringList& getResolutionMethods() const override;
    QString errorDescription() const override { return tr( "Multipart object with only one feature" ); }
    QString errorName() const override { return "QgsGeometryMultipartCheck"; }
  private:
    enum ResolutionMethod { ConvertToSingle, RemoveObject, NoChange };
};

#endif // QGS_GEOMETRY_MULTIPART_CHECK_H
