class QgsLinearMinMaxEnhancement : QgsContrastEnhancementFunction
{
%TypeHeaderCode
#include <qgslinearminmaxenhancement.h>
%End

  public:
    QgsLinearMinMaxEnhancement( Qgis::DataType, double, double );

    int enhance( double );

};
