
class QgsComposerMultiFrame: QObject
{
%TypeHeaderCode
#include "qgscomposermultiframe.h"
%End

  public:

    enum ResizeMode
    {
      UseExistingFrames = 0,
      ExtendToNextPage, //uses the next page(s) until the content has been printed
      RepeatOnEveryPage, //repeats the same frame on every page
      RepeatUntilFinished //duplicates last frame to next page to fit the total size
    };

    QgsComposerMultiFrame( QgsComposition* c, bool createUndoCommands );
    virtual ~QgsComposerMultiFrame();
    virtual QSizeF totalSize() const = 0;
    virtual void render( QPainter* p, const QRectF& renderExtent ) = 0;

    virtual void addFrame( QgsComposerFrame* frame, bool recalcFrameSizes = true ) = 0;

    void removeFrame( int i );

    void update();

    void setResizeMode( ResizeMode mode );
    ResizeMode resizeMode() const;

    virtual bool writeXML( QDomElement& elem, QDomDocument & doc, bool ignoreFrames = false ) const = 0;
    bool _writeXML( QDomElement& elem, QDomDocument& doc, bool ignoreFrames = false ) const;

    virtual bool readXML( const QDomElement& itemElem, const QDomDocument& doc, bool ignoreFrames = false ) = 0;
    bool _readXML( const QDomElement& itemElem, const QDomDocument& doc, bool ignoreFrames = false );

    QgsComposition* composition();

    bool createUndoCommands() const;
    void setCreateUndoCommands( bool enabled );

    /**Removes and deletes all frames from mComposition*/
    void deleteFrames();

    /** Return the number of frames associated with this multiframeset.
       @note added in 2.0, replaces nFrames
    **/
    int frameCount() const;
    QgsComposerFrame* frame( int i );

  protected slots:
    void recalculateFrameSizes();
    /**Called before a frame is going to be removed (update frame list)*/
    void handleFrameRemoval( QgsComposerItem* item );
    /**Adapts to changed number of pages if resize type is RepeatOnEveryPage*/
    void handlePageChange();

  signals:
    void changed();
};
