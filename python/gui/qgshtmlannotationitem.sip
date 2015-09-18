class QgsHtmlAnnotationItem: QObject, QgsAnnotationItem
{
%TypeHeaderCode
#include <qgshtmlannotationitem.h>
%End
  public:
    QgsHtmlAnnotationItem( QgsMapCanvas* canvas, QgsVectorLayer* vlayer = 0, bool hasFeature = false, int feature = 0 );
    ~QgsHtmlAnnotationItem();

    void paint( QPainter * painter );

    //! paint function called by map canvas
    void paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0 );

    QSizeF minimumFrameSize() const;

    /** Reimplemented from QgsAnnotationItem*/
    void setMapPosition( const QgsPoint& pos );

    void setHTMLPage( const QString& htmlFile );
    QString htmlPage() const;

    void writeXML( QDomDocument& doc ) const;
    void readXML( const QDomDocument& doc, const QDomElement& itemElem );

    QgsVectorLayer* vectorLayer() const;
};
