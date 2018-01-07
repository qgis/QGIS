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
    void collectErrors( QList<QgsGeometryCheckError *> &errors, QStringList &messages, QAtomicInt *progressCounter = nullptr, const QMap<QString, QgsFeatureIds> &ids = QMap<QString, QgsFeatureIds>() ) const override;
    void fixError( QgsGeometryCheckError *error, int method, const QMap<QString, int> &mergeAttributeIndices, Changes & ) const override;
    QStringList getResolutionMethods() const override;
    QString errorDescription() const override { return tr( "Self contact" ); }
    QString errorName() const override { return QStringLiteral( "QgsGeometrySelfContactCheck" ); }

    enum ResolutionMethod { NoChange };
};

#endif // QGS_GEOMETRY_SELFCONTACT_CHECK_H
