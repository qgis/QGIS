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
#include "../utils/qgsgeomutils.h"
#include "geos_c.h"
#include <QApplication>

class QgsGeometryCheckError;
class QgsFeaturePool;

#define FEATUREID_NULL std::numeric_limits<QgsFeatureId>::min()

class QgsGeometryCheckPrecision
{
  public:
    static void setPrecision( int precision );
    static int precision();
    static int reducedPrecision();
    static double tolerance();
    static double reducedTolerance();

  private:
    QgsGeometryCheckPrecision();
    static QgsGeometryCheckPrecision* get();

    int mPrecision;
    int mReducedPrecision;
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
      Change() {}
      Change( ChangeWhat _what, ChangeType _type, QgsVertexId _vidx = QgsVertexId() )
          : what( _what )
          , type( _type )
          , vidx( _vidx )
      {}
      ChangeWhat what;
      ChangeType type;
      QgsVertexId vidx;
    };

    typedef QMap<QgsFeatureId, QList<Change> > Changes;

    QgsGeometryCheck( CheckType checkType, QgsFeaturePool* featurePool )
        : mCheckType( checkType )
        , mFeaturePool( featurePool )
    {}
    virtual ~QgsGeometryCheck() {}
    virtual void collectErrors( QList<QgsGeometryCheckError*>& errors, QStringList& messages, QAtomicInt* progressCounter = nullptr, const QgsFeatureIds& ids = QgsFeatureIds() ) const = 0;
    virtual void fixError( QgsGeometryCheckError* error, int method, int mergeAttributeIndex, Changes& changes ) const = 0;
    virtual const QStringList& getResolutionMethods() const = 0;
    virtual QString errorDescription() const = 0;
    virtual QString errorName() const = 0;
    CheckType getCheckType() const { return mCheckType; }
    QgsFeaturePool* getFeaturePool() const { return mFeaturePool; }

  protected:
    const CheckType mCheckType;
    QgsFeaturePool* mFeaturePool;

    void replaceFeatureGeometryPart( QgsFeature& feature, int partIdx, QgsAbstractGeometryV2* newPartGeom, Changes& changes ) const;
    void deleteFeatureGeometryPart( QgsFeature& feature, int partIdx, Changes& changes ) const;
    void deleteFeatureGeometryRing( QgsFeature& feature, int partIdx, int ringIdx, Changes& changes ) const;
};


class QgsGeometryCheckError
{
  public:
    enum Status { StatusPending, StatusFixFailed, StatusFixed, StatusObsolete };
    enum ValueType { ValueLength, ValueArea, ValueOther };

    QgsGeometryCheckError( const QgsGeometryCheck* check,
                           QgsFeatureId featureId,
                           const QgsPointV2& errorLocation,
                           QgsVertexId vidx = QgsVertexId(),
                           const QVariant& value = QVariant(),
                           ValueType valueType = ValueOther );
    virtual ~QgsGeometryCheckError();
    const QgsGeometryCheck* check() const { return mCheck; }
    QgsFeatureId featureId() const { return mFeatureId; }
    virtual QgsAbstractGeometryV2* geometry();
    virtual QgsRectangle affectedAreaBBox() { return geometry() ? geometry()->boundingBox() : QgsRectangle(); }
    virtual QString description() const { return mCheck->errorDescription(); }
    const QgsPointV2& location() const { return mErrorLocation; }
    const QVariant& value() const { return mValue; }
    ValueType valueType() const { return mValueType; }
    QgsVertexId vidx() const { return mVidx; }
    Status status() const { return mStatus; }
    const QString& resolutionMessage() const { return mResolutionMessage; }
    void setFixed( int method )
    {
      mStatus = StatusFixed;
      mResolutionMessage = mCheck->getResolutionMethods()[method];
    }
    void setFixFailed( const QString& reason )
    {
      mStatus = StatusFixFailed;
      mResolutionMessage = reason;
    }
    void setObsolete() { mStatus = StatusObsolete; }
    virtual bool isEqual( QgsGeometryCheckError* other ) const
    {
      return other->check() == check() &&
             other->featureId() == featureId() &&
             other->vidx() == vidx();
    }
    virtual bool closeMatch( QgsGeometryCheckError* /*other*/ ) const
    {
      return false;
    }
    virtual void update( const QgsGeometryCheckError* other )
    {
      assert( mCheck == other->mCheck );
      assert( mFeatureId == other->mFeatureId );
      mErrorLocation = other->mErrorLocation;
      mVidx = other->mVidx;
      mValue = other->mValue;
    }

    virtual bool handleChanges( const QgsGeometryCheck::Changes& changes );

  protected:
    const QgsGeometryCheck* mCheck;
    QgsFeatureId mFeatureId;
    QgsPointV2 mErrorLocation;
    QgsVertexId mVidx;
    QVariant mValue;
    ValueType mValueType;
    Status mStatus;
    QString mResolutionMessage;

  private:
    const QgsGeometryCheckError& operator=( const QgsGeometryCheckError& );
};

Q_DECLARE_METATYPE( QgsGeometryCheckError* )

#endif // QGS_GEOMETRY_CHECK_H
