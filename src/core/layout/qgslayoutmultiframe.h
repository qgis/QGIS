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

// sip crashes out on this file - reexamine after composer removal
#ifndef SIP_RUN

class CORE_EXPORT QgsLayoutMultiFrame: public QgsLayoutObject
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

    /**
     * Construct a new multiframe item, attached to the specified \a layout.
     */
    QgsLayoutMultiFrame( QgsLayout *layout SIP_TRANSFERTHIS );

    ~QgsLayoutMultiFrame();

    /**
     * Returns the total size of the multiframe's content, in layout units.
     */
    virtual QSizeF totalSize() const = 0;

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
     * Stores state information about multiframe in DOM element. Implementations of writeXml
     * should also call the _writeXML method to save general multiframe properties.
     * \param elem is DOM element
     * \param doc is the DOM document
     * \param ignoreFrames set to false to avoid writing state information about child frames into DOM
     * \see _writeXML
     */
    virtual bool writeXml( QDomElement &elem, QDomDocument &doc, bool ignoreFrames = false ) const = 0;

    /**
     * Stores state information about base multiframe object in DOM element. Implementations of writeXml
     * should call this method.
     * \param elem is DOM element
     * \param doc is the DOM document
     * \param ignoreFrames set to false to avoid writing state information about child frames into DOM
     * \see writeXml
     */
    bool _writeXml( QDomElement &elem, QDomDocument &doc, bool ignoreFrames = false ) const;

    /**
     * Reads multiframe state information from a DOM element. Implementations of readXml
     * should also call the _readXML method to restore general multiframe properties.
     * \param itemElem is DOM element
     * \param doc is the DOM document
     * \param ignoreFrames set to false to avoid read state information about child frames from DOM
     * \see _readXML
     */
    virtual bool readXml( const QDomElement &itemElem, const QDomDocument &doc, bool ignoreFrames = false ) = 0;

    /**
     * Restores state information about base multiframe object from a DOM element. Implementations of readXml
     * should call this method.
     * \param itemElem is DOM element
     * \param doc is the DOM document
     * \param ignoreFrames set to false to avoid reading state information about child frames from DOM
     * \see readXml
     */
    bool _readXml( const QDomElement &itemElem, const QDomDocument &doc, bool ignoreFrames = false );

    /**
     * Returns a list of all child frames for this multiframe.
     * \see frameCount()
     * \note Not available in Python bindings
     */
    QList<QgsLayoutFrame *> frames() const SIP_SKIP;

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
};

#endif

#endif // QGSLAYOUTMULTIFRAME_H
