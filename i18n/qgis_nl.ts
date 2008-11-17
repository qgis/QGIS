<!DOCTYPE TS><TS>
<context>
    <name>@default</name>
    <message>
        <source>OGR Driver Manager</source>
        <translation type="obsolete">OGR Driver Instellingen</translation>
    </message>
    <message>
        <source>unable to get OGRDriverManager</source>
        <translation type="obsolete">laden van OGRDriverManager mislukt</translation>
    </message>
</context>
<context>
    <name>CoordinateCapture</name>
    <message>
        <source>Coordinate Capture</source>
        <translation>Coördinaat klikken</translation>
    </message>
    <message>
        <source>Click on the map to view coordinates and capture to clipboard.</source>
        <translation>Klik in de kaart om de coördinaten te zien en te kopiëren naar het klembord.</translation>
    </message>
    <message>
        <source>&amp;Coordinate Capture</source>
        <translation>&amp;Coördinaat Klikken</translation>
    </message>
    <message>
        <source>Click to select the CRS to use for coordinate display</source>
        <translation>Klik om het CRS te selecteren voor het tonen van coördinaten</translation>
    </message>
    <message>
        <source>Coordinate in your selected CRS</source>
        <translation>Coördinaat in geselecteerde CRS</translation>
    </message>
    <message>
        <source>Coordinate in map canvas coordinate reference system</source>
        <translation>Coördinaat in CRS van kaartscherm</translation>
    </message>
    <message>
        <source>Copy to clipboard</source>
        <translation>Kopiëren naar klembord</translation>
    </message>
    <message>
        <source>Click to enable mouse tracking. Click the canvas to stop</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>CoordinateCaptureGui</name>
    <message>
        <source>Welcome to your automatically generated plugin!</source>
        <translation>Welkom bij uw automatisch gegenereerde plugin!</translation>
    </message>
    <message>
        <source>This is just a starting point. You now need to modify the code to make it do something useful....read on for a more information to get yourself started.</source>
        <translation>Dit is maar een startpunt. U moet de code aanpassen om er een een bruikbare plugin van te maken... lees verder voor meer informatie om te beginnen.</translation>
    </message>
    <message>
        <source>Documentation:</source>
        <translation>Handleiding:</translation>
    </message>
    <message>
        <source>You really need to read the QGIS API Documentation now at:</source>
        <translation>U moet nu echt de QGIS API handleiding lezen op:</translation>
    </message>
    <message>
        <source>In particular look at the following classes:</source>
        <translation>Kijk vooral naar de volgende klassen:</translation>
    </message>
    <message>
        <source>QgsPlugin is an ABC that defines required behaviour your plugin must provide. See below for more details.</source>
        <translation>QgsPlugin is een ABC die het meest gebruikt gedrag bevat van uw plugin. Zie hieronder voor meer details.</translation>
    </message>
    <message>
        <source>What are all the files in my generated plugin directory for?</source>
        <translation>Waar zijn al die bestanden in mijn gegenereerde plugin directory?</translation>
    </message>
    <message>
        <source>This is the generated CMake file that builds the plugin. You should add you application specific dependencies and source files to this file.</source>
        <translation>Dit is het gegenereerde CMake bestand voor het bouwen van de plugin. Breidt het uit met de applicatiespecifieke afhankelijkheden en bronbestanden.</translation>
    </message>
    <message>
        <source>This is the class that provides the &apos;glue&apos; between your custom application logic and the QGIS application. You will see that a number of methods are already implemented for you - including some examples of how to add a raster or vector layer to the main application map canvas. This class is a concrete instance of the QgisPlugin interface which defines required behaviour for a plugin. In particular, a plugin has a number of static methods and members so that the QgsPluginManager and plugin loader logic can identify each plugin, create an appropriate menu entry for it etc. Note there is nothing stopping you creating multiple toolbar icons and menu entries for a single plugin. By default though a single menu entry and toolbar button is created and its pre-configured to call the run() method in this class when selected. This default implementation provided for you by the plugin builder is well documented, so please refer to the code for further advice.</source>
        <translation>Deze Klasse is de verbinding tussen uw aangepaste progamma-logica en het  QGIS-programma. U ziet dat een aantal methoden al zijn geïmplementeerd, bijvoorbeeld wat voorbeelden over hoe een raster- of vectorlaag aan het hoofdscherm toevoegt. Deze Klasse is een uitgewerking van de &apos;QgisPlugin-interface&apos; die het gedrag bepaald van uw plugin. Bijvoorbeeld: een plgugin heeft een aantal &apos;static&apos; methoden en eigenschappen die maken dat de QgsPluginManager en de pluginlader de individiuele plugin kunnen herkennen en de juiste menuonderdelen ervoor aanmaakt. Merk op dat niets u weerhoudt om meerdere gereeschapsbalken of menuonderdelen voor een plugin aan te maken. Maar standaard wordt één menuonderdeel en knop in de gereedschapsbalk aangemaakt en zal de &apos;run&apos;-methode van uw Klasse worden aangeroepen. Het standaard gedrag is in de code ruimschoots van commentaar voorzien, voor verdere informatie wordt daarheen verwezen.</translation>
    </message>
    <message>
        <source>This is a Qt designer &apos;ui&apos; file. It defines the look of the default plugin dialog without implementing any application logic. You can modify this form to suite your needs or completely remove it if your plugin does not need to display a user form (e.g. for custom MapTools).</source>
        <translation>Dit is een &apos;Qt designer ui&apos; bestand. Het definieert het uiterlijk van de standaard plugin dialoog zonder applicatie logica te implementeren. U kunt deze &apos;form&apos; aanpassen of helemaal verwijderen als uw plugin geen &apos;display&apos; heeft (b.v. zoals de huidige MapTools).</translation>
    </message>
    <message>
        <source>This is the concrete class where application logic for the above mentioned dialog should go. The world is your oyster here really....</source>
        <translation>Hier komt de echte applicatie logica voor de hierboven genoemde dialoog. Maar er wat leuks van...</translation>
    </message>
    <message>
        <source>This is the Qt4 resources file for your plugin. The Makefile generated for your plugin is all set up to compile the resource file so all you need to do is add your additional icons etc using the simple xml file format. Note the namespace used for all your resources e.g. (&apos;:/Homann/&apos;). It is important to use this prefix for all your resources. We suggest you include any other images and run time data in this resurce file too.</source>
        <translation>Dit is het Qt4 &apos;resource&apos;-bestand voor uw plugin. Het Make-bestand dat gegenereerd is voor uw plugin is al ingesteld om dit &apos;resource&apos;-bestand te compileren. Alleen bijkomende iconen toevoegen m.b.v. het simpele xml formaat. Let op: bij de &apos;namespace&apos; van al uw &apos;resources&apos; b.v. (&apos;:/Homann/&apos;). Het is belangrijk deze &apos;namespace&apos; te gebruiken voor al uw &apos;resources&apos;. We raden aan om andere afbeeldingen en &apos;runtime data&apos; aan dit &apos;resource&apos;-bestand toe te voegen.</translation>
    </message>
    <message>
        <source>This is the icon that will be used for your plugin menu entry and toolbar icon. Simply replace this icon with your own icon to make your plugin disctinctive from the rest.</source>
        <translation>Dit is de icoon welke gebruikt zal worden voor het plugin menu en als werkbalk icoon. Vervang eenvoudigweg dit icoon met uw eigen icoon om het onderscheidend te maken van de rest.</translation>
    </message>
    <message>
        <source>This file contains the documentation you are reading now!</source>
        <translation>Dit bestand bevat de documentatie welke u nu leest!</translation>
    </message>
    <message>
        <source>Getting developer help:</source>
        <translation>Ontwikkelaarshulp:</translation>
    </message>
    <message>
        <source>For Questions and Comments regarding the plugin builder template and creating your features in QGIS using the plugin interface please contact us via:</source>
        <translation>Voor Vragen en Opmerkingen over deze &apos;plugin builder template&apos; en het maken van objecten in QGIS via de plugin interface, neem contact op via:</translation>
    </message>
    <message>
        <source>&lt;li&gt; the QGIS developers mailing list, or &lt;/li&gt;&lt;li&gt; IRC (#qgis on freenode.net)&lt;/li&gt;</source>
        <translation>&lt;li&gt; the QGIS ontwikkellaars mailing list, of &lt;/li&gt;&lt;li&gt; IRC (#qgis on freenode.net)&lt;/li&gt;</translation>
    </message>
    <message>
        <source>QGIS is distributed under the Gnu Public License. If you create a useful plugin please consider contributing it back to the community.</source>
        <translation>QGIS wordt verspreid on de Gnu Public License. Als u een bruikbare plugin bouwt, overweeg die dan terug te spelen aan de &apos;community&apos;.</translation>
    </message>
    <message>
        <source>Have fun and thank you for choosing QGIS.</source>
        <translation>Veel plezier en bedankt voor het gebruik van QGIS.</translation>
    </message>
</context>
<context>
    <name>CoordinateCaptureGuiBase</name>
    <message>
        <source>QGIS Plugin Template</source>
        <translation>QGIS Plugin Template</translation>
    </message>
    <message>
        <source>Plugin Template</source>
        <translation>Plugin Template</translation>
    </message>
</context>
<context>
    <name>Dialog</name>
    <message>
        <source>Active repository:</source>
        <translation type="obsolete">Huidige repository:</translation>
    </message>
    <message>
        <source>Add</source>
        <translation type="obsolete">Toevoegen</translation>
    </message>
    <message>
        <source>Author</source>
        <translation type="obsolete">Auteur</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation type="obsolete">Verwijderen</translation>
    </message>
    <message>
        <source>Description</source>
        <translation type="obsolete">Omschrijving</translation>
    </message>
    <message>
        <source>Done</source>
        <translation type="obsolete">Gereed</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation type="obsolete">Bewerken</translation>
    </message>
    <message>
        <source>Get List</source>
        <translation type="obsolete">Haal de lijst op</translation>
    </message>
    <message>
        <source>Install Plugin</source>
        <translation type="obsolete">Installeer Plugin</translation>
    </message>
    <message>
        <source>Name</source>
        <translation type="obsolete">Naam</translation>
    </message>
    <message>
        <source>Name of plugin to install</source>
        <translation type="obsolete">Naam van de plugin om te installeren</translation>
    </message>
    <message>
        <source>QGIS Plugin Installer</source>
        <translation type="obsolete">QGIS Plugin Installeren</translation>
    </message>
    <message>
        <source>Repository</source>
        <translation type="obsolete">Repository</translation>
    </message>
    <message>
        <source>Select repository, retrieve the list of available plugins, select one and install it</source>
        <translation type="obsolete">Kies een repository, haal de lijst met beschikbare plugins op, selecteer er een en installeer die</translation>
    </message>
    <message>
        <source>The plugin will be installed to ~/.qgis/python/plugins</source>
        <translation type="obsolete">De plugin zal worden geinstalleerd in ~/.qgis/python/plugins</translation>
    </message>
    <message>
        <source>Version</source>
        <translation type="obsolete">Versie</translation>
    </message>
    <message>
        <source>Connect</source>
        <translation>Verbinden</translation>
    </message>
    <message>
        <source>Browse</source>
        <translation>Bladeren</translation>
    </message>
    <message>
        <source>OGR Converter</source>
        <translation>OGR Converter</translation>
    </message>
    <message>
        <source>Could not establish connection to: &apos;</source>
        <translation>Kan geen verbinding maken met; &apos;</translation>
    </message>
    <message>
        <source>Open OGR file</source>
        <translation>Open OGR-bestand</translation>
    </message>
    <message>
        <source>OGR File Data Source (*.*)</source>
        <translation>OGR-bestand (*.*)</translation>
    </message>
    <message>
        <source>Open Directory</source>
        <translation>Open Map</translation>
    </message>
    <message>
        <source>Input OGR dataset is missing!</source>
        <translation>Input OGR-bestand ontbreekt!</translation>
    </message>
    <message>
        <source>Input OGR layer name is missing!</source>
        <translation>Naam van Input OGR laag ontbreekt!</translation>
    </message>
    <message>
        <source>Target OGR format not selected!</source>
        <translation>Doel OGR-formaat niet geselecteerd!</translation>
    </message>
    <message>
        <source>Output OGR dataset is missing!</source>
        <translation>OGR-uitvoerbestand niet aanwezig!</translation>
    </message>
    <message>
        <source>Output OGR layer name is missing!</source>
        <translation>Geen naam voor de OGR-laag aanwezig!</translation>
    </message>
    <message>
        <source>Successfully translated layer &apos;</source>
        <translation>Succesvol geconverteerde laag &apos;</translation>
    </message>
    <message>
        <source>Failed to translate layer &apos;</source>
        <translation>Problemen bij het converteren van laag &apos;</translation>
    </message>
    <message>
        <source>Successfully connected to: &apos;</source>
        <translation>Verbinden geslaagd met: &apos;</translation>
    </message>
    <message>
        <source>Choose a file name to save to</source>
        <translation>Kies een bestandsnaam om te bewaren</translation>
    </message>
</context>
<context>
    <name>Geoprocessing</name>
    <message>
        <source>Geoprocessing</source>
        <translation type="obsolete">Geoprocessing</translation>
    </message>
    <message>
        <source>OK</source>
        <translation type="obsolete">OK</translation>
    </message>
    <message>
        <source>Exit</source>
        <translation type="obsolete">Afsluiten</translation>
    </message>
    <message>
        <source>Layer B:</source>
        <translation type="obsolete">Laag B:</translation>
    </message>
    <message>
        <source>Parameter:</source>
        <translation type="obsolete">Parameter:</translation>
    </message>
    <message>
        <source>0.0</source>
        <translation type="obsolete">0.0</translation>
    </message>
    <message>
        <source>Layer A:</source>
        <translation type="obsolete">Laag A:</translation>
    </message>
    <message>
        <source>Function:</source>
        <translation type="obsolete">Functie:</translation>
    </message>
    <message>
        <source>Select a function ...</source>
        <translation type="obsolete">Selecteer een functie ...</translation>
    </message>
    <message>
        <source>Buffer</source>
        <translation type="obsolete">Buffer</translation>
    </message>
    <message>
        <source>Convex Hull</source>
        <translation type="obsolete">Convex Hull</translation>
    </message>
    <message>
        <source>Difference A - B</source>
        <translation type="obsolete">Difference A - B</translation>
    </message>
    <message>
        <source>Intersection</source>
        <translation type="obsolete">Intersectie</translation>
    </message>
    <message>
        <source>SymDifference</source>
        <translation type="obsolete">SymDifference</translation>
    </message>
    <message>
        <source>Union</source>
        <translation type="obsolete">Union</translation>
    </message>
    <message>
        <source>Add Layer to Map?</source>
        <translation type="obsolete">Laag toevoegen aan Kaart?</translation>
    </message>
    <message>
        <source>Save as:</source>
        <translation type="obsolete">Opslaan als:</translation>
    </message>
    <message>
        <source>Browse</source>
        <translation type="obsolete">Bladeren</translation>
    </message>
    <message>
        <source>use selected Objects</source>
        <translation type="obsolete">gebruik geselecteerde Objecten</translation>
    </message>
</context>
<context>
    <name>Gui</name>
    <message>
        <source>Documentation:</source>
        <translation>Handleiding:</translation>
    </message>
    <message>
        <source>For Questions and Comments regarding the plugin builder template and creating your features in QGIS using the plugin interface please contact us via:</source>
        <translation>Voor Vragen en Opmerkingen over deze &apos;plugin builder template&apos; en het maken van &apos;features&apos; in QGIS via de plugin interface, neem contact op via:</translation>
    </message>
    <message>
        <source>Getting developer help:</source>
        <translation>Ontwikkelaarshulp:</translation>
    </message>
    <message>
        <source>Have fun and thank you for choosing QGIS.</source>
        <translation>Veel plezier en bedankt voor het gebruik van QGIS.</translation>
    </message>
    <message>
        <source>In particular look at the following classes:</source>
        <translation>Kijk vooral naar de volgende klassen:</translation>
    </message>
    <message>
        <source>&lt;li&gt; the QGIS developers mailing list, or &lt;/li&gt;&lt;li&gt; IRC (#qgis on freenode.net)&lt;/li&gt;</source>
        <translation>&lt;li&gt; the QGIS ontwikkellaars mailing list, or &lt;/li&gt;&lt;li&gt; IRC (#qgis on freenode.net)&lt;/li&gt;</translation>
    </message>
    <message>
        <source>QGIS is distributed under the Gnu Public License. If you create a useful plugin please consider contributing it back to the community.</source>
        <translation>QGIS wordt verspreid on de Gnu Public License. Als u een bruikbare plugin bouwt, overweeg die dan terug te spelen aan de &apos;community&apos;.</translation>
    </message>
    <message>
        <source>QgsPlugin is an ABC that defines required behaviour your plugin must provide. See below for more details.</source>
        <translation>QgsPlugin is een ABC die het meest gebruikt gedrag bevat van uw plugin. Zie hieronder voor meer details.</translation>
    </message>
    <message>
        <source>This file contains the documentation you are reading now!</source>
        <translation>Dit bestand bevat de documentatie welke u nu leest!</translation>
    </message>
    <message>
        <source>This is a Qt designer &apos;ui&apos; file. It defines the look of the default plugin dialog without implementing any application logic. You can modify this form to suite your needs or completely remove it if your plugin does not need to display a user form (e.g. for custom MapTools).</source>
        <translation>Dit is een &apos;Qt designer ui&apos; bestand. Het definieert het uiterluik van de standaard plugin dialoog zonder applicatie logica te implementeren. U kunt deze &apos;form&apos; aanpassen of helemaal verwijderen als uw plugin geen &apos;display&apos; heeft (b.v. zoals de huidige MapTools).</translation>
    </message>
    <message>
        <source>This is just a starting point. You now need to modify the code to make it do something useful....read on for a more information to get yourself started.</source>
        <translation>Dit is maar een startpunt. U moet de code aanpassen om er een een bruikbare plugin van te maken... lees verder voor meer informatie om te beginnen.</translation>
    </message>
    <message>
        <source>This is the class that provides the &apos;glue&apos; between your custom application logic and the QGIS application. You will see that a number of methods are already implemented for you - including some examples of how to add a raster or vector layer to the main application map canvas. This class is a concrete instance of the QgisPlugin interface which defines required behaviour for a plugin. In particular, a plugin has a number of static methods and members so that the QgsPluginManager and plugin loader logic can identify each plugin, create an appropriate menu entry for it etc. Note there is nothing stopping you creating multiple toolbar icons and menu entries for a single plugin. By default though a single menu entry and toolbar button is created and its pre-configured to call the run() method in this class when selected. This default implementation provided for you by the plugin builder is well documented, so please refer to the code for further advice.</source>
        <translation>This is the class that provides the &apos;glue&apos; between your custom application logic and the QGIS application. You will see that a number of methods are already implemented for you - including some examples of how to add a raster or vector layer to the main application map canvas. This class is a concrete instance of the QgisPlugin interface which defines required behaviour for a plugin. In particular, a plugin has a number of static methods and members so that the QgsPluginManager and plugin loader logic can identify each plugin, create an appropriate menu entry for it etc. Note there is nothing stopping you creating multiple toolbar icons and menu entries for a single plugin. By default though a single menu entry and toolbar button is created and its pre-configured to call the run() method in this class when selected. This default implementation provided for you by the plugin builder is well documented, so please refer to the code for further advice.</translation>
    </message>
    <message>
        <source>This is the concrete class where application logic for the above mentioned dialog should go. The world is your oyster here really....</source>
        <translation>Hier komt de echte applicatie logica voor de hierboven genoemde dialoog. Maar er wat leuks van...</translation>
    </message>
    <message>
        <source>This is the generated CMake file that builds the plugin. You should add you application specific dependencies and source files to this file.</source>
        <translation>Dit is het gegenereerde CMake bestand voor het bouwen van de plugin. Breid het uit met de applicatie specifieke afhankelijkheden en bronbestanden.</translation>
    </message>
    <message>
        <source>This is the icon that will be used for your plugin menu entry and toolbar icon. Simply replace this icon with your own icon to make your plugin disctinctive from the rest.</source>
        <translation>Dit is de icoon welke gebruikt zal worden voor het plugin menu en als werkbalk icoon. Vervang eenvoudigweg dit icoon met uw eigen icoon om het onderscheidend te maken van de rest.</translation>
    </message>
    <message>
        <source>This is the Qt4 resources file for your plugin. The Makefile generated for your plugin is all set up to compile the resource file so all you need to do is add your additional icons etc using the simple xml file format. Note the namespace used for all your resources e.g. (&apos;:/Homann/&apos;). It is important to use this prefix for all your resources. We suggest you include any other images and run time data in this resurce file too.</source>
        <translation>Dit is het Qt4 &apos;resource&apos;-bestand voor uw plugin. Het Make-bestand dat gegenereerd is voor up plugin is al ingesteld om dit &apos;resource&apos;-bestand te compileren. Alleen bijkomende iconen toevoegen aan het simpele xml formaat. Let op bij de &apos;namespace&apos; van al uw &apos;resources&apos; b.v. (&apos;:/Homann/&apos;). Het is belangrijk deze &apos;namespace&apos; te gebruiken voor al uw &apos;resources&apos;. We raden aan om andere afbeeldingen en &apos;runtime data&apos; aan dit &apos;resource&apos;-bestand toe te voegen.</translation>
    </message>
    <message>
        <source>Welcome to your automatically generated plugin!</source>
        <translation>Welkom bij uw automatisch gegenereerde plugin!</translation>
    </message>
    <message>
        <source>What are all the files in my generated plugin directory for?</source>
        <translation>Waar zijn al die bestanden in mijn gegenereerde plugin directory?</translation>
    </message>
    <message>
        <source>You really need to read the QGIS API Documentation now at:</source>
        <translation>U moet nu echt de QGIS API handleiding lezen op:</translation>
    </message>
</context>
<context>
    <name>MapCoordsDialogBase</name>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Annuleren</translation>
    </message>
    <message>
        <source>Enter map coordinates</source>
        <translation>Voer kaartcoördinaten in</translation>
    </message>
    <message>
        <source>Enter X and Y coordinates which correspond with the selected point on the image. Alternatively, click the button with icon of a pencil and then click a corresponding point on map canvas of QGIS to fill in coordinates of that point.</source>
        <translation>Voer X en Y coördinaten in die corresponderen met het geselecteerde punt in de afbeelding. Als alternatief, klik de knop met het potlood en klik een corresponderend punt op de kaart van QGIS om de coördinaten van dat punt te bepalen.</translation>
    </message>
    <message>
        <source> from map canvas</source>
        <translation> van de kaart</translation>
    </message>
    <message>
        <source>&amp;OK</source>
        <translation>&amp;OK</translation>
    </message>
    <message>
        <source>X:</source>
        <translation>X:</translation>
    </message>
    <message>
        <source>Y:</source>
        <translation>Y:</translation>
    </message>
</context>
<context>
    <name>NewPostgisLayer</name>
    <message>
        <source>Create new PostGIS layer</source>
        <translation type="obsolete">Nieuwe PostGIS-laag aanmaken</translation>
    </message>
    <message>
        <source>Available PostgreSQL connections</source>
        <translation type="obsolete">Beschikbare PostgreSQL verbindingen</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation type="obsolete">Verwijderen</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation type="obsolete">Bewerken</translation>
    </message>
    <message>
        <source>New</source>
        <translation type="obsolete">Nieuw</translation>
    </message>
    <message>
        <source>Connect</source>
        <translation type="obsolete">Verbinden</translation>
    </message>
    <message>
        <source>New layer definition</source>
        <translation type="obsolete">Nieuwe laag definitie</translation>
    </message>
    <message>
        <source>Schema name:</source>
        <translation type="obsolete">Schemanaam:</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:8pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;p, li { white-space: pre-wrap; }&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:8pt; font-weight:400; font-style:normal;&quot;&gt;&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>New layer name:</source>
        <translation type="obsolete">Nieuwe laagnaam:</translation>
    </message>
    <message>
        <source>Index column:</source>
        <translation type="obsolete">Indexkolom:</translation>
    </message>
    <message>
        <source>gid</source>
        <translation type="obsolete">gid</translation>
    </message>
    <message>
        <source>Geometry column:</source>
        <translation type="obsolete">Geometrie kolom:</translation>
    </message>
    <message>
        <source>the_geom</source>
        <translation type="obsolete">the_geom</translation>
    </message>
    <message>
        <source>Geometry type:</source>
        <translation type="obsolete">Type geometry:</translation>
    </message>
    <message>
        <source>MULTIPOINT</source>
        <translation type="obsolete">MULTIPOINT</translation>
    </message>
    <message>
        <source>MULTILINESTRING</source>
        <translation type="obsolete">MULTILINESTRING</translation>
    </message>
    <message>
        <source>MULTIPOLYGON</source>
        <translation type="obsolete">MULTIPOLYGON</translation>
    </message>
    <message>
        <source>SRID number:</source>
        <translation type="obsolete">SRID code:</translation>
    </message>
    <message>
        <source>Help</source>
        <translation type="obsolete">Help</translation>
    </message>
    <message>
        <source>F1</source>
        <translation type="obsolete">F1</translation>
    </message>
    <message>
        <source>Ok</source>
        <translation type="obsolete">Ok</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation type="obsolete">Annuleren</translation>
    </message>
</context>
<context>
    <name>OgrConverterGuiBase</name>
    <message>
        <source>OGR Layer Converter</source>
        <translation>OGR-laag Omzetten</translation>
    </message>
    <message>
        <source>Source</source>
        <translation>Bron</translation>
    </message>
    <message>
        <source>Format</source>
        <translation>Formaat</translation>
    </message>
    <message>
        <source>File</source>
        <translation>Bestand</translation>
    </message>
    <message>
        <source>Directory</source>
        <translation>Map</translation>
    </message>
    <message>
        <source>Remote source</source>
        <translation>Externe bron</translation>
    </message>
    <message>
        <source>Dataset</source>
        <translation>Dataset</translation>
    </message>
    <message>
        <source>Browse</source>
        <translation>Bladeren</translation>
    </message>
    <message>
        <source>Layer</source>
        <translation>Laag</translation>
    </message>
    <message>
        <source>Target</source>
        <translation>Doel</translation>
    </message>
</context>
<context>
    <name>OgrPlugin</name>
    <message>
        <source>Run OGR Layer Converter</source>
        <translation>OGR-lagen Omzetten</translation>
    </message>
    <message>
        <source>Replace this with a short description of the what the plugin does</source>
        <translation type="obsolete">Vervang dit door een korte beschrijving van de functie van de plugin</translation>
    </message>
    <message>
        <source>OG&amp;R Converter</source>
        <translation>OG&amp;R Converter</translation>
    </message>
    <message>
        <source>Translates vector layers between formats supported by OGR library</source>
        <translation>Transformeert vector-lagen tussen formaten die worden ondersteund door OGR</translation>
    </message>
</context>
<context>
    <name>QFileDialog</name>
    <message>
        <source>Load layer properties from style file (.qml)</source>
        <translation>Laad kaartlaageigenschappen in vanuit een stijlbestand (.qml)</translation>
    </message>
    <message>
        <source>Save experiment report to portable document format (.pdf)</source>
        <translation>Sla experiment rapportage op als portable document format (.pdf)</translation>
    </message>
    <message>
        <source>Save layer properties as style file (.qml)</source>
        <translation>Sla kaartlaageigenschappen op als stijlbestand (.qml)</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <source> 1 feature found</source>
        <translation> 1 kaartobject gevonden</translation>
    </message>
    <message>
        <source>2.5D shape type not supported</source>
        <translation>2,5D shape type wordt niet ondersteund</translation>
    </message>
    <message>
        <source>action</source>
        <translation>actie</translation>
    </message>
    <message>
        <source>Add Delimited Text Layer</source>
        <translation>Tekstgescheiden kaartlaag Toevoegen</translation>
    </message>
    <message>
        <source>Adding features to 2.5D shapetypes is not supported yet</source>
        <translation>Toevoegen van kaartobjecten aan 2,5D shapetype wordt nog niet ondersteund</translation>
    </message>
    <message>
        <source>Adding projection info to rasters</source>
        <translation>Projectie-informatie wordt aan raster toegevoegd</translation>
    </message>
    <message>
        <source>Adds WFS layers to the QGIS canvas</source>
        <translation>Voeg WFS-kaartlagen toe aan QGIS kaartvenster</translation>
    </message>
    <message>
        <source>An unknown error occured</source>
        <translation>Er is een onbekende fout opgetreden</translation>
    </message>
    <message>
        <source>A problem with geometry type occured</source>
        <translation>Er was een probleem met een geomtrie (-type)</translation>
    </message>
    <message>
        <source>Area</source>
        <translation>Vlak</translation>
    </message>
    <message>
        <source>Areas</source>
        <translation>Vlakken</translation>
    </message>
    <message>
        <source> at line </source>
        <translation> op regel </translation>
    </message>
    <message>
        <source>Band</source>
        <translation>Band</translation>
    </message>
    <message>
        <source>because</source>
        <translation>omdat</translation>
    </message>
    <message>
        <source>&lt;b&gt;Layer&lt;/b&gt;</source>
        <translation>&lt;b&gt;Laag&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Bottom</source>
        <translation>Onder</translation>
    </message>
    <message>
        <source>Boundaries</source>
        <translation>Grenzen</translation>
    </message>
    <message>
        <source>&lt;b&gt;Raster&lt;/b&gt;</source>
        <translation>&lt;b&gt;Raster&lt;/b&gt;</translation>
    </message>
    <message>
        <source>&lt;br&gt;Mapset: </source>
        <translation>&lt;br&gt;Mapset: </translation>
    </message>
    <message>
        <source>&lt;br&gt;Mapset: </source>
        <comment>Metadata in GRASS Browser</comment>
        <translation>&lt;br&gt;Mapset: </translation>
    </message>
    <message>
        <source>Builds a graticule</source>
        <translation>Maakt een kaartgrid</translation>
    </message>
    <message>
        <source>&lt;b&gt;Vector&lt;/b&gt;</source>
        <translation>&lt;b&gt;Vector&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Cannot add feature. Unknown WKB type</source>
        <translation>Kan kaartobject niet toevoegen. Onbekend WKB-type</translation>
    </message>
    <message>
        <source>Cannot apply the &apos;capture line&apos; tool on this vector layer</source>
        <translation>Kan het lijnintekengereedschap niet toepassen op deze vectorlaag</translation>
    </message>
    <message>
        <source>Cannot apply the &apos;capture point&apos; tool on this vector layer</source>
        <translation>Kan het puntintekengereedschap niet toepassen op deze vectorlaag</translation>
    </message>
    <message>
        <source>Cannot apply the &apos;capture polygon&apos; tool on this vector layer</source>
        <translation>kan het polygoonintekengereedschap niet toepassen op deze vectorlaag</translation>
    </message>
    <message>
        <source>Cannot create </source>
        <translation>Kan geen </translation>
    </message>
    <message>
        <source>Cannot create temporary directory </source>
        <translation>Kan geen tijdelijke map aanmaken </translation>
    </message>
    <message>
        <source>Cannot edit the vector layer. To make it editable, go to the file item of the layer, right click and check &apos;Allow Editing&apos;.</source>
        <translation>Kan de vectorlaag niet aanpassen. Om de laag aanpasbaar te maken, ga naar het &apos;file item&apos; van de laag, rechtermuisklik en vink &apos;Aanpasbaar Maken&apos;.</translation>
    </message>
    <message>
        <source>Cannot open raster header</source>
        <translation>Kan raster header niet openen</translation>
    </message>
    <message>
        <source>Cannot read raster map region</source>
        <translation>Kan kaartraster regio niet inlezen</translation>
    </message>
    <message>
        <source>Cannot read region</source>
        <translation>Kan regio niet inlezen</translation>
    </message>
    <message>
        <source>Cannot read vector map region</source>
        <translation>Kan vectorkaart regio niet inlezen</translation>
    </message>
    <message>
        <source>Cannot remove mapset lock: </source>
        <translation>Kan de mapset niet vrijkrijgen (&apos;locked&apos;): </translation>
    </message>
    <message>
        <source>Cannot start </source>
        <translation>Starten onmogelijk </translation>
    </message>
    <message>
        <source>Cannot transform the point to the layers coordinate system</source>
        <translation>Kan dit punt niet omzetten naar coördinatensysteem van de lagen</translation>
    </message>
    <message>
        <source>Categories</source>
        <translation>Categoriën</translation>
    </message>
    <message>
        <source>Caught a coordinate system exception while trying to transform a point. Unable to calculate line length.</source>
        <translation>Foutmelding ontvangen bij het projecteren van een punt. Lijnlengte-berekening niet mogelijk.</translation>
    </message>
    <message>
        <source>Caught a coordinate system exception while trying to transform a point. Unable to calculate polygon area.</source>
        <translation>Foutmelding ontvangen bij het projecteren van een punt. Oppervlakteberekening van deze polygoon niet mogelijk.</translation>
    </message>
    <message>
        <source>Centroids</source>
        <translation>Zwaartepunten</translation>
    </message>
    <message>
        <source>Choose GRASS installation path (GISBASE)</source>
        <translation>Geef het GRASS installatie pad aan (GISBASE)</translation>
    </message>
    <message>
        <source>Close line</source>
        <translation>Sluit lijn</translation>
    </message>
    <message>
        <source>Closing down connection</source>
        <translation>Verbinding wordt afgesloten</translation>
    </message>
    <message>
        <source> cm</source>
        <translation> cm</translation>
    </message>
    <message>
        <source> column </source>
        <translation>  kolom  </translation>
    </message>
    <message>
        <source>Columns</source>
        <translation>Kolommen</translation>
    </message>
    <message>
        <source>Comments</source>
        <translation>Opmerkingen</translation>
    </message>
    <message>
        <source>Connecting to &apos;%1&apos;</source>
        <translation>Verbinding maken met &apos;%1&apos;</translation>
    </message>
    <message>
        <source>Coordinate transform error</source>
        <translation>Coördinaattransformatie fout</translation>
    </message>
    <message>
        <source>CopyrightLabel</source>
        <translation>Copyrightlabel</translation>
    </message>
    <message>
        <source>Could not identify objects on</source>
        <translation>Geen indentificeerbaare kaartobjecten op</translation>
    </message>
    <message>
        <source>Could not remove polygon intersection</source>
        <translation>Kan polygoonintersectie niet verwijderen</translation>
    </message>
    <message>
        <source>Couldn&apos;t load plugin </source>
        <translation>Kan plugin niet laden </translation>
    </message>
    <message>
        <source>Couldn&apos;t load PyQt bindings.
Python support will be disabled.</source>
        <translation type="obsolete">PyQt-bindingen kon niet worden geladen.
Pythonkoppeling kan niet worden gebruikt.</translation>
    </message>
    <message>
        <source>Couldn&apos;t load QGIS bindings.
Python support will be disabled.</source>
        <translation type="obsolete">Kan QGIS-bindingen niet laden.
Pythonkoppeling kan niet worden gebruikt.</translation>
    </message>
    <message>
        <source>Couldn&apos;t load SIP module.
Python support will be disabled.</source>
        <translation type="obsolete">Dan SIP-module niet laden.
Pythonkoppeling kan niet worden gebruikt.</translation>
    </message>
    <message>
        <source>Couldn&apos;t open the data source: </source>
        <translation>De volgende databron kan niet worden geopend:</translation>
    </message>
    <message>
        <source>Created default style file as </source>
        <translation>Standaard stijlbestand is aangemaakt als</translation>
    </message>
    <message>
        <source>Currently only filebased datasets are supported</source>
        <translation type="obsolete">Op dit moment worden alleen bestandsgebaseerde datasets ondersteund</translation>
    </message>
    <message>
        <source>Database</source>
        <translation>Database</translation>
    </message>
    <message>
        <source>Data description</source>
        <translation>Data beschrijving</translation>
    </message>
    <message>
        <source>Data source</source>
        <translation>Databron</translation>
    </message>
    <message>
        <source> degree</source>
        <translation> graad</translation>
    </message>
    <message>
        <source> degrees</source>
        <translation>graden</translation>
    </message>
    <message>
        <source>Delete selected / select next</source>
        <translation>Verwijder geselecteerde / selecteer volgende</translation>
    </message>
    <message>
        <source>Delete vertex</source>
        <translation>Verwijder hoekpunt</translation>
    </message>
    <message>
        <source>Displays a north arrow overlayed onto the map</source>
        <translation>Toont een noordpijl over de kaart heen</translation>
    </message>
    <message>
        <source>Division by zero.</source>
        <translation>Deling door nul.</translation>
    </message>
    <message>
        <source>Draws a scale bar</source>
        <translation>Tekent een schaalbalk</translation>
    </message>
    <message>
        <source>Draws copyright information</source>
        <translation>Tekent copyright-informatie</translation>
    </message>
    <message>
        <source>Driver</source>
        <translation>Stuurbestand (driver)</translation>
    </message>
    <message>
        <source> due an error when calling its classFactory() method</source>
        <translation>door een fout bij het aanroepen van de classFactory()-methode</translation>
    </message>
    <message>
        <source> due an error when calling its initGui() method</source>
        <translation> door een fout bij het aanroepen van de initGui()-methode</translation>
    </message>
    <message>
        <source>East</source>
        <translation>Oost</translation>
    </message>
    <message>
        <source>Error</source>
        <translation>Fout</translation>
    </message>
    <message>
        <source>Error, could not add island</source>
        <translation>Fout, toevoegen van eiland niet mogelijk</translation>
    </message>
    <message>
        <source>Error, could not add ring</source>
        <translation>Fout, toevoegen van ring niet mogelijk</translation>
    </message>
    <message>
        <source>ERROR: Failed to created default style file as </source>
        <translation type="obsolete">FOUT: probleem bij het aanmaken van standaard stijlbestand </translation>
    </message>
    <message>
        <source>Error while unloading plugin </source>
        <translation>Fout bij het uitschakelen van plugin </translation>
    </message>
    <message>
        <source>E-W resolution</source>
        <translation>O-W resolutie</translation>
    </message>
    <message>
        <source> exist but is not writable</source>
        <translation> bestaat maar is niet beschrijfbaar</translation>
    </message>
    <message>
        <source>Faces</source>
        <translation>Aanzichten</translation>
    </message>
    <message>
        <source>Features</source>
        <translation>Objecten</translation>
    </message>
    <message>
        <source> features found</source>
        <translation> kaartobjecten gevonden</translation>
    </message>
    <message>
        <source> feet</source>
        <translation> voet</translation>
    </message>
    <message>
        <source>Fit to a Helmert transform requires at least 2 points.</source>
        <translation>Om een Helmert transformatie te kunnen doen zijn minstens 2 punten nodig.</translation>
    </message>
    <message>
        <source>Fit to a linear transform requires at least 2 points.</source>
        <translation>Om een lineaire transformatie te kunnen doen zijn minstens 2 punten nodig.</translation>
    </message>
    <message>
        <source>Fit to an affine transform requires at least 4 points.</source>
        <translation>Om een affine transformatie uit te voeren zijn minstens 4 punten nodig.</translation>
    </message>
    <message>
        <source> foot</source>
        <translation> voet</translation>
    </message>
    <message>
        <source> for file </source>
        <translation> voor bestand </translation>
    </message>
    <message>
        <source>Format</source>
        <translation>Formaat</translation>
    </message>
    <message>
        <source>Geoprocessing functions for working with PostgreSQL/PostGIS layers</source>
        <translation>&apos;Geoprocessing&apos; functies voor de bewerking van PostgreSQL/PostGIS-lagen</translation>
    </message>
    <message>
        <source>Georeferencer</source>
        <translation>&apos;Georeferencer&apos;</translation>
    </message>
    <message>
        <source>GISBASE is not set.</source>
        <translation>&apos;GISBASE&apos; is niet gezet.</translation>
    </message>
    <message>
        <source>GPS eXchange format provider</source>
        <translation>GPS-uitwisselingsformaat &apos;provider&apos;</translation>
    </message>
    <message>
        <source>GPS Tools</source>
        <translation>GPS-gereedschap</translation>
    </message>
    <message>
        <source>GRASS</source>
        <translation>GRASS</translation>
    </message>
    <message>
        <source>GRASS data won&apos;t be available if GISBASE is not specified.</source>
        <translation>GRASS-data is niet beschikbaar indien GISBASE niet is gespecificeerd.</translation>
    </message>
    <message>
        <source>GRASS layer</source>
        <translation>GRASS-laag</translation>
    </message>
    <message>
        <source>GRASS plugin</source>
        <translation>GRASS-plugin</translation>
    </message>
    <message>
        <source>Graticule Creator</source>
        <translation>Kaartgridbouwer</translation>
    </message>
    <message>
        <source> ha</source>
        <translation> ha</translation>
    </message>
    <message>
        <source>History&lt;br&gt;</source>
        <translation>Geschiedenis&lt;br&gt;</translation>
    </message>
    <message>
        <source>Islands</source>
        <translation>Eilanden</translation>
    </message>
    <message>
        <source> is not a GRASS mapset.</source>
        <translation> is geen GRASS-mapset.</translation>
    </message>
    <message>
        <source>Kernels</source>
        <translation>Kernels</translation>
    </message>
    <message>
        <source>Key column</source>
        <translation>Sleutelkolom</translation>
    </message>
    <message>
        <source> km</source>
        <translation> km</translation>
    </message>
    <message>
        <source> km2</source>
        <translation>km2</translation>
    </message>
    <message>
        <source>Label</source>
        <translation>Label</translation>
    </message>
    <message>
        <source>Layer cannot be added to</source>
        <translation>Laag can niet worden toegevoegd aan</translation>
    </message>
    <message>
        <source>Layer not editable</source>
        <translation>Layer niet aanpasbaar</translation>
    </message>
    <message>
        <source>Length</source>
        <translation>Lengte</translation>
    </message>
    <message>
        <source>Lines</source>
        <translation>Lijnen</translation>
    </message>
    <message>
        <source>Loaded default style file from </source>
        <translation type="obsolete">Standaard stylebestand geladen van </translation>
    </message>
    <message>
        <source>Loads and displays delimited text files containing x,y coordinates</source>
        <translation>Laad en toont tekstgescheiden bestanden die x,y-coördinaten bevatten</translation>
    </message>
    <message>
        <source>Location: </source>
        <comment>Metadata in GRASS Browser</comment>
        <translation>Locatie: </translation>
    </message>
    <message>
        <source>Location: </source>
        <translation>Locatie: </translation>
    </message>
    <message>
        <source>Looking up &apos;%1&apos;</source>
        <translation>Zoeken naar &apos;%1&apos;</translation>
    </message>
    <message>
        <source> m</source>
        <translation> m</translation>
    </message>
    <message>
        <source> m2</source>
        <translation>m2</translation>
    </message>
    <message>
        <source>Mapset is already in use.</source>
        <translation>Mapset is al in gebruik.</translation>
    </message>
    <message>
        <source>Maximum value</source>
        <translation>Maximale waarde</translation>
    </message>
    <message>
        <source>[menuitemname]</source>
        <translation></translation>
    </message>
    <message>
        <source> mile</source>
        <translation>mijl</translation>
    </message>
    <message>
        <source>Minimum value</source>
        <translation>Minimale waarde</translation>
    </message>
    <message>
        <source> mm</source>
        <translation> mm</translation>
    </message>
    <message>
        <source>New centroid</source>
        <translation>Nieuw zwaartepunt/centroid</translation>
    </message>
    <message>
        <source>New location</source>
        <translation>Nieuwe locatie</translation>
    </message>
    <message>
        <source>New point</source>
        <translation>Nieuw punt</translation>
    </message>
    <message>
        <source>New vertex</source>
        <translation>Nieuw hoekpunt</translation>
    </message>
    <message>
        <source>New vertex position</source>
        <translation>Nieuw hoekpunt positie</translation>
    </message>
    <message>
        <source>no</source>
        <translation>nee</translation>
    </message>
    <message>
        <source>No active layer</source>
        <translation>Geen aktieve laag</translation>
    </message>
    <message>
        <source>No Data Provider Plugins</source>
        <comment>No QGIS data provider plugins found in:</comment>
        <translation>Geen Dataprovider-plugins</translation>
    </message>
    <message>
        <source>No data provider plugins are available. No vector layers can be loaded</source>
        <translation>Geen dataprovider-plugins beschikbaar. Vectorlagen kunnen niet worden ingelezen</translation>
    </message>
    <message>
        <source>No Data Providers</source>
        <translation>Geen Dataproviders</translation>
    </message>
    <message>
        <source>No features found</source>
        <translation>Geen kaartobjecten gevonden</translation>
    </message>
    <message>
        <source>No features were found in the active layer at the point you clicked</source>
        <translation>Er zijn geen kaartobjecten gevonden in de aktieve laag op het punt waar u klikte</translation>
    </message>
    <message>
        <source>North</source>
        <translation>Noord</translation>
    </message>
    <message>
        <source>NorthArrow</source>
        <translation>Noordpijl</translation>
    </message>
    <message>
        <source>Not a vector layer</source>
        <translation>Geen vectorlaag</translation>
    </message>
    <message>
        <source>Not connected</source>
        <translation>Geen verbinding</translation>
    </message>
    <message>
        <source>No vector layers can be loaded. Check your QGIS installation</source>
        <translation>Vectorlagen kunnen niet ingelezen worden. Controleer uw QGIS installatie</translation>
    </message>
    <message>
        <source>N-S resolution</source>
        <translation>N-Z resolutie</translation>
    </message>
    <message>
        <source>original location: </source>
        <translation>oorspronkelijke locatie: </translation>
    </message>
    <message>
        <source>Parse error at line </source>
        <translation>Parseerfout op lijn</translation>
    </message>
    <message>
        <source>[plugindescription]</source>
        <translation></translation>
    </message>
    <message>
        <source>Points</source>
        <translation>Punten</translation>
    </message>
    <message>
        <source>PostgreSQL Geoprocessing</source>
        <translation></translation>
    </message>
    <message>
        <source>Project file read error: </source>
        <translation>Projectbestand leesfout: </translation>
    </message>
    <message>
        <source>Python error</source>
        <translation>Python fout</translation>
    </message>
    <message>
        <source>QGIS couldn&apos;t find your GRASS installation.
Would you like to specify path (GISBASE) to your GRASS installation?</source>
        <translation>QGIS kan uw GRASS installatie niet vinden.
Wilt u het pad (GISBASE) naar uw GRASS installatie wijzen?</translation>
    </message>
    <message>
        <source>QGis files (*.qgs)</source>
        <translation>QGis bestanden (*.qgs)</translation>
    </message>
    <message>
        <source>Quick Print</source>
        <translation>Snelle Afdruk</translation>
    </message>
    <message>
        <source>Quick Print is a plugin to quickly print a map with minimal effort.</source>
        <translation>Snelle Afdruk is een plugin om snel en eenvoudig een kaartafdruk te maken.</translation>
    </message>
    <message>
        <source>Received %1 bytes (total unknown)</source>
        <translation>Aantal ontvangen bytes %1 (van onbekend totaal aantal)</translation>
    </message>
    <message>
        <source>Received %1 of %2 bytes</source>
        <translation>Aantal ontvangen %1 van %2 bytes</translation>
    </message>
    <message>
        <source>Receiving reply</source>
        <translation>Antwoord wordt ontvangen</translation>
    </message>
    <message>
        <source>Referenced column wasn&apos;t found: </source>
        <translation>Verwijzende kolom is niet gevonden: </translation>
    </message>
    <message>
        <source>Regular expressions on numeric values don&apos;t make sense. Use comparison instead.</source>
        <translation>Reguliere expressie op numerieke waarden heeft geen zin. In plaats daarvan gebruik een vergelijking.</translation>
    </message>
    <message>
        <source>Release</source>
        <translation>Release</translation>
    </message>
    <message>
        <source>Release selected</source>
        <translation></translation>
    </message>
    <message>
        <source>Release the line</source>
        <translation>Release de lijn</translation>
    </message>
    <message>
        <source>Release vertex</source>
        <translation>Release hoekpunt</translation>
    </message>
    <message>
        <source>Response is complete</source>
        <translation>Antwoord is volledig</translation>
    </message>
    <message>
        <source>Rows</source>
        <translation>Rijen</translation>
    </message>
    <message>
        <source>ScaleBar</source>
        <translation>Schaalbalk</translation>
    </message>
    <message>
        <source>Select element</source>
        <translation>Selecteer element</translation>
    </message>
    <message>
        <source>Select line segment</source>
        <translation>Selecteer lijnsegment</translation>
    </message>
    <message>
        <source>Select new position</source>
        <translation>Selecteer nieuwe positie</translation>
    </message>
    <message>
        <source>Select point on line</source>
        <translation>Selecteer een punt op lijn</translation>
    </message>
    <message>
        <source>Select position on line</source>
        <translation>Selecteer positie op lijn</translation>
    </message>
    <message>
        <source>Select vertex</source>
        <translation>Selecteer hoekpunt</translation>
    </message>
    <message>
        <source>Sending request &apos;%1&apos;</source>
        <translation>Verzoek &apos;%1&apos; wordt verzonden</translation>
    </message>
    <message>
        <source>Shapefile to PostgreSQL/PostGIS Import Tool</source>
        <translation>Shape-bestand naar PostgreSQL/PostGIS omzetgereedschap</translation>
    </message>
    <message>
        <source>South</source>
        <translation>Zuid</translation>
    </message>
    <message>
        <source>SPIT</source>
        <translation>SPIT</translation>
    </message>
    <message>
        <source>Split the line</source>
        <translation>Deel de lijn</translation>
    </message>
    <message>
        <source> sq.deg.</source>
        <translation>graden in het vierkant.</translation>
    </message>
    <message>
        <source> sq ft</source>
        <translation>voet in het vierkant</translation>
    </message>
    <message>
        <source> sq mile</source>
        <translation>vierkante mijl</translation>
    </message>
    <message>
        <source>Table</source>
        <translation>Tabel</translation>
    </message>
    <message>
        <source>Temporary directory </source>
        <translation>Tijdelijke map</translation>
    </message>
    <message>
        <source>The current layer is not a vector layer</source>
        <translation>De huidige laag is geen vectorlaag</translation>
    </message>
    <message>
        <source>The data provider for this layer does not support the addition of features.</source>
        <translation>De databron-provider van deze laag staat het toevoegen van objecten niet toe.</translation>
    </message>
    <message>
        <source>The directory containing your dataset needs to be writeable!</source>
        <translation>De map/directory van uw dataset moet schrijfbaar zijn!</translation>
    </message>
    <message>
        <source>The inserted Ring crosses existing rings</source>
        <translation>De ingevoegde Ring kruist bestaande ringen</translation>
    </message>
    <message>
        <source>The inserted Ring is not a valid geometry</source>
        <translation>De ingevoegde Ring is geen geldige geometrie</translation>
    </message>
    <message>
        <source>The inserted Ring is not closed</source>
        <translation>De ingevoegde Ring is niet gesloten</translation>
    </message>
    <message>
        <source>The inserted Ring is not contained in a feature</source>
        <translation>De ingevoegde Ring is niet deel van een kaartobject</translation>
    </message>
    <message>
        <source>To identify features, you must choose an active layer by clicking on its name in the legend</source>
        <translation>Om kaartobjecten te identificeren, maak een laag actief door op de naam in het legendavenster te klikken</translation>
    </message>
    <message>
        <source>Tools for loading and importing GPS data</source>
        <translation>Gereedschappen voor het laden en importeren van GPS-data</translation>
    </message>
    <message>
        <source>Top</source>
        <translation>Boven</translation>
    </message>
    <message>
        <source>To select features, you must choose a vector layer by clicking on its name in the legend</source>
        <translation>Om kaartobjecten te selecteren moet u een vectorlaag kiezen door op de laag te klikken in het legendavenster</translation>
    </message>
    <message>
        <source>Unable to open </source>
        <translation>Openen mislukt van </translation>
    </message>
    <message>
        <source>Unable to save to file </source>
        <translation>Opslaan van bestand mislukt </translation>
    </message>
    <message>
        <source>Undo last point</source>
        <translation>Laatste punt ongedaan maken</translation>
    </message>
    <message>
        <source> unknown</source>
        <translation> onbekend</translation>
    </message>
    <message>
        <source>Version 0.1</source>
        <translation>Versie 0.1</translation>
    </message>
    <message>
        <source>Version 0.2</source>
        <translation>Versie 0.2</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Waarschuwing</translation>
    </message>
    <message>
        <source>West</source>
        <translation>West</translation>
    </message>
    <message>
        <source>WFS plugin</source>
        <translation>WFS-plugin</translation>
    </message>
    <message>
        <source>Where is &apos;</source>
        <translation>Waar is &apos;</translation>
    </message>
    <message>
        <source>Wrong editing tool</source>
        <translation>Verkeerd aanpasgereedschap</translation>
    </message>
    <message>
        <source>yes</source>
        <translation>ja</translation>
    </message>
    <message>
        <source>File could not been opened.</source>
        <translation type="obsolete">Bestand kan niet worden geopend.</translation>
    </message>
    <message>
        <source>Dxf2Shp Converter</source>
        <translation>Dxf2Shp Converter</translation>
    </message>
    <message>
        <source>Converts from dxf to shp file format</source>
        <translation>Converteert van dxf- naar shp-bestand</translation>
    </message>
    <message>
        <source>eVis</source>
        <translation type="obsolete">eVis</translation>
    </message>
    <message>
        <source>An event visualization pluigin for QGIS</source>
        <translation type="obsolete">Een &apos;event visualisatie&apos;-plugin voor QGIS</translation>
    </message>
    <message>
        <source>Version 0.7</source>
        <translation type="obsolete">Versie 0.7</translation>
    </message>
    <message>
        <source>This tool only supports vector data</source>
        <translation type="obsolete">Dit gereedschap ondersteund alleen vectordata</translation>
    </message>
    <message>
        <source>No active layers found</source>
        <translation type="obsolete">Geen aktieve laag gevonden</translation>
    </message>
    <message>
        <source>Interpolating...</source>
        <translation>Interpoleren...</translation>
    </message>
    <message>
        <source>Abort</source>
        <translation>Afbreken</translation>
    </message>
    <message>
        <source>Interpolation plugin</source>
        <translation>Interpolatie-plugin</translation>
    </message>
    <message>
        <source>A plugin for interpolation based on vertices of a vector layer</source>
        <translation>Een interpolatie-plugin gebaseerd op (hoek)punten van een vectorlaag</translation>
    </message>
    <message>
        <source>Version 0.001</source>
        <translation>Versie 0.001</translation>
    </message>
    <message>
        <source>SOS plugin</source>
        <translation type="obsolete">SOS plugin</translation>
    </message>
    <message>
        <source>Adds layers from Sensor Observation Service (SOS) to the QGIS canvas</source>
        <translation type="obsolete">Toevoegen van een &apos;Sensor Observation Service&apos; (SOS) aan de QGI-kaart</translation>
    </message>
    <message>
        <source>ERROR: Failed to created default style file as %1 Check file permissions and retry.</source>
        <translation>FOUT: Het aanmaken van de standaardstijl als %1 is niet gelukt. Controleer permissies en probeer het nog eens.</translation>
    </message>
    <message>
        <source> is not writeable.</source>
        <translation>is niet schrijfbaar.</translation>
    </message>
    <message>
        <source>Please adjust permissions (if possible) and try again.</source>
        <translation>Controleer permissies (indien mogelijk) en probeer het nog eens.</translation>
    </message>
    <message>
        <source>Couldn&apos;t load SIP module.</source>
        <translation>Problemen bij het laden van de SIP module.</translation>
    </message>
    <message>
        <source>Python support will be disabled.</source>
        <translation>Python-ondersteuning wordt uitgeschakeld.</translation>
    </message>
    <message>
        <source>Couldn&apos;t load PyQt4.</source>
        <translation>PyQt4 kon niet worden geladen.</translation>
    </message>
    <message>
        <source>Couldn&apos;t load PyQGIS.</source>
        <translation>PyQGIS kon niet worden geladen.</translation>
    </message>
    <message>
        <source>An error has occured while executing Python code:</source>
        <translation>Er is een fout opgetreden bij het uitvoeren van Python code:</translation>
    </message>
    <message>
        <source>Python version:</source>
        <translation>Python versie:</translation>
    </message>
    <message>
        <source>Python path:</source>
        <translation>Python pad:</translation>
    </message>
    <message>
        <source>An error occured during execution of following code:</source>
        <translation>Er is een fout opgetreden bij het uitvoeren van de volgende code:</translation>
    </message>
    <message>
        <source>Uncatched fatal GRASS error</source>
        <translation>Niet ondervangen fatale GRASS fout</translation>
    </message>
    <message>
        <source>Legend</source>
        <translation>Legenda</translation>
    </message>
    <message>
        <source>Coordinate Capture</source>
        <translation>Coördinaat Prikker</translation>
    </message>
    <message>
        <source>Capture mouse coordinates in different CRS</source>
        <translation>Prik coördinaten in afwijkend CRS</translation>
    </message>
    <message>
        <source>OGR Layer Converter</source>
        <translation>OGR Layer Converter</translation>
    </message>
    <message>
        <source>Translates vector layers between formats supported by OGR library</source>
        <translation>Transformeert vector-lagen tussen formaten die worden ondersteund door OGR</translation>
    </message>
    <message>
        <source>CRS Exception</source>
        <translation>CRS-fout</translation>
    </message>
    <message>
        <source>Selection extends beyond layer&apos;s coordinate system.</source>
        <translation>De selectie valt buiten het coördinatensysteem van de laag.</translation>
    </message>
    <message>
        <source>Loading style file </source>
        <translation>Laden van stijlbestand </translation>
    </message>
    <message>
        <source> failed because:</source>
        <translation> mislukt omdat:</translation>
    </message>
    <message>
        <source>Could not save symbology because:</source>
        <translation>Probleem bij het opslaan van symbologie omdat:</translation>
    </message>
    <message>
        <source>Unable to save to file. Your project may be corrupted on disk. Try clearing some space on the volume and check file permissions before pressing save again.</source>
        <translation>Bestand kon niet worden opgeslagen. Het project kan corrupt zijn geraakt. Probeer wat ruimte te krijgen op de schijf en controlleer schrijfrechten voordat u opnieuw probeert op te slaan.</translation>
    </message>
    <message>
        <source>Error Loading Plugin</source>
        <translation type="unfinished">Fout bij het Laden van Plugin</translation>
    </message>
    <message>
        <source>There was an error loading a plugin.The following diagnostic information may help the QGIS developers resolve the issue:
%1.</source>
        <translation type="unfinished">Er is een fout opgetreden bij het laden van een plugin. De volgende informatie kan de QGIS-ontwikkelaars helpen dit op te lossen:
%1.</translation>
    </message>
    <message>
        <source>Error when reading metadata of plugin </source>
        <translation type="unfinished">Fout bij het lezen van de metadata van plugin</translation>
    </message>
</context>
<context>
    <name>QgisApp</name>
    <message>
        <source>-</source>
        <comment>Remove all layers from overview map</comment>
        <translation>-</translation>
    </message>
    <message>
        <source>/</source>
        <comment>Capture Lines</comment>
        <translation>/</translation>
    </message>
    <message>
        <source>.</source>
        <comment>Capture Points</comment>
        <translation>.</translation>
    </message>
    <message>
        <source>+</source>
        <comment>Show all layers in the overview map</comment>
        <translation>+</translation>
    </message>
    <message>
        <source>%1 is an invalid layer and cannot be loaded.</source>
        <translation>%1 is een ongeldige laag en kan niet worden geladen.</translation>
    </message>
    <message>
        <source>About</source>
        <translation>Info</translation>
    </message>
    <message>
        <source>About QGIS</source>
        <translation>Info over QGIS</translation>
    </message>
    <message>
        <source>Add All To Overview</source>
        <translation type="obsolete">Alles aan Overzichtskaart Toevoegen</translation>
    </message>
    <message>
        <source>Add a PostGIS Layer</source>
        <translation>PostGIS-laag toevoegen</translation>
    </message>
    <message>
        <source>Add a PostGIS Layer...</source>
        <translation type="obsolete">PostGIS-laag toevoegen...</translation>
    </message>
    <message>
        <source>Add a Raster Layer</source>
        <translation>Rasterlaag toevoegen</translation>
    </message>
    <message>
        <source>Add a Raster Layer...</source>
        <translation type="obsolete">Rasterlaag toevoegen...</translation>
    </message>
    <message>
        <source>Add a Vector Layer</source>
        <translation>Vectorlaag toevoegen</translation>
    </message>
    <message>
        <source>Add a Vector Layer...</source>
        <translation type="obsolete">Vectorlaag toevoegen...</translation>
    </message>
    <message>
        <source>Add current layer to overview map</source>
        <translation>Huidige kaartlaag aan overzichtskaart toevoegen</translation>
    </message>
    <message>
        <source>Added locale options to options dialog.</source>
        <translation type="obsolete">Locale opties toegevoegd aan optiesdialoog.</translation>
    </message>
    <message>
        <source>Add Island</source>
        <translation>Eiland Toevoegen</translation>
    </message>
    <message>
        <source>Add Island to multipolygon</source>
        <translation>Eiland toevoegen aan multipolygoon</translation>
    </message>
    <message>
        <source>Add Ring</source>
        <translation>Ring Toevoegen</translation>
    </message>
    <message>
        <source>Add Vertex</source>
        <translation>Hoekpunt Toevoegen</translation>
    </message>
    <message>
        <source>Add Web Mapping Server Layer</source>
        <translation type="obsolete">Web Mapping Server-laag Toevoegen</translation>
    </message>
    <message>
        <source>Add WMS Layer...</source>
        <translation>WMS-laag Toevoegen...</translation>
    </message>
    <message>
        <source>A problem occured during deletion of features</source>
        <translation>Er is een fout opgetreden tijdens het verwijderen van kaartobjecten</translation>
    </message>
    <message>
        <source>Attributes</source>
        <translation>Attributen</translation>
    </message>
    <message>
        <source>Available Data Provider Plugins</source>
        <translation type="obsolete">Beschikbare Data Provider Plugins</translation>
    </message>
    <message>
        <source>B</source>
        <comment>Show Bookmarks</comment>
        <translation>B</translation>
    </message>
    <message>
        <source> Be sure to include the old project file, and state the version of QGIS you used to discover the error.</source>
        <translation> Stuur vooral het oude projectbestand mee, en vermeld de versie van QGIS toen u de fout vond.</translation>
    </message>
    <message>
        <source>Capture Line</source>
        <translation>Lijn Intekenen</translation>
    </message>
    <message>
        <source>Capture Lines</source>
        <translation>Lijnen Intekenen</translation>
    </message>
    <message>
        <source>Capture Point</source>
        <translation>Punt Intekenen</translation>
    </message>
    <message>
        <source>Capture Points</source>
        <translation>Punten Intekenen</translation>
    </message>
    <message>
        <source>Capture Polygon</source>
        <translation>Polygoon Intekenen</translation>
    </message>
    <message>
        <source>Capture Polygons</source>
        <translation>Polygonen Intekenen</translation>
    </message>
    <message>
        <source>Change various QGIS options</source>
        <translation>QGIS opties aanpassen</translation>
    </message>
    <message>
        <source>Check if your QGIS version is up to date (requires internet access)</source>
        <translation>Controleer of er een nieuwere QGIS versie beschikbaar is (vereist internet toegang)</translation>
    </message>
    <message>
        <source>Checking database</source>
        <translation>Database check</translation>
    </message>
    <message>
        <source>Checking provider plugins</source>
        <translation>Controle van Provider-plugins</translation>
    </message>
    <message>
        <source>Check Qgis Version</source>
        <translation>QGIS op updates controleren</translation>
    </message>
    <message>
        <source>Choose a filename to save the map image as</source>
        <translation type="obsolete">Kies een bestandsnaam voor het opslaan van de kaart als</translation>
    </message>
    <message>
        <source>Choose a filename to save the QGIS project file as</source>
        <translation type="obsolete">Kies een bestandsnaam om het QGIS-project op te slaan als</translation>
    </message>
    <message>
        <source>Choose a QGIS project file</source>
        <translation>Kies een QGIS-projectbestand</translation>
    </message>
    <message>
        <source>Choose a QGIS project file to open</source>
        <translation>Kies een QGIS-project om te openen</translation>
    </message>
    <message>
        <source>Click on features to identify them</source>
        <translation>Klik op een kaartobject voor identificatie</translation>
    </message>
    <message>
        <source>Clipboard contents set to: </source>
        <translation type="obsolete">Inhoud klembord gezet op:</translation>
    </message>
    <message>
        <source>
Compiled against Qt </source>
        <translation type="obsolete">
Gecompileerd met Qt-versie</translation>
    </message>
    <message>
        <source>Connection refused - server may be down</source>
        <translation>Verbinding maken mislukt - server kan uit staan</translation>
    </message>
    <message>
        <source>Copy Features</source>
        <translation>Kaartobjecten Kopiëren</translation>
    </message>
    <message>
        <source>Copy selected features</source>
        <translation>Kopieer geselecteerde kaartobjecten</translation>
    </message>
    <message>
        <source>Crash fix for 2.5D shapefiles</source>
        <translation type="obsolete">Reparatie voor 2.5D shapefile crash</translation>
    </message>
    <message>
        <source>Create a New Vector Layer</source>
        <translation>Nieuwe Vectorlaag Aanmaken</translation>
    </message>
    <message>
        <source>Ctrl+-</source>
        <comment>Zoom Out</comment>
        <translation>Ctrl+-</translation>
    </message>
    <message>
        <source>Ctrl+?</source>
        <comment>Help Documentation (Mac)</comment>
        <translation>Ctrl+?</translation>
    </message>
    <message>
        <source>Ctrl+/</source>
        <comment>Capture Polygons</comment>
        <translation>Ctrl+/</translation>
    </message>
    <message>
        <source>Ctrl++</source>
        <comment>Zoom In</comment>
        <translation>Ctrl++</translation>
    </message>
    <message>
        <source>Ctrl+A</source>
        <comment>Save Project under a new name</comment>
        <translation type="obsolete">Ctrl+A</translation>
    </message>
    <message>
        <source>Ctrl+B</source>
        <comment>New Bookmark</comment>
        <translation>Ctrl+B</translation>
    </message>
    <message>
        <source>Ctrl+D</source>
        <comment>Remove a Layer</comment>
        <translation>Ctrl+d</translation>
    </message>
    <message>
        <source>Ctrl+F</source>
        <comment>Zoom to selection</comment>
        <translation type="obsolete">Ctrl+F</translation>
    </message>
    <message>
        <source>Ctrl+H</source>
        <comment>QGIS Home Page</comment>
        <translation>Ctrl+H</translation>
    </message>
    <message>
        <source>Ctrl+I</source>
        <comment>Save map as image</comment>
        <translation type="obsolete">Ctrl+I</translation>
    </message>
    <message>
        <source>Ctrl+J</source>
        <comment>Measure an Area</comment>
        <translation type="obsolete">Ctrl+J</translation>
    </message>
    <message>
        <source>Ctrl+M</source>
        <comment>Measure a Line</comment>
        <translation type="obsolete">Ctrl+M</translation>
    </message>
    <message>
        <source>Ctrl+N</source>
        <comment>New Project</comment>
        <translation>Ctrl+N</translation>
    </message>
    <message>
        <source>Ctrl+O</source>
        <comment>Open a Project</comment>
        <translation>Ctrl+O</translation>
    </message>
    <message>
        <source>Ctrl+P</source>
        <comment>Print</comment>
        <translation type="obsolete">Ctrl+P</translation>
    </message>
    <message>
        <source>Ctrl+Q</source>
        <comment>Exit QGIS</comment>
        <translation>Ctrl+Q</translation>
    </message>
    <message>
        <source>Ctrl+R</source>
        <comment>Refresh Map</comment>
        <translation>Ctrl+R</translation>
    </message>
    <message>
        <source>Ctrl+S</source>
        <comment>Save Project</comment>
        <translation>Ctrl+S</translation>
    </message>
    <message>
        <source>Ctrl+T</source>
        <comment>Hide most toolbars</comment>
        <translation type="obsolete">Ctrl+T</translation>
    </message>
    <message>
        <source>Current map scale</source>
        <translation>Huidige kaartschaal</translation>
    </message>
    <message>
        <source>Current map scale (formatted as x:y)</source>
        <translation>Huidige kaartschaal (getoond als x:y)</translation>
    </message>
    <message>
        <source>Custom Projection...</source>
        <translation type="obsolete">Aangepaste projectie...</translation>
    </message>
    <message>
        <source>Cut Features</source>
        <translation>Kaartobjecten Knippen</translation>
    </message>
    <message>
        <source>Cut selected features</source>
        <translation>Geselecteerde kaartobjecten knippen</translation>
    </message>
    <message>
        <source>D</source>
        <comment>Add a PostGIS Layer</comment>
        <translation>D</translation>
    </message>
    <message>
        <source>Data provider does not support deleting features</source>
        <translation>Dataprovider heeft geen mogelijkheid om kaartobjecten te verwijderen</translation>
    </message>
    <message>
        <source>Delete Selected</source>
        <translation>Geselecteerd Object(en) Verwijderen</translation>
    </message>
    <message>
        <source>Delete Vertex</source>
        <translation>Hoekpunt Verwijderen</translation>
    </message>
    <message>
        <source>Deleting features only works on vector layers</source>
        <translation>Verwijderen van kaartobjecten werkt alleen met vectorlagen</translation>
    </message>
    <message>
        <source>Description: %1</source>
        <translation type="obsolete">Omschrijving: %1</translation>
    </message>
    <message>
        <source>Digitizing</source>
        <translation>Digitaliseren</translation>
    </message>
    <message>
        <source>Displays the current map scale</source>
        <translation>Toon de huidige kaartschaal</translation>
    </message>
    <message>
        <source>Do you want to save the current project?</source>
        <translation>Wilt het huidige project opslaan?</translation>
    </message>
    <message>
        <source>Enter a name for the new bookmark:</source>
        <translation>Geef een naam voor de nieuwe favoriet:</translation>
    </message>
    <message>
        <source>Enter the full path if the browser is not in your PATH.
</source>
        <translation type="obsolete">Voer het volledige pad in als de browser niet in uw PATH staat.
</translation>
    </message>
    <message>
        <source>Enter the name of a web browser to use (eg. konqueror).
</source>
        <translation type="obsolete">Voer de naam in van de te gebruiken webbrowser (bv. konqureror).
</translation>
    </message>
    <message>
        <source>Error</source>
        <translation>Fout</translation>
    </message>
    <message>
        <source>Error Loading Plugin</source>
        <translation type="obsolete">Fout bij het Laden van Plugin</translation>
    </message>
    <message>
        <source>Error when reading metadata of plugin </source>
        <translation type="obsolete">Fout bij het lezen van de metadata van plugin</translation>
    </message>
    <message>
        <source>Exit</source>
        <translation>Afsluiten</translation>
    </message>
    <message>
        <source>Exit QGIS</source>
        <translation>QGIS afsluiten</translation>
    </message>
    <message>
        <source>Extents: </source>
        <translation>Extents:</translation>
    </message>
    <message>
        <source>F</source>
        <comment>Zoom to Full Extents</comment>
        <translation>F</translation>
    </message>
    <message>
        <source>F1</source>
        <comment>Help Documentation</comment>
        <translation>F1</translation>
    </message>
    <message>
        <source>File</source>
        <translation>Bestand</translation>
    </message>
    <message>
        <source>&amp;File</source>
        <translation>&amp;Bestand</translation>
    </message>
    <message>
        <source>H</source>
        <comment>Hide all layers</comment>
        <translation>H</translation>
    </message>
    <message>
        <source>Help</source>
        <translation>Help</translation>
    </message>
    <message>
        <source>&amp;Help</source>
        <translation>&amp;Help</translation>
    </message>
    <message>
        <source>Help Contents</source>
        <translation>Inhoudsopgave</translation>
    </message>
    <message>
        <source>Help Documentation</source>
        <translation>Help Handleiding</translation>
    </message>
    <message>
        <source>Hide all layers</source>
        <translation>Verberg alle lagen</translation>
    </message>
    <message>
        <source>Hide All Layers</source>
        <translation>Verberg Alle Lagen</translation>
    </message>
    <message>
        <source>Hide most toolbars</source>
        <translation type="obsolete">Verberg werkbalken</translation>
    </message>
    <message>
        <source>http://www.gnu.org/licenses</source>
        <translation type="obsolete">http://www.gnu.org/licenses</translation>
    </message>
    <message>
        <source>I</source>
        <comment>Click on features to identify them</comment>
        <translation>I</translation>
    </message>
    <message>
        <source>Identify Features</source>
        <translation>Objecten Identificeren</translation>
    </message>
    <message>
        <source>Improvements to the GeoReferencer</source>
        <translation type="obsolete">Verbeteringen aan de GeoReferencer</translation>
    </message>
    <message>
        <source>Initializing file filters</source>
        <translation>Bestandsfilters worden geinitialiseerd</translation>
    </message>
    <message>
        <source>In Overview</source>
        <translation type="obsolete">In Kaartoverzicht</translation>
    </message>
    <message>
        <source>Invalid Data Source</source>
        <translation>Ongeldige Data</translation>
    </message>
    <message>
        <source>Invalid Layer</source>
        <translation>Ongeldige Laag</translation>
    </message>
    <message>
        <source>Invalid scale</source>
        <translation>Ongeldige schaal</translation>
    </message>
    <message>
        <source> is not a supported raster data source</source>
        <translation>is een ongeldige raster databron</translation>
    </message>
    <message>
        <source>is not a valid or recognized data source</source>
        <translation>is geen geldige of herkende data</translation>
    </message>
    <message>
        <source> is not a valid or recognized raster data source</source>
        <translation>is geen geldige of herkenbare raster databron</translation>
    </message>
    <message>
        <source>&amp;Layer</source>
        <translation>&amp;Kaartlagen</translation>
    </message>
    <message>
        <source>Layer is not valid</source>
        <translation>Ongeldige kaartlaag</translation>
    </message>
    <message>
        <source>Layer not editable</source>
        <translation>Laag is niet bewerkbaar</translation>
    </message>
    <message>
        <source>Manage custom projections</source>
        <translation type="obsolete">Aangepaste projecties instellen</translation>
    </message>
    <message>
        <source>Manage Layers</source>
        <translation>Kaartlagen aanpassen</translation>
    </message>
    <message>
        <source>Many new GRASS tools added (with thanks to http://faunalia.it/)</source>
        <translation type="obsolete">Vele nieuwe GRASS gereedschappen toegevoegd (met dank aan http://faunalia.it/)</translation>
    </message>
    <message>
        <source>Map canvas. This is where raster and vector layers are displayed when added to the map</source>
        <translation>Kaart venster. Hier worden raster- en vectorkaartlagen afgebeeld na toevoegen aan de kaart</translation>
    </message>
    <message>
        <source>Map Composer updates</source>
        <translation type="obsolete">Map Compositie updates</translation>
    </message>
    <message>
        <source>Map coordinates at mouse cursor position</source>
        <translation>Kaartcoordinaten onder muisaanwijzer</translation>
    </message>
    <message>
        <source>Map legend that displays all the layers currently on the map canvas. Click on the check box to turn a layer on or off. Double click on a layer in the legend to customize its appearance and set other properties.</source>
        <translation>Legenda welke alle lagen in de huidige kaart toont. Klik op het selectievakje om een laag zichtbaar te maken, of te verbergen.
Dubbelklik op een laag in de legenda om de verschijning of andere eigenschappen aan te passen .</translation>
    </message>
    <message>
        <source>Map Navigation</source>
        <translation>Kaart Navigatie</translation>
    </message>
    <message>
        <source>Map overview canvas. This canvas can be used to display a locator map that shows the current extent of the map canvas. The current extent is shown as a red rectangle. Any layer on the map can be added to the overview canvas.</source>
        <translation>Kaartoverzicht. Dit vlak kan worden gebruikt om een referentiekaart met de huidige extent van de kaart af te beelden. De huidige exent wordt getoond als een rode rechthoek. Elke kaartlaag in de kaart kan aan het kaartoverzicht worden toegevoegd.</translation>
    </message>
    <message>
        <source>Map Tips</source>
        <translation>Kaart Tips</translation>
    </message>
    <message>
        <source>Measure a Line</source>
        <translation>Lijn Opmeten</translation>
    </message>
    <message>
        <source>Measure an Area</source>
        <translation>Vlak Opmeten</translation>
    </message>
    <message>
        <source>Measure Area</source>
        <translation>Vlak  Opmeten</translation>
    </message>
    <message>
        <source>Measure Line </source>
        <translation>Lijn meten </translation>
    </message>
    <message>
        <source>Move Feature</source>
        <translation>Kaartobject verplaatsen</translation>
    </message>
    <message>
        <source>Move Vertex</source>
        <translation>Hoekpunt Verschuiven</translation>
    </message>
    <message>
        <source>N</source>
        <comment>Create a New Vector Layer</comment>
        <translation>N</translation>
    </message>
    <message>
        <source>Name</source>
        <translation type="obsolete">Naam</translation>
    </message>
    <message>
        <source>Name: %1</source>
        <translation type="obsolete">Naam: %1</translation>
    </message>
    <message>
        <source>Network error while communicating with server</source>
        <translation>Netwerkfout tijdens het communiceren met de server</translation>
    </message>
    <message>
        <source>New Bookmark</source>
        <translation>Nieuwe Favoriet</translation>
    </message>
    <message>
        <source>New Bookmark...</source>
        <translation>Nieuwe Favoriet...</translation>
    </message>
    <message>
        <source>New features</source>
        <translation>Nieuwe Kaartobjecten</translation>
    </message>
    <message>
        <source>&amp;New Project</source>
        <translation>&amp;Nieuw Project</translation>
    </message>
    <message>
        <source>New Project</source>
        <translation>Nieuw Project</translation>
    </message>
    <message>
        <source>New Vector Layer...</source>
        <translation>Nieuwe Vectorlaag...</translation>
    </message>
    <message>
        <source>No Layer Selected</source>
        <translation>Geen Kaartlaag Geselecteerd</translation>
    </message>
    <message>
        <source>No MapLayer Plugins</source>
        <translation type="obsolete">Geen KaartLaag-plugins</translation>
    </message>
    <message>
        <source>No MapLayer plugins in ../plugins/maplayer</source>
        <translation type="obsolete">Geen KaartLaag-plugins in ../plugins/maplayer</translation>
    </message>
    <message>
        <source>No Plugins</source>
        <translation type="obsolete">Geen Plugins</translation>
    </message>
    <message>
        <source>No plugins found in ../plugins. To test plugins, start qgis from the src directory</source>
        <translation type="obsolete">Geen plugins gevonden in ../plugins. Om plugins te testen, start QGIS vanuit de src directory</translation>
    </message>
    <message>
        <source> (no PostgreSQL support)</source>
        <translation type="obsolete"> (geen PostgreSQL ondersteuning)</translation>
    </message>
    <message>
        <source>No Vector Layer Selected</source>
        <translation>Geen Vectorlaag Geselecteerd</translation>
    </message>
    <message>
        <source>O</source>
        <comment>Add current layer to overview map</comment>
        <translation>O</translation>
    </message>
    <message>
        <source>Open a GDAL Supported Raster Data Source</source>
        <translation>Open een GDAL Ondersteunde Raster Databron</translation>
    </message>
    <message>
        <source>Open an OGR Supported Vector Layer</source>
        <translation>Open een OGR ondersteunde Vectorlaag</translation>
    </message>
    <message>
        <source>Open a Project</source>
        <translation>Open een project</translation>
    </message>
    <message>
        <source>&amp;Open Project...</source>
        <translation>&amp;Open Project...</translation>
    </message>
    <message>
        <source>&amp;Open Recent Projects</source>
        <translation>&amp;Open Recente Projecten</translation>
    </message>
    <message>
        <source>Open Table</source>
        <translation type="obsolete">Tabel Openen</translation>
    </message>
    <message>
        <source>Open the plugin manager</source>
        <translation>Open Plugin-instellingen</translation>
    </message>
    <message>
        <source>Options...</source>
        <translation>Opties...</translation>
    </message>
    <message>
        <source>P</source>
        <comment>Set project properties</comment>
        <translation>P</translation>
    </message>
    <message>
        <source>Pan Map</source>
        <translation>Kaart verschuiven</translation>
    </message>
    <message>
        <source>Pan the map</source>
        <translation>Verschuif de kaart</translation>
    </message>
    <message>
        <source>Paste Features</source>
        <translation>Kaartobjecten Plakken</translation>
    </message>
    <message>
        <source>Paste selected features</source>
        <translation>Geselecteerde kaartobjecten plakken</translation>
    </message>
    <message>
        <source>&lt;p&gt;Even though QGIS developers try to maintain backwards compatibility, some of the information from the old project file might be lost.</source>
        <translation>&lt;p&gt;Hoewel de QGIS-ontwikkelaars proberen rekening te houden met oudere versies, kan het zijn dat informatie uit het oude projectbestand verloren gaat.</translation>
    </message>
    <message>
        <source>Plugin %1 is named %2</source>
        <translation type="obsolete">Plugin %1 heeft als naam %2</translation>
    </message>
    <message>
        <source>Plugin Information</source>
        <translation type="obsolete">Plugin Informatie</translation>
    </message>
    <message>
        <source>Plugin Manager...</source>
        <translation type="obsolete">Plugin-instellingen...</translation>
    </message>
    <message>
        <source>&amp;Plugins</source>
        <translation>&amp;Plugins</translation>
    </message>
    <message>
        <source>Plugins</source>
        <translation>Plugins</translation>
    </message>
    <message>
        <source>Print</source>
        <translation type="obsolete">Afdrukken</translation>
    </message>
    <message>
        <source>&amp;Print...</source>
        <translation type="obsolete">&amp;Afdrukken...</translation>
    </message>
    <message>
        <source>Problem deleting features</source>
        <translation>Fout bij het verwijderen van kaartobjecten</translation>
    </message>
    <message>
        <source>Progress bar that displays the status of rendering layers and other time-intensive operations</source>
        <translation>De voortgangsindicator toont de status van het (her)tekenen van kaartlagen en andere tijdsintensieve operaties</translation>
    </message>
    <message>
        <source>Project file is older</source>
        <translation>Projectbestand is ouder</translation>
    </message>
    <message>
        <source>Projection status - Click to open projection dialog</source>
        <translation type="obsolete">Projectie status - Klikken om de projectiedialoog te openen</translation>
    </message>
    <message>
        <source>Project Properties...</source>
        <translation>Projectinstellingen...</translation>
    </message>
    <message>
        <source>Provider does not support deletion</source>
        <translation>Dataprovider heeft geen mogelijkheid tot verwijderen</translation>
    </message>
    <message>
        <source>&lt;p&gt;This project file was saved by an older version of QGIS.</source>
        <translation>&lt;p&gt;Dit projectbestand is opgeslagen in een oudere versie van QGIS.</translation>
    </message>
    <message>
        <source>&lt;p&gt;To remove this warning when opening an older project file, uncheck the box &apos;%5&apos; in the %4 menu.</source>
        <translation>&lt;p&gt;Om deze waarschuwingsmelding niet meer te zien, pas de optie aan in &apos;%5&apos; in het %4 menu.</translation>
    </message>
    <message>
        <source>&lt;p&gt;Version of the project file: %1&lt;br&gt;Current version of QGIS: %2</source>
        <translation>&lt;p&gt;Versie van het projectbestand: %1&lt;br&gt;Huidige QGIS-versie: %2</translation>
    </message>
    <message>
        <source>Python bindings - This is the major focus of this release it is now possible to create plugins using python. It is also possible to create GIS enabled applications written in python that use the QGIS libraries.</source>
        <translation type="obsolete">Python bindingen - Dit is de focus van deze release. Het is nu mogelijk om met python plugins te maken. Het is ook mogelijk om GIS-toepassingen te bouwen die gebruik maken van de QGIS-softwarebibliotheken.</translation>
    </message>
    <message>
        <source>Python console</source>
        <translation type="obsolete">Python console</translation>
    </message>
    <message>
        <source>Python error</source>
        <translation type="obsolete">Python fout</translation>
    </message>
    <message>
        <source>QGIS Browser Selection</source>
        <translation type="obsolete">QGIS Browser Selectie</translation>
    </message>
    <message>
        <source>QGIS - Changes in SVN Since Last Release</source>
        <translation>QGIS - Veranderingen in SVN sinds de Laatste Release</translation>
    </message>
    <message>
        <source>Qgis Home Page</source>
        <translation type="obsolete">QGIS Start Pagina</translation>
    </message>
    <message>
        <source>QGIS Home Page</source>
        <translation>QGIS Start Pagina</translation>
    </message>
    <message>
        <source>QGis loaded the following plugin:</source>
        <translation type="obsolete">QGIS heeft de volgende plugin geladen:</translation>
    </message>
    <message>
        <source>QGIS Project Read Error</source>
        <translation>QGIS-projectbestand Inleesfout</translation>
    </message>
    <message>
        <source>QGIS Ready!</source>
        <translation>QGIS Gereed!</translation>
    </message>
    <message>
        <source>QGIS server was not found</source>
        <translation>QGIS server is niet gevonden</translation>
    </message>
    <message>
        <source>QGIS: Unable to load project</source>
        <translation>QGIS: Fout bij laden van project</translation>
    </message>
    <message>
        <source>QGIS Version Information</source>
        <translation>QGIS Versie Informatie</translation>
    </message>
    <message>
        <source>QGIS was unable to load the plugin from: %1</source>
        <translation type="obsolete">QGIS kon de plugin niet alden van: %1</translation>
    </message>
    <message>
        <source>Quantum GIS - </source>
        <translation>Quantum GIS - </translation>
    </message>
    <message>
        <source>Quantum GIS is licensed under the GNU General Public License</source>
        <translation type="obsolete">Quantum Gis valt onder de GNU General Public License</translation>
    </message>
    <message>
        <source>R</source>
        <comment>Add a Raster Layer</comment>
        <translation>R</translation>
    </message>
    <message>
        <source>Reading settings</source>
        <translation>Voorkeuren inlezen</translation>
    </message>
    <message>
        <source>Ready</source>
        <translation>Gereed</translation>
    </message>
    <message>
        <source>Refresh</source>
        <translation>Bijwerken</translation>
    </message>
    <message>
        <source>Refresh Map</source>
        <translation>Kaart Bijwerken</translation>
    </message>
    <message>
        <source>Remove a Layer</source>
        <translation>Laag verwijderen</translation>
    </message>
    <message>
        <source>Remove All From Overview</source>
        <translation>Verwijder Alles Van Overzichtskaart</translation>
    </message>
    <message>
        <source>Remove all layers from overview map</source>
        <translation>Verwijder alle lagen van de overzichtskaart</translation>
    </message>
    <message>
        <source>Removed automake build system - QGIS now needs CMake for compilation.</source>
        <translation type="obsolete">Automake build systeem verwijderd - QGIS gebruikt nu CMake bij het compileren.</translation>
    </message>
    <message>
        <source>Remove Layer</source>
        <translation>Laag verwijderen</translation>
    </message>
    <message>
        <source>Render</source>
        <translation>(Her)teken</translation>
    </message>
    <message>
        <source>Restoring loaded plugins</source>
        <translation>Plugins worden herladen</translation>
    </message>
    <message>
        <source>Restoring window state</source>
        <translation>Schermstatus herbouwen</translation>
    </message>
    <message>
        <source>, running against Qt </source>
        <translation type="obsolete">, draaiend met Qt-versie</translation>
    </message>
    <message>
        <source>S</source>
        <comment>Show all layers</comment>
        <translation>S</translation>
    </message>
    <message>
        <source>Save?</source>
        <translation>Opslaan?</translation>
    </message>
    <message>
        <source>Save As</source>
        <translation>Opslaan Als</translation>
    </message>
    <message>
        <source>Save as Image...</source>
        <translation>Opslaan als afbeelding...</translation>
    </message>
    <message>
        <source>Saved map image to</source>
        <translation>Kaartafbeelding opgeslagen als</translation>
    </message>
    <message>
        <source>Saved project to:</source>
        <translation>Sla project op naar:</translation>
    </message>
    <message>
        <source>Save map as image</source>
        <translation>Kaart opslaan als afbeelding</translation>
    </message>
    <message>
        <source>&amp;Save Project</source>
        <translation>Project op&amp;slaan</translation>
    </message>
    <message>
        <source>Save Project</source>
        <translation>Project Opslaan</translation>
    </message>
    <message>
        <source>Save Project &amp;As...</source>
        <translation>Project opslaan &amp;als...</translation>
    </message>
    <message>
        <source>Save Project under a new name</source>
        <translation>Project opslaan onder nieuwe naam</translation>
    </message>
    <message>
        <source>Scale </source>
        <translation>Schaal </translation>
    </message>
    <message>
        <source>Select Features</source>
        <translation>Kaartobjecten Selecteren</translation>
    </message>
    <message>
        <source>Set project properties</source>
        <translation>Projectinstellingen aanpassen</translation>
    </message>
    <message>
        <source>&amp;Settings</source>
        <translation>E&amp;xtra</translation>
    </message>
    <message>
        <source>Setting up the GUI</source>
        <translation>De schermen worden opgebouwd</translation>
    </message>
    <message>
        <source>Show all layers</source>
        <translation>Toon alle lagen</translation>
    </message>
    <message>
        <source>Show All Layers</source>
        <translation>Toon Alle Lagen</translation>
    </message>
    <message>
        <source>Show all layers in the overview map</source>
        <translation>Toon alle lagen in de overzichtskaart</translation>
    </message>
    <message>
        <source>Show Bookmarks</source>
        <translation>Toon Favorieten</translation>
    </message>
    <message>
        <source>Show information about a feature when the mouse is hovered over it</source>
        <translation>Toon informatie over een kaartobject als u de muis eroverheen beweegt</translation>
    </message>
    <message>
        <source>Show most toolbars</source>
        <translation type="obsolete">Alle werkbalken weergeven</translation>
    </message>
    <message>
        <source>Shows the map coordinates at the current cursor position. The display is continuously updated as the mouse is moved.</source>
        <translation>Toont de kaartcoördinaten op de huidige cursorpositie. De aangegeven positie veranderd mee terwijl de muis beweegd.</translation>
    </message>
    <message>
        <source>Split Features</source>
        <translation>Kaartobjecten splitsen</translation>
    </message>
    <message>
        <source>Starting Python</source>
        <translation>Python wordt gestart</translation>
    </message>
    <message>
        <source>T</source>
        <comment>Show most toolbars</comment>
        <translation type="obsolete">T</translation>
    </message>
    <message>
        <source>The current layer is not editable. Choose &apos;Start editing&apos; in the digitizing toolbar.</source>
        <translation>De huidige kaartlaag is niet bewerkbaar. Kies &apos;Start bewerken&apos; in de digitaliseerwerkbalk.</translation>
    </message>
    <message>
        <source>The layer is not a valid layer and can not be added to the map</source>
        <translation>De kaartlaag is ongeldig en kan niet worden toegevoegd aan de kaart</translation>
    </message>
    <message>
        <source>The QGIS libraries have been refactored and better organised.</source>
        <translation type="obsolete">De QGIS software-bibliotheken zijn gerefactored en beter georganiseerd.</translation>
    </message>
    <message>
        <source>There is a new version of QGIS available</source>
        <translation>Er is een recentere versie van QGIS beschikbaar</translation>
    </message>
    <message>
        <source>There was an error loading %1.</source>
        <translation type="obsolete">Er was een fout bhij het laden van %1.</translation>
    </message>
    <message>
        <source>This icon shows whether on the fly projection is enabled or not. Click the icon to bring up the project properties dialog to alter this behaviour.</source>
        <translation type="obsolete">Dit icoon toon of de gelijktijdige herprojectie is aangevinkt of niet. Na het aanklikken van de icoon verschijnt de projectiedialoog om dit gedrag aan te passen.</translation>
    </message>
    <message>
        <source>To delete features, you must select a vector layer in the legend</source>
        <translation>Om kaartobjecten te verwijderen, selecter eerst een vectorlaag in de legenda</translation>
    </message>
    <message>
        <source>Toggle editing</source>
        <translation>Modus objectinvoer omzetten</translation>
    </message>
    <message>
        <source>Toggle map rendering</source>
        <translation>Kaart (her)tekenen omzetten</translation>
    </message>
    <message>
        <source>Toggles the editing state of the current layer</source>
        <translation>Zet de modus van de objectinvoer om</translation>
    </message>
    <message>
        <source> To improve the quality of QGIS, we appreciate if you file a bug report at %3.</source>
        <translation> Om de kwaliteit van QGIS te verhogen, stellen we het op prijs als u een foutenrapportage stuurt aan %3.</translation>
    </message>
    <message>
        <source>Toolbar Visibility...</source>
        <translation type="obsolete">Zichtbaarheid Werkbalk...</translation>
    </message>
    <message>
        <source>Try to find missing layers?</source>
        <translation>Missende lagen proberen te vinden?</translation>
    </message>
    <message>
        <source>&lt;tt&gt;Settings:Options:General&lt;/tt&gt;</source>
        <comment>Menu path to setting options</comment>
        <translation>&lt;tt&gt;Extra:Opties:Algemeen&lt;/tt&gt;</translation>
    </message>
    <message>
        <source>Unable to communicate with QGIS Version server</source>
        <translation>Problemen met de communicatie met de QGIS Versie server</translation>
    </message>
    <message>
        <source>Unable to create the bookmark. Your user database may be missing or corrupted</source>
        <translation>Aanmaken van de favoriet is mislukt. Uw gebruiksdatabase wordt niet gevonden of is corrupt</translation>
    </message>
    <message>
        <source>Unable to get current version information from server</source>
        <translation>Ophalen van versieinformatie op dit moment niet mogelijk</translation>
    </message>
    <message>
        <source>Unable to Load Plugin</source>
        <translation type="obsolete">Fout bij het Ladne van Plugin</translation>
    </message>
    <message>
        <source>Unable to load project </source>
        <translation>Fout bij het laden van project</translation>
    </message>
    <message>
        <source>Unable to open project</source>
        <translation>Project openen mislukt</translation>
    </message>
    <message>
        <source>Unable to save project</source>
        <translation>Opslaan van project mislukt</translation>
    </message>
    <message>
        <source>Unable to save project </source>
        <translation>Opslaan van project mislukt</translation>
    </message>
    <message>
        <source>Unable to save project to </source>
        <translation>Fout bij het opslaan van project naar</translation>
    </message>
    <message>
        <source>Unknown network socket error</source>
        <translation>Onbekende netwerk (socket) fout</translation>
    </message>
    <message>
        <source>Unsupported Data Source</source>
        <translation>Niet onderstuende Databron</translation>
    </message>
    <message>
        <source>V</source>
        <comment>Add a Vector Layer</comment>
        <translation>V</translation>
    </message>
    <message>
        <source>Version</source>
        <translation>Versie</translation>
    </message>
    <message>
        <source>Version </source>
        <translation type="obsolete">Versie </translation>
    </message>
    <message>
        <source>Version: %1</source>
        <translation type="obsolete">Versie: %1</translation>
    </message>
    <message>
        <source>&amp;View</source>
        <translation>&amp;Beeld</translation>
    </message>
    <message>
        <source>W</source>
        <comment>Add Web Mapping Server Layer</comment>
        <translation type="obsolete">W</translation>
    </message>
    <message>
        <source>Warn me when opening a project file saved with an older version of QGIS</source>
        <translation>Geef een waarschuwing bij het openen van een projectfile uit een oudere versie van QGIS</translation>
    </message>
    <message>
        <source>When checked, the map layers are rendered in response to map navigation commands and other events. When not checked, no rendering is done. This allows you to add a large number of layers and symbolize them before rendering.</source>
        <translation>Indien aangevinkt worden de kaartlagen (her)tekend bij elke kaart-zoom of -verschuiving en andere akties. Indien niet aangevinkt wordt niet direkt hertekend. Dit maakt het mogelijk om een groot aantal kaartlagen toe te voegen en de symboliek in te stellen voordat de kaart wordt hertekend.</translation>
    </message>
    <message>
        <source> When saving this project file, QGIS will update it to the latest version, possibly rendering it useless for older versions of QGIS.</source>
        <translation> Wanneer u dit project nu opslaat, zal het projectbestand worden opgeslagen in de laatste versie, en hiermee mogelijk onbruikbaar worden voor oudere versies van QGIS.</translation>
    </message>
    <message>
        <source> with PostgreSQL support</source>
        <translation type="obsolete"> met PostgreSQL ondersteuning</translation>
    </message>
    <message>
        <source>Would you like more information?</source>
        <translation>Wilt u meer informatie?</translation>
    </message>
    <message>
        <source>You are running a development version of QGIS</source>
        <translation>U gebruikt een ontwikkel-vesie van QGIS</translation>
    </message>
    <message>
        <source>You are running the current version of QGIS</source>
        <translation>U gebruikt de laatste versie van QGIS</translation>
    </message>
    <message>
        <source>You can change this option later by selecting Options from the Settings menu (Help Browser tab).</source>
        <translation type="obsolete">U kunt deze optie later aanpassen door Opties te selectie in het Extra menu (Tab Helpbrowser).</translation>
    </message>
    <message>
        <source>Zoom Full</source>
        <translation>Volledig Uitzoomen</translation>
    </message>
    <message>
        <source>Zoom In</source>
        <translation>Inzoomen</translation>
    </message>
    <message>
        <source>Zoom Last</source>
        <translation>Laatste Zoomniveau</translation>
    </message>
    <message>
        <source>Zoom Out</source>
        <translation>Uitzoomen</translation>
    </message>
    <message>
        <source>Zoom to Full Extents</source>
        <translation>Uitzoomen Tot Maximale Extent</translation>
    </message>
    <message>
        <source>Zoom to Last Extent</source>
        <translation>Zoom naar Laatste Extent</translation>
    </message>
    <message>
        <source>Zoom to Layer</source>
        <translation>Op Kaartlaag Inzoomen</translation>
    </message>
    <message>
        <source>Zoom To Layer</source>
        <translation type="obsolete">Op Kaartlaag Inzoomen</translation>
    </message>
    <message>
        <source>Zoom to selection</source>
        <translation type="obsolete">Inzoomen op selectie</translation>
    </message>
    <message>
        <source>Zoom To Selection</source>
        <translation type="obsolete">Inzoomen Op Selectie</translation>
    </message>
    <message>
        <source>Toggle full screen mode</source>
        <translation type="obsolete">Volledig scherm aan/uit</translation>
    </message>
    <message>
        <source>Ctrl-F</source>
        <comment>Toggle fullscreen mode</comment>
        <translation>Ctrl+V</translation>
    </message>
    <message>
        <source>Toggle fullscreen mode</source>
        <translation>Volledig scherm aan/uit</translation>
    </message>
    <message>
        <source>This release candidate includes over 40 bug fixes and enchancements over the QGIS 0.9.1 release. In addition we have added the following new features:</source>
        <translation type="obsolete">Deze rc versie bevat meer dan 40 verbeteringen ten opzicht van de 0.9.1 versie. Verder zijn de volgende nieuwe functionaliteiten toegevoegd:</translation>
    </message>
    <message>
        <source>Imrovements to digitising capabilities.</source>
        <translation type="obsolete">UItbreiding van de digitaliseringsmogelijkheden.</translation>
    </message>
    <message>
        <source>Supporting default and defined styles (.qml) files for file based vector layers. With styles you can save the symbolisation and other settings associated with a vector layer and they will be loaded whenever you load that layer.</source>
        <translation type="obsolete">Ondersteuning van stijl-bestanden (.qml) voor lagen uit vectorbestanden. Met stijlen kunt u de symbolisatie en andere instellingen opslaan samen met de vector laag. De stijlen zullen dan worden geladen wanneer u die laag inleest.</translation>
    </message>
    <message>
        <source>Improved support for transparency and contrast stretching in raster layers. Support for color ramps in raster layers. Support for non-north up rasters. Many other raster improvements &apos;under the hood&apos;.</source>
        <translation type="obsolete">Verbeterde transparantie en &apos;contrast-stretching&apos; in rasterlagen. Ondersteuning voor &apos;color ramps&apos; in rasterlagen. Ondersteuning voor rasters die niet noordgeorienteerd zijn. Vele andere verbeterigen &apos;onder de motorkap&apos;.</translation>
    </message>
    <message>
        <source>Resource Location Error</source>
        <translation>Probleeem met bronlocatie</translation>
    </message>
    <message>
        <source>Error reading icon resources from: 
 %1
 Quitting...</source>
        <translation>&apos;Icon&apos;-bestand: 
 %1
 kan niet gelezen worden. Wordt afgesloten...</translation>
    </message>
    <message>
        <source>Map canvas. This is where raster and vectorlayers are displayed when added to the map</source>
        <translation type="obsolete">Kaart venster. Hier worden raster- en vectorkaartlagen afgebeeld na toevoegen aan de kaart</translation>
    </message>
    <message>
        <source>Overview</source>
        <translation>Kaartoverzicht</translation>
    </message>
    <message>
        <source>Legend</source>
        <translation>Legenda</translation>
    </message>
    <message>
        <source>You are using QGIS version %1 built against code revision %2.</source>
        <translation>U gebruikt QGIS versie %1 gebouwd op basis van broncode revisie %2.</translation>
    </message>
    <message>
        <source> This copy of QGIS has been built with PostgreSQL support.</source>
        <translation>  Deze kopie van QGIS is gebouwd met PostgreSQL-ondersteuning.</translation>
    </message>
    <message>
        <source> This copy of QGIS has been built without PostgreSQL support.</source>
        <translation> Deze kopie van QGIS is gebouwd zonder PostgreSQL-ondersteuning.</translation>
    </message>
    <message>
        <source>
This binary was compiled against Qt %1,and is currently running against Qt %2</source>
        <translation> Deze binary is gecompileerd met Qt %1, and gebruikt nu Qt %2</translation>
    </message>
    <message>
        <source>This release candidate includes over 120 bug fixes and enchancements over the QGIS 0.9.1 release. In addition we have added the following new features:</source>
        <translation type="obsolete">Deze rc versie bevat meer dan 120 verbeteringen ten opzicht van de QGIS 0.9.1 versie. Verder zijn de volgende nieuwe functionaliteiten toegevoegd:</translation>
    </message>
    <message>
        <source>Updated icons for improved visual consistancy.</source>
        <translation type="obsolete">Aangepaste iconen mee in lijn met het geheel.</translation>
    </message>
    <message>
        <source>Support for migration of old projects to work in newer QGIS versions.</source>
        <translation type="obsolete">Ondersteuning om oude projecten te laten werken in nieuwere QGIS versies.</translation>
    </message>
    <message>
        <source>Stop map rendering</source>
        <translation>Stoppen met (her)tekenen</translation>
    </message>
    <message>
        <source>Multiple Instances of QgisApp</source>
        <translation>Meerdere Instanties van QgisApp</translation>
    </message>
    <message>
        <source>Multiple instances of Quantum GIS application object detected.
Please contact the developers.
</source>
        <translation>Meerdere draaiende Quantum-Gis-instanties gevonden.
Neem alstublieft contact op met de ontwikkelaars.
</translation>
    </message>
    <message>
        <source>Shift+Ctrl+S</source>
        <comment>Save Project under a new name</comment>
        <translation>Shift+Ctrl+S</translation>
    </message>
    <message>
        <source>&amp;Print Composer</source>
        <translation>&amp;Print Layouter</translation>
    </message>
    <message>
        <source>Ctrl+P</source>
        <comment>Print Composer</comment>
        <translation>Ctrl+P</translation>
    </message>
    <message>
        <source>Print Composer</source>
        <translation>Print Layouter</translation>
    </message>
    <message>
        <source>&amp;Undo</source>
        <translation>&amp;Ongedaan maken</translation>
    </message>
    <message>
        <source>Ctrl+Z</source>
        <translation>Ctrl+Z</translation>
    </message>
    <message>
        <source>Undo the last operation</source>
        <translation>Laatste aktie ongedaan maken</translation>
    </message>
    <message>
        <source>Cu&amp;t</source>
        <translation>K&amp;nippen</translation>
    </message>
    <message>
        <source>Ctrl+X</source>
        <translation>Ctrl+X</translation>
    </message>
    <message>
        <source>Cut the current selection&apos;s contents to the clipboard</source>
        <translation>Huidige selectie naar het klembord knippen</translation>
    </message>
    <message>
        <source>&amp;Copy</source>
        <translation>&amp;CKopïeren</translation>
    </message>
    <message>
        <source>Ctrl+C</source>
        <translation>Ctrl+C</translation>
    </message>
    <message>
        <source>Copy the current selection&apos;s contents to the clipboard</source>
        <translation>Inhoud van huidige selectie naar klembord kopïeren</translation>
    </message>
    <message>
        <source>&amp;Paste</source>
        <translation>&amp;Plakken</translation>
    </message>
    <message>
        <source>Ctrl+V</source>
        <translation>Ctrl+V</translation>
    </message>
    <message>
        <source>Paste the clipboard&apos;s contents into the current selection</source>
        <translation>Huidige selectie vervangen door inhoud van klembord</translation>
    </message>
    <message>
        <source>M</source>
        <comment>Measure a Line</comment>
        <translation>M</translation>
    </message>
    <message>
        <source>J</source>
        <comment>Measure an Area</comment>
        <translation>J</translation>
    </message>
    <message>
        <source>Zoom to Selection</source>
        <translation>Inzoomen Op Selectie</translation>
    </message>
    <message>
        <source>Ctrl+J</source>
        <comment>Zoom to Selection</comment>
        <translation>Ctrl+J</translation>
    </message>
    <message>
        <source>Zoom Actual Size</source>
        <translation>Zoom naar Ware Grootte</translation>
    </message>
    <message>
        <source>Zoom to Actual Size</source>
        <translation>Zoom naar Ware Grootte</translation>
    </message>
    <message>
        <source>Add Vector Layer...</source>
        <translation>Vectorlaag Toevoegen...</translation>
    </message>
    <message>
        <source>Add Raster Layer...</source>
        <translation>Rasterlaag Toevoegen...</translation>
    </message>
    <message>
        <source>Add PostGIS Layer...</source>
        <translation>PostGIS-laag Toevoegen...</translation>
    </message>
    <message>
        <source>W</source>
        <comment>Add a Web Mapping Server Layer</comment>
        <translation>W</translation>
    </message>
    <message>
        <source>Add a Web Mapping Server Layer</source>
        <translation>WebMappingServer-laag Toevoegen</translation>
    </message>
    <message>
        <source>Open Attribute Table</source>
        <translation>Open attributentabel</translation>
    </message>
    <message>
        <source>Save as Shapefile...</source>
        <translation>Opslaan als Shape-bestand...</translation>
    </message>
    <message>
        <source>Save the current layer as a shapefile</source>
        <translation>Huidige laag opslaan als shape-bestand</translation>
    </message>
    <message>
        <source>Save Selection as Shapefile...</source>
        <translation>Huidige Selectie opslaan als Shape-bestand...</translation>
    </message>
    <message>
        <source>Save the selection as a shapefile</source>
        <translation>Selectie opslaan als shape-bestand</translation>
    </message>
    <message>
        <source>Properties...</source>
        <translation>Eigenschappen...</translation>
    </message>
    <message>
        <source>Set properties of the current layer</source>
        <translation>Eigenschappen aanpassen van huidige layer</translation>
    </message>
    <message>
        <source>Add to Overview</source>
        <translation>Toevoegan aan Overzichtskaart</translation>
    </message>
    <message>
        <source>Add All to Overview</source>
        <translation>Alles aan Overzichtskaart Toevoegen</translation>
    </message>
    <message>
        <source>Manage Plugins...</source>
        <translation>Plugin-instellingen...</translation>
    </message>
    <message>
        <source>Toggle Full Screen Mode</source>
        <translation>Volledig scherm aan/uit</translation>
    </message>
    <message>
        <source>Custom CRS...</source>
        <translation>Aangepaste CRS...</translation>
    </message>
    <message>
        <source>Manage custom coordinate reference systems</source>
        <translation>Beheer aangepaste Ruimtelijke Referentie Systemen</translation>
    </message>
    <message>
        <source>Minimize</source>
        <translation>Minimaliseren</translation>
    </message>
    <message>
        <source>Ctrl+M</source>
        <comment>Minimize Window</comment>
        <translation>Ctrl+M</translation>
    </message>
    <message>
        <source>Minimizes the active window to the dock</source>
        <translation>Minimaliseert het actieve scherm naar de &apos;dock&apos;</translation>
    </message>
    <message>
        <source>Zoom</source>
        <translation>Zoom</translation>
    </message>
    <message>
        <source>Toggles between a predefined size and the window size set by the user</source>
        <translation>Wisselt tussen een voorgedefineerde venstergrootte en een door de gebruiker ingestelde grootte</translation>
    </message>
    <message>
        <source>Bring All to Front</source>
        <translation>Alle Naar de Voorgrond</translation>
    </message>
    <message>
        <source>Bring forward all open windows</source>
        <translation>Alle geopende vensters naar de voorgrond</translation>
    </message>
    <message>
        <source>&amp;Edit</source>
        <translation>&amp;Bewerken</translation>
    </message>
    <message>
        <source>Panels</source>
        <translation>Panelen</translation>
    </message>
    <message>
        <source>Toolbars</source>
        <translation>Werkbalken</translation>
    </message>
    <message>
        <source>&amp;Window</source>
        <translation>&amp;Venster</translation>
    </message>
    <message>
        <source>Toggle extents and mouse position display</source>
        <translation>Wissel tussen vertoning muispositie of extent</translation>
    </message>
    <message>
        <source>This icon shows whether on the fly coordinate reference system transformation is enabled or not. Click the icon to bring up the project properties dialog to alter this behaviour.</source>
        <translation>Dit icon toont of de &apos;on the fly&apos; transformatie van het Ruimtelijk Referentie Systeemis aangezet of niet. Klik op het icon om de projecteigenschappendialoog te laten verschijnen en dit aan te passen.</translation>
    </message>
    <message>
        <source>CRS status - Click to open coordinate reference system dialog</source>
        <translation>CRS status - Klik om de dialoog &apos;Ruimtelijk Referentie Systeem&apos; te tonen</translation>
    </message>
    <message>
        <source>This release candidate includes over 60 bug fixes and enchancements over the QGIS 0.10.0 release. In addition we have added the following new features:</source>
        <translation>This release candidate includes over 60 bug fixes and enchancements over the QGIS 0.10.0 release. In addition we have added the following new features:</translation>
    </message>
    <message>
        <source>Revision of all dialogs for user interface consistancy</source>
        <translation>Revisie van alle dialogen voor een consistenter uiterlijk</translation>
    </message>
    <message>
        <source>Improvements to unique value renderer vector dialog</source>
        <translation>Verbeteringen aan de dialoog voor het renderen van unieke waarden</translation>
    </message>
    <message>
        <source>Symbol previews when defining vector classes</source>
        <translation>Voorvertoning van symboliek bij het aanmaken van (vector) klassen</translation>
    </message>
    <message>
        <source>Separation of python support into its own library</source>
        <translation>Python heeft nu zijn eigen bibliotheek (&apos;library&apos;)</translation>
    </message>
    <message>
        <source>List view and filter for GRASS toolbox to find tools more quickly</source>
        <translation>Lijsten en fllters voorde GRASS-gereedschappen om sneller het juiste gereedschap te kunnen vinden</translation>
    </message>
    <message>
        <source>List view and filter for Plugin Manager to find plugins more easily</source>
        <translation>Lijsten en filters voor de Plugin Manager om plugins sneller te kunnen vinden</translation>
    </message>
    <message>
        <source>Updated Spatial Reference System definitions</source>
        <translation>Ruimtelijk Referentie Systemen zijn geupdate</translation>
    </message>
    <message>
        <source>QML Style support for rasters and database layers</source>
        <translation>QML-stijlen voor raster- en database-lagen</translation>
    </message>
    <message>
        <source>Choose a file name to save the QGIS project file as</source>
        <translation>Kies een bestandsnaam om het QGIS-project op te slaan als</translation>
    </message>
    <message>
        <source>Choose a file name to save the map image as</source>
        <translation>Kies een bestandsnaam voor het opslaan van de kaart als</translation>
    </message>
    <message>
        <source>Start editing failed</source>
        <translation>Aanpassen starte mislukt</translation>
    </message>
    <message>
        <source>Provider cannot be opened for editing</source>
        <translation>Aanpassen starten voor deze (Data)provider mislukt</translation>
    </message>
    <message>
        <source>Stop editing</source>
        <translation>Aanpassen stoppen</translation>
    </message>
    <message>
        <source>Do you want to save the changes to layer %1?</source>
        <translation>Wilt u de huidige aanpassingen in laag %1 opslaan?</translation>
    </message>
    <message>
        <source>Could not commit changes to layer %1

Errors:  %2
</source>
        <translation>Commiten van de aanpassingen aan laag %1 zijn mislukt

Fouten:  %2
</translation>
    </message>
    <message>
        <source>Problems during roll back</source>
        <translation>Problemen bij de &apos;roll-back&apos;</translation>
    </message>
    <message>
        <source>Python Console</source>
        <translation></translation>
    </message>
    <message>
        <source>There was an error loading a plugin.The following diagnostic information may help the QGIS developers resolve the issue:
%1.</source>
        <translation type="obsolete">Er is een fout opgetreden bij het laden van een plugin. De volgende informatie kan de QGIS-ontwikkelaars helpen dit op te lossen:
%1.</translation>
    </message>
    <message>
        <source>Map coordinates for the current view extents</source>
        <translation>Kaartcoordinaten van de extent van het huidige kaartbeeld</translation>
    </message>
    <message>
        <source>Maptips require an active layer</source>
        <translation>Maptips werken alleen met een aktieve laag</translation>
    </message>
    <message>
        <source></source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgisAppBase</name>
    <message>
        <source>Legend</source>
        <translation type="obsolete">Legenda</translation>
    </message>
    <message>
        <source>MainWindow</source>
        <translation type="obsolete">HoofdVenster</translation>
    </message>
    <message>
        <source>Map View</source>
        <translation type="obsolete">KaartVenster</translation>
    </message>
    <message>
        <source>QGIS</source>
        <translation>QGIS</translation>
    </message>
</context>
<context>
    <name>QgsAbout</name>
    <message>
        <source>About</source>
        <translation>Info</translation>
    </message>
    <message>
        <source>About Quantum GIS</source>
        <translation>Info over Quantum GIS</translation>
    </message>
    <message>
        <source>Developers</source>
        <translation>Ontwikkelaars</translation>
    </message>
    <message>
        <source>&lt;h2&gt;QGIS Developers&lt;/h2&gt;</source>
        <translation type="obsolete">&lt;h2&gt;QGIS Ontwikkelaars&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Quantum GIS (qgis)&lt;/h2&gt;</source>
        <translation type="obsolete">&lt;h2&gt;Quantum GIS (qgis)&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>Ok</source>
        <translation>Ok</translation>
    </message>
    <message>
        <source>Providers</source>
        <translation>Providers</translation>
    </message>
    <message>
        <source>QGIS Home Page</source>
        <translation>Qgis Start Pagina</translation>
    </message>
    <message>
        <source>Sponsors</source>
        <translation>Sponsors</translation>
    </message>
    <message>
        <source>Subscribe to the QGIS-User mailing list</source>
        <translation type="obsolete">Abbonneer u op de QGIS-User mailinglijst</translation>
    </message>
    <message>
        <source>Version</source>
        <translation>Versie</translation>
    </message>
    <message>
        <source>What&apos;s New</source>
        <translation>Wat is Nieuw</translation>
    </message>
    <message>
        <source>Quantum GIS is licensed under the GNU General Public License</source>
        <translation>Quantum Gis valt onder de GNU General Public License</translation>
    </message>
    <message>
        <source>QGIS Sponsors</source>
        <translation type="obsolete">QGIS Sponsors</translation>
    </message>
    <message>
        <source>The following have sponsored QGIS by contributing money to fund development and other project costs</source>
        <translation type="obsolete">De volgende bedrijven/mensen hebben QGIS gesponsord met gelden om ontwikkeling of ander kosten te betalen</translation>
    </message>
    <message>
        <source>Name</source>
        <translation type="obsolete">Naam</translation>
    </message>
    <message>
        <source>Website</source>
        <translation type="obsolete">Website</translation>
    </message>
    <message>
        <source>QGIS Browser Selection</source>
        <translation type="obsolete">QGIS Browser Selectie</translation>
    </message>
    <message>
        <source>Enter the name of a web browser to use (eg. konqueror).
Enter the full path if the browser is not in your PATH.
You can change this option later by selection Options from the Settings menu (Help Browser tab).</source>
        <translation type="obsolete">Geef de naam van de te gebruiken webbrowser (bv. konqueror).
Geef het volledige pad als de browser niet in uw PATH staat.
U kunt dit later aanpassen door Opties in het Settings-menu te selecteren (Help Browser tab).</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Lucida Grande&apos;; font-size:13pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:16px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:x-large; font-weight:600;&quot;&gt;&lt;span style=&quot; font-size:x-large;&quot;&gt;Quantum GIS (QGIS)&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Lucida Grande&apos;; font-size:13pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:16px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:x-large; font-weight:600;&quot;&gt;&lt;span style=&quot; font-size:x-large;&quot;&gt;Quantum GIS (QGIS)&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>http://www.gnu.org/licenses</source>
        <translation>http://www.gnu.org/licenses</translation>
    </message>
    <message>
        <source>Join our user mailing list</source>
        <translation>Abonneer u op de gebruikers-email-lijst</translation>
    </message>
    <message>
        <source>&lt;p&gt;The following have sponsored QGIS by contributing money to fund development and other project costs&lt;/p&gt;</source>
        <translation type="obsolete">&lt;p&gt;De volgende bedrijven/mensen hebben QGIS gesponsord met gelden om ontwikkeling of ander kosten te betalen&lt;/p&gt;</translation>
    </message>
    <message>
        <source>Available QGIS Data Provider Plugins</source>
        <translation type="obsolete">Beschikbare Data Provider Plugins</translation>
    </message>
    <message>
        <source>Available Qt Database Plugins</source>
        <translation type="obsolete">Beschikbare QT Database Plugins</translation>
    </message>
    <message>
        <source>Available Qt Image Plugins</source>
        <translation type="obsolete">Beschikbare Qt Image Plugins</translation>
    </message>
</context>
<context>
    <name>QgsAddAttrDialogBase</name>
    <message>
        <source>Add Attribute</source>
        <translation>Attribuut Toevoegen</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation type="obsolete">Annuleren</translation>
    </message>
    <message>
        <source>Name:</source>
        <translation>Naam:</translation>
    </message>
    <message>
        <source>OK</source>
        <translation type="obsolete">OK</translation>
    </message>
    <message>
        <source>Type:</source>
        <translation>Type:</translation>
    </message>
</context>
<context>
    <name>QgsApplication</name>
    <message>
        <source>Exception</source>
        <translation>Fout</translation>
    </message>
</context>
<context>
    <name>QgsAttributeActionDialog</name>
    <message>
        <source>Action</source>
        <translation type="obsolete">Actie</translation>
    </message>
    <message>
        <source>Capture</source>
        <translation type="obsolete">Intekenen</translation>
    </message>
    <message>
        <source>Name</source>
        <translation type="obsolete">Naam</translation>
    </message>
    <message>
        <source>Select an action</source>
        <comment>File dialog window title</comment>
        <translation>Selecteer een actie</translation>
    </message>
</context>
<context>
    <name>QgsAttributeActionDialogBase</name>
    <message>
        <source>Action:</source>
        <translation type="obsolete">Actie:</translation>
    </message>
    <message>
        <source>Browse</source>
        <translation type="obsolete">Bladeren</translation>
    </message>
    <message>
        <source>Browse for action commands</source>
        <translation type="obsolete">Blader voor actiecommando&apos;s</translation>
    </message>
    <message>
        <source>Capture output</source>
        <translation>Uitkomst bewaren</translation>
    </message>
    <message>
        <source>Captures any output from the action</source>
        <translation>Bewaar uitkomst van de actie</translation>
    </message>
    <message>
        <source>Captures the standard output or error generated by the action and displays it in a dialog box</source>
        <translation>Bewaar de standaard uitkomst (outpunt) of fouten die door de actie worden geleverd en toon het in een dialoog</translation>
    </message>
    <message>
        <source>Enter the action command here</source>
        <translation>Voer het actiecommando hier in</translation>
    </message>
    <message>
        <source>Enter the action here. This can be any program, script or command that is available on your system. When the action is invoked any set of characters that start with a % and then have the name of a field will be replaced by the value of that field. The special characters %% will be replaced by the value of the field that was selected. Double quote marks group text into single arguments to the program, script or command. Double quotes will be ignored if preceeded by a backslash</source>
        <translation>Voer het actiecommando hier. Dit kan een ander progamma, script of commando zijn beschikbaar op uw systeem. Wanneer de actie wordt aangeroepen zal elke set karakters beginnend met % en dan de naam van een veld, worden vervangen door de waarde van dat veld. De gereserveerde karakters %% worden vervangen door de waarde van het geselecteerde veld. Dubbele aanhalingstekens groeperen tekst in enkele argumenten voor het programma, script of commando. Dubbele quotes worden genegeerd als ze voorafgaan door een \</translation>
    </message>
    <message>
        <source>Enter the action name here</source>
        <translation>Voer de naam van de actie hier in</translation>
    </message>
    <message>
        <source>Enter the name of an action here. The name should be unique (qgis will make it unique if necessary).</source>
        <translation>Vul de naam van de actie hier in. De naam moet uniek zijn (QGIS zal het uniek maken als dat nodig is).</translation>
    </message>
    <message>
        <source>Form1</source>
        <translation type="obsolete">Form1</translation>
    </message>
    <message>
        <source>Insert action</source>
        <translation>Voer actie in</translation>
    </message>
    <message>
        <source>Insert field</source>
        <translation>Voer veld in</translation>
    </message>
    <message>
        <source>Inserts the action into the list above</source>
        <translation>Voegt de actie toe aan de lijst hierboven</translation>
    </message>
    <message>
        <source>Inserts the selected field into the action, prepended with a %</source>
        <translation>Voer de geselecteerde velden aan de actie, voorafgegaan met een %</translation>
    </message>
    <message>
        <source>Move down</source>
        <translation>Naar beneden</translation>
    </message>
    <message>
        <source>Move the selected action down</source>
        <translation>Verplaats geselecteerde actie naar beneden</translation>
    </message>
    <message>
        <source>Move the selected action up</source>
        <translation>Verplaats geselecteerde actie omhoog</translation>
    </message>
    <message>
        <source>Move up</source>
        <translation>Naar boven</translation>
    </message>
    <message>
        <source>Name:</source>
        <translation type="obsolete">Naam:</translation>
    </message>
    <message>
        <source>Remove</source>
        <translation>Verwijder</translation>
    </message>
    <message>
        <source>Remove the selected action</source>
        <translation>Verwijder de geselecteerde actie</translation>
    </message>
    <message>
        <source>The valid attribute names for this layer</source>
        <translation>Geldige attribuutnamen voor deze laag</translation>
    </message>
    <message>
        <source>This list contains all actions that have been defined for the current layer. Add actions by entering the details in the controls below and then pressing the Insert action button. Actions can be edited here by double clicking on the item.</source>
        <translation>Deze lijst bevat alle acties die voor de huidige laag zijn gedefinieerd. Voeg acties toe door de details in te voeren in het formulier hieronder en dan op de Voer actie in-knop te drukken. Acties kunnen worden aangepast door erop te dubbelklikken.</translation>
    </message>
    <message>
        <source>Update action</source>
        <translation>Actie bijwerken</translation>
    </message>
    <message>
        <source>Update the selected action</source>
        <translation>Bewerk de geselecteerde actie</translation>
    </message>
    <message>
        <source>Attribute Actions</source>
        <translation>Attribuut-acties</translation>
    </message>
    <message>
        <source>Action properties</source>
        <translation>Actie-eigenschappen</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Naam</translation>
    </message>
    <message>
        <source>Action</source>
        <translation>Actie</translation>
    </message>
    <message>
        <source>Browse for action</source>
        <translation>Browse voor een actie</translation>
    </message>
    <message>
        <source>Click to browse for an action</source>
        <translation>Blader voor actiecommando&apos;s</translation>
    </message>
    <message>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <source>Capture</source>
        <translation>Intekenen</translation>
    </message>
    <message>
        <source>Clicking the buttone will let you select an application to use as the action</source>
        <translation type="obsolete">Via de knoppen kunt een een applicatie keizen voor deze actie</translation>
    </message>
    <message>
        <source>Clicking the button will let you select an application to use as the action</source>
        <translation>Via de knoppen kunt een een applicatie keizen voor deze actie</translation>
    </message>
</context>
<context>
    <name>QgsAttributeDialog</name>
    <message>
        <source> (int)</source>
        <translation>(int)
</translation>
    </message>
    <message>
        <source> (dbl)</source>
        <translation>(dbl)</translation>
    </message>
    <message>
        <source> (txt)</source>
        <translation>(txt)</translation>
    </message>
    <message>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <source>Select a file</source>
        <translation>Selecteer een bestand</translation>
    </message>
</context>
<context>
    <name>QgsAttributeDialogBase</name>
    <message>
        <source>1</source>
        <translation type="obsolete">1</translation>
    </message>
    <message>
        <source>Attribute</source>
        <translation type="obsolete">Attribuut</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation type="obsolete">&amp;Annuleren</translation>
    </message>
    <message>
        <source>Enter Attribute Values</source>
        <translation>Voer Attribuutwaarden in</translation>
    </message>
    <message>
        <source>&amp;OK</source>
        <translation type="obsolete">&amp;OK</translation>
    </message>
    <message>
        <source>Value</source>
        <translation type="obsolete">Waarde</translation>
    </message>
</context>
<context>
    <name>QgsAttributeTable</name>
    <message>
        <source>Run action</source>
        <translation>Voer aktie uit</translation>
    </message>
    <message>
        <source>Updating selection...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Abort</source>
        <translation type="unfinished">Afbreken</translation>
    </message>
</context>
<context>
    <name>QgsAttributeTableBase</name>
    <message>
        <source>Adva&amp;nced...</source>
        <translation>Geava&amp;nceerd...</translation>
    </message>
    <message>
        <source>Alt+C</source>
        <translation type="obsolete">Alt+C</translation>
    </message>
    <message>
        <source>Alt+G</source>
        <translation type="obsolete">Alt+G</translation>
    </message>
    <message>
        <source>Alt+N</source>
        <translation>Alt+N</translation>
    </message>
    <message>
        <source>Attribute Table</source>
        <translation>Attribuuttabel</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation type="obsolete">Sl&amp;uiten</translation>
    </message>
    <message>
        <source>Copies the selected rows to the clipboard</source>
        <translation>Kopieert de geselecteerde rijen naar het klembord</translation>
    </message>
    <message>
        <source>Copy selected rows to clipboard (Ctrl+C)</source>
        <translation>Kopieer geselecteerde rijen naar klembord (Ctrl+C)</translation>
    </message>
    <message>
        <source>Ctrl+C</source>
        <translation>Ctrl+C</translation>
    </message>
    <message>
        <source>Ctrl+F</source>
        <translation type="obsolete">Ctrl+F</translation>
    </message>
    <message>
        <source>Ctrl+N</source>
        <translation type="obsolete">Ctrl+N</translation>
    </message>
    <message>
        <source>Ctrl+S</source>
        <translation>Ctrl+S</translation>
    </message>
    <message>
        <source>Ctrl+T</source>
        <translation>Ctrl+T</translation>
    </message>
    <message>
        <source>Ctrl+X</source>
        <translation type="obsolete">Ctrl+X</translation>
    </message>
    <message>
        <source>Delete column</source>
        <translation type="obsolete">Verwijder kolom</translation>
    </message>
    <message>
        <source>&amp;Help</source>
        <translation type="obsolete">&amp;Help</translation>
    </message>
    <message>
        <source>in</source>
        <translation>in</translation>
    </message>
    <message>
        <source>Invert selection</source>
        <translation>Selectie omdraaien</translation>
    </message>
    <message>
        <source>Move selected to top</source>
        <translation>Verplaats geselecteerde naar boven</translation>
    </message>
    <message>
        <source>New column</source>
        <translation type="obsolete">Nieuwe kolom</translation>
    </message>
    <message>
        <source>Remove selection</source>
        <translation>Verwijder selectie</translation>
    </message>
    <message>
        <source>Search</source>
        <translation>Zoek</translation>
    </message>
    <message>
        <source>Search for:</source>
        <translation type="obsolete">Zoek naar:</translation>
    </message>
    <message>
        <source>Start editing</source>
        <translation type="obsolete">Start aanpassen</translation>
    </message>
    <message>
        <source>Stop editin&amp;g</source>
        <translation type="obsolete">Stop aanpa&amp;ssen</translation>
    </message>
    <message>
        <source>Zoom map to the selected rows</source>
        <translation>Zoom naar de geselecteerde rijen</translation>
    </message>
    <message>
        <source>Zoom map to the selected rows (Ctrl-F)</source>
        <translation type="obsolete">Zoom naar de geselecteerde rijen (Ctrl+F)</translation>
    </message>
    <message>
        <source>Search for</source>
        <translation>Zoek naar</translation>
    </message>
    <message>
        <source>Zoom map to the selected rows (Ctrl-J)</source>
        <translation>Zoom naar de geselecteerde rijen (Ctrl-J)</translation>
    </message>
    <message>
        <source>Ctrl+J</source>
        <translation>Ctrl+J</translation>
    </message>
    <message>
        <source>Toggle editing mode</source>
        <translation>Modus objectinvoer omzetten</translation>
    </message>
    <message>
        <source>Click to toggle table editing</source>
        <translation>Attribuuttabel-aanpassen aan/uit zetten</translation>
    </message>
</context>
<context>
    <name>QgsAttributeTableDisplay</name>
    <message>
        <source>Could not commit changes</source>
        <translation type="obsolete">&apos;Commit&apos; van de aanpassingen niet mogelijk</translation>
    </message>
    <message>
        <source>Do you want to save the changes?</source>
        <translation type="obsolete">Wilt u de huidige aanpassingen opslaan?</translation>
    </message>
    <message>
        <source>Error</source>
        <translation type="obsolete">Fout</translation>
    </message>
    <message>
        <source>Error during search</source>
        <translation>Fout tijdens zoekactie</translation>
    </message>
    <message>
        <source>Found %d matching features.</source>
        <translation type="obsolete">%d passende objecten gevonden.
        
        
        
        </translation>
    </message>
    <message>
        <source>Name conflict</source>
        <translation type="obsolete">Naamconflict</translation>
    </message>
    <message>
        <source>No matching features found.</source>
        <translation>Geen passende objecten gevonden.</translation>
    </message>
    <message>
        <source>Search results</source>
        <translation>Zoek resultaten</translation>
    </message>
    <message>
        <source>Search string parsing error</source>
        <translation>Zoekstring parseerfout</translation>
    </message>
    <message>
        <source>select</source>
        <translation>selecteren</translation>
    </message>
    <message>
        <source>select and bring to top</source>
        <translation>selecteer ben breng naar boven</translation>
    </message>
    <message>
        <source>show only matching</source>
        <translation>toon alleen passenden</translation>
    </message>
    <message>
        <source>Stop editing</source>
        <translation type="obsolete">Stop aanpassen</translation>
    </message>
    <message>
        <source>The attribute could not be inserted. The name already exists in the table.</source>
        <translation type="obsolete">Het attribuut kon niet worden toegevoegd. De naam bestaat al in de tabel.</translation>
    </message>
    <message>
        <source>You&apos;ve supplied an empty search string.</source>
        <translation>U heeft een geen zoekwoord gegeven.</translation>
    </message>
    <message>
        <source>Could not commit changes - changes are still pending</source>
        <translation type="obsolete">Aanpassingen kunnen niet doorgevoerd worden - aanpassingen zijn nog gaande</translation>
    </message>
    <message>
        <source>Attribute table - </source>
        <translation>Attributentabel - </translation>
    </message>
    <message>
        <source>QGIS</source>
        <translation>QGIS</translation>
    </message>
    <message>
        <source>File</source>
        <translation>Bestand</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Sluiten</translation>
    </message>
    <message>
        <source>Ctrl+W</source>
        <translation>Ctrl+W</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation>Bewerken</translation>
    </message>
    <message>
        <source>&amp;Undo</source>
        <translation>&amp;Ongedaan maken</translation>
    </message>
    <message>
        <source>Ctrl+Z</source>
        <translation>Ctrl+Z</translation>
    </message>
    <message>
        <source>Cu&amp;t</source>
        <translation>K&amp;nippen</translation>
    </message>
    <message>
        <source>Ctrl+X</source>
        <translation>Ctrl+X</translation>
    </message>
    <message>
        <source>&amp;Copy</source>
        <translation>&amp;CKopïeren</translation>
    </message>
    <message>
        <source>Ctrl+C</source>
        <translation>Ctrl+C</translation>
    </message>
    <message>
        <source>&amp;Paste</source>
        <translation>&amp;Plakken</translation>
    </message>
    <message>
        <source>Ctrl+V</source>
        <translation>Ctrl+V</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>Verwijderen</translation>
    </message>
    <message>
        <source>Layer</source>
        <translation>Laag</translation>
    </message>
    <message>
        <source>Zoom to Selection</source>
        <translation>Inzoomen op Selectie</translation>
    </message>
    <message>
        <source>Ctrl+J</source>
        <translation>Ctrl+J</translation>
    </message>
    <message>
        <source>Toggle Editing</source>
        <translation>Modus objectinvoer omzetten</translation>
    </message>
    <message>
        <source>Table</source>
        <translation>Tabel</translation>
    </message>
    <message>
        <source>Move to Top</source>
        <translation>Naar Boven</translation>
    </message>
    <message>
        <source>Invert</source>
        <translation>Inverteren</translation>
    </message>
    <message>
        <source>bad_alloc exception</source>
        <translation>bad_alloc fout</translation>
    </message>
    <message>
        <source>Filling the attribute table has been stopped because there was no more virtual memory left</source>
        <translation>Het vullen van de attributentabel is gestopt omdat er onvoldoende virtueel geheugen beschikbaar is</translation>
    </message>
</context>
<context>
    <name>QgsBookmarks</name>
    <message>
        <source>Are you sure you want to delete the </source>
        <translation>Bent u zeker over de verwijdering van de </translation>
    </message>
    <message>
        <source> bookmark?</source>
        <translation> favoriet?</translation>
    </message>
    <message>
        <source> bookmark from the database. The database said:
</source>
        <translation> favoriet vanuit de database. De database meld: 
</translation>
    </message>
    <message>
        <source>Error deleting bookmark</source>
        <translation>Fout bij het verwijderen van favoriet</translation>
    </message>
    <message>
        <source>Failed to delete the </source>
        <translation>Fout bij het verwijderen van de </translation>
    </message>
    <message>
        <source>Really Delete?</source>
        <translation>Bevestig verwijderen?</translation>
    </message>
    <message>
        <source>&amp;Delete</source>
        <translation>&amp;Verwijder</translation>
    </message>
    <message>
        <source>&amp;Zoom to</source>
        <translation>&amp;Zoom naar</translation>
    </message>
</context>
<context>
    <name>QgsBookmarksBase</name>
    <message>
        <source>Close</source>
        <translation type="obsolete">Sluiten</translation>
    </message>
    <message>
        <source>Close the dialog</source>
        <translation type="obsolete">Sluit de dialoog</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation type="obsolete">Verwijderen</translation>
    </message>
    <message>
        <source>Delete the currently selected bookmark</source>
        <translation type="obsolete">Verwijder de huidige geselecteerde favoriet</translation>
    </message>
    <message>
        <source>Extent</source>
        <translation>Extent</translation>
    </message>
    <message>
        <source>Geospatial Bookmarks</source>
        <translation>Ruimtelijke Favorieten</translation>
    </message>
    <message>
        <source>Help</source>
        <translation type="obsolete">Help</translation>
    </message>
    <message>
        <source>Id</source>
        <translation>Id</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Naam</translation>
    </message>
    <message>
        <source>Project</source>
        <translation>Project</translation>
    </message>
    <message>
        <source>Zoom To</source>
        <translation type="obsolete">Zoom Naar</translation>
    </message>
    <message>
        <source>Zoom to the currently selected bookmark</source>
        <translation type="obsolete">Zoom naar de nu geselecteerde favoriet</translation>
    </message>
</context>
<context>
    <name>QgsComposer</name>
    <message>
        <source>Big image</source>
        <translation>Grote afbeelding</translation>
    </message>
    <message>
        <source>Cannot find BoundingBox</source>
        <translation type="obsolete">Extent niet gevonden</translation>
    </message>
    <message>
        <source>Cannot find translate</source>
        <translation type="obsolete">Translatie niet gevonden</translation>
    </message>
    <message>
        <source>Cannot overwrite BoundingBox</source>
        <translation type="obsolete">Extent niet overschrijfbaar</translation>
    </message>
    <message>
        <source>Cannot overwrite translate</source>
        <translation type="obsolete">Translatie niet overschrijfbaar</translation>
    </message>
    <message>
        <source>Cannot seek</source>
        <translation type="obsolete">Seek net mogelijk</translation>
    </message>
    <message>
        <source>Choose a filename to save the map as</source>
        <translation type="obsolete">Kies bestandsnaam om de kaart te bewaren als</translation>
    </message>
    <message>
        <source>Choose a filename to save the map image as</source>
        <translation type="obsolete">Kies bestandsnaam om de kaartafbeelding te bewaren als</translation>
    </message>
    <message>
        <source>Couldn&apos;t open </source>
        <translation type="obsolete">Probleem bij openen</translation>
    </message>
    <message>
        <source>Don&apos;t show this message again</source>
        <translation>Deze melding niet meer tonen</translation>
    </message>
    <message>
        <source>Error in Print</source>
        <translation type="obsolete">Fout bij printen</translation>
    </message>
    <message>
        <source>File IO Error</source>
        <translation type="obsolete">File IO Fout</translation>
    </message>
    <message>
        <source>format</source>
        <translation>formaat</translation>
    </message>
    <message>
        <source> for read/write</source>
        <translation type="obsolete"> voor lezen/schrijven</translation>
    </message>
    <message>
        <source>Map 1</source>
        <translation>Kaart 1</translation>
    </message>
    <message>
        <source> MB of memory</source>
        <translation> MB geheugen</translation>
    </message>
    <message>
        <source>Paper does not match</source>
        <translation type="obsolete">Papierafmeting past niet</translation>
    </message>
    <message>
        <source>&lt;p&gt;The SVG export function in Qgis has several problems due to bugs and deficiencies in the Qt4 svg code. Of note, text does not appear in the SVG file and there are problems with the map bounding box clipping other items such as the legend or scale bar.&lt;/p&gt;If you require a vector-based output file from Qgis it is suggested that you try printing to PostScript if the SVG output is not satisfactory.&lt;/p&gt;</source>
        <translation type="obsolete">&lt;p&gt;De SVG export functie in QGIS heeft verschillende problemen door &apos;bugs&apos; en afhankelijkheden in de Qt4 svg code. Bijvoorbeeld, tekst verschijnt niet in het SVG-bestand en er zijn problemen met de map extent en clippings met andere onderdelen zoals legenda en schaalbalk&lt;/p&gt;. Indien u niet tevreden bent over de SVG output, maar toch vectorgebaseerde output wenst vanuit QGIS, probeer dan te printen naar een PostScript bestand.&lt;/p&gt;</translation>
    </message>
    <message>
        <source>QGIS - print composer</source>
        <translation>QGIS - print layouter</translation>
    </message>
    <message>
        <source> requires circa </source>
        <translation> gebruikt ongeveer </translation>
    </message>
    <message>
        <source>SVG Format</source>
        <translation>SVG Formaat</translation>
    </message>
    <message>
        <source>SVG warning</source>
        <translation>SVG waarschuwing</translation>
    </message>
    <message>
        <source>The selected paper size does not match the composition size</source>
        <translation type="obsolete">Het geselecteerde papierformaat past nit bij de grootte van de layout</translation>
    </message>
    <message>
        <source>To create image </source>
        <translation>Om een afbeelding te maken</translation>
    </message>
    <message>
        <source>&lt;p&gt;The SVG export function in Qgis has several problems due to bugs and deficiencies in the </source>
        <translation type="obsolete">&lt;p&gt;De SVG export functie in QGIS heeft verschillende problemen door fouten en afhankelijkheden in de </translation>
    </message>
    <message>
        <source>Move Content</source>
        <translation>Inhoud Verschuiven</translation>
    </message>
    <message>
        <source>Move item content</source>
        <translation>Onderdeel inhoud verschuiven</translation>
    </message>
    <message>
        <source>&amp;Group</source>
        <translation>&amp;Groepeer</translation>
    </message>
    <message>
        <source>Group items</source>
        <translation>Groepeer onderdelen</translation>
    </message>
    <message>
        <source>&amp;Ungroup</source>
        <translation>Groepeneren Ongedaan maken &amp;U</translation>
    </message>
    <message>
        <source>Ungroup items</source>
        <translation>Onderdelengroepering ongedaan maken</translation>
    </message>
    <message>
        <source>Raise</source>
        <translation>Omhoog</translation>
    </message>
    <message>
        <source>Raise selected items</source>
        <translation>Geselecteerde omhooghalen</translation>
    </message>
    <message>
        <source>Lower</source>
        <translation>Omlaag</translation>
    </message>
    <message>
        <source>Lower selected items</source>
        <translation>Geselecteerde omlaaghalen</translation>
    </message>
    <message>
        <source>Bring to Front</source>
        <translation>Naar Voorgrond</translation>
    </message>
    <message>
        <source>Move selected items to top</source>
        <translation>Geselecteerde onderdelen naar boven</translation>
    </message>
    <message>
        <source>Send to Back</source>
        <translation>Breng naar Achtergrond</translation>
    </message>
    <message>
        <source>Move selected items to bottom</source>
        <translation>Geselecteerde onderdelen naar beneden</translation>
    </message>
    <message>
        <source>QGIS</source>
        <translation>QGIS</translation>
    </message>
    <message>
        <source>File</source>
        <translation>Bestand</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Sluiten</translation>
    </message>
    <message>
        <source>Ctrl+W</source>
        <translation>Ctrl+W</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation>Bewerken</translation>
    </message>
    <message>
        <source>&amp;Undo</source>
        <translation>&amp;Ongedaan maken</translation>
    </message>
    <message>
        <source>Ctrl+Z</source>
        <translation>Ctrl+Z</translation>
    </message>
    <message>
        <source>Cu&amp;t</source>
        <translation>&amp;CKnippen</translation>
    </message>
    <message>
        <source>Ctrl+X</source>
        <translation>Ctrl+X</translation>
    </message>
    <message>
        <source>&amp;Copy</source>
        <translation>&amp;CKopïeren</translation>
    </message>
    <message>
        <source>Ctrl+C</source>
        <translation>Ctrl+C</translation>
    </message>
    <message>
        <source>&amp;Paste</source>
        <translation>&amp;Plakken</translation>
    </message>
    <message>
        <source>Ctrl+V</source>
        <translation>Ctrl+V</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>Verwijderen</translation>
    </message>
    <message>
        <source>View</source>
        <translation>View</translation>
    </message>
    <message>
        <source>Layout</source>
        <translation>Layout</translation>
    </message>
    <message>
        <source>Choose a file name to save the map image as</source>
        <translation>Kies een bestandsnaam voor het opslaan van de kaart als</translation>
    </message>
    <message>
        <source>Choose a file name to save the map as</source>
        <translation>Kies bestandsnaam om de kaart te bewaren als</translation>
    </message>
    <message>
        <source>Project contains WMS layers</source>
        <translation>Project bevat WMS-lagen</translation>
    </message>
    <message>
        <source>Some WMS servers (e.g. UMN mapserver) have a limit for the WIDTH and HEIGHT parameter. Printing layers from such servers may exceed this limit. If this is the case, the WMS layer will not be printed</source>
        <translation>Sommige WMS-servers (bijvoorbeeld UMN-mapserver) hebben een beperking op de grootte van WIDTH- en HEIGHT-parameter. Bij het printen kunnen die worden overschreden. In dat geval zal de WMS-laag niet worden geprint</translation>
    </message>
</context>
<context>
    <name>QgsComposerBase</name>
    <message>
        <source>Add Image</source>
        <translation>Afbeelding toevoegen</translation>
    </message>
    <message>
        <source>Add new label</source>
        <translation>Nieuw label toevoegen</translation>
    </message>
    <message>
        <source>Add new map</source>
        <translation>Nieuwe kaart toevoegen</translation>
    </message>
    <message>
        <source>Add new scalebar</source>
        <translation>Nieuwe schaalbalk toevoegen</translation>
    </message>
    <message>
        <source>Add new vect legend</source>
        <translation>Nieuwe vector legenda toevoegen</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Sluiten</translation>
    </message>
    <message>
        <source>Composition</source>
        <translation>Layout</translation>
    </message>
    <message>
        <source>Export as image</source>
        <translation type="obsolete">Als afbeelding exporteren</translation>
    </message>
    <message>
        <source>Export as SVG</source>
        <translation type="obsolete">Exporteren naar SVG</translation>
    </message>
    <message>
        <source>General</source>
        <translation>Algemeen</translation>
    </message>
    <message>
        <source>Help</source>
        <translation>Help</translation>
    </message>
    <message>
        <source>Item</source>
        <translation>Onderdeel</translation>
    </message>
    <message>
        <source>MainWindow</source>
        <translation>Hoofdvenster</translation>
    </message>
    <message>
        <source>&amp;Open Template ...</source>
        <translation type="obsolete">&amp;Open Sjabloon ...</translation>
    </message>
    <message>
        <source>&amp;Print...</source>
        <translation>&amp;Afdrukken...</translation>
    </message>
    <message>
        <source>Refresh view</source>
        <translation>Scherm verversen</translation>
    </message>
    <message>
        <source>Save Template &amp;As...</source>
        <translation type="obsolete">Sla Sjabloon op &amp;Als...</translation>
    </message>
    <message>
        <source>Select/Move item</source>
        <translation>Selecteer/Veplaats onderdeel</translation>
    </message>
    <message>
        <source>Zoom All</source>
        <translation type="obsolete">Zoom naar Alles</translation>
    </message>
    <message>
        <source>Zoom In</source>
        <translation>Inzoomen</translation>
    </message>
    <message>
        <source>Zoom Out</source>
        <translation>Uitzoomen</translation>
    </message>
    <message>
        <source>&amp;Open Template...</source>
        <translation type="obsolete">&amp;Open Sjabloon ...</translation>
    </message>
    <message>
        <source>Zoom Full</source>
        <translation>Volledig Uitzoomen</translation>
    </message>
    <message>
        <source>Add Map</source>
        <translation>Kaart toevoegen</translation>
    </message>
    <message>
        <source>Add Label</source>
        <translation>Label Toevoegen</translation>
    </message>
    <message>
        <source>Add Vector Legend</source>
        <translation>Vectorlegenda Toevoegen</translation>
    </message>
    <message>
        <source>Move Item</source>
        <translation>Item Verschuiven</translation>
    </message>
    <message>
        <source>Export as Image...</source>
        <translation>Als afbeelding exporteren...</translation>
    </message>
    <message>
        <source>Export as SVG...</source>
        <translation>Exporteren naar SVG...</translation>
    </message>
    <message>
        <source>Add Scalebar</source>
        <translation>Schaalbalk Toevoegen</translation>
    </message>
    <message>
        <source>Refresh</source>
        <translation>Bijwerken</translation>
    </message>
</context>
<context>
    <name>QgsComposerItemWidgetBase</name>
    <message>
        <source>Form</source>
        <translation>Formulier</translation>
    </message>
    <message>
        <source>Composer item properties</source>
        <translation>Layouter onderdeeleigenschappen</translation>
    </message>
    <message>
        <source>Color:</source>
        <translation>Kleur:</translation>
    </message>
    <message>
        <source>Frame...</source>
        <translation>Frame...</translation>
    </message>
    <message>
        <source>Background...</source>
        <translation>Achtergrond...</translation>
    </message>
    <message>
        <source>Opacity:</source>
        <translation>Opaciteit:</translation>
    </message>
    <message>
        <source>Outline width: </source>
        <translation>Uitlijning Dikte: </translation>
    </message>
    <message>
        <source>Frame</source>
        <translation>Frame</translation>
    </message>
</context>
<context>
    <name>QgsComposerLabelBase</name>
    <message>
        <source>Box</source>
        <translation type="obsolete">Box</translation>
    </message>
    <message>
        <source>Font</source>
        <translation type="obsolete">Lettertype</translation>
    </message>
    <message>
        <source>Label Options</source>
        <translation type="obsolete">Labelopties</translation>
    </message>
</context>
<context>
    <name>QgsComposerLabelWidgetBase</name>
    <message>
        <source>Label Options</source>
        <translation>Labelopties</translation>
    </message>
    <message>
        <source>Font</source>
        <translation>Lettertype</translation>
    </message>
    <message>
        <source>Margin (mm):</source>
        <translation>Margin (mm):</translation>
    </message>
</context>
<context>
    <name>QgsComposerLegendItemDialogBase</name>
    <message>
        <source>Legend item properties</source>
        <translation>Eigenschappen Legenda-item</translation>
    </message>
    <message>
        <source>Item text:</source>
        <translation>Item-tekst:</translation>
    </message>
</context>
<context>
    <name>QgsComposerLegendWidgetBase</name>
    <message>
        <source>Barscale Options</source>
        <translation>Schaalbalk Opties</translation>
    </message>
    <message>
        <source>General</source>
        <translation>Algemeen</translation>
    </message>
    <message>
        <source>Title:</source>
        <translation>Titel:</translation>
    </message>
    <message>
        <source>Font:</source>
        <translation>Lettertype:</translation>
    </message>
    <message>
        <source>Title...</source>
        <translation>Titel...</translation>
    </message>
    <message>
        <source>Layer...</source>
        <translation>Laag...</translation>
    </message>
    <message>
        <source>Item...</source>
        <translation>Onderdeel...</translation>
    </message>
    <message>
        <source>Symbol width: </source>
        <translation>Symbool breedte: </translation>
    </message>
    <message>
        <source>Symbol height:</source>
        <translation>Symbool hoogte: </translation>
    </message>
    <message>
        <source>Layer space: </source>
        <translation>Tussenruimte lagen: </translation>
    </message>
    <message>
        <source>Symbol space:</source>
        <translation>Tussenruimte symbolen:</translation>
    </message>
    <message>
        <source>Icon label space:</source>
        <translation>Tussenruimte labeliconen:</translation>
    </message>
    <message>
        <source>Box space:</source>
        <translation>Rechthoek ruimte:</translation>
    </message>
    <message>
        <source>Legend items</source>
        <translation>Legenda-onderdelen</translation>
    </message>
    <message>
        <source>down</source>
        <translation>naar beneden</translation>
    </message>
    <message>
        <source>up</source>
        <translation>naar boven</translation>
    </message>
    <message>
        <source>remove</source>
        <translation>verwijderen</translation>
    </message>
    <message>
        <source>edit...</source>
        <translation>bewerken...</translation>
    </message>
    <message>
        <source>update</source>
        <translation>bijwerken</translation>
    </message>
    <message>
        <source>update all</source>
        <translation>alles bijwerken</translation>
    </message>
</context>
<context>
    <name>QgsComposerMap</name>
    <message>
        <source>Cache</source>
        <translation type="obsolete">Cache</translation>
    </message>
    <message>
        <source>Extent (calculate scale)</source>
        <translation type="obsolete">Extent (bereken schaal)</translation>
    </message>
    <message>
        <source>Map %1</source>
        <translation type="obsolete">Kaart %1</translation>
    </message>
    <message>
        <source>Rectangle</source>
        <translation type="obsolete">Rechthoek</translation>
    </message>
    <message>
        <source>Render</source>
        <translation type="obsolete">Herteken</translation>
    </message>
    <message>
        <source>Scale (calculate extent)</source>
        <translation type="obsolete">Schaal (bereken extent)</translation>
    </message>
    <message>
        <source>Map</source>
        <translation>Kaart</translation>
    </message>
    <message>
        <source>Map will be printed here</source>
        <translation>De kaart wordt hier afgedrukt</translation>
    </message>
</context>
<context>
    <name>QgsComposerMapBase</name>
    <message>
        <source>1:</source>
        <translation type="obsolete">1:</translation>
    </message>
    <message>
        <source>&lt;b&gt;Map&lt;/b&gt;</source>
        <translation type="obsolete">&lt;b&gt;Kaart&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Font size scale</source>
        <translation type="obsolete">Lettergrootte in schaalbalk</translation>
    </message>
    <message>
        <source>Frame</source>
        <translation type="obsolete">Frame</translation>
    </message>
    <message>
        <source>Height</source>
        <translation type="obsolete">Hoogte</translation>
    </message>
    <message>
        <source>Line width scale</source>
        <translation type="obsolete">Lijndikte schaalbalk</translation>
    </message>
    <message>
        <source>Map options</source>
        <translation type="obsolete">Kaart opties</translation>
    </message>
    <message>
        <source>Preview</source>
        <translation type="obsolete">Voorvertoning</translation>
    </message>
    <message>
        <source>Scale:</source>
        <translation type="obsolete">Schaal:</translation>
    </message>
    <message>
        <source>Set</source>
        <translation type="obsolete">Toepassen</translation>
    </message>
    <message>
        <source>Set Extent</source>
        <translation type="obsolete">Extent Toepassen</translation>
    </message>
    <message>
        <source>Set map extent to current extent in QGIS map canvas</source>
        <translation type="obsolete">Pas kaart extent aan aan de huidige kaartextent in QGIS kaartvenster</translation>
    </message>
    <message>
        <source>Symbol scale</source>
        <translation type="obsolete">Symboolschaal</translation>
    </message>
    <message>
        <source>Width</source>
        <translation type="obsolete">Breedte</translation>
    </message>
    <message>
        <source>Width of one unit in millimeters</source>
        <translation type="obsolete">Breedte van 1 unit in millimeters</translation>
    </message>
</context>
<context>
    <name>QgsComposerMapWidget</name>
    <message>
        <source>Cache</source>
        <translation>Werkgeheugen</translation>
    </message>
    <message>
        <source>Rectangle</source>
        <translation>Rechthoek</translation>
    </message>
    <message>
        <source>Render</source>
        <translation>Renderen</translation>
    </message>
</context>
<context>
    <name>QgsComposerMapWidgetBase</name>
    <message>
        <source>Map options</source>
        <translation>Kaart-opties</translation>
    </message>
    <message>
        <source>&lt;b&gt;Map&lt;/b&gt;</source>
        <translation>&lt;b&gt;Kaart&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Width</source>
        <translation>Breedte</translation>
    </message>
    <message>
        <source>Height</source>
        <translation>Hoogte</translation>
    </message>
    <message>
        <source>Scale:</source>
        <translation>Schaal:</translation>
    </message>
    <message>
        <source>1:</source>
        <translation>1:</translation>
    </message>
    <message>
        <source>Map extent</source>
        <translation>Kaartextent</translation>
    </message>
    <message>
        <source>X min:</source>
        <translation>X min:</translation>
    </message>
    <message>
        <source>Y min:</source>
        <translation>Y min:</translation>
    </message>
    <message>
        <source>X max:</source>
        <translation>X max:</translation>
    </message>
    <message>
        <source>Y max:</source>
        <translation>Y max:</translation>
    </message>
    <message>
        <source>set to map canvas extent</source>
        <translation>aanpassen aan kaartformaat</translation>
    </message>
    <message>
        <source>Preview</source>
        <translation>Voorvertoning</translation>
    </message>
    <message>
        <source>Update preview</source>
        <translation>Voorvertoning bijwerken</translation>
    </message>
</context>
<context>
    <name>QgsComposerPicture</name>
    <message>
        <source>Cannot load picture.</source>
        <translation type="obsolete">Probleem bij het laden van afbeelding.</translation>
    </message>
    <message>
        <source>Choose a file</source>
        <translation type="obsolete">Selecteer een bestand</translation>
    </message>
    <message>
        <source>Pictures (</source>
        <translation type="obsolete">Afbeeldingen (</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation type="obsolete">Waarschuwing</translation>
    </message>
</context>
<context>
    <name>QgsComposerPictureBase</name>
    <message>
        <source>Angle</source>
        <translation type="obsolete">Hoek</translation>
    </message>
    <message>
        <source>Browse</source>
        <translation type="obsolete">Bladeren</translation>
    </message>
    <message>
        <source>Frame</source>
        <translation type="obsolete">Frame</translation>
    </message>
    <message>
        <source>Height</source>
        <translation type="obsolete">Hoogte</translation>
    </message>
    <message>
        <source>Picture Options</source>
        <translation type="obsolete">Afbeelding-opties</translation>
    </message>
    <message>
        <source>Width</source>
        <translation type="obsolete">Breedte</translation>
    </message>
</context>
<context>
    <name>QgsComposerPictureWidget</name>
    <message>
        <source>Select svg or image file</source>
        <translation>Selecteer svg-bestand of afbeelding</translation>
    </message>
</context>
<context>
    <name>QgsComposerPictureWidgetBase</name>
    <message>
        <source>Picture Options</source>
        <translation>Afbeeldings-opties</translation>
    </message>
    <message>
        <source>Browse...</source>
        <translation>Bladeren...</translation>
    </message>
    <message>
        <source>Width:</source>
        <translation>Breedte:</translation>
    </message>
    <message>
        <source>Height:</source>
        <translation>Hoogte:</translation>
    </message>
    <message>
        <source>Rotation:</source>
        <translation>Draaiing:</translation>
    </message>
</context>
<context>
    <name>QgsComposerScaleBar</name>
    <message>
        <source>Single Box</source>
        <translation>Enkele Rechthoek</translation>
    </message>
    <message>
        <source>Double Box</source>
        <translation>Dubbele Rechthoek</translation>
    </message>
    <message>
        <source>Line Ticks Middle</source>
        <translation>Schaalstreepjes Midden</translation>
    </message>
    <message>
        <source>Line Ticks Down</source>
        <translation>Schaalstreepjes Onder</translation>
    </message>
    <message>
        <source>Line Ticks Up</source>
        <translation>Schaalstreepjes Boven</translation>
    </message>
    <message>
        <source>Numeric</source>
        <translation>Numeriek</translation>
    </message>
</context>
<context>
    <name>QgsComposerScaleBarWidget</name>
    <message>
        <source>Single Box</source>
        <translation>Enkele Rechthoek</translation>
    </message>
    <message>
        <source>Double Box</source>
        <translation>Dubbele Rechthoek</translation>
    </message>
    <message>
        <source>Line Ticks Middle</source>
        <translation>Schaalstreepjes Midden</translation>
    </message>
    <message>
        <source>Line Ticks Down</source>
        <translation>Schaalstreepjes Onder</translation>
    </message>
    <message>
        <source>Line Ticks Up</source>
        <translation>Schaalstreepjes Boven</translation>
    </message>
    <message>
        <source>Numeric</source>
        <translation>Numeriek</translation>
    </message>
    <message>
        <source>Map </source>
        <translation>Kaart </translation>
    </message>
</context>
<context>
    <name>QgsComposerScaleBarWidgetBase</name>
    <message>
        <source>Barscale Options</source>
        <translation>Schaalbalk-opties</translation>
    </message>
    <message>
        <source>Segment size (map units):</source>
        <translation>Segment-grootte (kaarteenheden):</translation>
    </message>
    <message>
        <source>Map units per bar unit:</source>
        <translation>Kaarteenheden per schaalbalkeenheid:</translation>
    </message>
    <message>
        <source>Number of segments:</source>
        <translation>Aantal segmenten:</translation>
    </message>
    <message>
        <source>Segments left:</source>
        <translation>Segmenten links: </translation>
    </message>
    <message>
        <source>Style:</source>
        <translation>Stijl:</translation>
    </message>
    <message>
        <source>Map:</source>
        <translation>Kaart:</translation>
    </message>
    <message>
        <source>Height (mm):</source>
        <translation>Hoogte (mm):</translation>
    </message>
    <message>
        <source>Line width:</source>
        <translation>Lijndikte:</translation>
    </message>
    <message>
        <source>Label space:</source>
        <translation>Label ruimte:</translation>
    </message>
    <message>
        <source>Box space:</source>
        <translation>Rechthoek ruimte:</translation>
    </message>
    <message>
        <source>Unit label:</source>
        <translation>Eenheden-label:</translation>
    </message>
    <message>
        <source>Font...</source>
        <translation>Lettertype...</translation>
    </message>
    <message>
        <source>Color...</source>
        <translation>Kleur...</translation>
    </message>
</context>
<context>
    <name>QgsComposerScalebarBase</name>
    <message>
        <source>Barscale Options</source>
        <translation type="obsolete">Schaalbalk Opties</translation>
    </message>
    <message>
        <source>Font</source>
        <translation type="obsolete">Lettertype</translation>
    </message>
    <message>
        <source>Line width</source>
        <translation type="obsolete">Lijndikte</translation>
    </message>
    <message>
        <source>Map</source>
        <translation type="obsolete">Kaart</translation>
    </message>
    <message>
        <source>Map units per scalebar unit</source>
        <translation type="obsolete">Kaarteenheden per schaalbalkeenheid</translation>
    </message>
    <message>
        <source>Number of segments</source>
        <translation type="obsolete">Aantal segmenten</translation>
    </message>
    <message>
        <source>Segment size</source>
        <translation type="obsolete">Segment grootte</translation>
    </message>
    <message>
        <source>Unit label</source>
        <translation type="obsolete">Eenheidlabel</translation>
    </message>
</context>
<context>
    <name>QgsComposerVectorLegend</name>
    <message>
        <source>Cache</source>
        <translation type="obsolete">Cache</translation>
    </message>
    <message>
        <source>Combine selected layers</source>
        <translation type="obsolete">Combineer geselecteerde lagen</translation>
    </message>
    <message>
        <source>Group</source>
        <translation type="obsolete">Groepeer</translation>
    </message>
    <message>
        <source>Layers</source>
        <translation type="obsolete">Lagen</translation>
    </message>
    <message>
        <source>Legend</source>
        <translation type="obsolete">Legenda</translation>
    </message>
    <message>
        <source>Rectangle</source>
        <translation type="obsolete">Rechthoek</translation>
    </message>
    <message>
        <source>Render</source>
        <translation type="obsolete">Herteken</translation>
    </message>
</context>
<context>
    <name>QgsComposerVectorLegendBase</name>
    <message>
        <source>Box</source>
        <translation>Vierkant</translation>
    </message>
    <message>
        <source>Column 1</source>
        <translation type="obsolete">Kolom 1</translation>
    </message>
    <message>
        <source>Font</source>
        <translation>Lettertype</translation>
    </message>
    <message>
        <source>Map</source>
        <translation>Kaart</translation>
    </message>
    <message>
        <source>Preview</source>
        <translation>Voorvertoning</translation>
    </message>
    <message>
        <source>Title</source>
        <translation>Titel</translation>
    </message>
    <message>
        <source>Vector Legend Options</source>
        <translation>Vectorlegenda opties</translation>
    </message>
    <message>
        <source>Layers</source>
        <translation>Lagen</translation>
    </message>
    <message>
        <source>Group</source>
        <translation>Groepeer</translation>
    </message>
    <message>
        <source>ID</source>
        <translation>ID</translation>
    </message>
</context>
<context>
    <name>QgsComposition</name>
    <message>
        <source>Cannot load picture.</source>
        <translation type="obsolete">Probleem bij het laden van afbeelding.</translation>
    </message>
    <message>
        <source>Custom</source>
        <translation type="obsolete">Aangepast</translation>
    </message>
    <message>
        <source>Label</source>
        <translation type="obsolete">Label</translation>
    </message>
    <message>
        <source>Landscape</source>
        <translation type="obsolete">Liggend</translation>
    </message>
    <message>
        <source>Out of memory</source>
        <translation type="obsolete">Geheugentekort</translation>
    </message>
    <message>
        <source>Portrait</source>
        <translation type="obsolete">Staand</translation>
    </message>
    <message>
        <source>Qgis is unable to resize the paper size due to insufficient memory.
 It is best that you avoid using the map composer until you restart qgis.
</source>
        <translation type="obsolete">QGIS kan de afmetingen niet aanpassen door een tekort aan geheugen.
 Probeer the kaart layouter niet gebruiken tot een herstart van QGIS.</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation type="obsolete">Waarschuwing</translation>
    </message>
</context>
<context>
    <name>QgsCompositionBase</name>
    <message>
        <source>Composition</source>
        <translation>Layout</translation>
    </message>
    <message>
        <source>Height</source>
        <translation>Hoogte</translation>
    </message>
    <message>
        <source>Orientation</source>
        <translation>Oriëntatie</translation>
    </message>
    <message>
        <source>Paper</source>
        <translation>Papier</translation>
    </message>
    <message>
        <source>Resolution (dpi)</source>
        <translation type="obsolete">Resolutie (dpi)</translation>
    </message>
    <message>
        <source>Size</source>
        <translation>Formaat</translation>
    </message>
    <message>
        <source>Units</source>
        <translation>Eenheden</translation>
    </message>
    <message>
        <source>Width</source>
        <translation>Breedte</translation>
    </message>
</context>
<context>
    <name>QgsCompositionWidget</name>
    <message>
        <source>Landscape</source>
        <translation>Liggend</translation>
    </message>
    <message>
        <source>Portrait</source>
        <translation>Staand</translation>
    </message>
    <message>
        <source>Custom</source>
        <translation>Aangepast</translation>
    </message>
    <message>
        <source>A5 (148x210 mm)</source>
        <translation>A5 (148x210 mm)</translation>
    </message>
    <message>
        <source>A4 (210x297 mm)</source>
        <translation>A4 (210x297 mm)</translation>
    </message>
    <message>
        <source>A3 (297x420 mm)</source>
        <translation>A3 (297x420 mm)</translation>
    </message>
    <message>
        <source>A2 (420x594 mm)</source>
        <translation>A2 (420x594 mm)</translation>
    </message>
    <message>
        <source>A1 (594x841 mm)</source>
        <translation>A1 (594x841 mm)</translation>
    </message>
    <message>
        <source>A0 (841x1189 mm)</source>
        <translation>A0 (841x1189 mm)</translation>
    </message>
    <message>
        <source>B5 (176 x 250 mm)</source>
        <translation>B5 (176 x 250 mm)</translation>
    </message>
    <message>
        <source>B4 (250 x 353 mm)</source>
        <translation>B4 (250 x 353 mm)</translation>
    </message>
    <message>
        <source>B3 (353 x 500 mm)</source>
        <translation>B3 (353 x 500 mm)</translation>
    </message>
    <message>
        <source>B2 (500 x 707 mm)</source>
        <translation>B2 (500 x 707 mm)</translation>
    </message>
    <message>
        <source>B1 (707 x 1000 mm)</source>
        <translation>B1 (707 x 1000 mm)</translation>
    </message>
    <message>
        <source>B0 (1000 x 1414 mm)</source>
        <translation>B0 (1000 x 1414 mm)</translation>
    </message>
    <message>
        <source>Letter (8.5x11 inches)</source>
        <translation>Letter (8.5x11 inches)</translation>
    </message>
    <message>
        <source>Legal (8.5x14 inches)</source>
        <translation>Legal (8.5x14 inches)</translation>
    </message>
</context>
<context>
    <name>QgsCompositionWidgetBase</name>
    <message>
        <source>Composition</source>
        <translation>Layout</translation>
    </message>
    <message>
        <source>Paper</source>
        <translation>Papier</translation>
    </message>
    <message>
        <source>Orientation</source>
        <translation>Oriëntatie</translation>
    </message>
    <message>
        <source>Height</source>
        <translation>Hoogte</translation>
    </message>
    <message>
        <source>Width</source>
        <translation>Breedte</translation>
    </message>
    <message>
        <source>Units</source>
        <translation>Eenheden</translation>
    </message>
    <message>
        <source>Size</source>
        <translation>Formaat</translation>
    </message>
    <message>
        <source>Print quality (dpi)</source>
        <translation>Afdrukkwaliteit (dpi)</translation>
    </message>
</context>
<context>
    <name>QgsConnectionDialog</name>
    <message>
        <source>Connection failed - Check settings and try again </source>
        <translation type="obsolete">Verbinding mislukt - Controleer uw instellingen en probeer opnieuw </translation>
    </message>
    <message>
        <source>Connection to </source>
        <translation type="obsolete">Verbinding naar </translation>
    </message>
    <message>
        <source>General Interface Help:

</source>
        <translation type="obsolete">Algemene Interface Help:

</translation>
    </message>
    <message>
        <source>Test connection</source>
        <translation type="obsolete">Test verbinding</translation>
    </message>
    <message>
        <source> was successfull</source>
        <translation type="obsolete"> geslaagd</translation>
    </message>
</context>
<context>
    <name>QgsConnectionDialogBase</name>
    <message>
        <source>5432</source>
        <translation type="obsolete">5432</translation>
    </message>
    <message>
        <source>Connection Information</source>
        <translation type="obsolete">Verbindingsinformatie</translation>
    </message>
    <message>
        <source>Create a New PostGIS connection</source>
        <translation type="obsolete">Nieuwe PostGIS-verbinding aanmaken</translation>
    </message>
    <message>
        <source>Database</source>
        <translation type="obsolete">Database</translation>
    </message>
    <message>
        <source>Host</source>
        <translation type="obsolete">Host</translation>
    </message>
    <message>
        <source>Name</source>
        <translation type="obsolete">Naam</translation>
    </message>
    <message>
        <source>Name of the new connection</source>
        <translation type="obsolete">Naam van de nieuwe verbinding</translation>
    </message>
    <message>
        <source>Password</source>
        <translation type="obsolete">Wachtwoord</translation>
    </message>
    <message>
        <source>Port</source>
        <translation type="obsolete">Poort</translation>
    </message>
    <message>
        <source>Save Password</source>
        <translation type="obsolete">Wachtwoord Opslaan</translation>
    </message>
    <message>
        <source>Test Connect</source>
        <translation type="obsolete">Test verbinding</translation>
    </message>
    <message>
        <source>Username</source>
        <translation type="obsolete">Gebruikersnaam</translation>
    </message>
</context>
<context>
    <name>QgsContinuousColorDialogBase</name>
    <message>
        <source>Classification Field:</source>
        <translation>Veld voor classificatie:</translation>
    </message>
    <message>
        <source>Continuous color</source>
        <translation>Continue kleur</translation>
    </message>
    <message>
        <source>Draw polygon outline</source>
        <translation>Teken polygoonomtrek</translation>
    </message>
    <message>
        <source>Maximum Value:</source>
        <translation>Maximale Waarde:</translation>
    </message>
    <message>
        <source>Minimum Value:</source>
        <translation>Minimum Waarde:</translation>
    </message>
    <message>
        <source>Outline Width:</source>
        <translation>Uitlijning dikte:</translation>
    </message>
</context>
<context>
    <name>QgsCoordinateTransform</name>
    <message>
        <source>Failed</source>
        <translation>Mislukt</translation>
    </message>
    <message>
        <source>The coordinates can not be reprojected. The SRS is: </source>
        <translation type="obsolete">De coördinaten kunnen niet worden geherprojecteerd. De SRS is: </translation>
    </message>
    <message>
        <source>The destination spatial reference system (SRS) is not valid. </source>
        <translation type="obsolete">Het doel-coördinatensysteem (Spatial Reference System) is niet geldig. </translation>
    </message>
    <message>
        <source>The source spatial reference system (SRS) is not valid. </source>
        <translation type="obsolete">Het bron-coördinatensysteem (Spatial Reference System) is niet geldig. </translation>
    </message>
    <message>
        <source>transform of</source>
        <translation>omzetten van</translation>
    </message>
    <message>
        <source>with error: </source>
        <translation>met fout: </translation>
    </message>
    <message>
        <source>The source spatial reference system (CRS) is not valid. </source>
        <translation>Het ruimtelijk referentie systeem (CRS) van de bron is niet geldig. </translation>
    </message>
    <message>
        <source>The coordinates can not be reprojected. The CRS is: </source>
        <translation>De coördinaten kunnen niet anders geprojecteerd worden. De CRS is: </translation>
    </message>
    <message>
        <source>The destination spatial reference system (CRS) is not valid. </source>
        <translation>Het ruimtelijk referentie systeem (CRS) van het doel is niet geldig. </translation>
    </message>
</context>
<context>
    <name>QgsCopyrightLabelPlugin</name>
    <message>
        <source>Bottom Left</source>
        <translation>LinksOnder</translation>
    </message>
    <message>
        <source>Bottom Right</source>
        <translation>RechtsOnder</translation>
    </message>
    <message>
        <source>&amp;Copyright Label</source>
        <translation>&amp;Copyrightlabel</translation>
    </message>
    <message>
        <source>Creates a copyright label that is displayed on the map canvas.</source>
        <translation>creëert een copyrightlabel welk wordt getoond op het de kaart.</translation>
    </message>
    <message>
        <source>&amp;Decorations</source>
        <translation>&amp;Decoraties</translation>
    </message>
    <message>
        <source>Top Left</source>
        <translation>LinksBoven</translation>
    </message>
    <message>
        <source>Top Right</source>
        <translation>RechtsBoven</translation>
    </message>
</context>
<context>
    <name>QgsCopyrightLabelPluginGuiBase</name>
    <message>
        <source>Bottom Left</source>
        <translation>LinksOnder</translation>
    </message>
    <message>
        <source>Bottom Right</source>
        <translation>RechtsOnder</translation>
    </message>
    <message>
        <source>Color</source>
        <translation>Kleur</translation>
    </message>
    <message>
        <source>Copyright Label Plugin</source>
        <translation>Copyrightlabelplugin</translation>
    </message>
    <message>
        <source>Enable Copyright Label</source>
        <translation>Copyrightlabel tonen</translation>
    </message>
    <message>
        <source>Horizontal</source>
        <translation>Horizontaal</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:12pt;&quot;&gt;Description&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Enter your copyright label below. This plugin supports basic html markup tags for formatting the label. For example:&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-weight:600;&quot;&gt;&amp;lt;B&amp;gt; Bold text &amp;lt;/B&amp;gt; &lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-weight:600;&quot;&gt;&lt;span style=&quot; font-weight:400; font-style:italic;&quot;&gt;&amp;lt;I&amp;gt; Italics &amp;lt;/I&amp;gt;&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-style:italic;&quot;&gt;&lt;span style=&quot; font-style:normal;&quot;&gt;(note: &amp;amp;copy; gives a copyright symbol)&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;p, li { white-space: pre-wrap; }&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:12pt;&quot;&gt;Beschrijving&lt;/span&gt;&lt;/p&gt;&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Voer de copyrightlabel-tekst hieronder in. The plugin kan gebruik maken vaan eenvoudige html. Bijvoorbeeld:&lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-weight:600;&quot;&gt;&amp;lt;B&amp;gt; Tekst Vet &amp;lt;/B&amp;gt; &lt;/span&gt;&lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-weight:600;&quot;&gt;&lt;span style=&quot; font-weight:400; font-style:italic;&quot;&gt;&amp;lt;I&amp;gt; Tekst Schuin &amp;lt;/I&amp;gt;&lt;/span&gt;&lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-style:italic;&quot;&gt;&lt;span style=&quot; font-style:normal;&quot;&gt;(opmerking: &amp;amp;copy; geeft je het copyrightsymbool)&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Orientation</source>
        <translation>Oriëntatie</translation>
    </message>
    <message>
        <source>Placement</source>
        <translation>Plaats</translation>
    </message>
    <message encoding="UTF-8">
        <source>© QGIS 2008</source>
        <translation type="obsolete">© QGIS 2008</translation>
    </message>
    <message>
        <source>Top Left</source>
        <translation>LinksBoven</translation>
    </message>
    <message>
        <source>Top Right</source>
        <translation>RechtsBoven</translation>
    </message>
    <message>
        <source>Vertical</source>
        <translation>Verticaal</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;&lt;span style=&quot; font-size:12pt;&quot;&gt;Description&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;Enter your copyright label below. This plugin supports basic html markup tags for formatting the label. For example:&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;&lt;span style=&quot; font-weight:600;&quot;&gt;&amp;lt;B&amp;gt; Bold text &amp;lt;/B&amp;gt; &lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:600;&quot;&gt;&lt;span style=&quot; font-weight:400; font-style:italic;&quot;&gt;&amp;lt;I&amp;gt; Italics &amp;lt;/I&amp;gt;&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-style:italic;&quot;&gt;&lt;span style=&quot; font-style:normal;&quot;&gt;(note: &amp;amp;copy; gives a copyright symbol)&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;(new line)
p, li { white-space: pre-wrap; }(new line)
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;(new line)
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;&lt;span style=&quot; font-size:12pt;&quot;&gt;Description&lt;/span&gt;&lt;/p&gt;(new line)
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;&lt;/p&gt;(new line)
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;Enter your copyright label below. This plugin supports basic html markup tags for formatting the label. For example:&lt;/p&gt;(new line)
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;&lt;span style=&quot; font-weight:600;&quot;&gt;&amp;lt;B&amp;gt; Bold text &amp;lt;/B&amp;gt; &lt;/span&gt;&lt;/p&gt;(new line)
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:600;&quot;&gt;&lt;span style=&quot; font-weight:400; font-style:italic;&quot;&gt;&amp;lt;I&amp;gt; Italics &amp;lt;/I&amp;gt;&lt;/span&gt;&lt;/p&gt;(new line)
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-style:italic;&quot;&gt;&lt;span style=&quot; font-style:normal;&quot;&gt;(note: &amp;amp;copy; gives a copyright symbol)&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message encoding="UTF-8">
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;© QGIS 2008&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;(new line)
p, li { white-space: pre-wrap; }(new line)
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;(new line)
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;© QGIS 2008&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
</context>
<context>
    <name>QgsCustomProjectionDialog</name>
    <message>
        <source>Abort</source>
        <translation>Afbreken</translation>
    </message>
    <message>
        <source>Delete Projection Definition?</source>
        <translation>Projectie-definitie Verwijderen?</translation>
    </message>
    <message>
        <source>Deleting a projection definition is not reversable. Do you want to delete it?</source>
        <translation>Een projectie-definitie verwijderen is onherroepelijk. Wilt u het verwijderen?</translation>
    </message>
    <message>
        <source>Internal Error (source projection invalid?)</source>
        <translation>Interne Fout (bron-projectie ongeldig?)</translation>
    </message>
    <message>
        <source>New</source>
        <translation>Nieuw</translation>
    </message>
    <message>
        <source>Northing and Easthing must be in decimal form.</source>
        <translation>&apos;Northing&apos; en &apos;Easthing&apos; moeten in decimaal formaat worden gegeven.</translation>
    </message>
    <message>
        <source>QGIS Custom Projection</source>
        <translation>QGIS Aangepaste Projectie</translation>
    </message>
    <message>
        <source>This proj4 ellipsoid definition is not valid. Please add a ellips= clause before pressing save.</source>
        <translation type="obsolete">De proj4 definitie voor de ellipsoïde is niet geldig. Voeg het &apos;ellips=&apos;-deel toe voor het opslaan.</translation>
    </message>
    <message>
        <source>This proj4 projection definition is not valid.</source>
        <translation>Deze proj4 projectie definitie is niet geldig.</translation>
    </message>
    <message>
        <source>This proj4 projection definition is not valid. Please add a proj= clause before pressing save.</source>
        <translation>De proj4 definitie voor de projectie is niet geldig. Voeg het &apos;proj=&apos;-deel toe voor het opslaan.</translation>
    </message>
    <message>
        <source>This proj4 projection definition is not valid. Please add the parameters before pressing save.</source>
        <translation>De proj4 definitie voor de projectie is niet geldig. Vul de juiste parameters aan voor het opslaan.</translation>
    </message>
    <message>
        <source>This proj4 projection definition is not valid. Please correct before pressing save.</source>
        <translation>De proj4 definitie voor de projectie is niet geldig. Pas aan voor het opslaan.</translation>
    </message>
    <message>
        <source>This proj4 projection definition is not valid. Please give the projection a name before pressing save.</source>
        <translation>De proj4 definitie voor de projectie is niet geldig. Geef de projectie een naam voor het opslaan.</translation>
    </message>
</context>
<context>
    <name>QgsCustomProjectionDialogBase</name>
    <message>
        <source>&lt;</source>
        <translation>&lt;</translation>
    </message>
    <message>
        <source>&gt;</source>
        <translation>&gt;</translation>
    </message>
    <message>
        <source>&gt;|</source>
        <translation>&gt;|</translation>
    </message>
    <message>
        <source>|&lt;</source>
        <translation>|&lt;</translation>
    </message>
    <message>
        <source>1 of 1</source>
        <translation>1 van 1</translation>
    </message>
    <message>
        <source>Calculate</source>
        <translation>Bereken</translation>
    </message>
    <message>
        <source>Close</source>
        <translation type="obsolete">Sluiten</translation>
    </message>
    <message>
        <source>Custom Projection Definition</source>
        <translation type="obsolete">Aangepaste Projectie Definitie</translation>
    </message>
    <message>
        <source>Define</source>
        <translation>Definieer</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation type="obsolete">Verwijderen</translation>
    </message>
    <message>
        <source>East:</source>
        <translation type="obsolete">Oost:</translation>
    </message>
    <message>
        <source>Geographic / WGS84</source>
        <translation>Geografisch / WGS84</translation>
    </message>
    <message>
        <source>Name:</source>
        <translation type="obsolete">Naam:</translation>
    </message>
    <message>
        <source>New</source>
        <translation type="obsolete">Nieuw</translation>
    </message>
    <message>
        <source>North:</source>
        <translation type="obsolete">Noord:</translation>
    </message>
    <message>
        <source>Parameters:</source>
        <translation type="obsolete">Parameters:</translation>
    </message>
    <message>
        <source>Projected Coordinate System</source>
        <translation type="obsolete">Geprojecteerd Coördinaatsysteem</translation>
    </message>
    <message>
        <source>Save</source>
        <translation type="obsolete">Opslaan</translation>
    </message>
    <message>
        <source>Test</source>
        <translation>Test</translation>
    </message>
    <message>
        <source>Transform from WGS84 to the chosen projection</source>
        <translation type="obsolete">Transformeer van WGS84 naar de gekozen projectie</translation>
    </message>
    <message>
        <source>Use the text boxes below to test the projection definition you are creating. Enter a coordinate where both the lat/long and the projected result are known (for example by reading off a map). Then press the calculate button to see if the projection definition you are creating is accurate.</source>
        <translation type="obsolete">Gebruik de invoervelden hieronder om de projectie definitie te testen. Voer een coördinaat in waarvan zowel de lat/lon waarde als de geprojecteerde resultaten bekend zijn (bijvoorbeeld door ze van een kaart af te lezen). Klik de &apos;bereken&apos;-knop om te zien of uw projectiedefinitie goed is.</translation>
    </message>
    <message>
        <source>You can define your own custom projection here. The definition must conform to the proj4 format for specifying a Spatial Reference System.</source>
        <translation type="obsolete">Hier kunt u zelf aangepaste projecties definiëren. De definitie moet voldoen aan het proj4 formaat voor Spatial Referentie Systemen (SRS).</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Naam</translation>
    </message>
    <message>
        <source>Parameters</source>
        <translation>Parameters</translation>
    </message>
    <message>
        <source>*</source>
        <translation>*</translation>
    </message>
    <message>
        <source>S</source>
        <translation>S</translation>
    </message>
    <message>
        <source>X</source>
        <translation>X</translation>
    </message>
    <message>
        <source>North</source>
        <translation>Noord</translation>
    </message>
    <message>
        <source>East</source>
        <translation>Oost</translation>
    </message>
    <message>
        <source>Custom Coordinate Reference System Definition</source>
        <translation>Aangepast Ruimtelijk Referentie Systeem (CRS) Definintie</translation>
    </message>
    <message>
        <source>You can define your own custom Coordinate Reference System (CRS) here. The definition must conform to the proj4 format for specifying a CRS.</source>
        <translation>U kunt uw eigen aangepaste Ruimtelijk Referentie Systeem (CRS) hier definiëren. De definitie moet voldoen aan het proj4-formaat.</translation>
    </message>
    <message>
        <source>Use the text boxes below to test the CRS definition you are creating. Enter a coordinate where both the lat/long and the transformed result are known (for example by reading off a map). Then press the calculate button to see if the CRS definition you are creating is accurate.</source>
        <translation>Gebruik onderstaande tekst-vensters om uw CRS-definitie te testen. Gebruik coördinaten waarvan u zowel de lat/long als het geprojecteerde resultaat weet (door ze bijvoorbeeld op een kaart af te lezen). Druk op de &quot;bereken&quot;-knop om te controleren of uw CRS-definitie nauwkeurig is.</translation>
    </message>
    <message>
        <source>Destination CRS        </source>
        <translation>Doel CRS        </translation>
    </message>
</context>
<context>
    <name>QgsDbSourceSelect</name>
    <message>
        <source>All</source>
        <translation>Alles</translation>
    </message>
    <message>
        <source>Are you sure you want to remove the </source>
        <translation>Weet u zeker dat u dit wilt verwijderen</translation>
    </message>
    <message>
        <source>Confirm Delete</source>
        <translation>Bevestig Verwijderen</translation>
    </message>
    <message>
        <source> connection and all associated settings?</source>
        <translation> verbinding en alle daarbij horende instellingen?</translation>
    </message>
    <message>
        <source>Connection failed</source>
        <translation>Verbinding mislukt</translation>
    </message>
    <message>
        <source>Connection to %1 on %2 failed. Either the database is down or your settings are incorrect.%3Check your username and password and try again.%4The database said:%5%6</source>
        <translation>Verbinding naar %1 op %2 mislukt. De database is uitgeschakeld of de instellingen zijn fout.%3Check gebruiksnaam en wachtwoorden en probeer opnieuw.%4Melding van de database:%5%6</translation>
    </message>
    <message>
        <source>Geometry column</source>
        <translation>Geometriekolom</translation>
    </message>
    <message>
        <source>Password for </source>
        <translation>Wachtwoord voor</translation>
    </message>
    <message>
        <source>Please enter your password:</source>
        <translation>Voer wachtwoord in:</translation>
    </message>
    <message>
        <source>RegExp</source>
        <translation>RegExp</translation>
    </message>
    <message>
        <source>Schema</source>
        <translation>Schema</translation>
    </message>
    <message>
        <source>Select Table</source>
        <translation>Selecteer Tabel</translation>
    </message>
    <message>
        <source>Sql</source>
        <translation>Sql</translation>
    </message>
    <message>
        <source>Table</source>
        <translation>Tabel</translation>
    </message>
    <message>
        <source>Type</source>
        <translation>Type</translation>
    </message>
    <message>
        <source>Wildcard</source>
        <translation>Wildcard</translation>
    </message>
    <message>
        <source>You must select a table in order to add a Layer.</source>
        <translation>Selecteer een tabel om een Laag toe te kunnen voegen.</translation>
    </message>
    <message>
        <source>Accessible tables could not be determined</source>
        <translation>Er zijn geen toegankelijke tabellen gevonden</translation>
    </message>
    <message>
        <source>Database connection was successful, but the accessible tables could not be determined.

The error message from the database was:
%1
</source>
        <translation>Databaseverbinding is gelukt, maar er zijn geen voor u toegankelijke tabellen gevonden.

De foutmelding van de database was:
%1
</translation>
    </message>
    <message>
        <source>No accessible tables found</source>
        <translation>Geen toegankelijke tabellen gevonden</translation>
    </message>
    <message>
        <source>Database connection was successful, but no accessible tables were found.

Please verify that you have SELECT privilege on a table carrying PostGIS
geometry.</source>
        <translation>Databaseverbinding is gelukt, maar er zijn geen voor u toegankelijke tabellen gevonden.

Verifieer dat u SELECT-rechten heeft op een tabel met PostGIS
geometrische objecten.</translation>
    </message>
</context>
<context>
    <name>QgsDbSourceSelectBase</name>
    <message>
        <source>Add</source>
        <translation>Toevoegen</translation>
    </message>
    <message>
        <source>Add PostGIS Table(s)</source>
        <translation>PostGIS Tabel(len) Toevoegen</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Sluiten</translation>
    </message>
    <message>
        <source>Connect</source>
        <translation>Verbinden</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>Verwijderen</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation>Bewerken</translation>
    </message>
    <message>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <source>Help</source>
        <translation>Help</translation>
    </message>
    <message>
        <source>New</source>
        <translation>Nieuw</translation>
    </message>
    <message>
        <source>PostgreSQL Connections</source>
        <translation>PostgreSQL Verbindingen</translation>
    </message>
    <message>
        <source>Search:</source>
        <translation>Zoek:</translation>
    </message>
    <message>
        <source>Search in columns:</source>
        <translation>Zoeken in kolommen:</translation>
    </message>
    <message>
        <source>Search mode:</source>
        <translation>Zoek modus:</translation>
    </message>
    <message>
        <source>Search options...</source>
        <translation>Zoek opties...</translation>
    </message>
</context>
<context>
    <name>QgsDbTableModel</name>
    <message>
        <source>Geometry column</source>
        <translation>Geometriekolom</translation>
    </message>
    <message>
        <source>Line</source>
        <translation>Lijn</translation>
    </message>
    <message>
        <source>Multiline</source>
        <translation>Multilijn</translation>
    </message>
    <message>
        <source>Multipoint</source>
        <translation>Multipunt</translation>
    </message>
    <message>
        <source>Multipolygon</source>
        <translation>Multipolygoon</translation>
    </message>
    <message>
        <source>Point</source>
        <translation>Punt</translation>
    </message>
    <message>
        <source>Polygon</source>
        <translation>Polygoon</translation>
    </message>
    <message>
        <source>Schema</source>
        <translation>Schema</translation>
    </message>
    <message>
        <source>Sql</source>
        <translation>Sql</translation>
    </message>
    <message>
        <source>Table</source>
        <translation>Tabel</translation>
    </message>
    <message>
        <source>Type</source>
        <translation>Type</translation>
    </message>
</context>
<context>
    <name>QgsDelAttrDialogBase</name>
    <message>
        <source>Cancel</source>
        <translation type="obsolete">Annuleren</translation>
    </message>
    <message>
        <source>Delete Attributes</source>
        <translation>Attributen Verwijderen</translation>
    </message>
    <message>
        <source>OK</source>
        <translation type="obsolete">OK</translation>
    </message>
</context>
<context>
    <name>QgsDelimitedTextPlugin</name>
    <message>
        <source>Add a delimited text file as a map layer. </source>
        <translation>Gebruik een tekengescheiden tekstbestand als kaartlaag.</translation>
    </message>
    <message>
        <source>&amp;Add Delimited Text Layer</source>
        <translation>&amp;Toevoegen Tekengescheidentekst Kaartlaag</translation>
    </message>
    <message>
        <source>&amp;Delimited text</source>
        <translation>&amp;Tekengescheiden tekst</translation>
    </message>
    <message>
        <source>DelimitedTextLayer</source>
        <translation>TekenGescheidenTekstLaag</translation>
    </message>
    <message>
        <source>The file must have a header row containing the field names. </source>
        <translation>Het bestand moet een bovenregel hebben met de veldnamen. </translation>
    </message>
    <message>
        <source>X and Y fields are required and must contain coordinates in decimal units.</source>
        <translation>X en Y velden zijn verplicht en moeten coördinaten in decimale getallen bevatten.</translation>
    </message>
</context>
<context>
    <name>QgsDelimitedTextPluginGui</name>
    <message>
        <source>Choose a delimited text file to open</source>
        <translation>Kies een tekengescheiden tekstbestand om te openen</translation>
    </message>
    <message>
        <source>Description</source>
        <translation>Omschrijving</translation>
    </message>
    <message>
        <source>No delimiter</source>
        <translation>Geen scheidingsteken</translation>
    </message>
    <message>
        <source>No layer name</source>
        <translation>Geen laagnaam</translation>
    </message>
    <message>
        <source>Parse</source>
        <translation>Parseer</translation>
    </message>
    <message>
        <source>Please enter a layer name before adding the layer to the map</source>
        <translation>Geef alstublief een naam voor de laag aan de kaart toe te voegen</translation>
    </message>
    <message>
        <source>Please specify a delimiter prior to parsing the file</source>
        <translation>Geef alstublieft het scheidingsteken vooraf aan het parseren van het bestand</translation>
    </message>
    <message>
        <source>Select a delimited text file containing a header row and one or more rows of x and y coordinates that you would like to use as a point layer and this plugin will do the job for you!</source>
        <translation>Selecteer een tekstgescheiden bestand met een kopregel en een of meer rijen met x en y coördinaten om als puntlaag te gebruiken, en deze plugin regelt dat voor u!</translation>
    </message>
    <message>
        <source>Use the layer name box to specify the legend name for the new layer. Use the delimiter box to specify what delimeter is used in your file (e.g. space, comma, tab or a regular expression in Perl style). After choosing a delimiter, press the parse button and select the columns containing the x and y values for the layer.</source>
        <translation>Gebruik het laagnaam invoerveld voor de legendanaam voor de nieuwe laag. Gebruik het scheidingsteken invoerveld om het scheidingsteken voor uw bestand te kiezen (b.v. spatie, komma, tab of een reguliere expressie in Perl-stijl). Na het kiezen van het scheidingsteken en het aangeven van de x en y kolommen klik de parseer knop.</translation>
    </message>
</context>
<context>
    <name>QgsDelimitedTextPluginGuiBase</name>
    <message>
        <source>Browse...</source>
        <translation>Bladeren...</translation>
    </message>
    <message>
        <source>Browse to find the delimited text file to be processed</source>
        <translation>Blader naar een tekstgescheiden bestand om te verwerken</translation>
    </message>
    <message>
        <source>Create a Layer from a Delimited Text File</source>
        <translation>Creëer een Kaartlaag van een Tekstgescheiden Bestand</translation>
    </message>
    <message>
        <source>Delimited text file</source>
        <translation>Tekstgescheiden bestand</translation>
    </message>
    <message>
        <source>Delimited Text Layer</source>
        <translation>Tekstgescheiden Kaartlaag</translation>
    </message>
    <message>
        <source>Delimiter</source>
        <translation>Scheidingsteken</translation>
    </message>
    <message>
        <source>Delimiter to use when splitting fields in the delimited text file. The delimiter can be 1 or more characters in length.</source>
        <translation>Scheidingsteken voor het scheiden van velden in het tekstgescheiden bestand. Het scheidingsteken kan een lengte van 1 of meer karakters hebben.</translation>
    </message>
    <message>
        <source>Delimiter to use when splitting fields in the text file. The delimiter can be more than one character.</source>
        <translation>Scheidingsteken voor het scheiden van velden in het tekstbestand. Het scheidingsteken kan een lengte van 1 of meer karakters hebben.</translation>
    </message>
    <message>
        <source>Full path to the delimited text file</source>
        <translation>Volledige pad naar het tekstbestand</translation>
    </message>
    <message>
        <source>Full path to the delimited text file. In order to properly parse the fields in the file, the delimiter must be defined prior to entering the file name. Use the Browse button to the right of this field to choose the input file.</source>
        <translation>Volledige pad naar het tekstgescheiden bestand. Om het bestand goed te kunnen verwerken moet het scheidingsteken bekend zijn vooraf aan het geven van de bestandsnaam. Gebruik de Bladeren-knop rechts van het veld om het input bestand te kiezen.</translation>
    </message>
    <message>
        <source>Layer name</source>
        <translation>Laagnaam</translation>
    </message>
    <message>
        <source>Name displayed in the map legend</source>
        <translation>Naam zoals getoond in de kaartlegenda</translation>
    </message>
    <message>
        <source>Name of the field containing x values</source>
        <translation>Naam van het veld met x-waarden</translation>
    </message>
    <message>
        <source>Name of the field containing x values. Choose a field from the list. The list is generated by parsing the header row of the delimited text file.</source>
        <translation>Naam van het veld met x-waarden. Kies een veld uit de lijst. De lijst is opgesteld door de kopregel van het tekstbestand te parseren.</translation>
    </message>
    <message>
        <source>Name of the field containing y values</source>
        <translation>Naam van het veld met y-waarden</translation>
    </message>
    <message>
        <source>Name of the field containing y values. Choose a field from the list. The list is generated by parsing the header row of the delimited text file.</source>
        <translation>Naam van het veld met y-waarden. Kies een veld uit de lijst. De lijst is opgesteld door de kopregel van het tekstbestand te parseren.</translation>
    </message>
    <message>
        <source>Name to display in the map legend</source>
        <translation>Naam zoals getoond in het kaartvenster</translation>
    </message>
    <message>
        <source>&lt;p align=&quot;right&quot;&gt;X field&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;right&quot;&gt;X-veld&lt;/p&gt;</translation>
    </message>
    <message>
        <source>&lt;p align=&quot;right&quot;&gt;Y field&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;right&quot;&gt;Y-veld&lt;/p&gt;</translation>
    </message>
    <message>
        <source>Plain characters</source>
        <translation>Tekstkarakters</translation>
    </message>
    <message>
        <source>Regular expression</source>
        <translation>Reguliere expressie</translation>
    </message>
    <message>
        <source>Sample text</source>
        <translation>Voorbeeld tekst</translation>
    </message>
    <message>
        <source>The delimiter is a regular expression</source>
        <translation>Het scheidingsteken is een reguliere expressie</translation>
    </message>
    <message>
        <source>The delimiter is taken as is</source>
        <translation>Gevonden scheidingsteken wordt gebruikt</translation>
    </message>
    <message>
        <source>Use this button to browse to the location of the delimited text file. This button will not be enabled until a delimiter has been entered in the &lt;i&gt;Delimiter&lt;/i&gt; box. Once a file is chosen, the X and Y field drop-down boxes will be populated with the fields from the delimited text file.</source>
        <translation>Gebruik deze knop om naar het tekstgescheiden bestand te bladeren. De knop zal pas geactiveerd worden nadat u een scheidingsteken in het  &lt;i&gt;Scheidingsteken&lt;/i&gt; invoerveld heeft gegeven. Wanneer een bestand is gekozen, zullen de X en Y-veld lijsten worden gevuld met de velden uit het bestand.</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;(new line)
p, li { white-space: pre-wrap; }(new line)
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;(new line)
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
</context>
<context>
    <name>QgsDelimitedTextProvider</name>
    <message>
        <source>Error</source>
        <translation>Fout</translation>
    </message>
    <message>
        <source>Note: the following lines were not loaded because Qgis was unable to determine values for the x and y coordinates:
</source>
        <translation>Opmerking: de volgende regels werden niet geladen omdat QGIS niet de x- en y-waarden kon bepalsen:</translation>
    </message>
</context>
<context>
    <name>QgsDetailedItemWidgetBase</name>
    <message>
        <source>Form</source>
        <translation>Formulier</translation>
    </message>
    <message>
        <source>Heading Label</source>
        <translation>Kop-label</translation>
    </message>
    <message>
        <source>Detail label</source>
        <translation>Detail-label</translation>
    </message>
</context>
<context>
    <name>QgsDlgPgBufferBase</name>
    <message>
        <source>Add the buffered layer to the map?</source>
        <translation>Voeg de gebufferde laag toe aan de kaart?</translation>
    </message>
    <message>
        <source>Buffer distance in map units:</source>
        <translation>Bufferafstand in kaarteenheden:</translation>
    </message>
    <message>
        <source>Buffer features</source>
        <translation>Buffer kaartobjecten</translation>
    </message>
    <message>
        <source>Create unique object id</source>
        <translation>Creëer een uniek object-id</translation>
    </message>
    <message>
        <source>Geometry column:</source>
        <translation>Geometrie kolom:</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Buffer the features in layer: &lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Buffer de objecten in laag: &lt;/h2&gt;</translation>
    </message>
    <message>
        <source>Parameters</source>
        <translation>Parameters</translation>
    </message>
    <message>
        <source>public</source>
        <translation>publiek</translation>
    </message>
    <message>
        <source>Schema:</source>
        <translation>Schema:</translation>
    </message>
    <message>
        <source>Spatial reference ID:</source>
        <translation>Spatial reference ID:</translation>
    </message>
    <message>
        <source>Table name for the buffered layer:</source>
        <translation>Tabelnaam voor de gebufferde laag:</translation>
    </message>
    <message>
        <source>Unique field to use as feature id:</source>
        <translation>Uniek veld om als object-id te gebruiken:</translation>
    </message>
</context>
<context>
    <name>QgsEditReservedWordsBase</name>
    <message>
        <source>Column Name</source>
        <translation type="obsolete">KolomNaam</translation>
    </message>
    <message>
        <source>Edit Reserved Words</source>
        <translation type="obsolete">Gereserveerde Termen Aanpassen</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Double click the Column Name column to change the name of the column.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Dubbelklik de kolomnaam om de naam van de kolom te veranderen.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;This shapefile contains reserved words. These may affect the import into PostgreSQL. Edit the column names so none of the reserved words listed at the right are used (click on a Column Name entry to edit). You may also change any other column name if desired.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Dit shape-bestand bevat gereserveerde termen. Die kunnen de import in PostgreSQL (negatief) beïnvloeden. Pas de kolomnamen aan zodat er geen gereserveerde termen zoals hier rechts worden gebruikt (klik op de kolomnaam om aan te passen). U kunt ook elke andere kolomnaam aanpassen.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Index</source>
        <translation type="obsolete">Inhoudsopgave</translation>
    </message>
    <message>
        <source>Reserved Words</source>
        <translation type="obsolete">Gereserveerde Termen</translation>
    </message>
    <message>
        <source>Status</source>
        <translation type="obsolete">Status</translation>
    </message>
</context>
<context>
    <name>QgsEditReservedWordsDialog</name>
    <message>
        <source>Column Name</source>
        <translation type="obsolete">KolomNaam</translation>
    </message>
    <message>
        <source>Index</source>
        <translation type="obsolete">Inhoud</translation>
    </message>
    <message>
        <source>Status</source>
        <translation type="obsolete">Status</translation>
    </message>
</context>
<context>
    <name>QgsEncodingFileDialog</name>
    <message>
        <source>Encoding:</source>
        <translation>Encoding:</translation>
    </message>
</context>
<context>
    <name>QgsFillStyleWidgetBase</name>
    <message>
        <source>col</source>
        <translation type="obsolete">kleur</translation>
    </message>
    <message>
        <source>Colour:</source>
        <translation type="obsolete">Kleur:</translation>
    </message>
    <message>
        <source>Fill Style</source>
        <translation type="obsolete">Vulling</translation>
    </message>
    <message>
        <source>Form1</source>
        <translation type="obsolete">Form1</translation>
    </message>
    <message>
        <source>PolyStyleWidget</source>
        <translation type="obsolete">PolyStyleWidget</translation>
    </message>
</context>
<context>
    <name>QgsGPSDeviceDialog</name>
    <message>
        <source>Are you sure?</source>
        <translation>Weet u het zeker?</translation>
    </message>
    <message>
        <source>Are you sure that you want to delete this device?</source>
        <translation>Weet u zeker dat u dit apparaat wilt verwijderen?</translation>
    </message>
    <message>
        <source>New device %1</source>
        <translation>Nieuw apparaat %1</translation>
    </message>
</context>
<context>
    <name>QgsGPSDeviceDialogBase</name>
    <message>
        <source>Close</source>
        <translation type="obsolete">Sluiten</translation>
    </message>
    <message>
        <source>Commands</source>
        <translation>Commando&apos;s</translation>
    </message>
    <message>
        <source>Delete device</source>
        <translation>Verwijder apparaat</translation>
    </message>
    <message>
        <source>Device name:</source>
        <translation type="obsolete">Apparaatnaam:</translation>
    </message>
    <message>
        <source>GPS Device Editor</source>
        <translation>GPS-apparaatinstellingen Aanpassen</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;In the download and upload commands there can be special words that will be replaced by QGIS when the commands are used. These words are:&lt;span style=&quot; font-style:italic;&quot;&gt;%babel&lt;/span&gt; - the path to GPSBabel&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%in&lt;/span&gt; - the GPX filename when uploading or the port when downloading&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%out&lt;/span&gt; - the port when uploading or the GPX filename when downloading&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;In de upload en download commando&apos;s kunnen speciale termen voorkomen die zullen worden vervangen door QGIS wanneer de commando&apos;s worden gebruikt. Deze termen zijn:&lt;span style=&quot; font-style:italic;&quot;&gt;%babel&lt;/span&gt; - het pad naar GPSBabel&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%in&lt;/span&gt; - de GPX-bestandsnaam bij uploaden of het poortnummer bij downloaden&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%out&lt;/span&gt; - het poortnummer bij uploaden of the GPX-bestandsnaam bij downloaden.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>New device</source>
        <translation>Nieuw apparaat</translation>
    </message>
    <message>
        <source>Route download:</source>
        <translation>Route downloaden:</translation>
    </message>
    <message>
        <source>Route upload:</source>
        <translation>Route uploaden:</translation>
    </message>
    <message>
        <source>The command that is used to download routes from the device</source>
        <translation>Het gebruikte commando wat wordt gebruikt voor het downloaden van het apparaat</translation>
    </message>
    <message>
        <source>The command that is used to download tracks from the device</source>
        <translation>Het gebruikte commando voor het downloaden van tracks van het apparaat</translation>
    </message>
    <message>
        <source>The command that is used to download waypoints from the device</source>
        <translation>Het commando om de waypoints van de gps te downloaden</translation>
    </message>
    <message>
        <source>The command that is used to upload routes to the device</source>
        <translation>Het commando om de routes te uploaden naar de gps</translation>
    </message>
    <message>
        <source>The command that is used to upload tracks to the device</source>
        <translation>Het commando om de tracks te uploaden naar de gps</translation>
    </message>
    <message>
        <source>The command that is used to upload waypoints to the device</source>
        <translation>Het commando om de waypoints te uploaden naar de gps</translation>
    </message>
    <message>
        <source>This is the name of the device as it will appear in the lists</source>
        <translation>Dit is de naam van het apparaat zoals te zien in de lijsten</translation>
    </message>
    <message>
        <source>Track download:</source>
        <translation>Track download:</translation>
    </message>
    <message>
        <source>Track upload:</source>
        <translation>Track upload:</translation>
    </message>
    <message>
        <source>Update device</source>
        <translation>Gps-apparaat update</translation>
    </message>
    <message>
        <source>Waypoint download:</source>
        <translation>Download waypoint:</translation>
    </message>
    <message>
        <source>Waypoint upload:</source>
        <translation>Upload waypoint:</translation>
    </message>
    <message>
        <source>Device name</source>
        <translation>Apparaatnaam</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;In the download and upload commands there can be special words that will be replaced by QGIS when the commands are used. These words are:&lt;span style=&quot; font-style:italic;&quot;&gt;%babel&lt;/span&gt; - the path to GPSBabel&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%in&lt;/span&gt; - the GPX filename when uploading or the port when downloading&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%out&lt;/span&gt; - the port when uploading or the GPX filename when downloading&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;p, li { white-space: pre-wrap; }&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;Bij de upload en download-commando&apos;s kunnen speciale termen worden gebruikt die door QGIS zullen worden vervangen wanneer de commandos&apos;s worden gebruikt. Dit zijn de termen:&lt;span style=&quot; font-style:italic;&quot;&gt;%babel&lt;/span&gt; - het pad naar GPSBabel&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%in&lt;/span&gt; - de GPX-bestandsnam bij een upload, of de poort bij een download&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%out&lt;/span&gt; - de poort bij een upload, de GPX-bestandsnaam bij een download.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
</context>
<context>
    <name>QgsGPSPlugin</name>
    <message>
        <source>Cancel</source>
        <translation>Annuleren</translation>
    </message>
    <message>
        <source>Could not convert data from %1!

</source>
        <translation>Probleem bij converteren van data van %1!

</translation>
    </message>
    <message>
        <source>Could not create file</source>
        <translation>Aanmaken van bestand mislukt</translation>
    </message>
    <message>
        <source>Could not download data from GPS!

</source>
        <translation>Download data van GPS niet mogelijk!

</translation>
    </message>
    <message>
        <source>Could not import data from %1!

</source>
        <translation>Importeren data mislukt van %1!

</translation>
    </message>
    <message>
        <source>Could not start GPSBabel!</source>
        <translation>GPSBabel kan niet worden opgestart!</translation>
    </message>
    <message>
        <source>Could not start process</source>
        <translation>Proces starten niet mogelijk</translation>
    </message>
    <message>
        <source>&amp;Create new GPX layer</source>
        <translation>&amp;Creëer nieuwe GPX-laag</translation>
    </message>
    <message>
        <source>Creates a new GPX layer and displays it on the map canvas</source>
        <translation>Creëert een nieuwe GPX-laag en toont die op de kaart</translation>
    </message>
    <message>
        <source>directory.</source>
        <translation>map.</translation>
    </message>
    <message>
        <source>Downloading data...</source>
        <translation>Data wordt gedownload...</translation>
    </message>
    <message>
        <source>Error converting data</source>
        <translation>Fout bij dataconversie</translation>
    </message>
    <message>
        <source>Error downloading data</source>
        <translation>Fout bij download van data</translation>
    </message>
    <message>
        <source>Error importing data</source>
        <translation>Fout bij importeren van data</translation>
    </message>
    <message>
        <source>Error uploading data</source>
        <translation>Fout bij uploaden van data</translation>
    </message>
    <message>
        <source>Error while uploading data to GPS!

</source>
        <translation>Fout tijdens het uploaden van data naar GPS!

</translation>
    </message>
    <message>
        <source>&amp;Gps</source>
        <translation>&amp;Gps</translation>
    </message>
    <message>
        <source>GPS eXchange file (*.gpx)</source>
        <translation>GPS uitwisselingsbestand (*.gpx)</translation>
    </message>
    <message>
        <source>&amp;Gps Tools</source>
        <translation>&amp;GPS-gereedschap</translation>
    </message>
    <message>
        <source>GPX Loader</source>
        <translation>GPX-lader</translation>
    </message>
    <message>
        <source>Importing data...</source>
        <translation>Data wordt geïmporteerd...</translation>
    </message>
    <message>
        <source>Not supported</source>
        <translation>Wordt niet ondersteund</translation>
    </message>
    <message>
        <source>of </source>
        <translation>van </translation>
    </message>
    <message>
        <source>Please reselect a valid file.</source>
        <translation>Selecteer een geldig bestand a.u.b.</translation>
    </message>
    <message>
        <source>Save new GPX file as...</source>
        <translation>Nieuwe GPX-bestand opslaan als...</translation>
    </message>
    <message>
        <source>This device does not support downloading </source>
        <translation>Downloaden  van data wordt niet ondersteund door dit GPS-apparaat </translation>
    </message>
    <message>
        <source>This device does not support uploading of </source>
        <translation>Uploaden wordt niet ondersteund door dit GPS-apparaat </translation>
    </message>
    <message>
        <source>Try again with another name or in another </source>
        <translation>Probeer opnieuw onder een andere naam of in een andere </translation>
    </message>
    <message>
        <source>Unable to create a GPX file with the given name. </source>
        <translation>Aanmaken van GPX-bestand met deze naam niet mogelijk.</translation>
    </message>
    <message>
        <source>Unable to read the selected file.
</source>
        <translation>Inlezen van geselecteerde bestand niet mogelijk.
</translation>
    </message>
    <message>
        <source>Uploading data...</source>
        <translation>Data wordt geupload...</translation>
    </message>
</context>
<context>
    <name>QgsGPSPluginGui</name>
    <message>
        <source>All file formats can not store waypoints, routes, and tracks, so some feature types may be disabled for some file formats.</source>
        <translation>Niet alle formaten kunnen waypoints, routes en tracks opslaan, sommige objecttypen zullen niet werkzaam zijn.</translation>
    </message>
    <message>
        <source>Choose a filename to save under</source>
        <translation type="obsolete">Kies bestandsnaam om te bewaren als</translation>
    </message>
    <message>
        <source>Choose the layer you want to upload, the device you want to upload it to, and the port your device is connected to.</source>
        <translation>Kies de te uploaden laag, het apparaat waarnaartoe geupload moet worden, en de poort waaraan het apparaat is verbonden.</translation>
    </message>
    <message>
        <source>Choose your GPS device, the port it is connected to, the feature type you want to download, a name for your new layer, and the GPX file where you want to store the data.</source>
        <translation>Kies uw type GPS-apparaat, de verbindingspoort, het te downloaden objecttype, een naam voor de nieuwe laag, en het GPX-bestand waarin u de data wilt opslaan.</translation>
    </message>
    <message>
        <source>GPS eXchange file format</source>
        <translation>GPS-uitwisselings (eXchange) bestandsformaat</translation>
    </message>
    <message>
        <source>GPS eXchange format (*.gpx)</source>
        <translation>GPs uitwisselingsformat (*.gpx)</translation>
    </message>
    <message>
        <source>GPX is the %1, which is used to store information about waypoints, routes, and tracks.</source>
        <translation>GPX is het %1, om waypoints, routes en tracks te bewaren.</translation>
    </message>
    <message>
        <source>If your device isn&apos;t listed, or if you want to change some settings, you can also edit the devices.</source>
        <translation>Als uw apparaat niet in de lijst staat, of als u instellingen wilt wijzigen, dan kunt u een apparaatinstelling aanpassen.</translation>
    </message>
    <message>
        <source>QGIS can only load GPX files by itself, but many other formats can be converted to GPX using GPSBabel (%1).</source>
        <translation>QGIS kan alleen sec GPX-bestanden laden, maar andere formaten kunnen worden omgezet naar GPX m.b.v. GPSBabel (%1).</translation>
    </message>
    <message>
        <source>QGIS can perform conversions of GPX files, by using GPSBabel (%1) to perform the conversions.</source>
        <translation>QGIS kan GPX-bestanden converteren, door gebruik te maken van GPSBabel (%1).</translation>
    </message>
    <message>
        <source>Routes</source>
        <translation>Routes</translation>
    </message>
    <message>
        <source>Select a GPS file format and the file that you want to import, the feature type that you want to use, a GPX filename that you want to save the converted file as, and a name for the new layer.</source>
        <translation type="obsolete">Selecteer een GPS-bestandsformaat en het te importeren bestand, het objecttype dat u wilt gebruiken, een GPX-bestandsnaam voor het geconverteerde bestand, en een naam voor de nieuwe laag.</translation>
    </message>
    <message>
        <source>Select a GPX file and then select the feature types that you want to load.</source>
        <translation>Selecteer een GPX-bestand en selecteer dan de objecttypen die u wilt inlezen.</translation>
    </message>
    <message>
        <source>Select a GPX input file name, the type of conversion you want to perform, a GPX filename that you want to save the converted file as, and a name for the new layer created from the result.</source>
        <translation type="obsolete">Selecteer een GPS-bestandsformaat, het type conversie, een GPX-bestandsnaam voor het geconverteerde bestand, en een naam voor de nieuwe laag welke het resultaat van de conversie zal zijn.</translation>
    </message>
    <message>
        <source>Select file and format to import</source>
        <translation>Selecteer een betand en formaat om te importeren</translation>
    </message>
    <message>
        <source>Select GPX file</source>
        <translation>Selecteer GPX-bestand</translation>
    </message>
    <message>
        <source>This requires that you have GPSBabel installed where QGIS can find it.</source>
        <translation>Dit vereist dat u GPSBabel heeft geinstalleerd en QGIS weet waar het het kan vinden.</translation>
    </message>
    <message>
        <source>This tool uses the program GPSBabel (%1) to transfer the data.</source>
        <translation>Deze functie gebruikt het programma GPSBabel (%1) om de data te over te zetten.</translation>
    </message>
    <message>
        <source>This tool will help you download data from a GPS device.</source>
        <translation>Deze functie zal u helpen om data van uw GPS-apparaat uit te lezen.</translation>
    </message>
    <message>
        <source>This tool will help you upload data from a GPX layer to a GPS device.</source>
        <translation>Deze functie zal u helpen om data vanuit de GPX-laag naar uw GPS-apparaat te uploaden.</translation>
    </message>
    <message>
        <source>Tracks</source>
        <translation>Tracks</translation>
    </message>
    <message>
        <source>Waypoints</source>
        <translation>Waypoints</translation>
    </message>
    <message>
        <source>Choose a file name to save under</source>
        <translation>Kies bestandsnaam om te bewaren als</translation>
    </message>
    <message>
        <source>Select a GPS file format and the file that you want to import, the feature type that you want to use, a GPX file name that you want to save the converted file as, and a name for the new layer.</source>
        <translation>Selecteer een GPS-bestandsformaat en het te importeren bestand, het objecttype dat u wilt gebruiken, een GPX-bestandsnaam voor het geconverteerde bestand, en een naam voor de nieuwe laag.</translation>
    </message>
    <message>
        <source>Select a GPX input file name, the type of conversion you want to perform, a GPX file name that you want to save the converted file as, and a name for the new layer created from the result.</source>
        <translation>Selecteer een GPS-bestandsformaat, het type conversie, een GPX-bestandsnaam voor het geconverteerde bestand, en een naam voor de nieuwe laag welke het resultaat van de conversie zal zijn.</translation>
    </message>
</context>
<context>
    <name>QgsGPSPluginGuiBase</name>
    <message>
        <source>Browse...</source>
        <translation>Bladeren...</translation>
    </message>
    <message>
        <source>Conversion:</source>
        <translation>Converteren:</translation>
    </message>
    <message>
        <source>Data layer:</source>
        <translation>Datalaag:</translation>
    </message>
    <message>
        <source>Download from GPS</source>
        <translation>Download van GPS</translation>
    </message>
    <message>
        <source>Edit devices</source>
        <translation>Apparaten aanpassen</translation>
    </message>
    <message>
        <source>Feature type:</source>
        <translation>Object type:</translation>
    </message>
    <message>
        <source>Feature types:</source>
        <translation>Object typen:</translation>
    </message>
    <message>
        <source>File:</source>
        <translation>Bestand:</translation>
    </message>
    <message>
        <source>File to import:</source>
        <translation>Te importeren bestand:</translation>
    </message>
    <message>
        <source>GPS device:</source>
        <translation>GPS-apparaat:</translation>
    </message>
    <message>
        <source>GPS Tools</source>
        <translation>GPS-gereedschap</translation>
    </message>
    <message>
        <source>GPX Conversions</source>
        <translation>GPX-Conversies</translation>
    </message>
    <message>
        <source>GPX input file:</source>
        <translation>GPS invoerbestand:</translation>
    </message>
    <message>
        <source>GPX output file:</source>
        <translation>GPX uitvoerbestand:</translation>
    </message>
    <message>
        <source>Import other file</source>
        <translation>Ander bestand importeren</translation>
    </message>
    <message>
        <source>Layer name:</source>
        <translation>Laagnaam:</translation>
    </message>
    <message>
        <source>Load GPX file</source>
        <translation>GPX bestand laden</translation>
    </message>
    <message>
        <source>(Note: Selecting correct file type in browser dialog important!)</source>
        <translation>(Opmerking: het is belangrijk het juiste bestandstype te selecteren in het dialoog!)</translation>
    </message>
    <message>
        <source>Output file:</source>
        <translation>Uitvoerbestand:</translation>
    </message>
    <message>
        <source>Port:</source>
        <translation>Poort:</translation>
    </message>
    <message>
        <source>Routes</source>
        <translation>Routes</translation>
    </message>
    <message>
        <source>Save As...</source>
        <translation>Opslaan Als...</translation>
    </message>
    <message>
        <source>Tracks</source>
        <translation>Tracks</translation>
    </message>
    <message>
        <source>Upload to GPS</source>
        <translation>Naar GPS uploaden</translation>
    </message>
    <message>
        <source>Waypoints</source>
        <translation>Waypoints</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;p, li { white-space: pre-wrap; }&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;p, li { white-space: pre-wrap; }&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Edit devices...</source>
        <translation>Apparaten aanpassen...</translation>
    </message>
    <message>
        <source>Refresh</source>
        <translation>Bijwerken</translation>
    </message>
</context>
<context>
    <name>QgsGPXProvider</name>
    <message>
        <source>Bad URI - you need to specify the feature type.</source>
        <translation>Foute URI - het objecttype moet worden gegeven.</translation>
    </message>
    <message>
        <source>Digitized in QGIS</source>
        <translation>Gedigitaliseerd in QGIS</translation>
    </message>
    <message>
        <source>GPS eXchange file</source>
        <translation>GPS uitwisselingsbestand (gpx)</translation>
    </message>
</context>
<context>
    <name>QgsGenericProjectionSelector</name>
    <message>
        <source>Define this layer&apos;s projection:</source>
        <translation>Definieer projectie voor deze laag:</translation>
    </message>
    <message>
        <source>This layer appears to have no projection specification.</source>
        <translation>De laag lijkt geen projectie-instelling te bevatten.</translation>
    </message>
    <message>
        <source>By default, this layer will now have its projection set to that of the project, but you may override this by selecting a different projection below.</source>
        <translation>Standaard krijgt de laag de projectie zoals die is ingesteld voor het project, maar u kunt dit aanpassen door een andere projectie hieronder te kiezen.</translation>
    </message>
</context>
<context>
    <name>QgsGenericProjectionSelectorBase</name>
    <message>
        <source>Projection Selector</source>
        <translation>Projectie Selector</translation>
    </message>
</context>
<context>
    <name>QgsGeomTypeDialog</name>
    <message>
        <source>Name</source>
        <translation type="obsolete">Naam</translation>
    </message>
    <message>
        <source>Type</source>
        <translation type="obsolete">Type</translation>
    </message>
    <message>
        <source>Real</source>
        <translation>Real</translation>
    </message>
    <message>
        <source>Integer</source>
        <translation>Integer</translation>
    </message>
    <message>
        <source>String</source>
        <translation>String</translation>
    </message>
</context>
<context>
    <name>QgsGeomTypeDialogBase</name>
    <message>
        <source>Add</source>
        <translation type="obsolete">Toevoegen</translation>
    </message>
    <message>
        <source>Attributes:</source>
        <translation type="obsolete">Attributen:</translation>
    </message>
    <message>
        <source>Column 1</source>
        <translation type="obsolete">Kolom 1</translation>
    </message>
    <message>
        <source>File Format:</source>
        <translation type="obsolete">Bestandsformaat:</translation>
    </message>
    <message>
        <source>Line</source>
        <translation>Lijn</translation>
    </message>
    <message>
        <source>New Vector Layer</source>
        <translation>Nieuwe Vectorlaag</translation>
    </message>
    <message>
        <source>Point</source>
        <translation>Punt</translation>
    </message>
    <message>
        <source>Polygon</source>
        <translation>Polygoon</translation>
    </message>
    <message>
        <source>Remove</source>
        <translation type="obsolete">Verwijder</translation>
    </message>
    <message>
        <source>Type</source>
        <translation>Type</translation>
    </message>
    <message>
        <source>File format</source>
        <translation>Bestandsformaat</translation>
    </message>
    <message>
        <source>Attributes</source>
        <translation>Attributen</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Naam</translation>
    </message>
    <message>
        <source>Remove selected row</source>
        <translation type="obsolete">Verwijder geselecteerde rij</translation>
    </message>
    <message>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <source>Add values manually</source>
        <translation type="obsolete">Voeg handmatig waarden in</translation>
    </message>
    <message>
        <source>Delete selected attribute</source>
        <translation>Verwijder geselecteerde attribuut</translation>
    </message>
    <message>
        <source>Add attribute</source>
        <translation>Attribuut Toevoegen</translation>
    </message>
</context>
<context>
    <name>QgsGeorefDescriptionDialogBase</name>
    <message>
        <source>Description georeferencer</source>
        <translation>Beschrijving GeoReferencer</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:12pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:11pt; font-weight:600;&quot;&gt;Description&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:9pt;&quot;&gt;This plugin can generate world files for rasters. You select points on the raster and give their world coordinates, and the plugin will compute the world file parameters. The more coordinates you can provide the better the result will be.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;p, li { white-space: pre-wrap; }&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:12pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:11pt; font-weight:600;&quot;&gt;Beschrijving&lt;/span&gt;&lt;/p&gt;&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:9pt;&quot;&gt;Deze plugin can zogenaamde &apos;world files&apos; voor rasterbestanden aanmaken. U selecteert punten op het raster en geeft daarvoor wereldcoördinaten aan. De plugin zal dan de parameters voor de &apos;world file&apos; berekenen. Hoe meer coördinaten u aangeeft hoe beter het resultaat zal zijn.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
</context>
<context>
    <name>QgsGeorefPlugin</name>
    <message>
        <source>&amp;Georeferencer</source>
        <translation>&amp;Georeferencer</translation>
    </message>
</context>
<context>
    <name>QgsGeorefPluginGui</name>
    <message>
        <source>Choose a raster file</source>
        <translation>Kies een rasterbestand</translation>
    </message>
    <message>
        <source>Error</source>
        <translation>Fout</translation>
    </message>
    <message>
        <source>new world file?&lt;/p&gt;</source>
        <translation>nieuw &apos;world file&apos;?&lt;/p&gt;</translation>
    </message>
    <message>
        <source>&lt;p&gt;The selected file already seems to have a </source>
        <translation>&lt;p&gt;Het geselecteerde bestand heeft blijkbaar al een  </translation>
    </message>
    <message>
        <source>Raster files (*.*)</source>
        <translation>Rasterbestanden (*.*)</translation>
    </message>
    <message>
        <source>The selected file is not a valid raster file.</source>
        <translation>Het geselecteerde bestand is niet een geldig rasterbestand.</translation>
    </message>
    <message>
        <source>world file! Do you want to replace it with the </source>
        <translation>&apos;world file&apos;! Wilt u het vervangen door de  </translation>
    </message>
    <message>
        <source>World file exists</source>
        <translation>&apos;World file&apos; bestaat al</translation>
    </message>
</context>
<context>
    <name>QgsGeorefPluginGuiBase</name>
    <message>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <source>Arrange plugin windows</source>
        <translation>Pluginvensters schikken</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Sluiten</translation>
    </message>
    <message>
        <source>Description...</source>
        <translation>Omschrijving...</translation>
    </message>
    <message>
        <source>Georeferencer</source>
        <translation>&apos;Georeferencer&apos;</translation>
    </message>
    <message>
        <source>Raster file:</source>
        <translation>Rasterbestand:</translation>
    </message>
</context>
<context>
    <name>QgsGeorefWarpOptionsDialog</name>
    <message>
        <source>unstable</source>
        <translation type="obsolete">onstabiel</translation>
    </message>
</context>
<context>
    <name>QgsGeorefWarpOptionsDialogBase</name>
    <message>
        <source>Compression:</source>
        <translation>Compressie:</translation>
    </message>
    <message>
        <source>Cubic</source>
        <translation>Cubic</translation>
    </message>
    <message>
        <source>Linear</source>
        <translation>Lineair</translation>
    </message>
    <message>
        <source>Nearest neighbour</source>
        <translation>Nearest neighbour</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>Resampling method:</source>
        <translation>&apos;Resample&apos;-methode:</translation>
    </message>
    <message>
        <source>Use 0 for transparency when needed</source>
        <translation>Gebruik 0 voor transparantie indien nodig</translation>
    </message>
    <message>
        <source>Warp options</source>
        <translation>&apos;Warp&apos;-opties</translation>
    </message>
</context>
<context>
    <name>QgsGraduatedSymbolDialog</name>
    <message>
        <source>Empty</source>
        <translation>Leeg</translation>
    </message>
    <message>
        <source>Equal Interval</source>
        <translation>Gelijke Intervals</translation>
    </message>
    <message>
        <source>Quantiles</source>
        <translation>Quantilen</translation>
    </message>
</context>
<context>
    <name>QgsGraduatedSymbolDialogBase</name>
    <message>
        <source>Classification Field:</source>
        <translation type="obsolete">Veld voor classificatie:</translation>
    </message>
    <message>
        <source>Classify</source>
        <translation>Classificeren</translation>
    </message>
    <message>
        <source>Delete class</source>
        <translation>Klasse Verwijderen</translation>
    </message>
    <message>
        <source>graduated Symbol</source>
        <translation>Symbool in graduaties</translation>
    </message>
    <message>
        <source>Mode:</source>
        <translation type="obsolete">Modus:</translation>
    </message>
    <message>
        <source>Number of Classes:</source>
        <translation type="obsolete">Aantal klassen:</translation>
    </message>
    <message>
        <source>Classification field</source>
        <translation>Veld voor classificatie:</translation>
    </message>
    <message>
        <source>Mode</source>
        <translation>Modus</translation>
    </message>
    <message>
        <source>Number of classes</source>
        <translation>Aantal klassen</translation>
    </message>
</context>
<context>
    <name>QgsGrassAttributes</name>
    <message>
        <source>Column</source>
        <translation>Kolom</translation>
    </message>
    <message>
        <source>ERROR</source>
        <translation>FOUT</translation>
    </message>
    <message>
        <source>Layer</source>
        <translation>Laag</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>Type</source>
        <translation>Type</translation>
    </message>
    <message>
        <source>Value</source>
        <translation>Waarde</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Waarschuwing</translation>
    </message>
</context>
<context>
    <name>QgsGrassAttributesBase</name>
    <message>
        <source>Add new category using settings in GRASS Edit toolbox</source>
        <translation>Nieuwe categorie toevoegen op basis van GRASS-Edit toolbox instellingen</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>Verwijderen</translation>
    </message>
    <message>
        <source>Delete selected category</source>
        <translation>Verwijder geselecteerde category</translation>
    </message>
    <message>
        <source>GRASS Attributes</source>
        <translation>GRASS Attributen</translation>
    </message>
    <message>
        <source>New</source>
        <translation>Nieuw</translation>
    </message>
    <message>
        <source>result</source>
        <translation>resultaat</translation>
    </message>
    <message>
        <source>Tab 1</source>
        <translation>Tab 1</translation>
    </message>
    <message>
        <source>Update</source>
        <translation>Bijwerken</translation>
    </message>
    <message>
        <source>Update database record</source>
        <translation>Database record updaten</translation>
    </message>
</context>
<context>
    <name>QgsGrassBrowser</name>
    <message>
        <source>Add selected map to canvas</source>
        <translation>Voeg geselecteerde kaart toe aan kaartvenster</translation>
    </message>
    <message>
        <source>&lt;br&gt;command: </source>
        <translation>&lt;br&gt;opdracht:</translation>
    </message>
    <message>
        <source>Cannot copy map </source>
        <translation>Kan kaart niet kopiëren</translation>
    </message>
    <message>
        <source>Cannot delete map </source>
        <translation>Kan kaart niet verwijderen</translation>
    </message>
    <message>
        <source>Cannot rename map </source>
        <translation>Kan kaart niet hernoemen</translation>
    </message>
    <message>
        <source>Cannot write new region</source>
        <translation>Kan nieuwe region niet opslaan</translation>
    </message>
    <message>
        <source>Copy selected map</source>
        <translation>Kopiëer geselecteerde kaart</translation>
    </message>
    <message>
        <source>Delete map &lt;b&gt;</source>
        <translation>Verwijder kaart &lt;b&gt;</translation>
    </message>
    <message>
        <source>Delete selected map</source>
        <translation>Verwijder geselecteerde kaart</translation>
    </message>
    <message>
        <source>New name</source>
        <translation>Nieuwe naam</translation>
    </message>
    <message>
        <source>Refresh</source>
        <translation>Bijwerken</translation>
    </message>
    <message>
        <source>Rename selected map</source>
        <translation>Hernoem geselecteerde kaart</translation>
    </message>
    <message>
        <source>Set current region to selected map</source>
        <translation>Huidige gebied toepassen op geselecteerde kaart</translation>
    </message>
    <message>
        <source>Tools</source>
        <translation>Hulpmiddelen</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Waarschuwing</translation>
    </message>
</context>
<context>
    <name>QgsGrassEdit</name>
    <message>
        <source>Add vertex</source>
        <translation>Voeg hoekpunt toe</translation>
    </message>
    <message>
        <source>Background</source>
        <translation>Achtergrond</translation>
    </message>
    <message>
        <source>Boundary (1 area)</source>
        <translation>Grenzen (1 area)</translation>
    </message>
    <message>
        <source>Boundary (2 areas)</source>
        <translation>Grenzen (2 areas)</translation>
    </message>
    <message>
        <source>Boundary (no area)</source>
        <translation>Grenzen (geen area)</translation>
    </message>
    <message>
        <source>Cannot check orphan record: </source>
        <translation>Check op &apos;orphan record&apos; niet mogelijk:</translation>
    </message>
    <message>
        <source>Cannot delete orphan record: </source>
        <translation>Verwijderen van &apos;orphan record&apos; niet mogelijk:</translation>
    </message>
    <message>
        <source>Cannot describe table for field </source>
        <translation>Geen beschrijving van tabel voor veld beschikbaar</translation>
    </message>
    <message>
        <source>Cannot open vector for update.</source>
        <translation>Openen van vector voor update mislukt</translation>
    </message>
    <message>
        <source>Centroid (duplicate in area)</source>
        <translation>Centroid (duplicate in area)</translation>
    </message>
    <message>
        <source>Centroid (in area)</source>
        <translation>Centroid (in area)</translation>
    </message>
    <message>
        <source>Centroid (outside area)</source>
        <translation>Centroid (buiten area)</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Sluiten</translation>
    </message>
    <message>
        <source>Color</source>
        <comment>Column title</comment>
        <translation type="obsolete">Kleur</translation>
    </message>
    <message>
        <source>Column</source>
        <translation type="obsolete">Kolom</translation>
    </message>
    <message>
        <source>Delete element</source>
        <translation>Verwijder element</translation>
    </message>
    <message>
        <source>Delete vertex</source>
        <translation>Verwijder hoekpunt</translation>
    </message>
    <message>
        <source>Disp</source>
        <comment>Column title</comment>
        <translation type="obsolete">Disp</translation>
    </message>
    <message>
        <source>Dynamic</source>
        <translation>Dynamisch</translation>
    </message>
    <message>
        <source>Edit attributes</source>
        <translation>Bewerk attributen</translation>
    </message>
    <message>
        <source>Edit tools</source>
        <translation>Aanpas-gereedschap</translation>
    </message>
    <message>
        <source>Highlight</source>
        <translation>Accentueren</translation>
    </message>
    <message>
        <source>Index</source>
        <comment>Column title</comment>
        <translation type="obsolete">Index</translation>
    </message>
    <message>
        <source>Info</source>
        <translation>Informatie</translation>
    </message>
    <message>
        <source>Left: </source>
        <translation>Links:</translation>
    </message>
    <message>
        <source>Length</source>
        <translation type="obsolete">Lengte</translation>
    </message>
    <message>
        <source>Line</source>
        <translation>Lijn</translation>
    </message>
    <message>
        <source>Manual entry</source>
        <translation>Handmatige invoer</translation>
    </message>
    <message>
        <source>Middle: </source>
        <translation>Midden:</translation>
    </message>
    <message>
        <source>Move element</source>
        <translation>Verplaats element</translation>
    </message>
    <message>
        <source>Move vertex</source>
        <translation>Verplaats hoekpunt</translation>
    </message>
    <message>
        <source>New boundary</source>
        <translation>Nieuwe begrenzing</translation>
    </message>
    <message>
        <source>New centroid</source>
        <translation>Nieuw zwaartepunt/centroid</translation>
    </message>
    <message>
        <source>New line</source>
        <translation>Nieuwe lijn</translation>
    </message>
    <message>
        <source>New point</source>
        <translation>Nieuw punt</translation>
    </message>
    <message>
        <source>Next not used</source>
        <translation>Volgende niet gebrukt</translation>
    </message>
    <message>
        <source>No category</source>
        <translation>Geen categorie</translation>
    </message>
    <message>
        <source>Node (1 line)</source>
        <translation>Knooppunt (1 lijn)</translation>
    </message>
    <message>
        <source>Node (2 lines)</source>
        <translation>Knooppunt (2 lijnen)</translation>
    </message>
    <message>
        <source>Orphan record was left in attribute table. &lt;br&gt;Delete the record?</source>
        <translation>Er is een &apos;orphan record&apos; achtergebleven in de attribuuttable.. &lt;br&gt;Die record verwijderen?</translation>
    </message>
    <message>
        <source>Point</source>
        <translation>Punt</translation>
    </message>
    <message>
        <source>Right: </source>
        <translation>Rechts:</translation>
    </message>
    <message>
        <source>Split line</source>
        <translation>Splits lijn</translation>
    </message>
    <message>
        <source>The table was created</source>
        <translation>De tabel is aangemaakt</translation>
    </message>
    <message>
        <source>Tool not yet implemented.</source>
        <translation>Hulpmiddel nog niet geïmplementeerd.</translation>
    </message>
    <message>
        <source>Type</source>
        <translation type="obsolete">Type</translation>
    </message>
    <message>
        <source>Type</source>
        <comment>Column title</comment>
        <translation type="obsolete">Type</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Waarschuwing</translation>
    </message>
    <message>
        <source>You are not owner of the mapset, cannot open the vector for editing.</source>
        <translation>U bent niet de eigenaar van de &quot;mapset&quot;, de &quot;vector&quot; kan niet bewerkt worden.</translation>
    </message>
</context>
<context>
    <name>QgsGrassEditBase</name>
    <message>
        <source>Add Column</source>
        <translation>Kolom Toevoegen</translation>
    </message>
    <message>
        <source>Category</source>
        <translation>Categorie</translation>
    </message>
    <message>
        <source>Column 1</source>
        <translation type="obsolete">Kolom 1</translation>
    </message>
    <message>
        <source>Create / Alter Table</source>
        <translation>Tabel Aanmaken / Veranderen</translation>
    </message>
    <message>
        <source>GRASS Edit</source>
        <translation>GRASS Edit</translation>
    </message>
    <message>
        <source>Layer</source>
        <translation>Laag</translation>
    </message>
    <message>
        <source>Line width</source>
        <translation>Lijndikte</translation>
    </message>
    <message>
        <source>Marker size</source>
        <translation>Markergrootte</translation>
    </message>
    <message>
        <source>Mode</source>
        <translation>Modus</translation>
    </message>
    <message>
        <source>Settings</source>
        <translation>Extra</translation>
    </message>
    <message>
        <source>Snapping in screen pixels</source>
        <translation>&apos;Snapping&apos; in schermpixels</translation>
    </message>
    <message>
        <source>Symbology</source>
        <translation>Symbologie</translation>
    </message>
    <message>
        <source>Table</source>
        <translation>Tabel</translation>
    </message>
    <message>
        <source>Disp</source>
        <translation>Disp</translation>
    </message>
    <message>
        <source>Color</source>
        <translation>Kleur</translation>
    </message>
    <message>
        <source>Type</source>
        <translation>Type</translation>
    </message>
    <message>
        <source>Index</source>
        <translation>Index</translation>
    </message>
    <message>
        <source>Column</source>
        <translation>Kolom</translation>
    </message>
    <message>
        <source>Length</source>
        <translation>Lengte</translation>
    </message>
</context>
<context>
    <name>QgsGrassElementDialog</name>
    <message>
        <source>Cancel</source>
        <translation>Annuleren</translation>
    </message>
    <message>
        <source>&lt;font color=&apos;red&apos;&gt;Enter a name!&lt;/font&gt;</source>
        <translation>&lt;font color=&apos;red&apos;&gt;Geef een naam!&lt;/font&gt;</translation>
    </message>
    <message>
        <source>&lt;font color=&apos;red&apos;&gt;Exists!&lt;/font&gt;</source>
        <translation>&lt;font color=&apos;red&apos;&gt;Bestaat!&lt;/font&gt;</translation>
    </message>
    <message>
        <source>&lt;font color=&apos;red&apos;&gt;This is name of the source!&lt;/font&gt;</source>
        <translation>&lt;font color=&apos;red&apos;&gt;Dit is de bronnaam!&lt;/font&gt;</translation>
    </message>
    <message>
        <source>Ok</source>
        <translation>Ok</translation>
    </message>
    <message>
        <source>Overwrite</source>
        <translation>Overschrijven</translation>
    </message>
</context>
<context>
    <name>QgsGrassMapcalc</name>
    <message>
        <source>1 if x is zero, 0 otherwise</source>
        <translation>1 als x nul is, anders 0</translation>
    </message>
    <message>
        <source>Absolute value of x</source>
        <translation>Absolutie waarde van x</translation>
    </message>
    <message>
        <source>Add connection</source>
        <translation>Verbinding toevoegen</translation>
    </message>
    <message>
        <source>Add constant value</source>
        <translation>Constante toevoegen</translation>
    </message>
    <message>
        <source>Addition</source>
        <translation>Optellen</translation>
    </message>
    <message>
        <source>Add map</source>
        <translation>Kaart toevoegen</translation>
    </message>
    <message>
        <source>Add operator or function</source>
        <translation>Operator of functie toevoegen</translation>
    </message>
    <message>
        <source>And</source>
        <translation>En</translation>
    </message>
    <message>
        <source>
at line </source>
        <translation>
in regel</translation>
    </message>
    <message>
        <source>Cannot check region of map </source>
        <translation>Kan regio van de kaart niet checken</translation>
    </message>
    <message>
        <source>Cannot create &apos;mapcalc&apos; directory in current mapset.</source>
        <translation>Kan geen &apos;mapcalc&apos;-map aanmaken in de huidige &apos;mapset&apos;</translation>
    </message>
    <message>
        <source>Cannot get current region</source>
        <translation>Huidige regio niet mogelijk</translation>
    </message>
    <message>
        <source>Cannot get region of map </source>
        <translation>Huidige kaartregio niet op te vragen</translation>
    </message>
    <message>
        <source>Cannot open mapcalc file</source>
        <translation>Openen van &apos;mapcalc&apos;-bestand mislukt</translation>
    </message>
    <message>
        <source>Cannot open mapcalc schema (</source>
        <translation>Openen van &apos;mapcalc&apos;-schema mislukt (</translation>
    </message>
    <message>
        <source>Cannot read mapcalc schema (</source>
        <translation>Inlezen van &apos;mapcalc&apos;-schema mislukt (</translation>
    </message>
    <message>
        <source>Check if x = NULL</source>
        <translation>Controleer op x = NULL waarde</translation>
    </message>
    <message>
        <source> column </source>
        <translation>  kolom  </translation>
    </message>
    <message>
        <source>Convert x to double-precision floating point</source>
        <translation>Converteer x naar &apos;double-precision floating point&apos;</translation>
    </message>
    <message>
        <source>Convert x to integer [ truncates ]</source>
        <translation>Converteer x naar integer [ afbreken ]</translation>
    </message>
    <message>
        <source>Convert x to single-precision floating point</source>
        <translation>Converteer x naar &apos;single-precision floating point&apos;</translation>
    </message>
    <message>
        <source>Cosine of x (x is in degrees)</source>
        <translation>Cosinus van x (x is in graden)</translation>
    </message>
    <message>
        <source>Current column of moving window (starts with 1)</source>
        <translation>Huidige kolom van &apos;moving window&apos; (met 1 beginnen)</translation>
    </message>
    <message>
        <source>Current east-west resolution</source>
        <translation>Huidige oost-west resolutie</translation>
    </message>
    <message>
        <source>Current north-south resolution</source>
        <translation>Huidige noord-zuid resolutie</translation>
    </message>
    <message>
        <source>Current row of moving window (Starts with 1)</source>
        <translation>Huidige rij van &apos;moving window&apos; (met 1 beginnen)</translation>
    </message>
    <message>
        <source>Current x-coordinate of moving window</source>
        <translation>Huidige x-coordinaat van &apos;moving window&apos;</translation>
    </message>
    <message>
        <source>Current y-coordinate of moving window</source>
        <translation>Huidige y-coordinaat van &apos;moving window&apos;</translation>
    </message>
    <message>
        <source>Decision: 1 if x not zero, 0 otherwise</source>
        <translation>Beslissing: 1 als x niet 0 is, anders 0</translation>
    </message>
    <message>
        <source>Decision: a if x &gt; 0, b if x is zero, c if x &lt; 0</source>
        <translation>Beslissing: a als x &gt; 0, b als x is 0, c als x &lt; 0</translation>
    </message>
    <message>
        <source>Decision: a if x not zero, 0 otherwise</source>
        <translation>Beslissing: a als x niet 0, anders 0</translation>
    </message>
    <message>
        <source>Decision: a if x not zero, b otherwise</source>
        <translation>Beslissing: a als x niet 0, anders b</translation>
    </message>
    <message>
        <source>Delete selected item</source>
        <translation>Verwijder geselecteerde item</translation>
    </message>
    <message>
        <source>Division</source>
        <translation>Delen</translation>
    </message>
    <message>
        <source>Enter new mapcalc name:</source>
        <translation>Voer een &apos;mapcalc&apos;-naam in:</translation>
    </message>
    <message>
        <source>Enter vector name</source>
        <translation>Geef naam voor vector</translation>
    </message>
    <message>
        <source>Equal</source>
        <translation>Gelijk</translation>
    </message>
    <message>
        <source>Exponential function of x</source>
        <translation>Exponentiele functie van x</translation>
    </message>
    <message>
        <source>Exponentiation</source>
        <translation></translation>
    </message>
    <message>
        <source>File name empty</source>
        <translation>Bestandsnaam niet gegeven</translation>
    </message>
    <message>
        <source>Greater than</source>
        <translation>Groter dan</translation>
    </message>
    <message>
        <source>Greater than or equal</source>
        <translation>Groter dan of gelijk aan</translation>
    </message>
    <message>
        <source>Inverse tangent of x (result is in degrees)</source>
        <translation>Inverse tangens van x (resultaat in graden)</translation>
    </message>
    <message>
        <source>Inverse tangent of y/x (result is in degrees)</source>
        <translation>Inverse tangens van y/x (resultaat in graden)</translation>
    </message>
    <message>
        <source>Largest value</source>
        <translation>Grootste waarde</translation>
    </message>
    <message>
        <source>Less than</source>
        <translation>Kleiner dan</translation>
    </message>
    <message>
        <source>Less than or equal</source>
        <translation>Kleiner dan of gelijk aan</translation>
    </message>
    <message>
        <source>Log of x base b</source>
        <translation>Log x basis b</translation>
    </message>
    <message>
        <source>Mapcalc tools</source>
        <translation>Mapcalc gereedschappen</translation>
    </message>
    <message>
        <source>Median value</source>
        <translation>Waarde van Mediaan</translation>
    </message>
    <message>
        <source>Mode value</source>
        <translation>Waarde van modus</translation>
    </message>
    <message>
        <source>Modulus</source>
        <translation>Modulus</translation>
    </message>
    <message>
        <source>Multiplication</source>
        <translation>Vermenigvuldiging</translation>
    </message>
    <message>
        <source>Natural log of x</source>
        <translation>Natuurlijke log van x</translation>
    </message>
    <message>
        <source>New mapcalc</source>
        <translation>Nieuwe &apos;mapcalc&apos;</translation>
    </message>
    <message>
        <source>No GRASS raster maps currently in QGIS</source>
        <translation>Geen GRASS rasterkaarten in QGIS op dit moment</translation>
    </message>
    <message>
        <source>Not equal</source>
        <translation>Niet gelijk aan</translation>
    </message>
    <message>
        <source>) not found.</source>
        <translation>) niet gevonden.</translation>
    </message>
    <message>
        <source>NULL value</source>
        <translation>NULL waarde</translation>
    </message>
    <message>
        <source>Open</source>
        <translation>Open</translation>
    </message>
    <message>
        <source>Or</source>
        <translation>Of</translation>
    </message>
    <message>
        <source>Output</source>
        <translation>Output</translation>
    </message>
    <message>
        <source>Random value between a and b</source>
        <translation>Random waarde tussen a en b</translation>
    </message>
    <message>
        <source>Round x to nearest integer</source>
        <translation>Rond x af naar dichtstbijzijnde integer</translation>
    </message>
    <message>
        <source>Save</source>
        <translation>Opslaan</translation>
    </message>
    <message>
        <source>Save as</source>
        <translation>Opslaan Als</translation>
    </message>
    <message>
        <source>Save mapcalc</source>
        <translation>Mapcalc Opslaan</translation>
    </message>
    <message>
        <source>Select item</source>
        <translation>Item selecteren</translation>
    </message>
    <message>
        <source>Sine of x (x is in degrees)</source>
        <comment>sin(x)</comment>
        <translation>Sinux van x (x in graden)</translation>
    </message>
    <message>
        <source>Smallest value</source>
        <translation>Kleinste waarde</translation>
    </message>
    <message>
        <source>Square root of x</source>
        <comment>sqrt(x)</comment>
        <translation>X-kwadraat</translation>
    </message>
    <message>
        <source>Subtraction</source>
        <translation>Aftrekken</translation>
    </message>
    <message>
        <source>Tangent of x (x is in degrees)</source>
        <comment>tan(x)</comment>
        <translation>Tangens van x (x in graden)</translation>
    </message>
    <message>
        <source>The file already exists. Overwrite? </source>
        <translation>Dit bestand bestaat al. Overschrijven?</translation>
    </message>
    <message>
        <source>The mapcalc schema (</source>
        <translation>Het &apos;mapcalc&apos;-schema (</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Waarschuwing</translation>
    </message>
    <message>
        <source>x to the power y</source>
        <translation>x tot de macht y</translation>
    </message>
</context>
<context>
    <name>QgsGrassMapcalcBase</name>
    <message>
        <source>MainWindow</source>
        <translation>HoofdVenster</translation>
    </message>
    <message>
        <source>Output</source>
        <translation>Output</translation>
    </message>
</context>
<context>
    <name>QgsGrassModule</name>
    <message>
        <source>)</source>
        <translation>)</translation>
    </message>
    <message>
        <source>):
</source>
        <translation>):
</translation>
    </message>
    <message>
        <source>
at line </source>
        <translation>
in regel</translation>
    </message>
    <message>
        <source>&lt;B&gt;Finished with error&lt;/B&gt;</source>
        <translation>&lt;B&gt;Geeindigd met en fout&lt;/B&gt;</translation>
    </message>
    <message>
        <source>&lt;B&gt;Module crashed or killed&lt;/B&gt;</source>
        <translation>&lt;B&gt;Module gestopt of er is een fout opgetreden&lt;/B&gt;</translation>
    </message>
    <message>
        <source>&lt;B&gt;Successfully finished&lt;/B&gt;</source>
        <translation>&lt;B&gt;Succesvol geeindigd&lt;/B&gt;</translation>
    </message>
    <message>
        <source>Cannot find man page </source>
        <translation>Kan man-pagina niet vinden</translation>
    </message>
    <message>
        <source>Cannot find module </source>
        <translation>Module niet gevonden</translation>
    </message>
    <message>
        <source>Cannot get input region</source>
        <translation>Huidige regio niet mogelijk</translation>
    </message>
    <message>
        <source>Cannot open module file (</source>
        <translation>Kan module-bestand niet openen (</translation>
    </message>
    <message>
        <source>Cannot read module file (</source>
        <translation>Kan module-bestand niet lezen (</translation>
    </message>
    <message>
        <source>Cannot start module: </source>
        <translation>Starten mislukt van module </translation>
    </message>
    <message>
        <source> column </source>
        <translation>  kolom  </translation>
    </message>
    <message>
        <source>Module</source>
        <translation>Module</translation>
    </message>
    <message>
        <source>Module </source>
        <translation>Module</translation>
    </message>
    <message>
        <source>Not available, cannot open description (</source>
        <translation>Niet beschikbaar, kan geen beschrijving openen (</translation>
    </message>
    <message>
        <source>Not available, description not found (</source>
        <translation>Niet beschikbaar, geen beschrijving gevonden (</translation>
    </message>
    <message>
        <source>Not available, incorrect description (</source>
        <translation>Niet beschikbaar, ongeldige beschrijving  (</translation>
    </message>
    <message>
        <source> not found</source>
        <translation> niet gevonden</translation>
    </message>
    <message>
        <source>) not found.</source>
        <translation>) niet gevonden.</translation>
    </message>
    <message>
        <source>Run</source>
        <translation>Uitvoeren</translation>
    </message>
    <message>
        <source>Stop</source>
        <translation>Stop</translation>
    </message>
    <message>
        <source>The module file (</source>
        <translation>Het module-bestand (</translation>
    </message>
    <message>
        <source>Use Input Region</source>
        <translation>Gebruik Input-Regio</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Waarschuwing</translation>
    </message>
    <message>
        <source>Please ensure you have the GRASS documentation installed.</source>
        <translation>Verzeker u ervan dat de documentatie van GRASS is geïnstalleerd.</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleBase</name>
    <message>
        <source>Close</source>
        <translation>Sluiten</translation>
    </message>
    <message>
        <source>GRASS Module</source>
        <translation>GRASS Module</translation>
    </message>
    <message>
        <source>Manual</source>
        <translation>Handmatig</translation>
    </message>
    <message>
        <source>Options</source>
        <translation>Options</translation>
    </message>
    <message>
        <source>Output</source>
        <translation>Output</translation>
    </message>
    <message>
        <source>Run</source>
        <translation>Uitvoeren</translation>
    </message>
    <message>
        <source>TextLabel</source>
        <translation>Tekstlabel</translation>
    </message>
    <message>
        <source>View output</source>
        <translation>Uitvoer Bekijken</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleField</name>
    <message>
        <source>Attribute field</source>
        <translation>Attribuutveld</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleFile</name>
    <message>
        <source>File</source>
        <translation>Bestand</translation>
    </message>
    <message>
        <source>:&amp;nbsp;directory does not exist</source>
        <translation></translation>
    </message>
    <message>
        <source>:&amp;nbsp;missing value</source>
        <translation>:&amp;nbsp;missende waarde</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleGdalInput</name>
    <message>
        <source>Cannot find layeroption </source>
        <translation>Laag-opties niet gevonden</translation>
    </message>
    <message>
        <source>Cannot find whereoption </source>
        <translation>&apos;Where&apos;-optie niet gevonden</translation>
    </message>
    <message>
        <source>:&amp;nbsp;no input</source>
        <translation>:&amp;nbsp;geen input</translation>
    </message>
    <message>
        <source>PostGIS driver in OGR does not support schemas!&lt;br&gt;Only the table name will be used.&lt;br&gt;It can result in wrong input if more tables of the same name&lt;br&gt;are present in the database.</source>
        <translation>De PostGIS driver in OGR ondersteund geen schema&apos;s!&lt;br&gt;Alleen de tabelnaam zal worden gebruikt.&lt;br&gt;Dit kan verkeerde input tot gevolg hebben als meer tabellen met&lt;br&gt;dezelfde naam in de database aanwezig zijn.</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Waarschuwing</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleInput</name>
    <message>
        <source>Cannot find layeroption </source>
        <translation>Laag-opties niet gevonden</translation>
    </message>
    <message>
        <source>Cannot find typeoption </source>
        <translation>Type-optie niet gevonden</translation>
    </message>
    <message>
        <source>Cannot find values for typeoption </source>
        <translation>Kan geen waarden voor dit type vinden</translation>
    </message>
    <message>
        <source>GRASS element </source>
        <translation></translation>
    </message>
    <message>
        <source>:&amp;nbsp;no input</source>
        <translation>:&amp;nbsp;geen input</translation>
    </message>
    <message>
        <source> not supported</source>
        <translation>niet ondersteund</translation>
    </message>
    <message>
        <source>Use region of this map</source>
        <translation>Gebruik de rego van de kaart</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Waarschuwing</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleOption</name>
    <message>
        <source>:&amp;nbsp;missing value</source>
        <translation>:&amp;nbsp;missende waarde</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleSelection</name>
    <message>
        <source>Attribute field</source>
        <translation>Attribuutveld</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleStandardOptions</name>
    <message>
        <source>):
</source>
        <translation>):
</translation>
    </message>
    <message>
        <source>
at line </source>
        <translation>
in regel  </translation>
    </message>
    <message>
        <source>Cannot check region of map </source>
        <translation>Kan regio van de kaart niet checken</translation>
    </message>
    <message>
        <source>Cannot find key </source>
        <translation>Sleutel niet gevonden</translation>
    </message>
    <message>
        <source>Cannot find module </source>
        <translation>Module niet gevonden</translation>
    </message>
    <message>
        <source>Cannot get current region</source>
        <translation>Huidige regio niet mogelijk</translation>
    </message>
    <message>
        <source>Cannot read module description (</source>
        <translation>Can moduleomschrijving niet lezen (</translation>
    </message>
    <message>
        <source>Cannot set region of map </source>
        <translation>Kan regio van de kaart niet zetten</translation>
    </message>
    <message>
        <source>Cannot start module </source>
        <translation>Starten mislukt van module </translation>
    </message>
    <message>
        <source> column </source>
        <translation>  kolom  </translation>
    </message>
    <message>
        <source>Item with id </source>
        <translation>Item met id</translation>
    </message>
    <message>
        <source> not found</source>
        <translation> niet gevonden</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Waarschuwing</translation>
    </message>
</context>
<context>
    <name>QgsGrassNewMapset</name>
    <message>
        <source>)</source>
        <translation>)</translation>
    </message>
    <message>
        <source>):
</source>
        <translation>):
</translation>
    </message>
    <message>
        <source>
at line </source>
        <translation>
in regel </translation>
    </message>
    <message>
        <source>Cannot create new location: </source>
        <translation>Kan geen nieuwe locatie aanmaken:</translation>
    </message>
    <message>
        <source>Cannot create new mapset directory</source>
        <translation>Kan de nieuwe map voor de mapset niet aanmaken</translation>
    </message>
    <message>
        <source>Cannot create projection.</source>
        <translation>Kan deze projectie niet aanmaken.</translation>
    </message>
    <message>
        <source>Cannot create QgsSpatialRefSys</source>
        <translation type="obsolete">QgsSpatialRefSys mislukt</translation>
    </message>
    <message>
        <source>Cannot open DEFAULT_WIND</source>
        <translation>Openen van DEFAULT_WIND mislukt</translation>
    </message>
    <message>
        <source>Cannot open locations file (</source>
        <translation>Openen van &apos;locatie&apos;-betand mislukt (</translation>
    </message>
    <message>
        <source>Cannot open WIND</source>
        <translation>Openen van WIND mislukt</translation>
    </message>
    <message>
        <source>Cannot read locations file (</source>
        <translation>Inlezen van &apos;locatie&apos;-betand mislukt (</translation>
    </message>
    <message>
        <source>Cannot reproject previously set region, default region set.</source>
        <translation>Het herprojecteren van de vorige regio is mislukt, standaard regio wordt gebruikt.</translation>
    </message>
    <message>
        <source>Cannot reproject region</source>
        <translation>Kan regio niet herprojecteren</translation>
    </message>
    <message>
        <source>Cannot reproject selected region.</source>
        <translation>Kan geselecteerde regio niet herprojecteren.</translation>
    </message>
    <message>
        <source> column </source>
        <translation>  kolom  </translation>
    </message>
    <message>
        <source>Comment</source>
        <translation type="obsolete">Opmerking</translation>
    </message>
    <message>
        <source>Create location</source>
        <translation>Locatie aanmaken</translation>
    </message>
    <message>
        <source>Create mapset</source>
        <translation>Mapset aanmaken</translation>
    </message>
    <message>
        <source>Create New Mapset</source>
        <translation type="obsolete">Nieuwe Mapset Aanmaken</translation>
    </message>
    <message>
        <source>Database</source>
        <translation>Database</translation>
    </message>
    <message>
        <source>Database: </source>
        <translation>Database: </translation>
    </message>
    <message>
        <source>Default GRASS Region</source>
        <translation type="obsolete">Standaard GRASS-Regio</translation>
    </message>
    <message>
        <source>East must be greater than west</source>
        <translation>Oost moet  een grotere waarde zijn dan west</translation>
    </message>
    <message>
        <source>Enter location name!</source>
        <translation>Geef naam voor locatie!</translation>
    </message>
    <message>
        <source>Enter mapset name.</source>
        <translation>Geen naam voor mapset.</translation>
    </message>
    <message>
        <source>Enter path to GRASS database</source>
        <translation>Geef pad  naar GRASS-database</translation>
    </message>
    <message>
        <source>GRASS database</source>
        <translation type="obsolete">GRASS-database</translation>
    </message>
    <message>
        <source>GRASS location</source>
        <translation type="obsolete">GRASS-locatie</translation>
    </message>
    <message>
        <source>Location: </source>
        <translation>Locatie: </translation>
    </message>
    <message>
        <source>Location 1</source>
        <translation>Locatie 1</translation>
    </message>
    <message>
        <source>Location 2</source>
        <translation>Locatie 2</translation>
    </message>
    <message>
        <source>Mapset</source>
        <translation type="obsolete">Mapset</translation>
    </message>
    <message>
        <source>Mapset: </source>
        <translation>Mapset: </translation>
    </message>
    <message>
        <source>New mapset</source>
        <translation>Nieuwe mapset</translation>
    </message>
    <message>
        <source>New mapset successfully created and set as current working mapset.</source>
        <translation>Nieuwe mapset succesvol aangemaakt en als huidige werk-mapset ingesteld</translation>
    </message>
    <message>
        <source>New mapset successfully created, but cannot be opened: </source>
        <translation>Nieuwe mapset succesvol aangemaakt, maar dan niet worden geopend:</translation>
    </message>
    <message>
        <source>North must be greater than south</source>
        <translation>Noord moet een grotere waarde zijn dan zuid</translation>
    </message>
    <message>
        <source>) not found.</source>
        <translation>) niet gevonden.</translation>
    </message>
    <message>
        <source>No writable locations, the database not writable!</source>
        <translation>Geen schrijfbare locaties, de database is niet schrijfbaar!</translation>
    </message>
    <message>
        <source>Owner</source>
        <translation type="obsolete">Eigenaar</translation>
    </message>
    <message>
        <source>Projection</source>
        <translation type="obsolete">Projectie</translation>
    </message>
    <message>
        <source>Regions file (</source>
        <translation>Regio-bestand (</translation>
    </message>
    <message>
        <source>Selected projection is not supported by GRASS!</source>
        <translation>De geselecteerde projectie wordt niet ondersteund in GRASS!</translation>
    </message>
    <message>
        <source>System mapset</source>
        <translation>Systeem mapset</translation>
    </message>
    <message>
        <source>The directory doesn&apos;t exist!</source>
        <translation>De map bestaat niet!</translation>
    </message>
    <message>
        <source>The location exists!</source>
        <translation>De locatie bestaat al!</translation>
    </message>
    <message>
        <source>The mapset already exists</source>
        <translation>De mapset bestaat al</translation>
    </message>
    <message>
        <source>Tree</source>
        <translation type="obsolete">Boom</translation>
    </message>
    <message>
        <source>User&apos;s mapset</source>
        <translation>Gebruikersmapset</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Waarschuwing</translation>
    </message>
    <message>
        <source>Cannot create QgsCoordinateReferenceSystem</source>
        <translation>Dit (Qgs)RuimtelijkReferentieSysteem kan niet worden aangemaakt</translation>
    </message>
</context>
<context>
    <name>QgsGrassNewMapsetBase</name>
    <message>
        <source>...</source>
        <translation type="obsolete">...</translation>
    </message>
    <message>
        <source>Column 1</source>
        <translation type="obsolete">Kolom 1</translation>
    </message>
    <message>
        <source>Coordinate system</source>
        <translation>Ruimtelijk Referentie Systeem</translation>
    </message>
    <message>
        <source>Create new location</source>
        <translation>Nieuwe locatie aanmaken</translation>
    </message>
    <message>
        <source>Database:</source>
        <translation>Database:</translation>
    </message>
    <message>
        <source>Database Error</source>
        <translation>Database-fout</translation>
    </message>
    <message>
        <source>E</source>
        <translation>O</translation>
    </message>
    <message>
        <source>Example directory tree:</source>
        <translation>Voorbeeld van mappenboom:</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;GRASS data are stored in tree directory structure.&lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;The GRASS database is the top-level directory in this tree structure.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;GRASS data wordt opgeslagen in een boomstructuur van mappen.&lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;De GRASS-database is de top van die boomstructuur.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;The GRASS location is a collection of maps for a particular territory or project.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;De GRASS-locatie is a collectie van kaarten voor een gegeven gebied of project.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;The GRASS mapset is a collection of maps used by one user. &lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;A user can read maps from all mapsets in the location but &lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;he can open for writing only his mapset (owned by user).&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Een GRASS-mapset is een verzameling kaarten gebruikt door 1 gebruiker. &lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Een gebruiker kan kaarten openen uit alle mapsets in de locatie maar &lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;hij heeft alleen op zijn eigen mapsets schrijfrechten.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;The GRASS region defines a workspace for raster modules. The default region is valid for one location. It is possible to set a different region in each mapset. &lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;It is possible to change the default location region later.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Een GRASS-regio definieert een werkgebied voor rastermodules. De standaard regio is geldig voor een locatie. Het is mogelijk om per mapset een verschillende regio te bepalen.&lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Het is mogelijk om de standaard locatie later aan te passen.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Location</source>
        <translation>Locatie</translation>
    </message>
    <message>
        <source>Location:</source>
        <translation>Locatie: </translation>
    </message>
    <message>
        <source>Location Error</source>
        <translation>Locatie-fout</translation>
    </message>
    <message>
        <source>Mapset:</source>
        <translation>Mapset:</translation>
    </message>
    <message>
        <source>Mapset Error</source>
        <translation>Mapset-Fout</translation>
    </message>
    <message>
        <source>N</source>
        <translation>N</translation>
    </message>
    <message>
        <source>New mapset:</source>
        <translation>Nieuwe mapset:</translation>
    </message>
    <message>
        <source>Not defined</source>
        <translation>Niet gedefinieërd</translation>
    </message>
    <message>
        <source>&lt;p align=&quot;center&quot;&gt;Existing masets&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;center&quot;&gt;Bestaande mapsets&lt;/p&gt;</translation>
    </message>
    <message>
        <source>Projection</source>
        <translation>Projectie</translation>
    </message>
    <message>
        <source>Projection Error</source>
        <translation>Projectie-Fout</translation>
    </message>
    <message>
        <source>Region Error</source>
        <translation>Regio-Fout</translation>
    </message>
    <message>
        <source>S</source>
        <translation>S</translation>
    </message>
    <message>
        <source>Select existing directory or create a new one:</source>
        <translation>Selecteer een bestaande map of maak een nieuwe aan:</translation>
    </message>
    <message>
        <source>Select location</source>
        <translation>Selecteer een locatie</translation>
    </message>
    <message>
        <source>Set</source>
        <translation>Toepassen</translation>
    </message>
    <message>
        <source>Set current QGIS extent</source>
        <translation>Set de huidige QGIS-extent</translation>
    </message>
    <message>
        <source>W</source>
        <translation>W</translation>
    </message>
    <message>
        <source>New Mapset</source>
        <translation>Nieuwe Mapset</translation>
    </message>
    <message>
        <source>GRASS Database</source>
        <translation>GRASS-database</translation>
    </message>
    <message>
        <source>Tree</source>
        <translation>Boom</translation>
    </message>
    <message>
        <source>Comment</source>
        <translation>Opmerking</translation>
    </message>
    <message>
        <source>&lt;!DOCTYPE HTML PUBLIC &quot;-//W3C//DTD HTML 4.0//EN&quot; &quot;http://www.w3.org/TR/REC-html40/strict.dtd&quot;&gt;
&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Lucida Grande&apos;; font-size:13pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;GRASS data are stored in tree directory structure. The GRASS database is the top-level directory in this tree structure.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;!DOCTYPE HTML PUBLIC &quot;-//W3C//DTD HTML 4.0//EN&quot; &quot;http://www.w3.org/TR/REC-html40/strict.dtd&quot;&gt;&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;p, li { white-space: pre-wrap; }&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Lucida Grande&apos;; font-size:13pt; font-weight:400; font-style:normal;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;De GRASS data wordt opgeslagen in een boomstructuur. De GRASS-database is de top van deze structuur.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Browse...</source>
        <translation>Bladeren...</translation>
    </message>
    <message>
        <source>GRASS Location</source>
        <translation>GRASS-locatie</translation>
    </message>
    <message>
        <source>&lt;!DOCTYPE HTML PUBLIC &quot;-//W3C//DTD HTML 4.0//EN&quot; &quot;http://www.w3.org/TR/REC-html40/strict.dtd&quot;&gt;
&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Lucida Grande&apos;; font-size:13pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;The GRASS location is a collection of maps for a particular territory or project.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;!DOCTYPE HTML PUBLIC &quot;-//W3C//DTD HTML 4.0//EN&quot; &quot;http://www.w3.org/TR/REC-html40/strict.dtd&quot;&gt;&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;p, li { white-space: pre-wrap; }&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Lucida Grande&apos;; font-size:13pt; font-weight:400; font-style:normal;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;Een GRASS-locatie is een verzameling kaarten van een bepaald gebied of project.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Default GRASS Region</source>
        <translation>Standaard GRASS-Regio</translation>
    </message>
    <message>
        <source>&lt;!DOCTYPE HTML PUBLIC &quot;-//W3C//DTD HTML 4.0//EN&quot; &quot;http://www.w3.org/TR/REC-html40/strict.dtd&quot;&gt;
&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Lucida Grande&apos;; font-size:13pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;The GRASS region defines a workspace for raster modules. The default region is valid for one location. It is possible to set a different region in each mapset. It is possible to change the default location region later.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;!DOCTYPE HTML PUBLIC &quot;-//W3C//DTD HTML 4.0//EN&quot; &quot;http://www.w3.org/TR/REC-html40/strict.dtd&quot;&gt;(new line)
&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;(new line)
p, li { white-space: pre-wrap; }(new line)
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Lucida Grande&apos;; font-size:13pt; font-weight:400; font-style:normal;&quot;&gt;(new line)
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;De GRASS region definieert een werkruimte voor raster-modules. De standaard region is geldig voor één locatie. Het is mogelijk om in elke mapset een andere region te definiëren. Het is mogelijk om de standaard locatie region later aan te passen.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Mapset</source>
        <translation>Mapset</translation>
    </message>
    <message>
        <source>Owner</source>
        <translation>Eigenaar</translation>
    </message>
    <message>
        <source>&lt;!DOCTYPE HTML PUBLIC &quot;-//W3C//DTD HTML 4.0//EN&quot; &quot;http://www.w3.org/TR/REC-html40/strict.dtd&quot;&gt;
&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Lucida Grande&apos;; font-size:13pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;The GRASS mapset is a collection of maps used by one user. A user can read maps from all mapsets in the location but he can open for writing only his mapset (owned by user).&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;!DOCTYPE HTML PUBLIC &quot;-//W3C//DTD HTML 4.0//EN&quot; &quot;http://www.w3.org/TR/REC-html40/strict.dtd&quot;&gt;&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;p, li { white-space: pre-wrap; }&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Lucida Grande&apos;; font-size:13pt; font-weight:400; font-style:normal;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;Een GRASS mapset is een door een gebruiker gebruikte kaartcollectie. Een gebruiker kan alle meapset in dezelfde &apos;location&apos; inlezen, maar kan alleen zijn eigen kaarten aanpassen.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Create New Mapset</source>
        <translation>Nieuwe Mapset Aanmaken</translation>
    </message>
</context>
<context>
    <name>QgsGrassPlugin</name>
    <message>
        <source>0.1</source>
        <translation>0.1</translation>
    </message>
    <message>
        <source>Add GRASS raster layer</source>
        <translation>GRASS-rasterlaag toevoegen</translation>
    </message>
    <message>
        <source>Add GRASS vector layer</source>
        <translation>GRASS-vectorlaag toevoegen</translation>
    </message>
    <message>
        <source>Adds a GRASS raster layer to the map canvas</source>
        <translation>Voegt een GRASS-rasterlaag toe aan de kaart</translation>
    </message>
    <message>
        <source>Adds a GRASS vector layer to the map canvas</source>
        <translation>Voegt een GRASS-vectorlaag toe aan de kaart</translation>
    </message>
    <message>
        <source>Cannot close current mapset. </source>
        <translation>Kan huidige mapset niet afsluiten.</translation>
    </message>
    <message>
        <source>Cannot close mapset. </source>
        <translation>Kan mapset niet afsluiten,</translation>
    </message>
    <message>
        <source>Cannot create new vector: </source>
        <translation>Pobleem bij aanmaken nieuwe vector:</translation>
    </message>
    <message>
        <source>Cannot open GRASS mapset. </source>
        <translation>Kan deze GRASS-mapset niet openen.</translation>
    </message>
    <message>
        <source>Cannot open the mapset. </source>
        <translation>Kan deze mapset niet openen.</translation>
    </message>
    <message>
        <source>Cannot read current region: </source>
        <translation>Kan huidige regio niet inlezen</translation>
    </message>
    <message>
        <source>Cannot start editing.</source>
        <translation>Start aanpassen mislukt</translation>
    </message>
    <message>
        <source>Close mapset</source>
        <translation>Mapset sluiten</translation>
    </message>
    <message>
        <source>Create new Grass Vector</source>
        <translation>Nieuwe GRASS-vector aanmaken</translation>
    </message>
    <message>
        <source>Display Current Grass Region</source>
        <translation>Toon Huidige GRASS-regio</translation>
    </message>
    <message>
        <source>Displays the current GRASS region as a rectangle on the map canvas</source>
        <translation>Toont huidige GRASS-regio als een rechthoek op de kaart</translation>
    </message>
    <message>
        <source>Edit Current Grass Region</source>
        <translation>Huidige GRASS-regio Aanpassen</translation>
    </message>
    <message>
        <source>Edit Grass Vector layer</source>
        <translation>GRASS-vectorlaag Aanpassen</translation>
    </message>
    <message>
        <source>Edit the current GRASS region</source>
        <translation>Huidige GRASS-regio aanpassen</translation>
    </message>
    <message>
        <source>Edit the currently selected GRASS vector layer.</source>
        <translation>Huidige geselecteerde GRASS-vectorlaag aanpassen.</translation>
    </message>
    <message>
        <source>GISDBASE, LOCATION_NAME or MAPSET is not set, cannot display current region.</source>
        <translation>GISDBASE, LOCATION_NAME of MAPSET zijn niet ingesteld. kan de huidige regio niet opslaan.</translation>
    </message>
    <message>
        <source>GRASS</source>
        <translation>GRASS</translation>
    </message>
    <message>
        <source>&amp;GRASS</source>
        <translation>&amp;GRASS</translation>
    </message>
    <message>
        <source>GRASS Edit is already running.</source>
        <translation>GRASS Aanpassen is al werkzaam.</translation>
    </message>
    <message>
        <source>GRASS layer</source>
        <translation>GRASS-laag</translation>
    </message>
    <message>
        <source>GrassVector</source>
        <translation>GRASS-Vector</translation>
    </message>
    <message>
        <source>New mapset</source>
        <translation>Nieuwe mapset</translation>
    </message>
    <message>
        <source>New vector created but cannot be opened by data provider.</source>
        <translation>Er is een nieuwe vector aangemaakt, maar deze kan die worden geopend met de data-provider.</translation>
    </message>
    <message>
        <source>New vector name</source>
        <translation>Naam voor Nieuwe Vector</translation>
    </message>
    <message>
        <source>Open GRASS tools</source>
        <translation>GRASS--Gereedschappen Openen</translation>
    </message>
    <message>
        <source>Open mapset</source>
        <translation>Mapset Openen</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Waarschuwing</translation>
    </message>
</context>
<context>
    <name>QgsGrassRegion</name>
    <message>
        <source>Cannot read current region: </source>
        <translation>Kan huidige regio niet inlezen</translation>
    </message>
    <message>
        <source>Cannot write region</source>
        <translation>Kan regio niet opslaan</translation>
    </message>
    <message>
        <source>GISDBASE, LOCATION_NAME or MAPSET is not set, cannot display current region.</source>
        <translation>GISDBASE, LOCATION_NAME of MAPSET zijn niet ingesteld. kan de huidige regio niet opslaan.</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Waarschuwing</translation>
    </message>
</context>
<context>
    <name>QgsGrassRegionBase</name>
    <message>
        <source>Cancel</source>
        <translation>Annuleren</translation>
    </message>
    <message>
        <source>Color</source>
        <translation>Kleur</translation>
    </message>
    <message>
        <source>Cols</source>
        <translation>Kol.</translation>
    </message>
    <message>
        <source>E</source>
        <translation>O</translation>
    </message>
    <message>
        <source>E-W Res</source>
        <translation>O-W Res</translation>
    </message>
    <message>
        <source>GRASS Region Settings</source>
        <translation>GRASS Regio Instellingen</translation>
    </message>
    <message>
        <source>N</source>
        <translation>N</translation>
    </message>
    <message>
        <source>N-S Res</source>
        <translation>N-Z Res</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>Rows</source>
        <translation>Rijen</translation>
    </message>
    <message>
        <source>S</source>
        <translation>S</translation>
    </message>
    <message>
        <source>W</source>
        <translation>W</translation>
    </message>
    <message>
        <source>Width</source>
        <translation>Breedte</translation>
    </message>
</context>
<context>
    <name>QgsGrassSelect</name>
    <message>
        <source>Cannot open vector on level 2 (topology not available).</source>
        <translation>Het openen van vector op niveau 2 is mislukt (topologie niet beschikbaar).</translation>
    </message>
    <message>
        <source>Choose existing GISDBASE</source>
        <translation>Selecteer een bestaande GISDBASE</translation>
    </message>
    <message>
        <source>No layer</source>
        <translation>Geen laag</translation>
    </message>
    <message>
        <source>No layers available in this map</source>
        <translation>Geen lagen beschikbaar in deze kaart</translation>
    </message>
    <message>
        <source>No map</source>
        <translation>Geen kaart</translation>
    </message>
    <message>
        <source>Select a map.</source>
        <translation>Selecteer een kaart.</translation>
    </message>
    <message>
        <source>Select GRASS mapcalc schema</source>
        <translation>Selecteer een GRASS-mapcalc-schema</translation>
    </message>
    <message>
        <source>Select GRASS Mapset</source>
        <translation>Selecteer een GRASS-Mapset</translation>
    </message>
    <message>
        <source>Select GRASS Raster Layer</source>
        <translation>Selecteer een GRASS-rasterlaag</translation>
    </message>
    <message>
        <source>Select GRASS Vector Layer</source>
        <translation>Selecteer een GRASS-vectorlaag</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Waarschuwing</translation>
    </message>
    <message>
        <source>Wrong GISDBASE</source>
        <translation>Foute GISDBASE</translation>
    </message>
    <message>
        <source>Wrong GISDBASE, no locations available.</source>
        <translation>Foute GISDBASE, geen locaties beschikbaar.</translation>
    </message>
</context>
<context>
    <name>QgsGrassSelectBase</name>
    <message>
        <source>Add GRASS Layer</source>
        <translation>GRASS-laag Toevoegen</translation>
    </message>
    <message>
        <source>Browse</source>
        <translation>Bladeren</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Annuleren</translation>
    </message>
    <message>
        <source>Gisdbase</source>
        <translation>Gisdbase</translation>
    </message>
    <message>
        <source>Layer</source>
        <translation>Laag</translation>
    </message>
    <message>
        <source>Location</source>
        <translation>Locatie</translation>
    </message>
    <message>
        <source>Map name</source>
        <translation>&apos;Map&apos;-naam</translation>
    </message>
    <message>
        <source>Mapset</source>
        <translation>Mapset</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>Select or type map name (wildcards &apos;*&apos; and &apos;?&apos; accepted for rasters)</source>
        <translation>Selecteer of voer de &apos;map&apos;-naam in (jokers &apos;*&apos; en &apos;?&apos; voor raster toegestaan)</translation>
    </message>
</context>
<context>
    <name>QgsGrassShellBase</name>
    <message>
        <source>Close</source>
        <translation>Sluiten</translation>
    </message>
    <message>
        <source>GRASS Shell</source>
        <translation>GRASS Shell</translation>
    </message>
</context>
<context>
    <name>QgsGrassTools</name>
    <message>
        <source>)</source>
        <translation>)</translation>
    </message>
    <message>
        <source>
at line </source>
        <translation>
in regel</translation>
    </message>
    <message>
        <source>Browser</source>
        <translation>Browser</translation>
    </message>
    <message>
        <source>Cannot find MSYS (</source>
        <translation>MSYS niet gevonden (</translation>
    </message>
    <message>
        <source>Cannot open config file (</source>
        <translation>Openen van config-bestand mislukt (</translation>
    </message>
    <message>
        <source>Cannot read config file (</source>
        <translation>Inlezen van config-bestand mislukt (</translation>
    </message>
    <message>
        <source> column </source>
        <translation>  kolom  </translation>
    </message>
    <message>
        <source>GRASS Shell is not compiled.</source>
        <translation>De &apos;GRASS-shell&apos; is niet gecompileerd.</translation>
    </message>
    <message>
        <source>GRASS Tools</source>
        <translation>GRASS Gereedschappen</translation>
    </message>
    <message>
        <source>GRASS Tools: </source>
        <translation>GRASS Gereedschappen:</translation>
    </message>
    <message>
        <source>Modules</source>
        <translation type="obsolete">Modules</translation>
    </message>
    <message>
        <source>) not found.</source>
        <translation>) niet gevonden.</translation>
    </message>
    <message>
        <source>The config file (</source>
        <translation>Het config-bestand (</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Waarschuwing</translation>
    </message>
    <message>
        <source>Modules Tree</source>
        <translation type="obsolete">Modules-boom</translation>
    </message>
    <message>
        <source>Modules List</source>
        <translation type="obsolete">Modules-lijst</translation>
    </message>
</context>
<context>
    <name>QgsGrassToolsBase</name>
    <message>
        <source>Grass Tools</source>
        <translation>Grass Gereedschappen</translation>
    </message>
    <message>
        <source>Modules Tree</source>
        <translation>Module-boom</translation>
    </message>
    <message>
        <source>1</source>
        <translation>1</translation>
    </message>
    <message>
        <source>Modules List</source>
        <translation>Module-lijst</translation>
    </message>
</context>
<context>
    <name>QgsGridMakerPlugin</name>
    <message>
        <source>Creates a graticule (grid) and stores the result as a shapefile</source>
        <translation>Kaartgrid aanmaken en het resultaat opslaan als een shape-bestand</translation>
    </message>
    <message>
        <source>&amp;Graticule Creator</source>
        <translation>Kaart&amp;grid Bouwer</translation>
    </message>
    <message>
        <source>&amp;Graticules</source>
        <translation>&amp;Kaartgrids</translation>
    </message>
</context>
<context>
    <name>QgsGridMakerPluginGui</name>
    <message>
        <source>Choose a filename to save under</source>
        <translation type="obsolete">Kies bestandsnaam om te bewaren als</translation>
    </message>
    <message>
        <source>ESRI Shapefile (*.shp)</source>
        <translation>ESRI Shapebestand (*.shp)</translation>
    </message>
    <message>
        <source>Please enter the file name before pressing OK!</source>
        <translation>Voer een naam in voor het bestand alvorens OK te klikken!</translation>
    </message>
    <message>
        <source>QGIS - Grid Maker</source>
        <translation>QGIS - Kaartgrid Bouwer</translation>
    </message>
    <message>
        <source>Please enter intervals before pressing OK!</source>
        <translation>Vul intervallen in voordat u op OK drukt!</translation>
    </message>
    <message>
        <source>Choose a file name to save under</source>
        <translation>Kies een bestandsnaam om te bewaren als</translation>
    </message>
</context>
<context>
    <name>QgsGridMakerPluginGuiBase</name>
    <message>
        <source>End point (upper right)</source>
        <translation>Eindpunt (rechtsboven)</translation>
    </message>
    <message>
        <source>Graticule Builder</source>
        <translation>Kaartgridbouwer</translation>
    </message>
    <message>
        <source>Origin (lower left)</source>
        <translation>Oorsprong (linksonder)</translation>
    </message>
    <message>
        <source>Output (shape) file</source>
        <translation>Uitvoerbestand (shape)</translation>
    </message>
    <message>
        <source>Point</source>
        <translation>Punt</translation>
    </message>
    <message>
        <source>Polygon</source>
        <translation>Polygoon</translation>
    </message>
    <message>
        <source>Save As...</source>
        <translation>Opslaan Als...</translation>
    </message>
    <message>
        <source>Type</source>
        <translation>Type</translation>
    </message>
    <message>
        <source>QGIS Graticule Creator</source>
        <translation>QGIS Kaartgrid Bouwer</translation>
    </message>
    <message>
        <source>Graticle size</source>
        <translation>Grootte van het Grid</translation>
    </message>
    <message>
        <source>Y Interval:</source>
        <translation>Y Interval:</translation>
    </message>
    <message>
        <source>X Interval:</source>
        <translation>X Interval:</translation>
    </message>
    <message>
        <source>Y</source>
        <translation>Y</translation>
    </message>
    <message>
        <source>X</source>
        <translation>X</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:11pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;This plugin will help you to build a graticule shapefile that you can use as an overlay within your qgis map viewer.&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:10pt;&quot;&gt;Please enter all units in decimal degrees&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;p, li { white-space: pre-wrap; }&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:11pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;Deze plugin helpt u een kaartgrid shape-bestand aan te maken die te gebruiken is als &apos;overlay&apos; van uw QGIS-kaart.&lt;/span&gt;&lt;/p&gt;&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:10pt;&quot;&gt;Alle eenheden invoeren in decimale graden&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:11pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;This plugin will help you to build a graticule shapefile that you can use as an overlay within your qgis map viewer.&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;;&quot;&gt;Please enter all units in decimal degrees&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;(new line)
p, li { white-space: pre-wrap; }(new line)
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;(new line)
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:11pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;Met deze plugin kunt u een shapefile maken van een kaartgrid die u als laag kunt tonen in qgis.&lt;/span&gt;&lt;/p&gt;(new line)
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;;&quot;&gt;Voer alle eenheden in als decimale graden&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
</context>
<context>
    <name>QgsHelpViewer</name>
    <message>
        <source>Error</source>
        <translation></translation>
    </message>
    <message>
        <source>Failed to get the help text from the database</source>
        <translation>Problemen bij het ophalen van de helpteksten uit de database</translation>
    </message>
    <message>
        <source>If you would like to create it, contact the QGIS development team</source>
        <translation>Indien u het wilt maken, neem contact op het met QGIS-ontwikkelteam</translation>
    </message>
    <message>
        <source>Quantum GIS Help</source>
        <translation>Quantum GIS Help</translation>
    </message>
    <message>
        <source>Quantum GIS Help - </source>
        <translation>Quantum GIS Help - </translation>
    </message>
    <message>
        <source>The QGIS help database is not installed</source>
        <translation>De QGIS-helpdatabase is niet geinstalleerd</translation>
    </message>
    <message>
        <source>This help file does not exist for your language</source>
        <translation>Dit help-bestand bestaat niet voor uw taalkeuze</translation>
    </message>
</context>
<context>
    <name>QgsHelpViewerBase</name>
    <message>
        <source>Alt+B</source>
        <translation>Alt+B</translation>
    </message>
    <message>
        <source>Alt+C</source>
        <translation>Alt+C</translation>
    </message>
    <message>
        <source>Alt+F</source>
        <translation>Alt+F</translation>
    </message>
    <message>
        <source>Alt+H</source>
        <translation>Alt+H</translation>
    </message>
    <message>
        <source>&amp;Back</source>
        <translation>&amp;Vorige</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>Sl&amp;uiten</translation>
    </message>
    <message>
        <source>&amp;Forward</source>
        <translation>V&amp;olgende</translation>
    </message>
    <message>
        <source>&amp;Home</source>
        <translation>&amp;Home</translation>
    </message>
    <message>
        <source>QGIS Help</source>
        <translation>QGIS Help</translation>
    </message>
</context>
<context>
    <name>QgsHttpTransaction</name>
    <message>
        <source>HTTP response completed, however there was an error: %1</source>
        <translation>HTTP response ontvangen, maar met een fout: %1</translation>
    </message>
    <message>
        <source>HTTP transaction completed, however there was an error: %1</source>
        <translation>HTTP transactie afgerond, maar met een fout: %1</translation>
    </message>
    <message>
        <source>Network timed out after %1 seconds of inactivity.
This may be a problem in your network connection or at the WMS server.</source>
        <translation type="obsolete">Er trad een &apos;network timeout&apos; op na %1 seconden van netwerkinactiviteit.
Dit zou kunnen zijn veroorzaakt door een probleem in uw lokale netwerk of het netwerk van de WMS server.
        
        
        
        </translation>
    </message>
    <message>
        <source>WMS Server responded unexpectedly with HTTP Status Code %1 (%2)</source>
        <translation>Onverwacht antwoord van WMS-server met HTTP Status Code %1 (%2)</translation>
    </message>
</context>
<context>
    <name>QgsIDWInterpolatorDialogBase</name>
    <message>
        <source>Dialog</source>
        <translation>Dialoog</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:12pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-weight:600;&quot;&gt;Inverse Distance Weighting&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-weight:600;&quot;&gt;&lt;span style=&quot; font-weight:400;&quot;&gt;The only parameter for the IDW interpolation method is the coefficient that describes the decrease of weights with distance.&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:12pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-weight:600;&quot;&gt;Inverse Distance Weighting&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-weight:600;&quot;&gt;&lt;span style=&quot; font-weight:400;&quot;&gt;De enige parameter voor de IDW-interpolatiemethode is de coëfficiënt die de afname van het gewicht met de afstand beschrijft.&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Distance coefficient P:</source>
        <translation>Afstandscoëfficiënt P:</translation>
    </message>
</context>
<context>
    <name>QgsIdentifyResults</name>
    <message>
        <source>(Derived)</source>
        <translation>(Afgeleid)</translation>
    </message>
    <message>
        <source>Feature</source>
        <translation>Object</translation>
    </message>
    <message>
        <source>Identify Results - </source>
        <translation>Identificeerresultaat</translation>
    </message>
    <message>
        <source>Run action</source>
        <translation>Voer aktie uit</translation>
    </message>
    <message>
        <source>Value</source>
        <translation>Waarde</translation>
    </message>
</context>
<context>
    <name>QgsIdentifyResultsBase</name>
    <message>
        <source>Close</source>
        <translation>Sluiten</translation>
    </message>
    <message>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <source>Help</source>
        <translation>Help</translation>
    </message>
    <message>
        <source>Identify Results</source>
        <translation>Identificeerresultaten</translation>
    </message>
</context>
<context>
    <name>QgsInterpolationDialog</name>
    <message>
        <source>Triangular interpolation (TIN)</source>
        <translation>Driehoeksinterpolatie (TIN)</translation>
    </message>
    <message>
        <source>Inverse Distance Weighting (IDW)</source>
        <translation>Inverse Distance Weighting (IDW)</translation>
    </message>
</context>
<context>
    <name>QgsInterpolationDialogBase</name>
    <message>
        <source>Output</source>
        <translation>Uitvoer</translation>
    </message>
    <message>
        <source>Dialog</source>
        <translation type="obsolete">Dialoog</translation>
    </message>
    <message>
        <source>Input</source>
        <translation>Invoer</translation>
    </message>
    <message>
        <source>Input vector layer:</source>
        <translation type="obsolete">Invoer vectorlaag:</translation>
    </message>
    <message>
        <source>Use z-Coordinate for interpolation</source>
        <translation>Gebruik z-coördinaten voor interpolatie</translation>
    </message>
    <message>
        <source>Interpolation attribute: </source>
        <translation type="obsolete">Interpolatie attribuut:</translation>
    </message>
    <message>
        <source>Interpolation method:</source>
        <translation type="obsolete">Interpolatie methode:</translation>
    </message>
    <message>
        <source>Configure interpolation method...</source>
        <translation type="obsolete">Interpolatie-methode configureren...</translation>
    </message>
    <message>
        <source>Number of columns:</source>
        <translation type="obsolete">Aantal kolommen:</translation>
    </message>
    <message>
        <source>Number of rows:</source>
        <translation type="obsolete">Aantal rijen:</translation>
    </message>
    <message>
        <source>Output File: </source>
        <translation type="obsolete">Uitvoerbestand:</translation>
    </message>
    <message>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <source>Interpolation plugin</source>
        <translation>Interpolatie-plugin</translation>
    </message>
    <message>
        <source>Input vector layer</source>
        <translation>Invoer vectorlaag</translation>
    </message>
    <message>
        <source>Interpolation attribute </source>
        <translation>Interpolatie attribuut </translation>
    </message>
    <message>
        <source>Interpolation method</source>
        <translation>Interpolatie methode</translation>
    </message>
    <message>
        <source>Number of columns</source>
        <translation>Aantal kolommen</translation>
    </message>
    <message>
        <source>Number of rows</source>
        <translation>Aantal rijen</translation>
    </message>
    <message>
        <source>Output file </source>
        <translation>Uitvoerbestand </translation>
    </message>
</context>
<context>
    <name>QgsInterpolationPlugin</name>
    <message>
        <source>&amp;Interpolation</source>
        <translation>&amp;Interpolatie</translation>
    </message>
</context>
<context>
    <name>QgsLUDialogBase</name>
    <message>
        <source>-</source>
        <translation>-</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation type="obsolete">Annuleren</translation>
    </message>
    <message>
        <source>Enter class bounds</source>
        <translation>Geef klassegrenzen aan</translation>
    </message>
    <message>
        <source>Lower value</source>
        <translation>Ondergrens</translation>
    </message>
    <message>
        <source>OK</source>
        <translation type="obsolete">OK</translation>
    </message>
    <message>
        <source>Upper value</source>
        <translation>Bovengrens</translation>
    </message>
</context>
<context>
    <name>QgsLabelDialog</name>
    <message>
        <source>Auto</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsLabelDialogBase</name>
    <message encoding="UTF-8">
        <source>°</source>
        <translation>°</translation>
    </message>
    <message>
        <source>%</source>
        <translation>%</translation>
    </message>
    <message>
        <source>Above</source>
        <translation>Boven</translation>
    </message>
    <message>
        <source>Above Left</source>
        <translation>Linksboven</translation>
    </message>
    <message>
        <source>Above Right</source>
        <translation>Rechtsboven</translation>
    </message>
    <message>
        <source>Below</source>
        <translation>Onder</translation>
    </message>
    <message>
        <source>Below Left</source>
        <translation>Linksonder</translation>
    </message>
    <message>
        <source>Below Right</source>
        <translation>Rechtsboven</translation>
    </message>
    <message>
        <source>Buffer</source>
        <translation>Buffer</translation>
    </message>
    <message>
        <source>Buffer size units</source>
        <translation>Buffergrootte eenheden</translation>
    </message>
    <message>
        <source>Font</source>
        <translation>Lettertype</translation>
    </message>
    <message>
        <source>Font size units</source>
        <translation>Fontgrootte eenheden</translation>
    </message>
    <message>
        <source>Form1</source>
        <translation>Form1</translation>
    </message>
    <message>
        <source>Left</source>
        <translation>Links</translation>
    </message>
    <message>
        <source>Map units</source>
        <translation>Kaart eenheden</translation>
    </message>
    <message>
        <source>Offset units</source>
        <translation>Eenheden voor verspringing</translation>
    </message>
    <message>
        <source>Over</source>
        <translation>Over</translation>
    </message>
    <message>
        <source>Placement</source>
        <translation>Plaats</translation>
    </message>
    <message>
        <source>Points</source>
        <translation>Punten</translation>
    </message>
    <message>
        <source>Position</source>
        <translation>Positie</translation>
    </message>
    <message>
        <source>Preview:</source>
        <translation>Voorvertoning:</translation>
    </message>
    <message>
        <source>QGIS Rocks!</source>
        <translation>QGIS Rocks!</translation>
    </message>
    <message>
        <source>Right</source>
        <translation>Rechts</translation>
    </message>
    <message>
        <source>Size:</source>
        <translation>Grootte:</translation>
    </message>
    <message>
        <source>Size is in map units</source>
        <translation>Grootte in kaarteenheden</translation>
    </message>
    <message>
        <source>Size is in points</source>
        <translation>Grootte in punten</translation>
    </message>
    <message>
        <source>Transparency:</source>
        <translation>Transparantie: </translation>
    </message>
    <message>
        <source>Field containing label</source>
        <translation>Veld te gebruiken voor labels</translation>
    </message>
    <message>
        <source>Default label</source>
        <translation>Standaard label</translation>
    </message>
    <message>
        <source>Data defined style</source>
        <translation>Data-bepaalde stijl</translation>
    </message>
    <message>
        <source>Data defined alignment</source>
        <translation>Data-bepaalde uitlijning</translation>
    </message>
    <message>
        <source>Data defined buffer</source>
        <translation>Data-bepaalde buffer</translation>
    </message>
    <message>
        <source>Data defined position</source>
        <translation>Data-bepaalde positie</translation>
    </message>
    <message>
        <source>Font transparency</source>
        <translation>Fonttransparantie</translation>
    </message>
    <message>
        <source>Color</source>
        <translation>Kleur</translation>
    </message>
    <message>
        <source>Angle (deg)</source>
        <translation>Hoek (graden)</translation>
    </message>
    <message>
        <source>Buffer labels?</source>
        <translation>Labels met buffer?</translation>
    </message>
    <message>
        <source>Buffer size</source>
        <translation>Grootte van buffer</translation>
    </message>
    <message>
        <source>Transparency</source>
        <translation>Transparantie</translation>
    </message>
    <message>
        <source>X Offset (pts)</source>
        <translation>Verspringing X (punten)</translation>
    </message>
    <message>
        <source>Y Offset (pts)</source>
        <translation>Verspringing Y (punten)</translation>
    </message>
    <message>
        <source>&amp;Font family</source>
        <translation>&amp;Font familie</translation>
    </message>
    <message>
        <source>&amp;Bold</source>
        <translation>&amp;Vet</translation>
    </message>
    <message>
        <source>&amp;Italic</source>
        <translation>Schu&amp;in</translation>
    </message>
    <message>
        <source>&amp;Underline</source>
        <translation>&amp;Onderstrepen</translation>
    </message>
    <message>
        <source>&amp;Size</source>
        <translation>&amp;Grootte</translation>
    </message>
    <message>
        <source>Size units</source>
        <translation>Eenheden voor grootte</translation>
    </message>
    <message>
        <source>X Coordinate</source>
        <translation>X-coördinaat</translation>
    </message>
    <message>
        <source>Y Coordinate</source>
        <translation>Y-coördinaat</translation>
    </message>
    <message>
        <source>Multiline labels?</source>
        <translation>Labels met meerdere regels?</translation>
    </message>
    <message>
        <source>General</source>
        <translation type="unfinished">Algemeen</translation>
    </message>
    <message>
        <source>Use scale dependent rendering</source>
        <translation type="unfinished">Gebruik schaalafhankelijke tonen</translation>
    </message>
    <message>
        <source>Maximum</source>
        <translation type="unfinished">Maximum</translation>
    </message>
    <message>
        <source>Minimum</source>
        <translation type="unfinished">Minimum</translation>
    </message>
    <message>
        <source>Minimum scale at which this layer will be displayed. </source>
        <translation type="unfinished">Minimale schaal bij welke deze laag nog getoond zal worden. </translation>
    </message>
    <message>
        <source>Maximum scale at which this layer will be displayed. </source>
        <translation type="unfinished">Maximale schaal bij welke deze laag nog getoond zal worden. </translation>
    </message>
</context>
<context>
    <name>QgsLayerProjectionSelector</name>
    <message>
        <source>By default, this layer will now have its projection set to that of the project, but you may override this by selecting a different projection below.</source>
        <translation type="obsolete">Standaard krijgt de laag de projectie zoals die is ingesteld voor het project, maar u kunt dit aanpassen door een andere projectie hieronder te kiezen.</translation>
    </message>
    <message>
        <source>Define this layer&apos;s projection:</source>
        <translation type="obsolete">Definieer projectie voor deze laag:</translation>
    </message>
    <message>
        <source>This layer appears to have no projection specification.</source>
        <translation type="obsolete">De laag lijkt geen projectie instelling te bevatten.</translation>
    </message>
</context>
<context>
    <name>QgsLayerProjectionSelectorBase</name>
    <message>
        <source>Layer Projection Selector</source>
        <translation type="obsolete">Laagprojectie Selector</translation>
    </message>
    <message>
        <source>OK</source>
        <translation type="obsolete">OK</translation>
    </message>
</context>
<context>
    <name>QgsLegend</name>
    <message>
        <source>&amp;Add group</source>
        <translation>Groep &amp;toevoegen</translation>
    </message>
    <message>
        <source>&amp;Collapse all</source>
        <translation>Alles &amp;inklappen</translation>
    </message>
    <message>
        <source>&amp;Expand all</source>
        <translation>Alles &amp;uitklappen</translation>
    </message>
    <message>
        <source>group</source>
        <translation>groep</translation>
    </message>
    <message>
        <source>&amp;Make to toplevel item</source>
        <translation>&amp;Maak hier een &apos;top level&apos; item van</translation>
    </message>
    <message>
        <source>No Layer Selected</source>
        <translation>Geen Kaartlaag Geselecteerd</translation>
    </message>
    <message>
        <source>&amp;Remove</source>
        <translation>&amp;Verwijder</translation>
    </message>
    <message>
        <source>Re&amp;name</source>
        <translation>&amp;Hernoem</translation>
    </message>
    <message>
        <source>Show file groups</source>
        <translation>Toon bestandsgroepen</translation>
    </message>
    <message>
        <source>To open an attribute table, you must select a vector layer in the legend</source>
        <translation>Om een attributentabel te openen, moet u een vectorlaag selecteren in de legenda</translation>
    </message>
</context>
<context>
    <name>QgsLegendLayer</name>
    <message>
        <source>More layers</source>
        <translation type="obsolete">Meer lagen</translation>
    </message>
    <message>
        <source>&amp;Open attribute table</source>
        <translation>&amp;Open attributentabel</translation>
    </message>
    <message>
        <source>&amp;Properties</source>
        <translation>&amp;Eigenschappen</translation>
    </message>
    <message>
        <source>&amp;Remove</source>
        <translation>&amp;Verwijder</translation>
    </message>
    <message>
        <source>Save as shapefile...</source>
        <translation>Opslaan als shapefile...</translation>
    </message>
    <message>
        <source>Save selection as shapefile...</source>
        <translation>Selectie opslaan als shapefile</translation>
    </message>
    <message>
        <source>&amp;Show in overview</source>
        <translation>&amp;Toon in overzichtskaart</translation>
    </message>
    <message>
        <source>This item contains more layer files. Displaying more layers in table is not supported.</source>
        <translation type="obsolete">Dit item bevat meerdere lagen. Vertonen van meerdere lagen in tabel wordt niet ondersteunnd.</translation>
    </message>
    <message>
        <source>&amp;Zoom to best scale (100%)</source>
        <translation>$Zoomen naar beste schaal (100%)</translation>
    </message>
    <message>
        <source>&amp;Zoom to layer extent</source>
        <translation>&amp;Zoom naar laagextent</translation>
    </message>
    <message>
        <source>Multiple layers</source>
        <translation>Meerdere lagen</translation>
    </message>
    <message>
        <source>This item contains multiple layers. Displaying multiple layers in the table is not supported.</source>
        <translation>Dit onderdeel bevat meerdere lagen. Het tonen van meerdere lagen in één tabel wordt niet ondersteund.</translation>
    </message>
</context>
<context>
    <name>QgsLegendLayerFile</name>
    <message>
        <source>Attribute table - </source>
        <translation type="obsolete">Attributentabel - </translation>
    </message>
    <message>
        <source>bad_alloc exception</source>
        <translation type="obsolete">bad_alloc fout</translation>
    </message>
    <message>
        <source>Could not commit changes</source>
        <translation type="obsolete">&apos;Commit&apos; van de aanpassingen niet mogelijk</translation>
    </message>
    <message>
        <source>Do you want to save the changes?</source>
        <translation type="obsolete">Wilt u de huidige aanpassingen opslaan?</translation>
    </message>
    <message>
        <source>Driver not found</source>
        <translation>&apos;Driver&apos; niet gevonden</translation>
    </message>
    <message>
        <source>Error</source>
        <translation>Fout</translation>
    </message>
    <message>
        <source>Error creating shapefile</source>
        <translation>Fout bij het maken van de shapefile</translation>
    </message>
    <message>
        <source>ESRI Shapefile driver is not available</source>
        <translation>ESRI shapefile software is niet beschikbaar</translation>
    </message>
    <message>
        <source>Export to Shapefile has been completed</source>
        <translation>Export naar shapefile is voltooid</translation>
    </message>
    <message>
        <source>Filling the attribute table has been stopped because there was no more virtual memory left</source>
        <translation type="obsolete">Het vullen van de attributentabel is gestopt omdat er onvoldoende virtueel geheugen beschikbaar is</translation>
    </message>
    <message>
        <source>Layer attribute table contains unsupported datatype(s)</source>
        <translation>Attributentabel van de laag bevat niet-ondersteund(e) datatype(n)</translation>
    </message>
    <message>
        <source>Layer creation failed</source>
        <translation>Laag aanmaken mislukt</translation>
    </message>
    <message>
        <source>Not a vector layer</source>
        <translation type="obsolete">Geen vectorlaag</translation>
    </message>
    <message>
        <source>&amp;Open attribute table</source>
        <translation>&amp;Open attributentabel</translation>
    </message>
    <message>
        <source>Problems during roll back</source>
        <translation type="obsolete">Problemen bij de &apos;roll-back&apos;</translation>
    </message>
    <message>
        <source>&amp;Properties</source>
        <translation>&amp;Eigenschappen</translation>
    </message>
    <message>
        <source>Provider cannot be opened for editing</source>
        <translation type="obsolete">(Data)provider kan niet voor aanpassingen worden geopend</translation>
    </message>
    <message>
        <source>&amp;Remove</source>
        <translation>&amp;Verwijderen</translation>
    </message>
    <message>
        <source>Save as shapefile...</source>
        <translation>Opslaan als shape-bestand...</translation>
    </message>
    <message>
        <source>Save layer as...</source>
        <translation>Layer opslaan als...</translation>
    </message>
    <message>
        <source>Save selection as shapefile...</source>
        <translation>Selectie opslaan als shapefile...</translation>
    </message>
    <message>
        <source>Saving done</source>
        <translation>Opslaan voltooid</translation>
    </message>
    <message>
        <source>&amp;Show in overview</source>
        <translation>&amp;Toon in overzichtskaart</translation>
    </message>
    <message>
        <source>Start editing failed</source>
        <translation type="obsolete">Start aanpassen mislukt</translation>
    </message>
    <message>
        <source>Stop editing</source>
        <translation type="obsolete">Stop aanpassen</translation>
    </message>
    <message>
        <source>The shapefile could not be created (</source>
        <translation>De shapefile kon niet worden aangemaakt</translation>
    </message>
    <message>
        <source>To open an attribute table, you must select a vector layer in the legend</source>
        <translation type="obsolete">Om een attributentabel te openen dient u eerst een vectorlaag te selecteren in de legenda</translation>
    </message>
    <message>
        <source>&amp;Zoom to layer extent</source>
        <translation>&amp;Zoom naar laagextent</translation>
    </message>
    <message>
        <source>Select the coordinate reference system for the saved shapefile.</source>
        <translation>Selecteer het  ruimtelijk referentie systeem (CRS) voor het opgeslagen shape-bestand.</translation>
    </message>
    <message>
        <source>The data points will be transformed from the layer coordinate reference system.</source>
        <translation>De puntdata zal worden getransformeerd vanuit het crs van de laag.</translation>
    </message>
</context>
<context>
    <name>QgsLineStyleDialogBase</name>
    <message>
        <source>Cancel</source>
        <translation type="obsolete">Annuleren</translation>
    </message>
    <message>
        <source>Ok</source>
        <translation type="obsolete">Ok</translation>
    </message>
    <message>
        <source>Select a line style</source>
        <translation type="obsolete">Selecteer een lijnstijl</translation>
    </message>
    <message>
        <source>Styles</source>
        <translation type="obsolete">Stijlen</translation>
    </message>
</context>
<context>
    <name>QgsLineStyleWidgetBase</name>
    <message>
        <source>col</source>
        <translation type="obsolete">kleur</translation>
    </message>
    <message>
        <source>Colour:</source>
        <translation type="obsolete">Kleur:</translation>
    </message>
    <message>
        <source>Form2</source>
        <translation type="obsolete">Form2</translation>
    </message>
    <message>
        <source>Outline Style</source>
        <translation type="obsolete">Lijnstijl</translation>
    </message>
    <message>
        <source>Width:</source>
        <translation type="obsolete">Dikte</translation>
    </message>
    <message>
        <source>LineStyleWidget</source>
        <translation type="obsolete">LijnStijlWidget</translation>
    </message>
</context>
<context>
    <name>QgsMapCanvas</name>
    <message>
        <source>because</source>
        <translation>omdat</translation>
    </message>
    <message>
        <source>Could not draw</source>
        <translation>Tekenen niet mogelijk</translation>
    </message>
</context>
<context>
    <name>QgsMapLayer</name>
    <message>
        <source>%1 at line %2 column %3</source>
        <translation>%1 in regel %2 kolom %3</translation>
    </message>
    <message>
        <source>could not open user database</source>
        <translation type="obsolete">gebruikersdatabase openen mislukt</translation>
    </message>
    <message>
        <source>style %1 not found in database</source>
        <translation type="obsolete">stijl %1 niet in database gevonden</translation>
    </message>
    <message>
        <source>User database could not be opened.</source>
        <translation>Gebruikersdatabase kan niet worden geopend.</translation>
    </message>
    <message>
        <source>The style table could not be created.</source>
        <translation>Problemen bij het aanmaken van de stijltabel.</translation>
    </message>
    <message>
        <source>The style %1 was saved to database</source>
        <translation>Stijl %1 opgeslagen in database</translation>
    </message>
    <message>
        <source>The style %1 was updated in the database.</source>
        <translation>Stijl %1 aangepast in de database.</translation>
    </message>
    <message>
        <source>The style %1 could not be updated in the database.</source>
        <translation>Stijl %1 kon niet worden aangepast in de database.</translation>
    </message>
    <message>
        <source>The style %1 could not be inserted into database.</source>
        <translation>Stijl %1 kon niet in de database worden ingevoerd.</translation>
    </message>
    <message>
        <source>style not found in database</source>
        <translation>stijl niet in database gevonden</translation>
    </message>
</context>
<context>
    <name>QgsMapToolIdentify</name>
    <message>
        <source>- %1 features found</source>
        <comment>Identify results window title</comment>
        <translation type="obsolete">- %1 object(en) gevonden
        
        
        
        </translation>
    </message>
    <message>
        <source>(clicked coordinate)</source>
        <translation>(aangeklikte coördinaat)</translation>
    </message>
    <message>
        <source>No features found</source>
        <translation type="obsolete">Geen kaartobjecten gevonden</translation>
    </message>
    <message>
        <source>&lt;p&gt;No features were found within the search radius. Note that it is currently not possible to use the identify tool on unsaved features.&lt;/p&gt;</source>
        <translation type="obsolete">&lt;p&gt;Er zijn geen objecten gevonden binnen de zoekstraal. (Het is momenteel niet mogelijk de identificeerfunctie te gebruiken op niet-opgeslagen objecten.)&lt;/p&gt;</translation>
    </message>
    <message>
        <source>WMS identify result for %1
%2</source>
        <translation>WMS identificeer-resultaat voor %1
%2</translation>
    </message>
</context>
<context>
    <name>QgsMapToolSplitFeatures</name>
    <message>
        <source>An error occured during feature splitting</source>
        <translation>Er is een fout opgetreden tijdens het splitsen van een object</translation>
    </message>
    <message>
        <source>Split error</source>
        <translation>Splitsingsfout</translation>
    </message>
    <message>
        <source>No feature split done</source>
        <translation>Kaartobjecten splitsen niet uitgevoerd</translation>
    </message>
    <message>
        <source>If there are selected features, the split tool only applies to the selected ones. If you like to split all features under the split line, clear the selection</source>
        <translation>Als er objecten geselecteerd zijn, zullen alleen deze geselecteerde objecten worden gesplitst. Als u alle objecten onder de &apos;split-lijn&apos; wilt splitsen, maak dan de selectie leeg</translation>
    </message>
</context>
<context>
    <name>QgsMapToolVertexEdit</name>
    <message>
        <source>Could not snap segment.</source>
        <translation>Segment kan geen &apos;snap&apos; maken</translation>
    </message>
    <message>
        <source>Don&apos;t show this message again</source>
        <translation>Deze melding niet meer tonen</translation>
    </message>
    <message>
        <source>Have you set the tolerance in Settings &gt; Project Properties &gt; General?</source>
        <translation>Heeft u de tolerantie ingesteld in Instellingen &gt; Project Eigenschappen &gt; Algemeen?</translation>
    </message>
    <message>
        <source>Snap tolerance</source>
        <translation>&apos;Snap&apos; tolerantie</translation>
    </message>
</context>
<context>
    <name>QgsMapserverExport</name>
    <message>
        <source>Choose the QGIS project file</source>
        <translation>Kies het QGIS projectbestand</translation>
    </message>
    <message>
        <source> exists. 
Do you want to overwrite it?</source>
        <translation> bestaat al.
Wilt u het overschrijven?</translation>
    </message>
    <message>
        <source> exists. 
Do you want to overwrite it?</source>
        <comment>a filename is prepended to this text, and appears in a dialog box</comment>
        <translation type="obsolete"> bestaat al.
Wilt u het overschrijven?</translation>
    </message>
    <message>
        <source>MapServer map files (*.map);;All files (*.*)</source>
        <comment>Filter list for selecting files from a dialog box</comment>
        <translation>MapServer map bestanden (*.map);;Alle bestanden (*.*)</translation>
    </message>
    <message>
        <source>Name for the map file</source>
        <translation>Naam voor het &apos;map&apos;-bestand</translation>
    </message>
    <message>
        <source>Overwrite File?</source>
        <translation>Overschrijven?</translation>
    </message>
    <message>
        <source>QGIS Project Files (*.qgs);;All files (*.*)</source>
        <comment>Filter list for selecting files from a dialog box</comment>
        <translation>QGIS Projectbestanden (*.qgs);;Alle bestanden (*.*)</translation>
    </message>
    <message>
        <source> exists. 
Do you want to overwrite it?</source>
        <comment>a fileName is prepended to this text, and appears in a dialog box</comment>
        <translation> bestaat al. 
Wilt u het overschrijven?</translation>
    </message>
</context>
<context>
    <name>QgsMapserverExportBase</name>
    <message>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <source>Browse...</source>
        <translation>Bladeren...</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Annuleren</translation>
    </message>
    <message>
        <source>dd</source>
        <translation>dd</translation>
    </message>
    <message>
        <source>Export LAYER information only</source>
        <translation>Alleen LAYER-informatie exporteren</translation>
    </message>
    <message>
        <source>Export to Mapserver</source>
        <translation>Export naar Mapserver</translation>
    </message>
    <message>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <source>feet</source>
        <translation>voet</translation>
    </message>
    <message>
        <source>Footer</source>
        <translation>Voetregel</translation>
    </message>
    <message>
        <source>Full path to the QGIS project file to export to MapServer map format</source>
        <translation>Volledige pad naar het QGIS-projectbestand welke moet worden geexporteerd naar Mapserver-map-formaat</translation>
    </message>
    <message>
        <source>gif</source>
        <translation>gif</translation>
    </message>
    <message>
        <source>gtiff</source>
        <translation>gtiff</translation>
    </message>
    <message>
        <source>Header</source>
        <translation>Kopregel</translation>
    </message>
    <message>
        <source>Height</source>
        <translation>Hoogte</translation>
    </message>
    <message>
        <source>&amp;Help</source>
        <translation>&amp;Help</translation>
    </message>
    <message>
        <source>If checked, only the layer information will be processed</source>
        <translation>Indien aangevinkt zal alleen de laaginformatie worden verwerkt</translation>
    </message>
    <message>
        <source>Image type</source>
        <translation>Image type</translation>
    </message>
    <message>
        <source>inches</source>
        <translation>inches</translation>
    </message>
    <message>
        <source>jpeg</source>
        <translation>jpeg</translation>
    </message>
    <message>
        <source>kilometers</source>
        <translation>kilometers</translation>
    </message>
    <message>
        <source>Map</source>
        <translation>Kaart</translation>
    </message>
    <message>
        <source>Map file</source>
        <translation>Map-bestand</translation>
    </message>
    <message>
        <source>MaxScale</source>
        <translation>MaxSchaal</translation>
    </message>
    <message>
        <source>meters</source>
        <translation>meters</translation>
    </message>
    <message>
        <source>miles</source>
        <translation>mijlen</translation>
    </message>
    <message>
        <source>MinScale</source>
        <translation>MinSchaal</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Naam</translation>
    </message>
    <message>
        <source>Name for the map file to be created from the QGIS project file</source>
        <translation>Geef een naam voor het &apos;map&apos;-bestand wat wordt gemaakt op basis van het QGIS-projectbestand</translation>
    </message>
    <message>
        <source>&amp;OK</source>
        <translation>&amp;OK</translation>
    </message>
    <message>
        <source>Path to the MapServer template file</source>
        <translation>Pad naar de MapServer &apos;template&apos;-bestanden</translation>
    </message>
    <message>
        <source>png</source>
        <translation>png</translation>
    </message>
    <message>
        <source>Prefix attached to map, scalebar and legend GIF filenames created using this MapFile</source>
        <translation>Voorvoegsel voor &apos;map&apos;, schaalbalk en legenda-GIF-bestanden met dit &apos;Map&apos;-bestand gemaakt</translation>
    </message>
    <message>
        <source>Prefix attached to map, scalebar and legend GIF filenames created using this MapFile. It should be kept short.</source>
        <translation>Voorvoegsel voor &apos;map&apos;, schaalbalk en legenda-GIF-bestanden met dit &apos;Map&apos;-bestand gemaakt. Houd zo kort mogelijk.</translation>
    </message>
    <message>
        <source>QGIS project file</source>
        <translation>QGIS-projectbestand</translation>
    </message>
    <message>
        <source>Save As...</source>
        <translation>Opslaan Als...</translation>
    </message>
    <message>
        <source>swf</source>
        <translation>swf</translation>
    </message>
    <message>
        <source>Template</source>
        <translation>Template</translation>
    </message>
    <message>
        <source>Units</source>
        <translation>Eenheden</translation>
    </message>
    <message>
        <source>userdefined</source>
        <translation>gebruikersgegeven</translation>
    </message>
    <message>
        <source>wbmp</source>
        <translation>wbmp</translation>
    </message>
    <message>
        <source>Web Interface Definition</source>
        <translation>Web Interface Definition</translation>
    </message>
    <message>
        <source>Width</source>
        <translation>Breedte</translation>
    </message>
</context>
<context>
    <name>QgsMarkerDialogBase</name>
    <message>
        <source>Cancel</source>
        <translation type="obsolete">Annuleren</translation>
    </message>
    <message>
        <source>Ok</source>
        <translation type="obsolete">Ok</translation>
    </message>
    <message>
        <source>Choose a marker symbol</source>
        <translation type="obsolete">Kies een symbool voor de marker</translation>
    </message>
    <message>
        <source>Directory</source>
        <translation type="obsolete">Map</translation>
    </message>
    <message>
        <source>...</source>
        <translation type="obsolete">...</translation>
    </message>
    <message>
        <source>New Item</source>
        <translation type="obsolete">Nieuw Onderdeel</translation>
    </message>
</context>
<context>
    <name>QgsMeasureBase</name>
    <message>
        <source>Cl&amp;ose</source>
        <translation>Sl&amp;uiten</translation>
    </message>
    <message>
        <source>Help</source>
        <translation>Help</translation>
    </message>
    <message>
        <source>Measure</source>
        <translation>Opmeten</translation>
    </message>
    <message>
        <source>New</source>
        <translation>Nieuw</translation>
    </message>
    <message>
        <source>Total:</source>
        <translation>Totaal:</translation>
    </message>
    <message>
        <source>Segments</source>
        <translation>Segmenten</translation>
    </message>
</context>
<context>
    <name>QgsMeasureDialog</name>
    <message>
        <source>Segments</source>
        <translation>Segmenten</translation>
    </message>
    <message>
        <source>Segments (in degrees)</source>
        <translation>Segmenten (in graden)</translation>
    </message>
    <message>
        <source>Segments (in feet)</source>
        <translation>Segmenten (in feet)</translation>
    </message>
    <message>
        <source>Segments (in meters)</source>
        <translation>Segmenten (in meters)</translation>
    </message>
</context>
<context>
    <name>QgsMeasureTool</name>
    <message>
        <source>Incorrect measure results</source>
        <translation>Ongeldig meetresultaat</translation>
    </message>
    <message>
        <source>&lt;p&gt;This map is defined with a geographic coordinate system (latitude/longitude) but the map extents suggests that it is actually a projected coordinate system (e.g., Mercator). If so, the results from line or area measurements will be incorrect.&lt;/p&gt;&lt;p&gt;To fix this, explicitly set an appropriate map coordinate system using the &lt;tt&gt;Settings:Project Properties&lt;/tt&gt; menu.</source>
        <translation>
Deze kaart is gemaakt op basis van een geografisch coördinatensysteem (graden latitude/longitude), maar de waarden van de kaartextent suggereren eigenlijk een geprojecteerd coördinatensysteem (b.v. Mercator). Als dit inderdaad zo is, zullen de lijn en oppervlakte metingen niet correct zijn.
Om dit te voorkomen, corrigigeer het juiste coördinatensysteem via
Instellingen
k
eigenschappen
.</translation>
    </message>
</context>
<context>
    <name>QgsMessageViewer</name>
    <message>
        <source>Close</source>
        <translation>Sluiten</translation>
    </message>
    <message>
        <source>Don&apos;t show this message again</source>
        <translation>Deze melding niet meer weergeven</translation>
    </message>
    <message>
        <source>QGIS Message</source>
        <translation>QGIS Melding</translation>
    </message>
</context>
<context>
    <name>QgsMySQLProvider</name>
    <message>
        <source>No GEOS Support!</source>
        <translation type="obsolete">Geen GEOS Ondersteuning!</translation>
    </message>
    <message>
        <source> relation.
The error message from the database was:
</source>
        <translation type="obsolete"> relatie.
De foutmelding van de database was:
</translation>
    </message>
    <message>
        <source>Unable to access relation</source>
        <translation type="obsolete">Relatie kan niet benaderd worden</translation>
    </message>
    <message>
        <source>Unable to access the </source>
        <translation type="obsolete">Verbinden niet mogelijk met </translation>
    </message>
    <message>
        <source>Your PostGIS installation has no GEOS support.
Feature selection and identification will not work properly.
Please install PostGIS with GEOS support (http://geos.refractions.net)</source>
        <translation type="obsolete">Uw PostGIS installatie biedt geen GEOS ondersteuning. 
Selecteren en identificeren van objecten zal niet werken.
Installeer PostGIS met GEOS-ondersteuning (http://geos.refractions.net)</translation>
    </message>
</context>
<context>
    <name>QgsNewConnection</name>
    <message>
        <source>Connection failed - Check settings and try again.

Extended error information:
</source>
        <translation>Verbinden mislukt - Controleer de instellingen en probeer opnieuw.

Uitgebreide foutinformatie:
</translation>
    </message>
    <message>
        <source>Connection to %1 was successful</source>
        <translation>Verbinding met %1 is geslaagd</translation>
    </message>
    <message>
        <source>Test connection</source>
        <translation>Test verbinding</translation>
    </message>
</context>
<context>
    <name>QgsNewConnectionBase</name>
    <message>
        <source>5432</source>
        <translation>5432</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Annuleren</translation>
    </message>
    <message>
        <source>Connection Information</source>
        <translation>Verbindingsinformatie</translation>
    </message>
    <message>
        <source>Create a New PostGIS connection</source>
        <translation>Nieuwe PostGIS-verbinding aanmaken</translation>
    </message>
    <message>
        <source>Database</source>
        <translation>Database</translation>
    </message>
    <message>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <source>Help</source>
        <translation>Help</translation>
    </message>
    <message>
        <source>Host</source>
        <translation>Host</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Naam</translation>
    </message>
    <message>
        <source>Name of the new connection</source>
        <translation>Naam van de nieuwe verbinding</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>Only look in the geometry_columns table</source>
        <translation>Alleen in de geometrie-kolommen kijken</translation>
    </message>
    <message>
        <source>Only look in the &apos;public&apos; schema</source>
        <translation>Alleen in het &apos;publieke-schema&apos; kijken</translation>
    </message>
    <message>
        <source>Password</source>
        <translation>Wachtwoord</translation>
    </message>
    <message>
        <source>Port</source>
        <translation>Poort</translation>
    </message>
    <message>
        <source>Restricts the displayed tables to those that are in the geometry_columns table. This can speed up the initial display of spatial tables.</source>
        <translation>Beperkt de getoonde tabellen tot de tabellen uit de tabel &apos;geometry_columns&apos;. Dit versneld het tonen van tabellen met ruimtelijke informatie.</translation>
    </message>
    <message>
        <source>Restrict the displayed tables to those that are in the geometry_columns table</source>
        <translation>Beperk de getoonde tabellen tot de tabellen uit de tabel &apos;geometry_columns&apos;</translation>
    </message>
    <message>
        <source>Restrict the search to the public schema for spatial tables not in the geometry_columns table</source>
        <translation>Beperk de zoekaktie naar tabellen met ruimtelijke informatie tot tabellen die niet in de tabel &apos;geometry_columns&apos; voorkomen</translation>
    </message>
    <message>
        <source>Save Password</source>
        <translation>Wachtwoord Opslaan</translation>
    </message>
    <message>
        <source>Test Connect</source>
        <translation>Test verbinding</translation>
    </message>
    <message>
        <source>Username</source>
        <translation>Gebruikersnaam</translation>
    </message>
    <message>
        <source>When searching for spatial tables that are not in the geometry_columns tables, restrict the search to tables that are in the public schema (for some databases this can save lots of time)</source>
        <translation>Bij het zoeken naar ruimtelijke tabellen die niet voorkomen in de &apos;geometry_columns&apos;-tabellen beperk het zoeken dan tot die in het publieke schema (voor sommige databases kan dit veel tijd besparen)</translation>
    </message>
</context>
<context>
    <name>QgsNewHttpConnectionBase</name>
    <message>
        <source>Cancel</source>
        <translation type="obsolete">Annuleren</translation>
    </message>
    <message>
        <source>Connection Information</source>
        <translation type="obsolete">Verbindingsinformatie</translation>
    </message>
    <message>
        <source>Create a New WMS connection</source>
        <translation type="obsolete">Nieuwe WMS-verbinding aanmaken</translation>
    </message>
    <message>
        <source>F1</source>
        <translation type="obsolete">F1</translation>
    </message>
    <message>
        <source>Help</source>
        <translation type="obsolete">Help</translation>
    </message>
    <message>
        <source>HTTP address of the Web Map Server</source>
        <translation>HTTP-adres van de WebMapServer</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Naam</translation>
    </message>
    <message>
        <source>Name of the new connection</source>
        <translation>Naam van de nieuwe verbinding</translation>
    </message>
    <message>
        <source>Name of your HTTP proxy (optional)</source>
        <translation type="obsolete">Naam van uw HTTP-proxy (optioneel)</translation>
    </message>
    <message>
        <source>OK</source>
        <translation type="obsolete">OK</translation>
    </message>
    <message>
        <source>Password for your HTTP proxy (optional)</source>
        <translation type="obsolete">Wachtwoord van uw HTTP-proxy (optioneel)</translation>
    </message>
    <message>
        <source>Port number of your HTTP proxy (optional)</source>
        <translation type="obsolete">Poortnummer van uw HTTP-proxy (optioneel)</translation>
    </message>
    <message>
        <source>Proxy Host</source>
        <translation type="obsolete">Proxy-Host</translation>
    </message>
    <message>
        <source>Proxy Password</source>
        <translation type="obsolete">Proxy-Wachtwoord</translation>
    </message>
    <message>
        <source>Proxy Port</source>
        <translation type="obsolete">Proxy-Poort</translation>
    </message>
    <message>
        <source>Proxy User</source>
        <translation type="obsolete">Proxy-Gebruiker</translation>
    </message>
    <message>
        <source>URL</source>
        <translation>URL</translation>
    </message>
    <message>
        <source>Your user name for the HTTP proxy (optional)</source>
        <translation type="obsolete">Uw gebruikersnaam van de HTTP-proxy (optioneel)</translation>
    </message>
    <message>
        <source>Create a new WMS connection</source>
        <translation>Nieuwe WMS-verbinding aanmaken</translation>
    </message>
    <message>
        <source>Connection details</source>
        <translation>Verbindingseigenschappen</translation>
    </message>
</context>
<context>
    <name>QgsNorthArrowPlugin</name>
    <message>
        <source>Bottom Left</source>
        <translation>LinksOnder</translation>
    </message>
    <message>
        <source>Bottom Right</source>
        <translation>RechtsOnder</translation>
    </message>
    <message>
        <source>Creates a north arrow that is displayed on the map canvas</source>
        <translation>Aanmaken van een Noordpijl om op de kaart te tonen</translation>
    </message>
    <message>
        <source>&amp;Decorations</source>
        <translation>&amp;Decoraties</translation>
    </message>
    <message>
        <source>&amp;North Arrow</source>
        <translation>&amp;Noord Pijl</translation>
    </message>
    <message>
        <source>North arrow pixmap not found</source>
        <translation>&apos;Pixmap&apos; voor de noordpijl niet gevonden</translation>
    </message>
    <message>
        <source>Top Left</source>
        <translation>LinksBoven</translation>
    </message>
    <message>
        <source>Top Right</source>
        <translation>RechtsBoven</translation>
    </message>
</context>
<context>
    <name>QgsNorthArrowPluginGui</name>
    <message>
        <source>Pixmap not found</source>
        <translation>&apos;Pixmap&apos; niet gevonden</translation>
    </message>
</context>
<context>
    <name>QgsNorthArrowPluginGuiBase</name>
    <message>
        <source>Angle</source>
        <translation>Hoek</translation>
    </message>
    <message>
        <source>Bottom Left</source>
        <translation>LinksOnder</translation>
    </message>
    <message>
        <source>Bottom Right</source>
        <translation>RechtsOnder</translation>
    </message>
    <message>
        <source>Browse...</source>
        <translation>Bladeren...</translation>
    </message>
    <message>
        <source>Enable North Arrow</source>
        <translation>Noordpijl gebruiken</translation>
    </message>
    <message>
        <source>Icon</source>
        <translation>Pictogram</translation>
    </message>
    <message>
        <source>New Item</source>
        <translation type="obsolete">Nieuw Onderdeel</translation>
    </message>
    <message>
        <source>North Arrow Plugin</source>
        <translation>Noordpijl Plugin</translation>
    </message>
    <message>
        <source>Placement</source>
        <translation>Plaats</translation>
    </message>
    <message>
        <source>Placement on screen</source>
        <translation>Plaats op het scherm</translation>
    </message>
    <message>
        <source>Preview of north arrow</source>
        <translation>Voorbeschouwing van de noordpijl</translation>
    </message>
    <message>
        <source>Properties</source>
        <translation>Eigenschappen</translation>
    </message>
    <message>
        <source>Set direction automatically</source>
        <translation>Richting automatisch bepalen</translation>
    </message>
    <message>
        <source>Top Left</source>
        <translation>LinksBoven</translation>
    </message>
    <message>
        <source>Top Right</source>
        <translation>RechtsBoven</translation>
    </message>
</context>
<context>
    <name>QgsOptions</name>
    <message>
        <source>Detected active locale on your system: </source>
        <translation>Gedetecteerde huidige &apos;locale&apos; op uw systeem: </translation>
    </message>
    <message>
        <source>to vertex</source>
        <translation>naar hoekpunt</translation>
    </message>
    <message>
        <source>to segment</source>
        <translation>naar segment</translation>
    </message>
    <message>
        <source>to vertex and segment</source>
        <translation>naar hoekpunt en segment</translation>
    </message>
    <message>
        <source>Semi transparent circle</source>
        <translation>Semi transparante cirkel</translation>
    </message>
    <message>
        <source>Cross</source>
        <translation>Kruis</translation>
    </message>
</context>
<context>
    <name>QgsOptionsBase</name>
    <message>
        <source>%</source>
        <translation>%</translation>
    </message>
    <message>
        <source>Additional Info</source>
        <translation>Aanvullende Informatie</translation>
    </message>
    <message>
        <source>&amp;Appearance</source>
        <translation type="obsolete">&amp;Opmaak</translation>
    </message>
    <message>
        <source>Appearance</source>
        <translation type="obsolete">Opmaak</translation>
    </message>
    <message>
        <source>Ask to save project changes when required</source>
        <translation type="obsolete">Vraag om projekt op te slaan indien nodig</translation>
    </message>
    <message>
        <source>Background Color:</source>
        <translation type="obsolete">Achtergrondkleur:</translation>
    </message>
    <message>
        <source>&lt;b&gt;Note: &lt;/b&gt;Theme changes take effect the next time QGIS is started</source>
        <translation>&lt;b&gt;Opmerking: &lt;/b&gt;Aanpassingen aan het thema verschijnen de eerstvolgende keer dat QGIS opstart</translation>
    </message>
    <message>
        <source>By default new la&amp;yers added to the map should be displayed</source>
        <translation>Standaard zullen nieuw toegevoegde la&amp;gen aan de kaart direct worden afgebeeld</translation>
    </message>
    <message>
        <source>Capitalise layer name</source>
        <translation type="obsolete">Zet laagnaam in hoofdletters</translation>
    </message>
    <message>
        <source>Continuously redraw the map when dragging the legend/map divider</source>
        <translation>Kaart continu hertekenen bij het verslepen van de legenda/kaart-tussenbalk</translation>
    </message>
    <message>
        <source>Default Map Appearance (Overridden by project properties)</source>
        <translation type="obsolete">Standaard Kaart Opmaak (Overschreven door projecteigenschappen)</translation>
    </message>
    <message>
        <source>Default Snapping Tolerance (in layer units):</source>
        <translation type="obsolete">Standaard &apos;Snap&apos;-tolerantie (in laageenheden):</translation>
    </message>
    <message>
        <source>Detected active locale on your system:</source>
        <translation>Gedetecteerde aktieve locale op uw systeem:</translation>
    </message>
    <message>
        <source>Digitizing</source>
        <translation>Digitaliseren</translation>
    </message>
    <message>
        <source>Ellipsoid for distance calculations:</source>
        <translation type="obsolete">Ellipsoïde voor afstandsberekeningen:</translation>
    </message>
    <message>
        <source>features</source>
        <translation type="obsolete">objecten</translation>
    </message>
    <message>
        <source>Fix problems with incorrectly filled polygons</source>
        <translation>Problemen met fout (gevulde) polygonen oplossen</translation>
    </message>
    <message>
        <source>Force Override System Locale</source>
        <translation type="obsolete">Bekrachtig het Overschrijven van de Systeem Locale</translation>
    </message>
    <message>
        <source>General</source>
        <translation type="obsolete">Algemeen</translation>
    </message>
    <message>
        <source>&amp;General</source>
        <translation>&amp;Algemeen</translation>
    </message>
    <message>
        <source>Global default projection displa&amp;yed below will be used.</source>
        <translation type="obsolete">On&amp;derstaande globale standaard projectie zal worden gebruikt.</translation>
    </message>
    <message>
        <source>Hide splash screen at startup</source>
        <translation>Verberg het openingsscherm bij de opstart</translation>
    </message>
    <message>
        <source>&amp;Icon Theme</source>
        <translation type="obsolete">&amp;Icoon Thema</translation>
    </message>
    <message>
        <source>Initial Visibility</source>
        <translation type="obsolete">Begin Zichtbaarheid</translation>
    </message>
    <message>
        <source>Line Colour:</source>
        <translation type="obsolete">Lijnkleur:</translation>
    </message>
    <message>
        <source>Line Width:</source>
        <translation type="obsolete">Lijndikte:</translation>
    </message>
    <message>
        <source>Line width in pixels</source>
        <translation>Lijndikte in pixels</translation>
    </message>
    <message>
        <source>Locale</source>
        <translation>Locale</translation>
    </message>
    <message>
        <source>Locale to use instead</source>
        <translation>Te gebruiken locale</translation>
    </message>
    <message>
        <source>Make lines appear less jagged at the expense of some drawing performance</source>
        <translation>Maak de lijnen minder rafeling ten koste van de tijd dat het tekenen kost</translation>
    </message>
    <message>
        <source>Map display will be updated (drawn) after this many features have been read from the data source</source>
        <translation>Kaartvensster zal worden hertekend nadat dit aantal objecten zijn ingelezen vanuit de databrond</translation>
    </message>
    <message>
        <source>&amp;Map tools</source>
        <translation>&amp;Kaart gereedschap</translation>
    </message>
    <message>
        <source>Measure tool</source>
        <translation>Meetgereedschap</translation>
    </message>
    <message>
        <source>Nothing</source>
        <translation>Niets</translation>
    </message>
    <message>
        <source>Panning and zooming</source>
        <translation>Schuiven en zoomen</translation>
    </message>
    <message>
        <source>Pro&amp;jection</source>
        <translation type="obsolete">Pro&amp;jectie</translation>
    </message>
    <message>
        <source>QGIS Options</source>
        <translation>QGIS Opties</translation>
    </message>
    <message>
        <source>&amp;Rendering</source>
        <translation>&amp;Renderen</translation>
    </message>
    <message>
        <source>Rubberband</source>
        <translation>Rubberband</translation>
    </message>
    <message>
        <source>Search radius</source>
        <translation>Zoek radius</translation>
    </message>
    <message>
        <source>Select Global Default ...</source>
        <translation>Selecteer Globale Standaard ...</translation>
    </message>
    <message>
        <source>Selecting this will unselect the &apos;make lines less&apos; jagged toggle</source>
        <translation>Dit selecteren zal &apos;maak lijnen minder rafelig&apos; deselecteren</translation>
    </message>
    <message>
        <source>Snapping</source>
        <translation>Snapping</translation>
    </message>
    <message>
        <source>Warn me when opening a project file saved with an older version of QGIS</source>
        <translation type="obsolete">Geef een waarschuwing bij het openen van een projektfile uit een oudere versie van QGIS</translation>
    </message>
    <message>
        <source>When layer is loaded that has no projection information</source>
        <translation type="obsolete">Wanneer een laag is geladen zonder projectie-informatie</translation>
    </message>
    <message>
        <source>Zoom</source>
        <translation>Zoom</translation>
    </message>
    <message>
        <source>Zoom and recenter</source>
        <translation>Zoom en recenter</translation>
    </message>
    <message>
        <source>Zoom to mouse cursor</source>
        <translation>Zoom naar muis-cursor</translation>
    </message>
    <message>
        <source>Project files</source>
        <translation>Project-bestanden</translation>
    </message>
    <message>
        <source>Prompt to save project changes when required</source>
        <translation>Geef een waarschuwing om project op te slaan indien nodig</translation>
    </message>
    <message>
        <source>Warn when opening a project file saved with an older version of QGIS</source>
        <translation>Geef een waarschuwing bij het openen van een projectfile uit een oudere versie van QGIS</translation>
    </message>
    <message>
        <source>Default Map Appearance (overridden by project properties)</source>
        <translation>Standaard Kaart Opmaak (Overschreven door projecteigenschappen)</translation>
    </message>
    <message>
        <source>Selection color</source>
        <translation>Selectie-kleur</translation>
    </message>
    <message>
        <source>Background color</source>
        <translation>Achtergrondkleur</translation>
    </message>
    <message>
        <source>&amp;Application</source>
        <translation>Programma (&amp;A)</translation>
    </message>
    <message>
        <source>Icon theme</source>
        <translation>Icoon Thema</translation>
    </message>
    <message>
        <source>Capitalise layer names in legend</source>
        <translation>Laagnamen in hoofdletters in legenda</translation>
    </message>
    <message>
        <source>Display classification attribute names in legend</source>
        <translation>Toon classificatie-attribuutnamen in legenda</translation>
    </message>
    <message>
        <source>Rendering behavior</source>
        <translation>Hertekengedrag</translation>
    </message>
    <message>
        <source>Number of features to draw before updating the display</source>
        <translation>Aantal objecten alvorens de kaart te hertekenen</translation>
    </message>
    <message>
        <source>&lt;b&gt;Note:&lt;/b&gt; Use zero to prevent display updates until all features have been rendered</source>
        <translation>&lt;b&gt;Opmerking:&lt;/b&gt;Gebruik nul om hertekening te voorkomen tot alle features zijn getekend</translation>
    </message>
    <message>
        <source>Rendering quality</source>
        <translation>Renderkwaliteit</translation>
    </message>
    <message>
        <source>Zoom factor</source>
        <translation>Zoomfactor</translation>
    </message>
    <message>
        <source>Mouse wheel action</source>
        <translation>Muiswielgedrag</translation>
    </message>
    <message>
        <source>Rubberband color</source>
        <translation>Rubberband kleur</translation>
    </message>
    <message>
        <source>Ellipsoid for distance calculations</source>
        <translation>Ellipsoïde voor afstandsberekeningen</translation>
    </message>
    <message>
        <source>&lt;b&gt;Note:&lt;/b&gt; Specify the search radius as a percentage of the map width</source>
        <translation>&lt;b&gt;Opmerking: &lt;/b&gt;Geef de zoekradius op als percentage van de kaartbreedte</translation>
    </message>
    <message>
        <source>Search radius for identifying features and displaying map tips</source>
        <translation>Zoekradius voor de identificatie van objecten en het vertonen van kaarttips</translation>
    </message>
    <message>
        <source>Line width</source>
        <translation>Lijndikte</translation>
    </message>
    <message>
        <source>Line colour</source>
        <translation>Lijnkleur</translation>
    </message>
    <message>
        <source>Default snap mode</source>
        <translation>Standaard &apos;snap&apos;-modus</translation>
    </message>
    <message>
        <source>Default snapping tolerance in layer units</source>
        <translation>Standaard &apos;snapping&apos;-tolerantie in laageenheden</translation>
    </message>
    <message>
        <source>Search radius for vertex edits in layer units</source>
        <translation>Zoekradius voor hoekpunt-aanpassingen in laageenheden</translation>
    </message>
    <message>
        <source>Vertex markers</source>
        <translation>Hoekpunten</translation>
    </message>
    <message>
        <source>Marker style</source>
        <translation>Markerstijl</translation>
    </message>
    <message>
        <source>Prompt for projection</source>
        <translation type="obsolete">Vraag om projectie</translation>
    </message>
    <message>
        <source>Project wide default projection will be used</source>
        <translation type="obsolete">Gebruik de standaard projectie voor dit project</translation>
    </message>
    <message>
        <source>Global default projection displa&amp;yed below will be used</source>
        <translation type="obsolete">De standaard projectie zoals hieronder getoond zal worden gebruikt</translation>
    </message>
    <message>
        <source>Override system locale</source>
        <translation>Systeem locale negeren</translation>
    </message>
    <message>
        <source>&lt;b&gt;Note:&lt;/b&gt; Enabling / changing overide on local requires an application restart</source>
        <translation>&lt;b&gt;Opmerking:&lt;/b&gt; Voor het aanpassen van de locale/taal moet het programma worden herstart</translation>
    </message>
    <message>
        <source>Proxy</source>
        <translation>Proxy</translation>
    </message>
    <message>
        <source>Use proxy for web access</source>
        <translation>Gebruik een proxy voor internettoegang</translation>
    </message>
    <message>
        <source>Host</source>
        <translation>Host</translation>
    </message>
    <message>
        <source>Port</source>
        <translation>Poort</translation>
    </message>
    <message>
        <source>User</source>
        <translation>Gebruiker</translation>
    </message>
    <message>
        <source>Leave this blank if no proxy username / password are required</source>
        <translation>Laat dit leeg als gebruikersnaam/wachtwoord niet nodig zijn</translation>
    </message>
    <message>
        <source>Password</source>
        <translation>Wachtwoord</translation>
    </message>
    <message>
        <source>Open attribute table in a dock window</source>
        <translation>Open de attribuuttabel in een &apos;dock&apos;-venster</translation>
    </message>
    <message>
        <source>CRS</source>
        <translation>CRS</translation>
    </message>
    <message>
        <source>When layer is loaded that has no coordinate reference system (CRS)</source>
        <translation>Wanneer een laag is geladen zonder een ruimtelijk referentie systeem (CRS)</translation>
    </message>
    <message>
        <source>Prompt for CRS</source>
        <translation>Vraam om CRS</translation>
    </message>
    <message>
        <source>Project wide default CRS will be used</source>
        <translation>Gebruik de standaard projectie voor dit project</translation>
    </message>
    <message>
        <source>Global default CRS displa&amp;yed below will be used</source>
        <translation>De standaard CRS zoals hieronder getoond zal worden gebruikt (&amp;y)</translation>
    </message>
</context>
<context>
    <name>QgsPasteTransformationsBase</name>
    <message>
        <source>Add New Transfer</source>
        <translation>Nieuwe &apos;Transfer&apos; Toeveogen</translation>
    </message>
    <message>
        <source>&lt;b&gt;Note: This function is not useful yet!&lt;/b&gt;</source>
        <translation>&lt;b&gt;Opmerking: deze functie werkt nog niet!&lt;/b&gt;</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Annuleren</translation>
    </message>
    <message>
        <source>Destination</source>
        <translation>Doel</translation>
    </message>
    <message>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <source>&amp;Help</source>
        <translation>&amp;Help</translation>
    </message>
    <message>
        <source>&amp;OK</source>
        <translation>&amp;OK</translation>
    </message>
    <message>
        <source>Paste Transformations</source>
        <translation>Transformaties Plakken</translation>
    </message>
    <message>
        <source>Source</source>
        <translation>Bron</translation>
    </message>
</context>
<context>
    <name>QgsPatternDialogBase</name>
    <message>
        <source>Cancel</source>
        <translation type="obsolete">Annuleren</translation>
    </message>
    <message>
        <source>Ok</source>
        <translation type="obsolete">Ok</translation>
    </message>
    <message>
        <source>Select a fill pattern</source>
        <translation type="obsolete">Kies een vulpatroon</translation>
    </message>
    <message>
        <source>No Fill</source>
        <translation type="obsolete">Geen vulling</translation>
    </message>
</context>
<context>
    <name>QgsPgGeoprocessing</name>
    <message>
        <source>A new layer is created in the database with the buffered features.</source>
        <translation>Er is een nieuw laag aangemaakt in de database met de gebufferde objecten.</translation>
    </message>
    <message>
        <source>&amp;Buffer features</source>
        <translation>&amp;Buffer objecten</translation>
    </message>
    <message>
        <source>Buffer features in layer %1</source>
        <translation>Buffer aanmaken voor objecten in laag %1</translation>
    </message>
    <message>
        <source>Buffer function requires GEOS support in PostGIS</source>
        <translation>De buffer-functie vereist GEOS-ondersteuning in uw PostGIS</translation>
    </message>
    <message>
        <source>Error connecting to the database</source>
        <translation>Fout bij het verbinden met de database</translation>
    </message>
    <message>
        <source>Failed to create the output table </source>
        <translation>Aanmaken van uitvoertabel mislukt</translation>
    </message>
    <message>
        <source>&amp;Geoprocessing</source>
        <translation>&amp;Geoprocessing</translation>
    </message>
    <message>
        <source>Geoprocessing functions are only available for PostgreSQL/PostGIS Layers</source>
        <translation>Geoprocessing functions zijn alleen beschikbaar voor PostgreSQL/PostGIS-lagen</translation>
    </message>
    <message>
        <source> is not a PostgreSQL/PostGIS layer.
</source>
        <translation> is geen PostgreSQL/PostGIS-laag.
</translation>
    </message>
    <message>
        <source>No Active Layer</source>
        <translation>Geen Aktieve Laag</translation>
    </message>
    <message>
        <source>No GEOS support</source>
        <translation>GEOS niet ondersteund</translation>
    </message>
    <message>
        <source>Not a PostgreSQL/PostGIS Layer</source>
        <translation>Geen PostgreSQL/PostGIS-laag</translation>
    </message>
    <message>
        <source>Unable to add geometry column</source>
        <translation>Toevoegen van geometrie-kolm niet mogelijk</translation>
    </message>
    <message>
        <source>Unable to add geometry column to the output table </source>
        <translation>Toevoegen van geometrie-kolm aan de uitvoertabel niet mogelijk</translation>
    </message>
    <message>
        <source>Unable to create table</source>
        <translation>Aanmaken van tabel niet mogelijk</translation>
    </message>
    <message>
        <source>You must select a layer in the legend to buffer</source>
        <translation>Selecteer een laag in de legenda om te bufferen</translation>
    </message>
    <message>
        <source>Create a buffer for a PostgreSQL layer. </source>
        <translation>Aanmaken van een buffer voor een PosgreSQL-laag.</translation>
    </message>
</context>
<context>
    <name>QgsPgQueryBuilder</name>
    <message>
        <source>An error occurred when executing the query:</source>
        <translation>Er is een fout opgetreden bij het uitvoeren van de volgende query:</translation>
    </message>
    <message>
        <source>Connection Failed</source>
        <translation>Verbinding mislukt</translation>
    </message>
    <message>
        <source>Connection to the database failed:</source>
        <translation>Verbinden met de database mislukt:</translation>
    </message>
    <message>
        <source>Database error</source>
        <translation>Database-fout</translation>
    </message>
    <message>
        <source>Error in Query</source>
        <translation>Fout in Query</translation>
    </message>
    <message>
        <source>No Query</source>
        <translation>Geen Query</translation>
    </message>
    <message>
        <source>No Records</source>
        <translation>Geen Records</translation>
    </message>
    <message>
        <source>&lt;p&gt;Failed to get sample of field values using SQL:&lt;/p&gt;&lt;p&gt;</source>
        <translation>&lt;p&gt;Geen voorbeeld voorhanden van het veld gebruikmakend van SQL:&lt;/p&gt;&lt;p&gt;</translation>
    </message>
    <message>
        <source>Query Failed</source>
        <translation>Query Mislukt</translation>
    </message>
    <message>
        <source>Query Result</source>
        <translation>Query Resultaten</translation>
    </message>
    <message>
        <source> rows.</source>
        <translation>rijen.</translation>
    </message>
    <message>
        <source>Table &lt;b&gt;%1&lt;/b&gt; in database &lt;b&gt;%2&lt;/b&gt; on host &lt;b&gt;%3&lt;/b&gt;, user &lt;b&gt;%4&lt;/b&gt;</source>
        <translation>Tabel &lt;b&gt;%1&lt;/b&gt; in database &lt;b&gt;%2&lt;/b&gt; op host &lt;b&gt;%3&lt;/b&gt;, gebruiker &lt;b&gt;%4&lt;/b&gt;</translation>
    </message>
    <message>
        <source>The query you specified results in zero records being returned. Valid PostgreSQL layers must have at least one feature.</source>
        <translation>De door u opgegeven query resulteert niet in records. Geldige PostgreSQL-lagen moeten minsten een object bevatten.</translation>
    </message>
    <message>
        <source>The where clause returned </source>
        <translation>Het &apos;where&apos;-deel levert op</translation>
    </message>
    <message>
        <source>You must create a query before you can test it</source>
        <translation>U moet een query aanmaken voor u die kunt testen</translation>
    </message>
</context>
<context>
    <name>QgsPgQueryBuilderBase</name>
    <message>
        <source>&lt;</source>
        <translation>&lt;</translation>
    </message>
    <message>
        <source>&lt;=</source>
        <translation>&lt;=</translation>
    </message>
    <message>
        <source>=</source>
        <translation>=</translation>
    </message>
    <message>
        <source>&gt;</source>
        <translation>&gt;</translation>
    </message>
    <message>
        <source>&gt;=</source>
        <translation>&gt;=</translation>
    </message>
    <message>
        <source>!=</source>
        <translation>!=</translation>
    </message>
    <message>
        <source>%</source>
        <translation>%</translation>
    </message>
    <message>
        <source>All</source>
        <translation>Alles</translation>
    </message>
    <message>
        <source>AND</source>
        <translation>AND</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Annuleren</translation>
    </message>
    <message>
        <source>Clear</source>
        <translation>Leegmaken</translation>
    </message>
    <message>
        <source>Fields</source>
        <translation>Velden</translation>
    </message>
    <message>
        <source>ILIKE</source>
        <translation>ILIKE</translation>
    </message>
    <message>
        <source>IN</source>
        <translation>IN</translation>
    </message>
    <message>
        <source>LIKE</source>
        <translation>LIKE</translation>
    </message>
    <message>
        <source>NOT</source>
        <translation>NOT</translation>
    </message>
    <message>
        <source>NOT IN</source>
        <translation>NOT IN</translation>
    </message>
    <message>
        <source>Ok</source>
        <translation>Ok</translation>
    </message>
    <message>
        <source>Operators</source>
        <translation>Operatoren</translation>
    </message>
    <message>
        <source>OR</source>
        <translation>OR</translation>
    </message>
    <message>
        <source>PostgreSQL Query Builder</source>
        <translation>PostgreSQL Query-Bouwer</translation>
    </message>
    <message>
        <source>Sample</source>
        <translation>Voorbeeld</translation>
    </message>
    <message>
        <source>SQL where clause</source>
        <translation>SQL &apos;where&apos;-deel</translation>
    </message>
    <message>
        <source>Test</source>
        <translation>Test</translation>
    </message>
    <message>
        <source>Values</source>
        <translation>Waarden</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Retrieve &lt;span style=&quot; font-weight:600;&quot;&gt;all&lt;/span&gt; the record in the vector file (&lt;span style=&quot; font-style:italic;&quot;&gt;if the table is big, the operation can consume some time&lt;/span&gt;)&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Haal &lt;span style=&quot; font-weight:600;&quot;&gt;alle&lt;/span&gt; records op in dit vectorbestand (&lt;span style=&quot; font-style:italic;&quot;&gt;bij een grote tabel kan dit veel tijd kosten&lt;/span&gt;)&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Take a &lt;span style=&quot; font-weight:600;&quot;&gt;sample&lt;/span&gt; of records in the vector file&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Neem een &lt;span style=&quot; font-weight:600;&quot;&gt;voorbeeldset&lt;/span&gt; van records in dit vectorbestand&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;List of values for the current field.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Lijst van aanwezige waarden in dit veld.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;List of fields in this vector file&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Lijst van velden voor dit vectorbestand&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Datasource</source>
        <translation>Databron</translation>
    </message>
</context>
<context>
    <name>QgsPluginInstaller</name>
    <message>
        <source>Couldn&apos;t parse output from the repository</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Couldn&apos;t open the system plugin directory</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Couldn&apos;t open the local plugin directory</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Fetch Python Plugins...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Install more plugins from remote repositories</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Looking for new plugins...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>There is a new plugin available</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>There is a plugin update available</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>QGIS Python Plugin Installer</source>
        <translation type="unfinished">QGIS Plugin Installeren</translation>
    </message>
    <message>
        <source>Error reading repository:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Nothing to remove! Plugin directory doesn&apos;t exist:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Failed to remove the directory:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Check permissions or remove it manually</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsPluginInstallerDialog</name>
    <message>
        <source>QGIS Python Plugin Installer</source>
        <translation type="unfinished">QGIS Plugin Installeren</translation>
    </message>
    <message>
        <source>Error reading repository:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>all repositories</source>
        <translation type="unfinished">alle repositories</translation>
    </message>
    <message>
        <source>connected</source>
        <translation type="unfinished">verbonden</translation>
    </message>
    <message>
        <source>This repository is connected</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>unavailable</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>This repository is enabled, but unavailable</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>disabled</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>This repository is disabled</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>This repository is blocked due to incompatibility with your Quantum GIS version</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>orphans</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>any status</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>not installed</source>
        <comment>plural</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>installed</source>
        <comment>plural</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>upgradeable and news</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>This plugin is not installed</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>This plugin is installed</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>This plugin is installed, but there is an updated version available</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>This plugin is installed, but I can&apos;t find it in any enabled repository</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>This plugin is not installed and is seen for the first time</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>This plugin is installed and is newer than its version available in a repository</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>not installed</source>
        <comment>singular</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>installed</source>
        <comment>singular</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>upgradeable</source>
        <comment>singular</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>new!</source>
        <comment>singular</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>invalid</source>
        <comment>singular</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>installed version</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>available version</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>That&apos;s the newest available version</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>There is no version available for download</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>only locally available</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Install plugin</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Reinstall plugin</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Upgrade plugin</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Install/upgrade plugin</source>
        <translation type="unfinished">Installeer/upgrade plugin</translation>
    </message>
    <message>
        <source>Downgrade plugin</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Are you sure you want to downgrade the plugin to the latest available version? The installed one is newer!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Plugin installation failed</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Plugin has disappeared</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The plugin seems to have been installed but I don&apos;t know where. Probably the plugin package contained a wrong named directory.
Please search the list of installed plugins. I&apos;m nearly sure you&apos;ll find the plugin there, but I just can&apos;t determine which of them it is. It also means that I won&apos;t be able to determine if this plugin is installed and inform you about available updates. However the plugin may work. Please contact the plugin author and submit this issue.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Plugin installed successfully</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Python plugin installed.
You have to enable it in the Plugin Manager.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Python plugin reinstalled.
You have to restart Quantum GIS to reload it.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Plugin uninstall failed</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Are you sure you want to uninstall the following plugin?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Warning: this plugin isn&apos;t available in any accessible repository!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Plugin uninstalled successfully</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>You are going to add some plugin repositories neither authorized nor supported by the Quantum GIS team, however provided by folks associated with us. Plugin authors generally make efforts to make their works useful and safe, but we can&apos;t assume any responsibility for them. FEEL WARNED!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Unable to add another repository with the same URL!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Are you sure you want to remove the following repository?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>This plugin is incompatible with your Quantum GIS version and probably won&apos;t work.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The required Python module is not installed.
For more information, please visit its homepage.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>This plugin seems to be broken.
It has been installed but can&apos;t be loaded.
Here is the error message:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Note that it&apos;s an uninstallable core plugin</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>This plugin is broken</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>This plugin requires a newer version of Quantum GIS</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>This plugin requires a missing module</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Plugin reinstalled successfully</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The plugin is designed for a newer version of Quantum GIS. The minimum required version is:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The plugin depends on some components missing on your system. You need to install the following Python module in order to enable it:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The plugin is broken. Python said:</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsPluginInstallerDialogBase</name>
    <message>
        <source>QGIS Python Plugin Installer</source>
        <translation>QGIS Plugin Installeren</translation>
    </message>
    <message>
        <source>QGIS Plugin Installer</source>
        <translation type="obsolete">QGIS Plugin Installeren</translation>
    </message>
    <message>
        <source>Plugins</source>
        <translation>Plugins</translation>
    </message>
    <message>
        <source>List of available and installed plugins</source>
        <translation>Lijst van beschikbare en geïnstalleerde plugins</translation>
    </message>
    <message>
        <source>Filter:</source>
        <translation>Filter:</translation>
    </message>
    <message>
        <source>Display only plugins containing this word in their metadata</source>
        <translation>Toon alleen de plugins met deze term in de metadata</translation>
    </message>
    <message>
        <source>Display only plugins from given repository</source>
        <translation>Toon alleen de plugins van de gegeven repository</translation>
    </message>
    <message>
        <source>all repositories</source>
        <translation>alle repositories</translation>
    </message>
    <message>
        <source>Display only plugins with matching status</source>
        <translation>Toon allen plugins met dezelfde status</translation>
    </message>
    <message>
        <source>Status</source>
        <translation>Status</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Naam</translation>
    </message>
    <message>
        <source>Version</source>
        <translation>Versie</translation>
    </message>
    <message>
        <source>Description</source>
        <translation>Omschrijving</translation>
    </message>
    <message>
        <source>Author</source>
        <translation>Auteur</translation>
    </message>
    <message>
        <source>Repository</source>
        <translation>Repository</translation>
    </message>
    <message>
        <source>Install, reinstall or upgrade the selected plugin</source>
        <translation>Installeer, herinstalleer of upgrade de geselecteerde plugin</translation>
    </message>
    <message>
        <source>Install/upgrade plugin</source>
        <translation>Installeer/upgrade plugin</translation>
    </message>
    <message>
        <source>Uninstall the selected plugin</source>
        <translation>Deinstalleer de geselecteerde plugin</translation>
    </message>
    <message>
        <source>Uninstall plugin</source>
        <translation>Deinstalleer plugin</translation>
    </message>
    <message>
        <source>Repositories</source>
        <translation>Repositories</translation>
    </message>
    <message>
        <source>List of plugin repositories</source>
        <translation>Lijst van pluginrepositories</translation>
    </message>
    <message>
        <source>URL</source>
        <translation>URL</translation>
    </message>
    <message>
        <source>Allow the Installer to look for updates and news in enabled repositories on QGIS startup</source>
        <translation>Toestaan dat de Installer kijkt of er updates of niews in bij de werkzame repositories bij het opstarten van QGIS</translation>
    </message>
    <message>
        <source>Check for updates on startup</source>
        <translation>Controleer op updates bij het opstarten</translation>
    </message>
    <message>
        <source>Add third party plugin repositories to the list</source>
        <translation>Voeg derdepartij pluginrepositories toe aan de lijst</translation>
    </message>
    <message>
        <source>Add 3rd party repositories</source>
        <translation>Toevoegen 3de partijrepositories</translation>
    </message>
    <message>
        <source>Add a new plugin repository</source>
        <translation>Toevoegen nieuwe pluginrepository</translation>
    </message>
    <message>
        <source>Add...</source>
        <translation>Toevoegen...</translation>
    </message>
    <message>
        <source>Edit the selected repository</source>
        <translation>Geselecteerde repository aanpassen </translation>
    </message>
    <message>
        <source>Edit...</source>
        <translation>Bewerken...</translation>
    </message>
    <message>
        <source>Remove the selected repository</source>
        <translation>Geselecteerde repository verwijderen</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>Verwijderen</translation>
    </message>
    <message>
        <source>The plugins will be installed to ~/.qgis/python/plugins</source>
        <translation>De plugins zullen worden geinstalleerd in  ~/.qgis/python/plugins</translation>
    </message>
    <message>
        <source>Close the Installer window</source>
        <translation>Installeerscherm sluiten</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Sluiten</translation>
    </message>
</context>
<context>
    <name>QgsPluginInstallerFetchingDialog</name>
    <message>
        <source>Success</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Resolving host name...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Connecting...</source>
        <translation type="unfinished">Verbinden...</translation>
    </message>
    <message>
        <source>Host connected. Sending request...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Downloading data...</source>
        <translation type="unfinished">Data wordt gedownload...</translation>
    </message>
    <message>
        <source>Idle</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Closing connection...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Error</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsPluginInstallerFetchingDialogBase</name>
    <message>
        <source>Fetching repositories</source>
        <translation>Repositories worden opgehaald</translation>
    </message>
    <message>
        <source>Overall progress:</source>
        <translation>Totale voortgang:</translation>
    </message>
    <message>
        <source>Abort fetching</source>
        <translation>Ophalen afgebroken</translation>
    </message>
    <message>
        <source>Repository</source>
        <translation>Repository</translation>
    </message>
    <message>
        <source>State</source>
        <translation>Status</translation>
    </message>
</context>
<context>
    <name>QgsPluginInstallerInstallingDialog</name>
    <message>
        <source>Installing...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Resolving host name...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Connecting...</source>
        <translation type="unfinished">Verbinden...</translation>
    </message>
    <message>
        <source>Host connected. Sending request...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Downloading data...</source>
        <translation type="unfinished">Data wordt gedownload...</translation>
    </message>
    <message>
        <source>Idle</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Closing connection...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Failed to unzip the plugin package. Probably it&apos;s broken or missing from the repository. You may also want to make sure that you have write permission to the plugin directory:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Aborted by user</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsPluginInstallerInstallingDialogBase</name>
    <message>
        <source>QGIS Python Plugin Installer</source>
        <translation>QGIS Plugin Installeren</translation>
    </message>
    <message>
        <source>Installing plugin:</source>
        <translation>Installeer plugin:</translation>
    </message>
    <message>
        <source>Connecting...</source>
        <translation>Verbinden...</translation>
    </message>
</context>
<context>
    <name>QgsPluginInstallerPluginErrorDialog</name>
    <message>
        <source>no error message received</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsPluginInstallerPluginErrorDialogBase</name>
    <message>
        <source>Error loading plugin</source>
        <translation>Fout bij laden van plugin</translation>
    </message>
    <message>
        <source>The plugin seems to be invalid or have unfulfilled dependencies. It has been installed, but can&apos;t be loaded. If you really need this plugin, you can contact its author or &lt;a href=&quot;http://lists.osgeo.org/mailman/listinfo/qgis-user&quot;&gt;QGIS users group&lt;/a&gt; and try to solve the problem. If not, you can just uninstall it. Here is the error message below:</source>
        <translation>De plugin heeft (software)afhankelijkheden. De plugin is geïnstalleerd, maar kan niet worden geladen. Als u deze plugin echt wilt gebruiken kunt u contact op nemen met de auteur, of  &lt;a href=&quot;http://lists.osgeo.org/mailman/listinfo/qgis-user&quot;&gt;de QGIS gebruikerslijst (engelstalig)&lt;/a&gt;  en zo proberen dit probleem op te lossen. Eventueel kunt u het gewoon deinstalleren. Dit is de foutmelding: </translation>
    </message>
    <message>
        <source>Do you want to uninstall this plugin now? If you&apos;re unsure, probably you would like to do this.</source>
        <translation>Wilt u deze plugin nu deinstalleren? Indien u twijfelt, is dit waarschijnlijk de juiste keus.</translation>
    </message>
</context>
<context>
    <name>QgsPluginInstallerRepositoryDetailsDialogBase</name>
    <message>
        <source>Repository details</source>
        <translation>Repository details</translation>
    </message>
    <message>
        <source>Name:</source>
        <translation>Naam:</translation>
    </message>
    <message>
        <source>Enter a name for the repository</source>
        <translation>Geef een naam voor deze repository</translation>
    </message>
    <message>
        <source>URL:</source>
        <translation>URL:</translation>
    </message>
    <message>
        <source>Enter the repository URL, beginning with &quot;http://&quot;</source>
        <translation>Geef de URL van de repositroy, beginnend met &quot;http://&quot;</translation>
    </message>
    <message>
        <source>Enable or disable the repository (disabled repositories will be omitted)</source>
        <translation>Wel of niet werkzaam maken van deze repository (niet werkzame worden genegeerd)</translation>
    </message>
    <message>
        <source>Enabled</source>
        <translation>Werkzaam</translation>
    </message>
    <message>
        <source>[place for a warning message]</source>
        <translation type="obsolete">[plaats voor een waarschuwing]</translation>
    </message>
</context>
<context>
    <name>QgsPluginManager</name>
    <message>
        <source>Description</source>
        <translation type="obsolete">Omschrijving</translation>
    </message>
    <message>
        <source>Name</source>
        <translation type="obsolete">Naam</translation>
    </message>
    <message>
        <source>No Plugins</source>
        <translation>Geen Plugins</translation>
    </message>
    <message>
        <source>No QGIS plugins found in </source>
        <translation>Geen QGIS-plugins gevonden in</translation>
    </message>
    <message>
        <source>Version</source>
        <translation type="obsolete">Versie</translation>
    </message>
    <message>
        <source>&amp;Select All</source>
        <translation>Alles &amp;Selecteren</translation>
    </message>
    <message>
        <source>&amp;Clear All</source>
        <translation>Alles &amp;Deselecteren</translation>
    </message>
    <message>
        <source>[ incompatible ]</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsPluginManagerBase</name>
    <message>
        <source>Alt+C</source>
        <translation type="obsolete">Alt+C</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation type="obsolete">Sl&amp;uiten</translation>
    </message>
    <message>
        <source>QGIS Plugin Manager</source>
        <translation>QGIS Plugin Manager</translation>
    </message>
    <message>
        <source>To enable / disable a plugin, click its checkbox or description</source>
        <translation>Om een plugin in- of uit te schakelen, klik op het aanvinkvakje of de beschrijving</translation>
    </message>
    <message>
        <source>&amp;Filter</source>
        <translation>&amp;Filter</translation>
    </message>
    <message>
        <source>Plugin Directory:</source>
        <translation>Pluginmap</translation>
    </message>
    <message>
        <source>Directory</source>
        <translation>Map</translation>
    </message>
</context>
<context>
    <name>QgsPointDialog</name>
    <message>
        <source>.</source>
        <translation>.</translation>
    </message>
    <message>
        <source>Add Point</source>
        <translation>Toevoegen Punt</translation>
    </message>
    <message>
        <source>Affine</source>
        <translation>Affiene</translation>
    </message>
    <message>
        <source>Capture Points</source>
        <translation>Punten Intekenen</translation>
    </message>
    <message>
        <source>Choose a name for the world file</source>
        <translation>Kies een naam voor de &apos;world file&apos;</translation>
    </message>
    <message>
        <source>Could not write to </source>
        <translation>Kan niet schrijven naar </translation>
    </message>
    <message>
        <source>Currently all modified files will be written in TIFF format.</source>
        <translation>Alle aangepaste bestanden zullen worden weggeschreven in TIFF-formaat.</translation>
    </message>
    <message>
        <source>Delete Point</source>
        <translation>Punt verwijderen</translation>
    </message>
    <message>
        <source>Delete Selected</source>
        <translation>Geselecteerd Object(en) Verwijderen</translation>
    </message>
    <message>
        <source>Error</source>
        <translation>Fout</translation>
    </message>
    <message>
        <source>Helmert</source>
        <translation>Helmert</translation>
    </message>
    <message>
        <source>Linear</source>
        <translation>Lineair</translation>
    </message>
    <message>
        <source>-modified</source>
        <comment>Georeferencer:QgsPointDialog.cpp - used to modify a user given filename</comment>
        <translation type="obsolete">-aangepast</translation>
    </message>
    <message>
        <source>Not implemented!</source>
        <translation>Niet geïmplementeerd!</translation>
    </message>
    <message>
        <source>&lt;p&gt;A Helmert transform requires modifications in the raster layer.&lt;/p&gt;&lt;p&gt;The modified raster will be saved in a new file and a world file will be generated for this new file instead.&lt;/p&gt;&lt;p&gt;Are you sure that this is what you want?&lt;/p&gt;</source>
        <translation>&lt;p&gt;Een Helmert transformatie vereist modificatie in te rasterlaag..&lt;/p&gt;&lt;p&gt;Het aangepaste raster zal worden opgeslagen in een nieuw bestand met bijbehorende &apos;world file&apos;.&lt;/p&gt;&lt;p&gt;Weet u zeker dat dit is wat u wilt?&lt;/p&gt;</translation>
    </message>
    <message>
        <source>&lt;p&gt;An affine transform requires changing the original raster file. This is not yet supported.&lt;/p&gt;</source>
        <translation>&lt;p&gt;Een affiene transformatie vereist aanpassing van het originele rasterbestand. Dit wordt nog niet ondersteund.&lt;/p&gt;</translation>
    </message>
    <message>
        <source>Pan Map</source>
        <translation>Kaart verschuiven</translation>
    </message>
    <message>
        <source>Pan the map</source>
        <translation>Verschuif de kaart</translation>
    </message>
    <message>
        <source>&lt;p&gt;The </source>
        <translation>&lt;p&gt;De</translation>
    </message>
    <message>
        <source> transform is not yet supported.&lt;/p&gt;</source>
        <translation> transformatie wordt nog niet ondersteund.&lt;/p&gt;</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Waarschuwing</translation>
    </message>
    <message>
        <source>z</source>
        <translation>z</translation>
    </message>
    <message>
        <source>Z</source>
        <translation>Z</translation>
    </message>
    <message>
        <source>Zoom In</source>
        <translation>Inzoomen</translation>
    </message>
    <message>
        <source>Zoom Out</source>
        <translation>Uitzoomen</translation>
    </message>
    <message>
        <source>Zoom to Layer</source>
        <translation>Op Kaartlaag Inzoomen</translation>
    </message>
    <message>
        <source>Zoom To Layer</source>
        <translation>Op Kaartlaag Inzoomen</translation>
    </message>
    <message>
        <source>-modified</source>
        <comment>Georeferencer:QgsPointDialog.cpp - used to modify a user given file name</comment>
        <translation>-aangepast</translation>
    </message>
</context>
<context>
    <name>QgsPointDialogBase</name>
    <message>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <source>Add points</source>
        <translation>Punten toevoegen</translation>
    </message>
    <message>
        <source>Create</source>
        <translation>Aanmaken</translation>
    </message>
    <message>
        <source>Create and load layer</source>
        <translation>Laag aanmaken en laden</translation>
    </message>
    <message>
        <source>Delete points</source>
        <translation>Punten verwijderen</translation>
    </message>
    <message>
        <source>Modified raster:</source>
        <translation>Aangepast raster:</translation>
    </message>
    <message>
        <source>Pan</source>
        <translation>Schuiven</translation>
    </message>
    <message>
        <source>Reference points</source>
        <translation>Referentiepunten</translation>
    </message>
    <message>
        <source>Transform type:</source>
        <translation>Type transformatie:</translation>
    </message>
    <message>
        <source>World file:</source>
        <translation>&apos;World file&apos;:</translation>
    </message>
    <message>
        <source>Zoom in</source>
        <translation>Inzoomen</translation>
    </message>
    <message>
        <source>Zoom out</source>
        <translation>Uitzoomen</translation>
    </message>
    <message>
        <source>Zoom to the raster extents</source>
        <translation>Zoom naar het extent van het raster</translation>
    </message>
</context>
<context>
    <name>QgsPointStyleWidgetBase</name>
    <message>
        <source>Form3</source>
        <translation type="obsolete">Form3</translation>
    </message>
    <message>
        <source>Symbol Style</source>
        <translation type="obsolete">Symboolstijl</translation>
    </message>
    <message>
        <source>Scale</source>
        <translation type="obsolete">Schaal </translation>
    </message>
</context>
<context>
    <name>QgsPostgresProvider</name>
    <message>
        <source>and </source>
        <translation>en</translation>
    </message>
    <message>
        <source> and does not have a suitable constraint)</source>
        <translation>en heeft geen bruikbare begrenzing)</translation>
    </message>
    <message>
        <source> and has a suitable constraint)</source>
        <translation>en heeft een bruikbare begrenzing)</translation>
    </message>
    <message>
        <source>and is not suitable </source>
        <translation>en is niet bruikbaar</translation>
    </message>
    <message>
        <source>and is suitable.</source>
        <translation>en is bruikbaar</translation>
    </message>
    <message>
        <source>Column </source>
        <translation>Kolom</translation>
    </message>
    <message>
        <source> derives from </source>
        <translation>afgeleid van</translation>
    </message>
    <message>
        <source> has a geometry type of </source>
        <translation>heeft een als geomtrie-type</translation>
    </message>
    <message>
        <source>has no column suitable for use as a unique key.
</source>
        <translation>heeft geen als unieke sleutel bruikbare kolom.
</translation>
    </message>
    <message>
        <source> in </source>
        <translation>in</translation>
    </message>
    <message>
        <source>initially appeared suitable but does not contain unique data, so is not suitable.
</source>
        <translation>leek in eerste instantie bruikbaar, maar bevat geen unieke data, dus is onbruikbaar.
</translation>
    </message>
    <message>
        <source> is unsuitable because Qgis does not currently support multiple columns as a key into the table.
</source>
        <translation>is onbruikbaar omdat QGIS geen sleutel over meerdere kolommen ondersteund.</translation>
    </message>
    <message>
        <source>is unsuitable because Qgis does not currently support non-int4 type columns as a key into the table.
</source>
        <translation>is onbruikbaar omdat QGIS niet kan omgaan met niet-int4-type kolommen als sleutel in een tabel.</translation>
    </message>
    <message>
        <source>No GEOS Support!</source>
        <translation type="obsolete">Geen GEOS Ondersteuning!</translation>
    </message>
    <message>
        <source>No suitable key column in table</source>
        <translation>Geen bruikbare sleutelkolom in tabel</translation>
    </message>
    <message>
        <source>No suitable key column in view</source>
        <translation>Geen bruikbare sleutelkolom in view</translation>
    </message>
    <message>
        <source>Note: </source>
        <translation>Opmerking:</translation>
    </message>
    <message>
        <source>Qgis requires that the view has a column that can be used as a unique key. Such a column should be derived from a table column of type int4 and be a primary key, have a unique constraint on it, or be a PostgreSQL oid column. To improve performance the column should also be indexed.
</source>
        <translation>QGIS vereist dat een view een kolom heeft die als unieke sleutel kan dienen. Zo&apos;n kolom kan een afgeleide kolom zijn uit een tabel die primaire sleutel is en van het type int4 en een unieke contraint bevat, of een PostgreSQL oid-kolom. Om de performance te verhogen dient de kolom geindexeerd te zijn.</translation>
    </message>
    <message>
        <source> relation.
The error message from the database was:
</source>
        <translation> relatie.
De foutmelding van de database was:
</translation>
    </message>
    <message>
        <source>. The database communication log was:
</source>
        <translation>- De database-communicatie-log was:
</translation>
    </message>
    <message>
        <source>The table has no column suitable for use as a key.

Qgis requires that the table either has a column of type
int4 with a unique constraint on it (which includes the
primary key) or has a PostgreSQL oid column.
</source>
        <translation>De tabel heeft geen als sleutel te gebruiken kolom.

QGIS eist dat de tabel ofwel een kolom heeft van het
type int4 met een unieke constraint erop (een primaire
sleutel ook mogelijk) of heeft een PostgreSQL oid kolom.
</translation>
    </message>
    <message>
        <source>The unique index based on columns </source>
        <translation>De unieke index gebaseerd op kolommen</translation>
    </message>
    <message>
        <source>The unique index on column</source>
        <translation>De unieke index op kolom</translation>
    </message>
    <message>
        <source>The view </source>
        <translation>De view</translation>
    </message>
    <message>
        <source>The view you selected has the following columns, none of which satisfy the above conditions:</source>
        <translation>De geselecteerde view heeft de volgende kolommen, geen daarvan voldoet aan de gestelde condities:</translation>
    </message>
    <message>
        <source>type is </source>
        <translation>type is</translation>
    </message>
    <message>
        <source>Unable to access relation</source>
        <translation>Relatie kan niet benaderd worden</translation>
    </message>
    <message>
        <source>Unable to access the </source>
        <translation>Verbinden niet mogelijk met </translation>
    </message>
    <message>
        <source>Unable to find a key column</source>
        <translation>Geen sleutel-kolom gevonden</translation>
    </message>
    <message>
        <source>Unable to get feature type and srid</source>
        <translation>Geen objecttype en srid gevonden</translation>
    </message>
    <message>
        <source>Unknown geometry type</source>
        <translation>Onbekende geometrie-type</translation>
    </message>
    <message>
        <source>, which Qgis does not currently support.</source>
        <translation> welke QGIS op dit moment niet ondersteund.</translation>
    </message>
    <message>
        <source>Your PostGIS installation has no GEOS support.
Feature selection and identification will not work properly.
Please install PostGIS with GEOS support (http://geos.refractions.net)</source>
        <translation type="obsolete">Uw PostGIS installatie biedt geen GEOS ondersteuning. 
Selecteren en identificeren van objecten zal niet werken.
Installeer PostGIS met GEOS-ondersteuning (http://geos.refractions.net)</translation>
    </message>
    <message>
        <source>Unable to determine table access privileges for the </source>
        <translation>Geen tabeltoegangsrechten gevonden voor</translation>
    </message>
    <message>
        <source>Error while adding features</source>
        <translation>Fout bij het toevoegen van objecten</translation>
    </message>
    <message>
        <source>Error while deleting features</source>
        <translation>Fout bij het verwijderen van objecten</translation>
    </message>
    <message>
        <source>Error while adding attributes</source>
        <translation>Fout bij het toevoegen van attributen</translation>
    </message>
    <message>
        <source>Error while deleting attributes</source>
        <translation>Fout bij het verwijderen van attributen</translation>
    </message>
    <message>
        <source>Error while changing attributes</source>
        <translation>Fout bij het aanpassen van attributen</translation>
    </message>
    <message>
        <source>Error while changing geometry values</source>
        <translation>Fout bij het aanpassen van geometrie-waarden</translation>
    </message>
    <message>
        <source>Qgis was unable to determine the type and srid of column </source>
        <translation>QGIS kan geen type of srid van de kolom vaststellen</translation>
    </message>
    <message>
        <source>unexpected PostgreSQL error</source>
        <translation>onverwachte PostgreSQL-fout</translation>
    </message>
</context>
<context>
    <name>QgsPostgresProvider::Conn</name>
    <message>
        <source>No GEOS Support!</source>
        <translation>Geen GEOS Ondersteuning!</translation>
    </message>
    <message>
        <source>Your PostGIS installation has no GEOS support.
Feature selection and identification will not work properly.
Please install PostGIS with GEOS support (http://geos.refractions.net)</source>
        <translation>Uw PostGIS installatie biedt geen GEOS ondersteuning. 
Selecteren en identificeren van objecten zal niet werken.
Installeer PostGIS met GEOS-ondersteuning (http://geos.refractions.net)</translation>
    </message>
</context>
<context>
    <name>QgsProjectPropertiesBase</name>
    <message>
        <source>Automatic</source>
        <translation>Automatisch</translation>
    </message>
    <message>
        <source>Automatically sets the number of decimal places in the mouse position display</source>
        <translation>Het aantal decimalen in de muispositie-vertoning automatisch bepalen</translation>
    </message>
    <message>
        <source>Avoid intersections of new polygons</source>
        <translation>Intersecties van nieuwe polygonen vermijden</translation>
    </message>
    <message>
        <source>Decimal degrees</source>
        <translation>Decimale graden</translation>
    </message>
    <message>
        <source>decimal places</source>
        <translation>Decimale posities</translation>
    </message>
    <message>
        <source>Default project title</source>
        <translation>Standaard projectbestand</translation>
    </message>
    <message>
        <source>Descriptive project name</source>
        <translation>Beschrijvende projectnaam</translation>
    </message>
    <message>
        <source>Digitizing</source>
        <translation>Digitaliseren</translation>
    </message>
    <message>
        <source>Enable on the fly projection</source>
        <translation type="obsolete">&apos;On the fly&apos;-projectie aanzetten</translation>
    </message>
    <message>
        <source>Enable topological editing</source>
        <translation>&apos;Topologisch aanpassen&apos; aanzetten</translation>
    </message>
    <message>
        <source>Feet</source>
        <translation>Voet</translation>
    </message>
    <message>
        <source>General</source>
        <translation>Algemeen</translation>
    </message>
    <message>
        <source>Manual</source>
        <translation>Handmatig</translation>
    </message>
    <message>
        <source>Meters</source>
        <translation>Meters</translation>
    </message>
    <message>
        <source>Precision</source>
        <translation>Precisie</translation>
    </message>
    <message>
        <source>Projection</source>
        <translation type="obsolete">Projectie</translation>
    </message>
    <message>
        <source>Project Properties</source>
        <translation>Projectinstellingen</translation>
    </message>
    <message>
        <source>Sets the number of decimal places to use for the mouse position display</source>
        <translation>Bepaald het aantal decimalen bij de muispositie-vertoning</translation>
    </message>
    <message>
        <source>Snapping options...</source>
        <translation>&apos;Snapping&apos;-optie...</translation>
    </message>
    <message>
        <source>The number of decimal places for the manual option</source>
        <translation>Het aantal decimalen bij de handmatige optie</translation>
    </message>
    <message>
        <source>The number of decimal places that are used when displaying the mouse position is automatically set to be enough so that moving the mouse by one pixel gives a change in the position display</source>
        <translation>Het aantal decimalen om de muispositie te toen wordt automatisch ingesteld op die waarde dat een verschuiving van de muis met 1 pixel ook een verandering in de muispositie toont</translation>
    </message>
    <message>
        <source>Title and colors</source>
        <translation>Titel en kleuren</translation>
    </message>
    <message>
        <source>Project title</source>
        <translation>Project titel</translation>
    </message>
    <message>
        <source>Selection color</source>
        <translation>Selectie-kleur</translation>
    </message>
    <message>
        <source>Background color</source>
        <translation>Achtergrondkleur</translation>
    </message>
    <message>
        <source>Map units</source>
        <translation>Kaarteenheden</translation>
    </message>
    <message>
        <source>Coordinate Reference System (CRS)</source>
        <translation>Ruimtelijk Referentie Systeem (CRS)</translation>
    </message>
    <message>
        <source>Enable &apos;on the fly&apos; CRS transformation</source>
        <translation>Gelijktijdige CRS-transformatie gebruiken</translation>
    </message>
</context>
<context>
    <name>QgsProjectionSelector</name>
    <message>
        <source>PostGIS SRID: </source>
        <translation type="obsolete">PostGIS SRID: </translation>
    </message>
    <message>
        <source>QGIS SRSID: </source>
        <translation type="obsolete">QGIS SRSID: </translation>
    </message>
    <message>
        <source>User Defined Coordinate Systems</source>
        <translation>Door gebruiker aangemaakt Ruimtelijk Referentie Systeem</translation>
    </message>
    <message>
        <source>Geographic Coordinate Systems</source>
        <translation>Geografisch Coördinaatsysteem</translation>
    </message>
    <message>
        <source>Projected Coordinate Systems</source>
        <translation>Geprojecteerd Coördinaatsysteem</translation>
    </message>
    <message>
        <source>Resource Location Error</source>
        <translation>Probleem met bronlocatie</translation>
    </message>
    <message>
        <source>Error reading database file from: 
 %1
Because of this the projection selector will not work...</source>
        <translation>Fout bij het lezen van het databasebestand: 
 %1
Hierdoor zal het kiezen van een projectie niet werken...</translation>
    </message>
</context>
<context>
    <name>QgsProjectionSelectorBase</name>
    <message>
        <source>EPSG ID</source>
        <translation>EPSG ID</translation>
    </message>
    <message>
        <source>Find</source>
        <translation>Zoeken</translation>
    </message>
    <message>
        <source>Id</source>
        <translation type="obsolete">Id</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Naam</translation>
    </message>
    <message>
        <source>Postgis SRID</source>
        <translation type="obsolete">Postgis SRID</translation>
    </message>
    <message>
        <source>Projection</source>
        <translation type="obsolete">Projectie</translation>
    </message>
    <message>
        <source>Projection Selector</source>
        <translation type="obsolete">Projectie Selector</translation>
    </message>
    <message>
        <source>QGIS SRSID</source>
        <translation type="obsolete">QGIS SRSID</translation>
    </message>
    <message>
        <source>Search</source>
        <translation>Zoek</translation>
    </message>
    <message>
        <source>Spatial Reference System</source>
        <translation type="obsolete">Ruimtelijk Referentie Systeem</translation>
    </message>
    <message>
        <source>Coordinate Reference System Selector</source>
        <translation>Ruimtelijk Referentie Systeem Keuze</translation>
    </message>
    <message>
        <source>Coordinate Reference System</source>
        <translation>Ruimtelijk Referentie Systeem</translation>
    </message>
    <message>
        <source>EPSG</source>
        <translation>EPSG</translation>
    </message>
    <message>
        <source>ID</source>
        <translation>ID</translation>
    </message>
</context>
<context>
    <name>QgsPythonDialog</name>
    <message>
        <source>&gt;&gt;&gt;</source>
        <translation>&gt;&gt;&gt;</translation>
    </message>
    <message>
        <source>Python console</source>
        <translation>Python console</translation>
    </message>
    <message>
        <source>To access Quantum GIS environment from this python console use object from global scope which is an instance of QgisInterface class.&lt;br&gt;Usage e.g.: iface.zoomFull()</source>
        <translation>Om de Quantum GIS omgeving te benaderen vanuit deze pythonconsole, gebruikt het object vanuit de globale scope welke een instantie is van de QgisInterface-klasse.&lt;br&gt;Usage e.g.: iface.zoomFull()</translation>
    </message>
</context>
<context>
    <name>QgsQuickPrint</name>
    <message>
        <source> cm</source>
        <translation> cm</translation>
    </message>
    <message>
        <source> degree</source>
        <translation> graad</translation>
    </message>
    <message>
        <source> degrees</source>
        <translation>graden</translation>
    </message>
    <message>
        <source> feet</source>
        <translation> voet</translation>
    </message>
    <message>
        <source> foot</source>
        <translation> voet</translation>
    </message>
    <message>
        <source> inches</source>
        <translation>inch</translation>
    </message>
    <message>
        <source> km</source>
        <translation> km</translation>
    </message>
    <message>
        <source> m</source>
        <translation> m</translation>
    </message>
    <message>
        <source> mile</source>
        <translation>mijl</translation>
    </message>
    <message>
        <source> miles</source>
        <translation>mijlen</translation>
    </message>
    <message>
        <source> mm</source>
        <translation> mm</translation>
    </message>
    <message>
        <source> unknown</source>
        <translation> onbekend</translation>
    </message>
</context>
<context>
    <name>QgsRasterLayer</name>
    <message>
        <source>and all other files</source>
        <translation>en alle andere bestanden</translation>
    </message>
    <message>
        <source>Average</source>
        <translation>Gemiddelde</translation>
    </message>
    <message>
        <source>Average Magphase</source>
        <translation>Gemiddelde &apos;Magphase&apos;</translation>
    </message>
    <message>
        <source>Band</source>
        <translation>Band</translation>
    </message>
    <message>
        <source>Band No</source>
        <translation>Band Nr</translation>
    </message>
    <message>
        <source> Bands: </source>
        <translation>Banden:</translation>
    </message>
    <message>
        <source>Cell Count</source>
        <translation>Aantal Cellen</translation>
    </message>
    <message>
        <source>Clipped area: </source>
        <translation type="obsolete">Clipped area: </translation>
    </message>
    <message>
        <source>Could not determine raster data type.</source>
        <translation>Kon het rasterdatatype niet bepalen.</translation>
    </message>
    <message>
        <source>Dataset Description</source>
        <translation>Dataset Beschrijving</translation>
    </message>
    <message>
        <source>Data Type:</source>
        <translation>Data-Type</translation>
    </message>
    <message>
        <source>Dimensions:</source>
        <translation>Dimensies:</translation>
    </message>
    <message>
        <source>Driver:</source>
        <translation>Driver:</translation>
    </message>
    <message>
        <source>GDT_Byte - Eight bit unsigned integer</source>
        <translation>GDT_Byte - Eight bit unsigned integer</translation>
    </message>
    <message>
        <source>GDT_CFloat32 - Complex Float32 </source>
        <translation>GDT_CFloat32 - Complex Float32 </translation>
    </message>
    <message>
        <source>GDT_CFloat64 - Complex Float64 </source>
        <translation>GDT_CFloat64 - Complex Float64 </translation>
    </message>
    <message>
        <source>GDT_CInt16 - Complex Int16 </source>
        <translation>GDT_CInt16 - Complex Int16 </translation>
    </message>
    <message>
        <source>GDT_CInt32 - Complex Int32 </source>
        <translation>GDT_CInt32 - Complex Int32 </translation>
    </message>
    <message>
        <source>GDT_Float32 - Thirty two bit floating point </source>
        <translation>GDT_Float32 - Thirty two bit floating point </translation>
    </message>
    <message>
        <source>GDT_Float64 - Sixty four bit floating point </source>
        <translation>GDT_Float64 - Sixty four bit floating point </translation>
    </message>
    <message>
        <source>GDT_Int16 - Sixteen bit signed integer </source>
        <translation>GDT_Int16 - Sixteen bit signed integer </translation>
    </message>
    <message>
        <source>GDT_Int32 - Thirty two bit signed integer </source>
        <translation>GDT_Int32 - Thirty two bit signed integer </translation>
    </message>
    <message>
        <source>GDT_UInt16 - Sixteen bit unsigned integer </source>
        <translation>GDT_UInt16 - Sixteen bit unsigned integer </translation>
    </message>
    <message>
        <source>GDT_UInt32 - Thirty two bit unsigned integer </source>
        <translation>GDT_UInt32 - Thirty two bit unsigned integer </translation>
    </message>
    <message>
        <source>Layer Spatial Reference System: </source>
        <translation>Ruimtelijk Referentie Systeem:</translation>
    </message>
    <message>
        <source>Max Val</source>
        <translation>Max Waarde</translation>
    </message>
    <message>
        <source>Mean</source>
        <translation>Gemiddelde</translation>
    </message>
    <message>
        <source>Min Val</source>
        <translation>Min Waarde</translation>
    </message>
    <message>
        <source>No Data Value</source>
        <translation>Waarde voor &apos;geen data&apos;</translation>
    </message>
    <message>
        <source>NoDataValue not set</source>
        <translation>&apos;Geen data&apos;-waarde niet gezet</translation>
    </message>
    <message>
        <source>No Stats</source>
        <translation>Geen Statistieken</translation>
    </message>
    <message>
        <source>No stats collected yet</source>
        <translation>Nog geen statistieken verzameld</translation>
    </message>
    <message>
        <source>Not Set</source>
        <translation>Geen Set</translation>
    </message>
    <message>
        <source>null (no data)</source>
        <translation>null (geen data)</translation>
    </message>
    <message>
        <source>Origin:</source>
        <translation>Oorsprong:</translation>
    </message>
    <message>
        <source>out of extent</source>
        <translation>Buiten de extent</translation>
    </message>
    <message>
        <source>Pixel Size:</source>
        <translation>Pixelgrootte</translation>
    </message>
    <message>
        <source>Property</source>
        <translation type="obsolete">Eigenschap</translation>
    </message>
    <message>
        <source>Pyramid overviews:</source>
        <translation>Pyramide overzichten:</translation>
    </message>
    <message>
        <source>Range</source>
        <translation>Range</translation>
    </message>
    <message>
        <source>Raster Extent: </source>
        <translation type="obsolete">Raster Extent: </translation>
    </message>
    <message>
        <source>Standard Deviation</source>
        <translation>Standaard Afwijking</translation>
    </message>
    <message>
        <source>Sum of all cells</source>
        <translation>Som van alle cellen</translation>
    </message>
    <message>
        <source>Sum of squares</source>
        <translation>Sum of squares</translation>
    </message>
    <message>
        <source>Value</source>
        <translation type="obsolete">Waarde</translation>
    </message>
    <message>
        <source>X: </source>
        <translation>X: </translation>
    </message>
    <message>
        <source> Y: </source>
        <translation> Y: </translation>
    </message>
    <message>
        <source>Band %1</source>
        <translation>Band %1</translation>
    </message>
</context>
<context>
    <name>QgsRasterLayerProperties</name>
    <message>
        <source>Blue</source>
        <translation>Blauw</translation>
    </message>
    <message>
        <source>Building pyramid overviews is not supported on this type of raster.</source>
        <translation>Het aanmaken van piramide-overzichten wordt niet ondersteund voor dit type raster.</translation>
    </message>
    <message>
        <source>Building pyramids failed.</source>
        <translation>Aanmaken van piramide mislukt.</translation>
    </message>
    <message>
        <source>By creating lower resolution copies of the data (pyramids) performance can be considerably improved as QGIS selects the most suitable resolution to use depending on the level of zoom.</source>
        <translation>Door het aanmaken van lage-resolutie kopien van de data (piramiden) kan de performance worden verhoogd, aangezien QGIS dan de meest toepasselijke resolutie gebruikt bij een gegeven zoom-niveau.</translation>
    </message>
    <message>
        <source>Clip To MinMax</source>
        <translation>Clip naar MinMax</translation>
    </message>
    <message>
        <source>Color Ramp</source>
        <translation>Kleurverloop</translation>
    </message>
    <message>
        <source>Columns: </source>
        <translation>Kolommen:</translation>
    </message>
    <message>
        <source>Custom Colormap</source>
        <translation type="obsolete">Aangepaste Kleurpalet</translation>
    </message>
    <message>
        <source>Description</source>
        <translation>Omschrijving</translation>
    </message>
    <message>
        <source>Discrete</source>
        <translation>Discreet</translation>
    </message>
    <message>
        <source>Equal interval</source>
        <translation>Gelijke interval</translation>
    </message>
    <message>
        <source>Freak Out</source>
        <translation>Freak Out</translation>
    </message>
    <message>
        <source>Gray</source>
        <translation>Grijs</translation>
    </message>
    <message>
        <source>Grayscale</source>
        <translation>Grijstinten</translation>
    </message>
    <message>
        <source>Green</source>
        <translation>Groen</translation>
    </message>
    <message>
        <source>Import Error</source>
        <translation>Fout bij importeren</translation>
    </message>
    <message>
        <source>Indexed Value</source>
        <translation>Geindexeerde Waarde</translation>
    </message>
    <message>
        <source>Large resolution raster layers can slow navigation in QGIS.</source>
        <translation>Hoge resolutie rasterlagen kunnen langzaam navigeren in QGIS.</translation>
    </message>
    <message>
        <source>Linearly</source>
        <translation type="obsolete">Lineaire</translation>
    </message>
    <message>
        <source>n/a</source>
        <translation>Niet beschikbaar</translation>
    </message>
    <message>
        <source>No-Data Value: </source>
        <translation>Waarde voor &apos;geen data&apos;: </translation>
    </message>
    <message>
        <source>No-Data Value: Not Set</source>
        <translation>&apos;Geen data&apos;-waarde niet gezet</translation>
    </message>
    <message>
        <source>No Stretch</source>
        <translation>Geen &apos;Strech&apos;</translation>
    </message>
    <message>
        <source>Open file</source>
        <translation>Bestand Openen</translation>
    </message>
    <message>
        <source>Palette</source>
        <translation type="obsolete">Palet</translation>
    </message>
    <message>
        <source>Percent Transparent</source>
        <translation>Percentage Transparant</translation>
    </message>
    <message>
        <source>Please note that building pyramids could corrupt your image - always make a backup of your data first!</source>
        <translation type="obsolete">Let op: het aanmaken van piramides kan tot corrupte data leiden - maak altijd een reservekopie van uw data!</translation>
    </message>
    <message>
        <source>Please note that building pyramids may alter the original data file and once created they cannot be removed!</source>
        <translation type="obsolete">Let op: het aanmaken van piramides kan uw originele databestand veranderen, en na het aanmaken kan dit niet meer ongedaan worden gemaakt!</translation>
    </message>
    <message>
        <source>Pseudocolor</source>
        <translation>Pseudocolor</translation>
    </message>
    <message>
        <source>QGIS Generated Transparent Pixel Value Export File</source>
        <translation>Door QGIS Gegenereerde Transparante Pixel Waarde Export Bestand</translation>
    </message>
    <message>
        <source>Quantiles</source>
        <translation>Quantilen</translation>
    </message>
    <message>
        <source>Read access denied</source>
        <translation>Geen leesrechten</translation>
    </message>
    <message>
        <source>Read access denied. Adjust the file permissions and try again.

</source>
        <translation>Geen leesrechten. Pas de bestandspermissies aan en probeer opnieuw.

</translation>
    </message>
    <message>
        <source>Red</source>
        <translation>Rood</translation>
    </message>
    <message>
        <source>Rows: </source>
        <translation>Rijen:</translation>
    </message>
    <message>
        <source>Save file</source>
        <translation>Bestand opslaan</translation>
    </message>
    <message>
        <source>Stretch And Clip To MinMax</source>
        <translation>&apos;Stretch&apos; en &apos;Clip&apos; tot MinMax</translation>
    </message>
    <message>
        <source>Stretch To MinMax</source>
        <translation>&apos;Stretch&apos; tot MinMax</translation>
    </message>
    <message>
        <source>Textfile (*.txt)</source>
        <translation>Tekstbestand (*.txt)</translation>
    </message>
    <message>
        <source>The file was not writeable. Some formats can not be written to, only read. You can also try to check the permissions and then try again.</source>
        <translation type="obsolete">Het bestand was niet beschrijfbaar. Sommige formaten zijn alleen inleesbaar, niet schrijfbaar. U kunt ook de permissies controleren en opnieuw proberen.</translation>
    </message>
    <message>
        <source>The following lines contained errors

</source>
        <translation>De volgende regels bevatten fouten

</translation>
    </message>
    <message>
        <source>User Defined</source>
        <translation>Door gebruiker gedefinieërd</translation>
    </message>
    <message>
        <source>Write access denied</source>
        <translation>Schrijftoegang gewijgerd</translation>
    </message>
    <message>
        <source>Write access denied. Adjust the file permissions and try again.

</source>
        <translation>Schrijfrechten geweigerd. Pas de file-permissies aan en probeer opnieuw.

</translation>
    </message>
    <message>
        <source>You must have write access in the directory where the original data is stored to build pyramids.</source>
        <translation>U moet schrijfrechten hebben in de map waar de originele data stond om piramides aan te kunnen maken.</translation>
    </message>
    <message>
        <source>Not Set</source>
        <translation>Niet Gezet</translation>
    </message>
    <message>
        <source>Default Style</source>
        <translation>Standaard Stijl</translation>
    </message>
    <message>
        <source>QGIS Layer Style File (*.qml)</source>
        <translation>QGIS Laagstijlbestand (*.qml)</translation>
    </message>
    <message>
        <source>QGIS</source>
        <translation>QGIS</translation>
    </message>
    <message>
        <source>Unknown style format: </source>
        <translation>Onbekend stijlformaat: </translation>
    </message>
    <message>
        <source>Colormap</source>
        <translation>Kleurenkaart</translation>
    </message>
    <message>
        <source>Linear</source>
        <translation>Lineair</translation>
    </message>
    <message>
        <source>Exact</source>
        <translation>Exact</translation>
    </message>
    <message>
        <source>Please note that building internal pyramids may alter the original data file and once created they cannot be removed!</source>
        <translation>Let op dat het aanmaken van interne (overzichts)pyramides het originele databestand kan veranderen een eenmaal aangemaakt niet meer kan worden verwijderd!</translation>
    </message>
    <message>
        <source>Please note that building internal pyramids could corrupt your image - always make a backup of your data first!</source>
        <translation>Let op dat het aanmaken van interne (overzichts)pyramides uw data kan vernielen - maak eerst een backup van uw data!</translation>
    </message>
    <message>
        <source>Default</source>
        <translation>Standaard</translation>
    </message>
    <message>
        <source>The file was not writeable. Some formats do not support pyramid overviews. Consult the GDAL documentation if in doubt.</source>
        <translation>Het bestand kon niet worden weggeschreven. Sommige formaten ondersteunen geen pyramideoverzichten. Raadpleet de GDAL-documentatie als u twijfelt.</translation>
    </message>
    <message>
        <source>Custom color map entry</source>
        <translation>Aangepaste Kleurpalet</translation>
    </message>
    <message>
        <source>QGIS Generated Color Map Export File</source>
        <translation>QGIS Gegenereerd Kleurenpalet ExportBestand</translation>
    </message>
    <message>
        <source>Load Color Map</source>
        <translation>Laad Kleurenpalet</translation>
    </message>
    <message>
        <source>The color map for Band %n failed to load</source>
        <translation type="obsolete">Het kleurenpalet voor Band %n kon niet worden geladen
        
        
        
        </translation>
    </message>
    <message>
        <source>Saved Style</source>
        <translation>Opgeslagen Stijl</translation>
    </message>
</context>
<context>
    <name>QgsRasterLayerPropertiesBase</name>
    <message>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <source> 00%</source>
        <translation> 00%</translation>
    </message>
    <message>
        <source>1</source>
        <translation>1</translation>
    </message>
    <message>
        <source>2</source>
        <translation>2</translation>
    </message>
    <message>
        <source>Average</source>
        <translation>Gemiddelde</translation>
    </message>
    <message>
        <source>Change</source>
        <translation type="obsolete">Aanpassen</translation>
    </message>
    <message>
        <source>Chart Type</source>
        <translation>Diagram Type</translation>
    </message>
    <message>
        <source>Classify</source>
        <translation>Classificeren</translation>
    </message>
    <message>
        <source>Colormap</source>
        <translation>Kleurenkaart</translation>
    </message>
    <message>
        <source>Columns:</source>
        <translation>Kolommen:</translation>
    </message>
    <message>
        <source>Delete entry</source>
        <translation>Verwider item</translation>
    </message>
    <message>
        <source>General</source>
        <translation>Algemeen</translation>
    </message>
    <message>
        <source>Histogram</source>
        <translation>Histogram</translation>
    </message>
    <message>
        <source>Legend:</source>
        <translation type="obsolete">Legenda:</translation>
    </message>
    <message>
        <source>Max</source>
        <translation>Max</translation>
    </message>
    <message>
        <source>Maximum scale at which this layer will be displayed. </source>
        <translation>Maximale schaal bij welke deze laag nog getoond zal worden. </translation>
    </message>
    <message>
        <source>Metadata</source>
        <translation>Metadata</translation>
    </message>
    <message>
        <source>Min</source>
        <translation>Min</translation>
    </message>
    <message>
        <source>Minimum scale at which this layer will be displayed. </source>
        <translation>Minimale schaal bij welke deze laag nog getoond zal worden. </translation>
    </message>
    <message>
        <source>Nearest Neighbour</source>
        <translation>Nearest neighbour</translation>
    </message>
    <message>
        <source>No Data:</source>
        <translation>Geen Data:</translation>
    </message>
    <message>
        <source>None</source>
        <translation>Geen</translation>
    </message>
    <message>
        <source>Options</source>
        <translation>Opties</translation>
    </message>
    <message>
        <source>Palette:</source>
        <translation type="obsolete">Palet:</translation>
    </message>
    <message>
        <source>&lt;p align=&quot;right&quot;&gt;Full&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;right&quot;&gt;Vol&lt;/p&gt;</translation>
    </message>
    <message>
        <source>Pyramids</source>
        <translation>Pyramiden</translation>
    </message>
    <message>
        <source>Raster Layer Properties</source>
        <translation>Rasterlaag Eigenschappen</translation>
    </message>
    <message>
        <source>Refresh</source>
        <translation>Bijwerken</translation>
    </message>
    <message>
        <source>Render as</source>
        <translation>Renderen als</translation>
    </message>
    <message>
        <source>Rows:</source>
        <translation>Rijen:</translation>
    </message>
    <message>
        <source>Symbology</source>
        <translation>Symbologie</translation>
    </message>
    <message>
        <source>Thumbnail</source>
        <translation>Thumbnail</translation>
    </message>
    <message>
        <source>Single band gray</source>
        <translation>Enkelbands grijs</translation>
    </message>
    <message>
        <source>Three band color</source>
        <translation>Driebands kleur</translation>
    </message>
    <message>
        <source>RGB mode band selection and scaling</source>
        <translation>RGB-modes band selectie en schalen</translation>
    </message>
    <message>
        <source>Red band</source>
        <translation>Rode band</translation>
    </message>
    <message>
        <source>Green band</source>
        <translation>Groene band</translation>
    </message>
    <message>
        <source>Blue band</source>
        <translation>Blauwe band</translation>
    </message>
    <message>
        <source>Custom min / max values</source>
        <translation>Aangepaste min / max waarden</translation>
    </message>
    <message>
        <source>Red min</source>
        <translation>Rood min</translation>
    </message>
    <message>
        <source>Red max</source>
        <translation>Rood max</translation>
    </message>
    <message>
        <source>Green min</source>
        <translation>Groen min</translation>
    </message>
    <message>
        <source>Green max</source>
        <translation>Groen max</translation>
    </message>
    <message>
        <source>Blue min</source>
        <translation>Blauw min</translation>
    </message>
    <message>
        <source>Blue max</source>
        <translation>Blauw max</translation>
    </message>
    <message>
        <source>Std. deviation</source>
        <translation type="obsolete">Std. afwijking</translation>
    </message>
    <message>
        <source>Single band properties</source>
        <translation>Enkelbandss eigenschappen</translation>
    </message>
    <message>
        <source>Gray band</source>
        <translation>Grijze band</translation>
    </message>
    <message>
        <source>Color map</source>
        <translation>Kleurenpalet</translation>
    </message>
    <message>
        <source>Invert color map</source>
        <translation>Inverteer kleurenpalet</translation>
    </message>
    <message>
        <source>Use standard deviation</source>
        <translation>Gebruik standaard afwijking</translation>
    </message>
    <message>
        <source>Note:</source>
        <translation>Opmerking:</translation>
    </message>
    <message>
        <source>Load min / max values from band</source>
        <translation>Laden min- / maxwaarden van band</translation>
    </message>
    <message>
        <source>Estimate (faster)</source>
        <translation>Schatten (sneller)</translation>
    </message>
    <message>
        <source>Actual (slower)</source>
        <translation>Actuele (langzamer)</translation>
    </message>
    <message>
        <source>Load</source>
        <translation>Laden</translation>
    </message>
    <message>
        <source>Contrast enhancement</source>
        <translation>Contrastverhoging</translation>
    </message>
    <message>
        <source>Current</source>
        <translation>Huidige</translation>
    </message>
    <message>
        <source>Save current contrast enhancement algorithm as default. This setting will be persistent between QGIS sessions.</source>
        <translation>Sla het huidige algorithme voor contrastverhoging op als standaard. Deze instelling geld dan voor alle QGIS-sessies.</translation>
    </message>
    <message>
        <source>Saves current contrast enhancement algorithm as a default. This setting will be persistent between QGIS sessions.</source>
        <translation>Slaat het huidige algorithme voor contrastverhoging op als standaard. Deze instelling geld dan voor alle QGIS-sessies.</translation>
    </message>
    <message>
        <source>Default</source>
        <translation>Standaard</translation>
    </message>
    <message>
        <source>TextLabel</source>
        <translation>Tekstlabel</translation>
    </message>
    <message>
        <source>Transparency</source>
        <translation>Transparantie</translation>
    </message>
    <message>
        <source>Global transparency</source>
        <translation>Globale transparantie</translation>
    </message>
    <message>
        <source>No data value</source>
        <translation>Waarde voor &apos;geen data&apos;</translation>
    </message>
    <message>
        <source>Reset no data value</source>
        <translation>Herstel &apos;waarde voor geen data&apos;</translation>
    </message>
    <message>
        <source>Custom transparency options</source>
        <translation>Angepaste transparantie-opties</translation>
    </message>
    <message>
        <source>Transparency band</source>
        <translation>Transparantieband</translation>
    </message>
    <message>
        <source>Transparent pixel list</source>
        <translation>Transparantie pixellijst</translation>
    </message>
    <message>
        <source>Add values manually</source>
        <translation>Voeg handmatig waarden in</translation>
    </message>
    <message>
        <source>Add Values from display</source>
        <translation>Waarden gebruiken uit de kaart</translation>
    </message>
    <message>
        <source>Remove selected row</source>
        <translation>Verwijder geselecteerde rij</translation>
    </message>
    <message>
        <source>Default values</source>
        <translation>Standaard waarden</translation>
    </message>
    <message>
        <source>Import from file</source>
        <translation>Van bestand importeren</translation>
    </message>
    <message>
        <source>Export to file</source>
        <translation>Naar bestand exporteren</translation>
    </message>
    <message>
        <source>Number of entries</source>
        <translation>Aantal entries</translation>
    </message>
    <message>
        <source>Color interpolation</source>
        <translation>Kleurinterpolatie</translation>
    </message>
    <message>
        <source>Classification mode</source>
        <translation>Clasdificatie modus</translation>
    </message>
    <message>
        <source>Spatial reference system</source>
        <translation type="obsolete">Ruimtelijk Referentie Systeem</translation>
    </message>
    <message>
        <source>Scale dependent visibility</source>
        <translation>Schaalafhankelijke zichtbaarheid</translation>
    </message>
    <message>
        <source>Maximum</source>
        <translation>Maximum</translation>
    </message>
    <message>
        <source>Minimum</source>
        <translation>Minimum</translation>
    </message>
    <message>
        <source>Show debug info</source>
        <translation type="obsolete">Debug-informatie tonen</translation>
    </message>
    <message>
        <source>Layer source</source>
        <translation>Laagbron</translation>
    </message>
    <message>
        <source>Display name</source>
        <translation>Toon naam</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Pyramid resolutions</source>
        <translation>Piramide resoluties</translation>
    </message>
    <message>
        <source>Resampling method</source>
        <translation>Hersampling methode</translation>
    </message>
    <message>
        <source>Build pyramids</source>
        <translation>Piramdie bouwen</translation>
    </message>
    <message>
        <source>Line graph</source>
        <translation>Lijndiagram</translation>
    </message>
    <message>
        <source>Bar chart</source>
        <translation>Staafdiagram</translation>
    </message>
    <message>
        <source>Column count</source>
        <translation>Aantal kolommen</translation>
    </message>
    <message>
        <source>Out of range OK?</source>
        <translation>Buiten waardengebied OK?</translation>
    </message>
    <message>
        <source>Allow approximation</source>
        <translation>Toestaan van benaderingen</translation>
    </message>
    <message>
        <source>Restore Default Style</source>
        <translation>Terug naar Standaard Stijl</translation>
    </message>
    <message>
        <source>Save As Default</source>
        <translation>Opslaan Als Standaard</translation>
    </message>
    <message>
        <source>Load Style ...</source>
        <translation>Stijl laden ...</translation>
    </message>
    <message>
        <source>Save Style ...</source>
        <translation>Stijl Oplaan ...</translation>
    </message>
    <message>
        <source>Default R:1 G:2 B:3</source>
        <translation>Standaard R:1 G:2 B:3</translation>
    </message>
    <message>
        <source>Add entry</source>
        <translation>Item toevoegen</translation>
    </message>
    <message>
        <source>Sort</source>
        <translation>Sorteren</translation>
    </message>
    <message>
        <source>Load color map from band</source>
        <translation>Laad kleurenpalet van band</translation>
    </message>
    <message>
        <source>Load color map from file</source>
        <translation>Laad kleurenpalet van bestand</translation>
    </message>
    <message>
        <source>Export color map to file</source>
        <translation>Exporteer kleurenpalet naar bestand</translation>
    </message>
    <message>
        <source>Generate new color map</source>
        <translation>Genereer nieuw kleurenpalet</translation>
    </message>
    <message>
        <source>Coordinate reference system</source>
        <translation>Ruimtelijk referentie systeem</translation>
    </message>
    <message>
        <source>Change ...</source>
        <translation>Aanpassen ...</translation>
    </message>
    <message>
        <source>Legend</source>
        <translation>Legenda</translation>
    </message>
    <message>
        <source>Palette</source>
        <translation>Palet</translation>
    </message>
    <message>
        <source>Notes</source>
        <translation>Opmerkingen</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;p, li { white-space: pre-wrap; }&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Build pyramids internally if possible</source>
        <translation>Interne pyramiden aanmaken indien mogelijk</translation>
    </message>
    <message>
        <source>&lt;!DOCTYPE HTML PUBLIC &quot;-//W3C//DTD HTML 4.0//EN&quot; &quot;http://www.w3.org/TR/REC-html40/strict.dtd&quot;&gt;
&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;DejaVu Sans&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsRunProcess</name>
    <message>
        <source>Done</source>
        <translation>Gereed</translation>
    </message>
    <message>
        <source>Starting</source>
        <translation>Starten</translation>
    </message>
    <message>
        <source>Unable to run command</source>
        <translation>Het runne/afdraaien van het command lukt niet</translation>
    </message>
    <message>
        <source>Action</source>
        <translation>Actie</translation>
    </message>
</context>
<context>
    <name>QgsSOSPlugin</name>
    <message>
        <source>&amp;Add Sensor layer</source>
        <translation type="obsolete">&amp;Toevoegen Sensorlaag</translation>
    </message>
</context>
<context>
    <name>QgsSOSSourceSelect</name>
    <message>
        <source>Are you sure you want to remove the </source>
        <translation type="obsolete">Weet u zeker dat u dit wilt verwijderen</translation>
    </message>
    <message>
        <source> connection and all associated settings?</source>
        <translation type="obsolete"> verbinding en alle daarbij horende instellingen?</translation>
    </message>
    <message>
        <source>Confirm Delete</source>
        <translation type="obsolete">Bevestig Verwijderen</translation>
    </message>
    <message>
        <source>Offering not found</source>
        <translation type="obsolete">&apos;Offering&apos; niet gevonden</translation>
    </message>
    <message>
        <source>An problem occured adding the layer. The information about the selected offering could not be found</source>
        <translation type="obsolete">Probleem bij het toevoegen van de laag. De informatie voor de geselecteerde &apos;offering&apos; kan niet worden gevonden</translation>
    </message>
</context>
<context>
    <name>QgsSOSSourceSelectBase</name>
    <message>
        <source>Delete</source>
        <translation type="obsolete">Verwijderen</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation type="obsolete">Bewerken</translation>
    </message>
    <message>
        <source>Name</source>
        <translation type="obsolete">Naam</translation>
    </message>
    <message>
        <source>Id</source>
        <translation type="obsolete">Id</translation>
    </message>
    <message>
        <source>Dialog</source>
        <translation type="obsolete">Dialoog</translation>
    </message>
    <message>
        <source>Server Connections</source>
        <translation type="obsolete">Server-verbindingen</translation>
    </message>
    <message>
        <source>&amp;New</source>
        <translation type="obsolete">&amp;Nieuw</translation>
    </message>
    <message>
        <source>C&amp;onnect</source>
        <translation type="obsolete">Ver&amp;binden</translation>
    </message>
    <message>
        <source>Offerings</source>
        <translation type="obsolete">&apos;Offerings&apos;</translation>
    </message>
    <message>
        <source>Optional settings</source>
        <translation type="obsolete">Optionele voorkeuren</translation>
    </message>
    <message>
        <source>Observed properties...</source>
        <translation type="obsolete">Geobserveerde eigenschappen</translation>
    </message>
    <message>
        <source>Procedures...</source>
        <translation type="obsolete">Procedures...</translation>
    </message>
    <message>
        <source>Features of interest...</source>
        <translation type="obsolete">Gevolgde objecten</translation>
    </message>
</context>
<context>
    <name>QgsScaleBarPlugin</name>
    <message>
        <source>Bar</source>
        <translation>Balk</translation>
    </message>
    <message>
        <source>Bottom Left</source>
        <translation>LinksOnder</translation>
    </message>
    <message>
        <source>Bottom Right</source>
        <translation>RechtsOnder</translation>
    </message>
    <message>
        <source>Box</source>
        <translation>Box</translation>
    </message>
    <message>
        <source> cm</source>
        <translation> cm</translation>
    </message>
    <message>
        <source>Creates a scale bar that is displayed on the map canvas</source>
        <translation>Maakt een schaalbalk aan voor vertoning op de kaart</translation>
    </message>
    <message>
        <source>&amp;Decorations</source>
        <translation>&amp;Decoraties</translation>
    </message>
    <message>
        <source> degree</source>
        <translation> graad</translation>
    </message>
    <message>
        <source> degrees</source>
        <translation>graden</translation>
    </message>
    <message>
        <source> feet</source>
        <translation> voet</translation>
    </message>
    <message>
        <source> feet/miles</source>
        <translation>voet/mijlen</translation>
    </message>
    <message>
        <source> foot</source>
        <translation> voet</translation>
    </message>
    <message>
        <source> inches</source>
        <translation>inch</translation>
    </message>
    <message>
        <source> km</source>
        <translation> km</translation>
    </message>
    <message>
        <source> m</source>
        <translation> m</translation>
    </message>
    <message>
        <source> metres/km</source>
        <translation>meters/km</translation>
    </message>
    <message>
        <source> mile</source>
        <translation>mijl</translation>
    </message>
    <message>
        <source> miles</source>
        <translation>mijl</translation>
    </message>
    <message>
        <source> mm</source>
        <translation> mm</translation>
    </message>
    <message>
        <source>&amp;Scale Bar</source>
        <translation>&amp;Schaalbalk</translation>
    </message>
    <message>
        <source>Tick Down</source>
        <translation>Waardeaanduiding Boven</translation>
    </message>
    <message>
        <source>Tick Up</source>
        <translation>Waardeaanduiding Onder</translation>
    </message>
    <message>
        <source>Top Left</source>
        <translation>LinksBoven</translation>
    </message>
    <message>
        <source>Top Right</source>
        <translation>RechtsBoven</translation>
    </message>
    <message>
        <source> unknown</source>
        <translation> onbekend</translation>
    </message>
</context>
<context>
    <name>QgsScaleBarPluginGuiBase</name>
    <message>
        <source>Automatically snap to round number on resize</source>
        <translation>Klik automatisch naar gehele getallen tijdens het aanpassen van de grootte</translation>
    </message>
    <message>
        <source>Bar</source>
        <translation>Balk</translation>
    </message>
    <message>
        <source>Bottom Left</source>
        <translation>LinksOnder</translation>
    </message>
    <message>
        <source>Bottom Right</source>
        <translation>RechtsOnder</translation>
    </message>
    <message>
        <source>Box</source>
        <translation>Box</translation>
    </message>
    <message>
        <source>Click to select the colour</source>
        <translation>Klik om een kleur te selecteren</translation>
    </message>
    <message>
        <source>Colour of bar:</source>
        <translation>Kleur van de balk:</translation>
    </message>
    <message>
        <source>Enable scale bar</source>
        <translation>Schaalbalk gebruiken</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;This plugin draws a scale bar on the map. Please note the size option below is a &apos;preferred&apos; size and may have to be altered by QGIS depending on the level of zoom.  The size is measured according to the map units specified in the project properties.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Deze plugin tekend een schaalbalk op de kaart. Let op dat de &apos;grootte-optie&apos; hieronder een voorkeursgrootte is, en door QGIS kan worden aangepast afhankelijjk van het zoom-niveau. De grootte is gemeten volgens de kaarteenheden zoals die in de project-eigenschappen zijn ingesteld.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Placement:</source>
        <translation>Plaats:</translation>
    </message>
    <message>
        <source>Scale Bar Plugin</source>
        <translation>Schaalbalkplugin</translation>
    </message>
    <message>
        <source>Scale bar style:</source>
        <translation>Schaalbalkstijl</translation>
    </message>
    <message>
        <source>Select the style of the scale bar</source>
        <translation>Selecteer de stijl voor de schaalbalk</translation>
    </message>
    <message>
        <source>Size of bar:</source>
        <translation>Grootte van de balk:</translation>
    </message>
    <message>
        <source>Tick Down</source>
        <translation>Waardeaanduiding Boven</translation>
    </message>
    <message>
        <source>Tick Up</source>
        <translation>Waardeaanduiding Onder</translation>
    </message>
    <message>
        <source>Top Left</source>
        <translation>LinksBoven</translation>
    </message>
    <message>
        <source>Top Right</source>
        <translation>RechtsBoven</translation>
    </message>
</context>
<context>
    <name>QgsSearchQueryBuilder</name>
    <message>
        <source>Found %d matching features.</source>
        <translation type="obsolete"> %d passend object gevonden.
        
        
        
        </translation>
    </message>
    <message>
        <source>No matching features found.</source>
        <translation>Geen passende objecten gevonden.</translation>
    </message>
    <message>
        <source>No Records</source>
        <translation>Geen Records</translation>
    </message>
    <message>
        <source>Search query builder</source>
        <translation>Zoekquery bouwer</translation>
    </message>
    <message>
        <source>Search results</source>
        <translation>Zoek resultaten</translation>
    </message>
    <message>
        <source>Search string parsing error</source>
        <translation>Zoekstring parseerfout</translation>
    </message>
    <message>
        <source>The query you specified results in zero records being returned.</source>
        <translation>De gebruikte query levert geen records op.</translation>
    </message>
</context>
<context>
    <name>QgsServerSourceSelect</name>
    <message>
        <source>Are you sure you want to remove the </source>
        <translation>Weet u zeker dat u dit wilt verwijderen</translation>
    </message>
    <message>
        <source>Confirm Delete</source>
        <translation>Bevestig Verwijderen</translation>
    </message>
    <message>
        <source> connection and all associated settings?</source>
        <translation> verbinding en alle daarbij horende instellingen?</translation>
    </message>
    <message>
        <source>Coordinate Reference System</source>
        <translation>Ruimtelijk Referentie Systeem</translation>
    </message>
    <message>
        <source>Coordinate Reference System (%1 available)</source>
        <translation type="obsolete">Ruimtelijk Referentie Systeem (%1 aanwezig)
        
        
        
        </translation>
    </message>
    <message>
        <source>Could not open the WMS Provider</source>
        <translation>WMS-provider kan niet worden geopend</translation>
    </message>
    <message>
        <source>Could not understand the response.  The</source>
        <translation>Antwoord niet duidelijk, De </translation>
    </message>
    <message>
        <source>provider said</source>
        <translation>provider meldt </translation>
    </message>
    <message>
        <source>Select Layer</source>
        <translation>Selecteer Laag</translation>
    </message>
    <message>
        <source>There are no available coordinate reference system for the set of layers you&apos;ve selected.</source>
        <translation>Er zijn geen Ruimtelijke Referentie Systemen beschikbaar voor de geselecteerde lagen.</translation>
    </message>
    <message>
        <source>WMS Provider</source>
        <translation>WMS Provider</translation>
    </message>
    <message>
        <source>WMS proxies</source>
        <translation>WMS proxies</translation>
    </message>
    <message>
        <source>You must select at least one layer first.</source>
        <translation>U moet eerst minstens 1 laag selecteren.</translation>
    </message>
    <message>
        <source>Several WMS servers have been added to the server list. Note that if you access the internet via a web proxy, you will need to set the proxy settings in the QGIS options dialog.</source>
        <translation>Er zijn verschillende WMS-servers toegevoegd aan de server-lijst. Wanneer u een internetverbinding heeft via een web-proxy moet u die aangeven in de proxyinstellingen van de QGIS-optie dialoog.</translation>
    </message>
</context>
<context>
    <name>QgsServerSourceSelectBase</name>
    <message>
        <source>Abstract</source>
        <translation>Beschrijving</translation>
    </message>
    <message>
        <source>&amp;Add</source>
        <translation>&amp;Toevoegen</translation>
    </message>
    <message>
        <source>Add default servers</source>
        <translation>Standaard servers toevoegen</translation>
    </message>
    <message>
        <source>Add Layer(s) from a Server</source>
        <translation>Lagen toevoegen van een Server</translation>
    </message>
    <message>
        <source>Adds a few example WMS servers</source>
        <translation>Voegt een aantal voorbeeld WMS-servers toe</translation>
    </message>
    <message>
        <source>Alt+A</source>
        <translation>Alt+A</translation>
    </message>
    <message>
        <source>Alt+L</source>
        <translation>Alt+L</translation>
    </message>
    <message>
        <source>Change ...</source>
        <translation>Aanpassen ...</translation>
    </message>
    <message>
        <source>C&amp;lose</source>
        <translation>Af&amp;sluiten</translation>
    </message>
    <message>
        <source>C&amp;onnect</source>
        <translation>Ver&amp;binden</translation>
    </message>
    <message>
        <source>Coordinate Reference System</source>
        <translation>Ruimtelijk Referentie Systeem</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>Verwijderen</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation>Bewerken</translation>
    </message>
    <message>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <source>Help</source>
        <translation>Help</translation>
    </message>
    <message>
        <source>ID</source>
        <translation>ID</translation>
    </message>
    <message>
        <source>Image encoding</source>
        <translation>Image encoding</translation>
    </message>
    <message>
        <source>Layers</source>
        <translation>Lagen</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Naam</translation>
    </message>
    <message>
        <source>&amp;New</source>
        <translation>&amp;Nieuw</translation>
    </message>
    <message>
        <source>Ready</source>
        <translation>Gereed</translation>
    </message>
    <message>
        <source>Server Connections</source>
        <translation>Server-verbindingen</translation>
    </message>
    <message>
        <source>Title</source>
        <translation>Titel</translation>
    </message>
</context>
<context>
    <name>QgsShapeFile</name>
    <message>
        <source>... (rest of SQL trimmed)</source>
        <comment>is appended to a truncated SQL statement</comment>
        <translation>... (rest van SQL is afgebroken )</translation>
    </message>
    <message>
        <source>The database gave an error while executing this SQL:</source>
        <translation>De database kwam terug met een fout bij het uitvoeren van deze SQL:</translation>
    </message>
    <message>
        <source>The error was:</source>
        <translation>De fout was:</translation>
    </message>
    <message>
        <source>Scanning </source>
        <translation>Bezig met scannen </translation>
    </message>
</context>
<context>
    <name>QgsSingleSymbolDialog</name>
    <message>
        <source>Dash Dot Dot Line</source>
        <translation>Streep-stippel-stippel-lijn</translation>
    </message>
    <message>
        <source>Dash Dot Line</source>
        <translation>Streep-stippellijn</translation>
    </message>
    <message>
        <source>Dash Line</source>
        <translation>Streeplijn</translation>
    </message>
    <message>
        <source>Dot Line</source>
        <translation>Stippellijn</translation>
    </message>
    <message>
        <source>No Brush</source>
        <translation>Geen Vulling</translation>
    </message>
    <message>
        <source>No Pen</source>
        <translation>Geen Lijn</translation>
    </message>
    <message>
        <source>Solid Line</source>
        <translation>Doorgetrokken Lijn</translation>
    </message>
    <message>
        <source>Solid</source>
        <translation>Doorgetrokken</translation>
    </message>
    <message>
        <source>Horizontal</source>
        <translation>Horizontaal</translation>
    </message>
    <message>
        <source>Vertical</source>
        <translation>Verticaal</translation>
    </message>
    <message>
        <source>Cross</source>
        <translation>Kruis</translation>
    </message>
    <message>
        <source>BDiagonal</source>
        <translation>BDiagonaal</translation>
    </message>
    <message>
        <source>FDiagonal</source>
        <translation>FDiagonaal</translation>
    </message>
    <message>
        <source>Diagonal X</source>
        <translation>Diagonaal X</translation>
    </message>
    <message>
        <source>Dense1</source>
        <translation>Dichtheid1</translation>
    </message>
    <message>
        <source>Dense2</source>
        <translation>Dichtheid2</translation>
    </message>
    <message>
        <source>Dense3</source>
        <translation>Dichtheid3</translation>
    </message>
    <message>
        <source>Dense4</source>
        <translation>Dichtheid4</translation>
    </message>
    <message>
        <source>Dense5</source>
        <translation>Dichtheid5</translation>
    </message>
    <message>
        <source>Dense6</source>
        <translation>Dichtheid6</translation>
    </message>
    <message>
        <source>Dense7</source>
        <translation>Dichtheid7</translation>
    </message>
    <message>
        <source>Texture</source>
        <translation>Textuur</translation>
    </message>
</context>
<context>
    <name>QgsSingleSymbolDialogBase</name>
    <message>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <source>Area scale field</source>
        <translation>Schalingsveld voor vlakken</translation>
    </message>
    <message>
        <source>Fill color</source>
        <translation>Kleurvulling</translation>
    </message>
    <message>
        <source>Fill style</source>
        <translation>Vulstijl</translation>
    </message>
    <message>
        <source>Label</source>
        <translation>Label</translation>
    </message>
    <message>
        <source>Outline color</source>
        <translation>Lijnkleur</translation>
    </message>
    <message>
        <source>Outline style</source>
        <translation>Lijnstijl</translation>
    </message>
    <message>
        <source>Outline width</source>
        <translation>Lijndikte</translation>
    </message>
    <message>
        <source>Point Symbol</source>
        <translation>Puntsymbool</translation>
    </message>
    <message>
        <source>Rotation field</source>
        <translation>Veld voor rotatie</translation>
    </message>
    <message>
        <source>Single Symbol</source>
        <translation>Enkel Symbool</translation>
    </message>
    <message>
        <source>Size</source>
        <translation>Formaat</translation>
    </message>
    <message>
        <source>Style Options</source>
        <translation>Stijlopties</translation>
    </message>
</context>
<context>
    <name>QgsSnappingDialog</name>
    <message>
        <source>to segment</source>
        <translation>naar segment</translation>
    </message>
    <message>
        <source>to vertex</source>
        <translation>naar hoekpunt</translation>
    </message>
    <message>
        <source>to vertex and segment</source>
        <translation>naar hoekpunt en segment</translation>
    </message>
</context>
<context>
    <name>QgsSnappingDialogBase</name>
    <message>
        <source>Layer</source>
        <translation>Laag</translation>
    </message>
    <message>
        <source>Mode</source>
        <translation>Modus</translation>
    </message>
    <message>
        <source>Snapping options</source>
        <translation>Snapping opties</translation>
    </message>
    <message>
        <source>Tolerance</source>
        <translation>Tolerantie</translation>
    </message>
</context>
<context>
    <name>QgsSpit</name>
    <message>
        <source>[Add ...] - open a File dialog and browse to the desired file(s) to import</source>
        <translation>[Toevoegen ...] -  Open de Bestandsdialoog en kies gewenste bestand(en) om te importeren</translation>
    </message>
    <message>
        <source>Add Shapefiles</source>
        <translation>Saphebestand toevoegen</translation>
    </message>
    <message>
        <source>Are you sure you want to remove the [</source>
        <translation>Bent u zeker over de verwijdering van [</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Annuleren</translation>
    </message>
    <message>
        <source>Checking to see if </source>
        <translation type="obsolete">Check om te zien of </translation>
    </message>
    <message>
        <source>Confirm Delete</source>
        <translation>Bevestig Verwijderen</translation>
    </message>
    <message>
        <source>] connection and all associated settings?</source>
        <translation>] verbinding en alle daaraan verbonden instellingen?</translation>
    </message>
    <message>
        <source>Connection failed - Check settings and try again</source>
        <translation>Verbindiging mislukt - Controleer uw instellingen en probeer het opnieuw</translation>
    </message>
    <message>
        <source>DB Relation Name</source>
        <translation>Database Relatie Naam</translation>
    </message>
    <message>
        <source>Do you want to overwrite the [</source>
        <translation>Bent u zeker van het overschrijven van [</translation>
    </message>
    <message>
        <source>[Edit ...] - edit the currently selected connection</source>
        <translation>[Aanpassen ...] - aanpassen van de huidige geselecteerde verbinding</translation>
    </message>
    <message>
        <source>Feature Class</source>
        <translation>Object Klasse</translation>
    </message>
    <message>
        <source>Features</source>
        <translation>Objecten</translation>
    </message>
    <message>
        <source>File Name</source>
        <translation>Bestandsnaam</translation>
    </message>
    <message>
        <source>for this Shapefile in the main dialog file list.</source>
        <translation>voor dit Shapebestand in de bestandslijst van de startdialoog.</translation>
    </message>
    <message>
        <source>General Interface Help:</source>
        <translation>Algemene Interface Help:</translation>
    </message>
    <message>
        <source>[Geometry Column Name] - name of the geometry column in the database</source>
        <translation>[Geometrie Kolomnaam] - naam van de geometriekolom in de database</translation>
    </message>
    <message>
        <source>[Glogal Schema] - set the schema for all files to be imported into</source>
        <translation>[Basis Schema] - instellen van het schema waarin alle bestanden in zullen worden geimporteerd</translation>
    </message>
    <message>
        <source>[Help] - display this help dialog</source>
        <translation>[Help] - toon deze helpdialoog</translation>
    </message>
    <message>
        <source>[Import] - import the current shapefiles in the list</source>
        <translation>[Importeren] - importeer de huidige shapebestanden in de lijst</translation>
    </message>
    <message>
        <source>Importing files</source>
        <translation>Bestanden worden geïmporteerd</translation>
    </message>
    <message>
        <source>Import Shapefiles</source>
        <translation>Shapebestanden Importeren</translation>
    </message>
    <message>
        <source>Import Shapefiles - Relation Exists</source>
        <translation>Shapebestanden Importeren - Relatie bestaat al</translation>
    </message>
    <message>
        <source>Invalid table name.</source>
        <translation>Ongeldige Tabelnaam.</translation>
    </message>
    <message>
        <source>New Connection</source>
        <translation type="obsolete">Nieuwe Verbinding</translation>
    </message>
    <message>
        <source>[New ...] - create a new connection</source>
        <translation>[Nieuw ...] - nieuwe verbinding aanmaken</translation>
    </message>
    <message>
        <source>No fields detected.</source>
        <translation>Geen velden gevonden.</translation>
    </message>
    <message>
        <source>&lt;p&gt;Error while executing the SQL:&lt;/p&gt;&lt;p&gt;</source>
        <translation>&lt;p&gt;Fout tijdens uitvoering van SQL:&lt;/p&gt;&lt;p&gt;</translation>
    </message>
    <message>
        <source>PostGIS not available</source>
        <translation>PostGIS niet beschikbaar</translation>
    </message>
    <message>
        <source>PostgreSQL Connections:</source>
        <translation>PostgreSQL Verbindingen:</translation>
    </message>
    <message>
        <source>&lt;/p&gt;&lt;p&gt;The database said:</source>
        <translation>&lt;/p&gt;&lt;p&gt;Antwoord van de database:</translation>
    </message>
    <message>
        <source>Problem inserting features from file:</source>
        <translation>Fout bij het toevoegen van objecten uit het bestand:</translation>
    </message>
    <message>
        <source>Progress</source>
        <translation>Voortgang</translation>
    </message>
    <message>
        <source>&lt;p&gt;The chosen database does not have PostGIS installed, but this is required for storage of spatial data.&lt;/p&gt;</source>
        <translation>&lt;p&gt;In de gekozen database is PostGIS niet geinstalleerd, maar dit is noodzakelijk om ruimtelijke data op te kunnen slaan&lt;/p&gt;</translation>
    </message>
    <message>
        <source>[Quit] - quit the program
</source>
        <translation>[Afsluiten] - programma afsluiten
</translation>
    </message>
    <message>
        <source>REASON: File cannot be opened</source>
        <translation>REDEN: Bestand kan niet worden geopend</translation>
    </message>
    <message>
        <source>REASON: One or both of the Shapefile files (*.dbf, *.shx) missing</source>
        <translation>REDEN: Een of meer van de Shapefile bestanden (*.dbf, *.shx) mist</translation>
    </message>
    <message>
        <source>] relation?</source>
        <translation>] relatie?</translation>
    </message>
    <message>
        <source>] relation for its data,</source>
        <translation>] relatie voor de data,</translation>
    </message>
    <message>
        <source>[Remove All] - remove all the files in the list</source>
        <translation>[Allen Verwijderen] - verwijder alle bestanden in de lijst</translation>
    </message>
    <message>
        <source>[Remove] - remove the currently selected connection</source>
        <translation>[Verwijderen] - verwijder de huidig geselecteerde verbinding</translation>
    </message>
    <message>
        <source>[Remove] - remove the currently selected file(s) from the list</source>
        <translation>[Verwijderen] - verwijder huidig geselecteerde bestand(en) uit de lijst</translation>
    </message>
    <message>
        <source>Schema</source>
        <translation>Schema</translation>
    </message>
    <message>
        <source>Shapefile List:</source>
        <translation>Shapebestandslijst:</translation>
    </message>
    <message>
        <source>Shapefiles (*.shp);;All files (*.*)</source>
        <translation>Shape-bestanden (*.shp);;Alle bestanden (*.*)</translation>
    </message>
    <message>
        <source>[SRID] - Reference ID for the shapefiles to be imported</source>
        <translation>[SRID] - Ruimtelijke Referentie ID voor de te importeren shape-bestanden</translation>
    </message>
    <message>
        <source>The following fields are duplicates:</source>
        <translation>De volgende velden zijn duplicaten:</translation>
    </message>
    <message>
        <source>The following Shapefile(s) could not be loaded:

</source>
        <translation>De volgende Shape-bestanden kunnen niet geladen worden:

</translation>
    </message>
    <message>
        <source>The Shapefile:</source>
        <translation>Het Shape-bestand:</translation>
    </message>
    <message>
        <source>To avoid data loss change the &quot;DB Relation Name&quot;</source>
        <translation>Om dataverlies te voorkomen, pas de DB Relation Name&quot; aan</translation>
    </message>
    <message>
        <source>[Use Default (Geometry Column Name)] - set column name to &apos;the_geom&apos;</source>
        <translation>[Gebruik Standaard (Geometrie-kolomnaam)] - gebruik &apos;the_geom&apos; als kolomnaam</translation>
    </message>
    <message>
        <source>[Use Default (SRID)] - set SRID to -1</source>
        <translation>[Gebruik Standaard (SRID)] - zet SRID op -1</translation>
    </message>
    <message>
        <source>-when changing connections Global Schema also changes accordingly</source>
        <translation>-bij het aanpassen van verbindingen verandert het &apos;Globale Schema&apos; mee</translation>
    </message>
    <message>
        <source>which already exists and possibly contains data.</source>
        <translation>welke al bestaat en misschien data bevat.</translation>
    </message>
    <message>
        <source>will use [</source>
        <translation>zal gebruiken [</translation>
    </message>
    <message>
        <source>You need to add shapefiles to the list first</source>
        <translation>Eerst shape-bestanden aan de lijst toevoegen alstublieft</translation>
    </message>
    <message>
        <source>-you need to select a connection that works (connects properly) in order to import files</source>
        <translation>-u dient een werkende database-verbinding te hebben om bestanden te kunnen importeren</translation>
    </message>
    <message>
        <source>You need to specify a Connection first</source>
        <translation>U moet eerst een Verbinding instellen</translation>
    </message>
    <message>
        <source>%1 of %2 shapefiles could not be imported.</source>
        <translation>%1 van %2 shape-bestanden konden niet worden geimporteerd.</translation>
    </message>
    <message>
        <source>Password for </source>
        <translation>Wachtwoord voor</translation>
    </message>
    <message>
        <source>Please enter your password:</source>
        <translation>Voer wachtwoord in:</translation>
    </message>
</context>
<context>
    <name>QgsSpitBase</name>
    <message>
        <source>Add</source>
        <translation>Toevoegen</translation>
    </message>
    <message>
        <source>Add a shapefile to the list of files to be imported</source>
        <translation>Shape-bestand toevoegen aan de lijst met te importen bestanden</translation>
    </message>
    <message>
        <source>Create a new PostGIS connection</source>
        <translation>Nieuwe PostGIS-verbinding aanmaken</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation>Bewerken</translation>
    </message>
    <message>
        <source>Edit the current PostGIS connection</source>
        <translation>Huidige PostGIS-verbinding aanpassen</translation>
    </message>
    <message>
        <source>Global Schema</source>
        <translation>Globale Schema</translation>
    </message>
    <message>
        <source>New</source>
        <translation>Nieuw</translation>
    </message>
    <message>
        <source>PostgreSQL Connections</source>
        <translation>PostgreSQL Verbindingen</translation>
    </message>
    <message>
        <source>Remove</source>
        <translation>Verwijder</translation>
    </message>
    <message>
        <source>Remove All</source>
        <translation>Verwijder Alles</translation>
    </message>
    <message>
        <source>Remove all the shapefiles from the import list</source>
        <translation>Verwijder alle shape-bestanden van de te importeren lijst</translation>
    </message>
    <message>
        <source>Remove the current PostGIS connection</source>
        <translation>Verwijder huidige PostGIS-verbinding</translation>
    </message>
    <message>
        <source>Remove the selected shapefile from the import list</source>
        <translation>Verwijder het geselecteerde shape-bestand van de de importeren lijst</translation>
    </message>
    <message>
        <source>Set the geometry column name to the default value</source>
        <translation>Zet naam van de geometie-kolom terug naar de standaardinstellingen.</translation>
    </message>
    <message>
        <source>Set the SRID to the default value</source>
        <translation>Zet de SRID terug naar de standaardinstelling</translation>
    </message>
    <message>
        <source>SPIT - Shapefile to PostGIS Import Tool</source>
        <translation>SPIT - Importeer Shape-bestanden naar PostGIS</translation>
    </message>
    <message>
        <source>Import options and shapefile list</source>
        <translation>Import-opties en shape-bestandslijst</translation>
    </message>
    <message>
        <source>Use Default SRID or specify here</source>
        <translation>Gebruik standaard SRID of definieer hier</translation>
    </message>
    <message>
        <source>Use Default Geometry Column Name or specify here</source>
        <translation>Gebruik de standaard Geometrie-kolom naam of definieer hier</translation>
    </message>
    <message>
        <source>Primary Key Column Name</source>
        <translation>Naam voor de kolom met Primaire Sleutel</translation>
    </message>
    <message>
        <source>Connect to PostGIS</source>
        <translation>Verbind met PostGIS</translation>
    </message>
    <message>
        <source>Connect</source>
        <translation>Verbinden</translation>
    </message>
</context>
<context>
    <name>QgsSpitPlugin</name>
    <message>
        <source>Import shapefiles into a PostGIS-enabled PostgreSQL database. The schema and field names can be customized on import</source>
        <translation>Importeer shape-bestanden in een PostGIS/PostgreSQL database. Het schema en de veldnamen kunnen worden aangepast bij het importeren.</translation>
    </message>
    <message>
        <source>&amp;Import Shapefiles to PostgreSQL</source>
        <translation>&amp;Importeer Shapefiles in PostgreSQL</translation>
    </message>
    <message>
        <source>&amp;Spit</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>QgsTINInterpolatorDialog</name>
    <message>
        <source>Linear interpolation</source>
        <translation>Lineaire interpolatie</translation>
    </message>
</context>
<context>
    <name>QgsTINInterpolatorDialogBase</name>
    <message>
        <source>Triangle based interpolation</source>
        <translation>Driehoekgebaseerde interpolatie</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:12pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;This interpolator provides different methods for interpolation in a triangular irregular network (TIN).&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:12pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Deze interpolator ondersteund meerdere methoden voor interpolatie in een &apos;triangular irregular network&apos; (TIN).&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Interpolation method:</source>
        <translation>Interpolatiemethode:</translation>
    </message>
</context>
<context>
    <name>QgsUniqueValueDialog</name>
    <message>
        <source>Confirm Delete</source>
        <translation>Bevestig Verwijderen</translation>
    </message>
    <message>
        <source>The classification field was changed from &apos;%1&apos; to &apos;%2&apos;.
Should the existing classes be deleted before classification?</source>
        <translation>Het veld voor klassificatie is veranderd van &apos;%1&apos; naar &apos;%2&apos;
Moeten de bestaande klassen worden verwijderd voor de nieuwe klassificatie?</translation>
    </message>
</context>
<context>
    <name>QgsUniqueValueDialogBase</name>
    <message>
        <source>Classification Field:</source>
        <translation type="obsolete">Veld voor classificatie:</translation>
    </message>
    <message>
        <source>Classify</source>
        <translation>Classificeren</translation>
    </message>
    <message>
        <source>Delete class</source>
        <translation type="obsolete">Klasse Verwijderen</translation>
    </message>
    <message>
        <source>Form1</source>
        <translation>Form1</translation>
    </message>
    <message>
        <source>Classification field</source>
        <translation>Veld voor classificatie</translation>
    </message>
    <message>
        <source>Add class</source>
        <translation>Klasse toevoegen</translation>
    </message>
    <message>
        <source>Delete classes</source>
        <translation>Klasses verwijderen</translation>
    </message>
    <message>
        <source>Randomize Colors</source>
        <translation>Willekeurige Kleuren</translation>
    </message>
    <message>
        <source>Reset Colors</source>
        <translation>Reset Kleuren</translation>
    </message>
</context>
<context>
    <name>QgsVectorLayer</name>
    <message>
        <source>Could not commit the added features.</source>
        <translation type="obsolete">Het toevoegen van de objecten kan niet doorgevoerd worden.</translation>
    </message>
    <message>
        <source>Could not commit the changed attributes.</source>
        <translation type="obsolete">De attribuutaanpassingen kunnen niet doorgevoerd worden.</translation>
    </message>
    <message>
        <source>Could not commit the changed geometries.</source>
        <translation type="obsolete">De geometrie-aanpassingen kunnen niet doorgevoerd worden.</translation>
    </message>
    <message>
        <source>Could not commit the deleted features.</source>
        <translation type="obsolete">Het verwijderen van de objecten kan niet worden doorgevoerd.</translation>
    </message>
    <message>
        <source>However, the added features were committed OK.</source>
        <translation type="obsolete">Echter, de het toevoegen van de object is goed doorgevoerd.</translation>
    </message>
    <message>
        <source>However, the changed attributes were committed OK.</source>
        <translation type="obsolete">Echter, de attribuut-aanpassing is wel doorgevoerd.</translation>
    </message>
    <message>
        <source>However, the changed geometries were committed OK.</source>
        <translation type="obsolete">Echter, de veranderingen aan de object geometrie is wel doorgevoerd.</translation>
    </message>
    <message>
        <source>No other types of changes will be committed at this time.</source>
        <translation type="obsolete">Er kunnen op dit moment geen anderen aanpassingen worden doorgevoerd.</translation>
    </message>
    <message>
        <source>ERROR: no provider</source>
        <translation>FOUT: geen provider</translation>
    </message>
    <message>
        <source>ERROR: layer not editable</source>
        <translation>FOUT: laag is niet te wijzigen</translation>
    </message>
    <message>
        <source>SUCCESS: %1 attributes added.</source>
        <translation>GESLAAGD: %1 attributen toegevoegd.</translation>
    </message>
    <message>
        <source>ERROR: %1 new attributes not added</source>
        <translation>FOUT: %1 nieuwe attributen niet toegevoegd</translation>
    </message>
    <message>
        <source>SUCCESS: %1 attributes deleted.</source>
        <translation>GESLAAGD; %1 attributen verwijderd.</translation>
    </message>
    <message>
        <source>ERROR: %1 attributes not deleted.</source>
        <translation>FOUT: %1 attributen niet verwijderd.</translation>
    </message>
    <message>
        <source>SUCCESS: attribute %1 was added.</source>
        <translation>GESLAAGD: attribuut %1 is toegevoegd.</translation>
    </message>
    <message>
        <source>ERROR: attribute %1 not added</source>
        <translation>FOUT: attribuut %1 niet toegevoegd</translation>
    </message>
    <message>
        <source>SUCCESS: %1 attribute values changed.</source>
        <translation>GESLAAGD: %1 attribuutwaarden aangepast.</translation>
    </message>
    <message>
        <source>ERROR: %1 attribute value changes not applied.</source>
        <translation>FOUT: %1 attribuutwaarden zijn niet aangepast.</translation>
    </message>
    <message>
        <source>SUCCESS: %1 features added.</source>
        <translation>GESLAAGD: %1 objecten toegevoegd.</translation>
    </message>
    <message>
        <source>ERROR: %1 features not added.</source>
        <translation>FOUT: %1 objecten niet toegevoegd.</translation>
    </message>
    <message>
        <source>SUCCESS: %1 geometries were changed.</source>
        <translation>GESLAAGD: %1 geometrieën zijn aangepast.</translation>
    </message>
    <message>
        <source>ERROR: %1 geometries not changed.</source>
        <translation>FOUT: %1 geometrieën niet aangepast.</translation>
    </message>
    <message>
        <source>SUCCESS: %1 features deleted.</source>
        <translation>GESLAAGD: %1 objecten verwijderd.</translation>
    </message>
    <message>
        <source>ERROR: %1 features not deleted.</source>
        <translation>FOUT: %1 objecten niet verwijderd.</translation>
    </message>
    <message>
        <source>No renderer object</source>
        <translation>Geen renderer object</translation>
    </message>
    <message>
        <source>Classification field not found</source>
        <translation>Veld voor classificatie niet gevonden </translation>
    </message>
</context>
<context>
    <name>QgsVectorLayerProperties</name>
    <message>
        <source>Attribute field info:</source>
        <translation>Attribuutveld informatie:</translation>
    </message>
    <message>
        <source>Comment</source>
        <translation>Opmerking</translation>
    </message>
    <message>
        <source>Continuous Color</source>
        <translation>Continue kleur</translation>
    </message>
    <message>
        <source>Creation of spatial index failed</source>
        <translation>Ruimtelijke index aanmaken mislukt</translation>
    </message>
    <message>
        <source>Creation of spatial index successfull</source>
        <translation type="obsolete">Ruimtelijke index aanmaken geslaagd</translation>
    </message>
    <message>
        <source>Default Style</source>
        <translation>Standaard Stijl</translation>
    </message>
    <message>
        <source>Editing capabilities of this layer : </source>
        <translation>Aanpas mogelijkheden voor deze laag : </translation>
    </message>
    <message>
        <source>Extents:</source>
        <translation>Extents:</translation>
    </message>
    <message>
        <source>Field</source>
        <translation>Veld</translation>
    </message>
    <message>
        <source>General:</source>
        <translation>Algemeen:</translation>
    </message>
    <message>
        <source>Geometry type of the features in this layer : </source>
        <translation>Type geometrie van de objecten in deze laag: </translation>
    </message>
    <message>
        <source>Graduated Symbol</source>
        <translation></translation>
    </message>
    <message>
        <source>In layer spatial reference system units : </source>
        <translation>In eenheden van het ruimtelijke referentie systeem: </translation>
    </message>
    <message>
        <source>In project spatial reference system units : </source>
        <translation>In eenheden van het ruimtelijk referentie systeem van dit project: </translation>
    </message>
    <message>
        <source>Layer comment: </source>
        <translation>Laag opmerking: </translation>
    </message>
    <message>
        <source>Layer Spatial Reference System:</source>
        <translation>Ruimtelijk Referentie Systeem:</translation>
    </message>
    <message>
        <source>Length</source>
        <translation>Lengte</translation>
    </message>
    <message>
        <source>Precision</source>
        <translation>Precisie</translation>
    </message>
    <message>
        <source>QGIS</source>
        <translation>QGIS</translation>
    </message>
    <message>
        <source>QGIS Layer Style File (*.qml)</source>
        <translation>QGIS Laagstijlbestand (*.qml)</translation>
    </message>
    <message>
        <source>Single Symbol</source>
        <translation>Enkel Symbool</translation>
    </message>
    <message>
        <source>Source for this layer : </source>
        <translation>Bron voor deze laag: </translation>
    </message>
    <message>
        <source>Spatial Index</source>
        <translation>Ruimtelijke Index</translation>
    </message>
    <message>
        <source>Storage type of this layer : </source>
        <translation>Opslagtype van deze laag : </translation>
    </message>
    <message>
        <source>The number of features in this layer : </source>
        <translation>Het aantal kaartobjecten in deze laag: </translation>
    </message>
    <message>
        <source>The query used to limit the features in the layer is shown here. This is currently only supported for PostgreSQL layers. To enter or modify the query, click on the Query Builder button</source>
        <translation>De query om het aantal object in deze laag te beperken wordt hier getoond. Dit wordt op dit moment alleen ondersteund in PostgreSQL-lagen. Om de query aan te passen, klik de QueryBouwer-knop</translation>
    </message>
    <message>
        <source>This button opens the PostgreSQL query builder and allows you to create a subset of features to display on the map canvas rather than displaying all features in the layer</source>
        <translation>Deze knop opent de PostgreSQL querybouwer and maakt het mogelijk om een selectie van de objecten to tonen op de kaart i.p.v. alle objecten in de laag</translation>
    </message>
    <message>
        <source>Transparency: </source>
        <translation>Transparantie: </translation>
    </message>
    <message>
        <source>Type</source>
        <translation>Type</translation>
    </message>
    <message>
        <source>Unique Value</source>
        <translation>Unieke Waarde</translation>
    </message>
    <message>
        <source>Unknown style format: </source>
        <translation>Onbekend stijlformaat: </translation>
    </message>
    <message>
        <source> : xMax,yMax </source>
        <translation> : Xmax,yMax </translation>
    </message>
    <message>
        <source>xMin,yMin </source>
        <translation>xMin,yMin </translation>
    </message>
    <message>
        <source>id</source>
        <translation>id</translation>
    </message>
    <message>
        <source>name</source>
        <translation>naam</translation>
    </message>
    <message>
        <source>type</source>
        <translation>type</translation>
    </message>
    <message>
        <source>length</source>
        <translation>lengte</translation>
    </message>
    <message>
        <source>precision</source>
        <translation>nauwkeurigheid</translation>
    </message>
    <message>
        <source>comment</source>
        <translation>opmerking</translation>
    </message>
    <message>
        <source>edit widget</source>
        <translation>wijzig-hulpmiddel</translation>
    </message>
    <message>
        <source>values</source>
        <translation>waarden</translation>
    </message>
    <message>
        <source>line edit</source>
        <translation>lijn wijzigen</translation>
    </message>
    <message>
        <source>unique values</source>
        <translation>unieke waarden</translation>
    </message>
    <message>
        <source>unique values (editable)</source>
        <translation>unieke waarden (aanpasbaar)</translation>
    </message>
    <message>
        <source>value map</source>
        <translation>aanwezige waarden</translation>
    </message>
    <message>
        <source>classification</source>
        <translation>classificatie</translation>
    </message>
    <message>
        <source>range (editable)</source>
        <translation>bereik (aanpasbaar)</translation>
    </message>
    <message>
        <source>range (slider)</source>
        <translation>bereik (schuiver)</translation>
    </message>
    <message>
        <source>file name</source>
        <translation>bestandsnaam</translation>
    </message>
    <message>
        <source>Name conflict</source>
        <translation>Naamconflict</translation>
    </message>
    <message>
        <source>The attribute could not be inserted. The name already exists in the table.</source>
        <translation>Het attribuut kon niet worden toegevoegd. De naam bestaat al in de tabel.</translation>
    </message>
    <message>
        <source>Creation of spatial index successful</source>
        <translation>Ruimtelijk index succesvol aangemaakt</translation>
    </message>
    <message>
        <source>Saved Style</source>
        <translation>Opgeslagen Stijl</translation>
    </message>
</context>
<context>
    <name>QgsVectorLayerPropertiesBase</name>
    <message>
        <source>Actions</source>
        <translation>Acties</translation>
    </message>
    <message>
        <source>Change</source>
        <translation type="obsolete">Aanpassen</translation>
    </message>
    <message>
        <source>Create</source>
        <translation type="obsolete">Aanmaken</translation>
    </message>
    <message>
        <source>Create Spatial Index</source>
        <translation>Ruimtelijke index maken</translation>
    </message>
    <message>
        <source>Display field</source>
        <translation>Veld tonen</translation>
    </message>
    <message>
        <source>Display field for the Identify Results dialog box</source>
        <translation>Toon het veld voor de Resultaten van het identificeren in de dialoog</translation>
    </message>
    <message>
        <source>Display labels</source>
        <translation>Toon labels</translation>
    </message>
    <message>
        <source>Display name</source>
        <translation>Toon naam</translation>
    </message>
    <message>
        <source>General</source>
        <translation>Algemeen</translation>
    </message>
    <message>
        <source>Labels</source>
        <translation>Labels</translation>
    </message>
    <message>
        <source>Layer Properties</source>
        <translation>Laageigenschappen</translation>
    </message>
    <message>
        <source>Legend type:</source>
        <translation type="obsolete">Legenda type:</translation>
    </message>
    <message>
        <source>Load Style ...</source>
        <translation>Stijl laden ...</translation>
    </message>
    <message>
        <source>Maximum 1:</source>
        <translation type="obsolete">Maximum 1:</translation>
    </message>
    <message>
        <source>Maximum scale at which this layer will be displayed. </source>
        <translation>Maximale schaal bij welke deze laag nog getoond zal worden. </translation>
    </message>
    <message>
        <source>Metadata</source>
        <translation>Metadata</translation>
    </message>
    <message>
        <source>Minimum 1:</source>
        <translation type="obsolete">Minimum 1:</translation>
    </message>
    <message>
        <source>Minimum scale at which this layer will be displayed. </source>
        <translation>Minimale schaal bij welke deze laag nog getoond zal worden. </translation>
    </message>
    <message>
        <source>Query Builder</source>
        <translation>Query Bouwer</translation>
    </message>
    <message>
        <source>Restore Default Style</source>
        <translation>Terug naar Standaard Stijl</translation>
    </message>
    <message>
        <source>Save As Default</source>
        <translation>Opslaan Als Standaard</translation>
    </message>
    <message>
        <source>Save Style ...</source>
        <translation>Stijl Oplaan ...</translation>
    </message>
    <message>
        <source>Spatial Index</source>
        <translation type="obsolete">Ruimtelijke Index</translation>
    </message>
    <message>
        <source>Spatial Reference System</source>
        <translation type="obsolete">Ruimtelijk Referentie Systeem</translation>
    </message>
    <message>
        <source>Subset</source>
        <translation>Subset</translation>
    </message>
    <message>
        <source>Symbology</source>
        <translation>Symbologie</translation>
    </message>
    <message>
        <source>This sets the display field for the Identify Results dialog box</source>
        <translation>Hiermee zet u het veld wat getoond wordt bij het Identificatie gereedschap</translation>
    </message>
    <message>
        <source>Transparency:</source>
        <translation type="obsolete">Transparantie: </translation>
    </message>
    <message>
        <source>Use scale dependent rendering</source>
        <translation>Gebruik schaalafhankelijke tonen</translation>
    </message>
    <message>
        <source>Use this control to set which field is placed at the top level of the Identify Results dialog box.</source>
        <translation>Gebruik deze instelling om aan te geven welk veld bovenaan moet staan bij het Identificeer gereedschap.</translation>
    </message>
    <message>
        <source>Legend type</source>
        <translation>Legenda type</translation>
    </message>
    <message>
        <source>Transparency</source>
        <translation>Transparantie:</translation>
    </message>
    <message>
        <source>Options</source>
        <translation>Opties</translation>
    </message>
    <message>
        <source>Change SRS</source>
        <translation type="obsolete">SRS aanpassen</translation>
    </message>
    <message>
        <source>Maximum</source>
        <translation>Maximum</translation>
    </message>
    <message>
        <source>Minimum</source>
        <translation>Minimum</translation>
    </message>
    <message>
        <source>Change CRS</source>
        <translation>CRS Aanpassen</translation>
    </message>
    <message>
        <source>Attributes</source>
        <translation>Attributen</translation>
    </message>
    <message>
        <source>New column</source>
        <translation>Nieuwe kolom</translation>
    </message>
    <message>
        <source>Ctrl+N</source>
        <translation>Ctrl+N</translation>
    </message>
    <message>
        <source>Delete column</source>
        <translation>Verwijder kolom</translation>
    </message>
    <message>
        <source>Ctrl+X</source>
        <translation>Ctrl+X</translation>
    </message>
    <message>
        <source>Toggle editing mode</source>
        <translation>Modus objectinvoer omzetten</translation>
    </message>
    <message>
        <source>Click to toggle table editing</source>
        <translation>Klik om modus &apos;tabelwijzigen&apos; om te zetten</translation>
    </message>
</context>
<context>
    <name>QgsVectorSymbologyWidgetBase</name>
    <message>
        <source>Count:</source>
        <translation type="obsolete">Aantal:</translation>
    </message>
    <message>
        <source>Field:</source>
        <translation type="obsolete">Veld:</translation>
    </message>
    <message>
        <source>Form2</source>
        <translation type="obsolete">Form2</translation>
    </message>
    <message>
        <source>Label</source>
        <translation type="obsolete">Label</translation>
    </message>
    <message>
        <source>Max</source>
        <translation type="obsolete">Max</translation>
    </message>
    <message>
        <source>Min</source>
        <translation type="obsolete">Min</translation>
    </message>
    <message>
        <source>Mode:</source>
        <translation type="obsolete">Modus:</translation>
    </message>
    <message>
        <source>Symbol Classes:</source>
        <translation type="obsolete">Symbool Klassen:</translation>
    </message>
</context>
<context>
    <name>QgsWFSPlugin</name>
    <message>
        <source>&amp;Add WFS layer</source>
        <translation>&amp;Toevoegen WFS-laag</translation>
    </message>
</context>
<context>
    <name>QgsWFSProvider</name>
    <message>
        <source>received %1 bytes from %2</source>
        <translation>ontvangen %1 bytes van %2</translation>
    </message>
    <message>
        <source>unknown</source>
        <translation>onbekend</translation>
    </message>
</context>
<context>
    <name>QgsWFSSourceSelect</name>
    <message>
        <source>Are you sure you want to remove the </source>
        <translation>Weet u zeker dat u dit wilt verwijderen </translation>
    </message>
    <message>
        <source>Confirm Delete</source>
        <translation>Bevestig Verwijderen</translation>
    </message>
    <message>
        <source> connection and all associated settings?</source>
        <translation> verbinding en alle daarbij horende instellingen?</translation>
    </message>
</context>
<context>
    <name>QgsWFSSourceSelectBase</name>
    <message>
        <source>Abstract</source>
        <translation>Beschrijving</translation>
    </message>
    <message>
        <source>Add WFS Layer from a Server</source>
        <translation>Toevoegen van een WFS-laag van een Server</translation>
    </message>
    <message>
        <source>Change ...</source>
        <translation>Aanpassen ...</translation>
    </message>
    <message>
        <source>C&amp;onnect</source>
        <translation>Ver&amp;binden</translation>
    </message>
    <message>
        <source>Coordinate Reference System</source>
        <translation>Ruimtelijk Referentie Systeem</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>Verwijderen</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation>Bewerken</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Naam</translation>
    </message>
    <message>
        <source>&amp;New</source>
        <translation>&amp;Nieuw</translation>
    </message>
    <message>
        <source>Server Connections</source>
        <translation>Server-verbindingen</translation>
    </message>
    <message>
        <source>Title</source>
        <translation>Titel</translation>
    </message>
</context>
<context>
    <name>QgsWmsProvider</name>
    <message>
        <source>Abstract</source>
        <translation>Beschrijving</translation>
    </message>
    <message>
        <source>Access Constraints</source>
        <translation>Toegangsbeperkingen</translation>
    </message>
    <message>
        <source>Available in CRS</source>
        <translation>Beschikbaar in CRS</translation>
    </message>
    <message>
        <source>Available in style</source>
        <translation>Beschikbaar in stijl</translation>
    </message>
    <message>
        <source>Can be Transparent</source>
        <translation>Kan Transparant zijn</translation>
    </message>
    <message>
        <source>Can Identify</source>
        <translation>Identificeergereedschap te gebruiken</translation>
    </message>
    <message>
        <source>Can Zoom In</source>
        <translation>Zoomin mogelijk</translation>
    </message>
    <message>
        <source>Cascade Count</source>
        <translation>Aantal &apos;Cascade&apos;</translation>
    </message>
    <message>
        <source>Contact Person</source>
        <translation>Contactpersoon</translation>
    </message>
    <message>
        <source>Could not get WMS capabilities: %1 at line %2 column %3</source>
        <translation>WMS-capabilities niet ontvangen: %1 op regel %2 kolom %3</translation>
    </message>
    <message>
        <source>Could not get WMS capabilities in the expected format (DTD): no %1 or %2 found</source>
        <translation>WMS-capabilities niet ontvangen in het verwachte formaat (DTD): geen %1 of %2 gevonden</translation>
    </message>
    <message>
        <source>Could not get WMS Service Exception at %1: %2 at line %3 column %4</source>
        <translation>WMS-service niet bereikbaar. Fout op %1: %2 op regel %3 kolom %4</translation>
    </message>
    <message>
        <source>DOM Exception</source>
        <translation type="obsolete">DOM-fout</translation>
    </message>
    <message>
        <source>Fees</source>
        <translation>Kosten</translation>
    </message>
    <message>
        <source>Fixed Height</source>
        <translation>Vaste Hoogte</translation>
    </message>
    <message>
        <source>Fixed Width</source>
        <translation>Vaste Breedte</translation>
    </message>
    <message>
        <source>GetFeatureInfo request contains invalid X or Y value.</source>
        <translation>GetFeatureInfo-verzoek bevat ongeldige X- of Y-waarde.</translation>
    </message>
    <message>
        <source>GetFeatureInfo request is applied to a Layer which is not declared queryable.</source>
        <translation>GetFeatureInfo-verzoek wordt gedaan op een laag die niet als &apos;queryable&apos; is gedefinieerd.</translation>
    </message>
    <message>
        <source>GetMap request is for a Layer not offered by the server, or GetFeatureInfo request is for a Layer not shown on the map.</source>
        <translation>GetMap-verzoek voor een Laag die niet wordt aangeboden door de server, of een GetFeatureInfo-verzoek voor een Laag niet getoond op de kaart.</translation>
    </message>
    <message>
        <source>Hidden</source>
        <translation>Verborgen</translation>
    </message>
    <message>
        <source>HTTP Exception</source>
        <translation>HTTP-Fout</translation>
    </message>
    <message>
        <source>Identify Formats</source>
        <translation>Identificeergereedschap-Formaten</translation>
    </message>
    <message>
        <source>Image Formats</source>
        <translation>AfbeeldingsFormaten</translation>
    </message>
    <message>
        <source>Keywords</source>
        <translation>Zoek/sleutelwoorden</translation>
    </message>
    <message>
        <source>Layer cannot be queried.</source>
        <translation>Layer kan niet bevraagd worden.</translation>
    </message>
    <message>
        <source>Layer Count</source>
        <translation>Aantal Lagen</translation>
    </message>
    <message>
        <source>Layer Properties: </source>
        <translation>Laag Eigenschappen:</translation>
    </message>
    <message>
        <source>n/a</source>
        <translation>Niet beschikbaar</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Naam</translation>
    </message>
    <message>
        <source>No</source>
        <translation>Nee</translation>
    </message>
    <message>
        <source>Online Resource</source>
        <translation>Internetbron </translation>
    </message>
    <message>
        <source>Property</source>
        <translation>Eigenschap</translation>
    </message>
    <message>
        <source>Request contains a CRS not offered by the server for one or more of the Layers in the request.</source>
        <translation>Verzoek bevat een CRS welke niet wordt aangeboden door de server voor één of meer Lagen in het verzoek.</translation>
    </message>
    <message>
        <source>Request contains a Format not offered by the server.</source>
        <translation>Verzoek bevat een &apos;Format&apos; niet beschikbaar op de server.</translation>
    </message>
    <message>
        <source>Request contains an invalid sample dimension value.</source>
        <translation>Verzoek bevat een niet geldige (sample) dimensie waarde.</translation>
    </message>
    <message>
        <source>Request contains a SRS not offered by the server for one or more of the Layers in the request.</source>
        <translation>Verzoek bevat een CRS welke niet wordt aangeboden door de server voor één of meer Lagen in het verzoek.</translation>
    </message>
    <message>
        <source>Request does not include a sample dimension value, and the server did not declare a default value for that dimension.</source>
        <translation>Verzoek bevat geen (sample) dimensie waarde, en de server heeft geen standaard waarde voor die dimensie.</translation>
    </message>
    <message>
        <source>Request is for a Layer in a Style not offered by the server.</source>
        <translation>Verzoek is voor een Laag in een Stijl die niet wordt aangeboden door de server.</translation>
    </message>
    <message>
        <source>Request is for an optional operation that is not supported by the server.</source>
        <translation>Verzoek voor een optionele operatie welke niet door de server wordt ondersteund.</translation>
    </message>
    <message>
        <source>Selected</source>
        <translation>Geselecteerd</translation>
    </message>
    <message>
        <source>Server Properties:</source>
        <translation>Server Eigenschappen:</translation>
    </message>
    <message>
        <source>The WMS vendor also reported: </source>
        <translation>De WMS bouwer meldde ook: </translation>
    </message>
    <message>
        <source>This is probably due to a bug in the QGIS program.  Please report this error.</source>
        <translation type="obsolete">Dit lijkt een bug in het QGIS programma.   Meld deze fout alstublieft.</translation>
    </message>
    <message>
        <source>This is probably due to an incorrect WMS Server URL.</source>
        <translation>De oorzaak is waarschijnlijk een niet correcte WMS-server URL.</translation>
    </message>
    <message>
        <source>Title</source>
        <translation>Titel</translation>
    </message>
    <message>
        <source>Tried URL: </source>
        <translation>Geprobeerde URL:</translation>
    </message>
    <message>
        <source>(Unknown error code from a post-1.3 WMS server)</source>
        <translation>(Onbekende fout van een post-1.3 WMS-server)</translation>
    </message>
    <message>
        <source>Value</source>
        <translation>Waarde</translation>
    </message>
    <message>
        <source>Value of (optional) UpdateSequence parameter in GetCapabilities request is equal to current value of service metadata update sequence number.</source>
        <translation>Waarde van de (optionele) Ophoog-parameter in GetCapabilities-verzoek is gelijk aan de huidige waarde in de servicemetadata ophoogwaarde.</translation>
    </message>
    <message>
        <source>Value of (optional) UpdateSequence parameter in GetCapabilities request is greater than current value of service metadata update sequence number.</source>
        <translation>Waarde van de (optionele) Ophoog-parameter in GetCapabilities-verzoek is gelijk aan de huidige waarde in de servicemetadata ophoogwaarde.</translation>
    </message>
    <message>
        <source>Visibility</source>
        <translation>Zichtbaarheid</translation>
    </message>
    <message>
        <source>Visible</source>
        <translation>Zichtbaar</translation>
    </message>
    <message>
        <source>WGS 84 Bounding Box</source>
        <translation>WGS 84 Extent</translation>
    </message>
    <message>
        <source>WMS Service Exception</source>
        <translation>WMS Service Fout</translation>
    </message>
    <message>
        <source>WMS Version</source>
        <translation>WMS-Versie</translation>
    </message>
    <message>
        <source>Yes</source>
        <translation>Ja</translation>
    </message>
    <message>
        <source>Dom Exception</source>
        <translation>DOM-fout</translation>
    </message>
</context>
<context>
    <name>QuickPrint</name>
    <message>
        <source>Quick Print</source>
        <translation type="obsolete">Snelle Afdruk</translation>
    </message>
    <message>
        <source>&amp;Quick Print</source>
        <translation type="obsolete">&amp;Snelle Afdruk</translation>
    </message>
    <message>
        <source>Replace this with a short description of the what the plugin does</source>
        <translation type="obsolete">Vervang dit door een korte beschrijving van de functie van de plugin</translation>
    </message>
</context>
<context>
    <name>QuickPrintGui</name>
    <message>
        <source> cm</source>
        <translation type="obsolete"> cm</translation>
    </message>
    <message>
        <source> degree</source>
        <translation type="obsolete"> graad</translation>
    </message>
    <message>
        <source> degrees</source>
        <translation type="obsolete">graden</translation>
    </message>
    <message>
        <source>Documentation:</source>
        <translation type="obsolete">Handleiding:</translation>
    </message>
    <message>
        <source> feet</source>
        <translation type="obsolete"> voet</translation>
    </message>
    <message>
        <source> foot</source>
        <translation type="obsolete"> voet</translation>
    </message>
    <message>
        <source>For Questions and Comments regarding the plugin builder template and creating your features in QGIS using the plugin interface please contact us via:</source>
        <translation type="obsolete">Voor Vragen en Opmerkingen over deze &apos;plugin builder template&apos; en het maken van &apos;features&apos; in QGIS via de plugin interface, neem contact op via:</translation>
    </message>
    <message>
        <source>Getting developer help:</source>
        <translation type="obsolete">Ontwikkelaarshulp:</translation>
    </message>
    <message>
        <source>Have fun and thank you for choosing QGIS.</source>
        <translation type="obsolete">Veel plezier en bedankt voor het gebruik van QGIS.</translation>
    </message>
    <message>
        <source> inches</source>
        <translation type="obsolete">inch</translation>
    </message>
    <message>
        <source>In particular look at the following classes:</source>
        <translation type="obsolete">Kijk vooral naar de volgende klassen:</translation>
    </message>
    <message>
        <source> km</source>
        <translation type="obsolete"> km</translation>
    </message>
    <message>
        <source>&lt;li&gt; the QGIS developers mailing list, or &lt;/li&gt;&lt;li&gt; IRC (#qgis on freenode.net)&lt;/li&gt;</source>
        <translation type="obsolete">&lt;li&gt; the QGIS ontwikkellaars mailing list, or &lt;/li&gt;&lt;li&gt; IRC (#qgis on freenode.net)&lt;/li&gt;</translation>
    </message>
    <message>
        <source> m</source>
        <translation type="obsolete"> m</translation>
    </message>
    <message>
        <source> mile</source>
        <translation type="obsolete">mijl</translation>
    </message>
    <message>
        <source> miles</source>
        <translation type="obsolete">mijl</translation>
    </message>
    <message>
        <source> mm</source>
        <translation type="obsolete"> mm</translation>
    </message>
    <message>
        <source>Portable Document Format (*.pdf)</source>
        <translation>Portable Document Format (*.pdf)</translation>
    </message>
    <message>
        <source>QGIS is distributed under the Gnu Public License. If you create a useful plugin please consider contributing it back to the community.</source>
        <translation type="obsolete">QGIS wordt verspreid on de Gnu Public License. Als u een bruikbare plugin bouwt, overweeg die dan terug te spelen aan de &apos;community&apos;.</translation>
    </message>
    <message>
        <source>QgsPlugin is an ABC that defines required behaviour your plugin must provide. See below for more details.</source>
        <translation type="obsolete">QgsPlugin is een ABC die het meest gebruikt gedrag bevat van uw plugin. Zie hieronder voor meer details.</translation>
    </message>
    <message>
        <source>quickprint</source>
        <translation>snelle afdruk</translation>
    </message>
    <message>
        <source>This file contains the documentation you are reading now!</source>
        <translation type="obsolete">Dit bestand bevat de documentatie welke u nu leest!</translation>
    </message>
    <message>
        <source>This is a Qt designer &apos;ui&apos; file. It defines the look of the default plugin dialog without implementing any application logic. You can modify this form to suite your needs or completely remove it if your plugin does not need to display a user form (e.g. for custom MapTools).</source>
        <translation type="obsolete">Dit is een &apos;Qt designer ui&apos; bestand. Het definieert het uiterluik van de standaard plugin dialoog zonder applicatie logica te implementeren. U kunt deze &apos;form&apos; aanpassen of helemaal verwijderen als uw plugin geen &apos;display&apos; heeft (b.v. zoals de huidige MapTools).</translation>
    </message>
    <message>
        <source>This is just a starting point. You now need to modify the code to make it do something useful....read on for a more information to get yourself started.</source>
        <translation type="obsolete">Dit is maar een startpunt. U moet de code aanpassen om er een een bruikbare plugin van te maken... lees verder voor meer informatie om te beginnen.</translation>
    </message>
    <message>
        <source>This is the class that provides the &apos;glue&apos; between your custom application logic and the QGIS application. You will see that a number of methods are already implemented for you - including some examples of how to add a raster or vector layer to the main application map canvas. This class is a concrete instance of the QgisPlugin interface which defines required behaviour for a plugin. In particular, a plugin has a number of static methods and members so that the QgsPluginManager and plugin loader logic can identify each plugin, create an appropriate menu entry for it etc. Note there is nothing stopping you creating multiple toolbar icons and menu entries for a single plugin. By default though a single menu entry and toolbar button is created and its pre-configured to call the run() method in this class when selected. This default implementation provided for you by the plugin builder is well documented, so please refer to the code for further advice.</source>
        <translation type="obsolete">This is the class that provides the &apos;glue&apos; between your custom application logic and the QGIS application. You will see that a number of methods are already implemented for you - including some examples of how to add a raster or vector layer to the main application map canvas. This class is a concrete instance of the QgisPlugin interface which defines required behaviour for a plugin. In particular, a plugin has a number of static methods and members so that the QgsPluginManager and plugin loader logic can identify each plugin, create an appropriate menu entry for it etc. Note there is nothing stopping you creating multiple toolbar icons and menu entries for a single plugin. By default though a single menu entry and toolbar button is created and its pre-configured to call the run() method in this class when selected. This default implementation provided for you by the plugin builder is well documented, so please refer to the code for further advice.</translation>
    </message>
    <message>
        <source>This is the concrete class where application logic for the above mentioned dialog should go. The world is your oyster here really....</source>
        <translation type="obsolete">Hier komt de echte applicatie logica voor de hierboven genoemde dialoog. Maar er wat leuks van...</translation>
    </message>
    <message>
        <source>This is the generated CMake file that builds the plugin. You should add you application specific dependencies and source files to this file.</source>
        <translation type="obsolete">Dit is het gegenereerde CMake bestand voor het bouwen van de plugin. Breid het uit met de applicatie specifieke afhankelijkheden en bronbestanden.</translation>
    </message>
    <message>
        <source>This is the icon that will be used for your plugin menu entry and toolbar icon. Simply replace this icon with your own icon to make your plugin disctinctive from the rest.</source>
        <translation type="obsolete">Dit is de icoon welke gebruikt zal worden voor het plugin menu en als werkbalk icoon. Vervang eenvoudigweg dit icoon met uw eigen icoon om het onderscheidend te maken van de rest.</translation>
    </message>
    <message>
        <source>This is the Qt4 resources file for your plugin. The Makefile generated for your plugin is all set up to compile the resource file so all you need to do is add your additional icons etc using the simple xml file format. Note the namespace used for all your resources e.g. (&apos;:/Homann/&apos;). It is important to use this prefix for all your resources. We suggest you include any other images and run time data in this resurce file too.</source>
        <translation type="obsolete">Dit is het Qt4 &apos;resource&apos;-bestand voor uw plugin. Het Make-bestand dat gegenereerd is voor up plugin is al ingesteld om dit &apos;resource&apos;-bestand te compileren. Alleen bijkomende iconen toevoegen aan het simpele xml formaat. Let op bij de &apos;namespace&apos; van al uw &apos;resources&apos; b.v. (&apos;:/Homann/&apos;). Het is belangrijk deze &apos;namespace&apos; te gebruiken voor al uw &apos;resources&apos;. We raden aan om andere afbeeldingen en &apos;runtime data&apos; aan dit &apos;resource&apos;-bestand toe te voegen.</translation>
    </message>
    <message>
        <source> unknown</source>
        <translation type="obsolete"> onbekend</translation>
    </message>
    <message>
        <source>Unknown format: </source>
        <translation>onbekend formaat:</translation>
    </message>
    <message>
        <source>Welcome to your automatically generated plugin!</source>
        <translation type="obsolete">Welkom bij uw automatisch gegenereerde plugin!</translation>
    </message>
    <message>
        <source>What are all the files in my generated plugin directory for?</source>
        <translation type="obsolete">Waar zijn al die bestanden in mijn gegenereerde plugin directory?</translation>
    </message>
    <message>
        <source>You really need to read the QGIS API Documentation now at:</source>
        <translation type="obsolete">U moet nu echt de QGIS API handleiding lezen op:</translation>
    </message>
</context>
<context>
    <name>QuickPrintGuiBase</name>
    <message>
        <source>Copyright</source>
        <translation>Copyright</translation>
    </message>
    <message>
        <source>last used filename but incremented will be shown here</source>
        <translation>laatst gebruikte bestandsnaam aangevuld zal hier worden getoond</translation>
    </message>
    <message>
        <source>Map Name e.g. Water Features</source>
        <translation>Kaartnaam b.v. Water-objecten</translation>
    </message>
    <message>
        <source>Map Title e.g. ACME inc.</source>
        <translation>Maptitel, b.v. Van Puffelen BV.</translation>
    </message>
    <message>
        <source>Note: If you want more control over the map layout please use the map composer function in QGIS.</source>
        <translation>Opmerking: als u meer controle wilt over de kaartlayout, gebruik dan de &apos;kaart-composer&apos; functie in QGIS.</translation>
    </message>
    <message>
        <source>Output</source>
        <translation>Output</translation>
    </message>
    <message>
        <source>Plugin Template</source>
        <translation type="obsolete">Plugin Template</translation>
    </message>
    <message>
        <source>Prompt for file name</source>
        <translation>Vraag om bestandsnaam</translation>
    </message>
    <message>
        <source>QGIS Plugin Template</source>
        <translation type="obsolete">QGIS Plugin Template</translation>
    </message>
    <message>
        <source>QGIS Quick Print Plugin</source>
        <translation>QGIS Snelle Afdruk Plugin</translation>
    </message>
    <message>
        <source>Quick Print</source>
        <translation>Snelle Afdruk</translation>
    </message>
    <message>
        <source>Use last filename but incremented.</source>
        <translation>Gebruik de laatst gebruikte bestandsnaam maar opgehoogd.</translation>
    </message>
    <message>
        <source>Page Size</source>
        <translation>Paginagrootte</translation>
    </message>
</context>
<context>
    <name>QuickPrintPlugin</name>
    <message>
        <source>Quick Print</source>
        <translation>Snelle Afdruk</translation>
    </message>
    <message>
        <source>&amp;Quick Print</source>
        <translation>&amp;Snelle Afdruk</translation>
    </message>
    <message>
        <source>Replace this with a short description of the what the plugin does</source>
        <translation type="obsolete">Vervang dit door een korte beschrijving van de functie van de plugin</translation>
    </message>
    <message>
        <source>Provides a way to quickly produce a map with minimal user input.</source>
        <translation>Verschaft een manier om snel een kaart te maken met minimale inspanning.</translation>
    </message>
</context>
<context>
    <name>RepositoryDetailsDialog</name>
    <message>
        <source>http://</source>
        <translation type="obsolete">http://</translation>
    </message>
    <message>
        <source>Name:</source>
        <translation type="obsolete">Naam:</translation>
    </message>
    <message>
        <source>Repository details</source>
        <translation type="obsolete">Repository details</translation>
    </message>
    <message>
        <source>URL:</source>
        <translation type="obsolete">URL:</translation>
    </message>
</context>
<context>
    <name>[pluginname]GuiBase</name>
    <message>
        <source>Plugin Template</source>
        <translation>Plugin Template</translation>
    </message>
    <message>
        <source>QGIS Plugin Template</source>
        <translation>QGIS Plugin Template</translation>
    </message>
</context>
<context>
    <name>dxf2shpConverter</name>
    <message>
        <source>Converts DXF files in Shapefile format</source>
        <translation>Zet DXF-bestanden om naar shapefile-formaat (.shp)</translation>
    </message>
    <message>
        <source>&amp;Dxf2Shp</source>
        <translation>&amp;Dxf2Shp</translation>
    </message>
</context>
<context>
    <name>dxf2shpConverterGui</name>
    <message>
        <source>Choose a delimited text file to open</source>
        <translation type="obsolete">Kies een tekengescheiden tekstbestand om te openen</translation>
    </message>
    <message>
        <source>QGIS Plugin Template</source>
        <translation type="obsolete">QGIS Plugin Template</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;MS Shell Dlg 2&apos;; font-size:8.25pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:22pt;&quot;&gt;Dxf to Shp Converter&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;p, li { white-space: pre-wrap; }&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;MS Shell Dlg 2&apos;; font-size:8.25pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:22pt;&quot;&gt;Dxf naar Shp converteren&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;MS Shell Dlg 2&apos;; font-size:8.25pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:8pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;Output Shp file:&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;p, li { white-space: pre-wrap; }&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;MS Shell Dlg 2&apos;; font-size:8.25pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:8pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;Resultaat Shp-bestand:&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;MS Shell Dlg 2&apos;; font-size:8.25pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:8pt;&quot;&gt;&lt;span style=&quot; font-size:9pt;&quot;&gt;Input DXF file:&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;p, li { white-space: pre-wrap; }&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;MS Shell Dlg 2&apos;; font-size:8.25pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:8pt;&quot;&gt;&lt;span style=&quot; font-size:9pt;&quot;&gt;DXF-bronbestand:&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Browse</source>
        <translation type="obsolete">Bladeren</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;MS Shell Dlg 2&apos;; font-size:8.25pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:9pt;&quot;&gt;Shp output file type:&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;p, li { white-space: pre-wrap; }&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;MS Shell Dlg 2&apos;; font-size:8.25pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:9pt;&quot;&gt;Uitvoertype Shp-bestand:&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Polyline</source>
        <translation>Polyline</translation>
    </message>
    <message>
        <source>Polygon</source>
        <translation>Polygoon</translation>
    </message>
    <message>
        <source>Point</source>
        <translation>Punt</translation>
    </message>
    <message>
        <source>Export text labels</source>
        <translation>Exporteer tekstlabels</translation>
    </message>
    <message>
        <source>Fields description:
* Input DXF file: path to the DXF file to be converted
* Output Shp file: desired name of the shape file to be created
* Shp output file type: specifies the type of the output shape file
* Export text labels checkbox: if checked, an additional shp points layer will be created,   and the associated dbf table will contain informations about the &quot;TEXT&quot; fields found in the dxf file, and the text strings themselves

---
Developed by Paolo L. Scala, Barbara Rita Barricelli, Marco Padula
CNR, Milan Unit (Information Technology), Construction Technologies Institute.
For support send a mail to scala@itc.cnr.it
</source>
        <translation type="obsolete">Veldbeschrijving:
* Input DXF-bestand: pad naar het te converteren DXF-bestand
* Output Shp-bestand: gewenste naam van het te genereren shp-bestand
* Shp output bestandstype: geeft het type van het te genereren shp-bestand aan
* Exporteer tekst-labels: als deze aangevinks is, zal er een extra shp puntenlaag worden aangemaakt. De bijbehorende dbf-tabel bevat informatie over de &quot;TEXT&quot;-velden uit het dxf-bestand en de teksten zelf.

---
Ontwikkeld door Paolo L. Scala, Barbara Rita Barricelli, Marco Padula
CNR, Milan Unit (Information Technology), Construction Technologies Institute.
Voor ondersteuning stuur email naar scala@itc.cnr.it</translation>
    </message>
    <message>
        <source>Choose a DXF file to open</source>
        <translation type="obsolete">Kies een DXF-bestand om te openen</translation>
    </message>
    <message>
        <source>Choose a file name to save to</source>
        <translation type="obsolete">Kies een bestandsnaam om op te slaan</translation>
    </message>
    <message>
        <source>Dxf Importer</source>
        <translation>Dxf Importer</translation>
    </message>
    <message>
        <source>Input Dxf file</source>
        <translation>Input Dxf-bestand</translation>
    </message>
    <message>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;DejaVu Sans&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;MS Shell Dlg 2&apos;; font-size:8pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;Output file&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;p, li { white-space: pre-wrap; }&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;DejaVu Sans&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;MS Shell Dlg 2&apos;; font-size:8pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;Output file&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Output file type</source>
        <translation>Output bestandstype</translation>
    </message>
</context>
<context>
    <name>eVis</name>
    <message>
        <source>Create layer from database query</source>
        <translation type="obsolete">Nieuwe laag aanmaken op basis van database-query</translation>
    </message>
    <message>
        <source>Tool for selecting indiviudual points</source>
        <translation type="obsolete">Gereedschap om individuele punten te selecteren</translation>
    </message>
    <message>
        <source>Open an Even Browser to explore the current layer</source>
        <translation type="obsolete">Open een &apos;Event Browser&apos; om de huidige laag te bekijken</translation>
    </message>
</context>
<context>
    <name>eVisDatabaseConnectionGui</name>
    <message>
        <source>Undefined</source>
        <translation type="obsolete">Ongedefinieërd</translation>
    </message>
    <message>
        <source>No predefined queries loaded</source>
        <translation type="obsolete">Geen voorgedefinieerde query&apos;s geladen</translation>
    </message>
    <message>
        <source>Open File</source>
        <translation type="obsolete">Bestand Openen</translation>
    </message>
    <message>
        <source>New Database connection requested...</source>
        <translation type="obsolete">Nieuwe databaseverbinding gevraagd...</translation>
    </message>
    <message>
        <source>Error: You must select a database type</source>
        <translation type="obsolete">Fout: Selecteer een database-type</translation>
    </message>
    <message>
        <source>Error: No host name entered</source>
        <translation type="obsolete">Fout: geen host opgegeven</translation>
    </message>
    <message>
        <source>Error: No username entered</source>
        <translation type="obsolete">Fout: Geen gebruikersnaam ingevoerd</translation>
    </message>
    <message>
        <source>Error: No password entered</source>
        <translation type="obsolete">Fout: Geen wachtwoord opgegeven</translation>
    </message>
    <message>
        <source>Error: No database name entered</source>
        <translation type="obsolete">Fout: Geen databasenaam ingevoerd</translation>
    </message>
    <message>
        <source>Connection to [</source>
        <translation type="obsolete">Verbinding naar [</translation>
    </message>
    <message>
        <source>] established.</source>
        <translation type="obsolete">] geslaagd.</translation>
    </message>
    <message>
        <source>connected</source>
        <translation type="obsolete">verbonden</translation>
    </message>
    <message>
        <source>] failed: </source>
        <translation type="obsolete">] mislukt:  </translation>
    </message>
    <message>
        <source>XML (*.xml)</source>
        <translation type="obsolete">XML (*.xml)</translation>
    </message>
    <message>
        <source>Error: Parse error at line %1, column %2: %3</source>
        <translation type="obsolete">Fout: Lees/parseerfout in regel %1, kolom %2: %3</translation>
    </message>
    <message>
        <source>Error: Unabled to open file [</source>
        <translation type="obsolete">Fout bij het openen van het bestand [</translation>
    </message>
    <message>
        <source>MSAccess (*.mdb)</source>
        <translation type="obsolete">MSAccess (*.mdb)</translation>
    </message>
    <message>
        <source>Sqlite (*.db)</source>
        <translation type="obsolete">Sqlite (*.db)</translation>
    </message>
    <message>
        <source>Error: Query failed: </source>
        <translation type="obsolete">Fout: Query mislukt: </translation>
    </message>
    <message>
        <source>Error: Counld not create temporary file, process halted</source>
        <translation type="obsolete">Fout: Kan geen tijdelijk bestand aanmaken, uitvoering gestopt</translation>
    </message>
    <message>
        <source>Error: A database connection is not currently established</source>
        <translation type="obsolete">Fout: Er is op dit moment geen verbinding met de database</translation>
    </message>
</context>
<context>
    <name>eVisDatabaseConnectionGuiBase</name>
    <message>
        <source>Database Connection</source>
        <translation type="obsolete">Database Verbinding</translation>
    </message>
    <message>
        <source>Predfined Queries</source>
        <translation type="obsolete">Voorgedefinieerde Qeury&apos;s</translation>
    </message>
    <message>
        <source>Load predefined queries</source>
        <translation type="obsolete">Lezen van voorgedefinieerde query&apos;s</translation>
    </message>
    <message>
        <source>Loads an XML file with predefined queries. Use the Open File window to locate the XML file that contains one or more predefined queries using the format described in the user guide.</source>
        <translation type="obsolete">Inlezen van een XML-bestand met voorgedefinieerde query&apos;s. Gebruik het Bestand Openen venster om een XML-bestand te selecteren met een of meer query&apos;s (in het formaat zoals in de handleiding beschreven).</translation>
    </message>
    <message>
        <source>The description of the selected query.</source>
        <translation type="obsolete">Beschrijving van de geselecteerde query.</translation>
    </message>
    <message>
        <source>Select the predefined query you want to use from the drop-down list containing queries identified form the file loaded using the Open File icon above. To run the query you need to click on the SQL Query tab. The query will be automatically entered in the query window.</source>
        <translation type="obsolete">Selecteer een voorgedefineerde query om te gebruiken uit de lijst opgebouwd uit het bestand geladen via het Bestand Openen icoon erboven. Om de query uit te voeren, klik op de SQL Query tab. De query zal dan automatisch worden ingevoerd in het query-venster.</translation>
    </message>
    <message>
        <source>not connected</source>
        <translation type="obsolete">Niet verbonden</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;Connection Status: &lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;p, li { white-space: pre-wrap; }&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;Verbindingsstatus: &lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Database Host</source>
        <translation type="obsolete">Database Host</translation>
    </message>
    <message encoding="UTF-8">
        <source>Enter the database host. If the database resides on your desktop you should enter ¨localhost¨. If you selected ¨MSAccess¨ as the database type this option will not be available. </source>
        <translation type="obsolete">Voer de hostnaam van de database in. Als de database zich op uw desktop bevindt, gebruik dan &quot;localhost&quot;. Als u &quot;MSAccess&quot; als database-type heeft gekozen zal deze optie niet beschikbaar zijn. </translation>
    </message>
    <message>
        <source>Password to access the database.</source>
        <translation type="obsolete">Toegangswachtwoord voor de database.</translation>
    </message>
    <message>
        <source>Enter the name of the database.</source>
        <translation type="obsolete">Geef de naam van de database.</translation>
    </message>
    <message>
        <source>Username</source>
        <translation type="obsolete">Gebruikersnaam</translation>
    </message>
    <message>
        <source>Enter the port through which the database must be accessed if a MYSQL database is used.</source>
        <translation type="obsolete">Voer het poortnummer in via welke de database verbinding moet lopen bij gebruik van een MYSQL database.</translation>
    </message>
    <message>
        <source>Connect to the database using the parameters selected above. If the connection was successful a message will be displayed in the Output Console below saying the connection was established. </source>
        <translation type="obsolete">Maak een verbinding met de database met de hierboven geselecteerde parameters. Als de verbinding slaagt zal een bericht getoond worden in de &apos;Output Console&apos; hieronder met de melding dat een verbinding is opgezet. </translation>
    </message>
    <message>
        <source>Connect</source>
        <translation type="obsolete">Verbinden</translation>
    </message>
    <message>
        <source>User name to access the database.</source>
        <translation type="obsolete">Database-gebruikersnaam.</translation>
    </message>
    <message>
        <source>Select the type of database from the list of supported databases in the drop-down menu.</source>
        <translation type="obsolete">Selecteer een database-type van de lijst van ondersteunde database-typen in het menu.</translation>
    </message>
    <message>
        <source>Database Name</source>
        <translation type="obsolete">Database Naam</translation>
    </message>
    <message>
        <source>Password</source>
        <translation type="obsolete">Wachtwoord</translation>
    </message>
    <message>
        <source>Database Type</source>
        <translation type="obsolete">Database Type</translation>
    </message>
    <message>
        <source>Port</source>
        <translation type="obsolete">Poort</translation>
    </message>
    <message>
        <source>SQL Query</source>
        <translation type="obsolete">SQL Query</translation>
    </message>
    <message>
        <source>Run the query entered above. The status of the query will be displayed in the Output  Console below.</source>
        <translation type="obsolete">Voer de query hierboven uit. De uitvoerstatus van de query zal worden getoond in de &apos;Output Console&apos; hieronder.</translation>
    </message>
    <message>
        <source>Run Query</source>
        <translation type="obsolete">Voer Query uit</translation>
    </message>
    <message>
        <source>Enter the query you want to run in this window.</source>
        <translation type="obsolete">Voer de uit te voeren query in dit venster in.</translation>
    </message>
    <message>
        <source>A window for status messages to be displayed.</source>
        <translation type="obsolete">Een venster om statusmeldingen te tonen.</translation>
    </message>
    <message>
        <source>Output Console</source>
        <translation type="obsolete">Output Console</translation>
    </message>
</context>
<context>
    <name>eVisDatabaseLayerFieldSelectionGuiBase</name>
    <message>
        <source>Database File Selection</source>
        <translation type="obsolete">Database Bestand Selectie</translation>
    </message>
    <message>
        <source>The name of the field that contains the Y coordinate of the points.</source>
        <translation type="obsolete">De naam van het veld welke de Y-coördinaten van de punten bevat.</translation>
    </message>
    <message>
        <source>The name of the field that contains the X coordinate of the points.</source>
        <translation type="obsolete">De naam van het veld welke de X-coördinaten van de punten bevat.</translation>
    </message>
    <message>
        <source>Enter the name for the new layer that will be created and displayed in QGIS.</source>
        <translation type="obsolete">Geef de naam voor de nieuwe laag die zal worden aangemaakt en worden getoond in QGIS.</translation>
    </message>
    <message>
        <source>Y Coordinate</source>
        <translation type="obsolete">Y-coördinaat</translation>
    </message>
    <message>
        <source>X Coordinate</source>
        <translation type="obsolete">X-coördinaat</translation>
    </message>
    <message>
        <source>Name of New Layer</source>
        <translation type="obsolete">Naam voor de Nieuwe Laag</translation>
    </message>
</context>
<context>
    <name>eVisGenericEventBrowserGui</name>
    <message>
        <source>Generic Event Browser</source>
        <translation type="obsolete">Generiek Event Browser</translation>
    </message>
    <message>
        <source>Field</source>
        <translation type="obsolete">Veld</translation>
    </message>
    <message>
        <source>Value</source>
        <translation type="obsolete">Waarde</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation type="obsolete">Waarschuwing</translation>
    </message>
    <message>
        <source>This tool only supports vector data</source>
        <translation type="obsolete">Alleen vectordata wordt ondersteund</translation>
    </message>
    <message>
        <source>No active layers found</source>
        <translation type="obsolete">Geen aktieve laag gevonden</translation>
    </message>
    <message>
        <source>Error</source>
        <translation type="obsolete">Fout</translation>
    </message>
    <message>
        <source>Unable to connecto to either the map canvas or application interface</source>
        <translation type="obsolete">Problemen bij het verbinden met het &apos;mapcanvas&apos; of the &apos;application interface&apos;</translation>
    </message>
    <message>
        <source>NULL was returned instead of a valid feature</source>
        <translation type="obsolete">NULL-waarde ontvangen in plaats van een geldig object</translation>
    </message>
    <message>
        <source>Generic Event Browser - Displaying records 01 of 0%n</source>
        <translation type="obsolete">
        </translation>
    </message>
    <message>
        <source>Generic Event Browser - Displaying records 01 of %n</source>
        <translation type="obsolete">
        </translation>
    </message>
    <message>
        <source>Attribute Contents</source>
        <translation type="obsolete">Attributen Inhoud</translation>
    </message>
    <message>
        <source>Generic Event Browser - Displaying records </source>
        <translation type="obsolete">Generiek Event Browser - Records tonen </translation>
    </message>
    <message>
        <source>of</source>
        <translation type="obsolete">van</translation>
    </message>
    <message>
        <source>Select Application</source>
        <translation type="obsolete">Selecteer Toepassing</translation>
    </message>
    <message>
        <source>All (*)</source>
        <translation type="obsolete">Alles (*)</translation>
    </message>
</context>
<context>
    <name>eVisGenericEventBrowserGuiBase</name>
    <message>
        <source>Display</source>
        <translation type="obsolete">Tonen</translation>
    </message>
    <message>
        <source>Use the Previous button to display the previous photo when more than one photo is available for display.</source>
        <translation type="obsolete">Gebruik de Vorige-knop om de vorige foto te tonen wanneer er meer dan 1 foto voorhanden is.</translation>
    </message>
    <message>
        <source>Previous</source>
        <translation type="obsolete">Vorige</translation>
    </message>
    <message>
        <source>Use the Next button to display the next photo when more than one photo is available for display.</source>
        <translation type="obsolete">Gebruik de Volgende-knop om de volgende foto te tonen wanneer er meer dan 1 foto voorhanden is.</translation>
    </message>
    <message>
        <source>Next</source>
        <translation type="obsolete">Volgende</translation>
    </message>
    <message encoding="UTF-8">
        <source>All of the attribute information for the point associated with the photo being viewed is displayed here. If the file type being referenced in the displayed record is not an image but is of a file type defined in the “Configure External Applications” tab then when you double-click on the value of the field containing the path to the file the application to open the file will be launched to view or hear the contents of the file. If the file extension is recognized the attribute data will be displayed in green.</source>
        <translation type="obsolete">Alle attribuutinformatie voor het punt van de foto wordt hier getoond. Als het gerefereerde bestandstype in het record geen afbeelding is, maar een als type zoals gedefinieerd onder de tab &quot;Externe Toepassingen Configureren&quot;, dan zal bij het dubbelklikken van de waarde de bijbehorende toepassing worden geopend om de inhoud van het bestand te bekijken of beluisteren. Als de extensie van het bestand wordt herkend zal de attribuutinformatie groen worden weergegeven.</translation>
    </message>
    <message>
        <source>1</source>
        <translation type="obsolete">1</translation>
    </message>
    <message>
        <source>Image display area</source>
        <translation type="obsolete">Gebied om Afbeeldingen te tonen</translation>
    </message>
    <message>
        <source>Display area for the image.</source>
        <translation type="obsolete">Gebied om Afbeeldingen te vertonen.</translation>
    </message>
    <message>
        <source>Options</source>
        <translation type="obsolete">Options</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-weight:600;&quot;&gt;Save?&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;p, li { white-space: pre-wrap; }&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-weight:600;&quot;&gt;Opslaan?&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Attribute Containing Path to File</source>
        <translation type="obsolete">Attribuut met Bestandspad</translation>
    </message>
    <message>
        <source>Use the drop-down list to select the field containing a directory path to the image. This can be an absolute or relative path.</source>
        <translation type="obsolete">Gebruik de lijst om een veld met het pad naar de directory van de afbeelding te selecteren. Dit can een absoluut of een relatief pad zijn.</translation>
    </message>
    <message encoding="UTF-8">
        <source>If checked the path to the image will be defined appending the attribute in the field selected from the “Attribute Containing Path to Image” drop-down list to the “Base Path” defined below.  </source>
        <translation type="obsolete">Indien aangevinkt zal het pad naar de afbeelding worden opgebouwd uit de attribuutwaarde in het veld uit de &quot;Attribuut voor Pad naar Afbeelding&quot;-lijst en het &quot;Basispad&quot; zoals hieronder aangegeven.  </translation>
    </message>
    <message>
        <source>Path Is Relative</source>
        <translation type="obsolete">Pad Is Relatief</translation>
    </message>
    <message>
        <source>If checked, the relative path values will be saved for the next session.</source>
        <translation type="obsolete">Indien aangevinkt zal de waarde van relatieve pad worden opgeslagen voor een volgende sessie.</translation>
    </message>
    <message>
        <source>Reset to default</source>
        <translation type="obsolete">Terug naar standaardinstellingen</translation>
    </message>
    <message>
        <source>Resets the values on this line to the default setting.</source>
        <translation type="obsolete">Zet de waarden op deze regel terug naar de standaardinstellingen.</translation>
    </message>
    <message>
        <source>If checked an arrow pointing in the direction defined by the attribute in the field selected from the drop-down list to the right will be displayed in the QGIS window on top of the point for this image.</source>
        <translation type="obsolete">Indien aangevinkt zal een pijl bovenop het afbeeldingspunt worden afgebeeld in het QGIS-venster wijzend in de richting gedefinieerd door de waarde van het attribuut uit de lijst rechts.</translation>
    </message>
    <message>
        <source>Display Compass Bearing</source>
        <translation type="obsolete">Toon Kompasrichting</translation>
    </message>
    <message>
        <source>Use the drop-down list to select the field containing the compass bearing for the image. This bearing usually references the direction the camera was pointing when the image was acquired. </source>
        <translation type="obsolete">Gebruik de lijst om een veld te selecteren met de kompasrichting voor de afbeelding. Deze richting wijst over het algemeen in de richting waar de kamera naar gericht was toen de afbeelding werd opgenomen. </translation>
    </message>
    <message>
        <source>If checked, the Display Compass Bearing values will be saved for the next session.</source>
        <translation type="obsolete">Indien aangevinkt zal de waarde voor &apos;Toon kompasrichting&apos; worden opgeslagen voor de volgende sessie.</translation>
    </message>
    <message>
        <source>Compass Offset</source>
        <translation type="obsolete">Kompas Afwijking</translation>
    </message>
    <message>
        <source>A value to be added to the compass bearing. This allows you to compensate for declination (adjust bearings collected using magnetic bearings to true north bearings). East declinations should be entered using positive values and west declinations should use negative values. </source>
        <translation type="obsolete">Toe te voegen waarde aan de kompasrichting. Dit maakt het mogelijk om te compenseren voor de declinatiehoek (verschil tussen het magnetische en ware noorden). Een oostelijke declinatiehoek worden ingevoerd met een positief getal, een westelijke hoek moet met een negatieve waarden worden ingevoerd. </translation>
    </message>
    <message>
        <source>Define the compass offset using a field from the vector layer attribute table.</source>
        <translation type="obsolete">Gebruik als kompasafwijking een veld uit de attribuuttabel van de vectorlaag.</translation>
    </message>
    <message>
        <source> From Attribute</source>
        <translation type="obsolete">   Afkomstig van Attributen</translation>
    </message>
    <message>
        <source>Use the drop-down list to select the field containing the compass bearing offset. This allows you to compensate for declination (adjust bearings collected using magnetic bearings to true north bearings). East declinations should be entered using positive values and west declinations should use negative values. </source>
        <translation type="obsolete">Gebruikde lijst om een veld voor de afwijking van de kompasrichting te selecteren. Dit maakt het mogelijk om te compenseren voor de declinatie (het verschil corrigeren tussen magnetische en ware noorden). Oostelijke declinaties moeten als positieve waarde worden ingevoerd, westelijke declinaties als negatieve waarden.</translation>
    </message>
    <message>
        <source>Define the compass offset manually.</source>
        <translation type="obsolete">Handmatige bepaling van de kompasafwijking.</translation>
    </message>
    <message>
        <source>Manual</source>
        <translation type="obsolete">Handmatig</translation>
    </message>
    <message>
        <source>If checked, the compass offset values will be saved for the next session.</source>
        <translation type="obsolete">Indien aangevinkt zal de kompasafwijking worden opgeslagen voor de volgende sessie.</translation>
    </message>
    <message>
        <source>Resets the compass offset values to the default settings.</source>
        <translation type="obsolete">Zet de kompatafwijking terug naar de standaardwaarde.</translation>
    </message>
    <message encoding="UTF-8">
        <source>The base path or url from which images and documents can be “relative”</source>
        <translation type="obsolete">Het basispad of url vanwaar afbeeldingen worden geladen kan relatief zijn</translation>
    </message>
    <message>
        <source>Base Path</source>
        <translation type="obsolete">Basispad</translation>
    </message>
    <message>
        <source>Base path</source>
        <translation type="obsolete">Basispad</translation>
    </message>
    <message>
        <source>The Base Path onto which the relative path defined above will be appended.</source>
        <translation type="obsolete">Het Basispad welke zal worden gevolgd door het relatieve pad zoals boven gedefinieerd.</translation>
    </message>
    <message>
        <source>If checked, the Base Path will be saved for the next session.</source>
        <translation type="obsolete">Indien aangevinkt als het Basispad worden opgeslagen voor de volgende sessie.</translation>
    </message>
    <message encoding="UTF-8">
        <source>Enters the default “Base Path” which is the  path to the directory of the vector layer containing the image information.</source>
        <translation type="obsolete">Geeft het &quot;standaard&quot; Basispad wat het   pad is naar de directory van de vectorlaag met de afbeeldingsinformatie.</translation>
    </message>
    <message>
        <source>If checked, the Base Path will append only the file name instead of the entire relative path (defined above) to create the full directory path to the file. </source>
        <translation type="obsolete">Indien aangevinkt zal aan het Basispad alleen de bestandsnaam worden toegevoegd in plaats van het gehele relatieve pad (zoals boven gegeven) om het volledige pad naar het bestand te geven. </translation>
    </message>
    <message>
        <source>Replace entire path/url stored in image path attribute with user defined
Base Path (i.e. keep only filename from attribute)</source>
        <translation type="obsolete">Vervang het gehele pad/url zoals opgeslagen in het afbeeldingspadattribuut door een
door de gebruiker gedefinieerd Basispad (b.v. gebruik alleen de bestandsnaam van de attribuutwaarde)</translation>
    </message>
    <message>
        <source>If checked, the  current check-box setting will be saved for the next session.</source>
        <translation type="obsolete">Indien aangevinkt zal de   waarde van het selectievakje worden opgeslagen voor de volgende sessie.</translation>
    </message>
    <message>
        <source>Clears the check-box on this line.</source>
        <translation type="obsolete">Maakt selectievakje op deze lijn leeg.</translation>
    </message>
    <message>
        <source>If checked, the same path rules that are defined  for images will be used for non-image documents such as movies, text documents, and sound files. If not checked the path rules will only apply to images and other documents will ignore the Base Path parameter.</source>
        <translation type="obsolete">indien aangevinkt zal het pad wat is gedefinieerd   voor afbeeldingen ook worden gebruikt voor niet-afbeeldingsbestanden zoals filmpjes, tekstdocumenten en geluidsbestanden. Indien niet geselecteerd zal de (pad)regel  alleen gelden voor afbeeldingen en zullen andere bestanden de Basispad-parameter negeren.</translation>
    </message>
    <message>
        <source>Apply Path to Image rules when loading docs in external applications</source>
        <translation type="obsolete">Gebruik &apos;Pad naar Afbeeldings&apos;-regels bij het laden van bestanden in externe programma&apos;s</translation>
    </message>
    <message encoding="UTF-8">
        <source>Clicking on Save will save the settings without closing the Options pane. Clicking on Restore Defaults will reset all of the fields to their default settings. It has the same effect as clicking all of the “Reset to default” buttons. </source>
        <translation type="obsolete">Klikken op &apos;Opslaan&apos; zal de instellingen bewaren zonder het &apos;Opties&apos;-scherm te sluiten. Klokken op &apos;Terug naar Standaardinstellingen&apos; zal alle velden terug zetten naar de standaardwaarden. Het heeft hetzelfde effect als klikken op de &apos;Terug naar standaard&apos;-knoppen.</translation>
    </message>
    <message>
        <source>Configure External Applications</source>
        <translation type="obsolete">Externe Programma&apos;s Configureren</translation>
    </message>
    <message>
        <source>File extension and external application in which to load a document of that type</source>
        <translation type="obsolete">Bestandsextensies en externe programma&apos;s waarmee bestanden van dat type zullen worden geladen</translation>
    </message>
    <message>
        <source>A table containing file types that can be opened using eVis. Each file type needs a file extension and the path to an application that can open that type of file. This provides the capability of opening a broad range of files such as movies, sound recording, and text documents instead of only images. </source>
        <translation type="obsolete">Een tabel met bestandstypen die kunnen worden geopend met eVis. Voor elk bestandstype is een bestandsextensie en pad naar een programma/toepassing nodig om dat type bestand te openen. Dit maakt het mogelijk om zeer veel verschillene soorten bestanden (zoals filmpjes, tekstdocumenten en geluidsbestanden) te kunnen openen in plaats van alleen afbeeldingen.</translation>
    </message>
    <message>
        <source>Extension</source>
        <translation type="obsolete">Extensie</translation>
    </message>
    <message>
        <source>Application</source>
        <translation type="obsolete">Programma</translation>
    </message>
    <message>
        <source>Add new file type</source>
        <translation type="obsolete">Nieuw bestandstype toevoegen</translation>
    </message>
    <message>
        <source>Add a new file type with a unique extension and the path for the application that can open the file.</source>
        <translation type="obsolete">Toevoegen van een nieuw bestandstype met een unieke extensie en een pad naar de programmatuur om dit bestand te openen.</translation>
    </message>
    <message>
        <source>Delete current row</source>
        <translation type="obsolete">Verwijder huidige regel</translation>
    </message>
    <message>
        <source>Delete the file type highlighted in the table and defined by a file extension and a path to an associated application.</source>
        <translation type="obsolete">Verwijder het bestandstype dat opgelicht/geselecteerd is in de tabel en gedefinieerd is bij een bestandsextensie en een pad naar de bijbehorende programmatuur.</translation>
    </message>
</context>
<context>
    <name>eVisImageDisplayWidget</name>
    <message>
        <source>Zoom in</source>
        <translation type="obsolete">Inzoomen</translation>
    </message>
    <message>
        <source>Zoom in to see more detal.</source>
        <translation type="obsolete">Inzoomen voor meer detail.</translation>
    </message>
    <message>
        <source>Zoom out</source>
        <translation type="obsolete">Uitzoomen</translation>
    </message>
    <message>
        <source>Zoom out to see more area.</source>
        <translation type="obsolete">Uitzoomen om meer gebied te zien.</translation>
    </message>
    <message>
        <source>Zoom to full extent</source>
        <translation type="obsolete">Uitzoomen Tot Maximale Extent</translation>
    </message>
    <message>
        <source>Zoom to display the entire image.</source>
        <translation type="obsolete">Zoomen om de gehele afbeelding te zien.</translation>
    </message>
</context>
<context>
    <name>pluginname</name>
    <message>
        <source>[menuitemname]</source>
        <translation>[menuitemname]</translation>
    </message>
    <message>
        <source>&amp;[menuname]</source>
        <translation>&amp;[menuname]</translation>
    </message>
    <message>
        <source>Replace this with a short description of the what the plugin does</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;(new line)
p, li { white-space: pre-wrap; }(new line)
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;(new line)
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;© QGIS 2008&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Replace this with a short description of what the plugin does</source>
        <translation>Vervang dit door een korte beschrijving van de functie van de plugin </translation>
    </message>
</context>
</TS>
