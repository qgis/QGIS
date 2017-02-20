
class QgsPathResolver
{
%TypeHeaderCode
#include <qgspathresolver.h>
%End

  public:
    explicit QgsPathResolver( const QString& baseFileName = QString() );

    QString writePath( const QString& filename ) const;

    QString readPath( const QString& filename ) const;

};
