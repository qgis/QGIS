/***************************************************************************
    qgsgeometrytypecheck.h
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

#ifndef QGS_GEOMETRY_TYPE_CHECK_H
#define QGS_GEOMETRY_TYPE_CHECK_H

#include "qgsgeometrycheck.h"

class QgsGeometryTypeCheckError : public QgsGeometryCheckError
{
  public:
    QgsGeometryTypeCheckError( const QgsGeometryCheck* check,
                               QgsFeatureId featureId,
                               const QgsPointV2& errorLocation,
                               QgsWKBTypes::Type flatType )
        : QgsGeometryCheckError( check, featureId, errorLocation )
    {
      mTypeName = QgsWKBTypes::displayString( flatType );
    }

    bool isEqual( QgsGeometryCheckError* other ) const override
    {
      return QgsGeometryCheckError::isEqual( other ) &&
             mTypeName == static_cast<QgsGeometryTypeCheckError*>( other )->mTypeName;
    }

    virtual QString description() const override { return QString( "%1 (%2)" ).arg( mCheck->errorDescription(), mTypeName ); }

  private:
    QString mTypeName;
};

class QgsGeometryTypeCheck : public QgsGeometryCheck
{
    Q_OBJECT

  public:
    QgsGeometryTypeCheck( QgsFeaturePool* featurePool, int allowedTypes )
        : QgsGeometryCheck( FeatureCheck, featurePool )
        , mAllowedTypes( allowedTypes )
    {}
    void collectErrors( QList<QgsGeometryCheckError*>& errors, QStringList &messages, QAtomicInt* progressCounter = nullptr, const QgsFeatureIds& ids = QgsFeatureIds() ) const override;
    void fixError( QgsGeometryCheckError* error, int method, int mergeAttributeIndex, Changes& changes ) const override;
    const QStringList& getResolutionMethods() const override;
    QString errorDescription() const override { return tr( "Geometry type" ); }
    QString errorName() const override { return "QgsGeometryTypeCheck"; }
  private:
    enum ResolutionMethod { Convert, Delete, NoChange };
    int mAllowedTypes;
};

#endif // QGS_GEOMETRY_TYPE_CHECK_H
