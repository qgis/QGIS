
class QgsContrastEnhancementFunction
{
%TypeHeaderCode
#include <qgscontrastenhancement.h>
#include <qgscontrastenhancementfunction.h>
%End

  public:
    QgsContrastEnhancementFunction( QGis::DataType, double, double );
    virtual ~QgsContrastEnhancementFunction();


    /** \brief A customizable method that takes in a double and returns a int between 0 and 255 */
    virtual int enhance( double );

    /** \brief A customicable method to indicate if the pixels is displayable */
    virtual bool isValueInDisplayableRange( double );

    /** \brief Mustator for the maximum value */
    void setMaximumValue( double );

    /** \brief Mutator for the minimum value */
    void setMinimumValue( double );
};

