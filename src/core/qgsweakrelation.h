/***************************************************************************
  qgsweakrelation.h - QgsWeakRelation

 ---------------------
 begin                : 5.12.2019
 copyright            : (C) 2019 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSWEAKRELATION_H
#define QGSWEAKRELATION_H

#include "qgis_core.h"
#include "qgsrelation.h"
#include "qgsvectorlayerref.h"

/**
 * The QgsWeakRelation class represent a QgsRelation with possibly
 * unresolved layers or unmatched fields.
 *
 * This class is used to store relation information attached to a
 * layer style, a method to attempt relation resolution is also
 * implemented and can be used to create a QgsRelation after the
 * dependent layers are loaded and available.
 *
 * In constrast to QgsRelation, QgsWeakRelation can be used to encapsulate
 * information about a relationship which does not currently exist in a QGIS project.
 * E.g. it can be used to represent a relationship which exists in a database
 * backend (but not within a QGIS project). Accordingly, some properties
 * available in QgsWeakRelation are included for informational purposes only,
 * and cannot be translated to QgsRelations or respected in QGIS relationships.
 *
 * \ingroup core
 * \since QGIS 3.12
 */
class CORE_EXPORT QgsWeakRelation
{
  public:

    /**
     * Enum to distinguish if the layer is referenced or referencing
     * \since QGIS 3.16
     */
    enum WeakRelationType
    {
      Referencing, //!< The layer is referencing (or the "child" / "right" layer in the relationship)
      Referenced //!< The layer is referenced (or the "parent" / "left" left in the relationship)
    };

    /**
     * Default constructor for an invalid relation.
     */
    QgsWeakRelation();

    /**
     * Creates a QgsWeakRelation.
     *
     * \since QGIS 3.30
     */
    QgsWeakRelation( const QString &relationId,
                     const QString &relationName,
                     const Qgis::RelationshipStrength strength,
                     const QString &referencingLayerId,
                     const QString &referencingLayerName,
                     const QString &referencingLayerSource,
                     const QString &referencingLayerProviderKey,
                     const QString &referencedLayerId,
                     const QString &referencedLayerName,
                     const QString &referencedLayerSource,
                     const QString &referencedLayerProviderKey
                   );

    /**
     * Resolves a weak relation in the given \a project returning a list of possibly invalid QgsRelations
     * and without performing any kind of validity check.
     *
     * \note Client code should never assume that the returned relations are valid and the
     * layer components are not NULL.
     */
#ifndef SIP_RUN
    QList< QgsRelation > resolvedRelations( const QgsProject *project, QgsVectorLayerRef::MatchType matchType = QgsVectorLayerRef::MatchType::All ) const;
#else
    QList< QgsRelation > resolvedRelations( const QgsProject *project ) const;
#endif

    /**
     * Returns the relationship's ID.
     *
     * \since QGIS 3.28
     */
    QString id() const { return mRelationId; }

    /**
     * Returns the relationship's name.
     *
     * \since QGIS 3.28
     */
    QString name() const { return mRelationName; }

    /**
     * Returns a weak reference to the referencing (or "child" / "right") layer.
     *
     * \note Not available in Python bindings.
     */
    QgsVectorLayerRef referencingLayer() const SIP_SKIP;

    /**
     * Returns the source URI for the referencing (or "child" / "right") layer.
     *
     * \since QGIS 3.28
     */
    QString referencingLayerSource() const;

    /**
     * Returns the provider ID for the referencing (or "child" / "right") layer.
     *
     * \since QGIS 3.28
     */
    QString referencingLayerProvider() const;

    /**
     * Returns the layer name of the referencing (or "child" / "right") layer.
     *
     * \note the layer name refers to the layer name used in the datasource, not in any associated
     * QgsVectorLayer.
     *
     * \since QGIS 3.28
     */
    QString referencingLayerName() const;

    /**
     * Returns a weak reference to the referenced (or "parent" / "left") layer.
     *
     * \note Not available in Python bindings.
     */
    QgsVectorLayerRef referencedLayer() const SIP_SKIP;

    /**
     * Returns the source URI for the referenced (or "parent" / "left") layer.
     *
     * \since QGIS 3.28
     */
    QString referencedLayerSource() const;

    /**
     * Returns the provider ID for the referenced (or "parent" / "left") layer.
     *
     * \since QGIS 3.28
     */
    QString referencedLayerProvider() const;

    /**
     * Returns the layer name of the referenced (or "parent" / "left") layer.
     *
     * \note the layer name refers to the layer name used in the datasource, not in any associated
     * QgsVectorLayer.
     *
     * \since QGIS 3.28
     */
    QString referencedLayerName() const;

    /**
     * Returns a weak reference to the mapping table, which forms the middle table in many-to-many relationships.
     *
     * \note Not available in Python bindings.
     *
     * \since QGIS 3.28
     */
    QgsVectorLayerRef mappingTable() const SIP_SKIP;

    /**
     * Sets a weak reference to the mapping \a table, which forms the middle table in many-to-many relationships.
     *
     * \note Not available in Python bindings.
     *
     * \since QGIS 3.28
     */
    void setMappingTable( const QgsVectorLayerRef &table ) SIP_SKIP;

    /**
     * Returns the source URI for the mapping table, which forms the middle table in many-to-many relationships.
     *
     * \since QGIS 3.28
     */
    QString mappingTableSource() const;

    /**
     * Returns the provider ID for the mapping table, which forms the middle table in many-to-many relationships.
     *
     * \since QGIS 3.28
     */
    QString mappingTableProvider() const;

    /**
     * Returns the layer name of the mapping table, which forms the middle table in many-to-many relationships.
     *
     * \note the layer name refers to the layer name used in the datasource, not in any associated
     * QgsVectorLayer.
     *
     * \since QGIS 3.28
     */
    QString mappingTableName() const;

    /**
     * Returns the list of fields from the referencingLayer() involved in the relationship.
     *
     * \since QGIS 3.28
     */
    QStringList referencingLayerFields() const { return mReferencingLayerFields; }

    /**
     * Sets the list of \a fields from the referencingLayer() involved in the relationship.
     *
     * \since QGIS 3.28
     */
    void setReferencingLayerFields( const QStringList &fields ) { mReferencingLayerFields = fields; }

    /**
     * Returns the list of fields from the mappingTable() involved in the relationship.
     *
     * These fields will be matched to the referencingLayerFields() in many-to-many joins.
     *
     * \since QGIS 3.28
     */
    QStringList mappingReferencingLayerFields() const { return mMappingReferencingLayerFields; }

    /**
     * Sets the list of \a fields from the mappingTable() involved in the relationship.
     *
     * These fields will be matched to the referencingLayerFields() in many-to-many joins.
     *
     * \since QGIS 3.28
     */
    void setMappingReferencingLayerFields( const QStringList &fields ) { mMappingReferencingLayerFields = fields; }

    /**
     * Returns the list of fields from the referencedLayer() involved in the relationship.
     *
     * \since QGIS 3.28
     */
    QStringList referencedLayerFields() const { return mReferencedLayerFields; }

    /**
     * Sets the list of \a fields from the referencedLayer() involved in the relationship.
     *
     * \since QGIS 3.28
     */
    void setReferencedLayerFields( const QStringList &fields ) { mReferencedLayerFields = fields; }

    /**
     * Returns the list of fields from the mappingTable() involved in the relationship.
     *
     * These fields will be matched to the referencedLayerFields() in many-to-many joins.
     *
     * \since QGIS 3.28
     */
    QStringList mappingReferencedLayerFields() const { return mMappingReferencedLayerFields; }

    /**
     * Sets the list of \a fields from the mappingTable() involved in the relationship.
     *
     * These fields will be matched to the referencedLayerFields() in many-to-many joins.
     *
     * \since QGIS 3.28
     */
    void setMappingReferencedLayerFields( const QStringList &fields ) { mMappingReferencedLayerFields = fields; }

    /**
     * Returns the strength of the relation.
     */
    Qgis::RelationshipStrength strength() const;

    /**
     * Returns the relationship's cardinality.
     *
     * \see setCardinality()
     * \since QGIS 3.28
     */
    Qgis::RelationshipCardinality cardinality() const { return mCardinality; }

    /**
     * Sets the relationship's \a cardinality.
     *
     * \see cardinality()
     * \since QGIS 3.28
     */
    void setCardinality( Qgis::RelationshipCardinality cardinality ) { mCardinality = cardinality; }

    /**
     * Returns the label of the forward path for the relationship.
     *
     * The forward and backward path labels are free-form, user-friendly strings
     * which can be used to generate descriptions of the relationship between features
     * from the right and left tables.
     *
     * E.g. when the left table contains buildings and the right table contains
     * furniture, the forward path label could be "contains" and the backward path
     * label could be "is located within". A client could then generate a
     * user friendly description string such as "fire hose 1234 is located within building 15a".
     *
     * \see setForwardPathLabel()
     * \see backwardPathLabel()
     *
     * \since QGIS 3.28
    */
    QString forwardPathLabel() const { return mForwardPathLabel; }

    /**
     * Returns the label of the backward path for the relationship.
     *
     * The forward and backward path labels are free-form, user-friendly strings
     * which can be used to generate descriptions of the relationship between features
     * from the right and left tables.
     *
     * E.g. when the left table contains buildings and the right table contains
     * furniture, the forward path label could be "contains" and the backward path
     * label could be "is located within". A client could then generate a
     * user friendly description string such as "fire hose 1234 is located within building 15a".
     *
     * \see setBackwardPathLabel()
     * \see forwardPathLabel()
     *
     * \since QGIS 3.28
    */
    QString backwardPathLabel() const { return mBackwardPathLabel; }

    /**
     * Sets the \a label of the forward path for the relationship.
     *
     * The forward and backward path labels are free-form, user-friendly strings
     * which can be used to generate descriptions of the relationship between features
     * from the right and left tables.
     *
     * E.g. when the left table contains buildings and the right table contains
     * furniture, the forward path label could be "contains" and the backward path
     * label could be "is located within". A client could then generate a
     * user friendly description string such as "fire hose 1234 is located within building 15a".
     *
     * \see forwardPathLabel()
     * \see setBackwardPathLabel()
     *
     * \since QGIS 3.28
    */
    void setForwardPathLabel( const QString &label ) { mForwardPathLabel = label; }

    /**
     * Sets the \a label of the backward path for the relationship.
     *
     * The forward and backward path labels are free-form, user-friendly strings
     * which can be used to generate descriptions of the relationship between features
     * from the right and left tables.
     *
     * E.g. when the left table contains buildings and the right table contains
     * furniture, the forward path label could be "contains" and the backward path
     * label could be "is located within". A client could then generate a
     * user friendly description string such as "fire hose 1234 is located within building 15a".
     *
     * \see backwardPathLabel()
     * \see setForwardPathLabel()
     *
     * \since QGIS 3.28
    */
    void setBackwardPathLabel( const QString &label ) { mBackwardPathLabel = label; }

    /**
     * Returns the type string of the related table.
     *
     * This a free-form string representing the type of related features, where the
     * exact interpretation is format dependent. For instance, table types from GeoPackage
     * relationships will directly reflect the categories from the GeoPackage related
     * tables extension (i.e. "media", "simple attributes", "features", "attributes" and "tiles").
     *
     * \see setRelatedTableType()
     * \since QGIS 3.28
    */
    QString relatedTableType() const { return mRelatedTableType; }

    /**
     * Sets the \a type string of the related table.
     *
     * This a free-form string representing the type of related features, where the
     * exact interpretation is format dependent. For instance, table types from GeoPackage
     * relationships will directly reflect the categories from the GeoPackage related
     * tables extension (i.e. "media", "simple attributes", "features", "attributes" and "tiles").
     *
     * \see relatedTableType()
     * \since QGIS 3.28
    */
    void setRelatedTableType( const QString &type ) { mRelatedTableType = type; }

    /**
     * Returns a weak relation for the given layer.
     *
     * \param layer the layer of the weak relation
     * \param type determines if the layer is referencing or referenced
     * \param node the QDomNode
     * \param resolver the path resolver
     * \since QGIS 3.16
     */
    static QgsWeakRelation readXml( const QgsVectorLayer *layer, WeakRelationType type, const QDomNode &node, const QgsPathResolver resolver );

    /**
     * Writes a weak relation infoto an XML structure. Used for saving .qgs projects
     *
     * \param layer the layer which we save the weak relation for
     * \param type determines if the layer is referencing or referenced
     * \param relation the relation to save as a weak relation
     * \param node The parent node in which the relation will be created
     * \param doc  The document in which the relation will be saved
     * \since QGIS 3.16
     */
    static void writeXml( const QgsVectorLayer *layer, WeakRelationType type, const QgsRelation &relation, QDomNode &node, QDomDocument &doc );

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode

    QString leftIdentifier;
    if ( !sipCpp->referencedLayer().source.isEmpty() )
      leftIdentifier = sipCpp->referencedLayer().source;

    QString rightIdentifier;
    if ( !sipCpp->referencingLayer().source.isEmpty() )
      rightIdentifier = sipCpp->referencingLayer().source;

    QString str;
    if ( leftIdentifier.isEmpty() && rightIdentifier.isEmpty() )
      str = QStringLiteral( "<QgsWeakRelation: %1>" ).arg( sipCpp->id() );
    else
      str = QStringLiteral( "<QgsWeakRelation: %1 - %2 -> %3>" ).arg( sipCpp->id(), leftIdentifier, rightIdentifier );

    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

  private:

    QgsVectorLayerRef mReferencingLayer;
    QgsVectorLayerRef mReferencedLayer;
    QgsVectorLayerRef mMappingTable;

    QString mRelationId;
    QString mRelationName;
    Qgis::RelationshipStrength mStrength = Qgis::RelationshipStrength::Association;

    QStringList mReferencingLayerFields;
    QStringList mMappingReferencingLayerFields;
    QStringList mReferencedLayerFields;
    QStringList mMappingReferencedLayerFields;

    Qgis::RelationshipCardinality mCardinality = Qgis::RelationshipCardinality::OneToMany;
    QString mForwardPathLabel;
    QString mBackwardPathLabel;
    QString mRelatedTableType;

    friend class TestQgsWeakRelation;

};

#endif // QGSWEAKRELATION_H
