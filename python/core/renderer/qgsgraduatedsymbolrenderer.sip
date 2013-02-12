/**This class contains the information for graduate symbol rendering*/
class QgsGraduatedSymbolRenderer : QgsRenderer
{
%TypeHeaderCode
#include <qgsgraduatedsymbolrenderer.h>
%End
 public:
    enum Mode
    {
      EqualInterval,
      Quantile,
      Empty
    };
    QgsGraduatedSymbolRenderer( QGis::GeometryType type );
    QgsGraduatedSymbolRenderer( const QgsGraduatedSymbolRenderer& other );
    virtual ~QgsGraduatedSymbolRenderer();
    /** Get the mode - which is only really used to be able to reinstate
     * the graduated dialog properties properly, so we
     * don't do anything else besides accessors and mutators in
     * this class.
     */
    Mode mode() const;

    /** Set the mode - which is only really used to be able to reinstate
     * the graduated dialog properties properly, so we
     * don't do anything else besides accessors and mutators in
     * this class.
     */
    void setMode( Mode theMode );

    /**Adds a new item
    \param sy a pointer to the QgsSymbol to be inserted. It has to be created using the new operator and is automatically destroyed when 'removeItems' is called or when this object is destroyed*/
    void addSymbol( QgsSymbol* sy /Transfer/ );

    /**Returns the index of the classification field*/
    int classificationField() const;

    /**Removes all symbols*/
    void removeSymbols();

    /** Determines if a feature will be rendered or not
    @param f a pointer to the feature to determine if rendering will happen*/
    bool willRenderFeature( QgsFeature *f );

    /**Renders a feature
     \param renderContext the render context
     \param f a pointer to a feature to render
     \param img image to render in
     \param selected feature is selected
     \param opacity opacity of feature
     \note added in 1.2 */
    void renderFeature( QgsRenderContext &renderContext, QgsFeature& f, QImage* img, bool selected, double opacity = 1.0 );

    /**Sets the classicifation field by index
    \param field the number of the field to classify*/
    void setClassificationField( int field );

    /**Reads the renderer configuration from an XML file
     @param rnode the Dom node to read
     @param vl the vector layer which will be associated with the renderer
     @return 0 in case of success, 1 if vector layer has no renderer, 2 if classification field not found
    */
    virtual int readXML( const QDomNode& rnode, QgsVectorLayer& vl );

    /**Writes the contents of the renderer to a configuration file
     @ return true in case of success*/
    virtual bool writeXML( QDomNode & layer_node, QDomDocument & document, const QgsVectorLayer& vl ) const;

    /** Returns true*/
    bool needsAttributes() const;

    /**Returns a list of all needed attributes*/
    QList<int> classificationAttributes() const;

    void updateSymbolAttributes();

    /**Returns the renderers name*/
    QString name() const;

    /**Returns the symbols of the items*/
    const QList<QgsSymbol*> symbols() const;

    /**Returns a copy of the renderer (a deep copy on the heap)*/
    QgsRenderer* clone() const /Factory/;

    QgsSymbol *symbolForFeature( const QgsFeature* f );
};

