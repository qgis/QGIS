class QgsVectorOverlay
{
%TypeHeaderCode
#include "qgsvectoroverlay.h"
%End
  public:
    QgsVectorOverlay( QgsVectorLayer* vl );
    virtual ~QgsVectorOverlay();

    /**Create the overlay objects contained in a view extent. Subclasses need to implement this method and assign                   width/height information to the overlay objects*/

    virtual void createOverlayObjects( const QgsRenderContext& renderContext ) = 0;

    /**Remove the overlay objects and release their memory*/
    void removeOverlayObjects();

    /**Draw the overlay objects*/
    virtual void drawOverlayObjects( QgsRenderContext& context ) const = 0;

    /**Gives direct access to oberlay objects*/
    QMap<int, QgsOverlayObject*>* overlayObjects();

    /**Describes the overlay type (e.g. "diagram" or "label")*/
    virtual QString typeName() const = 0;

    /**Set attribute indices necessary to fetch*/
    void setAttributes( const QList<int>& list );

    bool displayFlag() const;

    /**Display yes/no*/
    void setDisplayFlag( bool flag );

    /**Restore from project file*/
    virtual bool readXML( const QDomNode& overlayNode ) = 0;

    /**Save to project file*/
    virtual bool writeXML( QDomNode& layer_node, QDomDocument& doc ) const = 0;
};
