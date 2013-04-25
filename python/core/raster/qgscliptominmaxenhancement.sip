class QgsClipToMinMaxEnhancement : QgsContrastEnhancementFunction
{
%TypeHeaderCode
#include <qgscliptominmaxenhancement.h>
%End

  public:
    QgsClipToMinMaxEnhancement( QGis::DataType, double, double );

    int enhance( double );

    bool isValueInDisplayableRange( double );
};
