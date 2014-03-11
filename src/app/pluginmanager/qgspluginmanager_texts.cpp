#include <QCoreApplication>
#include "qgspluginmanager.h"

// QMap<QString, QString> QgsPluginManager::mTabDescriptions;

void QgsPluginManager::initTabDescriptions()
{
  if ( !mTabDescriptions.isEmpty() )
    return;

  mTabDescriptions.insert( "all_plugins", tr( "<h3>All Plugins</h3>\
\
<p>\
On the left you see the list of all plugins available for your QGIS, both installed and available for download. \
Some plugins come with your QGIS installation while most of them are made available via the plugin repositories.\
</p>\
\
<p>\
You can temporarily enable or disable a plugin.\
 To <i>enable</i> or <i>disable</i> a plugin, click its checkbox or doubleclick its name...\
</p>\
\
<p>\
Plugins showing in <span style='color:red'>red</span> are not loaded because there is a problem. They are also listed \
on the 'Invalid' tab. Click on the plugin name to see more details, or to reinstall or uninstall this plugin.\
</p>\
" ) );



  mTabDescriptions.insert( "installed_plugins", tr( "<h3>Installed Plugins</h3>\
\
<p>\
Here you only see plugins <b>installed on your QGIS</b>.\
</p>\
<p>\
Click on the name to see details. \
</p>\
<p>\
Click the checkbox or doubleclick the name to <i>activate</i> or <i>deactivate</i> the plugin.\
</p>\
<p>\
You can change the sorting via the context menu (right click).\
</p>\
" ) );



  mTabDescriptions.insert( "upgradeable_plugins", tr( "<h3>Upgradable plugins</h3>\
\
<p>\
Here are <b>upgradeable plugins</b>. It means more recent versions of installed \
plugins are available in the repositories.\
</p>\
\
" ) );



  mTabDescriptions.insert( "not_installed_plugins", tr( "<h3>Not installed plugins</h3>\
\
<p>\
Here you see the list of all plugins available in the repositories, but which are <b>not yet installed</b>.\
</p>\
<p>\
Click on the name to see details.\
</p>\
<p>\
You can change the sorting via the context menu (right click).\
</p>\
<p>\
A plugin can be downloaded and installed by clicking on it's name, and \
then click the 'Install plugin' button.\
</p>\
\
\
" ) );



  mTabDescriptions.insert( "new_plugins", tr( "<h3>New plugins</h3>\
\
<p>\
Here you see brand <b>new</b> plugins which can be installed.\
</p>\
\
\
" ) );



  mTabDescriptions.insert( "invalid_plugins", tr( "<h3>Invalid plugins</h3>\
\
<p>\
Plugins in this list here are <b>broken or incompatible</b> with your version of QGIS.\
</p>\
\
<p>\
Click on an individual plugin; if possible QGIS shows you more information.\
</p>\
\
<p>\
The main reasons to have invalid plugins is that this plugin is not build \
for this version of QGIS. Maybe you can download another version from <a href=\"http://plugins.qgis.org\">plugins.qgis.org</a>.\
</p>\
\
<p>\
Another common reason is that a python plugin needs some external python libraries (dependencies). \
You can install them yourself, depending on your operating system. After a correct \
install the plugin should work.\
</p>\
" ) );

}
