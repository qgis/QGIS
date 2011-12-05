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
      AddMap,          // add new map
      AddLegend, // add vector legend
      AddLabel,        // add label
      AddScalebar,     // add scalebar
      AddPicture,       // add raster/vector picture
      AddShape, //add shape item (ellipse, rectangle, triangle)
      AddTable, //add attribute table
      MoveItemContent //move content of item (e.g. content of map)
    };

    QgsComposerView( QWidget* parent = 0, const char* name = 0, Qt::WFlags f = 0 );

    /**Add an item group containing the selected items*/
    void groupItems();

    /**Ungroups the selected items*/
    void ungroupItems();

    QgsComposerView::Tool currentTool() const {return mCurrentTool;}
    void setCurrentTool( QgsComposerView::Tool t ) {mCurrentTool = t;}

    /**Sets composition (derived from QGraphicsScene)*/
    void setComposition( QgsComposition* c );
    /**Returns the composition or 0 in case of error*/
    QgsComposition* composition();

    /**Remove item from the graphics scene*/
    void removeItem( QgsComposerItem* item );

    /**Returns the composer main window*/
    QMainWindow* composerWindow();

    void setPaintingEnabled( bool enabled ) { mPaintingEnabled = enabled; }
    bool paintingEnabled() const { return mPaintingEnabled; }

    /**Convenience function to create a QgsAddRemoveItemCommand, connect its signals and push it to the undo stack*/
    void pushAddRemoveCommand( QgsComposerItem* item, const QString& text, QgsAddRemoveItemCommand::State state = QgsAddRemoveItemCommand::Added );

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

  private:
    /**Status of shift key (used for multiple selection)*/
    bool mShiftKeyPressed;
    /**Current composer tool*/
    QgsComposerView::Tool mCurrentTool;
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

    bool mPaintingEnabled;

    void connectAddRemoveCommandSignals( QgsAddRemoveItemCommand* c );


  public slots:
    /**Casts object to the proper subclass type and calls corresponding itemAdded signal*/
    void sendItemAddedSignal( QgsComposerItem* item );

  signals:
    /**Is emitted when selected item changed. If 0, no item is selected*/
    void selectedItemChanged( QgsComposerItem* selected );
    /**Is emitted when new composer arrow has been added to the view*/
    void composerArrowAdded( QgsComposerArrow* arrow );
    /**Is emitted when new composer label has been added to the view*/
    void composerLabelAdded( QgsComposerLabel* label );
    /**Is emitted when new composer map has been added to the view*/
    void composerMapAdded( QgsComposerMap* map );
    /**Is emitted when new composer scale bar has been added*/
    void composerScaleBarAdded( QgsComposerScaleBar* scalebar );
    /**Is emitted when a new composer legend has been added*/
    void composerLegendAdded( QgsComposerLegend* legend );
    /**Is emitted when a new composer picture has been added*/
    void composerPictureAdded( QgsComposerPicture* picture );
    /**Is emitted when a new composer shape has been added*/
    void composerShapeAdded( QgsComposerShape* shape );
    /**Is emitted when a new composer table has been added*/
    void composerTableAdded( QgsComposerAttributeTable* table );
    /**Is emitted when a composer item has been removed from the scene*/
    void itemRemoved( QgsComposerItem* );
    /**Current action (e.g. adding composer map) has been finished. The purpose of this signal is that
     QgsComposer may set the selection tool again*/
    void actionFinished();

    /**Emitted before composerview is shown*/
    void composerViewShow( QgsComposerView* );
    /**Emitted before composerview is hidden*/
    void composerViewHide( QgsComposerView* );
};

#endif
