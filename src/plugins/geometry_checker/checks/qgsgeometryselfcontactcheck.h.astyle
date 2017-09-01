/***************************************************************************
 *  qgsgeometryselfcontactcheck.h                                          *
 *  -------------------                                                    *
 *  copyright            : (C) 2017 by Sandro Mani / Sourcepole AG         *
 *  email                : smani@sourcepole.ch                             *
 ***************************************************************************/

#ifndef QGS_GEOMETRY_SELFCONTACT_CHECK_H
#define QGS_GEOMETRY_SELFCONTACT_CHECK_H

#include "qgsgeometrycheck.h"

class QgsGeometrySelfContactCheck : public QgsGeometryCheck
{
    Q_OBJECT

  public:
    QgsGeometrySelfContactCheck( QgsFeaturePool *featurePool )
      : QgsGeometryCheck( FeatureNodeCheck, featurePool ) {}
    void collectErrors( QList<QgsGeometryCheckError *> &errors, QStringList &messages, QAtomicInt *progressCounter = 0, const QgsFeatureIds &ids = QgsFeatureIds() ) const;
    void fixError( QgsGeometryCheckError *error, int method, int, Changes & ) const;
    QStringList getResolutionMethods() const;
    QString errorDescription() const { return tr( "Self contact" ); }
    QString errorName() const { return "QgsGeometrySelfContactCheck"; }
  private:
    enum ResolutionMethod { NoChange };
};

#endif // QGS_GEOMETRY_SELFCONTACT_CHECK_H
