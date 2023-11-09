/***************************************************************************
    qgspolymorphicrelation.h
     --------------------------------------
    Date                 : December 2020
    Copyright            : (C) 2020 Ivan Ivanov
    Email                : ivan at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOLYMORPHICRELATION_H
#define QGSPOLYMORPHICRELATION_H

#include <QList>
#include <QDomNode>
#include <QPair>

#include "qgis_core.h"
#include "qgsfields.h"
#include "qgsreadwritecontext.h"
#include "qgsrelationcontext.h"
#include "qgsrelation.h"

#include "qgis_sip.h"

class QgsFeatureIterator;
class QgsFeature;
class QgsFeatureRequest;
class QgsAttributes;
class QgsVectorLayer;
class QgsPolymorphicRelationPrivate;
class QgsExpressionContext;

/**
 * \brief A polymorphic relation consists of the same properties like a normal relation except for the referenced layer which is calculated based on one or several fields of the referencing layer.
 *
 * In its most simple form, the referencing layer will just insert the layer name of the referenced layer into this field.
 * To be more precise, a polymorphic relation is a set of normal relations having the same referencing layer but having the referenced layer dynamically defined.
 * The polymorphic setting of the layer is solved by using an expression which has to match some properties of the the referenced layer like the table name, schema, uri, layer id, ...
 * \ingroup core
 * \class QgsPolymorphicRelation
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsPolymorphicRelation
{
    Q_GADGET

    Q_PROPERTY( QString id READ id WRITE setId )
    Q_PROPERTY( QgsVectorLayer *referencingLayer READ referencingLayer )
    Q_PROPERTY( QString referencedLayerField READ referencedLayerField )
    Q_PROPERTY( QString referencedLayerExpression READ referencedLayerExpression )
    Q_PROPERTY( QString name READ name WRITE setName )
    Q_PROPERTY( bool isValid READ isValid )

  public:

    /**
     * Default constructor. Creates an invalid relation.
     */
    QgsPolymorphicRelation();
    ~QgsPolymorphicRelation();

    /**
     * Constructor with context. Creates an invalid relation.
     */
    QgsPolymorphicRelation( const QgsRelationContext &context );

    /**
     * Copies a relation.
     * This makes a shallow copy, relations are implicitly shared and only duplicated when the copy is
     * changed.
     */
    QgsPolymorphicRelation( const QgsPolymorphicRelation &other );

    /**
     * Copies a relation.
     * This makes a shallow copy, relations are implicitly shared and only duplicated when the copy is
     * changed.
     */
    QgsPolymorphicRelation &operator=( const QgsPolymorphicRelation &other );

    /**
     * Creates a relation from an XML structure. Used for reading .qgs projects.
     *
     * \param node The dom node containing the relation information
     * \param context to pass project translator
     * \param relationContext a relation context
     *
     * \returns A relation
     */
    static QgsPolymorphicRelation createFromXml( const QDomNode &node, QgsReadWriteContext &context, const QgsRelationContext &relationContext = QgsRelationContext() );

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
     * Set the referencing (child) layer id. This layer will be searched in the registry.
     */
    void setReferencingLayer( const QString &id );

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
    void addFieldPair( const QgsRelation::FieldPair &fieldPair ) SIP_SKIP;

    /**
     * Returns a human readable name for this relation. Mostly used as title for the children.
     *
     * \see id()
     *
     * \returns A name
     */
    QString name() const;

    /**
     * A (project-wide) unique id for this relation
     *
     * \returns The id
     */
    QString id() const;

    /**
     * Generate a (project-wide) unique id for this relation
     * \since QGIS 3.0
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
    QgsAttributeList referencedFields( const QString &layerId ) const;

    /**
     * Returns a list of attributes used to form the referencing fields
     * (foreign key) on the referencing (child) layer.
     *
     * \returns A list of attributes
     */
    QgsAttributeList referencingFields() const;

    /**
     * Returns the validity of this relation. Don't use the information if it's not valid.
     * A relation is considered valid if both referenced and referencig layers are valid.
     *
     * \returns TRUE if the relation is valid
     */
    bool isValid() const;

    /**
     * Compares the two QgsRelation, ignoring the name and the ID.
     *
     * \param other The other relation
     * \returns TRUE if they are similar
     */
    bool hasEqualDefinition( const QgsPolymorphicRelation &other ) const;

    /**
     * Updates the validity status of this relation.
     * Will be called internally whenever a member is changed.
     */
    void updateRelationStatus();

    /**
     * Sets the field in the referencing layer where the referenced layer identifier is stored
     */
    void setReferencedLayerField( const QString &referencedLayerField );

    /**
     * Returns the field in the referencing layer where the referenced layer identifier is stored
     */
    QString referencedLayerField() const;

    /**
      * Sets the \a expression to identify the parent layer
      */
    void setReferencedLayerExpression( const QString &expression );

    /**
      * Returns the expression to identify the parent layer
      */
    QString referencedLayerExpression() const;

    /**
     * Sets a list of layer ids to be used as potential referenced layers
     */
    void setReferencedLayerIds( const QStringList &childRelationIds );

    /**
     * Returns a list of layer ids to be used as potential referenced layers
     */
    QStringList referencedLayerIds() const;

    /**
     * Returns a list of generated relations, based on the currently set referencedLayerIds()
     */
    QList<QgsRelation> generateRelations() const;

    /**
     * Returns layer representation as evaluated string
     */
    QString layerRepresentation( const QgsVectorLayer *layer ) const;

    /**
     * Returns the relation strength for all the generated normal relations
     */
    Qgis::RelationshipStrength strength() const;

    /**
     * Sets the relation strength for all the generated normal relations
     */
    void setRelationStrength( Qgis::RelationshipStrength relationStrength );

  private:

    QExplicitlySharedDataPointer<QgsPolymorphicRelationPrivate> d;

    QgsRelationContext mContext;

};

// Register QgsPolymorphicRelation for usage with QVariant
Q_DECLARE_METATYPE( QgsPolymorphicRelation )

#endif // QGSPOLYMORPHICRELATION_H
