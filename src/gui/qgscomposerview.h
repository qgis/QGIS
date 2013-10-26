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
#include "qgsaddremoveitemcommand.h"

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
class QgsComposerAttributeTable;

/** \ingroup MapComposer
 * \ingroup gui
 * Widget to display the composer items. Manages the composer tools and the
 * mouse/key events.
 * Creates the composer items according to the current map tools and keeps track
 * of the rubber band item.
 */
class GUI_EXPORT QgsComposerView: public QGraphicsView
{
    Q_OBJECT

  public:

    /**Current tool*/
    enum Tool
    {
      Select = 0,      // Select/Move item
      AddArrow,         //add arrow
      AddHtml,
      AddMap,          // add new map
      AddLegend, // add vector legend
      AddLabel,        // add label
      AddScalebar,     // add scalebar
      AddPicture,       // add raster/vector picture
      AddRectangle,
      AddEllipse,
      AddTriangle,
      AddTable, //add attribute table
      MoveItemContent, //move content of item (e.g. content of map)
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

    QgsComposerView( QWidget* parent = 0, const char* name = 0, Qt::WFlags f = 0 );

    /**Add an item group containing the selected items*/
    void groupItems();

    /**Ungroups the selected items*/
    void ungroupItems();

    /**Cuts or copies the selected items*/
    void copyItems( ClipboardMode mode );

    /**Pastes items from clipboard*/
    void pasteItems( PasteMode mode );

    /**Deletes selected items*/
    void deleteSelectedItems();

    /**Selects all items*/
    void selectAll();

    /**Deselects all items*/
    void selectNone();

    /**Inverts current selection*/
    void selectInvert();

    QgsComposerView::Tool currentTool() const {return mCurrentTool;}
    void setCurrentTool( QgsComposerView::Tool t );

    /**Sets composition (derived from QGraphicsScene)*/
    void setComposition( QgsComposition* c );
    /**Returns the composition or 0 in case of error*/
    QgsComposition* composition();

    /**Returns the composer main window*/
    QMainWindow* composerWindow();

    void setPaintingEnabled( bool enabled ) { mPaintingEnabled = enabled; }
    bool paintingEnabled() const { return mPaintingEnabled; }

    /**Update rulers with current scene rect*/
    void updateRulers();

    void setHorizontalRuler( QgsComposerRuler* r ) { mHorizontalRuler = r; }
    void setVerticalRuler( QgsComposerRuler* r ) { mVerticalRuler = r; }

    /**Set zoom level, where a zoom level of 1.0 corresponds to 100%*/
    void setZoomLevel( double zoomLevel );

  protected:
    void mousePressEvent( QMouseEvent* );
    void mouseReleaseEvent( QMouseEvent* );
    void mouseMoveEvent( QMouseEvent* );
    void mouseDoubleClickEvent( QMouseEvent* e );

    void keyPressEvent( QKeyEvent * e );
    void keyReleaseEvent( QKeyEvent * e );

    void wheelEvent( QWheelEvent* event );

    void paintEvent( QPaintEvent* event );

    void hideEvent( QHideEvent* e );
    void showEvent( QShowEvent* e );

    void resizeEvent( QResizeEvent* event );
    void scrollContentsBy( int dx, int dy );

  private:
    /**Current composer tool*/
    QgsComposerView::Tool mCurrentTool;
    /**Previous composer tool*/
    QgsComposerView::Tool mPreviousTool;

    /**Rubber band item*/
    QGraphicsRectItem* mRubberBandItem;
    /**Rubber band item for arrows*/
    QGraphicsLineItem* mRubberBandLineItem;
    /**Item to move content*/
    QgsComposerItem* mMoveContentItem;
    /**Start position of content move*/
    QPointF mMoveContentStartPos;
    /**Start of rubber band creation*/
    QPointF mRubberBandStartPos;

    /**True if user is currently selecting by marquee*/
    bool mMarqueeSelect;
    /**True if user is currently zooming by marquee*/
    bool mMarqueeZoom;
    /**True if user is currently temporarily activating the zoom tool by holding control+space*/
    QgsComposerView::ToolStatus mTemporaryZoomStatus;

    bool mPaintingEnabled;

    QgsComposerRuler* mHorizontalRuler;
    QgsComposerRuler* mVerticalRuler;

    /** Draw a shape on the canvas */
    void addShape( Tool currentTool );

    bool mPanning;
    QPoint mMouseLastXY;
    QPoint mMouseCurrentXY;
    QPoint mMousePressStartPos;

    /**Zoom composition from a mouse wheel event*/
    void wheelZoom( QWheelEvent * event );
    /**Redraws the rubber band*/
    void updateRubberBand( QPointF & pos );
    /**Removes the rubber band and cleans up*/
    void removeRubberBand();

    /**Starts a marquee selection*/
    void startMarqueeSelect( QPointF & scenePoint );
    /**Finalises a marquee selection*/
    void endMarqueeSelect( QMouseEvent* e );
    /**Starts a zoom in marquee*/
    void startMarqueeZoom( QPointF & scenePoint );
    /**Finalises a marquee zoom*/
    void endMarqueeZoom( QMouseEvent* e );

    //void connectAddRemoveCommandSignals( QgsAddRemoveItemCommand* c );

  signals:
    /**Is emitted when selected item changed. If 0, no item is selected*/
    void selectedItemChanged( QgsComposerItem* selected );
    /**Is emitted when a composer item has been removed from the scene*/
    void itemRemoved( QgsComposerItem* );
    /**Current action (e.g. adding composer map) has been finished. The purpose of this signal is that
     QgsComposer may set the selection tool again*/
    void actionFinished();
    /**Is emitted when mouse cursor coordinates change*/
    void cursorPosChanged( QPointF );
    /**Is emitted when the view zoom changes*/
    void zoomLevelChanged();

    /**Emitted before composerview is shown*/
    void composerViewShow( QgsComposerView* );
    /**Emitted before composerview is hidden*/
    void composerViewHide( QgsComposerView* );
};

#endif
