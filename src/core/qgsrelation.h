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

class CORE_EXPORT QgsRelation
{
  public:
    /**
     * Defines a relation between matchin fields of the two involved tables of a relation.
     * Often, a relation is only defined by just one FieldPair with the name of the foreign key
     * column of the referencing table as first element and the name of the primary key column
     * of the referenced table as the second element.
     *
     */
    class FieldPair : public QPair< QString, QString >
    {
      public:
        //! Default constructor: NULL strings
        FieldPair()
            : QPair< QString, QString >() {}

        //! Constructor which takes two fields
        FieldPair( QString referencingField, QString referencedField )
            : QPair< QString, QString >( referencingField, referencedField ) {}

        //! Get the name of the referencing field
        const QString& referencingField() const { return first; }
        //! Get the name of the referenced field
        const QString& referencedField() const { return second; }
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
    void setRelationId( QString id );

    /**
     * Set a name for this relation
     *
     * @param name
     */
    void setRelationName( QString name );

    /**
     * Set the referencing layer id. This layer will be searched in the registry.
     *
     * @param id
     */
    void setReferencingLayer( QString id );

    /**
     * Set the referenced layer id. This layer will be searched in the registry.
     *
     * @param id
     */
    void setReferencedLayer( QString id );

    /**
     * Add a field pairs which is part of this relation
     * The first element of each pair are the field names fo the foreign key.
     * The second element of each pair are the field names of the matching primary key.
     *
     * @param referencingField  The field name on the referencing layer (FK)
     * @param referencedField   The field name on the referenced layer  (PK)
     */
    void addFieldPair( QString referencingField, QString referencedField );

    /**
     * Add a field pairs which is part of this relation
     * The first element of each pair are the field names fo the foreign key.
     * The second element of each pair are the field names of the matching primary key.
     *
     * @param fieldPair A pair of two strings
     * @note not available in python bindings
     */
    void addFieldPair( FieldPair fieldPair );

    /**
     * Creates an iterator which returns all the features on the referencing (child) layer
     * which have a foreign key pointing to the provided feature.
     *
     * @param feature A feature from the referenced (parent) layer
     *
     * @return An iterator with all the referenced features
     */
    QgsFeatureIterator getRelatedFeatures( const QgsFeature& feature ) const;

    /**
     * Creates a request to return all the features on the referencing (child) layer
     * which have a foreign key pointing to the provided feature.
     *
     * @param feature A feature from the referenced (parent) layer
     *
     * @return A request for all the referencing features
     */
    QgsFeatureRequest getRelatedFeaturesRequest( const QgsFeature& feature ) const;

    /**
     * Creates a request to return the feature on the referenced (parent) layer
     * which is referenced by the provided feature.
     *
     * @param attributes An attribute vector containing the foreign key
     *
     * @return A request the referenced feature
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
     * The first element of each pair are the field names fo the foreign key.
     * The second element of each pair are the field names of the matching primary key.
     *
     * @return The fields forming the relation
     */
    QList< FieldPair > fieldPairs() const;

    /**
     * Returns the validity of this relation. Don't use the information if it's not valid.
     *
     * @return true if the relation is valid
     */
    bool isValid() const;

  protected:
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
