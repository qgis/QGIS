/***************************************************************************
    qgslabelingenginerule.h
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
#ifndef QGSLABELINGENGINERULE_H
#define QGSLABELINGENGINERULE_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgis.h"
#include "qgsgeometry.h"

class QgsRenderContext;
class QDomDocument;
class QDomElement;
class QgsReadWriteContext;
class QgsProject;
#ifndef SIP_RUN
namespace pal
{
  class LabelPosition;
}
#endif

/**
 * \ingroup core
 * \brief Encapsulates the context for a labeling engine run.
 * \since QGIS 3.40
 */
class CORE_EXPORT QgsLabelingEngineContext
{
  public:

    /**
     * Constructor for QgsLabelingEngineContext.
     */
    QgsLabelingEngineContext( QgsRenderContext &renderContext );

#ifndef SIP_RUN
    QgsLabelingEngineContext( const QgsLabelingEngineContext &other ) = delete;
    QgsLabelingEngineContext &operator=( const QgsLabelingEngineContext &other ) = delete;
#endif

    /**
     * Returns a reference to the context's render context.
     */
    QgsRenderContext &renderContext() { return mRenderContext; }

    /**
     * Returns a reference to the context's render context.
     * \note Not available in Python bindings.
     */
    const QgsRenderContext &renderContext() const SIP_SKIP { return mRenderContext; }

    /**
     * Returns the map extent defining the limits for labeling.
     *
     * \see mapBoundaryGeometry()
     * \see setExtent()
     */
    QgsRectangle extent() const;

    /**
     * Sets the map \a extent defining the limits for labeling.
     *
     * \see setMapBoundaryGeometry()
     * \see extent()
     */
    void setExtent( const QgsRectangle &extent );

    /**
     * Returns the map label boundary geometry, which defines the limits within which labels may be placed
     * in the map.
     *
     * The map boundary geometry specifies the actual geometry of the map
     * boundary, which will be used to detect whether a label is visible (or partially visible) in
     * the rendered map. This may differ from extent() in the case of rotated or non-rectangular
     * maps.
     *
     * \see setMapBoundaryGeometry()
     * \see extent()
     */
    QgsGeometry mapBoundaryGeometry() const;

    /**
     * Sets the map label boundary \a geometry, which defines the limits within which labels may be placed
     * in the map.
     *
     * The map boundary geometry specifies the actual geometry of the map
     * boundary, which will be used to detect whether a label is visible (or partially visible) in
     * the rendered map. This may differ from extent() in the case of rotated or non-rectangular
     * maps.
     *
     * \see setExtent()
     * \see mapBoundaryGeometry()
     */
    void setMapBoundaryGeometry( const QgsGeometry &geometry );

  private:

#ifdef SIP_RUN
    QgsLabelingEngineContext( const QgsLabelingEngineContext &other );
#endif

    QgsRenderContext &mRenderContext;
    QgsRectangle mExtent;
    QgsGeometry mMapBoundaryGeometry;
};

/**
 * Abstract base class for labeling engine rules.
 *
 * Labeling engine rules implement custom logic to modify the labeling solution for a map render,
 * e.g. by preventing labels being placed which violate custom constraints.
 *
 * \note QgsAbstractLabelingEngineRule cannot be subclassed in Python. Use one of the existing
 * implementations of this class instead.
 *
 * \ingroup core
 * \since QGIS 3.40
 */
class CORE_EXPORT QgsAbstractLabelingEngineRule
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( sipCpp->id() == "minimumDistanceLabelToFeature" )
    {
      sipType = sipType_QgsLabelingEngineRuleMinimumDistanceLabelToFeature;
    }
    else if ( sipCpp->id() == "minimumDistanceLabelToLabel" )
    {
      sipType = sipType_QgsLabelingEngineRuleMinimumDistanceLabelToLabel;
    }
    else if ( sipCpp->id() == "maximumDistanceLabelToFeature" )
    {
      sipType = sipType_QgsLabelingEngineRuleMaximumDistanceLabelToFeature;
    }
    else if ( sipCpp->id() == "avoidLabelOverlapWithFeature" )
    {
      sipType = sipType_QgsLabelingEngineRuleAvoidLabelOverlapWithFeature;
    }
    else
    {
      sipType = 0;
    }
    SIP_END
#endif

  public:

    virtual ~QgsAbstractLabelingEngineRule();

    /**
     * Creates a clone of this rule.
     *
     * The caller takes ownership of the returned object.
     */
    virtual QgsAbstractLabelingEngineRule *clone() const = 0 SIP_FACTORY;

    /**
     * Returns a string uniquely identifying the rule subclass.
     */
    virtual QString id() const = 0;

    /**
     * Returns a user-friendly, translated string representing the rule type.
     */
    virtual QString displayType() const = 0;

    /**
     * Returns TRUE if the rule is available for use within the current QGIS environment.
     *
     * The base class method returns TRUE.
     *
     * Rules can return FALSE if required dependencies are not available, e.g. if a library version
     * is too old for the rule.
     */
    virtual bool isAvailable() const;

    /**
     * Returns a user-friendly description of the rule.
     *
     * This should include the rule name() if set, and other useful details for users
     * to quickly identify the rule's purpose when shown in a tooltip.
     *
     * The returned string may contain HTML formatting.
     */
    virtual QString description() const;

    /**
     * Returns the name for this instance of the rule.
     *
     * The name is a user-configurable value which helps them identify and describe the
     * rule within their projects.
     *
     * \see setName()
     */
    QString name() const { return mName; }

    /**
     * Sets the \a name for this instance of the rule.
     *
     * The name is a user-configurable value which helps them identify and describe the
     * rule within their projects.
     *
     * \see name()
     */
    void setName( const QString &name ) { mName = name; }

    /**
     * Returns TRUE if the rule is active.
     *
     * \see setActive()
     */
    bool active() const;

    /**
     * Sets whether the rule is \a active.
     *
     * \see active()
     */
    void setActive( bool active );

    /**
     * Prepares the rule.
     *
     * This must be called on the main render thread, prior to commencing the render operation. Thread sensitive
     * logic (such as creation of feature sources) can be performed in this method.
     */
    virtual bool prepare( QgsRenderContext &context ) = 0;

    /**
     * Writes the rule properties to an XML \a element.
     *
     * \see readXml()
     */
    virtual void writeXml( QDomDocument &doc, QDomElement &element, const QgsReadWriteContext &context ) const = 0;

    /**
     * Reads the rule properties from an XML \a element.
     *
     * \see resolveReferences()
     * \see writeXml()
     */
    virtual void readXml( const QDomElement &element, const QgsReadWriteContext &context ) = 0;

    /**
     * Resolves reference to layers from stored layer ID.
     *
     * Should be called following a call readXml().
     */
    virtual void resolveReferences( const QgsProject *project );

    /**
     * Returns TRUE if a labeling candidate \a lp1 conflicts with \a lp2 after applying the rule.
     *
     * The default implementation returns FALSE.
     */
    virtual bool candidatesAreConflicting( const pal::LabelPosition *lp1, const pal::LabelPosition *lp2 ) const SIP_SKIP;

    /**
     * Returns a (possibly expanded) bounding box to use when searching for conflicts for a candidate.
     *
     * The return value is permitted to grow the bounding box, but may NOT shrink it.
     *
     * The default implementation returns the same bounds.
     */
    virtual QgsRectangle modifyCandidateConflictSearchBoundingBox( const QgsRectangle &candidateBounds ) const SIP_SKIP;

    /**
     * Returns TRUE if a labeling \a candidate violates the rule and should be eliminated.
     *
     * The default implementation returns FALSE.
     */
    virtual bool candidateIsIllegal( const pal::LabelPosition *candidate, QgsLabelingEngineContext &context ) const SIP_SKIP;

    /**
     * Provides an opportunity for the rule to alter the cost for a \a candidate.
     *
     * The default implementation does nothing.
     */
    virtual void alterCandidateCost( pal::LabelPosition *candidate, QgsLabelingEngineContext &context ) const SIP_SKIP;

  protected:

    /**
     * Copies common properties from this object to an \a other.
     */
    virtual void copyCommonProperties( QgsAbstractLabelingEngineRule *other ) const;

  private:

    QString mName;
    bool mIsActive = true;

};

#endif // QGSLABELINGENGINESETTINGS_H
