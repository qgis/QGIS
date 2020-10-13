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

#define SIP_NO_FILE

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
 * \note not available in Python bindings
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
      Referencing, //!< The layer is referencing
      Referenced //!< The layer is referenced
    };


    /**
     * Creates a QgsWeakRelation
     */
    QgsWeakRelation( const QString &relationId,
                     const QString &relationName,
                     const QgsRelation::RelationStrength strength,
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

    /**
     * Resolves a weak relation in the given \a project returning a possibly invalid QgsRelation
     * and without performing any kind of validity check.
     *
     * \note Client code should never assume that the returned relation is valid and the
     * layer components are not NULL.
     */
    QgsRelation resolvedRelation( const QgsProject *project, QgsVectorLayerRef::MatchType matchType = QgsVectorLayerRef::MatchType::All ) const;

    /**
     * Returns a weak reference to the referencing layer
     */
    QgsVectorLayerRef referencingLayer() const;

    /**
     * Returns a weak reference to the referenced layer
     */
    QgsVectorLayerRef referencedLayer() const;

    /**
     * Returns the strength of the relation
     */
    QgsRelation::RelationStrength strength() const;

    /**
     * Returns the list of field pairs
     */
    QList<QgsRelation::FieldPair> fieldPairs() const;

    /**
     * Returns a weak relation for the given layer
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

  private:

    QgsVectorLayerRef mReferencingLayer;
    QgsVectorLayerRef mReferencedLayer;
    QString mRelationId;
    QString mRelationName;
    QgsRelation::RelationStrength mStrength = QgsRelation::RelationStrength::Association;
    QList<QgsRelation::FieldPair> mFieldPairs;

    friend class TestQgsWeakRelation;

};

#endif // QGSWEAKRELATION_H
