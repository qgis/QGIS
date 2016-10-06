/** \class QgsPointClusterRenderer
 * \ingroup core
 * A renderer that automatically clusters points with the same geographic position.
 * \note added in QGIS 3.0
*/
class QgsPointClusterRenderer : QgsPointDistanceRenderer
{
%TypeHeaderCode
#include <qgspointclusterrenderer.h>
%End
  public:

    QgsPointClusterRenderer();

    virtual QgsPointClusterRenderer* clone() const /Factory/;
    virtual void startRender( QgsRenderContext& context, const QgsFields& fields );
    void stopRender( QgsRenderContext& context );
    QDomElement save( QDomDocument& doc );
    virtual QSet<QString> usedAttributes() const;

    //! Create a renderer from XML element
    static QgsFeatureRenderer* create( QDomElement& symbologyElem ) /Factory/;

    /** Returns the symbol used for rendering clustered groups (but not ownership of the symbol).
     * @see setClusterSymbol()
    */
    QgsMarkerSymbol* clusterSymbol();

    /** Sets the symbol for rendering clustered groups.
     * @param symbol new cluster symbol. Ownership is transferred to the renderer.
     * @see clusterSymbol()
    */
    void setClusterSymbol( QgsMarkerSymbol* symbol /Transfer/ );

    /** Creates a QgsPointDisplacementRenderer from an existing renderer.
     * @note added in 2.5
     * @returns a new renderer if the conversion was possible, otherwise nullptr.
     */
    static QgsPointClusterRenderer* convertFromRenderer( const QgsFeatureRenderer *renderer ) /Factory/;

  private:
    QgsPointClusterRenderer( const QgsPointClusterRenderer & );
    QgsPointClusterRenderer & operator=( const QgsPointClusterRenderer & );

    void drawGroup( QPointF centerPoint, QgsRenderContext& context, const QgsPointDistanceRenderer::ClusteredGroup& group );
};
