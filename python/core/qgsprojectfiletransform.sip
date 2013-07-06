class QgsProjectFileTransform
{
%TypeHeaderCode
#include <qgsprojectfiletransform.h>
%End
  public:
    //Default constructor
    ~QgsProjectFileTransform();

    /*! Create an instance from a Dom and a supplied version
     * @param domDocument The Dom document to use as content
     * @param version Version number
     */
    QgsProjectFileTransform( QDomDocument & domDocument,
                             QgsProjectVersion version );
    bool updateRevision( QgsProjectVersion version );

    /*! Prints the contents via QgsDebugMsg()
     */
    void dump();

    static void convertRasterProperties( QDomDocument& doc, QDomNode& parentNode, QDomElement& rasterPropertiesElem, QgsRasterLayer* rlayer );
};
