%ModuleHeaderCode
// fix to allow compilation with sip 4.7
#include <qgscomposerhtml.h>
#include <qgscomposertablev2.h>
#include <qgscomposerattributetablev2.h>
%End

/**
 * \ingroup composer
 * \class QgsComposerMultiFrame
 * Abstract base class for composer items with the ability to distribute the content to several frames
 * (QgsComposerFrame items).
 */

class QgsComposerMultiFrame: QgsComposerObject
{
%TypeHeaderCode
#include <qgscomposermultiframe.h>
%End

%ConvertToSubClassCode
  if (dynamic_cast<QgsComposerHtml*>(sipCpp) != NULL)
    sipType = sipType_QgsComposerHtml;
  else if (dynamic_cast<QgsComposerAttributeTableV2*>(sipCpp) != NULL)
    sipType = sipType_QgsComposerAttributeTableV2;
  else if (dynamic_cast<QgsComposerTableV2*>(sipCpp) != NULL)
    sipType = sipType_QgsComposerTableV2;
  else if (dynamic_cast<QgsComposerMultiFrame*>(sipCpp) != NULL)
    sipType = sipType_QgsComposerMultiFrame;
  else
    sipType = NULL;
%End

  public:

    /** Specifies the behaviour for creating new frames to fit the multiframe's content
     */
    enum ResizeMode
    {
      UseExistingFrames, /*!< don't automatically create new frames, just use existing frames */
      ExtendToNextPage, /*!< creates new full page frames on the following page(s) until the entire multiframe content is visible */
      RepeatOnEveryPage, /*!< repeats the same frame on every page */
      RepeatUntilFinished /*!< creates new frames with the same position and dimensions as the existing frame on the following page(s),
                              until the entire multiframe content is visible */
    };

    /** Construct a new multiframe item.
     * @param c parent composition
     * @param createUndoCommands
     */
    QgsComposerMultiFrame( QgsComposition* c /TransferThis/, bool createUndoCommands );

    virtual ~QgsComposerMultiFrame();

    /** Returns the total size of the multiframe's content.
     * @returns total size required for content
     */
    virtual QSizeF totalSize() const = 0;

    /** Returns the fixed size for a frame, if desired. If the fixed frame size changes,
     * the sizes of all frames can be recalculated by calling recalculateFrameRects().
     * @param frameIndex frame number
     * @returns fixed size for frame. If the size has a width or height of 0, then
     * the frame size is not fixed in that direction and frames can have variable width
     * or height accordingly.
     * @note added in version 2.5
     * @see minFrameSize
     * @see recalculateFrameRects
     */
    virtual QSizeF fixedFrameSize( const int frameIndex = -1 ) const;

    /** Returns the minimum size for a frames, if desired. If the minimum
     * size changes, the sizes of all frames can be recalculated by calling
     * recalculateFrameRects().
     * @param frameIndex frame number
     * @returns minimum size for frame. If the size has a width or height of 0, then
     * the frame size has no minimum in that direction.
     * @note added in version 2.5
     * @see fixedFrameSize
     * @see recalculateFrameRects
     */
    virtual QSizeF minFrameSize( const int frameIndex = -1 ) const;

    /** Renders a portion of the multiframe's content into a painter.
     * @param p destination painter
     * @param renderExtent visible extent of content to render into the painter.
     * @deprecated use render( QPainter* painter, const QRectF& renderExtent, const int frameIndex ) instead
     */
    virtual void render( QPainter* p, const QRectF& renderExtent ) /Deprecated/;

    /** Renders a portion of the multiframe's content into a painter.
     * @param painter destination painter
     * @param renderExtent visible extent of content to render into the painter.
     * @param frameIndex frame number for content
     * @note added in version 2.5
     */
    virtual void render( QPainter* painter, const QRectF& renderExtent, const int frameIndex );

    /** Adds a frame to the multiframe.
     * @param frame frame to add
     * @param recalcFrameSizes set to true to force recalculation of all existing frame sizes
     * @see removeFrame
     */
    virtual void addFrame( QgsComposerFrame* frame /Transfer/, bool recalcFrameSizes = true ) = 0;

    /** Finds the optimal position to break a frame at.
     * @param yPos maximum vertical position for break
     * @returns the optimal breakable position which occurs in the multi frame close
     * to and before the specified yPos
     * @note added in version 2.3
     */
    virtual double findNearbyPageBreak( double yPos );

    /** Removes a frame from the multiframe. This method automatically removes the frame from the
     * composition.
     * @param i index of frame to remove
     * @param removeEmptyPages set to true to remove pages which are empty after the frame is removed
     * @see addFrame
     * @see deleteFrames
     */
    void removeFrame( int i, const bool removeEmptyPages = false );

    /** Removes and deletes all child frames.
     * @see removeFrame
     */
    void deleteFrames();

    /** Sets the resize mode for the multiframe, and recalculates frame sizes to match.
     * @param mode resize mode
     * @see resizeMode
     */
    void setResizeMode( ResizeMode mode );

    /** Returns the resize mode for the multiframe.
     * @returns resize mode
     * @see setResizeMode
     */
    ResizeMode resizeMode() const;

    /** Stores state information about multiframe in DOM element. Implementations of writeXML
     * should also call the _writeXML method to save general multiframe properties.
     * @param elem is DOM element
     * @param doc is the DOM document
     * @param ignoreFrames set to false to avoid writing state information about child frames into DOM
     * @see _writeXML
     */
    virtual bool writeXML( QDomElement& elem, QDomDocument & doc, bool ignoreFrames = false ) const = 0;

    /** Stores state information about base multiframe object in DOM element. Implementations of writeXML
     * should call this method.
     * @param elem is DOM element
     * @param doc is the DOM document
     * @param ignoreFrames set to false to avoid writing state information about child frames into DOM
     * @see writeXML
     */
    bool _writeXML( QDomElement& elem, QDomDocument& doc, bool ignoreFrames = false ) const;

    /** Reads multiframe state information from a DOM element. Implementations of readXML
     * should also call the _readXML method to restore general multiframe properties.
     * @param itemElem is DOM element
     * @param doc is the DOM document
     * @param ignoreFrames set to false to avoid read state information about child frames from DOM
     * @see _readXML
     */
    virtual bool readXML( const QDomElement& itemElem, const QDomDocument& doc, bool ignoreFrames = false ) = 0;

    /** Restores state information about base multiframe object from a DOM element. Implementations of readXML
     * should call this method.
     * @param itemElem is DOM element
     * @param doc is the DOM document
     * @param ignoreFrames set to false to avoid reading state information about child frames from DOM
     * @see readXML
     */
    bool _readXML( const QDomElement& itemElem, const QDomDocument& doc, bool ignoreFrames = false );

    /** Returns the parent composition for the multiframe.
     * @returns composition
     */
    QgsComposition* composition();

    /** Returns whether undo commands should be created for interactions with the multiframe.
     * @returns true if undo commands should be created
     * @see setCreateUndoCommands
     */
    bool createUndoCommands() const;

    /** Sets whether undo commands should be created for interactions with the multiframe.
     * @param enabled set to true if undo commands should be created
     * @see createUndoCommands
     */
    void setCreateUndoCommands( bool enabled );

    /** Returns the number of frames associated with this multiframe.
     * @returns number of child frames
    **/
    int frameCount() const;

    /** Returns a child frame from the multiframe.
     * @param i index of frame
     * @returns child frame if found
     * @see frameIndex
     */
    QgsComposerFrame* frame( int i );

    /** Returns the index of a frame within the multiframe
     * @param frame frame to find index of
     * @returns index for frame if found, -1 if frame not found in multiframe
     * @note added in version 2.5
     * @see frame
     */
    int frameIndex( QgsComposerFrame *frame ) const;

    /** Creates a new frame and adds it to the multi frame and composition.
     * @param currentFrame an existing QgsComposerFrame from which to copy the size
     * and general frame properties (eg frame style, background, rendering settings).
     * @param pos position of top-left corner of the new frame
     * @param size size of the new frame
     * @returns new QgsComposerFrame
     * @note added in version 2.3
     */
    QgsComposerFrame* createNewFrame( QgsComposerFrame* currentFrame, QPointF pos, QSizeF size );

    /** Get multiframe display name.
     * @returns display name for item
     * @note added in version 2.5
     */
    virtual QString displayName() const;

  public slots:

    /** Forces a redraw of all child frames.
     */
    void update();

    /** Recalculates the portion of the multiframe item which is shown in each of it's
     * component frames. If the resize mode is set to anything but UseExistingFrames then
     * this may cause new frames to be added or frames to be removed, in order to fit
     * the current size of the multiframe's content.
     * @see recalculateFrameRects
     */
    virtual void recalculateFrameSizes();

    /** Forces a recalculation of all the associated frame's scene rectangles. This
     * method is useful for multiframes which implement a minFrameSize() or
     * fixedFrameSize() method.
     * @note added in version 2.5
     * @see minFrameSize()
     * @see fixedFrameSize()
     * @see recalculateFrameSizes
     */
    void recalculateFrameRects();

  signals:

    /** Emitted when the properties of a multi frame have changed, and the GUI item widget
     * must be updated.
     */
    void changed();

    /** Emitted when the contents of the multi frame have changed and the frames
     * must be redrawn.
     */
    void contentsChanged();

  protected slots:

    /** Called before a frame is going to be removed. Updates frame list and recalculates
     * content of remaining frames.
     */
    void handleFrameRemoval( QgsComposerItem* item );

    /** Adapts to changed number of composition pages if resize type is RepeatOnEveryPage.
     */
    void handlePageChange();

};
