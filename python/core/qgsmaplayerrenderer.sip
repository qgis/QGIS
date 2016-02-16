
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

    //! Return list of errors (problems) that happened during the rendering
    QStringList errors() const;

    //! Get access to the ID of the layer rendered by this class
    QString layerID() const;
};
