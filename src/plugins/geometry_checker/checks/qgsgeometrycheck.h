/***************************************************************************
    qgsgeometrycheck.h
    ---------------------
    begin                : September 2014
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

#ifndef QGS_GEOMETRY_CHECK_H
#define QGS_GEOMETRY_CHECK_H

#include <limits>
#include <QStringList>
#include "qgsfeature.h"
#include "qgsvectorlayer.h"
#include "geometry/qgsgeometry.h"
#include "../utils/qgsgeometrycheckerutils.h"
#include "geos_c.h"
#include <QApplication>

class QgsGeometryCheckError;
class QgsFeaturePool;

#define FEATUREID_NULL std::numeric_limits<QgsFeatureId>::min()

struct QgsGeometryCheckerContext
{
  QgsGeometryCheckerContext( int _precision, const QString &_crs, const QMap<QString, QgsFeaturePool *> &_featurePools );
  const double tolerance;
  const double reducedTolerance;
  const QString crs;
  const QMap<QString, QgsFeaturePool *> featurePools;
};

class QgsGeometryCheck : public QObject
{
    Q_OBJECT

  public:
    enum ChangeWhat { ChangeFeature, ChangePart, ChangeRing, ChangeNode };
    enum ChangeType { ChangeAdded, ChangeRemoved, ChangeChanged };
    enum CheckType { FeatureNodeCheck, FeatureCheck, LayerCheck };

    struct Change
    {
      Change() = default;
      Change( ChangeWhat _what, ChangeType _type, QgsVertexId _vidx = QgsVertexId() )
        : what( _what )
        , type( _type )
        , vidx( _vidx )
      {}
      ChangeWhat what;
      ChangeType type;
      QgsVertexId vidx;
    };

    typedef QMap<QString, QMap<QgsFeatureId, QList<Change>>> Changes;

    QgsGeometryCheck( CheckType checkType, const QList<QgsWkbTypes::GeometryType> &compatibleGeometryTypes, QgsGeometryCheckerContext *context )
      : mCheckType( checkType )
      , mCompatibleGeometryTypes( compatibleGeometryTypes )
      , mContext( context )
    {}
    virtual void collectErrors( QList<QgsGeometryCheckError *> &errors, QStringList &messages, QAtomicInt *progressCounter = nullptr, const QMap<QString, QgsFeatureIds> &ids = QMap<QString, QgsFeatureIds>() ) const = 0;
    virtual void fixError( QgsGeometryCheckError *error, int method, const QMap<QString, int> &mergeAttributeIndices, Changes &changes ) const = 0;
    virtual QStringList getResolutionMethods() const = 0;
    virtual QString errorDescription() const = 0;
    virtual QString errorName() const = 0;
    CheckType getCheckType() const { return mCheckType; }
    bool getCompatibility( QgsWkbTypes::GeometryType type ) const { return mCompatibleGeometryTypes.contains( type ); }
    QgsGeometryCheckerContext *getContext() const { return mContext; }

  protected:
    QMap<QString, QgsFeatureIds> allLayerFeatureIds() const;
    void replaceFeatureGeometryPart( const QString &layerId, QgsFeature &feature, int partIdx, QgsAbstractGeometry *newPartGeom, Changes &changes ) const;
    void deleteFeatureGeometryPart( const QString &layerId, QgsFeature &feature, int partIdx, Changes &changes ) const;
    void deleteFeatureGeometryRing( const QString &layerId, QgsFeature &feature, int partIdx, int ringIdx, Changes &changes ) const;

  private:
    const CheckType mCheckType;
    QList<QgsWkbTypes::GeometryType> mCompatibleGeometryTypes;

  protected:
    QgsGeometryCheckerContext *mContext;
};


class QgsGeometryCheckError
{
  public:
    enum Status { StatusPending, StatusFixFailed, StatusFixed, StatusObsolete };
    enum ValueType { ValueLength, ValueArea, ValueOther };

    QgsGeometryCheckError( const QgsGeometryCheck *check,
                           const QString &layerId,
                           QgsFeatureId featureId,
                           const QgsPoint &errorLocation,
                           QgsVertexId vidx = QgsVertexId(),
                           const QVariant &value = QVariant(),
                           ValueType valueType = ValueOther );
    virtual ~QgsGeometryCheckError() = default;

    const QgsGeometryCheckError &operator=( const QgsGeometryCheckError & ) = delete;

    const QgsGeometryCheck *check() const { return mCheck; }
    const QString &layerId() const { return mLayerId; }
    QgsFeatureId featureId() const { return mFeatureId; }
    virtual QgsAbstractGeometry *geometry();
    virtual QgsRectangle affectedAreaBBox() { return geometry() ? geometry()->boundingBox() : QgsRectangle(); }
    virtual QString description() const { return mCheck->errorDescription(); }
    const QgsPoint &location() const { return mErrorLocation; }
    QVariant value() const { return mValue; }
    ValueType valueType() const { return mValueType; }
    QgsVertexId vidx() const { return mVidx; }
    Status status() const { return mStatus; }
    QString resolutionMessage() const { return mResolutionMessage; }
    void setFixed( int method )
    {
      mStatus = StatusFixed;
      mResolutionMessage = mCheck->getResolutionMethods()[method];
    }
    void setFixFailed( const QString &reason )
    {
      mStatus = StatusFixFailed;
      mResolutionMessage = reason;
    }
    void setObsolete() { mStatus = StatusObsolete; }
    virtual bool isEqual( QgsGeometryCheckError *other ) const
    {
      return other->check() == check() &&
             other->layerId() == layerId() &&
             other->featureId() == featureId() &&
             other->vidx() == vidx();
    }
    virtual bool closeMatch( QgsGeometryCheckError * /*other*/ ) const
    {
      return false;
    }
    virtual void update( const QgsGeometryCheckError *other )
    {
      Q_ASSERT( mCheck == other->mCheck );
      Q_ASSERT( mLayerId == other->mLayerId );
      Q_ASSERT( mFeatureId == other->mFeatureId );
      mErrorLocation = other->mErrorLocation;
      mVidx = other->mVidx;
      mValue = other->mValue;
    }

    virtual bool handleChanges( const QgsGeometryCheck::Changes &changes );

  protected:
    const QgsGeometryCheck *mCheck = nullptr;
    QString mLayerId;
    QgsFeatureId mFeatureId;
    QgsPoint mErrorLocation;
    QgsVertexId mVidx;
    QVariant mValue;
    ValueType mValueType;
    Status mStatus;
    QString mResolutionMessage;

};

Q_DECLARE_METATYPE( QgsGeometryCheckError * )

#endif // QGS_GEOMETRY_CHECK_H
