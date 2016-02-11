class QgsWkbPtr
{
%TypeHeaderCode
#include <qgswkbptr.h>
%End

  public:
    QgsWkbPtr( unsigned char *wkb /Array/, int size /ArraySize/ );

};

class QgsConstWkbPtr
{
%TypeHeaderCode
#include <qgswkbptr.h>
%End

  public:
    QgsConstWkbPtr( unsigned char *wkb /Array/, int size /ArraySize/ );

};
