/***************************************************************************
    qgslabelingenginerule_impl.h
    ---------------------
  Date                 : August 2024
  Copyright            : (C) 2024 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLABELINGENGINERULEIMPL_H
#define QGSLABELINGENGINERULEIMPL_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgis.h"
#include "qgslabelingenginerule.h"
#include "qgsvectorlayerref.h"
#include "qgsmapunitscale.h"

class QgsSpatialIndex;


/**
 * Base class for labeling engine rules which prevents labels being placed too close or to far from features from a different layer.
 *
 * \ingroup core
 * \since QGIS 3.40
 */
class CORE_EXPORT QgsAbstractLabelingEngineRuleDistanceFromFeature : public QgsAbstractLabelingEngineRule
{
  public:

    QgsAbstractLabelingEngineRuleDistanceFromFeature();
    ~QgsAbstractLabelingEngineRuleDistanceFromFeature() override;
    bool prepare( QgsRenderContext &context ) override;
    void writeXml( QDomDocument &doc, QDomElement &element, const QgsReadWriteContext &context ) const override;
    void readXml( const QDomElement &element, const QgsReadWriteContext &context ) override;
    void resolveReferences( const QgsProject *project ) override;
    bool candidateIsIllegal( const pal::LabelPosition *candidate, QgsLabelingEngineContext &context ) const override SIP_SKIP;
    void alterCandidateCost( pal::LabelPosition *candidate, QgsLabelingEngineContext &context ) const override SIP_SKIP;
    bool isAvailable() const override;

    /**
     * Returns the layer providing the labels.
     *
     * \see setLabeledLayer()
     */
    QgsMapLayer *labeledLayer() const;

    /**
     * Sets the \a layer providing the labels.
     *
     * \see labeledLayer()
     */
    void setLabeledLayer( QgsMapLayer *layer );

    /**
     * Returns the layer providing the features which labels must be distant from (or close to).
     *
     * \see setTargetLayer()
     */
    QgsVectorLayer *targetLayer() const;

    /**
     * Sets the \a layer providing the features which labels must be distant from (or close to).
     *
     * \see targetLayer()
     */
    void setTargetLayer( QgsVectorLayer *layer );

    /**
     * Returns the acceptable distance threshold between labels and the features
     * from the targetLayer().
     *
     * \see setDistance()
     * \see distanceUnit()
     */
    double distance() const { return mDistance; }

    /**
     * Sets the acceptable \a distance threshold between labels and the features
     * from the targetLayer().
     *
     * \see distance()
     * \see setDistanceUnit()
     */
    void setDistance( double distance ) { mDistance = distance; }

    /**
     * Returns the units for the distance between labels and the features
     * from the targetLayer().
     *
     * \see setDistanceUnit()
     * \see distance()
     */
    Qgis::RenderUnit distanceUnit() const { return mDistanceUnit; }

    /**
     * Sets the \a unit for the distance between labels and the features
     * from the targetLayer().
     *
     * \see distanceUnit()
     * \see setDistance()
     */
    void setDistanceUnit( Qgis::RenderUnit unit ) { mDistanceUnit = unit; }

    /**
     * Returns the scaling for the distance between labels and the features
     * from the targetLayer().
     *
     * \see setDistanceUnitScale()
     * \see distance()
     */
    const QgsMapUnitScale &distanceUnitScale() const { return mDistanceUnitScale; }

    /**
     * Sets the \a scale for the distance between labels and the features
     * from the targetLayer().
     *
     * \see distanceUnitScale()
     * \see setDistance()
     */
    void setDistanceUnitScale( const QgsMapUnitScale &scale ) { mDistanceUnitScale = scale; }

    /**
     * Returns the penalty cost incurred when the rule is violated.
     *
     * This is a value between 0 and 10, where 10 indicates that the rule must never be violated,
     * and 1-9 = nice to have if possible, where higher numbers will try harder to avoid violating the rule.
     *
     * \see setCost()
     */
    double cost() const { return mCost; }

    /**
     * Sets the penalty \a cost incurred when the rule is violated.
     *
     * This is a value between 0 and 10, where 10 indicates that the rule must never be violated,
     * and 1-9 = nice to have if possible, where higher numbers will try harder to avoid violating the rule.
     *
     * \see cost()
     */
    void setCost( double cost ) { mCost = cost; }

  protected:

    void copyCommonProperties( QgsAbstractLabelingEngineRule *other ) const override;

    //! TRUE if labels must be distant from features, FALSE if they must be close
    bool mMustBeDistant = true;

  private:
#ifdef SIP_RUN
    QgsAbstractLabelingEngineRuleDistanceFromFeature( const QgsAbstractLabelingEngineRuleDistanceFromFeature &other );
#endif

    void initialize( QgsLabelingEngineContext &context );

    //! Returns TRUE if \a candidate is too close / too far from features from target layer
    bool candidateExceedsTolerance( const pal::LabelPosition *candidate, QgsLabelingEngineContext &context ) const;

    //! Labeled layer
    QgsMapLayerRef mLabeledLayer;
    //! Target layer
    QgsVectorLayerRef mTargetLayer;
    //! Distance threshold
    double mDistance = 5;
    //! Distance threshold unit
    Qgis::RenderUnit mDistanceUnit = Qgis::RenderUnit::Millimeters;
    //! Distance threshold map unit scale
    QgsMapUnitScale mDistanceUnitScale;
    //! Associated cost
    double mCost = 10;

    // cached variables
    double mDistanceMapUnits = 0;
    std::unique_ptr< QgsAbstractFeatureSource > mTargetLayerSource;
    std::unique_ptr< QgsSpatialIndex > mIndex;
    bool mInitialized = false;
};


/**
 * A labeling engine rule which prevents labels being placed too close to features from a different layer.
 *
 * \ingroup core
 * \since QGIS 3.40
 */
class CORE_EXPORT QgsLabelingEngineRuleMinimumDistanceLabelToFeature : public QgsAbstractLabelingEngineRuleDistanceFromFeature
{
  public:

    QgsLabelingEngineRuleMinimumDistanceLabelToFeature();
    ~QgsLabelingEngineRuleMinimumDistanceLabelToFeature() override;
    QgsLabelingEngineRuleMinimumDistanceLabelToFeature *clone() const override SIP_FACTORY;
    QString id() const override;
    QString displayType() const override;
    QString description() const override;
#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    const QString str = QStringLiteral( "<QgsLabelingEngineRuleMinimumDistanceLabelToFeature: %1>" ).arg( sipCpp->name() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

  private:
#ifdef SIP_RUN
    QgsLabelingEngineRuleMinimumDistanceLabelToFeature( const QgsLabelingEngineRuleMinimumDistanceLabelToFeature & );
#endif
};


/**
 * A labeling engine rule which prevents labels being placed too far from features from a different layer.
 *
 * \ingroup core
 * \since QGIS 3.40
 */
class CORE_EXPORT QgsLabelingEngineRuleMaximumDistanceLabelToFeature : public QgsAbstractLabelingEngineRuleDistanceFromFeature
{
  public:
    QgsLabelingEngineRuleMaximumDistanceLabelToFeature();
    ~QgsLabelingEngineRuleMaximumDistanceLabelToFeature() override;
    QgsLabelingEngineRuleMaximumDistanceLabelToFeature *clone() const override SIP_FACTORY;
    QString id() const override;
    QString displayType() const override;
    QString description() const override;

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    const QString str = QStringLiteral( "<QgsLabelingEngineRuleMaximumDistanceLabelToFeature: %1>" ).arg( sipCpp->name() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

  private:
#ifdef SIP_RUN
    QgsLabelingEngineRuleMaximumDistanceLabelToFeature( const QgsLabelingEngineRuleMaximumDistanceLabelToFeature & );
#endif

};

/**
 * A labeling engine rule which prevents labels being placed too close to labels from a different layer.
 *
 * \ingroup core
 * \since QGIS 3.40
 */
class CORE_EXPORT QgsLabelingEngineRuleMinimumDistanceLabelToLabel : public QgsAbstractLabelingEngineRule
{
  public:
    QgsLabelingEngineRuleMinimumDistanceLabelToLabel();
    ~QgsLabelingEngineRuleMinimumDistanceLabelToLabel() override;

    QgsLabelingEngineRuleMinimumDistanceLabelToLabel *clone() const override SIP_FACTORY;
    QString id() const override;
    QString displayType() const override;
    QString description() const override;
    bool isAvailable() const override;
    void writeXml( QDomDocument &doc, QDomElement &element, const QgsReadWriteContext &context ) const override;
    void readXml( const QDomElement &element, const QgsReadWriteContext &context ) override;
    void resolveReferences( const QgsProject *project ) override;
    bool prepare( QgsRenderContext &context ) override;
    QgsRectangle modifyCandidateConflictSearchBoundingBox( const QgsRectangle &candidateBounds ) const override SIP_SKIP;
    bool candidatesAreConflicting( const pal::LabelPosition *lp1, const pal::LabelPosition *lp2 ) const override SIP_SKIP;

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    const QString str = QStringLiteral( "<QgsLabelingEngineRuleMinimumDistanceLabelToLabel: %1>" ).arg( sipCpp->name() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

    /**
     * Returns the layer providing the labels.
     *
     * \see setLabeledLayer()
     */
    QgsMapLayer *labeledLayer() const;

    /**
     * Sets the \a layer providing the labels.
     *
     * \see labeledLayer()
     */
    void setLabeledLayer( QgsMapLayer *layer );

    /**
     * Returns the layer providing the labels which labels must be distant from.
     *
     * \see setTargetLayer()
     */
    QgsMapLayer *targetLayer() const;

    /**
     * Sets the \a layer providing the labels which labels must be distant from.
     *
     * \see targetLayer()
     */
    void setTargetLayer( QgsMapLayer *layer );

    /**
     * Returns the minimum permitted distance between labels from the labeledLayer() and the labels
     * from the targetLayer().
     *
     * \see setDistance()
     * \see distanceUnit()
     */
    double distance() const { return mDistance; }

    /**
     * Sets the minimum permitted \a distance between labels from the labeledLayer() and the labels
     * from the targetLayer().
     *
     * \see distance()
     * \see setDistanceUnit()
     */
    void setDistance( double distance ) { mDistance = distance; }

    /**
     * Returns the units for the distance between labels from the labeledLayer() and the labels
     * from the targetLayer().
     *
     * \see setDistanceUnit()
     * \see distance()
     */
    Qgis::RenderUnit distanceUnit() const { return mDistanceUnit; }

    /**
     * Sets the \a unit for the distance between labels from the labeledLayer() and the labels
     * from the targetLayer().
     *
     * \see distanceUnit()
     * \see setDistance()
     */
    void setDistanceUnit( Qgis::RenderUnit unit ) { mDistanceUnit = unit; }

    /**
     * Returns the scaling for the distance between labels from the labeledLayer() and the labels
     * from the targetLayer().
     *
     * \see setDistanceUnitScale()
     * \see distance()
     */
    const QgsMapUnitScale &distanceUnitScale() const { return mDistanceUnitScale; }

    /**
     * Sets the \a scale for the distance between labels from the labeledLayer() and the labels
     * from the targetLayer().
     *
     * \see distanceUnitScale()
     * \see setDistance()
     */
    void setDistanceUnitScale( const QgsMapUnitScale &scale ) { mDistanceUnitScale = scale; }

  private:
#ifdef SIP_RUN
    QgsLabelingEngineRuleMinimumDistanceLabelToLabel( const QgsLabelingEngineRuleMinimumDistanceLabelToLabel & );
#endif

    QgsMapLayerRef mLabeledLayer;
    QgsMapLayerRef mTargetLayer;
    double mDistance = 5;
    Qgis::RenderUnit mDistanceUnit = Qgis::RenderUnit::Millimeters;
    QgsMapUnitScale mDistanceUnitScale;

    // cached variables
    double mDistanceMapUnits = 0;
};


/**
 * A labeling engine rule which prevents labels being placed overlapping features from a different layer.
 *
 * \ingroup core
 * \since QGIS 3.40
 */
class CORE_EXPORT QgsLabelingEngineRuleAvoidLabelOverlapWithFeature : public QgsAbstractLabelingEngineRule
{
  public:

    QgsLabelingEngineRuleAvoidLabelOverlapWithFeature();
    ~QgsLabelingEngineRuleAvoidLabelOverlapWithFeature() override;
    QgsLabelingEngineRuleAvoidLabelOverlapWithFeature *clone() const override SIP_FACTORY;
    QString id() const override;
    QString displayType() const override;
    QString description() const override;
    bool prepare( QgsRenderContext &context ) override;
    void writeXml( QDomDocument &doc, QDomElement &element, const QgsReadWriteContext &context ) const override;
    void readXml( const QDomElement &element, const QgsReadWriteContext &context ) override;
    void resolveReferences( const QgsProject *project ) override;
    bool candidateIsIllegal( const pal::LabelPosition *candidate, QgsLabelingEngineContext &context ) const override SIP_SKIP;

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    const QString str = QStringLiteral( "<QgsLabelingEngineRuleAvoidLabelOverlapWithFeature: %1>" ).arg( sipCpp->name() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

    /**
     * Returns the layer providing the labels.
     *
     * \see setLabeledLayer()
     */
    QgsMapLayer *labeledLayer() const;

    /**
     * Sets the \a layer providing the labels.
     *
     * \see labeledLayer()
     */
    void setLabeledLayer( QgsMapLayer *layer );

    /**
     * Returns the layer providing the features which labels must not overlap.
     *
     * \see setTargetLayer()
     */
    QgsVectorLayer *targetLayer() const;

    /**
     * Sets the \a layer providing the features which labels must not overlap.
     *
     * \see targetLayer()
     */
    void setTargetLayer( QgsVectorLayer *layer );

  private:
#ifdef SIP_RUN
    QgsLabelingEngineRuleAvoidLabelOverlapWithFeature( const QgsLabelingEngineRuleAvoidLabelOverlapWithFeature & );
#endif
    void initialize( QgsLabelingEngineContext &context );

    QgsMapLayerRef mLabeledLayer;
    QgsVectorLayerRef mTargetLayer;

    // cached variables
    std::unique_ptr< QgsAbstractFeatureSource > mTargetLayerSource;
    std::unique_ptr< QgsSpatialIndex > mIndex;
    bool mInitialized = false;
};


#endif // QGSLABELINGENGINERULEIMPL_H
