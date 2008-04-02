<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS><TS version="1.1" language="hu">
<context>
    <name>@default</name>
    <message>
        <location filename="" line="145"/>
        <source>OGR Driver Manager</source>
        <translation type="obsolete">OGR Driver Manager
</translation>
    </message>
    <message>
        <location filename="" line="145"/>
        <source>unable to get OGRDriverManager</source>
        <translation type="obsolete">nem tudom elérni az OGRDriverManager-t</translation>
    </message>
</context>
<context>
    <name>Dialog</name>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="13"/>
        <source>QGIS Plugin Installer</source>
        <translation>QGIS modul installáló</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="19"/>
        <source>Select repository, retrieve the list of available plugins, select one and install it</source>
        <translation>Válaszd ki a tárházat, kérd le a modulok listáját, válassz egyet és installáld</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="26"/>
        <source>Repository</source>
        <translation>Tárház</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="34"/>
        <source>Active repository:</source>
        <translation>Aktív tárház:</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="61"/>
        <source>Get List</source>
        <translation>Lista</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="81"/>
        <source>Add</source>
        <translation>Hozzáad</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="88"/>
        <source>Edit</source>
        <translation>Szerkeszt</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="95"/>
        <source>Delete</source>
        <translation>Töröl</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="117"/>
        <source>Name</source>
        <translation>Név</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="122"/>
        <source>Version</source>
        <translation>Verzió</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="127"/>
        <source>Description</source>
        <translation>Leírás</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="132"/>
        <source>Author</source>
        <translation>Szerző</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="142"/>
        <source>Name of plugin to install</source>
        <translation>Installálandó modul neve</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="152"/>
        <source>Install Plugin</source>
        <translation>Modul installálás</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="163"/>
        <source>The plugin will be installed to ~/.qgis/python/plugins</source>
        <translation>A modult a ~/.qgis/python/plugins könyvtárba installálom</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="176"/>
        <source>Done</source>
        <translation>Kész</translation>
    </message>
</context>
<context>
    <name>Gui</name>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="55"/>
        <source>Welcome to your automatically generated plugin!</source>
        <translation>Üdvözlünk a automatikusan generált modulodban!</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="56"/>
        <source>This is just a starting point. You now need to modify the code to make it do something useful....read on for a more information to get yourself started.</source>
        <translation>Ez egy kezdőpont. Most módosítanod kell a kódot, hogy valami hasznosat csináljon... olvasd tovább, hogy felkészítsd magad.</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="57"/>
        <source>Documentation:</source>
        <translation>Dokumentáció:</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="58"/>
        <source>You really need to read the QGIS API Documentation now at:</source>
        <translation>Valóban el kell most olvasnod a QGIS API dokumentáció itt:</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="59"/>
        <source>In particular look at the following classes:</source>
        <translation>Különösen a következő osztályokat nézd meg:</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="62"/>
        <source>QgsPlugin is an ABC that defines required behaviour your plugin must provide. See below for more details.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="63"/>
        <source>What are all the files in my generated plugin directory for?</source>
        <translation>Mire való a modul mappába generált valamennyi fájl?</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="64"/>
        <source>This is the generated CMake file that builds the plugin. You should add you application specific dependencies and source files to this file.</source>
        <translation>Ez a generált CMake fájl, mely összeállítja a modult. Az alaklamazás specifikus függõségeket és forrás fájlokat add hozzá ehhez a fájlhoz.</translation>
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
        <translation>&lt;li&gt; a QGIS fejlesztői levelező lista vagy&lt;/li&gt;&lt;li&gt; IRC (#qgis on freenode.net)&lt;/li&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="74"/>
        <source>QGIS is distributed under the Gnu Public License. If you create a useful plugin please consider contributing it back to the community.</source>
        <translation>A QGIS-t a Gnu Public License alapján terjesztjük. Ha egy hasznos modult készítesz, gondolj arra, hogy visszajuttasd a közösségnek.</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="75"/>
        <source>Have fun and thank you for choosing QGIS.</source>
        <translation>Élvezd és köszönjük, hogy a QGIS-t választottad.</translation>
    </message>
</context>
<context>
    <name>MapCoordsDialogBase</name>
    <message>
        <location filename="../src/plugins/georeferencer/mapcoordsdialogbase.ui" line="13"/>
        <source>Enter map coordinates</source>
        <translation>Add meg a térkép koordinátákat</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/mapcoordsdialogbase.ui" line="28"/>
        <source>Enter X and Y coordinates which correspond with the selected point on the image. Alternatively, click the button with icon of a pencil and then click a corresponding point on map canvas of QGIS to fill in coordinates of that point.</source>
        <translation>Add meg a képen kiválaszott pont X és Y koordinátáit vagy a ceruza eszközzel jelöld ki a pontot a térképen a koordináták kitöltéséhez.</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/mapcoordsdialogbase.ui" line="62"/>
        <source>X:</source>
        <translation>X:</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/mapcoordsdialogbase.ui" line="69"/>
        <source>Y:</source>
        <translation>Y:</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/mapcoordsdialogbase.ui" line="137"/>
        <source> from map canvas</source>
        <translation>a térképről</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/mapcoordsdialogbase.ui" line="172"/>
        <source>&amp;Cancel</source>
        <translation>&amp;Mégsem</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/mapcoordsdialogbase.ui" line="185"/>
        <source>&amp;OK</source>
        <translation>&amp;OK</translation>
    </message>
</context>
<context>
    <name>QFileDialog</name>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="651"/>
        <source>Load layer properties from style file (.qml)</source>
        <translation>Réteg tulajdonságok betöltése stílus fájlból (.qml)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="713"/>
        <source>Save layer properties as style file (.qml)</source>
        <translation>Réteg tulajdonságok mentése stílus fájlba (.qml)</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintgui.cpp" line="104"/>
        <source>Save experiment report to portable document format (.pdf)</source>
        <translation>Jelentés mentése hordozható dokumentum formátumba (.pdf)</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2348"/>
        <source>Where is &apos;</source>
        <translation>Hol van &apos;</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2348"/>
        <source>original location: </source>
        <translation>eredeti hely:</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2921"/>
        <source>QGis files (*.qgs)</source>
        <translation>QGIS fájlok (*.qgs)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="41"/>
        <source>Not a vector layer</source>
        <translation>Nem vektor réteg</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="42"/>
        <source>The current layer is not a vector layer</source>
        <translation>Az aktuális réteg nem vektor réteg</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="58"/>
        <source>2.5D shape type not supported</source>
        <translation>2.5D-s shape típus nem támogatott</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="58"/>
        <source>Adding features to 2.5D shapetypes is not supported yet</source>
        <translation>A 2.5D-s shape típushoz még nem lehet új elemet hozzáadni</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="71"/>
        <source>Layer cannot be added to</source>
        <translation>Réteg nem adható ehhez</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="72"/>
        <source>The data provider for this layer does not support the addition of features.</source>
        <translation>Ennek a rétegnek a kezelõje nem támogadja az elemek hozzáadását.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="48"/>
        <source>Layer not editable</source>
        <translation>A réteg nem szerkeszthető</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="50"/>
        <source>Cannot edit the vector layer. To make it editable, go to the file item of the layer, right click and check &apos;Allow Editing&apos;.</source>
        <translation>Nem tudom szerkeszteni a vektor réteget. A szerkeszthetővé tételhez kattints jobb egérgombbal a réteg nevére és engedélyezd a szerkesztést.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="199"/>
        <source>Wrong editing tool</source>
        <translation>Rossz szerkesztőeszköz</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="91"/>
        <source>Cannot apply the &apos;capture point&apos; tool on this vector layer</source>
        <translation>Nem használhatod a pont digitalizálás eszközt ezen a vektor rétegen</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="64"/>
        <source>Coordinate transform error</source>
        <translation>Koordináta transzformáció hiba</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="65"/>
        <source>Cannot transform the point to the layers coordinate system</source>
        <translation>Nem tudom a pontot a réteg koordinátarendszerébe transzformálni</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="192"/>
        <source>Cannot apply the &apos;capture line&apos; tool on this vector layer</source>
        <translation>Nem használhatod a vonal digitalizálás eszközt ezen a vektor rétegen</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="200"/>
        <source>Cannot apply the &apos;capture polygon&apos; tool on this vector layer</source>
        <translation>Nem használhatod a felület digitalizálás eszközt ezen a vektor rétegen</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="419"/>
        <source>Error</source>
        <translation>Hiba</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="408"/>
        <source>Cannot add feature. Unknown WKB type</source>
        <translation>Nem tudom hozzáadni az elemet. Ismeretlen WKB típus</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="419"/>
        <source>Could not remove polygon intersection</source>
        <translation>Nem tudom megszüntetni a felületek metszését</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdisland.cpp" line="112"/>
        <source>Error, could not add island</source>
        <translation>Hiba, nem tudom a szigetet hozzáadni</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="90"/>
        <source>A problem with geometry type occured</source>
        <translation>Probléma a geometria elem típussal</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="94"/>
        <source>The inserted Ring is not closed</source>
        <translation>A beillesztett gyűrű nem zárt</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="98"/>
        <source>The inserted Ring is not a valid geometry</source>
        <translation>A beillesztett gyűrű geomeriája hibás</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="102"/>
        <source>The inserted Ring crosses existing rings</source>
        <translation>A beillesztett gyűrű metsz egy korábbit</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="106"/>
        <source>The inserted Ring is not contained in a feature</source>
        <translation>A beilesztett gyűrű nem esik bele egy elembe</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="110"/>
        <source>An unknown error occured</source>
        <translation>Ismeretlen hiba fordult elő</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="112"/>
        <source>Error, could not add ring</source>
        <translation>Hiba, nem sikerült a gyűrűt hozzáadni</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolselect.cpp" line="75"/>
        <source>No active layer</source>
        <translation>Nincs aktív réteg</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="122"/>
        <source>To identify features, you must choose an active layer by clicking on its name in the legend</source>
        <translation>Egy elem azonosításához egy aktív réteget kell kijelölnöd a nevére kattintva a jelkulcsban </translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="153"/>
        <source>Band</source>
        <translation>Sáv</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="453"/>
        <source>Length</source>
        <translation>Hossz</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="459"/>
        <source>Area</source>
        <translation>Terület</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="466"/>
        <source>action</source>
        <translation>művelet</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="475"/>
        <source> features found</source>
        <translation>elemet találtam</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="479"/>
        <source> 1 feature found</source>
        <translation>1 elemet találtam</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="485"/>
        <source>No features found</source>
        <translation>Nem találtam elemet</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="485"/>
        <source>No features were found in the active layer at the point you clicked</source>
        <translation>Az aktív rétegen a megadott pontban nem találtam elemeket</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="586"/>
        <source>Could not identify objects on</source>
        <translation>Nem sikerült elemeket azonosítani ezen</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="586"/>
        <source>because</source>
        <translation>mert</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolselect.cpp" line="76"/>
        <source>To select features, you must choose a vector layer by clicking on its name in the legend</source>
        <translation>Az elemek kiválasztásához egy aktív vektor réteget kell kijelölnöd a nevére kattintva a jelkulcsban</translation>
    </message>
    <message>
        <location filename="../src/app/qgspythonutils.cpp" line="400"/>
        <source>Python error</source>
        <translation>Phyton hiba</translation>
    </message>
    <message>
        <location filename="../src/app/qgspythonutils.cpp" line="57"/>
        <source>Couldn&apos;t load SIP module.
Python support will be disabled.</source>
        <translation>Nerm tudom betölteni a SIP modult. A Phyton támogatást kikapcsolom.</translation>
    </message>
    <message>
        <location filename="../src/app/qgspythonutils.cpp" line="67"/>
        <source>Couldn&apos;t load PyQt bindings.
Python support will be disabled.</source>
        <translation>Nerm tudom betölteni a PyQt kötéseket. A Phyton támogatást kikapcsolom.</translation>
    </message>
    <message>
        <location filename="../src/app/qgspythonutils.cpp" line="78"/>
        <source>Couldn&apos;t load QGIS bindings.
Python support will be disabled.</source>
        <translation>Nerm tudom betölteni a QGIS kötéseket. A Phyton támogatást kikapcsolom.</translation>
    </message>
    <message>
        <location filename="../src/app/qgspythonutils.cpp" line="380"/>
        <source>Couldn&apos;t load plugin </source>
        <translation>Nem tudom betölteni a modult</translation>
    </message>
    <message>
        <location filename="../src/app/qgspythonutils.cpp" line="369"/>
        <source> due an error when calling its classFactory() method</source>
        <translation>egy hiba miatt, amikor a ClassFactory() metodusát hívtam</translation>
    </message>
    <message>
        <location filename="../src/app/qgspythonutils.cpp" line="381"/>
        <source> due an error when calling its initGui() method</source>
        <translation>egy hiba miatt, amikor az initGui() metodusát hívtam</translation>
    </message>
    <message>
        <location filename="../src/app/qgspythonutils.cpp" line="401"/>
        <source>Error while unloading plugin </source>
        <translation>Hiba a modul kivétele során</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="311"/>
        <source>Caught a coordinate system exception while trying to transform a point. Unable to calculate line length.</source>
        <translation>Egy koordinátarendszer kivételt kaptam egy pont transzformálása során. Nem tudom kiszámolni a vonalhasszat.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="402"/>
        <source>Caught a coordinate system exception while trying to transform a point. Unable to calculate polygon area.</source>
        <translation type="unfinished">Egy koordinátarendszer kivételt kaptam egy pont transzformálása során. Nem tudom kiszámolni a területet.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="648"/>
        <source> km2</source>
        <translation>km2</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="653"/>
        <source> ha</source>
        <translation>ha</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="658"/>
        <source> m2</source>
        <translation>m2</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="686"/>
        <source> m</source>
        <translation>m</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="671"/>
        <source> km</source>
        <translation>km</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="676"/>
        <source> mm</source>
        <translation>mm</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="681"/>
        <source> cm</source>
        <translation>cm</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="695"/>
        <source> sq mile</source>
        <translation>mérföld2</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="700"/>
        <source> sq ft</source>
        <translation>láb2</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="707"/>
        <source> mile</source>
        <translation>mérföld</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="713"/>
        <source> foot</source>
        <translation>láb</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="715"/>
        <source> feet</source>
        <translation>láb</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="722"/>
        <source> sq.deg.</source>
        <translation> fok2</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="727"/>
        <source> degree</source>
        <translation>fok</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="729"/>
        <source> degrees</source>
        <translation>fok</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="733"/>
        <source> unknown</source>
        <translation>ismeretlen</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="273"/>
        <source>Received %1 of %2 bytes</source>
        <translation>%1 bájtot fogadtam a %2 bájtból</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="279"/>
        <source>Received %1 bytes (total unknown)</source>
        <translation>%1 bájtot fogadtam (összes ismeretlen)</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="390"/>
        <source>Not connected</source>
        <translation>Nincs kapcsolat</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="396"/>
        <source>Looking up &apos;%1&apos;</source>
        <translation>Keresem &apos;%1&apos;-t</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="403"/>
        <source>Connecting to &apos;%1&apos;</source>
        <translation>Kapcsolódás &apos;%1&apos;-hez</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="410"/>
        <source>Sending request &apos;%1&apos;</source>
        <translation>&apos;%1&apos; kérés küldése</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="417"/>
        <source>Receiving reply</source>
        <translation>Válasz fogadása</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="423"/>
        <source>Response is complete</source>
        <translation>Válasz kész</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="429"/>
        <source>Closing down connection</source>
        <translation>Kapcsolat lezárása</translation>
    </message>
    <message>
        <location filename="../src/core/qgslabelattributes.cpp" line="58"/>
        <source>Label</source>
        <translation>Cimke</translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="428"/>
        <source>Currently only filebased datasets are supported</source>
        <translation>Aktuálisan csak a fájl alapú adathalmazokat támogatom</translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="488"/>
        <source>Loaded default style file from </source>
        <translation>Alapértelmezett stílus betöltése ebből a fájlból</translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="508"/>
        <source>The directory containing your dataset needs to be writeable!</source>
        <translation>Az adathalnazodat tartalmazó könyvtárnak írhatónak kell lennie! </translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="532"/>
        <source>Created default style file as </source>
        <translation>Alapértelmezett stílus fájlt hoztam létre mint</translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="536"/>
        <source>ERROR: Failed to created default style file as </source>
        <translation>HIBA: Nem sikerült az alapértelmezett stílus fájlt létrehozni</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="753"/>
        <source>Unable to open </source>
        <translation>nem tudom megnyitni</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="771"/>
        <source>Project file read error: </source>
        <translation>Projekt fájl olvasási hiba:</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="771"/>
        <source> at line </source>
        <translation>
sorban</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="772"/>
        <source> column </source>
        <translation type="unfinished">oszlop</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="778"/>
        <source> for file </source>
        <translation>fájlhoz</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="934"/>
        <source>Unable to save to file </source>
        <translation>Nem tudom menteni a fájlba</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproviderregistry.cpp" line="87"/>
        <source>No Data Provider Plugins</source>
        <comment>No QGIS data provider plugins found in:</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/core/qgsproviderregistry.cpp" line="89"/>
        <source>No vector layers can be loaded. Check your QGIS installation</source>
        <translation>Nem tudok vektor rétegeket betölteni. Ellenőrizd a QGIS telepítést</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproviderregistry.cpp" line="92"/>
        <source>No Data Providers</source>
        <translation>Nincs kiszolgáló</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproviderregistry.cpp" line="251"/>
        <source>No data provider plugins are available. No vector layers can be loaded</source>
        <translation>Nincs kiszolgáló modul az adatokhoz. Nem tudok vektor rétegeket betölteni</translation>
    </message>
    <message>
        <location filename="../src/core/qgssearchtreenode.cpp" line="253"/>
        <source>Regular expressions on numeric values don&apos;t make sense. Use comparison instead.</source>
        <translation>Szabályos kifejezéseknek nincs értelme numerikus adatokon. Használj összehasonlítást helyette.</translation>
    </message>
    <message>
        <location filename="../src/core/qgssearchtreenode.cpp" line="289"/>
        <source>Referenced column wasn&apos;t found: </source>
        <translation>A hivatkozott oszlopot nem találom:</translation>
    </message>
    <message>
        <location filename="../src/core/qgssearchtreenode.cpp" line="293"/>
        <source>Division by zero.</source>
        <translation>Nullával osztás.</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="51"/>
        <source>CopyrightLabel</source>
        <translation>Szerzői jog cimke</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="52"/>
        <source>Draws copyright information</source>
        <translation>Szerzői jog információ rajzolás</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfsplugin.cpp" line="30"/>
        <source>Version 0.1</source>
        <translation>Verzió 0.1</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="44"/>
        <source>Version 0.2</source>
        <translation>Verzió 0.2</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="45"/>
        <source>Loads and displays delimited text files containing x,y coordinates</source>
        <translation>x,y koordinátakat tartalmazó szöveges fájl betöltése és megjelenítése</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="161"/>
        <source>Add Delimited Text Layer</source>
        <translation>Szöveg fájl réteg hozzáadás</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="47"/>
        <source>PostgreSQL Geoprocessing</source>
        <translation>PostgreSQL térinformatikai funkciók</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="48"/>
        <source>Geoprocessing functions for working with PostgreSQL/PostGIS layers</source>
        <translation>Térinformatikai funkciók a PostrgeSQL/PostGIS rétegekhez</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugin.cpp" line="57"/>
        <source>Georeferencer</source>
        <translation>Georeferálás</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugin.cpp" line="58"/>
        <source>Adding projection info to rasters</source>
        <translation>Vetületi információ hozzáadása raszterhez</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsleastsquares.cpp" line="32"/>
        <source>Fit to a linear transform requires at least 2 points.</source>
        <translation>Beillesztés lineáris transzformációval, 2 pont szükséges.</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsleastsquares.cpp" line="71"/>
        <source>Fit to a Helmert transform requires at least 2 points.</source>
        <translation>Beillesztés Helmert transzformációval, 2 pont szükséges.</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsleastsquares.cpp" line="123"/>
        <source>Fit to an affine transform requires at least 4 points.</source>
        <translation>Beillesztés affin transzformációval, 4 pont szükséges.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="54"/>
        <source>GPS Tools</source>
        <translation>GPS eszközök</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="56"/>
        <source>Tools for loading and importing GPS data</source>
        <translation>GPS adat betöltő és importáló eszközök</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="71"/>
        <source>New centroid</source>
        <translation>Új centrális</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="222"/>
        <source>New point</source>
        <translation>Új pont</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="133"/>
        <source>New vertex</source>
        <translation>Új töréspont</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="222"/>
        <source>Undo last point</source>
        <translation>Utolsó pont visszavonása</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="222"/>
        <source>Close line</source>
        <translation>Vonal lezárás</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="542"/>
        <source>Select vertex</source>
        <translation>Válassz töréspontot</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="295"/>
        <source>Select new position</source>
        <translation>Válassz új pozíciót</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="426"/>
        <source>Select line segment</source>
        <translation>Válassz vonalszakaszt</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="413"/>
        <source>New vertex position</source>
        <translation>Új töréspont pozíció</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="413"/>
        <source>Release</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="529"/>
        <source>Delete vertex</source>
        <translation>Töréspont törlés</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="529"/>
        <source>Release vertex</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="783"/>
        <source>Select element</source>
        <translation>Elem választás</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="596"/>
        <source>New location</source>
        <translation>Új hely</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="672"/>
        <source>Release selected</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="672"/>
        <source>Delete selected / select next</source>
        <translation>Szelektált törlése/következő szelektálása</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="735"/>
        <source>Select position on line</source>
        <translation>Válassz pozíciót a vonalon</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="753"/>
        <source>Split the line</source>
        <translation>Vonal darabolás</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="753"/>
        <source>Release the line</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="767"/>
        <source>Select point on line</source>
        <translation>Válassz egy pontot a vonalon</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="136"/>
        <source>Location: </source>
        <comment>Metadata in GRASS Browser</comment>
        <translation>Hely:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="136"/>
        <source>&lt;br&gt;Mapset: </source>
        <comment>Metadata in GRASS Browser</comment>
        <translation>&lt;br&gt;Térkép halmaz:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="140"/>
        <source>Location: </source>
        <translation>Hely:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="140"/>
        <source>&lt;br&gt;Mapset: </source>
        <translation>&lt;br&gt;Térkép halmaz:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="145"/>
        <source>&lt;b&gt;Raster&lt;/b&gt;</source>
        <translation>&lt;b&gt;Raszter&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="154"/>
        <source>Cannot open raster header</source>
        <translation>Nem tudom megnyitni a raszterfejlécet</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="158"/>
        <source>Rows</source>
        <translation>Sorok</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="159"/>
        <source>Columns</source>
        <translation>Oszlopok</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="160"/>
        <source>N-S resolution</source>
        <translation>É-D felbontás</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="161"/>
        <source>E-W resolution</source>
        <translation>K-NY felbontás</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="299"/>
        <source>North</source>
        <translation>Észak</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="301"/>
        <source>South</source>
        <translation>Dél</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="303"/>
        <source>East</source>
        <translation>Kelet</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="305"/>
        <source>West</source>
        <translation>Nyugat</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="188"/>
        <source>Format</source>
        <translation>Formátum</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="199"/>
        <source>Minimum value</source>
        <translation>Minimális érték</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="200"/>
        <source>Maximum value</source>
        <translation>Maximális érték</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="211"/>
        <source>Data source</source>
        <translation>Adatforrás</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="216"/>
        <source>Data description</source>
        <translation>Adat leírás</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="225"/>
        <source>Comments</source>
        <translation>Megjegyzések</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="240"/>
        <source>Categories</source>
        <translation>Kategóriák</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="346"/>
        <source>&lt;b&gt;Vector&lt;/b&gt;</source>
        <translation>&lt;b&gt;Vektor&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="273"/>
        <source>Points</source>
        <translation>Pontok</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="274"/>
        <source>Lines</source>
        <translation>Vonalak</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="275"/>
        <source>Boundaries</source>
        <translation>Határok</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="276"/>
        <source>Centroids</source>
        <translation>Centrálisok</translation>
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
        <translation>Területek</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="284"/>
        <source>Islands</source>
        <translation>Szigetek</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="308"/>
        <source>Top</source>
        <translation>Felül</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="309"/>
        <source>Bottom</source>
        <translation>Alul</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="312"/>
        <source>yes</source>
        <translation>igen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="312"/>
        <source>no</source>
        <translation>nem</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="319"/>
        <source>History&lt;br&gt;</source>
        <translation>Előzmények</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="347"/>
        <source>&lt;b&gt;Layer&lt;/b&gt;</source>
        <translation>&lt;b&gt;Réteg&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="366"/>
        <source>Features</source>
        <translation>Elem</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="375"/>
        <source>Driver</source>
        <translation>Meghajtó</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="376"/>
        <source>Database</source>
        <translation>Adatbázis</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="377"/>
        <source>Table</source>
        <translation>Tábla</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="378"/>
        <source>Key column</source>
        <translation>Kulcs oszlop</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="829"/>
        <source>GRASS</source>
        <translation>GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="835"/>
        <source>GRASS layer</source>
        <translation>GRASS réteg</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugin.cpp" line="43"/>
        <source>Graticule Creator</source>
        <translation>Rács létrehozás</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugin.cpp" line="44"/>
        <source>Builds a graticule</source>
        <translation>Rács készítés</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="60"/>
        <source>NorthArrow</source>
        <translation>Észak jel</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="61"/>
        <source>Displays a north arrow overlayed onto the map</source>
        <translation>Észak jel megjelenítése a térképen</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugin.cpp" line="38"/>
        <source>[menuitemname]</source>
        <translation>[menuitemname]</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugin.cpp" line="39"/>
        <source>[plugindescription]</source>
        <translation>[plugindescription]</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintplugin.cpp" line="38"/>
        <source>Quick Print</source>
        <translation>Gyors nyomtatás</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintplugin.cpp" line="40"/>
        <source>Quick Print is a plugin to quickly print a map with minimal effort.</source>
        <translation>A gyors nyomtatás egy modul a térkép legkisebb erőfeszítéssel történő nyomtatásához.</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="62"/>
        <source>ScaleBar</source>
        <translation>Lépték</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="63"/>
        <source>Draws a scale bar</source>
        <translation>Lépték rajzolás</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitplugin.cpp" line="37"/>
        <source>SPIT</source>
        <translation>SPIT</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitplugin.cpp" line="38"/>
        <source>Shapefile to PostgreSQL/PostGIS Import Tool</source>
        <translation>Shape fálj importáló eszköz PostgeSQL/PostGIS-hez</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfsplugin.cpp" line="28"/>
        <source>WFS plugin</source>
        <translation>WFS modul</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfsplugin.cpp" line="29"/>
        <source>Adds WFS layers to the QGIS canvas</source>
        <translation>WFS réteg hozzáadása a QGIS térképhez</translation>
    </message>
    <message>
        <location filename="../src/providers/gpx/gpsdata.cpp" line="331"/>
        <source>Couldn&apos;t open the data source: </source>
        <translation>Nem tudom megnyitni ezt az adatforrást:</translation>
    </message>
    <message>
        <location filename="../src/providers/gpx/gpsdata.cpp" line="353"/>
        <source>Parse error at line </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/gpx/qgsgpxprovider.cpp" line="54"/>
        <source>GPS eXchange format provider</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="165"/>
        <source>GRASS plugin</source>
        <translation>GRASS modul</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="135"/>
        <source>QGIS couldn&apos;t find your GRASS installation.
Would you like to specify path (GISBASE) to your GRASS installation?</source>
        <translation>A QGIS nem találja a GRASS-t. Meg szeretnéd adni a könyvtárat (GISBASE) a GRASS installációdhoz?</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="149"/>
        <source>Choose GRASS installation path (GISBASE)</source>
        <translation>Válaszd ki a GRASS telepítési könyvtárat (GISBASE)</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="166"/>
        <source>GRASS data won&apos;t be available if GISBASE is not specified.</source>
        <translation>GRASS adatok nem érhetők el, ha a GISBASE nincs beállítva.</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="436"/>
        <source>GISBASE is not set.</source>
        <translation>GISBASE nincs beállítva.</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="441"/>
        <source> is not a GRASS mapset.</source>
        <translation>nem GRASS térkép halmaz.</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="463"/>
        <source>Cannot start </source>
        <translation>Nem tudom elindítani</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="480"/>
        <source>Mapset is already in use.</source>
        <translation>A térkép halmazt használja valaki.</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="495"/>
        <source>Temporary directory </source>
        <translation>Ideiglenes könyvtár</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="495"/>
        <source> exist but is not writable</source>
        <translation>létezik, de nem írható</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="501"/>
        <source>Cannot create temporary directory </source>
        <translation>Nem tudom létrehozni az ideiglenes könyvtárat</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="517"/>
        <source>Cannot create </source>
        <translation>Nem tudom létrehozni</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="594"/>
        <source>Cannot remove mapset lock: </source>
        <translation>Nem tudom eltávolítai a térkép halmaz zárolást:</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="1034"/>
        <source>Warning</source>
        <translation>Figyelmeztetés</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="982"/>
        <source>Cannot read raster map region</source>
        <translation>Nem tudom beolvasni a raszter térkép terjedelmét</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="999"/>
        <source>Cannot read vector map region</source>
        <translation>Nem tudom beolvasni a vektor térkép terjedelmét</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="1035"/>
        <source>Cannot read region</source>
        <translation>Nem tudom beolvasni a terjedelmet</translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="433"/>
        <source>File could not been opened.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgisApp</name>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="341"/>
        <source>Quantum GIS - </source>
        <translation>Quantum GIS -</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="309"/>
        <source>Checking database</source>
        <translation>Adatbázis ellenőrzés</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="315"/>
        <source>Reading settings</source>
        <translation>Beállítások beolvasása</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="319"/>
        <source>Setting up the GUI</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="354"/>
        <source>Checking provider plugins</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="359"/>
        <source>Starting Python</source>
        <translation>Python indítása</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="369"/>
        <source>Restoring loaded plugins</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="373"/>
        <source>Initializing file filters</source>
        <translation>Fájl szűrők inicializálása</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="398"/>
        <source>Restoring window state</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="402"/>
        <source>QGIS Ready!</source>
        <translation>QGIS kész!</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="513"/>
        <source>&amp;New Project</source>
        <translation>&amp;Új projekt</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="514"/>
        <source>Ctrl+N</source>
        <comment>New Project</comment>
        <translation type="unfinished">Ctrl+Ú</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="515"/>
        <source>New Project</source>
        <translation>Új projekt</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="518"/>
        <source>&amp;Open Project...</source>
        <translation>Projekt &amp;nyitás...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="519"/>
        <source>Ctrl+O</source>
        <comment>Open a Project</comment>
        <translation>Ctrl+N</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="520"/>
        <source>Open a Project</source>
        <translation>Projekt nyitás</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="523"/>
        <source>&amp;Save Project</source>
        <translation>Projekt &amp;mentés</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="524"/>
        <source>Ctrl+S</source>
        <comment>Save Project</comment>
        <translation>Ctrl+m</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="525"/>
        <source>Save Project</source>
        <translation>Projekt mentés</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="528"/>
        <source>Save Project &amp;As...</source>
        <translation>Projekt mentés m&amp;ásként...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="529"/>
        <source>Ctrl+A</source>
        <comment>Save Project under a new name</comment>
        <translation>Ctrl+Á</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="530"/>
        <source>Save Project under a new name</source>
        <translation>Projekt mentés más néven</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="533"/>
        <source>&amp;Print...</source>
        <translation>N&amp;yomtatás...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="534"/>
        <source>Ctrl+P</source>
        <comment>Print</comment>
        <translation>Ctrl+Y</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="535"/>
        <source>Print</source>
        <translation>Nyomtatás</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="538"/>
        <source>Save as Image...</source>
        <translation type="unfinished">Mentés &amp;képként...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="539"/>
        <source>Ctrl+I</source>
        <comment>Save map as image</comment>
        <translation>Ctrl+K</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="540"/>
        <source>Save map as image</source>
        <translation>Térkép mentése képként</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="551"/>
        <source>Exit</source>
        <translation>Kilépés</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="552"/>
        <source>Ctrl+Q</source>
        <comment>Exit QGIS</comment>
        <translation>Ctrl+Q</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="553"/>
        <source>Exit QGIS</source>
        <translation>Kilépés a QGIS-ből</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="558"/>
        <source>Add a Vector Layer...</source>
        <translation>Vektor réteg hozzáadás...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="559"/>
        <source>V</source>
        <comment>Add a Vector Layer</comment>
        <translation>V</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="560"/>
        <source>Add a Vector Layer</source>
        <translation>Vektor réteg hozááadása</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="563"/>
        <source>Add a Raster Layer...</source>
        <translation>Raszter réteg hozzáadás...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="564"/>
        <source>R</source>
        <comment>Add a Raster Layer</comment>
        <translation>R</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="565"/>
        <source>Add a Raster Layer</source>
        <translation>Raszter réteg hozzáadása</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="568"/>
        <source>Add a PostGIS Layer...</source>
        <translation>PostGIS réteg hozzáadás...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="569"/>
        <source>D</source>
        <comment>Add a PostGIS Layer</comment>
        <translation>D</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="570"/>
        <source>Add a PostGIS Layer</source>
        <translation>PostGIS réteg hozzáadása</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="580"/>
        <source>New Vector Layer...</source>
        <translation>Új vektor réteg...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="581"/>
        <source>N</source>
        <comment>Create a New Vector Layer</comment>
        <translation>N</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="582"/>
        <source>Create a New Vector Layer</source>
        <translation>Új vektor réteg létrehozása</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="585"/>
        <source>Remove Layer</source>
        <translation>Réteg törlés</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="586"/>
        <source>Ctrl+D</source>
        <comment>Remove a Layer</comment>
        <translation>Ctrl+D</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="587"/>
        <source>Remove a Layer</source>
        <translation>Egy réteg törlése</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="590"/>
        <source>Add All To Overview</source>
        <translation>Mindent hozzáad az áttekintőhöz</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="591"/>
        <source>+</source>
        <comment>Show all layers in the overview map</comment>
        <translation>+</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="592"/>
        <source>Show all layers in the overview map</source>
        <translation>Minden réteg megjelenítése az áttekintő térképen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="595"/>
        <source>Remove All From Overview</source>
        <translation>Mindent kivesz az áttekintőből</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="596"/>
        <source>-</source>
        <comment>Remove all layers from overview map</comment>
        <translation>-</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="597"/>
        <source>Remove all layers from overview map</source>
        <translation>Minden réteg kivétele az áttekintő térképből</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="606"/>
        <source>Show All Layers</source>
        <translation>Minden réteget mutat</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="607"/>
        <source>S</source>
        <comment>Show all layers</comment>
        <translation>S</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="608"/>
        <source>Show all layers</source>
        <translation>Minden réteget mutat</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="611"/>
        <source>Hide All Layers</source>
        <translation>Minden réteget elrejt</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="612"/>
        <source>H</source>
        <comment>Hide all layers</comment>
        <translation>H</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="613"/>
        <source>Hide all layers</source>
        <translation>Minden réteget elrejt</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="618"/>
        <source>Project Properties...</source>
        <translation>Projekt tulajdonságok...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="619"/>
        <source>P</source>
        <comment>Set project properties</comment>
        <translation>P</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="620"/>
        <source>Set project properties</source>
        <translation>Projekt tulajdonságok beállítása</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="623"/>
        <source>Options...</source>
        <translation>Opciók...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="625"/>
        <source>Change various QGIS options</source>
        <translation>Különböző QGIS beállítások módosítása</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="628"/>
        <source>Custom Projection...</source>
        <translation>Egyedi vetület...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="630"/>
        <source>Manage custom projections</source>
        <translation>Egyedi vetület menedzselés</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="635"/>
        <source>Help Contents</source>
        <translation>Súgó tartalom</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="637"/>
        <source>Ctrl+?</source>
        <comment>Help Documentation (Mac)</comment>
        <translation>Ctrl+?</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="639"/>
        <source>F1</source>
        <comment>Help Documentation</comment>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="641"/>
        <source>Help Documentation</source>
        <translation>Súgó</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="644"/>
        <source>Qgis Home Page</source>
        <translation>Qgis honlap</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="646"/>
        <source>Ctrl+H</source>
        <comment>QGIS Home Page</comment>
        <translation>Ctlr+H</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="648"/>
        <source>QGIS Home Page</source>
        <translation>QGIS honlap</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="651"/>
        <source>About</source>
        <translation>Névjegy</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="652"/>
        <source>About QGIS</source>
        <translation>QGIS névjegye</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="655"/>
        <source>Check Qgis Version</source>
        <translation>Qgis verzió ellenőrzés</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="656"/>
        <source>Check if your QGIS version is up to date (requires internet access)</source>
        <translation>QGIS verzió aktualitásának ellenőrzése (internet kapcsolat szükséges)</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="661"/>
        <source>Refresh</source>
        <translation>Firssítés</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="662"/>
        <source>Ctrl+R</source>
        <comment>Refresh Map</comment>
        <translation>Ctrl+R</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="663"/>
        <source>Refresh Map</source>
        <translation>Térkép újrarajzolás</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="668"/>
        <source>Zoom In</source>
        <translation>Nagyítás</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="667"/>
        <source>Ctrl++</source>
        <comment>Zoom In</comment>
        <translation>Ctrl++</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="673"/>
        <source>Zoom Out</source>
        <translation>Kicsinyítés</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="672"/>
        <source>Ctrl+-</source>
        <comment>Zoom Out</comment>
        <translation>Ctrl+-</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="676"/>
        <source>Zoom Full</source>
        <translation>Teljes nagyítás</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="677"/>
        <source>F</source>
        <comment>Zoom to Full Extents</comment>
        <translation>F</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="678"/>
        <source>Zoom to Full Extents</source>
        <translation>Teljes terjedelemre nagyítás</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="681"/>
        <source>Zoom To Selection</source>
        <translation>Szelekcióra nagyítás</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="682"/>
        <source>Ctrl+F</source>
        <comment>Zoom to selection</comment>
        <translation>Ctrl+F</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="683"/>
        <source>Zoom to selection</source>
        <translation>Szelekcióra nagyítás</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="686"/>
        <source>Pan Map</source>
        <translation>Térkép eltolás</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="687"/>
        <source>Pan the map</source>
        <translation>A térkép eltolása</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="690"/>
        <source>Zoom Last</source>
        <translation>Előző nagyítás</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="692"/>
        <source>Zoom to Last Extent</source>
        <translation>Nagyítás az utolsó terjedelemre</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="695"/>
        <source>Zoom To Layer</source>
        <translation>Rétegre nagyítás</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="697"/>
        <source>Zoom to Layer</source>
        <translation>Rétegre nagyítás</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="700"/>
        <source>Identify Features</source>
        <translation>Elem azonosítás</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="701"/>
        <source>I</source>
        <comment>Click on features to identify them</comment>
        <translation>I</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="702"/>
        <source>Click on features to identify them</source>
        <translation>Katiins az elemre az azonosításhoz</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="706"/>
        <source>Select Features</source>
        <translation>Elem szelektálás</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="712"/>
        <source>Open Table</source>
        <translation>Tábla nyitás</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="716"/>
        <source>Measure Line </source>
        <translation>Hosszmérés</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="717"/>
        <source>Ctrl+M</source>
        <comment>Measure a Line</comment>
        <translation>Ctrl+M</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="718"/>
        <source>Measure a Line</source>
        <translation>Hosszmérés</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="721"/>
        <source>Measure Area</source>
        <translation>Területmérés</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="722"/>
        <source>Ctrl+J</source>
        <comment>Measure an Area</comment>
        <translation>Ctrl+J</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="723"/>
        <source>Measure an Area</source>
        <translation>Egy terület lemérése</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="728"/>
        <source>Show Bookmarks</source>
        <translation>Könyvjelzők megjelenítése</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="727"/>
        <source>B</source>
        <comment>Show Bookmarks</comment>
        <translation>B</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="733"/>
        <source>Show most toolbars</source>
        <translation>Eszköztárak megjelenítése</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="732"/>
        <source>T</source>
        <comment>Show most toolbars</comment>
        <translation>T</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="739"/>
        <source>Hide most toolbars</source>
        <translation>Eszköztárak elrejtése</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="738"/>
        <source>Ctrl+T</source>
        <comment>Hide most toolbars</comment>
        <translation>Ctrl+T</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="743"/>
        <source>New Bookmark...</source>
        <translation>Új könyvjelző...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="744"/>
        <source>Ctrl+B</source>
        <comment>New Bookmark</comment>
        <translation>Ctrl+B</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5362"/>
        <source>New Bookmark</source>
        <translation>Új könyvjelző</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="748"/>
        <source>Add WMS Layer...</source>
        <translation>WMS réteg hozzáadás...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="749"/>
        <source>W</source>
        <comment>Add Web Mapping Server Layer</comment>
        <translation>W</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="750"/>
        <source>Add Web Mapping Server Layer</source>
        <translation>Réteg hozzáadás webes térképszerverről</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="753"/>
        <source>In Overview</source>
        <translation>Áttekintőbe</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="754"/>
        <source>O</source>
        <comment>Add current layer to overview map</comment>
        <translation>O</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="755"/>
        <source>Add current layer to overview map</source>
        <translation>Aktuális réteg hozzáadás az áttekintő térképhez</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="760"/>
        <source>Plugin Manager...</source>
        <translation>Modul menedzser...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="762"/>
        <source>Open the plugin manager</source>
        <translation>Modul menedzser megnyitása</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="773"/>
        <source>Toggle editing</source>
        <translation>Szerkesztés be/ki</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="774"/>
        <source>Toggles the editing state of the current layer</source>
        <translation>A réteg szerkeszthetõség be/kikapcsolása</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="779"/>
        <source>Capture Point</source>
        <translation>Pont digitalizálás</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="780"/>
        <source>.</source>
        <comment>Capture Points</comment>
        <translation>.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="781"/>
        <source>Capture Points</source>
        <translation>Pont digitalizálás</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="785"/>
        <source>Capture Line</source>
        <translation>Vonal digitalizálás</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="786"/>
        <source>/</source>
        <comment>Capture Lines</comment>
        <translation>/</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="787"/>
        <source>Capture Lines</source>
        <translation>Vonal digitalizálás</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="791"/>
        <source>Capture Polygon</source>
        <translation>Felület digitalizálás</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="792"/>
        <source>Ctrl+/</source>
        <comment>Capture Polygons</comment>
        <translation>Ctrl+/</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="793"/>
        <source>Capture Polygons</source>
        <translation>Felület digitalizálás</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="798"/>
        <source>Delete Selected</source>
        <translation>Szelektáltak törlése</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="803"/>
        <source>Move Feature</source>
        <translation>Elem mozgatása</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="808"/>
        <source>Split Features</source>
        <translation>Elemek darabolása</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="813"/>
        <source>Add Vertex</source>
        <translation>Töréspont hozzáadás</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="818"/>
        <source>Delete Vertex</source>
        <translation>Töréspont törlés</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="823"/>
        <source>Move Vertex</source>
        <translation>Töréspont mozgatás</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="828"/>
        <source>Add Ring</source>
        <translation>Gyűrű hozzáadás</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="832"/>
        <source>Add Island</source>
        <translation>Sziget hozzáadás</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="833"/>
        <source>Add Island to multipolygon</source>
        <translation>Sziget hozzáadása összetett felülethez</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="837"/>
        <source>Cut Features</source>
        <translation>Elemek vágólapra mozgatása</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="838"/>
        <source>Cut selected features</source>
        <translation>Szelektált elemek vágólapra mozgatása</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="842"/>
        <source>Copy Features</source>
        <translation>Elemek vágólapra másolása</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="843"/>
        <source>Copy selected features</source>
        <translation>Szelektált elemek másolása vágólapra</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="847"/>
        <source>Paste Features</source>
        <translation>Elemek beillesztése vágólapról</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="848"/>
        <source>Paste selected features</source>
        <translation>Szelektált elemek beillesztése vágólapról</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="853"/>
        <source>Map Tips</source>
        <translation>Térkép tippek</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="854"/>
        <source>Show information about a feature when the mouse is hovered over it</source>
        <translation>Információ mergjelenítése az elemről amikor az egér felette áll</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="859"/>
        <source>Python console</source>
        <translation>Phyton konzol</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="921"/>
        <source>&amp;File</source>
        <translation>&amp;Fájl</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="924"/>
        <source>&amp;Open Recent Projects</source>
        <translation>K&amp;orábbi projekt megnyitása</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="944"/>
        <source>&amp;View</source>
        <translation>&amp;Nézet</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="963"/>
        <source>&amp;Layer</source>
        <translation>&amp;Réteg</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="983"/>
        <source>&amp;Settings</source>
        <translation>&amp;Beállítások</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="990"/>
        <source>&amp;Plugins</source>
        <translation>&amp;Modulok</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1006"/>
        <source>&amp;Help</source>
        <translation>&amp;Súgó</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1025"/>
        <source>File</source>
        <translation>Fájl</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1036"/>
        <source>Manage Layers</source>
        <translation>Réteg kezelés</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1052"/>
        <source>Help</source>
        <translation>Súgó</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1059"/>
        <source>Digitizing</source>
        <translation>Digitalizálás</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1079"/>
        <source>Map Navigation</source>
        <translation>Térkép navigáció</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1092"/>
        <source>Attributes</source>
        <translation>Attribútumok</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1105"/>
        <source>Plugins</source>
        <translation>Modulok</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1115"/>
        <source>Toolbar Visibility...</source>
        <translation>Eszköztár láthatóság...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1129"/>
        <source>Progress bar that displays the status of rendering layers and other time-intensive operations</source>
        <translation>Elõrehaladás jelzõ, mely a réteg rajzolás és más idõigényes mûveletek státuszát mutatja</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1142"/>
        <source>Scale </source>
        <translation>Méretarány</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1143"/>
        <source>Current map scale</source>
        <translation>Aktuális térképi méretarány</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1155"/>
        <source>Displays the current map scale</source>
        <translation>Aktuális térképi méretarány megjelenítése</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1156"/>
        <source>Current map scale (formatted as x:y)</source>
        <translation>Aktuális térképi méretarány (x:y formában)</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1166"/>
        <source>Shows the map coordinates at the current cursor position. The display is continuously updated as the mouse is moved.</source>
        <translation>Az aktuális egér pozíció koordinátáinak megjelenítése. A megjelenítés folyamatosan változik, ahogy az egér mozog.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1167"/>
        <source>Map coordinates at mouse cursor position</source>
        <translation>Tréképi koordináták az egér pozícióban</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1170"/>
        <source>Render</source>
        <translation>Megjelenít</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1173"/>
        <source>When checked, the map layers are rendered in response to map navigation commands and other events. When not checked, no rendering is done. This allows you to add a large number of layers and symbolize them before rendering.</source>
        <translation>Amikor bekapcsolod a térképi rétegek megjelennek a navigációs parancsok és más események után. Amikor kikapcsolod nincs újrarajzolás. Ez lehetővé teszi, hogy sok rétegek és azok tematikus megjelenítését állítsd be folyamatos újrarajzolás nélkül.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1174"/>
        <source>Toggle map rendering</source>
        <translation>Térkép rajzolás be/ki</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1189"/>
        <source>This icon shows whether on the fly projection is enabled or not. Click the icon to bring up the project properties dialog to alter this behaviour.</source>
        <translation>Ez az ikon jelzi, hogy a röptében vetítés engedélyezett-e. Kattints az ikonra a projekt tulajdonságok megjelenítéséhez és ennek a viselkedésnek a módosításához.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1190"/>
        <source>Projection status - Click to open projection dialog</source>
        <translation>Vetület állapot - Kattints ide a vetület párbeszédablak megnyitásához</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1194"/>
        <source>Ready</source>
        <translation>Kész</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1314"/>
        <source>Map canvas. This is where raster and vector layers are displayed when added to the map</source>
        <translation>Térkép. Ez az a hely ahol a raszter és vektor rétegek megjelennek amikor a térképhez adod</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1363"/>
        <source>Map overview canvas. This canvas can be used to display a locator map that shows the current extent of the map canvas. The current extent is shown as a red rectangle. Any layer on the map can be added to the overview canvas.</source>
        <translation>Térkép áttekintő. Az áttekintõn megjelenik a térkép ablak helyzete, ezt egy piros téglalap jelzi. Bármelyik a térképen megjelenõ réteg hozzáadható az áttekintõhöz.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1396"/>
        <source>Map legend that displays all the layers currently on the map canvas. Click on the check box to turn a layer on or off. Double click on a layer in the legend to customize its appearance and set other properties.</source>
        <translation>A jelmagyarazázatban megjelenik az összes a térképen szereplõ réteg. Kattints a négyzetbe a rétek be vagy kikapcsolásához. Kattints duplán a rétegre a megjelenítés és más tulajdonságok beállításához.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1529"/>
        <source>Version </source>
        <translation>Verzió </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1536"/>
        <source> with PostgreSQL support</source>
        <translation> PostgreSQL támogatással</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1539"/>
        <source> (no PostgreSQL support)</source>
        <translation> (PostgreSQL támogatás nélkül)</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1541"/>
        <source>
Compiled against Qt </source>
        <translation>
Qt-vel fordítva </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1542"/>
        <source>, running against Qt </source>
        <translation>, Qt-vel futtatva </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1552"/>
        <source>Quantum GIS is licensed under the GNU General Public License</source>
        <translation>A Quantum GIS-re a GNU General Public License érvényes</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1554"/>
        <source>http://www.gnu.org/licenses</source>
        <translation>http://www.gnu.org/licenses</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1556"/>
        <source>Version</source>
        <translation>Verzió</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1558"/>
        <source>New features</source>
        <translation>Új elemek</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1561"/>
        <source>This release candidate includes over 40 bug fixes and enchancements over the QGIS 0.9.1 release. In addition we have added the following new features:</source>
        <translation>Ez a kibocsátás elõtti verzió több mint 40 hibajavítást és bõvítéseket tartalmaz a QGIS 0.9.1 verzióhoz képest. Emellett a következõ új funkciókat készítettük el:</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1564"/>
        <source>Imrovements to digitising capabilities.</source>
        <translation>A digitalizálási lehetõségek tökéletesítése.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1570"/>
        <source>Supporting default and defined styles (.qml) files for file based vector layers. With styles you can save the symbolisation and other settings associated with a vector layer and they will be loaded whenever you load that layer.</source>
        <translation>Az alapértelmezett és beállított stílus fájlok (.qml) támogatása fájl alapú vektor rétegekhez. A stílussal elmentheted a vektor réteghez tartozó tematikus és egyéb beállításokat, melyeket a rendszer automatikusan betölt a réteggel együtt.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1576"/>
        <source>Improved support for transparency and contrast stretching in raster layers. Support for color ramps in raster layers. Support for non-north up rasters. Many other raster improvements &apos;under the hood&apos;.</source>
        <translation>A raszter rétegek átlátszóságának és kontraszt növelésének tökéletesítése. Szín átmenetek támogatása raszter rétegekhez. Nem északra tájolt raszterek kezelése. Több további raszter fejlesztést tetõ alá hoztunk.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1586"/>
        <source>Available Data Provider Plugins</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1685"/>
        <source>Python error</source>
        <translation type="unfinished">Phyton hiba</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1685"/>
        <source>Error when reading metadata of plugin </source>
        <translation>Hiba a modul meta adatok betöltése során </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1948"/>
        <source>Open an OGR Supported Vector Layer</source>
        <translation>Vektor réteg megnyitása OGR támogatással</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2002"/>
        <source>is not a valid or recognized data source</source>
        <translation>érvénytelen vagy ismeretlen adatforrás</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5110"/>
        <source>Invalid Data Source</source>
        <translation>Hibás adatforrás</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2094"/>
        <source>Invalid Layer</source>
        <translation>Hibás réteg</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2094"/>
        <source>%1 is an invalid layer and cannot be loaded.</source>
        <translation>%1 hibás réteg és nem tudom betölteni.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2575"/>
        <source>Save As</source>
        <translation>Mentés másként</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2675"/>
        <source>Choose a QGIS project file to open</source>
        <translation>Válaszd ki a megnyitandó QGIS projektet</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2790"/>
        <source>QGIS Project Read Error</source>
        <translation>QGIS projekt olvasási hiba</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2792"/>
        <source>Try to find missing layers?</source>
        <translation>Megpróbálod megkeresni a hiányzó réteget?</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2808"/>
        <source>Unable to open project</source>
        <translation>Nem tudom megnyitni a projektet</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2847"/>
        <source>Choose a QGIS project file</source>
        <translation>Válassz egy QGIS projekt fájlt</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2964"/>
        <source>Saved project to:</source>
        <translation>Projekt mentés:</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2971"/>
        <source>Unable to save project</source>
        <translation>Nem tudom menteni a projektet</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2972"/>
        <source>Unable to save project to </source>
        <translation>Nem tudom menteni a projektet </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2978"/>
        <source>Unable to save project </source>
        <translation>Nem tudom menteni a projektet </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2920"/>
        <source>Choose a filename to save the QGIS project file as</source>
        <translation>Válassz egy fájlnevet a QGIS projekt fájlhoz</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3036"/>
        <source>QGIS: Unable to load project</source>
        <translation>QGIS: nem tudom betölteni a projektet</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3037"/>
        <source>Unable to load project </source>
        <translation>Nem tudom betölteni a projektet </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3160"/>
        <source>Choose a filename to save the map image as</source>
        <translation>Válassz egy fájlnevet a kép mentéséhez</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3202"/>
        <source>Saved map image to</source>
        <translation>Kép mentés</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3392"/>
        <source>No Layer Selected</source>
        <translation>Nincs szelektált réteg</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3393"/>
        <source>To delete features, you must select a vector layer in the legend</source>
        <translation>Egy rétegek kell szelektálnod a jelmagyarázatban az elemek törléséhez</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3400"/>
        <source>No Vector Layer Selected</source>
        <translation>Nincs szelektált vektor réteg</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3401"/>
        <source>Deleting features only works on vector layers</source>
        <translation>Az elemek törlése csak vektor rétegben lehetséges</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3407"/>
        <source>Provider does not support deletion</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3408"/>
        <source>Data provider does not support deleting features</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3414"/>
        <source>Layer not editable</source>
        <translation>A réteg nem szerkeszthető</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3415"/>
        <source>The current layer is not editable. Choose &apos;Start editing&apos; in the digitizing toolbar.</source>
        <translation>Az ektuális réteg nem szerkeszthetõ. Válaszd a szerkesztés kezdését a digitalizáló eszközsorban.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3422"/>
        <source>Problem deleting features</source>
        <translation>Probléma az elem törlése közben</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3423"/>
        <source>A problem occured during deletion of features</source>
        <translation>Probléma fordult elõ az elemek törlése közben</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3685"/>
        <source>Invalid scale</source>
        <translation>Hibás méretarány</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3921"/>
        <source>Error Loading Plugin</source>
        <translation>Hiba a modul betöltése közben</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3921"/>
        <source>There was an error loading %1.</source>
        <translation>Hiba történt a %1 betöltése közben.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3957"/>
        <source>No MapLayer Plugins</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3957"/>
        <source>No MapLayer plugins in ../plugins/maplayer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4034"/>
        <source>No Plugins</source>
        <translation>Nincsenek modulok</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4035"/>
        <source>No plugins found in ../plugins. To test plugins, start qgis from the src directory</source>
        <translation>Nem találtam modulokat a ../plugins-ban. A modulok teszteléséhez indítsd a QGIS-t az src mappából</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4069"/>
        <source>Name</source>
        <translation>Név</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4069"/>
        <source>Plugin %1 is named %2</source>
        <translation>A(z) %1 modul neve %2</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4086"/>
        <source>Plugin Information</source>
        <translation>Modul információ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4087"/>
        <source>QGis loaded the following plugin:</source>
        <translation>A QGIS a következõ modulokat töltötte be:</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4087"/>
        <source>Name: %1</source>
        <translation>Név: %1</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4087"/>
        <source>Version: %1</source>
        <translation>Verzió: %1</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4088"/>
        <source>Description: %1</source>
        <translation>Leírás: %1</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4106"/>
        <source>Unable to Load Plugin</source>
        <translation>Nem tudom betölteni a modult</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4107"/>
        <source>QGIS was unable to load the plugin from: %1</source>
        <translation>A QGIUS nem tudta betölteni a modult innen: %1</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4163"/>
        <source>There is a new version of QGIS available</source>
        <translation>A QGIS egy új változata áll rendelkezésre</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4169"/>
        <source>You are running a development version of QGIS</source>
        <translation>A QGIS egy fejlesztés közbeni változatát futtatod</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4173"/>
        <source>You are running the current version of QGIS</source>
        <translation>A QGIS aktuális változatát futtatod</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4178"/>
        <source>Would you like more information?</source>
        <translation>Több információra van szükséged?</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4225"/>
        <source>QGIS Version Information</source>
        <translation>QGIS verzió információ</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4185"/>
        <source>QGIS - Changes in SVN Since Last Release</source>
        <translation>QGIS - Az utolsó változat óta változások történtek az SVN-ben</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4197"/>
        <source>Unable to get current version information from server</source>
        <translation>Nem tudom beszerezni a az aktuális változat információt a szerverrõl</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4211"/>
        <source>Connection refused - server may be down</source>
        <translation>Sikertelen kapcsolódás - a szervert lehet hogy leállították</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4214"/>
        <source>QGIS server was not found</source>
        <translation>QGIS szervert nem találtam</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4217"/>
        <source>Network error while communicating with server</source>
        <translation>Hálózati hiba a szerver kommunikáció közben</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4220"/>
        <source>Unknown network socket error</source>
        <translation>Ismeretlen hálózati socket hiba</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4225"/>
        <source>Unable to communicate with QGIS Version server</source>
        <translation>Nem sikerült a QGIS verzió szerverrel kommunikálni</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5196"/>
        <source>Layer is not valid</source>
        <translation>A réteg hibás</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5197"/>
        <source>The layer is not a valid layer and can not be added to the map</source>
        <translation>A réteg hibás és nem tudom a térképhez hozzáadni</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4436"/>
        <source>Save?</source>
        <translation>Mentés?</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4437"/>
        <source>Do you want to save the current project?</source>
        <translation>Menteni akarod az aktuális projektet?</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4623"/>
        <source>Extents: </source>
        <translation>Terjedelem: </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4973"/>
        <source>Clipboard contents set to: </source>
        <translation>Vágólap tartalom beállítás: </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5018"/>
        <source>Open a GDAL Supported Raster Data Source</source>
        <translation>Raszter adatforrás megnyitása GDAL-lal</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5109"/>
        <source> is not a valid or recognized raster data source</source>
        <translation> hibás vagy ismeretlen raszter adatforrás</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5275"/>
        <source> is not a supported raster data source</source>
        <translation> ismeretlen raszter adatforrás</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5276"/>
        <source>Unsupported Data Source</source>
        <translation>Nem támogatott adatforrás</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5363"/>
        <source>Enter a name for the new bookmark:</source>
        <translation>Add meg az új könyvjelzõ nevét:</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5380"/>
        <source>Error</source>
        <translation>Hiba</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5380"/>
        <source>Unable to create the bookmark. Your user database may be missing or corrupted</source>
        <translation>Nem sikerült a könyvjelzõ létrehozása. Hiányzik az adatbázisod vagy összezavarodott</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5415"/>
        <source>Project file is older</source>
        <translation>A projekt fájl régebbi</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5417"/>
        <source>&lt;p&gt;This project file was saved by an older version of QGIS.</source>
        <translation>&lt;p&gt;Ezt a projekt fájl a QGIS egy korábbi változatával mentették.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5419"/>
        <source> When saving this project file, QGIS will update it to the latest version, possibly rendering it useless for older versions of QGIS.</source>
        <translation> A projekt fájl mentése során a QGIS aktualizálja az újabb verzióhoz, valószinûleg nem tudod használni a QGIS korábbi verzióival.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5422"/>
        <source>&lt;p&gt;Even though QGIS developers try to maintain backwards compatibility, some of the information from the old project file might be lost.</source>
        <translation>&lt;p&gt;Bár a QGIS fejlesztõk igyekeznek megtartani a kompatibilitást a korábbi verziókkal, néhány információ elveszhet a régi projekt fájlból.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5424"/>
        <source> To improve the quality of QGIS, we appreciate if you file a bug report at %3.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5426"/>
        <source> Be sure to include the old project file, and state the version of QGIS you used to discover the error.</source>
        <translation> Gyõzõdj meg róla, hogy a régi projekt fájlt, a használt QGIS verziószámot mellékelted a hiba feltárás érdekében.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5428"/>
        <source>&lt;p&gt;To remove this warning when opening an older project file, uncheck the box &apos;%5&apos; in the %4 menu.</source>
        <translation>&lt;p&gt; A régebbi projekt fájlok megnyitása során kapott figyelmeztetés eltávolításához kapcsold ki a &apos;%5&apos;-t a %4 menüben.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5428"/>
        <source>&lt;p&gt;Version of the project file: %1&lt;br&gt;Current version of QGIS: %2</source>
        <translation>&lt;p&gt;A projekt fájl verziója: %1&lt;br&gt;Aktuális QGIS verzió: %2</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5432"/>
        <source>&lt;tt&gt;Settings:Options:General&lt;/tt&gt;</source>
        <comment>Menu path to setting options</comment>
        <translation>&lt;tt&gt;Beállítások:Opciók:Általános&lt;/tt&gt;</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5433"/>
        <source>Warn me when opening a project file saved with an older version of QGIS</source>
        <translation>Figyelmeztess amikor a QGIS egy korábbi verziójával mentett projektet nyitok meg</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="600"/>
        <source>Toggle full screen mode</source>
        <translation>Teljes képernyőre váltás</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="601"/>
        <source>Ctrl-F</source>
        <comment>Toggle fullscreen mode</comment>
        <translation>Ctrl-F</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="602"/>
        <source>Toggle fullscreen mode</source>
        <translation>Teljes képernyõre váltás</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2791"/>
        <source></source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgisAppBase</name>
    <message>
        <location filename="../src/ui/qgisappbase.ui" line="13"/>
        <source>MainWindow</source>
        <translation>Fő ablak</translation>
    </message>
    <message>
        <location filename="../src/ui/qgisappbase.ui" line="102"/>
        <source>Legend</source>
        <translation>Jelmagyarázat</translation>
    </message>
    <message>
        <location filename="../src/ui/qgisappbase.ui" line="135"/>
        <source>Map View</source>
        <translation>Térkép</translation>
    </message>
</context>
<context>
    <name>QgsAbout</name>
    <message>
        <location filename="../src/app/qgsabout.cpp" line="54"/>
        <source>Quantum GIS is licensed under the GNU General Public License</source>
        <translation type="unfinished">A Quantum GIS-re a GNU General Public License érvényes</translation>
    </message>
    <message>
        <location filename="../src/app/qgsabout.cpp" line="118"/>
        <source>QGIS Sponsors</source>
        <translation type="unfinished">QGIS szponzorok</translation>
    </message>
    <message>
        <location filename="../src/app/qgsabout.cpp" line="125"/>
        <source>Name</source>
        <translation type="unfinished">Név</translation>
    </message>
    <message>
        <location filename="../src/app/qgsabout.cpp" line="125"/>
        <source>Website</source>
        <translation type="unfinished">Honlap</translation>
    </message>
    <message>
        <location filename="../src/app/qgsabout.cpp" line="255"/>
        <source>QGIS Browser Selection</source>
        <translation type="unfinished">QGIS böngésző szelektálás</translation>
    </message>
    <message>
        <location filename="../src/app/qgsabout.cpp" line="259"/>
        <source>Enter the name of a web browser to use (eg. konqueror).
Enter the full path if the browser is not in your PATH.
You can change this option later by selection Options from the Settings menu (Help Browser tab).</source>
        <translation type="unfinished">Add meg a használandó böngésző program nevét (pl. konqueror). A teljes elérési utat add meg, ha a böngészó program nincs a PATH-on. Később a Beállítások menü Opciók pontjával módosíthatod ezt.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="13"/>
        <source>About Quantum GIS</source>
        <translation>Quantum GIS névjegy</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="32"/>
        <source>About</source>
        <translation>Névjegy</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="76"/>
        <source>&lt;h2&gt;Quantum GIS (qgis)&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Quantum GIS (qgis)&lt;/h2&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="86"/>
        <source>Version</source>
        <translation>Verzió</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="107"/>
        <source>QGIS Home Page</source>
        <translation>QGIS honlap</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="117"/>
        <source>Subscribe to the QGIS-User mailing list</source>
        <translation>QGIS-User levelező listára feliratkozás</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="128"/>
        <source>What&apos;s New</source>
        <translation>Mi az újdonság</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="157"/>
        <source>Developers</source>
        <translation>Fejlesztők</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="225"/>
        <source>&lt;h2&gt;QGIS Developers&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;QGIS fejlesztők&lt;/h2&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="233"/>
        <source>Providers</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="249"/>
        <source>Sponsors</source>
        <translation>Szponzorok</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="296"/>
        <source>Ok</source>
        <translation>Ok</translation>
    </message>
    <message>
        <location filename="../src/app/qgsabout.cpp" line="121"/>
        <source>The following have sponsored QGIS by contributing money to fund development and other project costs</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsAddAttrDialogBase</name>
    <message>
        <location filename="../src/ui/qgsaddattrdialogbase.ui" line="13"/>
        <source>Add Attribute</source>
        <translation>Attribútum hozzáadás</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsaddattrdialogbase.ui" line="52"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsaddattrdialogbase.ui" line="59"/>
        <source>Cancel</source>
        <translation>Mégsem</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsaddattrdialogbase.ui" line="87"/>
        <source>Type:</source>
        <translation>Típus:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsaddattrdialogbase.ui" line="100"/>
        <source>Name:</source>
        <translation>Név:</translation>
    </message>
</context>
<context>
    <name>QgsAttributeActionDialog</name>
    <message>
        <location filename="../src/app/qgsattributeactiondialog.cpp" line="57"/>
        <source>Name</source>
        <translation>Név</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributeactiondialog.cpp" line="58"/>
        <source>Action</source>
        <translation>Művelet</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributeactiondialog.cpp" line="59"/>
        <source>Capture</source>
        <translation>Digitalizál</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributeactiondialog.cpp" line="142"/>
        <source>Select an action</source>
        <comment>File dialog window title</comment>
        <translation>Válassz egy műveletet</translation>
    </message>
</context>
<context>
    <name>QgsAttributeActionDialogBase</name>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="21"/>
        <source>Form1</source>
        <translation type="unfinished">Form1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="49"/>
        <source>Remove the selected action</source>
        <translation>Szelektált művelet törlése</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="52"/>
        <source>Remove</source>
        <translation>Törlés</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="59"/>
        <source>Move the selected action down</source>
        <translation>A szelektált művelet mozgatása lefelé</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="62"/>
        <source>Move down</source>
        <translation type="unfinished">Mozgatás lefelé</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="69"/>
        <source>Move the selected action up</source>
        <translation type="unfinished">Szelektált művelet mozgatása felfelé</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="72"/>
        <source>Move up</source>
        <translation>Mozgatás felfelé</translation>
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
        <translation>Tallóz</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="136"/>
        <source>Inserts the selected field into the action, prepended with a %</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="139"/>
        <source>Insert field</source>
        <translation type="unfinished">Mező beillsztés</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="162"/>
        <source>Update the selected action</source>
        <translation>A szelektált művelet frissítése</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="165"/>
        <source>Update action</source>
        <translation>Művelet frissítése</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="172"/>
        <source>Inserts the action into the list above</source>
        <translation>Művelet beillesztése a fenti listába</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="175"/>
        <source>Insert action</source>
        <translation>Művelet beillesztés</translation>
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
        <translation>Művelet:</translation>
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
        <translation>Név:</translation>
    </message>
</context>
<context>
    <name>QgsAttributeDialogBase</name>
    <message>
        <location filename="../src/ui/qgsattributedialogbase.ui" line="16"/>
        <source>Enter Attribute Values</source>
        <translation>Add meg az attribútum értékeket</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributedialogbase.ui" line="32"/>
        <source>1</source>
        <translation>1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributedialogbase.ui" line="37"/>
        <source>Attribute</source>
        <translation>Attribútum</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributedialogbase.ui" line="42"/>
        <source>Value</source>
        <translation>Érték</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributedialogbase.ui" line="50"/>
        <source>&amp;OK</source>
        <translation>&amp;OK</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributedialogbase.ui" line="57"/>
        <source>&amp;Cancel</source>
        <translation type="unfinished">Mégsem</translation>
    </message>
</context>
<context>
    <name>QgsAttributeTable</name>
    <message>
        <location filename="../src/app/qgsattributetable.cpp" line="288"/>
        <source>Run action</source>
        <translation>Művelet futtatása</translation>
    </message>
</context>
<context>
    <name>QgsAttributeTableBase</name>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="13"/>
        <source>Attribute Table</source>
        <translation type="unfinished">Attribútum tábla</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="36"/>
        <source>Remove selection</source>
        <translation>Szelekció megszüntetése</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="52"/>
        <source>Move selected to top</source>
        <translation>Szelektált legfelülre mozgatása</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="61"/>
        <source>Ctrl+T</source>
        <translation>Ctrl+T</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="68"/>
        <source>Invert selection</source>
        <translation>Szelekció megfordítsa</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="77"/>
        <source>Ctrl+S</source>
        <translation>Ctrl+S</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="84"/>
        <source>Copy selected rows to clipboard (Ctrl+C)</source>
        <translation>Szelektált sorok másolása a vágólapra (Ctrl+C)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="87"/>
        <source>Copies the selected rows to the clipboard</source>
        <translation>Szelektált sorok másolása a vágólapra</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="96"/>
        <source>Ctrl+C</source>
        <translation>Ctrl+C</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="103"/>
        <source>Zoom map to the selected rows (Ctrl-F)</source>
        <translation>Térkép nagyítás a szelektált sorokra</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="106"/>
        <source>Zoom map to the selected rows</source>
        <translation>Térkép nagyítás a szelektált sorokra</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="112"/>
        <source>Ctrl+F</source>
        <translation>Ctrl+F</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="127"/>
        <source>New column</source>
        <translation>Új oszlop</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="136"/>
        <source>Ctrl+N</source>
        <translation>Ctrl+N</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="143"/>
        <source>Delete column</source>
        <translation>Oszlop törlés</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="152"/>
        <source>Ctrl+X</source>
        <translation>Ctrl+X</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="175"/>
        <source>Start editing</source>
        <translation>Szerkesztés kezdése</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="182"/>
        <source>Stop editin&amp;g</source>
        <translation>&amp;Szerkesztés lezárása</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="185"/>
        <source>Alt+G</source>
        <translation>Alt+G</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="214"/>
        <source>&amp;Help</source>
        <translation>&amp;Súgó</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="282"/>
        <source>Alt+C</source>
        <translation>Alt+C</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="224"/>
        <source>Search for:</source>
        <translation>Keresett:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="237"/>
        <source>in</source>
        <translation>ebben</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="259"/>
        <source>Search</source>
        <translation>Keres</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="269"/>
        <source>Adva&amp;nced...</source>
        <translation>&amp;Haladó...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="272"/>
        <source>Alt+N</source>
        <translation>Alt+N</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="279"/>
        <source>&amp;Close</source>
        <translation>&amp;Lezár</translation>
    </message>
</context>
<context>
    <name>QgsAttributeTableDisplay</name>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="95"/>
        <source>select</source>
        <translation>szelektál</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="96"/>
        <source>select and bring to top</source>
        <translation>szelektál és előre hoz</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="97"/>
        <source>show only matching</source>
        <translation>csak az egyezőket mutasd</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="145"/>
        <source>Name conflict</source>
        <translation>Név ütközés</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="145"/>
        <source>The attribute could not be inserted. The name already exists in the table.</source>
        <translation>Az attribútumot nem tudom beszúrni. A név már szerepel a táblában.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="194"/>
        <source>Stop editing</source>
        <translation>Szerkesztés vége</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="195"/>
        <source>Do you want to save the changes?</source>
        <translation>Akarod menteni a változásokat?</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="201"/>
        <source>Error</source>
        <translation>Hiba</translation>
    </message>
    <message>
        <location filename="" line="145"/>
        <source>Could not commit changes</source>
        <translation type="obsolete">Nem tudom menteni a változásokat</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="327"/>
        <source>Search string parsing error</source>
        <translation>Keresési minta hibás</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="379"/>
        <source>Search results</source>
        <translation>Keresés eredmények</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="333"/>
        <source>You&apos;ve supplied an empty search string.</source>
        <translation>Üres keresési mintát adtál meg.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="367"/>
        <source>Error during search</source>
        <translation>Hiba a keresés közben</translation>
    </message>
    <message numerus="yes">
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="376"/>
        <source>Found %d matching features.</source>
        <translation type="unfinished">
            <numerusform>%d megfelelő elemet találtam.</numerusform>
        </translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="378"/>
        <source>No matching features found.</source>
        <translation>Nem találtam megfelelő elemet.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="201"/>
        <source>Could not commit changes - changes are still pending</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsBookmarks</name>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="140"/>
        <source>Really Delete?</source>
        <translation>Valóban törlöd?</translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="141"/>
        <source>Are you sure you want to delete the </source>
        <translation>Biztos, hogy törölni akarod ezt</translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="141"/>
        <source> bookmark?</source>
        <translation>könyvjelző?</translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="157"/>
        <source>Error deleting bookmark</source>
        <translation>Hiba a könyvjelző törlése során</translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="159"/>
        <source>Failed to delete the </source>
        <translation>Nem sikerült törölni</translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="161"/>
        <source> bookmark from the database. The database said:
</source>
        <translation> könyvjelzőt az adatbázisból. Adatbázis válasza:
</translation>
    </message>
</context>
<context>
    <name>QgsBookmarksBase</name>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="16"/>
        <source>Geospatial Bookmarks</source>
        <translation>Térinformatikai könyvjelző</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="45"/>
        <source>Name</source>
        <translation>Név</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="50"/>
        <source>Project</source>
        <translation>Projekt</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="55"/>
        <source>Extent</source>
        <translation>Terjedelem</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="60"/>
        <source>Id</source>
        <translation>Id</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="68"/>
        <source>Help</source>
        <translation>Súgó</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="75"/>
        <source>Close the dialog</source>
        <translation>Párbeszédablak lezárása</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="78"/>
        <source>Close</source>
        <translation>Lezár</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="85"/>
        <source>Delete the currently selected bookmark</source>
        <translation>A szelektált könyvjelző törlése</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="88"/>
        <source>Delete</source>
        <translation>Törlés</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="95"/>
        <source>Zoom to the currently selected bookmark</source>
        <translation>Nagyítás a szelektált könyvjelzőre</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="98"/>
        <source>Zoom To</source>
        <translation>Nagyítás erre</translation>
    </message>
</context>
<context>
    <name>QgsComposer</name>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="55"/>
        <source>QGIS - print composer</source>
        <translation>QGIS - nyomtatás vezérlő</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="77"/>
        <source>Map 1</source>
        <translation>1. Térkép</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="504"/>
        <source>Couldn&apos;t open </source>
        <translation>Nem tudom megnyitni</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="504"/>
        <source> for read/write</source>
        <translation>olvasásra/irásra</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="583"/>
        <source>Error in Print</source>
        <translation>Hiba a nyomtatás közben</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="561"/>
        <source>Cannot seek</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="486"/>
        <source>Cannot overwrite BoundingBox</source>
        <translation>Nem tudom felülírni a befoglaló téglalapot</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="494"/>
        <source>Cannot find BoundingBox</source>
        <translation>Nem találom a befoglaló téglalapot</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="577"/>
        <source>Cannot overwrite translate</source>
        <translation>Nem tudom felülírni a fordítást</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="583"/>
        <source>Cannot find translate</source>
        <translation>Nem találom a fordítást</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="591"/>
        <source>File IO Error</source>
        <translation>Fájl IO hiba</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="601"/>
        <source>Paper does not match</source>
        <translation>Papír nem egyezik</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="602"/>
        <source>The selected paper size does not match the composition size</source>
        <translation>A kiválasztott papírméret nem egyezik az összeállítás méretével</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="665"/>
        <source>Big image</source>
        <translation>Nagy kép</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="666"/>
        <source>To create image </source>
        <translation>A kép létrehozása</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="669"/>
        <source> requires circa </source>
        <translation> szükséges kb. </translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="669"/>
        <source> MB of memory</source>
        <translation>MB memória</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="696"/>
        <source>format</source>
        <translation>formátum</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="719"/>
        <source>Choose a filename to save the map image as</source>
        <translation>Válassz egy fájlnevet a kép elmentéséhez</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="788"/>
        <source>SVG warning</source>
        <translation>SVG figyelmeztetés</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="789"/>
        <source>Don&apos;t show this message again</source>
        <translation>Ne mutasd többé ezt az üzenetet</translation>
    </message>
    <message>
        <location filename="" line="145"/>
        <source>&lt;p&gt;The SVG export function in Qgis has several problems due to bugs and deficiencies in the Qt4 svg code. Of note, text does not appear in the SVG file and there are problems with the map bounding box clipping other items such as the legend or scale bar.&lt;/p&gt;If you require a vector-based output file from Qgis it is suggested that you try printing to PostScript if the SVG output is not satisfactory.&lt;/p&gt;</source>
        <translation type="obsolete">&lt;p&gt;Az SVG exporttal a QGIS-ben számos probléma van a QT4 svg kód hibáinak következményeként. Például a szövegek nem jelennek meg az SVG fájlban és problémák vannak a befoglaló téglalapra vágással és más elemekkel mint a  jelkulcs vagy lépték.&lt;/p&gt;Ha vektoros outputra van szükséged és az SVG eredmény nem kielégítõ, akkor javasoljuk a PostScript nyomtatás kipróbálását.&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="814"/>
        <source>Choose a filename to save the map as</source>
        <translation>Válassz egy fájlnevet a térkép másként mentéséhez</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="815"/>
        <source>SVG Format</source>
        <translation>SVG formátum</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="795"/>
        <source>&lt;p&gt;The SVG export function in Qgis has several problems due to bugs and deficiencies in the </source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsComposerBase</name>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="13"/>
        <source>MainWindow</source>
        <translation>Fő ablak</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="62"/>
        <source>General</source>
        <translation>Általános</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="107"/>
        <source>Composition</source>
        <translation>Összeállítás</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="115"/>
        <source>Item</source>
        <translation>Elem</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="150"/>
        <source>Help</source>
        <translation>Súgó</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="170"/>
        <source>Close</source>
        <translation>Lezár</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="208"/>
        <source>&amp;Open Template ...</source>
        <translation>&amp;Sablon nyitás ...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="216"/>
        <source>Save Template &amp;As...</source>
        <translation>Sablon mentés &amp;másként ...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="224"/>
        <source>&amp;Print...</source>
        <translation>&amp;Nyomtatás...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="232"/>
        <source>Zoom All</source>
        <translation>Nagyítás terjedelemre</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="240"/>
        <source>Zoom In</source>
        <translation>Nagyítás</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="248"/>
        <source>Zoom Out</source>
        <translation>Kicsinyítés</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="256"/>
        <source>Add new map</source>
        <translation>Új térkép hozzáadás</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="264"/>
        <source>Add new label</source>
        <translation>Új cimke hozzáadás</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="272"/>
        <source>Add new vect legend</source>
        <translation>Új vektor jelmagyarázat</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="280"/>
        <source>Select/Move item</source>
        <translation>Elem szelektálás és mozgatás</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="288"/>
        <source>Export as image</source>
        <translation>Képként exportálás</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="296"/>
        <source>Export as SVG</source>
        <translation>SVG-ként exportálás</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="304"/>
        <source>Add new scalebar</source>
        <translation>Új lépték vonalzó</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="312"/>
        <source>Refresh view</source>
        <translation>Ablak frissítés</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="320"/>
        <source>Add Image</source>
        <translation>Kép hozzáadás</translation>
    </message>
</context>
<context>
    <name>QgsComposerLabelBase</name>
    <message>
        <location filename="../src/ui/qgscomposerlabelbase.ui" line="21"/>
        <source>Label Options</source>
        <translation>Cimke beállítások</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlabelbase.ui" line="48"/>
        <source>Font</source>
        <translation>Betükészlet</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlabelbase.ui" line="55"/>
        <source>Box</source>
        <translation>Téglalap</translation>
    </message>
</context>
<context>
    <name>QgsComposerMap</name>
    <message>
        <location filename="../src/app/composer/qgscomposermap.cpp" line="74"/>
        <source>Map %1</source>
        <translation>%1 térkép</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposermap.cpp" line="96"/>
        <source>Extent (calculate scale)</source>
        <translation>Terjedelem (méretarány számítás)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposermap.cpp" line="97"/>
        <source>Scale (calculate extent)</source>
        <translation>Méretarány (terjedelem számítás)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposermap.cpp" line="104"/>
        <source>Cache</source>
        <translation>Átmeneti tár</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposermap.cpp" line="105"/>
        <source>Render</source>
        <translation>Megjelenít</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposermap.cpp" line="106"/>
        <source>Rectangle</source>
        <translation>Téglalap</translation>
    </message>
</context>
<context>
    <name>QgsComposerMapBase</name>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="21"/>
        <source>Map options</source>
        <translation>Térkép beállítások</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="69"/>
        <source>Set map extent to current extent in QGIS map canvas</source>
        <translation>Térkép terjedelem beállítása az aktuális QGIS térkép terjedelmére</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="72"/>
        <source>Set Extent</source>
        <translation>Terjedelem beállítás</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="79"/>
        <source>1:</source>
        <translation>1:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="97"/>
        <source>Scale:</source>
        <translation>Méretarány:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="116"/>
        <source>Width of one unit in millimeters</source>
        <translation>Egy egység szélessége milliméterben</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="147"/>
        <source>Set</source>
        <translation>Beállít</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="173"/>
        <source>&lt;b&gt;Map&lt;/b&gt;</source>
        <translation>&lt;b&gt;Térkép&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="180"/>
        <source>Height</source>
        <translation>Magasság</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="196"/>
        <source>Width</source>
        <translation>Szélesség</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="212"/>
        <source>Line width scale</source>
        <translation>Vonal szélesség szorzó</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="225"/>
        <source>Symbol scale</source>
        <translation>Szimbólum szorzó</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="238"/>
        <source>Font size scale</source>
        <translation>Betű méret szorzó</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="251"/>
        <source>Frame</source>
        <translation>Keret</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="258"/>
        <source>Preview</source>
        <translation>Előnézet</translation>
    </message>
</context>
<context>
    <name>QgsComposerPicture</name>
    <message>
        <location filename="../src/app/composer/qgscomposerpicture.cpp" line="401"/>
        <source>Warning</source>
        <translation>Figyelmeztetés</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposerpicture.cpp" line="402"/>
        <source>Cannot load picture.</source>
        <translation>Nem tudom a képet betölteni.</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposerpicture.cpp" line="468"/>
        <source>Pictures (</source>
        <translation>Képek (</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposerpicture.cpp" line="485"/>
        <source>Choose a file</source>
        <translation>Válassz egy fájlt</translation>
    </message>
</context>
<context>
    <name>QgsComposerPictureBase</name>
    <message>
        <location filename="../src/ui/qgscomposerpicturebase.ui" line="21"/>
        <source>Picture Options</source>
        <translation>Kép beállítások</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerpicturebase.ui" line="58"/>
        <source>Browse</source>
        <translation>Tallóz</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerpicturebase.ui" line="119"/>
        <source>Width</source>
        <translation>Szélesség</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerpicturebase.ui" line="140"/>
        <source>Height</source>
        <translation>Magasság</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerpicturebase.ui" line="161"/>
        <source>Angle</source>
        <translation>Szög</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerpicturebase.ui" line="197"/>
        <source>Frame</source>
        <translation>Keret</translation>
    </message>
</context>
<context>
    <name>QgsComposerScalebarBase</name>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="21"/>
        <source>Barscale Options</source>
        <translation>Lépték vonalzó beállítások</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="41"/>
        <source>Line width</source>
        <translation>Vonalvastagság</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="88"/>
        <source>Unit label</source>
        <translation>Mértékegység cimke</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="127"/>
        <source>Map</source>
        <translation>Térkép</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="140"/>
        <source>Segment size</source>
        <translation>Szegmens méret</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="159"/>
        <source>Map units per scalebar unit</source>
        <translation>Térkép egységek léptékvonalzó egységenként</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="172"/>
        <source>Number of segments</source>
        <translation>Szegmensek száma</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="195"/>
        <source>Font</source>
        <translation>Betűkészlet</translation>
    </message>
</context>
<context>
    <name>QgsComposerVectorLegend</name>
    <message>
        <location filename="../src/app/composer/qgscomposervectorlegend.cpp" line="104"/>
        <source>Legend</source>
        <translation>Jelmagyarázat</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposervectorlegend.cpp" line="117"/>
        <source>Layers</source>
        <translation>Rétegek</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposervectorlegend.cpp" line="118"/>
        <source>Group</source>
        <translation>Csoport</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposervectorlegend.cpp" line="125"/>
        <source>Combine selected layers</source>
        <translation>Szelektált rétegek kombinálása</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposervectorlegend.cpp" line="138"/>
        <source>Cache</source>
        <translation>Átmeneti tár</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposervectorlegend.cpp" line="139"/>
        <source>Render</source>
        <translation>Megjelenítés</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposervectorlegend.cpp" line="140"/>
        <source>Rectangle</source>
        <translation>Téglalap</translation>
    </message>
</context>
<context>
    <name>QgsComposerVectorLegendBase</name>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="21"/>
        <source>Vector Legend Options</source>
        <translation>Vektor jelkulcs beállítások</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="53"/>
        <source>Preview</source>
        <translation>Előnézet</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="92"/>
        <source>Map</source>
        <translation>Térkép</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="113"/>
        <source>Title</source>
        <translation>Cím</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="140"/>
        <source>Column 1</source>
        <translation>1. oszlop</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="148"/>
        <source>Box</source>
        <translation>Doboz</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="163"/>
        <source>Font</source>
        <translation>Betűkészlet</translation>
    </message>
</context>
<context>
    <name>QgsComposition</name>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="86"/>
        <source>Custom</source>
        <translation>Egyéni</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="87"/>
        <source>A5 (148x210 mm)</source>
        <translation>A5 (148x210 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="88"/>
        <source>A4 (210x297 mm)</source>
        <translation>A4 (210x297 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="89"/>
        <source>A3 (297x420 mm)</source>
        <translation>A3 (297x420 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="90"/>
        <source>A2 (420x594 mm)</source>
        <translation>A2 (420x594 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="91"/>
        <source>A1 (594x841 mm)</source>
        <translation>A1 (594x841 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="92"/>
        <source>A0 (841x1189 mm)</source>
        <translation>A0 (841x1189 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="93"/>
        <source>B5 (176 x 250 mm)</source>
        <translation>B5 (176x250 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="94"/>
        <source>B4 (250 x 353 mm)</source>
        <translation>B4 (250x353 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="95"/>
        <source>B3 (353 x 500 mm)</source>
        <translation>B3 (353x500 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="96"/>
        <source>B2 (500 x 707 mm)</source>
        <translation>B2 (500x707 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="97"/>
        <source>B1 (707 x 1000 mm)</source>
        <translation>B1 (707x1000 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="98"/>
        <source>B0 (1000 x 1414 mm)</source>
        <translation>B0 (1000x1414 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="99"/>
        <source>Letter (8.5x11 inches)</source>
        <translation>Letter (8.5x11inches)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="100"/>
        <source>Legal (8.5x14 inches)</source>
        <translation>Legal (8.5x14 inches)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="113"/>
        <source>Portrait</source>
        <translation>Álló</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="114"/>
        <source>Landscape</source>
        <translation>Fekvő</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="609"/>
        <source>Out of memory</source>
        <translation>Kévés a memória</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="612"/>
        <source>Qgis is unable to resize the paper size due to insufficient memory.
 It is best that you avoid using the map composer until you restart qgis.
</source>
        <translation>A QGIS nem tudja átméretezni a papírt mert kevés a memória.
A legjobb, ha nem használod a térkép összeállítót a QGIS újraindításáig.</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="756"/>
        <source>Label</source>
        <translation type="unfinished">Cimke</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="806"/>
        <source>Warning</source>
        <translation>Figyelmeztetés</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="807"/>
        <source>Cannot load picture.</source>
        <translation>Nem tudom betölteni a képet.</translation>
    </message>
</context>
<context>
    <name>QgsCompositionBase</name>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="21"/>
        <source>Composition</source>
        <translation>Összeállítás</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="33"/>
        <source>Paper</source>
        <translation>Papír</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="104"/>
        <source>Orientation</source>
        <translation>Tájolás</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="122"/>
        <source>Height</source>
        <translation>Magasság</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="140"/>
        <source>Width</source>
        <translation>Szélesség</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="158"/>
        <source>Units</source>
        <translation>Egységek</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="176"/>
        <source>Size</source>
        <translation>Méret</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="213"/>
        <source>Resolution (dpi)</source>
        <translation>Felbontás (dpi)</translation>
    </message>
</context>
<context>
    <name>QgsConnectionDialog</name>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialog.cpp" line="74"/>
        <source>Test connection</source>
        <translation>Kapcsolat tesztelés</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialog.cpp" line="72"/>
        <source>Connection to </source>
        <translation>Kapcsolat ehhez</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialog.cpp" line="72"/>
        <source> was successfull</source>
        <translation>sikeres</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialog.cpp" line="74"/>
        <source>Connection failed - Check settings and try again </source>
        <translation>Sikertelen kapcsolat - ellenőrizd a beállításokat és próbáld újra</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialog.cpp" line="96"/>
        <source>General Interface Help:

</source>
        <translation>Általános interfész súgó:
</translation>
    </message>
</context>
<context>
    <name>QgsConnectionDialogBase</name>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="13"/>
        <source>Create a New PostGIS connection</source>
        <translation>Új PostGIS kapcsolat létrehozása</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="31"/>
        <source>Connection Information</source>
        <translation>Kapcsolat információk</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="51"/>
        <source>Save Password</source>
        <translation>Jelszó mentése</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="58"/>
        <source>Test Connect</source>
        <translation>Kapcsolat teszt</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="83"/>
        <source>Name</source>
        <translation>Név</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="90"/>
        <source>Host</source>
        <translation>Gép</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="97"/>
        <source>Database</source>
        <translation>Adatbázis</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="104"/>
        <source>Port</source>
        <translation>Port</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="111"/>
        <source>Username</source>
        <translation>Felhasználó név</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="118"/>
        <source>Password</source>
        <translation>Jelszó</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="135"/>
        <source>Name of the new connection</source>
        <translation>Az új kapcsolat neve</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="148"/>
        <source>5432</source>
        <translation>5432</translation>
    </message>
</context>
<context>
    <name>QgsContinuousColorDialogBase</name>
    <message>
        <location filename="../src/ui/qgscontinuouscolordialogbase.ui" line="13"/>
        <source>Continuous color</source>
        <translation>Folytonos szín</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscontinuouscolordialogbase.ui" line="28"/>
        <source>Draw polygon outline</source>
        <translation>Felület körvonal rajzolás</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscontinuouscolordialogbase.ui" line="41"/>
        <source>Classification Field:</source>
        <translation>Osztályozás mező:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscontinuouscolordialogbase.ui" line="57"/>
        <source>Minimum Value:</source>
        <translation>Minimum érték:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscontinuouscolordialogbase.ui" line="73"/>
        <source>Outline Width:</source>
        <translation>Körvonal vastagság:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscontinuouscolordialogbase.ui" line="99"/>
        <source>Maximum Value:</source>
        <translation>Maximum érték:</translation>
    </message>
</context>
<context>
    <name>QgsCoordinateTransform</name>
    <message>
        <location filename="../src/core/qgscoordinatetransform.cpp" line="418"/>
        <source>The source spatial reference system (SRS) is not valid. </source>
        <translation>A kiinduló vetületi rendszer (SRS) érvénytelen.</translation>
    </message>
    <message>
        <location filename="../src/core/qgscoordinatetransform.cpp" line="426"/>
        <source>The coordinates can not be reprojected. The SRS is: </source>
        <translation>Nem tudom a koordinátákat átszámítani. Az SRS:</translation>
    </message>
    <message>
        <location filename="../src/core/qgscoordinatetransform.cpp" line="425"/>
        <source>The destination spatial reference system (SRS) is not valid. </source>
        <translation>A cél vetületi rendszer (SRS) érvénytelen.</translation>
    </message>
    <message>
        <location filename="../src/core/qgscoordinatetransform.cpp" line="483"/>
        <source>Failed</source>
        <translation>Sikertelen</translation>
    </message>
    <message>
        <location filename="../src/core/qgscoordinatetransform.cpp" line="483"/>
        <source>transform of</source>
        <translation>transzformáció</translation>
    </message>
    <message>
        <location filename="../src/core/qgscoordinatetransform.cpp" line="496"/>
        <source>with error: </source>
        <translation>hiba:</translation>
    </message>
</context>
<context>
    <name>QgsCopyrightLabelPlugin</name>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="66"/>
        <source>Bottom Left</source>
        <translation>Bal alsó</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="67"/>
        <source>Top Left</source>
        <translation>Bal felső</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="67"/>
        <source>Top Right</source>
        <translation>Jobb felső</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="67"/>
        <source>Bottom Right</source>
        <translation>Jobb alsó</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="79"/>
        <source>&amp;Copyright Label</source>
        <translation>&amp;Szerzői jog cimke</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="80"/>
        <source>Creates a copyright label that is displayed on the map canvas.</source>
        <translation type="unfinished">A térképen megjelenített szerzői jog cimke létrehozása.</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="204"/>
        <source>&amp;Decorations</source>
        <translation>&amp;Dekorációk</translation>
    </message>
</context>
<context>
    <name>QgsCopyrightLabelPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="13"/>
        <source>Copyright Label Plugin</source>
        <translation type="unfinished">Szerzői jog cimke modul</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="36"/>
        <source>Color</source>
        <translation>Szín</translation>
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
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:12pt;&quot;&gt;Description&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Add meg a szerzõi jogokra vonatkozó információt lent. Ez a modul az alapvetõ html formatáló tagokat engedi meg az információban. Például:&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-weight:600;&quot;&gt;&amp;lt;B&amp;gt; Félkövér szöveg &amp;lt;/B&amp;gt; &lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-weight:600;&quot;&gt;&lt;span style=&quot; font-weight:400; font-style:italic;&quot;&gt;&amp;lt;I&amp;gt; Dölt &amp;lt;/I&amp;gt;&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-style:italic;&quot;&gt;&lt;span style=&quot; font-style:normal;&quot;&gt;(megjegyzés: az &amp;amp;copy; a szerzõi jog szimbólum)&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="118"/>
        <source>Enable Copyright Label</source>
        <translation type="unfinished">Szerzői jog cimke bekapcsolás</translation>
    </message>
    <message encoding="UTF-8">
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="130"/>
        <source>© QGIS 2008</source>
        <translation>(c) QGIS 2008</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="145"/>
        <source>Placement</source>
        <translation>Elhelyezés</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="153"/>
        <source>Bottom Left</source>
        <translation>Bal alsó</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="158"/>
        <source>Top Left</source>
        <translation>Bal felső</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="163"/>
        <source>Bottom Right</source>
        <translation>Jobb alsó</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="168"/>
        <source>Top Right</source>
        <translation>Jobb felső</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="176"/>
        <source>Orientation</source>
        <translation>Tájolás</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="184"/>
        <source>Horizontal</source>
        <translation>Vízszintes</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="189"/>
        <source>Vertical</source>
        <translation>Függőleges</translation>
    </message>
</context>
<context>
    <name>QgsCustomProjectionDialog</name>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="165"/>
        <source>Delete Projection Definition?</source>
        <translation>Vetület definíció törlése?</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="166"/>
        <source>Deleting a projection definition is not reversable. Do you want to delete it?</source>
        <translation>A vetület definíció törlés nem visszefordítható. Törölni akarod?</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="876"/>
        <source>Abort</source>
        <translation>Megszakít</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="878"/>
        <source>New</source>
        <translation>Új</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="933"/>
        <source>QGIS Custom Projection</source>
        <translation>QGIS egyéni vetület</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="750"/>
        <source>This proj4 projection definition is not valid. Please give the projection a name before pressing save.</source>
        <translation>Ez a proj4 vetület definíció érvénytelen. Adj a vetületnek nevet a mentés előtt.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="756"/>
        <source>This proj4 projection definition is not valid. Please add the parameters before pressing save.</source>
        <translation>Ez a proj4 vetület definíció érvénytelen. Add meg a paramétereket a mentés előtt.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="771"/>
        <source>This proj4 projection definition is not valid. Please add a proj= clause before pressing save.</source>
        <translation>Ez a proj4 vetület definíció érvénytelen. Adj meg egy proj= kifejezést a mentés előtt.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="778"/>
        <source>This proj4 ellipsoid definition is not valid. Please add a ellips= clause before pressing save.</source>
        <translation>Ez a proj4 ellipszoid definíció érvénytelen. Adj meg egy ellips= kifejezést a mentés előtt.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="794"/>
        <source>This proj4 projection definition is not valid. Please correct before pressing save.</source>
        <translation>Ez a proj4 vetület definíció érvénytelen. Javítsd a mentés előtt.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="907"/>
        <source>This proj4 projection definition is not valid.</source>
        <translation>Ez a proj4 vetület definíció érvénytelen.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="922"/>
        <source>Northing and Easthing must be in decimal form.</source>
        <translation>X és Y szám formában kell.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="934"/>
        <source>Internal Error (source projection invalid?)</source>
        <translation>Belső hiba (kiinduló vetület érvénytelen?)</translation>
    </message>
</context>
<context>
    <name>QgsCustomProjectionDialogBase</name>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="62"/>
        <source>Custom Projection Definition</source>
        <translation>Egyeni vetület definíció</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="32"/>
        <source>Define</source>
        <translation>Definiál</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="44"/>
        <source>You can define your own custom projection here. The definition must conform to the proj4 format for specifying a Spatial Reference System.</source>
        <translation>Az egyéni vetületi rendszeredet definiálhatod itt. A térbeli referencia rendszer definíciójának meg kell felelnie a proj4 formátumnak.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="203"/>
        <source>Parameters:</source>
        <translation>Paraméterek:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="87"/>
        <source>|&lt;</source>
        <translation>|&lt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="94"/>
        <source>&lt;</source>
        <translation>&lt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="101"/>
        <source>1 of 1</source>
        <translation>1/1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="111"/>
        <source>&gt;</source>
        <translation>&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="118"/>
        <source>&gt;|</source>
        <translation>&gt;|</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="125"/>
        <source>New</source>
        <translation>Új</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="132"/>
        <source>Save</source>
        <translation>Ment</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="139"/>
        <source>Delete</source>
        <translation>Töröl</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="146"/>
        <source>Close</source>
        <translation>Lezár</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="155"/>
        <source>Name:</source>
        <translation>Név:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="166"/>
        <source>Test</source>
        <translation>Teszt</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="189"/>
        <source>Transform from WGS84 to the chosen projection</source>
        <translation>Transzformáció a WGS84-ből a válaszott vetületbe</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="196"/>
        <source>Use the text boxes below to test the projection definition you are creating. Enter a coordinate where both the lat/long and the projected result are known (for example by reading off a map). Then press the calculate button to see if the projection definition you are creating is accurate.</source>
        <translation>Használd az alábbi mezőket a létrehozott vetület ellenőrzésére. Adj meg egy pontot, ahol a szélesség/hosszúság és a vetületi koordináták ismertek (például olvasd le egy térképről). Utána nyomd meg a számítás gombot és ellenőrizd, hogy pontos a vetület definíció.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="232"/>
        <source>Projected Coordinate System</source>
        <translation>Vetületi koordinátarendszer</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="239"/>
        <source>Geographic / WGS84</source>
        <translation>Földrajzi/WGS84</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="265"/>
        <source>East:</source>
        <translation>Kelet:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="275"/>
        <source>North:</source>
        <translation>Észak:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="305"/>
        <source>Calculate</source>
        <translation>Számít</translation>
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
        <translation>Szabályos kifejezés</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="135"/>
        <source>All</source>
        <translation>Mind</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="139"/>
        <source>Schema</source>
        <translation>Séma</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="143"/>
        <source>Table</source>
        <translation>Tábla</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="147"/>
        <source>Type</source>
        <translation>Típus</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="151"/>
        <source>Geometry column</source>
        <translation>Geometria oszlop</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="155"/>
        <source>Sql</source>
        <translation>SQL</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="228"/>
        <source>Are you sure you want to remove the </source>
        <translation>Biztos, hogy törlöd</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="228"/>
        <source> connection and all associated settings?</source>
        <translation>Kapcsolat és minden hozzátartozó beállítást?</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="229"/>
        <source>Confirm Delete</source>
        <translation>Törlés megerősítése</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="334"/>
        <source>Select Table</source>
        <translation>Válassz táblát</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="334"/>
        <source>You must select a table in order to add a Layer.</source>
        <translation>Ki kell választanod egy táblát, hogy egy réteget hozzá tudjál adni.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="367"/>
        <source>Password for </source>
        <translation>Jelszó</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="368"/>
        <source>Please enter your password:</source>
        <translation>Add meg a jelszavad:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="433"/>
        <source>Connection failed</source>
        <translation>Sikertelen kapcsolódás</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="436"/>
        <source>Connection to %1 on %2 failed. Either the database is down or your settings are incorrect.%3Check your username and password and try again.%4The database said:%5%6</source>
        <translation>Kapcsolódás a %1-hez a %2-n sikertelen. Vagy az adatbáziskezelő nem fut vagy a beállítások hibásak.%3Ellenőrizd a felhasználó nevet és jelszót és próbáld újra.%4Az adatbázis üzenete:%5%6</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="543"/>
        <source>Accessible tables could not be determined</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="545"/>
        <source>Database connection was successful, but the accessible tables could not be determined.

The error message from the database was:
%1
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="550"/>
        <source>No accessible tables found</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="554"/>
        <source>Database connection was successful, but no accessible tables were found.

Please verify that you have SELECT privilege on a table carrying PostGIS
geometry.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsDbSourceSelectBase</name>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="13"/>
        <source>Add PostGIS Table(s)</source>
        <translation>PostGIS tábla hozzáadás</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="54"/>
        <source>PostgreSQL Connections</source>
        <translation>PostgraSQL kapcsolat</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="66"/>
        <source>Delete</source>
        <translation>Töröl</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="73"/>
        <source>Edit</source>
        <translation>Szerkeszt</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="80"/>
        <source>New</source>
        <translation>Új</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="87"/>
        <source>Connect</source>
        <translation>Kapcsolódás</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="111"/>
        <source>Help</source>
        <translation>Súgó</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="114"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="140"/>
        <source>Add</source>
        <translation>Hozzáad</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="156"/>
        <source>Close</source>
        <translation>Lezár</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="183"/>
        <source>Search:</source>
        <translation>Keres:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="190"/>
        <source>Search mode:</source>
        <translation>Keresési mód:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="200"/>
        <source>Search in columns:</source>
        <translation>Keresés az oszlopban:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="216"/>
        <source>Search options...</source>
        <translation>Keresési beállítások...</translation>
    </message>
</context>
<context>
    <name>QgsDbTableModel</name>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="24"/>
        <source>Schema</source>
        <translation>Séma</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="25"/>
        <source>Table</source>
        <translation>Tábla</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="26"/>
        <source>Type</source>
        <translation>Típus</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="27"/>
        <source>Geometry column</source>
        <translation>Geometria oszlop</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="28"/>
        <source>Sql</source>
        <translation>SQL</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="229"/>
        <source>Point</source>
        <translation>Pont</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="233"/>
        <source>Multipoint</source>
        <translation>Multipont</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="237"/>
        <source>Line</source>
        <translation>Vonal</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="241"/>
        <source>Multiline</source>
        <translation>Multivonal</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="245"/>
        <source>Polygon</source>
        <translation>Felület</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="249"/>
        <source>Multipolygon</source>
        <translation>Multifelület</translation>
    </message>
</context>
<context>
    <name>QgsDelAttrDialogBase</name>
    <message>
        <location filename="../src/ui/qgsdelattrdialogbase.ui" line="16"/>
        <source>Delete Attributes</source>
        <translation>Attribútumok törlése</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdelattrdialogbase.ui" line="52"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdelattrdialogbase.ui" line="59"/>
        <source>Cancel</source>
        <translation>Mégsem</translation>
    </message>
</context>
<context>
    <name>QgsDelimitedTextPlugin</name>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="56"/>
        <source>DelimitedTextLayer</source>
        <translation>Tagolt szöveg réteg</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="101"/>
        <source>&amp;Add Delimited Text Layer</source>
        <translation>&amp;Tagolt szöveg réteg hozzáadás</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="104"/>
        <source>Add a delimited text file as a map layer. </source>
        <translation>Egy tagolt szövegfájl hozzáadása mint térképi réteg.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="105"/>
        <source>The file must have a header row containing the field names. </source>
        <translation>A fájlban kell lenni egy fejléc sornak a mezők neveivel.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="105"/>
        <source>X and Y fields are required and must contain coordinates in decimal units.</source>
        <translation>Az Y és Y mezők kötelezők és a koordinátákat kell tartalmaznia.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="142"/>
        <source>&amp;Delimited text</source>
        <translation>&amp;Tagolt szöveg</translation>
    </message>
</context>
<context>
    <name>QgsDelimitedTextPluginGui</name>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="35"/>
        <source>Parse</source>
        <translation>Elemez</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="60"/>
        <source>Description</source>
        <translation>Leírás</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="63"/>
        <source>Select a delimited text file containing a header row and one or more rows of x and y coordinates that you would like to use as a point layer and this plugin will do the job for you!</source>
        <translation>Válassz egy szövegfájlt, mely egy fejléc sort és egy vagy több x és y koordinátát tartalmazó sorból áll. Ezt egy pont rétegként használhatod és ez a modul végrehajtja ezt a feladatot!</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="67"/>
        <source>Use the layer name box to specify the legend name for the new layer. Use the delimiter box to specify what delimeter is used in your file (e.g. space, comma, tab or a regular expression in Perl style). After choosing a delimiter, press the parse button and select the columns containing the x and y values for the layer.</source>
        <translation>Használd a réteg név mezőt az új réteg jelmagyarázatban megjelenő nevének megadásához. Használd az elválasztó mezőt a fájlban használt elválasztó karakter megadására (pl. szóköz, vessző, tabulátor vagy egy Perl stílusú szabályos kifejezés). Az elválasztó karakter kiválasztása után nyomd meg az elemez gombot és válaszd ki az x és y értéket tartalmazó oszlopokat a réteghez.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="125"/>
        <source>No layer name</source>
        <translation>Nincs rétegnév</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="125"/>
        <source>Please enter a layer name before adding the layer to the map</source>
        <translation>Kérem adj meg egy rétegnevet mielőtt a térképhez adnád a réteget</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="211"/>
        <source>No delimiter</source>
        <translation>Nincs elválasztó karakter</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="211"/>
        <source>Please specify a delimiter prior to parsing the file</source>
        <translation>Kérem add meg az elválasztó karaktert a fájl elemzése előtt</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="245"/>
        <source>Choose a delimited text file to open</source>
        <translation>Válaszd ki a megnyitandó szövegfájlt</translation>
    </message>
</context>
<context>
    <name>QgsDelimitedTextPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="13"/>
        <source>Create a Layer from a Delimited Text File</source>
        <translation>Réteg létrehozás szövegfájlból</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="39"/>
        <source>Delimited Text Layer</source>
        <translation>Szövegfájl réteg</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="67"/>
        <source>&lt;p align=&quot;right&quot;&gt;X field&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;right&quot;&gt;X mező&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="88"/>
        <source>Name of the field containing x values</source>
        <translation>x értékeket tartalmazó mező neve</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="91"/>
        <source>Name of the field containing x values. Choose a field from the list. The list is generated by parsing the header row of the delimited text file.</source>
        <translation>x értékeket tartalmazó mező neve. Válassz egy mezőt a listából. A listát a szövegfájl fejléc sorának elemzés alapján állítottuk elő.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="101"/>
        <source>&lt;p align=&quot;right&quot;&gt;Y field&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;right&quot;&gt;Y mező&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="122"/>
        <source>Name of the field containing y values</source>
        <translation>y értékeket tartalmazó mező neve</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="125"/>
        <source>Name of the field containing y values. Choose a field from the list. The list is generated by parsing the header row of the delimited text file.</source>
        <translation>y értékeket tartalmazó mező neve. Válassz egy mezőt a listából. A listát a szövegfájl fejléc sorának elemzés alapján állítottuk elő.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="145"/>
        <source>Delimited text file</source>
        <translation>Szövegfájl</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="152"/>
        <source>Full path to the delimited text file</source>
        <translation>Teljes elérési út a szövegfájlhoz</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="155"/>
        <source>Full path to the delimited text file. In order to properly parse the fields in the file, the delimiter must be defined prior to entering the file name. Use the Browse button to the right of this field to choose the input file.</source>
        <translation>Teljes elérési út a szövegfájlhoz. Az elválasztó karaktert a fájlnév megadása előtt kell megadnod, hogy pontosan értelmezhetők legyenek a fájlban szereplő mezők. Használd a tallóz gombot a mezőtől jobbra, a fájl kiválasztásához.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="168"/>
        <source>Browse to find the delimited text file to be processed</source>
        <translation>Tallózás a feldolgozandó szövegfájl megtalálásához</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="171"/>
        <source>Use this button to browse to the location of the delimited text file. This button will not be enabled until a delimiter has been entered in the &lt;i&gt;Delimiter&lt;/i&gt; box. Once a file is chosen, the X and Y field drop-down boxes will be populated with the fields from the delimited text file.</source>
        <translation>Használtd ezt a gombot a szövegfájl mappájába navigáláshoz. Ez a gomb addig nem aktív amíg az &lt;i&gt;Elválasztó&lt;/i&gt; mezőt ki nem töltötted. Amint választottál egy fájlt, az x és y mezők listáját feltölti a program a szövegfájl tartalma alapján.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="174"/>
        <source>Browse...</source>
        <translation>Tallóz...</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="191"/>
        <source>Layer name</source>
        <translation>Réteg név</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="198"/>
        <source>Name to display in the map legend</source>
        <translation>A jelmagyarázatban megjelenő név</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="201"/>
        <source>Name displayed in the map legend</source>
        <translation>A jelmagyarázatban megjelenített név</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="246"/>
        <source>Sample text</source>
        <translation>Minta szöveg</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="297"/>
        <source>Delimiter</source>
        <translation>Elválasztó</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="318"/>
        <source>Delimiter to use when splitting fields in the text file. The delimiter can be more than one character.</source>
        <translation>Elválasztó karakter a szövegfájl mezőinek feldarabolásához. Az elválasztó egynél több karakterből is állhat.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="321"/>
        <source>Delimiter to use when splitting fields in the delimited text file. The delimiter can be 1 or more characters in length.</source>
        <translation>Elválasztó karakter a szövegfájl mezőinek feldarabolásához. Az elválasztó egy vagy több karakter hosszú lehet.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="331"/>
        <source>The delimiter is taken as is</source>
        <translation>Az figyelembe vett elválasztó</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="334"/>
        <source>Plain characters</source>
        <translation>Sima karakterek</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="344"/>
        <source>The delimiter is a regular expression</source>
        <translation>Az elválasztó szabályos kifejezés</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="347"/>
        <source>Regular expression</source>
        <translation>Szabályos kifejezés</translation>
    </message>
</context>
<context>
    <name>QgsDelimitedTextProvider</name>
    <message>
        <location filename="../src/providers/delimitedtext/qgsdelimitedtextprovider.cpp" line="400"/>
        <source>Error</source>
        <translation>Hiba</translation>
    </message>
    <message>
        <location filename="../src/providers/delimitedtext/qgsdelimitedtextprovider.cpp" line="402"/>
        <source>Note: the following lines were not loaded because Qgis was unable to determine values for the x and y coordinates:
</source>
        <translation>Megjegyzés: a következő sorokat nem töltöttem be, mert nem tudtam meghatározni az x és y koordináták értékét:
</translation>
    </message>
</context>
<context>
    <name>QgsDlgPgBufferBase</name>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="13"/>
        <source>Buffer features</source>
        <translation>Elem övezetek</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="28"/>
        <source>Parameters</source>
        <translation>Paraméterek</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="59"/>
        <source>Geometry column:</source>
        <translation>Geometria oszlop:</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="66"/>
        <source>Add the buffered layer to the map?</source>
        <translation>Hozzáadjam az övezet réteget a térképhez?</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="92"/>
        <source>Spatial reference ID:</source>
        <translation>Térbeli referencia ID:</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="118"/>
        <source>Schema:</source>
        <translation>Séma:</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="125"/>
        <source>Unique field to use as feature id:</source>
        <translation>Egyedi mező amit elem ID-ként használhatok:</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="132"/>
        <source>Table name for the buffered layer:</source>
        <translation>Tábla név az övezet réteghez:</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="172"/>
        <source>Create unique object id</source>
        <translation>Egyedi objektum ID létrehozás</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="216"/>
        <source>public</source>
        <translation>nyilvános</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="224"/>
        <source>Buffer distance in map units:</source>
        <translation>Övezet távolság térkép egységekben:</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="234"/>
        <source>&lt;h2&gt;Buffer the features in layer: &lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Övezet a réteg elemeihez: &lt;/h2&gt;</translation>
    </message>
</context>
<context>
    <name>QgsEditReservedWordsBase</name>
    <message>
        <location filename="" line="145"/>
        <source>Edit Reserved Words</source>
        <translation type="obsolete">Fenntartott szavak szerkesztése</translation>
    </message>
    <message>
        <location filename="" line="145"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Double click the Column Name column to change the name of the column.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Kattints duplán az &quot;oszlopnév&quot; oszlopon az oszlop nevének módosításához.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="" line="145"/>
        <source>Status</source>
        <translation type="obsolete">Állapot</translation>
    </message>
    <message>
        <location filename="" line="145"/>
        <source>Column Name</source>
        <translation type="obsolete">Oszlopnév</translation>
    </message>
    <message>
        <location filename="" line="145"/>
        <source>Index</source>
        <translation type="obsolete">Index</translation>
    </message>
    <message>
        <location filename="" line="145"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;This shapefile contains reserved words. These may affect the import into PostgreSQL. Edit the column names so none of the reserved words listed at the right are used (click on a Column Name entry to edit). You may also change any other column name if desired.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Ez a shape fájl fenntartott szavakat tartalmaz. Ez befolyásolhatja a PostgreSQL importot. Módosítsd az oszlopneveket, hogy a jobboldali listában szereplõk fenntartott szavak közül egyik se használd. (kattint az oszlopnév elemre a szerkesztéshez). Más oszlopok nevét is megváltoztathatod, ha szükséges.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="" line="145"/>
        <source>Reserved Words</source>
        <translation type="obsolete">Fenntartott nevek</translation>
    </message>
</context>
<context>
    <name>QgsEditReservedWordsDialog</name>
    <message>
        <location filename="" line="145"/>
        <source>Status</source>
        <translation type="obsolete">Állapot</translation>
    </message>
    <message>
        <location filename="" line="145"/>
        <source>Column Name</source>
        <translation type="obsolete">Oszlopnév</translation>
    </message>
    <message>
        <location filename="" line="145"/>
        <source>Index</source>
        <translation type="obsolete">Index</translation>
    </message>
</context>
<context>
    <name>QgsEncodingFileDialog</name>
    <message>
        <location filename="../src/gui/qgsencodingfiledialog.cpp" line="29"/>
        <source>Encoding:</source>
        <translation>Kódolás:</translation>
    </message>
</context>
<context>
    <name>QgsFillStyleWidgetBase</name>
    <message>
        <location filename="../src/ui/qgsfillstylewidgetbase.ui" line="16"/>
        <source>Form1</source>
        <translation>Form1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsfillstylewidgetbase.ui" line="28"/>
        <source>Fill Style</source>
        <translation>Kitöltési minta</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsfillstylewidgetbase.ui" line="54"/>
        <source>col</source>
        <translation>oszl</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsfillstylewidgetbase.ui" line="61"/>
        <source>Colour:</source>
        <translation>Szín:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsfillstylewidgetbase.ui" line="72"/>
        <source>PolyStyleWidget</source>
        <translation>PolyStyleWidget</translation>
    </message>
</context>
<context>
    <name>QgsGPSDeviceDialog</name>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialog.cpp" line="43"/>
        <source>New device %1</source>
        <translation>Új eszköz %1</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialog.cpp" line="56"/>
        <source>Are you sure?</source>
        <translation>Biztos vagy benne?</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialog.cpp" line="57"/>
        <source>Are you sure that you want to delete this device?</source>
        <translation>Biztos, hogy törölni akarod ezt az eszközt?</translation>
    </message>
</context>
<context>
    <name>QgsGPSDeviceDialogBase</name>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="24"/>
        <source>GPS Device Editor</source>
        <translation>GPS eszköz szerkesztő</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="72"/>
        <source>New device</source>
        <translation>Új eszköz</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="87"/>
        <source>Delete device</source>
        <translation>Eszköz törlés</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="102"/>
        <source>Update device</source>
        <translation>Eszköz frissítés</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="135"/>
        <source>Device name:</source>
        <translation>Eszköz név:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="150"/>
        <source>This is the name of the device as it will appear in the lists</source>
        <translation>Az eszköz neve, ahogy a listában meg fog jelenni</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="159"/>
        <source>Commands</source>
        <translation>Parancsok</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="174"/>
        <source>Track download:</source>
        <translation>Track letöltés:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="181"/>
        <source>Route upload:</source>
        <translation>Útvonal feltöltés:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="188"/>
        <source>Waypoint download:</source>
        <translation>Útpont letöltés:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="195"/>
        <source>The command that is used to download routes from the device</source>
        <translation>Az eszközből az útvonalak letöltéséhez használt parancs </translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="202"/>
        <source>Route download:</source>
        <translation>Útvonal letöltés:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="209"/>
        <source>The command that is used to upload waypoints to the device</source>
        <translation>Az eszközbe az útpontok feltöltéséhez használt parancs</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="216"/>
        <source>Track upload:</source>
        <translation>Track feltöltés:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="223"/>
        <source>The command that is used to download tracks from the device</source>
        <translation>Az eszközből a trackek letöltéséhez használt parancs</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="230"/>
        <source>The command that is used to upload routes to the device</source>
        <translation>Az eszközbe az útvonalak feltöltéséhez használt parancs</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="237"/>
        <source>The command that is used to download waypoints from the device</source>
        <translation>Az eszközből az útpontok letöltéséhez használt parancs</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="244"/>
        <source>The command that is used to upload tracks to the device</source>
        <translation>Az eszközbe a trackek feltöltésére használt parancs</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="251"/>
        <source>Waypoint upload:</source>
        <translation>Útpont feltöltés:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="269"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;In the download and upload commands there can be special words that will be replaced by QGIS when the commands are used. These words are:&lt;span style=&quot; font-style:italic;&quot;&gt;%babel&lt;/span&gt; - the path to GPSBabel&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%in&lt;/span&gt; - the GPX filename when uploading or the port when downloading&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%out&lt;/span&gt; - the port when uploading or the GPX filename when downloading&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;A letöltés és feltöltés parancsokban speciális szavak lehetnek, melyeket a QGIS lecserél a parancsok végrehajtása közben. Ezek a szavak a következõk:&lt;span style=&quot; font-style:italic;&quot;&gt;%babel&lt;/span&gt; - az elérési út a GPSBabelhez&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%in&lt;/span&gt; - a GPX fájl neve feltöltésnél vagy a port neve letöltésnél&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%out&lt;/span&gt; - a port név feltöltésnél vagy a GPX fájl neve letöltésnél&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="301"/>
        <source>Close</source>
        <translation>Lezár</translation>
    </message>
</context>
<context>
    <name>QgsGPSPlugin</name>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="92"/>
        <source>&amp;Gps Tools</source>
        <translation>&amp;GPS eszközök</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="93"/>
        <source>&amp;Create new GPX layer</source>
        <translation>&amp;Új GPX réteg létrehozás</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="96"/>
        <source>Creates a new GPX layer and displays it on the map canvas</source>
        <translation>Új GPX réteg létrehozás és megjelenítés a térképen</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="195"/>
        <source>&amp;Gps</source>
        <translation>&amp;GPS</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="159"/>
        <source>Save new GPX file as...</source>
        <translation>GPX fájl mentés másként...</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="159"/>
        <source>GPS eXchange file (*.gpx)</source>
        <translation>GPS adatcsere fájl (*.gpx)</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="164"/>
        <source>Could not create file</source>
        <translation>Nem tudom létrehozni a fájlt</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="166"/>
        <source>Unable to create a GPX file with the given name. </source>
        <translation>A megadott névvel nem tudom létrehozni a GPX fájlt.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="167"/>
        <source>Try again with another name or in another </source>
        <translation>Próbáld újra egy másik névvel vagy egy másik</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="167"/>
        <source>directory.</source>
        <translation>könyvtárban.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="206"/>
        <source>GPX Loader</source>
        <translation>GPX betöltés</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="208"/>
        <source>Unable to read the selected file.
</source>
        <translation>Nem tudom olvasni a megadott fájlt.
</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="208"/>
        <source>Please reselect a valid file.</source>
        <translation>Kérem válassz egy megfelelő fájlt.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="491"/>
        <source>Could not start process</source>
        <translation>Nem tudom elkjezdeni a feldolgozást</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="492"/>
        <source>Could not start GPSBabel!</source>
        <translation>Nem tudom elindítani a GPSBabel programot!</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="330"/>
        <source>Importing data...</source>
        <translation>Adatok importálása...</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="497"/>
        <source>Cancel</source>
        <translation>Mégsem</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="275"/>
        <source>Could not import data from %1!

</source>
        <translation>Nem tudom importálni az adatokat ebből %1!</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="277"/>
        <source>Error importing data</source>
        <translation>Adat import hiba</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="345"/>
        <source>Could not convert data from %1!

</source>
        <translation>Nem tudom konvertálni az adatokat ebből %1!</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="347"/>
        <source>Error converting data</source>
        <translation>Adat konverzió hiba</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="481"/>
        <source>Not supported</source>
        <translation>Nem támogatott</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="397"/>
        <source>This device does not support downloading </source>
        <translation>Ez az eszköz nem támogatja a letöltést</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="397"/>
        <source>of </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="412"/>
        <source>Downloading data...</source>
        <translation>Adatletöltés...</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="426"/>
        <source>Could not download data from GPS!

</source>
        <translation>Nem tudom letölteni az adatokat a GPS-ből!

</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="428"/>
        <source>Error downloading data</source>
        <translation>Hiba az adatletöltésben</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="482"/>
        <source>This device does not support uploading of </source>
        <translation>Az eszköz nem támogatja a feltöltést</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="497"/>
        <source>Uploading data...</source>
        <translation>Adatfeltöltés...</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="511"/>
        <source>Error while uploading data to GPS!

</source>
        <translation>Hiba az adatfeltöltés közben a GPS-be!

</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="513"/>
        <source>Error uploading data</source>
        <translation>Hiba az adatfeltöltésben</translation>
    </message>
</context>
<context>
    <name>QgsGPSPluginGui</name>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="480"/>
        <source>Waypoints</source>
        <translation>Útpontok</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="480"/>
        <source>Routes</source>
        <translation>Útvonalak</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="266"/>
        <source>Tracks</source>
        <translation>Trackek</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="516"/>
        <source>Choose a filename to save under</source>
        <translation>Válassz egy fájlnevet</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="518"/>
        <source>GPS eXchange format (*.gpx)</source>
        <translation>GPS adatcsere fájl (*.gpx)</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="505"/>
        <source>Select GPX file</source>
        <translation>Válassz GPX fájlt</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="244"/>
        <source>Select file and format to import</source>
        <translation>Válassz fájlt és formátumot az importhoz</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="416"/>
        <source>GPX is the %1, which is used to store information about waypoints, routes, and tracks.</source>
        <translation>GPX a(z) %1, melyet útpontok, útvonalak és trackek tárolására használunk.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="416"/>
        <source>GPS eXchange file format</source>
        <translation>GPS adatcsere fájl formátum</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="417"/>
        <source>Select a GPX file and then select the feature types that you want to load.</source>
        <translation>Válaszd ki a GPX fájlt és válaszd ki a betöltendő elemtípust.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="429"/>
        <source>This tool will help you download data from a GPS device.</source>
        <translation>Ez az eszköz segít az adatok GPS-ről történő letöltésében.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="430"/>
        <source>Choose your GPS device, the port it is connected to, the feature type you want to download, a name for your new layer, and the GPX file where you want to store the data.</source>
        <translation>Válaszd ki a GPS eszközödet, a portot ahová csatlakozik, a betöltendő elemtípust, az új réteg nevét, és a GPX fájlt melyben az adatokat tárolni akarod.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="449"/>
        <source>If your device isn&apos;t listed, or if you want to change some settings, you can also edit the devices.</source>
        <translation>Ha az eszközöd nincs a listában vagy ha változtatni akarsz a beállításokon, akkor az eszközöket is szerkesztheted.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="451"/>
        <source>This tool uses the program GPSBabel (%1) to transfer the data.</source>
        <translation>Ez az eszköz a GPSBabel (%1) programot használja az átvitelhez.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="486"/>
        <source>This requires that you have GPSBabel installed where QGIS can find it.</source>
        <translation>Ehhez a GPDBabelt telepítened kell oda, ahol a QGIS megtalálja.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="447"/>
        <source>This tool will help you upload data from a GPX layer to a GPS device.</source>
        <translation>Ez az eszköz segít a GPX réteg adatainak feltöltésében a GPS eszközre.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="448"/>
        <source>Choose the layer you want to upload, the device you want to upload it to, and the port your device is connected to.</source>
        <translation>Válaszd ki a feltöltendő réteget, az eszközt, melyre fel akarod tölteni, és a portot ahová az eszköz csatlakozik.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="466"/>
        <source>QGIS can only load GPX files by itself, but many other formats can be converted to GPX using GPSBabel (%1).</source>
        <translation>A QGIS csak GPX fájlokat tud betölteni, de sok más formátum átalakítható GPX formátumba a GPSBabel (%1) segítségével.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="468"/>
        <source>Select a GPS file format and the file that you want to import, the feature type that you want to use, a GPX filename that you want to save the converted file as, and a name for the new layer.</source>
        <translation>Válassz egy GPS fájl formátumot és a fájlt amit importálni akarsz, az elem típust amit használni akarsz. egy GPX fájlnevet amibe menteni akarod a konvertált fájlt, és egy nevet az új rétegnek.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="469"/>
        <source>All file formats can not store waypoints, routes, and tracks, so some feature types may be disabled for some file formats.</source>
        <translation>Nem lehet minden fájl formátumban útpontokat, útvonalakat és trackeket tárolni, így néhány elemtípus ki lehet kapcsolva néhány fájl formátum esetén.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="485"/>
        <source>QGIS can perform conversions of GPX files, by using GPSBabel (%1) to perform the conversions.</source>
        <translation>QGIS a GPSBabel (%1)felhasználásával konvertálja a GPX fájlokat.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="487"/>
        <source>Select a GPX input file name, the type of conversion you want to perform, a GPX filename that you want to save the converted file as, and a name for the new layer created from the result.</source>
        <translation>Válassz egy GPX input fájl nevet, a végrehajtandó konverzió típusát, egy GPX fáj nevet amibe a konvertált fájlt menteni akarod, és egy nevet a létrehozott új réteg számára.</translation>
    </message>
</context>
<context>
    <name>QgsGPSPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="13"/>
        <source>GPS Tools</source>
        <translation>GPS eszközök</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="58"/>
        <source>Load GPX file</source>
        <translation>GPX fájl betöltés</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="94"/>
        <source>File:</source>
        <translation>Fájl:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="572"/>
        <source>Browse...</source>
        <translation>Tallóz...</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="111"/>
        <source>Feature types:</source>
        <translation>Elem típusok:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="314"/>
        <source>Waypoints</source>
        <translation>Útpontok</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="319"/>
        <source>Routes</source>
        <translation>Útvonalak</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="324"/>
        <source>Tracks</source>
        <translation>Trackek</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="158"/>
        <source>Import other file</source>
        <translation>Más fájl importálás</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="558"/>
        <source>Layer name:</source>
        <translation>Réteg név:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="565"/>
        <source>Save As...</source>
        <translation>Mentés másként...</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="582"/>
        <source>GPX output file:</source>
        <translation>GPX eredmény fájl:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="339"/>
        <source>Feature type:</source>
        <translation>Elem típus:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="250"/>
        <source>(Note: Selecting correct file type in browser dialog important!)</source>
        <translation>(Megjegyzés: a tallózás párbeszédablakban fontos a helyes fájl típus kiválasztása!)</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="264"/>
        <source>File to import:</source>
        <translation>Iportálandó fájl:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="272"/>
        <source>Download from GPS</source>
        <translation>Letöltés a GPS-ből</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="481"/>
        <source>Port:</source>
        <translation>Port:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="356"/>
        <source>Output file:</source>
        <translation>Eredmény fájl:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="515"/>
        <source>GPS device:</source>
        <translation>GPS eszköz:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="508"/>
        <source>Edit devices</source>
        <translation>Eszközök szerkesztése</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="430"/>
        <source>Upload to GPS</source>
        <translation>Feltöltés a GPS-be</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="522"/>
        <source>Data layer:</source>
        <translation>Adat réteg:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="530"/>
        <source>GPX Conversions</source>
        <translation>GPX konverziók</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="615"/>
        <source>Conversion:</source>
        <translation>Konverzió:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="629"/>
        <source>GPX input file:</source>
        <translation>GPX input fájl:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="545"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
</context>
<context>
    <name>QgsGPXProvider</name>
    <message>
        <location filename="../src/providers/gpx/qgsgpxprovider.cpp" line="69"/>
        <source>Bad URI - you need to specify the feature type.</source>
        <translation>Hibás URI - meg kell adnod az elem típust.</translation>
    </message>
    <message>
        <location filename="../src/providers/gpx/qgsgpxprovider.cpp" line="113"/>
        <source>GPS eXchange file</source>
        <translation>GPS adatcsere fájl</translation>
    </message>
    <message>
        <location filename="../src/providers/gpx/qgsgpxprovider.cpp" line="730"/>
        <source>Digitized in QGIS</source>
        <translation>QGIS-ben digitalizált</translation>
    </message>
</context>
<context>
    <name>QgsGeomTypeDialog</name>
    <message>
        <location filename="../src/app/qgsgeomtypedialog.cpp" line="31"/>
        <source>Name</source>
        <translation>Név</translation>
    </message>
    <message>
        <location filename="../src/app/qgsgeomtypedialog.cpp" line="32"/>
        <source>Type</source>
        <translation>Típus</translation>
    </message>
</context>
<context>
    <name>QgsGeomTypeDialogBase</name>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="13"/>
        <source>New Vector Layer</source>
        <translation>Új vektor réteg</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="28"/>
        <source>File Format:</source>
        <translation>Fájl formátum:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="41"/>
        <source>Remove</source>
        <translation>Töröl</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="64"/>
        <source>Attributes:</source>
        <translation>Attribútumok:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="74"/>
        <source>Add</source>
        <translation>Hozzáad</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="81"/>
        <source>Type</source>
        <translation>Típus</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="93"/>
        <source>Point</source>
        <translation>Pont</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="100"/>
        <source>Line</source>
        <translation>Vonal</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="107"/>
        <source>Polygon</source>
        <translation>Felület</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="118"/>
        <source>Column 1</source>
        <translation>1. oszlop</translation>
    </message>
</context>
<context>
    <name>QgsGeorefDescriptionDialogBase</name>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefdescriptiondialogbase.ui" line="13"/>
        <source>Description georeferencer</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefdescriptiondialogbase.ui" line="44"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:12pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:11pt; font-weight:600;&quot;&gt;Description&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:9pt;&quot;&gt;This plugin can generate world files for rasters. You select points on the raster and give their world coordinates, and the plugin will compute the world file parameters. The more coordinates you can provide the better the result will be.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:12pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:11pt; font-weight:600;&quot;&gt;Leírás&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:9pt;&quot;&gt;Ez a modul world fájlt generál a raszterekhez. Kijelölhetsz pontokat a raszteren és megadhatod a koordinátájukat ezután a modul  kiszámítja a world fájl paraméterek értékét. Minél több koordinátát adsz meg, annál jobb lesz az eredmény.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
</context>
<context>
    <name>QgsGeorefPlugin</name>
    <message>
        <location filename="../src/plugins/georeferencer/plugin.cpp" line="119"/>
        <source>&amp;Georeferencer</source>
        <translation>&amp;Georeferáló</translation>
    </message>
</context>
<context>
    <name>QgsGeorefPluginGui</name>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="85"/>
        <source>Choose a raster file</source>
        <translation>Válassz egy raszter fájlt</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="87"/>
        <source>Raster files (*.*)</source>
        <translation>Raszter fájlok (*.*)</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="97"/>
        <source>Error</source>
        <translation>Hiba</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="98"/>
        <source>The selected file is not a valid raster file.</source>
        <translation>A kiválaszott fájl nem megfelelő raszter fájl.</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="122"/>
        <source>World file exists</source>
        <translation>World fájl létezik</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="124"/>
        <source>&lt;p&gt;The selected file already seems to have a </source>
        <translation>&lt;p&gt;Úgy tűnik a  kiválasztott fájl már rendelkezik </translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="125"/>
        <source>world file! Do you want to replace it with the </source>
        <translation>World fájllal! Le akarod cserélni az új</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="125"/>
        <source>new world file?&lt;/p&gt;</source>
        <translation>az új world fájllal?&lt;/p&gt;</translation>
    </message>
</context>
<context>
    <name>QgsGeorefPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/georeferencer/pluginguibase.ui" line="13"/>
        <source>Georeferencer</source>
        <translation>Georeferáló</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/pluginguibase.ui" line="28"/>
        <source>Arrange plugin windows</source>
        <translation>Modul ablak elrendezése</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/pluginguibase.ui" line="43"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/pluginguibase.ui" line="62"/>
        <source>Raster file:</source>
        <translation>Raszter fájl:</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/pluginguibase.ui" line="77"/>
        <source>Description...</source>
        <translation>Leírás...</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/pluginguibase.ui" line="100"/>
        <source>Close</source>
        <translation>Lezár</translation>
    </message>
</context>
<context>
    <name>QgsGeorefWarpOptionsDialog</name>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialog.cpp" line="27"/>
        <source>unstable</source>
        <translation>instabil</translation>
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
        <translation>Tömörítés:</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="35"/>
        <source>Resampling method:</source>
        <translation>Újramintavételezési módszer:</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="46"/>
        <source>Nearest neighbour</source>
        <translation>Legközelebbi szomszéd</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="51"/>
        <source>Linear</source>
        <translation>Lineáris</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="56"/>
        <source>Cubic</source>
        <translation>Köbös</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="64"/>
        <source>Use 0 for transparency when needed</source>
        <translation>Használ 0-t az átláthatósághoz amikor szükséges</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="74"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
</context>
<context>
    <name>QgsGraduatedSymbolDialog</name>
    <message>
        <location filename="../src/app/qgsgraduatedsymboldialog.cpp" line="327"/>
        <source>Equal Interval</source>
        <translation>Egyenlő intervallumok</translation>
    </message>
    <message>
        <location filename="../src/app/qgsgraduatedsymboldialog.cpp" line="304"/>
        <source>Quantiles</source>
        <translation>Egyenlõ számú</translation>
    </message>
    <message>
        <location filename="../src/app/qgsgraduatedsymboldialog.cpp" line="351"/>
        <source>Empty</source>
        <translation>Üres</translation>
    </message>
</context>
<context>
    <name>QgsGraduatedSymbolDialogBase</name>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="25"/>
        <source>graduated Symbol</source>
        <translation>növekvő szimbólumok</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="73"/>
        <source>Classification Field:</source>
        <translation>Osztályzás mező:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="89"/>
        <source>Mode:</source>
        <translation>Mód:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="105"/>
        <source>Number of Classes:</source>
        <translation>Osztrályok száma:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="159"/>
        <source>Delete class</source>
        <translation>Osztály törlés</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="166"/>
        <source>Classify</source>
        <translation>Osztályoz</translation>
    </message>
</context>
<context>
    <name>QgsGrassAttributes</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="152"/>
        <source>Column</source>
        <translation>Oszlop</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="153"/>
        <source>Value</source>
        <translation>Érték</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="154"/>
        <source>Type</source>
        <translation>Típus</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="158"/>
        <source>Layer</source>
        <translation>Réteg</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="300"/>
        <source>Warning</source>
        <translation>Figyelmeztetés</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="301"/>
        <source>ERROR</source>
        <translation>Hiba</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="303"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
</context>
<context>
    <name>QgsGrassAttributesBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="48"/>
        <source>GRASS Attributes</source>
        <translation>GRASS attribútumok</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="78"/>
        <source>Tab 1</source>
        <translation>TAB 1</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="112"/>
        <source>result</source>
        <translation>eredmény</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="177"/>
        <source>Update database record</source>
        <translation>Adatbázis rekord frissítés</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="180"/>
        <source>Update</source>
        <translation>Aktualizálás</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="207"/>
        <source>Add new category using settings in GRASS Edit toolbox</source>
        <translation>Új kategória hozzáadás a GRASS szerkesztés eszköztár beállításainak felhasználásával</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="210"/>
        <source>New</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="237"/>
        <source>Delete selected category</source>
        <translation>Kivlasztott kategória törlése</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="240"/>
        <source>Delete</source>
        <translation>Törlés</translation>
    </message>
</context>
<context>
    <name>QgsGrassBrowser</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="66"/>
        <source>Tools</source>
        <translation>Eszközök</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="71"/>
        <source>Add selected map to canvas</source>
        <translation>Szelektált térkép hozzáadása</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="79"/>
        <source>Copy selected map</source>
        <translation>Szelektált térkép másolása</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="87"/>
        <source>Rename selected map</source>
        <translation>Szeletált térkép átnevezése</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="95"/>
        <source>Delete selected map</source>
        <translation>Szelektált térkép törlése</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="103"/>
        <source>Set current region to selected map</source>
        <translation>Aktuális terjedelem beállítása a szelektált térképre</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="111"/>
        <source>Refresh</source>
        <translation>Frissítés</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="339"/>
        <source>New name</source>
        <translation>Új név</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="453"/>
        <source>Warning</source>
        <translation>Figyelmeztetés</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="289"/>
        <source>Cannot copy map </source>
        <translation>Nem tudom másolni a térképet</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="411"/>
        <source>&lt;br&gt;command: </source>
        <translation>&lt;br&gt;parancs:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="355"/>
        <source>Cannot rename map </source>
        <translation>Nem tudom átnevezni a térképet</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="393"/>
        <source>Delete map &lt;b&gt;</source>
        <translation>Térkép törlés &lt;b&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="410"/>
        <source>Cannot delete map </source>
        <translation>Nem tudom töröln i a térképet</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="454"/>
        <source>Cannot write new region</source>
        <translation>Nem tudom az új terjedelmet kiírni</translation>
    </message>
</context>
<context>
    <name>QgsGrassEdit</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1458"/>
        <source>Warning</source>
        <translation>Figyelmeztetés</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="201"/>
        <source>You are not owner of the mapset, cannot open the vector for editing.</source>
        <translation>Nem vagy a tulajdonosa, nem nyithatod meg a vektort módosításra.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="206"/>
        <source>Cannot open vector for update.</source>
        <translation>Nem tudom megnyitni a vektort módosításra.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="214"/>
        <source>Edit tools</source>
        <translation>Szerkesztő eszközök</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="217"/>
        <source>New point</source>
        <translation>Új pont</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="223"/>
        <source>New line</source>
        <translation>Új vonal</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="229"/>
        <source>New boundary</source>
        <translation>Új határ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="235"/>
        <source>New centroid</source>
        <translation>Új centrális</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="241"/>
        <source>Move vertex</source>
        <translation>Töréspont mozgatás</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="247"/>
        <source>Add vertex</source>
        <translation>Töréspont hozzáadás</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="253"/>
        <source>Delete vertex</source>
        <translation>Töréspont törlés</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="259"/>
        <source>Move element</source>
        <translation>Elem mozgatás</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="265"/>
        <source>Split line</source>
        <translation>Vonal darabolás</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="271"/>
        <source>Delete element</source>
        <translation>Elem törlés</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="277"/>
        <source>Edit attributes</source>
        <translation>Attribútumok szerkesztése</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="282"/>
        <source>Close</source>
        <translation>Lezár</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="363"/>
        <source>Background</source>
        <translation>Háttér</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="364"/>
        <source>Highlight</source>
        <translation>Kiemel</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="365"/>
        <source>Dynamic</source>
        <translation>Dinamikus</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="366"/>
        <source>Point</source>
        <translation>Pont</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="367"/>
        <source>Line</source>
        <translation>Vonal</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="368"/>
        <source>Boundary (no area)</source>
        <translation>Határ (nem terület)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="369"/>
        <source>Boundary (1 area)</source>
        <translation>Határ (1 terület)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="370"/>
        <source>Boundary (2 areas)</source>
        <translation>Határ (2 terület)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="371"/>
        <source>Centroid (in area)</source>
        <translation>Centrális (területben)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="372"/>
        <source>Centroid (outside area)</source>
        <translation>Centrális (területen kívül)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="373"/>
        <source>Centroid (duplicate in area)</source>
        <translation>Centrális (dupla a területben)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="374"/>
        <source>Node (1 line)</source>
        <translation>Csomópont (1 vonal)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="375"/>
        <source>Node (2 lines)</source>
        <translation>Csomópont (2 vonal)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="408"/>
        <source>Disp</source>
        <comment>Column title</comment>
        <translation>Megj</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="410"/>
        <source>Color</source>
        <comment>Column title</comment>
        <translation>Szín</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="412"/>
        <source>Type</source>
        <comment>Column title</comment>
        <translation>Típus</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="414"/>
        <source>Index</source>
        <comment>Column title</comment>
        <translation>Index</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="444"/>
        <source>Column</source>
        <translation>Oszlop</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="445"/>
        <source>Type</source>
        <translation>Típus</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="446"/>
        <source>Length</source>
        <translation>Hossz</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="501"/>
        <source>Next not used</source>
        <translation>Következő nem használt</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="502"/>
        <source>Manual entry</source>
        <translation>Kézi bevitel</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="503"/>
        <source>No category</source>
        <translation>Nincs kategória</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="665"/>
        <source>Info</source>
        <translation>Info</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="665"/>
        <source>The table was created</source>
        <translation>A táblát létrehoztam</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1333"/>
        <source>Tool not yet implemented.</source>
        <translation>Még nem implementált eszköz.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1357"/>
        <source>Cannot check orphan record: </source>
        <translation>Nem tudom ellenőrizni az árva rekordot:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1364"/>
        <source>Orphan record was left in attribute table. &lt;br&gt;Delete the record?</source>
        <translation>Árva rekord maradt a táblában. &lt;br&gt;Töröljem?</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1373"/>
        <source>Cannot delete orphan record: </source>
        <translation>Nem tudom törölni az árva rekordot:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1401"/>
        <source>Cannot describe table for field </source>
        <translation>Nem tudom a mezőhöz tartozó táblát leírni</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1821"/>
        <source>Left: </source>
        <translation>Bal:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1822"/>
        <source>Middle: </source>
        <translation>Közép:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1823"/>
        <source>Right: </source>
        <translation>Jobb:</translation>
    </message>
</context>
<context>
    <name>QgsGrassEditBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="16"/>
        <source>GRASS Edit</source>
        <translation>GRASS szerkesztés</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="106"/>
        <source>Category</source>
        <translation>Kategória</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="66"/>
        <source>Mode</source>
        <translation>Mód</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="410"/>
        <source>Layer</source>
        <translation>Réteg</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="170"/>
        <source>Settings</source>
        <translation>Beállítások</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="190"/>
        <source>Snapping in screen pixels</source>
        <translation>Raszter képpontokban</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="241"/>
        <source>Symbology</source>
        <translation>Megjelenés</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="274"/>
        <source>Column 1</source>
        <translation>Oszlop 1</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="298"/>
        <source>Line width</source>
        <translation>Vonalvastagság</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="325"/>
        <source>Marker size</source>
        <translation>Jelméret</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="360"/>
        <source>Table</source>
        <translation>Tábla</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="496"/>
        <source>Add Column</source>
        <translation>Oszlop hozzáadás</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="511"/>
        <source>Create / Alter Table</source>
        <translation>Tábla létrehozás/módosítás</translation>
    </message>
</context>
<context>
    <name>QgsGrassElementDialog</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="132"/>
        <source>Cancel</source>
        <translation>Mégsem</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="163"/>
        <source>Ok</source>
        <translation>Ok</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="168"/>
        <source>&lt;font color=&apos;red&apos;&gt;Enter a name!&lt;/font&gt;</source>
        <translation>&lt;font color=&apos;red&apos;&gt;Add meg a nevet!&lt;/font&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="179"/>
        <source>&lt;font color=&apos;red&apos;&gt;This is name of the source!&lt;/font&gt;</source>
        <translation>&lt;font color=&apos;red&apos;&gt;Ez a forrás neve!&lt;/font&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="185"/>
        <source>&lt;font color=&apos;red&apos;&gt;Exists!&lt;/font&gt;</source>
        <translation>&lt;font color=&apos;red&apos;&gt;Létezik!&lt;/font&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="186"/>
        <source>Overwrite</source>
        <translation>Felülír</translation>
    </message>
</context>
<context>
    <name>QgsGrassMapcalc</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="111"/>
        <source>Mapcalc tools</source>
        <translation>Mapcalc eszköz</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="114"/>
        <source>Add map</source>
        <translation>Térkép hozzáadás</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="121"/>
        <source>Add constant value</source>
        <translation>Konstans érték hozzáadás</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="128"/>
        <source>Add operator or function</source>
        <translation>Művelet vagy függvény hozzáadás</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="135"/>
        <source>Add connection</source>
        <translation>Kapcsolat hozzáadás</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="142"/>
        <source>Select item</source>
        <translation>Válassz elemet</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="149"/>
        <source>Delete selected item</source>
        <translation>Szelektált elemek törlése</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="159"/>
        <source>Open</source>
        <translation>Megnyitás</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="164"/>
        <source>Save</source>
        <translation>Mentés</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="170"/>
        <source>Save as</source>
        <translation>Mentés másként</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="178"/>
        <source>Addition</source>
        <translation>Összeadás</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="179"/>
        <source>Subtraction</source>
        <translation>Kivonás</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="180"/>
        <source>Multiplication</source>
        <translation>Szorzás</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="181"/>
        <source>Division</source>
        <translation>Osztás</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="182"/>
        <source>Modulus</source>
        <translation>Maradék</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="183"/>
        <source>Exponentiation</source>
        <translation>Exponenciális</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="186"/>
        <source>Equal</source>
        <translation>Egyenlő</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="187"/>
        <source>Not equal</source>
        <translation>Nem egyenlő</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="188"/>
        <source>Greater than</source>
        <translation>Nagyobb mint</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="189"/>
        <source>Greater than or equal</source>
        <translation>Nagyobb vagy egyenlő</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="190"/>
        <source>Less than</source>
        <translation>Kisebb mint</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="191"/>
        <source>Less than or equal</source>
        <translation>Kisebb vagy egyenlő</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="192"/>
        <source>And</source>
        <translation>És</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="193"/>
        <source>Or</source>
        <translation>Vagy</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="196"/>
        <source>Absolute value of x</source>
        <translation>x abszolút értéke</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="197"/>
        <source>Inverse tangent of x (result is in degrees)</source>
        <translation>x arc tangense (eredmény fokokban)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="198"/>
        <source>Inverse tangent of y/x (result is in degrees)</source>
        <translation>y/x arc tangense (eredmény fokokban)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="199"/>
        <source>Current column of moving window (starts with 1)</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="200"/>
        <source>Cosine of x (x is in degrees)</source>
        <translation>x koszinusza (x fokokban)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="201"/>
        <source>Convert x to double-precision floating point</source>
        <translation>x konvertálása duplapontos lebegőpontos számmá</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="202"/>
        <source>Current east-west resolution</source>
        <translation>Aktuális kelet-nyugat felbontás</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="203"/>
        <source>Exponential function of x</source>
        <translation>x exponenciális függvénye</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="204"/>
        <source>x to the power y</source>
        <translation>x az y hatványon</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="205"/>
        <source>Convert x to single-precision floating point</source>
        <translation>x konvertálása szimplapontos lebegőpontos számmá</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="206"/>
        <source>Decision: 1 if x not zero, 0 otherwise</source>
        <translation>Döntés: 1 ha x nem nulla, különben 0</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="207"/>
        <source>Decision: a if x not zero, 0 otherwise</source>
        <translation>Döntés: a ha x nem nulla, különben 0</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="208"/>
        <source>Decision: a if x not zero, b otherwise</source>
        <translation>Döntés: a ha x nem nulla, különben b</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="209"/>
        <source>Decision: a if x &gt; 0, b if x is zero, c if x &lt; 0</source>
        <translation>Döntés: a ha x &gt; 0, b ha x nulla, c ha x &lt; 0</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="210"/>
        <source>Convert x to integer [ truncates ]</source>
        <translation>x egésszé konvertálása (csonkítás)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="211"/>
        <source>Check if x = NULL</source>
        <translation>Ellenőrzés x = NULL</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="212"/>
        <source>Natural log of x</source>
        <translation>x természetes logaritmusa</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="213"/>
        <source>Log of x base b</source>
        <translation>x b alapú logaritmusa</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="215"/>
        <source>Largest value</source>
        <translation>Legnagyobb érték</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="217"/>
        <source>Median value</source>
        <translation>Medián érték</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="219"/>
        <source>Smallest value</source>
        <translation>Legkisebb érték</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="221"/>
        <source>Mode value</source>
        <translation>Mód érték</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="222"/>
        <source>1 if x is zero, 0 otherwise</source>
        <translation>1 ha x nulla, különben 0</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="223"/>
        <source>Current north-south resolution</source>
        <translation>Aktuális észak-dél felbontás</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="224"/>
        <source>NULL value</source>
        <translation>NULL érték</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="225"/>
        <source>Random value between a and b</source>
        <translation>Véletlenszám a és b között</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="226"/>
        <source>Round x to nearest integer</source>
        <translation>x kerekítése a legközelebbi egészre</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="227"/>
        <source>Current row of moving window (Starts with 1)</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="228"/>
        <source>Sine of x (x is in degrees)</source>
        <comment>sin(x)</comment>
        <translation>x szinusza (x fokokban)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="229"/>
        <source>Square root of x</source>
        <comment>sqrt(x)</comment>
        <translation>x négyzetgyöke</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="230"/>
        <source>Tangent of x (x is in degrees)</source>
        <comment>tan(x)</comment>
        <translation>x tangense (x fokokban)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="231"/>
        <source>Current x-coordinate of moving window</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="232"/>
        <source>Current y-coordinate of moving window</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1388"/>
        <source>Output</source>
        <translation>Eredmény</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1317"/>
        <source>Warning</source>
        <translation>Figyelmeztetés</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="584"/>
        <source>Cannot get current region</source>
        <translation>Nem tudom megszerezni az aktuális terjedelmet</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="561"/>
        <source>Cannot check region of map </source>
        <translation>Nem tudom ellenőrizni a térkép terjedelmét</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="617"/>
        <source>Cannot get region of map </source>
        <translation>Nem tudom megszerezni a térkép terjedelmét</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="813"/>
        <source>No GRASS raster maps currently in QGIS</source>
        <translation>Nincs GRASS raszter térkép a QGIS-ben</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1102"/>
        <source>Cannot create &apos;mapcalc&apos; directory in current mapset.</source>
        <translation>Nem tudom létrehozni a &apos;mapcalc&apos; mappát az aktuális térkép készletben.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1112"/>
        <source>New mapcalc</source>
        <translation>Uj mapcalc</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1113"/>
        <source>Enter new mapcalc name:</source>
        <translation>Add meg az új mapcalc nevét:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1118"/>
        <source>Enter vector name</source>
        <translation>Add meg a vektor nevét</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1126"/>
        <source>The file already exists. Overwrite? </source>
        <translation>A fájl már létezik. Felülirhatom?</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1164"/>
        <source>Save mapcalc</source>
        <translation>mapcalc mentés</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1146"/>
        <source>File name empty</source>
        <translation>Fájlnév üres</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1165"/>
        <source>Cannot open mapcalc file</source>
        <translation>Nem tudom megnyíitni a mapcalc fájlt</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1295"/>
        <source>The mapcalc schema (</source>
        <translation>A mapcalc séma (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1295"/>
        <source>) not found.</source>
        <translation>) nem találom.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1302"/>
        <source>Cannot open mapcalc schema (</source>
        <translation>Nem tudom megnyitni a mapcalc sémát (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1313"/>
        <source>Cannot read mapcalc schema (</source>
        <translation>Nem tudom olvasni a mapcalc sémát (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1314"/>
        <source>
at line </source>
        <translation>
sorban</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1315"/>
        <source> column </source>
        <translation>oszlop</translation>
    </message>
</context>
<context>
    <name>QgsGrassMapcalcBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalcbase.ui" line="16"/>
        <source>MainWindow</source>
        <translation>Fő ablak</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalcbase.ui" line="37"/>
        <source>Output</source>
        <translation>Eredmény</translation>
    </message>
</context>
<context>
    <name>QgsGrassModule</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="205"/>
        <source>Module</source>
        <translation>Modul</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1352"/>
        <source>Warning</source>
        <translation>Figyelmeztetés</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="220"/>
        <source>The module file (</source>
        <translation>A modul fájl (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="220"/>
        <source>) not found.</source>
        <translation>) nem találom.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="224"/>
        <source>Cannot open module file (</source>
        <translation>Nem tudom megnyitni a modul fájlt (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="992"/>
        <source>)</source>
        <translation>)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="987"/>
        <source>Cannot read module file (</source>
        <translation>Nem tudom olvasni a modul fájlt (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="987"/>
        <source>):
</source>
        <translation>):
</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="988"/>
        <source>
at line </source>
        <translation>
sorban</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="260"/>
        <source>Module </source>
        <translation>Modul</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="260"/>
        <source> not found</source>
        <translation>nem találom</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="301"/>
        <source>Cannot find man page </source>
        <translation>Nem találom a kézikönyvet </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="978"/>
        <source>Not available, description not found (</source>
        <translation>Nem érhető el, a leírást nem találom (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="981"/>
        <source>Not available, cannot open description (</source>
        <translation>Nem érhető el, nem tudom megnyitni a leírást (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="988"/>
        <source> column </source>
        <translation>oszlop</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="992"/>
        <source>Not available, incorrect description (</source>
        <translation>Nem érhető el, hibás leírás (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1380"/>
        <source>Run</source>
        <translation>Futtatás</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1179"/>
        <source>Cannot get input region</source>
        <translation>Nem tudom beszerezni az input terjedelemet</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1167"/>
        <source>Use Input Region</source>
        <translation>Az input terjedelmet használom</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1281"/>
        <source>Cannot find module </source>
        <translation>Nem találom a modult</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1353"/>
        <source>Cannot start module: </source>
        <translation>Nem tudom elindítani a modult:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1358"/>
        <source>Stop</source>
        <translation>Megállít</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1369"/>
        <source>&lt;B&gt;Successfully finished&lt;/B&gt;</source>
        <translation>&lt;B&gt;Sikeresen befejeztem&lt;/B&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1375"/>
        <source>&lt;B&gt;Finished with error&lt;/B&gt;</source>
        <translation>&lt;B&gt;Hibával fejeztem be&lt;/B&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1378"/>
        <source>&lt;B&gt;Module crashed or killed&lt;/B&gt;</source>
        <translation>&lt;B&gt;A modul összeomlott vagy kilőtték&lt;/B&gt;</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="13"/>
        <source>GRASS Module</source>
        <translation>GRASS modul</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="26"/>
        <source>Options</source>
        <translation>Opciók</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="31"/>
        <source>Output</source>
        <translation>Eredmény</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="47"/>
        <source>Manual</source>
        <translation>Manuális</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="74"/>
        <source>TextLabel</source>
        <translation>Szöveg cimke</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="118"/>
        <source>Run</source>
        <translation>Futtatás</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="141"/>
        <source>View output</source>
        <translation>Eredmény megjelenítése</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="161"/>
        <source>Close</source>
        <translation>Lezár</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleField</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2714"/>
        <source>Attribute field</source>
        <translation>Attribútum mező</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleFile</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2916"/>
        <source>File</source>
        <translation>Fájl</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="3029"/>
        <source>:&amp;nbsp;missing value</source>
        <translation>:&amp;nbsp;hiányzó érték</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="3036"/>
        <source>:&amp;nbsp;directory does not exist</source>
        <translation>:&amp;nbsp;a mappa nem létezik</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleGdalInput</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2665"/>
        <source>Warning</source>
        <translation>Figyelmeztetés</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2527"/>
        <source>Cannot find layeroption </source>
        <translation>Nem találom a réteg beállításokat</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2540"/>
        <source>Cannot find whereoption </source>
        <translation>Nem találom a where paramétert</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2669"/>
        <source>PostGIS driver in OGR does not support schemas!&lt;br&gt;Only the table name will be used.&lt;br&gt;It can result in wrong input if more tables of the same name&lt;br&gt;are present in the database.</source>
        <translation>Az OGR PostGIS meghajtó nem támogatja a sémát!&lt;br&gt;Csak a táblanevet használom.&lt;br&gt;Ez hibás eredményre vezethet, ha azonos nevű tábla&lt;br&gt;van az adatbázisban.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2692"/>
        <source>:&amp;nbsp;no input</source>
        <translation>:&amp;nbsp;nincs input</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleInput</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2071"/>
        <source>Warning</source>
        <translation>Figyelmeztetés</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1984"/>
        <source>Cannot find typeoption </source>
        <translation>Nem találom a típus beállításokat</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1993"/>
        <source>Cannot find values for typeoption </source>
        <translation>Nem találom a típus beállításhoz tartozó értéket</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2054"/>
        <source>Cannot find layeroption </source>
        <translation>Nem találom a réteg beállításokat</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2071"/>
        <source>GRASS element </source>
        <translation>GRASS elem</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2071"/>
        <source> not supported</source>
        <translation>nem támogatott</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2095"/>
        <source>Use region of this map</source>
        <translation>Ennek a térképnek a terjedelmét használom</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2431"/>
        <source>:&amp;nbsp;no input</source>
        <translation>:&amp;nbsp;nincs input</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleOption</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1900"/>
        <source>:&amp;nbsp;missing value</source>
        <translation>:&amp;nbsp;hiányzó érték</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleSelection</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2803"/>
        <source>Attribute field</source>
        <translation>Attribútum mező</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleStandardOptions</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="886"/>
        <source>Warning</source>
        <translation>Figyelmeztetés</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="369"/>
        <source>Cannot find module </source>
        <translation>Nem találom a modult</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="386"/>
        <source>Cannot start module </source>
        <translation>Nem tudom elindítani a modult</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="399"/>
        <source>Cannot read module description (</source>
        <translation>Nem tudom olvasni a modul leírást (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="399"/>
        <source>):
</source>
        <translation>):
</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="400"/>
        <source>
at line </source>
        <translation>sorban</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="400"/>
        <source> column </source>
        <translation>oszlop</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="424"/>
        <source>Cannot find key </source>
        <translation>Nem találom a kulcsot</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="560"/>
        <source>Item with id </source>
        <translation>Elem azonosító</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="560"/>
        <source> not found</source>
        <translation>nem találom</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="848"/>
        <source>Cannot get current region</source>
        <translation>Nem tudom megszerezni az aktuális terjedelmet</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="825"/>
        <source>Cannot check region of map </source>
        <translation>Nem tudom ellenőrizni a térkép terjedelmét</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="887"/>
        <source>Cannot set region of map </source>
        <translation>Nem tudom beállítani a térkép terjedelmét</translation>
    </message>
</context>
<context>
    <name>QgsGrassNewMapset</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="124"/>
        <source>GRASS database</source>
        <translation>GRASS adatbázis</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="125"/>
        <source>GRASS location</source>
        <translation>GRASS munkaterület</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="126"/>
        <source>Projection</source>
        <translation>Vetület</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="127"/>
        <source>Default GRASS Region</source>
        <translation>Alapértelmezett GRASS terjedelem</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="184"/>
        <source>Mapset</source>
        <translation>Térkép halmaz</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="129"/>
        <source>Create New Mapset</source>
        <translation>Új térkép halmaz létrehozása</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="158"/>
        <source>Tree</source>
        <translation>Fa</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="159"/>
        <source>Comment</source>
        <translation>Megjegyzés</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="160"/>
        <source>Database</source>
        <translation>Adatbázis</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="164"/>
        <source>Location 2</source>
        <translation>Munkaterület 2</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="175"/>
        <source>User&apos;s mapset</source>
        <translation>Felhasználói térkép készlet</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="177"/>
        <source>System mapset</source>
        <translation>Rendszer térkép halmaz</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="171"/>
        <source>Location 1</source>
        <translation>Munkaterület 1</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="185"/>
        <source>Owner</source>
        <translation>Tulajdonos</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="233"/>
        <source>Enter path to GRASS database</source>
        <translation>Add meg az elérési utat a GASS adatbázishoz</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="241"/>
        <source>The directory doesn&apos;t exist!</source>
        <translation>A mappa nem létezik!</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="271"/>
        <source>No writable locations, the database not writable!</source>
        <translation>Nem írható munkaterület, az adatbázis nem írható!</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="369"/>
        <source>Enter location name!</source>
        <translation>Add meg a munkaterület nevét!</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="382"/>
        <source>The location exists!</source>
        <translation>A munkaterület létezik!</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="528"/>
        <source>Selected projection is not supported by GRASS!</source>
        <translation>A kiválasztott vetületet a GRASS nem támogatja!</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1155"/>
        <source>Warning</source>
        <translation>Figyelmeztetés</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="575"/>
        <source>Cannot create projection.</source>
        <translation>Nem tudok vetületet létrehozni.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="624"/>
        <source>Cannot reproject previously set region, default region set.</source>
        <translation>Nem tudom átvetíteni a korábban beállított terjedelmet, az alapértelmezett terjedelmet használom.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="759"/>
        <source>North must be greater than south</source>
        <translation>Észak értéknek nagyobbnak kell lennie mint a délnek</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="764"/>
        <source>East must be greater than west</source>
        <translation>Kelet értéknek nagyobbnak kell lennie mint a nyugatnak</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="811"/>
        <source>Regions file (</source>
        <translation>Terjedelmek fájl (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="811"/>
        <source>) not found.</source>
        <translation>) nem találom.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="816"/>
        <source>Cannot open locations file (</source>
        <translation>Nem tudom megnyitni a munkaterület fájlt (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="816"/>
        <source>)</source>
        <translation>)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="825"/>
        <source>Cannot read locations file (</source>
        <translation>Nem tudom olvasni a munkaterület fájlt (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="826"/>
        <source>):
</source>
        <translation>):
</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="826"/>
        <source>
at line </source>
        <translation>
sorban</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="827"/>
        <source> column </source>
        <translation>oszlop</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1156"/>
        <source>Cannot create QgsSpatialRefSys</source>
        <translation>Nem tudok létrehozni QgsSpatialRefSys-t</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="963"/>
        <source>Cannot reproject selected region.</source>
        <translation>Nem tudom átvetíteni a kiválasztott terjedelmet.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1052"/>
        <source>Cannot reproject region</source>
        <translation>Nem tudom átvetíteni a terjedelmet</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1284"/>
        <source>Enter mapset name.</source>
        <translation>Add meg a térkép halmaz nevét.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1301"/>
        <source>The mapset already exists</source>
        <translation>A térkép halmaz már létezik</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1325"/>
        <source>Database: </source>
        <translation>Adatbázis:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1336"/>
        <source>Location: </source>
        <translation>Munkaterület:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1338"/>
        <source>Mapset: </source>
        <translation>Térkép halmaz:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1373"/>
        <source>Create location</source>
        <translation>Munkaterület létrehozása</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1375"/>
        <source>Cannot create new location: </source>
        <translation>Nem tudok új munkaterületet létrehozni:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1422"/>
        <source>Create mapset</source>
        <translation>Térkép halmaz létrehozása</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1405"/>
        <source>Cannot create new mapset directory</source>
        <translation>Nem tudok új térkép készlet mappát létrehozni</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1415"/>
        <source>Cannot open DEFAULT_WIND</source>
        <translation>Nem tudom megnyitni a DEFAULT_WIND-et</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1422"/>
        <source>Cannot open WIND</source>
        <translation>Nem tudom megnyitni a WIND-et</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1449"/>
        <source>New mapset</source>
        <translation>Új térkép halmaz</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1445"/>
        <source>New mapset successfully created, but cannot be opened: </source>
        <translation>Az új térkép halmazt sikeresen létrehoztam, de nem tudom megnyitni:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1451"/>
        <source>New mapset successfully created and set as current working mapset.</source>
        <translation>Az új térkép halmazt sikeresen létrehoztam és beállítottam mint aktuális munka térkép halmazt.</translation>
    </message>
</context>
<context>
    <name>QgsGrassNewMapsetBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2068"/>
        <source>Column 1</source>
        <translation>1. oszlop</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="88"/>
        <source>Example directory tree:</source>
        <translation>Minta mappa fa:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="95"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;GRASS data are stored in tree directory structure.&lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;The GRASS database is the top-level directory in this tree structure.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;GRASS data are stored in tree directory structure.&lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;A GRASS adatbázis a legmagasabbszintû mappa ebben könyvtárfa struktúrában.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="399"/>
        <source>Database Error</source>
        <translation>Adatbázis hiba</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2153"/>
        <source>Database:</source>
        <translation>Adatbázis:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="440"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="457"/>
        <source>Select existing directory or create a new one:</source>
        <translation>Válassz egy létező mappát vagy hozz létre egyet:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="508"/>
        <source>Location</source>
        <translation>Munkaterület</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="535"/>
        <source>Select location</source>
        <translation>Válassz munkaterületet</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="552"/>
        <source>Create new location</source>
        <translation>Új munkaterület létrehozása</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="832"/>
        <source>Location Error</source>
        <translation>Munkaterület hiba</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="848"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;The GRASS location is a collection of maps for a particular territory or project.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;A GRASS munkaterület egy területhez vagy projekthez tartozó térképek gyûjteménye.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1159"/>
        <source>Projection Error</source>
        <translation>Vetületi hiba</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1174"/>
        <source>Coordinate system</source>
        <translation>Koordinátarendszer</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1186"/>
        <source>Projection</source>
        <translation>Vetület</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1193"/>
        <source>Not defined</source>
        <translation>Nem definiált</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1273"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;The GRASS region defines a workspace for raster modules. The default region is valid for one location. It is possible to set a different region in each mapset. &lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;It is possible to change the default location region later.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;A GRASS terjedelem egy területet definiál a raszter modulokhoz. Az alapértelmezett terjedelem egy munkaterületre érvényes. Az egyes térképhalmazokra eltérõ terjedelmet állíthatsz be. &lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Az alapértelmezett terjedelmet késõbb megváltoztathatod.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1334"/>
        <source>Set current QGIS extent</source>
        <translation>Aktuális QGIS terjedelem beállítása</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1376"/>
        <source>Set</source>
        <translation>Halmaz</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1396"/>
        <source>Region Error</source>
        <translation>Terjedelem hiba</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1441"/>
        <source>S</source>
        <translation>D</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1500"/>
        <source>W</source>
        <translation>Ny</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1555"/>
        <source>E</source>
        <translation>K</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1614"/>
        <source>N</source>
        <translation>É</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1699"/>
        <source>New mapset:</source>
        <translation>Új térkép halmaz:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1988"/>
        <source>Mapset Error</source>
        <translation>Térkép halmaz hiba</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2045"/>
        <source>&lt;p align=&quot;center&quot;&gt;Existing masets&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;center&quot;&gt;Létező térkép halmazok&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2101"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;The GRASS mapset is a collection of maps used by one user. &lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;A user can read maps from all mapsets in the location but &lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;he can open for writing only his mapset (owned by user).&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;A GRASS térképhalmaz egy felhasználó álhal használt térképek gyûjteménye. &lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Egy felhasználó a munkaterület összes térképét használhatja, de&lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;csak a saját térképhalmazát módosíthatja.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2174"/>
        <source>Location:</source>
        <translation>Munkaterület:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2195"/>
        <source>Mapset:</source>
        <translation>Térkép halmaz:</translation>
    </message>
</context>
<context>
    <name>QgsGrassPlugin</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="82"/>
        <source>GrassVector</source>
        <translation>GrassVector</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="83"/>
        <source>0.1</source>
        <translation>0.1</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="84"/>
        <source>GRASS layer</source>
        <translation>GRASS réteg</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="145"/>
        <source>Open mapset</source>
        <translation>Térkép halmaz megnyitása</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="146"/>
        <source>New mapset</source>
        <translation>Új térkép halmaz</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="147"/>
        <source>Close mapset</source>
        <translation>Térkép halmaz lezárása</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="150"/>
        <source>Add GRASS vector layer</source>
        <translation>GRASS vektor réteg hozzáadása</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="152"/>
        <source>Add GRASS raster layer</source>
        <translation>GRASS raszter réteg hozzáadása</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="168"/>
        <source>Open GRASS tools</source>
        <translation>GRASS eszközök megnyitása</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="157"/>
        <source>Display Current Grass Region</source>
        <translation>Aktuális GRASS terjedelem megjelenítése</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="161"/>
        <source>Edit Current Grass Region</source>
        <translation>Aktuális GRASS terjedelem módosítása</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="163"/>
        <source>Edit Grass Vector layer</source>
        <translation>GRASS vektor réteg szerkesztése</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="164"/>
        <source>Create new Grass Vector</source>
        <translation>Új GRASS vektor létrehozása</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="166"/>
        <source>Adds a GRASS vector layer to the map canvas</source>
        <translation>GRASS vektor réteg hozzáadása a térképhez</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="167"/>
        <source>Adds a GRASS raster layer to the map canvas</source>
        <translation>GRASS raszter réteg hozzáadása a térképhez</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="169"/>
        <source>Displays the current GRASS region as a rectangle on the map canvas</source>
        <translation>Aktuális GRASS terjedelem megjelenítése a térképen mint egy négyzet</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="170"/>
        <source>Edit the current GRASS region</source>
        <translation>Aktuális GRASS terjedelem módosítása</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="171"/>
        <source>Edit the currently selected GRASS vector layer.</source>
        <translation>Aktuális GRASS vektor réteg szerkesztése.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="788"/>
        <source>&amp;GRASS</source>
        <translation>&amp;GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="198"/>
        <source>GRASS</source>
        <translation>GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="758"/>
        <source>Warning</source>
        <translation>Figyelmeztetés</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="470"/>
        <source>GRASS Edit is already running.</source>
        <translation>A GRASS szerkesztés már folyamatban van.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="479"/>
        <source>New vector name</source>
        <translation>Új vektor név</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="495"/>
        <source>Cannot create new vector: </source>
        <translation>Nem tudom az új vektort létrehozni:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="515"/>
        <source>New vector created but cannot be opened by data provider.</source>
        <translation>Az új vektort létrehoztam, de nem lehet megnyitni.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="526"/>
        <source>Cannot start editing.</source>
        <translation>Nem tudom elkezdeni a szerkesztést.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="561"/>
        <source>GISDBASE, LOCATION_NAME or MAPSET is not set, cannot display current region.</source>
        <translation>GISDBASE, LOCATION_NAME vagy MAPSET nincs beállítva, nem tudom megjelenítteni az aktuális terjedelmet.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="571"/>
        <source>Cannot read current region: </source>
        <translation>Nem tudom olvasni az aktuális terjedelmet:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="675"/>
        <source>Cannot open the mapset. </source>
        <translation>Nem tudom megnyitnni a térkép halmazt.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="693"/>
        <source>Cannot close mapset. </source>
        <translation>Nem tudom lezárni a térkép halmazt.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="749"/>
        <source>Cannot close current mapset. </source>
        <translation>Nem tudom lezárni az aktuális térkép halmazt.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="758"/>
        <source>Cannot open GRASS mapset. </source>
        <translation>Nem tudom megnyitni a GRASS térkép halmazt.</translation>
    </message>
</context>
<context>
    <name>QgsGrassRegion</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregion.cpp" line="458"/>
        <source>Warning</source>
        <translation>Figyelmeztetés</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregion.cpp" line="194"/>
        <source>GISDBASE, LOCATION_NAME or MAPSET is not set, cannot display current region.</source>
        <translation>GISDBASE, LOCATION_NAME vagy MAPSET nincs beállítva, nem tudom megjelenítteni az aktuális terjedelmet.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregion.cpp" line="201"/>
        <source>Cannot read current region: </source>
        <translation>Nem tudom olvasni az aktuális terjedelmet:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregion.cpp" line="458"/>
        <source>Cannot write region</source>
        <translation>Nem tudom a terjedelmet kiírni</translation>
    </message>
</context>
<context>
    <name>QgsGrassRegionBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="13"/>
        <source>GRASS Region Settings</source>
        <translation>GRASS terjedelem beállítások</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="76"/>
        <source>N</source>
        <translation>É</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="146"/>
        <source>W</source>
        <translation>Ny</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="172"/>
        <source>E</source>
        <translation>K</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="236"/>
        <source>S</source>
        <translation>D</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="280"/>
        <source>N-S Res</source>
        <translation>É-D felbontás</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="293"/>
        <source>Rows</source>
        <translation>Sorok</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="303"/>
        <source>Cols</source>
        <translation>Oszl</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="316"/>
        <source>E-W Res</source>
        <translation>K-Ny felbontás</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="364"/>
        <source>Color</source>
        <translation>Szín</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="384"/>
        <source>Width</source>
        <translation>Szélesség</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="464"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="487"/>
        <source>Cancel</source>
        <translation>Mégsem</translation>
    </message>
</context>
<context>
    <name>QgsGrassSelect</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="68"/>
        <source>Select GRASS Vector Layer</source>
        <translation>Válassz GRASS vektor réteget</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="75"/>
        <source>Select GRASS Raster Layer</source>
        <translation>Válassz GRASS raszter réteget</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="82"/>
        <source>Select GRASS mapcalc schema</source>
        <translation>Válassz GRASS mapcalc sémát</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="90"/>
        <source>Select GRASS Mapset</source>
        <translation>Válassz GRASS térkép halmazt</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="408"/>
        <source>Warning</source>
        <translation>Figyelmeztetés</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="408"/>
        <source>Cannot open vector on level 2 (topology not available).</source>
        <translation>Nem tudom megnyitbni a vektor a 2. szinten (nincs topológia).</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="465"/>
        <source>Choose existing GISDBASE</source>
        <translation>Válassz egy létező GISDBASE-t</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="481"/>
        <source>Wrong GISDBASE, no locations available.</source>
        <translation>Rossz GISDBASE, nincs munkaterület.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="482"/>
        <source>Wrong GISDBASE</source>
        <translation>Rossz GISDBASE</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="499"/>
        <source>Select a map.</source>
        <translation>Válassz egy térképet.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="500"/>
        <source>No map</source>
        <translation>Nincs térkép</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="508"/>
        <source>No layer</source>
        <translation>Nincs réteg</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="509"/>
        <source>No layers available in this map</source>
        <translation>Nincsaenek rétegek ebben a térképben</translation>
    </message>
</context>
<context>
    <name>QgsGrassSelectBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="21"/>
        <source>Add GRASS Layer</source>
        <translation>Raszter réteg hozzáadása</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="65"/>
        <source>Gisdbase</source>
        <translation>Gisdbase</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="78"/>
        <source>Location</source>
        <translation>Munkaterület</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="85"/>
        <source>Mapset</source>
        <translation>Térkép halmaz</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="102"/>
        <source>Select or type map name (wildcards &apos;*&apos; and &apos;?&apos; accepted for rasters)</source>
        <translation>Válassz vagy írj be térkép nevet (joker karakterek &apos;*&apos; és &apos;?&apos;  használható raszterekhez)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="118"/>
        <source>Map name</source>
        <translation>Térkép név</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="125"/>
        <source>Layer</source>
        <translation>Réteg</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="161"/>
        <source>Browse</source>
        <translation>Tallózás</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="168"/>
        <source>Cancel</source>
        <translation>Mégsem</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="175"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
</context>
<context>
    <name>QgsGrassShellBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassshellbase.ui" line="19"/>
        <source>GRASS Shell</source>
        <translation>GRASS burok</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassshellbase.ui" line="49"/>
        <source>Close</source>
        <translation>Lezár</translation>
    </message>
</context>
<context>
    <name>QgsGrassTools</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="109"/>
        <source>GRASS Tools</source>
        <translation>GRASS eszközök</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="127"/>
        <source>Modules</source>
        <translation>Modulok</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="386"/>
        <source>GRASS Tools: </source>
        <translation>GRASS eszközök:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="159"/>
        <source>Browser</source>
        <translation>Böngésző</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="308"/>
        <source>Warning</source>
        <translation>Figyelmeztetés</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="221"/>
        <source>Cannot find MSYS (</source>
        <translation>MSYS-t nem találom (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="243"/>
        <source>GRASS Shell is not compiled.</source>
        <translation>A GRASS burokot nem fordították le.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="293"/>
        <source>The config file (</source>
        <translation>A konfigurációs fájl (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="293"/>
        <source>) not found.</source>
        <translation>) nem találom.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="297"/>
        <source>Cannot open config file (</source>
        <translation>Nem tudom megnyitni a konfigurációs fájlt (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="297"/>
        <source>)</source>
        <translation>)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="305"/>
        <source>Cannot read config file (</source>
        <translation>Nem tudom olvasni a konfigurációs fájlt (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="306"/>
        <source>
at line </source>
        <translation>sorban</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="306"/>
        <source> column </source>
        <translation>oszlop</translation>
    </message>
</context>
<context>
    <name>QgsGridMakerPlugin</name>
    <message>
        <location filename="../src/plugins/grid_maker/plugin.cpp" line="93"/>
        <source>&amp;Graticule Creator</source>
        <translation>&amp;Rács létrehozása</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugin.cpp" line="94"/>
        <source>Creates a graticule (grid) and stores the result as a shapefile</source>
        <translation>Rács létrehozása és tárolása mint shape fájl</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugin.cpp" line="136"/>
        <source>&amp;Graticules</source>
        <translation>&amp;Rácsok</translation>
    </message>
</context>
<context>
    <name>QgsGridMakerPluginGui</name>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="51"/>
        <source>QGIS - Grid Maker</source>
        <translation>QGIS - Rács készítő</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="52"/>
        <source>Please enter the file name before pressing OK!</source>
        <translation>Add meg a fájlnevet az OK megnyomása előtt!</translation>
    </message>
    <message>
        <location filename="" line="145"/>
        <source>Longitude Interval is invalid - please correct and try again.</source>
        <translation type="obsolete">Hosszúság intervallum hibás - korrigáld és próbáld újra.</translation>
    </message>
    <message>
        <location filename="" line="145"/>
        <source>Latitude Interval is invalid - please correct and try again.</source>
        <translation type="obsolete">Szélesség intervallum hibás - korrigáld és próbáld újra.</translation>
    </message>
    <message>
        <location filename="" line="145"/>
        <source>Longitude Origin is invalid - please correct and try again..</source>
        <translation type="obsolete">Hosszúság origó hibás - korrigáld és próbáld újra.</translation>
    </message>
    <message>
        <location filename="" line="145"/>
        <source>Latitude Origin is invalid - please correct and try again.</source>
        <translation type="obsolete">Szélesség origó hibás - korrigáld és próbáld újra.</translation>
    </message>
    <message>
        <location filename="" line="145"/>
        <source>End Point Longitude is invalid - please correct and try again.</source>
        <translation type="obsolete">A végpont hosszúsága hibás - korrigáld és próbáld újra.</translation>
    </message>
    <message>
        <location filename="" line="145"/>
        <source>End Point Latitude is invalid - please correct and try again.</source>
        <translation type="obsolete">A végpont szélessége hibás - korrigáld és próbáld újra.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="108"/>
        <source>Choose a filename to save under</source>
        <translation>Válassz egy fájlnevet a mentéshez</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="110"/>
        <source>ESRI Shapefile (*.shp)</source>
        <translation>ESRI shape fájl (*.shp)</translation>
    </message>
</context>
<context>
    <name>QgsGridMakerPluginGuiBase</name>
    <message>
        <location filename="" line="145"/>
        <source>QGIS Plugin Template</source>
        <translation type="obsolete">QGIS modul sablon</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="256"/>
        <source>Graticule Builder</source>
        <translation>Rács készítő</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="186"/>
        <source>Type</source>
        <translation>Típus</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="198"/>
        <source>Point</source>
        <translation>Pont</translation>
    </message>
    <message>
        <location filename="" line="145"/>
        <source>Line</source>
        <translation type="obsolete">Vonal</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="208"/>
        <source>Polygon</source>
        <translation>Felület</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="110"/>
        <source>Origin (lower left)</source>
        <translation>Origó (bal alsó)</translation>
    </message>
    <message>
        <location filename="" line="145"/>
        <source>Latitude:</source>
        <translation type="obsolete">Szélesség:</translation>
    </message>
    <message>
        <location filename="" line="145"/>
        <source>#000.00000; </source>
        <translation type="obsolete">#000.00000;</translation>
    </message>
    <message>
        <location filename="" line="145"/>
        <source>Longitude:</source>
        <translation type="obsolete">Hosszúság:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="69"/>
        <source>End point (upper right)</source>
        <translation>Végpont (jobb felső)</translation>
    </message>
    <message>
        <location filename="" line="145"/>
        <source>Graticle size (units in degrees)</source>
        <translation type="obsolete">Rács méret (fokokban)</translation>
    </message>
    <message>
        <location filename="" line="145"/>
        <source>Latitude Interval:</source>
        <translation type="obsolete">Szélesség intervallum:</translation>
    </message>
    <message>
        <location filename="" line="145"/>
        <source>Longitude Interval:</source>
        <translation type="obsolete">Hosszúság intervallum:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="161"/>
        <source>Output (shape) file</source>
        <translation>Eredmény (shape) fájl</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="176"/>
        <source>Save As...</source>
        <translation>Mentés másként...</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="13"/>
        <source>QGIS Graticule Creator</source>
        <translation>QGIS rács létrehozás</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="28"/>
        <source>Graticle size</source>
        <translation>Rács méret</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="46"/>
        <source>Y Interval:</source>
        <translation>Y intervallum:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="56"/>
        <source>X Interval:</source>
        <translation>X intervallum:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="128"/>
        <source>Y</source>
        <translation>Y</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="138"/>
        <source>X</source>
        <translation>X</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="221"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:11pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;This plugin will help you to build a graticule shapefile that you can use as an overlay within your qgis map viewer.&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:10pt;&quot;&gt;Please enter all units in decimal degrees&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:11pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;Ez a modul segítségével egy rács shape fájlt hozhatsz létre, melyet a QGIS térkép ablakban megjeleníthetsz más rétegekkel együtt.&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:10pt;&quot;&gt;Az összes értéket fok és tizedeiben add meg&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
</context>
<context>
    <name>QgsHelpViewer</name>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="139"/>
        <source>This help file does not exist for your language</source>
        <translation>Ez a súgó fájl nem áll rendelkezésre a te nyelveden</translation>
    </message>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="142"/>
        <source>If you would like to create it, contact the QGIS development team</source>
        <translation>Ha létre akarod hozni fordulj a QGIS fejlesztő csapathoz</translation>
    </message>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="157"/>
        <source>Quantum GIS Help</source>
        <translation>Quantum GIS súgó</translation>
    </message>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="185"/>
        <source>Quantum GIS Help - </source>
        <translation>Quantum GIS súgó -</translation>
    </message>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="214"/>
        <source>Error</source>
        <translation>Hiba</translation>
    </message>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="191"/>
        <source>Failed to get the help text from the database</source>
        <translation>Nem sikerült a súgó szöveget beszerezni az adatbázisból</translation>
    </message>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="215"/>
        <source>The QGIS help database is not installed</source>
        <translation>Quantum GIS súgó adatbázist nem installálták</translation>
    </message>
</context>
<context>
    <name>QgsHelpViewerBase</name>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="16"/>
        <source>QGIS Help</source>
        <translation>QGIS súgó</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="81"/>
        <source>&amp;Close</source>
        <translation>&amp;Lezár</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="84"/>
        <source>Alt+C</source>
        <translation>Alt+C</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="42"/>
        <source>&amp;Home</source>
        <translation>&amp;Home</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="45"/>
        <source>Alt+H</source>
        <translation>Alt+H</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="55"/>
        <source>&amp;Forward</source>
        <translation>&amp;Tovább</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="58"/>
        <source>Alt+F</source>
        <translation>Alt+F</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="68"/>
        <source>&amp;Back</source>
        <translation>&amp;Vissza</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="71"/>
        <source>Alt+B</source>
        <translation>Alt+B</translation>
    </message>
</context>
<context>
    <name>QgsHttpTransaction</name>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="234"/>
        <source>WMS Server responded unexpectedly with HTTP Status Code %1 (%2)</source>
        <translation>A WMS szerver nem várt válasza ezzel a HHTP státusz kóddal %1 (%2)</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="313"/>
        <source>HTTP response completed, however there was an error: %1</source>
        <translation>A HTTP válasz megérkezett, bár volt egy hiba: %1</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="362"/>
        <source>HTTP transaction completed, however there was an error: %1</source>
        <translation>A HTTP tranzakció befejeződött, bár volt egy hiba: %1</translation>
    </message>
    <message numerus="yes">
        <location filename="../src/core/qgshttptransaction.cpp" line="441"/>
        <source>Network timed out after %1 seconds of inactivity.
This may be a problem in your network connection or at the WMS server.</source>
        <translation type="unfinished">
            <numerusform>Hálózati időtúllépés %1 másodperc után.
A hiba a hálózati kapcsolatodban vagy a WMS szervernél lehet.</numerusform>
        </translation>
    </message>
</context>
<context>
    <name>QgsIdentifyResults</name>
    <message>
        <location filename="../src/app/qgsidentifyresults.cpp" line="44"/>
        <source>Feature</source>
        <translation>Elem</translation>
    </message>
    <message>
        <location filename="../src/app/qgsidentifyresults.cpp" line="45"/>
        <source>Value</source>
        <translation>Érték</translation>
    </message>
    <message>
        <location filename="../src/app/qgsidentifyresults.cpp" line="106"/>
        <source>Run action</source>
        <translation>Művelet futtatása</translation>
    </message>
    <message>
        <location filename="../src/app/qgsidentifyresults.cpp" line="196"/>
        <source>(Derived)</source>
        <translation>(Levezetett)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsidentifyresults.cpp" line="225"/>
        <source>Identify Results - </source>
        <translation>Azonosítás eredmények -</translation>
    </message>
</context>
<context>
    <name>QgsIdentifyResultsBase</name>
    <message>
        <location filename="../src/ui/qgsidentifyresultsbase.ui" line="16"/>
        <source>Identify Results</source>
        <translation>Azonosítás eredmények</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsidentifyresultsbase.ui" line="46"/>
        <source>Help</source>
        <translation>Súgó</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsidentifyresultsbase.ui" line="49"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsidentifyresultsbase.ui" line="75"/>
        <source>Close</source>
        <translation>Lezár</translation>
    </message>
</context>
<context>
    <name>QgsLUDialogBase</name>
    <message>
        <location filename="../src/ui/qgsludialogbase.ui" line="13"/>
        <source>Enter class bounds</source>
        <translation>Add meg az osztályok határait</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsludialogbase.ui" line="31"/>
        <source>Lower value</source>
        <translation>Alsó érték</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsludialogbase.ui" line="57"/>
        <source>-</source>
        <translation>-</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsludialogbase.ui" line="94"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsludialogbase.ui" line="101"/>
        <source>Cancel</source>
        <translation>Mégsem</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsludialogbase.ui" line="126"/>
        <source>Upper value</source>
        <translation>Felső érték</translation>
    </message>
</context>
<context>
    <name>QgsLabelDialogBase</name>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="21"/>
        <source>Form1</source>
        <translation>Form1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="41"/>
        <source>Preview:</source>
        <translation>Előnézet:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="56"/>
        <source>QGIS Rocks!</source>
        <translation>QGIS Rocks!</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="78"/>
        <source>Font Style</source>
        <translation>Betű stílus</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="103"/>
        <source>Font size units</source>
        <translation>Betűméret egységek</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="579"/>
        <source>Map units</source>
        <translation>Térkép egységek</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="586"/>
        <source>Points</source>
        <translation>Pontok</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="980"/>
        <source>Transparency:</source>
        <translation>Átlátszóság:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="183"/>
        <source>Font</source>
        <translation>Betűkészlet</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="534"/>
        <source>Colour</source>
        <translation>Szín</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="478"/>
        <source>%</source>
        <translation>%</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="236"/>
        <source>Font Alignment</source>
        <translation>Igazítás</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="261"/>
        <source>Placement</source>
        <translation>Elhelyezés</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="273"/>
        <source>Below Right</source>
        <translation>Jobb alul</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="280"/>
        <source>Right</source>
        <translation>Jobb</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="287"/>
        <source>Below</source>
        <translation>Alul</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="294"/>
        <source>Over</source>
        <translation>Rajta</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="304"/>
        <source>Above</source>
        <translation>Felül</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="311"/>
        <source>Left</source>
        <translation>Bal</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="318"/>
        <source>Below Left</source>
        <translation>Bal alul</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="325"/>
        <source>Above Right</source>
        <translation>Jobb felül</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="332"/>
        <source>Above Left</source>
        <translation>Bal felül</translation>
    </message>
    <message encoding="UTF-8">
        <location filename="../src/ui/qgslabeldialogbase.ui" line="342"/>
        <source>°</source>
        <translation>°</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="893"/>
        <source>Angle (deg):</source>
        <translation>Szög (fok):</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="384"/>
        <source>Buffer</source>
        <translation>Zóna</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="396"/>
        <source>Buffer size units</source>
        <translation>Zóna méret egységek</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="408"/>
        <source>Size is in map units</source>
        <translation>Méret térképi egységekben</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="415"/>
        <source>Size is in points</source>
        <translation>Méret pontokban</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="1005"/>
        <source>Size:</source>
        <translation>Méret:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="451"/>
        <source>Buffer Labels?</source>
        <translation>Cimke zóna?</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="542"/>
        <source>Position</source>
        <translation>Pozició</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="567"/>
        <source>Offset units</source>
        <translation>Eltolás egységek</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="1080"/>
        <source>X Offset (pts):</source>
        <translation>X offset (pontok):</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="1116"/>
        <source>Y Offset (pts):</source>
        <translation>Y eltolás (pontok):</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="625"/>
        <source>Data Defined Style</source>
        <translation>Adat szerinti stílus</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="645"/>
        <source>&amp;Italic:</source>
        <translation>&amp;Dölt:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="663"/>
        <source>&amp;Size:</source>
        <translation>&amp;Méret:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="681"/>
        <source>&amp;Bold:</source>
        <translation>&amp;Félkövér:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="719"/>
        <source>&amp;Underline:</source>
        <translation>&amp;Aláhúzás:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="767"/>
        <source>Size Units:</source>
        <translation>Méret egységek:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="813"/>
        <source>&amp;Font family:</source>
        <translation>&amp;Betűkészlet:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="847"/>
        <source>Data Defined Alignment</source>
        <translation>Adat szerinti igazítás</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="914"/>
        <source>Placement:</source>
        <translation>Elhelyezés:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="922"/>
        <source>Data Defined Buffer</source>
        <translation>Adat szerinti zóna</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="1016"/>
        <source>Data Defined Position</source>
        <translation>Adat szerinti pozició</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="1062"/>
        <source>X Coordinate:</source>
        <translation>X koordináta:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="1101"/>
        <source>Y Coordinate:</source>
        <translation>Y koordináta:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="1136"/>
        <source>Source</source>
        <translation>Forrás</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="1148"/>
        <source>Field containing label:</source>
        <translation>Cimkét tartalmazó mező:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="1191"/>
        <source>Default label:</source>
        <translation>Alapértelmezett cimke:</translation>
    </message>
</context>
<context>
    <name>QgsLayerProjectionSelector</name>
    <message>
        <location filename="../src/gui/qgslayerprojectionselector.cpp" line="34"/>
        <source>Define this layer&apos;s projection:</source>
        <translation>Add meg a réteg vetületét:</translation>
    </message>
    <message>
        <location filename="../src/gui/qgslayerprojectionselector.cpp" line="35"/>
        <source>This layer appears to have no projection specification.</source>
        <translation>Nincs vetületi beállítás ehhez a réteghez.</translation>
    </message>
    <message>
        <location filename="../src/gui/qgslayerprojectionselector.cpp" line="37"/>
        <source>By default, this layer will now have its projection set to that of the project, but you may override this by selecting a different projection below.</source>
        <translation>Ez a réteg alapértelmezés szerint most a projekt vetületét kapja, de ezt felülbírálhatod egy másik vetület kiválasztásával lent.</translation>
    </message>
</context>
<context>
    <name>QgsLayerProjectionSelectorBase</name>
    <message>
        <location filename="../src/ui/qgslayerprojectionselectorbase.ui" line="13"/>
        <source>Layer Projection Selector</source>
        <translation>Réteg vetület kiválasztás</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslayerprojectionselectorbase.ui" line="80"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
</context>
<context>
    <name>QgsLegend</name>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="110"/>
        <source>group</source>
        <translation>csoport</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="416"/>
        <source>&amp;Make to toplevel item</source>
        <translation>&amp;Legfelső elemmé tesz</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="423"/>
        <source>&amp;Remove</source>
        <translation>&amp;Töröl</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="428"/>
        <source>Re&amp;name</source>
        <translation>&amp;Átnevez</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="433"/>
        <source>&amp;Add group</source>
        <translation>&amp;Csoport hozzáadás</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="434"/>
        <source>&amp;Expand all</source>
        <translation>&amp;Mindent kinyit</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="435"/>
        <source>&amp;Collapse all</source>
        <translation>&amp;Mindent összezár</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="437"/>
        <source>Show file groups</source>
        <translation>Fájl csoport megjelenítés</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="1818"/>
        <source>No Layer Selected</source>
        <translation>Nincs szelektált réteg</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="1819"/>
        <source>To open an attribute table, you must select a vector layer in the legend</source>
        <translation>Az attribútum tábla megnyitásához egy vektor réteget kell szelektálnod a jelmagyarázatban</translation>
    </message>
</context>
<context>
    <name>QgsLegendLayer</name>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="474"/>
        <source>&amp;Zoom to layer extent</source>
        <translation>&amp;Nagyítás a réteg terjedelemre</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="477"/>
        <source>&amp;Zoom to best scale (100%)</source>
        <translation>&amp;Nagyítás a legjobb méretarányra (100%)</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="481"/>
        <source>&amp;Show in overview</source>
        <translation>&amp;Megjelenítés az áttekintő térképen</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="487"/>
        <source>&amp;Remove</source>
        <translation>&amp;Töröl</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="494"/>
        <source>&amp;Open attribute table</source>
        <translation>&amp;Attribútum tábla megnyitása</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="518"/>
        <source>Save as shapefile...</source>
        <translation>Mentés mint shape fájl...</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="525"/>
        <source>Save selection as shapefile...</source>
        <translation>Szelekció mentése mint shape fájl...</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="535"/>
        <source>&amp;Properties</source>
        <translation>&amp;Tulajdonságok</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="584"/>
        <source>More layers</source>
        <translation>További rétegek</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="585"/>
        <source>This item contains more layer files. Displaying more layers in table is not supported.</source>
        <translation>Ez az elem több réteg fájlt tartalmaz. Több réteg táblában megjelenítése nem lehetséges.</translation>
    </message>
</context>
<context>
    <name>QgsLegendLayerFile</name>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="227"/>
        <source>Not a vector layer</source>
        <translation>Nem vektor réteg</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="228"/>
        <source>To open an attribute table, you must select a vector layer in the legend</source>
        <translation>Az attribútum tábla megnyitásához egy réteget kell szelektálnod a jelmagyarázatban</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="271"/>
        <source>bad_alloc exception</source>
        <translation>bad_alloc exception</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="271"/>
        <source>Filling the attribute table has been stopped because there was no more virtual memory left</source>
        <translation>Az attribútum tábla kitöltését magszakítottam, mert elfogyott a virtuális memória</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="277"/>
        <source>Attribute table - </source>
        <translation>Attribútum tábla -</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="347"/>
        <source>Save layer as...</source>
        <translation>Layer mentés másként...</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="394"/>
        <source>Saving done</source>
        <translation>Mentés kész</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="394"/>
        <source>Export to Shapefile has been completed</source>
        <translation>Kész az shape fájl export</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="398"/>
        <source>Driver not found</source>
        <translation>A meghajtót nem találom</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="398"/>
        <source>ESRI Shapefile driver is not available</source>
        <translation>Nincs ESRI shape fájl meghajtó</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="402"/>
        <source>Error creating shapefile</source>
        <translation>Hiba a shape fájl létrehozásában</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="403"/>
        <source>The shapefile could not be created (</source>
        <translation>A shape fájlt nem tudom létrehozni (</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="458"/>
        <source>Error</source>
        <translation>Hiba</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="407"/>
        <source>Layer creation failed</source>
        <translation>A réteg létrehozás nem sikerült</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="411"/>
        <source>Layer attribute table contains unsupported datatype(s)</source>
        <translation>A réteg attribútum tábla nem támogatott adattípus(oka)t tartalmaz</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="427"/>
        <source>Start editing failed</source>
        <translation>Sikertelen szerkesztés kezdés</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="428"/>
        <source>Provider cannot be opened for editing</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="441"/>
        <source>Stop editing</source>
        <translation>Szerkesztés vége</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="441"/>
        <source>Do you want to save the changes?</source>
        <translation>Akarod menteni a változásokat?</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="447"/>
        <source>Could not commit changes</source>
        <translation>Nem tudom menteni a változásokat</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="459"/>
        <source>Problems during roll back</source>
        <translation>Probléma a visszavonás közben</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="502"/>
        <source>&amp;Zoom to layer extent</source>
        <translation>&amp;Nagyítás a réteg terjedelemre</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="505"/>
        <source>&amp;Show in overview</source>
        <translation>&amp;Megjelenítés az áttekintő térképen</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="513"/>
        <source>&amp;Remove</source>
        <translation>&amp;Töröl</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="522"/>
        <source>&amp;Open attribute table</source>
        <translation>&amp;Attribútum tábla megnyitása</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="536"/>
        <source>Save as shapefile...</source>
        <translation>Mentés mint shape fájl...</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="538"/>
        <source>Save selection as shapefile...</source>
        <translation>Szelekció mentése mint shape fájl...</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="555"/>
        <source>&amp;Properties</source>
        <translation>&amp;Tulajdonságok</translation>
    </message>
</context>
<context>
    <name>QgsLineStyleDialogBase</name>
    <message>
        <location filename="../src/ui/qgslinestyledialogbase.ui" line="13"/>
        <source>Select a line style</source>
        <translation>Válassz vonalstílust</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslinestyledialogbase.ui" line="28"/>
        <source>Styles</source>
        <translation>Stílusok</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslinestyledialogbase.ui" line="177"/>
        <source>Ok</source>
        <translation>Ok</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslinestyledialogbase.ui" line="184"/>
        <source>Cancel</source>
        <translation>Mégsem</translation>
    </message>
</context>
<context>
    <name>QgsLineStyleWidgetBase</name>
    <message>
        <location filename="../src/ui/qgslinestylewidgetbase.ui" line="16"/>
        <source>Form2</source>
        <translation>Form2</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslinestylewidgetbase.ui" line="36"/>
        <source>Outline Style</source>
        <translation>Körvonal stílus</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslinestylewidgetbase.ui" line="61"/>
        <source>Width:</source>
        <translation>Szélesség:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslinestylewidgetbase.ui" line="87"/>
        <source>Colour:</source>
        <translation>Szín:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslinestylewidgetbase.ui" line="98"/>
        <source>LineStyleWidget</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslinestylewidgetbase.ui" line="120"/>
        <source>col</source>
        <translation>oszl</translation>
    </message>
</context>
<context>
    <name>QgsMapCanvas</name>
    <message>
        <location filename="../src/gui/qgsmapcanvas.cpp" line="1172"/>
        <source>Could not draw</source>
        <translation>Nem lehet kirajzolni</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsmapcanvas.cpp" line="1172"/>
        <source>because</source>
        <translation>mert</translation>
    </message>
</context>
<context>
    <name>QgsMapLayer</name>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="537"/>
        <source> Check file permissions and retry.</source>
        <translation>Ellenőrizd a fájl jogokat és próbáld újra.</translation>
    </message>
</context>
<context>
    <name>QgsMapToolIdentify</name>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="423"/>
        <source>(clicked coordinate)</source>
        <translation>(megjelőlt koordináta)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="222"/>
        <source>WMS identify result for %1
%2</source>
        <translation>WMS azonosítás eredmény %1
%2</translation>
    </message>
    <message numerus="yes">
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="490"/>
        <source>- %1 features found</source>
        <comment>Identify results window title</comment>
        <translation type="unfinished">
            <numerusform>- %1 elemet találtam</numerusform>
        </translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="564"/>
        <source>No features found</source>
        <translation>Nem találtam elemeket</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="567"/>
        <source>&lt;p&gt;No features were found within the search radius. Note that it is currently not possible to use the identify tool on unsaved features.&lt;/p&gt;</source>
        <translation>&lt;p&gt;A keresési sugáron belül nem találtam elemeket. Vigyázz a nem mentett elemekre nem használhatod az azonosítás eszközt.&lt;/p&gt;</translation>
    </message>
</context>
<context>
    <name>QgsMapToolSplitFeatures</name>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="85"/>
        <source>Split error</source>
        <translation>Felosztás hiba</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="85"/>
        <source>An error occured during feature splitting</source>
        <translation>Hiba az elem felosztása során </translation>
    </message>
</context>
<context>
    <name>QgsMapToolVertexEdit</name>
    <message>
        <location filename="../src/app/qgsmaptoolvertexedit.cpp" line="51"/>
        <source>Snap tolerance</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolvertexedit.cpp" line="52"/>
        <source>Don&apos;t show this message again</source>
        <translation>Ne mutasd többé ezt az üzenetet</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolvertexedit.cpp" line="57"/>
        <source>Could not snap segment.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolvertexedit.cpp" line="60"/>
        <source>Have you set the tolerance in Settings &gt; Project Properties &gt; General?</source>
        <translation>Beállítottad a toleranciát a Beállítások &gt; Projekt tulajdonságok &gt; Általános pontban?</translation>
    </message>
</context>
<context>
    <name>QgsMapserverExport</name>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexport.cpp" line="197"/>
        <source>Overwrite File?</source>
        <translation>Felülírjam a fájlt?</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmapserverexport.cpp" line="74"/>
        <source> exists. 
Do you want to overwrite it?</source>
        <translation>létezik.
Felül akarod írni?</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexport.cpp" line="76"/>
        <source>Name for the map file</source>
        <translation>Térkép fájl neve</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexport.cpp" line="77"/>
        <source>MapServer map files (*.map);;All files (*.*)</source>
        <comment>Filter list for selecting files from a dialog box</comment>
        <translation>Mapserver térkép fájlok (*.map);;Minden fájl (*.*)</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexport.cpp" line="84"/>
        <source>Choose the QGIS project file</source>
        <translation>Válaszd ki a QGIS projekt fájlt</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexport.cpp" line="85"/>
        <source>QGIS Project Files (*.qgs);;All files (*.*)</source>
        <comment>Filter list for selecting files from a dialog box</comment>
        <translation>QGIS projekt fájlok (*.qgs);;Minden fájl (*.*)</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexport.cpp" line="199"/>
        <source> exists. 
Do you want to overwrite it?</source>
        <comment>a filename is prepended to this text, and appears in a dialog box</comment>
        <translation>létezik.
Felül akarod írni?</translation>
    </message>
</context>
<context>
    <name>QgsMapserverExportBase</name>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="14"/>
        <source>Export to Mapserver</source>
        <translation>Mapserverbe exportálás</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="29"/>
        <source>Web Interface Definition</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="58"/>
        <source>Template</source>
        <translation>Sablon</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="68"/>
        <source>Path to the MapServer template file</source>
        <translation>Útvonal a MapServer sablon fájlhoz</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="369"/>
        <source>Browse...</source>
        <translation>Tallóz...</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="98"/>
        <source>Header</source>
        <translation>Fej</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="134"/>
        <source>Footer</source>
        <translation>Láb</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="159"/>
        <source>Map</source>
        <translation>Térkép</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="182"/>
        <source>Units</source>
        <translation>Egységek</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="196"/>
        <source>dd</source>
        <translation>dd</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="201"/>
        <source>feet</source>
        <translation>láb</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="206"/>
        <source>meters</source>
        <translation>méter</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="211"/>
        <source>miles</source>
        <translation>mérföld</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="216"/>
        <source>inches</source>
        <translation>hüvelyk</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="221"/>
        <source>kilometers</source>
        <translation>kilométer</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="229"/>
        <source>Image type</source>
        <translation>Kép típus</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="243"/>
        <source>gif</source>
        <translation>gif</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="248"/>
        <source>gtiff</source>
        <translation>gtiff</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="253"/>
        <source>jpeg</source>
        <translation>jpeg</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="258"/>
        <source>png</source>
        <translation>png</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="263"/>
        <source>swf</source>
        <translation>swf</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="268"/>
        <source>userdefined</source>
        <translation>felhasználó definiált</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="273"/>
        <source>wbmp</source>
        <translation>wbmp</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="291"/>
        <source>Width</source>
        <translation>Szélesség</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="304"/>
        <source>Height</source>
        <translation>Magasság</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="330"/>
        <source>Name</source>
        <translation>Név</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="340"/>
        <source>Prefix attached to map, scalebar and legend GIF filenames created using this MapFile</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="352"/>
        <source>Map file</source>
        <translation>Térkép fájl</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="362"/>
        <source>Name for the map file to be created from the QGIS project file</source>
        <translation>A QGIS projektből létrehozandó térkép fájl neve</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="376"/>
        <source>Full path to the QGIS project file to export to MapServer map format</source>
        <translation>Teljes elérési út a QGIS projekt fájlhoz, melyet MApServer térkép formátumba exportálsz</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="383"/>
        <source>QGIS project file</source>
        <translation>QGIS projekt fájl</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="393"/>
        <source>Save As...</source>
        <translation>Mentés másként...</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="400"/>
        <source>If checked, only the layer information will be processed</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="403"/>
        <source>Export LAYER information only</source>
        <translation>Csak a réteg információ exportálása</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmapserverexportbase.ui" line="56"/>
        <source>&amp;Help</source>
        <translation>&amp;Súgó</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmapserverexportbase.ui" line="59"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmapserverexportbase.ui" line="85"/>
        <source>&amp;OK</source>
        <translation>&amp;OK</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmapserverexportbase.ui" line="101"/>
        <source>&amp;Cancel</source>
        <translation>&amp;Mégsem</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmapserverexportbase.ui" line="116"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmapserverexportbase.ui" line="226"/>
        <source>MinScale</source>
        <translation>Min. méretarány</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmapserverexportbase.ui" line="236"/>
        <source>MaxScale</source>
        <translation>Max. méretarány</translation>
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
        <translation>Válassz egy jelkulcsi jelet</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmarkerdialogbase.ui" line="28"/>
        <source>Directory</source>
        <translation>Mappa</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmarkerdialogbase.ui" line="38"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmarkerdialogbase.ui" line="71"/>
        <source>Ok</source>
        <translation>Ok</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmarkerdialogbase.ui" line="81"/>
        <source>Cancel</source>
        <translation>Mégsem</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmarkerdialogbase.ui" line="108"/>
        <source>New Item</source>
        <translation>Új elem</translation>
    </message>
</context>
<context>
    <name>QgsMeasureBase</name>
    <message>
        <location filename="../src/ui/qgsmeasurebase.ui" line="22"/>
        <source>Measure</source>
        <translation>Mérés</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmeasurebase.ui" line="69"/>
        <source>Total:</source>
        <translation>Összesen:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmeasurebase.ui" line="122"/>
        <source>Help</source>
        <translation>Súgó</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmeasurebase.ui" line="145"/>
        <source>New</source>
        <translation>Új</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmeasurebase.ui" line="152"/>
        <source>Cl&amp;ose</source>
        <translation>&amp;Lezár</translation>
    </message>
</context>
<context>
    <name>QgsMeasureDialog</name>
    <message>
        <location filename="../src/app/qgsmeasuredialog.cpp" line="206"/>
        <source>Segments (in meters)</source>
        <translation>Szakaszok (méterben)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmeasuredialog.cpp" line="209"/>
        <source>Segments (in feet)</source>
        <translation>Szakaszok (lábban)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmeasuredialog.cpp" line="212"/>
        <source>Segments (in degrees)</source>
        <translation>Szakaszok (fokban)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmeasuredialog.cpp" line="215"/>
        <source>Segments</source>
        <translation>Szakaszok</translation>
    </message>
</context>
<context>
    <name>QgsMeasureTool</name>
    <message>
        <location filename="../src/app/qgsmeasuretool.cpp" line="73"/>
        <source>Incorrect measure results</source>
        <translation>Hibás mérési eredmény</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmeasuretool.cpp" line="81"/>
        <source>&lt;p&gt;This map is defined with a geographic coordinate system (latitude/longitude) but the map extents suggests that it is actually a projected coordinate system (e.g., Mercator). If so, the results from line or area measurements will be incorrect.&lt;/p&gt;&lt;p&gt;To fix this, explicitly set an appropriate map coordinate system using the &lt;tt&gt;Settings:Project Properties&lt;/tt&gt; menu.</source>
        <translation>&lt;p&gt;Ez a térkép földrajzi kooridnátarendszert használ (hosszúság/szélesség), de a térkép terjedelme arra utal, hogy most egy vetületi koordinátarendszerben van (pl. Mercator). Ha ez így van akkor a hossz vagy területmérések eredménye hibás lehet.&lt;/p&gt;&lt;p&gt;Add meg a megfelelõ térkép koordinátarendszert a &lt;tt&gt;Beállítások: Projekt tulajdonságok&lt;/tt&gt; menüpontnál a hiba javításához.</translation>
    </message>
</context>
<context>
    <name>QgsMessageViewer</name>
    <message>
        <location filename="../src/ui/qgsmessageviewer.ui" line="13"/>
        <source>QGIS Message</source>
        <translation>QGIS üzenet</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmessageviewer.ui" line="28"/>
        <source>Don&apos;t show this message again</source>
        <translation>Ne mutasd többé ezt az üzenetet</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmessageviewer.ui" line="48"/>
        <source>Close</source>
        <translation>Lezár</translation>
    </message>
</context>
<context>
    <name>QgsMySQLProvider</name>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="164"/>
        <source>Unable to access relation</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="165"/>
        <source>Unable to access the </source>
        <translation>Nem tudom elérni a </translation>
    </message>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="167"/>
        <source> relation.
The error message from the database was:
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="184"/>
        <source>No GEOS Support!</source>
        <translation>Nincs GOES támogatás!</translation>
    </message>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="187"/>
        <source>Your PostGIS installation has no GEOS support.
Feature selection and identification will not work properly.
Please install PostGIS with GEOS support (http://geos.refractions.net)</source>
        <translation>A PostGIS telepítésedben nincs GEOS támogatás.
Az elem szelektálás és azonosítás nem fog jól mûködni.
Telepítsd a PostGIS-t GEOS támogatással (http://geos.refractions.net)</translation>
    </message>
    <message>
        <location filename="" line="145"/>
        <source>Save layer as...</source>
        <translation type="obsolete">Layer mentés másként...</translation>
    </message>
    <message>
        <location filename="" line="145"/>
        <source>Error</source>
        <translation type="obsolete">Hiba</translation>
    </message>
    <message>
        <location filename="" line="145"/>
        <source>Error creating field </source>
        <translation type="obsolete">Hiba a mező létrehozásában</translation>
    </message>
    <message>
        <location filename="" line="145"/>
        <source>Layer creation failed</source>
        <translation type="obsolete">A réteg létrehozás nem sikerült</translation>
    </message>
    <message>
        <location filename="" line="145"/>
        <source>Error creating shapefile</source>
        <translation type="obsolete">Hiba a shape fájl létrehozásában</translation>
    </message>
    <message>
        <location filename="" line="145"/>
        <source>The shapefile could not be created (</source>
        <translation type="obsolete">A shape fájlt nem tudom létrehozni (</translation>
    </message>
    <message>
        <location filename="" line="145"/>
        <source>Driver not found</source>
        <translation type="obsolete">A meghajtót nem találom</translation>
    </message>
    <message>
        <location filename="" line="145"/>
        <source> driver is not available</source>
        <translation type="obsolete">meghajtó nem érhető el</translation>
    </message>
</context>
<context>
    <name>QgsNewConnection</name>
    <message>
        <location filename="../src/app/qgsnewconnection.cpp" line="121"/>
        <source>Test connection</source>
        <translation>Kapcsolat tesztelés</translation>
    </message>
    <message>
        <location filename="../src/app/qgsnewconnection.cpp" line="118"/>
        <source>Connection to %1 was successful</source>
        <translation>%1-hez kapcsolódás sikerült</translation>
    </message>
    <message>
        <location filename="../src/app/qgsnewconnection.cpp" line="121"/>
        <source>Connection failed - Check settings and try again.

Extended error information:
</source>
        <translation>Sikertelen kapcsolódás - ellenőrizd a beállításokat és próbáld újra.

Bővebb hibaüzenet:
</translation>
    </message>
</context>
<context>
    <name>QgsNewConnectionBase</name>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="21"/>
        <source>Create a New PostGIS connection</source>
        <translation>Új PostGIS kapcsolat létrehozás</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="39"/>
        <source>Connection Information</source>
        <translation>Kapcsolat információk</translation>
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
        <translation>Jelszó mentése</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="115"/>
        <source>Test Connect</source>
        <translation>Kapcsolat teszt</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="140"/>
        <source>Name</source>
        <translation>Név</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="150"/>
        <source>Host</source>
        <translation>Gép</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="160"/>
        <source>Database</source>
        <translation>Adatbázis</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="170"/>
        <source>Port</source>
        <translation>Port</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="180"/>
        <source>Username</source>
        <translation>Felhasználó név</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="190"/>
        <source>Password</source>
        <translation>Jelszó</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="210"/>
        <source>Name of the new connection</source>
        <translation>Az új kapcsolat neve</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="223"/>
        <source>5432</source>
        <translation>5432</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="255"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="271"/>
        <source>Cancel</source>
        <translation>Mégsem</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="287"/>
        <source>Help</source>
        <translation>Súgó</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="290"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
</context>
<context>
    <name>QgsNewHttpConnectionBase</name>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="13"/>
        <source>Create a New WMS connection</source>
        <translation>Új WMS kapcsolat létrehozás</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="31"/>
        <source>Connection Information</source>
        <translation>Kapcsolat információk</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="62"/>
        <source>Name of the new connection</source>
        <translation>Az új kapcsolat neve</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="72"/>
        <source>Name</source>
        <translation>Név</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="85"/>
        <source>URL</source>
        <translation>URL</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="98"/>
        <source>Proxy Host</source>
        <translation>Proxy szerver</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="111"/>
        <source>Proxy Port</source>
        <translation>Proxy port</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="124"/>
        <source>Proxy User</source>
        <translation>Proxy felhasználó</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="137"/>
        <source>Proxy Password</source>
        <translation>Proxy jelszó</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="158"/>
        <source>Your user name for the HTTP proxy (optional)</source>
        <translation>Felhasználói név a HTTP proxy-hoz (opcionális)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="173"/>
        <source>Password for your HTTP proxy (optional)</source>
        <translation>Jelszó a HTTP proxy-hoz (opcionális)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="183"/>
        <source>HTTP address of the Web Map Server</source>
        <translation>A Web térkép szerver HTTP címe</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="190"/>
        <source>Name of your HTTP proxy (optional)</source>
        <translation>A HTTP proxy neve (opcionális)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="205"/>
        <source>Port number of your HTTP proxy (optional)</source>
        <translation>Port szám a HTTP proxy-hoz (opcionális)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="226"/>
        <source>OK</source>
        <translation>OKOK</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="242"/>
        <source>Cancel</source>
        <translation>Mégsem</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="258"/>
        <source>Help</source>
        <translation>Súgó</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="261"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
</context>
<context>
    <name>QgsNorthArrowPlugin</name>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="82"/>
        <source>Bottom Left</source>
        <translation>Bal alsó</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="83"/>
        <source>Top Left</source>
        <translation>Bal felső</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="83"/>
        <source>Top Right</source>
        <translation>Jobb felső</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="83"/>
        <source>Bottom Right</source>
        <translation>Jobb alsó</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="96"/>
        <source>&amp;North Arrow</source>
        <translation>&amp;Észak jel</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="97"/>
        <source>Creates a north arrow that is displayed on the map canvas</source>
        <translation>A térképen megjelenő éeszak jel létrehozása</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="255"/>
        <source>&amp;Decorations</source>
        <translation>&amp;Dekorációk</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="246"/>
        <source>North arrow pixmap not found</source>
        <translation>Észak nyíl pixmappet nem találom</translation>
    </message>
</context>
<context>
    <name>QgsNorthArrowPluginGui</name>
    <message>
        <location filename="../src/plugins/north_arrow/plugingui.cpp" line="157"/>
        <source>Pixmap not found</source>
        <translation>Pixmap-et nem találom</translation>
    </message>
</context>
<context>
    <name>QgsNorthArrowPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="237"/>
        <source>North Arrow Plugin</source>
        <translation>Észak jel modul</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="35"/>
        <source>Properties</source>
        <translation>Tulajdonságok</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="55"/>
        <source>Angle</source>
        <translation>Szög</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="62"/>
        <source>Placement</source>
        <translation>Elhelyezés</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="75"/>
        <source>Set direction automatically</source>
        <translation>Irány automatikus beállítása</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="85"/>
        <source>Enable North Arrow</source>
        <translation>Észak jel bekapcsolás</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="137"/>
        <source>Placement on screen</source>
        <translation>Elkhelyezés a képernyőn</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="141"/>
        <source>Top Left</source>
        <translation>Bal felső</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="146"/>
        <source>Top Right</source>
        <translation>Jobb felső</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="151"/>
        <source>Bottom Left</source>
        <translation>Bal alsó</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="156"/>
        <source>Bottom Right</source>
        <translation>Jobb alsó</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="164"/>
        <source>Preview of north arrow</source>
        <translation>Észak jel előnézet</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="183"/>
        <source>Icon</source>
        <translation>Ikon</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="198"/>
        <source>Browse...</source>
        <translation>Tallóz...</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="206"/>
        <source>New Item</source>
        <translation>Új elem</translation>
    </message>
</context>
<context>
    <name>QgsOGRFactory</name>
    <message>
        <location filename="" line="145"/>
        <source>Wrong Path/URI</source>
        <translation type="obsolete">Hibás útvonal/URI</translation>
    </message>
    <message>
        <location filename="" line="145"/>
        <source>The provided path for the dataset is not valid.</source>
        <translation type="obsolete">Az adat halmazhoz megadott útvonal hibás.</translation>
    </message>
</context>
<context>
    <name>QgsOptions</name>
    <message>
        <location filename="../src/app/qgsoptions.cpp" line="148"/>
        <source>Detected active locale on your system: </source>
        <translation>A rendszer detektált helyi beállítása:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsoptions.cpp" line="300"/>
        <source>to vertex</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsoptions.cpp" line="304"/>
        <source>to segment</source>
        <translation type="unfinished">szakaszhoz</translation>
    </message>
    <message>
        <location filename="../src/app/qgsoptions.cpp" line="308"/>
        <source>to vertex and segment</source>
        <translation type="unfinished">törésponthoz és szakaszhoz</translation>
    </message>
</context>
<context>
    <name>QgsOptionsBase</name>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="13"/>
        <source>QGIS Options</source>
        <translation>QGIS beállítások</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="38"/>
        <source>&amp;General</source>
        <translation>&amp;Általános</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="50"/>
        <source>General</source>
        <translation>Általános</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="62"/>
        <source>Ask to save project changes when required</source>
        <translation>Rákérdez a projekt módosítások mentésére amikor szükséges</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="69"/>
        <source>Warn me when opening a project file saved with an older version of QGIS</source>
        <translation>Figylemeztessen amikor egy korábbi QGIS verzióval mentett projekt fájlt nyit meg</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="93"/>
        <source>&amp;Appearance</source>
        <translation>&amp;Megjelenés</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="105"/>
        <source>&amp;Splash screen</source>
        <translation>&amp;Indító kép</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="117"/>
        <source>Hide splash screen at startup</source>
        <translation>Indító kép elrejtése</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="127"/>
        <source>&amp;Icon Theme</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="139"/>
        <source>&lt;b&gt;Note: &lt;/b&gt;Theme changes take effect the next time QGIS is started</source>
        <translation>&lt;b&gt;Megjegyzés: &lt;/b&gt;A módosítások a QGIS legközelebbi indítása után lépnek életbe</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="157"/>
        <source>Theme</source>
        <translation>Téma</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="195"/>
        <source>Default Map Appearance (Overridden by project properties)</source>
        <translation>Alapértelmezett térkép megjelenés (projekt tulajdonságok felülírják)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="207"/>
        <source>Selection Color:</source>
        <translation>Szelekció szín:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="243"/>
        <source>Background Color:</source>
        <translation>Háttérszín:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="282"/>
        <source>Appearance</source>
        <translation>Megjelenés</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="294"/>
        <source>Capitalise layer name</source>
        <translation>Nagybetűs rétegnevek</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="318"/>
        <source>&amp;Rendering</source>
        <translation>&amp;Megjelenítés</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="330"/>
        <source>&amp;Update during drawing</source>
        <translation>&amp;Frissítés rajzolás közben</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="342"/>
        <source>features</source>
        <translation>elemek</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="349"/>
        <source>Map display will be updated (drawn) after this many features have been read from the data source</source>
        <translation>A térkép megjelenítést ennyi elem beolvasása után frissítem (rajzolom)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="362"/>
        <source>Update display after reading</source>
        <translation>Megjelenítés frissítése ennyi beolvasása után</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="372"/>
        <source>(Set to 0 to not update the display until all features have been read)</source>
        <translation>(Állítsd 0-ra, ha csak valamennyi elem beolvasása után szeretnéd a térképet frissíteni)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="382"/>
        <source>Initial Visibility</source>
        <translation>Kezdeti láthatóság</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="394"/>
        <source>By default new la&amp;yers added to the map should be displayed</source>
        <translation>Alapértelmezés szerint a térképhez adott &amp;rétegek megjelennek</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="404"/>
        <source>Rendering</source>
        <translation>Megjelenítés</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="416"/>
        <source>Make lines appear less jagged at the expense of some drawing performance</source>
        <translation>A vonalak kevésbé töredezett megjelenítése, a rajzolási sebesség lassulása árán</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="423"/>
        <source>Selecting this will unselect the &apos;make lines less&apos; jagged toggle</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="426"/>
        <source>Fix problems with incorrectly filled polygons</source>
        <translation>A hibásan kitöltött felületek probléma javítása</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="433"/>
        <source>Continuously redraw the map when dragging the legend/map divider</source>
        <translation>Folyamatos újrarajzolás a térkép vagy jelkulcs módosításakor</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="457"/>
        <source>&amp;Map tools</source>
        <translation>&amp;Térkép eszközök</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="469"/>
        <source>Panning and zooming</source>
        <translation>Eltolás és nagyítás</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="482"/>
        <source>Zoom</source>
        <translation>Nagyít</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="487"/>
        <source>Zoom and recenter</source>
        <translation>Nagyítás és középre igazítás</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="497"/>
        <source>Nothing</source>
        <translation>Semmi</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="505"/>
        <source>Zoom factor:</source>
        <translation>Nagyítási faktor:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="512"/>
        <source>Mouse wheel action:</source>
        <translation>Egérgörgő művelet:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="535"/>
        <source>Measure tool</source>
        <translation>Mérőeszköz</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="576"/>
        <source>Rubberband color:</source>
        <translation>Gumiszalag szín:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="586"/>
        <source>Ellipsoid for distance calculations:</source>
        <translation>Ellipszoid a távolság számításhoz:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="599"/>
        <source>Search radius</source>
        <translation>Keresési sugár</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="611"/>
        <source>(Specify the search radius as a percentage of the map width)</source>
        <translation>(Add meg a keresési sugarat mint a térkép szélesség százaléka)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="621"/>
        <source>Search Radius for Identifying Features and displaying Map Tips</source>
        <translation>Keresési sugár az elemek azonosításához és a térkép tippek megjelenítéséhez</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="631"/>
        <source>%</source>
        <translation>%</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="664"/>
        <source>Digitizing</source>
        <translation>Digitalizálás</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="820"/>
        <source>Rubberband</source>
        <translation>Gumiszalag</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="840"/>
        <source>Line Width:</source>
        <translation>Vonalvastagság:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="850"/>
        <source>Line width in pixels</source>
        <translation>Vonalvastagság képpontokban</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="873"/>
        <source>Line Colour:</source>
        <translation>Vonal szín:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="689"/>
        <source>Snapping</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="788"/>
        <source>Default Snapping Tolerance (in layer units):</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="751"/>
        <source>Search radius for vertex edits (in layer units):</source>
        <translation>Keresési sugár töréspont szerkesztéshez (réteg egységekben):</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="902"/>
        <source>Pro&amp;jection</source>
        <translation>&amp;Vetület</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="927"/>
        <source>Select Global Default ...</source>
        <translation>Globális alapértelmezés kiválasztás ...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="937"/>
        <source>When layer is loaded that has no projection information</source>
        <translation>Amikor vetületi információ nélküli réteget töltesz be</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="949"/>
        <source>Prompt for projection.</source>
        <translation>Rákérdezés a vetületre.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="956"/>
        <source>Project wide default projection will be used.</source>
        <translation>A projekt alapértelmezett vetületet használom.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="963"/>
        <source>Global default projection displa&amp;yed below will be used.</source>
        <translation>A lent meg&amp;jelenõ globális alapértelmezett vetületet használom.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="974"/>
        <source>Locale</source>
        <translation>Helyi beállítás</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="986"/>
        <source>Force Override System Locale</source>
        <translation>Helyi beállítás felülírása</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="1001"/>
        <source>Locale to use instead</source>
        <translation>Használandó helyi beállítás</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="1014"/>
        <source>Note: Enabling / changing overide on local requires an application restart.</source>
        <translation>Megjegyzés: a helyi beállítás engedélyezése/módosítása az alkalmazás újraindítását igényli.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="1040"/>
        <source>Additional Info</source>
        <translation>További információ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="1052"/>
        <source>Detected active locale on your system:</source>
        <translation>A rendszereden az aktív helyi beállítás:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="492"/>
        <source>Zoom to mouse cursor</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="709"/>
        <source>Default Snap Mode:</source>
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
        <translation>&lt;b&gt;Megjegyzés: Ez a funkció nem mûködik még!&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="65"/>
        <source>Source</source>
        <translation>Forrás</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="86"/>
        <source>Destination</source>
        <translation>Cél</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="125"/>
        <source>&amp;Help</source>
        <translation>&amp;Súgó</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="128"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="154"/>
        <source>Add New Transfer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="161"/>
        <source>&amp;OK</source>
        <translation>&amp;OK</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="177"/>
        <source>&amp;Cancel</source>
        <translation>&amp;Mégsem</translation>
    </message>
</context>
<context>
    <name>QgsPatternDialogBase</name>
    <message>
        <location filename="../src/ui/qgspatterndialogbase.ui" line="13"/>
        <source>Select a fill pattern</source>
        <translation>Válassz egy kitöltési mintát</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspatterndialogbase.ui" line="189"/>
        <source>No Fill</source>
        <translation>Nincs kitöltés</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspatterndialogbase.ui" line="205"/>
        <source>Cancel</source>
        <translation>Mégsem</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspatterndialogbase.ui" line="212"/>
        <source>Ok</source>
        <translation>Ok</translation>
    </message>
</context>
<context>
    <name>QgsPgGeoprocessing</name>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="78"/>
        <source>&amp;Buffer features</source>
        <translation>&amp;Elem övezetek</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="80"/>
        <source>A new layer is created in the database with the buffered features.</source>
        <translation>Egy új réteget készítettem az adatbázisban az elem övezetekbõl.</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="410"/>
        <source>&amp;Geoprocessing</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="117"/>
        <source>Buffer features in layer %1</source>
        <translation>Elem övezetek a %1 rétegben</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="325"/>
        <source>Unable to add geometry column</source>
        <translation>Nem tudok geometria oszlopot hozzáadni</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="327"/>
        <source>Unable to add geometry column to the output table </source>
        <translation>Nem tudok geometriai oszlopot hozzáadni az eredmény táblához </translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="331"/>
        <source>Unable to create table</source>
        <translation>Nem tudom a táblát létrehozni</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="333"/>
        <source>Failed to create the output table </source>
        <translation>Nem sikerült az eredmény táblát létrehozni </translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="340"/>
        <source>Error connecting to the database</source>
        <translation>Hiba az adatbázishoz csatlakozásban</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="346"/>
        <source>No GEOS support</source>
        <translation>Nincs GOES támogatás</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="347"/>
        <source>Buffer function requires GEOS support in PostGIS</source>
        <translation>Az övezet funkcióhoz GEOS támogatás szükséges a PostGIS-ben</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="350"/>
        <source>Not a PostgreSQL/PostGIS Layer</source>
        <translation>Nem PostgreSQL/PostGIS réteg</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="353"/>
        <source> is not a PostgreSQL/PostGIS layer.
</source>
        <translation> nem PostgreSQL/PostGIS réteg.
</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="353"/>
        <source>Geoprocessing functions are only available for PostgreSQL/PostGIS Layers</source>
        <translation>A térinformatikai funkciók csak a PostgreSQL/PostGIS rétegekre használhatók</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="356"/>
        <source>No Active Layer</source>
        <translation>Nincs aktív réteg</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="357"/>
        <source>You must select a layer in the legend to buffer</source>
        <translation>Az övezet készítéshez egy réteget kell választani a jelmagyarázatból</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="80"/>
        <source>Create a buffer for a PostgreSQL layer. </source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsPgQueryBuilder</name>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="77"/>
        <source>Table &lt;b&gt;%1&lt;/b&gt; in database &lt;b&gt;%2&lt;/b&gt; on host &lt;b&gt;%3&lt;/b&gt;, user &lt;b&gt;%4&lt;/b&gt;</source>
        <translation>A(z) &lt;b&gt;%1&lt;/b&gt; tábla a &lt;b&gt;%2&lt;/b&gt; adatbázisban a &lt;b&gt;%3&lt;/b&gt; gépen, &lt;b&gt;%4&lt;/b&gt;felhasználó</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="61"/>
        <source>Connection Failed</source>
        <translation>Sikertelen kapcsolódás</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="61"/>
        <source>Connection to the database failed:</source>
        <translation>Nem sikerült a csatlakozás az adatbázishoz:</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="198"/>
        <source>Database error</source>
        <translation>Adatbázis hiba</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="198"/>
        <source>&lt;p&gt;Failed to get sample of field values using SQL:&lt;/p&gt;&lt;p&gt;</source>
        <translation>&lt;p&gt;Nem sikerült a mezõ értékeket megkapni ezzel az SQL utasítással:&lt;/p&gt;&lt;p&gt;</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="262"/>
        <source>No Query</source>
        <translation>Nincs lekérdezés</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="262"/>
        <source>You must create a query before you can test it</source>
        <translation>A lekérdezést létre kell hoznod a tesztelése elõtt</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="273"/>
        <source>Query Result</source>
        <translation>Lekérdezés eredmény</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="275"/>
        <source>The where clause returned </source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="275"/>
        <source> rows.</source>
        <translation> sor.</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="279"/>
        <source>Query Failed</source>
        <translation>Sikertelen lekérdezés</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="281"/>
        <source>An error occurred when executing the query:</source>
        <translation>Hiba történt a lekérdezés során:</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="328"/>
        <source>Error in Query</source>
        <translation>Hiba a lekérdezésben</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="334"/>
        <source>No Records</source>
        <translation>Nincsenek rekordok</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="334"/>
        <source>The query you specified results in zero records being returned. Valid PostgreSQL layers must have at least one feature.</source>
        <translation>A megadott lekérdezés nulla sort eredményezett. Egy érvényes PostgreSQL rétegnek legaláb egy elemet kell tartalmaznia.</translation>
    </message>
</context>
<context>
    <name>QgsPgQueryBuilderBase</name>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="23"/>
        <source>PostgreSQL Query Builder</source>
        <translation>PostgreSQL lekérdezés szerkesztõ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="41"/>
        <source>Operators</source>
        <translation>Mûveletek</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="53"/>
        <source>=</source>
        <translation>=</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="60"/>
        <source>&lt;</source>
        <translation>&lt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="67"/>
        <source>NOT</source>
        <translation>Nem</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="74"/>
        <source>OR</source>
        <translation>Vagy</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="81"/>
        <source>AND</source>
        <translation>És</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="88"/>
        <source>%</source>
        <translation>%</translation>
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
        <translation>!=</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="116"/>
        <source>&gt;</source>
        <translation>&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="123"/>
        <source>LIKE</source>
        <translation>Mint</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="130"/>
        <source>ILIKE</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="137"/>
        <source>&gt;=</source>
        <translation>&gt;=</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="144"/>
        <source>&lt;=</source>
        <translation>&lt;=</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="178"/>
        <source>Clear</source>
        <translation>Törlés</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="188"/>
        <source>Test</source>
        <translation>Teszt</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="198"/>
        <source>Ok</source>
        <translation>Ok</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="208"/>
        <source>Cancel</source>
        <translation>Mégsem</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="241"/>
        <source>Values</source>
        <translation>Értékek</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="253"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Retrieve &lt;span style=&quot; font-weight:600;&quot;&gt;all&lt;/span&gt; the record in the vector file (&lt;span style=&quot; font-style:italic;&quot;&gt;if the table is big, the operation can consume some time&lt;/span&gt;)&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Retrieve &lt;span style=&quot; font-weight:600;&quot;&gt;all&lt;/span&gt; rekord a vektor fájlban (&lt;span style=&quot; font-style:italic;&quot;&gt;ha a tábla nagy, a mûvelet sokáig tarthat&lt;/span&gt;)&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="259"/>
        <source>All</source>
        <translation>Mind</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="266"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Take a &lt;span style=&quot; font-weight:600;&quot;&gt;sample&lt;/span&gt; of records in the vector file&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Nézz egy &lt;span style=&quot; font-weight:600;&quot;&gt;mintát&lt;/span&gt; a rekordokból a vektor fájlban&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="272"/>
        <source>Sample</source>
        <translation>Minta</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="279"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;List of values for the current field.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Az aktuália mezõ értékeinek a listája.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="309"/>
        <source>Fields</source>
        <translation>Mezõk</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="321"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;List of fields in this vector file&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Az aktuália mezõ értékeinek a listája.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="334"/>
        <source>Datasource:</source>
        <translation>Adatforrás:</translation>
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
        <translation>Név</translation>
    </message>
    <message>
        <location filename="../src/app/qgspluginmanager.cpp" line="77"/>
        <source>Version</source>
        <translation>Verzió</translation>
    </message>
    <message>
        <location filename="../src/app/qgspluginmanager.cpp" line="78"/>
        <source>Description</source>
        <translation>Leírás</translation>
    </message>
    <message>
        <location filename="../src/app/qgspluginmanager.cpp" line="79"/>
        <source>Library name</source>
        <translation>Könyvtárnév</translation>
    </message>
    <message>
        <location filename="../src/app/qgspluginmanager.cpp" line="180"/>
        <source>No Plugins</source>
        <translation>Nincsenek modulok</translation>
    </message>
    <message>
        <location filename="../src/app/qgspluginmanager.cpp" line="180"/>
        <source>No QGIS plugins found in </source>
        <translation>Nem találok QGIS modulokat itt </translation>
    </message>
</context>
<context>
    <name>QgsPluginManagerBase</name>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="16"/>
        <source>QGIS Plugin Manager</source>
        <translation>QGIS model menedzser</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="39"/>
        <source>Plugin Directory</source>
        <translation>Modul mappa</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="61"/>
        <source>To load a plugin, click the checkbox next to the plugin and click Ok</source>
        <translation>A modul betöltéséhez kattints a négyzetbe a modul mellett és kattints az OK-ra</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="92"/>
        <source>&amp;Select All</source>
        <translation>&amp;Mindent szelektál</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="95"/>
        <source>Alt+S</source>
        <translation>Alt+M</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="102"/>
        <source>C&amp;lear All</source>
        <translation>Mindent &amp;töröl</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="105"/>
        <source>Alt+L</source>
        <translation>Alt+T</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="112"/>
        <source>&amp;Ok</source>
        <translation>&amp;OK</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="115"/>
        <source>Alt+O</source>
        <translation>Alt+O</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="122"/>
        <source>&amp;Close</source>
        <translation>&amp;Lezár</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="125"/>
        <source>Alt+C</source>
        <translation>Alt+L</translation>
    </message>
</context>
<context>
    <name>QgsPointDialog</name>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="556"/>
        <source>Linear</source>
        <translation>Lineáris</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="197"/>
        <source>Choose a name for the world file</source>
        <translation>Válassz egy nevet a world fájlhoz</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="557"/>
        <source>Helmert</source>
        <translation>Helmert</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="215"/>
        <source>-modified</source>
        <comment>Georeferencer:QgsPointDialog.cpp - used to modify a user given filename</comment>
        <translation>-módosítva</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="264"/>
        <source>Warning</source>
        <translation>Figyelmeztetés</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="270"/>
        <source>&lt;p&gt;A Helmert transform requires modifications in the raster layer.&lt;/p&gt;&lt;p&gt;The modified raster will be saved in a new file and a world file will be generated for this new file instead.&lt;/p&gt;&lt;p&gt;Are you sure that this is what you want?&lt;/p&gt;</source>
        <translation>&lt;p&gt;A Helmert transzformáció a raszter réteget módosítja.&lt;/p&gt;&lt;p&gt;A módosított rasztert egy új fájlba mentem és a world fájl ehhez az új fájlhoz készítem el.&lt;/p&gt;&lt;p&gt;Biztos vagy, hogy ezt akarod?&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="271"/>
        <source>Currently all modified files will be written in TIFF format.</source>
        <translation>Minden módosított fájlt TIFF formátumban írom ki.</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="279"/>
        <source>Affine</source>
        <translation>Affin</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="289"/>
        <source>Not implemented!</source>
        <translation>Nincs implementálva!</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="284"/>
        <source>&lt;p&gt;An affine transform requires changing the original raster file. This is not yet supported.&lt;/p&gt;</source>
        <translation>&lt;p&gt;Az affin transzformáció a raszter réteget módosítja. Ez most még nem támogatott&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="291"/>
        <source>&lt;p&gt;The </source>
        <translation>&lt;p&gt;A(z) </translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="292"/>
        <source> transform is not yet supported.&lt;/p&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="323"/>
        <source>Error</source>
        <translation>Hiba</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="324"/>
        <source>Could not write to </source>
        <translation>Nem tudok írni ide </translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="487"/>
        <source>Zoom In</source>
        <translation>Nagyítás</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="486"/>
        <source>z</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="492"/>
        <source>Zoom Out</source>
        <translation>Kicsinyítés</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="491"/>
        <source>Z</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="495"/>
        <source>Zoom To Layer</source>
        <translation>Rétegre nagyítás</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="497"/>
        <source>Zoom to Layer</source>
        <translation>Rétegre nagyítás</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="500"/>
        <source>Pan Map</source>
        <translation>Térkép eltolás</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="501"/>
        <source>Pan the map</source>
        <translation>A térkép eltolása</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="504"/>
        <source>Add Point</source>
        <translation>Pont hozzáadás</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="505"/>
        <source>.</source>
        <translation>.</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="506"/>
        <source>Capture Points</source>
        <translation>Pont digitalizálás</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="509"/>
        <source>Delete Point</source>
        <translation>Pont  törlés</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="510"/>
        <source>Delete Selected</source>
        <translation>Szelektáltak törlése</translation>
    </message>
</context>
<context>
    <name>QgsPointDialogBase</name>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="13"/>
        <source>Reference points</source>
        <translation>Referencia pontok</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="38"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="45"/>
        <source>Modified raster:</source>
        <translation>Módosított raszter:</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="52"/>
        <source>World file:</source>
        <translation>World fájl:</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="65"/>
        <source>Transform type:</source>
        <translation>Transzformáció típus:</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="75"/>
        <source>Create</source>
        <translation>Létrehoz</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="105"/>
        <source>Add points</source>
        <translation>Pontok hozzáadása</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="130"/>
        <source>Delete points</source>
        <translation>Pontok törlése</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="178"/>
        <source>Zoom in</source>
        <translation>Nagyítás</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="200"/>
        <source>Zoom out</source>
        <translation>Kicsinyítés</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="222"/>
        <source>Zoom to the raster extents</source>
        <translation>Nagyítás a raszter terjedelemre</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="244"/>
        <source>Pan</source>
        <translation>Eltolás</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="282"/>
        <source>Create and load layer</source>
        <translation>Réteg létrehozás és betöltés</translation>
    </message>
</context>
<context>
    <name>QgsPointStyleWidgetBase</name>
    <message>
        <location filename="../src/ui/qgspointstylewidgetbase.ui" line="16"/>
        <source>Form3</source>
        <translation>Form3</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspointstylewidgetbase.ui" line="36"/>
        <source>Symbol Style</source>
        <translation>Szimbólum stílus</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspointstylewidgetbase.ui" line="51"/>
        <source>Scale</source>
        <translation>Méretarány</translation>
    </message>
</context>
<context>
    <name>QgsPostgresProvider</name>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="132"/>
        <source>Unable to access relation</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="112"/>
        <source>Unable to access the </source>
        <translation>Nem tudok hozzáférni a </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="135"/>
        <source> relation.
The error message from the database was:
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="321"/>
        <source>No GEOS Support!</source>
        <translation>Nincs GOES támogatás!</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="325"/>
        <source>Your PostGIS installation has no GEOS support.
Feature selection and identification will not work properly.
Please install PostGIS with GEOS support (http://geos.refractions.net)</source>
        <translation>A PostGIS telepítésedben nincs GEOS támogatás.Az elem szelektálás és azonosítás nem fog jól mûködni.Telepítsd a PostGIS-t GEOS támogatással (http://geos.refractions.net)</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="918"/>
        <source>No suitable key column in table</source>
        <translation>Nincs megfelelõ kuls oszlop a táblában</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="922"/>
        <source>The table has no column suitable for use as a key.

Qgis requires that the table either has a column of type
int4 with a unique constraint on it (which includes the
primary key) or has a PostgreSQL oid column.
</source>
        <translation>A táblában nincs megfelelpõ oszlop ami kulcsként használható.

A QGIS-nek szüksége van egy int4 típusú oszlopra az egyediség korlátozásával 
(amit az elsõdleges kulcs magában foglal) vagy egy PostgreSQL iod oszlopra.
</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="963"/>
        <source>The unique index on column</source>
        <translation>Az egyedi index az oszlopon</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="965"/>
        <source>is unsuitable because Qgis does not currently support non-int4 type columns as a key into the table.
</source>
        <translation>nem megfelelõ, mert QGIS csak az int4 típusú oszlopokat fogadja el a mint a tábla kulcsa.
</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="988"/>
        <source>and </source>
        <translation>és </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="994"/>
        <source>The unique index based on columns </source>
        <translation>Az egyedi index az oszlopokon </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="996"/>
        <source> is unsuitable because Qgis does not currently support multiple columns as a key into the table.
</source>
        <translation> nem megfelelõ, mert a QGIS nem engedi meg a táblában az összetett kulcsokat.
</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1039"/>
        <source>Unable to find a key column</source>
        <translation>Nem találom a kulcs oszlopot</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1119"/>
        <source> derives from </source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1123"/>
        <source>and is suitable.</source>
        <translation> és megfelelõ.</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1127"/>
        <source>and is not suitable </source>
        <translation>és nem megfelelõ </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1128"/>
        <source>type is </source>
        <translation>típus </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1130"/>
        <source> and has a suitable constraint)</source>
        <translation> és megfelelõ korlátozással bír)</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1132"/>
        <source> and does not have a suitable constraint)</source>
        <translation> és nem bír megfelelõ korlátozással)</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1218"/>
        <source>Note: </source>
        <translation>Megjegyzés: </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1220"/>
        <source>initially appeared suitable but does not contain unique data, so is not suitable.
</source>
        <translation>kezdetben megfelelõnek tünt, de nem tartalmaz egyedi adatokat így nem megfelelõ.
</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1232"/>
        <source>The view you selected has the following columns, none of which satisfy the above conditions:</source>
        <translation>A kiválasztott nézet táblában a következõ oszlopok találhatók, melyek közül egyik sem elégíti ki a fenti feltételeket:</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1238"/>
        <source>Qgis requires that the view has a column that can be used as a unique key. Such a column should be derived from a table column of type int4 and be a primary key, have a unique constraint on it, or be a PostgreSQL oid column. To improve performance the column should also be indexed.
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1239"/>
        <source>The view </source>
        <translation>A nézet tábla</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1240"/>
        <source>has no column suitable for use as a unique key.
</source>
        <translation>nincsmegfelelõ, kulcsként használható oszlopa.
</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1241"/>
        <source>No suitable key column in view</source>
        <translation>Nincs megfelelõkulcs oszlop a nézet táblában</translation>
    </message>
    <message>
        <location filename="" line="145"/>
        <source>INSERT error</source>
        <translation type="obsolete">INSERT hiba</translation>
    </message>
    <message>
        <location filename="" line="145"/>
        <source>An error occured during feature insertion</source>
        <translation type="obsolete">Hiba az elem beszúrása közben</translation>
    </message>
    <message>
        <location filename="" line="145"/>
        <source>DELETE error</source>
        <translation type="obsolete">DELETE hiba</translation>
    </message>
    <message>
        <location filename="" line="145"/>
        <source>An error occured during deletion from disk</source>
        <translation type="obsolete">Hiba fordult elõ a lemezrõl törlés közben</translation>
    </message>
    <message>
        <location filename="" line="145"/>
        <source>PostGIS error</source>
        <translation type="obsolete">PostGIS hiba</translation>
    </message>
    <message>
        <location filename="" line="145"/>
        <source>An error occured contacting the PostgreSQL database</source>
        <translation type="obsolete">Hiba fordult elõ a PostgreSQL adatbázissal érintkezés közben</translation>
    </message>
    <message>
        <location filename="" line="145"/>
        <source>The PostgreSQL database returned: </source>
        <translation type="obsolete">A PostgreSQL adatbázis válasza: </translation>
    </message>
    <message>
        <location filename="" line="145"/>
        <source>When trying: </source>
        <translation type="obsolete">Amikor próbálom: </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2529"/>
        <source>Unknown geometry type</source>
        <translation>Ismeretlen geometria típus</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2530"/>
        <source>Column </source>
        <translation type="unfinished">Oszlop </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2540"/>
        <source> in </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2532"/>
        <source> has a geometry type of </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2532"/>
        <source>, which Qgis does not currently support.</source>
        <translation>, melyet a QGIS nem támogat.</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2541"/>
        <source>. The database communication log was:
</source>
        <translation>. Az adatbázis kommunikáció logja:
</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2542"/>
        <source>Unable to get feature type and srid</source>
        <translation>Nem tudom beszerezni az elem típust és az srid-t</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="133"/>
        <source>Unable to determine table access privileges for the </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1874"/>
        <source>Error while adding features</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1906"/>
        <source>Error while deleting features</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1936"/>
        <source>Error while adding attributes</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1973"/>
        <source>Error while deleting attributes</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2037"/>
        <source>Error while changing attributes</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2116"/>
        <source>Error while changing geometry values</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2539"/>
        <source>Qgis was unable to determine the type and srid of column </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.h" line="505"/>
        <source>unexpected PostgreSQL error</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsProjectPropertiesBase</name>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="13"/>
        <source>Project Properties</source>
        <translation>Projekt tulajdonságok</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="38"/>
        <source>General</source>
        <translation>Általános</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="50"/>
        <source>Project Title</source>
        <translation>Projekt cím</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="62"/>
        <source>Descriptive project name</source>
        <translation>LEíró projektnév</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="65"/>
        <source>Default project title</source>
        <translation>Alapértelmezett projekt cím</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="75"/>
        <source>Precision</source>
        <translation>Pontosság</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="87"/>
        <source>Automatically sets the number of decimal places in the mouse position display</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="90"/>
        <source>The number of decimal places that are used when displaying the mouse position is automatically set to be enough so that moving the mouse by one pixel gives a change in the position display</source>
        <translation>A tizedesjegyek számát, melyet az egérpozíció megjelenítéséhez használok, automatikusan állítom be úgy, hogy egy képpontnyi elmozdulás változást okozzon a megjelenített koordinátában</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="93"/>
        <source>Automatic</source>
        <translation>Automatikus</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="106"/>
        <source>Sets the number of decimal places to use for the mouse position display</source>
        <translation>Az egés pozicíó megjelenítésénél használ tizedesjegyek számának beállítása</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="109"/>
        <source>Manual</source>
        <translation>Manuális</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="119"/>
        <source>The number of decimal places for the manual option</source>
        <translation>A tizedesjegyek száma a kézi beállításhoz</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="134"/>
        <source>decimal places</source>
        <translation>tizedesjegyek</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="157"/>
        <source>Map Appearance</source>
        <translation>Térkép megjelenés</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="169"/>
        <source>Selection Color:</source>
        <translation>Szelekció szín:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="208"/>
        <source>Background Color:</source>
        <translation>Háttér szín:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="255"/>
        <source>Digitizing</source>
        <translation>Digitalizálás</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="267"/>
        <source>Enable topological editing</source>
        <translation>Topológia szerkesztés engedélyezése</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="280"/>
        <source>Snapping options...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="293"/>
        <source>Avoid intersections of new polygons</source>
        <translation>Új felületek metszésének elkerülése</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="301"/>
        <source>Map Units</source>
        <translation>Térkép egységek</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="313"/>
        <source>Meters</source>
        <translation>Méter</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="323"/>
        <source>Feet</source>
        <translation>Láb</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="330"/>
        <source>Decimal degrees</source>
        <translation>Fok és tizedei</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="354"/>
        <source>Projection</source>
        <translation>Vetület</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="366"/>
        <source>Enable on the fly projection</source>
        <translation>Átvetítés engedélyezése</translation>
    </message>
</context>
<context>
    <name>QgsProjectionSelector</name>
    <message>
        <location filename="../src/gui/qgsprojectionselector.cpp" line="783"/>
        <source>QGIS SRSID: </source>
        <translation>QGIS SRSID: </translation>
    </message>
    <message>
        <location filename="../src/gui/qgsprojectionselector.cpp" line="784"/>
        <source>PostGIS SRID: </source>
        <translation>PostGIS SRID: </translation>
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
        <translation>Vetület</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="67"/>
        <source>Search</source>
        <translation>Keres</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="85"/>
        <source>Find</source>
        <translation>Keres</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="98"/>
        <source>Name</source>
        <translation>Név</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="111"/>
        <source>QGIS SRSID</source>
        <translation>QGIS SRSID</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="124"/>
        <source>EPSG ID</source>
        <translation>EPSG ID</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="137"/>
        <source>Postgis SRID</source>
        <translation>Postgis SRID</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="196"/>
        <source>Spatial Reference System</source>
        <translation>Térbeli referencia rendszer</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="201"/>
        <source>Id</source>
        <translation>Id</translation>
    </message>
</context>
<context>
    <name>QgsPythonDialog</name>
    <message>
        <location filename="../src/ui/qgspythondialog.ui" line="13"/>
        <source>Python console</source>
        <translation>Phyton konzol</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspythondialog.ui" line="33"/>
        <source>To access Quantum GIS environment from this python console use object from global scope which is an instance of QgisInterface class.&lt;br&gt;Usage e.g.: iface.zoomFull()</source>
        <translation>A phyton konzolról a QGIOS környezetet a globális objektum használatával érheted el, mely a QgisInterface osztály példánya. &lt;br&gt;Használat pl.: iface.zoomFull()</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspythondialog.ui" line="58"/>
        <source>&gt;&gt;&gt;</source>
        <translation>&gt;&gt;&gt;</translation>
    </message>
</context>
<context>
    <name>QgsQuickPrint</name>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="845"/>
        <source> km</source>
        <translation> km</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="850"/>
        <source> mm</source>
        <translation> mm</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="855"/>
        <source> cm</source>
        <translation> cm</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="859"/>
        <source> m</source>
        <translation> m</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="864"/>
        <source> miles</source>
        <translation> mérföld</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="869"/>
        <source> mile</source>
        <translation> mérföld</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="874"/>
        <source> inches</source>
        <translation> hüvelyk</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="879"/>
        <source> foot</source>
        <translation> láb</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="883"/>
        <source> feet</source>
        <translation> láb</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="888"/>
        <source> degree</source>
        <translation> fok</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="890"/>
        <source> degrees</source>
        <translation> fok</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="893"/>
        <source> unknown</source>
        <translation> ismeretlen</translation>
    </message>
</context>
<context>
    <name>QgsRasterLayer</name>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="281"/>
        <source>and all other files</source>
        <translation>és minden egyéb fájl</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2275"/>
        <source>Raster Extent: </source>
        <translation>Raszter terjedelem: </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2278"/>
        <source>Clipped area: </source>
        <translation>Kivágott terület: </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3500"/>
        <source>Driver:</source>
        <translation>Meghajtó:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3526"/>
        <source>Dataset Description</source>
        <translation>Adathalmaz leírás</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3577"/>
        <source>Dimensions:</source>
        <translation>Méretek:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3580"/>
        <source>X: </source>
        <translation>X: </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3581"/>
        <source> Y: </source>
        <translation> Y: </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3581"/>
        <source> Bands: </source>
        <translation> sávok: </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3588"/>
        <source>No Data Value</source>
        <translation>Nincs adat érték</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3597"/>
        <source>NoDataValue not set</source>
        <translation>A NoDataValue nincs beállítva</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3603"/>
        <source>Data Type:</source>
        <translation>Adat típus:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3609"/>
        <source>GDT_Byte - Eight bit unsigned integer</source>
        <translation>GDT_Byte - Nyolc bites elõjel nélküli egész</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3612"/>
        <source>GDT_UInt16 - Sixteen bit unsigned integer </source>
        <translation>GDT_UInt16 - Tizenhat bites elõjel nélküli egész </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3615"/>
        <source>GDT_Int16 - Sixteen bit signed integer </source>
        <translation>GDT_Int16 - Tizenhat bites elõjeles egész </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3618"/>
        <source>GDT_UInt32 - Thirty two bit unsigned integer </source>
        <translation>GDT_UInt32 - Harminckét bites elõjel nélküli egész </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3621"/>
        <source>GDT_Int32 - Thirty two bit signed integer </source>
        <translation>GDT_Int32 - Harminckét bites elõjeles egész </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3624"/>
        <source>GDT_Float32 - Thirty two bit floating point </source>
        <translation>GDT_Float32 - Harminkét bites lebegõpontos szám </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3627"/>
        <source>GDT_Float64 - Sixty four bit floating point </source>
        <translation>GDT_Float64 - Hatvannégy bites lebegõpontos szám </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3630"/>
        <source>GDT_CInt16 - Complex Int16 </source>
        <translation>GDT_CInt16 - Komplex Int16 </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3633"/>
        <source>GDT_CInt32 - Complex Int32 </source>
        <translation>GDT_CInt32 - Komplex Int32 </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3636"/>
        <source>GDT_CFloat32 - Complex Float32 </source>
        <translation>GDT_CFloat32 - Komplex Float32 </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3639"/>
        <source>GDT_CFloat64 - Complex Float64 </source>
        <translation>GDT_CFloat64 - Komplex Float64 </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3642"/>
        <source>Could not determine raster data type.</source>
        <translation>Nem tudom meghatározni a raszter adat típusát.</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3647"/>
        <source>Pyramid overviews:</source>
        <translation>Piramis áttekintõk:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3668"/>
        <source>Layer Spatial Reference System: </source>
        <translation>Réteg térbeli referencia rendszer: </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3696"/>
        <source>Origin:</source>
        <translation>Origó:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3705"/>
        <source>Pixel Size:</source>
        <translation>Pixel méret:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3722"/>
        <source>Property</source>
        <translation>Tulajdonság</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3725"/>
        <source>Value</source>
        <translation>Érték</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4843"/>
        <source>Band</source>
        <translation>Sáv</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3741"/>
        <source>Band No</source>
        <translation>Sáv szám</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3753"/>
        <source>No Stats</source>
        <translation>Nincs statisztika</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3756"/>
        <source>No stats collected yet</source>
        <translation>Nem készült még statisztika</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3766"/>
        <source>Min Val</source>
        <translation>Min érték</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3774"/>
        <source>Max Val</source>
        <translation>Max érték</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3782"/>
        <source>Range</source>
        <translation>Tartomány</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3790"/>
        <source>Mean</source>
        <translation>Átlag</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3798"/>
        <source>Sum of squares</source>
        <translation>Négyzetösszeg</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3806"/>
        <source>Standard Deviation</source>
        <translation>Szórás</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3814"/>
        <source>Sum of all cells</source>
        <translation>Összes cellá szám</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3822"/>
        <source>Cell Count</source>
        <translation>Cella szám</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3914"/>
        <source>Average Magphase</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3919"/>
        <source>Average</source>
        <translation>Átlag</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4799"/>
        <source>out of extent</source>
        <translation>tartományon kívûl</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4837"/>
        <source>null (no data)</source>
        <translation>null (nincs adat)</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.h" line="528"/>
        <source>Not Set</source>
        <translation>Nem beállított</translation>
    </message>
</context>
<context>
    <name>QgsRasterLayerProperties</name>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="996"/>
        <source>Grayscale</source>
        <translation>Szükefokozatok</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2400"/>
        <source>Pseudocolor</source>
        <translation>Álszínes</translation>
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
        <translation>Siszkrét</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="871"/>
        <source>Linearly</source>
        <translation>Lineárisan</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2612"/>
        <source>Equal interval</source>
        <translation>Egyenlõ intervallumok</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2634"/>
        <source>Quantiles</source>
        <translation type="unfinished">Egyenlõ számú</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="174"/>
        <source>Palette</source>
        <translation>Paletta</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="339"/>
        <source>Description</source>
        <translation>Leírás</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="340"/>
        <source>Large resolution raster layers can slow navigation in QGIS.</source>
        <translation>A nagyfelbontású raszter rétegek lassíthatják a navigációt a GGIS-ben.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="341"/>
        <source>By creating lower resolution copies of the data (pyramids) performance can be considerably improved as QGIS selects the most suitable resolution to use depending on the level of zoom.</source>
        <translation>Az adatok kisebb felbontású másolatainak (piramis) elkészítése sokat gyorsíthat, mivel a QGIS a nagyítás mértékétõl függõen a legmegfelõbb felbontást választja.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="342"/>
        <source>You must have write access in the directory where the original data is stored to build pyramids.</source>
        <translation>A piramis készítéshez írási joggal kell rendelkeznie a mappára, ahol az eredeti adatok találhatók.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="343"/>
        <source>Please note that building pyramids may alter the original data file and once created they cannot be removed!</source>
        <translation>Vedd figyelenbe, hogy a piramis készítés módosíthatja az eredeti adatokat és a létrehozás után nem állítható vissza!</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="344"/>
        <source>Please note that building pyramids could corrupt your image - always make a backup of your data first!</source>
        <translation>Vedd figyelembe, hogy a piramis készítés összekeverheti az adatokat - mindig készíts elõbb másolatot az adataidról!</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1810"/>
        <source>Red</source>
        <translation>Vörös</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1810"/>
        <source>Green</source>
        <translation>Zöld</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1810"/>
        <source>Blue</source>
        <translation>Kék</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1827"/>
        <source>Percent Transparent</source>
        <translation>Átlátszóság százalék</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1823"/>
        <source>Gray</source>
        <translation>Szürke</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1827"/>
        <source>Indexed Value</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1684"/>
        <source>User Defined</source>
        <translation>Felhasználó definiált</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="713"/>
        <source>No Scaling</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="789"/>
        <source>Columns: </source>
        <translation>Oszlopok: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="790"/>
        <source>Rows: </source>
        <translation>Sorok: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="791"/>
        <source>No-Data Value: </source>
        <translation>Nincs adat érték: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="783"/>
        <source>No-Data Value: Not Set</source>
        <translation>Nincs adat érték: nem beállított</translation>
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
        <translation>Sikertelen piramis készítés.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1550"/>
        <source>The file was not writeable. Some formats can not be written to, only read. You can also try to check the permissions and then try again.</source>
        <translation>A fájl nem írható. Néhany formátum nem írható csak olvasható. Ellenõrizd a jogokat és próbáld újra.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1555"/>
        <source>Building pyramid overviews is not supported on this type of raster.</source>
        <translation>A piramis készítés nem támogatot az ilyen típusú raszteren.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1795"/>
        <source>Save file</source>
        <translation>Fájl mentés</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2297"/>
        <source>Textfile (*.txt)</source>
        <translation>Szövegfájl (*.txt)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1807"/>
        <source>QGIS Generated Transparent Pixel Value Export File</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2297"/>
        <source>Open file</source>
        <translation>Fájl nyitás</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2370"/>
        <source>Import Error</source>
        <translation>Importálási hiba</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2370"/>
        <source>The following lines contained errors

</source>
        <translation>A következõ sorok hibát tartalmaznak

</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2375"/>
        <source>Read access denied</source>
        <translation>Olvasás visszautasítva</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2375"/>
        <source>Read access denied. Adjust the file permissions and try again.

</source>
        <translation>Olvasás visszautasítva. Módosítsd a fájl jogokat és próbáld újra.

</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2400"/>
        <source>Color Ramp</source>
        <translation>Szín skála</translation>
    </message>
</context>
<context>
    <name>QgsRasterLayerPropertiesBase</name>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="13"/>
        <source>Raster Layer Properties</source>
        <translation>Raszter réteg tulajdonságok</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="38"/>
        <source>Symbology</source>
        <translation>Megjelenés</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="50"/>
        <source>Grayscale Band Scaling</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="152"/>
        <source>Max</source>
        <translation>Max</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="332"/>
        <source>Std Deviation</source>
        <translation>Szórás</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="179"/>
        <source>Custom Min Max Values:</source>
        <translation>Egyéni Min Max értékek:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="207"/>
        <source>Min</source>
        <translation>Min</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="249"/>
        <source>Contrast Enhancement</source>
        <translation>Kontraszt növelés</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="291"/>
        <source>Load Min Max Values From Band(s)</source>
        <translation>Min Max értékek a sáv(ok)ból</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="313"/>
        <source>RGB Scaling</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="325"/>
        <source>&lt;b&gt;&lt;font color=&apos;blue&apos;&gt;Max&lt;/font&gt;&lt;/b&gt;</source>
        <translation>&lt;b&gt;&lt;font color=&apos;blue&apos;&gt;Max&lt;/font&gt;&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="339"/>
        <source>Custom Min Max Values</source>
        <translation>Egyéni Egyéni Min Max értékek</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="381"/>
        <source>&lt;b&gt;&lt;font color=&apos;green&apos;&gt;Min&lt;/font&gt;&lt;/b&gt;</source>
        <translation>&lt;b&gt;&lt;font color=&apos;green&apos;&gt;Min&lt;/font&gt;&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="396"/>
        <source>&lt;b&gt;&lt;font color=&apos;green&apos;&gt;Max&lt;/font&gt;&lt;/b&gt;</source>
        <translation>&lt;b&gt;&lt;font color=&apos;green&apos;&gt;Max&lt;/font&gt;&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="411"/>
        <source>&lt;b&gt;&lt;font color=&apos;red&apos;&gt;Min&lt;/font&gt;&lt;/b&gt;</source>
        <translation>&lt;b&gt;&lt;font color=&apos;red&apos;&gt;Min&lt;/font&gt;&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="466"/>
        <source>&lt;b&gt;&lt;font color=&apos;red&apos;&gt;Max&lt;/font&gt;&lt;/b&gt;</source>
        <translation>&lt;b&gt;&lt;font color=&apos;red&apos;&gt;Max&lt;/font&gt;&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="548"/>
        <source>&lt;b&gt;&lt;font color=&apos;blue&apos;&gt;Min&lt;/font&gt;&lt;/b&gt;</source>
        <translation>&lt;b&gt;&lt;font color=&apos;blue&apos;&gt;Min&lt;/font&gt;&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="651"/>
        <source>Grayscale Band Selection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="671"/>
        <source>Gray</source>
        <translation>Szürke</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="699"/>
        <source>RGB Mode Band Selection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="711"/>
        <source>&lt;b&gt;&lt;font color=&apos;blue&apos;&gt;Blue&lt;/font&gt;&lt;/b&gt;</source>
        <translation>&lt;b&gt;&lt;font color=&apos;blue&apos;&gt;Kék&lt;/font&gt;&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="721"/>
        <source>&lt;b&gt;&lt;font color=&apos;green&apos;&gt;Green&lt;/font&gt;&lt;/b&gt;</source>
        <translation>&lt;b&gt;&lt;font color=&apos;green&apos;&gt;Zöld&lt;/font&gt;&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="731"/>
        <source>&lt;b&gt;&lt;font color=&apos;red&apos;&gt;Red&lt;/font&gt;&lt;/b&gt;</source>
        <translation>&lt;b&gt;&lt;font color=&apos;red&apos;&gt;Vörös&lt;/font&gt;&lt;/b&gt;</translation>
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
        <translation>Globális átlátszóság</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="923"/>
        <source> 00%</source>
        <translation> 00%</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="930"/>
        <source>None</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="937"/>
        <source>&lt;p align=&quot;right&quot;&gt;Full&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;right&quot;&gt;Teljes&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="947"/>
        <source>Render as</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="972"/>
        <source>Single Band Gray</source>
        <translation>Egy sáv szürke</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="979"/>
        <source>Three Band Color</source>
        <translation>Három sáv színes</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="990"/>
        <source>Transparent Pixels</source>
        <translation>Átlátszó pixelek</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1036"/>
        <source>Transparent Band:</source>
        <translation>Átlátszó sáv:</translation>
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
        <translation>Manuális érték hozzáadás</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1184"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1116"/>
        <source>Add Values From Display</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1129"/>
        <source>Remove Selected Row</source>
        <translation>Szelektált sor törlése</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1142"/>
        <source>Default Values</source>
        <translation>Alapértelmezett értékek</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1168"/>
        <source>Import From File</source>
        <translation>Fájlból importálás</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1181"/>
        <source>Export To File</source>
        <translation>Fájlba exportálás</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1196"/>
        <source>No Data Value:</source>
        <translation>Nincs adat érték:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1206"/>
        <source>Reset No Data Value</source>
        <translation>Nincs adat érték visszaállítása</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1214"/>
        <source>Colormap</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1260"/>
        <source>Number of entries:</source>
        <translation>Elemek száma:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1294"/>
        <source>Delete entry</source>
        <translation>Elem törlés</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1301"/>
        <source>Classify</source>
        <translation>Osztályoz</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1317"/>
        <source>1</source>
        <translation>1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1322"/>
        <source>2</source>
        <translation>2</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1338"/>
        <source>Color interpolation:</source>
        <translation>Szín interpoláció:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1358"/>
        <source>Classification mode:</source>
        <translation>Osztályozási mód:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1371"/>
        <source>General</source>
        <translation>Általános</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1383"/>
        <source>Spatial Reference System</source>
        <translation>Térbeli referencia rendszer</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1395"/>
        <source>Change</source>
        <translation>Módosítás</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1412"/>
        <source>Scale Dependent Visibility</source>
        <translation>Méretarány függõ megjelenítés</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1427"/>
        <source>Maximum scale at which this layer will be displayed. </source>
        <translation>Maximális méretarány amikor a réteg megjelenik. </translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1440"/>
        <source>Maximum 1:</source>
        <translation>Maximum 1:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1450"/>
        <source>Minimum scale at which this layer will be displayed. </source>
        <translation>Minimális méretarány amikor a réteg megjelenik. </translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1463"/>
        <source>Minimum 1:</source>
        <translation>Minimum 1:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1476"/>
        <source>DebugInfo</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1491"/>
        <source>Columns:</source>
        <translation>Oszlopok: </translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1498"/>
        <source>Rows:</source>
        <translation>Sorok: </translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1505"/>
        <source>No Data:</source>
        <translation>Nincs adat:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1514"/>
        <source>Layer Source:</source>
        <translation>Réteg forrás:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1534"/>
        <source>Display Name:</source>
        <translation>Megjelenõ név:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1552"/>
        <source>Thumbnail</source>
        <translation>Miniatür</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1602"/>
        <source>Legend:</source>
        <translation>Jelkulcs:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1652"/>
        <source>Palette:</source>
        <translation>Paletta:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1705"/>
        <source>Metadata</source>
        <translation>Meta adat</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1721"/>
        <source>Pyramids</source>
        <translation>Piramisok</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1748"/>
        <source>Pyramid Resolutions</source>
        <translation>Piramis felbontás</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1800"/>
        <source>Resampling Method</source>
        <translation>Újramintavételezési módszer</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1811"/>
        <source>Average</source>
        <translation>Átlag</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1816"/>
        <source>Nearest Neighbour</source>
        <translation>Legközelebbi szomszéd</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1824"/>
        <source>Build Pyramids</source>
        <translation>Piramis készítés</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1834"/>
        <source>Histogram</source>
        <translation>Hisztogramm</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1881"/>
        <source>Chart Type</source>
        <translation>Grafikon típus</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1893"/>
        <source>Line Graph</source>
        <translation>Vonalas grafikon</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1903"/>
        <source>Bar Chart</source>
        <translation>Oszlop grafikon</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1913"/>
        <source>Refresh</source>
        <translation>Firssítés</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1920"/>
        <source>Options</source>
        <translation>Opciók</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1932"/>
        <source>Column Count:</source>
        <translation>Oszlop szám:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1939"/>
        <source>Out Of Range OK?</source>
        <translation>Tartományok kívûl OK?</translation>
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
        <translation></translation>
    </message>
    <message>
        <location filename="../src/core/qgsrunprocess.cpp" line="115"/>
        <source>Done</source>
        <translation>Kész</translation>
    </message>
    <message>
        <location filename="../src/core/qgsrunprocess.cpp" line="146"/>
        <source>Unable to run command</source>
        <translation>Nem tudom futtatni a parancsot</translation>
    </message>
</context>
<context>
    <name>QgsScaleBarPlugin</name>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="78"/>
        <source>Bottom Left</source>
        <translation>Bal alsó</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="79"/>
        <source>Top Left</source>
        <translation>Bal felső</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="79"/>
        <source>Top Right</source>
        <translation>Jobb felső</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="79"/>
        <source>Bottom Right</source>
        <translation>Jobb alsó</translation>
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
        <translation>&amp;Lépték</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="103"/>
        <source>Creates a scale bar that is displayed on the map canvas</source>
        <translation>A térképen megjelenõ lépték létrehozása</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="543"/>
        <source>&amp;Decorations</source>
        <translation>&amp;Dekorációk</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="164"/>
        <source> metres/km</source>
        <translation> méter/km</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="165"/>
        <source> feet/miles</source>
        <translation> láb/mérföld</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="288"/>
        <source> degrees</source>
        <translation> fok</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="243"/>
        <source> km</source>
        <translation> km</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="248"/>
        <source> mm</source>
        <translation> mm</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="253"/>
        <source> cm</source>
        <translation> cm</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="257"/>
        <source> m</source>
        <translation> m</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="262"/>
        <source> miles</source>
        <translation> mérföld</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="267"/>
        <source> mile</source>
        <translation> mérföld</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="272"/>
        <source> inches</source>
        <translation>  hüvelyk</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="277"/>
        <source> foot</source>
        <translation> láb</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="281"/>
        <source> feet</source>
        <translation> láb</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="286"/>
        <source> degree</source>
        <translation> fok</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="291"/>
        <source> unknown</source>
        <translation> ismeretlen</translation>
    </message>
</context>
<context>
    <name>QgsScaleBarPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="300"/>
        <source>Scale Bar Plugin</source>
        <translation>Lépték modul</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="76"/>
        <source>Click to select the colour</source>
        <translation>Kattints a szín beállításhoz</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="94"/>
        <source>Size of bar:</source>
        <translation>Oszlop méret:</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="109"/>
        <source>Automatically snap to round number on resize</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="127"/>
        <source>Colour of bar:</source>
        <translation>Oszlop szín:</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="143"/>
        <source>Top Left</source>
        <translation>Bal felső</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="148"/>
        <source>Top Right</source>
        <translation>Jobb felső</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="153"/>
        <source>Bottom Left</source>
        <translation>Bal alsó</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="158"/>
        <source>Bottom Right</source>
        <translation>Jobb alsó</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="174"/>
        <source>Enable scale bar</source>
        <translation>Lépték bekapcsolás</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="192"/>
        <source>Scale bar style:</source>
        <translation>Lépték stílus:</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="207"/>
        <source>Select the style of the scale bar</source>
        <translation>Lépték stílus kiválasztás</translation>
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
        <translation>Elhelyezés:</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="274"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;This plugin draws a scale bar on the map. Please note the size option below is a &apos;preferred&apos; size and may have to be altered by QGIS depending on the level of zoom.  The size is measured according to the map units specified in the project properties.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Ez a modul egy léptéket rajzol a térképre. Vedd figyelembe a megadott &apos;irányadó&apos; méretet a QGIS módosíthatja a méretarány függvényében. A méretet a projekt tulajdonságoknál megadott  térkép egységekben kel megadni.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
</context>
<context>
    <name>QgsSearchQueryBuilder</name>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="37"/>
        <source>Search query builder</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="169"/>
        <source>No matching features found.</source>
        <translation>Nem találtam megfelelő elemet.</translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="170"/>
        <source>Search results</source>
        <translation>Keresés eredmények</translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="179"/>
        <source>Search string parsing error</source>
        <translation>Keresési minta hibás</translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="235"/>
        <source>No Records</source>
        <translation>Nincsenek rekordok</translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="235"/>
        <source>The query you specified results in zero records being returned.</source>
        <translation>A megadott lekérdezés nulla rekorddal tért vissza.</translation>
    </message>
    <message numerus="yes">
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="167"/>
        <source>Found %d matching features.</source>
        <translation type="unfinished">
            <numerusform></numerusform>
        </translation>
    </message>
</context>
<context>
    <name>QgsServerSourceSelect</name>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="171"/>
        <source>Are you sure you want to remove the </source>
        <translation>Biztos, hogy törlöd a </translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="171"/>
        <source> connection and all associated settings?</source>
        <translation> kapcsolatot és minden hozzátartozó beállítást?</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="172"/>
        <source>Confirm Delete</source>
        <translation>Törlés megerősítése</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="454"/>
        <source>WMS Provider</source>
        <translation>WMS szolgáltató</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="456"/>
        <source>Could not open the WMS Provider</source>
        <translation>Nem tudom megnyitni a WMS szolgáltató</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="465"/>
        <source>Select Layer</source>
        <translation>Válassz réteget</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="465"/>
        <source>You must select at least one layer first.</source>
        <translation>Elõször legalább egy réteget kell választanod.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="469"/>
        <source>Coordinate Reference System</source>
        <translation>Referencia koordinátarendszer</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="469"/>
        <source>There are no available coordinate reference system for the set of layers you&apos;ve selected.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="767"/>
        <source>Could not understand the response.  The</source>
        <translation>Nem értem a választ.  A(z)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="768"/>
        <source>provider said</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="822"/>
        <source>WMS proxies</source>
        <translation>WMS proxik</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="822"/>
        <source>&lt;p&gt;Several WMS servers have been added to the server list. Note that the proxy fields have been left blank and if you access the internet via a web proxy, you will need to individually set the proxy fields with appropriate values.&lt;/p&gt;</source>
        <translation>&lt;p&gt;Több WMS szervert adtál a szerver listához. A proxy mezõket üresen hagytad, ha egy web proxyn keresztûl éred el az internetet a megfelõ értékekkel egyenként ki kell tölteni a proxy mezõt.&lt;/p&gt;</translation>
    </message>
    <message numerus="yes">
        <location filename="../src/app/qgsserversourceselect.cpp" line="588"/>
        <source>Coordinate Reference System (%1 available)</source>
        <translation type="unfinished">
            <numerusform></numerusform>
        </translation>
    </message>
</context>
<context>
    <name>QgsServerSourceSelectBase</name>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="13"/>
        <source>Add Layer(s) from a Server</source>
        <translation>Réteg hozzáadás egy szerverrõl</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="34"/>
        <source>Server Connections</source>
        <translation>Szerver kapcsolat</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="46"/>
        <source>Adds a few example WMS servers</source>
        <translation>Példa WMS szerverek hozzáadása</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="52"/>
        <source>Add default servers</source>
        <translation>Alapértelmezett szerverek hozzáadása</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="81"/>
        <source>C&amp;onnect</source>
        <translation>&amp;Kapcsolódás</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="91"/>
        <source>Edit</source>
        <translation>Szerkeszt</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="101"/>
        <source>Delete</source>
        <translation>Töröl</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="108"/>
        <source>&amp;New</source>
        <translation>&amp;Új</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="118"/>
        <source>Coordinate Reference System</source>
        <translation>Referencia koordinátarendszer</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="156"/>
        <source>Change ...</source>
        <translation>Módosít...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="174"/>
        <source>Ready</source>
        <translation>Kész</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="187"/>
        <source>&amp;Add</source>
        <translation>&amp;Hozzáad</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="190"/>
        <source>Alt+A</source>
        <translation>Alt+H</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="203"/>
        <source>Layers</source>
        <translation>Rétegek</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="236"/>
        <source>ID</source>
        <translation>ID</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="241"/>
        <source>Name</source>
        <translation>Név</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="246"/>
        <source>Title</source>
        <translation>Cím</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="251"/>
        <source>Abstract</source>
        <translation>Összefoglalás</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="276"/>
        <source>Image encoding</source>
        <translation>Kép kódolás</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="302"/>
        <source>Help</source>
        <translation>Súgó</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="305"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="315"/>
        <source>C&amp;lose</source>
        <translation>&amp;Lezár</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="318"/>
        <source>Alt+L</source>
        <translation>Alt+L</translation>
    </message>
</context>
<context>
    <name>QgsShapeFile</name>
    <message>
        <location filename="../src/plugins/spit/qgsshapefile.cpp" line="421"/>
        <source>The database gave an error while executing this SQL:</source>
        <translation>Az adatbázis hibát jelzett az SQL végrehajtása során:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsshapefile.cpp" line="429"/>
        <source>The error was:</source>
        <translation>A hiba:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsshapefile.cpp" line="426"/>
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
        <translation>Folytonos vonal</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="122"/>
        <source>Dash Line</source>
        <translation>Szaggatott vonal</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="123"/>
        <source>Dot Line</source>
        <translation>Pontvonal</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="124"/>
        <source>Dash Dot Line</source>
        <translation>Eredmény vonal</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="125"/>
        <source>Dash Dot Dot Line</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="126"/>
        <source>No Pen</source>
        <translation>Nincs toll</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="131"/>
        <source>Solid Pattern</source>
        <translation>Szolid kitöltés</translation>
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
        <translation>Nincs ecset</translation>
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
        <translation>Egy szimbólum</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="41"/>
        <source>Point Symbol</source>
        <translation>Pont szimbólum</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="69"/>
        <source>Size</source>
        <translation>Méret</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="79"/>
        <source>Area scale field</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="89"/>
        <source>Rotation field</source>
        <translation>Forgatás mezõ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="158"/>
        <source>Style Options</source>
        <translation>Stílus beállítások</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="170"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="187"/>
        <source>Outline style</source>
        <translation>Körvonal stílus</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="197"/>
        <source>Outline color</source>
        <translation>Körvonal szín</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="207"/>
        <source>Outline width</source>
        <translation>Körvonal vastagság</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="217"/>
        <source>Fill color</source>
        <translation>Kitöltés szín</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="227"/>
        <source>Fill style</source>
        <translation>Kitöltés stílus</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="347"/>
        <source>Label</source>
        <translation type="unfinished">Cimke</translation>
    </message>
</context>
<context>
    <name>QgsSnappingDialog</name>
    <message>
        <location filename="../src/app/qgssnappingdialog.cpp" line="147"/>
        <source>to vertex</source>
        <translation type="unfinished">törésponthoz</translation>
    </message>
    <message>
        <location filename="../src/app/qgssnappingdialog.cpp" line="151"/>
        <source>to segment</source>
        <translation>szakaszhoz</translation>
    </message>
    <message>
        <location filename="../src/app/qgssnappingdialog.cpp" line="89"/>
        <source>to vertex and segment</source>
        <translation>törésponthoz és szakaszhoz</translation>
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
        <translation>Réteg</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssnappingdialogbase.ui" line="31"/>
        <source>Mode</source>
        <translation>Mód</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssnappingdialogbase.ui" line="36"/>
        <source>Tolerance</source>
        <translation>Tolerancia</translation>
    </message>
</context>
<context>
    <name>QgsSpit</name>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="69"/>
        <source>File Name</source>
        <translation>Fájlnév</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="69"/>
        <source>Feature Class</source>
        <translation>Elemosztály</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="70"/>
        <source>Features</source>
        <translation>Elem</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="70"/>
        <source>DB Relation Name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="70"/>
        <source>Schema</source>
        <translation>Séma</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="122"/>
        <source>New Connection</source>
        <translation>Új kapcsolat</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="147"/>
        <source>Are you sure you want to remove the [</source>
        <translation>Biztos, goy törlöd a [</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="147"/>
        <source>] connection and all associated settings?</source>
        <translation>] kapcsolatot és minden hozzátartozó beállítást?</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="148"/>
        <source>Confirm Delete</source>
        <translation>Törlés megerősítése</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="172"/>
        <source>Add Shapefiles</source>
        <translation>Shape fájl hozzáadás</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="174"/>
        <source>Shapefiles (*.shp);;All files (*.*)</source>
        <translation>Shape fájlok (*.shp);;Minden fájl (*.*)</translation>
    </message>
    <message>
        <location filename="" line="145"/>
        <source> - Edit Column Names</source>
        <translation type="obsolete"> - oszlopnevek szerkesztése</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="268"/>
        <source>The following Shapefile(s) could not be loaded:

</source>
        <translation>A következõ shape fájlt nem tudom betölteni:

</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="272"/>
        <source>REASON: File cannot be opened</source>
        <translation>OK: A fájlt nem tudom megnyitni</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="277"/>
        <source>REASON: One or both of the Shapefile files (*.dbf, *.shx) missing</source>
        <translation>OK: Az egyik vagy mindkettõ shape fálj hiányzik (*.dbf. *.shx)</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="354"/>
        <source>General Interface Help:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="356"/>
        <source>PostgreSQL Connections:</source>
        <translation>PostgreSQL kapcsolatok:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="358"/>
        <source>[New ...] - create a new connection</source>
        <translation>[Új ...] - új kapcsolat létrehozás</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="359"/>
        <source>[Edit ...] - edit the currently selected connection</source>
        <translation>[Szerkeszt ...] - a kiválasztott kapcsolat szerkesztése</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="360"/>
        <source>[Remove] - remove the currently selected connection</source>
        <translation>[Töröl] - a kiválasztott kapcsolat törlése</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="361"/>
        <source>-you need to select a connection that works (connects properly) in order to import files</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="362"/>
        <source>-when changing connections Global Schema also changes accordingly</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="363"/>
        <source>Shapefile List:</source>
        <translation>Shape fájl lista:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="365"/>
        <source>[Add ...] - open a File dialog and browse to the desired file(s) to import</source>
        <translation>[Hozzáad ...] - a fájl párbeszédablak megnyitás és tallózás az importálandó fájlhoz</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="366"/>
        <source>[Remove] - remove the currently selected file(s) from the list</source>
        <translation>[Törlés] - a kiválasztott fájl(ok) törlése a listából</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="367"/>
        <source>[Remove All] - remove all the files in the list</source>
        <translation>[Mindent töröl] - Minden fájl törlése a listából</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="368"/>
        <source>[SRID] - Reference ID for the shapefiles to be imported</source>
        <translation>[SRID] - Referencia ID az importálandó shape fájlhoz</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="369"/>
        <source>[Use Default (SRID)] - set SRID to -1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="370"/>
        <source>[Geometry Column Name] - name of the geometry column in the database</source>
        <translation>[Geometria oszlopnév] - a geometria oszlop neve az adatbázisban</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="371"/>
        <source>[Use Default (Geometry Column Name)] - set column name to &apos;the_geom&apos;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="372"/>
        <source>[Glogal Schema] - set the schema for all files to be imported into</source>
        <translation type="unfinished">[Globális séma]</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="374"/>
        <source>[Import] - import the current shapefiles in the list</source>
        <translation>[Import] - a listában kiválaszott shape fájlok importálása</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="375"/>
        <source>[Quit] - quit the program
</source>
        <translation>[Kilép] - Kilépés a programból
</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="376"/>
        <source>[Help] - display this help dialog</source>
        <translation>[Súgó] - Ez a súgó párbeszédablak megjelenítse</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="830"/>
        <source>Import Shapefiles</source>
        <translation>Shape fájl importálás</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="390"/>
        <source>You need to specify a Connection first</source>
        <translation>Elõször a kapcsolatot kell megadnod</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="406"/>
        <source>Connection failed - Check settings and try again</source>
        <translation>Sikertelen kapcsolódás - Ellenõrizd a beállításokat és próbáld újra</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="440"/>
        <source>PostGIS not available</source>
        <translation>PostGIS nem érhetõ el</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="442"/>
        <source>&lt;p&gt;The chosen database does not have PostGIS installed, but this is required for storage of spatial data.&lt;/p&gt;</source>
        <translation>&lt;p&gt;Nincs telepítrve a PostGIS a kiválasztott adatbázisban, pedig ez szükséges a térbeli adatok tárolásához.&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="514"/>
        <source>You need to add shapefiles to the list first</source>
        <translation>Elõször egy shape fájlt kell adnod a listához</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="581"/>
        <source>Importing files</source>
        <translation>Fájl importálás</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="519"/>
        <source>Cancel</source>
        <translation>Mégsem</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="523"/>
        <source>Progress</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="532"/>
        <source>Problem inserting features from file:</source>
        <translation>Probléma az elemek beszúrása közben a fájlból:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="539"/>
        <source>Invalid table name.</source>
        <translation>Hibás táblanév.</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="549"/>
        <source>No fields detected.</source>
        <translation>Nem találok mezõket.</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="565"/>
        <source>Checking to see if </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="574"/>
        <source>The following fields are duplicates:</source>
        <translation>A következõ mezõk duplikáltak:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="814"/>
        <source>&lt;p&gt;Error while executing the SQL:&lt;/p&gt;&lt;p&gt;</source>
        <translation>&lt;p&gt;Hiba az SQL végrehajtása közben:&lt;/p&gt;&lt;p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="815"/>
        <source>&lt;/p&gt;&lt;p&gt;The database said:</source>
        <translation>&lt;/p&gt;&lt;p&gt;Az adatbázis üzenete:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="673"/>
        <source>Import Shapefiles - Relation Exists</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="674"/>
        <source>The Shapefile:</source>
        <translation>A shape fájl:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="675"/>
        <source>will use [</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="675"/>
        <source>] relation for its data,</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="675"/>
        <source>which already exists and possibly contains data.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="676"/>
        <source>To avoid data loss change the &quot;DB Relation Name&quot;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="676"/>
        <source>for this Shapefile in the main dialog file list.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="677"/>
        <source>Do you want to overwrite the [</source>
        <translation>Fölül akarod írni a [</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="677"/>
        <source>] relation?</source>
        <translation>] relációt?</translation>
    </message>
    <message>
        <location filename="" line="145"/>
        <source>Use the table below to edit column names. Make sure that none of the columns are named using a PostgreSQL reserved word</source>
        <translation type="obsolete">Használd az alábbi táblát az oszlopnevek szerkesztéséhez. Gyõzõdj meg róla egyik oszlopnév sem egy PostgreSQL fenntartott név</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="830"/>
        <source>%1 of %2 shapefiles could not be imported.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsSpitBase</name>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="19"/>
        <source>SPIT - Shapefile to PostGIS Import Tool</source>
        <translation>SPIT - Shape fájl importáló eszköz PostGIS-be</translation>
    </message>
    <message>
        <location filename="" line="145"/>
        <source>Shapefile to PostGIS Import Tool</source>
        <translation type="obsolete">Shape fájl importáló eszköz PostGIS-be</translation>
    </message>
    <message>
        <location filename="" line="145"/>
        <source>Shapefile List</source>
        <translation type="obsolete">Shape fájl lista</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="159"/>
        <source>Add a shapefile to the list of files to be imported</source>
        <translation>Add az importálandó shape fájlokat a listához </translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="162"/>
        <source>Add</source>
        <translation>Hozzáad</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="172"/>
        <source>Remove the selected shapefile from the import list</source>
        <translation>A kiválasztott shape fájl törlése a listából</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="175"/>
        <source>Remove</source>
        <translation>Törlés</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="185"/>
        <source>Remove all the shapefiles from the import list</source>
        <translation>Minden shape fájl törlése az import listából</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="188"/>
        <source>Remove All</source>
        <translation>Mindet töröl</translation>
    </message>
    <message>
        <location filename="" line="145"/>
        <source>SRID</source>
        <translation type="obsolete">SRID</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="211"/>
        <source>Set the SRID to the default value</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="224"/>
        <source>Set the geometry column name to the default value</source>
        <translation>A geometria oszlopnév beállítása az alapértelmezettre</translation>
    </message>
    <message>
        <location filename="" line="145"/>
        <source>Use Default Geometry Column Name</source>
        <translation type="obsolete">Alapértelmezett geometria oszlopnév használata</translation>
    </message>
    <message>
        <location filename="" line="145"/>
        <source>Geometry Column Name</source>
        <translation type="obsolete">Geometria oszlopnév</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="260"/>
        <source>Global Schema</source>
        <translation>Globális séma</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="54"/>
        <source>PostgreSQL Connections</source>
        <translation>PostgraSQL kapcsolat</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="118"/>
        <source>Create a new PostGIS connection</source>
        <translation>Új PostGIS kapcsolat létrehozás</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="121"/>
        <source>New</source>
        <translation>Új</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="105"/>
        <source>Remove the current PostGIS connection</source>
        <translation>Aktuális PostGIS kapcsolat törlése</translation>
    </message>
    <message>
        <location filename="" line="145"/>
        <source>Connect</source>
        <translation type="obsolete">Kapcsolódás</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="92"/>
        <source>Edit the current PostGIS connection</source>
        <translation>Aktuális PostGIS kapcsolat szerkesztése</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="95"/>
        <source>Edit</source>
        <translation>Szerkeszt</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="131"/>
        <source>Import options and shapefile list</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="214"/>
        <source>Use Default SRID or specify here</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="227"/>
        <source>Use Default Geometry Column Name or specify here</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="250"/>
        <source>Primary Key Column Name</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsSpitPlugin</name>
    <message>
        <location filename="../src/plugins/spit/qgsspitplugin.cpp" line="68"/>
        <source>&amp;Import Shapefiles to PostgreSQL</source>
        <translation>Shape fájl &amp;importálás PostgreSQL-be</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitplugin.cpp" line="70"/>
        <source>Import shapefiles into a PostGIS-enabled PostgreSQL database. The schema and field names can be customized on import</source>
        <translation>Shape fájl importálás PostGIS-képes PostgreSQL adatbázisba. A séma és a mezõnevek módosíthatóak az importálás során</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitplugin.cpp" line="93"/>
        <source>&amp;Spit</source>
        <translation>&amp;Spit</translation>
    </message>
</context>
<context>
    <name>QgsUniqueValueDialogBase</name>
    <message>
        <location filename="../src/ui/qgsuniquevaluedialogbase.ui" line="13"/>
        <source>Form1</source>
        <translation>Form1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsuniquevaluedialogbase.ui" line="37"/>
        <source>Classification Field:</source>
        <translation>Osztályozás mezõ:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsuniquevaluedialogbase.ui" line="47"/>
        <source>Delete class</source>
        <translation>Osztály törlés</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsuniquevaluedialogbase.ui" line="80"/>
        <source>Classify</source>
        <translation>Osztályoz</translation>
    </message>
</context>
<context>
    <name>QgsVectorLayer</name>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2446"/>
        <source>Could not commit the added features.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2537"/>
        <source>No other types of changes will be committed at this time.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2468"/>
        <source>Could not commit the changed attributes.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2527"/>
        <source>However, the added features were committed OK.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2494"/>
        <source>Could not commit the changed geometries.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2531"/>
        <source>However, the changed attributes were committed OK.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2524"/>
        <source>Could not commit the deleted features.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2535"/>
        <source>However, the changed geometries were committed OK.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsVectorLayerProperties</name>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="107"/>
        <source>Transparency: </source>
        <translation>Átlátszóság: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="198"/>
        <source>Single Symbol</source>
        <translation>Egy szimbólum</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="201"/>
        <source>Graduated Symbol</source>
        <translation>Növekvő szimbólumok</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="202"/>
        <source>Continuous Color</source>
        <translation>Folytonos szín</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="203"/>
        <source>Unique Value</source>
        <translation>Egyedi érték</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="158"/>
        <source>This button opens the PostgreSQL query builder and allows you to create a subset of features to display on the map canvas rather than displaying all features in the layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="161"/>
        <source>The query used to limit the features in the layer is shown here. This is currently only supported for PostgreSQL layers. To enter or modify the query, click on the Query Builder button</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="385"/>
        <source>Spatial Index</source>
        <translation>Térbeli index</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="380"/>
        <source>Creation of spatial index successfull</source>
        <translation>A térbeli index létrehozása sikerült</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="385"/>
        <source>Creation of spatial index failed</source>
        <translation>A térbeli index létrehozása nem sikerült</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="398"/>
        <source>General:</source>
        <translation>Általános:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="406"/>
        <source>Layer comment: </source>
        <translation>Réteg megjegyzés: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="413"/>
        <source>Storage type of this layer : </source>
        <translation>Réteg tárolás típus: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="419"/>
        <source>Source for this layer : </source>
        <translation>A réteg forrása: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="436"/>
        <source>Geometry type of the features in this layer : </source>
        <translation>Elemek geometria típusa a rétegben: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="444"/>
        <source>The number of features in this layer : </source>
        <translation>Elemek száma a rétegben: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="449"/>
        <source>Editing capabilities of this layer : </source>
        <translation>Réteg szerkesztési lehetõségek: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="456"/>
        <source>Extents:</source>
        <translation>Terjedelem:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="461"/>
        <source>In layer spatial reference system units : </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="462"/>
        <source>xMin,yMin </source>
        <translation>xMin,yMin </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="466"/>
        <source> : xMax,yMax </source>
        <translation> : xMax,yMax </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="495"/>
        <source>Layer Spatial Reference System:</source>
        <translation>Réteg térbeli referencia rendszer: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="521"/>
        <source>In project spatial reference system units : </source>
        <translation>A projekt térbeli referencia rendszer egységeiben: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="532"/>
        <source>Attribute field info:</source>
        <translation>Attribútum mezõ info:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="539"/>
        <source>Field</source>
        <translation>Mezõ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="542"/>
        <source>Type</source>
        <translation>Típus</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="545"/>
        <source>Length</source>
        <translation>Hossz</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="548"/>
        <source>Precision</source>
        <translation>Pontosság</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="551"/>
        <source>Comment</source>
        <translation>Megjegyzés</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="749"/>
        <source>Default Style</source>
        <translation>Alapértelmezett stílus</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="734"/>
        <source>QGIS Layer Style File (*.qml)</source>
        <translation>QGIS réteg stílus fájl (*.qml)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="755"/>
        <source>QGIS</source>
        <translation>QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="756"/>
        <source>Unknown style format: </source>
        <translation>Ismeretlen stílus formátum: </translation>
    </message>
</context>
<context>
    <name>QgsVectorLayerPropertiesBase</name>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="19"/>
        <source>Layer Properties</source>
        <translation>Réteg tulajdonságok</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="44"/>
        <source>Symbology</source>
        <translation>Megjelenés</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="69"/>
        <source>Transparency:</source>
        <translation>Átlátszóság:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="85"/>
        <source>Legend type:</source>
        <translation>Jelkulcs típus:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="129"/>
        <source>General</source>
        <translation>Általános</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="141"/>
        <source>Display name</source>
        <translation>Megjelenõ név</translation>
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
        <translation>Megjelenõ mezõ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="167"/>
        <source>Use this control to set which field is placed at the top level of the Identify Results dialog box.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="177"/>
        <source>Use scale dependent rendering</source>
        <translation>Méretarány függõ rajzolás használata</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="192"/>
        <source>Maximum 1:</source>
        <translation>Maximum 1:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="202"/>
        <source>Minimum 1:</source>
        <translation>Minimum 1:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="212"/>
        <source>Minimum scale at which this layer will be displayed. </source>
        <translation>Minimális méretarány amikor a réteg megjelenik. </translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="225"/>
        <source>Maximum scale at which this layer will be displayed. </source>
        <translation>Maximális méretarány amikor a réteg megjelenik. </translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="241"/>
        <source>Spatial Index</source>
        <translation>Térbeli index</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="253"/>
        <source>Create Spatial Index</source>
        <translation>Térbeli index létrehozás</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="263"/>
        <source>Create</source>
        <translation>Létrehoz</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="289"/>
        <source>Spatial Reference System</source>
        <translation>Térbeli referencia rendszer</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="308"/>
        <source>Change</source>
        <translation>Módosítás</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="318"/>
        <source>Subset</source>
        <translation>Részhalmaz</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="359"/>
        <source>Query Builder</source>
        <translation>Lekérdezés készítés</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="370"/>
        <source>Metadata</source>
        <translation>Meta adat</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="390"/>
        <source>Labels</source>
        <translation>Cimkék</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="410"/>
        <source>Display labels</source>
        <translation>Cimke megjelenítés</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="436"/>
        <source>Actions</source>
        <translation>Műveletek</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="478"/>
        <source>Restore Default Style</source>
        <translation>Alapértelmezett stílus visszaállítása</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="485"/>
        <source>Save As Default</source>
        <translation>Mentés alapértelmezettként</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="492"/>
        <source>Load Style ...</source>
        <translation>Stílus betöltés ...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="499"/>
        <source>Save Style ...</source>
        <translation>Stílus mentés ...</translation>
    </message>
</context>
<context>
    <name>QgsVectorSymbologyWidgetBase</name>
    <message>
        <location filename="../src/ui/qgsvectorsymbologywidgetbase.ui" line="16"/>
        <source>Form2</source>
        <translation>Form2</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorsymbologywidgetbase.ui" line="44"/>
        <source>Label</source>
        <translation>Cimke</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorsymbologywidgetbase.ui" line="49"/>
        <source>Min</source>
        <translation>Min</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorsymbologywidgetbase.ui" line="54"/>
        <source>Max</source>
        <translation>Max</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorsymbologywidgetbase.ui" line="62"/>
        <source>Symbol Classes:</source>
        <translation>Szimbólum osztályok:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorsymbologywidgetbase.ui" line="77"/>
        <source>Count:</source>
        <translation>Darabszám:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorsymbologywidgetbase.ui" line="90"/>
        <source>Mode:</source>
        <translation>Mód:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorsymbologywidgetbase.ui" line="100"/>
        <source>Field:</source>
        <translation>Mezõ:</translation>
    </message>
</context>
<context>
    <name>QgsWFSPlugin</name>
    <message>
        <location filename="../src/plugins/wfs/qgswfsplugin.cpp" line="58"/>
        <source>&amp;Add WFS layer</source>
        <translation>&amp;WSF réteg hozzáadás</translation>
    </message>
</context>
<context>
    <name>QgsWFSProvider</name>
    <message>
        <location filename="../src/providers/wfs/qgswfsprovider.cpp" line="1390"/>
        <source>unknown</source>
        <translation>ismeretlen</translation>
    </message>
    <message>
        <location filename="../src/providers/wfs/qgswfsprovider.cpp" line="1396"/>
        <source>received %1 bytes from %2</source>
        <translation>%1 bájtot kaptam a %2-ből</translation>
    </message>
</context>
<context>
    <name>QgsWFSSourceSelect</name>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselect.cpp" line="259"/>
        <source>Are you sure you want to remove the </source>
        <translation>Biztos, hogy törlöd</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselect.cpp" line="259"/>
        <source> connection and all associated settings?</source>
        <translation>kapcsolatot és minden hozzátartozó beállítást?</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselect.cpp" line="260"/>
        <source>Confirm Delete</source>
        <translation>Törlés megerősítése</translation>
    </message>
</context>
<context>
    <name>QgsWFSSourceSelectBase</name>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="13"/>
        <source>Add WFS Layer from a Server</source>
        <translation>Új WSF réteg egy szerverről</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="29"/>
        <source>Title</source>
        <translation>Cím</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="34"/>
        <source>Name</source>
        <translation>Név</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="39"/>
        <source>Abstract</source>
        <translation>Összefoglalás</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="47"/>
        <source>Coordinate Reference System</source>
        <translation>Koordináta-rendszer</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="85"/>
        <source>Change ...</source>
        <translation>Módosít...</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="95"/>
        <source>Server Connections</source>
        <translation>Szerver kapcsolatok</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="107"/>
        <source>&amp;New</source>
        <translation>&amp;Új</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="117"/>
        <source>Delete</source>
        <translation>Töröl</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="127"/>
        <source>Edit</source>
        <translation>Szerkeszt</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="153"/>
        <source>C&amp;onnect</source>
        <translation>&amp;Kapcsolódás</translation>
    </message>
</context>
<context>
    <name>QgsWmsProvider</name>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="718"/>
        <source>Tried URL: </source>
        <translation>Próbált URL: </translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="698"/>
        <source>HTTP Exception</source>
        <translation>HTTP hiba</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="661"/>
        <source>WMS Service Exception</source>
        <translation>WMS szolgáltatás hiba</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1512"/>
        <source>DOM Exception</source>
        <translation>DOM hiba</translation>
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
        <translation>Szerver tulajdonságok:</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1937"/>
        <source>Property</source>
        <translation>Tulajdonság</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1940"/>
        <source>Value</source>
        <translation>Érték</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1823"/>
        <source>WMS Version</source>
        <translation>WMS verzió</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2069"/>
        <source>Title</source>
        <translation>Cím</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2077"/>
        <source>Abstract</source>
        <translation>Összefoglalás</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1847"/>
        <source>Keywords</source>
        <translation>Kulcsszavak</translation>
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
        <translation>Díjak</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1883"/>
        <source>Access Constraints</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1891"/>
        <source>Image Formats</source>
        <translation>Kép formátumok</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1899"/>
        <source>Identify Formats</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1907"/>
        <source>Layer Count</source>
        <translation>Réteg szám</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1927"/>
        <source>Layer Properties: </source>
        <translation>Réteg tulajdonságok: </translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1945"/>
        <source>Selected</source>
        <translation>Szelektált</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2002"/>
        <source>Yes</source>
        <translation>Igen</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2002"/>
        <source>No</source>
        <translation>Nem</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1954"/>
        <source>Visibility</source>
        <translation>Láthatóság</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1960"/>
        <source>Visible</source>
        <translation>Látható</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1961"/>
        <source>Hidden</source>
        <translation>Nem látható</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1962"/>
        <source>n/a</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1983"/>
        <source>Can Identify</source>
        <translation>Azonosítható</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1991"/>
        <source>Can be Transparent</source>
        <translation>Lehet átlátszó</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1999"/>
        <source>Can Zoom In</source>
        <translation>Nagyítható</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2007"/>
        <source>Cascade Count</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2015"/>
        <source>Fixed Width</source>
        <translation>Rögzített szélesség</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2023"/>
        <source>Fixed Height</source>
        <translation>Rögzített magasság</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2031"/>
        <source>WGS 84 Bounding Box</source>
        <translation>WGS 84 befoglaló téglalap</translation>
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
        <translation>Név</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2162"/>
        <source>Layer cannot be queried.</source>
        <translation>A réteg nem kérdezhetõ le.</translation>
    </message>
</context>
<context>
    <name>QuickPrintGui</name>
    <message>
        <location filename="../src/plugins/quick_print/quickprintgui.cpp" line="125"/>
        <source>Portable Document Format (*.pdf)</source>
        <translation>Portable Document Format (*.pdf)</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintgui.cpp" line="150"/>
        <source>quickprint</source>
        <translation>Gyors nyomtatás</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintgui.cpp" line="151"/>
        <source>Unknown format: </source>
        <translation>Ismeretlen formátum: </translation>
    </message>
</context>
<context>
    <name>QuickPrintGuiBase</name>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="13"/>
        <source>QGIS Quick Print Plugin</source>
        <translation>QGIS Gyors nyomtatás modul</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="158"/>
        <source>Quick Print</source>
        <translation>Gyors nyomtatás</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="129"/>
        <source>Map Title e.g. ACME inc.</source>
        <translation>Térkép cím pl. Seholsincs Kft.</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="116"/>
        <source>Map Name e.g. Water Features</source>
        <translation>Térkép név pl. Vízrajz</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="103"/>
        <source>Copyright</source>
        <translation>Szerzõi jog</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="48"/>
        <source>Output</source>
        <translation>Eredmény</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="60"/>
        <source>Use last filename but incremented.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="67"/>
        <source>last used filename but incremented will be shown here</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="77"/>
        <source>Prompt for file name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="38"/>
        <source>Note: If you want more control over the map layout please use the map composer function in QGIS.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="93"/>
        <source>Page Size</source>
        <translation>Oldalméret</translation>
    </message>
</context>
<context>
    <name>QuickPrintPlugin</name>
    <message>
        <location filename="../src/plugins/quick_print/quickprintplugin.cpp" line="75"/>
        <source>Quick Print</source>
        <translation>Gyors nyomtatás</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintplugin.cpp" line="77"/>
        <source>Replace this with a short description of the what the plugin does</source>
        <translation>Cseréld le ezt a modul rövid leírásával</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintplugin.cpp" line="82"/>
        <source>&amp;Quick Print</source>
        <translation>&amp;Gyors nyomtatás</translation>
    </message>
</context>
<context>
    <name>RepositoryDetailsDialog</name>
    <message>
        <location filename="../python/plugins/plugin_installer/repository.ui" line="13"/>
        <source>Repository details</source>
        <translation>Tárház részletek</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/repository.ui" line="19"/>
        <source>Name:</source>
        <translation>Név:</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/repository.ui" line="29"/>
        <source>URL:</source>
        <translation>URL:</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/repository.ui" line="36"/>
        <source>http://</source>
        <translation>http://</translation>
    </message>
</context>
<context>
    <name>[pluginname]GuiBase</name>
    <message>
        <location filename="../src/plugins/plugin_template/pluginguibase.ui" line="13"/>
        <source>QGIS Plugin Template</source>
        <translation>QGIS modul sablon</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/pluginguibase.ui" line="47"/>
        <source>Plugin Template</source>
        <translation>Modul sablon</translation>
    </message>
</context>
<context>
    <name>pluginname</name>
    <message>
        <location filename="../src/plugins/plugin_template/plugin.cpp" line="73"/>
        <source>[menuitemname]</source>
        <translation>[menuitemname]</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugin.cpp" line="75"/>
        <source>Replace this with a short description of the what the plugin does</source>
        <translation>Cseréld le ezt a modul rövid leírásával</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugin.cpp" line="80"/>
        <source>&amp;[menuname]</source>
        <translation>&amp;[nemuname]</translation>
    </message>
</context>
</TS>
