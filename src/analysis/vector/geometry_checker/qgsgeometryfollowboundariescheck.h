/***************************************************************************
    qgsgeometryfollowboundariescheck.h
    ---------------------
    begin                : September 2017
    copyright            : (C) 2017 by Sandro Mani / Sourcepole AG
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

#ifndef QGSGEOMETRYFOLLOWBOUNDARIESCHECK_H
#define QGSGEOMETRYFOLLOWBOUNDARIESCHECK_H

#include "qgsgeometrycheck.h"

class QgsSpatialIndex;


class ANALYSIS_EXPORT QgsGeometryFollowBoundariesCheck : public QgsGeometryCheck
{
    Q_OBJECT

  public:
    QgsGeometryFollowBoundariesCheck( QgsGeometryCheckerContext *context, QgsVectorLayer *checkLayer );
    ~QgsGeometryFollowBoundariesCheck() override;
    void collectErrors( QList<QgsGeometryCheckError *> &errors, QStringList &messages, QAtomicInt *progressCounter = nullptr, const QMap<QString, QgsFeatureIds> &ids = QMap<QString, QgsFeatureIds>() ) const override;
    void fixError( QgsGeometryCheckError *error, int method, const QMap<QString, int> &mergeAttributeIndices, Changes &changes ) const override;
    QStringList resolutionMethods() const override;
    QString errorDescription() const override { return tr( "Polygon does not follow boundaries" ); }
    QString errorName() const override { return QStringLiteral( "QgsGeometryFollowBoundariesCheck" ); }
  private:
    enum ResolutionMethod { NoChange };
    QgsVectorLayer *mCheckLayer;
    QgsSpatialIndex *mIndex = nullptr;
};

#endif // QGSGEOMETRYFOLLOWBOUNDARIESCHECK_H
