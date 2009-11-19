/** \ingroup MapComposer
 * A scale bar item that can be added to a map composition.
 */

class QgsComposerScaleBar: QgsComposerItem
{
%TypeHeaderCode
#include "qgscomposerscalebar.h"
%End
  public:

    QgsComposerScaleBar( QgsComposition* composition /TransferThis/);
    ~QgsComposerScaleBar();

    /** \brief Reimplementation of QCanvasItem::paint*/
    void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget );

    //getters and setters
    int numSegments() const;
    void setNumSegments( int nSegments );

    int numSegmentsLeft() const;
    void setNumSegmentsLeft( int nSegmentsLeft );

    double numUnitsPerSegment() const;
    void setNumUnitsPerSegment( double units );

    double numMapUnitsPerScaleBarUnit() const;
    void setNumMapUnitsPerScaleBarUnit( double d );

    QString unitLabeling() const;
    void setUnitLabeling( const QString& label );

    QFont font() const;

    void setFont( const QFont& font );

    QPen pen() const;
    void setPen( const QPen& pen );

    QBrush brush() const;
    void setBrush( const QBrush& brush );

    double height() const;
    void setHeight( double h );

    void setComposerMap( const QgsComposerMap* map );
    const QgsComposerMap* composerMap();

    double labelBarSpace() const;
    void setLabelBarSpace( double space );

    double boxContentSpace() const;
    void setBoxContentSpace( double space );

    double segmentMillimeters() const;

    /**Apply default settings (scale bar 1/5 of map item width)*/
    void applyDefaultSettings();

    /**Sets style by name
     @param styleName (untranslated) style name. Possibilities are: 'Single Box', 'Double Box', 'Line Ticks Middle', 'Line Ticks Down', 'Line Ticks Up', 'Numeric'*/
    void setStyle( const QString& styleName );

    /**Returns style name*/
    QString style() const;

    /**Returns the x - positions of the segment borders (in item coordinates) and the width
     of the segment*/
    void segmentPositions( QList<QPair<double, double> >& posWidthList ) const;

    /**Sets box size suitable to content*/
    void adjustBoxSize();

    /**Adjusts box size and calls QgsComposerItem::update()*/
    void update();

    /**Returns string of first label (important for drawing, labeling, size calculation*/
    QString firstLabelString() const;

    /** stores state in Dom node
       * @param elem is Dom element corresponding to 'Composer' tag
       * @param temp write template file
       */
    bool writeXML( QDomElement& elem, QDomDocument & doc ) const;

    /** sets state from Dom document
       * @param itemElem is Dom node corresponding to item tag
       */
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc );


  public slots:
    void updateSegmentSize();
    /**Sets mCompositionMap to 0 if the map is deleted*/
    void invalidateCurrentMap();

  protected:
    /**Calculates with of a segment in mm and stores it in mSegmentMillimeters*/
    void refreshSegmentMillimeters();
};
