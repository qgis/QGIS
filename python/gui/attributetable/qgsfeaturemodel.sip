class QgsFeatureModel
{
%TypeHeaderCode
#include <qgsfeaturemodel.h>
%End
  public:
    virtual QModelIndex fidToIndex( QgsFeatureId fid ) = 0;
};
