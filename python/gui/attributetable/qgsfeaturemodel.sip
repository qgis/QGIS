class QgsFeatureModel
{
%TypeHeaderCode
#include <qgsfeaturemodel.h>
%End
  public:
    virtual ~QgsFeatureModel();
    virtual QModelIndex fidToIndex( QgsFeatureId fid ) = 0;
};
