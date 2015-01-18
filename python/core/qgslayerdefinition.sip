class CORE_EXPORT QgsLayerDefinition
{
%TypeHeaderCode
#include <qgslayerdefinition.h>
%End
public:
  static bool openLayerDefinition( const QString & path, QgsLayerTreeGroup* rootGroup, QString &errorMessage /Out/ );
  static bool openLayerDefinition( QDomDocument doc, QgsLayerTreeGroup* rootGroup, QString &errorMessage /Out/ );
  static bool exportLayerDefinition( QString path, QList<QgsLayerTreeNode*> selectedTreeNodes, QString &errorMessage /Out/ );
};

