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

#ifndef SIP_RUN

    /**
     * Creates a QgsWeakRelation.
     *
     * \note Not available in Python bindings.
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
                     const QString &referencedLayerProviderKey,
                     const QList<QgsRelation::FieldPair> &fieldPairs
                   );
#endif

    /**
     * Resolves a weak relation in the given \a project returning a possibly invalid QgsRelation
     * and without performing any kind of validity check.
     *
     * \note Client code should never assume that the returned relation is valid and the
     * layer components are not NULL.
     */
#ifndef SIP_RUN
    QgsRelation resolvedRelation( const QgsProject *project, QgsVectorLayerRef::MatchType matchType = QgsVectorLayerRef::MatchType::All ) const;
#else
    QgsRelation resolvedRelation( const QgsProject *project ) const;
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
     * Returns the strength of the relation.
     */
    Qgis::RelationshipStrength strength() const;

    /**
     * Returns the list of field pairs.
     */
#ifndef SIP_RUN
    QList<QgsRelation::FieldPair> fieldPairs() const;
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
    QString mRelationId;
    QString mRelationName;
    Qgis::RelationshipStrength mStrength = Qgis::RelationshipStrength::Association;
    QList<QgsRelation::FieldPair> mFieldPairs;

    friend class TestQgsWeakRelation;

};

#endif // QGSWEAKRELATION_H
