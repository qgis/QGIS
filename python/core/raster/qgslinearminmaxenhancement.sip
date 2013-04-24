class QgsLinearMinMaxEnhancement : QgsContrastEnhancementFunction
{
%TypeHeaderCode
#include <qgslinearminmaxenhancement.h>
%End

  public:
    QgsLinearMinMaxEnhancement( QGis::DataType, double, double );

    int enhance( double );

};
