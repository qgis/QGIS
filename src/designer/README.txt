Notes on the usage of the QGIS custom designer plugins

                 Tim Sutton, 2006
--------------------------------------------------------

Introduction:

The purpose of the QGIS designer plugins is to enable all third
party developers to create GIS enabled Qt4 based applications
with minimal programming. The idea being that you use the 
standard Qt4 Designer GUI design tool to create your user
interface and then add map canvas, legend, projection selector
etc. type of widgets from the toolbox of widgets in designer
- with QGIS having added its own group of custome widgets there.
The QGIS custom widgets can then be graphically 'programmed' by
setting widget properties and using interactive signal/slot 
connectors.

Plugin Paths:

There are two options for having Qt4 Designer find your
plugins at startup:

1) copy the plugin from {QGIS Install Prefix}/lib/qgis/designer
   into the standard Qt4 designer plugin directory at 
   $QTDIR/plugins/designer/

2) export the environment variable QT_PLUGIN_PATH with all the
   places designer should look for your plugins in. Separate
   each entry with a colon. So for example:
   
   export QT_PLUGIN_PATH={QGIS Install Prefix}/lib/qgis

   Note that the 'designer' directory is omitted from the path.

   Its probably a good idea to add the above export clause to 
   your ~/.bash_profile or ~/.bashrc if you plan to use the 
   designer plugins frequently.


Additional Notes:

If you built Qt4 in debug mode then the designer plugins must also 
be built in debug mode or they will be ignored. The converse is also
true.

