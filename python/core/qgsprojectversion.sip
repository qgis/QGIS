/** \ingroup core
 * A class to describe the version of a project.
 * Used in places where you need to check if the current version
 * of QGIS is greater than the one used to create a project file.
 */

class QgsProjectVersion
{
%TypeHeaderCode
#include "qgsprojectversion.h"
%End

  public:

    QgsProjectVersion();
    ~QgsProjectVersion();
    QgsProjectVersion( int major, int minor, int sub, QString name = "" );
    QgsProjectVersion( QString string );
    int majorVersion();
    int minorVersion();
    int subVersion();
    QString text();

    /** Boolean equal operator
    */
    bool operator==( const QgsProjectVersion &other );

    /** Boolean >= operator
    */
    bool operator>=( const QgsProjectVersion &other );

    /** Boolean > operator
    */
    bool operator>( const QgsProjectVersion &other );

};

