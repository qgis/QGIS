
/*! \class QgisPlugin 
 * \brief Abstract base class from which all plugins must inherit
 *
 */
class QgisPlugin
{
%TypeHeaderCode
#include <qgisplugin.h>
%End

  public:

    //! Interface to gui element collection object
    //virtual QgisPluginGui *gui()=0;
    //! Element types that can be added to the interface
    /* enum ELEMENTS {
       MENU,
       MENU_ITEM,
       TOOLBAR,
       TOOLBAR_BUTTON,
       };

       @todo XXX this may be a hint that there should be subclasses
       */

    enum PLUGINTYPE
    {
      UI = 1,
      MAPLAYER,
      RENDERER
    };



// TODO: needs %MethodCode
    QgisPlugin ( const QString & name = "", 
        const QString & description = "",
        const QString & version = "",
        PLUGINTYPE type = MAPLAYER );

    virtual ~QgisPlugin();

    //! Get the name of the plugin
    QString & name(); 

    //! Version of the plugin
    QString & version();

    //! A brief description of the plugin
    QString & description();

    //! Plugin type, either UI or map layer
    PLUGINTYPE type(); 

    /// function to initialize connection to GUI
    virtual void initGui() = 0;

    //! Unload the plugin and cleanup the GUI
    virtual void unload() = 0;

}; // class QgisPlugin


