class QgsMimeDataUtils
{
%TypeHeaderCode
#include <qgsmimedatautils.h>
%End
  public:

    struct Uri
    {
      Uri( QgsLayerItem* layer );
      Uri( QString& encData );

      QString data() const;

      QString layerType;
      QString providerKey;
      QString name;
      QString uri;
      QStringList supportedCrs;
      QStringList supportedFormats;
    };
    typedef QList<QgsMimeDataUtils::Uri> UriList;

    static QMimeData* encodeUriList( UriList layers );

    static bool isUriList( const QMimeData* data );

    static UriList decodeUriList( const QMimeData* data );

};
