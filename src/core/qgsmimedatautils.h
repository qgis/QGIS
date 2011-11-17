#ifndef QGSMIMEDATAUTILS_H
#define QGSMIMEDATAUTILS_H

#include <QMimeData>

class QgsLayerItem;

class CORE_EXPORT QgsMimeDataUtils
{
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
    };
    typedef QList<Uri> UriList;

    static QMimeData* encodeUriList( UriList layers );

    static bool isUriList( const QMimeData* data );

    static UriList decodeUriList( const QMimeData* data );

};

#endif // QGSMIMEDATAUTILS_H
