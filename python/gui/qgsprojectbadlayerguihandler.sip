
/** \ingroup gui
  Handler for missing layers within project.

  Gives user a chance to select path to the missing layers.

  @note added in 1.4
 */
class QgsProjectBadLayerGuiHandler : QObject, QgsProjectBadLayerHandler
{
%TypeHeaderCode
#include <qgsprojectbadlayerguihandler.h>
%End

  public:
    QgsProjectBadLayerGuiHandler();

    /** implementation of the handler */
    virtual void handleBadLayers( QList<QDomNode> layers, QDomDocument projectDom );

    /** Flag to store the Ignore button press of MessageBox used by QgsLegend */
    static bool mIgnore;
};
