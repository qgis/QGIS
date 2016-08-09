
class QgsMapLayerRenderer
{
%TypeHeaderCode
#include <qgsmaplayerrenderer.h>
%End

  public:
    QgsMapLayerRenderer( const QString& layerID );
    virtual ~QgsMapLayerRenderer();

    //! Do the rendering (based on data stored in the class)
    virtual bool render() = 0;

    //! Access to feedback object of the layer renderer (may be null)
    //! @note added in QGIS 3.0
    virtual QgsFeedback* feedback() const;

    //! Return list of errors (problems) that happened during the rendering
    QStringList errors() const;

    //! Get access to the ID of the layer rendered by this class
    QString layerId() const;
};
