class QgsSvgAnnotationItem : QgsAnnotationItem
{
%TypeHeaderCode
#include <qgssvgannotationitem.h>
%End
  public:

    QgsSvgAnnotationItem( QgsMapCanvas* canvas );
    ~QgsSvgAnnotationItem();

    void writeXML( QDomDocument& doc ) const;
    void readXML( const QDomDocument& doc, const QDomElement& itemElem );

    void paint( QPainter* painter );

    void setFilePath( const QString& file );
    QString filePath() const;
};
