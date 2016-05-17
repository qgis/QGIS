class QgsComposerPolygon: QgsComposerNodesItem
{
%TypeHeaderCode
#include <qgscomposerpolygon.h>
%End
  public:

    QgsComposerPolygon( QgsComposition* c );
    QgsComposerPolygon( QPolygonF polygon, QgsComposition* c );
    ~QgsComposerPolygon();

    /** Overridden to return shape name */
    virtual QString displayName() const;

    /** Returns the QgsSymbolV2 used to draw the shape. */
    QgsFillSymbolV2* polygonStyleSymbol();

    /** Set the QgsSymbolV2 used to draw the shape. */
    void setPolygonStyleSymbol( QgsFillSymbolV2* symbol );

    /** Return correct graphics item type. */
    virtual int type() const;

  protected:

    bool _addNode( const int indexPoint, const QPointF &newPoint, const double radius );

    bool _removeNode( const int indexPoint );

    /** Draw points for the current shape. */
    void _draw( QPainter *painter );

    /** Read symbol in XML. */
    void _readXMLStyle( const QDomElement &elmt  );

    /** Write the symbol in an XML document. */
    void _writeXMLStyle( QDomDocument &doc, QDomElement &elmt ) const;

    /** Create a default symbol. */
    void createDefaultPolygonStyleSymbol();
};
