class QgsFormAnnotationItem: QObject, QgsAnnotationItem
{
%TypeHeaderCode
  #include <qgsformannotationitem.h>
%End

  public:
    QgsFormAnnotationItem( QgsMapCanvas* canvas /TransferThis/, QgsVectorLayer* vlayer = 0, bool hasFeature = false, int feature = 0 );
    ~QgsFormAnnotationItem();

    void paint( QPainter * painter );

    //! paint function called by map canvas
    void paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0 );

    QSizeF minimumFrameSize() const;
    /** Returns the optimal frame size*/
    QSizeF preferredFrameSize() const;

    /** Reimplemented from QgsAnnotationItem*/
    void setMapPosition( const QgsPoint& pos );

    void setDesignerForm( const QString& uiFile );
    QString designerForm() const;

    void writeXML( QDomDocument& doc ) const;
    void readXML( const QDomDocument& doc, const QDomElement& itemElem );

    QgsVectorLayer* vectorLayer() const;
};
