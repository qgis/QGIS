/***************************************************************************
    qgsrelation.h
     --------------------------------------
    Date                 : 29.4.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRELATION_H
#define QGSRELATION_H

#include <QList>
#include <QDomNode>
#include <QPair>

#include "qgsfield.h"
#include "qgsfeatureiterator.h"

class QgsVectorLayer;

/** \ingroup core
 * \class QgsRelation
 */
class CORE_EXPORT QgsRelation
{
  public:
    /**
     * \ingroup core
     * Defines a relation between matching fields of the two involved tables of a relation.
     * Often, a relation is only defined by just one FieldPair with the name of the foreign key
     * column of the referencing (child) table as first element and the name of the primary key column
     * of the referenced (parent) table as the second element.
     * @note not available in Python bindings
     */
    class FieldPair : public QPair< QString, QString >
    {
      public:
        //! Default constructor: NULL strings
        FieldPair()
            : QPair< QString, QString >() {}

        //! Constructor which takes two fields
        FieldPair( const QString& referencingField, const QString& referencedField )
            : QPair< QString, QString >( referencingField, referencedField ) {}

        //! Get the name of the referencing (child) field
        QString referencingField() const { return first; }
        //! Get the name of the referenced (parent) field
        QString referencedField() const { return second; }
    };

    /**
     * Default constructor. Creates an invalid relation.
     */
    QgsRelation();

    /**
     * Creates a relation from an XML structure. Used for reading .qgs projects.
     *
     * @param node The dom node containing the relation information
     *
     * @return A relation
     */
    static QgsRelation createFromXML( const QDomNode& node );

    /**
     * Writes a relation to an XML structure. Used for saving .qgs projects
     *
     * @param node The parent node in which the relation will be created
     * @param doc  The document in which the relation will be saved
     */
    void writeXML( QDomNode& node, QDomDocument& doc ) const;

    /**
     * Set a name for this relation
     *
     * @param id
     */
    void setRelationId( const QString& id );

    /**
     * Set a name for this relation
     *
     * @param name
     */
    void setRelationName( const QString& name );

    /**
     * Set the referencing (child) layer id. This layer will be searched in the registry.
     *
     * @param id
     */
    void setReferencingLayer( const QString& id );

    /**
     * Set the referenced (parent) layer id. This layer will be searched in the registry.
     *
     * @param id
     */
    void setReferencedLayer( const QString& id );

    /**
     * Add a field pairs which is part of this relation
     * The first element of each pair are the field names of the foreign key.
     * The second element of each pair are the field names of the matching primary key.
     *
     * @param referencingField  The field name on the referencing (child) layer (FK)
     * @param referencedField   The field name on the referenced (parent) layer  (PK)
     */
    void addFieldPair( const QString& referencingField, const QString& referencedField );

    /**
     * Add a field pairs which is part of this relation
     * The first element of each pair are the field names of the foreign key.
     * The second element of each pair are the field names of the matching primary key.
     *
     * @param fieldPair A pair of two strings
     * @note not available in python bindings
     */
    void addFieldPair( const FieldPair& fieldPair );

    /**
     * Creates an iterator which returns all the features on the referencing (child) layer
     * which have a foreign key pointing to the provided feature.
     *
     * @param feature A feature from the referenced (parent) layer
     *
     * @return An iterator with all the referenced features
     * @see getRelatedFeaturesRequest()
     * @see getRelatedFeaturesFilter()
     */
    QgsFeatureIterator getRelatedFeatures( const QgsFeature& feature ) const;

    /**
     * Creates a request to return all the features on the referencing (child) layer
     * which have a foreign key pointing to the provided feature.
     *
     * @param feature A feature from the referenced (parent) layer
     *
     * @return A request for all the referencing features
     * @see getRelatedFeatures()
     * @see getRelatedFeaturesFilter()
     */
    QgsFeatureRequest getRelatedFeaturesRequest( const QgsFeature& feature ) const;

    /** Returns a filter expression which returns all the features on the referencing (child) layer
     * which have a foreign key pointing to the provided feature.
     * @param feature A feature from the referenced (parent) layer
     * @return expression filter string for all the referencing features
     * @note added in QGIS 2.16
     * @see getRelatedFeatures()
     * @see getRelatedFeaturesRequest()
     */
    QString getRelatedFeaturesFilter( const QgsFeature& feature ) const;

    /**
     * Creates a request to return the feature on the referenced (parent) layer
     * which is referenced by the provided feature.
     *
     * @param attributes An attribute vector containing the foreign key
     *
     * @return A request the referenced feature
     * @note not available in python bindings
     */
    QgsFeatureRequest getReferencedFeatureRequest( const QgsAttributes& attributes ) const;

    /**
     * Creates a request to return the feature on the referenced (parent) layer
     * which is referenced by the provided feature.
     *
     * @param feature A feature from the referencing (child) layer
     *
     * @return A request the referenced feature
     */
    QgsFeatureRequest getReferencedFeatureRequest( const QgsFeature& feature ) const;

    /**
     * Creates a request to return the feature on the referenced (parent) layer
     * which is referenced by the provided feature.
     *
     * @param feature A feature from the referencing (child) layer
     *
     * @return A request the referenced feature
     */
    QgsFeature getReferencedFeature( const QgsFeature& feature ) const;

    /**
     * Returns a human readable name for this relation. Mostly used as title for the children.
     *
     * @see id()
     *
     * @return A name
     */
    QString name() const;

    /**
     * A (project-wide) unique id for this relation
     *
     * @return The id
     */
    QString id() const;

    /**
     * Access the referencing (child) layer's id
     * This is the layer which has the field(s) which point to another layer
     *
     * @return The id of the referencing layer
     */
    QString referencingLayerId() const;

    /**
     * Access the referencing (child) layer
     * This is the layer which has the field(s) which point to another layer
     *
     * @return The referencing layer
     */
    QgsVectorLayer* referencingLayer() const;

    /**
     * Access the referenced (parent) layer's id
     *
     * @return The id of the referenced layer
     */
    QString referencedLayerId() const;

    /**
     * Access the referenced (parent) layer
     *
     * @return referenced layer
     */
    QgsVectorLayer* referencedLayer() const;

    /**
     * Returns the field pairs which form this relation
     * The first element of each pair are the field names of the foreign key.
     * The second element of each pair are the field names of the matching primary key.
     *
     * @return The fields forming the relation
     */
    QList< FieldPair > fieldPairs() const;

    /**
     * Returns a list of attributes used to form the referenced fields
     * (most likely primary key) on the referenced (parent) layer.
     *
     * @return A list of attributes
     */
    QgsAttributeList referencedFields() const;

    /**
     * Returns a list of attributes used to form the referencing fields
     * (foreign key) on the referencing (child) layer.
     *
     * @return A list of attributes
     */
    QgsAttributeList referencingFields() const;

    /**
     * Returns the validity of this relation. Don't use the information if it's not valid.
     *
     * @return true if the relation is valid
     */
    bool isValid() const;

  protected:
    /**
     * Updates the validity status of this relation.
     * Will be called internally whenever a member is changed.
     */
    void updateRelationStatus();

  private:
    /** Unique Id */
    QString mRelationId;
    /** Human redable name*/
    QString mRelationName;
    /** The child layer */
    QString mReferencingLayerId;
    /** The child layer */
    QgsVectorLayer* mReferencingLayer;
    /** The parent layer id */
    QString mReferencedLayerId;
    /** The parent layer */
    QgsVectorLayer* mReferencedLayer;
    /** A list of fields which define the relation.
     *  In most cases there will be only one value, but multiple values
     *  are supported for composited foreign keys.
     *  The first field is on the referencing layer, the second on the referenced */
    QList< FieldPair > mFieldPairs;

    bool mValid;
};

// Register QgsRelation for usage with QVariant
Q_DECLARE_METATYPE( QgsRelation )

#endif // QGSRELATION_H
