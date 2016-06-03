/***************************************************************************
    qgsgeometrygapcheck.h
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

#ifndef QGS_GEOMETRY_GAP_CHECK_H
#define QGS_GEOMETRY_GAP_CHECK_H

#include "qgsgeometrycheck.h"


class QgsGeometryGapCheckError : public QgsGeometryCheckError
{
  public:
    QgsGeometryGapCheckError( const QgsGeometryCheck* check,
                              QgsAbstractGeometryV2* geometry,
                              const QgsFeatureIds& neighbors,
                              double area,
                              const QgsRectangle& gapAreaBBox )
        : QgsGeometryCheckError( check, FEATUREID_NULL, geometry->centroid(), QgsVertexId(), area, ValueArea )
        , mNeighbors( neighbors )
        , mGapAreaBBox( gapAreaBBox )
    {
      mGeometry = geometry;
    }
    ~QgsGeometryGapCheckError()
    {
      delete mGeometry;
    }

    QgsAbstractGeometryV2* geometry() override { return mGeometry->clone(); }
    const QgsFeatureIds& neighbors() const { return mNeighbors; }

    bool isEqual( QgsGeometryCheckError* other ) const override
    {
      QgsGeometryGapCheckError* err = dynamic_cast<QgsGeometryGapCheckError*>( other );
      return err && QgsGeomUtils::pointsFuzzyEqual( err->location(), location(), QgsGeometryCheckPrecision::reducedTolerance() ) && err->neighbors() == neighbors();
    }

    bool closeMatch( QgsGeometryCheckError *other ) const override
    {
      QgsGeometryGapCheckError* err = dynamic_cast<QgsGeometryGapCheckError*>( other );
      return err && err->neighbors() == neighbors();
    }

    void update( const QgsGeometryCheckError* other ) override
    {
      QgsGeometryCheckError::update( other );
      // Static cast since this should only get called if isEqual == true
      const QgsGeometryGapCheckError* err = static_cast<const QgsGeometryGapCheckError*>( other );
      delete mGeometry;
      mGeometry = err->mGeometry->clone();
      mNeighbors = err->mNeighbors;
      mGapAreaBBox = err->mGapAreaBBox;
    }

    bool handleChanges( const QgsGeometryCheck::Changes& /*changes*/ ) override
    {
      return true;
    }

    QgsRectangle affectedAreaBBox() override
    {
      return mGapAreaBBox;
    }

  private:
    QgsFeatureIds mNeighbors;
    QgsRectangle mGapAreaBBox;
    QgsAbstractGeometryV2* mGeometry;
};

class QgsGeometryGapCheck : public QgsGeometryCheck
{
    Q_OBJECT

  public:
    QgsGeometryGapCheck( QgsFeaturePool* featurePool, double threshold )
        : QgsGeometryCheck( LayerCheck, featurePool )
        , mThreshold( threshold )
    {}
    void collectErrors( QList<QgsGeometryCheckError*>& errors, QStringList &messages, QAtomicInt* progressCounter = nullptr, const QgsFeatureIds& ids = QgsFeatureIds() ) const override;
    void fixError( QgsGeometryCheckError* error, int method, int mergeAttributeIndex, Changes& changes ) const override;
    const QStringList& getResolutionMethods() const override;
    QString errorDescription() const override { return tr( "Gap" ); }
    QString errorName() const override { return "QgsGeometryGapCheck"; }

  private:
    enum ResolutionMethod { MergeLongestEdge, NoChange };

    double mThreshold;

    bool mergeWithNeighbor( QgsGeometryGapCheckError *err, Changes &changes , QString &errMsg ) const;
};

#endif // QGS_GEOMETRY_GAP_CHECK_H
