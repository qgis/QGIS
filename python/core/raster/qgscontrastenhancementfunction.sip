
class QgsContrastEnhancementFunction
{
%TypeHeaderCode
#include <qgscontrastenhancement.h>
#include <qgscontrastenhancementfunction.h>
%End

  public:
    QgsContrastEnhancementFunction( QGis::DataType, double, double );
    virtual ~QgsContrastEnhancementFunction();

    /** \brief Mustator for the maximum value */
    void setMaximumValue( double );

    /** \brief Mutator for the minimum value */
    void setMinimumValue( double );
};

