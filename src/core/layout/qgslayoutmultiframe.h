/***************************************************************************
                              qgslayoutmultiframe.h
                              --------------------
    begin                : October 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYOUTMULTIFRAME_H
#define QGSLAYOUTMULTIFRAME_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgslayoutobject.h"
#include "qgslayoutundocommand.h"
#include <QObject>
#include <QSizeF>
#include <QPointF>

class QgsLayoutFrame;
class QgsLayoutItem;
class QgsLayout;
class QDomDocument;
class QDomElement;
class QRectF;
class QPainter;
class QStyleOptionGraphicsItem;
class QgsRenderContext;

/**
 * \ingroup core
 * \class QgsLayoutMultiFrame
 * Abstract base class for layout items with the ability to distribute the content to
 * several frames (QgsLayoutFrame items).
 * \since QGIS 3.0
 */

class CORE_EXPORT QgsLayoutMultiFrame: public QgsLayoutObject, public QgsLayoutUndoObjectInterface
{

    Q_OBJECT

  public:

    /**
     * Specifies the behavior for creating new frames to fit the multiframe's content
     */
    enum ResizeMode
    {
      UseExistingFrames = 0, //!< Don't automatically create new frames, just use existing frames
      ExtendToNextPage, //!< Creates new full page frames on the following page(s) until the entire multiframe content is visible
      RepeatOnEveryPage, //!< Repeats the same frame on every page
      RepeatUntilFinished /*!< creates new frames with the same position and dimensions as the existing frame on the following page(s),
                              until the entire multiframe content is visible */
    };

    //! Multiframe item undo commands, used for collapsing undo commands
    enum UndoCommand
    {
      UndoNone = -1, //!< No command suppression
    };

    /**
     * Construct a new multiframe item, attached to the specified \a layout.
     */
    QgsLayoutMultiFrame( QgsLayout *layout SIP_TRANSFERTHIS );

    ~QgsLayoutMultiFrame();

    /**
     * Returns the multiframe identification string. This is a unique random string set for the multiframe
     * upon creation.
     * \note There is no corresponding setter for the uuid - it's created automatically.
    */
    QString uuid() const { return mUuid; }

    /**
     * Returns the total size of the multiframe's content, in layout units.
     */
    virtual QSizeF totalSize() const = 0;

    /**
     * Returns unique multiframe type id.
     */
    virtual int type() const = 0;

    /**
     * Return the multiframe type as a string.
     *
     * This string must be a unique, single word, character only representation of the item type, eg "LayoutHtml"
     * \see type()
     */
    virtual QString stringType() const = 0;

    /**
     * Returns the fixed size for a frame, if desired. If the fixed frame size changes,
     * the sizes of all frames can be recalculated by calling recalculateFrameRects().
     * \param frameIndex frame number
     * \returns fixed size for frame. If the size has a width or height of 0, then
     * the frame size is not fixed in that direction and frames can have variable width
     * or height accordingly.
     * \see minFrameSize()
     * \see recalculateFrameRects()
     */
    virtual QSizeF fixedFrameSize( const int frameIndex = -1 ) const;

    /**
     * Returns the minimum size for a frames, if desired. If the minimum
     * size changes, the sizes of all frames can be recalculated by calling
     * recalculateFrameRects().
     * \param frameIndex frame number
     * \returns minimum size for frame. If the size has a width or height of 0, then
     * the frame size has no minimum in that direction.
     * \see fixedFrameSize()
     * \see recalculateFrameRects()
     */
    virtual QSizeF minFrameSize( const int frameIndex = -1 ) const;

    /**
     * Renders a portion of the multiframe's content into a render \a context.
     * \param context destination render painter
     * \param renderExtent visible extent of content to render into the painter.
     * \param frameIndex frame number for content
     * \param itemStyle item style options for graphics item rendering
     */
    virtual void render( QgsRenderContext &context, const QRectF &renderExtent, const int frameIndex,
                         const QStyleOptionGraphicsItem *itemStyle = nullptr ) = 0;

    /**
     * Adds a \a frame to the multiframe.
     *
     * If \a recalcFrameSizes is set to true, then a recalculation of all existing frame sizes will be forced.
     *
     * \see removeFrame()
     */
    virtual void addFrame( QgsLayoutFrame *frame SIP_TRANSFER, bool recalcFrameSizes = true );

    /**
     * Finds the optimal position to break a frame at.
     * \param yPos maximum vertical position for break, in layout units.
     * \returns the optimal breakable position which occurs in the multi frame close
     * to and before the specified yPos
     */
    virtual double findNearbyPageBreak( double yPos );

    /**
     * Removes a frame by \a index from the multiframe. This method automatically removes the frame from the
     * layout too.
     *
     * If \a removeEmptyPages is set to true, then pages which are empty after the frame is removed will
     * also be removed from the layout.
     *
     * \see addFrame()
     * \see deleteFrames()
     */
    void removeFrame( int index, bool removeEmptyPages = false );

    /**
     * Removes and deletes all child frames.
     * \see removeFrame()
     */
    void deleteFrames();

    /**
     * Sets the resize \a mode for the multiframe, and recalculates frame sizes to match.
     * \see resizeMode()
     */
    void setResizeMode( ResizeMode mode );

    /**
     * Returns the resize mode for the multiframe.
     * \see setResizeMode()
     */
    ResizeMode resizeMode() const { return mResizeMode; }

    /**
     * Stores the multiframe state in a DOM element.
     * \param parentElement parent DOM element (e.g. 'Layout' element)
     * \param document DOM document
     * \param context read write context
     * \param ignoreFrames set to false to avoid writing state information about child frames into DOM
     * \see readXml()
     */
    bool writeXml( QDomElement &elem, QDomDocument &doc, const QgsReadWriteContext &context, bool ignoreFrames = false ) const;

    /**
     * Sets the item state from a DOM element.
     * \param itemElement is the DOM node corresponding to item (e.g. 'LayoutItem' element)
     * \param document DOM document
     * \param context read write context
     * \param ignoreFrames set to false to avoid read state information about child frames from DOM
     * \see writeXml()
     */
    bool readXml( const QDomElement &itemElem, const QDomDocument &doc, const QgsReadWriteContext &context, bool ignoreFrames = false );

    /**
     * Returns a list of all child frames for this multiframe.
     * \see frameCount()
     */
    QList<QgsLayoutFrame *> frames() const;

    /**
     * Returns the number of frames associated with this multiframe.
     * \see frames()
     */
    int frameCount() const { return mFrameItems.size(); }

    /**
     * Returns the child frame at a specified \a index from the multiframe.
     * \see frameIndex()
     */
    QgsLayoutFrame *frame( int index ) const;

    /**
     * Returns the index of a \a frame within the multiframe.
     * \returns index for frame if found, -1 if frame not found in multiframe
     * \see frame()
     */
    int frameIndex( QgsLayoutFrame *frame ) const;

    /**
     * Creates a new frame and adds it to the multi frame and layout.
     * \param currentFrame an existing QgsLayoutFrame from which to copy the size
     * and general frame properties (e.g., frame style, background, rendering settings).
     * \param pos position of top-left corner of the new frame, in layout units
     * \param size size of the new frame, in layout units
     */
    QgsLayoutFrame *createNewFrame( QgsLayoutFrame *currentFrame, QPointF pos, QSizeF size );

    /**
     * Returns the multiframe display name.
     */
    virtual QString displayName() const;

    QgsAbstractLayoutUndoCommand *createCommand( const QString &text, int id, QUndoCommand *parent = nullptr ) override SIP_FACTORY;

    /**
     * Starts new undo command for this item.
     * The \a commandText should be a capitalized, imperative tense description (e.g. "Add Map Item").
     * If specified, multiple consecutive commands for this item with the same \a command will
     * be collapsed into a single undo command in the layout history.
     * \see endCommand()
     * \see cancelCommand()
    */
    void beginCommand( const QString &commandText, UndoCommand command = UndoNone );

    /**
     * Completes the current item command and push it onto the layout's undo stack.
     * \see beginCommand()
     * \see cancelCommand()
     */
    void endCommand();

    /**
     * Cancels the current item command and discards it.
     * \see beginCommand()
     * \see endCommand()
     */
    void cancelCommand();

  public slots:

    /**
     * Forces a redraw of all child frames.
     */
    void update();

    /**
     * Recalculates the portion of the multiframe item which is shown in each of its
     * component frames. If the resize mode is set to anything but UseExistingFrames then
     * this may cause new frames to be added or frames to be removed, in order to fit
     * the current size of the multiframe's content.
     * \see recalculateFrameRects()
     */
    virtual void recalculateFrameSizes();

    /**
     * Forces a recalculation of all the associated frame's scene rectangles. This
     * method is useful for multiframes which implement a minFrameSize() or
     * fixedFrameSize() method.
     * \see minFrameSize()
     * \see fixedFrameSize()
     * \see recalculateFrameSizes
     */
    void recalculateFrameRects();

  signals:

    /**
     * Emitted when the properties of a multi frame have changed, and the GUI item widget
     * must be updated.
     */
    void changed();

    /**
     * Emitted when the contents of the multi frame have changed and the frames
     * must be redrawn.
     */
    void contentsChanged();

  protected:

    /**
     * Stores multiframe state within an XML DOM element.
     * \param element is the DOM element to store the multiframe's properties in
     * \param document DOM document
     * \param context read write context
     * \see writeXml()
     * \see readPropertiesFromElement()
     */
    virtual bool writePropertiesToElement( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const;

    /**
     * Sets multiframe state from a DOM element.
     * \param element is the DOM element for the multiframe
     * \param document DOM document
     * \param context read write context
     * \see writePropertiesToElement()
     * \see readXml()
     */
    virtual bool readPropertiesFromElement( const QDomElement &element, const QDomDocument &document, const QgsReadWriteContext &context );

    QList<QgsLayoutFrame *> mFrameItems;

    ResizeMode mResizeMode = UseExistingFrames;

  protected slots:

    /**
     * Adapts to changed number of layout pages if resize type is RepeatOnEveryPage.
     */
    void handlePageChange();

    /**
     * Called when a frame is removed. Updates frame list and recalculates
     * content of remaining frames.
     */
    void handleFrameRemoval();


  private:
    QgsLayoutMultiFrame() = delete;

    bool mIsRecalculatingSize = false;

    bool mBlockUpdates = false;
    bool mBlockUndoCommands = false;

    //! Unique id
    QString mUuid;
};


#endif // QGSLAYOUTMULTIFRAME_H
