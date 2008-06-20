<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS><TS version="1.1">
<context>
    <name>Dialog</name>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="13"/>
        <source>QGIS Plugin Installer</source>
        <translation>QGIS spraudņu instalators</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="142"/>
        <source>Name of plugin to install</source>
        <translation>Instalējamā spraudņa nosaukums</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="61"/>
        <source>Get List</source>
        <translation>Saņemt sarakstu</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="176"/>
        <source>Done</source>
        <translation>Darīts</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="152"/>
        <source>Install Plugin</source>
        <translation>Instalēt spraudni</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="163"/>
        <source>The plugin will be installed to ~/.qgis/python/plugins</source>
        <translation>Spraudnis tiks instalēts iekš ~/.qgis/python/plugins</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="117"/>
        <source>Name</source>
        <translation>Nosaukums</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="122"/>
        <source>Version</source>
        <translation>Versija</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="127"/>
        <source>Description</source>
        <translation>Apraksts</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="132"/>
        <source>Author</source>
        <translation>Autors</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="19"/>
        <source>Select repository, retrieve the list of available plugins, select one and install it</source>
        <translation>Izvēlieties repozitoriju, saņemiet pieejamo spraudņu sarakstu, izvēlēties vēlamo un instalējiet to</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="26"/>
        <source>Repository</source>
        <translation>Repozitorijs</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="34"/>
        <source>Active repository:</source>
        <translation>Aktīvais repozitorijs:</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="81"/>
        <source>Add</source>
        <translation>Pievienot</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="88"/>
        <source>Edit</source>
        <translation>Rediģēt</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="95"/>
        <source>Delete</source>
        <translation>Dzēst</translation>
    </message>
</context>
<context>
    <name>Gui</name>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="55"/>
        <source>Welcome to your automatically generated plugin!</source>
        <translation>Laipni lūgti automātiski ģenerētajā spraudnī!</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="56"/>
        <source>This is just a starting point. You now need to modify the code to make it do something useful....read on for a more information to get yourself started.</source>
        <translation>Šis ir tikai sākuma punkts. Jums vēl ir nepieciešams modificēt kodu, lai tas darīt ko noderīgu... Turpiniet lasīt, lai uzzinātu ko darīt tālāk.</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="57"/>
        <source>Documentation:</source>
        <translation>Dokumentācija:</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="58"/>
        <source>You really need to read the QGIS API Documentation now at:</source>
        <translation>Jums patiešām ir nepieciešams izlasīt QGIS API dokumentāciju iekš:</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="59"/>
        <source>In particular look at the following classes:</source>
        <translation>Īpaši pievērsiet uzmanību sekojošām klasēm:</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="62"/>
        <source>QgsPlugin is an ABC that defines required behaviour your plugin must provide. See below for more details.</source>
        <translation>QgsPlugin ir spraudņu ābece. Tas definē jūsu spraudņa izturēšanos. Vairāk informācijas skatiet zemāk.</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="63"/>
        <source>What are all the files in my generated plugin directory for?</source>
        <translation>Priekškam manā spraudņa mapē tika saģenerēti tik daudzi faili?</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="64"/>
        <source>This is the generated CMake file that builds the plugin. You should add you application specific dependencies and source files to this file.</source>
        <translation>Šis ir automātiski ģenerēts CMake fails spraudņa kompilēšanai. Jums vajadzētu pievienot jūsu aplikācijas specifiskās atkarības un avota failus šim failam.</translation>
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
        <translation>Šis fails satur dokumentāciju, ko jūs šobrīd lasiet.</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="71"/>
        <source>Getting developer help:</source>
        <translation>No izstrādātājiem palīdzību var saņemt:</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="72"/>
        <source>For Questions and Comments regarding the plugin builder template and creating your features in QGIS using the plugin interface please contact us via:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="73"/>
        <source>&lt;li&gt; the QGIS developers mailing list, or &lt;/li&gt;&lt;li&gt; IRC (#qgis on freenode.net)&lt;/li&gt;</source>
        <translation>&lt;li&gt; QGIS izstrādātāju vēstkopā vai &lt;/li&gt;&lt;li&gt; IRC (#qgis uz irc.freenode.net)&lt;/li&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="74"/>
        <source>QGIS is distributed under the Gnu Public License. If you create a useful plugin please consider contributing it back to the community.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="75"/>
        <source>Have fun and thank you for choosing QGIS.</source>
        <translation>Lai jums veicas un paldies, ka izvēlējāties QGIS.</translation>
    </message>
</context>
<context>
    <name>MapCoordsDialogBase</name>
    <message>
        <location filename="../src/plugins/georeferencer/mapcoordsdialogbase.ui" line="13"/>
        <source>Enter map coordinates</source>
        <translation>Ievadiet kartes koordinātas</translation>
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
        <location filename="../src/plugins/georeferencer/mapcoordsdialogbase.ui" line="185"/>
        <source>&amp;OK</source>
        <translation>&amp;Labi</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/mapcoordsdialogbase.ui" line="172"/>
        <source>&amp;Cancel</source>
        <translation>&amp;Atcelt</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/mapcoordsdialogbase.ui" line="28"/>
        <source>Enter X and Y coordinates which correspond with the selected point on the image. Alternatively, click the button with icon of a pencil and then click a corresponding point on map canvas of QGIS to fill in coordinates of that point.</source>
        <translation>Ievadiet X un Y koordinātas, kas atbilst uz kartes izvēlētajam punktam. Otrs variants ir noklikšķināt uz zīmuļa ikonas un uzklikšķināt atbilstošajam punktam QGIS kartes logā.</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/mapcoordsdialogbase.ui" line="137"/>
        <source> from map canvas</source>
        <translation> no kartes skata</translation>
    </message>
</context>
<context>
    <name>QFileDialog</name>
    <message>
        <location filename="../src/plugins/quick_print/quickprintgui.cpp" line="108"/>
        <source>Save experiment report to portable document format (.pdf)</source>
        <translation>Saglabāt eksperimenta atskaiti PDF formātā</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="666"/>
        <source>Load layer properties from style file (.qml)</source>
        <translation>Ielādēt slāņa īpašības no stila faila (.qml)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="727"/>
        <source>Save layer properties as style file (.qml)</source>
        <translation>Saglabāt slāņa īpašības kā stila failu (.qml)</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2995"/>
        <source>QGis files (*.qgs)</source>
        <translation>QGIS faili (*.qgs)</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproviderregistry.cpp" line="87"/>
        <source>No Data Provider Plugins</source>
        <comment>No QGIS data provider plugins found in:</comment>
        <translation>Nav datu sniedzēja spraudņa</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproviderregistry.cpp" line="89"/>
        <source>No vector layers can be loaded. Check your QGIS installation</source>
        <translation>Vektordatu slāņi nevar tikt ielādēti. Pārbaudiet savu QGIS instalāciju</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproviderregistry.cpp" line="92"/>
        <source>No Data Providers</source>
        <translation>Nav datu sniedzēju</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproviderregistry.cpp" line="251"/>
        <source>No data provider plugins are available. No vector layers can be loaded</source>
        <translation>Nav datu sniedzēju spraudņu. Vektordatu slāņi nevar tikt ielādēti</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="771"/>
        <source> at line </source>
        <translation> rindā </translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="772"/>
        <source> column </source>
        <translation> kolonā </translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="778"/>
        <source> for file </source>
        <translation> failam </translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="934"/>
        <source>Unable to save to file </source>
        <translation>Nav iespējams saglabāt failu </translation>
    </message>
    <message>
        <location filename="../src/core/qgssearchtreenode.cpp" line="289"/>
        <source>Referenced column wasn&apos;t found: </source>
        <translation>Atskaites kolonna netika atrasta: </translation>
    </message>
    <message>
        <location filename="../src/core/qgssearchtreenode.cpp" line="293"/>
        <source>Division by zero.</source>
        <translation>Dalīšana ar nulli.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolselect.cpp" line="75"/>
        <source>No active layer</source>
        <translation>Nav aktīvā slāņa</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="154"/>
        <source>Band</source>
        <translation>Kanāls</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="467"/>
        <source>action</source>
        <translation>darbība</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="476"/>
        <source> features found</source>
        <translation> objekti atrasti</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="480"/>
        <source> 1 feature found</source>
        <translation> 1 objekts atrasts</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="486"/>
        <source>No features found</source>
        <translation>Neviens objekts nav atrasts</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="486"/>
        <source>No features were found in the active layer at the point you clicked</source>
        <translation>Vietā, kur Jūs norādījāt, aktīvajā slānī nekas netika atrasts</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="587"/>
        <source>Could not identify objects on</source>
        <translation>Nebija iespejams identificēt objektus uz</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="587"/>
        <source>because</source>
        <translation>jo</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="72"/>
        <source>New centroid</source>
        <translation>Jauns centroīds</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="223"/>
        <source>New point</source>
        <translation>Jauns punkts</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="134"/>
        <source>New vertex</source>
        <translation>Jauna virsotne</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="223"/>
        <source>Undo last point</source>
        <translation>Atcelt pēdējo punktu</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="223"/>
        <source>Close line</source>
        <translation>Noslēgt līniju</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="543"/>
        <source>Select vertex</source>
        <translation>Izvēlieties virsotni</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="296"/>
        <source>Select new position</source>
        <translation>Izvēlieties jaunu novietojumu</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="427"/>
        <source>Select line segment</source>
        <translation>Izvēlieties līnijas segmentu</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="414"/>
        <source>New vertex position</source>
        <translation>Jauns virsotnes novietojums</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="414"/>
        <source>Release</source>
        <translation>Atlaist</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="530"/>
        <source>Delete vertex</source>
        <translation>Dzēst virsotni</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="530"/>
        <source>Release vertex</source>
        <translation>Atlaist virsotni</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="784"/>
        <source>Select element</source>
        <translation>Izvēlieties elementu</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="597"/>
        <source>New location</source>
        <translation>Jauns novietojums</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="673"/>
        <source>Release selected</source>
        <translation>Atlaist izvēlēto</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="673"/>
        <source>Delete selected / select next</source>
        <translation>Dzēst izvēlēto / izvēlēties nākamo</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="736"/>
        <source>Select position on line</source>
        <translation>Izvēlieties novietojumu uz līnijas</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="754"/>
        <source>Split the line</source>
        <translation>Sadalīt līniju</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="754"/>
        <source>Release the line</source>
        <translation>Atlaist līniju</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="768"/>
        <source>Select point on line</source>
        <translation>Izvēlēties punktu uz līnijas</translation>
    </message>
    <message>
        <location filename="../src/core/qgslabelattributes.cpp" line="58"/>
        <source>Label</source>
        <translation>Birka</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="454"/>
        <source>Length</source>
        <translation>Garums</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="460"/>
        <source>Area</source>
        <translation>Platība</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="771"/>
        <source>Project file read error: </source>
        <translation>Projekta faila nolasīšanas kļūda: </translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsleastsquares.cpp" line="32"/>
        <source>Fit to a linear transform requires at least 2 points.</source>
        <translation>Lineārai transformācijai ir nepieciešami vismaz divi punkti.</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsleastsquares.cpp" line="71"/>
        <source>Fit to a Helmert transform requires at least 2 points.</source>
        <translation>Helmerta transformācijai ir nepieciešami vismaz divi punkti.</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsleastsquares.cpp" line="123"/>
        <source>Fit to an affine transform requires at least 4 points.</source>
        <translation>Affine transformācijai ir nepieciešami vismaz četri punkti.</translation>
    </message>
    <message>
        <location filename="../src/providers/gpx/gpsdata.cpp" line="332"/>
        <source>Couldn&apos;t open the data source: </source>
        <translation>Nav iespējams atvērt datu avotu: </translation>
    </message>
    <message>
        <location filename="../src/providers/gpx/gpsdata.cpp" line="354"/>
        <source>Parse error at line </source>
        <translation>Parsēšanas kļūda līnijā </translation>
    </message>
    <message>
        <location filename="../src/providers/gpx/qgsgpxprovider.cpp" line="55"/>
        <source>GPS eXchange format provider</source>
        <translation>GPS apmaiņas formāta sniedzējs</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="303"/>
        <source>Caught a coordinate system exception while trying to transform a point. Unable to calculate line length.</source>
        <translation>Mēģinot transformēt punktu, tika konstatēts koordinātu transofmācijas sistēmas izņēmums. Nav iespējams noteikt līnijas garumu.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="394"/>
        <source>Caught a coordinate system exception while trying to transform a point. Unable to calculate polygon area.</source>
        <translation>Mēģinot transformēt punktu, tika konstatēts koordinātu transofmācijas sistēmas izņēmums. Nav iespējams noteikt poligona laukumu.</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="51"/>
        <source>CopyrightLabel</source>
        <translation>Autortiesību birka</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="52"/>
        <source>Draws copyright information</source>
        <translation>Parāda autortiesību informāciju</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfsplugin.cpp" line="31"/>
        <source>Version 0.1</source>
        <translation>Versija 0.1</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="44"/>
        <source>Version 0.2</source>
        <translation>Versija 0.2</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="45"/>
        <source>Loads and displays delimited text files containing x,y coordinates</source>
        <translation>Ielādē un parāda atdalīta teksta slāņus, kas satur x,y koordinātas</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="161"/>
        <source>Add Delimited Text Layer</source>
        <translation>Atdalīta teksta slāņi</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugin.cpp" line="57"/>
        <source>Georeferencer</source>
        <translation>Telpiskā piesaiste</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugin.cpp" line="58"/>
        <source>Adding projection info to rasters</source>
        <translation>Pievieno projekcijas informāciju rastra datiem</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="55"/>
        <source>GPS Tools</source>
        <translation>GPS rīki</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="57"/>
        <source>Tools for loading and importing GPS data</source>
        <translation>Rīki GPS datu ielādēšanai un importēšanai</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="829"/>
        <source>GRASS</source>
        <translation>GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="835"/>
        <source>GRASS layer</source>
        <translation>GRASS slānis</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugin.cpp" line="43"/>
        <source>Graticule Creator</source>
        <translation>Grādu tīkla veidotājs</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugin.cpp" line="44"/>
        <source>Builds a graticule</source>
        <translation>Izveido grādu tīklu</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="60"/>
        <source>NorthArrow</source>
        <translation>Ziemeļu bulta</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="61"/>
        <source>Displays a north arrow overlayed onto the map</source>
        <translation>Parāda ziemeļu virziena bultu uz kartes</translation>
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
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="62"/>
        <source>ScaleBar</source>
        <translation>Mērogjosla</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="63"/>
        <source>Draws a scale bar</source>
        <translation>Uzzīmē mērogjoslu</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitplugin.cpp" line="37"/>
        <source>SPIT</source>
        <translation>SPIT</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitplugin.cpp" line="38"/>
        <source>Shapefile to PostgreSQL/PostGIS Import Tool</source>
        <translation>Rīks, kas importē shapefile formāta datus PostGIS vidē</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfsplugin.cpp" line="29"/>
        <source>WFS plugin</source>
        <translation>WFS spraudnis</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfsplugin.cpp" line="30"/>
        <source>Adds WFS layers to the QGIS canvas</source>
        <translation>Pievieno WFS slāni QGIS kartei</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="166"/>
        <source>GRASS plugin</source>
        <translation>GRASS spraudnis</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="136"/>
        <source>QGIS couldn&apos;t find your GRASS installation.
Would you like to specify path (GISBASE) to your GRASS installation?</source>
        <translation>QGIS nevarēja atrast GRASS instalāciju.
Vai vēlaties norādīt ceļu līdz GRASS instalācijai (GISBASE)?</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="150"/>
        <source>Choose GRASS installation path (GISBASE)</source>
        <translation>Izvēlieties GRASS instalācijas mapi (GISBASE)</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="167"/>
        <source>GRASS data won&apos;t be available if GISBASE is not specified.</source>
        <translation>GRASS dati nebūs pieejami, ja GISBASE nebūs norādīts.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="42"/>
        <source>Not a vector layer</source>
        <translation>Nav vektoru slānis</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="43"/>
        <source>The current layer is not a vector layer</source>
        <translation>Pašreizējais slānis nav vektoru slānis</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="72"/>
        <source>Layer cannot be added to</source>
        <translation>Nav iespējams pievienot slāni</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="73"/>
        <source>The data provider for this layer does not support the addition of features.</source>
        <translation>Slāņa datu sniedzējs neatbalsta objektu pievienošanu.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="49"/>
        <source>Layer not editable</source>
        <translation>Slānis nav rediģējams</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="51"/>
        <source>Cannot edit the vector layer. To make it editable, go to the file item of the layer, right click and check &apos;Allow Editing&apos;.</source>
        <translation>Nav iespējams rediģēt vektoru slāni. Lai to padarītu rediģējamu, noklikšķiniet ar peles labo pogu un izvēlieties \&quot;Atļaut rediģēt\&quot;.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolselect.cpp" line="76"/>
        <source>To select features, you must choose a vector layer by clicking on its name in the legend</source>
        <translation>Lai izvēlētos objektus, jums vispirms ir jāzivēlas vektoru slānis uzklikšķinot uz tā nosaukuma</translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="193"/>
        <source>Python error</source>
        <translation>Python kļūda</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Couldn&apos;t load SIP module.
Python support will be disabled.</source>
        <translation type="obsolete">Nevar pievienot SIP moduli
Python atbalsts tiks atslēgts.</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Couldn&apos;t load PyQt bindings.
Python support will be disabled.</source>
        <translation type="obsolete">Nevar pievienot PyQt piesaistes
Python atbalsts tiks atslēgts.</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Couldn&apos;t load QGIS bindings.
Python support will be disabled.</source>
        <translation type="obsolete">Nevar pievienot QGIS piesaistes
Python atbalsts tiks atslēgts.</translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="477"/>
        <source>Couldn&apos;t load plugin </source>
        <translation>Nevar pievienot spraudni</translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="481"/>
        <source> due an error when calling its classFactory() method</source>
        <translation> dēļ kļūdas izsaucot tā classFactory() metodi</translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="485"/>
        <source> due an error when calling its initGui() method</source>
        <translation> dēļ kļūdas izsaucot tā initGui() metodi</translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="497"/>
        <source>Error while unloading plugin </source>
        <translation>Kļūda sparudņa atslēgšanā</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="59"/>
        <source>2.5D shape type not supported</source>
        <translation>2.5D datu tips nav atbalstīts</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="59"/>
        <source>Adding features to 2.5D shapetypes is not supported yet</source>
        <translation>Objektu pievienošana 2.5D datu formātam vēl netiek atbalstīta</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="207"/>
        <source>Wrong editing tool</source>
        <translation>Nepareizs labošanas rīks</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="92"/>
        <source>Cannot apply the &apos;capture point&apos; tool on this vector layer</source>
        <translation>Nevar pielietot &apos;punkta tveršanas&apos; rīku šim vektoru slānim</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="65"/>
        <source>Coordinate transform error</source>
        <translation>Koordinātu pārreiķina kļūda</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="66"/>
        <source>Cannot transform the point to the layers coordinate system</source>
        <translation>Nebija iespējams pārrēķināt punkta koordinātas uz slāņa koodrinātu sistēmu</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="200"/>
        <source>Cannot apply the &apos;capture line&apos; tool on this vector layer</source>
        <translation>Nevar pielietot &apos;līnijas tveršanas&apos; rīku šim vektoru slānim</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="208"/>
        <source>Cannot apply the &apos;capture polygon&apos; tool on this vector layer</source>
        <translation>Nevar pielietot &apos;poligonu tveršanas&apos; rīku šim vektoru slānim</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="427"/>
        <source>Error</source>
        <translation>Kļūda</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="416"/>
        <source>Cannot add feature. Unknown WKB type</source>
        <translation>Nevar pievienot objektu. Nezināms WKB tips</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdisland.cpp" line="113"/>
        <source>Error, could not add island</source>
        <translation>Kļūda, nevarēja pievienot salu</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="91"/>
        <source>A problem with geometry type occured</source>
        <translation>Gadījās problēma ar ģeometrijas tipu</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="95"/>
        <source>The inserted Ring is not closed</source>
        <translation>Ievietotais aplis nav noslēgts</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="99"/>
        <source>The inserted Ring is not a valid geometry</source>
        <translation>Ievietotais aplis nav derīga ģeometrija</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="103"/>
        <source>The inserted Ring crosses existing rings</source>
        <translation>Ievietotais aplis šķēso eksistējošu apli</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="107"/>
        <source>The inserted Ring is not contained in a feature</source>
        <translation>Ievietotais aplis neatrodas objektā</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="111"/>
        <source>An unknown error occured</source>
        <translation>Konstatēta neatpazīta kļūda</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="113"/>
        <source>Error, could not add ring</source>
        <translation>Kļūda, nevar pievienot apli</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="640"/>
        <source> km2</source>
        <translation> km2</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="645"/>
        <source> ha</source>
        <translation> ha</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="650"/>
        <source> m2</source>
        <translation> m2</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="678"/>
        <source> m</source>
        <translation> m</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="663"/>
        <source> km</source>
        <translation> km</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="668"/>
        <source> mm</source>
        <translation> mm</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="673"/>
        <source> cm</source>
        <translation> cm</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="687"/>
        <source> sq mile</source>
        <translation>kv. jūdze</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="692"/>
        <source> sq ft</source>
        <translation>kv. pēda</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="699"/>
        <source> mile</source>
        <translation>jūdze</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="705"/>
        <source> foot</source>
        <translation> pēdas</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="707"/>
        <source> feet</source>
        <translation> pēdas</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="714"/>
        <source> sq.deg.</source>
        <translation> kv. grādi</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="719"/>
        <source> degree</source>
        <translation> grāds</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="721"/>
        <source> degrees</source>
        <translation> grādi</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="725"/>
        <source> unknown</source>
        <translation> nezināms</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="273"/>
        <source>Received %1 of %2 bytes</source>
        <translation>Saņemti %1 no %2 baitiem</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="279"/>
        <source>Received %1 bytes (total unknown)</source>
        <translation>Saņemti %1 baiti (kopējais apjoms nezināms)</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="390"/>
        <source>Not connected</source>
        <translation>Nav pieslēdzies</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="396"/>
        <source>Looking up &apos;%1&apos;</source>
        <translation>Meklē &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="403"/>
        <source>Connecting to &apos;%1&apos;</source>
        <translation>Pieslēdzas &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="410"/>
        <source>Sending request &apos;%1&apos;</source>
        <translation>Sūta pieprasījumu &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="417"/>
        <source>Receiving reply</source>
        <translation>Saņem atbildi</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="423"/>
        <source>Response is complete</source>
        <translation>Atbildes process ir pabeigts</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="429"/>
        <source>Closing down connection</source>
        <translation>Savienojums tiek slēgts</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="753"/>
        <source>Unable to open </source>
        <translation>Nav iespējams atvērt</translation>
    </message>
    <message>
        <location filename="../src/core/qgssearchtreenode.cpp" line="253"/>
        <source>Regular expressions on numeric values don&apos;t make sense. Use comparison instead.</source>
        <translation>Regulārās izteksmes priekš skaitļiem ir bezjēdzīgas. Izmantojiet to vietā salīdzināšanu.</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="48"/>
        <source>Geoprocessing functions for working with PostgreSQL/PostGIS layers</source>
        <translation>Ģeoapstrādes funkcijas darbam ar PostgreSQL/PostGIS slāņiem</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="137"/>
        <source>Location: </source>
        <comment>Metadata in GRASS Browser</comment>
        <translation>Novietojums: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="137"/>
        <source>&lt;br&gt;Mapset: </source>
        <comment>Metadata in GRASS Browser</comment>
        <translation>&lt;br&gt;Karšu kopa: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="141"/>
        <source>Location: </source>
        <translation>Novietojums:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="141"/>
        <source>&lt;br&gt;Mapset: </source>
        <translation>&lt;br&gt;Karšu kopa:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="146"/>
        <source>&lt;b&gt;Raster&lt;/b&gt;</source>
        <translation>&lt;b&gt;Rastrs&lt;b&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="155"/>
        <source>Cannot open raster header</source>
        <translation>Nevar atvērt rastra iesākumu</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="159"/>
        <source>Rows</source>
        <translation>Rindas</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="160"/>
        <source>Columns</source>
        <translation>Kolonnas</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="161"/>
        <source>N-S resolution</source>
        <translation>Z-D izšķirtspēja</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="162"/>
        <source>E-W resolution</source>
        <translation>A-R izšķirtspēja</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="300"/>
        <source>North</source>
        <translation>Ziemeļi</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="302"/>
        <source>South</source>
        <translation>Dienvidi</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="304"/>
        <source>East</source>
        <translation>Austrumi</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="306"/>
        <source>West</source>
        <translation>Rietumi</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="189"/>
        <source>Format</source>
        <translation>Formāts</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="200"/>
        <source>Minimum value</source>
        <translation>Minimālā vērtība</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="201"/>
        <source>Maximum value</source>
        <translation>Maksimālā vērtība</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="212"/>
        <source>Data source</source>
        <translation>Datu avots</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="217"/>
        <source>Data description</source>
        <translation>Datu apraksts</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="226"/>
        <source>Comments</source>
        <translation>Komentāri</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="241"/>
        <source>Categories</source>
        <translation>Kategorijas</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="347"/>
        <source>&lt;b&gt;Vector&lt;/b&gt;</source>
        <translation>&lt;b&gt;Vektors&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="274"/>
        <source>Points</source>
        <translation>Punkti</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="275"/>
        <source>Lines</source>
        <translation>Līnijas</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="276"/>
        <source>Boundaries</source>
        <translation>Robežas</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="277"/>
        <source>Centroids</source>
        <translation>Centroīdi</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="280"/>
        <source>Faces</source>
        <translation>Plaknes</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="281"/>
        <source>Kernels</source>
        <translation>Kodoli</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="284"/>
        <source>Areas</source>
        <translation>Poligoni</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="285"/>
        <source>Islands</source>
        <translation>Salas</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="309"/>
        <source>Top</source>
        <translation>Augša</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="310"/>
        <source>Bottom</source>
        <translation>Apakša</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="313"/>
        <source>yes</source>
        <translation>jā</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="313"/>
        <source>no</source>
        <translation>nē</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="320"/>
        <source>History&lt;br&gt;</source>
        <translation>Vēsture&lt;br&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="348"/>
        <source>&lt;b&gt;Layer&lt;/b&gt;</source>
        <translation>&lt;b&gt;Slānis&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="367"/>
        <source>Features</source>
        <translation>Objekti</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="376"/>
        <source>Driver</source>
        <translation>Dzinējs</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="377"/>
        <source>Database</source>
        <translation>Datu bāze</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="378"/>
        <source>Table</source>
        <translation>Tabula</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="379"/>
        <source>Key column</source>
        <translation>Atslēgas kolonna</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="453"/>
        <source>GISBASE is not set.</source>
        <translation>GISBASE nav norādīta.</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="458"/>
        <source> is not a GRASS mapset.</source>
        <translation>nav GRASS karšu kopa.</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="480"/>
        <source>Cannot start </source>
        <translation>Nevar sākt</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="497"/>
        <source>Mapset is already in use.</source>
        <translation>Karšu kopa ir jau lietošanā.</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="512"/>
        <source>Temporary directory </source>
        <translation>Pagaidu mape</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="512"/>
        <source> exist but is not writable</source>
        <translation> eksistē bet nevar rakstīt</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="518"/>
        <source>Cannot create temporary directory </source>
        <translation>Nevar izveidot pagaidu mapi</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="534"/>
        <source>Cannot create </source>
        <translation>Nevar izveidot</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="611"/>
        <source>Cannot remove mapset lock: </source>
        <translation>Nevar noņemt mapset slēdzi: </translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="1051"/>
        <source>Warning</source>
        <translation>Brīdinājums</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="999"/>
        <source>Cannot read raster map region</source>
        <translation>Nevar nolasīt rastra kartes reģionu</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="1016"/>
        <source>Cannot read vector map region</source>
        <translation>Nevar nolasīt vektoru kartes reģionu</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="1052"/>
        <source>Cannot read region</source>
        <translation>Nevar nolasīt reģionu</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2422"/>
        <source>Where is &apos;</source>
        <translation>Kur &apos;</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2422"/>
        <source>original location: </source>
        <translation>sākotnējais novietojums: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="123"/>
        <source>To identify features, you must choose an active layer by clicking on its name in the legend</source>
        <translation>Lai identificētu objektus, jums ir jāizvēlas aktīvais slānis, noklikšķinot uz tā nosaukuma</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="47"/>
        <source>PostgreSQL Geoprocessing</source>
        <translation>PostgreSQL ģeoprocesēšana</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintplugin.cpp" line="38"/>
        <source>Quick Print</source>
        <translation>Ātrā druka</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintplugin.cpp" line="40"/>
        <source>Quick Print is a plugin to quickly print a map with minimal effort.</source>
        <translation>Ātrās drukas spraudnis ļauj izdrukāt karti pieliekot tam minimālas pūles.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="427"/>
        <source>Could not remove polygon intersection</source>
        <translation>Nebija iespējams aizvākt poligonu pārklāšanos</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Currently only filebased datasets are supported</source>
        <translation type="obsolete">Pagaidām ir atbalstītas tikai failu datu kopas</translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="521"/>
        <source>Loaded default style file from </source>
        <translation>Ielādēts noklusējuma stils no faila</translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="552"/>
        <source>The directory containing your dataset needs to be writeable!</source>
        <translation>Mapei, kurā atrodas jūsu datu kopas, ir jābūt ar rakstīšanas iespēju!</translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="565"/>
        <source>Created default style file as </source>
        <translation>Izveidots noklusējuma stila fails kā </translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>ERROR: Failed to created default style file as </source>
        <translation type="obsolete">KĻŪDA: Kļūda izveidojoto noklusējuma stilu failu kā </translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>File could not been opened.</source>
        <translation type="obsolete">Nav iespējams atvērt failu.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="569"/>
        <source>ERROR: Failed to created default style file as %1 Check file permissions and retry.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="946"/>
        <source> is not writeable.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="946"/>
        <source>Please adjust permissions (if possible) and try again.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsgridfilewriter.cpp" line="61"/>
        <source>Interpolating...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsgridfilewriter.cpp" line="61"/>
        <source>Abort</source>
        <translation type="unfinished">Atcelt</translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsinterpolationplugin.cpp" line="24"/>
        <source>Interpolation plugin</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsinterpolationplugin.cpp" line="25"/>
        <source>A plugin for interpolation based on vertices of a vector layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsinterpolationplugin.cpp" line="26"/>
        <source>Version 0.001</source>
        <translation type="unfinished">Versija 0.001</translation>
    </message>
    <message>
        <location filename="../src/plugins/sos/qgssosplugin.cpp" line="25"/>
        <source>SOS plugin</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/sos/qgssosplugin.cpp" line="26"/>
        <source>Adds layers from Sensor Observation Service (SOS) to the QGIS canvas</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="400"/>
        <source>Uncatched fatal GRASS error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="63"/>
        <source>Couldn&apos;t load SIP module.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="78"/>
        <source>Python support will be disabled.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="71"/>
        <source>Couldn&apos;t load PyQt4.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="78"/>
        <source>Couldn&apos;t load PyQGIS.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="89"/>
        <source>An error has occured while executing Python code:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="188"/>
        <source>Python version:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="189"/>
        <source>Python path:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="179"/>
        <source>An error occured during execution of following code:</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgisApp</name>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="345"/>
        <source>Quantum GIS - </source>
        <translation>Quantum GIS - </translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Version </source>
        <translation type="obsolete">Versija </translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source> with PostgreSQL support</source>
        <translation type="obsolete"> ar PostgreSQL atbalsu</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source> (no PostgreSQL support)</source>
        <translation type="obsolete"> (bez PostgreSQL atbalsta)</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Quantum GIS is licensed under the GNU General Public License</source>
        <translation type="obsolete">Quantum GIS tiek izplatīts izmantojot GPL licenci</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>http://www.gnu.org/licenses</source>
        <translation type="obsolete">http://www.gnu.org/licenses</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1625"/>
        <source>Version</source>
        <translation>Versija</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Available Data Provider Plugins</source>
        <translation type="obsolete">Pieejamie datu sniedzēju spraudņi</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2081"/>
        <source>is not a valid or recognized data source</source>
        <translation>ir nederīgs vai neatpazīts datu avots</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5184"/>
        <source>Invalid Data Source</source>
        <translation>Nederīgs datu avots</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2173"/>
        <source>Invalid Layer</source>
        <translation>Nederīgs slānis</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2173"/>
        <source>%1 is an invalid layer and cannot be loaded.</source>
        <translation>%1 ir nederīgs slānis un nevar tikt ielādēts.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3234"/>
        <source>Choose a filename to save the map image as</source>
        <translation>Izvēlieties faila nosaukumu ar kādu saglabāt kartes attēlu</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3276"/>
        <source>Saved map image to</source>
        <translation>Saglabāt kartes attēlu uz</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3487"/>
        <source>No Layer Selected</source>
        <translation>Nav izvēlēts slānis</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3517"/>
        <source>Problem deleting features</source>
        <translation>Problēma ar objektu dzēšanu</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3518"/>
        <source>A problem occured during deletion of features</source>
        <translation>Dzēšot objektus gadījās problēma</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3495"/>
        <source>No Vector Layer Selected</source>
        <translation>Nav izvēlēts vektordatu slānis</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3496"/>
        <source>Deleting features only works on vector layers</source>
        <translation>Objektu dzēšana darbojas tikai vektordatu slāņiem</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3488"/>
        <source>To delete features, you must select a vector layer in the legend</source>
        <translation>Lai izdzēstu kādu objektu, Jums vispirms jāizvēlas vektordatu slānis no slāņu saraksta</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4012"/>
        <source>Error Loading Plugin</source>
        <translation>Kļūda ielādējot spraudni</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4012"/>
        <source>There was an error loading %1.</source>
        <translation>Gadījās kļūda ielādējot %1.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4048"/>
        <source>No MapLayer Plugins</source>
        <translation>Nav MapLayer spraudņu</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4048"/>
        <source>No MapLayer plugins in ../plugins/maplayer</source>
        <translation>Nav MapLayer spraudņu iekš ../plugins/maplayer</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4125"/>
        <source>No Plugins</source>
        <translation>Nav spraudņu</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4126"/>
        <source>No plugins found in ../plugins. To test plugins, start qgis from the src directory</source>
        <translation>Spraudņi iekš ../plugins nav atrasti. Lai notestētu spraudņus, startējiet QGIS no src mapes</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4160"/>
        <source>Name</source>
        <translation>Nosaukums</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4160"/>
        <source>Plugin %1 is named %2</source>
        <translation>Spraudni %1 sauc par %2</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4177"/>
        <source>Plugin Information</source>
        <translation>Spraudņa informācija</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4178"/>
        <source>QGis loaded the following plugin:</source>
        <translation>QGIS ielādēja sekojošu spraudni:</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4178"/>
        <source>Name: %1</source>
        <translation>Nosaukums: %1</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4178"/>
        <source>Version: %1</source>
        <translation>Versija: %1</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4179"/>
        <source>Description: %1</source>
        <translation>Apraksts: %1</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4197"/>
        <source>Unable to Load Plugin</source>
        <translation>Spraudni nevar ielādēt</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4198"/>
        <source>QGIS was unable to load the plugin from: %1</source>
        <translation>QGIS nevarēja ielādēt spraudni no: %1</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4254"/>
        <source>There is a new version of QGIS available</source>
        <translation>Ir pieejama jaunāka QGIS versija</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4260"/>
        <source>You are running a development version of QGIS</source>
        <translation>Jūs lietojat QGIS izstrādes versiju</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4264"/>
        <source>You are running the current version of QGIS</source>
        <translation>Jūs lietojat jaunāko QGIS versiju</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4269"/>
        <source>Would you like more information?</source>
        <translation>Vai Jūs vēlaties sīkāku informāciju?</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4316"/>
        <source>QGIS Version Information</source>
        <translation>QGIS versijas informācija</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4288"/>
        <source>Unable to get current version information from server</source>
        <translation>Nevaru saņemt informāciju par pašreizējo versiju no servera</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4302"/>
        <source>Connection refused - server may be down</source>
        <translation>Savienojums nav atļauts - serveris, iespējams, ir izslēgts</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4305"/>
        <source>QGIS server was not found</source>
        <translation>QGIS serveris netika atrasts</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4684"/>
        <source>Extents: </source>
        <translation>Apjoms: </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1464"/>
        <source>Map legend that displays all the layers currently on the map canvas. Click on the check box to turn a layer on or off. Double click on a layer in the legend to customize its appearance and set other properties.</source>
        <translation>Kartes slāņu saraksts, kas parāda visus kartes skata rāmī esošos slāņus. 
Uzklikšķiniet uz ķekškastes, lai parādītu / paslēptu slāni. Dubultklikšķis uz slāņa nosaukuma ļaus pielāgot tā attēlošanu un iestatīt citus tā parametrus.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1425"/>
        <source>Map overview canvas. This canvas can be used to display a locator map that shows the current extent of the map canvas. The current extent is shown as a red rectangle. Any layer on the map can be added to the overview canvas.</source>
        <translation>Kartes pārskata rāmis. Tas ļauj parādīt kartes pārskatu un pašreizējā kartes rāmī redzamā skata atrašanās vietu, kas redzams kā sarkans taisnstūris. Kartes pārskata rāmim var pievienot jebkuru kartes slāni.</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Map canvas. This is where raster and vector layers are displayed when added to the map</source>
        <translation type="obsolete">Kartes audekls. Šeit tiek rādīti visi rastra un vektordatu slāņi, kad tos pievieno kartei</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1023"/>
        <source>&amp;Plugins</source>
        <translation>&amp;Spraudņi</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1160"/>
        <source>Progress bar that displays the status of rendering layers and other time-intensive operations</source>
        <translation>Progresa josla rāda cik daudz no slāņiem jau ir parādīts, kā arī citu laikietilpīgu procesu paveikto daļu</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1189"/>
        <source>Displays the current map scale</source>
        <translation>Attēlo pašreizējo kartes mērogu</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1215"/>
        <source>Render</source>
        <translation>Renderēt</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1221"/>
        <source>When checked, the map layers are rendered in response to map navigation commands and other events. When not checked, no rendering is done. This allows you to add a large number of layers and symbolize them before rendering.</source>
        <translation>Ja atzīmēts, kartes slāņi tiek renderēti uzreiz tiklīdz kāda darbība tiek veikta. Ja nav atzīmēts, renderēšana nenotiek. Tas ļauj pievienot lielu daudzumu slāņu un iestatīt tiem apzīmējumus pirms to attēlošanas kartē.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2921"/>
        <source>Choose a QGIS project file</source>
        <translation>Izvēlieties QGIS projekta failu</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3045"/>
        <source>Unable to save project</source>
        <translation>Nav iespējams saglabāt projektu</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3046"/>
        <source>Unable to save project to </source>
        <translation>Nav iespējams saglabāt projektu uz </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1222"/>
        <source>Toggle map rendering</source>
        <translation>Pārslēdz kartes renderēšanu</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1245"/>
        <source>This icon shows whether on the fly projection is enabled or not. Click the icon to bring up the project properties dialog to alter this behaviour.</source>
        <translation>Šī ikona rāda vai tūlītēja projekciju maiņa ir ieslēgta vai nē. Klikšķiniet uz ikonas, lai mainītu projekta īpašības.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1247"/>
        <source>Projection status - Click to open projection dialog</source>
        <translation>Projekcijas statuss - klikšķiniet lai atvērtu projekcijas dialogu</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2027"/>
        <source>Open an OGR Supported Vector Layer</source>
        <translation>Atvērt OGR atbalstītu vektordatu slāni</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2649"/>
        <source>Save As</source>
        <translation>Saglabāt kā</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2749"/>
        <source>Choose a QGIS project file to open</source>
        <translation>Izvēlieties QGIS projekta failu ko atvērt</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2864"/>
        <source>QGIS Project Read Error</source>
        <translation>QGIS projekta faila nolasīšanas kļūda</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2866"/>
        <source>Try to find missing layers?</source>
        <translation>Mēģini atrast trūkstošos slāņus?</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3038"/>
        <source>Saved project to:</source>
        <translation>Saglabāt projektu iekš:</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5092"/>
        <source>Open a GDAL Supported Raster Data Source</source>
        <translation>Atvērt GDAL atbalstītu datu avotu</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1627"/>
        <source>New features</source>
        <translation>Jaunās iespējas</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2882"/>
        <source>Unable to open project</source>
        <translation>Nav iespējams atvērt projektu</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3052"/>
        <source>Unable to save project </source>
        <translation>Nav iespējams saglabāt projektu </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3110"/>
        <source>QGIS: Unable to load project</source>
        <translation>QGIS: Nav iespējams ielādēt projektu</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3111"/>
        <source>Unable to load project </source>
        <translation>Nav iespējams ielādēt projektu </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5264"/>
        <source>Layer is not valid</source>
        <translation>Slānis ir nederīgs</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5265"/>
        <source>The layer is not a valid layer and can not be added to the map</source>
        <translation>Slānis nav derīgs un tādēļ to nevar pievienot kartei</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4533"/>
        <source>Save?</source>
        <translation>Saglabāt?</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4534"/>
        <source>Do you want to save the current project?</source>
        <translation>Vai Jūs vēlaties saglabāt projektu?</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="644"/>
        <source>Show all layers</source>
        <translation>Rādīt visus slāņus</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="649"/>
        <source>Hide all layers</source>
        <translation>Slēpt visus slāņus</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5047"/>
        <source>Clipboard contents set to: </source>
        <translation>Starpliktuves saturu iestatīt uz: </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5183"/>
        <source> is not a valid or recognized raster data source</source>
        <translation> ir nederīgs vai neatpazīts rastra datu avots</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5344"/>
        <source> is not a supported raster data source</source>
        <translation> ir neatbalstīts rastra datu avots</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5345"/>
        <source>Unsupported Data Source</source>
        <translation>Neatbalstīts datu avots</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5440"/>
        <source>New Bookmark</source>
        <translation>Jauna grāmatzīme</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5441"/>
        <source>Enter a name for the new bookmark:</source>
        <translation>Ievadiet nosaukumu jaunajai grāmatzīmei:</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5458"/>
        <source>Error</source>
        <translation>Kļūda</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5458"/>
        <source>Unable to create the bookmark. Your user database may be missing or corrupted</source>
        <translation>Nav iespējams izveidot grāmatzīmi. Jūsu lietotāju datubāze ir vai nu bojāta, vai tās vispār nav</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="318"/>
        <source>Reading settings</source>
        <translation>Iestatījumu nolasīšana</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="322"/>
        <source>Setting up the GUI</source>
        <translation>Grafiskās vides iestatīšana</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="312"/>
        <source>Checking database</source>
        <translation>Datu bāzes pārbaude</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="405"/>
        <source>Restoring loaded plugins</source>
        <translation>Ielādēto spraudņu atjaunošana</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="409"/>
        <source>Initializing file filters</source>
        <translation>Failu filtru inicializēšana</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="434"/>
        <source>Restoring window state</source>
        <translation>Loga statusa atjaunošana</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="438"/>
        <source>QGIS Ready!</source>
        <translation>QGIS darbam gatavs!</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="548"/>
        <source>&amp;New Project</source>
        <translation>&amp;Jauns projekts</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="549"/>
        <source>Ctrl+N</source>
        <comment>New Project</comment>
        <translation>Ctrl+N</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="550"/>
        <source>New Project</source>
        <translation>Jauns projekts</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="553"/>
        <source>&amp;Open Project...</source>
        <translation>&amp;Atvērt projektu...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="554"/>
        <source>Ctrl+O</source>
        <comment>Open a Project</comment>
        <translation>Ctrl+O</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="555"/>
        <source>Open a Project</source>
        <translation>Atvērt projektu</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="558"/>
        <source>&amp;Save Project</source>
        <translation>&amp;Saglabāt projektu</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="559"/>
        <source>Ctrl+S</source>
        <comment>Save Project</comment>
        <translation>Ctrl+S</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="560"/>
        <source>Save Project</source>
        <translation>Saglabāt projektu</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="563"/>
        <source>Save Project &amp;As...</source>
        <translation>Saglabāt projektu &amp;kā...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="564"/>
        <source>Ctrl+A</source>
        <comment>Save Project under a new name</comment>
        <translation>Ctrl+A</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="565"/>
        <source>Save Project under a new name</source>
        <translation>Saglabā projektu ar jaunu nosaukumu</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="568"/>
        <source>&amp;Print...</source>
        <translation>&amp;Drukāt...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="569"/>
        <source>Ctrl+P</source>
        <comment>Print</comment>
        <translation>Ctrl+P</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="570"/>
        <source>Print</source>
        <translation>Drukāt</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="573"/>
        <source>Save as Image...</source>
        <translation>Saglabāt kā attēlu...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="574"/>
        <source>Ctrl+I</source>
        <comment>Save map as image</comment>
        <translation>Ctrl+I</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="575"/>
        <source>Save map as image</source>
        <translation>Saglabā karti kā attēlu</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="586"/>
        <source>Exit</source>
        <translation>Iziet</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="587"/>
        <source>Ctrl+Q</source>
        <comment>Exit QGIS</comment>
        <translation>Ctrl+Q</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="588"/>
        <source>Exit QGIS</source>
        <translation>Iziet no QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="593"/>
        <source>Add a Vector Layer...</source>
        <translation>Pievienot vektordatu slāni...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="594"/>
        <source>V</source>
        <comment>Add a Vector Layer</comment>
        <translation>V</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="595"/>
        <source>Add a Vector Layer</source>
        <translation>Pievieno vektordatu slāni</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="598"/>
        <source>Add a Raster Layer...</source>
        <translation>Pievienot rastra datu slāni...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="599"/>
        <source>R</source>
        <comment>Add a Raster Layer</comment>
        <translation>R</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="600"/>
        <source>Add a Raster Layer</source>
        <translation>Pievieno rastra datu slāni</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="603"/>
        <source>Add a PostGIS Layer...</source>
        <translation>Pievienot PostGIS slāni...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="604"/>
        <source>D</source>
        <comment>Add a PostGIS Layer</comment>
        <translation>D</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="605"/>
        <source>Add a PostGIS Layer</source>
        <translation>Pievieno PostGIS slāni</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="615"/>
        <source>New Vector Layer...</source>
        <translation>Jauns vektordatu slānis...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="616"/>
        <source>N</source>
        <comment>Create a New Vector Layer</comment>
        <translation>N</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="617"/>
        <source>Create a New Vector Layer</source>
        <translation>Izveido jaunu vektordatu slāni</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="620"/>
        <source>Remove Layer</source>
        <translation>Noņemt slāni</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="621"/>
        <source>Ctrl+D</source>
        <comment>Remove a Layer</comment>
        <translation>Ctrl+D</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="622"/>
        <source>Remove a Layer</source>
        <translation>Noņem slāni</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="626"/>
        <source>Add All To Overview</source>
        <translation>Pievienot visu pārskatam</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="627"/>
        <source>+</source>
        <comment>Show all layers in the overview map</comment>
        <translation>+</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="628"/>
        <source>Show all layers in the overview map</source>
        <translation>Rādīt visus slāņus pārskata kartē</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="631"/>
        <source>Remove All From Overview</source>
        <translation>Noņemt visu no pārskata</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="632"/>
        <source>-</source>
        <comment>Remove all layers from overview map</comment>
        <translation>-</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="633"/>
        <source>Remove all layers from overview map</source>
        <translation>Noņem visus slāņus no pārskata kartes</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="642"/>
        <source>Show All Layers</source>
        <translation>Rādīt visus slāņus</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="643"/>
        <source>S</source>
        <comment>Show all layers</comment>
        <translation>S</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="647"/>
        <source>Hide All Layers</source>
        <translation>Slēpt visus slāņus</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="648"/>
        <source>H</source>
        <comment>Hide all layers</comment>
        <translation>H</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="654"/>
        <source>Project Properties...</source>
        <translation>Projekta īpašības...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="655"/>
        <source>P</source>
        <comment>Set project properties</comment>
        <translation>P</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="656"/>
        <source>Set project properties</source>
        <translation>Iestata projekta īpašības</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="659"/>
        <source>Options...</source>
        <translation>Opcijas...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="661"/>
        <source>Change various QGIS options</source>
        <translation>Maina dažādas QGIS opcijas</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="664"/>
        <source>Custom Projection...</source>
        <translation>Pielāgota projekcija...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="666"/>
        <source>Manage custom projections</source>
        <translation>Pārvaldīt pielāgotās projekcijas</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="671"/>
        <source>Help Contents</source>
        <translation>Palīdzības saturs</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="675"/>
        <source>F1</source>
        <comment>Help Documentation</comment>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="677"/>
        <source>Help Documentation</source>
        <translation>Palīdzības dokumentācija</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="680"/>
        <source>Qgis Home Page</source>
        <translation>QGIS mājaslapa</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="682"/>
        <source>Ctrl+H</source>
        <comment>QGIS Home Page</comment>
        <translation>Ctrl+H</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="684"/>
        <source>QGIS Home Page</source>
        <translation>QGIS mājaslapa</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="687"/>
        <source>About</source>
        <translation>Par</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="688"/>
        <source>About QGIS</source>
        <translation>Par QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="691"/>
        <source>Check Qgis Version</source>
        <translation>Pārbaudīt QGIS versiju</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="692"/>
        <source>Check if your QGIS version is up to date (requires internet access)</source>
        <translation>Pārbauda, vai izmantotā QGIS versija ir pati jaunākā (nepieciešama pieeja internetam)</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="697"/>
        <source>Refresh</source>
        <translation>Atjaunināt</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="698"/>
        <source>Ctrl+R</source>
        <comment>Refresh Map</comment>
        <translation>Ctrl+R</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="699"/>
        <source>Refresh Map</source>
        <translation>Atjaunināt karti</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="704"/>
        <source>Zoom In</source>
        <translation>Tuvināt</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="703"/>
        <source>Ctrl++</source>
        <comment>Zoom In</comment>
        <translation>Ctrl++</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="709"/>
        <source>Zoom Out</source>
        <translation>Tālināt</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="708"/>
        <source>Ctrl+-</source>
        <comment>Zoom Out</comment>
        <translation>Ctrl+-</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="712"/>
        <source>Zoom Full</source>
        <translation>Tuvināt kopskatu</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="713"/>
        <source>F</source>
        <comment>Zoom to Full Extents</comment>
        <translation>F</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="714"/>
        <source>Zoom to Full Extents</source>
        <translation>Tālināt līdz kopskatam</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="717"/>
        <source>Zoom To Selection</source>
        <translation>Tuvināt līdz izvēlei</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="718"/>
        <source>Ctrl+F</source>
        <comment>Zoom to selection</comment>
        <translation>Ctrl+F</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="719"/>
        <source>Zoom to selection</source>
        <translation>Tuvināt līdz izvēlei</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="722"/>
        <source>Pan Map</source>
        <translation>Panoramēt karti</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="723"/>
        <source>Pan the map</source>
        <translation>Velk karti</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="726"/>
        <source>Zoom Last</source>
        <translation>Pēdējā tāllummaiņa</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="728"/>
        <source>Zoom to Last Extent</source>
        <translation>Tuvina līdz iepriekšējam skatam</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="731"/>
        <source>Zoom To Layer</source>
        <translation>Tuvināt līdz slānim</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="733"/>
        <source>Zoom to Layer</source>
        <translation>Tuvina līdz slānim</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="736"/>
        <source>Identify Features</source>
        <translation>Identificēt objektus</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="737"/>
        <source>I</source>
        <comment>Click on features to identify them</comment>
        <translation>I</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="738"/>
        <source>Click on features to identify them</source>
        <translation>Klikšķiniet uz objektiem, lai tos identificētu</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="742"/>
        <source>Select Features</source>
        <translation>Izvēlēties objektus</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="748"/>
        <source>Open Table</source>
        <translation>Atvērt tabulu</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="752"/>
        <source>Measure Line </source>
        <translation>Mērīt līniju </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="753"/>
        <source>Ctrl+M</source>
        <comment>Measure a Line</comment>
        <translation>Ctrl+M</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="754"/>
        <source>Measure a Line</source>
        <translation>Mēra līniju</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="757"/>
        <source>Measure Area</source>
        <translation>Mērīt laukumu</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="758"/>
        <source>Ctrl+J</source>
        <comment>Measure an Area</comment>
        <translation>Ctrl+J</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="759"/>
        <source>Measure an Area</source>
        <translation>Mēra laukumu</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="764"/>
        <source>Show Bookmarks</source>
        <translation>Rādīt grāmatzīmes</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="763"/>
        <source>B</source>
        <comment>Show Bookmarks</comment>
        <translation>B</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="779"/>
        <source>New Bookmark...</source>
        <translation>Jauna grāmatzīme...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="780"/>
        <source>Ctrl+B</source>
        <comment>New Bookmark</comment>
        <translation>Ctrl+B</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="784"/>
        <source>Add WMS Layer...</source>
        <translation>Pievienot WMS slāni...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="785"/>
        <source>W</source>
        <comment>Add Web Mapping Server Layer</comment>
        <translation>W</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="786"/>
        <source>Add Web Mapping Server Layer</source>
        <translation>Pievieno WMS slāni</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="789"/>
        <source>In Overview</source>
        <translation>Pievienot pārskatam</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="790"/>
        <source>O</source>
        <comment>Add current layer to overview map</comment>
        <translation>O</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="791"/>
        <source>Add current layer to overview map</source>
        <translation>Pievienot pašreizējo slāni pārskata kartei</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="797"/>
        <source>Plugin Manager...</source>
        <translation>Spraudņu pārvaldnieks...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="799"/>
        <source>Open the plugin manager</source>
        <translation>Atver spraudņu pārvaldnieku</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="817"/>
        <source>Capture Point</source>
        <translation>Atlikt punktu</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="818"/>
        <source>.</source>
        <comment>Capture Points</comment>
        <translation>.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="819"/>
        <source>Capture Points</source>
        <translation>Atliek punktus</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="823"/>
        <source>Capture Line</source>
        <translation>Atlikt līniju</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="824"/>
        <source>/</source>
        <comment>Capture Lines</comment>
        <translation>/</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="825"/>
        <source>Capture Lines</source>
        <translation>Atliek līnijas</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="829"/>
        <source>Capture Polygon</source>
        <translation>Atlikt poligonu</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="830"/>
        <source>Ctrl+/</source>
        <comment>Capture Polygons</comment>
        <translation>Ctrl+/</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="831"/>
        <source>Capture Polygons</source>
        <translation>Atliek poligonus</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="836"/>
        <source>Delete Selected</source>
        <translation>Dzēst izvēlēto</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="851"/>
        <source>Add Vertex</source>
        <translation>Pievienot virsotni</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="856"/>
        <source>Delete Vertex</source>
        <translation>Dzēst virsotni</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="861"/>
        <source>Move Vertex</source>
        <translation>Pārvietot virsotni</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="954"/>
        <source>&amp;File</source>
        <translation>&amp;Fails</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="957"/>
        <source>&amp;Open Recent Projects</source>
        <translation>&amp;Atvērt nesenos projektus</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="977"/>
        <source>&amp;View</source>
        <translation>&amp;Skats</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="996"/>
        <source>&amp;Layer</source>
        <translation>&amp;Slānis</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1016"/>
        <source>&amp;Settings</source>
        <translation>&amp;Iestatījumi</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1036"/>
        <source>&amp;Help</source>
        <translation>&amp;Palīdzība</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1055"/>
        <source>File</source>
        <translation>Fails</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1072"/>
        <source>Manage Layers</source>
        <translation>Pārvaldīt slāņus</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1133"/>
        <source>Help</source>
        <translation>Palīdzība</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1082"/>
        <source>Digitizing</source>
        <translation>Digitizēšana</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1102"/>
        <source>Map Navigation</source>
        <translation>Kartes navigācija</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1115"/>
        <source>Attributes</source>
        <translation>Atribūti</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1128"/>
        <source>Plugins</source>
        <translation>Spraudņi</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1251"/>
        <source>Ready</source>
        <translation>Gatavs</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2994"/>
        <source>Choose a filename to save the QGIS project file as</source>
        <translation>Izvēlieties nosaukumu ar kādu saglabāt QGIS projekta failu</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4276"/>
        <source>QGIS - Changes in SVN Since Last Release</source>
        <translation>QGIS - izmaiņas pašreizējas versijas sistēmā (SVN) kopš pēdējā laidiena</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="673"/>
        <source>Ctrl+?</source>
        <comment>Help Documentation (Mac)</comment>
        <translation>Ctrl+?</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="769"/>
        <source>Show most toolbars</source>
        <translation>Rādīt rīkjoslas</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="775"/>
        <source>Hide most toolbars</source>
        <translation>Slēpt rīkjoslas</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="875"/>
        <source>Cut Features</source>
        <translation>Izgriezt objektus</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="876"/>
        <source>Cut selected features</source>
        <translation>Izgriež izvēlētos objektus</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="880"/>
        <source>Copy Features</source>
        <translation>Kopēt objektus</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="881"/>
        <source>Copy selected features</source>
        <translation>Kopē izvēlētos objektus</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="885"/>
        <source>Paste Features</source>
        <translation>Ielītmēt objektus</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="886"/>
        <source>Paste selected features</source>
        <translation>Ielīmē izvēlētos objektus</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>
Compiled against Qt </source>
        <translation type="obsolete">
Kompilēts ar Qt </translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>, running against Qt </source>
        <translation type="obsolete">, darbināts ar Qt </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4308"/>
        <source>Network error while communicating with server</source>
        <translation>Tīkla kļūda komunicējot ar serveri</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4311"/>
        <source>Unknown network socket error</source>
        <translation>Nezināma tīkla ligzdas kļūda</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4316"/>
        <source>Unable to communicate with QGIS Version server</source>
        <translation>Nav iespējams sazināties ar QGIS versijas serveri</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="768"/>
        <source>T</source>
        <comment>Show most toolbars</comment>
        <translation>R</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="774"/>
        <source>Ctrl+T</source>
        <comment>Hide most toolbars</comment>
        <translation>Ctrl+R</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="358"/>
        <source>Checking provider plugins</source>
        <translation>Pārbaudam izplatītāja spraudņus</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="362"/>
        <source>Starting Python</source>
        <translation>Startējam Python</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="396"/>
        <source>Python console</source>
        <translation>Python konsole</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1762"/>
        <source>Python error</source>
        <translation>Python kļūda</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1762"/>
        <source>Error when reading metadata of plugin </source>
        <translation>Kļūda nolasot spraudņa metadatus </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3502"/>
        <source>Provider does not support deletion</source>
        <translation>Sniedzējs neuztur dzēšanu</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3503"/>
        <source>Data provider does not support deleting features</source>
        <translation>Datu sniedzējs neatbalsta objektu dzēšanu</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3509"/>
        <source>Layer not editable</source>
        <translation>Slānis nav rediģējams</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3510"/>
        <source>The current layer is not editable. Choose &apos;Start editing&apos; in the digitizing toolbar.</source>
        <translation>Aktīvais slānis ir nelabojams. Izvēlieties &quot;sākt labot&quot; no digitizēšanas rīkjoslas.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="810"/>
        <source>Toggle editing</source>
        <translation>Labošanas slēdzis</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="811"/>
        <source>Toggles the editing state of the current layer</source>
        <translation>Pārslēdz slāņa labošans statusu</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="866"/>
        <source>Add Ring</source>
        <translation>Pieveinot apli</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="870"/>
        <source>Add Island</source>
        <translation>Pievienot salu</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="871"/>
        <source>Add Island to multipolygon</source>
        <translation>Pievienot salu multipoligonam</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1145"/>
        <source>Toolbar Visibility...</source>
        <translation>Rīkjoslas redzamība...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1175"/>
        <source>Scale </source>
        <translation>Mērogs </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1190"/>
        <source>Current map scale (formatted as x:y)</source>
        <translation>Pašreizējais kartes mērogs (formēts kā x:y)</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1204"/>
        <source>Map coordinates at mouse cursor position</source>
        <translation>Kartes koordinātes peles atrašanās vietā</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3780"/>
        <source>Invalid scale</source>
        <translation>Nederīgs mērogs</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1203"/>
        <source>Shows the map coordinates at the current cursor position. The display is continuously updated as the mouse is moved.</source>
        <translation>Rāda pašreizējās peles kursora koordinātas. Tās tiek neprārtraukti atjauniātas, kad tiek kustināta pele.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="841"/>
        <source>Move Feature</source>
        <translation>Pārvietot objektu</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="846"/>
        <source>Split Features</source>
        <translation>Sadalīt objektus</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="891"/>
        <source>Map Tips</source>
        <translation>Kartes padomi</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="892"/>
        <source>Show information about a feature when the mouse is hovered over it</source>
        <translation>Rādīt informāciju par objektu, kad virs tā pārvieto peli.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1176"/>
        <source>Current map scale</source>
        <translation>Pašreizējais kartes mērogs</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5493"/>
        <source>Project file is older</source>
        <translation>Projekta fails ir vecāks</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5495"/>
        <source>&lt;p&gt;This project file was saved by an older version of QGIS.</source>
        <translation>&lt;p&gt;Šis projekta fails ir saglabāts ar vecāku QGIS versiju.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5497"/>
        <source> When saving this project file, QGIS will update it to the latest version, possibly rendering it useless for older versions of QGIS.</source>
        <translation> Saglabājot projektu, QGIS to atjauninās līdz jaunākajai versijai, kas, iespējams, nozīmēs, ka to vairs nebūs iespējams atvērt ar vecākām QGIS versijām.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5500"/>
        <source>&lt;p&gt;Even though QGIS developers try to maintain backwards compatibility, some of the information from the old project file might be lost.</source>
        <translation>&lt;p&gt;Lai arī QGIS izstrādātāji cenšas saglabāt atpakaļsavietojamību, tomēr, iespējams, ka daļa no vecā projekta faila informācijas tiks zaudēta.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5502"/>
        <source> To improve the quality of QGIS, we appreciate if you file a bug report at %3.</source>
        <translation> Lai uzlabotu QGIS kvalitāti, mēs priecāsimies, ja jūs iesniegsiet kļūdas ziņojumu iekš %3.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5504"/>
        <source> Be sure to include the old project file, and state the version of QGIS you used to discover the error.</source>
        <translation> Neaizmirstiet iekļaut veco projekta failu un norādīt QGIS versiju, kurā parādījās kļūda.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5506"/>
        <source>&lt;p&gt;To remove this warning when opening an older project file, uncheck the box &apos;%5&apos; in the %4 menu.</source>
        <translation>&lt;p&gt;Lai turpmāk vairs neredzētu šo paziņojumu atverot vecākus projekta failus, noņemiet iezīmi &apos;%5&apos; iekš %4 izvēlnes.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5506"/>
        <source>&lt;p&gt;Version of the project file: %1&lt;br&gt;Current version of QGIS: %2</source>
        <translation>&lt;p&gt;Projekta faila versija: %1&lt;br&gt;Pašreizējā QGIS versija: %2</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5510"/>
        <source>&lt;tt&gt;Settings:Options:General&lt;/tt&gt;</source>
        <comment>Menu path to setting options</comment>
        <translation>&lt;tt&gt;Iestatījumi:Opcijas:Vispārēji&lt;/tt&gt;</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5511"/>
        <source>Warn me when opening a project file saved with an older version of QGIS</source>
        <translation>Brīdināt mani atverot projekta failu, kas ir saglabāt ar vecāku QGIS versiju</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="636"/>
        <source>Toggle full screen mode</source>
        <translation>Pārslēgt pilnekrāna režīmu</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="637"/>
        <source>Ctrl-F</source>
        <comment>Toggle fullscreen mode</comment>
        <translation>Ctrl-F</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="638"/>
        <source>Toggle fullscreen mode</source>
        <translation>Pārslēdz pilnekrāna režīmu</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>This release candidate includes over 40 bug fixes and enchancements over the QGIS 0.9.1 release. In addition we have added the following new features:</source>
        <translation type="obsolete">Šajā laidiena kandidāta versijā ir iekļauti vairāk kā 40 kļūdu labojumi un uzlabojumi salīdzinājumā ar QGIS 0.9.1 laidienu. Papildus tam ir pievienoti sekojoši uzlabojumi:</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1633"/>
        <source>Imrovements to digitising capabilities.</source>
        <translation>Uzlabotas zīmēšanas iespējas.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1639"/>
        <source>Supporting default and defined styles (.qml) files for file based vector layers. With styles you can save the symbolisation and other settings associated with a vector layer and they will be loaded whenever you load that layer.</source>
        <translation>Noklusējuma un definēto stilu atbalsts failu bāzētiem vektordatiem (.qml). Ar stilu palīdzību ir iespējams asociēt stilus un simbolizāciju, kā arī citus iestatījumus, ar konkrētu vekrordatu slāni, kas tiks ielādēti katru reizi, kad pievienosiet slāni projektam.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1645"/>
        <source>Improved support for transparency and contrast stretching in raster layers. Support for color ramps in raster layers. Support for non-north up rasters. Many other raster improvements &apos;under the hood&apos;.</source>
        <translation>Uzlabots rastra datu caurspīdīgums un kontrasta uzlabošana. Krāsu karšu atbalsts, rotētu rastru atbalsts, kā arī daudzi citi rastru uzlabojumi.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1239"/>
        <source>Resource Location Error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1240"/>
        <source>Error reading icon resources from: 
 %1
 Quitting...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1375"/>
        <source>Map canvas. This is where raster and vectorlayers are displayed when added to the map</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1434"/>
        <source>Overview</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1468"/>
        <source>Legend</source>
        <translation type="unfinished">Leģenda</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1605"/>
        <source>You are using QGIS version %1 built against code revision %2.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1609"/>
        <source> This copy of QGIS has been built with PostgreSQL support.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1612"/>
        <source> This copy of QGIS has been built without PostgreSQL support.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1616"/>
        <source>
This binary was compiled against Qt %1,and is currently running against Qt %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1630"/>
        <source>This release candidate includes over 120 bug fixes and enchancements over the QGIS 0.9.1 release. In addition we have added the following new features:</source>
        <translation type="unfinished">Šajā laidiena kandidāta versijā ir iekļauti vairāk kā 40 kļūdu labojumi un uzlabojumi salīdzinājumā ar QGIS 0.9.1 laidienu. Papildus tam ir pievienoti sekojoši uzlabojumi: {120 ?} {0.9.1 ?}</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1648"/>
        <source>Updated icons for improved visual consistancy.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1651"/>
        <source>Support for migration of old projects to work in newer QGIS versions.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2865"/>
        <source></source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgisAppBase</name>
    <message>
        <location filename="../src/ui/qgisappbase.ui" line="13"/>
        <source>MainWindow</source>
        <translation>GalvenaisLogs</translation>
    </message>
    <message>
        <location filename="../src/ui/qgisappbase.ui" line="102"/>
        <source>Legend</source>
        <translation>Leģenda</translation>
    </message>
    <message>
        <location filename="../src/ui/qgisappbase.ui" line="135"/>
        <source>Map View</source>
        <translation>Kartes skats</translation>
    </message>
</context>
<context>
    <name>QgsAbout</name>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="13"/>
        <source>About Quantum GIS</source>
        <translation>Par Quantum GIS</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="230"/>
        <source>Ok</source>
        <translation>Labi</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="26"/>
        <source>About</source>
        <translation>Par</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&lt;h2&gt;Quantum GIS (qgis)&lt;/h2&gt;</source>
        <translation type="obsolete">&lt;h2&gt;Quantum GIS (qgis)&lt;/h2&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="129"/>
        <source>QGIS Home Page</source>
        <translation>QGIS mājaslapa</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Subscribe to the QGIS-User mailing list</source>
        <translation type="obsolete">Pieteikties QGIS lietotāju vēstkopai</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="65"/>
        <source>Version</source>
        <translation>Versija</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="152"/>
        <source>What&apos;s New</source>
        <translation>Kas jauns</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="189"/>
        <source>Providers</source>
        <translation>Sniedzēji</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="175"/>
        <source>Developers</source>
        <translation>Izstrādātāji</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&lt;h2&gt;QGIS Developers&lt;/h2&gt;</source>
        <translation type="obsolete">&lt;h2&gt; QGIS izstrādātāji &lt;/h2&gt;</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>QGIS Browser Selection</source>
        <translation type="obsolete">QGIS pārlūka izvēle</translation>
    </message>
    <message>
        <location filename="../src/app/qgsabout.cpp" line="114"/>
        <source>Name</source>
        <translation>Nosaukums</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="199"/>
        <source>Sponsors</source>
        <translation>Sponsori</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>QGIS Sponsors</source>
        <translation type="obsolete">QGIS sponsori</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>The following have sponsored QGIS by contributing money to fund development and other project costs</source>
        <translation type="obsolete">Tie, kas sponsorējuši QGIS ziedojot naudu, lai atbalstītu izstrādi un citas projekta izmaksas</translation>
    </message>
    <message>
        <location filename="../src/app/qgsabout.cpp" line="115"/>
        <source>Website</source>
        <translation>Mājaslapa</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Enter the name of a web browser to use (eg. konqueror).
Enter the full path if the browser is not in your PATH.
You can change this option later by selection Options from the Settings menu (Help Browser tab).</source>
        <translation type="obsolete">Ievadi lietojamā interneta pārlūka nosaukumu (piem. konqueror)
Ievadi pilno ceļu, ja pārlūks nav norādīts sitēmas PATH vērtībās. 
Šo izvēli vēlāk ir iespējams izmainīt pie QGIS iestatījumiem.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="91"/>
        <source>Quantum GIS is licensed under the GNU General Public License</source>
        <translation>Quantum GIS tiek izplatīts izmantojot GPL licenci</translation>
    </message>
    <message>
        <location filename="../src/app/qgsabout.cpp" line="112"/>
        <source>&lt;p&gt;The following have sponsored QGIS by contributing money to fund development and other project costs&lt;/p&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsabout.cpp" line="170"/>
        <source>Available QGIS Data Provider Plugins</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsabout.cpp" line="173"/>
        <source>Available Qt Database Plugins</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsabout.cpp" line="179"/>
        <source>Available Qt Image Plugins</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="50"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Lucida Grande&apos;; font-size:13pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:16px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:x-large; font-weight:600;&quot;&gt;&lt;span style=&quot; font-size:x-large;&quot;&gt;Quantum GIS (QGIS)&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="104"/>
        <source>http://www.gnu.org/licenses</source>
        <translation type="unfinished">http://www.gnu.org/licenses</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="139"/>
        <source>Join our user mailing list</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsAddAttrDialogBase</name>
    <message>
        <location filename="../src/ui/qgsaddattrdialogbase.ui" line="13"/>
        <source>Add Attribute</source>
        <translation>Pievienot atribūtu</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsaddattrdialogbase.ui" line="22"/>
        <source>Name:</source>
        <translation>Nosaukums:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsaddattrdialogbase.ui" line="35"/>
        <source>Type:</source>
        <translation>Tips:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>OK</source>
        <translation type="obsolete">Labi</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Cancel</source>
        <translation type="obsolete">Atcelt</translation>
    </message>
</context>
<context>
    <name>QgsAttributeActionDialog</name>
    <message>
        <location filename="" line="135533324"/>
        <source>Name</source>
        <translation type="obsolete">Nosaukums</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Action</source>
        <translation type="obsolete">Darbība</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Capture</source>
        <translation type="obsolete">Tvert</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributeactiondialog.cpp" line="150"/>
        <source>Select an action</source>
        <comment>File dialog window title</comment>
        <translation>Izvēlieties darbību</translation>
    </message>
</context>
<context>
    <name>QgsAttributeActionDialogBase</name>
    <message>
        <location filename="" line="135533324"/>
        <source>Form1</source>
        <translation type="obsolete">Forma1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="144"/>
        <source>This list contains all actions that have been defined for the current layer. Add actions by entering the details in the controls below and then pressing the Insert action button. Actions can be edited here by double clicking on the item.</source>
        <translation>Šis saraksts satur visas darbības, kas ir definētas pašreizējam slānim. Jaunas darbības var pievienot norādot to parametrus zemāk redzamajos lodziņos un nospiežot pievienošanas pogu. Eksistējošās darbības var tikt labotas ar dubultklikšķi uz tās nosaukuma.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="214"/>
        <source>Move up</source>
        <translation>Pārvietot uz augšu</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="211"/>
        <source>Move the selected action up</source>
        <translation>Pārvieto izvēlēto darbību uz augšu</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="224"/>
        <source>Move down</source>
        <translation>Pārvietot uz leju</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="221"/>
        <source>Move the selected action down</source>
        <translation>Pārvieto izvēlēto darbību uz leju</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="250"/>
        <source>Remove</source>
        <translation>Noņemt</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="247"/>
        <source>Remove the selected action</source>
        <translation>Noņem izvēlēto darbību</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Name:</source>
        <translation type="obsolete">Nosaukums:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="59"/>
        <source>Enter the name of an action here. The name should be unique (qgis will make it unique if necessary).</source>
        <translation>Šeit ievadiet darbības nosaukumu. Nosaukumam vajadzētu būt unikālam (QGIS padarīs to unikālu, ja tas būs vajadzīgs).</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="56"/>
        <source>Enter the action name here</source>
        <translation>Šeit ievadiet darbības nosaukumu</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Action:</source>
        <translation type="obsolete">Darbība:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="79"/>
        <source>Enter the action command here</source>
        <translation>Šeit ievadiet darbības komandu</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Browse</source>
        <translation type="obsolete">Pārlūkot</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Browse for action commands</source>
        <translation type="obsolete">Pārlūkot darbību komandas</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="178"/>
        <source>Insert action</source>
        <translation>Ievietot darbību</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="175"/>
        <source>Inserts the action into the list above</source>
        <translation>Ievieto darbību sarakstā</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="188"/>
        <source>Update action</source>
        <translation>Atjaunot darbību</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="185"/>
        <source>Update the selected action</source>
        <translation>Atjauno izvēlēto darbību</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="115"/>
        <source>Insert field</source>
        <translation>Ievietot lauku</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="112"/>
        <source>Inserts the selected field into the action, prepended with a %</source>
        <translation>Ievieto izvēlēto lauku darbībā un priekšā pieliek %</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="105"/>
        <source>The valid attribute names for this layer</source>
        <translation>Derīgi atribūtu nosaukumi šim slānim</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="128"/>
        <source>Capture output</source>
        <translation>Tvert izvadi</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="122"/>
        <source>Captures any output from the action</source>
        <translation>Tver jebkādu darbības izvadi</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="125"/>
        <source>Captures the standard output or error generated by the action and displays it in a dialog box</source>
        <translation>Tver programmas standarta izvadi vai kļūdu paziņojumus un parāda to</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="82"/>
        <source>Enter the action here. This can be any program, script or command that is available on your system. When the action is invoked any set of characters that start with a % and then have the name of a field will be replaced by the value of that field. The special characters %% will be replaced by the value of the field that was selected. Double quote marks group text into single arguments to the program, script or command. Double quotes will be ignored if preceeded by a backslash</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="19"/>
        <source>Attribute Actions</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="37"/>
        <source>Action properties</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="157"/>
        <source>Name</source>
        <translation type="unfinished">Nosaukums</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="162"/>
        <source>Action</source>
        <translation type="unfinished">Darbība</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="89"/>
        <source>Browse for action</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="92"/>
        <source>Click to browse for an action</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="95"/>
        <source>Clicking the buttone will let you select an application to use as the action</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="98"/>
        <source>...</source>
        <translation type="unfinished">...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="167"/>
        <source>Capture</source>
        <translation type="unfinished">Tvert</translation>
    </message>
</context>
<context>
    <name>QgsAttributeDialog</name>
    <message>
        <location filename="../src/app/qgsattributedialog.cpp" line="76"/>
        <source> (int)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributedialog.cpp" line="81"/>
        <source> (dbl)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributedialog.cpp" line="86"/>
        <source> (txt)</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsAttributeDialogBase</name>
    <message>
        <location filename="../src/ui/qgsattributedialogbase.ui" line="13"/>
        <source>Enter Attribute Values</source>
        <translation>Ievadiet atribūtu vērtības</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>1</source>
        <translation type="obsolete">1</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Attribute</source>
        <translation type="obsolete">Atribūts</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Value</source>
        <translation type="obsolete">Vērtība</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&amp;OK</source>
        <translation type="obsolete">&amp;Labi</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&amp;Cancel</source>
        <translation type="obsolete">&amp;Atcelt</translation>
    </message>
</context>
<context>
    <name>QgsAttributeTable</name>
    <message>
        <location filename="../src/app/qgsattributetable.cpp" line="340"/>
        <source>Run action</source>
        <translation>Startēt darbību</translation>
    </message>
</context>
<context>
    <name>QgsAttributeTableBase</name>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="13"/>
        <source>Attribute Table</source>
        <translation>Atribūtu tabula</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="167"/>
        <source>Start editing</source>
        <translation>Sākt rediģēt</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&amp;Close</source>
        <translation type="obsolete">&amp;Aizvērt</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Alt+C</source>
        <translation type="obsolete">Alt+C</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="144"/>
        <source>Ctrl+X</source>
        <translation>Ctrl+X</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="128"/>
        <source>Ctrl+N</source>
        <translation>Ctrl+N</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="71"/>
        <source>Ctrl+S</source>
        <translation>Ctrl+S</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="62"/>
        <source>Invert selection</source>
        <translation>Pretēja izvēle</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="55"/>
        <source>Ctrl+T</source>
        <translation>Ctrl+T</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="46"/>
        <source>Move selected to top</source>
        <translation>Pārvietot izvēli uz pašu augšu</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="30"/>
        <source>Remove selection</source>
        <translation>Aizvākt izvēli</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="78"/>
        <source>Copy selected rows to clipboard (Ctrl+C)</source>
        <translation>Kopēt atlasītās rindas uz starpliktuvi (Ctrl+C)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="81"/>
        <source>Copies the selected rows to the clipboard</source>
        <translation>Kopē atlasītās rindas uz starpliktuvi</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="90"/>
        <source>Ctrl+C</source>
        <translation>Ctrl+C</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="174"/>
        <source>Stop editin&amp;g</source>
        <translation>&amp;Beigt rediģēt</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="177"/>
        <source>Alt+G</source>
        <translation>Alt+G</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Search for:</source>
        <translation type="obsolete">Meklēt:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="211"/>
        <source>in</source>
        <translation>iekš</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="231"/>
        <source>Search</source>
        <translation>Meklēt</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="241"/>
        <source>Adva&amp;nced...</source>
        <translation>Paplaši&amp;nāti...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="244"/>
        <source>Alt+N</source>
        <translation>Alt+N</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&amp;Help</source>
        <translation type="obsolete">&amp;Palīdzība</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="119"/>
        <source>New column</source>
        <translation>Jauna kolonna</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="135"/>
        <source>Delete column</source>
        <translation>Dzēst kolonnu</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="97"/>
        <source>Zoom map to the selected rows (Ctrl-F)</source>
        <translation>Tuvināt kartes skatu uz izvēlētajiem ierakstiem (Ctrl+F)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="100"/>
        <source>Zoom map to the selected rows</source>
        <translation>Tuvināt kartes skatu uz izvēlētajiem ierakstiem</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="106"/>
        <source>Ctrl+F</source>
        <translation>Ctrl+F</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="198"/>
        <source>Search for</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsAttributeTableDisplay</name>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="98"/>
        <source>select</source>
        <translation>izvēlēties</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="99"/>
        <source>select and bring to top</source>
        <translation>izvēlēties un pārvietot uz augšu</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="100"/>
        <source>show only matching</source>
        <translation>rādīt tikai atbilstošos</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="330"/>
        <source>Search string parsing error</source>
        <translation>Meklēšanas virknes parsēšanas kļūda</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="382"/>
        <source>Search results</source>
        <translation>Meklēšanas rezultāti</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="336"/>
        <source>You&apos;ve supplied an empty search string.</source>
        <translation>Jūs esat norādījis tukšu meklēšanas virkni.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="370"/>
        <source>Error during search</source>
        <translation>Meklēšanas kļūda</translation>
    </message>
    <message numerus="yes">
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="379"/>
        <source>Found %d matching features.</source>
        <translation type="unfinished">
            <numerusform>Atrasti %d atbilstoši objekti.</numerusform>
        </translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="381"/>
        <source>No matching features found.</source>
        <translation>Neviens atbilstošs objekts netika atrasts.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="148"/>
        <source>Name conflict</source>
        <translation>Nosaukumu konflikts</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="197"/>
        <source>Stop editing</source>
        <translation>Beigt labošanau</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="198"/>
        <source>Do you want to save the changes?</source>
        <translation>Vai Jūs velaties saglabāt izmaiņas?</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="204"/>
        <source>Error</source>
        <translation>Kļūda</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="148"/>
        <source>The attribute could not be inserted. The name already exists in the table.</source>
        <translation>Nebija iespējams pievienot atribūtu. Tāds nosaukums jau eksistē tabulā.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="204"/>
        <source>Could not commit changes - changes are still pending</source>
        <translation>Nebija iespējams apstiprināt izmaiņas - izmaiņas vēl ir neapstiprinātas</translation>
    </message>
</context>
<context>
    <name>QgsBookmarks</name>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="160"/>
        <source>Really Delete?</source>
        <translation>Tiešām dzēst?</translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="161"/>
        <source>Are you sure you want to delete the </source>
        <translation>Vai Jūs tiešām vēlaties dzēst  </translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="161"/>
        <source> bookmark?</source>
        <translation>  grāmatzīmi?</translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="177"/>
        <source>Error deleting bookmark</source>
        <translation>Kļūda dzēšot grāmatzīmi</translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="179"/>
        <source>Failed to delete the </source>
        <translation>Neizdevās dzēst </translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="181"/>
        <source> bookmark from the database. The database said:
</source>
        <translation> grāmatzīmi no datu bāzes. Datu bāze atbildēja:
</translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="60"/>
        <source>&amp;Delete</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="61"/>
        <source>&amp;Zoom to</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsBookmarksBase</name>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="13"/>
        <source>Geospatial Bookmarks</source>
        <translation>Ģeogrāfiskās grāmatzīmes</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="29"/>
        <source>Name</source>
        <translation>Nosaukums</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="34"/>
        <source>Project</source>
        <translation>Projekts</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="39"/>
        <source>Extent</source>
        <translation>Apjoms</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="44"/>
        <source>Id</source>
        <translation>Id</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Zoom To</source>
        <translation type="obsolete">Tuvināt līdz</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Zoom to the currently selected bookmark</source>
        <translation type="obsolete">Tuvināt līdz izvēlētajai grāmatzīmei</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Delete</source>
        <translation type="obsolete">Dzēst</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Delete the currently selected bookmark</source>
        <translation type="obsolete">Dzēst izvēlēto grāmatzīmi</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Close</source>
        <translation type="obsolete">Aizvērt</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Close the dialog</source>
        <translation type="obsolete">Aizvērt šo dialogu</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Help</source>
        <translation type="obsolete">Palīdzība</translation>
    </message>
</context>
<context>
    <name>QgsComposer</name>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="472"/>
        <source> for read/write</source>
        <translation> lasīšanai/rakstīšanai</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="688"/>
        <source>Choose a filename to save the map image as</source>
        <translation>Izvēlieties faila nosaukumu ar kādu saglabāt kartes attēlu</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="783"/>
        <source>Choose a filename to save the map as</source>
        <translation>Izvēlieties faila nosaukumu ar kādu saglabāt karti</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="551"/>
        <source>Error in Print</source>
        <translation>Kļūda drukājot</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="529"/>
        <source>Cannot seek</source>
        <translation>Nevar meklēt</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="454"/>
        <source>Cannot overwrite BoundingBox</source>
        <translation>Nevar pārrakstīt ietverošo robežu taisnstūri</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="462"/>
        <source>Cannot find BoundingBox</source>
        <translation>Nevar atrast ietverošo robežu taisnstūri</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="545"/>
        <source>Cannot overwrite translate</source>
        <translation>Nevar pārrakstīt tulkojumu</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="551"/>
        <source>Cannot find translate</source>
        <translation>Nevar atrast tulkojumu</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="559"/>
        <source>File IO Error</source>
        <translation>Faila IO kļūda</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="569"/>
        <source>Paper does not match</source>
        <translation>Papīrs neatbilst</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="570"/>
        <source>The selected paper size does not match the composition size</source>
        <translation>Izvēlētā papīra izmērs neatbilst salikuma izmēram</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="634"/>
        <source>Big image</source>
        <translation>Liels attēls</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="635"/>
        <source>To create image </source>
        <translation>Lai izveidotu attēlu, </translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="638"/>
        <source> requires circa </source>
        <translation> ir nepieciešams aptuveni </translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="638"/>
        <source> MB of memory</source>
        <translation> Mb atmiņas</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="56"/>
        <source>QGIS - print composer</source>
        <translation>QGIS - drukas salicējs</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="81"/>
        <source>Map 1</source>
        <translation>Karte 1</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="472"/>
        <source>Couldn&apos;t open </source>
        <translation>Nebija iespējams atvērt </translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="665"/>
        <source>format</source>
        <translation>formātā</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="757"/>
        <source>SVG warning</source>
        <translation>SVG brīdinājums</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="758"/>
        <source>Don&apos;t show this message again</source>
        <translation>Turpmāk nerādīt šo paziņojumu</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="784"/>
        <source>SVG Format</source>
        <translation>SVG formāts</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="764"/>
        <source>&lt;p&gt;The SVG export function in Qgis has several problems due to bugs and deficiencies in the </source>
        <translation>&lt;p&gt;QGIS SVG eksportēšanas funkcijai ir vairākas kļūdas dēļ nepilnībām un kļūdām iekš </translation>
    </message>
</context>
<context>
    <name>QgsComposerBase</name>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="62"/>
        <source>General</source>
        <translation>Vispārējs</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="107"/>
        <source>Composition</source>
        <translation>Kompozīcija</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="115"/>
        <source>Item</source>
        <translation>Vienums</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="208"/>
        <source>&amp;Open Template ...</source>
        <translation>&amp;Atvērt šablonu...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="216"/>
        <source>Save Template &amp;As...</source>
        <translation>Saglabāt šablonu &amp;kā...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="224"/>
        <source>&amp;Print...</source>
        <translation>&amp;Drunāt...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="256"/>
        <source>Add new map</source>
        <translation>Pievienot jaunu karti</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="264"/>
        <source>Add new label</source>
        <translation>Pievienot jaunu birku</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="272"/>
        <source>Add new vect legend</source>
        <translation>Pievieno jaunu vektordatu leģendu</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="280"/>
        <source>Select/Move item</source>
        <translation>Izvēlēties/pārvietot vienību</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="288"/>
        <source>Export as image</source>
        <translation>Eksportēt kā attēlu</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="296"/>
        <source>Export as SVG</source>
        <translation>Eksportēt kā SVG</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="304"/>
        <source>Add new scalebar</source>
        <translation>Pievienot jaunu mērogjoslu</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="312"/>
        <source>Refresh view</source>
        <translation>Atjaunināt skatu</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="13"/>
        <source>MainWindow</source>
        <translation>GalvenaisLogs</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="232"/>
        <source>Zoom All</source>
        <translation>Tuvināt kopskatu</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="240"/>
        <source>Zoom In</source>
        <translation>Tuvināt</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="248"/>
        <source>Zoom Out</source>
        <translation>Tālināt</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="320"/>
        <source>Add Image</source>
        <translation>Pievienot attēlu</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="170"/>
        <source>Close</source>
        <translation>Aizvērt</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="150"/>
        <source>Help</source>
        <translation>Palīdzība</translation>
    </message>
</context>
<context>
    <name>QgsComposerLabelBase</name>
    <message>
        <location filename="../src/ui/qgscomposerlabelbase.ui" line="21"/>
        <source>Label Options</source>
        <translation>Birkas opcijas</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlabelbase.ui" line="48"/>
        <source>Font</source>
        <translation>Fonts</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlabelbase.ui" line="55"/>
        <source>Box</source>
        <translation>Rāmis</translation>
    </message>
</context>
<context>
    <name>QgsComposerMap</name>
    <message>
        <location filename="../src/app/composer/qgscomposermap.cpp" line="77"/>
        <source>Map %1</source>
        <translation>Karte %1</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposermap.cpp" line="99"/>
        <source>Extent (calculate scale)</source>
        <translation>Kopskats (aprēķināt mērogu)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposermap.cpp" line="100"/>
        <source>Scale (calculate extent)</source>
        <translation>Mērogs (aprēķināt kopskatu)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposermap.cpp" line="107"/>
        <source>Cache</source>
        <translation>Kešatmiņa</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposermap.cpp" line="108"/>
        <source>Render</source>
        <translation>Renderēt</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposermap.cpp" line="109"/>
        <source>Rectangle</source>
        <translation>Taisnstūris</translation>
    </message>
</context>
<context>
    <name>QgsComposerMapBase</name>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="21"/>
        <source>Map options</source>
        <translation>Kartes īpašības</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="173"/>
        <source>&lt;b&gt;Map&lt;/b&gt;</source>
        <translation>&lt;b&gt;Karte&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="147"/>
        <source>Set</source>
        <translation>Iestatīt</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="196"/>
        <source>Width</source>
        <translation>Platums</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="180"/>
        <source>Height</source>
        <translation>Augstums</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="72"/>
        <source>Set Extent</source>
        <translation>Iestatīt skatu</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="69"/>
        <source>Set map extent to current extent in QGIS map canvas</source>
        <translation>Pielāgot kartes robežas pašreizējam QGIS kartes skatam</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="212"/>
        <source>Line width scale</source>
        <translation>Līnijas platuma mērogs</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="116"/>
        <source>Width of one unit in millimeters</source>
        <translation>Vienas vienības platums milimetros</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="225"/>
        <source>Symbol scale</source>
        <translation>Simbolu mērogs</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="238"/>
        <source>Font size scale</source>
        <translation>Fontu izmēra mērogs</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="251"/>
        <source>Frame</source>
        <translation>Rāmis</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="258"/>
        <source>Preview</source>
        <translation>Priekšapskate</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="79"/>
        <source>1:</source>
        <translation>1:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="97"/>
        <source>Scale:</source>
        <translation>Mērogs:</translation>
    </message>
</context>
<context>
    <name>QgsComposerPicture</name>
    <message>
        <location filename="../src/app/composer/qgscomposerpicture.cpp" line="399"/>
        <source>Warning</source>
        <translation>Brīdinājums</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposerpicture.cpp" line="400"/>
        <source>Cannot load picture.</source>
        <translation>Nevar ielādēt bildi.</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposerpicture.cpp" line="483"/>
        <source>Choose a file</source>
        <translation>Izvēlieties failu</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposerpicture.cpp" line="466"/>
        <source>Pictures (</source>
        <translation>Bildes (</translation>
    </message>
</context>
<context>
    <name>QgsComposerPictureBase</name>
    <message>
        <location filename="../src/ui/qgscomposerpicturebase.ui" line="21"/>
        <source>Picture Options</source>
        <translation>Bildes opcijas</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerpicturebase.ui" line="197"/>
        <source>Frame</source>
        <translation>Rāmis</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerpicturebase.ui" line="161"/>
        <source>Angle</source>
        <translation>Leņķis</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerpicturebase.ui" line="119"/>
        <source>Width</source>
        <translation>Platums</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerpicturebase.ui" line="140"/>
        <source>Height</source>
        <translation>Augstums</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerpicturebase.ui" line="58"/>
        <source>Browse</source>
        <translation>Pārlūkot</translation>
    </message>
</context>
<context>
    <name>QgsComposerScalebarBase</name>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="21"/>
        <source>Barscale Options</source>
        <translation>Mērogjoslas parametri</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="140"/>
        <source>Segment size</source>
        <translation>Segmenta izmērs</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="172"/>
        <source>Number of segments</source>
        <translation>Segmentu skaits</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="159"/>
        <source>Map units per scalebar unit</source>
        <translation>Kartes vienības uz mērogjoslas vienību</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="88"/>
        <source>Unit label</source>
        <translation>Vienību birka</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="127"/>
        <source>Map</source>
        <translation>Karte</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="195"/>
        <source>Font</source>
        <translation>Fonts</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="41"/>
        <source>Line width</source>
        <translation>Līnijas platums</translation>
    </message>
</context>
<context>
    <name>QgsComposerVectorLegend</name>
    <message>
        <location filename="" line="135533324"/>
        <source>Layers</source>
        <translation type="obsolete">Slāņi</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Group</source>
        <translation type="obsolete">Grupa</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposervectorlegend.cpp" line="718"/>
        <source>Combine selected layers</source>
        <translation>Kombinēt izvēlētos slāņus</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposervectorlegend.cpp" line="134"/>
        <source>Cache</source>
        <translation>Kešatmiņa</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposervectorlegend.cpp" line="135"/>
        <source>Render</source>
        <translation>Renderēt</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposervectorlegend.cpp" line="136"/>
        <source>Rectangle</source>
        <translation>Taisnstūris</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposervectorlegend.cpp" line="110"/>
        <source>Legend</source>
        <translation>Leģenda</translation>
    </message>
</context>
<context>
    <name>QgsComposerVectorLegendBase</name>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="21"/>
        <source>Vector Legend Options</source>
        <translation>Vektordatu leģendas īpašības</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="113"/>
        <source>Title</source>
        <translation>Virsraksts</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="92"/>
        <source>Map</source>
        <translation>Karte</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="182"/>
        <source>Font</source>
        <translation>Fonts</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="167"/>
        <source>Box</source>
        <translation>Kaste</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Column 1</source>
        <translation type="obsolete">Kolonna 1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="53"/>
        <source>Preview</source>
        <translation>Priekšapskate</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="149"/>
        <source>Layers</source>
        <translation type="unfinished">Slāņi</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="154"/>
        <source>Group</source>
        <translation type="unfinished">Grupa</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="159"/>
        <source>ID</source>
        <translation type="unfinished">ID</translation>
    </message>
</context>
<context>
    <name>QgsComposition</name>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="88"/>
        <source>Custom</source>
        <translation>Pielāgots</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="89"/>
        <source>A5 (148x210 mm)</source>
        <translation>A5 (148x210 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="90"/>
        <source>A4 (210x297 mm)</source>
        <translation>A4 (210x297 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="91"/>
        <source>A3 (297x420 mm)</source>
        <translation>A3 (297x420 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="92"/>
        <source>A2 (420x594 mm)</source>
        <translation>A2 (420x594 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="93"/>
        <source>A1 (594x841 mm)</source>
        <translation>A1 (594x841 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="94"/>
        <source>A0 (841x1189 mm)</source>
        <translation>A0 (841x1189 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="95"/>
        <source>B5 (176 x 250 mm)</source>
        <translation>B5 (176 x 250 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="96"/>
        <source>B4 (250 x 353 mm)</source>
        <translation>B4 (250 x 353 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="97"/>
        <source>B3 (353 x 500 mm)</source>
        <translation>B3 (353 x 500 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="98"/>
        <source>B2 (500 x 707 mm)</source>
        <translation>B2 (500 x 707 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="99"/>
        <source>B1 (707 x 1000 mm)</source>
        <translation>B1 (707 x 1000 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="100"/>
        <source>B0 (1000 x 1414 mm)</source>
        <translation>B0 (1000 x 1414 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="101"/>
        <source>Letter (8.5x11 inches)</source>
        <translation>Letter (8.5x11 inches)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="102"/>
        <source>Legal (8.5x14 inches)</source>
        <translation>Legal (8.5x14 inches)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="115"/>
        <source>Portrait</source>
        <translation>Portrets</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="116"/>
        <source>Landscape</source>
        <translation>Ainava</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="632"/>
        <source>Out of memory</source>
        <translation>Nepietiek atmiņas</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="635"/>
        <source>Qgis is unable to resize the paper size due to insufficient memory.
 It is best that you avoid using the map composer until you restart qgis.
</source>
        <translation>QGIS nebija iespējams izmainīt papīra izmēru, jo tam nepietika atmiņas.
 Ir ieteicams vairs nelietot karšu veidotāju līdz QGIS programmas pārstartēšanai.
</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="779"/>
        <source>Label</source>
        <translation>Birka</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="829"/>
        <source>Warning</source>
        <translation>Brīdinājums</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="830"/>
        <source>Cannot load picture.</source>
        <translation>Nevar ielādēt bildi.</translation>
    </message>
</context>
<context>
    <name>QgsCompositionBase</name>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="21"/>
        <source>Composition</source>
        <translation>Kompozīcija</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="33"/>
        <source>Paper</source>
        <translation>Papīrs</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="176"/>
        <source>Size</source>
        <translation>Izmērs</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="158"/>
        <source>Units</source>
        <translation>Vienības</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="140"/>
        <source>Width</source>
        <translation>Platums</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="122"/>
        <source>Height</source>
        <translation>Augstums</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="104"/>
        <source>Orientation</source>
        <translation>Orientācija</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="213"/>
        <source>Resolution (dpi)</source>
        <translation>Izšķirtspēja (dpi)</translation>
    </message>
</context>
<context>
    <name>QgsConnectionDialog</name>
    <message>
        <location filename="" line="135533324"/>
        <source>Test connection</source>
        <translation type="obsolete">Testēt savienojumu</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Connection to </source>
        <translation type="obsolete">Savienojums ar </translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source> was successfull</source>
        <translation type="obsolete"> bija sekmīgs</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Connection failed - Check settings and try again </source>
        <translation type="obsolete">Savienojums neizdevās - Pārbaudiet iestatījumus un mēģiniet vēlreiz </translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>General Interface Help:

</source>
        <translation type="obsolete">Vispārēja saskarnes palīdzība:

</translation>
    </message>
</context>
<context>
    <name>QgsConnectionDialogBase</name>
    <message>
        <location filename="" line="135533324"/>
        <source>Create a New PostGIS connection</source>
        <translation type="obsolete">Izveido jaunu PostGIS savienojumu</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Connection Information</source>
        <translation type="obsolete">Savienojuma informācija</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Name</source>
        <translation type="obsolete">Nosaukums</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Host</source>
        <translation type="obsolete">Hosts</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Database</source>
        <translation type="obsolete">Datu bāze</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Port</source>
        <translation type="obsolete">Ports</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Username</source>
        <translation type="obsolete">Lietotājvārds</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Password</source>
        <translation type="obsolete">Parole</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Name of the new connection</source>
        <translation type="obsolete">Jaunā savienojuma nosaukums</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>5432</source>
        <translation type="obsolete">5432</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Save Password</source>
        <translation type="obsolete">Saglabāt paroli</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Test Connect</source>
        <translation type="obsolete">Testēt savienojumu</translation>
    </message>
</context>
<context>
    <name>QgsContinuousColorDialogBase</name>
    <message>
        <location filename="../src/ui/qgscontinuouscolordialogbase.ui" line="13"/>
        <source>Continuous color</source>
        <translation>Vienlaidus krāsa</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscontinuouscolordialogbase.ui" line="54"/>
        <source>Maximum Value:</source>
        <translation>Maksimālā vērtība:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscontinuouscolordialogbase.ui" line="80"/>
        <source>Outline Width:</source>
        <translation>Kontūrlīnijas platums:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscontinuouscolordialogbase.ui" line="93"/>
        <source>Minimum Value:</source>
        <translation>Minimālā vērtība:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscontinuouscolordialogbase.ui" line="109"/>
        <source>Classification Field:</source>
        <translation>Klasifikācijas lauks:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscontinuouscolordialogbase.ui" line="119"/>
        <source>Draw polygon outline</source>
        <translation>Zīmēt poligona kontūru</translation>
    </message>
</context>
<context>
    <name>QgsCoordinateTransform</name>
    <message>
        <location filename="../src/core/qgscoordinatetransform.cpp" line="483"/>
        <source>Failed</source>
        <translation>Neveiksmīgs</translation>
    </message>
    <message>
        <location filename="../src/core/qgscoordinatetransform.cpp" line="483"/>
        <source>transform of</source>
        <translation>transformēšana no</translation>
    </message>
    <message>
        <location filename="../src/core/qgscoordinatetransform.cpp" line="496"/>
        <source>with error: </source>
        <translation>ar kļūdu: </translation>
    </message>
    <message>
        <location filename="../src/core/qgscoordinatetransform.cpp" line="418"/>
        <source>The source spatial reference system (SRS) is not valid. </source>
        <translation>Avota telpisko norāžu sistēma (SRS) nav derīga. </translation>
    </message>
    <message>
        <location filename="../src/core/qgscoordinatetransform.cpp" line="426"/>
        <source>The coordinates can not be reprojected. The SRS is: </source>
        <translation>Koordinātu sistēma nevar tikt pārprojecēta. SRS ir: </translation>
    </message>
    <message>
        <location filename="../src/core/qgscoordinatetransform.cpp" line="425"/>
        <source>The destination spatial reference system (SRS) is not valid. </source>
        <translation>Mērķa telpisko norāžu sistēma (SRS) nav derīga. </translation>
    </message>
</context>
<context>
    <name>QgsCopyrightLabelPlugin</name>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="66"/>
        <source>Bottom Left</source>
        <translation>Apakšējais kreisais</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="67"/>
        <source>Top Left</source>
        <translation>Augšējais kreisais</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="67"/>
        <source>Top Right</source>
        <translation>Augšējais labais</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="204"/>
        <source>&amp;Decorations</source>
        <translation>&amp;Dekorācijas</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="80"/>
        <source>Creates a copyright label that is displayed on the map canvas.</source>
        <translation>Izveido autortiesību birku, ko attēlot uz kartes.</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="67"/>
        <source>Bottom Right</source>
        <translation>Apakšējais labais</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="79"/>
        <source>&amp;Copyright Label</source>
        <translation>&amp;Autortiesību birka</translation>
    </message>
</context>
<context>
    <name>QgsCopyrightLabelPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="13"/>
        <source>Copyright Label Plugin</source>
        <translation>Autortiesību birkas spraudnis</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="145"/>
        <source>Placement</source>
        <translation>Novietojums</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="153"/>
        <source>Bottom Left</source>
        <translation>Apakšējais kreisais</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="158"/>
        <source>Top Left</source>
        <translation>Augšējais kreisais</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="163"/>
        <source>Bottom Right</source>
        <translation>Apakšējais labais</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="168"/>
        <source>Top Right</source>
        <translation>Augšējais labais</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="176"/>
        <source>Orientation</source>
        <translation>Orientācija</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="184"/>
        <source>Horizontal</source>
        <translation>Horizontāls</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="189"/>
        <source>Vertical</source>
        <translation>Vertikāls</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="118"/>
        <source>Enable Copyright Label</source>
        <translation>Ieslēgt autortiesību spraudni</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="36"/>
        <source>Color</source>
        <translation>Krāsa</translation>
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
        <location filename="" line="135533324"/>
        <source>&#xa9; QGIS 2008</source>
        <translation type="obsolete">© QGIS 2008</translation>
    </message>
    <message encoding="UTF-8">
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="130"/>
        <source>© QGIS 2008</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsCustomProjectionDialog</name>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="175"/>
        <source>Delete Projection Definition?</source>
        <translation>Dzēst projekcijas informāciju?</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="176"/>
        <source>Deleting a projection definition is not reversable. Do you want to delete it?</source>
        <translation>Projekcijas informācijas dzēšana ir neatgriezeniska darbība. Vai Jūs tiešām vēlaties to dzēst?</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="882"/>
        <source>Abort</source>
        <translation>Atcelt</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="884"/>
        <source>New</source>
        <translation>Jauna</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="939"/>
        <source>QGIS Custom Projection</source>
        <translation>QGIS pielāgotā projekcija</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="756"/>
        <source>This proj4 projection definition is not valid. Please give the projection a name before pressing save.</source>
        <translation>Šī proj4 projekcijas definīcija ir nepareiza. Lūdzu ievadiet projekcijas nosaukumu pirms saglabāšanas.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="762"/>
        <source>This proj4 projection definition is not valid. Please add the parameters before pressing save.</source>
        <translation>Šī proj4 projekcijas definīcija ir nepareiza. Lūdzu pievienojiet parametrus pirms saglabāšanas.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="777"/>
        <source>This proj4 projection definition is not valid. Please add a proj= clause before pressing save.</source>
        <translation>Šī proj4 projekcijas definīcija ir nepareiza. Lūdzu pievienojiet proj= sadaļu pirms saglabāšanas.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="784"/>
        <source>This proj4 ellipsoid definition is not valid. Please add a ellips= clause before pressing save.</source>
        <translation>Šī proj4 elipsoīda definīcija nav pareiza. Lūdzu pievienjiet ellips= sadaļu pirms saglabāšanas.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="800"/>
        <source>This proj4 projection definition is not valid. Please correct before pressing save.</source>
        <translation>Šī proj4 projekcijas definīcija ir nepareiza. Lūdzu izlabojiet to pirms saglabāšanas.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="913"/>
        <source>This proj4 projection definition is not valid.</source>
        <translation>Šī proj4 projekcijas definīcija nav pareiza.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="928"/>
        <source>Northing and Easthing must be in decimal form.</source>
        <translation>Ziemeļiem un Austrumiem ir jābūt decimālajā formā.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="940"/>
        <source>Internal Error (source projection invalid?)</source>
        <translation>Iekšēja kļūda (nepareiza avota projekcija?)</translation>
    </message>
</context>
<context>
    <name>QgsCustomProjectionDialogBase</name>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="13"/>
        <source>Custom Projection Definition</source>
        <translation>Pielāgota projekcijas informācija</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="22"/>
        <source>Define</source>
        <translation>Definēt</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Parameters:</source>
        <translation type="obsolete">Parametri:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="66"/>
        <source>|&lt;</source>
        <translation>|&lt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="76"/>
        <source>&lt;</source>
        <translation>&lt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="86"/>
        <source>1 of 1</source>
        <translation>1 no 1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="96"/>
        <source>&gt;</source>
        <translation>&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="106"/>
        <source>&gt;|</source>
        <translation>&gt;|</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>New</source>
        <translation type="obsolete">Jauns</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Save</source>
        <translation type="obsolete">Saglabāt</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Delete</source>
        <translation type="obsolete">Dzēst</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Close</source>
        <translation type="obsolete">Aizvērt</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Name:</source>
        <translation type="obsolete">Nosaukums:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="28"/>
        <source>You can define your own custom projection here. The definition must conform to the proj4 format for specifying a Spatial Reference System.</source>
        <translation>Šeit Jūs varat definēt savu pielāgoto projekciju. Definīcijai ir jābūt proj4 formātā.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="151"/>
        <source>Test</source>
        <translation>Testēt</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Transform from WGS84 to the chosen projection</source>
        <translation type="obsolete">Transformēt no WGS84 uz izvēlēto projekciju</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="253"/>
        <source>Calculate</source>
        <translation>Aprēķināt</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="180"/>
        <source>Geographic / WGS84</source>
        <translation>Ģeogrāfisks / WGS84</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>North:</source>
        <translation type="obsolete">Ziemeļi:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>East:</source>
        <translation type="obsolete">Austrumi:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="157"/>
        <source>Use the text boxes below to test the projection definition you are creating. Enter a coordinate where both the lat/long and the projected result are known (for example by reading off a map). Then press the calculate button to see if the projection definition you are creating is accurate.</source>
        <translation>Izmantojiet zemāk esošos teksta laukus jaunās projekcijas definīcijas pārbaudei. Ievadiet koordinātas, kurām avots un rezultāts ir zināmi (piem. nolasījumi no kartes). Nospiediet Aprēķināt pogu un salīdziniet rezultātu.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="187"/>
        <source>Projected Coordinate System</source>
        <translation>Projecēta koordinātu sistēma</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="38"/>
        <source>Name</source>
        <translation type="unfinished">Nosaukums</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="167"/>
        <source>Parameters</source>
        <translation type="unfinished">Parametri</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="116"/>
        <source>*</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="126"/>
        <source>S</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="136"/>
        <source>X</source>
        <translation type="unfinished">X</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="194"/>
        <source>North</source>
        <translation type="unfinished">Ziemeļi</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="224"/>
        <source>East</source>
        <translation type="unfinished">Austrumi</translation>
    </message>
</context>
<context>
    <name>QgsDbSourceSelect</name>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="230"/>
        <source>Are you sure you want to remove the </source>
        <translation>Vai Jūs tiešām vēlaties aizvākt </translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="230"/>
        <source> connection and all associated settings?</source>
        <translation>savienojumu un visus ar to saistītos parametrus?</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="231"/>
        <source>Confirm Delete</source>
        <translation>Apstiprināt dzēšanu</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="338"/>
        <source>Select Table</source>
        <translation>Izvēlieties tabulu</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="338"/>
        <source>You must select a table in order to add a Layer.</source>
        <translation>Jums ir jāizvēlas tabula, lai varētu pievienot slāni.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="371"/>
        <source>Password for </source>
        <translation>Parole priekš</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="372"/>
        <source>Please enter your password:</source>
        <translation>Lūdzu, ievadiet paroli:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="437"/>
        <source>Connection failed</source>
        <translation>Savienošanās neizdevās</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="147"/>
        <source>Type</source>
        <translation>Tips</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="155"/>
        <source>Sql</source>
        <translation>Sql</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="440"/>
        <source>Connection to %1 on %2 failed. Either the database is down or your settings are incorrect.%3Check your username and password and try again.%4The database said:%5%6</source>
        <translation>Savienojums ar %1 uz %2 neizdevās. Vai nu datu bāze nestrādā, vai arī norādītie parametri ir nepareizi. %3 Pārbaudiet lietotājvārdu un paroli un mēģiniet vēlreiz. %4 Datu bāze atbildēja: %5%6</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="123"/>
        <source>Wildcard</source>
        <translation>Aizstājējzīme</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="127"/>
        <source>RegExp</source>
        <translation>Regulāra izteiksme</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="135"/>
        <source>All</source>
        <translation>Viss</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="139"/>
        <source>Schema</source>
        <translation>Shēma</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="143"/>
        <source>Table</source>
        <translation>Tabula</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="151"/>
        <source>Geometry column</source>
        <translation>Ģeometrijas kolonna</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="547"/>
        <source>Accessible tables could not be determined</source>
        <translation>Nebija iespējams noteikt pieejamās tabulas</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="549"/>
        <source>Database connection was successful, but the accessible tables could not be determined.

The error message from the database was:
%1
</source>
        <translation>Savienojums ar datu bāzi tika izveidots sekmīgi, bet nebija iespējams noteikt pieejamās tabulas. 

Datu bāzes ziņotā kļūda bija: 
%1</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="554"/>
        <source>No accessible tables found</source>
        <translation>Nav atrastas pieejamas tabulas</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="558"/>
        <source>Database connection was successful, but no accessible tables were found.

Please verify that you have SELECT privilege on a table carrying PostGIS
geometry.</source>
        <translation>Savienojums ar datu bāzi tika izveidots sekmīgi, bet pieejamas tabulas netika atrastas. 

Pārliecinieties, ka jums ir SELECT tiesības tai tabulai, kas satur PostGIS ģeometriju.</translation>
    </message>
</context>
<context>
    <name>QgsDbSourceSelectBase</name>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="13"/>
        <source>Add PostGIS Table(s)</source>
        <translation>Pievienot PostGIS tabulas</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="54"/>
        <source>PostgreSQL Connections</source>
        <translation>PostgreSQL savienojumi</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="87"/>
        <source>Connect</source>
        <translation>Pieslēgties</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="80"/>
        <source>New</source>
        <translation>Jauns</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="73"/>
        <source>Edit</source>
        <translation>Rediģēt</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="66"/>
        <source>Delete</source>
        <translation>Dzēst</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="111"/>
        <source>Help</source>
        <translation>Palīdzība</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="114"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="140"/>
        <source>Add</source>
        <translation>Pievienot</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="156"/>
        <source>Close</source>
        <translation>Aizvērt</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="183"/>
        <source>Search:</source>
        <translation>Meklēt:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="190"/>
        <source>Search mode:</source>
        <translation>Meklēšanas režīms:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="200"/>
        <source>Search in columns:</source>
        <translation>Meklēt kolonnās:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="216"/>
        <source>Search options...</source>
        <translation>Meklēšanas opcijas...</translation>
    </message>
</context>
<context>
    <name>QgsDbTableModel</name>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="24"/>
        <source>Schema</source>
        <translation>Shēma</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="25"/>
        <source>Table</source>
        <translation>Tabula</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="26"/>
        <source>Type</source>
        <translation>Tips</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="27"/>
        <source>Geometry column</source>
        <translation>Ģeometrijas kolonna</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="28"/>
        <source>Sql</source>
        <translation>Sql</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="229"/>
        <source>Point</source>
        <translation>Punkts</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="233"/>
        <source>Multipoint</source>
        <translation>Multipunkts</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="237"/>
        <source>Line</source>
        <translation>Līnija</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="241"/>
        <source>Multiline</source>
        <translation>Multilīnija</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="245"/>
        <source>Polygon</source>
        <translation>Poligons</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="249"/>
        <source>Multipolygon</source>
        <translation>Multipoligons</translation>
    </message>
</context>
<context>
    <name>QgsDelAttrDialogBase</name>
    <message>
        <location filename="../src/ui/qgsdelattrdialogbase.ui" line="13"/>
        <source>Delete Attributes</source>
        <translation>Dzēst atribūtus</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>OK</source>
        <translation type="obsolete">Labi</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Cancel</source>
        <translation type="obsolete">Atcelt</translation>
    </message>
</context>
<context>
    <name>QgsDelimitedTextPlugin</name>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="101"/>
        <source>&amp;Add Delimited Text Layer</source>
        <translation>&amp;Pievienot atdalīta teksta slāni</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="104"/>
        <source>Add a delimited text file as a map layer. </source>
        <translation>Pievienot atdalīta teksta failu kā kartes slāni. </translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="105"/>
        <source>The file must have a header row containing the field names. </source>
        <translation>Failam jāsatur galvenes rinda ar lauku nosaukumiem. </translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="105"/>
        <source>X and Y fields are required and must contain coordinates in decimal units.</source>
        <translation>X un Y lauki ir obligāti un tiem jāsatur koordinātes decimālajās vienībās.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="142"/>
        <source>&amp;Delimited text</source>
        <translation>&amp;Atdalīts teksts</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="56"/>
        <source>DelimitedTextLayer</source>
        <translation>AtdalītaTekstaSlānis</translation>
    </message>
</context>
<context>
    <name>QgsDelimitedTextPluginGui</name>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="125"/>
        <source>No layer name</source>
        <translation>Nav slāņa nosaukuma</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="125"/>
        <source>Please enter a layer name before adding the layer to the map</source>
        <translation>Lūdzu ievadiet slāņa nosaukumu pirms tā pievienošanas kartei</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="211"/>
        <source>No delimiter</source>
        <translation>Nav atdalītāja</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="211"/>
        <source>Please specify a delimiter prior to parsing the file</source>
        <translation>Lūdzu norādiet atdalītāju pirms faila apsrtādes</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="245"/>
        <source>Choose a delimited text file to open</source>
        <translation>Izvēlieties atdalītā teksta failu</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="35"/>
        <source>Parse</source>
        <translation>Parsēt</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="60"/>
        <source>Description</source>
        <translation>Apraksts</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="63"/>
        <source>Select a delimited text file containing a header row and one or more rows of x and y coordinates that you would like to use as a point layer and this plugin will do the job for you!</source>
        <translation>Izvēlieties atdalīta teksta failu, kas satur galvnes rindu un vismaz vienu rindu ar X un Y vērtībām, kuras jūs vēlaties izmantot kā punktu koordinātas. Šis spraudnis paveiks pārējo.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="67"/>
        <source>Use the layer name box to specify the legend name for the new layer. Use the delimiter box to specify what delimeter is used in your file (e.g. space, comma, tab or a regular expression in Perl style). After choosing a delimiter, press the parse button and select the columns containing the x and y values for the layer.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsDelimitedTextPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="13"/>
        <source>Create a Layer from a Delimited Text File</source>
        <translation>Izveidot slāni no atdalīta teksta faila</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="67"/>
        <source>&lt;p align=&quot;right&quot;&gt;X field&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;right&quot;&gt;X lauks&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="88"/>
        <source>Name of the field containing x values</source>
        <translation>X vērtības saturošā lauka nosaukums</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="91"/>
        <source>Name of the field containing x values. Choose a field from the list. The list is generated by parsing the header row of the delimited text file.</source>
        <translation>X vērtības saturošā lauka nosaukums. Izvēlieties no saraksta. Saraksts tiek veidots parsējot ievaddatu faila galvnes rindu.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="101"/>
        <source>&lt;p align=&quot;right&quot;&gt;Y field&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;right&quot;&gt;Y lauks&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="122"/>
        <source>Name of the field containing y values</source>
        <translation>Y vērtības saturošā lauka nosaukums</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="125"/>
        <source>Name of the field containing y values. Choose a field from the list. The list is generated by parsing the header row of the delimited text file.</source>
        <translation>Y vērtības saturošā lauka nosaukums. Izvēlieties no saraksta. Saraksts tiek veidots parsējot ievaddatu faila galvnes rindu.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="191"/>
        <source>Layer name</source>
        <translation>Sāņa nosaukums</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="198"/>
        <source>Name to display in the map legend</source>
        <translation>Nosaukums, ko rādīt kartes leģendā</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="201"/>
        <source>Name displayed in the map legend</source>
        <translation>Nosaukums, kas tiks rādīts kartes leģendā</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="297"/>
        <source>Delimiter</source>
        <translation>Atdalītājs</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="318"/>
        <source>Delimiter to use when splitting fields in the text file. The delimiter can be more than one character.</source>
        <translation>Atdalītājs, kas ir izmantots datu atdalīšanai teksta failā. Atdalītājs var būt lielāks par vienu simbolu.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="321"/>
        <source>Delimiter to use when splitting fields in the delimited text file. The delimiter can be 1 or more characters in length.</source>
        <translation>Atdalītājs, kas ir izmantots datu atdalīšanai teksta failā. Atdalītājs var būt 1 vai vairāku simbolu liels.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="39"/>
        <source>Delimited Text Layer</source>
        <translation>Atdalīta teksta slānis</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="145"/>
        <source>Delimited text file</source>
        <translation>Atdalīta teksta fails</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="152"/>
        <source>Full path to the delimited text file</source>
        <translation>Pilns ceļš līdz atdalīta teksta failam</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="155"/>
        <source>Full path to the delimited text file. In order to properly parse the fields in the file, the delimiter must be defined prior to entering the file name. Use the Browse button to the right of this field to choose the input file.</source>
        <translation>Pilns ceļš līdz atdalītā teksta failam. Lai pareizi apstrādātu failu, nepieciešams atdalītāju definēt pirms faila nosaukuma ievadīšanas. Izmantojiet Pārlūkot pogu, lai izvēlētos ievaddatu failu.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="168"/>
        <source>Browse to find the delimited text file to be processed</source>
        <translation>Parlūkot, lai atrastu atdalītā teksta failu</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="171"/>
        <source>Use this button to browse to the location of the delimited text file. This button will not be enabled until a delimiter has been entered in the &lt;i&gt;Delimiter&lt;/i&gt; box. Once a file is chosen, the X and Y field drop-down boxes will be populated with the fields from the delimited text file.</source>
        <translation>Izmantojiet šo pogu, lai sameklētu atdalītā teksta failu. Šī poga tiks aktivizēta tikai tad, kad būs norādīts teksta atdalītājs &lt;i&gt;Atdalītāja&lt;/i&gt; lauciņā. Tiklīdz fails būs izvēlēts, X un Y lauciņu saraksti tiks piepildīti ar informāciju no faila.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="246"/>
        <source>Sample text</source>
        <translation>Paraugs</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="174"/>
        <source>Browse...</source>
        <translation>Pārlūkot...</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="331"/>
        <source>The delimiter is taken as is</source>
        <translation>Atdalītājs ir pieņemts kā ir</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="334"/>
        <source>Plain characters</source>
        <translation>Vienkāršs teksts</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="344"/>
        <source>The delimiter is a regular expression</source>
        <translation>Atdalītājs ir regulāra izteiksme</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="347"/>
        <source>Regular expression</source>
        <translation>Regulāra izteiksme</translation>
    </message>
</context>
<context>
    <name>QgsDelimitedTextProvider</name>
    <message>
        <location filename="../src/providers/delimitedtext/qgsdelimitedtextprovider.cpp" line="402"/>
        <source>Note: the following lines were not loaded because Qgis was unable to determine values for the x and y coordinates:
</source>
        <translation>Piezīme: sekojošās rindas netika ielādētas, jo QGIS nevarēja noteikt vērtības X un Y koordinātām: </translation>
    </message>
    <message>
        <location filename="../src/providers/delimitedtext/qgsdelimitedtextprovider.cpp" line="400"/>
        <source>Error</source>
        <translation>Kļūda</translation>
    </message>
</context>
<context>
    <name>QgsDetailedItemWidgetBase</name>
    <message>
        <location filename="../src/ui/qgsdetaileditemwidgetbase.ui" line="13"/>
        <source>Form</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdetaileditemwidgetbase.ui" line="96"/>
        <source>Heading Label</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdetaileditemwidgetbase.ui" line="117"/>
        <source>Detail label</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsDlgPgBufferBase</name>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="13"/>
        <source>Buffer features</source>
        <translation>Izveidot buferi</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="224"/>
        <source>Buffer distance in map units:</source>
        <translation>Bufera platums kartes vienībās:</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="132"/>
        <source>Table name for the buffered layer:</source>
        <translation>Bufera slāņa nosaukums:</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="172"/>
        <source>Create unique object id</source>
        <translation>Izveidot unikālu objekta ID</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="216"/>
        <source>public</source>
        <translation>publisks</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="59"/>
        <source>Geometry column:</source>
        <translation>Ģeometrijas kolonna:</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="92"/>
        <source>Spatial reference ID:</source>
        <translation>Telpiskās norādes ID:</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="125"/>
        <source>Unique field to use as feature id:</source>
        <translation>Unikāls lauks, ko lietot kā objekta ID:</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="118"/>
        <source>Schema:</source>
        <translation>Shēma:</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="66"/>
        <source>Add the buffered layer to the map?</source>
        <translation>Pievienot bufera slāni kartei?</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="234"/>
        <source>&lt;h2&gt;Buffer the features in layer: &lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Izveido buferi objektiem slānī: &lt;/h2&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="28"/>
        <source>Parameters</source>
        <translation>Parametri</translation>
    </message>
</context>
<context>
    <name>QgsEncodingFileDialog</name>
    <message>
        <location filename="../src/gui/qgsencodingfiledialog.cpp" line="29"/>
        <source>Encoding:</source>
        <translation>Kodējums:</translation>
    </message>
</context>
<context>
    <name>QgsFillStyleWidgetBase</name>
    <message>
        <location filename="../build/src/ui/ui_qgsfillstylewidgetbase.h" line="83"/>
        <source>Form1</source>
        <translation>Forma1</translation>
    </message>
    <message>
        <location filename="../build/src/ui/ui_qgsfillstylewidgetbase.h" line="84"/>
        <source>Fill Style</source>
        <translation>Aizpildījuma stils</translation>
    </message>
    <message>
        <location filename="../build/src/ui/ui_qgsfillstylewidgetbase.h" line="90"/>
        <source>PolyStyleWidget</source>
        <translation>Daudzstiluvidžets</translation>
    </message>
    <message>
        <location filename="../build/src/ui/ui_qgsfillstylewidgetbase.h" line="86"/>
        <source>Colour:</source>
        <translation>Krāsa:</translation>
    </message>
    <message>
        <location filename="../build/src/ui/ui_qgsfillstylewidgetbase.h" line="85"/>
        <source>col</source>
        <translation>kol</translation>
    </message>
</context>
<context>
    <name>QgsGPSDeviceDialog</name>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialog.cpp" line="44"/>
        <source>New device %1</source>
        <translation>Jauna iekārta %1</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialog.cpp" line="57"/>
        <source>Are you sure?</source>
        <translation>Vai esat pārliecināts?</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialog.cpp" line="58"/>
        <source>Are you sure that you want to delete this device?</source>
        <translation>Vai Jūs tiešām vēlaties dzēst šo iekārtu?</translation>
    </message>
</context>
<context>
    <name>QgsGPSDeviceDialogBase</name>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="21"/>
        <source>GPS Device Editor</source>
        <translation>GPS iekārtas redaktos</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="132"/>
        <source>Device name:</source>
        <translation>Iekārtas nosaukums:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="147"/>
        <source>This is the name of the device as it will appear in the lists</source>
        <translation>Šis ir iekārtas nosaukums, kas tiks rādīts pie iekārtu saraksta</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="99"/>
        <source>Update device</source>
        <translation>Atjaunināt iekārtu</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="84"/>
        <source>Delete device</source>
        <translation>Dzēst iekārtu</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="69"/>
        <source>New device</source>
        <translation>Jauna iekārta</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="298"/>
        <source>Close</source>
        <translation>Aizvērt</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="156"/>
        <source>Commands</source>
        <translation>Komandas</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="185"/>
        <source>Waypoint download:</source>
        <translation>Ceļapunktu lejupielāde:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="248"/>
        <source>Waypoint upload:</source>
        <translation>Ceļapunktu augšupielāde:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="199"/>
        <source>Route download:</source>
        <translation>Maršrutu lejupielāde:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="178"/>
        <source>Route upload:</source>
        <translation>Maršrutu augšupielāde:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="171"/>
        <source>Track download:</source>
        <translation>Ceļu lejupielāde:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="241"/>
        <source>The command that is used to upload tracks to the device</source>
        <translation>Komanda, ko izmanto ceļu augšupielādei uz iekārtu</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="213"/>
        <source>Track upload:</source>
        <translation>Ceļu augšupielāde:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="220"/>
        <source>The command that is used to download tracks from the device</source>
        <translation>Komanda, ko izmanto ceļu lejupielādei no iekārtas</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="227"/>
        <source>The command that is used to upload routes to the device</source>
        <translation>Komanda, ko izmanto maršrutu augšupielādei uz iekārtu</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="192"/>
        <source>The command that is used to download routes from the device</source>
        <translation>Komanda, ko izmanto maršrutu lejupielādei no iekārtas</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="206"/>
        <source>The command that is used to upload waypoints to the device</source>
        <translation>Komanda, ko izmanto ceļapunktu augšupielādei uz iekārtu</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="234"/>
        <source>The command that is used to download waypoints from the device</source>
        <translation>Komanda, ko izmanto ceļapunktu lejupielādei no iekārtas</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="266"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;In the download and upload commands there can be special words that will be replaced by QGIS when the commands are used. These words are:&lt;span style=&quot; font-style:italic;&quot;&gt;%babel&lt;/span&gt; - the path to GPSBabel&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%in&lt;/span&gt; - the GPX filename when uploading or the port when downloading&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%out&lt;/span&gt; - the port when uploading or the GPX filename when downloading&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Lejup un augšuplādēšanas komandās var būt speciālie vārdi, ko darbināšanas laikā QGIS aizstās ar vērtībām. Šie vārdi ir:&lt;span style=&quot; font-style:italic;&quot;&gt;%babel&lt;/span&gt; - ceļš līdz GPSBabel&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%in&lt;/span&gt; - GPX faila nosaukums pie augšuplādēšanas vai ports pie lejuplādēšanas&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%out&lt;/span&gt; - ports pie augšuplādēšanas vai GPX faila nosaukums pie lejuplādēšanas&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
</context>
<context>
    <name>QgsGPSPlugin</name>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="93"/>
        <source>&amp;Gps Tools</source>
        <translation>&amp;GPS rīki</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="94"/>
        <source>&amp;Create new GPX layer</source>
        <translation>&amp;Izveidot jaunu GPX slāni</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="97"/>
        <source>Creates a new GPX layer and displays it on the map canvas</source>
        <translation>Izveido jaunu GPX slāni un parāda to uz kartes audekla</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="196"/>
        <source>&amp;Gps</source>
        <translation>&amp;GPS</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="160"/>
        <source>Save new GPX file as...</source>
        <translation>Saglabāt jauno GPX failu kā...</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="160"/>
        <source>GPS eXchange file (*.gpx)</source>
        <translation>GPS apmaiņas fails (*.gpx)</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="165"/>
        <source>Could not create file</source>
        <translation>Failu nebija iespējams izveidot</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="167"/>
        <source>Unable to create a GPX file with the given name. </source>
        <translation>Nebija iespējams izveidot GPX failu ar doto nosaukumu. </translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="168"/>
        <source>Try again with another name or in another </source>
        <translation>Mēģiniet vēlreiz ar citu nosaukumu vai citā </translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="168"/>
        <source>directory.</source>
        <translation>mapē.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="207"/>
        <source>GPX Loader</source>
        <translation>GPX lādētājs</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="209"/>
        <source>Unable to read the selected file.
</source>
        <translation>Nav iespējams nolasīt izvēlēto failu.
</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="209"/>
        <source>Please reselect a valid file.</source>
        <translation>Lūdzu izvēlieties derīgu failu.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="487"/>
        <source>Could not start process</source>
        <translation>Nebija iespējams startēt procesu</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="488"/>
        <source>Could not start GPSBabel!</source>
        <translation>Nebija iespējams startēt GPSBabel!</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="330"/>
        <source>Importing data...</source>
        <translation>Datu importēšana...</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="493"/>
        <source>Cancel</source>
        <translation>Atcelt</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="274"/>
        <source>Could not import data from %1!

</source>
        <translation>Nebija iespējams importēt datus no %1!

</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="276"/>
        <source>Error importing data</source>
        <translation>Kļūda importējot datus</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="476"/>
        <source>Not supported</source>
        <translation>Nav atbalstīts</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="394"/>
        <source>This device does not support downloading </source>
        <translation>Iekārta neatbalsta lejuplādēšanu </translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="394"/>
        <source>of </source>
        <translation>no </translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="410"/>
        <source>Downloading data...</source>
        <translation>Lejuplādēju datus...</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="421"/>
        <source>Could not download data from GPS!

</source>
        <translation>Nebija iespējams lejuplādēt datus no GPS!

</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="423"/>
        <source>Error downloading data</source>
        <translation>Kļūda lejuplādējot datus</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="477"/>
        <source>This device does not support uploading of </source>
        <translation>Iekārta neatbalsta augšuplādēšanu </translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="493"/>
        <source>Uploading data...</source>
        <translation>Augšuplādēju datus...</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="504"/>
        <source>Error while uploading data to GPS!

</source>
        <translation>Kļūda augšuplādējot datus uz GPS!

</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="506"/>
        <source>Error uploading data</source>
        <translation>Kļūda augšuplādējot datus</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="342"/>
        <source>Could not convert data from %1!

</source>
        <translation>Nebija iespējams konvertēt datus no %1!</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="344"/>
        <source>Error converting data</source>
        <translation>Kļūda konvertējot datus</translation>
    </message>
</context>
<context>
    <name>QgsGPSPluginGui</name>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="522"/>
        <source>Choose a filename to save under</source>
        <translation>Izvēlieties faila nosaukumu ar kādu saglabāt</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="524"/>
        <source>GPS eXchange format (*.gpx)</source>
        <translation>GPS apmaiņas formāts (*.gpx)</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="511"/>
        <source>Select GPX file</source>
        <translation>Izvēlieties GPX failu</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="250"/>
        <source>Select file and format to import</source>
        <translation>Izvēlieties importējamo failu un formātu</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="486"/>
        <source>Waypoints</source>
        <translation>Ceļapunkti</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="486"/>
        <source>Routes</source>
        <translation>Maršruti</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="272"/>
        <source>Tracks</source>
        <translation>Ceļi</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="491"/>
        <source>QGIS can perform conversions of GPX files, by using GPSBabel (%1) to perform the conversions.</source>
        <translation>QGIS var konvertēt GPX failus, ja ir pieejama GPSBabel (%1) programma, kas veic reālo konvertāciju.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="492"/>
        <source>This requires that you have GPSBabel installed where QGIS can find it.</source>
        <translation>Tam ir nepieciešams, lai GPSBabel programma būtu instalēta QGIS pieejamā vietā.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="493"/>
        <source>Select a GPX input file name, the type of conversion you want to perform, a GPX filename that you want to save the converted file as, and a name for the new layer created from the result.</source>
        <translation>Izvēlieties GPX ievades faila nosaukumu, veicamās konvertēšanas tipu, GPX saglabājamā faila nosaukumu un jaunā slāņa nosaukumu.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="422"/>
        <source>GPX is the %1, which is used to store information about waypoints, routes, and tracks.</source>
        <translation>GPX ir %1, ko izmanto ceļapunktu, ceļu un maršrutu informācijas glabāšanai.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="422"/>
        <source>GPS eXchange file format</source>
        <translation>GPS apmaiņas failu formāts</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="423"/>
        <source>Select a GPX file and then select the feature types that you want to load.</source>
        <translation>Izvēlieties GPX failu un ielādējamo objektu tipu.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="435"/>
        <source>This tool will help you download data from a GPS device.</source>
        <translation>Šis rīks palīdzēs lejuplādēt datus no GPS iekārtas.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="436"/>
        <source>Choose your GPS device, the port it is connected to, the feature type you want to download, a name for your new layer, and the GPX file where you want to store the data.</source>
        <translation>Izvēlieties sava GPS aparāta iekārtu, portu, lejuplādējamo objektu tipu un jaunveidojamā slāņa nosaukumu, kā arī GPX faila nosaukumu, kurā saglabāt rezultātu.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="455"/>
        <source>If your device isn&apos;t listed, or if you want to change some settings, you can also edit the devices.</source>
        <translation>Ja jūsu iekārta nav uzrādīta vai arī vēlaties izmainīt iestatījumus, jūs varat rediģēt iekārtas.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="457"/>
        <source>This tool uses the program GPSBabel (%1) to transfer the data.</source>
        <translation>Šis rīks izmanto GPSBabel (%1) datu konvertēšanai.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="453"/>
        <source>This tool will help you upload data from a GPX layer to a GPS device.</source>
        <translation>Šis rīks palīdzēs augšuplādēt GPX slāni uz GPS iekārtu.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="454"/>
        <source>Choose the layer you want to upload, the device you want to upload it to, and the port your device is connected to.</source>
        <translation>Izvēlieties augšuplādējamo slāni, iekārtu, portu pie kura pieslēgta iekārta.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="472"/>
        <source>QGIS can only load GPX files by itself, but many other formats can be converted to GPX using GPSBabel (%1).</source>
        <translation>QGIS var ielādēt tikai GPX failus, bet citus formātus var konvertēt uz GPX formātu ar GPSBabel palīdzību (%1).</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="474"/>
        <source>Select a GPS file format and the file that you want to import, the feature type that you want to use, a GPX filename that you want to save the converted file as, and a name for the new layer.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="475"/>
        <source>All file formats can not store waypoints, routes, and tracks, so some feature types may be disabled for some file formats.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGPSPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="13"/>
        <source>GPS Tools</source>
        <translation>GPS rīki</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="58"/>
        <source>Load GPX file</source>
        <translation>Ielādēt GPX failu</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="94"/>
        <source>File:</source>
        <translation>Fails:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="111"/>
        <source>Feature types:</source>
        <translation>Objektu tips:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="314"/>
        <source>Waypoints</source>
        <translation>Ceļapunkti</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="319"/>
        <source>Routes</source>
        <translation>Maršruti</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="324"/>
        <source>Tracks</source>
        <translation>Ceļi</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="158"/>
        <source>Import other file</source>
        <translation>Importēt citu failu</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="264"/>
        <source>File to import:</source>
        <translation>Importējamais fails:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="339"/>
        <source>Feature type:</source>
        <translation>Objektu tips:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="582"/>
        <source>GPX output file:</source>
        <translation>GPX izvades fails:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="558"/>
        <source>Layer name:</source>
        <translation>Slāņa nosaukums:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="272"/>
        <source>Download from GPS</source>
        <translation>Lejupielādēt no GPS</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="508"/>
        <source>Edit devices</source>
        <translation>Rediģēt iekārtas</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="515"/>
        <source>GPS device:</source>
        <translation>GPS iekārta:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="356"/>
        <source>Output file:</source>
        <translation>Izvades fails:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="481"/>
        <source>Port:</source>
        <translation>Ports:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="430"/>
        <source>Upload to GPS</source>
        <translation>Augšupielādēt uz GPS</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="522"/>
        <source>Data layer:</source>
        <translation>Datu slānis:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="572"/>
        <source>Browse...</source>
        <translation>Pārlūkot...</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="565"/>
        <source>Save As...</source>
        <translation>Saglabāt kā...</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="250"/>
        <source>(Note: Selecting correct file type in browser dialog important!)</source>
        <translation>(Piezīme: Pareiza faila tipa izvēle faila izvēles logā ir svarīga!)</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="530"/>
        <source>GPX Conversions</source>
        <translation>GPX konvertēšana</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="615"/>
        <source>Conversion:</source>
        <translation>Konvertēšana:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="629"/>
        <source>GPX input file:</source>
        <translation>GPX ievades fails:</translation>
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
        <location filename="../src/providers/gpx/qgsgpxprovider.cpp" line="70"/>
        <source>Bad URI - you need to specify the feature type.</source>
        <translation>Nederīgs URI - jums jānorāda objektu tips.</translation>
    </message>
    <message>
        <location filename="../src/providers/gpx/qgsgpxprovider.cpp" line="114"/>
        <source>GPS eXchange file</source>
        <translation>GPS apmaiņas fails</translation>
    </message>
    <message>
        <location filename="../src/providers/gpx/qgsgpxprovider.cpp" line="731"/>
        <source>Digitized in QGIS</source>
        <translation>Digitizēts iekš QGIS</translation>
    </message>
</context>
<context>
    <name>QgsGeomTypeDialog</name>
    <message>
        <location filename="" line="135533324"/>
        <source>Name</source>
        <translation type="obsolete">Nosaukums</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Type</source>
        <translation type="obsolete">Tips</translation>
    </message>
    <message>
        <location filename="../src/app/qgsgeomtypedialog.cpp" line="30"/>
        <source>Real</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsgeomtypedialog.cpp" line="31"/>
        <source>Integer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsgeomtypedialog.cpp" line="32"/>
        <source>String</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGeomTypeDialogBase</name>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="155"/>
        <source>Type</source>
        <translation>Tips</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="41"/>
        <source>Point</source>
        <translation>Punkts</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="48"/>
        <source>Line</source>
        <translation>Līnija</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="55"/>
        <source>Polygon</source>
        <translation>Poligons</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="13"/>
        <source>New Vector Layer</source>
        <translation>Jauns vektordatu slānis</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Attributes:</source>
        <translation type="obsolete">Atribūti:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Add</source>
        <translation type="obsolete">Pievienot</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Column 1</source>
        <translation type="obsolete">Kolonna 1</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Remove</source>
        <translation type="obsolete">Noņemt</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>File Format:</source>
        <translation type="obsolete">Faila formāts:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="22"/>
        <source>File format</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="65"/>
        <source>Attributes</source>
        <translation type="unfinished">Atribūti</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="150"/>
        <source>Name</source>
        <translation type="unfinished">Nosaukums</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="111"/>
        <source>Remove selected row</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="127"/>
        <source>...</source>
        <translation type="unfinished">...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="124"/>
        <source>Add values manually</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGeorefDescriptionDialogBase</name>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefdescriptiondialogbase.ui" line="13"/>
        <source>Description georeferencer</source>
        <translation>Apraksta ģeoreferencētājs</translation>
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
        <translation>&amp;Telpiskā piesaiste</translation>
    </message>
</context>
<context>
    <name>QgsGeorefPluginGui</name>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="85"/>
        <source>Choose a raster file</source>
        <translation>Izvēlieties rastra failu</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="87"/>
        <source>Raster files (*.*)</source>
        <translation>Rastra faili (*.*)</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="97"/>
        <source>Error</source>
        <translation>Kļūda</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="98"/>
        <source>The selected file is not a valid raster file.</source>
        <translation>Izvēlētais fails nav derīgs rastra fails.</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="122"/>
        <source>World file exists</source>
        <translation>World fails jau eksistē</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="124"/>
        <source>&lt;p&gt;The selected file already seems to have a </source>
        <translation>&lt;p&gt;Izvēlētajam failam jau eksistē </translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="125"/>
        <source>world file! Do you want to replace it with the </source>
        <translation> world fails. Vai vēlaties to aizstāt ar </translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="125"/>
        <source>new world file?&lt;/p&gt;</source>
        <translation> jaunu world failu?&lt;/p&gt;</translation>
    </message>
</context>
<context>
    <name>QgsGeorefPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/georeferencer/pluginguibase.ui" line="13"/>
        <source>Georeferencer</source>
        <translation>Telpiskā piesaiste</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/pluginguibase.ui" line="100"/>
        <source>Close</source>
        <translation>Aizvērt</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/pluginguibase.ui" line="43"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/pluginguibase.ui" line="62"/>
        <source>Raster file:</source>
        <translation>Rastra fails:</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/pluginguibase.ui" line="28"/>
        <source>Arrange plugin windows</source>
        <translation>Izkārtot spraudņa logu</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/pluginguibase.ui" line="77"/>
        <source>Description...</source>
        <translation>Apraksts...</translation>
    </message>
</context>
<context>
    <name>QgsGeorefWarpOptionsDialog</name>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialog.cpp" line="27"/>
        <source>unstable</source>
        <translation>nestabils</translation>
    </message>
</context>
<context>
    <name>QgsGeorefWarpOptionsDialogBase</name>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="13"/>
        <source>Warp options</source>
        <translation>Transformācijas parametri</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="35"/>
        <source>Resampling method:</source>
        <translation>Transformācijas metode:</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="46"/>
        <source>Nearest neighbour</source>
        <translation>Tuvākais kaimiņš</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="51"/>
        <source>Linear</source>
        <translation>Lineārs</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="56"/>
        <source>Cubic</source>
        <translation>Kubisks</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="74"/>
        <source>OK</source>
        <translation>Labi</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="64"/>
        <source>Use 0 for transparency when needed</source>
        <translation>Izmantot 0 caurspīdīgumam, ja nepieciešams</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="28"/>
        <source>Compression:</source>
        <translation>Kompresija:</translation>
    </message>
</context>
<context>
    <name>QgsGraduatedSymbolDialog</name>
    <message>
        <location filename="../src/app/qgsgraduatedsymboldialog.cpp" line="324"/>
        <source>Equal Interval</source>
        <translation>Vienādi intervāli</translation>
    </message>
    <message>
        <location filename="../src/app/qgsgraduatedsymboldialog.cpp" line="301"/>
        <source>Quantiles</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsgraduatedsymboldialog.cpp" line="350"/>
        <source>Empty</source>
        <translation>Tukšs</translation>
    </message>
</context>
<context>
    <name>QgsGraduatedSymbolDialogBase</name>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="25"/>
        <source>graduated Symbol</source>
        <translation>graduēts simbols</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Classification Field:</source>
        <translation type="obsolete">Klasifikācijas lauks:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Mode:</source>
        <translation type="obsolete">Režīms:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Number of Classes:</source>
        <translation type="obsolete">Klašu skaits:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="188"/>
        <source>Delete class</source>
        <translation>Dzēst klasi</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="181"/>
        <source>Classify</source>
        <translation>Klasificēt</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="55"/>
        <source>Classification field</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="93"/>
        <source>Mode</source>
        <translation type="unfinished">Režīms</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="131"/>
        <source>Number of classes</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassAttributes</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="300"/>
        <source>Warning</source>
        <translation>Brīdinājums</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="152"/>
        <source>Column</source>
        <translation>Kolonna</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="153"/>
        <source>Value</source>
        <translation>Vērtība</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="154"/>
        <source>Type</source>
        <translation>Tips</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="301"/>
        <source>ERROR</source>
        <translation>KĻŪDA</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="303"/>
        <source>OK</source>
        <translation>Labi</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="158"/>
        <source>Layer</source>
        <translation>Slānis</translation>
    </message>
</context>
<context>
    <name>QgsGrassAttributesBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="48"/>
        <source>GRASS Attributes</source>
        <translation>GRASS atribūti</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="78"/>
        <source>Tab 1</source>
        <translation>Tab 1</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="112"/>
        <source>result</source>
        <translation>rezultāts</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="180"/>
        <source>Update</source>
        <translation>Atjaunot</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="177"/>
        <source>Update database record</source>
        <translation>Atjaunot datubāzes ierakstu</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="210"/>
        <source>New</source>
        <translation>Jauns</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="207"/>
        <source>Add new category using settings in GRASS Edit toolbox</source>
        <translation>Pievienot jaunu kategoriju izmantojot GRASS labošanas rīkkopu</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="240"/>
        <source>Delete</source>
        <translation>Dzēst</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="237"/>
        <source>Delete selected category</source>
        <translation>Dzēst izvēlēto kategoriju</translation>
    </message>
</context>
<context>
    <name>QgsGrassBrowser</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="66"/>
        <source>Tools</source>
        <translation>Rīki</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="71"/>
        <source>Add selected map to canvas</source>
        <translation>Pievienot izvēlēto karti audeklam</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="79"/>
        <source>Copy selected map</source>
        <translation>Kopēt izvēlēto karti</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="87"/>
        <source>Rename selected map</source>
        <translation>Pārsaukt izvēlēto karti</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="95"/>
        <source>Delete selected map</source>
        <translation>Dzēst izvēlēto karti</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="103"/>
        <source>Set current region to selected map</source>
        <translation>Iestatīt pašreizējo reģionu tā, lai tas atbilstu izvēlētajai kartei</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="111"/>
        <source>Refresh</source>
        <translation>Atsvaidzināt</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="453"/>
        <source>Warning</source>
        <translation>Brīdinājums</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="289"/>
        <source>Cannot copy map </source>
        <translation>Nevar kopēt karti </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="411"/>
        <source>&lt;br&gt;command: </source>
        <translation>&lt;br&gt;komanda: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="355"/>
        <source>Cannot rename map </source>
        <translation>Nevar pārsaukt karti </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="393"/>
        <source>Delete map &lt;b&gt;</source>
        <translation>Dzēst karti &lt;b&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="410"/>
        <source>Cannot delete map </source>
        <translation>Nevar dzēst karti </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="454"/>
        <source>Cannot write new region</source>
        <translation>Nevar ierakstīt jaunu reģionu</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="339"/>
        <source>New name</source>
        <translation>Jauns nosaukums</translation>
    </message>
</context>
<context>
    <name>QgsGrassEdit</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="217"/>
        <source>New point</source>
        <translation>Jauns punkts</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="235"/>
        <source>New centroid</source>
        <translation>Jauns centroīds</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="253"/>
        <source>Delete vertex</source>
        <translation>Dzēst virsotni</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1821"/>
        <source>Left: </source>
        <translation>Pa kreisi: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1822"/>
        <source>Middle: </source>
        <translation>Pa vidu: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="214"/>
        <source>Edit tools</source>
        <translation>Rediģēšanas rīki</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="223"/>
        <source>New line</source>
        <translation>Jauna līnija</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="229"/>
        <source>New boundary</source>
        <translation>Jauna robeža</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="241"/>
        <source>Move vertex</source>
        <translation>Pārvietot virsotni</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="247"/>
        <source>Add vertex</source>
        <translation>Pievienot virsotni</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="259"/>
        <source>Move element</source>
        <translation>Pārvietot elementu</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="265"/>
        <source>Split line</source>
        <translation>Sadalīt līniju</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="271"/>
        <source>Delete element</source>
        <translation>Dzēst elementu</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="277"/>
        <source>Edit attributes</source>
        <translation>Rediģēt atribūtus</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="282"/>
        <source>Close</source>
        <translation>Aizvērt</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1458"/>
        <source>Warning</source>
        <translation>Brīdinājums</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="201"/>
        <source>You are not owner of the mapset, cannot open the vector for editing.</source>
        <translation>Jūs neesat karšu kopas īpašnieks un tādēļ nav iespējams sākt rediģēt vektoru slāni.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="206"/>
        <source>Cannot open vector for update.</source>
        <translation>Nevar atvērt vektorus atjaunināšanai.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="665"/>
        <source>Info</source>
        <translation>Informācija</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="665"/>
        <source>The table was created</source>
        <translation>Tabula ir izveidota</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1333"/>
        <source>Tool not yet implemented.</source>
        <translation>Rīks vēl nav izstrādāts.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1357"/>
        <source>Cannot check orphan record: </source>
        <translation>Nevar pārbaudīt bāreņa ierakstu: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1364"/>
        <source>Orphan record was left in attribute table. &lt;br&gt;Delete the record?</source>
        <translation>Tabulā ir palicis bāreņu ieraksts. &lt;br&gt;Dzēst ierakstu?</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1373"/>
        <source>Cannot delete orphan record: </source>
        <translation>Nevar dzēst bāreņa ierakstu: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1401"/>
        <source>Cannot describe table for field </source>
        <translation>Nevar aprakstīt tabulu laukam </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="363"/>
        <source>Background</source>
        <translation>Fons</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="364"/>
        <source>Highlight</source>
        <translation>Izcelt</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="365"/>
        <source>Dynamic</source>
        <translation>Dinamisks</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="366"/>
        <source>Point</source>
        <translation>Punkts</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="367"/>
        <source>Line</source>
        <translation>Līnija</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="368"/>
        <source>Boundary (no area)</source>
        <translation>Robeža (nav poligona)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="369"/>
        <source>Boundary (1 area)</source>
        <translation>Robeža (1 poligons)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="370"/>
        <source>Boundary (2 areas)</source>
        <translation>Robeža (2 poligoni)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="371"/>
        <source>Centroid (in area)</source>
        <translation>Centroīds (poligonā)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="372"/>
        <source>Centroid (outside area)</source>
        <translation>Centroīds (ārpus poligona)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="373"/>
        <source>Centroid (duplicate in area)</source>
        <translation>Centroīds (dublikāts poligonā)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="374"/>
        <source>Node (1 line)</source>
        <translation>Mezgls (1 līnija)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="375"/>
        <source>Node (2 lines)</source>
        <translation>Mezgls (2 līnijas)</translation>
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
        <translation>Krāsa</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="412"/>
        <source>Type</source>
        <comment>Column title</comment>
        <translation>Tips</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="414"/>
        <source>Index</source>
        <comment>Column title</comment>
        <translation>Indekss</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="444"/>
        <source>Column</source>
        <translation>Kolonna</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="445"/>
        <source>Type</source>
        <translation>Tips</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="446"/>
        <source>Length</source>
        <translation>Garums</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="501"/>
        <source>Next not used</source>
        <translation>Nākamais neizmantotais</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="502"/>
        <source>Manual entry</source>
        <translation>Manuāla ievade</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="503"/>
        <source>No category</source>
        <translation>Nav katergorijas</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1823"/>
        <source>Right: </source>
        <translation>Pa labi:</translation>
    </message>
</context>
<context>
    <name>QgsGrassEditBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="16"/>
        <source>GRASS Edit</source>
        <translation>GRASS labošana</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="106"/>
        <source>Category</source>
        <translation>Kategorija</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="66"/>
        <source>Mode</source>
        <translation>Režīms</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="170"/>
        <source>Settings</source>
        <translation>Parametri</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="190"/>
        <source>Snapping in screen pixels</source>
        <translation>Snapošana ekrāna pikseļos</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="241"/>
        <source>Symbology</source>
        <translation>Siboloģija</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="274"/>
        <source>Column 1</source>
        <translation>Kolonna 1</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="360"/>
        <source>Table</source>
        <translation>Tabula</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="496"/>
        <source>Add Column</source>
        <translation>Pievienot kolonnu</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="511"/>
        <source>Create / Alter Table</source>
        <translation>Izveidot / Modificēt tabulu</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="298"/>
        <source>Line width</source>
        <translation>Līnijas platums</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="325"/>
        <source>Marker size</source>
        <translation>Marķiera izmērs</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="410"/>
        <source>Layer</source>
        <translation>Slānis</translation>
    </message>
</context>
<context>
    <name>QgsGrassElementDialog</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="132"/>
        <source>Cancel</source>
        <translation>Atcelt</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="163"/>
        <source>Ok</source>
        <translation>Labi</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="168"/>
        <source>&lt;font color=&apos;red&apos;&gt;Enter a name!&lt;/font&gt;</source>
        <translation>&lt;font color=&apos;red&apos;&gt;Ievadiet nosaukumu!&lt;/font&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="179"/>
        <source>&lt;font color=&apos;red&apos;&gt;This is name of the source!&lt;/font&gt;</source>
        <translation>&lt;font color=&apos;red&apos;&gt;Šis ir avota nosaukums!&lt;/font&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="185"/>
        <source>&lt;font color=&apos;red&apos;&gt;Exists!&lt;/font&gt;</source>
        <translation>&lt;font color=&apos;red&apos;&gt;Eksistē!&lt;/font&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="186"/>
        <source>Overwrite</source>
        <translation>Pārrakstīt</translation>
    </message>
</context>
<context>
    <name>QgsGrassMapcalc</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="111"/>
        <source>Mapcalc tools</source>
        <translation>Mapcalc rīki</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="114"/>
        <source>Add map</source>
        <translation>Pievienot karti</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="121"/>
        <source>Add constant value</source>
        <translation>Pievienot konstantu vērtību</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="128"/>
        <source>Add operator or function</source>
        <translation>Pievienot operatoru vai funkciju</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="135"/>
        <source>Add connection</source>
        <translation>Pievienot savienojumu</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="142"/>
        <source>Select item</source>
        <translation>Izvēlēties vienību</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="149"/>
        <source>Delete selected item</source>
        <translation>Dzēst izvēlēto vienību</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="159"/>
        <source>Open</source>
        <translation>Atvērt</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="164"/>
        <source>Save</source>
        <translation>Saglabāt</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="170"/>
        <source>Save as</source>
        <translation>Saglabāt kā</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="178"/>
        <source>Addition</source>
        <translation>Pieskaitīt</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="179"/>
        <source>Subtraction</source>
        <translation>Atņemt</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="180"/>
        <source>Multiplication</source>
        <translation>Reizināt</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="181"/>
        <source>Division</source>
        <translation>Dalīt</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="182"/>
        <source>Modulus</source>
        <translation>Modulis</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="183"/>
        <source>Exponentiation</source>
        <translation>Eksponents</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="186"/>
        <source>Equal</source>
        <translation>Vienāds</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="187"/>
        <source>Not equal</source>
        <translation>Nav vienāds</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="188"/>
        <source>Greater than</source>
        <translation>Lielāks</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="189"/>
        <source>Greater than or equal</source>
        <translation>Lielāks vai vienāds</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="190"/>
        <source>Less than</source>
        <translation>Mazāks</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="191"/>
        <source>Less than or equal</source>
        <translation>Mazāks vai vienāds</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="192"/>
        <source>And</source>
        <translation>Un</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="193"/>
        <source>Or</source>
        <translation>Vai</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="196"/>
        <source>Absolute value of x</source>
        <translation>Absolūtā x vērtība</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="197"/>
        <source>Inverse tangent of x (result is in degrees)</source>
        <translation>Inversais tangenss no x (rezultāts ir grādos)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="198"/>
        <source>Inverse tangent of y/x (result is in degrees)</source>
        <translation>Inversais tangenss no y/x (rezultāts ir grādos)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="199"/>
        <source>Current column of moving window (starts with 1)</source>
        <translation>Pašreizējā kolonna kustīgajā logā (sākas ar 1)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="200"/>
        <source>Cosine of x (x is in degrees)</source>
        <translation>Kosinuss no x (x ir grādos)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="201"/>
        <source>Convert x to double-precision floating point</source>
        <translation>Konvertē x uz dubultas precizitātes daļskaitli</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="202"/>
        <source>Current east-west resolution</source>
        <translation>Pašreizējā austrumu-rietumu izšķirtspēja</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="203"/>
        <source>Exponential function of x</source>
        <translation>Eksponenta funkcija no x</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="204"/>
        <source>x to the power y</source>
        <translation>x pakāpē y</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="205"/>
        <source>Convert x to single-precision floating point</source>
        <translation>Konvertē x uz vienas precizitātes daļskaitli</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="206"/>
        <source>Decision: 1 if x not zero, 0 otherwise</source>
        <translation>Lēmums: 1, ja x nav nulle, 0 pretējā gadījumā</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="207"/>
        <source>Decision: a if x not zero, 0 otherwise</source>
        <translation>Lēmums: a, ja x nav nulle, 0 pretējā gadījumā</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="208"/>
        <source>Decision: a if x not zero, b otherwise</source>
        <translation>Lēmums: a, ja x nav nulle, b pretējā gadījumā</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="209"/>
        <source>Decision: a if x &gt; 0, b if x is zero, c if x &lt; 0</source>
        <translation>Lēmums: a, ja x &gt; 0, b, ja x ir nulle, c, ja x &lt; 0</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="210"/>
        <source>Convert x to integer [ truncates ]</source>
        <translation>Konvertē x uz veselu skaitli [nocērt]</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="211"/>
        <source>Check if x = NULL</source>
        <translation>Pārbaudīt vai X = NULL</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="212"/>
        <source>Natural log of x</source>
        <translation>Naturāllogaritms no X</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="213"/>
        <source>Log of x base b</source>
        <translation>Logaritms no x pie bāzes b</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="215"/>
        <source>Largest value</source>
        <translation>Lielākā vērtība</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="217"/>
        <source>Median value</source>
        <translation>Mediānas vērtība</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="219"/>
        <source>Smallest value</source>
        <translation>Mazākā vērtība</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="221"/>
        <source>Mode value</source>
        <translation>Modas vērtība</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="222"/>
        <source>1 if x is zero, 0 otherwise</source>
        <translation>1, ja x ir nulle, 0 pretējā gadījumā</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="223"/>
        <source>Current north-south resolution</source>
        <translation>Pašreizējā ziemeļu-dienvidu izšķirtspēja</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="224"/>
        <source>NULL value</source>
        <translation>NULL vērtība</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="225"/>
        <source>Random value between a and b</source>
        <translation>Gadījumskaitlis robežās no a līdz b</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="226"/>
        <source>Round x to nearest integer</source>
        <translation>Noapaļot X līdz tuvākajam veselajam skaitlim</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="227"/>
        <source>Current row of moving window (Starts with 1)</source>
        <translation>Pašreizējā rinda kustīgajā logā (sākas ar 1)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="228"/>
        <source>Sine of x (x is in degrees)</source>
        <comment>sin(x)</comment>
        <translation>Sinuss no x (x ir grādos)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="229"/>
        <source>Square root of x</source>
        <comment>sqrt(x)</comment>
        <translation>Kvadrātsakne no x</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="230"/>
        <source>Tangent of x (x is in degrees)</source>
        <comment>tan(x)</comment>
        <translation>Tangenss no x (x ir grādos)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="231"/>
        <source>Current x-coordinate of moving window</source>
        <translation>Pašreizējā X koordināta kustīgajā logā</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="232"/>
        <source>Current y-coordinate of moving window</source>
        <translation>Pašreizējā Y koordināta kustīgajā logā</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1317"/>
        <source>Warning</source>
        <translation>Brīdinājums</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="584"/>
        <source>Cannot get current region</source>
        <translation>Nevar saņemt pašreizējo reģionu</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="561"/>
        <source>Cannot check region of map </source>
        <translation>Nevar pārbaudīt reģionu kartei </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="617"/>
        <source>Cannot get region of map </source>
        <translation>Nevar saņemt reģionu kartei </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="813"/>
        <source>No GRASS raster maps currently in QGIS</source>
        <translation>QGIS nav GRASS rastra kartes</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1102"/>
        <source>Cannot create &apos;mapcalc&apos; directory in current mapset.</source>
        <translation>Nebija iespējams izveidot &apos;mapcalc&apos; mapi pašreizējā karšu kopā.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1112"/>
        <source>New mapcalc</source>
        <translation>Jauns mapcalc</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1113"/>
        <source>Enter new mapcalc name:</source>
        <translation>Ievadiet jaunā mapcalc nosaukumu:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1118"/>
        <source>Enter vector name</source>
        <translation>Ievadiet vektora nosaukumu</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1126"/>
        <source>The file already exists. Overwrite? </source>
        <translation>Fails jau eksistē. Pārrakstīt? </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1164"/>
        <source>Save mapcalc</source>
        <translation>Saglabāt mapcalc</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1146"/>
        <source>File name empty</source>
        <translation>Faila nosaukums ir tukšs</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1165"/>
        <source>Cannot open mapcalc file</source>
        <translation>Nevar atvērt mapcalc failu</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1295"/>
        <source>The mapcalc schema (</source>
        <translation>Mapcalc shēma (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1295"/>
        <source>) not found.</source>
        <translation>) nav atrasta.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1302"/>
        <source>Cannot open mapcalc schema (</source>
        <translation>Nevar atvērt mapcalc shēmu(</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1313"/>
        <source>Cannot read mapcalc schema (</source>
        <translation>Nevar nolasīt mapcalc shēmu (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1314"/>
        <source>
at line </source>
        <translation>
rindā </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1315"/>
        <source> column </source>
        <translation> kolonā </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1388"/>
        <source>Output</source>
        <translation>Izvade</translation>
    </message>
</context>
<context>
    <name>QgsGrassMapcalcBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalcbase.ui" line="16"/>
        <source>MainWindow</source>
        <translation>GalvenaisLogs</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalcbase.ui" line="37"/>
        <source>Output</source>
        <translation>Izvade</translation>
    </message>
</context>
<context>
    <name>QgsGrassModule</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1380"/>
        <source>Run</source>
        <translation>Darbināt</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1358"/>
        <source>Stop</source>
        <translation>Apturēt</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="205"/>
        <source>Module</source>
        <translation>Modulis</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1352"/>
        <source>Warning</source>
        <translation>Brīdinājums</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="220"/>
        <source>The module file (</source>
        <translation>Moduļa fails (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="220"/>
        <source>) not found.</source>
        <translation>) nav atrasts.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="224"/>
        <source>Cannot open module file (</source>
        <translation>Nevar atvērt moduļa failu (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="992"/>
        <source>)</source>
        <translation>)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="987"/>
        <source>Cannot read module file (</source>
        <translation>Nevar nolasīt moduļa failu (</translation>
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
rindā </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="260"/>
        <source>Module </source>
        <translation>Modulis</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="260"/>
        <source> not found</source>
        <translation>nav atrasta</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="301"/>
        <source>Cannot find man page </source>
        <translation>Nevar atrast &quot;man&quot; lapu</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="981"/>
        <source>Not available, cannot open description (</source>
        <translation>Nav pieejams, nevar atvērt aprakstu (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="988"/>
        <source> column </source>
        <translation> kolonā </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="992"/>
        <source>Not available, incorrect description (</source>
        <translation>Nav pieejams, nekorekts apraksts (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1179"/>
        <source>Cannot get input region</source>
        <translation>Nevar saņemt ievades reģionu</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1167"/>
        <source>Use Input Region</source>
        <translation>Lietot ievades reģionu</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1281"/>
        <source>Cannot find module </source>
        <translation>Nevar atrast moduli </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1353"/>
        <source>Cannot start module: </source>
        <translation>Nevar startēt moduli: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1369"/>
        <source>&lt;B&gt;Successfully finished&lt;/B&gt;</source>
        <translation>&lt;b&gt;Sekmīgi pabeigts&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1375"/>
        <source>&lt;B&gt;Finished with error&lt;/B&gt;</source>
        <translation>&lt;b&gt;Beidza ar kļūdu&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1378"/>
        <source>&lt;B&gt;Module crashed or killed&lt;/B&gt;</source>
        <translation>&lt;b&gt;Modulis avarēja vai tika nogalitāts&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="978"/>
        <source>Not available, description not found (</source>
        <translation>Nav pieejams, apraksts nav atrasts (</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="13"/>
        <source>GRASS Module</source>
        <translation>GRASS modulis</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="26"/>
        <source>Options</source>
        <translation>Parametri</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="31"/>
        <source>Output</source>
        <translation>Izvade</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="47"/>
        <source>Manual</source>
        <translation>Manuāls</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="118"/>
        <source>Run</source>
        <translation>Darbināt</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="161"/>
        <source>Close</source>
        <translation>Aizvērt</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="141"/>
        <source>View output</source>
        <translation>Aplūkot izvadi</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="74"/>
        <source>TextLabel</source>
        <translation>TekstaBirka</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleField</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2714"/>
        <source>Attribute field</source>
        <translation>Atribūta lauks</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleFile</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2916"/>
        <source>File</source>
        <translation>Fails</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="3029"/>
        <source>:&amp;nbsp;missing value</source>
        <translation>:&amp;nbsp;trūkst vērtība</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="3036"/>
        <source>:&amp;nbsp;directory does not exist</source>
        <translation>:&amp;nbsp;mape neeksistē</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleGdalInput</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2665"/>
        <source>Warning</source>
        <translation>Brīdinājums</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2527"/>
        <source>Cannot find layeroption </source>
        <translation>Nevar atrast slāņa iestatījumus</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2669"/>
        <source>PostGIS driver in OGR does not support schemas!&lt;br&gt;Only the table name will be used.&lt;br&gt;It can result in wrong input if more tables of the same name&lt;br&gt;are present in the database.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2692"/>
        <source>:&amp;nbsp;no input</source>
        <translation>:&amp;nbsp;nav ievades</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2540"/>
        <source>Cannot find whereoption </source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleInput</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2071"/>
        <source>Warning</source>
        <translation>Brīdinājums</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1984"/>
        <source>Cannot find typeoption </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1993"/>
        <source>Cannot find values for typeoption </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2054"/>
        <source>Cannot find layeroption </source>
        <translation>Nevar atrast slāņa iestatījumus</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2071"/>
        <source>GRASS element </source>
        <translation>GRASS elemants</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2071"/>
        <source> not supported</source>
        <translation>nav atbalstīts</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2095"/>
        <source>Use region of this map</source>
        <translation>Lietot šīs kartes reģionu</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2431"/>
        <source>:&amp;nbsp;no input</source>
        <translation>:&amp;nbsp;nav ievades</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleOption</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1900"/>
        <source>:&amp;nbsp;missing value</source>
        <translation>:&amp;nbsp;trūkst vērtība</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleSelection</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2803"/>
        <source>Attribute field</source>
        <translation>Atribūta lauks</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleStandardOptions</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="886"/>
        <source>Warning</source>
        <translation>Brīdinājums</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="369"/>
        <source>Cannot find module </source>
        <translation>Nevar atrast moduli </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="386"/>
        <source>Cannot start module </source>
        <translation>Nevar startēt moduli </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="399"/>
        <source>Cannot read module description (</source>
        <translation>Nevar nolasīt moduļa aprakstu (</translation>
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
        <translation>
rindā </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="400"/>
        <source> column </source>
        <translation> kolonā </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="424"/>
        <source>Cannot find key </source>
        <translation>Nevar atrast atslēgu </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="560"/>
        <source>Item with id </source>
        <translation>Vienums ar id </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="560"/>
        <source> not found</source>
        <translation> nav atrasts</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="848"/>
        <source>Cannot get current region</source>
        <translation>Nevar saņemt pašreizējo reģionu</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="825"/>
        <source>Cannot check region of map </source>
        <translation>Nevar pārbaudīt reģionu kartei </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="887"/>
        <source>Cannot set region of map </source>
        <translation>Nevar iestatīt kartes reģionu </translation>
    </message>
</context>
<context>
    <name>QgsGrassNewMapset</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="124"/>
        <source>GRASS database</source>
        <translation>GRASS datubāze</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="125"/>
        <source>GRASS location</source>
        <translation>GRASS novietojums</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="126"/>
        <source>Projection</source>
        <translation>Projekcija</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="127"/>
        <source>Default GRASS Region</source>
        <translation>GRASS noklusējuma reģions</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="184"/>
        <source>Mapset</source>
        <translation>Karšu kopa</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="129"/>
        <source>Create New Mapset</source>
        <translation>Izveidot jaunu karšu kopu</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="158"/>
        <source>Tree</source>
        <translation>Koks</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="159"/>
        <source>Comment</source>
        <translation>Komentārs</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="160"/>
        <source>Database</source>
        <translation>Datu bāze</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="164"/>
        <source>Location 2</source>
        <translation>Novietojums 2</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="175"/>
        <source>User&apos;s mapset</source>
        <translation>Lietotāja karšu kopa</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="177"/>
        <source>System mapset</source>
        <translation>Sistēmas karšu kopa</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="171"/>
        <source>Location 1</source>
        <translation>Novietojums 1</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="185"/>
        <source>Owner</source>
        <translation>Īpašnieks</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="233"/>
        <source>Enter path to GRASS database</source>
        <translation>Ievadiet ceļu līdz GRASS datu bāzei</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="241"/>
        <source>The directory doesn&apos;t exist!</source>
        <translation>Mape neeksistē!</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="271"/>
        <source>No writable locations, the database not writable!</source>
        <translation>Nav rakstāmu novietojumu, datu bāze nav rakstāma!</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="369"/>
        <source>Enter location name!</source>
        <translation>Ievadiet novietojuma nosaukumu!</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="382"/>
        <source>The location exists!</source>
        <translation>Novietojums jau eksistē!</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="533"/>
        <source>Selected projection is not supported by GRASS!</source>
        <translation>Izvēlēto projekciju GRASS neatbalsta!</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1160"/>
        <source>Warning</source>
        <translation>Brīdinājums</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="580"/>
        <source>Cannot create projection.</source>
        <translation>Nevar izveidot projekciju.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="629"/>
        <source>Cannot reproject previously set region, default region set.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="764"/>
        <source>North must be greater than south</source>
        <translation>Ziemeļiem ir jābūt lielākiem par dienvidiem</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="769"/>
        <source>East must be greater than west</source>
        <translation>Austrumiem ir jābūt lielākiem par rietumiem</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="816"/>
        <source>Regions file (</source>
        <translation>Reģiona fails (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="816"/>
        <source>) not found.</source>
        <translation>) nav atrasts.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="821"/>
        <source>Cannot open locations file (</source>
        <translation>Nevar atvērt novietojumu failu (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="821"/>
        <source>)</source>
        <translation>)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="830"/>
        <source>Cannot read locations file (</source>
        <translation>Nevar nolasīt novietojumu failu (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="831"/>
        <source>):
</source>
        <translation>):
</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="831"/>
        <source>
at line </source>
        <translation>
rindā </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="832"/>
        <source> column </source>
        <translation> kolonā </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1161"/>
        <source>Cannot create QgsSpatialRefSys</source>
        <translation>Nevar izveidot QgsSpatialRefSys</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="968"/>
        <source>Cannot reproject selected region.</source>
        <translation>Nevar pārprojecēt izvēlēto reģionu.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1057"/>
        <source>Cannot reproject region</source>
        <translation>Nevar pārprojecēt reģionu</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1289"/>
        <source>Enter mapset name.</source>
        <translation>Ievadiet karšu kopas nosaukumu.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1306"/>
        <source>The mapset already exists</source>
        <translation>Karšu kopa jau eksistē</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1330"/>
        <source>Database: </source>
        <translation>Datu bāze: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1341"/>
        <source>Location: </source>
        <translation>Novietojums: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1343"/>
        <source>Mapset: </source>
        <translation>Karšu kopa: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1378"/>
        <source>Create location</source>
        <translation>Izveidot novietojumu</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1380"/>
        <source>Cannot create new location: </source>
        <translation>Nevar izveidot jaunu novietojumu: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1427"/>
        <source>Create mapset</source>
        <translation>Izveidot karšu kopu</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1420"/>
        <source>Cannot open DEFAULT_WIND</source>
        <translation>Nevar atvērt DEFAULT_WIND</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1427"/>
        <source>Cannot open WIND</source>
        <translation>Nevar atvērt WIND</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1454"/>
        <source>New mapset</source>
        <translation>Jauna karšu kopa</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1450"/>
        <source>New mapset successfully created, but cannot be opened: </source>
        <translation>Jauna karšu kopa tika sekmīgi izveidota, bet nevar tikt atvērta: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1456"/>
        <source>New mapset successfully created and set as current working mapset.</source>
        <translation>Jauna karšu kopa tika sekmīgi izveidota un iestatīta par darba karšu kopu.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1410"/>
        <source>Cannot create new mapset directory</source>
        <translation>Nevar izveidot jaunu karšu kopas mapi</translation>
    </message>
</context>
<context>
    <name>QgsGrassNewMapsetBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2068"/>
        <source>Column 1</source>
        <translation>Kolonna 1</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="88"/>
        <source>Example directory tree:</source>
        <translation>Mapju koka piemērs:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="95"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;GRASS data are stored in tree directory structure.&lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;The GRASS database is the top-level directory in this tree structure.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;GRASS dati tiek glabāti kokveida mapju struktūrā.&lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;GRASS datu bāze ir augstākais līmenis šajā kokveida struktūrā.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="399"/>
        <source>Database Error</source>
        <translation>Datu bāzes kļūda</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2153"/>
        <source>Database:</source>
        <translation>Datu bāze:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="440"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="457"/>
        <source>Select existing directory or create a new one:</source>
        <translation>Izvēlieties eksistējošu mapi vai izveidojiet jaunu:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="508"/>
        <source>Location</source>
        <translation>Novietojums</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="535"/>
        <source>Select location</source>
        <translation>Izvēlēties novietojumu</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="552"/>
        <source>Create new location</source>
        <translation>Izveidot jaunu novietojumu</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="832"/>
        <source>Location Error</source>
        <translation>Novietojuma kļūda</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="848"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;The GRASS location is a collection of maps for a particular territory or project.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;GRASS novietojums ir konkrētās teritorijas vai projekta karšu apkopojums.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1159"/>
        <source>Projection Error</source>
        <translation>Projekcijas kļūda</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1174"/>
        <source>Coordinate system</source>
        <translation>Koordinātu sistēma</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1186"/>
        <source>Projection</source>
        <translation>Projekcija</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1193"/>
        <source>Not defined</source>
        <translation>Nav definēta</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1273"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;The GRASS region defines a workspace for raster modules. The default region is valid for one location. It is possible to set a different region in each mapset. &lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;It is possible to change the default location region later.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;GRASS reģions definē rastra moduļu darba telpu. Noklusējuma reģions ir derīgs tikai vienam novietojumam. Katrai karšu kopai var norādīt savu reģionu. &lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Ir iespējams vēlāk izmainīt noklusējuma reģionu.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1334"/>
        <source>Set current QGIS extent</source>
        <translation>Iestatīt pašreizējo QGIS apjomu</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1376"/>
        <source>Set</source>
        <translation>Iestatīt</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1396"/>
        <source>Region Error</source>
        <translation>Reģiona kļūda</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1441"/>
        <source>S</source>
        <translation>D</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1500"/>
        <source>W</source>
        <translation>R</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1555"/>
        <source>E</source>
        <translation>A</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1614"/>
        <source>N</source>
        <translation>Z</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1699"/>
        <source>New mapset:</source>
        <translation>Jauna karšu kopa:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1988"/>
        <source>Mapset Error</source>
        <translation>Karšu kopas kļūda</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2045"/>
        <source>&lt;p align=&quot;center&quot;&gt;Existing masets&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;center&quot;&gt;Eksistējošās karšu kopas&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2101"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;The GRASS mapset is a collection of maps used by one user. &lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;A user can read maps from all mapsets in the location but &lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;he can open for writing only his mapset (owned by user).&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;GRASS karšu kopa ir karšu kolekcija, ko lieto viens lietotājs. &lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Lietotājs var lasīt jebkurā karšu kopā esošās kartes, &lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;bet rakstīt var tikai viņam piedeošajā karšu kopā.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2174"/>
        <source>Location:</source>
        <translation>Location:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2195"/>
        <source>Mapset:</source>
        <translation>Karšu kopa:</translation>
    </message>
</context>
<context>
    <name>QgsGrassPlugin</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="198"/>
        <source>GRASS</source>
        <translation>GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="788"/>
        <source>&amp;GRASS</source>
        <translation>&amp;GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="145"/>
        <source>Open mapset</source>
        <translation>Atvērt karšu kopu</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="146"/>
        <source>New mapset</source>
        <translation>Jauna karšu kopa</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="147"/>
        <source>Close mapset</source>
        <translation>Aizvērt karšu kopu</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="150"/>
        <source>Add GRASS vector layer</source>
        <translation>Pievienot GRASS vektoru slāni</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="152"/>
        <source>Add GRASS raster layer</source>
        <translation>Pievienot GRASS rastra slāni</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="168"/>
        <source>Open GRASS tools</source>
        <translation>Atvērt GRASS rīkus</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="157"/>
        <source>Display Current Grass Region</source>
        <translation>Rādīt pašreizējo GRASS reģionu</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="161"/>
        <source>Edit Current Grass Region</source>
        <translation>Rediģēt pašreizējo GRASS reģionu</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="163"/>
        <source>Edit Grass Vector layer</source>
        <translation>Rediģēt GRASS vektoru slāni</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="166"/>
        <source>Adds a GRASS vector layer to the map canvas</source>
        <translation>Pievieno GRASS vektoru slāni kartes audeklam</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="167"/>
        <source>Adds a GRASS raster layer to the map canvas</source>
        <translation>Pievieno GRASS rastra slāni kartes audeklam</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="169"/>
        <source>Displays the current GRASS region as a rectangle on the map canvas</source>
        <translation>Parāda pašreizējo GRASS reģionu kā taisnstūri uz kartes audekla</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="170"/>
        <source>Edit the current GRASS region</source>
        <translation>Rediģēt pašreizējo GRASS reģionu</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="171"/>
        <source>Edit the currently selected GRASS vector layer.</source>
        <translation>Rediģēt pašreiz izvēlēto GRASS vektoru slāni.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="82"/>
        <source>GrassVector</source>
        <translation>GRASSVektors</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="83"/>
        <source>0.1</source>
        <translation>0.1</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="84"/>
        <source>GRASS layer</source>
        <translation>GRASS slānis</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="164"/>
        <source>Create new Grass Vector</source>
        <translation>Izveidot jaunu GRASS vektoru slāni</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="758"/>
        <source>Warning</source>
        <translation>Brīdinājums</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="470"/>
        <source>GRASS Edit is already running.</source>
        <translation>GRASS rediģēšana jau darbojas.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="479"/>
        <source>New vector name</source>
        <translation>Jauns vektoru nosaukums</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="495"/>
        <source>Cannot create new vector: </source>
        <translation>Nevar izveidot jaunu vektoru: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="515"/>
        <source>New vector created but cannot be opened by data provider.</source>
        <translation>Jauna vektordatu kopa ir izveidota, taču datu sniedzējs nevar to atvērt.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="526"/>
        <source>Cannot start editing.</source>
        <translation>Nevar sākt rediģēt.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="561"/>
        <source>GISDBASE, LOCATION_NAME or MAPSET is not set, cannot display current region.</source>
        <translation>GISDBASE, LOCATION_NAME vai MAPSET nav iestatīts, tādēļ nevar parādīt pašreizējo reģionu.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="571"/>
        <source>Cannot read current region: </source>
        <translation>Nevar nolasīt pašreizējo reģionu: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="675"/>
        <source>Cannot open the mapset. </source>
        <translation>Nevar atvērt karšu kopu. </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="693"/>
        <source>Cannot close mapset. </source>
        <translation>Nevar aizvērt karšu kopu. </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="749"/>
        <source>Cannot close current mapset. </source>
        <translation>Nevar aizvērt pašreizējo karšu kopu. </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="758"/>
        <source>Cannot open GRASS mapset. </source>
        <translation>Nevar atvērt GRASS karšu kopu. </translation>
    </message>
</context>
<context>
    <name>QgsGrassRegion</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregion.cpp" line="459"/>
        <source>Warning</source>
        <translation>Brīdinājums</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregion.cpp" line="195"/>
        <source>GISDBASE, LOCATION_NAME or MAPSET is not set, cannot display current region.</source>
        <translation>GISDBASE, LOCATION_NAME vai MAPSET nav iestatīts, tādēļ nevar parādīt pašreizējo reģionu.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregion.cpp" line="202"/>
        <source>Cannot read current region: </source>
        <translation>Nevar nolasīt pašreizējo reģionu: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregion.cpp" line="459"/>
        <source>Cannot write region</source>
        <translation>Nevar rakstīt reģionu</translation>
    </message>
</context>
<context>
    <name>QgsGrassRegionBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="13"/>
        <source>GRASS Region Settings</source>
        <translation>GRASS reģiona parametri</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="76"/>
        <source>N</source>
        <translation>Z</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="146"/>
        <source>W</source>
        <translation>R</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="172"/>
        <source>E</source>
        <translation>A</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="236"/>
        <source>S</source>
        <translation>D</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="280"/>
        <source>N-S Res</source>
        <translation>Z-D Izšķ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="293"/>
        <source>Rows</source>
        <translation>Rindas</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="303"/>
        <source>Cols</source>
        <translation>Kolonnas</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="316"/>
        <source>E-W Res</source>
        <translation>A-R Izšķ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="364"/>
        <source>Color</source>
        <translation>Krāsa</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="384"/>
        <source>Width</source>
        <translation>Platums</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="464"/>
        <source>OK</source>
        <translation>Labi</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="487"/>
        <source>Cancel</source>
        <translation>Atcelt</translation>
    </message>
</context>
<context>
    <name>QgsGrassSelect</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="68"/>
        <source>Select GRASS Vector Layer</source>
        <translation>Izvēlieties GRASS vektoru slāni</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="75"/>
        <source>Select GRASS Raster Layer</source>
        <translation>Izvēlieties GRASS rastra slāni</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="82"/>
        <source>Select GRASS mapcalc schema</source>
        <translation>Izvēlieties GRASS mapcalc shēmu</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="90"/>
        <source>Select GRASS Mapset</source>
        <translation>Izvēlieties GRASS karšu kopu</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="409"/>
        <source>Warning</source>
        <translation>Brīdinājums</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="409"/>
        <source>Cannot open vector on level 2 (topology not available).</source>
        <translation>Nevar atvērt otrā līmeņa vektoru failu (topoloģija nav pieejama).</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="466"/>
        <source>Choose existing GISDBASE</source>
        <translation>Izvēlieties eksistējošu GISDBASE</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="482"/>
        <source>Wrong GISDBASE, no locations available.</source>
        <translation>Nederīga GISDBASE, nav pieejami novietojumi.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="483"/>
        <source>Wrong GISDBASE</source>
        <translation>Nepareiza GISDBASE</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="500"/>
        <source>Select a map.</source>
        <translation>Izvēlieties karti.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="501"/>
        <source>No map</source>
        <translation>Nav kartes</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="509"/>
        <source>No layer</source>
        <translation>Nav slāņa</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="510"/>
        <source>No layers available in this map</source>
        <translation>Šai kartei slāņi nav pieejami</translation>
    </message>
</context>
<context>
    <name>QgsGrassSelectBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="21"/>
        <source>Add GRASS Layer</source>
        <translation>Pievienot GRASS slāni</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="65"/>
        <source>Gisdbase</source>
        <translation>Gisdbase</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="78"/>
        <source>Location</source>
        <translation>Location</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="85"/>
        <source>Mapset</source>
        <translation>Mapset</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="102"/>
        <source>Select or type map name (wildcards &apos;*&apos; and &apos;?&apos; accepted for rasters)</source>
        <translation>Izvēlieties vai ierakstiet kartes nosaukumu (rastra nosaukumiem atļauts lietot &apos;*&apos; un &apos;?&apos; aizstājējzīmes)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="118"/>
        <source>Map name</source>
        <translation>Kartes nosaukums</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="125"/>
        <source>Layer</source>
        <translation>Slānis</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="161"/>
        <source>Browse</source>
        <translation>Pārlūkot</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="168"/>
        <source>Cancel</source>
        <translation>Atcelt</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="175"/>
        <source>OK</source>
        <translation>Labi</translation>
    </message>
</context>
<context>
    <name>QgsGrassShellBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassshellbase.ui" line="19"/>
        <source>GRASS Shell</source>
        <translation>GRASS čaula</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassshellbase.ui" line="49"/>
        <source>Close</source>
        <translation>Aizvērt</translation>
    </message>
</context>
<context>
    <name>QgsGrassTools</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="146"/>
        <source>Modules</source>
        <translation>Moduļi</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="205"/>
        <source>Browser</source>
        <translation>Pārlūks</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="123"/>
        <source>GRASS Tools</source>
        <translation>GRASS rīki</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="470"/>
        <source>GRASS Tools: </source>
        <translation>GRASS rīki: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="358"/>
        <source>Warning</source>
        <translation>Brīdinājums</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="271"/>
        <source>Cannot find MSYS (</source>
        <translation>Nav iespējams atrast MSYS(</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="293"/>
        <source>GRASS Shell is not compiled.</source>
        <translation>GRASS čaula nav kompilēta.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="343"/>
        <source>The config file (</source>
        <translation>Konfigurācijas fails (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="343"/>
        <source>) not found.</source>
        <translation>) nav atrasts.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="347"/>
        <source>Cannot open config file (</source>
        <translation>Nevar atvērt konfigurācijas failu (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="347"/>
        <source>)</source>
        <translation>)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="355"/>
        <source>Cannot read config file (</source>
        <translation>Nevar noalsīt konfigurācijas failu (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="356"/>
        <source>
at line </source>
        <translation>
rindā </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="356"/>
        <source> column </source>
        <translation> kolonā </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="143"/>
        <source>Modules Tree</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="182"/>
        <source>Modules List</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGridMakerPlugin</name>
    <message>
        <location filename="../src/plugins/grid_maker/plugin.cpp" line="93"/>
        <source>&amp;Graticule Creator</source>
        <translation>&amp;Grādu tīkla veidotājs</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugin.cpp" line="94"/>
        <source>Creates a graticule (grid) and stores the result as a shapefile</source>
        <translation>Izveido grādu tīklu un saglabā to kā shapefile</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugin.cpp" line="136"/>
        <source>&amp;Graticules</source>
        <translation>&amp;Grādu tīkls</translation>
    </message>
</context>
<context>
    <name>QgsGridMakerPluginGui</name>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="51"/>
        <source>QGIS - Grid Maker</source>
        <translation>QGIS - Grādu tīkla veidotājs</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="108"/>
        <source>Choose a filename to save under</source>
        <translation>Izvēlieties faila nosaukumu ar kādu saglabāt</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="110"/>
        <source>ESRI Shapefile (*.shp)</source>
        <translation>ESRI Shapefile (*.shp)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="52"/>
        <source>Please enter the file name before pressing OK!</source>
        <translation>Pirms spiest Labi, ievadiet faila nosaukumu!</translation>
    </message>
</context>
<context>
    <name>QgsGridMakerPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="256"/>
        <source>Graticule Builder</source>
        <translation>Grādu tīkla veidotājs</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="186"/>
        <source>Type</source>
        <translation>Tips</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="198"/>
        <source>Point</source>
        <translation>Punkts</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="208"/>
        <source>Polygon</source>
        <translation>Poligons</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="110"/>
        <source>Origin (lower left)</source>
        <translation>Izcelsme (apakšējais kreisais)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="69"/>
        <source>End point (upper right)</source>
        <translation>Beigu punkts (augsējais labais)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="161"/>
        <source>Output (shape) file</source>
        <translation>Izvades fails</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="176"/>
        <source>Save As...</source>
        <translation>Saglabāt kā...</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="13"/>
        <source>QGIS Graticule Creator</source>
        <translation>QGIS grādu tīkla veidotājs</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="28"/>
        <source>Graticle size</source>
        <translation>Grādu tīkla elementa izmērs</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="46"/>
        <source>Y Interval:</source>
        <translation>Y intervāls:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="56"/>
        <source>X Interval:</source>
        <translation>X intervāls:</translation>
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
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsHelpViewer</name>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="185"/>
        <source>Quantum GIS Help - </source>
        <translation>Quantum GIS palīdzība - </translation>
    </message>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="191"/>
        <source>Failed to get the help text from the database</source>
        <translation>Neizdevās nolasīt palīdzības tekstu no datubāzes</translation>
    </message>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="214"/>
        <source>Error</source>
        <translation>Kļūda</translation>
    </message>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="215"/>
        <source>The QGIS help database is not installed</source>
        <translation>QGIS palīdzības datubāze nav instalēta</translation>
    </message>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="139"/>
        <source>This help file does not exist for your language</source>
        <translation>Palīdzības fails jūsu valodai neeksistē</translation>
    </message>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="142"/>
        <source>If you would like to create it, contact the QGIS development team</source>
        <translation>Ja jūs vēlaties to izveidot, sazinieties ar QGIS izstrādes komandu</translation>
    </message>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="157"/>
        <source>Quantum GIS Help</source>
        <translation>Quantum GIS palīdzība</translation>
    </message>
</context>
<context>
    <name>QgsHelpViewerBase</name>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="13"/>
        <source>QGIS Help</source>
        <translation>QGIS palīdzība</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="39"/>
        <source>&amp;Home</source>
        <translation>&amp;Mājas</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="42"/>
        <source>Alt+H</source>
        <translation>Alt+H</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="52"/>
        <source>&amp;Forward</source>
        <translation>&amp;Uz priekšu</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="55"/>
        <source>Alt+F</source>
        <translation>Alt+F</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="65"/>
        <source>&amp;Back</source>
        <translation>&amp;Atpakaļ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="68"/>
        <source>Alt+B</source>
        <translation>Alt+B</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="78"/>
        <source>&amp;Close</source>
        <translation>&amp;Aizvērt</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="81"/>
        <source>Alt+C</source>
        <translation>Alt+C</translation>
    </message>
</context>
<context>
    <name>QgsHttpTransaction</name>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="234"/>
        <source>WMS Server responded unexpectedly with HTTP Status Code %1 (%2)</source>
        <translation>WMS serveris negaidīti atbildēja ar HTTP statusa kodu %1 (%2)</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="313"/>
        <source>HTTP response completed, however there was an error: %1</source>
        <translation>Saņemta HTTP atbilde, taču bija kļūda: %1</translation>
    </message>
    <message numerus="yes">
        <location filename="../src/core/qgshttptransaction.cpp" line="441"/>
        <source>Network timed out after %1 seconds of inactivity.
This may be a problem in your network connection or at the WMS server.</source>
        <translation type="unfinished">
            <numerusform>Tīklā iestājās noildze pēc %1 sekunžu aktivitātes trūkuma.
Tā var būt problēma ar jūsu tīkla savienojumu vai WMS serveri.</numerusform>
        </translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="362"/>
        <source>HTTP transaction completed, however there was an error: %1</source>
        <translation>HTTP transakcija ir pabeigta, taču bija kļūda: %1</translation>
    </message>
</context>
<context>
    <name>QgsIDWInterpolatorDialogBase</name>
    <message>
        <location filename="../src/plugins/interpolation/qgsidwinterpolatordialogbase.ui" line="13"/>
        <source>Dialog</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsidwinterpolatordialogbase.ui" line="19"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:12pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-weight:600;&quot;&gt;Inverse Distance Weighting&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-weight:600;&quot;&gt;&lt;span style=&quot; font-weight:400;&quot;&gt;The only parameter for the IDW interpolation method is the coefficient that describes the decrease of weights with distance.&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsidwinterpolatordialogbase.ui" line="32"/>
        <source>Distance coefficient P:</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsIdentifyResults</name>
    <message>
        <location filename="../src/app/qgsidentifyresults.cpp" line="225"/>
        <source>Identify Results - </source>
        <translation>Identificēšanas rezultāti - </translation>
    </message>
    <message>
        <location filename="../src/app/qgsidentifyresults.cpp" line="44"/>
        <source>Feature</source>
        <translation>Objekts</translation>
    </message>
    <message>
        <location filename="../src/app/qgsidentifyresults.cpp" line="45"/>
        <source>Value</source>
        <translation>Vērtība</translation>
    </message>
    <message>
        <location filename="../src/app/qgsidentifyresults.cpp" line="106"/>
        <source>Run action</source>
        <translation>Startēt darbību</translation>
    </message>
    <message>
        <location filename="../src/app/qgsidentifyresults.cpp" line="196"/>
        <source>(Derived)</source>
        <translation>(Atvasināta)</translation>
    </message>
</context>
<context>
    <name>QgsIdentifyResultsBase</name>
    <message>
        <location filename="../src/ui/qgsidentifyresultsbase.ui" line="13"/>
        <source>Identify Results</source>
        <translation>Identificēt rezultātus</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsidentifyresultsbase.ui" line="43"/>
        <source>Help</source>
        <translation>Palīdzība</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsidentifyresultsbase.ui" line="46"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsidentifyresultsbase.ui" line="72"/>
        <source>Close</source>
        <translation>Aizvērt</translation>
    </message>
</context>
<context>
    <name>QgsInterpolationDialog</name>
    <message>
        <location filename="../src/plugins/interpolation/qgsinterpolationdialog.cpp" line="206"/>
        <source>Inverse Distance Weighting (IDW)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsinterpolationdialog.cpp" line="210"/>
        <source>Triangular interpolation (TIN)</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsInterpolationDialogBase</name>
    <message>
        <location filename="../src/plugins/interpolation/qgsinterpolationdialogbase.ui" line="19"/>
        <source>Dialog</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsinterpolationdialogbase.ui" line="31"/>
        <source>Input</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsinterpolationdialogbase.ui" line="39"/>
        <source>Input vector layer:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsinterpolationdialogbase.ui" line="51"/>
        <source>Use z-Coordinate for interpolation</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsinterpolationdialogbase.ui" line="60"/>
        <source>Interpolation attribute: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsinterpolationdialogbase.ui" line="81"/>
        <source>Output</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsinterpolationdialogbase.ui" line="89"/>
        <source>Interpolation method:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsinterpolationdialogbase.ui" line="101"/>
        <source>Configure interpolation method...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsinterpolationdialogbase.ui" line="123"/>
        <source>Number of columns:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsinterpolationdialogbase.ui" line="141"/>
        <source>Number of rows:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsinterpolationdialogbase.ui" line="159"/>
        <source>Output File: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsinterpolationdialogbase.ui" line="169"/>
        <source>...</source>
        <translation type="unfinished">...</translation>
    </message>
</context>
<context>
    <name>QgsInterpolationPlugin</name>
    <message>
        <location filename="../src/plugins/interpolation/qgsinterpolationplugin.cpp" line="45"/>
        <source>&amp;Interpolation</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsLUDialogBase</name>
    <message>
        <location filename="../src/ui/qgsludialogbase.ui" line="13"/>
        <source>Enter class bounds</source>
        <translation>Ievadiet klases robežas</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsludialogbase.ui" line="40"/>
        <source>Lower value</source>
        <translation>Mazākā vērtība</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsludialogbase.ui" line="79"/>
        <source>-</source>
        <translation>- </translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>OK</source>
        <translation type="obsolete">Labi</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Cancel</source>
        <translation type="obsolete">Atcelt</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsludialogbase.ui" line="66"/>
        <source>Upper value</source>
        <translation>Lielākā vērtība</translation>
    </message>
</context>
<context>
    <name>QgsLabelDialogBase</name>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="19"/>
        <source>Form1</source>
        <translation>Forma1</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Field containing label:</source>
        <translation type="obsolete">Birkas lauks:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Default label:</source>
        <translation type="obsolete">Noklusējuma birka:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="890"/>
        <source>Preview:</source>
        <translation>Priekšapskate:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="908"/>
        <source>QGIS Rocks!</source>
        <translation>QGIS ruļevoj!</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Font Style</source>
        <translation type="obsolete">Fontu stils</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="153"/>
        <source>Font</source>
        <translation>Fonts</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="505"/>
        <source>Points</source>
        <translation>Punktos</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="512"/>
        <source>Map units</source>
        <translation>Kartes vienībās</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="411"/>
        <source>%</source>
        <translation>%</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="776"/>
        <source>Transparency:</source>
        <translation>Caurspīdīgums:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Colour</source>
        <translation type="obsolete">Krāsa</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="104"/>
        <source>Position</source>
        <translation>Novietojums</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>X Offset (pts):</source>
        <translation type="obsolete">X nobīde (punktos):</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Y Offset (pts):</source>
        <translation type="obsolete">Y nobīde (punktos):</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Buffer Labels?</source>
        <translation type="obsolete">Veidot buferi ap birku?</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="799"/>
        <source>Size:</source>
        <translation>Izmērs:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="444"/>
        <source>Size is in map units</source>
        <translation>kartes vienībās</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="437"/>
        <source>Size is in points</source>
        <translation>punktos</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="285"/>
        <source>Above</source>
        <translation>Virs</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="275"/>
        <source>Over</source>
        <translation>Pāri</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="292"/>
        <source>Left</source>
        <translation>Pa kreisi</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="268"/>
        <source>Below</source>
        <translation>Zem</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="261"/>
        <source>Right</source>
        <translation>Pa labi</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="306"/>
        <source>Above Right</source>
        <translation>Virs pa labi</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="254"/>
        <source>Below Right</source>
        <translation>Zem pa labi</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="313"/>
        <source>Above Left</source>
        <translation>Virs pa kreisi</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="299"/>
        <source>Below Left</source>
        <translation>Zem pa kreisi</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Angle (deg):</source>
        <translation type="obsolete">Leņķis (grādi):</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Data Defined Style</source>
        <translation type="obsolete">Datu definēts stils</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&amp;Font family:</source>
        <translation type="obsolete">&amp;Fontu saime:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&amp;Italic:</source>
        <translation type="obsolete">&amp;Kursīvs:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&amp;Underline:</source>
        <translation type="obsolete">&amp;Pasvītrots:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&amp;Bold:</source>
        <translation type="obsolete">&amp;Treknraksts:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&amp;Size:</source>
        <translation type="obsolete">&amp;Izmērs:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>X Coordinate:</source>
        <translation type="obsolete">X koordināta:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Y Coordinate:</source>
        <translation type="obsolete">Y koordināta:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Placement:</source>
        <translation type="obsolete">Novietojums:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&#xb0;</source>
        <translation type="obsolete">°</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="323"/>
        <source>Font size units</source>
        <translation>Fonta izmēra vienības</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Font Alignment</source>
        <translation type="obsolete">Fonta novietojums</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="731"/>
        <source>Placement</source>
        <translation>Novietojums</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="99"/>
        <source>Buffer</source>
        <translation>Buferis</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="431"/>
        <source>Buffer size units</source>
        <translation>Bufera izmēra vienības</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="499"/>
        <source>Offset units</source>
        <translation>Nobīdes vienības</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Data Defined Alignment</source>
        <translation type="obsolete">Datu definēts novietojums</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Data Defined Buffer</source>
        <translation type="obsolete">Datu definēts buferis</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Data Defined Position</source>
        <translation type="obsolete">Datu definēta vieta</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Source</source>
        <translation type="obsolete">Avots</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Size Units:</source>
        <translation type="obsolete">Izmēra vienības:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="39"/>
        <source>Field containing label</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="62"/>
        <source>Default label</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="109"/>
        <source>Data defined style</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="114"/>
        <source>Data defined alignment</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="119"/>
        <source>Data defined buffer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="124"/>
        <source>Data defined position</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="163"/>
        <source>Font transparency</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="424"/>
        <source>Color</source>
        <translation type="unfinished">Krāsa</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="747"/>
        <source>Angle (deg)</source>
        <translation type="unfinished"></translation>
    </message>
    <message encoding="UTF-8">
        <location filename="../src/ui/qgslabeldialogbase.ui" line="221"/>
        <source>°</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="363"/>
        <source>Buffer labels?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="373"/>
        <source>Buffer size</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="701"/>
        <source>Transparency</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="854"/>
        <source>X Offset (pts)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="870"/>
        <source>Y Offset (pts)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="545"/>
        <source>&amp;Font family</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="571"/>
        <source>&amp;Bold</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="597"/>
        <source>&amp;Italic</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="623"/>
        <source>&amp;Underline</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="649"/>
        <source>&amp;Size</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="675"/>
        <source>Size units</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="822"/>
        <source>X Coordinate</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="838"/>
        <source>Y Coordinate</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsLayerProjectionSelector</name>
    <message>
        <location filename="../src/gui/qgslayerprojectionselector.cpp" line="34"/>
        <source>Define this layer&apos;s projection:</source>
        <translation>Definēt slāņa projekciju:</translation>
    </message>
    <message>
        <location filename="../src/gui/qgslayerprojectionselector.cpp" line="35"/>
        <source>This layer appears to have no projection specification.</source>
        <translation>Izskatās, ka šim slānim nav definēta koordinātu sistēma.</translation>
    </message>
    <message>
        <location filename="../src/gui/qgslayerprojectionselector.cpp" line="37"/>
        <source>By default, this layer will now have its projection set to that of the project, but you may override this by selecting a different projection below.</source>
        <translation>Pēs noklusējuma šim slānim tiek lietota projekta koordinātu sistēma, taču jūs to varat mainīt šeit definējot slāņa koordinātu sistēmu.</translation>
    </message>
</context>
<context>
    <name>QgsLayerProjectionSelectorBase</name>
    <message>
        <location filename="../src/ui/qgslayerprojectionselectorbase.ui" line="13"/>
        <source>Layer Projection Selector</source>
        <translation>Slāņa projekcijas izvēle</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslayerprojectionselectorbase.ui" line="80"/>
        <source>OK</source>
        <translation>Labi</translation>
    </message>
</context>
<context>
    <name>QgsLegend</name>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="113"/>
        <source>group</source>
        <translation>grupēt</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="426"/>
        <source>&amp;Remove</source>
        <translation>&amp;Noņemt</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="419"/>
        <source>&amp;Make to toplevel item</source>
        <translation>&amp;Pārcelt uz augšu</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="431"/>
        <source>Re&amp;name</source>
        <translation>Pā&amp;rsaukt</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="436"/>
        <source>&amp;Add group</source>
        <translation>&amp;Pievienot grupu</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="437"/>
        <source>&amp;Expand all</source>
        <translation>&amp;Izplest visu</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="438"/>
        <source>&amp;Collapse all</source>
        <translation>&amp;Sakļaut visu</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="440"/>
        <source>Show file groups</source>
        <translation>Rādīt failu grupas</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="1827"/>
        <source>No Layer Selected</source>
        <translation>Nav izvēlēts slānis</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="1828"/>
        <source>To open an attribute table, you must select a vector layer in the legend</source>
        <translation>Lai atvērtu atribūtu tabulu, jums vispirms no leģendas ir jāizvēlas vektordatu slānis</translation>
    </message>
</context>
<context>
    <name>QgsLegendLayer</name>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="490"/>
        <source>&amp;Zoom to layer extent</source>
        <translation>&amp;Tuvināt līdz slāņa kopskatam</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="493"/>
        <source>&amp;Zoom to best scale (100%)</source>
        <translation>&amp;Tuvināt līdz labākajam skatam (100%)</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="497"/>
        <source>&amp;Show in overview</source>
        <translation>&amp;Rādīt pārskatā</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="503"/>
        <source>&amp;Remove</source>
        <translation>&amp;Noņemt</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="510"/>
        <source>&amp;Open attribute table</source>
        <translation>&amp;Atvērt atribūtu tabulu</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="534"/>
        <source>Save as shapefile...</source>
        <translation>Saglabāt kā shapefile...</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="541"/>
        <source>Save selection as shapefile...</source>
        <translation>Saglabāt izvēli kā shapefile...</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="551"/>
        <source>&amp;Properties</source>
        <translation>Ī&amp;pašības</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="600"/>
        <source>More layers</source>
        <translation>Vairāk slāņu</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="601"/>
        <source>This item contains more layer files. Displaying more layers in table is not supported.</source>
        <translation>Šim vienumam ir vairāki slāņi. Vairāku slāņu attēlošanā tabulā nav atbalstīta.</translation>
    </message>
</context>
<context>
    <name>QgsLegendLayerFile</name>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="276"/>
        <source>Attribute table - </source>
        <translation>Atribūtu tabula - </translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="346"/>
        <source>Save layer as...</source>
        <translation>Saglabāt slāni kā...</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="426"/>
        <source>Start editing failed</source>
        <translation>Rediģēšanas sākšana neizdevās</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="427"/>
        <source>Provider cannot be opened for editing</source>
        <translation>Sniedzējs nevar tikt atvērts rediģēšanai</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="440"/>
        <source>Stop editing</source>
        <translation>Pārtraukt rediģēšanu</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="440"/>
        <source>Do you want to save the changes?</source>
        <translation>Vai Jūs velaties saglabāt izmaiņas?</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="457"/>
        <source>Error</source>
        <translation>Kļūda</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="446"/>
        <source>Could not commit changes</source>
        <translation>Nevar apstiprināt izmaiņas</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="458"/>
        <source>Problems during roll back</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="227"/>
        <source>Not a vector layer</source>
        <translation>Nav vektordatu slānis</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="228"/>
        <source>To open an attribute table, you must select a vector layer in the legend</source>
        <translation>Lai atvērtu atribūtu tabulu, jums vispirms no leģendas ir jāizvēlas vektordatu slānis</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="393"/>
        <source>Saving done</source>
        <translation>Saglabāšana ir pabeigta</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="393"/>
        <source>Export to Shapefile has been completed</source>
        <translation>Eskportēšana uz Shapefile ir pabeigta</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="397"/>
        <source>Driver not found</source>
        <translation>Draiveris nav atrasts</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="397"/>
        <source>ESRI Shapefile driver is not available</source>
        <translation>ESRI Shapefile draiveris nav pieejams</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="401"/>
        <source>Error creating shapefile</source>
        <translation>Kļūda veidojot shapefile</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="402"/>
        <source>The shapefile could not be created (</source>
        <translation>Nebija iespējams izveidot shapefile (</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="406"/>
        <source>Layer creation failed</source>
        <translation>Slāņa izveide bija neveiksmīga</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="501"/>
        <source>&amp;Zoom to layer extent</source>
        <translation>&amp;Tuvināt līdz slāņa kopskatam</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="504"/>
        <source>&amp;Show in overview</source>
        <translation>&amp;Rādīt pārskatā</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="512"/>
        <source>&amp;Remove</source>
        <translation>&amp;Noņemt</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="521"/>
        <source>&amp;Open attribute table</source>
        <translation>&amp;Atvērt atribūtu tabulu</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="535"/>
        <source>Save as shapefile...</source>
        <translation>Saglabāt kā shapefile...</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="537"/>
        <source>Save selection as shapefile...</source>
        <translation>Saglabāt izvēli kā shapefile...</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="554"/>
        <source>&amp;Properties</source>
        <translation>Ī&amp;pašības</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="271"/>
        <source>bad_alloc exception</source>
        <translation>Bad_alloc kļūda</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="271"/>
        <source>Filling the attribute table has been stopped because there was no more virtual memory left</source>
        <translation>Atribūtu tabulas aizpildīšana tika pārtraukta, jo nepietiek virtuālās atmiņas </translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="410"/>
        <source>Layer attribute table contains unsupported datatype(s)</source>
        <translation>Slāņa atribūtu tabula satur neatbalstītus datu tipus.</translation>
    </message>
</context>
<context>
    <name>QgsLineStyleDialogBase</name>
    <message>
        <location filename="" line="135533324"/>
        <source>Select a line style</source>
        <translation type="obsolete">Izvēlieties līnijas stilu</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Styles</source>
        <translation type="obsolete">Stili</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Ok</source>
        <translation type="obsolete">Labi</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Cancel</source>
        <translation type="obsolete">Atcelt</translation>
    </message>
</context>
<context>
    <name>QgsLineStyleWidgetBase</name>
    <message>
        <location filename="" line="135533324"/>
        <source>Form2</source>
        <translation type="obsolete">Forma2</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Outline Style</source>
        <translation type="obsolete">Rāmja līnijas stils</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Width:</source>
        <translation type="obsolete">Platums:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Colour:</source>
        <translation type="obsolete">Krāsa:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>LineStyleWidget</source>
        <translation type="obsolete">LīnijasStilaLogdaļa</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>col</source>
        <translation type="obsolete">kol</translation>
    </message>
</context>
<context>
    <name>QgsMapCanvas</name>
    <message>
        <location filename="../src/gui/qgsmapcanvas.cpp" line="1224"/>
        <source>Could not draw</source>
        <translation>Nav iespējams zīmēt</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsmapcanvas.cpp" line="1224"/>
        <source>because</source>
        <translation>jo</translation>
    </message>
</context>
<context>
    <name>QgsMapLayer</name>
    <message>
        <location filename="" line="135533324"/>
        <source> Check file permissions and retry.</source>
        <translation type="obsolete"> Pārbaudiet faila pieejas atļaujas un mēģiniet vēlreiz.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="468"/>
        <source>%1 at line %2 column %3</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="453"/>
        <source>could not open user database</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="474"/>
        <source>style %1 not found in database</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="585"/>
        <source>User database could not be opened.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="599"/>
        <source>The style table could not be created.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="614"/>
        <source>The style %1 was saved to database</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="631"/>
        <source>The style %1 was updated in the database.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="636"/>
        <source>The style %1 could not be updated in the database.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="643"/>
        <source>The style %1 could not be inserted into database.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsMapToolIdentify</name>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="565"/>
        <source>No features found</source>
        <translation>Neviens objekts nav atrasts</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="568"/>
        <source>&lt;p&gt;No features were found within the search radius. Note that it is currently not possible to use the identify tool on unsaved features.&lt;/p&gt;</source>
        <translation>&lt;p&gt;Meklēšanas radiusā neviens objekts netika atrasts. Piezīme. Pagaidām nav iespējams lietot identificešanas rīku nesaglabātiem objektiem.&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>- %1 features found</source>
        <comment>
Identify results window title</comment>
        <translation type="obsolete">- atrasti %1 objekti</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="424"/>
        <source>(clicked coordinate)</source>
        <translation>(klikšķa koordinātas)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="223"/>
        <source>WMS identify result for %1
%2</source>
        <translation>WMS identificēšanas rezultāti priekš %1 
%2</translation>
    </message>
    <message numerus="yes">
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="491"/>
        <source>- %1 features found</source>
        <comment>Identify results window title</comment>
        <translation type="unfinished">
            <numerusform>- atrasti %1 objekti</numerusform>
        </translation>
    </message>
</context>
<context>
    <name>QgsMapToolSplitFeatures</name>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="86"/>
        <source>Split error</source>
        <translation>Sadalīšanas kļūda</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="86"/>
        <source>An error occured during feature splitting</source>
        <translation>Dalot objektus notika kļūda</translation>
    </message>
</context>
<context>
    <name>QgsMapToolVertexEdit</name>
    <message>
        <location filename="../src/app/qgsmaptoolvertexedit.cpp" line="51"/>
        <source>Snap tolerance</source>
        <translation>Pielipšanas tolerance</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolvertexedit.cpp" line="52"/>
        <source>Don&apos;t show this message again</source>
        <translation>Turpmāk nerādīt šo paziņojumu</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolvertexedit.cpp" line="57"/>
        <source>Could not snap segment.</source>
        <translation>Nevar pielipināt segmentu.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolvertexedit.cpp" line="60"/>
        <source>Have you set the tolerance in Settings &gt; Project Properties &gt; General?</source>
        <translation>Vai esat iestatījuši pielipšanas toleranci iekš Iestatījumi &gt; Projekta īpašības &gt; Vispārīgi?</translation>
    </message>
</context>
<context>
    <name>QgsMapserverExport</name>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexport.cpp" line="76"/>
        <source>Name for the map file</source>
        <translation>Jaunā MAP faila nosaukums</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexport.cpp" line="84"/>
        <source>Choose the QGIS project file</source>
        <translation>Izvēlieties QGIS projekta failu</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexport.cpp" line="85"/>
        <source>QGIS Project Files (*.qgs);;All files (*.*)</source>
        <comment>Filter list for selecting files from a dialog box</comment>
        <translation>QGIS Projekta faili (*.qgs);;Visi faili (*.*)</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexport.cpp" line="197"/>
        <source>Overwrite File?</source>
        <translation>Pārrakstīt failu?</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexport.cpp" line="199"/>
        <source> exists. 
Do you want to overwrite it?</source>
        <comment>a filename is prepended to this text, and appears in a dialog box</comment>
        <translation> eksistē. 
Vai vēlaties to pārrakstīt?</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmapserverexport.cpp" line="74"/>
        <source> exists. 
Do you want to overwrite it?</source>
        <translation> eksistē. 
Vai vēlaties to pārrakstīt?</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexport.cpp" line="77"/>
        <source>MapServer map files (*.map);;All files (*.*)</source>
        <comment>Filter list for selecting files from a dialog box</comment>
        <translation>MapServer map faili (*.map);;Visi faili (*.*)</translation>
    </message>
</context>
<context>
    <name>QgsMapserverExportBase</name>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="14"/>
        <source>Export to Mapserver</source>
        <translation>Eksportēt uz Mapserver</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="352"/>
        <source>Map file</source>
        <translation>Map fails</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="403"/>
        <source>Export LAYER information only</source>
        <translation>Eksportēt tikai slāņa informāciju</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="159"/>
        <source>Map</source>
        <translation>Karte</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="330"/>
        <source>Name</source>
        <translation>Nosaukums</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="304"/>
        <source>Height</source>
        <translation>Augstums</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="291"/>
        <source>Width</source>
        <translation>Platums</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="196"/>
        <source>dd</source>
        <translation>dd</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="201"/>
        <source>feet</source>
        <translation>pēdas</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="206"/>
        <source>meters</source>
        <translation>metri</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="211"/>
        <source>miles</source>
        <translation>jūdzes</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="216"/>
        <source>inches</source>
        <translation>collas</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="221"/>
        <source>kilometers</source>
        <translation>kilometri</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="182"/>
        <source>Units</source>
        <translation>Vienības</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="229"/>
        <source>Image type</source>
        <translation>Attēla tips</translation>
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
        <translation>lietotāja definēts</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="273"/>
        <source>wbmp</source>
        <translation>wbmp</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmapserverexportbase.ui" line="226"/>
        <source>MinScale</source>
        <translation>Min mērogs</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmapserverexportbase.ui" line="236"/>
        <source>MaxScale</source>
        <translation>Maks mērogs</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmapserverexportbase.ui" line="252"/>
        <source>Prefix attached to map, scalebar and legend GIF filenames created using this MapFile. It should be kept short.</source>
        <translation>Priedēklis, kas tiks pievienots kartei, mērogjoslai un leģendas GIF failam. Tam vajadzētu būt īsam.</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="29"/>
        <source>Web Interface Definition</source>
        <translation>Tīmekļa saskarnes definīcija</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="98"/>
        <source>Header</source>
        <translation>Galvene</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="134"/>
        <source>Footer</source>
        <translation>Kājene</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="58"/>
        <source>Template</source>
        <translation>Sagatave</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmapserverexportbase.ui" line="56"/>
        <source>&amp;Help</source>
        <translation>&amp;Palīdzība</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmapserverexportbase.ui" line="59"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmapserverexportbase.ui" line="85"/>
        <source>&amp;OK</source>
        <translation>&amp;Labi</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmapserverexportbase.ui" line="101"/>
        <source>&amp;Cancel</source>
        <translation>&amp;Atcelt</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmapserverexportbase.ui" line="116"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="362"/>
        <source>Name for the map file to be created from the QGIS project file</source>
        <translation>No QGIS faila veidojamā Map faila nosaukums</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="400"/>
        <source>If checked, only the layer information will be processed</source>
        <translation>Ja atzīmēts, tikai slāņu informācija tiks apstrādāta</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="68"/>
        <source>Path to the MapServer template file</source>
        <translation>Ceļš līdz MapServer šablona failam</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="340"/>
        <source>Prefix attached to map, scalebar and legend GIF filenames created using this MapFile</source>
        <translation>Kartes, leģendas un mērogjoslas failiem pievienotais prefikss, kas tiks izmantots šim Map failam</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="376"/>
        <source>Full path to the QGIS project file to export to MapServer map format</source>
        <translation>Plins ceļš līdz QGIS projeta failam, kurš tiks eksportēts uz MapServer Map failu</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="383"/>
        <source>QGIS project file</source>
        <translation>QGIS projekta fails</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="369"/>
        <source>Browse...</source>
        <translation>Pārlūkot...</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="393"/>
        <source>Save As...</source>
        <translation>Saglabāt kā...</translation>
    </message>
</context>
<context>
    <name>QgsMarkerDialogBase</name>
    <message>
        <location filename="" line="135533324"/>
        <source>Choose a marker symbol</source>
        <translation type="obsolete">Izvēlieties marķiera simbolu</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Directory</source>
        <translation type="obsolete">Mape</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>...</source>
        <translation type="obsolete">...</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Ok</source>
        <translation type="obsolete">Labi</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Cancel</source>
        <translation type="obsolete">Atcelt</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>New Item</source>
        <translation type="obsolete">Jauns vienums</translation>
    </message>
</context>
<context>
    <name>QgsMeasureBase</name>
    <message>
        <location filename="../src/ui/qgsmeasurebase.ui" line="19"/>
        <source>Measure</source>
        <translation>Mērīt</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmeasurebase.ui" line="102"/>
        <source>Help</source>
        <translation>Palīdzība</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmeasurebase.ui" line="125"/>
        <source>New</source>
        <translation>Jauns</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmeasurebase.ui" line="132"/>
        <source>Cl&amp;ose</source>
        <translation>&amp;Aizvērt</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmeasurebase.ui" line="66"/>
        <source>Total:</source>
        <translation>Kopā:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmeasurebase.ui" line="86"/>
        <source>Segments</source>
        <translation type="unfinished">Segmenti</translation>
    </message>
</context>
<context>
    <name>QgsMeasureDialog</name>
    <message>
        <location filename="../src/app/qgsmeasuredialog.cpp" line="198"/>
        <source>Segments (in meters)</source>
        <translation>Segmenti (metros)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmeasuredialog.cpp" line="201"/>
        <source>Segments (in feet)</source>
        <translation>Segmenti (pēdās)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmeasuredialog.cpp" line="204"/>
        <source>Segments (in degrees)</source>
        <translation>Segmenti (grādos)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmeasuredialog.cpp" line="207"/>
        <source>Segments</source>
        <translation>Segmenti</translation>
    </message>
</context>
<context>
    <name>QgsMeasureTool</name>
    <message>
        <location filename="../src/app/qgsmeasuretool.cpp" line="74"/>
        <source>Incorrect measure results</source>
        <translation>Nekorekti mērīšanas rezultāti</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmeasuretool.cpp" line="82"/>
        <source>&lt;p&gt;This map is defined with a geographic coordinate system (latitude/longitude) but the map extents suggests that it is actually a projected coordinate system (e.g., Mercator). If so, the results from line or area measurements will be incorrect.&lt;/p&gt;&lt;p&gt;To fix this, explicitly set an appropriate map coordinate system using the &lt;tt&gt;Settings:Project Properties&lt;/tt&gt; menu.</source>
        <translation>&lt;p&gt;Šī karte ir definēta ģeogrāfiskā koordinātu sistēmā (garums/platums), bet, spriežot pēc kartes apjoma, karte ir projecētā koordinātu sistēmā (piem. Merkatora). Ja tas ir tā, tad attāluma un platības mērījumi būs nepareizi.&lt;/p&gt;&lt;p&gt;Lai šo izlabotu, iestatiet pareizu projekta koordinātu sistēmu iekš &lt;tt&gt;Iestatījumi &gt; Projekta īpašības&lt;/tt&gt;.</translation>
    </message>
</context>
<context>
    <name>QgsMessageViewer</name>
    <message>
        <location filename="../src/ui/qgsmessageviewer.ui" line="13"/>
        <source>QGIS Message</source>
        <translation>QGIS paziņojums</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmessageviewer.ui" line="48"/>
        <source>Close</source>
        <translation>Aizvērt</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmessageviewer.ui" line="28"/>
        <source>Don&apos;t show this message again</source>
        <translation>Turpmāk nerādīt šo paziņojumu</translation>
    </message>
</context>
<context>
    <name>QgsMySQLProvider</name>
    <message>
        <location filename="" line="135533324"/>
        <source>Unable to access relation</source>
        <translation type="obsolete">Nevaru piekļūt relācijai</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Unable to access the </source>
        <translation type="obsolete">Nevaru piekļūt </translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source> relation.
The error message from the database was:
</source>
        <translation type="obsolete"> relācijai. 
Datubāzes ziņotā kļūda bija: </translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>No GEOS Support!</source>
        <translation type="obsolete">Nav GEOS atbalsta!</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Your PostGIS installation has no GEOS support.
Feature selection and identification will not work properly.
Please install PostGIS with GEOS support (http://geos.refractions.net)</source>
        <translation type="obsolete">Jūsu PostGIS instalācijai nav GEOS atbalsta.
Objektu izvēle un identificēšana var nedarboties pareizi.
Lūdzu instalējiet PostGIS ar GEOS atbalstu (http://geos.refractions.net)</translation>
    </message>
</context>
<context>
    <name>QgsNewConnection</name>
    <message>
        <location filename="../src/app/qgsnewconnection.cpp" line="121"/>
        <source>Test connection</source>
        <translation>Testēt savienojumu</translation>
    </message>
    <message>
        <location filename="../src/app/qgsnewconnection.cpp" line="121"/>
        <source>Connection failed - Check settings and try again.

Extended error information:
</source>
        <translation>Savienojums neizdevās - pārbaudiet parametrus un mēģiniet vēlreiz.


Paplašināta kļūdas informācija: 
</translation>
    </message>
    <message>
        <location filename="../src/app/qgsnewconnection.cpp" line="118"/>
        <source>Connection to %1 was successful</source>
        <translation>Savienojums ar %1 bija sekmīgs</translation>
    </message>
</context>
<context>
    <name>QgsNewConnectionBase</name>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="21"/>
        <source>Create a New PostGIS connection</source>
        <translation>Izveidot jaunu PostGIS savienojumu</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="252"/>
        <source>OK</source>
        <translation>Labi</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="268"/>
        <source>Cancel</source>
        <translation>Atcelt</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="284"/>
        <source>Help</source>
        <translation>Palīdzība</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="287"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="39"/>
        <source>Connection Information</source>
        <translation>Savienojuma informācija</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="137"/>
        <source>Name</source>
        <translation>Nosaukums</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="147"/>
        <source>Host</source>
        <translation>Hosts</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="157"/>
        <source>Database</source>
        <translation>Datu bāze</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="167"/>
        <source>Port</source>
        <translation>Ports</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="177"/>
        <source>Username</source>
        <translation>Lietotājvārds</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="187"/>
        <source>Password</source>
        <translation>Parole</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="207"/>
        <source>Name of the new connection</source>
        <translation>Jaunā savienojuma nosaukums</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="220"/>
        <source>5432</source>
        <translation>5432</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="105"/>
        <source>Save Password</source>
        <translation>Saglabāt paroli</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="112"/>
        <source>Test Connect</source>
        <translation>Testēt savienojumu</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="67"/>
        <source>Only look in the &apos;public&apos; schema</source>
        <translation>Meklēt tikai &apos;public&apos; shēmā</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="90"/>
        <source>Only look in the geometry_columns table</source>
        <translation>Meklēt tikai geometry_columns tabulā</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="61"/>
        <source>Restrict the search to the public schema for spatial tables not in the geometry_columns table</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="64"/>
        <source>When searching for spatial tables that are not in the geometry_columns tables, restrict the search to tables that are in the public schema (for some databases this can save lots of time)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="84"/>
        <source>Restrict the displayed tables to those that are in the geometry_columns table</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="87"/>
        <source>Restricts the displayed tables to those that are in the geometry_columns table. This can speed up the initial display of spatial tables.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsNewHttpConnectionBase</name>
    <message>
        <location filename="" line="135533324"/>
        <source>Create a New WMS connection</source>
        <translation type="obsolete">Izveidot jaunu WMS savienojumu</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Connection Information</source>
        <translation type="obsolete">Savienojuma informācija</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="31"/>
        <source>Name</source>
        <translation>Nosaukums</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="60"/>
        <source>URL</source>
        <translation>URL</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Proxy Host</source>
        <translation type="obsolete">Starpniekserveris</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Proxy Port</source>
        <translation type="obsolete">Starpniekservera ports</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Proxy User</source>
        <translation type="obsolete">Starpniekservera lietotājvārds</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Proxy Password</source>
        <translation type="obsolete">Starpniekservera parole</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Your user name for the HTTP proxy (optional)</source>
        <translation type="obsolete">Jūsu lietotājvārds HTTP starpniekdatoram (neobligāts)</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Password for your HTTP proxy (optional)</source>
        <translation type="obsolete">Parole jūsu HTTP starpniekserverim (neobligāta)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="50"/>
        <source>Name of the new connection</source>
        <translation>Jaunā savienojuma nosaukums</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="73"/>
        <source>HTTP address of the Web Map Server</source>
        <translation>WMS servera HTTP adrese</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Name of your HTTP proxy (optional)</source>
        <translation type="obsolete">Jūsu HTTP starpniekservera nosaukums (neobligāts)</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Port number of your HTTP proxy (optional)</source>
        <translation type="obsolete">Jūsu HTTP starpniekservera porta numurs (neobligāts)</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>OK</source>
        <translation type="obsolete">Labi</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Cancel</source>
        <translation type="obsolete">Atcelt</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Help</source>
        <translation type="obsolete">Palīdzība</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>F1</source>
        <translation type="obsolete">F1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="13"/>
        <source>Create a new WMS connection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="25"/>
        <source>Connection details</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsNorthArrowPlugin</name>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="82"/>
        <source>Bottom Left</source>
        <translation>Apakšējais kreisais</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="83"/>
        <source>Top Right</source>
        <translation>Augšējais labais</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="83"/>
        <source>Bottom Right</source>
        <translation>Apakšējais labais</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="255"/>
        <source>&amp;Decorations</source>
        <translation>&amp;Dekorācijas</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="97"/>
        <source>Creates a north arrow that is displayed on the map canvas</source>
        <translation>Izveido ziemeļu virziena bultu rādīšanai uz kartes</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="83"/>
        <source>Top Left</source>
        <translation>Augšējais kreisais</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="96"/>
        <source>&amp;North Arrow</source>
        <translation>&amp;Ziemeļu bulta</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="246"/>
        <source>North arrow pixmap not found</source>
        <translation>Ziemeļu bultas pikseļkarte nav atrasta</translation>
    </message>
</context>
<context>
    <name>QgsNorthArrowPluginGui</name>
    <message>
        <location filename="../src/plugins/north_arrow/plugingui.cpp" line="157"/>
        <source>Pixmap not found</source>
        <translation>Pikseļkarte nav atrasta</translation>
    </message>
</context>
<context>
    <name>QgsNorthArrowPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="235"/>
        <source>North Arrow Plugin</source>
        <translation>Ziemeļu bultas spraudnis</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="35"/>
        <source>Properties</source>
        <translation>Īpašības</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="55"/>
        <source>Angle</source>
        <translation>Leņķis</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="62"/>
        <source>Placement</source>
        <translation>Novietojums</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="75"/>
        <source>Set direction automatically</source>
        <translation>Iestatīt virzienu automātiski</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="85"/>
        <source>Enable North Arrow</source>
        <translation>Ieslēgt ziemeļu bultu</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="141"/>
        <source>Top Left</source>
        <translation>Augšējais kreisais</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="146"/>
        <source>Top Right</source>
        <translation>Augšējais labais</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="151"/>
        <source>Bottom Left</source>
        <translation>Apakšējais kreisais</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="156"/>
        <source>Bottom Right</source>
        <translation>Apakšējais labais</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="137"/>
        <source>Placement on screen</source>
        <translation>Novietojums uz ekrāna</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="164"/>
        <source>Preview of north arrow</source>
        <translation>Ziemeļu bultas priekšapskate</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="183"/>
        <source>Icon</source>
        <translation>Ikona</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>New Item</source>
        <translation type="obsolete">Jauns vienums</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="198"/>
        <source>Browse...</source>
        <translation>Pārlūkot...</translation>
    </message>
</context>
<context>
    <name>QgsOptions</name>
    <message>
        <location filename="../src/app/qgsoptions.cpp" line="154"/>
        <source>Detected active locale on your system: </source>
        <translation>Jūsu sistēmas detektētā lokāle: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsoptions.cpp" line="332"/>
        <source>to vertex</source>
        <translation>uz virsotni</translation>
    </message>
    <message>
        <location filename="../src/app/qgsoptions.cpp" line="336"/>
        <source>to segment</source>
        <translation>uz segmentu</translation>
    </message>
    <message>
        <location filename="../src/app/qgsoptions.cpp" line="340"/>
        <source>to vertex and segment</source>
        <translation>uz virsotni un segmentu</translation>
    </message>
    <message>
        <location filename="../src/app/qgsoptions.cpp" line="349"/>
        <source>Semi transparent circle</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsoptions.cpp" line="353"/>
        <source>Cross</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsOptionsBase</name>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="13"/>
        <source>QGIS Options</source>
        <translation>QGIS īpašības</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&amp;Appearance</source>
        <translation type="obsolete">&amp;Izskats</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="212"/>
        <source>Hide splash screen at startup</source>
        <translation>Paslēpt uzplaiksnījumekrānu pie startēšanas</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&amp;Icon Theme</source>
        <translation type="obsolete">&amp;Ikonu tēma</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Theme</source>
        <translation type="obsolete">Tēma</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="188"/>
        <source>&lt;b&gt;Note: &lt;/b&gt;Theme changes take effect the next time QGIS is started</source>
        <translation>&lt;b&gt; Piezīme: &lt;/b&gt; Tēmas izmaiņas stājas spēkā tikai pēc QGIS pārstartēšanas</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="236"/>
        <source>&amp;Rendering</source>
        <translation>&amp;Renderēšana</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Update display after reading</source>
        <translation type="obsolete">Atjaunot ekrānu pēc </translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="265"/>
        <source>Map display will be updated (drawn) after this many features have been read from the data source</source>
        <translation>Karte tiks rādīta tikai pēctam, kad būs nolasīti </translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>features</source>
        <translation type="obsolete">objektu nolasīšanas</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>(Set to 0 to not update the display until all features have been read)</source>
        <translation type="obsolete">(Iestatiet uz 0, lai neajaunotu ekrānu līdz visi objektu ir nolasīti)</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Initial Visibility</source>
        <translation type="obsolete">Sākotnējā redzamība</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="819"/>
        <source>Select Global Default ...</source>
        <translation>Izvēlieties globālo noklusējumu...</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Prompt for projection.</source>
        <translation type="obsolete">Jautāt projekcijas informāciju.</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Project wide default projection will be used.</source>
        <translation type="obsolete">Tiks lietota projekta noklusējuma projekcija.</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&amp;Splash screen</source>
        <translation type="obsolete">&amp;Uzplaiksnījumekrāns</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Default Map Appearance (Overridden by project properties)</source>
        <translation type="obsolete">Noklusējuma kartes izskats (Var mainīt projekta īpašībās)</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Background Color:</source>
        <translation type="obsolete">Fona krāsa:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Selection Color:</source>
        <translation type="obsolete">Atlases krāsa:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Appearance</source>
        <translation type="obsolete">Izskats</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Capitalise layer name</source>
        <translation type="obsolete">Nomainīt slāņa nosaukuma pirmo burtu pret lieto burtu</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="306"/>
        <source>Make lines appear less jagged at the expense of some drawing performance</source>
        <translation>Padarīt līnijas smukākas uz zīmēšanas ātruma rēķina</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="248"/>
        <source>By default new la&amp;yers added to the map should be displayed</source>
        <translation>Pēc &amp;noklusējuma uzreiz rādīt no jauna pievienotos slāņus</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&amp;Update during drawing</source>
        <translation type="obsolete">&amp;Atjaunināt zīmēšanas laikā</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="437"/>
        <source>Measure tool</source>
        <translation>Mērīšanas rīks</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Ellipsoid for distance calculations:</source>
        <translation type="obsolete">Attālumu mērīšanai izmantotais elipsoīds:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="507"/>
        <source>Search radius</source>
        <translation>Meklēšanas radiuss</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="788"/>
        <source>Pro&amp;jection</source>
        <translation>Pro&amp;jekcija</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="829"/>
        <source>When layer is loaded that has no projection information</source>
        <translation>Ja tiek ielādēts slānis bez projekcijas informācijas</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Global default projection displa&amp;yed below will be used.</source>
        <translation type="obsolete">&amp;Tiks lietota globālā noklusējuma projekcija, kas ir parādīta zemāk.</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Rendering</source>
        <translation type="obsolete">Renderēšana</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="316"/>
        <source>Fix problems with incorrectly filled polygons</source>
        <translation>Salabot problēmas ar nekorekti aizpildītiem poligoniem</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="347"/>
        <source>&amp;Map tools</source>
        <translation>&amp;Kartes rīki</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="545"/>
        <source>%</source>
        <translation>%</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="365"/>
        <source>Panning and zooming</source>
        <translation>Panoramēšana un tālummaiņa</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="384"/>
        <source>Zoom</source>
        <translation>Tuvināt</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="389"/>
        <source>Zoom and recenter</source>
        <translation>Tuvināt un pārcentrēt</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="399"/>
        <source>Nothing</source>
        <translation>Nedarīt neko</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Zoom factor:</source>
        <translation type="obsolete">Tuvināšanas pakāpe:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Mouse wheel action:</source>
        <translation type="obsolete">Peles ritenīša darbība:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="323"/>
        <source>Continuously redraw the map when dragging the legend/map divider</source>
        <translation>Nepārtraukti atjaunināt karti kamēr tiek pārbīdīts kartes/leģendas atdalītājs</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="32"/>
        <source>&amp;General</source>
        <translation>&amp;Vispārējs</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>General</source>
        <translation type="obsolete">Vispārējs</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Ask to save project changes when required</source>
        <translation type="obsolete">Jautāt vai saglabāt projekta īpašības, ja nepieciešams</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Rubberband color:</source>
        <translation type="obsolete">Iezīmēšanas krāsa:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="872"/>
        <source>Locale</source>
        <translation>Lokāle</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Force Override System Locale</source>
        <translation type="obsolete">Izmantot citu lokāli sistēmas lokāles vietā</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="887"/>
        <source>Locale to use instead</source>
        <translation>Izmantojāmā lokāle</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Note: Enabling / changing overide on local requires an application restart.</source>
        <translation type="obsolete">Piezīme: lokāles izmaiņas stājas spēkā pēc programmas pārstartēšanas.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="926"/>
        <source>Additional Info</source>
        <translation>Papildus informācija</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="932"/>
        <source>Detected active locale on your system:</source>
        <translation>Detektētā aktīvā sistēmas lokāle:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Warn me when opening a project file saved with an older version of QGIS</source>
        <translation type="obsolete">Brīdināt, kad atver projekta failu, kas saglabāts ar vecāku QGIS versiju</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="313"/>
        <source>Selecting this will unselect the &apos;make lines less&apos; jagged toggle</source>
        <translation>Izvēloties šo, tiks deaktivizēta \&quot;līniju nogludināšanas\&quot; funkcija</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>(Specify the search radius as a percentage of the map width)</source>
        <translation type="obsolete">(Norādiet meklēšanas rādiusu kā procentus no krates platuma)</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Search Radius for Identifying Features and displaying Map Tips</source>
        <translation type="obsolete">Meklēšanas rādiuss objektu identificēšanai un kartes padomu rādīšanai</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="578"/>
        <source>Digitizing</source>
        <translation>Digitizēšana</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="584"/>
        <source>Rubberband</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Line Width:</source>
        <translation type="obsolete">Līnijas platums:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="600"/>
        <source>Line width in pixels</source>
        <translation>Līniju platums pikseļos</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Line Colour:</source>
        <translation type="obsolete">Līnijas krāsa:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="636"/>
        <source>Snapping</source>
        <translation>Pielipšana</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Default Snapping Tolerance (in layer units):</source>
        <translation type="obsolete">Noklusējuma pielipšanas tolerance (slāņa vienībās):</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Search radius for vertex edits (in layer units):</source>
        <translation type="obsolete">Meklēšanas rādiuss virsotņu labošanai (slāņa vienībās):</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="394"/>
        <source>Zoom to mouse cursor</source>
        <translation>Tuvināt peles kursora vietā</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Default Snap Mode:</source>
        <translation type="obsolete">Noklusējuma pielipšanas režīms:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="38"/>
        <source>Project files</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="56"/>
        <source>Prompt to save project changes when required</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="63"/>
        <source>Warn when opening a project file saved with an older version of QGIS</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="73"/>
        <source>Default Map Appearance (overridden by project properties)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="79"/>
        <source>Selection color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="115"/>
        <source>Background color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="154"/>
        <source>&amp;Application</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="166"/>
        <source>Icon theme</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="198"/>
        <source>Capitalise layer names in legend</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="205"/>
        <source>Display classification attribute names in legend</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="242"/>
        <source>Rendering behavior</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="255"/>
        <source>Number of features to draw before updating the display</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="278"/>
        <source>&lt;b&gt;Note:&lt;/b&gt; Use zero to prevent display updates until all features have been rendered</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="288"/>
        <source>Rendering quality</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="407"/>
        <source>Zoom factor</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="414"/>
        <source>Mouse wheel action</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="484"/>
        <source>Rubberband color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="494"/>
        <source>Ellipsoid for distance calculations</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="525"/>
        <source>&lt;b&gt;Note:&lt;/b&gt; Specify the search radius as a percentage of the map width</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="535"/>
        <source>Search radius for identifying features and displaying map tips</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="590"/>
        <source>Line width</source>
        <translation type="unfinished">Līnijas platums</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="610"/>
        <source>Line colour</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="642"/>
        <source>Default snap mode</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="672"/>
        <source>Default snapping tolerance in layer units</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="702"/>
        <source>Search radius for vertex edits in layer units</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="735"/>
        <source>Vertex markers</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="741"/>
        <source>Marker style</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="847"/>
        <source>Prompt for projection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="854"/>
        <source>Project wide default projection will be used</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="861"/>
        <source>Global default projection displa&amp;yed below will be used</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="878"/>
        <source>Override system locale</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="900"/>
        <source>&lt;b&gt;Note:&lt;/b&gt; Enabling / changing overide on local requires an application restart</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="943"/>
        <source>Proxy</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="949"/>
        <source>Use proxy for web access</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="961"/>
        <source>Host</source>
        <translation type="unfinished">Hosts</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="974"/>
        <source>Port</source>
        <translation type="unfinished">Ports</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="987"/>
        <source>User</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="1014"/>
        <source>Leave this blank if no proxy username / password are required</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="1004"/>
        <source>Password</source>
        <translation type="unfinished">Parole</translation>
    </message>
</context>
<context>
    <name>QgsPasteTransformationsBase</name>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="16"/>
        <source>Paste Transformations</source>
        <translation>Ielīmēt transformācijas</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="39"/>
        <source>&lt;b&gt;Note: This function is not useful yet!&lt;/b&gt;</source>
        <translation>&lt;b&gt;Piezīme: Šī funkcija vēl nestrādā!&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="62"/>
        <source>Source</source>
        <translation>Avots</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="83"/>
        <source>Destination</source>
        <translation>Mērķis</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="122"/>
        <source>&amp;Help</source>
        <translation>&amp;Palīdzība</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="125"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="151"/>
        <source>Add New Transfer</source>
        <translation>Pievienot jaunu pārsūtīšanu</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="158"/>
        <source>&amp;OK</source>
        <translation>&amp;Labi</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="174"/>
        <source>&amp;Cancel</source>
        <translation>&amp;Atcelt</translation>
    </message>
</context>
<context>
    <name>QgsPatternDialogBase</name>
    <message>
        <location filename="" line="135533324"/>
        <source>Select a fill pattern</source>
        <translation type="obsolete">Izvēlieties aizpildījuma rakstu</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Cancel</source>
        <translation type="obsolete">Atcelt</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Ok</source>
        <translation type="obsolete">Labi</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>No Fill</source>
        <translation type="obsolete">Bez aizpildījuma</translation>
    </message>
</context>
<context>
    <name>QgsPgGeoprocessing</name>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="117"/>
        <source>Buffer features in layer %1</source>
        <translation>Veidoju buferi objektiem slānī %1</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="340"/>
        <source>Error connecting to the database</source>
        <translation>Kļūda pieslēdzoties datu bāzei</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="78"/>
        <source>&amp;Buffer features</source>
        <translation>Izveidot &amp;buferi ap obejktiem</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="80"/>
        <source>A new layer is created in the database with the buffered features.</source>
        <translation>Tiek izveidots jauns slānis datu bāzē ar objektu buferiem.</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="410"/>
        <source>&amp;Geoprocessing</source>
        <translation>Ģe&amp;oapstrāde</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="325"/>
        <source>Unable to add geometry column</source>
        <translation>Nav iespējams pievienot ģeometrijas kolonnu</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="327"/>
        <source>Unable to add geometry column to the output table </source>
        <translation>Nebija iespējams pievienot ģeometrijas kolonnu izvades tabulai </translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="331"/>
        <source>Unable to create table</source>
        <translation>Nav iespējams izveidot tabulu</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="333"/>
        <source>Failed to create the output table </source>
        <translation>Neizdevās izveidot izvades tabulu </translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="346"/>
        <source>No GEOS support</source>
        <translation>Nav GEOS atbalsta</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="347"/>
        <source>Buffer function requires GEOS support in PostGIS</source>
        <translation>Buferu veidošanai ir nepieciešams GEOS atbalsts PostGIS</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="356"/>
        <source>No Active Layer</source>
        <translation>Nav aktīvā slāņa</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="357"/>
        <source>You must select a layer in the legend to buffer</source>
        <translation>Jums ir jāizvēlas slānis, kuram tiks veidoti buferi</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="80"/>
        <source>Create a buffer for a PostgreSQL layer. </source>
        <translation>Izveidot buferi PostgrSQL slānim. </translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="350"/>
        <source>Not a PostgreSQL/PostGIS Layer</source>
        <translation>Nav PostgreSQL/PostGIS slānis</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="353"/>
        <source> is not a PostgreSQL/PostGIS layer.
</source>
        <translation> nav PostgreSQL/PostGIS slānis.</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="353"/>
        <source>Geoprocessing functions are only available for PostgreSQL/PostGIS Layers</source>
        <translation>Ģeoprocesēšana ir pieejama tikai PostgreSQL/PostGIS slāņiem</translation>
    </message>
</context>
<context>
    <name>QgsPgQueryBuilder</name>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="83"/>
        <source>Table &lt;b&gt;%1&lt;/b&gt; in database &lt;b&gt;%2&lt;/b&gt; on host &lt;b&gt;%3&lt;/b&gt;, user &lt;b&gt;%4&lt;/b&gt;</source>
        <translation>Tabula &lt;b&gt;%1&lt;/b&gt; datubāzē &lt;b&gt;%2&lt;/b&gt; uz hosta &lt;b&gt;%3&lt;/b&gt;, lietotājs &lt;b&gt;%4&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="67"/>
        <source>Connection Failed</source>
        <translation>Savienojums neizdevās</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="67"/>
        <source>Connection to the database failed:</source>
        <translation>Savienojums ar datubāzi neizdevās: </translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="204"/>
        <source>Database error</source>
        <translation>Datubāzes kļūda</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="279"/>
        <source>Query Result</source>
        <translation>Pieprasīt rezultātu</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="281"/>
        <source>The where clause returned </source>
        <translation>where nosacījums deva </translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="281"/>
        <source> rows.</source>
        <translation> rindas.</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="285"/>
        <source>Query Failed</source>
        <translation>Pieprasījums neveiksmīgs</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="287"/>
        <source>An error occurred when executing the query:</source>
        <translation>Izpildot pieprasījumu gadījās kļūda: </translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="340"/>
        <source>No Records</source>
        <translation>Nav ierakstu</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="340"/>
        <source>The query you specified results in zero records being returned. Valid PostgreSQL layers must have at least one feature.</source>
        <translation>Jūsu izveidotais pieprasījums nedeva ne vienu rezultātu. Derīgiem PostgreSQL slāņiem jāsatur vismaz viens objekts.</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="204"/>
        <source>&lt;p&gt;Failed to get sample of field values using SQL:&lt;/p&gt;&lt;p&gt;</source>
        <translation>&lt;p&gt;Neizdevās iegūt lauka vērtību paraugus izmantojot SQL:&lt;/p&gt;&lt;p&gt;</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="268"/>
        <source>No Query</source>
        <translation>Nav vaicājums</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="268"/>
        <source>You must create a query before you can test it</source>
        <translation>Pirms pārbaudi, izveido vaicājumu</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="334"/>
        <source>Error in Query</source>
        <translation>Kļūda vaicājumā</translation>
    </message>
</context>
<context>
    <name>QgsPgQueryBuilderBase</name>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="21"/>
        <source>PostgreSQL Query Builder</source>
        <translation>PostgreSQL pieprasījumu veidotājs</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="328"/>
        <source>Clear</source>
        <translation>Attīrīt</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="338"/>
        <source>Test</source>
        <translation>Testēt</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="348"/>
        <source>Ok</source>
        <translation>Labi</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="358"/>
        <source>Cancel</source>
        <translation>Atcelt</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="83"/>
        <source>Values</source>
        <translation>Vērtības</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="139"/>
        <source>All</source>
        <translation>Viss</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="126"/>
        <source>Sample</source>
        <translation>Paraugs</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="46"/>
        <source>Fields</source>
        <translation>Lauki</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Datasource:</source>
        <translation type="obsolete">Datu avots:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="149"/>
        <source>Operators</source>
        <translation>Operatori</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="167"/>
        <source>=</source>
        <translation>=</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="209"/>
        <source>IN</source>
        <translation>IN</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="216"/>
        <source>NOT IN</source>
        <translation>NOT IN</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="174"/>
        <source>&lt;</source>
        <translation>&lt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="230"/>
        <source>&gt;</source>
        <translation>&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="202"/>
        <source>%</source>
        <translation>%</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="258"/>
        <source>&lt;=</source>
        <translation>&lt;=</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="251"/>
        <source>&gt;=</source>
        <translation>&gt;=</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="223"/>
        <source>!=</source>
        <translation>!=</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="237"/>
        <source>LIKE</source>
        <translation>LIKE</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="195"/>
        <source>AND</source>
        <translation>AND</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="244"/>
        <source>ILIKE</source>
        <translation>ILIKE</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="188"/>
        <source>OR</source>
        <translation>OR</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="181"/>
        <source>NOT</source>
        <translation>NOT</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="274"/>
        <source>SQL where clause</source>
        <translation>SQL where nosacījums</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="133"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Retrieve &lt;span style=&quot; font-weight:600;&quot;&gt;all&lt;/span&gt; the record in the vector file (&lt;span style=&quot; font-style:italic;&quot;&gt;if the table is big, the operation can consume some time&lt;/span&gt;)&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="120"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Take a &lt;span style=&quot; font-weight:600;&quot;&gt;sample&lt;/span&gt; of records in the vector file&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Noņemt ierakstu &lt;span style=&quot; font-weight:600;&quot;&gt;paraugu&lt;/span&gt; no vektoru faila.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="101"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;List of values for the current field.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Parādīt pašreizējā lauka vērtības.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="64"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;List of fields in this vector file&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Šī vektoru faila lauku saraksts&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="33"/>
        <source>Datasource</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsPluginManager</name>
    <message>
        <location filename="../src/app/qgspluginmanager.cpp" line="203"/>
        <source>No Plugins</source>
        <translation>Nav spraudņu</translation>
    </message>
    <message>
        <location filename="../src/app/qgspluginmanager.cpp" line="203"/>
        <source>No QGIS plugins found in </source>
        <translation>QGIS spraudņi nav atrasti iekš </translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Name</source>
        <translation type="obsolete">Nosaukums</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Version</source>
        <translation type="obsolete">Versija</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Description</source>
        <translation type="obsolete">Apraksts</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Library name</source>
        <translation type="obsolete">Bibliotēkas nosaukums</translation>
    </message>
    <message>
        <location filename="../src/app/qgspluginmanager.cpp" line="84"/>
        <source>&amp;Select All</source>
        <translation type="unfinished">&amp;Izvēlēties visus</translation>
    </message>
    <message>
        <location filename="../src/app/qgspluginmanager.cpp" line="85"/>
        <source>&amp;Clear All</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsPluginManagerBase</name>
    <message>
        <location filename="" line="135533324"/>
        <source>Plugin Directory</source>
        <translation type="obsolete">Spraudņu mape</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>To load a plugin, click the checkbox next to the plugin and click Ok</source>
        <translation type="obsolete">Lai ielādētu spraudni, iezīmējiet ķekškasti blakus spraudņa nosaukumam un nospiediet Labi</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&amp;Select All</source>
        <translation type="obsolete">&amp;Izvēlēties visus</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Alt+S</source>
        <translation type="obsolete">Alt+S</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>C&amp;lear All</source>
        <translation type="obsolete">&amp;Tīrīt visus</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Alt+L</source>
        <translation type="obsolete">Alt+L</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&amp;Ok</source>
        <translation type="obsolete">&amp;Labi</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Alt+O</source>
        <translation type="obsolete">Alt+O</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&amp;Close</source>
        <translation type="obsolete">&amp;Aizvērt</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Alt+C</source>
        <translation type="obsolete">Alt+C</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="16"/>
        <source>QGIS Plugin Manager</source>
        <translation>QGIS spraudņu pārvaldnieks</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="25"/>
        <source>To enable / disable a plugin, click its checkbox or description</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="45"/>
        <source>&amp;Filter</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="58"/>
        <source>Plugin Directory:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="71"/>
        <source>Directory</source>
        <translation type="unfinished">Mape</translation>
    </message>
</context>
<context>
    <name>QgsPointDialog</name>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="488"/>
        <source>Zoom In</source>
        <translation>Tuvināt</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="487"/>
        <source>z</source>
        <translation>z</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="493"/>
        <source>Zoom Out</source>
        <translation>Tālināt</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="492"/>
        <source>Z</source>
        <translation>Z</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="496"/>
        <source>Zoom To Layer</source>
        <translation>Tuvināt līdz slānim</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="498"/>
        <source>Zoom to Layer</source>
        <translation>Tuvina līdz slānim</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="501"/>
        <source>Pan Map</source>
        <translation>Panoramēt karti</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="502"/>
        <source>Pan the map</source>
        <translation>Panoramē karti</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="505"/>
        <source>Add Point</source>
        <translation>Pievienot punktu</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="506"/>
        <source>.</source>
        <translation>.</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="507"/>
        <source>Capture Points</source>
        <translation>Atliek punktus</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="510"/>
        <source>Delete Point</source>
        <translation>Dzēst punktu</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="511"/>
        <source>Delete Selected</source>
        <translation>Dzēst izvēlēto</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="559"/>
        <source>Linear</source>
        <translation>Lineārs</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="560"/>
        <source>Helmert</source>
        <translation>Helmerta</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="198"/>
        <source>Choose a name for the world file</source>
        <translation>Izvēlieties piesaistes faila nosaukumu</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="216"/>
        <source>-modified</source>
        <comment>Georeferencer:QgsPointDialog.cpp - used to modify a user given filename</comment>
        <translation>-mainīts</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="265"/>
        <source>Warning</source>
        <translation>Brīdinājums</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="280"/>
        <source>Affine</source>
        <translation>Affine</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="290"/>
        <source>Not implemented!</source>
        <translation>Rīks vēl nav izstrādāts!</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="285"/>
        <source>&lt;p&gt;An affine transform requires changing the original raster file. This is not yet supported.&lt;/p&gt;</source>
        <translation>&lt;p&gt;Affine transformācijai ir nepieciešams pārveidot orģinālo failu. Tas vēl nav atbalstīts.&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="292"/>
        <source>&lt;p&gt;The </source>
        <translation>&lt;p&gt; </translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="293"/>
        <source> transform is not yet supported.&lt;/p&gt;</source>
        <translation> transformācija vēl nav atbalstīta.&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="324"/>
        <source>Error</source>
        <translation>Kļūda</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="325"/>
        <source>Could not write to </source>
        <translation>Nebija iespējams rakstīt </translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="272"/>
        <source>Currently all modified files will be written in TIFF format.</source>
        <translation>Pašlaik visi labotie faili tiks saglabāti TIFF formātā.</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="271"/>
        <source>&lt;p&gt;A Helmert transform requires modifications in the raster layer.&lt;/p&gt;&lt;p&gt;The modified raster will be saved in a new file and a world file will be generated for this new file instead.&lt;/p&gt;&lt;p&gt;Are you sure that this is what you want?&lt;/p&gt;</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsPointDialogBase</name>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="65"/>
        <source>Transform type:</source>
        <translation>Transformācijas tips:</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="178"/>
        <source>Zoom in</source>
        <translation>Tuvināt</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="200"/>
        <source>Zoom out</source>
        <translation>Tālināt</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="222"/>
        <source>Zoom to the raster extents</source>
        <translation>Tuvināt līdz rastra kopskatam</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="244"/>
        <source>Pan</source>
        <translation>Panoramēt</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="105"/>
        <source>Add points</source>
        <translation>Pievienot punktus</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="130"/>
        <source>Delete points</source>
        <translation>Dzēst punktus</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="52"/>
        <source>World file:</source>
        <translation>Piesaistes fails:</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="38"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="45"/>
        <source>Modified raster:</source>
        <translation>Modificētais rastrs:</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="13"/>
        <source>Reference points</source>
        <translation>Atskaites punkti</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="75"/>
        <source>Create</source>
        <translation>Izveidot</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="282"/>
        <source>Create and load layer</source>
        <translation>Izveidot un ielādēt slāni</translation>
    </message>
</context>
<context>
    <name>QgsPointStyleWidgetBase</name>
    <message>
        <location filename="" line="135533324"/>
        <source>Form3</source>
        <translation type="obsolete">Forma3</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Symbol Style</source>
        <translation type="obsolete">Simbolu stils</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Scale</source>
        <translation type="obsolete">Mērogs</translation>
    </message>
</context>
<context>
    <name>QgsPostgresProvider</name>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="141"/>
        <source>Unable to access relation</source>
        <translation>Nevaru piekļūt relācijai</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="121"/>
        <source>Unable to access the </source>
        <translation>Nevaru piekļūt </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="144"/>
        <source> relation.
The error message from the database was:
</source>
        <translation> relācijai. 
Datubāzes ziņōtā kļūda bija: </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="105"/>
        <source>No GEOS Support!</source>
        <translation>Nav GEOS atbalsta!</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="109"/>
        <source>Your PostGIS installation has no GEOS support.
Feature selection and identification will not work properly.
Please install PostGIS with GEOS support (http://geos.refractions.net)</source>
        <translation>Jūsu PostGIS instalācijai nav GEOS atbalsta.
Objektu izvēle un identificēšana var nedarboties pareizi.
Lūdzu instalējiet PostGIS ar GEOS atbalstu (http://geos.refractions.net)</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="907"/>
        <source>No suitable key column in table</source>
        <translation>Tabulai nav derīgas atslēgas kolonnas</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="911"/>
        <source>The table has no column suitable for use as a key.

Qgis requires that the table either has a column of type
int4 with a unique constraint on it (which includes the
primary key) or has a PostgreSQL oid column.
</source>
        <translation>Tabulai nav kolonnas, kuru varētu izmantot kā atslēgas kolonnu.

QGIS ir nepieciešams, lai tabula saturēt vai nu kolonnu ar tipu int4 kā
primāro atslēgu, vai arī PostgreSQL oid kolonnu.
</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="952"/>
        <source>The unique index on column</source>
        <translation>Unikālais kolonnas indekss</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="954"/>
        <source>is unsuitable because Qgis does not currently support non-int4 type columns as a key into the table.
</source>
        <translation>nav derīgs, jo QGIS neatbalsta ne-int4 atslēgas kolonnas.</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="977"/>
        <source>and </source>
        <translation>un </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="983"/>
        <source>The unique index based on columns </source>
        <translation>Unikālais indekss, kas bāzēts uz kolonnām </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="985"/>
        <source> is unsuitable because Qgis does not currently support multiple columns as a key into the table.
</source>
        <translation> nav derīgs, jo QGIS pagaidām neatbalsta vairākas kolonnas kā tabulas atslēgu.
</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1028"/>
        <source>Unable to find a key column</source>
        <translation>Nav iespējams atrast atslēgas kolonnu</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1108"/>
        <source> derives from </source>
        <translation> atvasināts no </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1112"/>
        <source>and is suitable.</source>
        <translation>un ir derīgs.</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1116"/>
        <source>and is not suitable </source>
        <translation>un nav derīgs </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1117"/>
        <source>type is </source>
        <translation>tips ir </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1119"/>
        <source> and has a suitable constraint)</source>
        <translation> un ir derīgs ierobežojums)</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1121"/>
        <source> and does not have a suitable constraint)</source>
        <translation>  un nav derīga ierobežojuma)</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1221"/>
        <source>The view you selected has the following columns, none of which satisfy the above conditions:</source>
        <translation>Jūsu izvēlētajam skatam ir sekojošas kolonnas, no kurām ne viena neatbilst sekojošiem nosacījumiem:</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1227"/>
        <source>Qgis requires that the view has a column that can be used as a unique key. Such a column should be derived from a table column of type int4 and be a primary key, have a unique constraint on it, or be a PostgreSQL oid column. To improve performance the column should also be indexed.
</source>
        <translation>QGIS ir nepieciešams, lai skatam būtu kolonna, ko var izmantot kā atslēgas kolonnu. Šāda kolonna var būt no tabulas kolonnas ar tipu int4 un primāro atslēgu, tai var būt ierobežojums, ka tā ir unikāla, vai tā var būt PostgreSQL oid kolonna. Vektspējas uzlabošanai, to ir ieteicams indeksēt.
</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1228"/>
        <source>The view </source>
        <translation>Skats </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1229"/>
        <source>has no column suitable for use as a unique key.
</source>
        <translation>nesatur primārajai atslēgai derīgu kolonnu.
</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1230"/>
        <source>No suitable key column in view</source>
        <translation>Skatam nav derīgas atslēgas kolonnas</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2530"/>
        <source>Unknown geometry type</source>
        <translation>Nezināms ģeometrijas tips</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2531"/>
        <source>Column </source>
        <translation>Kolonna </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2541"/>
        <source> in </source>
        <translation> iekš </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2533"/>
        <source> has a geometry type of </source>
        <translation> ir ģeometrijas tips </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2533"/>
        <source>, which Qgis does not currently support.</source>
        <translation>, ko QGIS pagaidām neatbalsta.</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2542"/>
        <source>. The database communication log was:
</source>
        <translation>. Saruna ar datu bāzi bija šāda: 
</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2543"/>
        <source>Unable to get feature type and srid</source>
        <translation>Nav iespējams saņemt objektu tipu un srid</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1207"/>
        <source>Note: </source>
        <translation>Piezīme: </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1209"/>
        <source>initially appeared suitable but does not contain unique data, so is not suitable.
</source>
        <translation>no sākuma izskatījās derīga, taču vēlāk atklājās, ka dati nav unikāli un tādēļ kolonna ir nederīga.
</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="142"/>
        <source>Unable to determine table access privileges for the </source>
        <translation>Nav iespējams noskaidrot tabulas piekļuves atļaujas priekš </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1869"/>
        <source>Error while adding features</source>
        <translation>Kļūda pievienojot objektus</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1903"/>
        <source>Error while deleting features</source>
        <translation>Kļūda dzēšot objektus</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1935"/>
        <source>Error while adding attributes</source>
        <translation>Kļūda pievienojot atribūtus</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1974"/>
        <source>Error while deleting attributes</source>
        <translation>Kļūda dzēšot atribūtus</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2038"/>
        <source>Error while changing attributes</source>
        <translation>Kļūda mainot atribūtus</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2117"/>
        <source>Error while changing geometry values</source>
        <translation>Kļūda maibot ģeometrijas vērtības</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2540"/>
        <source>Qgis was unable to determine the type and srid of column </source>
        <translation>QGIS nespēja noteikt kolonnas tipu un srid</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.h" line="495"/>
        <source>unexpected PostgreSQL error</source>
        <translation>negaidīta PostgreSQL kļūda</translation>
    </message>
</context>
<context>
    <name>QgsProjectPropertiesBase</name>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="13"/>
        <source>Project Properties</source>
        <translation>Projekta īpašības</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Map Units</source>
        <translation type="obsolete">Kartes vienībās</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="157"/>
        <source>Meters</source>
        <translation>Metri</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="167"/>
        <source>Feet</source>
        <translation>Pēdas</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="174"/>
        <source>Decimal degrees</source>
        <translation>Decimālie grādi</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="54"/>
        <source>Default project title</source>
        <translation>Projekta noklusējuma nosaukums</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="32"/>
        <source>General</source>
        <translation>Vispārējas īpašības</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="196"/>
        <source>Automatic</source>
        <translation>Automātiska</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="190"/>
        <source>Automatically sets the number of decimal places in the mouse position display</source>
        <translation>Automātiski iestata decimālo vietu skaitu kursora novietojuma precizitātei</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="193"/>
        <source>The number of decimal places that are used when displaying the mouse position is automatically set to be enough so that moving the mouse by one pixel gives a change in the position display</source>
        <translation>Decimālo virtu skaits tiek iestatīts pietiekams tā, lai pārvietojot peli pa vienu pikseli, būtu redzama arī pārvietošanās uz ekrāna</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="212"/>
        <source>Manual</source>
        <translation>Manuāla</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="209"/>
        <source>Sets the number of decimal places to use for the mouse position display</source>
        <translation>Iestata decimālo vietu skaitu, ko izmanto kursora novietojuma precizitātes noteikšanai</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="222"/>
        <source>The number of decimal places for the manual option</source>
        <translation>Decimālo vietu skaits manuālai iestatīšanai</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="235"/>
        <source>decimal places</source>
        <translation>decimālas vietas</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Map Appearance</source>
        <translation type="obsolete">Kartes izskats</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Selection Color:</source>
        <translation type="obsolete">Atlases krāsa:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Project Title</source>
        <translation type="obsolete">Projekta nosaukums</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="282"/>
        <source>Projection</source>
        <translation>Projekcija</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="300"/>
        <source>Enable on the fly projection</source>
        <translation>Projekcijas pārveidošana pēc vajadzības</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Background Color:</source>
        <translation type="obsolete">Fona krāsa:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="184"/>
        <source>Precision</source>
        <translation>Precizitāte</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="251"/>
        <source>Digitizing</source>
        <translation>Digitizēšana</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="51"/>
        <source>Descriptive project name</source>
        <translation>Aprakstošs projekta nosaukums</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="257"/>
        <source>Enable topological editing</source>
        <translation>Ieslēgt topoloģisko rediģēšanu</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="271"/>
        <source>Snapping options...</source>
        <translation>Pielipšanas opcijas...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="264"/>
        <source>Avoid intersections of new polygons</source>
        <translation>Novērst jaunu poligonu pārklāšanos</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="38"/>
        <source>Title and colors</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="44"/>
        <source>Project title</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="61"/>
        <source>Selection color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="100"/>
        <source>Background color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="139"/>
        <source>Map units</source>
        <translation type="unfinished">Kartes vienībās</translation>
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
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="19"/>
        <source>Projection Selector</source>
        <translation>Projekciju izvēle</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="52"/>
        <source>Projection</source>
        <translation>Projekcija</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="82"/>
        <source>Search</source>
        <translation>Meklēt</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="112"/>
        <source>Find</source>
        <translation>Atrast</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="164"/>
        <source>Postgis SRID</source>
        <translation>Postgis SRID</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="151"/>
        <source>EPSG ID</source>
        <translation>EPSG ID</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="138"/>
        <source>QGIS SRSID</source>
        <translation>QGIS SRSID</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="125"/>
        <source>Name</source>
        <translation>Nosaukums</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="221"/>
        <source>Spatial Reference System</source>
        <translation>Telpisko norāžu sistēma</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="226"/>
        <source>Id</source>
        <translation>Id</translation>
    </message>
</context>
<context>
    <name>QgsPythonDialog</name>
    <message>
        <location filename="../src/ui/qgspythondialog.ui" line="13"/>
        <source>Python console</source>
        <translation>Python konsole</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspythondialog.ui" line="58"/>
        <source>&gt;&gt;&gt;</source>
        <translation>&gt;&gt;&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspythondialog.ui" line="33"/>
        <source>To access Quantum GIS environment from this python console use object from global scope which is an instance of QgisInterface class.&lt;br&gt;Usage e.g.: iface.zoomFull()</source>
        <translation type="unfinished"></translation>
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
        <translation> jūdzes</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="869"/>
        <source> mile</source>
        <translation> jūdze</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="874"/>
        <source> inches</source>
        <translation> collas</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="879"/>
        <source> foot</source>
        <translation> pēdas</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="883"/>
        <source> feet</source>
        <translation> pēda</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="888"/>
        <source> degree</source>
        <translation> grāds</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="890"/>
        <source> degrees</source>
        <translation> grādi</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="893"/>
        <source> unknown</source>
        <translation> nezināms</translation>
    </message>
</context>
<context>
    <name>QgsRasterLayer</name>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.h" line="513"/>
        <source>Not Set</source>
        <translation>Neiestatīts</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2202"/>
        <source>Raster Extent: </source>
        <translation>Rastra apjoms: </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2205"/>
        <source>Clipped area: </source>
        <translation>Izgriestais apgabals: </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3425"/>
        <source>Driver:</source>
        <translation>Dzinējs:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3502"/>
        <source>Dimensions:</source>
        <translation>Dimensijas:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3505"/>
        <source>X: </source>
        <translation>X: </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3506"/>
        <source> Y: </source>
        <translation> Y: </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3506"/>
        <source> Bands: </source>
        <translation> Kanāli: </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3528"/>
        <source>Data Type:</source>
        <translation>Datu tips: </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3534"/>
        <source>GDT_Byte - Eight bit unsigned integer</source>
        <translation>GDT_Byte - Astoņu bitu neiezīmētu veselu skaitļu</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3537"/>
        <source>GDT_UInt16 - Sixteen bit unsigned integer </source>
        <translation>GDT_UInt16 - Sešpadsmit bitu neiezīmētu veselu skaitļu</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3540"/>
        <source>GDT_Int16 - Sixteen bit signed integer </source>
        <translation>GDT_Int16 - Sešpadsmit bitu zīmētu veselu skaitļu</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3543"/>
        <source>GDT_UInt32 - Thirty two bit unsigned integer </source>
        <translation>GDT_UInt32 - Trīsdesmit divu bitu nezīmētu veselu skaitļu</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3546"/>
        <source>GDT_Int32 - Thirty two bit signed integer </source>
        <translation>GDT_Int32 - Trīsdesmit divu bitu zīmēts vesels skaitlis </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3549"/>
        <source>GDT_Float32 - Thirty two bit floating point </source>
        <translation>GDT_Float32 - Trīsdesmit četru bitu daļskaitlis </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3552"/>
        <source>GDT_Float64 - Sixty four bit floating point </source>
        <translation>GDT_Float64 - Sešdesmit četru bitu daļskaitlis </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3555"/>
        <source>GDT_CInt16 - Complex Int16 </source>
        <translation>GDT_CInt16 - Komplekss Int16 </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3558"/>
        <source>GDT_CInt32 - Complex Int32 </source>
        <translation>GDT_CInt32 - Komplekss Int32 </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3561"/>
        <source>GDT_CFloat32 - Complex Float32 </source>
        <translation>GDT_CFloat32 - Komplekss Float32 </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3564"/>
        <source>GDT_CFloat64 - Complex Float64 </source>
        <translation>GDT_CFloat64 - Komplekss Float64 </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3567"/>
        <source>Could not determine raster data type.</source>
        <translation>Rastra tipu nebija iespējams noteikt.</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3572"/>
        <source>Pyramid overviews:</source>
        <translation>Piramīdu pārskats:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3621"/>
        <source>Origin:</source>
        <translation>Izcelsme:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3630"/>
        <source>Pixel Size:</source>
        <translation>Pikseļa izmērs:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Property</source>
        <translation type="obsolete">Īpašība</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Value</source>
        <translation type="obsolete">Vērtība</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4793"/>
        <source>Band</source>
        <translation>Kanāls</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3655"/>
        <source>Band No</source>
        <translation>Kanāla Nr</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3667"/>
        <source>No Stats</source>
        <translation>Nav statistikas</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3670"/>
        <source>No stats collected yet</source>
        <translation>Statistika vēl nav savākta</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3680"/>
        <source>Min Val</source>
        <translation>Min vērt</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3688"/>
        <source>Max Val</source>
        <translation>Maks vērt</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3696"/>
        <source>Range</source>
        <translation>Diapazons</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3704"/>
        <source>Mean</source>
        <translation>Vidējā vērtība</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3712"/>
        <source>Sum of squares</source>
        <translation>Kvadrātu summa</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3720"/>
        <source>Standard Deviation</source>
        <translation>Standarta novirze</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3728"/>
        <source>Sum of all cells</source>
        <translation>Visu šūnu summa</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3736"/>
        <source>Cell Count</source>
        <translation>Šūnu skaits</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3822"/>
        <source>Average Magphase</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3827"/>
        <source>Average</source>
        <translation>Vidēji</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3593"/>
        <source>Layer Spatial Reference System: </source>
        <translation>Slāņa telpisko norāžu sistēma: </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4749"/>
        <source>out of extent</source>
        <translation>ārpus apjoma</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4787"/>
        <source>null (no data)</source>
        <translation>null (nav datu)</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3450"/>
        <source>Dataset Description</source>
        <translation>Datu kopas apraksts</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3513"/>
        <source>No Data Value</source>
        <translation>Nav datu vērtība</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="242"/>
        <source>and all other files</source>
        <translation>un visi citi faili</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3522"/>
        <source>NoDataValue not set</source>
        <translation>Nav iestatīta NavDatu vērtība</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3471"/>
        <source>Band %1</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsRasterLayerProperties</name>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1032"/>
        <source>Grayscale</source>
        <translation>Pelēktoņu</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2412"/>
        <source>Pseudocolor</source>
        <translation>Pseidokrāsu</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2412"/>
        <source>Freak Out</source>
        <translation>Izceļošs</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="177"/>
        <source>Palette</source>
        <translation>Palete</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="825"/>
        <source>Columns: </source>
        <translation>Kolonnas: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="826"/>
        <source>Rows: </source>
        <translation>Rindas: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="827"/>
        <source>No-Data Value: </source>
        <translation>Nav datu vērtība: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="827"/>
        <source>n/a</source>
        <translation>n/p</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1849"/>
        <source>Write access denied</source>
        <translation>Rakstīšanas atļauja liegta</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1849"/>
        <source>Write access denied. Adjust the file permissions and try again.

</source>
        <translation>Rakstīšanas atļauja liegta. Izmaniet faila piekļuves tiesības un mēģiniet vēlreiz.
</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1594"/>
        <source>Building pyramids failed.</source>
        <translation>Piramīdu izveidošana neizdevās.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1590"/>
        <source>The file was not writeable. Some formats can not be written to, only read. You can also try to check the permissions and then try again.</source>
        <translation>Fails nav rakstāms. Dažus formātus var tikai lasīt, bet ne rakstīt. Jūs varat pārbaudīt faila piekļuves tiesības un mēģināt vēlreiz.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1595"/>
        <source>Building pyramid overviews is not supported on this type of raster.</source>
        <translation>Pārskata piramīdu veidošana šī tipa rastram nav atbalstīta.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1714"/>
        <source>Custom Colormap</source>
        <translation>Pielāgota krāsu karte</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2777"/>
        <source>No Stretch</source>
        <translation>Neizstiept</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2782"/>
        <source>Stretch To MinMax</source>
        <translation>Izstiept līdz MinMaks</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2787"/>
        <source>Stretch And Clip To MinMax</source>
        <translation>Izstiept un apcirst līdz MinMaks</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2792"/>
        <source>Clip To MinMax</source>
        <translation>Apcirst līdz MinMaks</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1427"/>
        <source>Discrete</source>
        <translation>Diskrēti</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="907"/>
        <source>Linearly</source>
        <translation>Lineāri</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2622"/>
        <source>Equal interval</source>
        <translation>Vienādos intervālos</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2644"/>
        <source>Quantiles</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="332"/>
        <source>Description</source>
        <translation>Apraksts</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="333"/>
        <source>Large resolution raster layers can slow navigation in QGIS.</source>
        <translation>Augstas izšķirtspējas rastra slāņi var padarīt navigāciju iekš QGIS lēnu.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="334"/>
        <source>By creating lower resolution copies of the data (pyramids) performance can be considerably improved as QGIS selects the most suitable resolution to use depending on the level of zoom.</source>
        <translation>Izveidojot zemākas izšķirtspējas datu kopijas (piramīdas) var krietni uzlabot pārzīmēšanas ātrumu, jo QGIS automātiski izvēlēsies piemērotāko kopiju atkarībā no pašreizējās kartes tuvinājuma pakāpes.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="335"/>
        <source>You must have write access in the directory where the original data is stored to build pyramids.</source>
        <translation>Jums ir jābūt rakstīšanas tiesībām mapē, lai varētu izveidot piramīdas.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="336"/>
        <source>Please note that building pyramids may alter the original data file and once created they cannot be removed!</source>
        <translation>Lūdzu ievērojiet, ka piramīdu veidošana ietekmē orģinālos datus un šo darbību vairs nav iespējams atdarīt.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="337"/>
        <source>Please note that building pyramids could corrupt your image - always make a backup of your data first!</source>
        <translation>Ievērojiet, ka piramīdu veidošana var sabojāt jūsu datus, tādēļ vispirms izveidojiet datu rezerves kopiju!</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1821"/>
        <source>Red</source>
        <translation>Sarkans</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1821"/>
        <source>Green</source>
        <translation>Zaļš</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1821"/>
        <source>Blue</source>
        <translation>Zils</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1838"/>
        <source>Percent Transparent</source>
        <translation>Caurspīdīgums procentos</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1834"/>
        <source>Gray</source>
        <translation>Pelēks</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1838"/>
        <source>Indexed Value</source>
        <translation>Indeksēta vērtība</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2774"/>
        <source>User Defined</source>
        <translation>Lietotāja definēts</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>No Scaling</source>
        <translation type="obsolete">Bez mērogošanas</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="819"/>
        <source>No-Data Value: Not Set</source>
        <translation>Nav datu vērība: Neiestatīta</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1806"/>
        <source>Save file</source>
        <translation>Saglabāt failu</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2308"/>
        <source>Textfile (*.txt)</source>
        <translation>Teksta fails (*.txt)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1818"/>
        <source>QGIS Generated Transparent Pixel Value Export File</source>
        <translation>QGIS ģenerēts caurspīdīgo pikseļu vērtību eksporta fails</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2308"/>
        <source>Open file</source>
        <translation>Atvērt failu</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2381"/>
        <source>Import Error</source>
        <translation>Importēšanas kļūda</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2381"/>
        <source>The following lines contained errors

</source>
        <translation>Sekojošās rindās bija kļūdas</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2386"/>
        <source>Read access denied</source>
        <translation>Lasīšanas pieeja ir liegta</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2386"/>
        <source>Read access denied. Adjust the file permissions and try again.

</source>
        <translation>Lasīšanas pieeja ir liegta. Pārbaudiet faila piekļuves tiesības un mēģiniet vēlreiz.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2412"/>
        <source>Color Ramp</source>
        <translation>Krāsu rampa</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="56"/>
        <source>Not Set</source>
        <translation type="unfinished">Neiestatīts</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2999"/>
        <source>Default Style</source>
        <translation type="unfinished">Noklusējuma stils</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2984"/>
        <source>QGIS Layer Style File (*.qml)</source>
        <translation type="unfinished">QGIS slāņa stila fails (*.qml)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="3005"/>
        <source>QGIS</source>
        <translation type="unfinished">QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="3006"/>
        <source>Unknown style format: </source>
        <translation type="unfinished">Nezināms stila formāts: </translation>
    </message>
</context>
<context>
    <name>QgsRasterLayerPropertiesBase</name>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="13"/>
        <source>Raster Layer Properties</source>
        <translation>Rastra slāņa īpašības</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="32"/>
        <source>Symbology</source>
        <translation>Siboloģija</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="881"/>
        <source>&lt;p align=&quot;right&quot;&gt;Full&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;right&quot;&gt;Pilns&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="835"/>
        <source>None</source>
        <translation>Nav</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Invert Color Map</source>
        <translation type="obsolete">Inversas krāsas</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Gray</source>
        <translation type="obsolete">Pelēks</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Color Map</source>
        <translation type="obsolete">Krāsu karte</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1306"/>
        <source>General</source>
        <translation>Vispārējs</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Display Name:</source>
        <translation type="obsolete">Attēlošanas nosaukums:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Layer Source:</source>
        <translation type="obsolete">Slāņa avots:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1517"/>
        <source>Thumbnail</source>
        <translation>Sīkatēls</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1450"/>
        <source>Columns:</source>
        <translation>Kolonnas:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1464"/>
        <source>No Data:</source>
        <translation>Nav datu:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1457"/>
        <source>Rows:</source>
        <translation>Rindas:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1570"/>
        <source>Legend:</source>
        <translation>Leģenda:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1623"/>
        <source>Palette:</source>
        <translation>Palete:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1679"/>
        <source>Metadata</source>
        <translation>Metadati</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1701"/>
        <source>Pyramids</source>
        <translation>Piramīdas</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Resampling Method</source>
        <translation type="obsolete">Transformācijas metode</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1790"/>
        <source>Average</source>
        <translation>Vidējs</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1795"/>
        <source>Nearest Neighbour</source>
        <translation>Tuvākais kaimiņš</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Build Pyramids</source>
        <translation type="obsolete">Veidot piramīdas</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Pyramid Resolutions</source>
        <translation type="obsolete">Piramīdu izšķirtspēja</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Maximum 1:</source>
        <translation type="obsolete">Maksimāli 1:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1380"/>
        <source>Maximum scale at which this layer will be displayed. </source>
        <translation>Maksimālais mērogs pie kāda šis slānis tiks rādīts. </translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Minimum 1:</source>
        <translation type="obsolete">Minimāli 1:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1403"/>
        <source>Minimum scale at which this layer will be displayed. </source>
        <translation>Minimālais mērogs pie kāda šis slānis tiks rādīts.  </translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Spatial Reference System</source>
        <translation type="obsolete">Telpisko norāžu sistēma</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1342"/>
        <source>Change</source>
        <translation>Mainīt</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1813"/>
        <source>Histogram</source>
        <translation>Histogramma</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1907"/>
        <source>Options</source>
        <translation>Parametri</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Out Of Range OK?</source>
        <translation type="obsolete">Iekļaut ārpusreģiona datus?</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Allow Approximation</source>
        <translation type="obsolete">Atļaut noapaļošanu</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1862"/>
        <source>Chart Type</source>
        <translation>Grafika tips</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Bar Chart</source>
        <translation type="obsolete">Stabiņu</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Line Graph</source>
        <translation type="obsolete">Līniju</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1900"/>
        <source>Refresh</source>
        <translation>Atsvaidzināt</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>DebugInfo</source>
        <translation type="obsolete">AtkļūdošanasInformācija</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Scale Dependent Visibility</source>
        <translation type="obsolete">Mērogatkarīga redzamība</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Column Count:</source>
        <translation type="obsolete">Kolonnu skaits:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Grayscale Band Scaling</source>
        <translation type="obsolete">Pelēktoņu kanāla mērogošana</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="584"/>
        <source>Max</source>
        <translation>Maks</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Std Deviation</source>
        <translation type="obsolete">Standartnovirze</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Custom Min Max Values:</source>
        <translation type="obsolete">Pielāgotas Min Maks vērtības:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="546"/>
        <source>Min</source>
        <translation>Min</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Contrast Enhancement</source>
        <translation type="obsolete">Kontrasta uzlabošana</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Load Min Max Values From Band(s)</source>
        <translation type="obsolete">Ielādēt Min Maks vērtības no kalāliem</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>RGB Scaling</source>
        <translation type="obsolete">RGB mērogošana</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&lt;b&gt;&lt;font color=&apos;blue&apos;&gt;Max&lt;/font&gt;&lt;/b&gt;</source>
        <translation type="obsolete">&lt;b&gt;&lt;font color=&apos;blue&apos;&gt;Maks&lt;/font&gt;&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Custom Min Max Values</source>
        <translation type="obsolete">Pielāgotas Min Maks vērtības</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&lt;b&gt;&lt;font color=&apos;green&apos;&gt;Min&lt;/font&gt;&lt;/b&gt;</source>
        <translation type="obsolete">&lt;b&gt;&lt;font color=&apos;green&apos;&gt;Min&lt;/font&gt;&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&lt;b&gt;&lt;font color=&apos;green&apos;&gt;Max&lt;/font&gt;&lt;/b&gt;</source>
        <translation type="obsolete">&lt;b&gt;&lt;font color=&apos;green&apos;&gt;Maks&lt;/font&gt;&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&lt;b&gt;&lt;font color=&apos;red&apos;&gt;Min&lt;/font&gt;&lt;/b&gt;</source>
        <translation type="obsolete">&lt;b&gt;&lt;font color=&apos;red&apos;&gt;Min&lt;/font&gt;&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&lt;b&gt;&lt;font color=&apos;red&apos;&gt;Max&lt;/font&gt;&lt;/b&gt;</source>
        <translation type="obsolete">&lt;b&gt;&lt;font color=&apos;red&apos;&gt;Maks&lt;/font&gt;&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&lt;b&gt;&lt;font color=&apos;blue&apos;&gt;Min&lt;/font&gt;&lt;/b&gt;</source>
        <translation type="obsolete">&lt;b&gt;&lt;font color=&apos;blue&apos;&gt;Min&lt;/font&gt;&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Grayscale Band Selection</source>
        <translation type="obsolete">Pelēktoņu kanāla izvēle</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>RGB Mode Band Selection</source>
        <translation type="obsolete">RGB režīma kanālu izvēle</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&lt;b&gt;&lt;font color=&apos;blue&apos;&gt;Blue&lt;/font&gt;&lt;/b&gt;</source>
        <translation type="obsolete">&lt;b&gt;&lt;font color=&apos;blue&apos;&gt;Zils&lt;/font&gt;&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&lt;b&gt;&lt;font color=&apos;green&apos;&gt;Green&lt;/font&gt;&lt;/b&gt;</source>
        <translation type="obsolete">&lt;b&gt;&lt;font color=&apos;green&apos;&gt;Zaļš&lt;/font&gt;&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&lt;b&gt;&lt;font color=&apos;red&apos;&gt;Red&lt;/font&gt;&lt;/b&gt;</source>
        <translation type="obsolete">&lt;b&gt;&lt;font color=&apos;red&apos;&gt;Sarkans&lt;/font&gt;&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Global Transparency</source>
        <translation type="obsolete">Globālais caurspīdīgums</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="861"/>
        <source> 00%</source>
        <translation> 00%</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="38"/>
        <source>Render as</source>
        <translation>Renderēt kā</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Single Band Gray</source>
        <translation type="obsolete">Vienkalāla pelēktoņu</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Three Band Color</source>
        <translation type="obsolete">Trīskanālu krāsu</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Transparent Pixels</source>
        <translation type="obsolete">Caurspīdīgie pikseļi</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Transparent Band:</source>
        <translation type="obsolete">Caurspīdīguma kanāls:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Custom Transparency List</source>
        <translation type="obsolete">Pielāgots caurspīdīguma saraksts</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Transparency Layer;</source>
        <translation type="obsolete">Caurspīdīguma slānis;</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Add Values Manually</source>
        <translation type="obsolete">Pievienot vērtības manuāli</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1098"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Add Values From Display</source>
        <translation type="obsolete">Pievienot vērtības no ekrāna</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Remove Selected Row</source>
        <translation type="obsolete">Aizvākt izvēlēto rindu</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Default Values</source>
        <translation type="obsolete">Noklusējuma vērtības</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Import From File</source>
        <translation type="obsolete">Importēt no faila</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Export To File</source>
        <translation type="obsolete">Eksportēt uz failu</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>No Data Value:</source>
        <translation type="obsolete">Nav datu vērtība:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Reset No Data Value</source>
        <translation type="obsolete">Atiestatīt nav datu vērtību</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1127"/>
        <source>Colormap</source>
        <translation>Krāsu karte</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Number of entries:</source>
        <translation type="obsolete">Ierakstu skaits:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1217"/>
        <source>Delete entry</source>
        <translation>Dzēst ierakstu</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1224"/>
        <source>Classify</source>
        <translation>Klasificēt</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1240"/>
        <source>1</source>
        <translation>1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1245"/>
        <source>2</source>
        <translation>2</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Color interpolation:</source>
        <translation type="obsolete">Krāsu interpolācija:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Classification mode:</source>
        <translation type="obsolete">Klasificēšanas režīms:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="44"/>
        <source>Single band gray</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="51"/>
        <source>Three band color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="86"/>
        <source>RGB mode band selection and scaling</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="92"/>
        <source>Red band</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="124"/>
        <source>Green band</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="156"/>
        <source>Blue band</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="533"/>
        <source>Custom min / max values</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="201"/>
        <source>Red min</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="239"/>
        <source>Red max</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="277"/>
        <source>Green min</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="315"/>
        <source>Green max</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="353"/>
        <source>Blue min</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="391"/>
        <source>Blue max</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="423"/>
        <source>Std. deviation</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="462"/>
        <source>Single band properties</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="474"/>
        <source>Gray band</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="497"/>
        <source>Color map</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="526"/>
        <source>Invert color map</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="616"/>
        <source>Use standard deviation</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="643"/>
        <source>Note:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="670"/>
        <source>Load min / max values from band</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="676"/>
        <source>Estimate (faster)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="696"/>
        <source>Actual (slower)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="716"/>
        <source>Load</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="726"/>
        <source>Contrast enhancement</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="747"/>
        <source>Current</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="770"/>
        <source>Save current contrast enhancement algorithm as default. This setting will be persistent between QGIS sessions.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="773"/>
        <source>Saves current contrast enhancement algorithm as a default. This setting will be persistent between QGIS sessions.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="786"/>
        <source>Default</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="793"/>
        <source>TextLabel</source>
        <translation type="unfinished">TekstaBirka</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="804"/>
        <source>Transparency</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="810"/>
        <source>Global transparency</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="891"/>
        <source>No data value</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="900"/>
        <source>Reset no data value</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="910"/>
        <source>Custom transparency options</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="916"/>
        <source>Transparency band</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="968"/>
        <source>Transparent pixel list</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1014"/>
        <source>Add values manually</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1030"/>
        <source>Add Values from display</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1043"/>
        <source>Remove selected row</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1056"/>
        <source>Default values</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1082"/>
        <source>Import from file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1095"/>
        <source>Export to file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1185"/>
        <source>Number of entries</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1267"/>
        <source>Color interpolation</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1293"/>
        <source>Classification mode</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1324"/>
        <source>Spatial reference system</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1359"/>
        <source>Scale dependent visibility</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1393"/>
        <source>Maximum</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1416"/>
        <source>Minimum</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1429"/>
        <source>Show debug info</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1473"/>
        <source>Layer source</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1493"/>
        <source>Display name</source>
        <translation type="unfinished">Attēlošanas nosaukums</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1713"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1723"/>
        <source>Pyramid resolutions</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1779"/>
        <source>Resampling method</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1803"/>
        <source>Build pyramids</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1880"/>
        <source>Line graph</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1890"/>
        <source>Bar chart</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1925"/>
        <source>Column count</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1932"/>
        <source>Out of range OK?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1939"/>
        <source>Allow approximation</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="2002"/>
        <source>Restore Default Style</source>
        <translation type="unfinished">Atjaunot noklusējuma stilu</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="2009"/>
        <source>Save As Default</source>
        <translation type="unfinished">Saglabāt kā noklusējumu</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="2016"/>
        <source>Load Style ...</source>
        <translation type="unfinished">Ielādēt stilu...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="2023"/>
        <source>Save Style ...</source>
        <translation type="unfinished">Saglabāt stilu...</translation>
    </message>
</context>
<context>
    <name>QgsRunProcess</name>
    <message>
        <location filename="../src/core/qgsrunprocess.cpp" line="146"/>
        <source>Unable to run command</source>
        <translation>Nav iespējams darbināt komandu</translation>
    </message>
    <message>
        <location filename="../src/core/qgsrunprocess.cpp" line="59"/>
        <source>Starting</source>
        <translation>Startē</translation>
    </message>
    <message>
        <location filename="../src/core/qgsrunprocess.cpp" line="115"/>
        <source>Done</source>
        <translation>Darīts</translation>
    </message>
</context>
<context>
    <name>QgsSOSPlugin</name>
    <message>
        <location filename="../src/plugins/sos/qgssosplugin.cpp" line="46"/>
        <source>&amp;Add Sensor layer</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsSOSSourceSelect</name>
    <message>
        <location filename="../src/plugins/sos/qgssossourceselect.cpp" line="92"/>
        <source>Offering not found</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/sos/qgssossourceselect.cpp" line="92"/>
        <source>An problem occured adding the layer. The information about the selected offering could not be found</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/sos/qgssossourceselect.cpp" line="219"/>
        <source>Are you sure you want to remove the </source>
        <translation type="unfinished">Vai Jūs tiešām vēlaties aizvākt </translation>
    </message>
    <message>
        <location filename="../src/plugins/sos/qgssossourceselect.cpp" line="219"/>
        <source> connection and all associated settings?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/sos/qgssossourceselect.cpp" line="220"/>
        <source>Confirm Delete</source>
        <translation type="unfinished">Apstiprināt dzēšanu</translation>
    </message>
</context>
<context>
    <name>QgsSOSSourceSelectBase</name>
    <message>
        <location filename="../src/plugins/sos/qgssossourceselectbase.ui" line="13"/>
        <source>Dialog</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/sos/qgssossourceselectbase.ui" line="25"/>
        <source>Server Connections</source>
        <translation type="unfinished">Savienojumi ar serveriem</translation>
    </message>
    <message>
        <location filename="../src/plugins/sos/qgssossourceselectbase.ui" line="49"/>
        <source>&amp;New</source>
        <translation type="unfinished">&amp;Jauns</translation>
    </message>
    <message>
        <location filename="../src/plugins/sos/qgssossourceselectbase.ui" line="59"/>
        <source>Delete</source>
        <translation type="unfinished">Dzēst</translation>
    </message>
    <message>
        <location filename="../src/plugins/sos/qgssossourceselectbase.ui" line="69"/>
        <source>Edit</source>
        <translation type="unfinished">Rediģēt</translation>
    </message>
    <message>
        <location filename="../src/plugins/sos/qgssossourceselectbase.ui" line="95"/>
        <source>C&amp;onnect</source>
        <translation type="unfinished">&amp;Pieslēgties</translation>
    </message>
    <message>
        <location filename="../src/plugins/sos/qgssossourceselectbase.ui" line="108"/>
        <source>Offerings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/sos/qgssossourceselectbase.ui" line="118"/>
        <source>Name</source>
        <translation type="unfinished">Nosaukums</translation>
    </message>
    <message>
        <location filename="../src/plugins/sos/qgssossourceselectbase.ui" line="123"/>
        <source>Id</source>
        <translation type="unfinished">Id</translation>
    </message>
    <message>
        <location filename="../src/plugins/sos/qgssossourceselectbase.ui" line="134"/>
        <source>Optional settings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/sos/qgssossourceselectbase.ui" line="142"/>
        <source>Observed properties...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/sos/qgssossourceselectbase.ui" line="162"/>
        <source>Procedures...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/sos/qgssossourceselectbase.ui" line="182"/>
        <source>Features of interest...</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsScaleBarPlugin</name>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="164"/>
        <source> metres/km</source>
        <translation> metri/km</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="281"/>
        <source> feet</source>
        <translation> pēdas</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="288"/>
        <source> degrees</source>
        <translation> grādi</translation>
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
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="277"/>
        <source> foot</source>
        <translation> pēdas</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="286"/>
        <source> degree</source>
        <translation> grādi</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="291"/>
        <source> unknown</source>
        <translation> nezināms</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="79"/>
        <source>Top Left</source>
        <translation>Augšējais kreisais</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="78"/>
        <source>Bottom Left</source>
        <translation>Apakšējais kreisais</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="79"/>
        <source>Top Right</source>
        <translation>Augšējais labais</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="79"/>
        <source>Bottom Right</source>
        <translation>Apakšējais labais</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="81"/>
        <source>Tick Down</source>
        <translation>Ragi uz leju</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="82"/>
        <source>Tick Up</source>
        <translation>Ragi uz augšu</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="82"/>
        <source>Bar</source>
        <translation>Josla</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="82"/>
        <source>Box</source>
        <translation>Kaste</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="543"/>
        <source>&amp;Decorations</source>
        <translation>&amp;Dekorācijas</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="103"/>
        <source>Creates a scale bar that is displayed on the map canvas</source>
        <translation>Izveido mērogjoslu attēlošanai uz kartes</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="102"/>
        <source>&amp;Scale Bar</source>
        <translation>&amp;Mērogjosla</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="165"/>
        <source> feet/miles</source>
        <translation> pēdas/jūdzes</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="262"/>
        <source> miles</source>
        <translation> jūdzes</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="267"/>
        <source> mile</source>
        <translation> jūdze</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="272"/>
        <source> inches</source>
        <translation> collas</translation>
    </message>
</context>
<context>
    <name>QgsScaleBarPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="300"/>
        <source>Scale Bar Plugin</source>
        <translation>Mērogjoslas spraudnis</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="143"/>
        <source>Top Left</source>
        <translation>Augšējais kreisais</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="148"/>
        <source>Top Right</source>
        <translation>Augšējais labais</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="153"/>
        <source>Bottom Left</source>
        <translation>Apakšējais kreisais</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="158"/>
        <source>Bottom Right</source>
        <translation>Apakšējais labais</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="94"/>
        <source>Size of bar:</source>
        <translation>Joslas izmērs:</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="254"/>
        <source>Placement:</source>
        <translation>Novietojums:</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="211"/>
        <source>Tick Down</source>
        <translation>Ragi uz leju</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="216"/>
        <source>Tick Up</source>
        <translation>Ragi uz augšu</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="221"/>
        <source>Box</source>
        <translation>Kaste</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="226"/>
        <source>Bar</source>
        <translation>Josla</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="207"/>
        <source>Select the style of the scale bar</source>
        <translation>Izvēlieties mērogjoslas stilu</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="127"/>
        <source>Colour of bar:</source>
        <translation>Joslas krāsa:</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="192"/>
        <source>Scale bar style:</source>
        <translation>Mērogjoslas stils:</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="174"/>
        <source>Enable scale bar</source>
        <translation>Ieslēgt mērogjoslu</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="109"/>
        <source>Automatically snap to round number on resize</source>
        <translation>Automātiski noapaļot uz veseliem skaitļiem pie izmēra maiņas</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="76"/>
        <source>Click to select the colour</source>
        <translation>Klikšķiniet, lai izvēlētos krāsu</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="274"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;This plugin draws a scale bar on the map. Please note the size option below is a &apos;preferred&apos; size and may have to be altered by QGIS depending on the level of zoom.  The size is measured according to the map units specified in the project properties.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Šis spraudnis uzzīmēs uz kartes mērogjoslu. Ievērojiet, ka zemāk norādītais izmērs ir &apos;ieteicamais&apos; izmērs un QGIS var ti izmainīt atkarībā no mēroga. Izmērs ir kartes vienībās, kas ir definētas kā projekta īpašības.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
</context>
<context>
    <name>QgsSearchQueryBuilder</name>
    <message numerus="yes">
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="171"/>
        <source>Found %d matching features.</source>
        <translation type="unfinished">
            <numerusform>Atrasti %d atbilstoši objekti.</numerusform>
        </translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="173"/>
        <source>No matching features found.</source>
        <translation>Neviens atbilstošs objekts netika atrasts.</translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="174"/>
        <source>Search results</source>
        <translation>Meklēšanas rezultāti</translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="183"/>
        <source>Search string parsing error</source>
        <translation>Meklēšanas virknes parsēšanas kļūda</translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="239"/>
        <source>No Records</source>
        <translation>Nav ierakstu</translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="239"/>
        <source>The query you specified results in zero records being returned.</source>
        <translation>Norādītais pieprasījums neatgrieza neivenu rezultātu.</translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="41"/>
        <source>Search query builder</source>
        <translation>Meklēšanas pieprasījuma veidotājs</translation>
    </message>
</context>
<context>
    <name>QgsServerSourceSelect</name>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="173"/>
        <source>Are you sure you want to remove the </source>
        <translation>Vai Jūs tiešām vēlaties aizvākt </translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="173"/>
        <source> connection and all associated settings?</source>
        <translation> savienojumu un visus ar to saistītos parametrus?</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="174"/>
        <source>Confirm Delete</source>
        <translation>Apstiprināt dzēšanu</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="399"/>
        <source>WMS Provider</source>
        <translation>WMS sniedzējs</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="401"/>
        <source>Could not open the WMS Provider</source>
        <translation>Nebija iespējams atvērt WMS datu sniedzēju</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="410"/>
        <source>Select Layer</source>
        <translation>Izvēlieties slāni</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="410"/>
        <source>You must select at least one layer first.</source>
        <translation>Jums vispirms ir jāizvēlas vismaz viens slānis.</translation>
    </message>
    <message numerus="yes">
        <location filename="../src/app/qgsserversourceselect.cpp" line="520"/>
        <source>Coordinate Reference System (%1 available)</source>
        <translation type="unfinished">
            <numerusform>Koordinātu atskaites sistēma (%1 pieejamas)</numerusform>
        </translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="679"/>
        <source>Could not understand the response.  The</source>
        <translation>Nebija iespējams saprast atbildi.  Datu</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="680"/>
        <source>provider said</source>
        <translation>sniedzējs atbildēja</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="731"/>
        <source>WMS proxies</source>
        <translation>WMS starpniekserveri</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>&lt;p&gt;Several WMS servers have been added to the server list. Note that the proxy fields have been left blank and if you access the internet via a web proxy, you will need to individually set the proxy fields with appropriate values.&lt;/p&gt;</source>
        <translation type="obsolete">&lt;p&gt;Sarakstam ir pievienoti vairāki WMS serveri. Ievērojiet, ka starpniekserveru lauki ir atstāti tukši. Ja piekļūšanai internetam Jums ir nepieciešams izmantot starpniekservera pakalpojumus, aizpildiet atbilstošos starpniekservera konfigurācijas laukus.&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="414"/>
        <source>Coordinate Reference System</source>
        <translation>Koordinātu atskaites sistēma</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="414"/>
        <source>There are no available coordinate reference system for the set of layers you&apos;ve selected.</source>
        <translation>Izvēlētajiem slāņiem koordinātu atskaites stitēma nav pieejama.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="734"/>
        <source>Several WMS servers have been added to the server list. Note that if you access the internet via a web proxy, you will need to set the proxy settings in the QGIS options dialog.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsServerSourceSelectBase</name>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="13"/>
        <source>Add Layer(s) from a Server</source>
        <translation>Pievienot slāņus no servera</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="309"/>
        <source>C&amp;lose</source>
        <translation>&amp;Aizvērt</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="312"/>
        <source>Alt+L</source>
        <translation>Alt+L</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="296"/>
        <source>Help</source>
        <translation>Palīdzība</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="299"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="270"/>
        <source>Image encoding</source>
        <translation>Attēla kodējums</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="203"/>
        <source>Layers</source>
        <translation>Slāņi</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="230"/>
        <source>ID</source>
        <translation>ID</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="235"/>
        <source>Name</source>
        <translation>Nosaukums</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="240"/>
        <source>Title</source>
        <translation>Virsraksts</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="245"/>
        <source>Abstract</source>
        <translation>Kopsavilkums</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="187"/>
        <source>&amp;Add</source>
        <translation>&amp;Pievienot</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="190"/>
        <source>Alt+A</source>
        <translation>Alt+A</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="34"/>
        <source>Server Connections</source>
        <translation>Savienojumi ar serveriem</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="108"/>
        <source>&amp;New</source>
        <translation>&amp;Jauns</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="101"/>
        <source>Delete</source>
        <translation>Dzēst</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="91"/>
        <source>Edit</source>
        <translation>Rediģēt</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="81"/>
        <source>C&amp;onnect</source>
        <translation>&amp;Pieslēgties</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="174"/>
        <source>Ready</source>
        <translation>Gatavs</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="118"/>
        <source>Coordinate Reference System</source>
        <translation>Koordinātu atskaites sistēma</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="156"/>
        <source>Change ...</source>
        <translation>Mainīt...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="46"/>
        <source>Adds a few example WMS servers</source>
        <translation>Pievieno dažus WMS parauga serverus</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="52"/>
        <source>Add default servers</source>
        <translation>Pievienot noklusējuma serverus</translation>
    </message>
</context>
<context>
    <name>QgsShapeFile</name>
    <message>
        <location filename="../src/plugins/spit/qgsshapefile.cpp" line="419"/>
        <source>The database gave an error while executing this SQL:</source>
        <translation>Datu bāze atbildēja ar kļūdu mēģinot izpildīt sekojošu SQL:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsshapefile.cpp" line="427"/>
        <source>The error was:</source>
        <translation>Kļūda bija:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsshapefile.cpp" line="424"/>
        <source>... (rest of SQL trimmed)</source>
        <comment>is appended to a truncated SQL statement</comment>
        <translation>... (pārējā SQL daļa netiek rādīta)</translation>
    </message>
</context>
<context>
    <name>QgsSingleSymbolDialog</name>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="121"/>
        <source>Solid Line</source>
        <translation>Vienlaidus līnija</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="122"/>
        <source>Dash Line</source>
        <translation>Pārtraukta līnija</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="123"/>
        <source>Dot Line</source>
        <translation>Punktēta līnija</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="124"/>
        <source>Dash Dot Line</source>
        <translation>Pārtraukti punktēta līnija</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="125"/>
        <source>Dash Dot Dot Line</source>
        <translation>Pārtraukti punktēti pārtraukta līnija</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="126"/>
        <source>No Pen</source>
        <translation>Nezīmēt</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Solid Pattern</source>
        <translation type="obsolete">Vienlaidus aizpildījums</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Hor Pattern</source>
        <translation type="obsolete">Horiz aizpildījums</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Ver Pattern</source>
        <translation type="obsolete">Vert aizpildījums</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Cross Pattern</source>
        <translation type="obsolete">Krustisks aizpildījums</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Diag Cross Pattern</source>
        <translation type="obsolete">Diognāli krustisks aizpildījums</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Dense1 Pattern</source>
        <translation type="obsolete">Blīvuma 1 aizpildījums</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Dense2 Pattern</source>
        <translation type="obsolete">Blīvuma 2 aizpildījums</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Dense3 Pattern</source>
        <translation type="obsolete">Blīvuma 3 aizpildījums</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Dense4 Pattern</source>
        <translation type="obsolete">Blīvuma 4 aizpildījums</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Dense5 Pattern</source>
        <translation type="obsolete">Blīvuma 5 aizpildījums</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Dense6 Pattern</source>
        <translation type="obsolete">Blīvuma 6 aizpildījums</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Dense7 Pattern</source>
        <translation type="obsolete">Blīvuma 7 aizpildījums</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="145"/>
        <source>No Brush</source>
        <translation>Bez aizpildījuma</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Texture Pattern</source>
        <translation type="obsolete">Tekstūras aizpildījums</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="131"/>
        <source>Solid</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="132"/>
        <source>Horizontal</source>
        <translation type="unfinished">Horizontāls</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="133"/>
        <source>Vertical</source>
        <translation type="unfinished">Vertikāls</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="134"/>
        <source>Cross</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="135"/>
        <source>BDiagonal</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="136"/>
        <source>FDiagonal</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="137"/>
        <source>Diagonal X</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="138"/>
        <source>Dense1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="139"/>
        <source>Dense2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="140"/>
        <source>Dense3</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="141"/>
        <source>Dense4</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="142"/>
        <source>Dense5</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="143"/>
        <source>Dense6</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="144"/>
        <source>Dense7</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="146"/>
        <source>Texture</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsSingleSymbolDialogBase</name>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="19"/>
        <source>Single Symbol</source>
        <translation>Atsevišķs simbols</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="95"/>
        <source>Size</source>
        <translation>Izmērs</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="341"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="73"/>
        <source>Point Symbol</source>
        <translation>Punkta simbols</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="105"/>
        <source>Area scale field</source>
        <translation>Laukuma mērogošanas lauks</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="115"/>
        <source>Rotation field</source>
        <translation>Rotācijas lauks</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="182"/>
        <source>Style Options</source>
        <translation>Stila opcijas</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="188"/>
        <source>Outline style</source>
        <translation>Līnijas stils</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="220"/>
        <source>Outline color</source>
        <translation>Līnijas krāsa</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="255"/>
        <source>Outline width</source>
        <translation>Līnijas platums</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="278"/>
        <source>Fill color</source>
        <translation>Aizpildījuma krāsa</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="313"/>
        <source>Fill style</source>
        <translation>Aizpildījuma stils</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="45"/>
        <source>Label</source>
        <translation>Birka</translation>
    </message>
</context>
<context>
    <name>QgsSnappingDialog</name>
    <message>
        <location filename="../src/app/qgssnappingdialog.cpp" line="147"/>
        <source>to vertex</source>
        <translation>uz virsotni</translation>
    </message>
    <message>
        <location filename="../src/app/qgssnappingdialog.cpp" line="151"/>
        <source>to segment</source>
        <translation>uz segmentu</translation>
    </message>
    <message>
        <location filename="../src/app/qgssnappingdialog.cpp" line="89"/>
        <source>to vertex and segment</source>
        <translation>uz virsotni un segmentu</translation>
    </message>
</context>
<context>
    <name>QgsSnappingDialogBase</name>
    <message>
        <location filename="../src/ui/qgssnappingdialogbase.ui" line="13"/>
        <source>Snapping options</source>
        <translation>Pielipšanas opcijas</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssnappingdialogbase.ui" line="26"/>
        <source>Layer</source>
        <translation>Slānis</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssnappingdialogbase.ui" line="31"/>
        <source>Mode</source>
        <translation>Režīms</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssnappingdialogbase.ui" line="36"/>
        <source>Tolerance</source>
        <translation>Tolerance</translation>
    </message>
</context>
<context>
    <name>QgsSpit</name>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="148"/>
        <source>Are you sure you want to remove the [</source>
        <translation>Vai Jūs tiešām vēlaties aizvākt [</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="148"/>
        <source>] connection and all associated settings?</source>
        <translation>] savienojumu un visus ar to saistītos parametrus?</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="149"/>
        <source>Confirm Delete</source>
        <translation>Apstiprināt dzēšanu</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="268"/>
        <source>The following Shapefile(s) could not be loaded:

</source>
        <translation>Sekojošo šeipfailu nevarēju ielādēt:
</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="272"/>
        <source>REASON: File cannot be opened</source>
        <translation>Iemesls: Failu nevar atvērt</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="277"/>
        <source>REASON: One or both of the Shapefile files (*.dbf, *.shx) missing</source>
        <translation>IEMESLS: Trūkst viens vai abi šeipfaila palīgfaili (*.dbf, *.shx)</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="354"/>
        <source>General Interface Help:</source>
        <translation>Vispārēja saskarnes palīdzība:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="356"/>
        <source>PostgreSQL Connections:</source>
        <translation>PostgreSQL savienojumi:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="358"/>
        <source>[New ...] - create a new connection</source>
        <translation>[Jauns ...] - izveido jaunu savienojumu</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="359"/>
        <source>[Edit ...] - edit the currently selected connection</source>
        <translation>[Labot ...] - labo jau eksistējošu savienojumu</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="360"/>
        <source>[Remove] - remove the currently selected connection</source>
        <translation>[Aizvākt] - aizvāc jau eksistējošu savienojumu</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="361"/>
        <source>-you need to select a connection that works (connects properly) in order to import files</source>
        <translation>- Jums ir nepieciešams strādājošs savienojums, lai varētu importēt failus</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="362"/>
        <source>-when changing connections Global Schema also changes accordingly</source>
        <translation> - ja maināt savienojumu, globālā shēma arī tiek mainīta</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="363"/>
        <source>Shapefile List:</source>
        <translation>Šeipfailu saraksts:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="365"/>
        <source>[Add ...] - open a File dialog and browse to the desired file(s) to import</source>
        <translation>[Pievienot ...] - atver failu izvēles dialogu un ļauj sameklēt importējamo(s) failu(s)</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="366"/>
        <source>[Remove] - remove the currently selected file(s) from the list</source>
        <translation>[Noņemt] - noņem izvlēlētos failus no saraksta</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="367"/>
        <source>[Remove All] - remove all the files in the list</source>
        <translation>[Noņemt visus] - noņem visus failus no saraksta</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="368"/>
        <source>[SRID] - Reference ID for the shapefiles to be imported</source>
        <translation>[SRID] - Atskaites ID importējamajam šeipfailam</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="369"/>
        <source>[Use Default (SRID)] - set SRID to -1</source>
        <translation>[Lietot noklusējuma (SRID)] - iestata SRID uz -1</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="370"/>
        <source>[Geometry Column Name] - name of the geometry column in the database</source>
        <translation>[Ģeometrijas kolonnas nosaukums] - iestata ģeometrijas kolonnas nosaukumu</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="371"/>
        <source>[Use Default (Geometry Column Name)] - set column name to &apos;the_geom&apos;</source>
        <translation>[Lietot noklusējuma (ģeometrijas kolonnas nosaukumu)] - iestata kolonnas nosaukumu uz &apos;the_geom&apos;</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="372"/>
        <source>[Glogal Schema] - set the schema for all files to be imported into</source>
        <translation>[Globālā shēma] - iestata shēmu visiem importējamajiem failiem</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="374"/>
        <source>[Import] - import the current shapefiles in the list</source>
        <translation>[Importēt] - importē saraksta aktīvo šeipfailu</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="375"/>
        <source>[Quit] - quit the program
</source>
        <translation>[Quit] - iziet no programmas
</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="376"/>
        <source>[Help] - display this help dialog</source>
        <translation>[Help] - Parāda šo informāciju</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="833"/>
        <source>Import Shapefiles</source>
        <translation>Importēt šeipfailus</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="833"/>
        <source>You need to specify a Connection first</source>
        <translation>Jums vispirms jānorāda savienojums</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="428"/>
        <source>Connection failed - Check settings and try again</source>
        <translation>Savienojums neizdevās - pārbaudiet konfigurāciju</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="513"/>
        <source>You need to add shapefiles to the list first</source>
        <translation>Jums vispirms jāpievieno šeipfaili sarakstam</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="580"/>
        <source>Importing files</source>
        <translation>Importēju failus</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="518"/>
        <source>Cancel</source>
        <translation>Atcelt</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="522"/>
        <source>Progress</source>
        <translation>Progress</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="531"/>
        <source>Problem inserting features from file:</source>
        <translation>Problēma ievietojot objektus no faila:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="538"/>
        <source>Invalid table name.</source>
        <translation>Nepareizs tabulas nosaukums.</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="548"/>
        <source>No fields detected.</source>
        <translation>Nav izvēlēti lauki.</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="573"/>
        <source>The following fields are duplicates:</source>
        <translation>Sekojoši lauki ir duplikāti:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="672"/>
        <source>Import Shapefiles - Relation Exists</source>
        <translation>Importēt šeipfailu - sakarība eksistē</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="673"/>
        <source>The Shapefile:</source>
        <translation>Šeipfails:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="674"/>
        <source>will use [</source>
        <translation>izmantos [ </translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="674"/>
        <source>] relation for its data,</source>
        <translation>] relāciju datiem,</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="674"/>
        <source>which already exists and possibly contains data.</source>
        <translation> kas jau eksistē un, iespējams, satur datus. </translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="675"/>
        <source>To avoid data loss change the &quot;DB Relation Name&quot;</source>
        <translation>Lai novērstu datu zudumu, nomainiet &quot;DB relācijas nosaukumu&quot;</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="675"/>
        <source>for this Shapefile in the main dialog file list.</source>
        <translation>šim šeipfailam galvenajā failu dialogā.</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="676"/>
        <source>Do you want to overwrite the [</source>
        <translation>Vai Jūs vēlaties pārrakstīt [ </translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="676"/>
        <source>] relation?</source>
        <translation> ] relāciju?</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="70"/>
        <source>File Name</source>
        <translation>Faila nosaukums</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="70"/>
        <source>Feature Class</source>
        <translation>Objektu klase</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="71"/>
        <source>Features</source>
        <translation>Objekti</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="71"/>
        <source>DB Relation Name</source>
        <translation>DB relācijas nosaukums</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="71"/>
        <source>Schema</source>
        <translation>Shēma</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>New Connection</source>
        <translation type="obsolete">Jauns savienojums</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="172"/>
        <source>Add Shapefiles</source>
        <translation>Pievienot shapefile</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="174"/>
        <source>Shapefiles (*.shp);;All files (*.*)</source>
        <translation>Shapefiles (*.shp);;Visi faili (*.*)</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="467"/>
        <source>PostGIS not available</source>
        <translation>PostGIS nav pieejams</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="469"/>
        <source>&lt;p&gt;The chosen database does not have PostGIS installed, but this is required for storage of spatial data.&lt;/p&gt;</source>
        <translation>&lt;p&gt;Izvēlētajai datu bāzei nav instalēts PostGIS atbalsts, taču tas ir nepieciešams telpisko datu glabāšanai.&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="564"/>
        <source>Checking to see if </source>
        <translation>Pārbaudu vai </translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="814"/>
        <source>&lt;p&gt;Error while executing the SQL:&lt;/p&gt;&lt;p&gt;</source>
        <translation>&lt;p&gt;Kļūda izpildot SQL:&lt;/p&gt;&lt;p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="815"/>
        <source>&lt;/p&gt;&lt;p&gt;The database said:</source>
        <translation>&lt;/p&gt;&lt;p&gt;Datu bāze atbildēja:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="829"/>
        <source>%1 of %2 shapefiles could not be imported.</source>
        <translation>%1 no %2 šeipfailiem nebija iespējams importēt.</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="408"/>
        <source>Password for </source>
        <translation type="unfinished">Parole priekš</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="409"/>
        <source>Please enter your password:</source>
        <translation type="unfinished">Lūdzu, ievadiet paroli:</translation>
    </message>
</context>
<context>
    <name>QgsSpitBase</name>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="19"/>
        <source>SPIT - Shapefile to PostGIS Import Tool</source>
        <translation>SPIT - Rīks datu pārnešanai no šeipfaila uz PostGIS</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="175"/>
        <source>Add</source>
        <translation>Pievienot</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="172"/>
        <source>Add a shapefile to the list of files to be imported</source>
        <translation>Pievienot šeipfailu importējamo failu sarakstam</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="188"/>
        <source>Remove</source>
        <translation>Noņemt</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="185"/>
        <source>Remove the selected shapefile from the import list</source>
        <translation>Noņemt izvēlēto šeipfailu no importējamo failu saraksta</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="201"/>
        <source>Remove All</source>
        <translation>Noņemt visu</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="198"/>
        <source>Remove all the shapefiles from the import list</source>
        <translation>Noņemt visus šeipfailus no importējamo failu saraksta</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="273"/>
        <source>Global Schema</source>
        <translation>Globālā shēma</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="224"/>
        <source>Set the SRID to the default value</source>
        <translation>Iestatīt SRID uz noklusējuma vērtību</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="237"/>
        <source>Set the geometry column name to the default value</source>
        <translation>Iestatīt ģeometrijas kolonnas nosaukumu uz noklusējuma vērtību</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="54"/>
        <source>PostgreSQL Connections</source>
        <translation>PostgreSQL savienojumi</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="121"/>
        <source>New</source>
        <translation>Jauns</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="118"/>
        <source>Create a new PostGIS connection</source>
        <translation>Izveidot jaunu PostGIS savienojumu</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="105"/>
        <source>Remove the current PostGIS connection</source>
        <translation>Noņemt pašreizejo PostGIS savienojumu</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="95"/>
        <source>Edit</source>
        <translation>Rediģēt</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="92"/>
        <source>Edit the current PostGIS connection</source>
        <translation>Labot pašreizējo PostGIS savienojumu</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="144"/>
        <source>Import options and shapefile list</source>
        <translation>Importēšanas opcijas un šeipfailu saraksts</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="227"/>
        <source>Use Default SRID or specify here</source>
        <translation>Lietot noklusējuma SRID vai norādīt šeit</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="240"/>
        <source>Use Default Geometry Column Name or specify here</source>
        <translation>Lietot noklusējuma ģeometrijas kolonnu vai norādīt šeit</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="263"/>
        <source>Primary Key Column Name</source>
        <translation>Primārās atslēgas kolonnas nosaukums</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="131"/>
        <source>Connect to PostGIS</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="134"/>
        <source>Connect</source>
        <translation type="unfinished">Pieslēgties</translation>
    </message>
</context>
<context>
    <name>QgsSpitPlugin</name>
    <message>
        <location filename="../src/plugins/spit/qgsspitplugin.cpp" line="68"/>
        <source>&amp;Import Shapefiles to PostgreSQL</source>
        <translation>&amp;Importēt shapfile datus uz PostgreSQL</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitplugin.cpp" line="70"/>
        <source>Import shapefiles into a PostGIS-enabled PostgreSQL database. The schema and field names can be customized on import</source>
        <translation>Importē shapefile formāta datus uz PostGIS datu bāzi PostgreSQL. Izmantotā shēma un lauku nosaukumi var tikt pielāgoti importēšanas laikā</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitplugin.cpp" line="93"/>
        <source>&amp;Spit</source>
        <translation>&amp;Spit</translation>
    </message>
</context>
<context>
    <name>QgsTINInterpolatorDialog</name>
    <message>
        <location filename="../src/plugins/interpolation/qgstininterpolatordialog.cpp" line="25"/>
        <source>Linear interpolation</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsTINInterpolatorDialogBase</name>
    <message>
        <location filename="../src/plugins/interpolation/qgstininterpolatordialogbase.ui" line="13"/>
        <source>Triangle based interpolation</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgstininterpolatordialogbase.ui" line="19"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:12pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;This interpolator provides different methods for interpolation in a triangular irregular network (TIN).&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgstininterpolatordialogbase.ui" line="31"/>
        <source>Interpolation method:</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsUniqueValueDialog</name>
    <message>
        <location filename="../src/app/qgsuniquevaluedialog.cpp" line="282"/>
        <source>Confirm Delete</source>
        <translation type="unfinished">Apstiprināt dzēšanu</translation>
    </message>
    <message>
        <location filename="../src/app/qgsuniquevaluedialog.cpp" line="285"/>
        <source>The classification field was changed from &apos;%1&apos; to &apos;%2&apos;.
Should the existing classes be deleted before classification?</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsUniqueValueDialogBase</name>
    <message>
        <location filename="../src/ui/qgsuniquevaluedialogbase.ui" line="19"/>
        <source>Form1</source>
        <translation>Forma1</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Classification Field:</source>
        <translation type="obsolete">Klasifikācijas lauks:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Delete class</source>
        <translation type="obsolete">Dzēst klasi</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsuniquevaluedialogbase.ui" line="93"/>
        <source>Classify</source>
        <translation>Klasificēt</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsuniquevaluedialogbase.ui" line="49"/>
        <source>Classification field</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsuniquevaluedialogbase.ui" line="100"/>
        <source>Add class</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsuniquevaluedialogbase.ui" line="107"/>
        <source>Delete classes</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsuniquevaluedialogbase.ui" line="114"/>
        <source>Randomize Colors</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsuniquevaluedialogbase.ui" line="121"/>
        <source>Reset Colors</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsVectorLayer</name>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2470"/>
        <source>Could not commit the added features.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2561"/>
        <source>No other types of changes will be committed at this time.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2492"/>
        <source>Could not commit the changed attributes.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2551"/>
        <source>However, the added features were committed OK.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2518"/>
        <source>Could not commit the changed geometries.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2555"/>
        <source>However, the changed attributes were committed OK.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2548"/>
        <source>Could not commit the deleted features.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2559"/>
        <source>However, the changed geometries were committed OK.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsVectorLayerProperties</name>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="113"/>
        <source>Transparency: </source>
        <translation>Caurspīdīgums: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="205"/>
        <source>Single Symbol</source>
        <translation>Atsevišķs simbols</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="208"/>
        <source>Graduated Symbol</source>
        <translation>Graduēts simbols</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="209"/>
        <source>Continuous Color</source>
        <translation>Nepārtraukta krāsa</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="210"/>
        <source>Unique Value</source>
        <translation>Unikāla vērtība</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="164"/>
        <source>This button opens the PostgreSQL query builder and allows you to create a subset of features to display on the map canvas rather than displaying all features in the layer</source>
        <translation>Šī poga atver PostgreSQL vaicājumu veidotāju un atļauj izveidot attēlojamo objektu apakškopu tā vietā, lai attēlotu pilnīgi visus slāņa objektus</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="167"/>
        <source>The query used to limit the features in the layer is shown here. This is currently only supported for PostgreSQL layers. To enter or modify the query, click on the Query Builder button</source>
        <translation>Vaicājums, kas ir izmantots attēlojamo objektu izvēlei. Šī iespēja pagaitām ir pieejama tikai PostgreSQL slāņiem. Lai ievadītu vai labotu vaicājumu, nospiediet uz Vaicājumu veidotāja pogas</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="398"/>
        <source>Spatial Index</source>
        <translation>Telpiskais indekss</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="393"/>
        <source>Creation of spatial index successfull</source>
        <translation>Telpiskā indeksa izveide sekmīgi pabeigta</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="398"/>
        <source>Creation of spatial index failed</source>
        <translation>Telpiskā indeksa izvede neveiksmīga</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="411"/>
        <source>General:</source>
        <translation>Vispārējs:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="426"/>
        <source>Storage type of this layer : </source>
        <translation>Slāņa glabāšanas tips : </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="432"/>
        <source>Source for this layer : </source>
        <translation>Slāņa avots : </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="449"/>
        <source>Geometry type of the features in this layer : </source>
        <translation>Slāņa objektu ģeometrijas tips: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="457"/>
        <source>The number of features in this layer : </source>
        <translation>Objektu skaits slānī : </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="462"/>
        <source>Editing capabilities of this layer : </source>
        <translation>Slāņa labošanas iespējas : </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="469"/>
        <source>Extents:</source>
        <translation>Apjoms : </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="474"/>
        <source>In layer spatial reference system units : </source>
        <translation>Slāņa telpiskās norādes sistēmas vienībās : </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="475"/>
        <source>xMin,yMin </source>
        <translation>xMin,yMin </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="479"/>
        <source> : xMax,yMax </source>
        <translation> : xMax,yMax </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="534"/>
        <source>In project spatial reference system units : </source>
        <translation>Projekta telpiskās norādes sistēmas vienībās : </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="508"/>
        <source>Layer Spatial Reference System:</source>
        <translation>Slāņa telpisko norāžu sistēma:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="545"/>
        <source>Attribute field info:</source>
        <translation>Atribūtu lauka informācija: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="552"/>
        <source>Field</source>
        <translation>Lauks</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="555"/>
        <source>Type</source>
        <translation>Tips</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="558"/>
        <source>Length</source>
        <translation>Garums</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="561"/>
        <source>Precision</source>
        <translation>Precizitāte</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="419"/>
        <source>Layer comment: </source>
        <translation>Slāņa komentārs: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="564"/>
        <source>Comment</source>
        <translation>Komentārs</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="766"/>
        <source>Default Style</source>
        <translation>Noklusējuma stils</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="748"/>
        <source>QGIS Layer Style File (*.qml)</source>
        <translation>QGIS slāņa stila fails (*.qml)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="772"/>
        <source>QGIS</source>
        <translation>QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="773"/>
        <source>Unknown style format: </source>
        <translation>Nezināms stila formāts: </translation>
    </message>
</context>
<context>
    <name>QgsVectorLayerPropertiesBase</name>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="19"/>
        <source>Layer Properties</source>
        <translation>Slāņa īpašības</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Legend type:</source>
        <translation type="obsolete">Leģendas tips:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="38"/>
        <source>Symbology</source>
        <translation>Simboloģija</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Transparency:</source>
        <translation type="obsolete">Caurspīdīgums:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="145"/>
        <source>General</source>
        <translation>Vispārējs</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="217"/>
        <source>Use scale dependent rendering</source>
        <translation>Izmantot no mēroga atkarīgu renderēšanu</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Maximum 1:</source>
        <translation type="obsolete">Maksimālais mērogs 1:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Minimum 1:</source>
        <translation type="obsolete">Minimālais mērogs 1:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="258"/>
        <source>Minimum scale at which this layer will be displayed. </source>
        <translation>Minimālais mērogs pie kāda šis slānis tiks rādīts. </translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="271"/>
        <source>Maximum scale at which this layer will be displayed. </source>
        <translation>Maksimālais mērogs pie kāda šis slānis tiks rādīts. </translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="157"/>
        <source>Display name</source>
        <translation>Attēlošanas nosaukums</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="186"/>
        <source>Use this control to set which field is placed at the top level of the Identify Results dialog box.</source>
        <translation>Izmantojiet šo kontroli, lai noteiktu kurš no laukiem tiks rādīts kā augšējais rezultātu identificēšanas logā.</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Spatial Reference System</source>
        <translation type="obsolete">Telpisko norāžu sistēma</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Change</source>
        <translation type="obsolete">Mainīt</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="170"/>
        <source>Display field for the Identify Results dialog box</source>
        <translation>Rādīt lauku identificēto objektu dialogā</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="173"/>
        <source>This sets the display field for the Identify Results dialog box</source>
        <translation>Iestata attēlošanas lauku priekš identificēšanas dialoga</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="176"/>
        <source>Display field</source>
        <translation>Attēlošanas lauks</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="287"/>
        <source>Subset</source>
        <translation>Apakškopa</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="334"/>
        <source>Query Builder</source>
        <translation>Vaicājumu veidotājs</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Spatial Index</source>
        <translation type="obsolete">Telpiskais indekss</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="200"/>
        <source>Create Spatial Index</source>
        <translation>Izveidot telpisko indeksu</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Create</source>
        <translation type="obsolete">Izveidot</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="345"/>
        <source>Metadata</source>
        <translation>Metadati</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="374"/>
        <source>Labels</source>
        <translation>Birkas</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="386"/>
        <source>Display labels</source>
        <translation>Attēlot birkas</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="410"/>
        <source>Actions</source>
        <translation>Darbības</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="453"/>
        <source>Restore Default Style</source>
        <translation>Atjaunot noklusējuma stilu</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="460"/>
        <source>Save As Default</source>
        <translation>Saglabāt kā noklusējumu</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="467"/>
        <source>Load Style ...</source>
        <translation>Ielādēt stilu...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="474"/>
        <source>Save Style ...</source>
        <translation>Saglabāt stilu...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="56"/>
        <source>Legend type</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="94"/>
        <source>Transparency</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="151"/>
        <source>Options</source>
        <translation type="unfinished">Parametri</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="207"/>
        <source>Change SRS</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="238"/>
        <source>Maximum</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="248"/>
        <source>Minimum</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsVectorSymbologyWidgetBase</name>
    <message>
        <location filename="" line="135533324"/>
        <source>Form2</source>
        <translation type="obsolete">Forma2</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Label</source>
        <translation type="obsolete">Birka</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Min</source>
        <translation type="obsolete">Min</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Max</source>
        <translation type="obsolete">Maks</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Symbol Classes:</source>
        <translation type="obsolete">Simbolu klases:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Count:</source>
        <translation type="obsolete">Skaits:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Mode:</source>
        <translation type="obsolete">Režīms:</translation>
    </message>
    <message>
        <location filename="" line="135533324"/>
        <source>Field:</source>
        <translation type="obsolete">Lauks:</translation>
    </message>
</context>
<context>
    <name>QgsWFSPlugin</name>
    <message>
        <location filename="../src/plugins/wfs/qgswfsplugin.cpp" line="59"/>
        <source>&amp;Add WFS layer</source>
        <translation>&amp;Pievienot WFS slāni</translation>
    </message>
</context>
<context>
    <name>QgsWFSProvider</name>
    <message>
        <location filename="../src/providers/wfs/qgswfsprovider.cpp" line="1391"/>
        <source>unknown</source>
        <translation>nezināms</translation>
    </message>
    <message>
        <location filename="../src/providers/wfs/qgswfsprovider.cpp" line="1397"/>
        <source>received %1 bytes from %2</source>
        <translation>saņemti %1 baiti no %2</translation>
    </message>
</context>
<context>
    <name>QgsWFSSourceSelect</name>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselect.cpp" line="260"/>
        <source>Are you sure you want to remove the </source>
        <translation>Vai Jūs tiešām vēlaties aizvākt </translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselect.cpp" line="260"/>
        <source> connection and all associated settings?</source>
        <translation> savienojumu un visus ar to saistītos iestatījumus?</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselect.cpp" line="261"/>
        <source>Confirm Delete</source>
        <translation>Apstiprināt dzēšanu</translation>
    </message>
</context>
<context>
    <name>QgsWFSSourceSelectBase</name>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="29"/>
        <source>Title</source>
        <translation>Virsraksts</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="34"/>
        <source>Name</source>
        <translation>Nosaukums</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="39"/>
        <source>Abstract</source>
        <translation>Kopsavilkums</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="47"/>
        <source>Coordinate Reference System</source>
        <translation>Koordinātu atskaites sistēma</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="85"/>
        <source>Change ...</source>
        <translation>Mainīt...</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="95"/>
        <source>Server Connections</source>
        <translation>Savienojumi ar serveriem</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="107"/>
        <source>&amp;New</source>
        <translation>&amp;Jauns</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="117"/>
        <source>Delete</source>
        <translation>Dzēst</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="127"/>
        <source>Edit</source>
        <translation>Rediģēt</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="153"/>
        <source>C&amp;onnect</source>
        <translation>&amp;Pieslēgties</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="13"/>
        <source>Add WFS Layer from a Server</source>
        <translation>Pievienot WFS slāni no servera</translation>
    </message>
</context>
<context>
    <name>QgsWmsProvider</name>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="706"/>
        <source>Tried URL: </source>
        <translation>Mēģinātais URL: </translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="686"/>
        <source>HTTP Exception</source>
        <translation>HTTP izņēmums</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="649"/>
        <source>WMS Service Exception</source>
        <translation>WMS servisa izņēmums</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1500"/>
        <source>DOM Exception</source>
        <translation>DOM izņēmums</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="760"/>
        <source>Could not get WMS capabilities: %1 at line %2 column %3</source>
        <translation>Nebija iespējams saņemt WMS iespējas: %1 līnijā %2 kolonnā %3</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="791"/>
        <source>This is probably due to an incorrect WMS Server URL.</source>
        <translation>Tam par iemeslu, iespējams, ir kļūdains WMS servera URL.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="787"/>
        <source>Could not get WMS capabilities in the expected format (DTD): no %1 or %2 found</source>
        <translation>Nebija iespējams saņemt WMS iespējas gaidītajā formātā (DTD): %1 vai %2 nav atrasts</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1502"/>
        <source>Could not get WMS Service Exception at %1: %2 at line %3 column %4</source>
        <translation>Nebija iespējams saņemt WMS servisa izņēmumu. %1: %2 līnijā %3 kolonnā %4</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1552"/>
        <source>Request contains a Format not offered by the server.</source>
        <translation>Pieprasījums satur formātu, kādu serveris nepiedāvā.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1556"/>
        <source>Request contains a CRS not offered by the server for one or more of the Layers in the request.</source>
        <translation>Pieprasījums satur CRS, ko serveris nepiedāvā vienam vai vairākiem pieprasītajiem slāņiem.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1560"/>
        <source>Request contains a SRS not offered by the server for one or more of the Layers in the request.</source>
        <translation>Pieprasījums satur SRS, ko serveris nepiedāvā vienam vai vairākiem pieprasītajiem slāņiem.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1565"/>
        <source>GetMap request is for a Layer not offered by the server, or GetFeatureInfo request is for a Layer not shown on the map.</source>
        <translation>GetMap pieprasījums ir slānim, kuru serveris nepiedāvā, vai GetFeatureInfo pieprasījums ir slānim, kurš netiek rādīts kartē.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1569"/>
        <source>Request is for a Layer in a Style not offered by the server.</source>
        <translation>Pieprasījums ir slānim stilā, kuru serveris nepiedāvā.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1573"/>
        <source>GetFeatureInfo request is applied to a Layer which is not declared queryable.</source>
        <translation>GetFeatureInfo pieprasījums ir lietots slānim, kurš nav deklarēts kā vaicājams.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1577"/>
        <source>GetFeatureInfo request contains invalid X or Y value.</source>
        <translation>GetFeatureInfo pieprasījums satur nederīgu X vai Y vērtību.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1582"/>
        <source>Value of (optional) UpdateSequence parameter in GetCapabilities request is equal to current value of service metadata update sequence number.</source>
        <translation>Vērtība (neobligātajam) UpdateSequence parametram iekš GetCapabilities pieprasījuma ir vienāda ar pašreizējo servisa metadatu UpdateSequence numuru.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1587"/>
        <source>Value of (optional) UpdateSequence parameter in GetCapabilities request is greater than current value of service metadata update sequence number.</source>
        <translation>Vērtība (neobligātajam) UpdateSequence parametram iekš GetCapabilities pieprasījuma ir lielāka par pašreizējo servisa metadatu UpdateSequence numuru.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1592"/>
        <source>Request does not include a sample dimension value, and the server did not declare a default value for that dimension.</source>
        <translation>Pieprasījums nesatur parauga dimensijas vērtību un serveris nav deklarējis noklusējuma vērību šai dimensijai.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1596"/>
        <source>Request contains an invalid sample dimension value.</source>
        <translation>Pieprasījums satur nederīgu parauga dimensijas vērtību.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1600"/>
        <source>Request is for an optional operation that is not supported by the server.</source>
        <translation>Pieprasījums ir priekš neobligātas darbības, kuru serveris neatbalsta.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1604"/>
        <source>(Unknown error code from a post-1.3 WMS server)</source>
        <translation>(Nezināms kļūdas kods no pēc-1.3 WMS servera)</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1607"/>
        <source>The WMS vendor also reported: </source>
        <translation>WMS sniedzējs arī ziņoja: </translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1610"/>
        <source>This is probably due to a bug in the QGIS program.  Please report this error.</source>
        <translation>Iespējams, ka tas ir dēļ kļūdas QGIS programmā.  Lūdzu ziņojiet par to.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1794"/>
        <source>Server Properties:</source>
        <translation>Servera īpašības:</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1925"/>
        <source>Property</source>
        <translation>Īpašība</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1928"/>
        <source>Value</source>
        <translation>Vērtība</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1811"/>
        <source>WMS Version</source>
        <translation>WMS versija</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2057"/>
        <source>Title</source>
        <translation>Virsraksts</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2065"/>
        <source>Abstract</source>
        <translation>Kopsavilkums</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1835"/>
        <source>Keywords</source>
        <translation>Atslēgvārdi</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1843"/>
        <source>Online Resource</source>
        <translation>Tiešsaistes resurss</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1851"/>
        <source>Contact Person</source>
        <translation>Kontaktpersona</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1863"/>
        <source>Fees</source>
        <translation>Maksa</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1871"/>
        <source>Access Constraints</source>
        <translation>Pieejas ierobežojumi</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1879"/>
        <source>Image Formats</source>
        <translation>Attēla formāti</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1887"/>
        <source>Identify Formats</source>
        <translation>Identificēt formātus</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1895"/>
        <source>Layer Count</source>
        <translation>Slāņu skaits</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1915"/>
        <source>Layer Properties: </source>
        <translation>Slāņu īpašības: </translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1933"/>
        <source>Selected</source>
        <translation>Atlasīts</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1990"/>
        <source>Yes</source>
        <translation>Jā</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1990"/>
        <source>No</source>
        <translation>Nē</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1942"/>
        <source>Visibility</source>
        <translation>Redzamība</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1948"/>
        <source>Visible</source>
        <translation>Redzams</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1949"/>
        <source>Hidden</source>
        <translation>Slēpts</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1950"/>
        <source>n/a</source>
        <translation>n/p</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1971"/>
        <source>Can Identify</source>
        <translation>Var identificēt</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1979"/>
        <source>Can be Transparent</source>
        <translation>Var būt caurspīdīgs</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1987"/>
        <source>Can Zoom In</source>
        <translation>Var pietuvināt</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1995"/>
        <source>Cascade Count</source>
        <translation>Kaskāžu skaits</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2003"/>
        <source>Fixed Width</source>
        <translation>Fiksēts platums</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2011"/>
        <source>Fixed Height</source>
        <translation>Fiksēts augstums</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2019"/>
        <source>WGS 84 Bounding Box</source>
        <translation>WGS 84 ietverošais robežu taisnstūris</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2029"/>
        <source>Available in CRS</source>
        <translation>Pieejams ar CRS</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2040"/>
        <source>Available in style</source>
        <translation>Pieejams ar stilu</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2049"/>
        <source>Name</source>
        <translation>Nosaukums</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2150"/>
        <source>Layer cannot be queried.</source>
        <translation>Slānim nevar veikt vaicājumu.</translation>
    </message>
</context>
<context>
    <name>QuickPrintGui</name>
    <message>
        <location filename="../src/plugins/quick_print/quickprintgui.cpp" line="129"/>
        <source>Portable Document Format (*.pdf)</source>
        <translation>PDF (*.pdf)</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintgui.cpp" line="154"/>
        <source>quickprint</source>
        <translation>ātrdruka</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintgui.cpp" line="155"/>
        <source>Unknown format: </source>
        <translation>Nezināms formāts: </translation>
    </message>
</context>
<context>
    <name>QuickPrintGuiBase</name>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="13"/>
        <source>QGIS Quick Print Plugin</source>
        <translation>QGIS ātrās drukas spraudnis</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="158"/>
        <source>Quick Print</source>
        <translation>Ātrā druka</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="129"/>
        <source>Map Title e.g. ACME inc.</source>
        <translation>Kartes virsraksts, piem., Tūtardienu zvanītājs</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="116"/>
        <source>Map Name e.g. Water Features</source>
        <translation>Kartes nosaukums, piem., Upes</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="103"/>
        <source>Copyright</source>
        <translation>Autortiesības</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="48"/>
        <source>Output</source>
        <translation>uIzvade</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="60"/>
        <source>Use last filename but incremented.</source>
        <translation>Lietot palielinātu pēdējo faila nosaukumu.</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="67"/>
        <source>last used filename but incremented will be shown here</source>
        <translation>pēdējo reizi lietotais faila nosaukums bez palielinājuma būs parādīts šeit</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="77"/>
        <source>Prompt for file name</source>
        <translation>Vaicāt faila nosaukumu</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="38"/>
        <source>Note: If you want more control over the map layout please use the map composer function in QGIS.</source>
        <translation>Piezīme: Ja vēlaties plašāku kontroli pār kartes izkārtojumu, izmantojiet QGIS karšu salikšanas funkciju.</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="93"/>
        <source>Page Size</source>
        <translation>Lapas izmērs</translation>
    </message>
</context>
<context>
    <name>QuickPrintPlugin</name>
    <message>
        <location filename="../src/plugins/quick_print/quickprintplugin.cpp" line="75"/>
        <source>Quick Print</source>
        <translation>Ātrā druka</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintplugin.cpp" line="77"/>
        <source>Replace this with a short description of the what the plugin does</source>
        <translation>Aizstājiet šo ar īsu spraudņa darbības aprakstu</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintplugin.cpp" line="82"/>
        <source>&amp;Quick Print</source>
        <translation>Ā&amp;trā druka</translation>
    </message>
</context>
<context>
    <name>RepositoryDetailsDialog</name>
    <message>
        <location filename="../python/plugins/plugin_installer/repository.ui" line="13"/>
        <source>Repository details</source>
        <translation>Repozitorija informācija</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/repository.ui" line="19"/>
        <source>Name:</source>
        <translation>Nosaukums:</translation>
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
        <translation>QGIS spraudņa paraugs</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/pluginguibase.ui" line="47"/>
        <source>Plugin Template</source>
        <translation>Spraudņa paraugs</translation>
    </message>
</context>
<context>
    <name>pluginname</name>
    <message>
        <location filename="../src/plugins/plugin_template/plugin.cpp" line="75"/>
        <source>Replace this with a short description of the what the plugin does</source>
        <translation>Aizstājiet šo ar īsu spraudņa darbības aprakstu</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugin.cpp" line="73"/>
        <source>[menuitemname]</source>
        <translation>[menuitemname]</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugin.cpp" line="80"/>
        <source>&amp;[menuname]</source>
        <translation>&amp;[menuname]</translation>
    </message>
</context>
</TS>
