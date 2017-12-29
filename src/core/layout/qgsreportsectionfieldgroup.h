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
     * Constructor for QgsReportSectionFieldGroup, attached to the specified \a parent section.
     * Note that ownership is not transferred to \a parent.
     */
    QgsReportSectionFieldGroup( QgsAbstractReportSection *parent = nullptr );

    QString type() const override { return QStringLiteral( "SectionFieldGroup" ); }

    /**
     * Returns the body layout for the section.
     * \see setBody()
     */
    QgsLayout *body() { return mBody.get(); }

    /**
     * Sets the \a body layout for the section. Ownership of \a body
     * is transferred to the report section.
     * \see body()
     */
    void setBody( QgsLayout *body SIP_TRANSFER ) { mBody.reset( body ); }

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
     * Returns true if the field values should be sorted ascending,
     * or false for descending sort.
     * \see setSortAscending()
     */
    bool sortAscending() const;

    /**
     * Sets whether the field values should be sorted ascending. Set to true to sort
     * ascending, or false for descending sort.
     * \see sortAscending()
     */
    void setSortAscending( bool sortAscending );

    QgsReportSectionFieldGroup *clone() const override SIP_FACTORY;
    bool beginRender() override;
    QgsLayout *nextBody( bool &ok ) override;
    void reset() override;
    void setParentSection( QgsAbstractReportSection *parent ) override;

  protected:

    bool writePropertiesToElement( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const override;
    bool readPropertiesFromElement( const QDomElement &element, const QDomDocument &document, const QgsReadWriteContext &context ) override;

  private:

    QgsVectorLayerRef mCoverageLayer;
    QString mField;
    bool mSortAscending = true;
    int mFieldIndex = -1;
    QgsFeatureIterator mFeatures;
    QSet< QVariant > mEncounteredValues;

    std::unique_ptr< QgsLayout > mBody;

    QgsFeatureRequest buildFeatureRequest() const;

    QgsFeature getNextFeature();
    void updateChildContexts( const QgsFeature &feature );

};


///@endcond

#endif //QGSREPORTSECTIONFIELDGROUP_H
