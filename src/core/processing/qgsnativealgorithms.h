/***************************************************************************
                         qgsnativealgorithms.h
                         ---------------------
    begin                : April 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSNATIVEALGORITHMS_H
#define QGSNATIVEALGORITHMS_H

#define SIP_NO_FILE

#include "qgis_core.h"
#include "qgis.h"
#include "qgsprocessingalgorithm.h"
#include "qgsprocessingprovider.h"
#include "qgsprocessingutils.h"
#include "qgsmaptopixelgeometrysimplifier.h"

///@cond PRIVATE

class QgsNativeAlgorithms: public QgsProcessingProvider
{
    Q_OBJECT

  public:

    QgsNativeAlgorithms( QObject *parent = nullptr );

    QIcon icon() const override;
    QString svgIconPath() const override;
    QString id() const override;
    QString name() const override;
    bool supportsNonFileBasedOutput() const override;

  protected:

    void loadAlgorithms() override;

};

/**
 * Native save selected features algorithm.
 */
class QgsSaveSelectedFeatures : public QgsProcessingAlgorithm
{

  public:

    QgsSaveSelectedFeatures() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override { return QStringLiteral( "saveselectedfeatures" ); }
    QString displayName() const override { return QObject::tr( "Save Selected Features" ); }
    QStringList tags() const override { return QObject::tr( "selection,save" ).split( ',' ); }
    QString group() const override { return QObject::tr( "Vector general" ); }
    QString shortHelpString() const override;
    QgsSaveSelectedFeatures *createInstance() const override SIP_FACTORY;

  protected:

    virtual QVariantMap processAlgorithm( const QVariantMap &parameters,
                                          QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
};

/**
 * Native centroid algorithm.
 */
class QgsCentroidAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{

  public:

    QgsCentroidAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override { return QStringLiteral( "centroids" ); }
    QString displayName() const override { return QObject::tr( "Centroids" ); }
    QStringList tags() const override { return QObject::tr( "centroid,center,average,point,middle" ).split( ',' ); }
    QString group() const override { return QObject::tr( "Vector geometry" ); }
    QString shortHelpString() const override;
    QgsCentroidAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    QString outputName() const override { return QObject::tr( "Centroids" ); }
    QgsProcessing::SourceType outputLayerType() const override { return QgsProcessing::TypeVectorPoint; }
    QgsWkbTypes::Type outputWkbType( QgsWkbTypes::Type inputWkbType ) const override { Q_UNUSED( inputWkbType ); return QgsWkbTypes::Point; }

    QgsFeature processFeature( const QgsFeature &feature, QgsProcessingFeedback *feedback ) override;
};

/**
 * Native transform algorithm.
 */
class QgsTransformAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{

  public:

    QgsTransformAlgorithm() = default;
    QString name() const override { return QStringLiteral( "reprojectlayer" ); }
    QString displayName() const override { return QObject::tr( "Reproject layer" ); }
    virtual QStringList tags() const override { return QObject::tr( "transform,reproject,crs,srs,warp" ).split( ',' ); }
    QString group() const override { return QObject::tr( "Vector general" ); }
    QString shortHelpString() const override;
    QgsTransformAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;
    QgsCoordinateReferenceSystem outputCrs( const QgsCoordinateReferenceSystem & ) const override { return mDestCrs; }
    QString outputName() const override { return QObject::tr( "Reprojected" ); }

    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeature processFeature( const QgsFeature &feature, QgsProcessingFeedback *feedback ) override;

  private:

    bool mCreatedTransform = false;
    QgsCoordinateReferenceSystem mDestCrs;
    QgsCoordinateTransform mTransform;

};

/**
 * Native buffer algorithm.
 */
class QgsBufferAlgorithm : public QgsProcessingAlgorithm
{

  public:

    QgsBufferAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;

    QString name() const override { return QStringLiteral( "buffer" ); }
    QString displayName() const override { return QObject::tr( "Buffer" ); }
    virtual QStringList tags() const override { return QObject::tr( "buffer,grow" ).split( ',' ); }
    QString group() const override { return QObject::tr( "Vector geometry" ); }
    QString shortHelpString() const override;
    QgsBufferAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    virtual QVariantMap processAlgorithm( const QVariantMap &parameters,
                                          QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

};

/**
 * Base class for dissolve/collect type algorithms.
 */
class QgsCollectorAlgorithm : public QgsProcessingAlgorithm
{
  protected:

    QVariantMap processCollection( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback,
                                   const std::function<QgsGeometry( const QList<QgsGeometry>& )> &collector, int maxQueueLength = 0 );
};

/**
 * Native dissolve algorithm.
 */
class QgsDissolveAlgorithm : public QgsCollectorAlgorithm
{

  public:

    QgsDissolveAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override { return QStringLiteral( "dissolve" ); }
    QString displayName() const override { return QObject::tr( "Dissolve" ); }
    virtual QStringList tags() const override { return QObject::tr( "dissolve,union,combine,collect" ).split( ',' ); }
    QString group() const override { return QObject::tr( "Vector geometry" ); }
    QString shortHelpString() const override;
    QgsDissolveAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    virtual QVariantMap processAlgorithm( const QVariantMap &parameters,
                                          QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

};

/**
 * Native collect geometries algorithm.
 */
class QgsCollectAlgorithm : public QgsCollectorAlgorithm
{

  public:

    QgsCollectAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override { return QStringLiteral( "collect" ); }
    QString displayName() const override { return QObject::tr( "Collect geometries" ); }
    virtual QStringList tags() const override { return QObject::tr( "union,combine,collect,multipart,parts,single" ).split( ',' ); }
    QString group() const override { return QObject::tr( "Vector geometry" ); }
    QString shortHelpString() const override;
    QgsCollectAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    virtual QVariantMap processAlgorithm( const QVariantMap &parameters,
                                          QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

};

/**
 * Native extract by attribute algorithm.
 */
class QgsExtractByAttributeAlgorithm : public QgsProcessingAlgorithm
{

  public:

    enum Operation
    {
      Equals,
      NotEquals,
      GreaterThan,
      GreaterThanEqualTo,
      LessThan,
      LessThanEqualTo,
      BeginsWith,
      Contains,
      IsNull,
      IsNotNull,
      DoesNotContain,
    };

    QgsExtractByAttributeAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override { return QStringLiteral( "extractbyattribute" ); }
    QString displayName() const override { return QObject::tr( "Extract by attribute" ); }
    virtual QStringList tags() const override { return QObject::tr( "extract,filter,attribute,value,contains,null,field" ).split( ',' ); }
    QString group() const override { return QObject::tr( "Vector selection" ); }
    QString shortHelpString() const override;
    QgsExtractByAttributeAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    virtual QVariantMap processAlgorithm( const QVariantMap &parameters,
                                          QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

};

/**
 * Native extract by expression algorithm.
 */
class QgsExtractByExpressionAlgorithm : public QgsProcessingAlgorithm
{

  public:

    QgsExtractByExpressionAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override { return QStringLiteral( "extractbyexpression" ); }
    QString displayName() const override { return QObject::tr( "Extract by expression" ); }
    virtual QStringList tags() const override { return QObject::tr( "extract,filter,expression,field" ).split( ',' ); }
    QString group() const override { return QObject::tr( "Vector selection" ); }
    QString shortHelpString() const override;
    QgsExtractByExpressionAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    virtual QVariantMap processAlgorithm( const QVariantMap &parameters,
                                          QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

};

/**
 * Native clip algorithm.
 */
class QgsClipAlgorithm : public QgsProcessingAlgorithm
{

  public:

    QgsClipAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override { return QStringLiteral( "clip" ); }
    QString displayName() const override { return QObject::tr( "Clip" ); }
    virtual QStringList tags() const override { return QObject::tr( "clip,intersect,intersection,mask" ).split( ',' ); }
    QString group() const override { return QObject::tr( "Vector overlay" ); }
    QString shortHelpString() const override;
    QgsClipAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    virtual QVariantMap processAlgorithm( const QVariantMap &parameters,
                                          QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

};


/**
 * Native subdivide algorithm.
 */
class QgsSubdivideAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{

  public:

    QgsSubdivideAlgorithm() = default;
    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override { return QStringLiteral( "subdivide" ); }
    QString displayName() const override { return QObject::tr( "Subdivide" ); }
    virtual QStringList tags() const override { return QObject::tr( "subdivide,segmentize,split,tesselate" ).split( ',' ); }
    QString group() const override { return QObject::tr( "Vector geometry" ); }
    QString shortHelpString() const override;
    QgsSubdivideAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QString outputName() const override { return QObject::tr( "Subdivided" ); }

    QgsWkbTypes::Type outputWkbType( QgsWkbTypes::Type inputWkbType ) const override;
    QgsFeature processFeature( const QgsFeature &feature, QgsProcessingFeedback *feedback ) override;

    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:

    int mMaxNodes = -1;

};

/**
 * Native multipart to singlepart algorithm.
 */
class QgsMultipartToSinglepartAlgorithm : public QgsProcessingAlgorithm
{

  public:

    QgsMultipartToSinglepartAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override { return QStringLiteral( "multiparttosingleparts" ); }
    QString displayName() const override { return QObject::tr( "Multipart to singleparts" ); }
    virtual QStringList tags() const override { return QObject::tr( "multi,single,multiple,split,dump" ).split( ',' ); }
    QString group() const override { return QObject::tr( "Vector geometry" ); }
    QString shortHelpString() const override;
    QgsMultipartToSinglepartAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    virtual QVariantMap processAlgorithm( const QVariantMap &parameters,
                                          QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

};

/**
 * Native promote to multipart algorithm.
 */
class QgsPromoteToMultipartAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{

  public:

    QgsPromoteToMultipartAlgorithm() = default;
    QString name() const override { return QStringLiteral( "promotetomulti" ); }
    QString displayName() const override { return QObject::tr( "Promote to multipart" ); }
    virtual QStringList tags() const override { return QObject::tr( "multi,single,multiple,convert,force,parts" ).split( ',' ); }
    QString group() const override { return QObject::tr( "Vector geometry" ); }
    QString shortHelpString() const override;
    QgsPromoteToMultipartAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QString outputName() const override { return QObject::tr( "Multiparts" ); }

    QgsWkbTypes::Type outputWkbType( QgsWkbTypes::Type inputWkbType ) const override;
    QgsFeature processFeature( const QgsFeature &feature, QgsProcessingFeedback *feedback ) override;

};

/**
 * Remove null geometry algorithm.
 */
class QgsRemoveNullGeometryAlgorithm : public QgsProcessingAlgorithm
{

  public:

    QgsRemoveNullGeometryAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override { return QStringLiteral( "removenullgeometries" ); }
    QString displayName() const override { return QObject::tr( "Remove null geometries" ); }
    virtual QStringList tags() const override { return QObject::tr( "remove,drop,delete,empty,geometry" ).split( ',' ); }
    QString group() const override { return QObject::tr( "Vector selection" ); }
    QString shortHelpString() const override;
    QgsRemoveNullGeometryAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    virtual QVariantMap processAlgorithm( const QVariantMap &parameters,
                                          QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

};

/**
 * Native bounding boxes algorithm.
 */
class QgsBoundingBoxAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{

  public:

    QgsBoundingBoxAlgorithm() = default;
    QString name() const override { return QStringLiteral( "boundingboxes" ); }
    QString displayName() const override { return QObject::tr( "Bounding boxes" ); }
    virtual QStringList tags() const override { return QObject::tr( "bounding,boxes,envelope,rectangle,extent" ).split( ',' ); }
    QString group() const override { return QObject::tr( "Vector geometry" ); }
    QString shortHelpString() const override;
    QgsBoundingBoxAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QString outputName() const override { return QObject::tr( "Bounds" ); }
    QgsWkbTypes::Type outputWkbType( QgsWkbTypes::Type ) const override { return QgsWkbTypes::Polygon; }
    QgsFields outputFields( const QgsFields &inputFields ) const override;
    QgsFeature processFeature( const QgsFeature &feature, QgsProcessingFeedback *feedback ) override;

};

/**
 * Native minimum oriented bounding box algorithm.
 */
class QgsOrientedMinimumBoundingBoxAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{

  public:

    QgsOrientedMinimumBoundingBoxAlgorithm() = default;
    QString name() const override { return QStringLiteral( "orientedminimumboundingbox" ); }
    QString displayName() const override { return QObject::tr( "Oriented minimum bounding box" ); }
    virtual QStringList tags() const override { return QObject::tr( "bounding,boxes,envelope,rectangle,extent,oriented,angle" ).split( ',' ); }
    QString group() const override { return QObject::tr( "Vector geometry" ); }
    QString shortHelpString() const override;
    QgsOrientedMinimumBoundingBoxAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QString outputName() const override { return QObject::tr( "Bounding boxes" ); }
    QgsWkbTypes::Type outputWkbType( QgsWkbTypes::Type ) const override { return QgsWkbTypes::Polygon; }
    QgsFields outputFields( const QgsFields &inputFields ) const override;
    QgsFeature processFeature( const QgsFeature &feature, QgsProcessingFeedback *feedback ) override;

};

/**
 * Native minimum enclosing circle algorithm.
 */
class QgsMinimumEnclosingCircleAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{

  public:

    QgsMinimumEnclosingCircleAlgorithm() = default;
    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override { return QStringLiteral( "minimumenclosingcircle" ); }
    QString displayName() const override { return QObject::tr( "Minimum enclosing circles" ); }
    virtual QStringList tags() const override { return QObject::tr( "minimum,circle,ellipse,extent,bounds,bounding" ).split( ',' ); }
    QString group() const override { return QObject::tr( "Vector geometry" ); }
    QString shortHelpString() const override;
    QgsMinimumEnclosingCircleAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QString outputName() const override { return QObject::tr( "Minimum enclosing circles" ); }
    QgsWkbTypes::Type outputWkbType( QgsWkbTypes::Type ) const override { return QgsWkbTypes::Polygon; }
    QgsFields outputFields( const QgsFields &inputFields ) const override;
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeature processFeature( const QgsFeature &feature, QgsProcessingFeedback *feedback ) override;

  private:

    int mSegments = 72;
};

/**
 * Native convex hull algorithm.
 */
class QgsConvexHullAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{

  public:

    QgsConvexHullAlgorithm() = default;
    QString name() const override { return QStringLiteral( "convexhull" ); }
    QString displayName() const override { return QObject::tr( "Convex hull" ); }
    virtual QStringList tags() const override { return QObject::tr( "convex,hull,bounds,bounding" ).split( ',' ); }
    QString group() const override { return QObject::tr( "Vector geometry" ); }
    QString shortHelpString() const override;
    QgsConvexHullAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QString outputName() const override { return QObject::tr( "Convex hulls" ); }
    QgsWkbTypes::Type outputWkbType( QgsWkbTypes::Type ) const override { return QgsWkbTypes::Polygon; }
    QgsFields outputFields( const QgsFields &inputFields ) const override;
    QgsFeature processFeature( const QgsFeature &feature, QgsProcessingFeedback *feedback ) override;

};


/**
 * Base class for location based extraction/selection algorithms.
 */
class QgsLocationBasedAlgorithm : public QgsProcessingAlgorithm
{

  protected:

    enum Predicate
    {
      Intersects,
      Contains,
      Disjoint,
      IsEqual,
      Touches,
      Overlaps,
      Within,
      Crosses,
    };

    void addPredicateParameter();
    Predicate reversePredicate( Predicate predicate ) const;
    QStringList predicateOptionsList() const;
    void process( QgsFeatureSource *targetSource, QgsFeatureSource *intersectSource, const QList<int> &selectedPredicates, const std::function< void( const QgsFeature & )> &handleFeatureFunction, bool onlyRequireTargetIds, QgsFeedback *feedback );
};

/**
 * Native select by location algorithm
 */
class QgsSelectByLocationAlgorithm : public QgsLocationBasedAlgorithm
{

  public:

    QgsSelectByLocationAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override { return QStringLiteral( "selectbylocation" ); }
    QString displayName() const override { return QObject::tr( "Select by location" ); }
    virtual QStringList tags() const override { return QObject::tr( "select,intersects,intersecting,disjoint,touching,within,contains,overlaps,relation" ).split( ',' ); }
    QString group() const override { return QObject::tr( "Vector selection" ); }
    QString shortHelpString() const override;
    QgsSelectByLocationAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    virtual QVariantMap processAlgorithm( const QVariantMap &parameters,
                                          QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

};

/**
 * Native extract by location algorithm
 */
class QgsExtractByLocationAlgorithm : public QgsLocationBasedAlgorithm
{

  public:

    QgsExtractByLocationAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override { return QStringLiteral( "extractbylocation" ); }
    QString displayName() const override { return QObject::tr( "Extract by location" ); }
    virtual QStringList tags() const override { return QObject::tr( "extract,filter,intersects,intersecting,disjoint,touching,within,contains,overlaps,relation" ).split( ',' ); }
    QString group() const override { return QObject::tr( "Vector selection" ); }
    QString shortHelpString() const override;
    QgsExtractByLocationAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    virtual QVariantMap processAlgorithm( const QVariantMap &parameters,
                                          QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

};

/**
 * Native repair geometries algorithm.
 */
class QgsFixGeometriesAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{

  public:

    QgsFixGeometriesAlgorithm() = default;
    QString name() const override { return QStringLiteral( "fixgeometries" ); }
    QString displayName() const override { return QObject::tr( "Fix geometries" ); }
    virtual QStringList tags() const override { return QObject::tr( "repair,invalid,geometry,make,valid" ).split( ',' ); }
    QString group() const override { return QObject::tr( "Vector geometry" ); }
    QString shortHelpString() const override;
    QgsFixGeometriesAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QgsProcessingFeatureSource::Flag sourceFlags() const override { return QgsProcessingFeatureSource::FlagSkipGeometryValidityChecks; }
    QString outputName() const override { return QObject::tr( "Fixed geometries" ); }
    QgsWkbTypes::Type outputWkbType( QgsWkbTypes::Type type ) const override { return QgsWkbTypes::multiType( type ); }
    QgsFeature processFeature( const QgsFeature &feature, QgsProcessingFeedback *feedback ) override;

};

/**
 * Native merge lines algorithm.
 */
class QgsMergeLinesAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{

  public:

    QgsMergeLinesAlgorithm() = default;
    QString name() const override { return QStringLiteral( "mergelines" ); }
    QString displayName() const override { return QObject::tr( "Merge lines" ); }
    virtual QStringList tags() const override { return QObject::tr( "line,merge,join,parts" ).split( ',' ); }
    QString group() const override { return QObject::tr( "Vector geometry" ); }
    QString shortHelpString() const override;
    QgsMergeLinesAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QString outputName() const override { return QObject::tr( "Merged" ); }
    QgsProcessing::SourceType outputLayerType() const override { return QgsProcessing::TypeVectorLine; }
    QgsWkbTypes::Type outputWkbType( QgsWkbTypes::Type ) const override { return QgsWkbTypes::MultiLineString; }
    QgsFeature processFeature( const QgsFeature &feature, QgsProcessingFeedback *feedback ) override;

};

/**
 * Native smooth algorithm.
 */
class QgsSmoothAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{

  public:

    QgsSmoothAlgorithm() = default;
    QString name() const override { return QStringLiteral( "smoothgeometry" ); }
    QString displayName() const override { return QObject::tr( "Smooth geometries" ); }
    virtual QStringList tags() const override { return QObject::tr( "smooth,curve,generalize,round,bend,corners" ).split( ',' ); }
    QString group() const override { return QObject::tr( "Vector geometry" ); }
    QString shortHelpString() const override;
    QgsSmoothAlgorithm *createInstance() const override SIP_FACTORY;
    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;

  protected:
    QString outputName() const override { return QObject::tr( "Smoothed" ); }
    QgsProcessing::SourceType outputLayerType() const override { return QgsProcessing::TypeVectorLine; }
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeature processFeature( const QgsFeature &feature, QgsProcessingFeedback *feedback ) override;

  private:
    int mIterations = 1;
    double mOffset = 0.25;
    double mMaxAngle = 0;
};

/**
 * Native simplify algorithm.
 */
class QgsSimplifyAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{

  public:

    QgsSimplifyAlgorithm() = default;
    QString name() const override { return QStringLiteral( "simplifygeometries" ); }
    QString displayName() const override { return QObject::tr( "Simplify geometries" ); }
    virtual QStringList tags() const override { return QObject::tr( "simplify,generalize,douglas,peucker,visvalingam" ).split( ',' ); }
    QString group() const override { return QObject::tr( "Vector geometry" ); }
    QString shortHelpString() const override;
    QgsSimplifyAlgorithm *createInstance() const override SIP_FACTORY;
    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;

  protected:
    QString outputName() const override { return QObject::tr( "Simplified" ); }
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeature processFeature( const QgsFeature &feature, QgsProcessingFeedback *feedback ) override;

  private:

    double mTolerance = 1.0;
    QgsMapToPixelSimplifier::SimplifyAlgorithm mMethod = QgsMapToPixelSimplifier::Distance;
    std::unique_ptr< QgsMapToPixelSimplifier > mSimplifier;

};


/**
 * Native extract/clip by extent algorithm.
 */
class QgsExtractByExtentAlgorithm : public QgsProcessingAlgorithm
{

  public:

    QgsExtractByExtentAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override { return QStringLiteral( "extractbyextent" ); }
    QString displayName() const override { return QObject::tr( "Extract/clip by extent" ); }
    virtual QStringList tags() const override { return QObject::tr( "clip,extract,intersect,intersection,mask,extent" ).split( ',' ); }
    QString group() const override { return QObject::tr( "Vector overlay" ); }
    QString shortHelpString() const override;
    QgsExtractByExtentAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    virtual QVariantMap processAlgorithm( const QVariantMap &parameters,
                                          QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

};

/**
 * Native extent to layer algorithm.
 */
class QgsExtentToLayerAlgorithm : public QgsProcessingAlgorithm
{

  public:

    QgsExtentToLayerAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override { return QStringLiteral( "extenttolayer" ); }
    QString displayName() const override { return QObject::tr( "Create layer from extent" ); }
    virtual QStringList tags() const override { return QObject::tr( "extent,layer,polygon,create,new" ).split( ',' ); }
    QString group() const override { return QObject::tr( "Vector geometry" ); }
    QString shortHelpString() const override;
    QgsExtentToLayerAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    virtual QVariantMap processAlgorithm( const QVariantMap &parameters,
                                          QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

};

/**
 * Native line intersection algorithm.
 */
class QgsLineIntersectionAlgorithm : public QgsProcessingAlgorithm
{

  public:

    QgsLineIntersectionAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override { return QStringLiteral( "lineintersections" ); }
    QString displayName() const override { return QObject::tr( "Line intersections" ); }
    virtual QStringList tags() const override { return QObject::tr( "line,intersection" ).split( ',' ); }
    QString group() const override { return QObject::tr( "Vector overlay" ); }
    QString shortHelpString() const override;
    QgsLineIntersectionAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    virtual QVariantMap processAlgorithm( const QVariantMap &parameters,
                                          QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

};

/**
 * Native split with lines algorithm.
 */
class QgsSplitWithLinesAlgorithm : public QgsProcessingAlgorithm
{

  public:

    QgsSplitWithLinesAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override { return QStringLiteral( "splitwithlines" ); }
    QString displayName() const override { return QObject::tr( "Split with lines" ); }
    virtual QStringList tags() const override { return QObject::tr( "split,cut,lines" ).split( ',' ); }
    QString group() const override { return QObject::tr( "Vector overlay" ); }
    QString shortHelpString() const override;
    QgsSplitWithLinesAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    virtual QVariantMap processAlgorithm( const QVariantMap &parameters,
                                          QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

};

/**
 * Native mean coordinates algorithm.
 */
class QgsMeanCoordinatesAlgorithm : public QgsProcessingAlgorithm
{

  public:

    QgsMeanCoordinatesAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override { return QStringLiteral( "meancoordinates" ); }
    QString displayName() const override { return QObject::tr( "Mean coordinate(s)" ); }
    virtual QStringList tags() const override { return QObject::tr( "mean,average,coordinate" ).split( ',' ); }
    QString group() const override { return QObject::tr( "Vector analysis" ); }
    QString shortHelpString() const override;
    QgsMeanCoordinatesAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    virtual QVariantMap processAlgorithm( const QVariantMap &parameters,
                                          QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

};

///@endcond PRIVATE

#endif // QGSNATIVEALGORITHMS_H


