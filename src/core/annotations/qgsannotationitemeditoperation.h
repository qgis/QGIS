/***************************************************************************
    qgsannotationitemeditoperation.h
    ----------------
    copyright            : (C) 2021 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSANNOTATIONITEMEDITOPERATION_H
#define QGSANNOTATIONITEMEDITOPERATION_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgis.h"
#include "qgspointxy.h"
#include "qgsabstractgeometry.h"
#include "qgsgeometry.h"

/**
 * \ingroup core
 * \brief Abstract base class for annotation item edit operations
 * \since QGIS 3.22
 */
class CORE_EXPORT QgsAbstractAnnotationItemEditOperation
{
  public:

    /**
     * Constructor for QgsAbstractAnnotationItemEditOperation, for the specified item id.
     */
    QgsAbstractAnnotationItemEditOperation( const QString &itemId );

    virtual ~QgsAbstractAnnotationItemEditOperation();

    /**
     * Returns the associated item ID.
     */
    QString itemId() const { return mItemId; }

  protected:

    QString mItemId;

};

/**
 * \ingroup core
 * \brief Annotation item edit operation consisting of moving a node
 * \since QGIS 3.22
 */
class CORE_EXPORT QgsAnnotationItemEditOperationMoveNode : public QgsAbstractAnnotationItemEditOperation
{
  public:

    /**
     * Constructor for QgsAnnotationItemEditOperationMoveNode, where the node with the specified \a id moves
     * from \a before to \a after (in layer coordinates).
     */
    QgsAnnotationItemEditOperationMoveNode( const QString &itemId, QgsVertexId nodeId, const QgsPointXY &before, const QgsPointXY &after );

    /**
     * Returns the associated node ID.
     */
    QgsVertexId nodeId() const { return mNodeId; }

    /**
     * Returns the node position before the move occurred (in layer coordinates).
     *
     * \see after()
     */
    QgsPointXY before() const { return mBefore; }

    /**
     * Returns the node position after the move occurred (in layer coordinates).
     *
     * \see before()
     */
    QgsPointXY after() const { return mAfter; }

  private:

    QgsVertexId mNodeId;
    QgsPointXY mBefore;
    QgsPointXY mAfter;

};

/**
 * \ingroup core
 * \brief Encapsulates the transient results of an in-progress annotation edit operation.
 * \since QGIS 3.22
 */
class CORE_EXPORT QgsAnnotationItemEditOperationTransientResults
{
  public:

    /**
     * Constructor for QgsAnnotationItemEditOperationTransientResults.
     *
     * The \a representativeGeometry parameter specifies a geometry (in layer CRS) which represents
     * the shape of the item if the operation were to be applied. It is used for creating a graphical
     * representation of the operation during interactive edits.
     */
    QgsAnnotationItemEditOperationTransientResults( const QgsGeometry &representativeGeometry )
      : mRepresentativeGeometry( representativeGeometry )
    {}

    /**
     * Returns the geometry (in layer CRS) which represents the shape of the item if the operation were to be applied.
     *
     * This is used for creating a graphical representation of the operation during interactive edits.
     */
    QgsGeometry representativeGeometry() const { return mRepresentativeGeometry; }

  private:

    QgsGeometry mRepresentativeGeometry;

};

#endif // QGSANNOTATIONITEMEDITOPERATION_H
