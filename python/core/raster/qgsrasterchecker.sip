
/** Raster checker for tests in python */

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
     * @param theVerifiedKey verified provider key
     * @param theVerifiedUri URI of the raster to be verified
     * @param theExpectedKey expected provider key
     * @param theExpectedUri URI of the expected (control) raster
     */
    bool runTest( QString theVerifiedKey, QString theVerifiedUri,
                  QString theExpectedKey, QString theExpectedUri );
};

