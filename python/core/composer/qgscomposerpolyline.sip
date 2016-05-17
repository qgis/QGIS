class QgsComposerPolyline: QgsComposerNodesItem
{
%TypeHeaderCode
#include <qgscomposerpolyline.h>
%End
  public:

    QgsComposerPolyline( QgsComposition* c );
    QgsComposerPolyline( QPolygonF polyline, QgsComposition* c );
    ~QgsComposerPolyline();

    /** Overridden to return shape name */
    virtual QString displayName() const;

    /** Returns the QgsSymbolV2 used to draw the shape. */
    QgsLineSymbolV2* polylineStyleSymbol();

    /** Set the QgsSymbolV2 used to draw the shape. */
    void setPolylineStyleSymbol( QgsLineSymbolV2* symbol );

    /** Overridden to return shape type */
    virtual int type() const;

  protected:

    bool _addNode( const int indexPoint, const QPointF &newPoint, const double radius );

    bool _removeNode( const int indexPoint );

    /** Draw points for the current shape. */
    void _draw( QPainter *painter );

    /** Read symbol in XML. */
    void _readXMLStyle( const QDomElement &elmt );

    /** Write the symbol in an XML document. */
    void _writeXMLStyle( QDomDocument &doc, QDomElement &elmt ) const;

    /** Create a default symbol. */
    void createDefaultPolylineStyleSymbol();
};

