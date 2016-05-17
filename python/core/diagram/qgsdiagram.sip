class QgsDiagram
{
%TypeHeaderCode
#include <qgsdiagram.h>
%End
  public:
    virtual ~QgsDiagram();
    /** Returns an instance that is equivalent to this one
     * @note added in 2.4 */
    virtual QgsDiagram* clone() const = 0 /Factory/;

    void clearCache();

    //! @deprecated use QgsExpressionContext variant instead
    QgsExpression* getExpression( const QString& expression, const QgsFields* fields ) /Deprecated/;

    /** Returns a prepared expression for the specified context.
     * @param expression expression string
     * @param context expression context
     * @note added in QGIS 2.12
     */
    QgsExpression* getExpression( const QString& expression, const QgsExpressionContext& context );

    /** @deprecated `void renderDiagram( const QgsFeature& feature, QgsRenderContext& c, const QgsDiagramSettings& s, const QPointF& position )` should be used instead */
    virtual void renderDiagram( const QgsAttributes& att, QgsRenderContext& c, const QgsDiagramSettings& s, QPointF position ) /Deprecated/;
    /** Draws the diagram at the given position (in pixel coordinates)*/
    virtual void renderDiagram( const QgsFeature& feature, QgsRenderContext& c, const QgsDiagramSettings& s, QPointF position ) = 0;
    virtual QString diagramName() const = 0;
    /** Returns the size in map units the diagram will use to render.*/
    virtual QSizeF diagramSize( const QgsAttributes& attributes, const QgsRenderContext& c, const QgsDiagramSettings& s ) = 0;
    /** @deprecated `QSizeF diagramSize( const QgsFeature& feature, const QgsRenderContext& c, const QgsDiagramSettings& s, const QgsDiagramInterpolationSettings& is )` should be used instead */
    virtual QSizeF diagramSize( const QgsAttributes& attributes, const QgsRenderContext& c, const QgsDiagramSettings& s, const QgsDiagramInterpolationSettings& is ) /Deprecated/;
    /** Returns the size in map units the diagram will use to render. Interpolate size*/
    virtual QSizeF diagramSize( const QgsFeature& feature, const QgsRenderContext& c, const QgsDiagramSettings& s, const QgsDiagramInterpolationSettings& is ) = 0;

    /** Returns the size of the legend item for the diagram corresponding to a specified value.
     * @param value value to return legend item size for
     * @param s diagram settings
     * @param is interpolation settings
     * @note added in QGIS 2.16
     */
    virtual double legendSize( double value, const QgsDiagramSettings& s, const QgsDiagramInterpolationSettings& is ) const = 0;

  protected:
    QgsDiagram();
    QgsDiagram( const QgsDiagram& other );

    /** Changes the pen width to match the current settings and rendering context
     *  @param pen The pen to modify
     *  @param s   The settings that specify the pen width
     *  @param c   The rendering specifying the proper scale units for pixel conversion
     */
    void setPenWidth( QPen& pen, const QgsDiagramSettings& s, const QgsRenderContext& c );

    /** Calculates a size to match the current settings and rendering context
     *  @param size The size to convert
     *  @param s    The settings that specify the size type
     *  @param c    The rendering specifying the proper scale units for pixel conversion
     *
     *  @return The converted size for rendering
     */
    QSizeF sizePainterUnits( QSizeF size, const QgsDiagramSettings& s, const QgsRenderContext& c );

    /** Calculates a length to match the current settings and rendering context
     *  @param l    The length to convert
     *  @param s    Unused
     *  @param c    The rendering specifying the proper scale units for pixel conversion
     *
     *  @return The converted length for rendering
     */
    float sizePainterUnits( float l, const QgsDiagramSettings& s, const QgsRenderContext& c );

    /** Calculates a size to match the current settings and rendering context
     *  @param s    The settings that contain the font size and size type
     *  @param c    The rendering specifying the proper scale units for pixel conversion
     *
     *  @return The properly scaled font for rendering
     */
    QFont scaledFont( const QgsDiagramSettings& s, const QgsRenderContext& c );

    /** Returns the scaled size of a diagram for a value, respecting the specified diagram interpolation settings.
     * @param value value to calculate corresponding circular size for
     * @param s diagram settings
     * @param is interpolation settings
     * @note added in QGIS 2.16
     */
    QSizeF sizeForValue( double value, const QgsDiagramSettings& s, const QgsDiagramInterpolationSettings& is ) const;
};
