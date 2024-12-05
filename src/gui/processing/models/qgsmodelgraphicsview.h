/***************************************************************************
                             qgsmodelgraphicsview.h
                             -----------------------
    Date                 : March 2020
    Copyright            : (C) 2020 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMODELGRAPHICVIEW_H
#define QGSMODELGRAPHICVIEW_H

#include "qgis.h"
#include "qgis_gui.h"
#include "qgsprocessingcontext.h"
#include "qgsmodelsnapper.h"
#include <QGraphicsView>
#include <QGraphicsRectItem>

class QgsModelViewTool;
class QgsModelViewToolTemporaryKeyPan;
class QgsModelViewToolTemporaryKeyZoom;
class QgsModelViewToolTemporaryMousePan;
class QgsModelComponentGraphicItem;
class QgsModelGraphicsScene;
class QgsModelViewSnapMarker;

///@cond NOT_STABLE

/**
 * \ingroup gui
 * \brief QGraphicsView subclass representing the model designer.
 * \warning Not stable API
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsModelGraphicsView : public QGraphicsView
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsModelGraphicsView, with the specified \a parent widget.
     */
    QgsModelGraphicsView( QWidget *parent = nullptr );
    ~QgsModelGraphicsView() override;

    void dragEnterEvent( QDragEnterEvent *event ) override;
    void dropEvent( QDropEvent *event ) override;
    void dragMoveEvent( QDragMoveEvent *event ) override;
    void wheelEvent( QWheelEvent *event ) override;
    void mousePressEvent( QMouseEvent *event ) override;
    void mouseReleaseEvent( QMouseEvent *event ) override;
    void mouseMoveEvent( QMouseEvent *event ) override;
    void mouseDoubleClickEvent( QMouseEvent *event ) override;
    void keyPressEvent( QKeyEvent *event ) override;
    void keyReleaseEvent( QKeyEvent *event ) override;

    /**
     * Sets the related \a scene.
     */
    void setModelScene( QgsModelGraphicsScene *scene );

    /**
     * Returns the scene associated with the tool.
     * \see view()
     */
    QgsModelGraphicsScene *modelScene() const;

    /**
     * Returns the currently active tool for the view.
     * \see setTool()
     */
    QgsModelViewTool *tool() SIP_SKIP;

    /**
     * Sets the \a tool currently being used in the view.
     * \see unsetTool()
     * \see tool()
     */
    void setTool( QgsModelViewTool *tool ) SIP_SKIP;

    /**
     * Unsets the current view tool, if it matches the specified \a tool.
     *
     * This is called from destructor of view tools to make sure
     * that the tool won't be used any more.
     * You don't have to call it manually, QgsModelViewTool takes care of it.
     */
    void unsetTool( QgsModelViewTool *tool ) SIP_SKIP;

    /**
     * Returns the view's coordinate snapper.
     */
    QgsModelSnapper *snapper() SIP_SKIP;

    /**
     * Starts a macro command, containing a group of interactions in the view.
     */
    void startMacroCommand( const QString &text );

    /**
     * Ends a macro command, containing a group of interactions in the view.
     */
    void endMacroCommand();


    //! Clipboard operations
    enum ClipboardOperation
    {
      ClipboardCut,  //!< Cut items
      ClipboardCopy, //!< Copy items
    };

    /**
     * Cuts or copies the selected items, respecting the specified \a operation.
     * \see copyItems()
     * \see pasteItems()
     */
    void copySelectedItems( ClipboardOperation operation );

    /**
     * Cuts or copies the a list of \a items, respecting the specified \a operation.
     * \see copySelectedItems()
     * \see pasteItems()
     */
    void copyItems( const QList<QgsModelComponentGraphicItem *> &items, ClipboardOperation operation );

    //! Paste modes
    enum PasteMode
    {
      PasteModeCursor,  //!< Paste items at cursor position
      PasteModeCenter,  //!< Paste items in center of view
      PasteModeInPlace, //!< Paste items in place
    };

    /**
     * Pastes items from clipboard, using the specified \a mode.
     *
     * \see copySelectedItems()
     * \see hasItemsInClipboard()
     */
    void pasteItems( PasteMode mode );

  public slots:

    /**
     * Snaps the selected items to the grid.
     */
    void snapSelected();

  signals:

    /**
     * Emitted when an algorithm is dropped onto the view.
     */
    void algorithmDropped( const QString &algorithmId, const QPointF &pos );

    /**
     * Emitted when an input parameter is dropped onto the view.
     */
    void inputDropped( const QString &inputId, const QPointF &pos );

    /**
     * Emitted when the current \a tool is changed.
     * \see setTool()
     */
    void toolSet( QgsModelViewTool *tool ) SIP_SKIP;

    /**
     * Emitted when an \a item is "focused" in the view, i.e. it becomes the active
     * item and should have its properties displayed in any designer windows.
     */
    void itemFocused( QgsModelComponentGraphicItem *item );

    /**
     * Emitted in the destructor when the view is about to be deleted,
     * but is still in a perfectly valid state.
     */
    void willBeDeleted();

    /**
     * Emitted when a macro command containing a group of interactions is started in the view.
     */
    void macroCommandStarted( const QString &text );

    /**
     * Emitted when a macro command containing a group of interactions in the view has ended.
     */
    void macroCommandEnded();

    /**
     * Emitted when an undo command is started in the view.
     */
    void beginCommand( const QString &text );

    /**
     * Emitted when an undo command in the view has ended.
     */
    void endCommand();

    /**
     * Emitted when the selected items should be deleted;
     */
    void deleteSelectedItems();

  private:
    //! Zoom layout from a mouse wheel event
    void wheelZoom( QWheelEvent *event );

    /**
     * Scales the view in a safe way, by limiting the acceptable range
     * of the scale applied. The \a scale parameter specifies the zoom factor to scale the view by.
     */
    void scaleSafe( double scale );

    /**
     * Returns the delta (in model coordinates) by which to move items
     * for the given key \a event.
     */
    QPointF deltaForKeyEvent( QKeyEvent *event );

    QPointer<QgsModelViewTool> mTool;

    QgsModelViewToolTemporaryKeyPan *mSpacePanTool = nullptr;
    QgsModelViewToolTemporaryMousePan *mMidMouseButtonPanTool = nullptr;
    QgsModelViewToolTemporaryKeyZoom *mSpaceZoomTool = nullptr;

    QPoint mMouseCurrentXY;

    QgsModelSnapper mSnapper;
    QgsModelViewSnapMarker *mSnapMarker = nullptr;
};


/**
 * \ingroup gui
 * \brief A simple graphics item rendered as an 'x'.
 */
class GUI_EXPORT QgsModelViewSnapMarker : public QGraphicsRectItem
{
  public:
    QgsModelViewSnapMarker();

    void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr ) override;

  private:
    int mSize = 0;
};


///@endcond

#endif // QGSMODELGRAPHICVIEW_H
