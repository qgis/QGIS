/***************************************************************************
                         qgscomposerview.h
                             -------------------
    begin                : January 2005
    copyright            : (C) 2005 by Radim Blazek
    email                : blazek@itc.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCOMPOSERVIEW_H
#define QGSCOMPOSERVIEW_H

#include <QGraphicsView>
#include "qgis.h"
#include "qgsprevieweffect.h" // for QgsPreviewEffect::PreviewMode
#include <QGraphicsPolygonItem>
#include "qgis_gui.h"
#include <memory>

class QDomDocument;
class QDomElement;
class QKeyEvent;
class QMainWindow;
class QMouseEvent;
class QgsComposition;
class QgsComposerArrow;
class QgsComposerItem;
class QgsComposerLabel;
class QgsComposerLegend;
class QgsComposerMap;
class QgsComposerPicture;
class QgsComposerRuler;
class QgsComposerScaleBar;
class QgsComposerShape;
class QgsComposerNodesItem;
class QgsComposerAttributeTableV2;
class QgsMapCanvas;

/**
 * \ingroup gui
 * Widget to display the composer items. Manages the composer tools and the
 * mouse/key events.
 * Creates the composer items according to the current map tools and keeps track
 * of the rubber band item.
 */
class GUI_EXPORT QgsComposerView: public QGraphicsView
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( sipCpp->inherits( "QgsComposerView" ) )
      sipType = sipType_QgsComposerView;
    else
      sipType = NULL;
    SIP_END
#endif
    Q_OBJECT

  public:

    //! Current tool
    enum Tool
    {
      Select = 0,      // Select/Move item
      AddArrow,        // add arrow
      AddHtml,
      AddMap,          // add new map
      AddLegend,       // add vector legend
      AddLabel,        // add label
      AddScalebar,     // add scalebar
      AddPicture,      // add raster/vector picture
      AddRectangle,
      AddEllipse,
      AddPolygon,
      AddPolyline,
      AddTriangle,
      AddTable,        // add attribute table
      AddAttributeTable,
      MoveItemContent, // move content of item (e.g. content of map)
      EditNodesItem,
      Pan,
      Zoom
    };

    enum ClipboardMode
    {
      ClipboardModeCut,
      ClipboardModeCopy
    };

    enum PasteMode
    {
      PasteModeCursor,
      PasteModeCenter,
      PasteModeInPlace
    };

    enum ToolStatus
    {
      Inactive,
      Active,
      ActiveUntilMouseRelease
    };

    QgsComposerView( QWidget *parent SIP_TRANSFERTHIS = nullptr, const char *name = nullptr, Qt::WindowFlags f = 0 );

    //! Add an item group containing the selected items
    void groupItems();

    //! Ungroups the selected items
    void ungroupItems();

    //! Cuts or copies the selected items
    void copyItems( ClipboardMode mode );

    //! Pastes items from clipboard
    void pasteItems( PasteMode mode );

    //! Deletes selected items
    void deleteSelectedItems();

    //! Selects all items
    void selectAll();

    //! Deselects all items
    void selectNone();

    //! Inverts current selection
    void selectInvert();

    QgsComposerView::Tool currentTool() const {return mCurrentTool;}
    void setCurrentTool( QgsComposerView::Tool t );

    /**
     * Sets the composition for the view. If the composition is being set manually and not by a QgsComposer, then this must
     * be set BEFORE adding any items to the composition.
     */
    void setComposition( QgsComposition *c SIP_KEEPREFERENCE );

    //! Returns the composition or 0 in case of error
    QgsComposition *composition();

    //! Returns the composer main window
    QMainWindow *composerWindow();

    void setPaintingEnabled( bool enabled ) { mPaintingEnabled = enabled; }
    bool paintingEnabled() const { return mPaintingEnabled; }

    //! Update rulers with current scene rect
    void updateRulers();

    void setHorizontalRuler( QgsComposerRuler *r ) { mHorizontalRuler = r; }
    void setVerticalRuler( QgsComposerRuler *r ) { mVerticalRuler = r; }

    //! Set zoom level, where a zoom level of 1.0 corresponds to 100%
    void setZoomLevel( double zoomLevel );

    /**
     * Scales the view in a safe way, by limiting the acceptable range
     * of the scale applied.
     * \param scale factor to scale view by
     * \since QGIS 2.16
     */
    void scaleSafe( double scale );

    /**
     * Sets whether a preview effect should be used to alter the view's appearance
     * \param enabled Set to true to enable the preview effect on the view
     * \since QGIS 2.3
     * \see setPreviewMode
     */
    void setPreviewModeEnabled( bool enabled );

    /**
     * Sets the preview mode which should be used to modify the view's appearance. Preview modes are only used
     * if setPreviewMode is set to true.
     * \param mode PreviewMode to be used to draw the view
     * \since QGIS 2.3
     * \see setPreviewModeEnabled
     */
    void setPreviewMode( QgsPreviewEffect::PreviewMode mode );

    /**
     * Sets the map canvas associated with the view. This allows the
     * view to retrieve map settings from the canvas.
     * \since QGIS 3.0
     * \see mapCanvas()
     */
    void setMapCanvas( QgsMapCanvas *canvas );

    /**
     * Returns the map canvas associated with the view.
     * \see setMapCanvas()
     * \since QGIS 3.0
     */
    QgsMapCanvas *mapCanvas() const;

  protected:
    void mousePressEvent( QMouseEvent * ) override;
    void mouseReleaseEvent( QMouseEvent * ) override;
    void mouseMoveEvent( QMouseEvent * ) override;
    void mouseDoubleClickEvent( QMouseEvent *e ) override;

    void keyPressEvent( QKeyEvent *e ) override;
    void keyReleaseEvent( QKeyEvent *e ) override;

    void wheelEvent( QWheelEvent *event ) override;

    void paintEvent( QPaintEvent *event ) override;

    void hideEvent( QHideEvent *e ) override;
    void showEvent( QShowEvent *e ) override;

    void resizeEvent( QResizeEvent *event ) override;
    void scrollContentsBy( int dx, int dy ) override;

  private:
    //! Current composer tool
    QgsComposerView::Tool mCurrentTool = Select;
    //! Previous composer tool
    QgsComposerView::Tool mPreviousTool = Select;

    //! Rubber band item
    QGraphicsRectItem *mRubberBandItem = nullptr;
    //! Rubber band item for arrows
    QGraphicsLineItem *mRubberBandLineItem = nullptr;
    //! Item to move content
    QgsComposerItem *mMoveContentItem = nullptr;
    //! Start position of content move
    QPointF mMoveContentStartPos;
    //! Start of rubber band creation
    QPointF mRubberBandStartPos;

    //! True if user is currently selecting by marquee
    bool mMarqueeSelect = false;
    //! True if user is currently zooming by marquee
    bool mMarqueeZoom = false;
    //! True if user is currently temporarily activating the zoom tool by holding control+space
    QgsComposerView::ToolStatus mTemporaryZoomStatus = QgsComposerView::Inactive;

    bool mPaintingEnabled = true;

    QgsComposerRuler *mHorizontalRuler = nullptr;
    QgsComposerRuler *mVerticalRuler = nullptr;

    QgsMapCanvas *mCanvas = nullptr;

    //! Draw a shape on the canvas
    void addShape( Tool currentTool );

    //! Point based shape stuff
    void addPolygonNode( QPointF scenePoint );
    void movePolygonNode( QPointF scenePoint, bool constrainAngle );
    void displayNodes( const bool display = true );
    void setSelectedNode( QgsComposerNodesItem *shape, const int index );
    void deselectNode();

    float mMoveContentSearchRadius = 25;
    QgsComposerNodesItem *mNodesItem = nullptr;
    int mNodesItemIndex = -1;
    std::unique_ptr<QGraphicsPolygonItem> mPolygonItem;
    std::unique_ptr<QGraphicsPathItem> mPolylineItem;

    //! True if user is currently panning by clicking and dragging with the pan tool
    bool mToolPanning = false;
    //! True if user is currently panning by holding the middle mouse button
    bool mMousePanning = false;
    //! True if user is currently panning by holding the space key
    bool mKeyPanning = false;

    //! True if user is currently dragging with the move item content tool
    bool mMovingItemContent = false;

    QPoint mMouseLastXY;
    QPoint mMouseCurrentXY;
    QPoint mMousePressStartPos;

    QgsPreviewEffect *mPreviewEffect = nullptr;

    //! Returns the default mouse cursor for a tool
    QCursor defaultCursorForTool( Tool currentTool );

    //! Zoom composition from a mouse wheel event
    void wheelZoom( QWheelEvent *event );
    //! Redraws the rectangular rubber band
    void updateRubberBandRect( QPointF &pos, const bool constrainSquare = false, const bool fromCenter = false );
    //! Redraws the linear rubber band
    void updateRubberBandLine( QPointF pos, const bool constrainAngles = false );
    //! Removes the rubber band and cleans up
    void removeRubberBand();

    //! Starts a marquee selection
    void startMarqueeSelect( QPointF &scenePoint );
    //! Finalises a marquee selection
    void endMarqueeSelect( QMouseEvent *e );
    //! Starts a zoom in marquee
    void startMarqueeZoom( QPointF &scenePoint );
    //! Finalises a marquee zoom
    void endMarqueeZoom( QMouseEvent *e );

    //void connectAddRemoveCommandSignals( QgsAddRemoveItemCommand* c );

  signals:
    //! Is emitted when selected item changed. If 0, no item is selected
    void selectedItemChanged( QgsComposerItem *selected );
    //! Is emitted when a composer item has been removed from the scene
    void itemRemoved( QgsComposerItem * );

    /**
     * Current action (e.g. adding composer map) has been finished. The purpose of this signal is that
     QgsComposer may set the selection tool again*/
    void actionFinished();
    //! Is emitted when mouse cursor coordinates change
    void cursorPosChanged( QPointF );
    //! Is emitted when the view zoom changes
    void zoomLevelChanged();

    //! Emitted before composerview is shown
    void composerViewShow( QgsComposerView * );
    //! Emitted before composerview is hidden
    void composerViewHide( QgsComposerView * );

    //! Emitted when the composition is set for the view
    void compositionSet( QgsComposition * );
};

#endif
