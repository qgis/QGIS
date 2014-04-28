/**A class to display feature attributes in the print composer*/
class QgsComposerTable: QgsComposerItem
{
%TypeHeaderCode
#include <qgscomposertable.h>
%End

  public:
    QgsComposerTable( QgsComposition* composition /TransferThis/ );
    virtual ~QgsComposerTable();

    /** return correct graphics item type. Added in v1.7 */
    virtual int type() const;

    /** \brief Reimplementation of QCanvasItem::paint*/
    virtual void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget );

    virtual bool writeXML( QDomElement& elem, QDomDocument & doc ) const = 0;
    virtual bool readXML( const QDomElement& itemElem, const QDomDocument& doc ) = 0;

    void setLineTextDistance( double d );
    double lineTextDistance() const;

    void setHeaderFont( const QFont& f );
    QFont headerFont() const;

    void setContentFont( const QFont& f );
    QFont contentFont() const;

    void setShowGrid( bool show );
    bool showGrid() const;

    void setGridStrokeWidth( double w );
    double gridStrokeWidth() const;

    void setGridColor( const QColor& c );
    QColor gridColor() const;
    
  public slots:

    /**Refreshes the attributes shown in the table by querying the vector layer for new data.
     * This also causes the column widths and size of the table to change to accomodate the
     * new data.
     * @note added in 2.3
     * @see adjustFrameToSize
    */
    virtual void refreshAttributes();

    /**Adapts the size of the frame to match the content. First, the optimal width of the columns
     * is recalculated by checking the maximum width of attributes shown in the table. Then, the
     * table is resized to fit its contents. This slot utilises the table's attribute cache so
     * that a re-query of the vector layer is not required.
     * @note added in 2.3
     * @see refreshAttributes
    */
    virtual void adjustFrameToSize();   

  protected:
    /**Retrieves feature attributes*/
    // virtual bool getFeatureAttributes( QList<QgsAttributeMap>& attributeMaps );

    /**Calculate the maximum width values of the vector attributes*/
    // virtual bool calculateMaxColumnWidths( QMap<int, double>& maxWidthMap, const QList< QMap<int, QVariant> >& attributeMaps ) const;
    /**Adapts the size of the item frame to match the content*/
    // void adaptItemFrame( const QMap<int, double>& maxWidthMap, const QList< QMap<int, QVariant> >& attributeMaps );
    void drawHorizontalGridLines( QPainter* p, int nAttributes );
    // void drawVerticalGridLines( QPainter* p, const QMap<int, double>& maxWidthMap );

    bool tableWriteXML( QDomElement& itemElem, QDomDocument& doc ) const;
    bool tableReadXML( const QDomElement& itemElem, const QDomDocument& doc );
};
