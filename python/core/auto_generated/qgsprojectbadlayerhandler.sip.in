/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsprojectbadlayerhandler.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsProjectBadLayerHandler
{
%Docstring
Interface for classes that handle missing layer files when reading project file.
%End

%TypeHeaderCode
#include "qgsprojectbadlayerhandler.h"
%End
  public:

    virtual void handleBadLayers( const QList<QDomNode> &layers );
%Docstring
This method will be called whenever the project tries to load layers which
cannot be accessed. It should inform the user about this and if possible offer
to fix the unavailable layers by setting a valid datasource, e.g. by showing a file
dialog.

The default implementation will dismiss all bad layers and write information to the
log.

.. versionadded:: 3.0
%End
    virtual ~QgsProjectBadLayerHandler();


  protected:

    enum DataType
    {
      IS_VECTOR,
      IS_RASTER,
      IS_BOGUS
    };

    enum ProviderType
    {
      IS_FILE,
      IS_DATABASE,
      IS_URL,
      IS_Unknown
    };


    DataType dataType( const QDomNode &layerNode );
%Docstring
Returns data type associated with the given QgsProject file Dom node

The Dom node should represent the state associated with a specific layer.

.. versionadded:: 3.0
%End

    QString dataSource( const QDomNode &layerNode );
%Docstring
Returns the data source for the given layer

The QDomNode is a QgsProject Dom node corresponding to a map layer state.

Essentially dumps datasource tag.

.. versionadded:: 3.0
%End

    ProviderType providerType( const QDomNode &layerNode );
%Docstring
Returns the physical storage type associated with the given layer

The QDomNode is a QgsProject Dom node corresponding to a map layer state.

If the provider tag is "ogr", then it's a file type.

However, if the layer is a raster, then there won't be a
provider tag.  It will always have an associated file.

If the layer doesn't fall into either of the previous two categories, then
it's either a database or URL.  If the datasource tag has "url=", then it's
URL based and if it has "dbname=">, then the layer data is in a database.

.. versionadded:: 3.0
%End

    void setDataSource( QDomNode &layerNode, const QString &dataSource );
%Docstring
Set the datasource element to the new value

.. versionadded:: 3.0
%End
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsprojectbadlayerhandler.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
