class QgsHtmlAnnotation : QgsAnnotation
{
%TypeHeaderCode
#include <qgshtmlannotation.h>
%End
  public:

    QgsHtmlAnnotation( QObject* parent /TransferThis/ = nullptr );

    ~QgsHtmlAnnotation();

    QSizeF minimumFrameSize() const;

    void setSourceFile( const QString& htmlFile );
    QString sourceFile() const;

    virtual void writeXml( QDomElement& elem, QDomDocument & doc ) const;
    virtual void readXml( const QDomElement& itemElem, const QDomDocument& doc );

  protected:

    void renderAnnotation( QgsRenderContext& context, QSizeF size ) const;

};
