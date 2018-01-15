/***************************************************************************
  topolError.h
  TOPOLogy checker
  -------------------
         begin                : May 2009
         copyright            : Vita Cizek
         email                : weetya (at) gmail.com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef TOPOLERROR_H
#define TOPOLERROR_H

#include "qgsvectorlayer.h"
#include "qgsgeometry.h"
#include "qgsrectangle.h"

class TopolError;
typedef QList<TopolError *> ErrorList;
typedef bool ( TopolError::*fixFunction )();

class FeatureLayer
{
  public:
    FeatureLayer() = default;

    /**
     * Constructor
     * \param layer layer pointer
     * \param feature QgsFeature
     */
    FeatureLayer( QgsVectorLayer *layer, const QgsFeature &feature )
      : layer( layer )
      , feature( feature )
    {}

    QgsVectorLayer *layer = nullptr;
    QgsFeature feature;
};

class TopolError
{
  protected:
    QString mName;
    QgsRectangle mBoundingBox;
    QgsGeometry mConflict;
    QList<FeatureLayer> mFeaturePairs;
    QMap<QString, fixFunction> mFixMap;

    /**
     * A dummy fix - does nothing
     */
    bool fixDummy() { return false; }

    /**
     * Snaps to a feature
     */
    bool fixSnap();

    /**
     * Moves first feature
     */
    bool fixMoveFirst();

    /**
     * Moves second feature
     */
    bool fixMoveSecond();

    /**
     * Unions features to the first
     */
    bool fixUnionFirst();

    /**
     * Unions features to the first
     */
    bool fixUnionSecond();

    /**
     * Deletes first feature
     */
    bool fixDeleteFirst();

    /**
     * Deletes second feature
     */
    bool fixDeleteSecond();

    //helper fix functions

    /**
     * Makes geometry difference
     * \param fl1 first FeatureLayer pair
     * \param fl2 second FeatureLayer pair
     */
    bool fixMove( const FeatureLayer &fl1, const FeatureLayer &fl2 );

    /**
     * Unions features to the first one
     * \param fl1 first FeatureLayer pair
     * \param fl2 second FeatureLayer pair
     */
    bool fixUnion( const FeatureLayer &fl1, const FeatureLayer &fl2 );

  public:

    /**
     * Constructor
     * \param boundingBox bounding box of the two features
     * \param conflict geometry representation of the conflict
     * \param featurePairs FeatureLayer pairs of the two features
     */
    TopolError( const QgsRectangle &boundingBox, const QgsGeometry &conflict, const QList<FeatureLayer> &featurePairs );

    virtual ~TopolError() = default;

    /**
     * Runs fixing function
     * \param fixName name of the fix
     */
    virtual bool fix( const QString &fixName );

    /**
     * Returns error's name
     */
    virtual QString name() { return mName; }

    /**
     * Returns topology conflict
     */
    virtual QgsGeometry conflict() const { return mConflict; }

    /**
     * Returns bounding box of the error
     */
    virtual QgsRectangle boundingBox() { return mBoundingBox; }

    /**
     * Returns FeatureLayer pairs from the error
     */
    virtual QList<FeatureLayer> featurePairs() { return mFeaturePairs; }

    /**
     * Returns the names of possible fixes
     */
    virtual QStringList fixNames() { return mFixMap.keys(); }
};

class TopolErrorIntersection : public TopolError
{
  public:
    TopolErrorIntersection( const QgsRectangle &boundingBox, const QgsGeometry &conflict, QList<FeatureLayer> featurePairs );
};

class TopolErrorClose : public TopolError
{
  public:
    TopolErrorClose( const QgsRectangle &boundingBox, const QgsGeometry &conflict, const QList<FeatureLayer> &featurePairs );
};

class TopolErrorCovered : public TopolError
{
  public:
    TopolErrorCovered( const QgsRectangle &boundingBox, const QgsGeometry &conflict, const QList<FeatureLayer> &featurePairs );
};

class TopolErrorShort : public TopolError
{
  public:
    TopolErrorShort( const QgsRectangle &boundingBox, const QgsGeometry &conflict, const QList<FeatureLayer> &featurePairs );
};

class TopolErrorValid : public TopolError
{
  public:
    TopolErrorValid( const QgsRectangle &boundingBox, const QgsGeometry &conflict, const QList<FeatureLayer> &featurePairs );
};

class TopolErrorDangle : public TopolError
{
  public:
    TopolErrorDangle( const QgsRectangle &boundingBox, const QgsGeometry &conflict, const QList<FeatureLayer> &featurePairs );
};

class TopolErrorDuplicates : public TopolError
{
  public:
    TopolErrorDuplicates( const QgsRectangle &boundingBox, const QgsGeometry &conflict, const QList<FeatureLayer> &featurePairs );
};

class TopolErrorPseudos : public TopolError
{
  public:
    TopolErrorPseudos( const QgsRectangle &boundingBox, const QgsGeometry &conflict, const QList<FeatureLayer> &featurePairs );
};

class TopolErrorOverlaps : public TopolError
{
  public:
    TopolErrorOverlaps( const QgsRectangle &boundingBox, const QgsGeometry &conflict, const QList<FeatureLayer> &featurePairs );
};

class TopolErrorGaps : public TopolError
{
  public:
    TopolErrorGaps( const QgsRectangle &boundingBox, const QgsGeometry &conflict, const QList<FeatureLayer> &featurePairs );
};

class TopolErrorPointNotCoveredByLineEnds : public TopolError
{
  public:
    TopolErrorPointNotCoveredByLineEnds( const QgsRectangle &boundingBox, const QgsGeometry &conflict, const QList<FeatureLayer> &featurePairs );
};

class TopolErrorLineEndsNotCoveredByPoints : public TopolError
{
  public:
    TopolErrorLineEndsNotCoveredByPoints( const QgsRectangle &boundingBox, const QgsGeometry &conflict, const QList<FeatureLayer> &featurePairs );
};

class TopolErrorPointNotInPolygon : public TopolError
{
  public:
    TopolErrorPointNotInPolygon( const QgsRectangle &boundingBox, const QgsGeometry &conflict, const QList<FeatureLayer> &featurePairs );
};

class TopolErrorPolygonContainsPoint : public TopolError
{
  public:
    TopolErrorPolygonContainsPoint( const QgsRectangle &boundingBox, const QgsGeometry &conflict, const QList<FeatureLayer> &featurePairs );
};

class TopolErroMultiPart : public TopolError
{
  public:
    TopolErroMultiPart( const QgsRectangle &boundingBox, const QgsGeometry &conflict, const QList<FeatureLayer> &featurePairs );
};

#endif
