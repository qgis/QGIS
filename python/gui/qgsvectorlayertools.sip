class QgsVectorLayerTools
{
%TypeHeaderCode
#include <qgsvectorlayertools.h>
%End
  public:
     virtual bool addFeature( QgsVectorLayer* layer, QgsAttributeMap defaultValues = QgsAttributeMap(), const QgsGeometry& = QgsGeometry() ) = 0;
     virtual bool startEditing( QgsVectorLayer* layer ) = 0;
     virtual bool stopEditing( QgsVectorLayer* layer, bool allowCancel = true ) = 0;
};
