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

#include "qgis_core.h"
#include "qgsreadwritecontext.h"
#include "qgsrelationcontext.h"
#include "qgsattributes.h"

#include "qgis_sip.h"

class QgsFeatureIterator;
class QgsFeature;
class QgsFeatureRequest;
class QgsVectorLayer;
class QgsRelationPrivate;
class QgsPolymorphicRelation;

/**
 * \ingroup core
 * \class QgsRelation
 *
 * \brief Represents a relationship between two vector layers.
 */
class CORE_EXPORT QgsRelation
{
    Q_GADGET

    Q_PROPERTY( QString id READ id WRITE setId )
    Q_PROPERTY( QgsVectorLayer *referencingLayer READ referencingLayer )
    Q_PROPERTY( QgsVectorLayer *referencedLayer READ referencedLayer )
    Q_PROPERTY( QString name READ name WRITE setName )
    Q_PROPERTY( bool isValid READ isValid )
    Q_PROPERTY( QString polymorphicRelationId READ polymorphicRelationId WRITE setPolymorphicRelationId )
    Q_PROPERTY( QgsPolymorphicRelation polymorphicRelation READ polymorphicRelation )

  public:

#ifndef SIP_RUN

    /**
     * \ingroup core
     * \brief Defines a relation between matching fields of the two involved tables of a relation.
     *
     * Often, a relation is only defined by just one FieldPair with the name of the foreign key
     * column of the referencing (child) table as first element and the name of the primary key column
     * of the referenced (parent) table as the second element.
     * \note not available in Python bindings
     */
    class FieldPair : public QPair< QString, QString >
    {
      public:
        //! Default constructor: NULL strings
        FieldPair() = default;

        //! Constructor which takes two fields
        FieldPair( const QString &referencingField, const QString &referencedField )
          : QPair< QString, QString >( referencingField, referencedField ) {}

        //! Gets the name of the referencing (child) field
        QString referencingField() const { return first; }
        //! Gets the name of the referenced (parent) field
        QString referencedField() const { return second; }

        bool operator==( const FieldPair &other ) const { return first == other.first && second == other.second; }
    };
#endif

    /**
     * Default constructor. Creates an invalid relation.
     */
    QgsRelation();
    ~QgsRelation();

    /**
     * Constructor with context. Creates an invalid relation.
     */
    QgsRelation( const QgsRelationContext &context );

    /**
     * Copies a relation.
     * This makes a shallow copy, relations are implicitly shared and only duplicated when the copy is
     * changed.
     */
    QgsRelation( const QgsRelation &other );

    /**
     * Copies a relation.
     * This makes a shallow copy, relations are implicitly shared and only duplicated when the copy is
     * changed.
     */
    QgsRelation &operator=( const QgsRelation &other );

    /**
     * Creates a relation from an XML structure. Used for reading .qgs projects.
     *
     * \param node The dom node containing the relation information
     * \param context to pass project translator
     * \param relationContext a relation context
     *
     * \returns A relation
     */
    static QgsRelation createFromXml( const QDomNode &node, QgsReadWriteContext &context, const QgsRelationContext &relationContext = QgsRelationContext() );

    /**
     * Writes a relation to an XML structure. Used for saving .qgs projects
     *
     * \param node The parent node in which the relation will be created
     * \param doc  The document in which the relation will be saved
     */
    void writeXml( QDomNode &node, QDomDocument &doc ) const;

    /**
     * Set an id for this relation
     */
    void setId( const QString &id );

    /**
     * Set a name for this relation
     */
    void setName( const QString &name );

    /**
     * Set a strength for this relation
     */
    void setStrength( Qgis::RelationshipStrength strength );

    /**
     * Set the referencing (child) layer id. This layer will be searched in the registry.
     */
    void setReferencingLayer( const QString &id );

    /**
     * Set the referenced (parent) layer id. This layer will be searched in the registry.
     */
    void setReferencedLayer( const QString &id );

    /**
     * Add a field pair which is part of this relation
     * The first element of each pair are the field names of the foreign key.
     * The second element of each pair are the field names of the matching primary key.
     *
     * \param referencingField  The field name on the referencing (child) layer (FK)
     * \param referencedField   The field name on the referenced (parent) layer  (PK)
     */
    void addFieldPair( const QString &referencingField, const QString &referencedField );

    /**
     * Add a field pair which is part of this relation
     * The first element of each pair are the field names of the foreign key.
     * The second element of each pair are the field names of the matching primary key.
     *
     * \param fieldPair A pair of two strings
     * \note not available in Python bindings
     */
    void addFieldPair( const FieldPair &fieldPair ) SIP_SKIP;

    /**
     * Creates an iterator which returns all the features on the referencing (child) layer
     * which have a foreign key pointing to the provided feature.
     *
     * \param feature A feature from the referenced (parent) layer
     *
     * \returns An iterator with all the referenced features
     * \see getRelatedFeaturesRequest()
     * \see getRelatedFeaturesFilter()
     */
    QgsFeatureIterator getRelatedFeatures( const QgsFeature &feature ) const;

    /**
     * Creates a request to return all the features on the referencing (child) layer
     * which have a foreign key pointing to the provided feature.
     *
     * \param feature A feature from the referenced (parent) layer
     *
     * \returns A request for all the referencing features
     * \see getRelatedFeatures()
     * \see getRelatedFeaturesFilter()
     */
    QgsFeatureRequest getRelatedFeaturesRequest( const QgsFeature &feature ) const;

    /**
     * Returns a filter expression which returns all the features on the referencing (child) layer
     * which have a foreign key pointing to the provided feature.
     * \param feature A feature from the referenced (parent) layer
     * \returns expression filter string for all the referencing features
     * \see getRelatedFeatures()
     * \see getRelatedFeaturesRequest()
     */
    QString getRelatedFeaturesFilter( const QgsFeature &feature ) const;

    /**
     * Creates a request to return the feature on the referenced (parent) layer
     * which is referenced by the provided feature.
     *
     * \param attributes An attribute vector containing the foreign key
     *
     * \returns A request the referenced feature
     */
    QgsFeatureRequest getReferencedFeatureRequest( const QgsAttributes &attributes ) const;

    /**
     * Creates a request to return the feature on the referenced (parent) layer
     * which is referenced by the provided feature.
     *
     * \param feature A feature from the referencing (child) layer
     *
     * \returns A request the referenced feature
     */
    QgsFeatureRequest getReferencedFeatureRequest( const QgsFeature &feature ) const;

    /**
     * Creates a request to return the feature on the referenced (parent) layer
     * which is referenced by the provided feature.
     *
     * \param feature A feature from the referencing (child) layer
     *
     * \returns A request the referenced feature
     */
    QgsFeature getReferencedFeature( const QgsFeature &feature ) const;

    /**
     * Returns a human readable name for this relation. Mostly used as title for the children.
     *
     * \see id()
     *
     * \returns A name
     */
    QString name() const;

    /**
     * Returns the relation strength as a string
     *
     * \returns strength
     */
    Qgis::RelationshipStrength strength() const;

    /**
     * A (project-wide) unique id for this relation
     *
     * \returns The id
     */
    QString id() const;

    /**
     * Generate a (project-wide) unique id for this relation
     */
    void generateId();

    /**
     * Access the referencing (child) layer's id
     * This is the layer which has the field(s) which point to another layer
     *
     * \returns The id of the referencing layer
     */
    QString referencingLayerId() const;

    /**
     * Access the referencing (child) layer
     * This is the layer which has the field(s) which point to another layer
     *
     * \returns The referencing layer
     */
    QgsVectorLayer *referencingLayer() const;

    /**
     * Access the referenced (parent) layer's id
     *
     * \returns The id of the referenced layer
     */
    QString referencedLayerId() const;

    /**
     * Access the referenced (parent) layer
     *
     * \returns referenced layer
     */
    QgsVectorLayer *referencedLayer() const;

    /**
     * Returns the field pairs which form this relation
     * The first element of each pair are the field names of the foreign key.
     * The second element of each pair are the field names of the matching primary key.
     *
     * \returns The fields forming the relation
     */
#ifndef SIP_RUN
    QList< QgsRelation::FieldPair > fieldPairs() const;
#else
    QMap< QString, QString > fieldPairs() const;
    % MethodCode
    const QList< QgsRelation::FieldPair > &pairs = sipCpp->fieldPairs();
    sipRes = new QMap< QString, QString >();
    for ( const QgsRelation::FieldPair &pair : pairs )
    {
      sipRes->insert( pair.first, pair.second );
    }
    % End
#endif

    /**
     * Returns a list of attributes used to form the referenced fields
     * (most likely primary key) on the referenced (parent) layer.
     *
     * \returns A list of attributes
     */
    QgsAttributeList referencedFields() const;

    /**
     * Returns a list of attributes used to form the referencing fields
     * (foreign key) on the referencing (child) layer.
     *
     * \returns A list of attributes
     */
    QgsAttributeList referencingFields() const;

    /**
     * Returns TRUE if none of the referencing fields has a NOT NULL constraint.
     * \since QGIS 3.28
     */
    bool referencingFieldsAllowNull() const;

    /**
     * Returns the validity of this relation. Don't use the information if it's not valid.
     * A relation is considered valid if both referenced and referencig layers are valid.
     *
     * \returns TRUE if the relation is valid
     *
     * \see validationError()
     */
    bool isValid() const;

    /**
     * Returns a user-friendly explanation for why the relationship is invalid.
     *
     * Returns an empty string if the relationship isValid().
     *
     * \see isValid()
     * \since QGIS 3.28
     */
    QString validationError() const;

    /**
     * Compares the two QgsRelation, ignoring the name and the ID.
     *
     * \param other The other relation
     * \returns TRUE if they are similar
     */
    bool hasEqualDefinition( const QgsRelation &other ) const;

    /**
     * Gets the referenced field counterpart given a referencing field.
     *
     */
    Q_INVOKABLE QString resolveReferencedField( const QString &referencingField ) const;

    /**
     * Gets the referencing field counterpart given a referenced field.
     *
     */
    Q_INVOKABLE QString resolveReferencingField( const QString &referencedField ) const;

    /**
     * Updates the validity status of this relation.
     * Will be called internally whenever a member is changed.
     *
     * \since QGIS 3.6
     */
    void updateRelationStatus();

    /**
     * Sets the parent polymorphic relation id.
     * \since QGIS 3.18
     */
    void setPolymorphicRelationId( const QString &polymorphicRelationId );

    /**
     * Returns the parent polymorphic relation id. If the relation is a normal relation, a null string is returned.
     * \since QGIS 3.18
     */
    QString polymorphicRelationId() const;

    /**
     * Returns the parent polymorphic relation. If the relation is a normal relation, an invalid polymorphic relation is returned.
     * \since QGIS 3.18
     */
    QgsPolymorphicRelation polymorphicRelation() const;

    /**
     * Returns the type of the relation
     * \since QGIS 3.18
     */
    Qgis::RelationshipType type() const;

    /**
     * Returns a user-friendly translated string representing a relationship \a cardinality.
     *
     * \since QGIS 3.28
     */
    static QString cardinalityToDisplayString( Qgis::RelationshipCardinality cardinality );

    /**
     * Returns a user-friendly translated string representing a relationship \a strength.
     *
     * \since QGIS 3.28
     */
    static QString strengthToDisplayString( Qgis::RelationshipStrength strength );

  private:

    mutable QExplicitlySharedDataPointer<QgsRelationPrivate> d;

    QgsRelationContext mContext;
};

// Register QgsRelation for usage with QVariant
Q_DECLARE_METATYPE( QgsRelation )

#endif // QGSRELATION_H
