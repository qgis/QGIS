/**Render class to display all the features with a single QgsSymbol*/
class QgsSingleSymbolRenderer : QgsRenderer
{
%TypeHeaderCode
#include <qgssinglesymbolrenderer.h>
%End

 public:
    QgsSingleSymbolRenderer(QGis::GeometryType type);
    QgsSingleSymbolRenderer(const QgsSingleSymbolRenderer& other);
    virtual ~QgsSingleSymbolRenderer();

    /**Replaces the current mSymbol by sy*/
    void addSymbol(QgsSymbol* sy /Transfer/);
    /*Returns a pointer to mSymbol*/
    const QgsSymbol* symbol() const;

    /**Renders a feature added in 1.2 */
    void renderFeature(QgsRenderContext &renderContext, QgsFeature& f, QImage* img, bool selected, double opacity = 1.0 );

    /**Reads the renderer configuration from an XML file
     @param rnode the Dom node to read 
     @param vl the vector layer which will be associated with the renderer*/
    virtual int readXML(const QDomNode& rnode, QgsVectorLayer& vl);
    /**Writes the contents of the renderer to a configuration file*/
    /*virtual void writeXML(std::ostream& xml);*/
    /**Writes the contents of the renderer to a configuration file
     @ return true in case of success*/
    virtual bool writeXML( QDomNode & layer_node, QDomDocument & document, const QgsVectorLayer& vl  ) const;
    /**Returns false, no attributes neede for single symbol*/
    bool needsAttributes() const;
    /**Returns an empty list, since no classification attributes are used*/
    QList<int> classificationAttributes() const;
    /**Returns the renderers name*/
    virtual QString name() const;
    /**Returns a list containing mSymbol*/
    const QList<QgsSymbol*> symbols() const;
    /**Returns a deep copy of this renderer*/
    QgsRenderer* clone() const /Factory/;
};

