class QgsClipToMinMaxEnhancement : QgsContrastEnhancementFunction
{
%TypeHeaderCode
#include <qgscliptominmaxenhancement.h>
%End

  public:
    QgsClipToMinMaxEnhancement( Qgis::DataType, double, double );

    int enhance( double );

    bool isValueInDisplayableRange( double );
};
