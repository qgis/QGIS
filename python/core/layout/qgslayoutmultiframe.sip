/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutmultiframe.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsLayoutMultiFrame: QgsLayoutObject, QgsLayoutUndoObjectInterface
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

    enum UndoCommand
    {
      UndoHtmlBreakDistance,
      UndoHtmlSource,
      UndoHtmlStylesheet,
      UndoTableCellStyle,
      UndoTableMaximumFeatures,
      UndoTableMargin,
      UndoTableHeaderFontColor,
      UndoTableContentFontColor,
      UndoTableGridStrokeWidth,
      UndoTableGridColor,
      UndoTableBackgroundColor,
      UndoNone,
    };

    QgsLayoutMultiFrame( QgsLayout *layout /TransferThis/ );
%Docstring
 Construct a new multiframe item, attached to the specified ``layout``.
%End

    ~QgsLayoutMultiFrame();

    QString uuid() const;
%Docstring
 Returns the multiframe identification string. This is a unique random string set for the multiframe
 upon creation.
.. note::

   There is no corresponding setter for the uuid - it's created automatically.
 :rtype: str
%End

    virtual QSizeF totalSize() const = 0;
%Docstring
 Returns the total size of the multiframe's content, in layout units.
 :rtype: QSizeF
%End

    virtual int type() const = 0;
%Docstring
 Returns unique multiframe type id.
 :rtype: int
%End

    virtual QString stringType() const = 0;
%Docstring
 Return the multiframe type as a string.

 This string must be a unique, single word, character only representation of the item type, eg "LayoutHtml"
.. seealso:: type()
 :rtype: str
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

    bool writeXml( QDomElement &elem, QDomDocument &doc, const QgsReadWriteContext &context, bool ignoreFrames = false ) const;
%Docstring
 Stores the multiframe state in a DOM element.
 \param parentElement parent DOM element (e.g. 'Layout' element)
 \param document DOM document
 \param context read write context
 \param ignoreFrames set to false to avoid writing state information about child frames into DOM
.. seealso:: readXml()
 :rtype: bool
%End

    bool readXml( const QDomElement &itemElem, const QDomDocument &doc, const QgsReadWriteContext &context, bool ignoreFrames = false );
%Docstring
 Sets the item state from a DOM element.
 \param itemElement is the DOM node corresponding to item (e.g. 'LayoutItem' element)
 \param document DOM document
 \param context read write context
 \param ignoreFrames set to false to avoid read state information about child frames from DOM
.. seealso:: writeXml()
 :rtype: bool
%End

    QList<QgsLayoutFrame *> frames() const;
%Docstring
 Returns a list of all child frames for this multiframe.
.. seealso:: frameCount()
 :rtype: list of QgsLayoutFrame
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

    virtual QgsAbstractLayoutUndoCommand *createCommand( const QString &text, int id, QUndoCommand *parent = 0 ) /Factory/;


    void beginCommand( const QString &commandText, UndoCommand command = UndoNone );
%Docstring
 Starts new undo command for this item.
 The ``commandText`` should be a capitalized, imperative tense description (e.g. "Add Map Item").
 If specified, multiple consecutive commands for this item with the same ``command`` will
 be collapsed into a single undo command in the layout history.
.. seealso:: endCommand()
.. seealso:: cancelCommand()
%End

    void endCommand();
%Docstring
 Completes the current item command and push it onto the layout's undo stack.
.. seealso:: beginCommand()
.. seealso:: cancelCommand()
%End

    void cancelCommand();
%Docstring
 Cancels the current item command and discards it.
.. seealso:: beginCommand()
.. seealso:: endCommand()
%End

  public slots:

    virtual void refresh();

%Docstring
 Refreshes the multiframe, causing a recalculation of any property overrides.
%End

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

    virtual void refreshDataDefinedProperty( const QgsLayoutObject::DataDefinedProperty property = QgsLayoutObject::AllProperties );
%Docstring
 Refreshes a data defined ``property`` for the multi frame by reevaluating the property's value
 and redrawing the item with this new value. If ``property`` is set to
 QgsLayoutObject.AllProperties then all data defined properties for the item will be
 refreshed.
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

    virtual bool writePropertiesToElement( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const;
%Docstring
 Stores multiframe state within an XML DOM element.
 \param element is the DOM element to store the multiframe's properties in
 \param document DOM document
 \param context read write context
.. seealso:: writeXml()
.. seealso:: readPropertiesFromElement()
 :rtype: bool
%End

    virtual bool readPropertiesFromElement( const QDomElement &element, const QDomDocument &document, const QgsReadWriteContext &context );
%Docstring
 Sets multiframe state from a DOM element.
 \param element is the DOM element for the multiframe
 \param document DOM document
 \param context read write context
.. seealso:: writePropertiesToElement()
.. seealso:: readXml()
 :rtype: bool
%End



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
