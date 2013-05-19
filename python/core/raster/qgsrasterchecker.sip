
/**  Raster checker for tests in python */

class QgsRasterChecker
{
%TypeHeaderCode
#include <qgsrasterchecker.h>
%End
  public:

    QgsRasterChecker();

    ~QgsRasterChecker();

    QString report();

    bool runTest( QString theVerifiedKey, QString theVerifiedUri,
                  QString theExpectedKey, QString theExpectedUri );
};

