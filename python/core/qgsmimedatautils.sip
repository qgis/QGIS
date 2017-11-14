/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsmimedatautils.h                                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsMimeDataUtils
{

%TypeHeaderCode
#include "qgsmimedatautils.h"
%End
  public:

    struct Uri
    {
      Uri();
%Docstring
Constructs invalid URI
%End
      explicit Uri( QString &encData );
%Docstring
Constructs URI from encoded data
%End

      bool isValid() const;
%Docstring
 Returns whether the object contains valid data
.. versionadded:: 3.0
 :rtype: bool
%End

      QString data() const;
%Docstring
Returns encoded representation of the object
 :rtype: str
%End

      QgsVectorLayer *vectorLayer( bool &owner, QString &error ) const;
%Docstring
 Get vector layer from uri if possible, otherwise returns 0 and error is set
 \param owner set to true if caller becomes owner
 \param error set to error message if cannot get vector
 :rtype: QgsVectorLayer
%End

      QgsRasterLayer *rasterLayer( bool &owner, QString &error ) const;
%Docstring
 Get raster layer from uri if possible, otherwise returns 0 and error is set
 \param owner set to true if caller becomes owner
 \param error set to error message if cannot get raster
 :rtype: QgsRasterLayer
%End

      QString layerType;
%Docstring
Type of URI. Recognized types: "vector" / "raster" / "plugin" / "custom"
%End

      QString providerKey;
%Docstring
 For "vector" / "raster" type: provider id.
 For "plugin" type: plugin layer type name.
 For "custom" type: key of its QgsCustomDropHandler
%End
      QString name;
%Docstring
Human readable name to be used e.g. in layer tree
%End
      QString uri;
%Docstring
Identifier of the data source recognized by its providerKey
%End
      QStringList supportedCrs;
      QStringList supportedFormats;
    };
    typedef QList<QgsMimeDataUtils::Uri> UriList;

    static QMimeData *encodeUriList( const UriList &layers );
%Docstring
 :rtype: QMimeData
%End

    static bool isUriList( const QMimeData *data );
%Docstring
 :rtype: bool
%End

    static UriList decodeUriList( const QMimeData *data );
%Docstring
 :rtype: UriList
%End

    static QByteArray layerTreeNodesToUriList( const QList<QgsLayerTreeNode *> &nodes );
%Docstring
 Returns encoded URI list from a list of layer tree nodes.
.. versionadded:: 3.0
 :rtype: QByteArray
%End

};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsmimedatautils.h                                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
