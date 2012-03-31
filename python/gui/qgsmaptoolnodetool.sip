class QgsMapToolNodeTool : QgsMapToolVertexEdit
{
%TypeHeaderCode
#include <qgsmaptoolnodetool.h>
%End

  public:
    //! constructor
    QgsMapToolNodeTool(QgsMapCanvas* canvas);

    virtual ~QgsMapToolNodeTool();

    virtual bool isEditTool();

    // SelectionFeature* selectionFeature();

    int featureId();

};

