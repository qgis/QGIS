class QgsComposerHtml: QgsComposerMultiFrame
{
%TypeHeaderCode
#include "qgscomposerhtml.h"
%End

 public:
    QgsComposerHtml( QgsComposition* c, bool createUndoCommands );
    QgsComposerHtml();
    ~QgsComposerHtml();

    void setUrl( const QUrl& url );
    const QUrl& url() const;

    QSizeF totalSize() const;
    void render( QPainter* p, const QRectF& renderExtent );

    bool writeXML( QDomElement& elem, QDomDocument & doc, bool ignoreFrames = false ) const;
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc, bool ignoreFrames = false );

    void addFrame( QgsComposerFrame* frame, bool recalcFrameSizes = true );
};