/**This class contains the information for graduate symbol rendering*/
class QgsGraduatedSymbolRenderer : QgsRenderer
{
%TypeHeaderCode
#include <qgsgraduatedsymbolrenderer.h>
%End

 public:
    QgsGraduatedSymbolRenderer(QGis::VectorType type);
    QgsGraduatedSymbolRenderer(const QgsGraduatedSymbolRenderer& other);
    virtual ~QgsGraduatedSymbolRenderer();
    /**Adds a new item
    \param sy a pointer to the QgsSymbol to be inserted. It has to be created using the new operator and is automatically destroyed when 'removeItems' is called or when this object is destroyed*/
    void addSymbol(QgsSymbol* sy /Transfer/);
    /**Returns the number of the classification field*/
    int classificationField() const;
    /**Removes all symbols*/
    void removeSymbols();
    /** Determines if a feature will be rendered or not 
	@param f a pointer to the feature to determine if rendering will happen*/
    bool willRenderFeature(QgsFeature *f);
    /**Renders an OGRFeature
     \param p a painter (usually the one from the current map canvas)
     \param f a pointer to a feature to render
     \param t the transform object containing the information how to transform the map coordinates to screen coordinates*/
    void renderFeature(QPainter* p, QgsFeature& f, QImage* img, bool selected, double widthScale = 1.0, double rasterScaleFactor = 1.0);	
    /**Sets the number of the classicifation field
    \param field the number of the field to classify*/
    void setClassificationField(int field);
    /**Reads the renderer configuration from an XML file
     @param rnode the Dom node to read 
     @param vl the vector layer which will be associated with the renderer*/
    virtual void readXML(const QDomNode& rnode, QgsVectorLayer& vl);
    /**Writes the contents of the renderer to a configuration file
     @ return true in case of success*/
    virtual bool writeXML( QDomNode & layer_node, QDomDocument & document ) const;
    /** Returns true*/
    bool needsAttributes() const;
    /**Returns a list with the index to the classification field*/
    QList<int> classificationAttributes() const;
    /**Returns the renderers name*/
    QString name() const;
    /**Returns the symbols of the items*/
    const QList<QgsSymbol*> symbols() const;
    /**Returns a copy of the renderer (a deep copy on the heap)*/
    QgsRenderer* clone() const /Factory/;
};

