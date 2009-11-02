
/**Renderer class which interpolates rgb values linear between the minimum and maximum value of the classification field*/
class QgsContinuousColorRenderer : QgsRenderer
{
%TypeHeaderCode
#include <qgscontinuouscolorrenderer.h>
%End

 public:
    QgsContinuousColorRenderer(QGis::GeometryType type);
    QgsContinuousColorRenderer(const QgsContinuousColorRenderer& other);
    virtual ~QgsContinuousColorRenderer();
    /**Renders the feature using the minimum and maximum value of the classification field
      * added in 1.2 */
    void renderFeature(QgsRenderContext &renderContext, QgsFeature& f, QImage* img, bool selected);
    /**Returns the number of the classification field*/
    int classificationField() const;
    /**Sets the id of the classification field*/
    void setClassificationField(int id);
    /**Sets the symbol for the minimum value. The symbol has to be created using the new operator and is automatically deleted when inserting a new symbol or when the instance is destroyed*/
    void setMinimumSymbol(QgsSymbol* sy /Transfer/);
    /**Sets the symbol for the maximum value. The symbol has to be created using the new operator and is automatically deleted when inserting a new symbol or when the instance is destroyed*/
    void setMaximumSymbol(QgsSymbol* sy /Transfer/);
    /** Sets whether to draw the polygon outline*/
    void setDrawPolygonOutline(bool draw);
    /**Returns the symbol for the minimum value*/
    const QgsSymbol* minimumSymbol() const;
    /**Returns the symbol for the maximum value*/
    const QgsSymbol* maximumSymbol() const;
    /** whether to draw a polygon outline*/
    bool drawPolygonOutline() const;
    /**Reads the renderer configuration from an XML file
     @param rnode the Dom node to read 
     @param vl the vector layer which will be associated with the renderer*/
    virtual int readXML(const QDomNode& rnode, QgsVectorLayer& vl);
    /**Writes the contents of the renderer to a configuration file
     @ return true in case of success*/
    virtual bool writeXML( QDomNode & layer_node, QDomDocument & document, const QgsVectorLayer& vl  ) const;
    /** Returns true*/
    bool needsAttributes() const;
    /**Returns a list with the index of the classification attribute*/
    QList<int> classificationAttributes() const;
    /**Returns the renderers name*/
    QString name() const;
    /**Return symbology items*/
    const QList<QgsSymbol*> symbols() const;
    QgsRenderer* clone() const /Factory/;
};

