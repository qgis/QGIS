/***************************************************************************
                              qgsmapcanvasannotationitem.h
                              ----------------------------
  begin                : January 2017
  copyright            : (C) 2017 by Nyall Dawson
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

#ifndef QGSMAPCANVASANNOTATIONITEM_H
#define QGSMAPCANVASANNOTATIONITEM_H

#ifdef SIP_RUN
% ModuleHeaderCode
// For ConvertToSubClassCode.
#include <qgsmapcanvasannotationitem.h>
% End
#endif

#include "qgsmapcanvasitem.h"
#include "qgis_gui.h"

class QgsAnnotation;

/**
 * \class QgsMapCanvasAnnotationItem
 * \ingroup gui
 * \brief An interactive map canvas item which displays a QgsAnnotation.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsMapCanvasAnnotationItem: public QObject, public QgsMapCanvasItem
{
    Q_OBJECT

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( dynamic_cast<QgsMapCanvasAnnotationItem *>( sipCpp ) )
    {
      sipType = sipType_QgsMapCanvasAnnotationItem;
      *sipCppRet = dynamic_cast<QgsMapCanvasAnnotationItem *>( sipCpp );
    }
    else
      sipType = nullptr;
    SIP_END
#endif

  public:

    //! Mouse actions for interacting with item
    enum MouseMoveAction
    {
      NoAction, //!< No action
      MoveMapPosition, //!< Moving annotation map position
      MoveFramePosition, //!< Moving position of frame relative to annotation
      ResizeFrameUp, //!< Resize frame up
      ResizeFrameDown, //!< Resize frame down
      ResizeFrameLeft, //!< Resize frame left
      ResizeFrameRight, //!< Resize frame right
      ResizeFrameLeftUp, //!< Resize frame left up
      ResizeFrameRightUp, //!< Resize frame right up
      ResizeFrameLeftDown, //!< Resize frame left down
      ResizeFrameRightDown //!< Resize frame right down
    };

    /**
     * Constructor for QgsMapCanvasAnnotationItem.
     */
    QgsMapCanvasAnnotationItem( QgsAnnotation *annotation SIP_TRANSFER, QgsMapCanvas *mapCanvas SIP_TRANSFERTHIS );

    /**
     * Returns the item's annotation.
     * \note not available in Python bindings
     */
    const QgsAnnotation *annotation() const { return mAnnotation; } SIP_SKIP

    /**
     * Returns the item's annotation.
     */
    QgsAnnotation *annotation() { return mAnnotation; }

    void updatePosition() override;

    QRectF boundingRect() const override;

    void paint( QPainter *painter ) override;

    /**
     * Returns the mouse move behavior for a given position in scene coordinates
     */
    MouseMoveAction moveActionForPosition( QPointF pos ) const;

    /**
     * Returns matching cursor shape for a mouse move action.
     */
    Qt::CursorShape cursorShapeForAction( MouseMoveAction moveAction ) const;

  private slots:

    void updateBoundingRect();

    void onCanvasLayersChanged();

    //! Sets a feature for the current map position
    void setFeatureForMapPosition();

    void annotationDeleted();

  private:

    //! Draws selection handles around the item
    void drawSelectionBoxes( QPainter *p ) const;

    //! Returns the symbol size scaled in (mapcanvas) pixels. Used for the counting rect calculation
    double scaledSymbolSize() const;

    QgsAnnotation *mAnnotation = nullptr;

    //! Bounding rect (including item frame and balloon)
    QRectF mBoundingRect;

};

#endif // QGSMAPCANVASANNOTATIONITEM_H
