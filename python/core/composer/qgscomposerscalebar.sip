/** \ingroup MapComposer
 * A scale bar item that can be added to a map composition.
 */

class QgsComposerScaleBar: QgsComposerItem
{
%TypeHeaderCode
#include "qgscomposerscalebar.h"
%End

  public:

    /**Added in version 1.8*/
    enum Alignment
    {
      Left = 0,
      Middle,
      Right
    };

    /**Added in version 1.9*/
    enum ScaleBarUnits
    {
      MapUnits = 0,
      Meters,
      Feet
    };

    QgsComposerScaleBar( QgsComposition* composition /TransferThis/ );
    ~QgsComposerScaleBar();

    /** return correct graphics item type. Added in v1.7 */
    virtual int type() const;

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

    QColor fontColor() const;
    void setFontColor( const QColor& c );

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

    /**Left / Middle/ Right
      @note: this method was added in version 1.8*/
    Alignment alignment() const;

    /**@note: this method was added in version 1.8*/
    void setAlignment( Alignment a );

    /**@note: this method was added in version 1.9*/
    ScaleBarUnits units() const;

    /**@note: this method was added in version 1.9*/
    void setUnits( ScaleBarUnits u );

    /**Apply default settings*/
    void applyDefaultSettings();
    /**Apply default size (scale bar 1/5 of map item width)
      @note this method was added in version 1.7*/
    void applyDefaultSize();

    /**Sets style by name
     @param styleName (untranslated) style name. Possibilities are: 'Single Box', 'Double Box', 'Line Ticks Middle', 'Line Ticks Down', 'Line Ticks Up', 'Numeric'*/
    void setStyle( const QString& styleName );

    /**Returns style name*/
    QString style() const;

    /**Returns the x - positions of the segment borders (in item coordinates) and the width
     of the segment*/
%If (!ANDROID)
    void segmentPositions( QList<QPair<double, double> >& posWidthList ) const;
%End

    /**Sets box size suitable to content*/
    void adjustBoxSize();

    /**Adjusts box size and calls QgsComposerItem::update()*/
    void update();

    /**Returns string of first label (important for drawing, labeling, size calculation*/
    QString firstLabelString() const;

    /** stores state in Dom element
       * @param elem is Dom element corresponding to 'Composer' tag
       * @param doc Dom document
       */
    bool writeXML( QDomElement& elem, QDomDocument & doc ) const;

    /** sets state from Dom document
       * @param itemElem is Dom node corresponding to item tag
       * @param doc is Dom document
       */
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc );

    /**Moves scalebar position to the left / right depending on alignment and change in item width*/
    void correctXPositionAlignment( double width, double widthAfter );

  public slots:
    void updateSegmentSize();
    /**Sets mCompositionMap to 0 if the map is deleted*/
    void invalidateCurrentMap();

  protected:
    /**Calculates with of a segment in mm and stores it in mSegmentMillimeters*/
    void refreshSegmentMillimeters();

    /**Returns diagonal of composer map in selected units (map units / meters / feet)*/
    double mapWidth() const;
};
