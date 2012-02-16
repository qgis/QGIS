
class QgsMapToolEdit : QgsMapTool
{
%TypeHeaderCode
#include <qgsmaptooledit.h>
%End

%ConvertToSubClassCode
  if (dynamic_cast<QgsMapToolVertexEdit*>(sipCpp) != NULL)
    sipClass = sipClass_QgsMapToolVertexEdit;
  else if (dynamic_cast<QgsMapToolNodeTool*>(sipCpp) != NULL)
    sipClass = sipClass_QgsMapToolNodeTool;
  else
    sipClass = NULL;
%End

  public:
    //! constructor
    QgsMapToolEdit(QgsMapCanvas* canvas);

    virtual ~QgsMapToolEdit();

    virtual bool isEditTool();

};

