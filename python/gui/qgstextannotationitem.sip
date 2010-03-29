class QgsTextAnnotationItem: QgsAnnotationItem
{
%TypeHeaderCode
  #include <qgstextannotationitem.h>
%End
  public:
    QgsTextAnnotationItem( QgsMapCanvas* canvas );
    ~QgsTextAnnotationItem();

    /**Returns document (caller takes ownership)*/
    QTextDocument* document() const;
    /**Sets document (does not take ownership)*/
    void setDocument( const QTextDocument* doc );

    void writeXML( QDomDocument& doc ) const;
    void readXML( const QDomDocument& doc, const QDomElement& itemElem );
    
    void paint( QPainter* painter );
};
