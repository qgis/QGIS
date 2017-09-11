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
                                   std::function<QgsGeometry( const QList< QgsGeometry >& )> collector, int maxQueueLength = 0 );
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

    Predicate reversePredicate( Predicate predicate ) const;
    QStringList predicateOptionsList() const;
    void process( QgsFeatureSource *targetSource, QgsFeatureSource *intersectSource, const QList<int> &selectedPredicates, std::function<void ( const QgsFeature & )> handleFeatureFunction, bool onlyRequireTargetIds, QgsFeedback *feedback );
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

///@endcond PRIVATE

#endif // QGSNATIVEALGORITHMS_H


