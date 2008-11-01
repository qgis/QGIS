
class QgsContrastEnhancementFunction
{
%TypeHeaderCode
#include <qgscontrastenhancement.h>
#include <qgscontrastenhancementfunction.h>
%End

  public:
    QgsContrastEnhancementFunction(QgsContrastEnhancement::QgsRasterDataType, double, double);
    virtual ~QgsContrastEnhancementFunction();
    
    /** \brief Mustator for the maximum value */
    void setMaximumValue(double);    
    /** \brief Mutator for the minimum value */
    void setMinimumValue(double);
    
    /** \brief A customizable method that takes in a double and returns a int between 0 and 255 */
    virtual int enhance(double);
    /** \brief A customicable method to indicate if the pixels is displayable */
    virtual bool isValueInDisplayableRange(double);
    
};

