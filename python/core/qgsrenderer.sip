
/**Abstract base class for renderers. A renderer holds all the information necessary to draw the contents of a vector layer to a map canvas. The vector layer then passes each feature to paint to the renderer*/
class QgsRenderer
{
%TypeHeaderCode
#include <qgsrenderer.h>
%End
  
 public:
    /** Default ctor sets up selection colour from project properties */
    QgsRenderer();
    /** Virtual destructor because we have virtual methods... */
    virtual ~QgsRenderer();
    /**A vector layer passes features to a renderer object to change the brush and pen of the qpainter
     @param p the painter storing brush and pen
     @param f a pointer to the feature to be rendered
     @param pic pointer to a marker from SVG (is only used by marker renderers)
     @param selected feature is to be shown selected
     @param widthScale scale factor
     @param rasterScaleFactor scale factor for rasters
     
     deprecated */
    void renderFeature(QPainter* p, QgsFeature& f,QImage* pic, bool selected, double widthScale = 1.0, double rasterScaleFactor = 1.0);
     
    /**A vector layer passes features to a renderer object to change the brush ans pen of the qpainter
      @param renderContext context of the rendering operation
      @param f the feature to render
      @param pic pointer to marker to render (is only used by marker renderers)
      @param selected the feature is to be shown selected
      
      added in 1.2 */
    void renderFeature( QgsRenderContext &renderContext, QgsFeature& f, QImage* pic, bool selected );

    virtual void renderFeature( QgsRenderContext &renderContext, QgsFeature& f, QImage* pic, bool selected, double opacity ) = 0;

    /**Reads the renderer configuration from an XML file
     @param rnode the Dom node to read 
     @param vl the vector layer which will be associated with the renderer*/
    virtual int readXML(const QDomNode& rnode, QgsVectorLayer& vl)=0;
    /**Writes the contents of the renderer to a configuration file
     @ return true in case of success*/
    virtual bool writeXML( QDomNode & layer_node, QDomDocument & document,  const QgsVectorLayer& vl  ) const=0;
    /** Returns true, if attribute values are used by the renderer and false otherwise*/
    virtual bool needsAttributes() const=0;
    /**Returns a list with indexes of classification attributes*/
    virtual QList<int> classificationAttributes() const=0;
    /**Returns the renderers name*/
    virtual QString name() const=0;    
    /**Return symbology items*/
    virtual const QList<QgsSymbol*> symbols() const=0;
    /**Returns a copy of the renderer (a deep copy on the heap)*/
    virtual QgsRenderer* clone() const=0;
    /** Change selection color */
    static void setSelectionColor(QColor color);
    /**Returns true if this renderer returns a pixmap in the render method (e.g. for point data or diagrams)*/
    virtual bool containsPixmap() const;
    /**Returns true if this renderer uses its own transparency settings, e.g. differentiated by classification.
     This is a hint for QgsVectorLayer to not use the transparency setting on layer level in this cases*/
    virtual bool usesTransparency() const;
   
};

