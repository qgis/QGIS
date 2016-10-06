/***************************************************************************
    qgsgeometryduplicatecheck.h
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

#ifndef QGS_GEOMETRY_DUPLICATE_CHECK_H
#define QGS_GEOMETRY_DUPLICATE_CHECK_H

#include "qgsgeometrycheck.h"

class QgsGeometryDuplicateCheckError : public QgsGeometryCheckError
{
  public:
    QgsGeometryDuplicateCheckError( const QgsGeometryCheck* check,
                                    QgsFeatureId featureId,
                                    const QgsPointV2& errorLocation,
                                    const QList<QgsFeatureId>& duplicates )
        : QgsGeometryCheckError( check, featureId, errorLocation, QgsVertexId(), duplicatesString( duplicates ) )
        , mDuplicates( duplicates )
    { }
    const QList<QgsFeatureId>& duplicates() const { return mDuplicates; }

    bool isEqual( QgsGeometryCheckError* other ) const override
    {
      return other->check() == check() &&
             other->featureId() == featureId() &&
             // static_cast: since other->checker() == checker is only true if the types are actually the same
             static_cast<QgsGeometryDuplicateCheckError*>( other )->duplicates() == duplicates();
    }

  private:
    QList<QgsFeatureId> mDuplicates;

    static inline QString duplicatesString( const QList<QgsFeatureId>& duplicates )
    {
      QStringList str;
      Q_FOREACH ( QgsFeatureId id, duplicates )
      {
        str.append( QString::number( id ) );
      }
      return str.join( ", " );
    }
};

class QgsGeometryDuplicateCheck : public QgsGeometryCheck
{
    Q_OBJECT

  public:
    explicit QgsGeometryDuplicateCheck( QgsFeaturePool* featurePool )
        : QgsGeometryCheck( FeatureCheck, featurePool ) {}
    void collectErrors( QList<QgsGeometryCheckError*>& errors, QStringList &messages, QAtomicInt* progressCounter = nullptr, const QgsFeatureIds& ids = QgsFeatureIds() ) const override;
    void fixError( QgsGeometryCheckError* error, int method, int mergeAttributeIndex, Changes& changes ) const override;
    const QStringList& getResolutionMethods() const override;
    QString errorDescription() const override { return tr( "Duplicate" ); }
    QString errorName() const override { return "QgsGeometryDuplicateCheck"; }

  private:
    enum ResolutionMethod { NoChange, RemoveDuplicates };
};

#endif // QGS_GEOMETRY_DUPLICATE_CHECK_H
