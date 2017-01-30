class QgsTextAnnotation : QgsAnnotation
{
%TypeHeaderCode
#include <qgstextannotation.h>
%End
  public:

    QgsTextAnnotation( QObject* parent /TransferThis/ = nullptr );

    const QTextDocument* document() const;
    void setDocument( const QTextDocument* doc );

    virtual void writeXml( QDomElement& elem, QDomDocument & doc ) const;
    virtual void readXml( const QDomElement& itemElem, const QDomDocument& doc );

    static QgsTextAnnotation* create() /Factory/;

  protected:

    void renderAnnotation( QgsRenderContext& context, QSizeF size ) const;

};
