/***************************************************************************
    qgsgeometrycontainedcheck.h
    ---------------------
    begin                : September 2015
    copyright            : (C) 2014 by Sandro Mani / Sourcepole AG
    email                : smani at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS_GEOMETRY_COVER_CHECK_H
#define QGS_GEOMETRY_COVER_CHECK_H

#include "qgsgeometrycheck.h"

class QgsGeometryContainedCheckError : public QgsGeometryCheckError
{
  public:
    QgsGeometryContainedCheckError( const QgsGeometryCheck* check,
                                    QgsFeatureId featureId,
                                    const QgsPointV2& errorLocation,
                                    QgsFeatureId otherId
                                  )
        : QgsGeometryCheckError( check, featureId, errorLocation, QgsVertexId(), otherId, ValueOther )
        , mOtherId( otherId )
    { }
    QgsFeatureId otherId() const { return mOtherId; }

    bool isEqual( QgsGeometryCheckError* other ) const override
    {
      return other->check() == check() &&
             other->featureId() == featureId() &&
             static_cast<QgsGeometryContainedCheckError*>( other )->otherId() == otherId();
    }

    virtual QString description() const override { return QApplication::translate( "QgsGeometryContainedCheckError", "Within %1" ).arg( otherId() ); }

  private:
    QgsFeatureId mOtherId;
};

class QgsGeometryContainedCheck : public QgsGeometryCheck
{
    Q_OBJECT

  public:
    explicit QgsGeometryContainedCheck( QgsFeaturePool* featurePool ) : QgsGeometryCheck( FeatureCheck, featurePool ) {}
    void collectErrors( QList<QgsGeometryCheckError*>& errors, QStringList& messages, QAtomicInt* progressCounter = nullptr, const QgsFeatureIds& ids = QgsFeatureIds() ) const override;
    void fixError( QgsGeometryCheckError* error, int method, int mergeAttributeIndex, Changes& changes ) const override;
    const QStringList& getResolutionMethods() const override;
    QString errorDescription() const override { return tr( "Within" ); }
    QString errorName() const override { return "QgsGeometryContainedCheck"; }
  private:
    enum ResolutionMethod { Delete, NoChange };
};

#endif // QGS_GEOMETRY_COVER_CHECK_H
