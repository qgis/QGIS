/***************************************************************************
    qgsrelationmanager.h
     --------------------------------------
    Date                 : 1.3.2013
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

#ifndef QGSRELATIONMANAGER_H
#define QGSRELATIONMANAGER_H

#include "qgis_core.h"
#include <QObject>
#include <QPair>
#include <QDomNode>
#include <QDomDocument>

#include "qgsrelation.h"

class QgsProject;
class QgsVectorLayer;

/**
 * \ingroup core
 * This class manages a set of relations between layers.
 */
class CORE_EXPORT QgsRelationManager : public QObject
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsRelationManager.
     * \param project associated project (used to notify project of changes)
     */
    explicit QgsRelationManager( QgsProject *project = nullptr );

    /**
     * Will set the specified relations and remove any relation currently set.
     *
     * \param relations A list of relations to set.
     */
    void setRelations( const QList<QgsRelation> &relations );

    /**
     * Gets access to the relations managed by this class.
     *
     * \returns A QMap where the key is the relation id, the value the relation object.
     */
    QMap<QString, QgsRelation> relations() const;

    /**
     * Add a relation.
     * Invalid relations are added only if both referencing layer and referenced
     * layer exist.
     *
     * \param relation The relation to add.
     */
    void addRelation( const QgsRelation &relation );

    /**
     * Remove a relation.
     *
     * \param id The id of the relation to remove.
     */
    void removeRelation( const QString &id );

    /**
     * Remove a relation.
     *
     * \param relation The relation to remove.
     */
    void removeRelation( const QgsRelation &relation );

    /**
     * Gets access to a relation by its id.
     *
     * \param id The id to search for
     *
     * \returns A relation. Invalid if not found.
     * \see relationsByName()
     */
    Q_INVOKABLE QgsRelation relation( const QString &id ) const;

    /**
     * Returns a list of relations with matching names.
     * \param name relation name to search for. Searching is case insensitive.
     * \returns a list of matching relations
     * \see relation()
     * \since QGIS 2.16
     */
    QList<QgsRelation> relationsByName( const QString &name ) const;

    /**
     * Remove any relation managed by this class.
     */
    void clear();

    /**
     * Gets all relations where the specified layer (and field) is the referencing part (i.e. the child table with the foreign key).
     *
     * \param layer     The layer which should be searched for.
     * \param fieldIdx  The field which should be part of the foreign key. If not set will return all relations.
     *
     * \returns A list of relations matching the given layer and fieldIdx.
     */
    QList<QgsRelation> referencingRelations( const QgsVectorLayer *layer = nullptr, int fieldIdx = -2 ) const;

    /**
     * Gets all relations where this layer is the referenced part (i.e. the parent table with the primary key being referenced from another layer).
     *
     * \param layer   The layer which should be searched for.
     *
     * \returns A list of relations where the specified layer is the referenced part.
     */
    QList<QgsRelation> referencedRelations( QgsVectorLayer *layer = nullptr ) const;

    /**
     * Discover all the relations available from the current layers.
     *
     * \param existingRelations the existing relations to filter them out
     * \param layers the current layers
     * \returns the list of discovered relations
     * \since QGIS 3.0
     */
    static QList<QgsRelation> discoverRelations( const QList<QgsRelation> &existingRelations, const QList<QgsVectorLayer *> &layers );

  signals:
    //! Emitted when the relations were loaded after reading a project
    void relationsLoaded();

    /**
     * Emitted when relations are added or removed to the manager.
     * \since QGIS 2.5
     */
    void changed();

  public slots:

    /**
     * Updates relations status
     */
    void updateRelationsStatus();

  private slots:
    void readProject( const QDomDocument &doc, QgsReadWriteContext &context );
    void writeProject( QDomDocument &doc );
    void layersRemoved( const QStringList &layers );

  private:
    //! The references
    QMap<QString, QgsRelation> mRelations;

    QgsProject *mProject = nullptr;
};

#endif // QGSRELATIONMANAGER_H
