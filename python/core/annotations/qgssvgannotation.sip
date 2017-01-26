class QgsSvgAnnotation : QgsAnnotation
{
%TypeHeaderCode
#include <qgssvgannotation.h>
%End
  public:

    QgsSvgAnnotation( QObject* parent /TransferThis/ = nullptr );

    virtual void writeXml( QDomElement& elem, QDomDocument & doc ) const;
    virtual void readXml( const QDomElement& itemElem, const QDomDocument& doc );

    void setFilePath( const QString& file );
    QString filePath() const;
    
  protected:

    void renderAnnotation( QgsRenderContext& context, QSizeF size ) const;

};
