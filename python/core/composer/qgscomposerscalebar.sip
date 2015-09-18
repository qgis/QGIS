/** \ingroup MapComposer
 * A scale bar item that can be added to a map composition.
 */

class QgsComposerScaleBar: QgsComposerItem
{
%TypeHeaderCode
#include "qgscomposerscalebar.h"
%End

  public:

    enum Alignment
    {
      Left,
      Middle,
      Right
    };

    enum ScaleBarUnits
    {
      MapUnits,
      Meters,
      Feet,
      NauticalMiles
    };

    /** Modes for setting size for scale bar segments
    */
    enum SegmentSizeMode
    {
      SegmentSizeFixed, /*!< Scale bar segment size is fixed to a map unit*/
      SegmentSizeFitWidth /*!< Scale bar segment size is calculated to fit a size range*/
    };

    QgsComposerScaleBar( QgsComposition* composition /TransferThis/ );
    ~QgsComposerScaleBar();

    /** Return correct graphics item type. */
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

    /** Returns the size mode for scale bar segments.
     * @see setSegmentSizeMode
     * @see minBarWidth
     * @see maxBarWidth
     * @note added in QGIS 2.9
     */
    SegmentSizeMode segmentSizeMode() const;

    /** Sets the size mode for scale bar segments.
     * @param mode size mode
     * @see segmentSizeMode
     * @see setMinBarWidth
     * @see setMaxBarWidth
     * @note added in QGIS 2.9
     */
    void setSegmentSizeMode( SegmentSizeMode mode );

    /** Returns the minimum size (in millimeters) for scale bar segments. This
     * property is only effective if the @link segmentSizeMode @endlink is set
     * to @link SegmentSizeFitWidth @endlink.
     * @see segmentSizeMode
     * @see setMinBarWidth
     * @see maxBarWidth
     * @note added in QGIS 2.9
     */
    double minBarWidth() const;

    /** Sets the minimum size (in millimeters) for scale bar segments. This
     * property is only effective if the @link segmentSizeMode @endlink is set
     * to @link SegmentSizeFitWidth @endlink.
     * @param minWidth minimum width in millimeters
     * @see minBarWidth
     * @see setMaxBarWidth
     * @see setSegmentSizeMode
     * @note added in QGIS 2.9
     */
    void setMinBarWidth( double minWidth );

    /** Returns the maximum size (in millimeters) for scale bar segments. This
     * property is only effective if the @link segmentSizeMode @endlink is set
     * to @link SegmentSizeFitWidth @endlink.
     * @see segmentSizeMode
     * @see setMaxBarWidth
     * @see minBarWidth
     * @note added in QGIS 2.9
     */
    double maxBarWidth() const;

    /** Sets the maximum size (in millimeters) for scale bar segments. This
     * property is only effective if the @link segmentSizeMode @endlink is set
     * to @link SegmentSizeFitWidth @endlink.
     * @param minWidth maximum width in millimeters
     * @see minBarWidth
     * @see setMaxBarWidth
     * @see setSegmentSizeMode
     * @note added in QGIS 2.9
     */
    void setMaxBarWidth( double maxWidth );

    double numMapUnitsPerScaleBarUnit() const;
    void setNumMapUnitsPerScaleBarUnit( double d );

    QString unitLabeling() const;
    void setUnitLabeling( const QString& label );

    QFont font() const;
    void setFont( const QFont& font );

    /** Returns the color used for drawing text in the scalebar.
     * @returns font color for scalebar.
     * @see setFontColor
     * @see font
    */
    QColor fontColor() const;

    /** Sets the color used for drawing text in the scalebar.
     * @param c font color for scalebar.
     * @see fontColor
     * @see setFont
    */
    void setFontColor( const QColor& c );

    /** Returns the pen used for drawing the scalebar.
     * @returns QPen used for drawing the scalebar outlines.
     * @see setPen
     * @see brush
    */
    QPen pen() const;

    /** Sets the pen used for drawing the scalebar.
     * @param pen QPen to use for drawing the scalebar outlines.
     * @see pen
     * @see setBrush
    */
    void setPen( const QPen& pen );

    /** Returns the primary brush for the scalebar.
     * @returns QBrush used for filling the scalebar
     * @see setBrush
     * @see brush2
     * @see pen
    */
    QBrush brush() const;

    /** Sets primary brush for the scalebar.
     * @param brush QBrush to use for filling the scalebar
     * @see brush
     * @see setBrush2
     * @see setPen
    */
    void setBrush( const QBrush& brush );

    /** Returns the secondary brush for the scalebar. This is used for alternating color style scalebars, such
     * as single and double box styles.
     * @returns QBrush used for secondary color areas
     * @see setBrush2
     * @see brush
    */
    QBrush brush2() const;

    /** Sets secondary brush for the scalebar. This is used for alternating color style scalebars, such
     * as single and double box styles.
     * @param brush QBrush to use for secondary color areas
     * @see brush2
     * @see setBrush
    */
    void setBrush2( const QBrush& brush );

    double height() const;
    void setHeight( double h );

    void setComposerMap( const QgsComposerMap* map );
    const QgsComposerMap* composerMap();

    double labelBarSpace() const;
    void setLabelBarSpace( double space );

    double boxContentSpace() const;
    void setBoxContentSpace( double space );

    double segmentMillimeters() const;

    /** Left / Middle/ Right */
    Alignment alignment() const;

    void setAlignment( Alignment a );

    ScaleBarUnits units() const;

    void setUnits( ScaleBarUnits u );

    /** Returns the join style used for drawing lines in the scalebar
     * @returns Join style for lines
     * @note introduced in 2.3
     * @see setLineJoinStyle
     */
    Qt::PenJoinStyle lineJoinStyle() const;
    /** Sets join style used when drawing the lines in the scalebar
     * @param style Join style for lines
     * @returns nothing
     * @note introduced in 2.3
     * @see lineJoinStyle
     */
    void setLineJoinStyle( Qt::PenJoinStyle style );

    /** Returns the cap style used for drawing lines in the scalebar
     * @returns Cap style for lines
     * @note introduced in 2.3
     * @see setLineCapStyle
     */
    Qt::PenCapStyle lineCapStyle() const;
    /** Sets cap style used when drawing the lines in the scalebar
     * @param style Cap style for lines
     * @returns nothing
     * @note introduced in 2.3
     * @see lineCapStyle
     */
    void setLineCapStyle( Qt::PenCapStyle style );

    /** Apply default settings*/
    void applyDefaultSettings();
    /** Apply default size (scale bar 1/5 of map item width) */
    void applyDefaultSize( ScaleBarUnits u = Meters );

    /** Sets style by name
     @param styleName (untranslated) style name. Possibilities are: 'Single Box', 'Double Box', 'Line Ticks Middle', 'Line Ticks Down', 'Line Ticks Up', 'Numeric'*/
    void setStyle( const QString& styleName );

    /** Returns style name*/
    QString style() const;

    /** Returns the x - positions of the segment borders (in item coordinates) and the width
     of the segment
     @note python bindings not available on android
     */
%If (!ARM)
    void segmentPositions( QList<QPair<double, double> >& posWidthList ) const;
%End

    /** Sets box size suitable to content*/
    void adjustBoxSize();

    /** Adjusts box size and calls QgsComposerItem::update()*/
    void update();

    /** Returns string of first label (important for drawing, labeling, size calculation*/
    QString firstLabelString() const;

    /** Stores state in Dom element
       * @param elem is Dom element corresponding to 'Composer' tag
       * @param doc Dom document
       */
    bool writeXML( QDomElement& elem, QDomDocument & doc ) const;

    /** Sets state from Dom document
       * @param itemElem is Dom node corresponding to item tag
       * @param doc is Dom document
       */
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc );

    /** Moves scalebar position to the left / right depending on alignment and change in item width*/
    void correctXPositionAlignment( double width, double widthAfter );

    //overriden to apply minimum size
    void setSceneRect( const QRectF &rectangle );

  public slots:
    void updateSegmentSize();
    /** Sets mCompositionMap to 0 if the map is deleted*/
    void invalidateCurrentMap();

  protected:
    /** Calculates with of a segment in mm and stores it in mSegmentMillimeters*/
    void refreshSegmentMillimeters();

    /** Returns diagonal of composer map in selected units (map units / meters / feet / nautical miles)*/
    double mapWidth() const;

};
