/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposermultiframe.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





%ModuleHeaderCode
#include <qgscomposerhtml.h>
#include <qgscomposertablev2.h>
#include <qgscomposerattributetablev2.h>
%End


class QgsComposerMultiFrame: QgsComposerObject
{
%Docstring
Abstract base class for composer items with the ability to distribute the content to several frames
(QgsComposerFrame items).
%End

%TypeHeaderCode
#include "qgscomposermultiframe.h"
%End
%ConvertToSubClassCode
    if ( dynamic_cast<QgsComposerHtml *>( sipCpp ) != NULL )
      sipType = sipType_QgsComposerHtml;
    else if ( dynamic_cast<QgsComposerAttributeTableV2 *>( sipCpp ) != NULL )
      sipType = sipType_QgsComposerAttributeTableV2;
    else if ( dynamic_cast<QgsComposerTableV2 *>( sipCpp ) != NULL )
      sipType = sipType_QgsComposerTableV2;
    else if ( dynamic_cast<QgsComposerMultiFrame *>( sipCpp ) != NULL )
      sipType = sipType_QgsComposerMultiFrame;
    else
      sipType = NULL;
%End
  public:

    enum ResizeMode
    {
      UseExistingFrames,
      ExtendToNextPage,
      RepeatOnEveryPage,
      RepeatUntilFinished
    };

    QgsComposerMultiFrame( QgsComposition *c /TransferThis/, bool createUndoCommands );
%Docstring
Construct a new multiframe item.

:param c: parent composition
:param createUndoCommands:
%End

    ~QgsComposerMultiFrame();

    virtual QSizeF totalSize() const = 0;
%Docstring
Returns the total size of the multiframe's content.

:return: total size required for content
%End

    virtual QSizeF fixedFrameSize( const int frameIndex = -1 ) const;
%Docstring
Returns the fixed size for a frame, if desired. If the fixed frame size changes,
the sizes of all frames can be recalculated by calling recalculateFrameRects().

:param frameIndex: frame number

:return: fixed size for frame. If the size has a width or height of 0, then
the frame size is not fixed in that direction and frames can have variable width
or height accordingly.

.. versionadded:: 2.5

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

.. versionadded:: 2.5

.. seealso:: :py:func:`fixedFrameSize`

.. seealso:: :py:func:`recalculateFrameRects`
%End

    virtual void render( QPainter *painter, const QRectF &renderExtent, const int frameIndex ) = 0;
%Docstring
Renders a portion of the multiframe's content into a painter.

:param painter: destination painter
:param renderExtent: visible extent of content to render into the painter.
:param frameIndex: frame number for content

.. versionadded:: 2.5
%End

    virtual void addFrame( QgsComposerFrame *frame /Transfer/, bool recalcFrameSizes = true ) = 0;
%Docstring
Adds a frame to the multiframe.

:param frame: frame to add
:param recalcFrameSizes: set to true to force recalculation of all existing frame sizes

.. seealso:: :py:func:`removeFrame`
%End

    virtual double findNearbyPageBreak( double yPos );
%Docstring
Finds the optimal position to break a frame at.

:param yPos: maximum vertical position for break

:return: the optimal breakable position which occurs in the multi frame close
to and before the specified yPos

.. versionadded:: 2.3
%End

    void removeFrame( int i, const bool removeEmptyPages = false );
%Docstring
Removes a frame from the multiframe. This method automatically removes the frame from the
composition.

:param i: index of frame to remove
:param removeEmptyPages: set to true to remove pages which are empty after the frame is removed

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
Sets the resize mode for the multiframe, and recalculates frame sizes to match.

:param mode: resize mode

.. seealso:: :py:func:`resizeMode`
%End

    ResizeMode resizeMode() const;
%Docstring
Returns the resize mode for the multiframe.

:return: resize mode

.. seealso:: :py:func:`setResizeMode`
%End

    virtual bool writeXml( QDomElement &elem, QDomDocument &doc, bool ignoreFrames = false ) const = 0;
%Docstring
Stores state information about multiframe in DOM element. Implementations of writeXml
should also call the _writeXML method to save general multiframe properties.

:param elem: is DOM element
:param doc: is the DOM document
:param ignoreFrames: set to false to avoid writing state information about child frames into DOM

.. seealso:: _writeXML
%End

    bool _writeXml( QDomElement &elem, QDomDocument &doc, bool ignoreFrames = false ) const;
%Docstring
Stores state information about base multiframe object in DOM element. Implementations of writeXml
should call this method.

:param elem: is DOM element
:param doc: is the DOM document
:param ignoreFrames: set to false to avoid writing state information about child frames into DOM

.. seealso:: :py:func:`writeXml`
%End

    virtual bool readXml( const QDomElement &itemElem, const QDomDocument &doc, bool ignoreFrames = false ) = 0;
%Docstring
Reads multiframe state information from a DOM element. Implementations of readXml
should also call the _readXML method to restore general multiframe properties.

:param itemElem: is DOM element
:param doc: is the DOM document
:param ignoreFrames: set to false to avoid read state information about child frames from DOM

.. seealso:: _readXML
%End

    bool _readXml( const QDomElement &itemElem, const QDomDocument &doc, bool ignoreFrames = false );
%Docstring
Restores state information about base multiframe object from a DOM element. Implementations of readXml
should call this method.

:param itemElem: is DOM element
:param doc: is the DOM document
:param ignoreFrames: set to false to avoid reading state information about child frames from DOM

.. seealso:: :py:func:`readXml`
%End

    QgsComposition *composition();
%Docstring
Returns the parent composition for the multiframe.

:return: composition
%End

    bool createUndoCommands() const;
%Docstring
Returns whether undo commands should be created for interactions with the multiframe.

:return: true if undo commands should be created

.. seealso:: :py:func:`setCreateUndoCommands`
%End

    void setCreateUndoCommands( bool enabled );
%Docstring
Sets whether undo commands should be created for interactions with the multiframe.

:param enabled: set to true if undo commands should be created

.. seealso:: :py:func:`createUndoCommands`
%End

    int frameCount() const;
%Docstring
Returns the number of frames associated with this multiframe.

:return: number of child frames
*
%End

    QgsComposerFrame *frame( int i ) const;
%Docstring
Returns a child frame from the multiframe.

:param i: index of frame

:return: child frame if found

.. seealso:: :py:func:`frameIndex`
%End

    int frameIndex( QgsComposerFrame *frame ) const;
%Docstring
Returns the index of a frame within the multiframe

:param frame: frame to find index of

:return: index for frame if found, -1 if frame not found in multiframe

.. versionadded:: 2.5

.. seealso:: :py:func:`frame`
%End

    QgsComposerFrame *createNewFrame( QgsComposerFrame *currentFrame, QPointF pos, QSizeF size );
%Docstring
Creates a new frame and adds it to the multi frame and composition.

:param currentFrame: an existing QgsComposerFrame from which to copy the size
and general frame properties (e.g., frame style, background, rendering settings).
:param pos: position of top-left corner of the new frame
:param size: size of the new frame

:return: new QgsComposerFrame

.. versionadded:: 2.3
%End

    virtual QString displayName() const;
%Docstring
Get multiframe display name.

:return: display name for item

.. versionadded:: 2.5
%End

  public slots:

    void update();
%Docstring
Forces a redraw of all child frames.
%End

    virtual void recalculateFrameSizes();
%Docstring
Recalculates the portion of the multiframe item which is shown in each of it's
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

.. versionadded:: 2.5

.. seealso:: :py:func:`minFrameSize()`

.. seealso:: :py:func:`fixedFrameSize()`

.. seealso:: :py:func:`recalculateFrameSizes`
%End

    void handleFrameRemoval( QgsComposerItem *item );
%Docstring
Called before a frame is going to be removed. Updates frame list and recalculates
content of remaining frames.
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
Adapts to changed number of composition pages if resize type is RepeatOnEveryPage.
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposermultiframe.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
