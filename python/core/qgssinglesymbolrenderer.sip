/**Render class to display all the features with a single QgsSymbol*/
class QgsSingleSymbolRenderer : QgsRenderer
{
%TypeHeaderCode
#include <qgssinglesymbolrenderer.h>
%End

 public:
    QgsSingleSymbolRenderer(QGis::VectorType type);
    QgsSingleSymbolRenderer(const QgsSingleSymbolRenderer& other);
    virtual ~QgsSingleSymbolRenderer();
    /**Replaces the current mSymbol by sy*/
    void addSymbol(QgsSymbol* sy /Transfer/);
    /*Returns a pointer to mSymbol*/
    const QgsSymbol* symbol() const;
    /**Renders an OGRFeature*/
    void renderFeature(QPainter* p, QgsFeature& f, QImage* img, double* scalefactor, bool selected, double widthScale = 1);
    /**Reads the renderer configuration from an XML file
     @param rnode the DOM node to read 
     @param vl the vector layer which will be associated with the renderer*/
    virtual void readXML(const QDomNode& rnode, QgsVectorLayer& vl);
    /**Writes the contents of the renderer to a configuration file*/
    /*virtual void writeXML(std::ostream& xml);*/
    /**Writes the contents of the renderer to a configuration file
     @ return true in case of success*/
    virtual bool writeXML( QDomNode & layer_node, QDomDocument & document ) const;
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

