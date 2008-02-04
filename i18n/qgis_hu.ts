<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS><TS version="1.1">
<context>
    <name>@default</name>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1703"/>
        <source>OGR Driver Manager</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1703"/>
        <source>unable to get OGRDriverManager</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>Dialog</name>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="13"/>
        <source>QGIS Plugin Installer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="19"/>
        <source>Select repository, retrieve the list of available plugins, select one and install it</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="26"/>
        <source>Repository</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="34"/>
        <source>Active repository:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="61"/>
        <source>Get List</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="81"/>
        <source>Add</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="88"/>
        <source>Edit</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="95"/>
        <source>Delete</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="117"/>
        <source>Name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="122"/>
        <source>Version</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="127"/>
        <source>Description</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="132"/>
        <source>Author</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="142"/>
        <source>Name of plugin to install</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="152"/>
        <source>Install Plugin</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="163"/>
        <source>The plugin will be installed to ~/.qgis/python/plugins</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="176"/>
        <source>Done</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>Gui</name>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="55"/>
        <source>Welcome to your automatically generated plugin!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="56"/>
        <source>This is just a starting point. You now need to modify the code to make it do something useful....read on for a more information to get yourself started.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="57"/>
        <source>Documentation:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="58"/>
        <source>You really need to read the QGIS API Documentation now at:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="59"/>
        <source>In particular look at the following classes:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="62"/>
        <source>QgsPlugin is an ABC that defines required behaviour your plugin must provide. See below for more details.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="63"/>
        <source>What are all the files in my generated plugin directory for?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="64"/>
        <source>This is the generated CMake file that builds the plugin. You should add you application specific dependencies and source files to this file.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="65"/>
        <source>This is the class that provides the &apos;glue&apos; between your custom application logic and the QGIS application. You will see that a number of methods are already implemented for you - including some examples of how to add a raster or vector layer to the main application map canvas. This class is a concrete instance of the QgisPlugin interface which defines required behaviour for a plugin. In particular, a plugin has a number of static methods and members so that the QgsPluginManager and plugin loader logic can identify each plugin, create an appropriate menu entry for it etc. Note there is nothing stopping you creating multiple toolbar icons and menu entries for a single plugin. By default though a single menu entry and toolbar button is created and its pre-configured to call the run() method in this class when selected. This default implementation provided for you by the plugin builder is well documented, so please refer to the code for further advice.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="66"/>
        <source>This is a Qt designer &apos;ui&apos; file. It defines the look of the default plugin dialog without implementing any application logic. You can modify this form to suite your needs or completely remove it if your plugin does not need to display a user form (e.g. for custom MapTools).</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="67"/>
        <source>This is the concrete class where application logic for the above mentioned dialog should go. The world is your oyster here really....</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="68"/>
        <source>This is the Qt4 resources file for your plugin. The Makefile generated for your plugin is all set up to compile the resource file so all you need to do is add your additional icons etc using the simple xml file format. Note the namespace used for all your resources e.g. (&apos;:/Homann/&apos;). It is important to use this prefix for all your resources. We suggest you include any other images and run time data in this resurce file too.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="69"/>
        <source>This is the icon that will be used for your plugin menu entry and toolbar icon. Simply replace this icon with your own icon to make your plugin disctinctive from the rest.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="70"/>
        <source>This file contains the documentation you are reading now!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="71"/>
        <source>Getting developer help:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="72"/>
        <source>For Questions and Comments regarding the plugin builder template and creating your features in QGIS using the plugin interface please contact us via:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="73"/>
        <source>&lt;li&gt; the QGIS developers mailing list, or &lt;/li&gt;&lt;li&gt; IRC (#qgis on freenode.net)&lt;/li&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="74"/>
        <source>QGIS is distributed under the Gnu Public License. If you create a useful plugin please consider contributing it back to the community.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="75"/>
        <source>Have fun and thank you for choosing QGIS.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>MapCoordsDialogBase</name>
    <message>
        <location filename="../src/plugins/georeferencer/mapcoordsdialogbase.ui" line="13"/>
        <source>Enter map coordinates</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/mapcoordsdialogbase.ui" line="28"/>
        <source>Enter X and Y coordinates which correspond with the selected point on the image. Alternatively, click the button with icon of a pencil and then click a corresponding point on map canvas of QGIS to fill in coordinates of that point.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/mapcoordsdialogbase.ui" line="62"/>
        <source>X:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/mapcoordsdialogbase.ui" line="69"/>
        <source>Y:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/mapcoordsdialogbase.ui" line="137"/>
        <source> from map canvas</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/mapcoordsdialogbase.ui" line="172"/>
        <source>&amp;Cancel</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/mapcoordsdialogbase.ui" line="185"/>
        <source>&amp;OK</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QFileDialog</name>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="650"/>
        <source>Load layer properties from style file (.qml)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="712"/>
        <source>Save layer properties as style file (.qml)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintgui.cpp" line="69"/>
        <source>Save experiment report to portable document format (.pdf)</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2311"/>
        <source>Where is &apos;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2311"/>
        <source>original location: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2880"/>
        <source>QGis files (*.qgs)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="41"/>
        <source>Not a vector layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="42"/>
        <source>The current layer is not a vector layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="58"/>
        <source>2.5D shape type not supported</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="58"/>
        <source>Adding features to 2.5D shapetypes is not supported yet</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="71"/>
        <source>Layer cannot be added to</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="72"/>
        <source>The data provider for this layer does not support the addition of features.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="48"/>
        <source>Layer not editable</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="50"/>
        <source>Cannot edit the vector layer. To make it editable, go to the file item of the layer, right click and check &apos;Allow Editing&apos;.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="199"/>
        <source>Wrong editing tool</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="91"/>
        <source>Cannot apply the &apos;capture point&apos; tool on this vector layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="64"/>
        <source>Coordinate transform error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="65"/>
        <source>Cannot transform the point to the layers coordinate system</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="192"/>
        <source>Cannot apply the &apos;capture line&apos; tool on this vector layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="200"/>
        <source>Cannot apply the &apos;capture polygon&apos; tool on this vector layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="419"/>
        <source>Error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="408"/>
        <source>Cannot add feature. Unknown WKB type</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="419"/>
        <source>Could not remove polygon intersection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdisland.cpp" line="112"/>
        <source>Error, could not add island</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="90"/>
        <source>A problem with geometry type occured</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="94"/>
        <source>The inserted Ring is not closed</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="98"/>
        <source>The inserted Ring is not a valid geometry</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="102"/>
        <source>The inserted Ring crosses existing rings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="106"/>
        <source>The inserted Ring is not contained in a feature</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="110"/>
        <source>An unknown error occured</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="112"/>
        <source>Error, could not add ring</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolselect.cpp" line="75"/>
        <source>No active layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="122"/>
        <source>To identify features, you must choose an active layer by clicking on its name in the legend</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="153"/>
        <source>Band</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="453"/>
        <source>Length</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="459"/>
        <source>Area</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="466"/>
        <source>action</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="475"/>
        <source> features found</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="479"/>
        <source> 1 feature found</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="485"/>
        <source>No features found</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="485"/>
        <source>No features were found in the active layer at the point you clicked</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="586"/>
        <source>Could not identify objects on</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="586"/>
        <source>because</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolselect.cpp" line="76"/>
        <source>To select features, you must choose a vector layer by clicking on its name in the legend</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgspythonutils.cpp" line="400"/>
        <source>Python error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgspythonutils.cpp" line="57"/>
        <source>Couldn&apos;t load SIP module.
Python support will be disabled.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgspythonutils.cpp" line="67"/>
        <source>Couldn&apos;t load PyQt bindings.
Python support will be disabled.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgspythonutils.cpp" line="78"/>
        <source>Couldn&apos;t load QGIS bindings.
Python support will be disabled.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgspythonutils.cpp" line="380"/>
        <source>Couldn&apos;t load plugin </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgspythonutils.cpp" line="369"/>
        <source> due an error when calling its classFactory() method</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgspythonutils.cpp" line="381"/>
        <source> due an error when calling its initGui() method</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgspythonutils.cpp" line="401"/>
        <source>Error while unloading plugin </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="311"/>
        <source>Caught a coordinate system exception while trying to transform a point. Unable to calculate line length.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="402"/>
        <source>Caught a coordinate system exception while trying to transform a point. Unable to calculate polygon area.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="648"/>
        <source> km2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="653"/>
        <source> ha</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="658"/>
        <source> m2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="686"/>
        <source> m</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="671"/>
        <source> km</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="676"/>
        <source> mm</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="681"/>
        <source> cm</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="695"/>
        <source> sq mile</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="700"/>
        <source> sq ft</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="707"/>
        <source> mile</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="713"/>
        <source> foot</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="715"/>
        <source> feet</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="722"/>
        <source> sq.deg.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="727"/>
        <source> degree</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="729"/>
        <source> degrees</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="733"/>
        <source> unknown</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="264"/>
        <source>Received %1 of %2 bytes</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="270"/>
        <source>Received %1 bytes (total unknown)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="381"/>
        <source>Not connected</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="387"/>
        <source>Looking up &apos;%1&apos;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="394"/>
        <source>Connecting to &apos;%1&apos;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="401"/>
        <source>Sending request &apos;%1&apos;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="408"/>
        <source>Receiving reply</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="414"/>
        <source>Response is complete</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="420"/>
        <source>Closing down connection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgslabelattributes.cpp" line="58"/>
        <source>Label</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="429"/>
        <source>Currently only filebased datasets are supported</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="486"/>
        <source>Loaded default style file from </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="509"/>
        <source>The directory containing your dataset needs to be writeable!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="533"/>
        <source>Created default style file as </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="537"/>
        <source>ERROR: Failed to created default style file as </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="753"/>
        <source>Unable to open </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="771"/>
        <source>Project file read error: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="771"/>
        <source> at line </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="772"/>
        <source> column </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="778"/>
        <source> for file </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="934"/>
        <source>Unable to save to file </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsproviderregistry.cpp" line="87"/>
        <source>No Data Provider Plugins</source>
        <comment>No QGIS data provider plugins found in:</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsproviderregistry.cpp" line="89"/>
        <source>No vector layers can be loaded. Check your QGIS installation</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsproviderregistry.cpp" line="92"/>
        <source>No Data Providers</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsproviderregistry.cpp" line="251"/>
        <source>No data provider plugins are available. No vector layers can be loaded</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgssearchtreenode.cpp" line="253"/>
        <source>Regular expressions on numeric values don&apos;t make sense. Use comparison instead.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgssearchtreenode.cpp" line="289"/>
        <source>Referenced column wasn&apos;t found: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgssearchtreenode.cpp" line="293"/>
        <source>Division by zero.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="51"/>
        <source>CopyrightLabel</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="52"/>
        <source>Draws copyright information</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfsplugin.cpp" line="30"/>
        <source>Version 0.1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="44"/>
        <source>Version 0.2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="45"/>
        <source>Loads and displays delimited text files containing x,y coordinates</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="161"/>
        <source>Add Delimited Text Layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="47"/>
        <source>PostgreSQL Geoprocessing</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="48"/>
        <source>Geoprocessing functions for working with PostgreSQL/PostGIS layers</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugin.cpp" line="57"/>
        <source>Georeferencer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugin.cpp" line="58"/>
        <source>Adding projection info to rasters</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsleastsquares.cpp" line="32"/>
        <source>Fit to a linear transform requires at least 2 points.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsleastsquares.cpp" line="71"/>
        <source>Fit to a Helmert transform requires at least 2 points.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsleastsquares.cpp" line="123"/>
        <source>Fit to an affine transform requires at least 4 points.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="54"/>
        <source>GPS Tools</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="56"/>
        <source>Tools for loading and importing GPS data</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="72"/>
        <source>New centroid</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="223"/>
        <source>New point</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="134"/>
        <source>New vertex</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="223"/>
        <source>Undo last point</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="223"/>
        <source>Close line</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="543"/>
        <source>Select vertex</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="296"/>
        <source>Select new position</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="427"/>
        <source>Select line segment</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="414"/>
        <source>New vertex position</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="414"/>
        <source>Release</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="530"/>
        <source>Delete vertex</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="530"/>
        <source>Release vertex</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="784"/>
        <source>Select element</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="597"/>
        <source>New location</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="673"/>
        <source>Release selected</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="673"/>
        <source>Delete selected / select next</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="736"/>
        <source>Select position on line</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="754"/>
        <source>Split the line</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="754"/>
        <source>Release the line</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="768"/>
        <source>Select point on line</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="136"/>
        <source>Location: </source>
        <comment>Metadata in GRASS Browser</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="136"/>
        <source>&lt;br&gt;Mapset: </source>
        <comment>Metadata in GRASS Browser</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="140"/>
        <source>Location: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="140"/>
        <source>&lt;br&gt;Mapset: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="145"/>
        <source>&lt;b&gt;Raster&lt;/b&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="154"/>
        <source>Cannot open raster header</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="158"/>
        <source>Rows</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="159"/>
        <source>Columns</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="160"/>
        <source>N-S resolution</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="161"/>
        <source>E-W resolution</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="299"/>
        <source>North</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="301"/>
        <source>South</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="303"/>
        <source>East</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="305"/>
        <source>West</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="188"/>
        <source>Format</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="199"/>
        <source>Minimum value</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="200"/>
        <source>Maximum value</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="211"/>
        <source>Data source</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="216"/>
        <source>Data description</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="225"/>
        <source>Comments</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="240"/>
        <source>Categories</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="346"/>
        <source>&lt;b&gt;Vector&lt;/b&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="273"/>
        <source>Points</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="274"/>
        <source>Lines</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="275"/>
        <source>Boundaries</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="276"/>
        <source>Centroids</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="279"/>
        <source>Faces</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="280"/>
        <source>Kernels</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="283"/>
        <source>Areas</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="284"/>
        <source>Islands</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="308"/>
        <source>Top</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="309"/>
        <source>Bottom</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="312"/>
        <source>yes</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="312"/>
        <source>no</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="319"/>
        <source>History&lt;br&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="347"/>
        <source>&lt;b&gt;Layer&lt;/b&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="366"/>
        <source>Features</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="375"/>
        <source>Driver</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="376"/>
        <source>Database</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="377"/>
        <source>Table</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="378"/>
        <source>Key column</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="831"/>
        <source>GRASS</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="837"/>
        <source>GRASS layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugin.cpp" line="43"/>
        <source>Graticule Creator</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugin.cpp" line="44"/>
        <source>Builds a graticule</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="60"/>
        <source>NorthArrow</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="61"/>
        <source>Displays a north arrow overlayed onto the map</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugin.cpp" line="38"/>
        <source>[menuitemname]</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugin.cpp" line="39"/>
        <source>[plugindescription]</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintplugin.cpp" line="38"/>
        <source>Quick Print</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintplugin.cpp" line="40"/>
        <source>Quick Print is a plugin to quickly print a map with minimal effort.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="62"/>
        <source>ScaleBar</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="63"/>
        <source>Draws a scale bar</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitplugin.cpp" line="37"/>
        <source>SPIT</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitplugin.cpp" line="38"/>
        <source>Shapefile to PostgreSQL/PostGIS Import Tool</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfsplugin.cpp" line="28"/>
        <source>WFS plugin</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfsplugin.cpp" line="29"/>
        <source>Adds WFS layers to the QGIS canvas</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/gpx/gpsdata.cpp" line="330"/>
        <source>Couldn&apos;t open the data source: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/gpx/gpsdata.cpp" line="352"/>
        <source>Parse error at line </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/gpx/qgsgpxprovider.cpp" line="53"/>
        <source>GPS eXchange format provider</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="156"/>
        <source>GRASS plugin</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="129"/>
        <source>QGIS couldn&apos;t find your GRASS installation.
Would you like to specify path (GISBASE) to your GRASS installation?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="143"/>
        <source>Choose GRASS installation path (GISBASE)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="157"/>
        <source>GRASS data won&apos;t be available if GISBASE is not specified.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="413"/>
        <source>GISBASE is not set.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="418"/>
        <source> is not a GRASS mapset.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="440"/>
        <source>Cannot start </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="457"/>
        <source>Mapset is already in use.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="472"/>
        <source>Temporary directory </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="472"/>
        <source> exist but is not writable</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="478"/>
        <source>Cannot create temporary directory </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="494"/>
        <source>Cannot create </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="567"/>
        <source>Cannot remove mapset lock: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="1007"/>
        <source>Warning</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="955"/>
        <source>Cannot read raster map region</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="972"/>
        <source>Cannot read vector map region</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="1008"/>
        <source>Cannot read region</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgisApp</name>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="340"/>
        <source>Quantum GIS - </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="308"/>
        <source>Checking database</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="314"/>
        <source>Reading settings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="318"/>
        <source>Setting up the GUI</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="353"/>
        <source>Checking provider plugins</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="358"/>
        <source>Starting Python</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="368"/>
        <source>Restoring loaded plugins</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="372"/>
        <source>Initializing file filters</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="397"/>
        <source>Restoring window state</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="401"/>
        <source>QGIS Ready!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="469"/>
        <source>&amp;New Project</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="470"/>
        <source>Ctrl+N</source>
        <comment>New Project</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="471"/>
        <source>New Project</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="474"/>
        <source>&amp;Open Project...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="475"/>
        <source>Ctrl+O</source>
        <comment>Open a Project</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="476"/>
        <source>Open a Project</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="479"/>
        <source>&amp;Save Project</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="480"/>
        <source>Ctrl+S</source>
        <comment>Save Project</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="481"/>
        <source>Save Project</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="484"/>
        <source>Save Project &amp;As...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="485"/>
        <source>Ctrl+A</source>
        <comment>Save Project under a new name</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="486"/>
        <source>Save Project under a new name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="489"/>
        <source>&amp;Print...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="490"/>
        <source>Ctrl+P</source>
        <comment>Print</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="491"/>
        <source>Print</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="494"/>
        <source>Save as Image...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="495"/>
        <source>Ctrl+I</source>
        <comment>Save map as image</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="496"/>
        <source>Save map as image</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="507"/>
        <source>Exit</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="508"/>
        <source>Ctrl+Q</source>
        <comment>Exit QGIS</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="509"/>
        <source>Exit QGIS</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="514"/>
        <source>Add a Vector Layer...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="515"/>
        <source>V</source>
        <comment>Add a Vector Layer</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="516"/>
        <source>Add a Vector Layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="519"/>
        <source>Add a Raster Layer...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="520"/>
        <source>R</source>
        <comment>Add a Raster Layer</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="521"/>
        <source>Add a Raster Layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="524"/>
        <source>Add a PostGIS Layer...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="525"/>
        <source>D</source>
        <comment>Add a PostGIS Layer</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="526"/>
        <source>Add a PostGIS Layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="536"/>
        <source>New Vector Layer...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="537"/>
        <source>N</source>
        <comment>Create a New Vector Layer</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="538"/>
        <source>Create a New Vector Layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="541"/>
        <source>Remove Layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="542"/>
        <source>Ctrl+D</source>
        <comment>Remove a Layer</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="543"/>
        <source>Remove a Layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="546"/>
        <source>Add All To Overview</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="547"/>
        <source>+</source>
        <comment>Show all layers in the overview map</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="548"/>
        <source>Show all layers in the overview map</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="551"/>
        <source>Remove All From Overview</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="552"/>
        <source>-</source>
        <comment>Remove all layers from overview map</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="553"/>
        <source>Remove all layers from overview map</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="556"/>
        <source>Show All Layers</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="557"/>
        <source>S</source>
        <comment>Show all layers</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="558"/>
        <source>Show all layers</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="561"/>
        <source>Hide All Layers</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="562"/>
        <source>H</source>
        <comment>Hide all layers</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="563"/>
        <source>Hide all layers</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="568"/>
        <source>Project Properties...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="569"/>
        <source>P</source>
        <comment>Set project properties</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="570"/>
        <source>Set project properties</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="573"/>
        <source>Options...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="575"/>
        <source>Change various QGIS options</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="578"/>
        <source>Custom Projection...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="580"/>
        <source>Manage custom projections</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="585"/>
        <source>Help Contents</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="587"/>
        <source>Ctrl+?</source>
        <comment>Help Documentation (Mac)</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="589"/>
        <source>F1</source>
        <comment>Help Documentation</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="591"/>
        <source>Help Documentation</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="594"/>
        <source>Qgis Home Page</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="596"/>
        <source>Ctrl+H</source>
        <comment>QGIS Home Page</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="598"/>
        <source>QGIS Home Page</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="601"/>
        <source>About</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="602"/>
        <source>About QGIS</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="605"/>
        <source>Check Qgis Version</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="606"/>
        <source>Check if your QGIS version is up to date (requires internet access)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="611"/>
        <source>Refresh</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="612"/>
        <source>Ctrl+R</source>
        <comment>Refresh Map</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="613"/>
        <source>Refresh Map</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="618"/>
        <source>Zoom In</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="617"/>
        <source>Ctrl++</source>
        <comment>Zoom In</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="623"/>
        <source>Zoom Out</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="622"/>
        <source>Ctrl+-</source>
        <comment>Zoom Out</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="626"/>
        <source>Zoom Full</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="627"/>
        <source>F</source>
        <comment>Zoom to Full Extents</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="628"/>
        <source>Zoom to Full Extents</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="631"/>
        <source>Zoom To Selection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="632"/>
        <source>Ctrl+F</source>
        <comment>Zoom to selection</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="633"/>
        <source>Zoom to selection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="636"/>
        <source>Pan Map</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="637"/>
        <source>Pan the map</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="640"/>
        <source>Zoom Last</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="642"/>
        <source>Zoom to Last Extent</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="645"/>
        <source>Zoom To Layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="647"/>
        <source>Zoom to Layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="650"/>
        <source>Identify Features</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="651"/>
        <source>I</source>
        <comment>Click on features to identify them</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="652"/>
        <source>Click on features to identify them</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="656"/>
        <source>Select Features</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="662"/>
        <source>Open Table</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="666"/>
        <source>Measure Line </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="667"/>
        <source>Ctrl+M</source>
        <comment>Measure a Line</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="668"/>
        <source>Measure a Line</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="671"/>
        <source>Measure Area</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="672"/>
        <source>Ctrl+J</source>
        <comment>Measure an Area</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="673"/>
        <source>Measure an Area</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="678"/>
        <source>Show Bookmarks</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="677"/>
        <source>B</source>
        <comment>Show Bookmarks</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="683"/>
        <source>Show most toolbars</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="682"/>
        <source>T</source>
        <comment>Show most toolbars</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="689"/>
        <source>Hide most toolbars</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="688"/>
        <source>Ctrl+T</source>
        <comment>Hide most toolbars</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="693"/>
        <source>New Bookmark...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="694"/>
        <source>Ctrl+B</source>
        <comment>New Bookmark</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5343"/>
        <source>New Bookmark</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="698"/>
        <source>Add WMS Layer...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="699"/>
        <source>W</source>
        <comment>Add Web Mapping Server Layer</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="700"/>
        <source>Add Web Mapping Server Layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="703"/>
        <source>In Overview</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="704"/>
        <source>O</source>
        <comment>Add current layer to overview map</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="705"/>
        <source>Add current layer to overview map</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="710"/>
        <source>Plugin Manager...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="712"/>
        <source>Open the plugin manager</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="723"/>
        <source>Toggle editing</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="724"/>
        <source>Toggles the editing state of the current layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="729"/>
        <source>Capture Point</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="730"/>
        <source>.</source>
        <comment>Capture Points</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="731"/>
        <source>Capture Points</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="735"/>
        <source>Capture Line</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="736"/>
        <source>/</source>
        <comment>Capture Lines</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="737"/>
        <source>Capture Lines</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="741"/>
        <source>Capture Polygon</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="742"/>
        <source>Ctrl+/</source>
        <comment>Capture Polygons</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="743"/>
        <source>Capture Polygons</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="748"/>
        <source>Delete Selected</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="753"/>
        <source>Move Feature</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="758"/>
        <source>Split Features</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="763"/>
        <source>Add Vertex</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="768"/>
        <source>Delete Vertex</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="773"/>
        <source>Move Vertex</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="778"/>
        <source>Add Ring</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="782"/>
        <source>Add Island</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="783"/>
        <source>Add Island to multipolygon</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="787"/>
        <source>Cut Features</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="788"/>
        <source>Cut selected features</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="792"/>
        <source>Copy Features</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="793"/>
        <source>Copy selected features</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="797"/>
        <source>Paste Features</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="798"/>
        <source>Paste selected features</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="803"/>
        <source>Map Tips</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="804"/>
        <source>Show information about a feature when the mouse is hovered over it</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="809"/>
        <source>Python console</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="871"/>
        <source>&amp;File</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="874"/>
        <source>&amp;Open Recent Projects</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="894"/>
        <source>&amp;View</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="912"/>
        <source>&amp;Layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="932"/>
        <source>&amp;Settings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="939"/>
        <source>&amp;Plugins</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="955"/>
        <source>&amp;Help</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="974"/>
        <source>File</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="985"/>
        <source>Manage Layers</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1003"/>
        <source>Help</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1010"/>
        <source>Digitizing</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1030"/>
        <source>Map Navigation</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1043"/>
        <source>Attributes</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1056"/>
        <source>Plugins</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1066"/>
        <source>Toolbar Visibility...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1080"/>
        <source>Progress bar that displays the status of rendering layers and other time-intensive operations</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1093"/>
        <source>Scale </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1094"/>
        <source>Current map scale</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1106"/>
        <source>Displays the current map scale</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1107"/>
        <source>Current map scale (formatted as x:y)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1117"/>
        <source>Shows the map coordinates at the current cursor position. The display is continuously updated as the mouse is moved.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1118"/>
        <source>Map coordinates at mouse cursor position</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1121"/>
        <source>Render</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1124"/>
        <source>When checked, the map layers are rendered in response to map navigation commands and other events. When not checked, no rendering is done. This allows you to add a large number of layers and symbolize them before rendering.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1125"/>
        <source>Toggle map rendering</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1140"/>
        <source>This icon shows whether on the fly projection is enabled or not. Click the icon to bring up the project properties dialog to alter this behaviour.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1141"/>
        <source>Projection status - Click to open projection dialog</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1145"/>
        <source>Ready</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1265"/>
        <source>Map canvas. This is where raster and vector layers are displayed when added to the map</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1314"/>
        <source>Map overview canvas. This canvas can be used to display a locator map that shows the current extent of the map canvas. The current extent is shown as a red rectangle. Any layer on the map can be added to the overview canvas.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1347"/>
        <source>Map legend that displays all the layers currently on the map canvas. Click on the check box to turn a layer on or off. Double click on a layer in the legend to customize its appearance and set other properties.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1480"/>
        <source>Version </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1487"/>
        <source> with PostgreSQL support</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1490"/>
        <source> (no PostgreSQL support)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1492"/>
        <source>
Compiled against Qt </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1493"/>
        <source>, running against Qt </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1503"/>
        <source>Quantum GIS is licensed under the GNU General Public License</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1505"/>
        <source>http://www.gnu.org/licenses</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1507"/>
        <source>Version</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1509"/>
        <source>New features</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1512"/>
        <source>This release candidate includes over 40 bug fixes and enchancements over the QGIS 0.9.1 release. In addition we have added the following new features:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1515"/>
        <source>Imrovements to digitising capabilities.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1521"/>
        <source>Supporting default and defined styles (.qml) files for file based vector layers. With styles you can save the symbolisation and other settings associated with a vector layer and they will be loaded whenever you load that layer.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1527"/>
        <source>Improved support for transparency and contrast stretching in raster layers. Support for color ramps in raster layers. Support for non-north up rasters. Many other raster improvements &apos;under the hood&apos;.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1537"/>
        <source>Available Data Provider Plugins</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1636"/>
        <source>Python error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1636"/>
        <source>Error when reading metadata of plugin </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1911"/>
        <source>Open an OGR Supported Vector Layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1965"/>
        <source>is not a valid or recognized data source</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5091"/>
        <source>Invalid Data Source</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2057"/>
        <source>Invalid Layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2057"/>
        <source>%1 is an invalid layer and cannot be loaded.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2538"/>
        <source>Save As</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2638"/>
        <source>Choose a QGIS project file to open</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2749"/>
        <source>QGIS Project Read Error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2750"/>
        <source></source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2751"/>
        <source>Try to find missing layers?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2767"/>
        <source>Unable to open project</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2806"/>
        <source>Choose a QGIS project file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2923"/>
        <source>Saved project to:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2930"/>
        <source>Unable to save project</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2931"/>
        <source>Unable to save project to </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2937"/>
        <source>Unable to save project </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2879"/>
        <source>Choose a filename to save the QGIS project file as</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2995"/>
        <source>QGIS: Unable to load project</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2996"/>
        <source>Unable to load project </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3119"/>
        <source>Choose a filename to save the map image as</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3161"/>
        <source>Saved map image to</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3338"/>
        <source>No Layer Selected</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3339"/>
        <source>To delete features, you must select a vector layer in the legend</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3346"/>
        <source>No Vector Layer Selected</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3347"/>
        <source>Deleting features only works on vector layers</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3353"/>
        <source>Provider does not support deletion</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3354"/>
        <source>Data provider does not support deleting features</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3360"/>
        <source>Layer not editable</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3361"/>
        <source>The current layer is not editable. Choose &apos;Start editing&apos; in the digitizing toolbar.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3368"/>
        <source>Problem deleting features</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3369"/>
        <source>A problem occured during deletion of features</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3631"/>
        <source>Invalid scale</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3862"/>
        <source>Error Loading Plugin</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3862"/>
        <source>There was an error loading %1.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3898"/>
        <source>No MapLayer Plugins</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3898"/>
        <source>No MapLayer plugins in ../plugins/maplayer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3975"/>
        <source>No Plugins</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3976"/>
        <source>No plugins found in ../plugins. To test plugins, start qgis from the src directory</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4010"/>
        <source>Name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4010"/>
        <source>Plugin %1 is named %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4027"/>
        <source>Plugin Information</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4028"/>
        <source>QGis loaded the following plugin:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4028"/>
        <source>Name: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4028"/>
        <source>Version: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4029"/>
        <source>Description: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4047"/>
        <source>Unable to Load Plugin</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4048"/>
        <source>QGIS was unable to load the plugin from: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4104"/>
        <source>There is a new version of QGIS available</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4110"/>
        <source>You are running a development version of QGIS</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4114"/>
        <source>You are running the current version of QGIS</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4119"/>
        <source>Would you like more information?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4166"/>
        <source>QGIS Version Information</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4126"/>
        <source>QGIS - Changes in SVN Since Last Release</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4138"/>
        <source>Unable to get current version information from server</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4152"/>
        <source>Connection refused - server may be down</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4155"/>
        <source>QGIS server was not found</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4158"/>
        <source>Network error while communicating with server</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4161"/>
        <source>Unknown network socket error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4166"/>
        <source>Unable to communicate with QGIS Version server</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4245"/>
        <source>QGIS Browser Selection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4246"/>
        <source>Enter the name of a web browser to use (eg. konqueror).
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4247"/>
        <source>Enter the full path if the browser is not in your PATH.
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4248"/>
        <source>You can change this option later by selecting Options from the Settings menu (Help Browser tab).</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5177"/>
        <source>Layer is not valid</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5178"/>
        <source>The layer is not a valid layer and can not be added to the map</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4417"/>
        <source>Save?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4418"/>
        <source>Do you want to save the current project?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4604"/>
        <source>Extents: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4954"/>
        <source>Clipboard contents set to: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4999"/>
        <source>Open a GDAL Supported Raster Data Source</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5090"/>
        <source> is not a valid or recognized raster data source</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5256"/>
        <source> is not a supported raster data source</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5257"/>
        <source>Unsupported Data Source</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5344"/>
        <source>Enter a name for the new bookmark:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5361"/>
        <source>Error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5361"/>
        <source>Unable to create the bookmark. Your user database may be missing or corrupted</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5396"/>
        <source>Project file is older</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5398"/>
        <source>&lt;p&gt;This project file was saved by an older version of QGIS.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5400"/>
        <source> When saving this project file, QGIS will update it to the latest version, possibly rendering it useless for older versions of QGIS.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5403"/>
        <source>&lt;p&gt;Even though QGIS developers try to maintain backwards compatibility, some of the information from the old project file might be lost.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5405"/>
        <source> To improve the quality of QGIS, we appreciate if you file a bug report at %3.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5407"/>
        <source> Be sure to include the old project file, and state the version of QGIS you used to discover the error.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5409"/>
        <source>&lt;p&gt;To remove this warning when opening an older project file, uncheck the box &apos;%5&apos; in the %4 menu.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5409"/>
        <source>&lt;p&gt;Version of the project file: %1&lt;br&gt;Current version of QGIS: %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5413"/>
        <source>&lt;tt&gt;Settings:Options:General&lt;/tt&gt;</source>
        <comment>Menu path to setting options</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5414"/>
        <source>Warn me when opening a project file saved with an older version of QGIS</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgisAppBase</name>
    <message>
        <location filename="../src/ui/qgisappbase.ui" line="13"/>
        <source>MainWindow</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgisappbase.ui" line="102"/>
        <source>Legend</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgisappbase.ui" line="135"/>
        <source>Map View</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsAbout</name>
    <message>
        <location filename="../src/app/qgsabout.cpp" line="54"/>
        <source>Quantum GIS is licensed under the GNU General Public License</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsabout.cpp" line="118"/>
        <source>QGIS Sponsors</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsabout.cpp" line="121"/>
        <source>The following have sponsored QGIS by contributing money to fund development and other project costs</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsabout.cpp" line="125"/>
        <source>Name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsabout.cpp" line="125"/>
        <source>Website</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsabout.cpp" line="255"/>
        <source>QGIS Browser Selection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsabout.cpp" line="259"/>
        <source>Enter the name of a web browser to use (eg. konqueror).
Enter the full path if the browser is not in your PATH.
You can change this option later by selection Options from the Settings menu (Help Browser tab).</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="13"/>
        <source>About Quantum GIS</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="32"/>
        <source>About</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="76"/>
        <source>&lt;h2&gt;Quantum GIS (qgis)&lt;/h2&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="86"/>
        <source>Version</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="107"/>
        <source>QGIS Home Page</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="117"/>
        <source>Subscribe to the QGIS-User mailing list</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="128"/>
        <source>What&apos;s New</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="157"/>
        <source>Developers</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="225"/>
        <source>&lt;h2&gt;QGIS Developers&lt;/h2&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="233"/>
        <source>Providers</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="249"/>
        <source>Sponsors</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="296"/>
        <source>Ok</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsAddAttrDialogBase</name>
    <message>
        <location filename="../src/ui/qgsaddattrdialogbase.ui" line="13"/>
        <source>Add Attribute</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsaddattrdialogbase.ui" line="52"/>
        <source>OK</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsaddattrdialogbase.ui" line="59"/>
        <source>Cancel</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsaddattrdialogbase.ui" line="87"/>
        <source>Type:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsaddattrdialogbase.ui" line="100"/>
        <source>Name:</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsAttributeActionDialog</name>
    <message>
        <location filename="../src/app/qgsattributeactiondialog.cpp" line="57"/>
        <source>Name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributeactiondialog.cpp" line="58"/>
        <source>Action</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributeactiondialog.cpp" line="59"/>
        <source>Capture</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributeactiondialog.cpp" line="142"/>
        <source>Select an action</source>
        <comment>File dialog window title</comment>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsAttributeActionDialogBase</name>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="21"/>
        <source>Form1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="49"/>
        <source>Remove the selected action</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="52"/>
        <source>Remove</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="59"/>
        <source>Move the selected action down</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="62"/>
        <source>Move down</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="69"/>
        <source>Move the selected action up</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="72"/>
        <source>Move up</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="87"/>
        <source>This list contains all actions that have been defined for the current layer. Add actions by entering the details in the controls below and then pressing the Insert action button. Actions can be edited here by double clicking on the item.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="119"/>
        <source>The valid attribute names for this layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="126"/>
        <source>Browse for action commands</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="129"/>
        <source>Browse</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="136"/>
        <source>Inserts the selected field into the action, prepended with a %</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="139"/>
        <source>Insert field</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="162"/>
        <source>Update the selected action</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="165"/>
        <source>Update action</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="172"/>
        <source>Inserts the action into the list above</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="175"/>
        <source>Insert action</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="182"/>
        <source>Captures any output from the action</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="185"/>
        <source>Captures the standard output or error generated by the action and displays it in a dialog box</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="188"/>
        <source>Capture output</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="195"/>
        <source>Enter the action command here</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="205"/>
        <source>Enter the action here. This can be any program, script or command that is available on your system. When the action is invoked any set of characters that start with a % and then have the name of a field will be replaced by the value of that field. The special characters %% will be replaced by the value of the field that was selected. Double quote marks group text into single arguments to the program, script or command. Double quotes will be ignored if preceeded by a backslash</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="208"/>
        <source>Action:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="218"/>
        <source>Enter the action name here</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="228"/>
        <source>Enter the name of an action here. The name should be unique (qgis will make it unique if necessary).</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="231"/>
        <source>Name:</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsAttributeDialogBase</name>
    <message>
        <location filename="../src/ui/qgsattributedialogbase.ui" line="16"/>
        <source>Enter Attribute Values</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributedialogbase.ui" line="32"/>
        <source>1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributedialogbase.ui" line="37"/>
        <source>Attribute</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributedialogbase.ui" line="42"/>
        <source>Value</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributedialogbase.ui" line="50"/>
        <source>&amp;OK</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributedialogbase.ui" line="57"/>
        <source>&amp;Cancel</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsAttributeTable</name>
    <message>
        <location filename="../src/app/qgsattributetable.cpp" line="280"/>
        <source>Run action</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsAttributeTableBase</name>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="13"/>
        <source>Attribute Table</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="36"/>
        <source>Remove selection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="52"/>
        <source>Move selected to top</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="61"/>
        <source>Ctrl+T</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="68"/>
        <source>Invert selection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="77"/>
        <source>Ctrl+S</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="84"/>
        <source>Copy selected rows to clipboard (Ctrl+C)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="87"/>
        <source>Copies the selected rows to the clipboard</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="96"/>
        <source>Ctrl+C</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="103"/>
        <source>Zoom map to the selected rows (Ctrl-F)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="106"/>
        <source>Zoom map to the selected rows</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="112"/>
        <source>Ctrl+F</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="127"/>
        <source>New column</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="136"/>
        <source>Ctrl+N</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="143"/>
        <source>Delete column</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="152"/>
        <source>Ctrl+X</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="175"/>
        <source>Start editing</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="182"/>
        <source>Stop editin&amp;g</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="185"/>
        <source>Alt+G</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="214"/>
        <source>&amp;Help</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="282"/>
        <source>Alt+C</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="224"/>
        <source>Search for:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="237"/>
        <source>in</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="259"/>
        <source>Search</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="269"/>
        <source>Adva&amp;nced...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="272"/>
        <source>Alt+N</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="279"/>
        <source>&amp;Close</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsAttributeTableDisplay</name>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="94"/>
        <source>select</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="95"/>
        <source>select and bring to top</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="96"/>
        <source>show only matching</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="144"/>
        <source>Name conflict</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="144"/>
        <source>The attribute could not be inserted. The name already exists in the table.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="193"/>
        <source>Stop editing</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="194"/>
        <source>Do you want to save the changes?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="200"/>
        <source>Error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="200"/>
        <source>Could not commit changes</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="321"/>
        <source>Search string parsing error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="373"/>
        <source>Search results</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="327"/>
        <source>You&apos;ve supplied an empty search string.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="361"/>
        <source>Error during search</source>
        <translation type="unfinished"></translation>
    </message>
    <message numerus="yes">
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="370"/>
        <source>Found %d matching features.</source>
        <translation type="unfinished">
            <numerusform></numerusform>
        </translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="372"/>
        <source>No matching features found.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsBookmarks</name>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="140"/>
        <source>Really Delete?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="141"/>
        <source>Are you sure you want to delete the </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="141"/>
        <source> bookmark?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="157"/>
        <source>Error deleting bookmark</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="159"/>
        <source>Failed to delete the </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="161"/>
        <source> bookmark from the database. The database said:
</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsBookmarksBase</name>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="16"/>
        <source>Geospatial Bookmarks</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="45"/>
        <source>Name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="50"/>
        <source>Project</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="55"/>
        <source>Extent</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="60"/>
        <source>Id</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="68"/>
        <source>Help</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="75"/>
        <source>Close the dialog</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="78"/>
        <source>Close</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="85"/>
        <source>Delete the currently selected bookmark</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="88"/>
        <source>Delete</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="95"/>
        <source>Zoom to the currently selected bookmark</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="98"/>
        <source>Zoom To</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsComposer</name>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="51"/>
        <source>QGIS - print composer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="73"/>
        <source>Map 1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="500"/>
        <source>Couldn&apos;t open </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="500"/>
        <source> for read/write</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="579"/>
        <source>Error in Print</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="557"/>
        <source>Cannot seek</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="482"/>
        <source>Cannot overwrite BoundingBox</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="490"/>
        <source>Cannot find BoundingBox</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="573"/>
        <source>Cannot overwrite translate</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="579"/>
        <source>Cannot find translate</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="587"/>
        <source>File IO Error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="597"/>
        <source>Paper does not match</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="598"/>
        <source>The selected paper size does not match the composition size</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="661"/>
        <source>Big image</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="662"/>
        <source>To create image </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="665"/>
        <source> requires circa </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="665"/>
        <source> MB of memory</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="692"/>
        <source>format</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="715"/>
        <source>Choose a filename to save the map image as</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="787"/>
        <source>SVG warning</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="788"/>
        <source>Don&apos;t show this message again</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="802"/>
        <source>&lt;p&gt;The SVG export function in Qgis has several problems due to bugs and deficiencies in the Qt4 svg code. Of note, text does not appear in the SVG file and there are problems with the map bounding box clipping other items such as the legend or scale bar.&lt;/p&gt;If you require a vector-based output file from Qgis it is suggested that you try printing to PostScript if the SVG output is not satisfactory.&lt;/p&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="807"/>
        <source>Choose a filename to save the map as</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="808"/>
        <source>SVG Format</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsComposerBase</name>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="13"/>
        <source>MainWindow</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="62"/>
        <source>General</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="107"/>
        <source>Composition</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="115"/>
        <source>Item</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="150"/>
        <source>Help</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="170"/>
        <source>Close</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="208"/>
        <source>&amp;Open Template ...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="216"/>
        <source>Save Template &amp;As...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="224"/>
        <source>&amp;Print...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="232"/>
        <source>Zoom All</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="240"/>
        <source>Zoom In</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="248"/>
        <source>Zoom Out</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="256"/>
        <source>Add new map</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="264"/>
        <source>Add new label</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="272"/>
        <source>Add new vect legend</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="280"/>
        <source>Select/Move item</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="288"/>
        <source>Export as image</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="296"/>
        <source>Export as SVG</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="304"/>
        <source>Add new scalebar</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="312"/>
        <source>Refresh view</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="320"/>
        <source>Add Image</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsComposerLabelBase</name>
    <message>
        <location filename="../src/ui/qgscomposerlabelbase.ui" line="21"/>
        <source>Label Options</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlabelbase.ui" line="48"/>
        <source>Font</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlabelbase.ui" line="55"/>
        <source>Box</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsComposerMap</name>
    <message>
        <location filename="../src/app/composer/qgscomposermap.cpp" line="74"/>
        <source>Map %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposermap.cpp" line="96"/>
        <source>Extent (calculate scale)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposermap.cpp" line="97"/>
        <source>Scale (calculate extent)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposermap.cpp" line="104"/>
        <source>Cache</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposermap.cpp" line="105"/>
        <source>Render</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposermap.cpp" line="106"/>
        <source>Rectangle</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsComposerMapBase</name>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="21"/>
        <source>Map options</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="69"/>
        <source>Set map extent to current extent in QGIS map canvas</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="72"/>
        <source>Set Extent</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="79"/>
        <source>1:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="97"/>
        <source>Scale:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="116"/>
        <source>Width of one unit in millimeters</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="147"/>
        <source>Set</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="173"/>
        <source>&lt;b&gt;Map&lt;/b&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="180"/>
        <source>Height</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="196"/>
        <source>Width</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="212"/>
        <source>Line width scale</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="225"/>
        <source>Symbol scale</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="238"/>
        <source>Font size scale</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="251"/>
        <source>Frame</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="258"/>
        <source>Preview</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsComposerPicture</name>
    <message>
        <location filename="../src/app/composer/qgscomposerpicture.cpp" line="401"/>
        <source>Warning</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposerpicture.cpp" line="402"/>
        <source>Cannot load picture.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposerpicture.cpp" line="468"/>
        <source>Pictures (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposerpicture.cpp" line="485"/>
        <source>Choose a file</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsComposerPictureBase</name>
    <message>
        <location filename="../src/ui/qgscomposerpicturebase.ui" line="21"/>
        <source>Picture Options</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerpicturebase.ui" line="58"/>
        <source>Browse</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerpicturebase.ui" line="119"/>
        <source>Width</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerpicturebase.ui" line="140"/>
        <source>Height</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerpicturebase.ui" line="161"/>
        <source>Angle</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerpicturebase.ui" line="197"/>
        <source>Frame</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsComposerScalebarBase</name>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="21"/>
        <source>Barscale Options</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="41"/>
        <source>Line width</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="88"/>
        <source>Unit label</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="127"/>
        <source>Map</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="140"/>
        <source>Segment size</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="159"/>
        <source>Map units per scalebar unit</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="172"/>
        <source>Number of segments</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="195"/>
        <source>Font</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsComposerVectorLegend</name>
    <message>
        <location filename="../src/app/composer/qgscomposervectorlegend.cpp" line="104"/>
        <source>Legend</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposervectorlegend.cpp" line="117"/>
        <source>Layers</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposervectorlegend.cpp" line="118"/>
        <source>Group</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposervectorlegend.cpp" line="125"/>
        <source>Combine selected layers</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposervectorlegend.cpp" line="138"/>
        <source>Cache</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposervectorlegend.cpp" line="139"/>
        <source>Render</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposervectorlegend.cpp" line="140"/>
        <source>Rectangle</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsComposerVectorLegendBase</name>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="21"/>
        <source>Vector Legend Options</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="53"/>
        <source>Preview</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="92"/>
        <source>Map</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="113"/>
        <source>Title</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="140"/>
        <source>Column 1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="148"/>
        <source>Box</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="163"/>
        <source>Font</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsComposition</name>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="84"/>
        <source>Custom</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="85"/>
        <source>A5 (148x210 mm)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="86"/>
        <source>A4 (210x297 mm)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="87"/>
        <source>A3 (297x420 mm)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="88"/>
        <source>A2 (420x594 mm)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="89"/>
        <source>A1 (594x841 mm)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="90"/>
        <source>A0 (841x1189 mm)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="91"/>
        <source>B5 (176 x 250 mm)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="92"/>
        <source>B4 (250 x 353 mm)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="93"/>
        <source>B3 (353 x 500 mm)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="94"/>
        <source>B2 (500 x 707 mm)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="95"/>
        <source>B1 (707 x 1000 mm)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="96"/>
        <source>B0 (1000 x 1414 mm)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="97"/>
        <source>Letter (8.5x11 inches)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="98"/>
        <source>Legal (8.5x14 inches)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="111"/>
        <source>Portrait</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="112"/>
        <source>Landscape</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="607"/>
        <source>Out of memory</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="610"/>
        <source>Qgis is unable to resize the paper size due to insufficient memory.
 It is best that you avoid using the map composer until you restart qgis.
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="754"/>
        <source>Label</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="804"/>
        <source>Warning</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="805"/>
        <source>Cannot load picture.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsCompositionBase</name>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="21"/>
        <source>Composition</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="33"/>
        <source>Paper</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="104"/>
        <source>Orientation</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="122"/>
        <source>Height</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="140"/>
        <source>Width</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="158"/>
        <source>Units</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="176"/>
        <source>Size</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="213"/>
        <source>Resolution (dpi)</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsConnectionDialog</name>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialog.cpp" line="74"/>
        <source>Test connection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialog.cpp" line="72"/>
        <source>Connection to </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialog.cpp" line="72"/>
        <source> was successfull</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialog.cpp" line="74"/>
        <source>Connection failed - Check settings and try again </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialog.cpp" line="96"/>
        <source>General Interface Help:

</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsConnectionDialogBase</name>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="13"/>
        <source>Create a New PostGIS connection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="31"/>
        <source>Connection Information</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="51"/>
        <source>Save Password</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="58"/>
        <source>Test Connect</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="83"/>
        <source>Name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="90"/>
        <source>Host</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="97"/>
        <source>Database</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="104"/>
        <source>Port</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="111"/>
        <source>Username</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="118"/>
        <source>Password</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="135"/>
        <source>Name of the new connection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="148"/>
        <source>5432</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsContinuousColorDialogBase</name>
    <message>
        <location filename="../src/ui/qgscontinuouscolordialogbase.ui" line="13"/>
        <source>Continuous color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscontinuouscolordialogbase.ui" line="28"/>
        <source>Draw polygon outline</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscontinuouscolordialogbase.ui" line="41"/>
        <source>Classification Field:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscontinuouscolordialogbase.ui" line="57"/>
        <source>Minimum Value:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscontinuouscolordialogbase.ui" line="73"/>
        <source>Outline Width:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscontinuouscolordialogbase.ui" line="99"/>
        <source>Maximum Value:</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsCoordinateTransform</name>
    <message>
        <location filename="../src/core/qgscoordinatetransform.cpp" line="418"/>
        <source>The source spatial reference system (SRS) is not valid. </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgscoordinatetransform.cpp" line="426"/>
        <source>The coordinates can not be reprojected. The SRS is: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgscoordinatetransform.cpp" line="425"/>
        <source>The destination spatial reference system (SRS) is not valid. </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgscoordinatetransform.cpp" line="483"/>
        <source>Failed</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgscoordinatetransform.cpp" line="483"/>
        <source>transform of</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgscoordinatetransform.cpp" line="496"/>
        <source>with error: </source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsCopyrightLabelPlugin</name>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="66"/>
        <source>Bottom Left</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="67"/>
        <source>Top Left</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="67"/>
        <source>Top Right</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="67"/>
        <source>Bottom Right</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="79"/>
        <source>&amp;Copyright Label</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="80"/>
        <source>Creates a copyright label that is displayed on the map canvas.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="204"/>
        <source>&amp;Decorations</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsCopyrightLabelPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="13"/>
        <source>Copyright Label Plugin</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="36"/>
        <source>Color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="79"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:12pt;&quot;&gt;Description&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Enter your copyright label below. This plugin supports basic html markup tags for formatting the label. For example:&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-weight:600;&quot;&gt;&amp;lt;B&amp;gt; Bold text &amp;lt;/B&amp;gt; &lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-weight:600;&quot;&gt;&lt;span style=&quot; font-weight:400; font-style:italic;&quot;&gt;&amp;lt;I&amp;gt; Italics &amp;lt;/I&amp;gt;&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-style:italic;&quot;&gt;&lt;span style=&quot; font-style:normal;&quot;&gt;(note: &amp;amp;copy; gives a copyright symbol)&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="118"/>
        <source>Enable Copyright Label</source>
        <translation type="unfinished"></translation>
    </message>
    <message encoding="UTF-8">
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="130"/>
        <source>© QGIS 2008</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="145"/>
        <source>Placement</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="153"/>
        <source>Bottom Left</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="158"/>
        <source>Top Left</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="163"/>
        <source>Bottom Right</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="168"/>
        <source>Top Right</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="176"/>
        <source>Orientation</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="184"/>
        <source>Horizontal</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="189"/>
        <source>Vertical</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsCustomProjectionDialog</name>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="165"/>
        <source>Delete Projection Definition?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="166"/>
        <source>Deleting a projection definition is not reversable. Do you want to delete it?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="876"/>
        <source>Abort</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="878"/>
        <source>New</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="933"/>
        <source>QGIS Custom Projection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="750"/>
        <source>This proj4 projection definition is not valid. Please give the projection a name before pressing save.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="756"/>
        <source>This proj4 projection definition is not valid. Please add the parameters before pressing save.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="771"/>
        <source>This proj4 projection definition is not valid. Please add a proj= clause before pressing save.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="778"/>
        <source>This proj4 ellipsoid definition is not valid. Please add a ellips= clause before pressing save.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="794"/>
        <source>This proj4 projection definition is not valid. Please correct before pressing save.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="907"/>
        <source>This proj4 projection definition is not valid.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="922"/>
        <source>Northing and Easthing must be in decimal form.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="934"/>
        <source>Internal Error (source projection invalid?)</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsCustomProjectionDialogBase</name>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="62"/>
        <source>Custom Projection Definition</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="32"/>
        <source>Define</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="44"/>
        <source>You can define your own custom projection here. The definition must conform to the proj4 format for specifying a Spatial Reference System.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="203"/>
        <source>Parameters:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="87"/>
        <source>|&lt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="94"/>
        <source>&lt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="101"/>
        <source>1 of 1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="111"/>
        <source>&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="118"/>
        <source>&gt;|</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="125"/>
        <source>New</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="132"/>
        <source>Save</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="139"/>
        <source>Delete</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="146"/>
        <source>Close</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="155"/>
        <source>Name:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="166"/>
        <source>Test</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="189"/>
        <source>Transform from WGS84 to the chosen projection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="196"/>
        <source>Use the text boxes below to test the projection definition you are creating. Enter a coordinate where both the lat/long and the projected result are known (for example by reading off a map). Then press the calculate button to see if the projection definition you are creating is accurate.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="232"/>
        <source>Projected Coordinate System</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="239"/>
        <source>Geographic / WGS84</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="265"/>
        <source>East:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="275"/>
        <source>North:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="305"/>
        <source>Calculate</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsDbSourceSelect</name>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="123"/>
        <source>Wildcard</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="127"/>
        <source>RegExp</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="135"/>
        <source>All</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="139"/>
        <source>Schema</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="143"/>
        <source>Table</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="147"/>
        <source>Type</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="151"/>
        <source>Geometry column</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="155"/>
        <source>Sql</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="229"/>
        <source>Are you sure you want to remove the </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="229"/>
        <source> connection and all associated settings?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="230"/>
        <source>Confirm Delete</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="335"/>
        <source>Select Table</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="335"/>
        <source>You must select a table in order to add a Layer.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="368"/>
        <source>Password for </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="369"/>
        <source>Please enter your password:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="434"/>
        <source>Connection failed</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="437"/>
        <source>Connection to %1 on %2 failed. Either the database is down or your settings are incorrect.%3Check your username and password and try again.%4The database said:%5%6</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsDbSourceSelectBase</name>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="13"/>
        <source>Add PostGIS Table(s)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="54"/>
        <source>PostgreSQL Connections</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="66"/>
        <source>Delete</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="73"/>
        <source>Edit</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="80"/>
        <source>New</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="87"/>
        <source>Connect</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="111"/>
        <source>Help</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="114"/>
        <source>F1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="140"/>
        <source>Add</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="156"/>
        <source>Close</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="183"/>
        <source>Search:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="190"/>
        <source>Search mode:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="200"/>
        <source>Search in columns:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="216"/>
        <source>Search options...</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsDbTableModel</name>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="24"/>
        <source>Schema</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="25"/>
        <source>Table</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="26"/>
        <source>Type</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="27"/>
        <source>Geometry column</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="28"/>
        <source>Sql</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="232"/>
        <source>Point</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="236"/>
        <source>Multipoint</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="240"/>
        <source>Line</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="244"/>
        <source>Multiline</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="248"/>
        <source>Polygon</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="252"/>
        <source>Multipolygon</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsDelAttrDialogBase</name>
    <message>
        <location filename="../src/ui/qgsdelattrdialogbase.ui" line="16"/>
        <source>Delete Attributes</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdelattrdialogbase.ui" line="52"/>
        <source>OK</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdelattrdialogbase.ui" line="59"/>
        <source>Cancel</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsDelimitedTextPlugin</name>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="56"/>
        <source>DelimitedTextLayer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="101"/>
        <source>&amp;Add Delimited Text Layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="104"/>
        <source>Add a delimited text file as a map layer. </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="105"/>
        <source>The file must have a header row containing the field names. </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="105"/>
        <source>X and Y fields are required and must contain coordinates in decimal units.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="142"/>
        <source>&amp;Delimited text</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsDelimitedTextPluginGui</name>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="35"/>
        <source>Parse</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="60"/>
        <source>Description</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="63"/>
        <source>Select a delimited text file containing a header row and one or more rows of x and y coordinates that you would like to use as a point layer and this plugin will do the job for you!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="67"/>
        <source>Use the layer name box to specify the legend name for the new layer. Use the delimiter box to specify what delimeter is used in your file (e.g. space, comma, tab or a regular expression in Perl style). After choosing a delimiter, press the parse button and select the columns containing the x and y values for the layer.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="125"/>
        <source>No layer name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="125"/>
        <source>Please enter a layer name before adding the layer to the map</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="211"/>
        <source>No delimiter</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="211"/>
        <source>Please specify a delimiter prior to parsing the file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="245"/>
        <source>Choose a delimited text file to open</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsDelimitedTextPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="13"/>
        <source>Create a Layer from a Delimited Text File</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="39"/>
        <source>Delimited Text Layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="67"/>
        <source>&lt;p align=&quot;right&quot;&gt;X field&lt;/p&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="88"/>
        <source>Name of the field containing x values</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="91"/>
        <source>Name of the field containing x values. Choose a field from the list. The list is generated by parsing the header row of the delimited text file.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="101"/>
        <source>&lt;p align=&quot;right&quot;&gt;Y field&lt;/p&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="122"/>
        <source>Name of the field containing y values</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="125"/>
        <source>Name of the field containing y values. Choose a field from the list. The list is generated by parsing the header row of the delimited text file.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="145"/>
        <source>Delimited text file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="152"/>
        <source>Full path to the delimited text file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="155"/>
        <source>Full path to the delimited text file. In order to properly parse the fields in the file, the delimiter must be defined prior to entering the file name. Use the Browse button to the right of this field to choose the input file.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="168"/>
        <source>Browse to find the delimited text file to be processed</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="171"/>
        <source>Use this button to browse to the location of the delimited text file. This button will not be enabled until a delimiter has been entered in the &lt;i&gt;Delimiter&lt;/i&gt; box. Once a file is chosen, the X and Y field drop-down boxes will be populated with the fields from the delimited text file.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="174"/>
        <source>Browse...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="191"/>
        <source>Layer name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="198"/>
        <source>Name to display in the map legend</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="201"/>
        <source>Name displayed in the map legend</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="246"/>
        <source>Sample text</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="297"/>
        <source>Delimiter</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="318"/>
        <source>Delimiter to use when splitting fields in the text file. The delimiter can be more than one character.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="321"/>
        <source>Delimiter to use when splitting fields in the delimited text file. The delimiter can be 1 or more characters in length.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="331"/>
        <source>The delimiter is taken as is</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="334"/>
        <source>Plain characters</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="344"/>
        <source>The delimiter is a regular expression</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="347"/>
        <source>Regular expression</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsDelimitedTextProvider</name>
    <message>
        <location filename="../src/providers/delimitedtext/qgsdelimitedtextprovider.cpp" line="400"/>
        <source>Error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/delimitedtext/qgsdelimitedtextprovider.cpp" line="402"/>
        <source>Note: the following lines were not loaded because Qgis was unable to determine values for the x and y coordinates:
</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsDlgPgBufferBase</name>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="13"/>
        <source>Buffer features</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="28"/>
        <source>Parameters</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="59"/>
        <source>Geometry column:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="66"/>
        <source>Add the buffered layer to the map?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="92"/>
        <source>Spatial reference ID:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="118"/>
        <source>Schema:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="125"/>
        <source>Unique field to use as feature id:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="132"/>
        <source>Table name for the buffered layer:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="172"/>
        <source>Create unique object id</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="216"/>
        <source>public</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="224"/>
        <source>Buffer distance in map units:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="234"/>
        <source>&lt;h2&gt;Buffer the features in layer: &lt;/h2&gt;</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsEditReservedWordsBase</name>
    <message>
        <location filename="../src/plugins/spit/qgseditreservedwordsbase.ui" line="13"/>
        <source>Edit Reserved Words</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgseditreservedwordsbase.ui" line="37"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Double click the Column Name column to change the name of the column.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgseditreservedwordsbase.ui" line="47"/>
        <source>Status</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgseditreservedwordsbase.ui" line="52"/>
        <source>Column Name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgseditreservedwordsbase.ui" line="57"/>
        <source>Index</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgseditreservedwordsbase.ui" line="82"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;This shapefile contains reserved words. These may affect the import into PostgreSQL. Edit the column names so none of the reserved words listed at the right are used (click on a Column Name entry to edit). You may also change any other column name if desired.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgseditreservedwordsbase.ui" line="89"/>
        <source>Reserved Words</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsEditReservedWordsDialog</name>
    <message>
        <location filename="../src/plugins/spit/qgseditreservedwordsdialog.cpp" line="34"/>
        <source>Status</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgseditreservedwordsdialog.cpp" line="34"/>
        <source>Column Name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgseditreservedwordsdialog.cpp" line="34"/>
        <source>Index</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsEncodingFileDialog</name>
    <message>
        <location filename="../src/gui/qgsencodingfiledialog.cpp" line="29"/>
        <source>Encoding:</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsFillStyleWidgetBase</name>
    <message>
        <location filename="../src/ui/qgsfillstylewidgetbase.ui" line="16"/>
        <source>Form1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsfillstylewidgetbase.ui" line="28"/>
        <source>Fill Style</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsfillstylewidgetbase.ui" line="54"/>
        <source>col</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsfillstylewidgetbase.ui" line="61"/>
        <source>Colour:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsfillstylewidgetbase.ui" line="72"/>
        <source>PolyStyleWidget</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGPSDeviceDialog</name>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialog.cpp" line="43"/>
        <source>New device %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialog.cpp" line="56"/>
        <source>Are you sure?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialog.cpp" line="57"/>
        <source>Are you sure that you want to delete this device?</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGPSDeviceDialogBase</name>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="24"/>
        <source>GPS Device Editor</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="72"/>
        <source>New device</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="87"/>
        <source>Delete device</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="102"/>
        <source>Update device</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="135"/>
        <source>Device name:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="150"/>
        <source>This is the name of the device as it will appear in the lists</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="159"/>
        <source>Commands</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="174"/>
        <source>Track download:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="181"/>
        <source>Route upload:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="188"/>
        <source>Waypoint download:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="195"/>
        <source>The command that is used to download routes from the device</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="202"/>
        <source>Route download:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="209"/>
        <source>The command that is used to upload waypoints to the device</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="216"/>
        <source>Track upload:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="223"/>
        <source>The command that is used to download tracks from the device</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="230"/>
        <source>The command that is used to upload routes to the device</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="237"/>
        <source>The command that is used to download waypoints from the device</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="244"/>
        <source>The command that is used to upload tracks to the device</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="251"/>
        <source>Waypoint upload:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="269"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;In the download and upload commands there can be special words that will be replaced by QGIS when the commands are used. These words are:&lt;span style=&quot; font-style:italic;&quot;&gt;%babel&lt;/span&gt; - the path to GPSBabel&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%in&lt;/span&gt; - the GPX filename when uploading or the port when downloading&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%out&lt;/span&gt; - the port when uploading or the GPX filename when downloading&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="301"/>
        <source>Close</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGPSPlugin</name>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="92"/>
        <source>&amp;Gps Tools</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="93"/>
        <source>&amp;Create new GPX layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="96"/>
        <source>Creates a new GPX layer and displays it on the map canvas</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="195"/>
        <source>&amp;Gps</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="159"/>
        <source>Save new GPX file as...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="159"/>
        <source>GPS eXchange file (*.gpx)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="164"/>
        <source>Could not create file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="166"/>
        <source>Unable to create a GPX file with the given name. </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="167"/>
        <source>Try again with another name or in another </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="167"/>
        <source>directory.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="206"/>
        <source>GPX Loader</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="208"/>
        <source>Unable to read the selected file.
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="208"/>
        <source>Please reselect a valid file.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="491"/>
        <source>Could not start process</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="492"/>
        <source>Could not start GPSBabel!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="330"/>
        <source>Importing data...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="497"/>
        <source>Cancel</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="275"/>
        <source>Could not import data from %1!

</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="277"/>
        <source>Error importing data</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="345"/>
        <source>Could not convert data from %1!

</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="347"/>
        <source>Error converting data</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="481"/>
        <source>Not supported</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="397"/>
        <source>This device does not support downloading </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="397"/>
        <source>of </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="412"/>
        <source>Downloading data...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="426"/>
        <source>Could not download data from GPS!

</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="428"/>
        <source>Error downloading data</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="482"/>
        <source>This device does not support uploading of </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="497"/>
        <source>Uploading data...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="511"/>
        <source>Error while uploading data to GPS!

</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="513"/>
        <source>Error uploading data</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGPSPluginGui</name>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="480"/>
        <source>Waypoints</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="480"/>
        <source>Routes</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="266"/>
        <source>Tracks</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="516"/>
        <source>Choose a filename to save under</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="518"/>
        <source>GPS eXchange format (*.gpx)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="505"/>
        <source>Select GPX file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="244"/>
        <source>Select file and format to import</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="416"/>
        <source>GPX is the %1, which is used to store information about waypoints, routes, and tracks.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="416"/>
        <source>GPS eXchange file format</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="417"/>
        <source>Select a GPX file and then select the feature types that you want to load.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="429"/>
        <source>This tool will help you download data from a GPS device.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="430"/>
        <source>Choose your GPS device, the port it is connected to, the feature type you want to download, a name for your new layer, and the GPX file where you want to store the data.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="449"/>
        <source>If your device isn&apos;t listed, or if you want to change some settings, you can also edit the devices.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="451"/>
        <source>This tool uses the program GPSBabel (%1) to transfer the data.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="486"/>
        <source>This requires that you have GPSBabel installed where QGIS can find it.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="447"/>
        <source>This tool will help you upload data from a GPX layer to a GPS device.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="448"/>
        <source>Choose the layer you want to upload, the device you want to upload it to, and the port your device is connected to.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="466"/>
        <source>QGIS can only load GPX files by itself, but many other formats can be converted to GPX using GPSBabel (%1).</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="468"/>
        <source>Select a GPS file format and the file that you want to import, the feature type that you want to use, a GPX filename that you want to save the converted file as, and a name for the new layer.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="469"/>
        <source>All file formats can not store waypoints, routes, and tracks, so some feature types may be disabled for some file formats.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="485"/>
        <source>QGIS can perform conversions of GPX files, by using GPSBabel (%1) to perform the conversions.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="487"/>
        <source>Select a GPX input file name, the type of conversion you want to perform, a GPX filename that you want to save the converted file as, and a name for the new layer created from the result.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGPSPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="13"/>
        <source>GPS Tools</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="58"/>
        <source>Load GPX file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="91"/>
        <source>File:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="558"/>
        <source>Browse...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="108"/>
        <source>Feature types:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="305"/>
        <source>Waypoints</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="310"/>
        <source>Routes</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="315"/>
        <source>Tracks</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="155"/>
        <source>Import other file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="544"/>
        <source>Layer name:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="551"/>
        <source>Save As...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="568"/>
        <source>GPX output file:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="330"/>
        <source>Feature type:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="244"/>
        <source>(Note: Selecting correct file type in browser dialog important!)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="258"/>
        <source>File to import:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="266"/>
        <source>Download from GPS</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="469"/>
        <source>Port:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="347"/>
        <source>Output file:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="503"/>
        <source>GPS device:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="496"/>
        <source>Edit devices</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="421"/>
        <source>Upload to GPS</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="510"/>
        <source>Data layer:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="519"/>
        <source>GPX Conversions</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="601"/>
        <source>Conversion:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="615"/>
        <source>GPX input file:</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGPXProvider</name>
    <message>
        <location filename="../src/providers/gpx/qgsgpxprovider.cpp" line="68"/>
        <source>Bad URI - you need to specify the feature type.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/gpx/qgsgpxprovider.cpp" line="112"/>
        <source>GPS eXchange file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/gpx/qgsgpxprovider.cpp" line="729"/>
        <source>Digitized in QGIS</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGeomTypeDialog</name>
    <message>
        <location filename="../src/app/qgsgeomtypedialog.cpp" line="31"/>
        <source>Name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsgeomtypedialog.cpp" line="32"/>
        <source>Type</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGeomTypeDialogBase</name>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="13"/>
        <source>New Vector Layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="28"/>
        <source>File Format:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="41"/>
        <source>Remove</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="64"/>
        <source>Attributes:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="74"/>
        <source>Add</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="81"/>
        <source>Type</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="93"/>
        <source>Point</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="100"/>
        <source>Line</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="107"/>
        <source>Polygon</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="118"/>
        <source>Column 1</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGeorefDescriptionDialogBase</name>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefdescriptiondialogbase.ui" line="13"/>
        <source>Description georeferencer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefdescriptiondialogbase.ui" line="44"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:12pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:11pt; font-weight:600;&quot;&gt;Description&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:9pt;&quot;&gt;This plugin can generate world files for rasters. You select points on the raster and give their world coordinates, and the plugin will compute the world file parameters. The more coordinates you can provide the better the result will be.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGeorefPlugin</name>
    <message>
        <location filename="../src/plugins/georeferencer/plugin.cpp" line="119"/>
        <source>&amp;Georeferencer</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGeorefPluginGui</name>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="85"/>
        <source>Choose a raster file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="87"/>
        <source>Raster files (*.*)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="97"/>
        <source>Error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="98"/>
        <source>The selected file is not a valid raster file.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="122"/>
        <source>World file exists</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="124"/>
        <source>&lt;p&gt;The selected file already seems to have a </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="125"/>
        <source>world file! Do you want to replace it with the </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="125"/>
        <source>new world file?&lt;/p&gt;</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGeorefPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/georeferencer/pluginguibase.ui" line="13"/>
        <source>Georeferencer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/pluginguibase.ui" line="28"/>
        <source>Arrange plugin windows</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/pluginguibase.ui" line="43"/>
        <source>...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/pluginguibase.ui" line="62"/>
        <source>Raster file:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/pluginguibase.ui" line="77"/>
        <source>Description...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/pluginguibase.ui" line="100"/>
        <source>Close</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGeorefWarpOptionsDialog</name>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialog.cpp" line="27"/>
        <source>unstable</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGeorefWarpOptionsDialogBase</name>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="13"/>
        <source>Warp options</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="28"/>
        <source>Compression:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="35"/>
        <source>Resampling method:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="46"/>
        <source>Nearest neighbour</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="51"/>
        <source>Linear</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="56"/>
        <source>Cubic</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="64"/>
        <source>Use 0 for transparency when needed</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="74"/>
        <source>OK</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGraduatedSymbolDialog</name>
    <message>
        <location filename="../src/app/qgsgraduatedsymboldialog.cpp" line="327"/>
        <source>Equal Interval</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsgraduatedsymboldialog.cpp" line="304"/>
        <source>Quantiles</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsgraduatedsymboldialog.cpp" line="351"/>
        <source>Empty</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGraduatedSymbolDialogBase</name>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="25"/>
        <source>graduated Symbol</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="73"/>
        <source>Classification Field:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="89"/>
        <source>Mode:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="105"/>
        <source>Number of Classes:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="159"/>
        <source>Delete class</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="166"/>
        <source>Classify</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassAttributes</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="152"/>
        <source>Column</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="153"/>
        <source>Value</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="154"/>
        <source>Type</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="158"/>
        <source>Layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="300"/>
        <source>Warning</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="301"/>
        <source>ERROR</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="303"/>
        <source>OK</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassAttributesBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="48"/>
        <source>GRASS Attributes</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="78"/>
        <source>Tab 1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="112"/>
        <source>result</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="177"/>
        <source>Update database record</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="180"/>
        <source>Update</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="207"/>
        <source>Add new category using settings in GRASS Edit toolbox</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="210"/>
        <source>New</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="237"/>
        <source>Delete selected category</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="240"/>
        <source>Delete</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassBrowser</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="66"/>
        <source>Tools</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="71"/>
        <source>Add selected map to canvas</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="79"/>
        <source>Copy selected map</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="87"/>
        <source>Rename selected map</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="95"/>
        <source>Delete selected map</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="103"/>
        <source>Set current region to selected map</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="111"/>
        <source>Refresh</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="339"/>
        <source>New name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="453"/>
        <source>Warning</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="289"/>
        <source>Cannot copy map </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="411"/>
        <source>&lt;br&gt;command: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="355"/>
        <source>Cannot rename map </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="393"/>
        <source>Delete map &lt;b&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="410"/>
        <source>Cannot delete map </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="454"/>
        <source>Cannot write new region</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassEdit</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1417"/>
        <source>Warning</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="201"/>
        <source>You are not owner of the mapset, cannot open the vector for editing.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="206"/>
        <source>Cannot open vector for update.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="214"/>
        <source>Edit tools</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="217"/>
        <source>New point</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="223"/>
        <source>New line</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="229"/>
        <source>New boundary</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="235"/>
        <source>New centroid</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="241"/>
        <source>Move vertex</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="247"/>
        <source>Add vertex</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="253"/>
        <source>Delete vertex</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="259"/>
        <source>Move element</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="265"/>
        <source>Split line</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="271"/>
        <source>Delete element</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="277"/>
        <source>Edit attributes</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="282"/>
        <source>Close</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="363"/>
        <source>Background</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="364"/>
        <source>Highlight</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="365"/>
        <source>Dynamic</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="366"/>
        <source>Point</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="367"/>
        <source>Line</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="368"/>
        <source>Boundary (no area)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="369"/>
        <source>Boundary (1 area)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="370"/>
        <source>Boundary (2 areas)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="371"/>
        <source>Centroid (in area)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="372"/>
        <source>Centroid (outside area)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="373"/>
        <source>Centroid (duplicate in area)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="374"/>
        <source>Node (1 line)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="375"/>
        <source>Node (2 lines)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="408"/>
        <source>Disp</source>
        <comment>Column title</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="410"/>
        <source>Color</source>
        <comment>Column title</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="412"/>
        <source>Type</source>
        <comment>Column title</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="414"/>
        <source>Index</source>
        <comment>Column title</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="444"/>
        <source>Column</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="445"/>
        <source>Type</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="446"/>
        <source>Length</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="501"/>
        <source>Next not used</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="502"/>
        <source>Manual entry</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="503"/>
        <source>No category</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="665"/>
        <source>Info</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="665"/>
        <source>The table was created</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1290"/>
        <source>Tool not yet implemented.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1316"/>
        <source>Cannot check orphan record: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1323"/>
        <source>Orphan record was left in attribute table. &lt;br&gt;Delete the record?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1332"/>
        <source>Cannot delete orphan record: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1360"/>
        <source>Cannot describe table for field </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1780"/>
        <source>Left: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1781"/>
        <source>Middle: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1782"/>
        <source>Right: </source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassEditBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="16"/>
        <source>GRASS Edit</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="106"/>
        <source>Category</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="66"/>
        <source>Mode</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="410"/>
        <source>Layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="170"/>
        <source>Settings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="190"/>
        <source>Snapping in screen pixels</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="241"/>
        <source>Symbology</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="274"/>
        <source>Column 1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="298"/>
        <source>Line width</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="325"/>
        <source>Marker size</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="360"/>
        <source>Table</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="496"/>
        <source>Add Column</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="511"/>
        <source>Create / Alter Table</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassElementDialog</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="131"/>
        <source>Cancel</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="162"/>
        <source>Ok</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="167"/>
        <source>&lt;font color=&apos;red&apos;&gt;Enter a name!&lt;/font&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="178"/>
        <source>&lt;font color=&apos;red&apos;&gt;This is name of the source!&lt;/font&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="184"/>
        <source>&lt;font color=&apos;red&apos;&gt;Exists!&lt;/font&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="185"/>
        <source>Overwrite</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassMapcalc</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="110"/>
        <source>Mapcalc tools</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="113"/>
        <source>Add map</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="120"/>
        <source>Add constant value</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="127"/>
        <source>Add operator or function</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="134"/>
        <source>Add connection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="141"/>
        <source>Select item</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="148"/>
        <source>Delete selected item</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="158"/>
        <source>Open</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="163"/>
        <source>Save</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="169"/>
        <source>Save as</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="177"/>
        <source>Addition</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="178"/>
        <source>Subtraction</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="179"/>
        <source>Multiplication</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="180"/>
        <source>Division</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="181"/>
        <source>Modulus</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="182"/>
        <source>Exponentiation</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="185"/>
        <source>Equal</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="186"/>
        <source>Not equal</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="187"/>
        <source>Greater than</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="188"/>
        <source>Greater than or equal</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="189"/>
        <source>Less than</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="190"/>
        <source>Less than or equal</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="191"/>
        <source>And</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="192"/>
        <source>Or</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="195"/>
        <source>Absolute value of x</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="196"/>
        <source>Inverse tangent of x (result is in degrees)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="197"/>
        <source>Inverse tangent of y/x (result is in degrees)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="198"/>
        <source>Current column of moving window (starts with 1)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="199"/>
        <source>Cosine of x (x is in degrees)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="200"/>
        <source>Convert x to double-precision floating point</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="201"/>
        <source>Current east-west resolution</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="202"/>
        <source>Exponential function of x</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="203"/>
        <source>x to the power y</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="204"/>
        <source>Convert x to single-precision floating point</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="205"/>
        <source>Decision: 1 if x not zero, 0 otherwise</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="206"/>
        <source>Decision: a if x not zero, 0 otherwise</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="207"/>
        <source>Decision: a if x not zero, b otherwise</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="208"/>
        <source>Decision: a if x &gt; 0, b if x is zero, c if x &lt; 0</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="209"/>
        <source>Convert x to integer [ truncates ]</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="210"/>
        <source>Check if x = NULL</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="211"/>
        <source>Natural log of x</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="212"/>
        <source>Log of x base b</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="214"/>
        <source>Largest value</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="216"/>
        <source>Median value</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="218"/>
        <source>Smallest value</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="220"/>
        <source>Mode value</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="221"/>
        <source>1 if x is zero, 0 otherwise</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="222"/>
        <source>Current north-south resolution</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="223"/>
        <source>NULL value</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="224"/>
        <source>Random value between a and b</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="225"/>
        <source>Round x to nearest integer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="226"/>
        <source>Current row of moving window (Starts with 1)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="227"/>
        <source>Sine of x (x is in degrees)</source>
        <comment>sin(x)</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="228"/>
        <source>Square root of x</source>
        <comment>sqrt(x)</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="229"/>
        <source>Tangent of x (x is in degrees)</source>
        <comment>tan(x)</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="230"/>
        <source>Current x-coordinate of moving window</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="231"/>
        <source>Current y-coordinate of moving window</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1388"/>
        <source>Output</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1317"/>
        <source>Warning</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="583"/>
        <source>Cannot get current region</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="560"/>
        <source>Cannot check region of map </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="616"/>
        <source>Cannot get region of map </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="812"/>
        <source>No GRASS raster maps currently in QGIS</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1102"/>
        <source>Cannot create &apos;mapcalc&apos; directory in current mapset.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1112"/>
        <source>New mapcalc</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1113"/>
        <source>Enter new mapcalc name:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1118"/>
        <source>Enter vector name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1126"/>
        <source>The file already exists. Overwrite? </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1164"/>
        <source>Save mapcalc</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1146"/>
        <source>File name empty</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1165"/>
        <source>Cannot open mapcalc file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1295"/>
        <source>The mapcalc schema (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1295"/>
        <source>) not found.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1302"/>
        <source>Cannot open mapcalc schema (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1313"/>
        <source>Cannot read mapcalc schema (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1314"/>
        <source>
at line </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1315"/>
        <source> column </source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassMapcalcBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalcbase.ui" line="16"/>
        <source>MainWindow</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalcbase.ui" line="37"/>
        <source>Output</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassModule</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="192"/>
        <source>Module</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1339"/>
        <source>Warning</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="207"/>
        <source>The module file (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="207"/>
        <source>) not found.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="211"/>
        <source>Cannot open module file (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="979"/>
        <source>)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="974"/>
        <source>Cannot read module file (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="974"/>
        <source>):
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="975"/>
        <source>
at line </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="247"/>
        <source>Module </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="247"/>
        <source> not found</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="288"/>
        <source>Cannot find man page </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="965"/>
        <source>Not available, description not found (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="968"/>
        <source>Not available, cannot open description (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="975"/>
        <source> column </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="979"/>
        <source>Not available, incorrect description (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1367"/>
        <source>Run</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1166"/>
        <source>Cannot get input region</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1154"/>
        <source>Use Input Region</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1268"/>
        <source>Cannot find module </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1340"/>
        <source>Cannot start module: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1345"/>
        <source>Stop</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1356"/>
        <source>&lt;B&gt;Successfully finished&lt;/B&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1362"/>
        <source>&lt;B&gt;Finished with error&lt;/B&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1365"/>
        <source>&lt;B&gt;Module crashed or killed&lt;/B&gt;</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="13"/>
        <source>GRASS Module</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="26"/>
        <source>Options</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="31"/>
        <source>Output</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="47"/>
        <source>Manual</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="74"/>
        <source>TextLabel</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="118"/>
        <source>Run</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="141"/>
        <source>View output</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="161"/>
        <source>Close</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleField</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2701"/>
        <source>Attribute field</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleFile</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2903"/>
        <source>File</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="3016"/>
        <source>:&amp;nbsp;missing value</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="3023"/>
        <source>:&amp;nbsp;directory does not exist</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleGdalInput</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2652"/>
        <source>Warning</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2514"/>
        <source>Cannot find layeroption </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2527"/>
        <source>Cannot find whereoption </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2656"/>
        <source>PostGIS driver in OGR does not support schemas!&lt;br&gt;Only the table name will be used.&lt;br&gt;It can result in wrong input if more tables of the same name&lt;br&gt;are present in the database.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2679"/>
        <source>:&amp;nbsp;no input</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleInput</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2058"/>
        <source>Warning</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1971"/>
        <source>Cannot find typeoption </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1980"/>
        <source>Cannot find values for typeoption </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2041"/>
        <source>Cannot find layeroption </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2058"/>
        <source>GRASS element </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2058"/>
        <source> not supported</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2082"/>
        <source>Use region of this map</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2418"/>
        <source>:&amp;nbsp;no input</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleOption</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1887"/>
        <source>:&amp;nbsp;missing value</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleSelection</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2790"/>
        <source>Attribute field</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleStandardOptions</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="873"/>
        <source>Warning</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="356"/>
        <source>Cannot find module </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="373"/>
        <source>Cannot start module </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="386"/>
        <source>Cannot read module description (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="386"/>
        <source>):
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="387"/>
        <source>
at line </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="387"/>
        <source> column </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="411"/>
        <source>Cannot find key </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="547"/>
        <source>Item with id </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="547"/>
        <source> not found</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="835"/>
        <source>Cannot get current region</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="812"/>
        <source>Cannot check region of map </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="874"/>
        <source>Cannot set region of map </source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassNewMapset</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="114"/>
        <source>GRASS database</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="115"/>
        <source>GRASS location</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="116"/>
        <source>Projection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="117"/>
        <source>Default GRASS Region</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="174"/>
        <source>Mapset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="119"/>
        <source>Create New Mapset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="148"/>
        <source>Tree</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="149"/>
        <source>Comment</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="150"/>
        <source>Database</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="154"/>
        <source>Location 2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="165"/>
        <source>User&apos;s mapset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="167"/>
        <source>System mapset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="161"/>
        <source>Location 1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="175"/>
        <source>Owner</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="224"/>
        <source>Enter path to GRASS database</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="232"/>
        <source>The directory doesn&apos;t exist!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="262"/>
        <source>No writable locations, the database not writable!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="360"/>
        <source>Enter location name!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="373"/>
        <source>The location exists!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="521"/>
        <source>Selected projection is not supported by GRASS!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1148"/>
        <source>Warning</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="568"/>
        <source>Cannot create projection.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="617"/>
        <source>Cannot reproject previously set region, default region set.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="752"/>
        <source>North must be greater than south</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="757"/>
        <source>East must be greater than west</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="804"/>
        <source>Regions file (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="804"/>
        <source>) not found.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="809"/>
        <source>Cannot open locations file (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="809"/>
        <source>)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="818"/>
        <source>Cannot read locations file (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="819"/>
        <source>):
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="819"/>
        <source>
at line </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="820"/>
        <source> column </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1149"/>
        <source>Cannot create QgsSpatialRefSys</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="956"/>
        <source>Cannot reproject selected region.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1045"/>
        <source>Cannot reproject region</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1277"/>
        <source>Enter mapset name.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1294"/>
        <source>The mapset already exists</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1318"/>
        <source>Database: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1329"/>
        <source>Location: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1331"/>
        <source>Mapset: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1362"/>
        <source>Create location</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1364"/>
        <source>Cannot create new location: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1411"/>
        <source>Create mapset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1394"/>
        <source>Cannot create new mapset directory</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1404"/>
        <source>Cannot open DEFAULT_WIND</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1411"/>
        <source>Cannot open WIND</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1438"/>
        <source>New mapset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1434"/>
        <source>New mapset successfully created, but cannot be opened: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1440"/>
        <source>New mapset successfully created and set as current working mapset.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassNewMapsetBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2068"/>
        <source>Column 1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="88"/>
        <source>Example directory tree:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="95"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;GRASS data are stored in tree directory structure.&lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;The GRASS database is the top-level directory in this tree structure.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="399"/>
        <source>Database Error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2153"/>
        <source>Database:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="440"/>
        <source>...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="457"/>
        <source>Select existing directory or create a new one:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="508"/>
        <source>Location</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="535"/>
        <source>Select location</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="552"/>
        <source>Create new location</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="832"/>
        <source>Location Error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="848"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;The GRASS location is a collection of maps for a particular territory or project.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1159"/>
        <source>Projection Error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1174"/>
        <source>Coordinate system</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1186"/>
        <source>Projection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1193"/>
        <source>Not defined</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1273"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;The GRASS region defines a workspace for raster modules. The default region is valid for one location. It is possible to set a different region in each mapset. &lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;It is possible to change the default location region later.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1334"/>
        <source>Set current QGIS extent</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1376"/>
        <source>Set</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1396"/>
        <source>Region Error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1441"/>
        <source>S</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1500"/>
        <source>W</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1555"/>
        <source>E</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1614"/>
        <source>N</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1699"/>
        <source>New mapset:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1988"/>
        <source>Mapset Error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2045"/>
        <source>&lt;p align=&quot;center&quot;&gt;Existing masets&lt;/p&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2101"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;The GRASS mapset is a collection of maps used by one user. &lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;A user can read maps from all mapsets in the location but &lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;he can open for writing only his mapset (owned by user).&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2174"/>
        <source>Location:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2195"/>
        <source>Mapset:</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassPlugin</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="82"/>
        <source>GrassVector</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="83"/>
        <source>0.1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="84"/>
        <source>GRASS layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="147"/>
        <source>Open mapset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="148"/>
        <source>New mapset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="149"/>
        <source>Close mapset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="152"/>
        <source>Add GRASS vector layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="154"/>
        <source>Add GRASS raster layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="170"/>
        <source>Open GRASS tools</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="159"/>
        <source>Display Current Grass Region</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="163"/>
        <source>Edit Current Grass Region</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="165"/>
        <source>Edit Grass Vector layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="166"/>
        <source>Create new Grass Vector</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="168"/>
        <source>Adds a GRASS vector layer to the map canvas</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="169"/>
        <source>Adds a GRASS raster layer to the map canvas</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="171"/>
        <source>Displays the current GRASS region as a rectangle on the map canvas</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="172"/>
        <source>Edit the current GRASS region</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="173"/>
        <source>Edit the currently selected GRASS vector layer.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="791"/>
        <source>&amp;GRASS</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="200"/>
        <source>GRASS</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="760"/>
        <source>Warning</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="470"/>
        <source>GRASS Edit is already running.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="479"/>
        <source>New vector name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="495"/>
        <source>Cannot create new vector: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="515"/>
        <source>New vector created but cannot be opened by data provider.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="526"/>
        <source>Cannot start editing.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="563"/>
        <source>GISDBASE, LOCATION_NAME or MAPSET is not set, cannot display current region.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="573"/>
        <source>Cannot read current region: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="677"/>
        <source>Cannot open the mapset. </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="695"/>
        <source>Cannot close mapset. </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="751"/>
        <source>Cannot close current mapset. </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="760"/>
        <source>Cannot open GRASS mapset. </source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassRegion</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregion.cpp" line="460"/>
        <source>Warning</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregion.cpp" line="196"/>
        <source>GISDBASE, LOCATION_NAME or MAPSET is not set, cannot display current region.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregion.cpp" line="203"/>
        <source>Cannot read current region: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregion.cpp" line="460"/>
        <source>Cannot write region</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassRegionBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="13"/>
        <source>GRASS Region Settings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="76"/>
        <source>N</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="146"/>
        <source>W</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="172"/>
        <source>E</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="236"/>
        <source>S</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="280"/>
        <source>N-S Res</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="293"/>
        <source>Rows</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="303"/>
        <source>Cols</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="316"/>
        <source>E-W Res</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="364"/>
        <source>Color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="384"/>
        <source>Width</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="464"/>
        <source>OK</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="487"/>
        <source>Cancel</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassSelect</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="68"/>
        <source>Select GRASS Vector Layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="75"/>
        <source>Select GRASS Raster Layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="82"/>
        <source>Select GRASS mapcalc schema</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="90"/>
        <source>Select GRASS Mapset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="408"/>
        <source>Warning</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="408"/>
        <source>Cannot open vector on level 2 (topology not available).</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="466"/>
        <source>Choose existing GISDBASE</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="482"/>
        <source>Wrong GISDBASE, no locations available.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="483"/>
        <source>Wrong GISDBASE</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="500"/>
        <source>Select a map.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="501"/>
        <source>No map</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="509"/>
        <source>No layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="510"/>
        <source>No layers available in this map</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassSelectBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="21"/>
        <source>Add GRASS Layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="65"/>
        <source>Gisdbase</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="78"/>
        <source>Location</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="85"/>
        <source>Mapset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="102"/>
        <source>Select or type map name (wildcards &apos;*&apos; and &apos;?&apos; accepted for rasters)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="118"/>
        <source>Map name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="125"/>
        <source>Layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="161"/>
        <source>Browse</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="168"/>
        <source>Cancel</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="175"/>
        <source>OK</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassShellBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassshellbase.ui" line="19"/>
        <source>GRASS Shell</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassshellbase.ui" line="49"/>
        <source>Close</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassTools</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="99"/>
        <source>GRASS Tools</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="117"/>
        <source>Modules</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="376"/>
        <source>GRASS Tools: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="149"/>
        <source>Browser</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="298"/>
        <source>Warning</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="211"/>
        <source>Cannot find MSYS (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="233"/>
        <source>GRASS Shell is not compiled.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="283"/>
        <source>The config file (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="283"/>
        <source>) not found.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="287"/>
        <source>Cannot open config file (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="287"/>
        <source>)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="295"/>
        <source>Cannot read config file (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="296"/>
        <source>
at line </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="296"/>
        <source> column </source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGridMakerPlugin</name>
    <message>
        <location filename="../src/plugins/grid_maker/plugin.cpp" line="93"/>
        <source>&amp;Graticule Creator</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugin.cpp" line="94"/>
        <source>Creates a graticule (grid) and stores the result as a shapefile</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugin.cpp" line="136"/>
        <source>&amp;Graticules</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGridMakerPluginGui</name>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="101"/>
        <source>QGIS - Grid Maker</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="52"/>
        <source>Please enter the file name before pressing OK!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="62"/>
        <source>Longitude Interval is invalid - please correct and try again.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="70"/>
        <source>Latitude Interval is invalid - please correct and try again.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="78"/>
        <source>Longitude Origin is invalid - please correct and try again..</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="86"/>
        <source>Latitude Origin is invalid - please correct and try again.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="94"/>
        <source>End Point Longitude is invalid - please correct and try again.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="102"/>
        <source>End Point Latitude is invalid - please correct and try again.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="162"/>
        <source>Choose a filename to save under</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="164"/>
        <source>ESRI Shapefile (*.shp)</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGridMakerPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="13"/>
        <source>QGIS Plugin Template</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="39"/>
        <source>Graticule Builder</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="94"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:Arial; font-size:11pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;This plugin will help you to build a graticule shapefile that you can use as an overlay within your qgis map viewer.&lt;/span&gt;&lt;/p&gt;&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:Arial; font-size:10pt;&quot;&gt;Please enter all units in decimal degrees&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="101"/>
        <source>Type</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="113"/>
        <source>Point</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="123"/>
        <source>Line</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="130"/>
        <source>Polygon</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="140"/>
        <source>Origin (lower left)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="195"/>
        <source>Latitude:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="259"/>
        <source>#000.00000; </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="209"/>
        <source>Longitude:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="183"/>
        <source>End point (upper right)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="226"/>
        <source>Graticle size (units in degrees)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="238"/>
        <source>Latitude Interval:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="252"/>
        <source>Longitude Interval:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="269"/>
        <source>Output (shape) file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="284"/>
        <source>Save As...</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsHelpViewer</name>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="139"/>
        <source>This help file does not exist for your language</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="142"/>
        <source>If you would like to create it, contact the QGIS development team</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="157"/>
        <source>Quantum GIS Help</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="185"/>
        <source>Quantum GIS Help - </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="214"/>
        <source>Error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="191"/>
        <source>Failed to get the help text from the database</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="215"/>
        <source>The QGIS help database is not installed</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsHelpViewerBase</name>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="16"/>
        <source>QGIS Help</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="81"/>
        <source>&amp;Close</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="84"/>
        <source>Alt+C</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="42"/>
        <source>&amp;Home</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="45"/>
        <source>Alt+H</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="55"/>
        <source>&amp;Forward</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="58"/>
        <source>Alt+F</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="68"/>
        <source>&amp;Back</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="71"/>
        <source>Alt+B</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsHttpTransaction</name>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="225"/>
        <source>WMS Server responded unexpectedly with HTTP Status Code %1 (%2)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="304"/>
        <source>HTTP response completed, however there was an error: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="353"/>
        <source>HTTP transaction completed, however there was an error: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message numerus="yes">
        <location filename="../src/core/qgshttptransaction.cpp" line="432"/>
        <source>Network timed out after %1 seconds of inactivity.
This may be a problem in your network connection or at the WMS server.</source>
        <translation type="unfinished">
            <numerusform></numerusform>
        </translation>
    </message>
</context>
<context>
    <name>QgsIdentifyResults</name>
    <message>
        <location filename="../src/app/qgsidentifyresults.cpp" line="44"/>
        <source>Feature</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsidentifyresults.cpp" line="45"/>
        <source>Value</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsidentifyresults.cpp" line="106"/>
        <source>Run action</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsidentifyresults.cpp" line="196"/>
        <source>(Derived)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsidentifyresults.cpp" line="225"/>
        <source>Identify Results - </source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsIdentifyResultsBase</name>
    <message>
        <location filename="../src/ui/qgsidentifyresultsbase.ui" line="16"/>
        <source>Identify Results</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsidentifyresultsbase.ui" line="46"/>
        <source>Help</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsidentifyresultsbase.ui" line="49"/>
        <source>F1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsidentifyresultsbase.ui" line="75"/>
        <source>Close</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsLUDialogBase</name>
    <message>
        <location filename="../src/ui/qgsludialogbase.ui" line="13"/>
        <source>Enter class bounds</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsludialogbase.ui" line="31"/>
        <source>Lower value</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsludialogbase.ui" line="57"/>
        <source>-</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsludialogbase.ui" line="94"/>
        <source>OK</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsludialogbase.ui" line="101"/>
        <source>Cancel</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsludialogbase.ui" line="126"/>
        <source>Upper value</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsLabelDialogBase</name>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="21"/>
        <source>Form1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="41"/>
        <source>Preview:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="56"/>
        <source>QGIS Rocks!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="78"/>
        <source>Font Style</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="103"/>
        <source>Font size units</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="579"/>
        <source>Map units</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="586"/>
        <source>Points</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="980"/>
        <source>Transparency:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="183"/>
        <source>Font</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="534"/>
        <source>Colour</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="478"/>
        <source>%</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="236"/>
        <source>Font Alignment</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="261"/>
        <source>Placement</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="273"/>
        <source>Below Right</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="280"/>
        <source>Right</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="287"/>
        <source>Below</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="294"/>
        <source>Over</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="304"/>
        <source>Above</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="311"/>
        <source>Left</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="318"/>
        <source>Below Left</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="325"/>
        <source>Above Right</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="332"/>
        <source>Above Left</source>
        <translation type="unfinished"></translation>
    </message>
    <message encoding="UTF-8">
        <location filename="../src/ui/qgslabeldialogbase.ui" line="342"/>
        <source>°</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="893"/>
        <source>Angle (deg):</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="384"/>
        <source>Buffer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="396"/>
        <source>Buffer size units</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="408"/>
        <source>Size is in map units</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="415"/>
        <source>Size is in points</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="1005"/>
        <source>Size:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="451"/>
        <source>Buffer Labels?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="542"/>
        <source>Position</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="567"/>
        <source>Offset units</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="1080"/>
        <source>X Offset (pts):</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="1116"/>
        <source>Y Offset (pts):</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="625"/>
        <source>Data Defined Style</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="645"/>
        <source>&amp;Italic:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="663"/>
        <source>&amp;Size:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="681"/>
        <source>&amp;Bold:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="719"/>
        <source>&amp;Underline:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="767"/>
        <source>Size Units:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="813"/>
        <source>&amp;Font family:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="847"/>
        <source>Data Defined Alignment</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="914"/>
        <source>Placement:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="922"/>
        <source>Data Defined Buffer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="1016"/>
        <source>Data Defined Position</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="1062"/>
        <source>X Coordinate:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="1101"/>
        <source>Y Coordinate:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="1136"/>
        <source>Source</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="1148"/>
        <source>Field containing label:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="1191"/>
        <source>Default label:</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsLayerProjectionSelector</name>
    <message>
        <location filename="../src/gui/qgslayerprojectionselector.cpp" line="34"/>
        <source>Define this layer&apos;s projection:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/gui/qgslayerprojectionselector.cpp" line="35"/>
        <source>This layer appears to have no projection specification.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/gui/qgslayerprojectionselector.cpp" line="37"/>
        <source>By default, this layer will now have its projection set to that of the project, but you may override this by selecting a different projection below.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsLayerProjectionSelectorBase</name>
    <message>
        <location filename="../src/ui/qgslayerprojectionselectorbase.ui" line="13"/>
        <source>Layer Projection Selector</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslayerprojectionselectorbase.ui" line="80"/>
        <source>OK</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsLegend</name>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="110"/>
        <source>group</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="416"/>
        <source>&amp;Make to toplevel item</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="423"/>
        <source>&amp;Remove</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="428"/>
        <source>Re&amp;name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="433"/>
        <source>&amp;Add group</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="434"/>
        <source>&amp;Expand all</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="435"/>
        <source>&amp;Collapse all</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="437"/>
        <source>Show file groups</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="1818"/>
        <source>No Layer Selected</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="1819"/>
        <source>To open an attribute table, you must select a vector layer in the legend</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsLegendLayer</name>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="474"/>
        <source>&amp;Zoom to layer extent</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="477"/>
        <source>&amp;Zoom to best scale (100%)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="481"/>
        <source>&amp;Show in overview</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="487"/>
        <source>&amp;Remove</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="494"/>
        <source>&amp;Open attribute table</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="518"/>
        <source>Save as shapefile...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="525"/>
        <source>Save selection as shapefile...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="535"/>
        <source>&amp;Properties</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="584"/>
        <source>More layers</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="585"/>
        <source>This item contains more layer files. Displaying more layers in table is not supported.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsLegendLayerFile</name>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="227"/>
        <source>Not a vector layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="228"/>
        <source>To open an attribute table, you must select a vector layer in the legend</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="271"/>
        <source>bad_alloc exception</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="271"/>
        <source>Filling the attribute table has been stopped because there was no more virtual memory left</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="277"/>
        <source>Attribute table - </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="347"/>
        <source>Save layer as...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="394"/>
        <source>Saving done</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="394"/>
        <source>Export to Shapefile has been completed</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="398"/>
        <source>Driver not found</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="398"/>
        <source>ESRI Shapefile driver is not available</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="402"/>
        <source>Error creating shapefile</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="403"/>
        <source>The shapefile could not be created (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="460"/>
        <source>Error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="407"/>
        <source>Layer creation failed</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="411"/>
        <source>Layer attribute table contains unsupported datatype(s)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="427"/>
        <source>Start editing failed</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="428"/>
        <source>Provider cannot be opened for editing</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="441"/>
        <source>Stop editing</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="442"/>
        <source>Do you want to save the changes?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="449"/>
        <source>Could not commit changes</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="461"/>
        <source>Problems during roll back</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="494"/>
        <source>&amp;Zoom to layer extent</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="497"/>
        <source>&amp;Show in overview</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="505"/>
        <source>&amp;Remove</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="514"/>
        <source>&amp;Open attribute table</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="528"/>
        <source>Save as shapefile...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="530"/>
        <source>Save selection as shapefile...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="547"/>
        <source>&amp;Properties</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsLineStyleDialogBase</name>
    <message>
        <location filename="../src/ui/qgslinestyledialogbase.ui" line="13"/>
        <source>Select a line style</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslinestyledialogbase.ui" line="28"/>
        <source>Styles</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslinestyledialogbase.ui" line="177"/>
        <source>Ok</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslinestyledialogbase.ui" line="184"/>
        <source>Cancel</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsLineStyleWidgetBase</name>
    <message>
        <location filename="../src/ui/qgslinestylewidgetbase.ui" line="16"/>
        <source>Form2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslinestylewidgetbase.ui" line="36"/>
        <source>Outline Style</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslinestylewidgetbase.ui" line="61"/>
        <source>Width:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslinestylewidgetbase.ui" line="87"/>
        <source>Colour:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslinestylewidgetbase.ui" line="98"/>
        <source>LineStyleWidget</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslinestylewidgetbase.ui" line="120"/>
        <source>col</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsMapCanvas</name>
    <message>
        <location filename="../src/gui/qgsmapcanvas.cpp" line="1148"/>
        <source>Could not draw</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/gui/qgsmapcanvas.cpp" line="1148"/>
        <source>because</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsMapLayer</name>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="538"/>
        <source> Check file permissions and retry.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsMapToolIdentify</name>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="423"/>
        <source>(clicked coordinate)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="222"/>
        <source>WMS identify result for %1
%2</source>
        <translation type="unfinished"></translation>
    </message>
    <message numerus="yes">
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="490"/>
        <source>- %1 features found</source>
        <comment>Identify results window title</comment>
        <translation type="unfinished">
            <numerusform></numerusform>
        </translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="564"/>
        <source>No features found</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="567"/>
        <source>&lt;p&gt;No features were found within the search radius. Note that it is currently not possible to use the identify tool on unsaved features.&lt;/p&gt;</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsMapToolSplitFeatures</name>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="85"/>
        <source>Split error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="85"/>
        <source>An error occured during feature splitting</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsMapToolVertexEdit</name>
    <message>
        <location filename="../src/app/qgsmaptoolvertexedit.cpp" line="51"/>
        <source>Snap tolerance</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolvertexedit.cpp" line="52"/>
        <source>Don&apos;t show this message again</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolvertexedit.cpp" line="57"/>
        <source>Could not snap segment.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolvertexedit.cpp" line="60"/>
        <source>Have you set the tolerance in Settings &gt; Project Properties &gt; General?</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsMapserverExport</name>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexport.cpp" line="197"/>
        <source>Overwrite File?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmapserverexport.cpp" line="74"/>
        <source> exists. 
Do you want to overwrite it?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexport.cpp" line="76"/>
        <source>Name for the map file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexport.cpp" line="77"/>
        <source>MapServer map files (*.map);;All files (*.*)</source>
        <comment>Filter list for selecting files from a dialog box</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexport.cpp" line="84"/>
        <source>Choose the QGIS project file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexport.cpp" line="85"/>
        <source>QGIS Project Files (*.qgs);;All files (*.*)</source>
        <comment>Filter list for selecting files from a dialog box</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexport.cpp" line="199"/>
        <source> exists. 
Do you want to overwrite it?</source>
        <comment>a filename is prepended to this text, and appears in a dialog box</comment>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsMapserverExportBase</name>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="14"/>
        <source>Export to Mapserver</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="29"/>
        <source>Web Interface Definition</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="58"/>
        <source>Template</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="68"/>
        <source>Path to the MapServer template file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="369"/>
        <source>Browse...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="98"/>
        <source>Header</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="134"/>
        <source>Footer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="159"/>
        <source>Map</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="182"/>
        <source>Units</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="196"/>
        <source>dd</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="201"/>
        <source>feet</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="206"/>
        <source>meters</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="211"/>
        <source>miles</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="216"/>
        <source>inches</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="221"/>
        <source>kilometers</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="229"/>
        <source>Image type</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="243"/>
        <source>gif</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="248"/>
        <source>gtiff</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="253"/>
        <source>jpeg</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="258"/>
        <source>png</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="263"/>
        <source>swf</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="268"/>
        <source>userdefined</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="273"/>
        <source>wbmp</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="291"/>
        <source>Width</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="304"/>
        <source>Height</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="330"/>
        <source>Name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="340"/>
        <source>Prefix attached to map, scalebar and legend GIF filenames created using this MapFile</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="352"/>
        <source>Map file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="362"/>
        <source>Name for the map file to be created from the QGIS project file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="376"/>
        <source>Full path to the QGIS project file to export to MapServer map format</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="383"/>
        <source>QGIS project file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="393"/>
        <source>Save As...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="400"/>
        <source>If checked, only the layer information will be processed</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="403"/>
        <source>Export LAYER information only</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmapserverexportbase.ui" line="56"/>
        <source>&amp;Help</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmapserverexportbase.ui" line="59"/>
        <source>F1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmapserverexportbase.ui" line="85"/>
        <source>&amp;OK</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmapserverexportbase.ui" line="101"/>
        <source>&amp;Cancel</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmapserverexportbase.ui" line="116"/>
        <source>...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmapserverexportbase.ui" line="226"/>
        <source>MinScale</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmapserverexportbase.ui" line="236"/>
        <source>MaxScale</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmapserverexportbase.ui" line="252"/>
        <source>Prefix attached to map, scalebar and legend GIF filenames created using this MapFile. It should be kept short.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsMarkerDialogBase</name>
    <message>
        <location filename="../src/ui/qgsmarkerdialogbase.ui" line="16"/>
        <source>Choose a marker symbol</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmarkerdialogbase.ui" line="28"/>
        <source>Directory</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmarkerdialogbase.ui" line="38"/>
        <source>...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmarkerdialogbase.ui" line="71"/>
        <source>Ok</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmarkerdialogbase.ui" line="81"/>
        <source>Cancel</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmarkerdialogbase.ui" line="108"/>
        <source>New Item</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsMeasureBase</name>
    <message>
        <location filename="../src/ui/qgsmeasurebase.ui" line="22"/>
        <source>Measure</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmeasurebase.ui" line="69"/>
        <source>Total:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmeasurebase.ui" line="122"/>
        <source>Help</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmeasurebase.ui" line="145"/>
        <source>New</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmeasurebase.ui" line="152"/>
        <source>Cl&amp;ose</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsMeasureDialog</name>
    <message>
        <location filename="../src/app/qgsmeasuredialog.cpp" line="206"/>
        <source>Segments (in meters)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmeasuredialog.cpp" line="209"/>
        <source>Segments (in feet)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmeasuredialog.cpp" line="212"/>
        <source>Segments (in degrees)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmeasuredialog.cpp" line="215"/>
        <source>Segments</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsMeasureTool</name>
    <message>
        <location filename="../src/app/qgsmeasuretool.cpp" line="73"/>
        <source>Incorrect measure results</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmeasuretool.cpp" line="81"/>
        <source>&lt;p&gt;This map is defined with a geographic coordinate system (latitude/longitude) but the map extents suggests that it is actually a projected coordinate system (e.g., Mercator). If so, the results from line or area measurements will be incorrect.&lt;/p&gt;&lt;p&gt;To fix this, explicitly set an appropriate map coordinate system using the &lt;tt&gt;Settings:Project Properties&lt;/tt&gt; menu.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsMessageViewer</name>
    <message>
        <location filename="../src/ui/qgsmessageviewer.ui" line="13"/>
        <source>QGIS Message</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmessageviewer.ui" line="28"/>
        <source>Don&apos;t show this message again</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmessageviewer.ui" line="48"/>
        <source>Close</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsMySQLProvider</name>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="166"/>
        <source>Unable to access relation</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="167"/>
        <source>Unable to access the </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="169"/>
        <source> relation.
The error message from the database was:
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="186"/>
        <source>No GEOS Support!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="189"/>
        <source>Your PostGIS installation has no GEOS support.
Feature selection and identification will not work properly.
Please install PostGIS with GEOS support (http://geos.refractions.net)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="813"/>
        <source>Save layer as...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="965"/>
        <source>Error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="886"/>
        <source>Error creating field </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="965"/>
        <source>Layer creation failed</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="971"/>
        <source>Error creating shapefile</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="973"/>
        <source>The shapefile could not be created (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="981"/>
        <source>Driver not found</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="982"/>
        <source> driver is not available</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsNewConnection</name>
    <message>
        <location filename="../src/app/qgsnewconnection.cpp" line="115"/>
        <source>Test connection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsnewconnection.cpp" line="112"/>
        <source>Connection to %1 was successful</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsnewconnection.cpp" line="115"/>
        <source>Connection failed - Check settings and try again.

Extended error information:
</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsNewConnectionBase</name>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="21"/>
        <source>Create a New PostGIS connection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="39"/>
        <source>Connection Information</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="64"/>
        <source>Restrict the search to the public schema for spatial tables not in the geometry_columns table</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="67"/>
        <source>When searching for spatial tables that are not in the geometry_columns tables, restrict the search to tables that are in the public schema (for some databases this can save lots of time)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="70"/>
        <source>Only look in the &apos;public&apos; schema</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="87"/>
        <source>Restrict the displayed tables to those that are in the geometry_columns table</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="90"/>
        <source>Restricts the displayed tables to those that are in the geometry_columns table. This can speed up the initial display of spatial tables.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="93"/>
        <source>Only look in the geometry_columns table</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="108"/>
        <source>Save Password</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="115"/>
        <source>Test Connect</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="140"/>
        <source>Name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="150"/>
        <source>Host</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="160"/>
        <source>Database</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="170"/>
        <source>Port</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="180"/>
        <source>Username</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="190"/>
        <source>Password</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="210"/>
        <source>Name of the new connection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="223"/>
        <source>5432</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="255"/>
        <source>OK</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="271"/>
        <source>Cancel</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="287"/>
        <source>Help</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="290"/>
        <source>F1</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsNewHttpConnectionBase</name>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="13"/>
        <source>Create a New WMS connection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="31"/>
        <source>Connection Information</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="62"/>
        <source>Name of the new connection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="72"/>
        <source>Name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="85"/>
        <source>URL</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="98"/>
        <source>Proxy Host</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="111"/>
        <source>Proxy Port</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="124"/>
        <source>Proxy User</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="137"/>
        <source>Proxy Password</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="158"/>
        <source>Your user name for the HTTP proxy (optional)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="173"/>
        <source>Password for your HTTP proxy (optional)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="183"/>
        <source>HTTP address of the Web Map Server</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="190"/>
        <source>Name of your HTTP proxy (optional)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="205"/>
        <source>Port number of your HTTP proxy (optional)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="226"/>
        <source>OK</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="242"/>
        <source>Cancel</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="258"/>
        <source>Help</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="261"/>
        <source>F1</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsNorthArrowPlugin</name>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="82"/>
        <source>Bottom Left</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="83"/>
        <source>Top Left</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="83"/>
        <source>Top Right</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="83"/>
        <source>Bottom Right</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="96"/>
        <source>&amp;North Arrow</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="97"/>
        <source>Creates a north arrow that is displayed on the map canvas</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="255"/>
        <source>&amp;Decorations</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="246"/>
        <source>North arrow pixmap not found</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsNorthArrowPluginGui</name>
    <message>
        <location filename="../src/plugins/north_arrow/plugingui.cpp" line="157"/>
        <source>Pixmap not found</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsNorthArrowPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="237"/>
        <source>North Arrow Plugin</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="35"/>
        <source>Properties</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="55"/>
        <source>Angle</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="62"/>
        <source>Placement</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="75"/>
        <source>Set direction automatically</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="85"/>
        <source>Enable North Arrow</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="137"/>
        <source>Placement on screen</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="141"/>
        <source>Top Left</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="146"/>
        <source>Top Right</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="151"/>
        <source>Bottom Left</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="156"/>
        <source>Bottom Right</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="164"/>
        <source>Preview of north arrow</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="183"/>
        <source>Icon</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="198"/>
        <source>Browse...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="206"/>
        <source>New Item</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsOGRFactory</name>
    <message>
        <location filename="../src/providers/ogr/qgsogrfactory.cpp" line="63"/>
        <source>Wrong Path/URI</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/ogr/qgsogrfactory.cpp" line="63"/>
        <source>The provided path for the dataset is not valid.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsOptions</name>
    <message>
        <location filename="../src/app/qgsoptions.cpp" line="150"/>
        <source>Detected active locale on your system: </source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsOptionsBase</name>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="13"/>
        <source>QGIS Options</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="48"/>
        <source>&amp;General</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="60"/>
        <source>General</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="72"/>
        <source>Ask to save project changes when required</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="79"/>
        <source>Warn me when opening a project file saved with an older version of QGIS</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="103"/>
        <source>&amp;Appearance</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="115"/>
        <source>&amp;Splash screen</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="127"/>
        <source>Hide splash screen at startup</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="137"/>
        <source>&amp;Icon Theme</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="149"/>
        <source>&lt;b&gt;Note: &lt;/b&gt;Theme changes take effect the next time QGIS is started</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="167"/>
        <source>Theme</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="205"/>
        <source>Default Map Appearance (Overridden by project properties)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="217"/>
        <source>Selection Color:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="253"/>
        <source>Background Color:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="292"/>
        <source>Appearance</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="304"/>
        <source>Capitalise layer name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="328"/>
        <source>&amp;Rendering</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="340"/>
        <source>&amp;Update during drawing</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="352"/>
        <source>features</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="359"/>
        <source>Map display will be updated (drawn) after this many features have been read from the data source</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="372"/>
        <source>Update display after reading</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="382"/>
        <source>(Set to 0 to not update the display until all features have been read)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="392"/>
        <source>Initial Visibility</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="404"/>
        <source>By default new la&amp;yers added to the map should be displayed</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="414"/>
        <source>Rendering</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="426"/>
        <source>Make lines appear less jagged at the expense of some drawing performance</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="433"/>
        <source>Selecting this will unselect the &apos;make lines less&apos; jagged toggle</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="436"/>
        <source>Fix problems with incorrectly filled polygons</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="443"/>
        <source>Continuously redraw the map when dragging the legend/map divider</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="467"/>
        <source>&amp;Map tools</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="479"/>
        <source>Panning and zooming</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="492"/>
        <source>Zoom</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="497"/>
        <source>Zoom and recenter</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="502"/>
        <source>Nothing</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="510"/>
        <source>Zoom factor:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="517"/>
        <source>Mouse wheel action:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="540"/>
        <source>Measure tool</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="581"/>
        <source>Rubberband color:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="591"/>
        <source>Ellipsoid for distance calculations:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="604"/>
        <source>Search radius</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="616"/>
        <source>(Specify the search radius as a percentage of the map width)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="626"/>
        <source>Search Radius for Identifying Features and displaying Map Tips</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="636"/>
        <source>%</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="669"/>
        <source>Digitizing</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="681"/>
        <source>Rubberband</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="701"/>
        <source>Line Width:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="711"/>
        <source>Line width in pixels</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="734"/>
        <source>Line Colour:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="762"/>
        <source>Snapping</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="782"/>
        <source>Default Snapping Tolerance (in layer units):</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="819"/>
        <source>Search radius for vertex edits (in layer units):</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="865"/>
        <source>Pro&amp;jection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="890"/>
        <source>Select Global Default ...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="900"/>
        <source>When layer is loaded that has no projection information</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="912"/>
        <source>Prompt for projection.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="919"/>
        <source>Project wide default projection will be used.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="926"/>
        <source>Global default projection displa&amp;yed below will be used.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="937"/>
        <source>Locale</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="949"/>
        <source>Force Override System Locale</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="964"/>
        <source>Locale to use instead</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="977"/>
        <source>Note: Enabling / changing overide on local requires an application restart.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="1003"/>
        <source>Additional Info</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="1015"/>
        <source>Detected active locale on your system:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="1026"/>
        <source>Help &amp;Browser</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="1051"/>
        <source>&lt;b&gt;Note:&lt;/b&gt; The browser must be in your PATH or you can specify the full path above</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="1067"/>
        <source>...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="1078"/>
        <source>epiphany</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="1083"/>
        <source>firefox</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="1088"/>
        <source>mozilla-firefox</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="1093"/>
        <source>galeon</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="1098"/>
        <source>konqueror</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="1103"/>
        <source>mozilla</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="1108"/>
        <source>opera</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="1116"/>
        <source>Open help documents with</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsPasteTransformationsBase</name>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="19"/>
        <source>Paste Transformations</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="42"/>
        <source>&lt;b&gt;Note: This function is not useful yet!&lt;/b&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="65"/>
        <source>Source</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="86"/>
        <source>Destination</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="125"/>
        <source>&amp;Help</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="128"/>
        <source>F1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="154"/>
        <source>Add New Transfer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="161"/>
        <source>&amp;OK</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="177"/>
        <source>&amp;Cancel</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsPatternDialogBase</name>
    <message>
        <location filename="../src/ui/qgspatterndialogbase.ui" line="13"/>
        <source>Select a fill pattern</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspatterndialogbase.ui" line="189"/>
        <source>No Fill</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspatterndialogbase.ui" line="205"/>
        <source>Cancel</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspatterndialogbase.ui" line="212"/>
        <source>Ok</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsPgGeoprocessing</name>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="78"/>
        <source>&amp;Buffer features</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="80"/>
        <source>Create a buffer for a PostgreSQL layer. </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="80"/>
        <source>A new layer is created in the database with the buffered features.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="410"/>
        <source>&amp;Geoprocessing</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="117"/>
        <source>Buffer features in layer %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="325"/>
        <source>Unable to add geometry column</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="327"/>
        <source>Unable to add geometry column to the output table </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="331"/>
        <source>Unable to create table</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="333"/>
        <source>Failed to create the output table </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="340"/>
        <source>Error connecting to the database</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="346"/>
        <source>No GEOS support</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="347"/>
        <source>Buffer function requires GEOS support in PostGIS</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="350"/>
        <source>Not a PostgreSQL/PostGIS Layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="353"/>
        <source> is not a PostgreSQL/PostGIS layer.
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="353"/>
        <source>Geoprocessing functions are only available for PostgreSQL/PostGIS Layers</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="356"/>
        <source>No Active Layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="357"/>
        <source>You must select a layer in the legend to buffer</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsPgQueryBuilder</name>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="77"/>
        <source>Table &lt;b&gt;%1&lt;/b&gt; in database &lt;b&gt;%2&lt;/b&gt; on host &lt;b&gt;%3&lt;/b&gt;, user &lt;b&gt;%4&lt;/b&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="61"/>
        <source>Connection Failed</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="61"/>
        <source>Connection to the database failed:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="199"/>
        <source>Database error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="199"/>
        <source>&lt;p&gt;Failed to get sample of field values using SQL:&lt;/p&gt;&lt;p&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="264"/>
        <source>No Query</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="264"/>
        <source>You must create a query before you can test it</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="275"/>
        <source>Query Result</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="277"/>
        <source>The where clause returned </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="277"/>
        <source> rows.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="281"/>
        <source>Query Failed</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="283"/>
        <source>An error occurred when executing the query:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="330"/>
        <source>Error in Query</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="336"/>
        <source>No Records</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="336"/>
        <source>The query you specified results in zero records being returned. Valid PostgreSQL layers must have at least one feature.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsPgQueryBuilderBase</name>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="23"/>
        <source>PostgreSQL Query Builder</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="41"/>
        <source>Operators</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="53"/>
        <source>=</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="60"/>
        <source>&lt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="67"/>
        <source>NOT</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="74"/>
        <source>OR</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="81"/>
        <source>AND</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="88"/>
        <source>%</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="95"/>
        <source>IN</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="102"/>
        <source>NOT IN</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="109"/>
        <source>!=</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="116"/>
        <source>&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="123"/>
        <source>LIKE</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="130"/>
        <source>ILIKE</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="137"/>
        <source>&gt;=</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="144"/>
        <source>&lt;=</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="178"/>
        <source>Clear</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="188"/>
        <source>Test</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="198"/>
        <source>Ok</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="208"/>
        <source>Cancel</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="241"/>
        <source>Values</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="253"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Retrieve &lt;span style=&quot; font-weight:600;&quot;&gt;all&lt;/span&gt; the record in the vector file (&lt;span style=&quot; font-style:italic;&quot;&gt;if the table is big, the operation can consume some time&lt;/span&gt;)&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="259"/>
        <source>All</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="266"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Take a &lt;span style=&quot; font-weight:600;&quot;&gt;sample&lt;/span&gt; of records in the vector file&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="272"/>
        <source>Sample</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="279"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;List of values for the current field.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="309"/>
        <source>Fields</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="321"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;List of fields in this vector file&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="334"/>
        <source>Datasource:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="347"/>
        <source>SQL where clause</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsPluginManager</name>
    <message>
        <location filename="../src/app/qgspluginmanager.cpp" line="76"/>
        <source>Name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgspluginmanager.cpp" line="77"/>
        <source>Version</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgspluginmanager.cpp" line="78"/>
        <source>Description</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgspluginmanager.cpp" line="79"/>
        <source>Library name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgspluginmanager.cpp" line="180"/>
        <source>No Plugins</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgspluginmanager.cpp" line="180"/>
        <source>No QGIS plugins found in </source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsPluginManagerBase</name>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="16"/>
        <source>QGIS Plugin Manager</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="39"/>
        <source>Plugin Directory</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="61"/>
        <source>To load a plugin, click the checkbox next to the plugin and click Ok</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="92"/>
        <source>&amp;Select All</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="95"/>
        <source>Alt+S</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="102"/>
        <source>C&amp;lear All</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="105"/>
        <source>Alt+L</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="112"/>
        <source>&amp;Ok</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="115"/>
        <source>Alt+O</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="122"/>
        <source>&amp;Close</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="125"/>
        <source>Alt+C</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsPointDialog</name>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="556"/>
        <source>Linear</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="197"/>
        <source>Choose a name for the world file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="557"/>
        <source>Helmert</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="215"/>
        <source>-modified</source>
        <comment>Georeferencer:QgsPointDialog.cpp - used to modify a user given filename</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="264"/>
        <source>Warning</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="270"/>
        <source>&lt;p&gt;A Helmert transform requires modifications in the raster layer.&lt;/p&gt;&lt;p&gt;The modified raster will be saved in a new file and a world file will be generated for this new file instead.&lt;/p&gt;&lt;p&gt;Are you sure that this is what you want?&lt;/p&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="271"/>
        <source>Currently all modified files will be written in TIFF format.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="279"/>
        <source>Affine</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="289"/>
        <source>Not implemented!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="284"/>
        <source>&lt;p&gt;An affine transform requires changing the original raster file. This is not yet supported.&lt;/p&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="291"/>
        <source>&lt;p&gt;The </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="292"/>
        <source> transform is not yet supported.&lt;/p&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="323"/>
        <source>Error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="324"/>
        <source>Could not write to </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="487"/>
        <source>Zoom In</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="486"/>
        <source>z</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="492"/>
        <source>Zoom Out</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="491"/>
        <source>Z</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="495"/>
        <source>Zoom To Layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="497"/>
        <source>Zoom to Layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="500"/>
        <source>Pan Map</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="501"/>
        <source>Pan the map</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="504"/>
        <source>Add Point</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="505"/>
        <source>.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="506"/>
        <source>Capture Points</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="509"/>
        <source>Delete Point</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="510"/>
        <source>Delete Selected</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsPointDialogBase</name>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="13"/>
        <source>Reference points</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="38"/>
        <source>...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="45"/>
        <source>Modified raster:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="52"/>
        <source>World file:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="65"/>
        <source>Transform type:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="75"/>
        <source>Create</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="105"/>
        <source>Add points</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="130"/>
        <source>Delete points</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="178"/>
        <source>Zoom in</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="200"/>
        <source>Zoom out</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="222"/>
        <source>Zoom to the raster extents</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="244"/>
        <source>Pan</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="282"/>
        <source>Create and load layer</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsPointStyleWidgetBase</name>
    <message>
        <location filename="../src/ui/qgspointstylewidgetbase.ui" line="16"/>
        <source>Form3</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspointstylewidgetbase.ui" line="36"/>
        <source>Symbol Style</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspointstylewidgetbase.ui" line="51"/>
        <source>Scale</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsPostgresProvider</name>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="120"/>
        <source>Unable to access relation</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="121"/>
        <source>Unable to access the </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="123"/>
        <source> relation.
The error message from the database was:
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="287"/>
        <source>No GEOS Support!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="291"/>
        <source>Your PostGIS installation has no GEOS support.
Feature selection and identification will not work properly.
Please install PostGIS with GEOS support (http://geos.refractions.net)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="880"/>
        <source>No suitable key column in table</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="884"/>
        <source>The table has no column suitable for use as a key.

Qgis requires that the table either has a column of type
int4 with a unique constraint on it (which includes the
primary key) or has a PostgreSQL oid column.
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="928"/>
        <source>The unique index on column</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="930"/>
        <source>is unsuitable because Qgis does not currently support non-int4 type columns as a key into the table.
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="955"/>
        <source>and </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="960"/>
        <source>The unique index based on columns </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="962"/>
        <source> is unsuitable because Qgis does not currently support multiple columns as a key into the table.
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1008"/>
        <source>Unable to find a key column</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1091"/>
        <source> derives from </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1095"/>
        <source>and is suitable.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1099"/>
        <source>and is not suitable </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1100"/>
        <source>type is </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1102"/>
        <source> and has a suitable constraint)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1104"/>
        <source> and does not have a suitable constraint)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1190"/>
        <source>Note: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1192"/>
        <source>initially appeared suitable but does not contain unique data, so is not suitable.
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1204"/>
        <source>The view you selected has the following columns, none of which satisfy the above conditions:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1210"/>
        <source>Qgis requires that the view has a column that can be used as a unique key. Such a column should be derived from a table column of type int4 and be a primary key, have a unique constraint on it, or be a PostgreSQL oid column. To improve performance the column should also be indexed.
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1211"/>
        <source>The view </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1212"/>
        <source>has no column suitable for use as a unique key.
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1213"/>
        <source>No suitable key column in view</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1817"/>
        <source>INSERT error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1811"/>
        <source>An error occured during feature insertion</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1870"/>
        <source>DELETE error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1864"/>
        <source>An error occured during deletion from disk</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2184"/>
        <source>PostGIS error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2178"/>
        <source>An error occured contacting the PostgreSQL database</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2185"/>
        <source>The PostgreSQL database returned: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2186"/>
        <source>When trying: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2615"/>
        <source>Unknown geometry type</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2616"/>
        <source>Column </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2626"/>
        <source> in </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2618"/>
        <source> has a geometry type of </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2618"/>
        <source>, which Qgis does not currently support.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2625"/>
        <source>Qgis was unable to determine the type and srid of column </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2627"/>
        <source>. The database communication log was:
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2628"/>
        <source>Unable to get feature type and srid</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsProjectPropertiesBase</name>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="13"/>
        <source>Project Properties</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="38"/>
        <source>General</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="50"/>
        <source>Project Title</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="62"/>
        <source>Descriptive project name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="65"/>
        <source>Default project title</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="75"/>
        <source>Precision</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="87"/>
        <source>Automatically sets the number of decimal places in the mouse position display</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="90"/>
        <source>The number of decimal places that are used when displaying the mouse position is automatically set to be enough so that moving the mouse by one pixel gives a change in the position display</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="93"/>
        <source>Automatic</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="106"/>
        <source>Sets the number of decimal places to use for the mouse position display</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="109"/>
        <source>Manual</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="119"/>
        <source>The number of decimal places for the manual option</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="134"/>
        <source>decimal places</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="157"/>
        <source>Map Appearance</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="169"/>
        <source>Selection Color:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="208"/>
        <source>Background Color:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="255"/>
        <source>Digitizing</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="267"/>
        <source>Enable topological editing</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="280"/>
        <source>Snapping options...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="293"/>
        <source>Avoid intersections of new polygons</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="301"/>
        <source>Map Units</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="313"/>
        <source>Meters</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="323"/>
        <source>Feet</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="330"/>
        <source>Decimal degrees</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="354"/>
        <source>Projection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="366"/>
        <source>Enable on the fly projection</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsProjectionSelector</name>
    <message>
        <location filename="../src/gui/qgsprojectionselector.cpp" line="783"/>
        <source>QGIS SRSID: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/gui/qgsprojectionselector.cpp" line="784"/>
        <source>PostGIS SRID: </source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsProjectionSelectorBase</name>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="24"/>
        <source>Projection Selector</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="47"/>
        <source>Projection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="67"/>
        <source>Search</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="85"/>
        <source>Find</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="98"/>
        <source>Name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="111"/>
        <source>QGIS SRSID</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="124"/>
        <source>EPSG ID</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="137"/>
        <source>Postgis SRID</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="196"/>
        <source>Spatial Reference System</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="201"/>
        <source>Id</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsPythonDialog</name>
    <message>
        <location filename="../src/ui/qgspythondialog.ui" line="13"/>
        <source>Python console</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspythondialog.ui" line="33"/>
        <source>To access Quantum GIS environment from this python console use object from global scope which is an instance of QgisInterface class.&lt;br&gt;Usage e.g.: iface.zoomFull()</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspythondialog.ui" line="58"/>
        <source>&gt;&gt;&gt;</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsQuickPrint</name>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="819"/>
        <source> km</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="824"/>
        <source> mm</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="829"/>
        <source> cm</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="833"/>
        <source> m</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="838"/>
        <source> miles</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="843"/>
        <source> mile</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="848"/>
        <source> inches</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="853"/>
        <source> foot</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="857"/>
        <source> feet</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="862"/>
        <source> degree</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="864"/>
        <source> degrees</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="867"/>
        <source> unknown</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsRasterLayer</name>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="287"/>
        <source>and all other files</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2275"/>
        <source>Raster Extent: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2278"/>
        <source>Clipped area: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3500"/>
        <source>Driver:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3526"/>
        <source>Dataset Description</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3576"/>
        <source>Dimensions:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3579"/>
        <source>X: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3580"/>
        <source> Y: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3580"/>
        <source> Bands: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3587"/>
        <source>No Data Value</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3596"/>
        <source>NoDataValue not set</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3602"/>
        <source>Data Type:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3608"/>
        <source>GDT_Byte - Eight bit unsigned integer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3611"/>
        <source>GDT_UInt16 - Sixteen bit unsigned integer </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3614"/>
        <source>GDT_Int16 - Sixteen bit signed integer </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3617"/>
        <source>GDT_UInt32 - Thirty two bit unsigned integer </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3620"/>
        <source>GDT_Int32 - Thirty two bit signed integer </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3623"/>
        <source>GDT_Float32 - Thirty two bit floating point </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3626"/>
        <source>GDT_Float64 - Sixty four bit floating point </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3629"/>
        <source>GDT_CInt16 - Complex Int16 </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3632"/>
        <source>GDT_CInt32 - Complex Int32 </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3635"/>
        <source>GDT_CFloat32 - Complex Float32 </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3638"/>
        <source>GDT_CFloat64 - Complex Float64 </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3641"/>
        <source>Could not determine raster data type.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3646"/>
        <source>Pyramid overviews:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3667"/>
        <source>Layer Spatial Reference System: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3695"/>
        <source>Origin:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3704"/>
        <source>Pixel Size:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3721"/>
        <source>Property</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3724"/>
        <source>Value</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4841"/>
        <source>Band</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3740"/>
        <source>Band No</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3752"/>
        <source>No Stats</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3755"/>
        <source>No stats collected yet</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3765"/>
        <source>Min Val</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3773"/>
        <source>Max Val</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3781"/>
        <source>Range</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3789"/>
        <source>Mean</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3797"/>
        <source>Sum of squares</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3805"/>
        <source>Standard Deviation</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3813"/>
        <source>Sum of all cells</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3821"/>
        <source>Cell Count</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3913"/>
        <source>Average Magphase</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3918"/>
        <source>Average</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4798"/>
        <source>out of extent</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4835"/>
        <source>null (no data)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.h" line="527"/>
        <source>Not Set</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsRasterLayerProperties</name>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="996"/>
        <source>Grayscale</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2400"/>
        <source>Pseudocolor</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2400"/>
        <source>Freak Out</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1674"/>
        <source>Custom Colormap</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2491"/>
        <source>No Stretch</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1081"/>
        <source>Stretch To MinMax</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1085"/>
        <source>Stretch And Clip To MinMax</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1089"/>
        <source>Clip To MinMax</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1390"/>
        <source>Discrete</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="871"/>
        <source>Linearly</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2612"/>
        <source>Equal interval</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2634"/>
        <source>Quantiles</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="174"/>
        <source>Palette</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="339"/>
        <source>Description</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="340"/>
        <source>Large resolution raster layers can slow navigation in QGIS.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="341"/>
        <source>By creating lower resolution copies of the data (pyramids) performance can be considerably improved as QGIS selects the most suitable resolution to use depending on the level of zoom.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="342"/>
        <source>You must have write access in the directory where the original data is stored to build pyramids.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="343"/>
        <source>Please note that building pyramids may alter the original data file and once created they cannot be removed!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="344"/>
        <source>Please note that building pyramids could corrupt your image - always make a backup of your data first!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1810"/>
        <source>Red</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1810"/>
        <source>Green</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1810"/>
        <source>Blue</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1827"/>
        <source>Percent Transparent</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1823"/>
        <source>Gray</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1827"/>
        <source>Indexed Value</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1684"/>
        <source>User Defined</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="713"/>
        <source>No Scaling</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="789"/>
        <source>Columns: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="790"/>
        <source>Rows: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="791"/>
        <source>No-Data Value: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="783"/>
        <source>No-Data Value: Not Set</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="791"/>
        <source>n/a</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1838"/>
        <source>Write access denied</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1838"/>
        <source>Write access denied. Adjust the file permissions and try again.

</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1554"/>
        <source>Building pyramids failed.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1550"/>
        <source>The file was not writeable. Some formats can not be written to, only read. You can also try to check the permissions and then try again.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1555"/>
        <source>Building pyramid overviews is not supported on this type of raster.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1795"/>
        <source>Save file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2297"/>
        <source>Textfile (*.txt)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1807"/>
        <source>QGIS Generated Transparent Pixel Value Export File</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2297"/>
        <source>Open file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2370"/>
        <source>Import Error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2370"/>
        <source>The following lines contained errors

</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2375"/>
        <source>Read access denied</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2375"/>
        <source>Read access denied. Adjust the file permissions and try again.

</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2400"/>
        <source>Color Ramp</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsRasterLayerPropertiesBase</name>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="13"/>
        <source>Raster Layer Properties</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="38"/>
        <source>Symbology</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="50"/>
        <source>Grayscale Band Scaling</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="152"/>
        <source>Max</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="332"/>
        <source>Std Deviation</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="179"/>
        <source>Custom Min Max Values:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="207"/>
        <source>Min</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="249"/>
        <source>Contrast Enhancement</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="291"/>
        <source>Load Min Max Values From Band(s)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="313"/>
        <source>RGB Scaling</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="325"/>
        <source>&lt;b&gt;&lt;font color=&apos;blue&apos;&gt;Max&lt;/font&gt;&lt;/b&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="339"/>
        <source>Custom Min Max Values</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="381"/>
        <source>&lt;b&gt;&lt;font color=&apos;green&apos;&gt;Min&lt;/font&gt;&lt;/b&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="396"/>
        <source>&lt;b&gt;&lt;font color=&apos;green&apos;&gt;Max&lt;/font&gt;&lt;/b&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="411"/>
        <source>&lt;b&gt;&lt;font color=&apos;red&apos;&gt;Min&lt;/font&gt;&lt;/b&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="466"/>
        <source>&lt;b&gt;&lt;font color=&apos;red&apos;&gt;Max&lt;/font&gt;&lt;/b&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="548"/>
        <source>&lt;b&gt;&lt;font color=&apos;blue&apos;&gt;Min&lt;/font&gt;&lt;/b&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="651"/>
        <source>Grayscale Band Selection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="671"/>
        <source>Gray</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="699"/>
        <source>RGB Mode Band Selection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="711"/>
        <source>&lt;b&gt;&lt;font color=&apos;blue&apos;&gt;Blue&lt;/font&gt;&lt;/b&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="721"/>
        <source>&lt;b&gt;&lt;font color=&apos;green&apos;&gt;Green&lt;/font&gt;&lt;/b&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="731"/>
        <source>&lt;b&gt;&lt;font color=&apos;red&apos;&gt;Red&lt;/font&gt;&lt;/b&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="816"/>
        <source>Color Map</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="846"/>
        <source>Invert Color Map</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="856"/>
        <source>Global Transparency</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="923"/>
        <source> 00%</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="930"/>
        <source>None</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="937"/>
        <source>&lt;p align=&quot;right&quot;&gt;Full&lt;/p&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="947"/>
        <source>Render as</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="972"/>
        <source>Single Band Gray</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="979"/>
        <source>Three Band Color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="990"/>
        <source>Transparent Pixels</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1036"/>
        <source>Transparent Band:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1050"/>
        <source>Custom Transparency List</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1057"/>
        <source>Transparency Layer;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1100"/>
        <source>Add Values Manually</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1184"/>
        <source>...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1116"/>
        <source>Add Values From Display</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1129"/>
        <source>Remove Selected Row</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1142"/>
        <source>Default Values</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1168"/>
        <source>Import From File</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1181"/>
        <source>Export To File</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1196"/>
        <source>No Data Value:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1206"/>
        <source>Reset No Data Value</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1214"/>
        <source>Colormap</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1260"/>
        <source>Number of entries:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1294"/>
        <source>Delete entry</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1301"/>
        <source>Classify</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1317"/>
        <source>1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1322"/>
        <source>2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1338"/>
        <source>Color interpolation:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1358"/>
        <source>Classification mode:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1371"/>
        <source>General</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1383"/>
        <source>Spatial Reference System</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1395"/>
        <source>Change</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1412"/>
        <source>Scale Dependent Visibility</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1427"/>
        <source>Maximum scale at which this layer will be displayed. </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1440"/>
        <source>Maximum 1:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1450"/>
        <source>Minimum scale at which this layer will be displayed. </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1463"/>
        <source>Minimum 1:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1476"/>
        <source>DebugInfo</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1491"/>
        <source>Columns:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1498"/>
        <source>Rows:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1505"/>
        <source>No Data:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1514"/>
        <source>Layer Source:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1534"/>
        <source>Display Name:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1552"/>
        <source>Thumbnail</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1602"/>
        <source>Legend:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1652"/>
        <source>Palette:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1705"/>
        <source>Metadata</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1721"/>
        <source>Pyramids</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1748"/>
        <source>Pyramid Resolutions</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1800"/>
        <source>Resampling Method</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1811"/>
        <source>Average</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1816"/>
        <source>Nearest Neighbour</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1824"/>
        <source>Build Pyramids</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1834"/>
        <source>Histogram</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1881"/>
        <source>Chart Type</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1893"/>
        <source>Line Graph</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1903"/>
        <source>Bar Chart</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1913"/>
        <source>Refresh</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1920"/>
        <source>Options</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1932"/>
        <source>Column Count:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1939"/>
        <source>Out Of Range OK?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1946"/>
        <source>Allow Approximation</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsRunProcess</name>
    <message>
        <location filename="../src/core/qgsrunprocess.cpp" line="59"/>
        <source>Starting</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsrunprocess.cpp" line="115"/>
        <source>Done</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsrunprocess.cpp" line="146"/>
        <source>Unable to run command</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsScaleBarPlugin</name>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="78"/>
        <source>Bottom Left</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="79"/>
        <source>Top Left</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="79"/>
        <source>Top Right</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="79"/>
        <source>Bottom Right</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="81"/>
        <source>Tick Down</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="82"/>
        <source>Tick Up</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="82"/>
        <source>Bar</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="82"/>
        <source>Box</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="102"/>
        <source>&amp;Scale Bar</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="103"/>
        <source>Creates a scale bar that is displayed on the map canvas</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="543"/>
        <source>&amp;Decorations</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="164"/>
        <source> metres/km</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="165"/>
        <source> feet/miles</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="288"/>
        <source> degrees</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="243"/>
        <source> km</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="248"/>
        <source> mm</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="253"/>
        <source> cm</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="257"/>
        <source> m</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="262"/>
        <source> miles</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="267"/>
        <source> mile</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="272"/>
        <source> inches</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="277"/>
        <source> foot</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="281"/>
        <source> feet</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="286"/>
        <source> degree</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="291"/>
        <source> unknown</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsScaleBarPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="300"/>
        <source>Scale Bar Plugin</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="76"/>
        <source>Click to select the colour</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="94"/>
        <source>Size of bar:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="109"/>
        <source>Automatically snap to round number on resize</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="127"/>
        <source>Colour of bar:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="143"/>
        <source>Top Left</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="148"/>
        <source>Top Right</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="153"/>
        <source>Bottom Left</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="158"/>
        <source>Bottom Right</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="174"/>
        <source>Enable scale bar</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="192"/>
        <source>Scale bar style:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="207"/>
        <source>Select the style of the scale bar</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="211"/>
        <source>Tick Down</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="216"/>
        <source>Tick Up</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="221"/>
        <source>Box</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="226"/>
        <source>Bar</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="254"/>
        <source>Placement:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="274"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;This plugin draws a scale bar on the map. Please note the size option below is a &apos;preferred&apos; size and may have to be altered by QGIS depending on the level of zoom.  The size is measured according to the map units specified in the project properties.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsSearchQueryBuilder</name>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="37"/>
        <source>Search query builder</source>
        <translation type="unfinished"></translation>
    </message>
    <message numerus="yes">
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="167"/>
        <source>Found %d matching features.</source>
        <translation type="unfinished">
            <numerusform></numerusform>
        </translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="169"/>
        <source>No matching features found.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="170"/>
        <source>Search results</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="179"/>
        <source>Search string parsing error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="235"/>
        <source>No Records</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="235"/>
        <source>The query you specified results in zero records being returned.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsServerSourceSelect</name>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="171"/>
        <source>Are you sure you want to remove the </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="171"/>
        <source> connection and all associated settings?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="172"/>
        <source>Confirm Delete</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="454"/>
        <source>WMS Provider</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="456"/>
        <source>Could not open the WMS Provider</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="465"/>
        <source>Select Layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="465"/>
        <source>You must select at least one layer first.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="469"/>
        <source>Coordinate Reference System</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="469"/>
        <source>There are no available coordinate reference system for the set of layers you&apos;ve selected.</source>
        <translation type="unfinished"></translation>
    </message>
    <message numerus="yes">
        <location filename="../src/app/qgsserversourceselect.cpp" line="588"/>
        <source>Coordinate Reference System (%1 available)</source>
        <translation type="unfinished">
            <numerusform></numerusform>
        </translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="767"/>
        <source>Could not understand the response.  The</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="768"/>
        <source>provider said</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="822"/>
        <source>WMS proxies</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="822"/>
        <source>&lt;p&gt;Several WMS servers have been added to the server list. Note that the proxy fields have been left blank and if you access the internet via a web proxy, you will need to individually set the proxy fields with appropriate values.&lt;/p&gt;</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsServerSourceSelectBase</name>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="13"/>
        <source>Add Layer(s) from a Server</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="34"/>
        <source>Server Connections</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="46"/>
        <source>Adds a few example WMS servers</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="52"/>
        <source>Add default servers</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="81"/>
        <source>C&amp;onnect</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="91"/>
        <source>Edit</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="101"/>
        <source>Delete</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="108"/>
        <source>&amp;New</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="118"/>
        <source>Coordinate Reference System</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="156"/>
        <source>Change ...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="174"/>
        <source>Ready</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="187"/>
        <source>&amp;Add</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="190"/>
        <source>Alt+A</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="203"/>
        <source>Layers</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="236"/>
        <source>ID</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="241"/>
        <source>Name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="246"/>
        <source>Title</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="251"/>
        <source>Abstract</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="276"/>
        <source>Image encoding</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="302"/>
        <source>Help</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="305"/>
        <source>F1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="315"/>
        <source>C&amp;lose</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="318"/>
        <source>Alt+L</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsShapeFile</name>
    <message>
        <location filename="../src/plugins/spit/qgsshapefile.cpp" line="435"/>
        <source>The database gave an error while executing this SQL:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsshapefile.cpp" line="443"/>
        <source>The error was:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsshapefile.cpp" line="440"/>
        <source>... (rest of SQL trimmed)</source>
        <comment>is appended to a truncated SQL statement</comment>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsSingleSymbolDialog</name>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="121"/>
        <source>Solid Line</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="122"/>
        <source>Dash Line</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="123"/>
        <source>Dot Line</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="124"/>
        <source>Dash Dot Line</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="125"/>
        <source>Dash Dot Dot Line</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="126"/>
        <source>No Pen</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="131"/>
        <source>Solid Pattern</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="132"/>
        <source>Hor Pattern</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="133"/>
        <source>Ver Pattern</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="134"/>
        <source>Cross Pattern</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="135"/>
        <source>BDiag Pattern</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="136"/>
        <source>FDiag Pattern</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="137"/>
        <source>Diag Cross Pattern</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="138"/>
        <source>Dense1 Pattern</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="139"/>
        <source>Dense2 Pattern</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="140"/>
        <source>Dense3 Pattern</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="141"/>
        <source>Dense4 Pattern</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="142"/>
        <source>Dense5 Pattern</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="143"/>
        <source>Dense6 Pattern</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="144"/>
        <source>Dense7 Pattern</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="145"/>
        <source>No Brush</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="146"/>
        <source>Texture Pattern</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsSingleSymbolDialogBase</name>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="21"/>
        <source>Single Symbol</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="41"/>
        <source>Point Symbol</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="69"/>
        <source>Size</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="79"/>
        <source>Area scale field</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="89"/>
        <source>Rotation field</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="158"/>
        <source>Style Options</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="170"/>
        <source>...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="187"/>
        <source>Outline style</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="197"/>
        <source>Outline color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="207"/>
        <source>Outline width</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="217"/>
        <source>Fill color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="227"/>
        <source>Fill style</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="347"/>
        <source>Label</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsSnappingDialog</name>
    <message>
        <location filename="../src/app/qgssnappingdialog.cpp" line="147"/>
        <source>to vertex</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssnappingdialog.cpp" line="151"/>
        <source>to segment</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssnappingdialog.cpp" line="89"/>
        <source>to vertex and segment</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsSnappingDialogBase</name>
    <message>
        <location filename="../src/ui/qgssnappingdialogbase.ui" line="13"/>
        <source>Snapping options</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgssnappingdialogbase.ui" line="26"/>
        <source>Layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgssnappingdialogbase.ui" line="31"/>
        <source>Mode</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgssnappingdialogbase.ui" line="36"/>
        <source>Tolerance</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsSpit</name>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="70"/>
        <source>File Name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="70"/>
        <source>Feature Class</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="71"/>
        <source>Features</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="71"/>
        <source>DB Relation Name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="71"/>
        <source>Schema</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="120"/>
        <source>New Connection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="145"/>
        <source>Are you sure you want to remove the [</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="145"/>
        <source>] connection and all associated settings?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="146"/>
        <source>Confirm Delete</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="170"/>
        <source>Add Shapefiles</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="172"/>
        <source>Shapefiles (*.shp);;All files (*.*)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="866"/>
        <source> - Edit Column Names</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="308"/>
        <source>The following Shapefile(s) could not be loaded:

</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="312"/>
        <source>REASON: File cannot be opened</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="317"/>
        <source>REASON: One or both of the Shapefile files (*.dbf, *.shx) missing</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="394"/>
        <source>General Interface Help:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="396"/>
        <source>PostgreSQL Connections:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="398"/>
        <source>[New ...] - create a new connection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="399"/>
        <source>[Edit ...] - edit the currently selected connection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="400"/>
        <source>[Remove] - remove the currently selected connection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="401"/>
        <source>-you need to select a connection that works (connects properly) in order to import files</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="402"/>
        <source>-when changing connections Global Schema also changes accordingly</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="403"/>
        <source>Shapefile List:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="405"/>
        <source>[Add ...] - open a File dialog and browse to the desired file(s) to import</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="406"/>
        <source>[Remove] - remove the currently selected file(s) from the list</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="407"/>
        <source>[Remove All] - remove all the files in the list</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="408"/>
        <source>[SRID] - Reference ID for the shapefiles to be imported</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="409"/>
        <source>[Use Default (SRID)] - set SRID to -1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="410"/>
        <source>[Geometry Column Name] - name of the geometry column in the database</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="411"/>
        <source>[Use Default (Geometry Column Name)] - set column name to &apos;the_geom&apos;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="412"/>
        <source>[Glogal Schema] - set the schema for all files to be imported into</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="414"/>
        <source>[Import] - import the current shapefiles in the list</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="415"/>
        <source>[Quit] - quit the program
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="416"/>
        <source>[Help] - display this help dialog</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="842"/>
        <source>Import Shapefiles</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="430"/>
        <source>You need to specify a Connection first</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="446"/>
        <source>Connection failed - Check settings and try again</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="477"/>
        <source>PostGIS not available</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="479"/>
        <source>&lt;p&gt;The chosen database does not have PostGIS installed, but this is required for storage of spatial data.&lt;/p&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="550"/>
        <source>You need to add shapefiles to the list first</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="611"/>
        <source>Importing files</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="555"/>
        <source>Cancel</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="559"/>
        <source>Progress</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="566"/>
        <source>Problem inserting features from file:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="573"/>
        <source>Invalid table name.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="583"/>
        <source>No fields detected.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="596"/>
        <source>Checking to see if </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="604"/>
        <source>The following fields are duplicates:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="844"/>
        <source>&lt;p&gt;Error while executing the SQL:&lt;/p&gt;&lt;p&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="845"/>
        <source>&lt;/p&gt;&lt;p&gt;The database said:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="704"/>
        <source>Import Shapefiles - Relation Exists</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="705"/>
        <source>The Shapefile:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="706"/>
        <source>will use [</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="706"/>
        <source>] relation for its data,</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="706"/>
        <source>which already exists and possibly contains data.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="707"/>
        <source>To avoid data loss change the &quot;DB Relation Name&quot;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="707"/>
        <source>for this Shapefile in the main dialog file list.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="708"/>
        <source>Do you want to overwrite the [</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="708"/>
        <source>] relation?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="869"/>
        <source>Use the table below to edit column names. Make sure that none of the columns are named using a PostgreSQL reserved word</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsSpitBase</name>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="19"/>
        <source>SPIT - Shapefile to PostGIS Import Tool</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="53"/>
        <source>Shapefile to PostGIS Import Tool</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="63"/>
        <source>Shapefile List</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="96"/>
        <source>Add a shapefile to the list of files to be imported</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="99"/>
        <source>Add</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="109"/>
        <source>Remove the selected shapefile from the import list</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="399"/>
        <source>Remove</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="122"/>
        <source>Remove all the shapefiles from the import list</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="125"/>
        <source>Remove All</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="187"/>
        <source>SRID</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="212"/>
        <source>Set the SRID to the default value</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="215"/>
        <source>Use Default SRID</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="251"/>
        <source>Set the geometry column name to the default value</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="254"/>
        <source>Use Default Geometry Column Name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="269"/>
        <source>Geometry Column Name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="330"/>
        <source>Global Schema</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="360"/>
        <source>PostgreSQL Connections</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="383"/>
        <source>Create a new PostGIS connection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="386"/>
        <source>New</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="396"/>
        <source>Remove the current PostGIS connection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="409"/>
        <source>Connect</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="419"/>
        <source>Edit the current PostGIS connection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="422"/>
        <source>Edit</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsSpitPlugin</name>
    <message>
        <location filename="../src/plugins/spit/qgsspitplugin.cpp" line="68"/>
        <source>&amp;Import Shapefiles to PostgreSQL</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitplugin.cpp" line="70"/>
        <source>Import shapefiles into a PostGIS-enabled PostgreSQL database. The schema and field names can be customized on import</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitplugin.cpp" line="93"/>
        <source>&amp;Spit</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsUniqueValueDialogBase</name>
    <message>
        <location filename="../src/ui/qgsuniquevaluedialogbase.ui" line="13"/>
        <source>Form1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsuniquevaluedialogbase.ui" line="37"/>
        <source>Classification Field:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsuniquevaluedialogbase.ui" line="47"/>
        <source>Delete class</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsuniquevaluedialogbase.ui" line="80"/>
        <source>Classify</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsVectorLayer</name>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2445"/>
        <source>Could not commit the added features.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2536"/>
        <source>No other types of changes will be committed at this time.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2467"/>
        <source>Could not commit the changed attributes.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2526"/>
        <source>However, the added features were committed OK.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2493"/>
        <source>Could not commit the changed geometries.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2530"/>
        <source>However, the changed attributes were committed OK.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2523"/>
        <source>Could not commit the deleted features.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2534"/>
        <source>However, the changed geometries were committed OK.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsVectorLayerProperties</name>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="106"/>
        <source>Transparency: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="197"/>
        <source>Single Symbol</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="200"/>
        <source>Graduated Symbol</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="201"/>
        <source>Continuous Color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="202"/>
        <source>Unique Value</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="157"/>
        <source>This button opens the PostgreSQL query builder and allows you to create a subset of features to display on the map canvas rather than displaying all features in the layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="160"/>
        <source>The query used to limit the features in the layer is shown here. This is currently only supported for PostgreSQL layers. To enter or modify the query, click on the Query Builder button</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="384"/>
        <source>Spatial Index</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="379"/>
        <source>Creation of spatial index successfull</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="384"/>
        <source>Creation of spatial index failed</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="397"/>
        <source>General:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="405"/>
        <source>Layer comment: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="412"/>
        <source>Storage type of this layer : </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="418"/>
        <source>Source for this layer : </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="435"/>
        <source>Geometry type of the features in this layer : </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="443"/>
        <source>The number of features in this layer : </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="448"/>
        <source>Editing capabilities of this layer : </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="455"/>
        <source>Extents:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="460"/>
        <source>In layer spatial reference system units : </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="461"/>
        <source>xMin,yMin </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="465"/>
        <source> : xMax,yMax </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="494"/>
        <source>Layer Spatial Reference System:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="520"/>
        <source>In project spatial reference system units : </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="531"/>
        <source>Attribute field info:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="538"/>
        <source>Field</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="541"/>
        <source>Type</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="544"/>
        <source>Length</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="547"/>
        <source>Precision</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="550"/>
        <source>Comment</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="748"/>
        <source>Default Style</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="733"/>
        <source>QGIS Layer Style File (*.qml)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="754"/>
        <source>QGIS</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="755"/>
        <source>Unknown style format: </source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsVectorLayerPropertiesBase</name>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="19"/>
        <source>Layer Properties</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="44"/>
        <source>Symbology</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="69"/>
        <source>Transparency:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="85"/>
        <source>Legend type:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="129"/>
        <source>General</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="141"/>
        <source>Display name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="151"/>
        <source>Display field for the Identify Results dialog box</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="154"/>
        <source>This sets the display field for the Identify Results dialog box</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="157"/>
        <source>Display field</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="167"/>
        <source>Use this control to set which field is placed at the top level of the Identify Results dialog box.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="177"/>
        <source>Use scale dependent rendering</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="192"/>
        <source>Maximum 1:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="202"/>
        <source>Minimum 1:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="212"/>
        <source>Minimum scale at which this layer will be displayed. </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="225"/>
        <source>Maximum scale at which this layer will be displayed. </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="241"/>
        <source>Spatial Index</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="253"/>
        <source>Create Spatial Index</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="263"/>
        <source>Create</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="289"/>
        <source>Spatial Reference System</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="308"/>
        <source>Change</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="318"/>
        <source>Subset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="359"/>
        <source>Query Builder</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="370"/>
        <source>Metadata</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="390"/>
        <source>Labels</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="410"/>
        <source>Display labels</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="436"/>
        <source>Actions</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="478"/>
        <source>Restore Default Style</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="485"/>
        <source>Save As Default</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="492"/>
        <source>Load Style ...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="499"/>
        <source>Save Style ...</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsVectorSymbologyWidgetBase</name>
    <message>
        <location filename="../src/ui/qgsvectorsymbologywidgetbase.ui" line="16"/>
        <source>Form2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorsymbologywidgetbase.ui" line="44"/>
        <source>Label</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorsymbologywidgetbase.ui" line="49"/>
        <source>Min</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorsymbologywidgetbase.ui" line="54"/>
        <source>Max</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorsymbologywidgetbase.ui" line="62"/>
        <source>Symbol Classes:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorsymbologywidgetbase.ui" line="77"/>
        <source>Count:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorsymbologywidgetbase.ui" line="90"/>
        <source>Mode:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorsymbologywidgetbase.ui" line="100"/>
        <source>Field:</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsWFSPlugin</name>
    <message>
        <location filename="../src/plugins/wfs/qgswfsplugin.cpp" line="58"/>
        <source>&amp;Add WFS layer</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsWFSProvider</name>
    <message>
        <location filename="../src/providers/wfs/qgswfsprovider.cpp" line="1390"/>
        <source>unknown</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wfs/qgswfsprovider.cpp" line="1396"/>
        <source>received %1 bytes from %2</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsWFSSourceSelect</name>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselect.cpp" line="259"/>
        <source>Are you sure you want to remove the </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselect.cpp" line="259"/>
        <source> connection and all associated settings?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselect.cpp" line="260"/>
        <source>Confirm Delete</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsWFSSourceSelectBase</name>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="13"/>
        <source>Add WFS Layer from a Server</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="29"/>
        <source>Title</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="34"/>
        <source>Name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="39"/>
        <source>Abstract</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="47"/>
        <source>Coordinate Reference System</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="85"/>
        <source>Change ...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="95"/>
        <source>Server Connections</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="107"/>
        <source>&amp;New</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="117"/>
        <source>Delete</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="127"/>
        <source>Edit</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="153"/>
        <source>C&amp;onnect</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsWmsProvider</name>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="718"/>
        <source>Tried URL: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="698"/>
        <source>HTTP Exception</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="661"/>
        <source>WMS Service Exception</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1512"/>
        <source>DOM Exception</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="772"/>
        <source>Could not get WMS capabilities: %1 at line %2 column %3</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="803"/>
        <source>This is probably due to an incorrect WMS Server URL.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="799"/>
        <source>Could not get WMS capabilities in the expected format (DTD): no %1 or %2 found</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1514"/>
        <source>Could not get WMS Service Exception at %1: %2 at line %3 column %4</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1564"/>
        <source>Request contains a Format not offered by the server.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1568"/>
        <source>Request contains a CRS not offered by the server for one or more of the Layers in the request.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1572"/>
        <source>Request contains a SRS not offered by the server for one or more of the Layers in the request.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1577"/>
        <source>GetMap request is for a Layer not offered by the server, or GetFeatureInfo request is for a Layer not shown on the map.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1581"/>
        <source>Request is for a Layer in a Style not offered by the server.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1585"/>
        <source>GetFeatureInfo request is applied to a Layer which is not declared queryable.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1589"/>
        <source>GetFeatureInfo request contains invalid X or Y value.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1594"/>
        <source>Value of (optional) UpdateSequence parameter in GetCapabilities request is equal to current value of service metadata update sequence number.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1599"/>
        <source>Value of (optional) UpdateSequence parameter in GetCapabilities request is greater than current value of service metadata update sequence number.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1604"/>
        <source>Request does not include a sample dimension value, and the server did not declare a default value for that dimension.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1608"/>
        <source>Request contains an invalid sample dimension value.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1612"/>
        <source>Request is for an optional operation that is not supported by the server.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1616"/>
        <source>(Unknown error code from a post-1.3 WMS server)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1619"/>
        <source>The WMS vendor also reported: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1622"/>
        <source>This is probably due to a bug in the QGIS program.  Please report this error.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1806"/>
        <source>Server Properties:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1937"/>
        <source>Property</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1940"/>
        <source>Value</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1823"/>
        <source>WMS Version</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2069"/>
        <source>Title</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2077"/>
        <source>Abstract</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1847"/>
        <source>Keywords</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1855"/>
        <source>Online Resource</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1863"/>
        <source>Contact Person</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1875"/>
        <source>Fees</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1883"/>
        <source>Access Constraints</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1891"/>
        <source>Image Formats</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1899"/>
        <source>Identify Formats</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1907"/>
        <source>Layer Count</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1927"/>
        <source>Layer Properties: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1945"/>
        <source>Selected</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2002"/>
        <source>Yes</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2002"/>
        <source>No</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1954"/>
        <source>Visibility</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1960"/>
        <source>Visible</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1961"/>
        <source>Hidden</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1962"/>
        <source>n/a</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1983"/>
        <source>Can Identify</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1991"/>
        <source>Can be Transparent</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1999"/>
        <source>Can Zoom In</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2007"/>
        <source>Cascade Count</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2015"/>
        <source>Fixed Width</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2023"/>
        <source>Fixed Height</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2031"/>
        <source>WGS 84 Bounding Box</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2041"/>
        <source>Available in CRS</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2052"/>
        <source>Available in style</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2061"/>
        <source>Name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2162"/>
        <source>Layer cannot be queried.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QuickPrintGui</name>
    <message>
        <location filename="../src/plugins/quick_print/quickprintgui.cpp" line="90"/>
        <source>Portable Document Format (*.pdf)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintgui.cpp" line="112"/>
        <source>quickprint</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintgui.cpp" line="113"/>
        <source>Unknown format: </source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QuickPrintGuiBase</name>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="13"/>
        <source>QGIS Quick Print Plugin</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="47"/>
        <source>Quick Print</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="57"/>
        <source>Map Title e.g. ACME inc.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="70"/>
        <source>Map Name e.g. Water Features</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="83"/>
        <source>Copyright</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="96"/>
        <source>Output</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="108"/>
        <source>Use last filename but incremented.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="115"/>
        <source>last used filename but incremented will be shown here</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="125"/>
        <source>Prompt for file name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="138"/>
        <source>Note: If you want more control over the map layout please use the map composer function in QGIS.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QuickPrintPlugin</name>
    <message>
        <location filename="../src/plugins/quick_print/quickprintplugin.cpp" line="75"/>
        <source>Quick Print</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintplugin.cpp" line="77"/>
        <source>Replace this with a short description of the what the plugin does</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintplugin.cpp" line="82"/>
        <source>&amp;Quick Print</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>RepositoryDetailsDialog</name>
    <message>
        <location filename="../python/plugins/plugin_installer/repository.ui" line="13"/>
        <source>Repository details</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/repository.ui" line="19"/>
        <source>Name:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/repository.ui" line="29"/>
        <source>URL:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/repository.ui" line="36"/>
        <source>http://</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>[pluginname]GuiBase</name>
    <message>
        <location filename="../src/plugins/plugin_template/pluginguibase.ui" line="13"/>
        <source>QGIS Plugin Template</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/pluginguibase.ui" line="47"/>
        <source>Plugin Template</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>pluginname</name>
    <message>
        <location filename="../src/plugins/plugin_template/plugin.cpp" line="73"/>
        <source>[menuitemname]</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugin.cpp" line="75"/>
        <source>Replace this with a short description of the what the plugin does</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugin.cpp" line="80"/>
        <source>&amp;[menuname]</source>
        <translation type="unfinished"></translation>
    </message>
</context>
</TS>
