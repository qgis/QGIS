
class QgsComposerMultiFrame: QObject
{
%TypeHeaderCode
#include "qgscomposermultiframe.h"
%End
public:

    enum ResizeMode
    {
      UseExistingFrames = 0,
      ExtendToNextPage //duplicates last frame to next page to fit the total size
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

    int nFrames() const;
};