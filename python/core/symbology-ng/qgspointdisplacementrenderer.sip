/** \class QgsPointDisplacementRenderer
 * \ingroup core
 * A renderer that automatically displaces points with the same geographic location.
*/
class QgsPointDisplacementRenderer : QgsPointDistanceRenderer
{
%TypeHeaderCode
#include <qgspointdisplacementrenderer.h>
%End
  public:

    /** Placement methods for dispersing points
     */
    enum Placement
    {
      Ring, /*!< Place points in a single ring around group*/
      ConcentricRings /*!< Place points in concentric rings around group*/
    };

    /** Constructor for QgsPointDisplacementRenderer.
     * @param labelAttributeName optional attribute name for labeling points
     */
    QgsPointDisplacementRenderer( const QString& labelAttributeName = QString() );

    virtual QgsPointDisplacementRenderer* clone() const /Factory/;
    virtual void startRender( QgsRenderContext& context, const QgsFields& fields );
    void stopRender( QgsRenderContext& context );
    QDomElement save( QDomDocument& doc );
    virtual QSet<QString> usedAttributes() const;

    //! Create a renderer from XML element
    static QgsFeatureRenderer* create( QDomElement& symbologyElem ) /Factory/;

    /** Sets the line width for the displacement group circle.
     * @param width line width in mm
     * @see circleWidth()
     * @see setCircleColor()
     */
    void setCircleWidth( double width );

    /** Returns the line width for the displacement group circle in mm.
     * @see setCircleWidth()
     * @see circleColor()
     */
    double circleWidth() const;

    /** Sets the color used for drawing the displacement group circle.
     * @param color circle color
     * @see circleColor()
     * @see setCircleWidth()
     */
    void setCircleColor( const QColor& color );

    /** Returns the color used for drawing the displacement group circle.
     * @see setCircleColor()
     * @see circleWidth()
     */
    QColor circleColor() const;

    /** Sets a factor for increasing the ring size of displacement groups.
     * @param distance addition factor
     * @see circleRadiusAddition()
     */
    void setCircleRadiusAddition( double distance );

    /** Returns the factor for increasing the ring size of displacement groups.
     * @see setCircleRadiusAddition()
     */
    double circleRadiusAddition() const;

    /** Returns the placement method used for dispersing the points.
     * @see setPlacement()
     * @note added in QGIS 2.12
     */
    Placement placement() const;

    /** Sets the placement method used for dispersing the points.
     * @param placement placement method
     * @see placement()
     * @note added in QGIS 2.12
     */
    void setPlacement( Placement placement );

    /** Returns the symbol for the center of a displacement group (but not ownership of the symbol).
     * @see setCenterSymbol()
    */
    QgsMarkerSymbol* centerSymbol();

    /** Sets the center symbol for a displacement group.
     * @param symbol new center symbol. Ownership is transferred to the renderer.
     * @see centerSymbol()
    */
    void setCenterSymbol( QgsMarkerSymbol* symbol /Transfer/ );

    /** Creates a QgsPointDisplacementRenderer from an existing renderer.
     * @note added in 2.5
     * @returns a new renderer if the conversion was possible, otherwise nullptr.
     */
    static QgsPointDisplacementRenderer* convertFromRenderer( const QgsFeatureRenderer *renderer ) /Factory/;

  private:
    QgsPointDisplacementRenderer( const QgsPointDisplacementRenderer & );
    QgsPointDisplacementRenderer & operator=( const QgsPointDisplacementRenderer & );

    void drawGroup( QPointF centerPoint, QgsRenderContext& context, const QgsPointDistanceRenderer::ClusteredGroup& group );

};
