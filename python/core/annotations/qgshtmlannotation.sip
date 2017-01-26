class QgsHtmlAnnotation : QgsAnnotation
{
%TypeHeaderCode
#include <qgshtmlannotation.h>
%End
  public:

    QgsHtmlAnnotation( QObject* parent /TransferThis/ = nullptr, QgsVectorLayer* vlayer = nullptr, bool hasFeature = false, int feature = 0 );

    ~QgsHtmlAnnotation();

    QSizeF minimumFrameSize() const;

    void setSourceFile( const QString& htmlFile );
    QString sourceFile() const;

    virtual void writeXml( QDomElement& elem, QDomDocument & doc ) const;
    virtual void readXml( const QDomElement& itemElem, const QDomDocument& doc );

    QgsVectorLayer* vectorLayer() const;

  protected:

    void renderAnnotation( QgsRenderContext& context, QSizeF size ) const;

};
