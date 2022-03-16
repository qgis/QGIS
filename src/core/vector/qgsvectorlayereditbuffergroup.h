/***************************************************************************
  qgsvectorlayereditbuffergroup.h - QgsVectorLayerEditBufferGroup

 ---------------------
 begin                : 22.12.2021
 copyright            : (C) 2021 by Damiano Lombardi
 email                : damiano@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORLAYEREDITBUFFERGROUP_H
#define QGSVECTORLAYEREDITBUFFERGROUP_H

#include "qgis_core.h"

#include <QObject>
#include <QSet>

class QgsVectorLayer;

/**
 * The edit buffer group manages a group of edit buffers. Commands like commit and rollback are
 * managed by the group invokes individual addFeature(), deleteFeature(), ... in the correct order
 * across all contained edit buffers.
 *
 * \ingroup core
 * \class QgsVectorLayerEditBufferGroup
 *
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsVectorLayerEditBufferGroup : public QObject
{
    Q_OBJECT

  public:

    //! Constructor for QgsEditBufferGroup
    explicit QgsVectorLayerEditBufferGroup( QObject *parent = nullptr );

    /**
     * Add a layer to this edit buffer group.
     */
    void addLayer( QgsVectorLayer *layer );

    /**
     * Remove all layers from this edit buffer group
     */
    void clear();

    /**
     * Gets the set of layers currently managed by this edit buffer group.
     *
     * \returns Layer set
     */
    QSet<QgsVectorLayer *> layers() const;

    /**
     * Gets the set of modified layers currently managed by this edit buffer group.
     *
     * \returns Layer set
     */
    QSet<QgsVectorLayer *> modifiedLayers() const;

    /**
     * Start editing
     *
     * \returns TRUE on success
     */
    bool startEditing();

    /**
     * Attempts to commit any changes to disk.  Returns the result of the attempt.
     * If a commit fails, the in-memory changes are left alone.
     *
     * This allows editing to continue if the commit failed on e.g. a
     * disallowed value in a Postgres database - the user can re-edit and try
     * again.
     *
     * The commits occur in distinct stages,
     * (add attributes, add features, change attribute values, change
     * geometries, delete features, delete attributes)
     * so if a stage fails, it's difficult to roll back cleanly.
     * Therefore any error message also includes which stage failed so
     * that the user has some chance of repairing the damage cleanly.
     *
     * \returns TRUE on success
     */
    bool commitChanges( QStringList &commitErrors, bool stopEditing = true );

    /**
     * Stop editing and discard the edits
     *
     * \returns FALSE if errors occurred during rollback
     */
    bool rollBack( QStringList &rollbackErrors, bool stopEditing = true );

    /**
     * Returns TRUE if the layers are in editing mode
     */
    bool isEditing() const;

  private:

    QList<QgsVectorLayer *> orderLayersParentsToChildren( QSet<QgsVectorLayer *> layers );
    void editingFinished( bool stopEditing );

    QSet<QgsVectorLayer *> mLayers;

    bool mIsEditing = false;
};

#endif // QGSVECTORLAYEREDITBUFFERGROUP_H
