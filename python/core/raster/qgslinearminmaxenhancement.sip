class QgsLinearMinMaxEnhancement : QgsContrastEnhancementFunction
{
%TypeHeaderCode
#include <qgslinearminmaxenhancement.h>
%End

  public:
    QgsLinearMinMaxEnhancement( QgsContrastEnhancement::QgsRasterDataType, double, double );

    int enhance( double );

};
