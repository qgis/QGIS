class QgsTextAnnotationItem: QgsAnnotationItem
{
%TypeHeaderCode
  #include <qgstextannotationitem.h>
%End
  public:
    QgsTextAnnotationItem( QgsMapCanvas* canvas /TransferThis/ );
    ~QgsTextAnnotationItem();

    /** Returns document (caller takes ownership)*/
    QTextDocument* document() const /Factory/;
    /** Sets document (does not take ownership)*/
    void setDocument( const QTextDocument* doc );

    void writeXml( QDomDocument& doc ) const;
    void readXml( const QDomDocument& doc, const QDomElement& itemElem );

    void paint( QPainter* painter );
};
