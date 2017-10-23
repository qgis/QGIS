/***************************************************************************
 *  qgsgeometryselfcontactcheck.h                                          *
 *  -------------------                                                    *
 *  copyright            : (C) 2017 by Sandro Mani / Sourcepole AG         *
 *  email                : smani@sourcepole.ch                             *
 ***************************************************************************/

#define SIP_NO_FILE

#ifndef QGS_GEOMETRY_SELFCONTACT_CHECK_H
#define QGS_GEOMETRY_SELFCONTACT_CHECK_H

#include "qgsgeometrycheck.h"

class ANALYSIS_EXPORT QgsGeometrySelfContactCheck : public QgsGeometryCheck
{
    Q_OBJECT

  public:
    QgsGeometrySelfContactCheck( QgsGeometryCheckerContext *context )
      : QgsGeometryCheck( FeatureNodeCheck, {QgsWkbTypes::LineGeometry, QgsWkbTypes::PolygonGeometry}, context ) {}
    void collectErrors( QList<QgsGeometryCheckError *> &errors, QStringList &messages, QAtomicInt *progressCounter = 0, const QMap<QString, QgsFeatureIds> &ids = QMap<QString, QgsFeatureIds>() ) const;
    void fixError( QgsGeometryCheckError *error, int method, const QMap<QString, int> &mergeAttributeIndices, Changes & ) const;
    QStringList getResolutionMethods() const;
    QString errorDescription() const { return tr( "Self contact" ); }
    QString errorName() const { return QStringLiteral( "QgsGeometrySelfContactCheck" ); }

    enum ResolutionMethod { NoChange };
};

#endif // QGS_GEOMETRY_SELFCONTACT_CHECK_H
