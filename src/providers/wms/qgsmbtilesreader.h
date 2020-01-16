#ifndef QGSMBTILESREADER_H
#define QGSMBTILESREADER_H


#include "sqlite3.h"
#include "qgssqliteutils.h"

class QImage;
class QgsRectangle;

class QgsMBTilesReader
{
  public:
    explicit QgsMBTilesReader( const QString &filename );

    bool open();

    bool isOpen() const;

    QString metadataValue( const QString &key );

    //! given in WGS 84 (if available)
    QgsRectangle extent();

    QByteArray tileData( int z, int x, int y );

    QImage tileDataAsImage( int z, int x, int y );

  private:
    QString mFilename;
    sqlite3_database_unique_ptr mDatabase;
};


#endif // QGSMBTILESREADER_H
