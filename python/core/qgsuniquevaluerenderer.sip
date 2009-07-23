class QgsUniqueValueRenderer : QgsRenderer
{
%TypeHeaderCode
#include <qgsuniquevaluerenderer.h>
%End

 public:
    QgsUniqueValueRenderer(QGis::GeometryType type);
    QgsUniqueValueRenderer(const QgsUniqueValueRenderer& other);
    virtual ~QgsUniqueValueRenderer();
    /** Determines if a feature will be rendered or not 
	@param f a pointer to the feature to determine if rendering will happen*/
    bool willRenderFeature(QgsFeature *f);

    /* render feature
     * added in 1.2 */
    void renderFeature(QgsRenderContext &renderContext, QgsFeature& f, QImage* img, bool selected);

    /**Reads the renderer configuration from an XML file
     @param rnode the Dom node to read 
     @param vl the vector layer which will be associated with the renderer*/
    virtual int readXML(const QDomNode& rnode, QgsVectorLayer& vl);
    /**Writes the contents of the renderer to a configuration file
     @ return true in case of success*/
    virtual bool writeXML( QDomNode & layer_node, QDomDocument & document, const QgsVectorLayer& vl ) const;
    /** Returns true, if attribute values are used by the renderer and false otherwise*/
    bool needsAttributes() const;
    /**Returns a list with indexes of classification attributes*/
    QList<int> classificationAttributes() const;
    /**Returns the renderers name*/
    QString name() const;
    /**Inserts an entry into mEntries. The render items have to be created with the new operator and are automatically destroyed if not needed anymore*/
    void insertValue(QString name, QgsSymbol* symbol /Transfer/);
    /**Removes all entries from mEntries*/
    void clearValues();
    /**Sets the Field index used for classification*/
    void setClassificationField(int field);
    /**Returns the index of the classification field*/
    int classificationField();
    /**Return symbology items*/
    const QList<QgsSymbol*> symbols() const;
    QgsRenderer* clone() const /Factory/;
};

