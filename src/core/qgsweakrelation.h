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
     * Creates a QgsWeakRelation
     * \param relationId relation ID
     * \param relationName relation name
     * \param strength relation strength
     * \param referencingLayerId ID of the referencing layer
     * \param referencingLayerName name of the referencing layer
     * \param referencingLayerSource source of the referencing layer
     * \param referencingLayerProviderKey provider name of the referencing layer
     * \param referencedLayerId ID of the referenced layer
     * \param referencedLayerName name of the referenced layer
     * \param referencedLayerSource name of the referenced layer
     * \param referencedLayerProviderKey provider name of the referenced layer
     * \param fieldPairs list of the relation field pairs
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

  private:

    QgsVectorLayerRef mReferencingLayer;
    QgsVectorLayerRef mReferencedLayer;
    QString mRelationId;
    QString mRelationName;
    QgsRelation::RelationStrength mStrength;
    QList<QgsRelation::FieldPair> mFieldPairs;

    friend class TestQgsWeakRelation;

};

#endif // QGSWEAKRELATION_H
