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
#include "qgsvertexid.h"
#include "qgsgeometry.h"
#include "qgsrendercontext.h"

/**
 * \ingroup core
 * \brief Encapsulates the context for an annotation item edit operation.
 * \since QGIS 3.40
 */
class CORE_EXPORT QgsAnnotationItemEditContext
{
  public:

    QgsAnnotationItemEditContext() = default;

    /**
     * Returns the current rendered bounds of the item, in the annotation layer's CRS.
     *
     * \see setCurrentItemBounds()
     */
    QgsRectangle currentItemBounds() const;

    /**
     * Sets the current rendered \a bounds of the item, in the annotation layer's CRS.
     *
     * \see currentItemBounds()
     */
    void setCurrentItemBounds( const QgsRectangle &bounds );

    /**
     * Returns the render context associated with the edit operation.
     *
     * \see setRenderContext()
     */
    QgsRenderContext renderContext() const;

    /**
     * Sets the render \a context associated with the edit operation.
     *
     * \see renderContext()
     */
    void setRenderContext( const QgsRenderContext &context );

  private:

    QgsRectangle mCurrentItemBounds;
    QgsRenderContext mRenderContext;

};

/**
 * \ingroup core
 * \brief Abstract base class for annotation item edit operations
 * \since QGIS 3.22
 */
class CORE_EXPORT QgsAbstractAnnotationItemEditOperation
{
  public:

    /**
     * Operation type
     */
    enum class Type : int
    {
      MoveNode, //!< Move a node
      DeleteNode, //!< Delete a node
      AddNode, //!< Add a node
      TranslateItem, //!< Translate (move) an item
    };

    /**
     * Constructor for QgsAbstractAnnotationItemEditOperation, for the specified item id.
     */
    QgsAbstractAnnotationItemEditOperation( const QString &itemId );

    virtual ~QgsAbstractAnnotationItemEditOperation();

    /**
     * Returns the operation type.
     */
    virtual Type type() const = 0;

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
     *
     * Since QGIS 3.40 the \a translatePixelsX and \a translatePixelsY arguments specify the translation in pixels.
     */
    QgsAnnotationItemEditOperationMoveNode( const QString &itemId, QgsVertexId nodeId, const QgsPoint &before, const QgsPoint &after,
                                            double translatePixelsX = 0, double translatePixelsY = 0 );

    Type type() const override;

    /**
     * Returns the associated node ID.
     */
    QgsVertexId nodeId() const { return mNodeId; }

    /**
     * Returns the node position before the move occurred (in layer coordinates).
     *
     * \see after()
     */
    QgsPoint before() const { return mBefore; }

    /**
     * Returns the node position after the move occurred (in layer coordinates).
     *
     * \see before()
     */
    QgsPoint after() const { return mAfter; }

    /**
     * Returns the x-axis translation, in pixels.
     *
     * \since translationYPixels()
     * \since QGIS 3.40
     */
    double translationXPixels() const { return mTranslatePixelsX; }

    /**
     * Returns the y-axis translation, in pixels.
     *
     * \since translationXPixels()
     * \since QGIS 3.40
     */
    double translationYPixels() const { return mTranslatePixelsY; }

  private:

    QgsVertexId mNodeId;
    QgsPoint mBefore;
    QgsPoint mAfter;

    double mTranslatePixelsX = 0;
    double mTranslatePixelsY = 0;
};


/**
 * \ingroup core
 * \brief Annotation item edit operation consisting of deleting a node
 * \since QGIS 3.22
 */
class CORE_EXPORT QgsAnnotationItemEditOperationDeleteNode : public QgsAbstractAnnotationItemEditOperation
{
  public:

    /**
     * Constructor for QgsAnnotationItemEditOperationDeleteNode, where the node with the specified \a id and previous
     * position \a before is deleted.
     */
    QgsAnnotationItemEditOperationDeleteNode( const QString &itemId, QgsVertexId nodeId, const QgsPoint &before );

    Type type() const override;

    /**
     * Returns the deleted node ID.
     */
    QgsVertexId nodeId() const { return mNodeId; }

    /**
     * Returns the node position before the delete occurred (in layer coordinates).
     */
    QgsPoint before() const { return mBefore; }

  private:

    QgsVertexId mNodeId;
    QgsPoint mBefore;

};

/**
 * \ingroup core
 * \brief Annotation item edit operation consisting of adding a node
 * \since QGIS 3.22
 */
class CORE_EXPORT QgsAnnotationItemEditOperationAddNode : public QgsAbstractAnnotationItemEditOperation
{
  public:

    /**
     * Constructor for QgsAnnotationItemEditOperationAddNode at the specified \a point.
     */
    QgsAnnotationItemEditOperationAddNode( const QString &itemId, const QgsPoint &point );

    Type type() const override;

    /**
     * Returns the node position (in layer coordinates).
     */
    QgsPoint point() const { return mPoint; }

  private:

    QgsPoint mPoint;

};


/**
 * \ingroup core
 * \brief Annotation item edit operation consisting of translating (moving) an item
 * \since QGIS 3.22
 */
class CORE_EXPORT QgsAnnotationItemEditOperationTranslateItem : public QgsAbstractAnnotationItemEditOperation
{
  public:

    /**
     * Constructor for QgsAnnotationItemEditOperationTranslateItem, where the node with the specified \a id and translation
     * (in map units)
     *
     * Since QGIS 3.40 the \a translatePixelsX and \a translatePixelsY arguments specify the translation in pixels.
     */
    QgsAnnotationItemEditOperationTranslateItem( const QString &itemId, double translateX, double translateY,
        double translatePixelsX = 0, double translatePixelsY = 0 );

    Type type() const override;

    /**
     * Returns the deleted node ID.
     */
    QgsVertexId nodeId() const { return mNodeId; }

    /**
     * Returns the x-axis translation, in layer units.
     *
     * \since translationY()
     */
    double translationX() const { return mTranslateX; }

    /**
     * Returns the y-axis translation, in layer units.
     *
     * \since translationX()
     */
    double translationY() const { return mTranslateY; }

    /**
     * Returns the x-axis translation, in pixels.
     *
     * \since translationYPixels()
     * \since QGIS 3.40
     */
    double translationXPixels() const { return mTranslatePixelsX; }

    /**
     * Returns the y-axis translation, in pixels.
     *
     * \since translationXPixels()
     * \since QGIS 3.40
     */
    double translationYPixels() const { return mTranslatePixelsY; }

  private:

    QgsVertexId mNodeId;
    double mTranslateX = 0;
    double mTranslateY = 0;
    double mTranslatePixelsX = 0;
    double mTranslatePixelsY = 0;

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
