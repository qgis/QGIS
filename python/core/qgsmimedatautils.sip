class QgsMimeDataUtils
{
%TypeHeaderCode
#include <qgsmimedatautils.h>
%End
  public:

    struct Uri
    {
      //! Constructs invalid URI
      Uri();
      //! Constructs URI from encoded data
      explicit Uri( QString& encData );

      //! Returns whether the object contains valid data
      //! @note added in 3.0
      bool isValid() const;

      //! Returns encoded representation of the object
      QString data() const;

      //! Type of URI. Recognized types: "vector" / "raster" / "plugin" / "custom"
      QString layerType;
      //! For "vector" / "raster" type: provider id.
      //! For "plugin" type: plugin layer type name.
      //! For "custom" type: key of its QgsCustomDropHandler
      QString providerKey;
      //! Human readable name to be used e.g. in layer tree
      QString name;
      //! Identifier of the data source recognized by its providerKey
      QString uri;
      QStringList supportedCrs;
      QStringList supportedFormats;
    };
    typedef QList<QgsMimeDataUtils::Uri> UriList;

    static QMimeData* encodeUriList( const UriList& layers );

    static bool isUriList( const QMimeData* data );

    static UriList decodeUriList( const QMimeData* data );

    /**
     * Returns encoded URI list from a list of layer tree nodes.
     * @note added in QGIS 3.0
     */
    static QByteArray layerTreeNodesToUriList( const QList<QgsLayerTreeNode*>& nodes );

};
