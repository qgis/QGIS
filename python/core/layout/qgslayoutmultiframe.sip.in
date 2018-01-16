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
%End

    virtual QSizeF totalSize() const = 0;
%Docstring
Returns the total size of the multiframe's content, in layout units.
%End

    virtual int type() const = 0;
%Docstring
Returns unique multiframe type id.
%End

    virtual QIcon icon() const;
%Docstring
Returns the item's icon.
%End

    virtual QSizeF fixedFrameSize( const int frameIndex = -1 ) const;
%Docstring
Returns the fixed size for a frame, if desired. If the fixed frame size changes,
the sizes of all frames can be recalculated by calling recalculateFrameRects().

:param frameIndex: frame number

:return: fixed size for frame. If the size has a width or height of 0, then
the frame size is not fixed in that direction and frames can have variable width
or height accordingly.

.. seealso:: :py:func:`minFrameSize`

.. seealso:: :py:func:`recalculateFrameRects`
%End

    virtual QSizeF minFrameSize( const int frameIndex = -1 ) const;
%Docstring
Returns the minimum size for a frames, if desired. If the minimum
size changes, the sizes of all frames can be recalculated by calling
recalculateFrameRects().

:param frameIndex: frame number

:return: minimum size for frame. If the size has a width or height of 0, then
the frame size has no minimum in that direction.

.. seealso:: :py:func:`fixedFrameSize`

.. seealso:: :py:func:`recalculateFrameRects`
%End

    virtual void render( QgsRenderContext &context, const QRectF &renderExtent, int frameIndex,
                         const QStyleOptionGraphicsItem *itemStyle = 0 ) = 0;
%Docstring
Renders a portion of the multiframe's content into a render ``context``.

:param context: destination render painter
:param renderExtent: visible extent of content to render into the painter.
:param frameIndex: frame number for content
:param itemStyle: item style options for graphics item rendering
%End

    virtual void addFrame( QgsLayoutFrame *frame /Transfer/, bool recalcFrameSizes = true );
%Docstring
Adds a ``frame`` to the multiframe.

If ``recalcFrameSizes`` is set to true, then a recalculation of all existing frame sizes will be forced.

.. seealso:: :py:func:`removeFrame`
%End

    virtual double findNearbyPageBreak( double yPos );
%Docstring
Finds the optimal position to break a frame at.

:param yPos: maximum vertical position for break, in layout units.

:return: the optimal breakable position which occurs in the multi frame close
to and before the specified yPos
%End

    void removeFrame( int index, bool removeEmptyPages = false );
%Docstring
Removes a frame by ``index`` from the multiframe. This method automatically removes the frame from the
layout too.

If ``removeEmptyPages`` is set to true, then pages which are empty after the frame is removed will
also be removed from the layout.

.. seealso:: :py:func:`addFrame`

.. seealso:: :py:func:`deleteFrames`
%End

    void deleteFrames();
%Docstring
Removes and deletes all child frames.

.. seealso:: :py:func:`removeFrame`
%End

    void setResizeMode( ResizeMode mode );
%Docstring
Sets the resize ``mode`` for the multiframe, and recalculates frame sizes to match.

.. seealso:: :py:func:`resizeMode`
%End

    ResizeMode resizeMode() const;
%Docstring
Returns the resize mode for the multiframe.

.. seealso:: :py:func:`setResizeMode`
%End

    bool writeXml( QDomElement &parentElement, QDomDocument &document, const QgsReadWriteContext &context, bool ignoreFrames = false ) const;
%Docstring
Stores the multiframe state in a DOM element.

:param parentElement: parent DOM element (e.g. 'Layout' element)
:param document: DOM document
:param context: read write context
:param ignoreFrames: set to false to avoid writing state information about child frames into DOM

.. seealso:: :py:func:`readXml`
%End

    bool readXml( const QDomElement &itemElement, const QDomDocument &document, const QgsReadWriteContext &context, bool ignoreFrames = false );
%Docstring
Sets the item state from a DOM element.

:param itemElement: is the DOM node corresponding to item (e.g. 'LayoutItem' element)
:param document: DOM document
:param context: read write context
:param ignoreFrames: set to false to avoid read state information about child frames from DOM

.. seealso:: :py:func:`writeXml`
%End

    QList<QgsLayoutFrame *> frames() const;
%Docstring
Returns a list of all child frames for this multiframe.

.. seealso:: :py:func:`frameCount`
%End

    int frameCount() const;
%Docstring
Returns the number of frames associated with this multiframe.

.. seealso:: :py:func:`frames`
%End

    QgsLayoutFrame *frame( int index ) const;
%Docstring
Returns the child frame at a specified ``index`` from the multiframe.

.. seealso:: :py:func:`frameIndex`
%End

    int frameIndex( QgsLayoutFrame *frame ) const;
%Docstring
Returns the index of a ``frame`` within the multiframe.

:return: index for frame if found, -1 if frame not found in multiframe

.. seealso:: :py:func:`frame`
%End

    QgsLayoutFrame *createNewFrame( QgsLayoutFrame *currentFrame, QPointF pos, QSizeF size );
%Docstring
Creates a new frame and adds it to the multi frame and layout.

:param currentFrame: an existing QgsLayoutFrame from which to copy the size
and general frame properties (e.g., frame style, background, rendering settings).
:param pos: position of top-left corner of the new frame, in layout units
:param size: size of the new frame, in layout units
%End

    virtual QString displayName() const;
%Docstring
Returns the multiframe display name.
%End

    virtual QgsAbstractLayoutUndoCommand *createCommand( const QString &text, int id, QUndoCommand *parent = 0 ) /Factory/;


    void beginCommand( const QString &commandText, UndoCommand command = UndoNone );
%Docstring
Starts new undo command for this item.
The ``commandText`` should be a capitalized, imperative tense description (e.g. "Add Map Item").
If specified, multiple consecutive commands for this item with the same ``command`` will
be collapsed into a single undo command in the layout history.

.. seealso:: :py:func:`endCommand`

.. seealso:: :py:func:`cancelCommand`
%End

    void endCommand();
%Docstring
Completes the current item command and push it onto the layout's undo stack.

.. seealso:: :py:func:`beginCommand`

.. seealso:: :py:func:`cancelCommand`
%End

    void cancelCommand();
%Docstring
Cancels the current item command and discards it.

.. seealso:: :py:func:`beginCommand`

.. seealso:: :py:func:`endCommand`
%End

    virtual void finalizeRestoreFromXml();
%Docstring
Called after all pending items have been restored from XML. Multiframes can use
this method to run steps which must take place after all items have been restored to the layout,
such as connecting to signals emitted by other items, which may not have existed in the layout
at the time readXml() was called. E.g. a scalebar can use this to connect to its linked
map item after restoration from XML.

.. seealso:: :py:func:`readXml`
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

.. seealso:: :py:func:`recalculateFrameRects`
%End

    void recalculateFrameRects();
%Docstring
Forces a recalculation of all the associated frame's scene rectangles. This
method is useful for multiframes which implement a minFrameSize() or
fixedFrameSize() method.

.. seealso:: :py:func:`minFrameSize`

.. seealso:: :py:func:`fixedFrameSize`

.. seealso:: :py:func:`recalculateFrameSizes`
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

:param element: is the DOM element to store the multiframe's properties in
:param document: DOM document
:param context: read write context

.. seealso:: :py:func:`writeXml`

.. seealso:: :py:func:`readPropertiesFromElement`
%End

    virtual bool readPropertiesFromElement( const QDomElement &element, const QDomDocument &document, const QgsReadWriteContext &context );
%Docstring
Sets multiframe state from a DOM element.

:param element: is the DOM element for the multiframe
:param document: DOM document
:param context: read write context

.. seealso:: :py:func:`writePropertiesToElement`

Note that item subclasses should not rely on all other items being present in the
layout at the time this method is called. Instead, any connections and links to
other items must be made in the finalizeRestoreFromXml() method. E.g. when restoring
a scalebar, the connection to the linked map's signals should be implemented
in finalizeRestoreFromXml(), not readPropertiesFromElement().

.. seealso:: :py:func:`readXml`
%End



};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutmultiframe.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
