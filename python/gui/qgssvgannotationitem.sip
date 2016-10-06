class QgsSvgAnnotationItem : QgsAnnotationItem
{
%TypeHeaderCode
#include <qgssvgannotationitem.h>
%End
  public:

    QgsSvgAnnotationItem( QgsMapCanvas* canvas );
    ~QgsSvgAnnotationItem();

    void writeXml( QDomDocument& doc ) const;
    void readXml( const QDomDocument& doc, const QDomElement& itemElem );

    void paint( QPainter* painter );

    void setFilePath( const QString& file );
    QString filePath() const;
};
