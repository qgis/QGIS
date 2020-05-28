/***************************************************************************
                             qgsreportsectionfieldgroup.h
                             ---------------------------
    begin                : December 2017
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
#ifndef QGSREPORTSECTIONFIELDGROUP_H
#define QGSREPORTSECTIONFIELDGROUP_H

#include "qgis_core.h"
#include "qgsabstractreportsection.h"
#include "qgsfeatureiterator.h"


///@cond NOT_STABLE

// This is not considered stable API - it is exposed to python bindings only for unit testing!

/**
 * \ingroup core
 * \class QgsReportSectionFieldGroup
 * \brief A report section consisting of a features
 *
 * \warning This is not considered stable API, and may change in future QGIS releases. It is
 * exposed to the Python bindings for unit testing purposes only.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsReportSectionFieldGroup : public QgsAbstractReportSection
{
  public:

    /**
     * Visibility modes for header and footer sections
     */
    enum SectionVisibility
    {
      IncludeWhenFeaturesFound,      //!< The section will be included when features are found
      AlwaysInclude                  //!< The section will always be included
    };

    /**
     * Constructor for QgsReportSectionFieldGroup, attached to the specified \a parent section.
     * Note that ownership is not transferred to \a parent.
     */
    QgsReportSectionFieldGroup( QgsAbstractReportSection *parentSection = nullptr );

    QString type() const override { return QStringLiteral( "SectionFieldGroup" ); }
    QString description() const override;
    QIcon icon() const override;

    /**
     * Returns the body layout for the section.
     * \see setBody()
     * \see bodyEnabled()
     * \see setBodyEnabled()
     */
    QgsLayout *body() { return mBody.get(); }

    /**
     * Sets the \a body layout for the section. Ownership of \a body
     * is transferred to the report section.
     * \see body()
     * \see bodyEnabled()
     * \see setBodyEnabled()
     */
    void setBody( QgsLayout *body SIP_TRANSFER ) { mBody.reset( body ); }

    /**
     * Returns TRUE if the body for the section is enabled.
     * \see setBodyEnabled()
     * \see body()
     * \see setBody()
     */
    bool bodyEnabled() const { return mBodyEnabled; }

    /**
     * Sets whether the body for the section is \a enabled.
     * \see bodyEnabled()
     * \see body()
     * \see setBody()
     */
    void setBodyEnabled( bool enabled ) { mBodyEnabled = enabled; }


    /**
     * Returns the vector layer associated with this section.
     * \see setLayer()
     */
    QgsVectorLayer *layer() { return mCoverageLayer.get(); }

    /**
     * Sets the vector \a layer associated with this section.
     * \see layer()
     */
    void setLayer( QgsVectorLayer *layer ) { mCoverageLayer = layer; }

    /**
     * Returns the field associated with this section.
     * \see setField()
     */
    QString field() const { return mField; }

    /**
     * Sets the \a field associated with this section.
     * \see field()
     */
    void setField( const QString &field ) { mField = field; }

    /**
     * Returns TRUE if the field values should be sorted ascending,
     * or FALSE for descending sort.
     * \see setSortAscending()
     */
    bool sortAscending() const;

    /**
     * Sets whether the field values should be sorted ascending. Set to TRUE to sort
     * ascending, or FALSE for descending sort.
     * \see sortAscending()
     */
    void setSortAscending( bool sortAscending );

    /**
     * Returns the header visibility mode.
     * \see setHeaderVisibility()
     */
    SectionVisibility headerVisibility() const { return mHeaderVisibility; }

    /**
     * Sets the visibility mode for the header.
     * \see headerVisibility()
     */
    void setHeaderVisibility( SectionVisibility visibility ) { mHeaderVisibility = visibility; }

    /**
     * Returns the footer visibility mode.
     * \see setFooterVisibility()
     */
    SectionVisibility footerVisibility() const { return mFooterVisibility; }

    /**
     * Sets the visibility mode for the footer.
     * \see footerVisibility()
     */
    void setFooterVisibility( SectionVisibility visibility ) { mFooterVisibility = visibility; }

    QgsReportSectionFieldGroup *clone() const override SIP_FACTORY;
    bool beginRender() override;
    bool prepareHeader() override;
    bool prepareFooter() override;
    QgsLayout *nextBody( bool &ok ) override;
    void reset() override;
    void setParentSection( QgsAbstractReportSection *parentSection ) override;
    void reloadSettings() override;

  protected:

    bool writePropertiesToElement( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const override;
    bool readPropertiesFromElement( const QDomElement &element, const QDomDocument &document, const QgsReadWriteContext &context ) override;

  private:

    QgsVectorLayerRef mCoverageLayer;
    QString mField;
    bool mSortAscending = true;
    int mFieldIndex = -1;
    QgsFeatureIterator mFeatures;
    bool mSkipNextRequest = false;
    bool mNoFeatures = false;
    SectionVisibility mHeaderVisibility = IncludeWhenFeaturesFound;
    SectionVisibility mFooterVisibility = IncludeWhenFeaturesFound;
    QgsFeature mHeaderFeature;
    QgsFeature mLastFeature;
    QSet< QVariant > mEncounteredValues;

    bool mBodyEnabled = false;
    std::unique_ptr< QgsLayout > mBody;

    QgsFeatureRequest buildFeatureRequest() const;

    QgsFeature getNextFeature();
    void updateChildContexts( const QgsFeature &feature );

};


///@endcond

#endif //QGSREPORTSECTIONFIELDGROUP_H
