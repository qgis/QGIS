
/** Raster checker for tests in python */

/** \ingroup UnitTests
 * This is a helper class for unit tests that need to
 * write an image and compare it to an expected result
 * or render time.
 */
class QgsRasterChecker
{
%TypeHeaderCode
#include <qgsrasterchecker.h>
%End
  public:

    QgsRasterChecker();

    //! Destructor
    ~QgsRasterChecker();

    QString report();
    /**
     * Test using renderer to generate the image to be compared.
     * @param verifiedKey verified provider key
     * @param verifiedUri URI of the raster to be verified
     * @param expectedKey expected provider key
     * @param expectedUri URI of the expected (control) raster
     */
    bool runTest( const QString& verifiedKey, QString verifiedUri,
                  const QString& expectedKey, QString expectedUri );
};

