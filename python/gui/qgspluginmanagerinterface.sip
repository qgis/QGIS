/**
 * \class QgsPluginManagerInterface
 * \brief Abstract base class to make QgsPluginManager available to pyplugin_installer.
 */
class QgsPluginManagerInterface : QObject
{
%TypeHeaderCode
#include <qgspluginmanagerinterface.h>
%End

  public:

    //! Constructor
    QgsPluginManagerInterface();

    //! Virtual destructor
    ~QgsPluginManagerInterface();

    //! remove python plugins from the metadata registry (c++ plugins stay)
    virtual void clearPythonPluginMetadata() = 0;

    //! add a single plugin to the metadata registry
    virtual void addPluginMetadata( QMap<QString, QString> metadata ) = 0;

    //! refresh plugin list model (and metadata browser content if necessary)
    virtual void reloadModel() = 0;

    //! return given plugin metadata
    virtual QMap<QString, QString> * pluginMetadata( QString key ) = 0;

    //! clear the repository listWidget
    virtual void clearRepositoryList() = 0;

    //! add repository to the repository listWidget
    virtual void addToRepositoryList( QMap<QString, QString> repository ) = 0;

    //! show the Plugin Manager window and optionally open tab tabIndex
    virtual void showPluginManager( int tabIndex = -1 ) = 0;
};
