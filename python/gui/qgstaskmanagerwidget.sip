/** \ingroup gui
 * \class QgsTaskManagerWidget
 * A widget which displays tasks from a QgsTaskManager and allows for interaction with the manager
 * @see QgsTaskManager
 * @note introduced in QGIS 3.0
 */
class QgsTaskManagerWidget : QWidget
{
%TypeHeaderCode
#include <qgstaskmanagerwidget.h>
%End
  public:

     /** Constructor for QgsTaskManagerWidget
     * @param manager task manager associated with widget
     * @param parent parent widget
     */
     QgsTaskManagerWidget( QgsTaskManager* manager, QWidget* parent /TransferThis/ = nullptr );

};

