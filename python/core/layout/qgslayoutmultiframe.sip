/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutmultiframe.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsLayoutMultiFrame: QgsLayoutObject
{
%Docstring
 Abstract base class for layout items with the ability to distribute the content to
 several frames (QgsLayoutFrame items).
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutmultiframe.h"
%End
  public:

    enum ResizeMode
    {
      UseExistingFrames,
      ExtendToNextPage,
      RepeatOnEveryPage,
      RepeatUntilFinished
    };

    QgsLayoutMultiFrame( QgsLayout *layout /TransferThis/ );
%Docstring
 Construct a new multiframe item, attached to the specified ``layout``.
%End

    ~QgsLayoutMultiFrame();

    virtual QSizeF totalSize() const = 0;
%Docstring
 Returns the total size of the multiframe's content, in layout units.
 :rtype: QSizeF
%End

    virtual QSizeF fixedFrameSize( const int frameIndex = -1 ) const;
%Docstring
 Returns the fixed size for a frame, if desired. If the fixed frame size changes,
 the sizes of all frames can be recalculated by calling recalculateFrameRects().
 \param frameIndex frame number
 :return: fixed size for frame. If the size has a width or height of 0, then
 the frame size is not fixed in that direction and frames can have variable width
 or height accordingly.
.. seealso:: minFrameSize()
.. seealso:: recalculateFrameRects()
 :rtype: QSizeF
%End

    virtual QSizeF minFrameSize( const int frameIndex = -1 ) const;
%Docstring
 Returns the minimum size for a frames, if desired. If the minimum
 size changes, the sizes of all frames can be recalculated by calling
 recalculateFrameRects().
 \param frameIndex frame number
 :return: minimum size for frame. If the size has a width or height of 0, then
 the frame size has no minimum in that direction.
.. seealso:: fixedFrameSize()
.. seealso:: recalculateFrameRects()
 :rtype: QSizeF
%End

    virtual void render( QgsRenderContext &context, const QRectF &renderExtent, const int frameIndex,
                         const QStyleOptionGraphicsItem *itemStyle = 0 ) = 0;
%Docstring
 Renders a portion of the multiframe's content into a render ``context``.
 \param context destination render painter
 \param renderExtent visible extent of content to render into the painter.
 \param frameIndex frame number for content
 \param itemStyle item style options for graphics item rendering
%End

    virtual void addFrame( QgsLayoutFrame *frame /Transfer/, bool recalcFrameSizes = true );
%Docstring
 Adds a ``frame`` to the multiframe.

 If ``recalcFrameSizes`` is set to true, then a recalculation of all existing frame sizes will be forced.

.. seealso:: removeFrame()
%End

    virtual double findNearbyPageBreak( double yPos );
%Docstring
 Finds the optimal position to break a frame at.
 \param yPos maximum vertical position for break, in layout units.
 :return: the optimal breakable position which occurs in the multi frame close
 to and before the specified yPos
 :rtype: float
%End

    void removeFrame( int index, bool removeEmptyPages = false );
%Docstring
 Removes a frame by ``index`` from the multiframe. This method automatically removes the frame from the
 layout too.

 If ``removeEmptyPages`` is set to true, then pages which are empty after the frame is removed will
 also be removed from the layout.

.. seealso:: addFrame()
.. seealso:: deleteFrames()
%End

    void deleteFrames();
%Docstring
 Removes and deletes all child frames.
.. seealso:: removeFrame()
%End

    void setResizeMode( ResizeMode mode );
%Docstring
 Sets the resize ``mode`` for the multiframe, and recalculates frame sizes to match.
.. seealso:: resizeMode()
%End

    ResizeMode resizeMode() const;
%Docstring
 Returns the resize mode for the multiframe.
.. seealso:: setResizeMode()
 :rtype: ResizeMode
%End

    virtual bool writeXml( QDomElement &elem, QDomDocument &doc, bool ignoreFrames = false ) const = 0;
%Docstring
 Stores state information about multiframe in DOM element. Implementations of writeXml
 should also call the _writeXML method to save general multiframe properties.
 \param elem is DOM element
 \param doc is the DOM document
 \param ignoreFrames set to false to avoid writing state information about child frames into DOM
.. seealso:: _writeXML
 :rtype: bool
%End

    bool _writeXml( QDomElement &elem, QDomDocument &doc, bool ignoreFrames = false ) const;
%Docstring
 Stores state information about base multiframe object in DOM element. Implementations of writeXml
 should call this method.
 \param elem is DOM element
 \param doc is the DOM document
 \param ignoreFrames set to false to avoid writing state information about child frames into DOM
.. seealso:: writeXml
 :rtype: bool
%End

    virtual bool readXml( const QDomElement &itemElem, const QDomDocument &doc, bool ignoreFrames = false ) = 0;
%Docstring
 Reads multiframe state information from a DOM element. Implementations of readXml
 should also call the _readXML method to restore general multiframe properties.
 \param itemElem is DOM element
 \param doc is the DOM document
 \param ignoreFrames set to false to avoid read state information about child frames from DOM
.. seealso:: _readXML
 :rtype: bool
%End

    bool _readXml( const QDomElement &itemElem, const QDomDocument &doc, bool ignoreFrames = false );
%Docstring
 Restores state information about base multiframe object from a DOM element. Implementations of readXml
 should call this method.
 \param itemElem is DOM element
 \param doc is the DOM document
 \param ignoreFrames set to false to avoid reading state information about child frames from DOM
.. seealso:: readXml
 :rtype: bool
%End


    int frameCount() const;
%Docstring
 Returns the number of frames associated with this multiframe.
.. seealso:: frames()
 :rtype: int
%End

    QgsLayoutFrame *frame( int index ) const;
%Docstring
 Returns the child frame at a specified ``index`` from the multiframe.
.. seealso:: frameIndex()
 :rtype: QgsLayoutFrame
%End

    int frameIndex( QgsLayoutFrame *frame ) const;
%Docstring
 Returns the index of a ``frame`` within the multiframe.
 :return: index for frame if found, -1 if frame not found in multiframe
.. seealso:: frame()
 :rtype: int
%End

    QgsLayoutFrame *createNewFrame( QgsLayoutFrame *currentFrame, QPointF pos, QSizeF size );
%Docstring
 Creates a new frame and adds it to the multi frame and layout.
 \param currentFrame an existing QgsLayoutFrame from which to copy the size
 and general frame properties (e.g., frame style, background, rendering settings).
 \param pos position of top-left corner of the new frame, in layout units
 \param size size of the new frame, in layout units
 :rtype: QgsLayoutFrame
%End

    virtual QString displayName() const;
%Docstring
 Returns the multiframe display name.
 :rtype: str
%End

  public slots:

    void update();
%Docstring
 Forces a redraw of all child frames.
%End

    virtual void recalculateFrameSizes();
%Docstring
 Recalculates the portion of the multiframe item which is shown in each of its
 component frames. If the resize mode is set to anything but UseExistingFrames then
 this may cause new frames to be added or frames to be removed, in order to fit
 the current size of the multiframe's content.
.. seealso:: recalculateFrameRects()
%End

    void recalculateFrameRects();
%Docstring
 Forces a recalculation of all the associated frame's scene rectangles. This
 method is useful for multiframes which implement a minFrameSize() or
 fixedFrameSize() method.
.. seealso:: minFrameSize()
.. seealso:: fixedFrameSize()
.. seealso:: recalculateFrameSizes
%End

  signals:

    void changed();
%Docstring
 Emitted when the properties of a multi frame have changed, and the GUI item widget
 must be updated.
%End

    void contentsChanged();
%Docstring
 Emitted when the contents of the multi frame have changed and the frames
 must be redrawn.
%End

  protected:



  protected slots:

    void handlePageChange();
%Docstring
 Adapts to changed number of layout pages if resize type is RepeatOnEveryPage.
%End

    void handleFrameRemoval();
%Docstring
 Called when a frame is removed. Updates frame list and recalculates
 content of remaining frames.
%End


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutmultiframe.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
