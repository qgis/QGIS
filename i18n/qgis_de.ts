<!DOCTYPE TS><TS>
<context>
    <name>CoordinateCapture</name>
    <message>
        <source>Coordinate Capture</source>
        <translation>Koordinaten abgreifen</translation>
    </message>
    <message>
        <source>Click on the map to view coordinates and capture to clipboard.</source>
        <translation>Klicken Sie auf die Karte um Koordinaten anzuzeigen und in die Zwischenanlage zu übertragen</translation>
    </message>
    <message>
        <source>&amp;Coordinate Capture</source>
        <translation>&amp;Koordinaten abgreifen</translation>
    </message>
    <message>
        <source>Click to select the CRS to use for coordinate display</source>
        <translation>Klicken Sie um das KBS zur Koordinatenanzeige auszuwählen</translation>
    </message>
    <message>
        <source>Coordinate in your selected CRS</source>
        <translation>Koordinate im gewählten Koordinatenbezugssystem</translation>
    </message>
    <message>
        <source>Coordinate in map canvas coordinate reference system</source>
        <translation>Koordinate im Koordinatenbezugssystem des Kartenbereichs</translation>
    </message>
    <message>
        <source>Copy to clipboard</source>
        <translation>In die Zwischenablage kopieren</translation>
    </message>
    <message>
        <source>Click to enable mouse tracking. Click the canvas to stop</source>
        <translation>Klicken um die Mausverfolgung zu aktivieren.  Zum Beenden in den Kartenbereich klicken</translation>
    </message>
</context>
<context>
    <name>CoordinateCaptureGui</name>
    <message>
        <source>Welcome to your automatically generated plugin!</source>
        <translation type="unfinished">Willkommen zu Ihrem automatisch erzeugten Plugin!</translation>
    </message>
    <message>
        <source>This is just a starting point. You now need to modify the code to make it do something useful....read on for a more information to get yourself started.</source>
        <translation type="unfinished">Dies ist erst der erste Schritt. Sie müssen nun den Quellcode anpassen, damit es etwas sinnvolles macht ... lesen Sie dazu weiter.</translation>
    </message>
    <message>
        <source>Documentation:</source>
        <translation type="unfinished">Dokumentation:</translation>
    </message>
    <message>
        <source>You really need to read the QGIS API Documentation now at:</source>
        <translation type="unfinished">Sie sollten nun unbedingt die QGIS API-Dokumentation lesen unter:</translation>
    </message>
    <message>
        <source>In particular look at the following classes:</source>
        <translation type="unfinished">Schauen Sie insbesondere nach den folgenden Klassen:</translation>
    </message>
    <message>
        <source>QgsPlugin is an ABC that defines required behaviour your plugin must provide. See below for more details.</source>
        <translation type="unfinished">QgsPlugin ist eine Grundlage, die das notwendige Verhalten Ihres Plugins definiert und bereitstellt. Lesen Sie weiter für mehr Details.</translation>
    </message>
    <message>
        <source>What are all the files in my generated plugin directory for?</source>
        <translation type="unfinished">Wozu dienen die ganzen Dateien in dem gerade erstellten Plugin-Ordner?</translation>
    </message>
    <message>
        <source>This is the generated CMake file that builds the plugin. You should add you application specific dependencies and source files to this file.</source>
        <translation type="unfinished">Dies ist die CMake-Datei, die den Plugin erstellt. Sie sollten die anwendungsspezifischen Abhängigkeiten und die Quelldateien in der CMake-Datei ergänzen.</translation>
    </message>
    <message>
        <source>This is the class that provides the &apos;glue&apos; between your custom application logic and the QGIS application. You will see that a number of methods are already implemented for you - including some examples of how to add a raster or vector layer to the main application map canvas. This class is a concrete instance of the QgisPlugin interface which defines required behaviour for a plugin. In particular, a plugin has a number of static methods and members so that the QgsPluginManager and plugin loader logic can identify each plugin, create an appropriate menu entry for it etc. Note there is nothing stopping you creating multiple toolbar icons and menu entries for a single plugin. By default though a single menu entry and toolbar button is created and its pre-configured to call the run() method in this class when selected. This default implementation provided for you by the plugin builder is well documented, so please refer to the code for further advice.</source>
        <translation type="unfinished">Dies ist die Klasse, die Ihre Applikation mit QGIS verbindet. Sie werden sehen, dass bereits eine Vielzahl an Methoden bereitstehen, inklusive einiger Beispiele, etwa wie Raster- oder Vektorlayer in das Kartenfenster integriert werden können. Diese Klasse ist eine feste Instanz des QGIS Plugin-Interfaces, welche notwendiges Verhalten des Plugins definiert. Genau gesagt, enthält ein Plugin eine Reihe statischer Methoden und Klassenmitglieder, damit der QGIS Pluginmanager und der Pluginlader jedes Plugin identifizieren, einen passenden Menüeintrag erstellen kann usw. Beachten Sie, dass Sie auch mehrere Icons für die Werkzeugleiste sowie mehere Menüeinträge für ein einzelnes Plugin erstellen können. Standardmässig wird jedoch ein einzelnes Icon und ein Menüeintrag erstellt und so vorkonfiguriert, dass die Methode run() dieser Klasse bei ihrer Auswahl gestarted wird. Diese durch den Pluginbuilder bereitgestellte Standardimplementierung ist sehr gut dokumentiert. Beziehen Sie sich daher bitte auf den Quellcode für weitere Hinweise.</translation>
    </message>
    <message>
        <source>This is a Qt designer &apos;ui&apos; file. It defines the look of the default plugin dialog without implementing any application logic. You can modify this form to suite your needs or completely remove it if your plugin does not need to display a user form (e.g. for custom MapTools).</source>
        <translation type="unfinished">Dies ist eine Qt Designer &apos;ui&apos; Datei. Sie definiert das Aussehen des Standard Plugindialogs ohne irgendeine Anwendungsfunktion. Sie können die Vorlage an Ihre Bedürfnisse anpassen oder auch löschen, wenn Ihr Plugin keinen Benutzerdialog braucht (z.B. für angepasste MapTools).</translation>
    </message>
    <message>
        <source>This is the concrete class where application logic for the above mentioned dialog should go. The world is your oyster here really....</source>
        <translation type="unfinished">Dies ist eine feste Klasse in welche die Applikationstechnologie des oben beschriebenen Dialogs,eingefügt werden sollte Die Welt steht Ihnen an dieser Stelle absolut offen....</translation>
    </message>
    <message>
        <source>This is the Qt4 resources file for your plugin. The Makefile generated for your plugin is all set up to compile the resource file so all you need to do is add your additional icons etc using the simple xml file format. Note the namespace used for all your resources e.g. (&apos;:/Homann/&apos;). It is important to use this prefix for all your resources. We suggest you include any other images and run time data in this resurce file too.</source>
        <translation type="unfinished">Dies ist die Qt4-Resourcendatei für Ihren plugin. Der für Ihr Plugin erstellte Makefile ist erstellt, um die Resourcendatei zu kompilieren. Alles was Sie hier tun müssen, ist die zusätzlichen Icons usw. mit Hilfe des einfachen XML-Formates zu ergänzen. Beachten Sie, die Namensräume für Ihre Resourcen z.B. (&apos;:/Homann/&apos;). Es ist wichtig diesen Prefix für all Ihre Resourcen zu verwenden. Wir schlagen vor, Sie bauen ein irgendwelche anderen Bilder und Laufzeitdaten in die Resourcendatei ein.</translation>
    </message>
    <message>
        <source>This is the icon that will be used for your plugin menu entry and toolbar icon. Simply replace this icon with your own icon to make your plugin disctinctive from the rest.</source>
        <translation type="unfinished">Dies ist das Icon, dass für Ihr Pluginmenü und die Werkzeugleiste benutzt wird. Ersetzen Sie das Icon einfach durch ihr eigenes, um Ihr Plugin von den anderen zu unterscheiden.</translation>
    </message>
    <message>
        <source>This file contains the documentation you are reading now!</source>
        <translation type="unfinished">Diese Datei enthält die Dokumentation, die Sie gerade lesen!</translation>
    </message>
    <message>
        <source>Getting developer help:</source>
        <translation type="unfinished">Entwickler-Hilfe bekommen:</translation>
    </message>
    <message>
        <source>For Questions and Comments regarding the plugin builder template and creating your features in QGIS using the plugin interface please contact us via:</source>
        <translation type="unfinished">Für Fragen und Kommentare in Bezug auf das &apos;Plugin-Builder&apos; Template und die Erstellung eigener Funktionen in QGIS mit Hilfe des Plugin-Interfaces kontaktieren Sie uns bitter unter:</translation>
    </message>
    <message>
        <source>&lt;li&gt; the QGIS developers mailing list, or &lt;/li&gt;&lt;li&gt; IRC (#qgis on freenode.net)&lt;/li&gt;</source>
        <translation type="unfinished">&lt;li&gt; Die QGIS Entwickler-Mailingliste, oder &lt;/li&gt;&lt;li&gt; IRC (#qgis on freenode.net)&lt;/li&gt;</translation>
    </message>
    <message>
        <source>QGIS is distributed under the Gnu Public License. If you create a useful plugin please consider contributing it back to the community.</source>
        <translation type="unfinished">QGIS ist veröffentlicht unter der GNU General Public License. Wenn Sie ein nützliches Plugin erstellt haben, überlegen Sie bitte, es der Community bereitzustellen.</translation>
    </message>
    <message>
        <source>Have fun and thank you for choosing QGIS.</source>
        <translation type="unfinished">Viel Spass und danke, dass Sie sich für QGIS entschieden haben.</translation>
    </message>
</context>
<context>
    <name>CoordinateCaptureGuiBase</name>
    <message>
        <source>QGIS Plugin Template</source>
        <translation type="unfinished">QGIS Plugin-Vorlage</translation>
    </message>
    <message>
        <source>Plugin Template</source>
        <translation type="unfinished">Plugin-Vorlage</translation>
    </message>
</context>
<context>
    <name>Dialog</name>
    <message>
        <source>QGIS Plugin Installer</source>
        <translation type="obsolete">QGIS Plugin Installation</translation>
    </message>
    <message>
        <source>Name of plugin to install</source>
        <translation type="obsolete">Name des zu installierenden Plugins</translation>
    </message>
    <message>
        <source>Get List</source>
        <translation type="obsolete">Hole Liste</translation>
    </message>
    <message>
        <source>Done</source>
        <translation type="obsolete">Fertig</translation>
    </message>
    <message>
        <source>Install Plugin</source>
        <translation type="obsolete">Plugin-Installation</translation>
    </message>
    <message>
        <source>The plugin will be installed to ~/.qgis/python/plugins</source>
        <translation type="obsolete">Das Plugin wird unter ~/.qgis/python/plugins installiert</translation>
    </message>
    <message>
        <source>Name</source>
        <translation type="obsolete">Name</translation>
    </message>
    <message>
        <source>Version</source>
        <translation type="obsolete">Version</translation>
    </message>
    <message>
        <source>Description</source>
        <translation type="obsolete">Beschreibung</translation>
    </message>
    <message>
        <source>Author</source>
        <translation type="obsolete">Autor</translation>
    </message>
    <message>
        <source>Select repository, retrieve the list of available plugins, select one and install it</source>
        <translation type="obsolete">Wähle ein Repository, empfange die Liste mit vorhandenen Plugins und installiere eines davon.</translation>
    </message>
    <message>
        <source>Repository</source>
        <translation type="obsolete">Repository</translation>
    </message>
    <message>
        <source>Active repository:</source>
        <translation type="obsolete">Aktives Repository:</translation>
    </message>
    <message>
        <source>Add</source>
        <translation type="obsolete">Hinzufügen</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation type="obsolete">Bearbeiten</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation type="obsolete">Löschen</translation>
    </message>
    <message>
        <source>Dialog</source>
        <translation type="obsolete">Dialog</translation>
    </message>
    <message>
        <source>Point Symbol</source>
        <translation type="obsolete">Punktsymbol </translation>
    </message>
    <message>
        <source>Size</source>
        <translation type="obsolete">Grösse</translation>
    </message>
    <message>
        <source>Area scale field</source>
        <translation type="obsolete">Flächenmaßstabs Feld</translation>
    </message>
    <message>
        <source>Rotation field</source>
        <translation type="obsolete">Rotationsfeld</translation>
    </message>
    <message>
        <source>Style Options</source>
        <translation type="obsolete">Stiloption</translation>
    </message>
    <message>
        <source>Outline style</source>
        <translation type="obsolete">Umrandungsstil</translation>
    </message>
    <message>
        <source>Outline color</source>
        <translation type="obsolete">Umrandungsfarbe
</translation>
    </message>
    <message>
        <source>Outline width</source>
        <translation type="obsolete">Umrandungsbreite</translation>
    </message>
    <message>
        <source>Fill color</source>
        <translation type="obsolete">Füllfarbe</translation>
    </message>
    <message>
        <source>Fill style</source>
        <translation type="obsolete">Füllstil</translation>
    </message>
    <message>
        <source>...</source>
        <translation type="obsolete">...</translation>
    </message>
    <message>
        <source>Connect</source>
        <translation type="unfinished">Verbinden</translation>
    </message>
    <message>
        <source>Browse</source>
        <translation type="unfinished">Durchsuchen</translation>
    </message>
    <message>
        <source>OGR Converter</source>
        <translation type="unfinished">OGR-Konverter</translation>
    </message>
    <message>
        <source>Could not establish connection to: &apos;</source>
        <translation type="unfinished">Konnte keine Verbindung herstellen zu: &apos;</translation>
    </message>
    <message>
        <source>Open OGR file</source>
        <translation type="unfinished">OGR-Datei öffnen</translation>
    </message>
    <message>
        <source>OGR File Data Source (*.*)</source>
        <translation type="unfinished">OGR-Quelldatei (*.*)</translation>
    </message>
    <message>
        <source>Open Directory</source>
        <translation type="unfinished">Verzeichnis öffnen</translation>
    </message>
    <message>
        <source>Input OGR dataset is missing!</source>
        <translation type="unfinished">OGR-Quelle fehlt!</translation>
    </message>
    <message>
        <source>Input OGR layer name is missing!</source>
        <translation type="unfinished">OGR-Layername fehlt!</translation>
    </message>
    <message>
        <source>Target OGR format not selected!</source>
        <translation type="unfinished">OGR-Zielformat nicht gewählt</translation>
    </message>
    <message>
        <source>Output OGR dataset is missing!</source>
        <translation type="unfinished">OGR-Ziel fehlt!</translation>
    </message>
    <message>
        <source>Output OGR layer name is missing!</source>
        <translation type="unfinished">Name des OGR-Ausgabelayers fehlt!</translation>
    </message>
    <message>
        <source>Successfully translated layer &apos;</source>
        <translation type="unfinished">Layer wurde erfolgreich konvertiert: &apos;</translation>
    </message>
    <message>
        <source>Failed to translate layer &apos;</source>
        <translation type="unfinished">Konvertierung des Layer gescheitert: &apos;</translation>
    </message>
    <message>
        <source>Successfully connected to: &apos;</source>
        <translation type="unfinished">Erfolgreich verbunden mit &apos;</translation>
    </message>
    <message>
        <source>Choose a file name to save to</source>
        <translation type="unfinished">Dateiname zum Speichern wählen</translation>
    </message>
</context>
<context>
    <name>Gui</name>
    <message>
        <source>Welcome to your automatically generated plugin!</source>
        <translation>Willkommen zu Ihrem automatisch installierten Plugin!</translation>
    </message>
    <message>
        <source>This is just a starting point. You now need to modify the code to make it do something useful....read on for a more information to get yourself started.</source>
        <translation>Dies ist erst der erste Schritt. Sie müssen nun den Quellcode anpassen, damit es etwas sinnvolles macht ... lesen Sie dazu weiter.</translation>
    </message>
    <message>
        <source>Documentation:</source>
        <translation>Dokumentation:</translation>
    </message>
    <message>
        <source>You really need to read the QGIS API Documentation now at:</source>
        <translation>Sie sollten nun unbedingt die QGIS API-Dokumentation lesen unter:</translation>
    </message>
    <message>
        <source>In particular look at the following classes:</source>
        <translation>Schauen Sie insbesondere nach den folgenden Klassen:</translation>
    </message>
    <message>
        <source>QgsPlugin is an ABC that defines required behaviour your plugin must provide. See below for more details.</source>
        <translation>QgsPlugin ist eine Grundlage, die das notwendige Verhalten Ihres Plugins definiert und bereitstellt. Lesen Sie weiter für mehr Details.</translation>
    </message>
    <message>
        <source>What are all the files in my generated plugin directory for?</source>
        <translation>Wozu sind die ganzen Dateien in dem gerade erstellten Plugin-Ordner nützlich?</translation>
    </message>
    <message>
        <source>This is the generated CMake file that builds the plugin. You should add you application specific dependencies and source files to this file.</source>
        <translation>Dies ist die CMake-Datei, die den Plugin erstellt. Sie sollten die anwendungsspezifischen Abhängigkeiten und die Quelldateien in der CMake-Datei ergänzen.</translation>
    </message>
    <message>
        <source>This is the class that provides the &apos;glue&apos; between your custom application logic and the QGIS application. You will see that a number of methods are already implemented for you - including some examples of how to add a raster or vector layer to the main application map canvas. This class is a concrete instance of the QgisPlugin interface which defines required behaviour for a plugin. In particular, a plugin has a number of static methods and members so that the QgsPluginManager and plugin loader logic can identify each plugin, create an appropriate menu entry for it etc. Note there is nothing stopping you creating multiple toolbar icons and menu entries for a single plugin. By default though a single menu entry and toolbar button is created and its pre-configured to call the run() method in this class when selected. This default implementation provided for you by the plugin builder is well documented, so please refer to the code for further advice.</source>
        <translation>Dies ist die Klasse, die Ihre Applikation mit QGIS verbindet. Sie werden sehen, dass bereits eine Vielzahl an Methoden bereitstehen, inklusive einiger Beispiele, etwa wie Raster- oder Vektorlayer in das Kartenfenster integriert werden können. Diese Klasse ist eine feste Instanz des QGIS Plugin-Interfaces, welche notwendiges Verhalten des Plugins definiert. Genau gesagt, enthält ein Plugin eine Reihe statischer Methoden und Klassenmitglieder, damit der QGIS Pluginmanager und der Pluginlader jedes Plugin identifizieren, einen passenden Menüeintrag erstellen kann usw. Beachten Sie, dass Sie auch mehrere Icons für die Werkzeugleiste sowie mehere Menüeinträge für ein einzelnes Plugin erstellen können. Standardmässig wird jedoch ein einzelnes Icon und ein Menüeintrag erstellt und so vorkonfiguriert, dass die Methode run() dieser Klasse bei ihrer Auswahl gestarted wird. Diese durch den Pluginbuilder bereitgestellte Standardimplementierung ist sehr gut dokumentiert. Beziehen Sie sich daher bitte auf den Quellcode für weitere Hinweise.</translation>
    </message>
    <message>
        <source>This is a Qt designer &apos;ui&apos; file. It defines the look of the default plugin dialog without implementing any application logic. You can modify this form to suite your needs or completely remove it if your plugin does not need to display a user form (e.g. for custom MapTools).</source>
        <translation>Dies ist eine Qt Designer &apos;ui&apos; Datei. Sie definiert das Aussehen des Standard Plugindialogs ohne irgendeine Anwendungsfunktion. Sie können die Vorlage an Ihre Bedürfnisse anpassen oder auch löschen, wenn Ihr Plugin keinen Benutzerdialog braucht (z.B. für angepasste MapTools).</translation>
    </message>
    <message>
        <source>This is the concrete class where application logic for the above mentioned dialog should go. The world is your oyster here really....</source>
        <translation>Dies ist eine feste Klasse in welche die Applikationstechnologie des oben beschriebenen Dialogs,eingefügt werden sollte Die Welt steht Ihnen an dieser Stelle absolut offen....</translation>
    </message>
    <message>
        <source>This is the Qt4 resources file for your plugin. The Makefile generated for your plugin is all set up to compile the resource file so all you need to do is add your additional icons etc using the simple xml file format. Note the namespace used for all your resources e.g. (&apos;:/Homann/&apos;). It is important to use this prefix for all your resources. We suggest you include any other images and run time data in this resurce file too.</source>
        <translation>Dies ist die Qt4-Resourcendatei für Ihren plugin. Der für Ihr Plugin erstellte Makefile ist erstellt, um die Resourcendatei zu kompilieren. Alles was Sie hier tun müssen, ist die zusätzlichen Icons usw. mit Hilfe des einfachen XML-Formates zu ergänzen. Beachten Sie, die Namensräume für Ihre Resourcen z.B. (&apos;:/Homann/&apos;). Es ist wichtig diesen Prefix für all Ihre Resourcen zu verwenden. Wir schlagen vor, Sie bauen ein irgendwelche anderen Bilder und Laufzeitdaten in die Resourcendatei ein.</translation>
    </message>
    <message>
        <source>This is the icon that will be used for your plugin menu entry and toolbar icon. Simply replace this icon with your own icon to make your plugin disctinctive from the rest.</source>
        <translation>Dies ist das Icon, dass für Ihr Pluginmenü und die Werkzeugleiste benutzt wird. Ersetzen Sie das Icon einfach durch ihr eigenes, um Ihr Plugin von den anderen zu unterscheiden.</translation>
    </message>
    <message>
        <source>This file contains the documentation you are reading now!</source>
        <translation>Diese Datei enthält die Dokumentation, die Sie gerade lesen!</translation>
    </message>
    <message>
        <source>Getting developer help:</source>
        <translation>Entwickler-Hilfe bekommen:</translation>
    </message>
    <message>
        <source>For Questions and Comments regarding the plugin builder template and creating your features in QGIS using the plugin interface please contact us via:</source>
        <translation>Für Fragen und Kommentare in Bezug auf das &apos;Plugin-Builder&apos; Template und die Erstellung eigener Funktionen in QGIS mit Hilfe des Plugin-Interfaces kontaktieren Sie uns bitter unter:</translation>
    </message>
    <message>
        <source>&lt;li&gt; the QGIS developers mailing list, or &lt;/li&gt;&lt;li&gt; IRC (#qgis on freenode.net)&lt;/li&gt;</source>
        <translation>&lt;li&gt; Die QGIS Entwickler-Mailingliste, oder &lt;/li&gt;&lt;li&gt; IRC (#qgis on freenode.net)&lt;/li&gt;</translation>
    </message>
    <message>
        <source>QGIS is distributed under the Gnu Public License. If you create a useful plugin please consider contributing it back to the community.</source>
        <translation>QGIS ist veröffentlicht unter der GNU General Public License. Wenn Sie ein nützliches Plugin erstellt haben, überlegen Sie bitte, es der Community bereitzustellen.</translation>
    </message>
    <message>
        <source>Have fun and thank you for choosing QGIS.</source>
        <translation>Viel Spass und danke, dass Sie sich für QGIS entschieden haben.</translation>
    </message>
</context>
<context>
    <name>MapCoordsDialogBase</name>
    <message>
        <source>Enter map coordinates</source>
        <translation>Kartenkoordinaten eingeben</translation>
    </message>
    <message>
        <source>X:</source>
        <translation>X:</translation>
    </message>
    <message>
        <source>Y:</source>
        <translation>Y:</translation>
    </message>
    <message>
        <source>&amp;OK</source>
        <translation>&amp;OK</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Abbrechen</translation>
    </message>
    <message>
        <source>Enter X and Y coordinates which correspond with the selected point on the image. Alternatively, click the button with icon of a pencil and then click a corresponding point on map canvas of QGIS to fill in coordinates of that point.</source>
        <translation>Klicken Sie auf den &apos;aus Karte&apos; Knopf und wählen Sie dann den korrespondierenden Punkt in der Hauptkarte per Klick aus, um die Koordinaten von dort zu übernehmen. Alternativ, können Sie die Koordinaten auch manuell eingeben.</translation>
    </message>
    <message>
        <source> from map canvas</source>
        <translation> aus Karte</translation>
    </message>
</context>
<context>
    <name>OgrConverterGuiBase</name>
    <message>
        <source>OGR Layer Converter</source>
        <translation type="unfinished">OGR-Layer-Konverter</translation>
    </message>
    <message>
        <source>Source</source>
        <translation type="unfinished">Quelle</translation>
    </message>
    <message>
        <source>Format</source>
        <translation type="unfinished">Format</translation>
    </message>
    <message>
        <source>File</source>
        <translation type="unfinished">Datei</translation>
    </message>
    <message>
        <source>Directory</source>
        <translation type="unfinished">Verzeichnis</translation>
    </message>
    <message>
        <source>Remote source</source>
        <translation type="unfinished">Entfernte Quelle</translation>
    </message>
    <message>
        <source>Dataset</source>
        <translation type="unfinished">Datensatz</translation>
    </message>
    <message>
        <source>Browse</source>
        <translation type="unfinished">Durchsuchen</translation>
    </message>
    <message>
        <source>Layer</source>
        <translation type="unfinished">Layer</translation>
    </message>
    <message>
        <source>Target</source>
        <translation type="unfinished">Ziel</translation>
    </message>
</context>
<context>
    <name>OgrPlugin</name>
    <message>
        <source>Run OGR Layer Converter</source>
        <translation type="unfinished">OGR-Layer-Konverter starten</translation>
    </message>
    <message>
        <source>Replace this with a short description of the what the plugin does</source>
        <translation type="obsolete">Diese Notiz mit einer kurzen Beschreibung, was das Plugin macht, ersetzen</translation>
    </message>
    <message>
        <source>OG&amp;R Converter</source>
        <translation type="unfinished">OG&amp;R-Konverter</translation>
    </message>
    <message>
        <source>Translates vector layers between formats supported by OGR library</source>
        <translation type="unfinished">Vektorlayer zwischen von der OGR-Bibliothek unterstützten Formaten umwandeln</translation>
    </message>
</context>
<context>
    <name>QFileDialog</name>
    <message>
        <source>Save experiment report to portable document format (.pdf)</source>
        <translation>Vorläufigen Bericht im &apos;Portable Document Format&apos; (.pdf) speichern</translation>
    </message>
    <message>
        <source>Load layer properties from style file (.qml)</source>
        <translation>Layereigenschaften aus der Style-Datei (.qml) laden</translation>
    </message>
    <message>
        <source>Save layer properties as style file (.qml)</source>
        <translation>Layereigenschaften als Style-Datei (.qml) speichern</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <source>No Data Providers</source>
        <translation>Keine Datenlieferanten</translation>
    </message>
    <message>
        <source>No Data Provider Plugins</source>
        <comment>No QGIS data provider plugins found in:</comment>
        <translation>Keine Datenlieferanten Plugins</translation>
    </message>
    <message>
        <source>No vector layers can be loaded. Check your QGIS installation</source>
        <translation>Es können keine Vektorlayer geladen werden. Bitte QGIS Installation überprüfen</translation>
    </message>
    <message>
        <source>No data provider plugins are available. No vector layers can be loaded</source>
        <translation>Keine Datenlieferanten-Plugins verfügbar. Es können keine Vektorlayer geladen werden</translation>
    </message>
    <message>
        <source>QGis files (*.qgs)</source>
        <translation>QGIS Dateien (*.qgs)</translation>
    </message>
    <message>
        <source> at line </source>
        <translation> in Zeile </translation>
    </message>
    <message>
        <source> column </source>
        <translation> Spalte </translation>
    </message>
    <message>
        <source> for file </source>
        <translation>Für Datei</translation>
    </message>
    <message>
        <source>Unable to save to file </source>
        <translation>Datei kann nicht gespeichert werden</translation>
    </message>
    <message>
        <source>Referenced column wasn&apos;t found: </source>
        <translation>Die Referenzspalte wurde nicht gefunden: </translation>
    </message>
    <message>
        <source>Division by zero.</source>
        <translation>Division durch Null.</translation>
    </message>
    <message>
        <source>No active layer</source>
        <translation>Keine aktiven Layer</translation>
    </message>
    <message>
        <source>Band</source>
        <translation>Kanal</translation>
    </message>
    <message>
        <source>action</source>
        <translation>Aktion</translation>
    </message>
    <message>
        <source> features found</source>
        <translation> Objekte gefunden.</translation>
    </message>
    <message>
        <source> 1 feature found</source>
        <translation> 1 Objekt gefunden.</translation>
    </message>
    <message>
        <source>No features found</source>
        <translation>Keine Objekte gefunden</translation>
    </message>
    <message>
        <source>No features were found in the active layer at the point you clicked</source>
        <translation>Keine Objekte im aktiven Layer am gewählten Punkt gefunden</translation>
    </message>
    <message>
        <source>Could not identify objects on</source>
        <translation>Konnte Objekte nicht identifizieren auf</translation>
    </message>
    <message>
        <source>because</source>
        <translation>weil</translation>
    </message>
    <message>
        <source>New centroid</source>
        <translation>Neues Zentroid</translation>
    </message>
    <message>
        <source>New point</source>
        <translation>Neuer Punkt</translation>
    </message>
    <message>
        <source>New vertex</source>
        <translation>Neuer Vertex</translation>
    </message>
    <message>
        <source>Undo last point</source>
        <translation>Undo letzter Punkt</translation>
    </message>
    <message>
        <source>Close line</source>
        <translation>Linie schliessen</translation>
    </message>
    <message>
        <source>Select vertex</source>
        <translation>Vertex wählen</translation>
    </message>
    <message>
        <source>Select new position</source>
        <translation>Neue Position wählen</translation>
    </message>
    <message>
        <source>Select line segment</source>
        <translation>Liniensegment wählen</translation>
    </message>
    <message>
        <source>New vertex position</source>
        <translation>Neue Vertexposition</translation>
    </message>
    <message>
        <source>Release</source>
        <translation>Freigeben</translation>
    </message>
    <message>
        <source>Delete vertex</source>
        <translation>Lösche Vertex</translation>
    </message>
    <message>
        <source>Release vertex</source>
        <translation>Vertex freigeben</translation>
    </message>
    <message>
        <source>Select element</source>
        <translation>Element wählen</translation>
    </message>
    <message>
        <source>New location</source>
        <translation>Neue Location</translation>
    </message>
    <message>
        <source>Release selected</source>
        <translation>Ausgewähltes freigeben</translation>
    </message>
    <message>
        <source>Delete selected / select next</source>
        <translation>Auswahl löschen / nächstes wählen</translation>
    </message>
    <message>
        <source>Select position on line</source>
        <translation>Position auf Linie wählen</translation>
    </message>
    <message>
        <source>Split the line</source>
        <translation>Linie auftrennen</translation>
    </message>
    <message>
        <source>Release the line</source>
        <translation>Linie freigeben</translation>
    </message>
    <message>
        <source>Select point on line</source>
        <translation>Punkt in der Mitte wählen</translation>
    </message>
    <message>
        <source>Length</source>
        <translation>Länge</translation>
    </message>
    <message>
        <source>Area</source>
        <translation>Fläche</translation>
    </message>
    <message>
        <source>Project file read error: </source>
        <translation>Fehler beim Lesen der Projektdatei: </translation>
    </message>
    <message>
        <source>Label</source>
        <translation>Beschriftung</translation>
    </message>
    <message>
        <source>Fit to a linear transform requires at least 2 points.</source>
        <translation>Anpassung an eine lineare Transformation benötigt mindestens 2 Punkte.</translation>
    </message>
    <message>
        <source>Fit to a Helmert transform requires at least 2 points.</source>
        <translation>Eine Helmert-Transformation benötigt mindestens 2 Punkte.</translation>
    </message>
    <message>
        <source>Fit to an affine transform requires at least 4 points.</source>
        <translation>Anpassung an eine affine Transformation benötigt mindestens 4 Punkte.</translation>
    </message>
    <message>
        <source>Couldn&apos;t open the data source: </source>
        <translation>Kann die Datenquelle nicht öffnen.</translation>
    </message>
    <message>
        <source>Parse error at line </source>
        <translation>Interpretationsfehler in Linie</translation>
    </message>
    <message>
        <source>GPS eXchange format provider</source>
        <translation>GPS eXchange Format Provider</translation>
    </message>
    <message>
        <source>Caught a coordinate system exception while trying to transform a point. Unable to calculate line length.</source>
        <translation>Ein Problem beim Versuch einer Koordinatentransformation eines Punktes aus. Konnte daher die Linienlänge nicht berechnen.</translation>
    </message>
    <message>
        <source>Caught a coordinate system exception while trying to transform a point. Unable to calculate polygon area.</source>
        <translation>Ein Problem beim Versuch einer Koordinatentransformation eines Punktes aus. Konnte daher die Fläches des Polygons nicht berechnen.</translation>
    </message>
    <message>
        <source>GRASS plugin</source>
        <translation>GRASS plugin</translation>
    </message>
    <message>
        <source>QGIS couldn&apos;t find your GRASS installation.
Would you like to specify path (GISBASE) to your GRASS installation?</source>
        <translation>QGIS konnte Ihre GRASS-Installation nicht finden. Wollen Sie den Pfad zu Ihrer GRASS-Installation (GISBASE) angeben?</translation>
    </message>
    <message>
        <source>Choose GRASS installation path (GISBASE)</source>
        <translation>Bitte wählen Sie einen GRASS-Installationspfad (GISBASE)</translation>
    </message>
    <message>
        <source>GRASS data won&apos;t be available if GISBASE is not specified.</source>
        <translation>GRASS-Daten können nicht benutzt werden, wenn keine GISBASE definiert ist.</translation>
    </message>
    <message>
        <source>CopyrightLabel</source>
        <translation>CopyrightLabel</translation>
    </message>
    <message>
        <source>Draws copyright information</source>
        <translation>Zeichnet Urhebersrechtsinformationen</translation>
    </message>
    <message>
        <source>Version 0.1</source>
        <translation>Version 0.1</translation>
    </message>
    <message>
        <source>Version 0.2</source>
        <translation>Version 0.2</translation>
    </message>
    <message>
        <source>Loads and displays delimited text files containing x,y coordinates</source>
        <translation>Lädt und stellt Textdateien in CSV-Format, die x und y-Koordinaten haben, dar.</translation>
    </message>
    <message>
        <source>Add Delimited Text Layer</source>
        <translation>Layer aus Textdatei laden</translation>
    </message>
    <message>
        <source>Georeferencer</source>
        <translation>Georeferenzierer</translation>
    </message>
    <message>
        <source>Adding projection info to rasters</source>
        <translation>Fügt Projektionsinformationen zu Rasterdateien hinzu.</translation>
    </message>
    <message>
        <source>GPS Tools</source>
        <translation>GPS Werkzeuge</translation>
    </message>
    <message>
        <source>Tools for loading and importing GPS data</source>
        <translation>Werkzeuge zum Laden und Importieren von GPS-Daten.</translation>
    </message>
    <message>
        <source>GRASS</source>
        <translation>GRASS</translation>
    </message>
    <message>
        <source>GRASS layer</source>
        <translation>GRASS-Layer anzeigen und GRASS-Module auf Daten in GRASS Locations anwenden.</translation>
    </message>
    <message>
        <source>Graticule Creator</source>
        <translation>Gradnetz Generator</translation>
    </message>
    <message>
        <source>Builds a graticule</source>
        <translation>Erstellt ein Gradnetz</translation>
    </message>
    <message>
        <source>NorthArrow</source>
        <translation>Nordpfeil</translation>
    </message>
    <message>
        <source>Displays a north arrow overlayed onto the map</source>
        <translation>Stelle einen Nordpfeil auf der Kartendarstellung dar.</translation>
    </message>
    <message>
        <source>[menuitemname]</source>
        <translation>[menuitemname]</translation>
    </message>
    <message>
        <source>[plugindescription]</source>
        <translation>[plugindescription]</translation>
    </message>
    <message>
        <source>ScaleBar</source>
        <translation>Maßstab</translation>
    </message>
    <message>
        <source>Draws a scale bar</source>
        <translation>Zeichnet einen Maßstab</translation>
    </message>
    <message>
        <source>SPIT</source>
        <translation>SPIT</translation>
    </message>
    <message>
        <source>Shapefile to PostgreSQL/PostGIS Import Tool</source>
        <translation>Werkzeug zum Importieren von Shapes in PostgreSQL/PostGIS</translation>
    </message>
    <message>
        <source>WFS plugin</source>
        <translation>WFS-Plugin</translation>
    </message>
    <message>
        <source>Adds WFS layers to the QGIS canvas</source>
        <translation>Fügt einen WFS-Layer zur Kartendarstellung hinzu.</translation>
    </message>
    <message>
        <source>Not a vector layer</source>
        <translation>Keine Vektorlayer</translation>
    </message>
    <message>
        <source>The current layer is not a vector layer</source>
        <translation>Der aktuelle Layer ist kein Vektorlayer</translation>
    </message>
    <message>
        <source>Layer cannot be added to</source>
        <translation>Der Layer kann nicht hinzugefügt werden zu</translation>
    </message>
    <message>
        <source>The data provider for this layer does not support the addition of features.</source>
        <translation>Der Datenlieferant dieses Layers unterstützt das Hinzufügen von neuen Objekten nicht.</translation>
    </message>
    <message>
        <source>Layer not editable</source>
        <translation>Der Layer kann nicht bearbeitet werden</translation>
    </message>
    <message>
        <source>Cannot edit the vector layer. To make it editable, go to the file item of the layer, right click and check &apos;Allow Editing&apos;.</source>
        <translation>Der Vektorlayer kann nicht geändert werden. Um ihn zu bearbeiten, klicken Sie bitte erst mit der rechten Maustaste auf den Dateieintrag des Layers und dann auf &apos;Bearbeitungsstatus umschalten&apos;.</translation>
    </message>
    <message>
        <source>To select features, you must choose a vector layer by clicking on its name in the legend</source>
        <translation>Um Objekte zu selektieren, müssen Sie einen Vektorlayer durch anklicken in der Legende auswählen.</translation>
    </message>
    <message>
        <source>Python error</source>
        <translation type="unfinished">Python-Fehler</translation>
    </message>
    <message>
        <source>Couldn&apos;t load plugin </source>
        <translation>Plugin konnte nicht geladen werden</translation>
    </message>
    <message>
        <source> due an error when calling its classFactory() method</source>
        <translation> konnte durch einen Fehler beim Aufruf dessen classFactory()-Methode nicht laden</translation>
    </message>
    <message>
        <source> due an error when calling its initGui() method</source>
        <translation> konnte durch einen Fehler beim Aufruf dessen initGui()-Methode nicht laden.</translation>
    </message>
    <message>
        <source>Error while unloading plugin </source>
        <translation type="unfinished">Fehler beim Entladen des Plugins </translation>
    </message>
    <message>
        <source>2.5D shape type not supported</source>
        <translation>2,5D Shapetyp wird nicht unterstützt</translation>
    </message>
    <message>
        <source>Adding features to 2.5D shapetypes is not supported yet</source>
        <translation>Das Hinzufügen von 2.5D Shape Informationen wird zur Zeit nicht unterstützt</translation>
    </message>
    <message>
        <source>Wrong editing tool</source>
        <translation type="unfinished">Falsches Bearbeitungswerkzeug</translation>
    </message>
    <message>
        <source>Cannot apply the &apos;capture point&apos; tool on this vector layer</source>
        <translation>Das &apos;Punkt digitalisieren&apos;-Werkzeug kann nicht auf diesen Vektorlayer angewendet werden</translation>
    </message>
    <message>
        <source>Coordinate transform error</source>
        <translation type="unfinished">Koordinatentransformationsfehler</translation>
    </message>
    <message>
        <source>Cannot transform the point to the layers coordinate system</source>
        <translation>Konnte den Punkt nicht auf das Koordinatensystem des Layers transformieren.</translation>
    </message>
    <message>
        <source>Cannot apply the &apos;capture line&apos; tool on this vector layer</source>
        <translation>Das &apos;Linie digitalisieren&apos;-Werkzeug kann nicht auf diesen Vektorlayer angewendet werden</translation>
    </message>
    <message>
        <source>Cannot apply the &apos;capture polygon&apos; tool on this vector layer</source>
        <translation>Das &apos;Polygon digitalisieren&apos;-Werkzeug kann nicht auf diesen Vektorlayer angewendet werden.</translation>
    </message>
    <message>
        <source>Error</source>
        <translation type="unfinished">Fehler</translation>
    </message>
    <message>
        <source>Cannot add feature. Unknown WKB type</source>
        <translation type="unfinished">Konnte Objekt nicht hinzufügen. Unbekannter WKB-Typ</translation>
    </message>
    <message>
        <source>Error, could not add island</source>
        <translation>Fehler beim Hinzufügen des Insel-Polygons</translation>
    </message>
    <message>
        <source>A problem with geometry type occured</source>
        <translation>Es ist ein Problem mit dem Geometrietyp aufgetreten</translation>
    </message>
    <message>
        <source>The inserted Ring is not closed</source>
        <translation type="unfinished">Der eingefügte Ring ist nicht geschlossen</translation>
    </message>
    <message>
        <source>The inserted Ring is not a valid geometry</source>
        <translation type="unfinished">Der eingefügte Ring hat keine gültige Geometrie</translation>
    </message>
    <message>
        <source>The inserted Ring crosses existing rings</source>
        <translation type="unfinished">Der eingefügte Ring überschneidet sich mit vorhandenen Ringen</translation>
    </message>
    <message>
        <source>The inserted Ring is not contained in a feature</source>
        <translation type="unfinished">Der eingefügte Ring befindet sich nicht innerhalb des Objekts.</translation>
    </message>
    <message>
        <source>An unknown error occured</source>
        <translation type="unfinished">Ein unbekannter Fehler trat auf.</translation>
    </message>
    <message>
        <source>Error, could not add ring</source>
        <translation type="unfinished">Es ist ein Fehler beim Einfügen des Rings aufgetreten.</translation>
    </message>
    <message>
        <source> km2</source>
        <translation type="unfinished"> km2</translation>
    </message>
    <message>
        <source> ha</source>
        <translation type="unfinished"> ha</translation>
    </message>
    <message>
        <source> m2</source>
        <translation type="unfinished"> m2</translation>
    </message>
    <message>
        <source> m</source>
        <translation type="unfinished"> m</translation>
    </message>
    <message>
        <source> km</source>
        <translation type="unfinished"> km</translation>
    </message>
    <message>
        <source> mm</source>
        <translation type="unfinished"> mm</translation>
    </message>
    <message>
        <source> cm</source>
        <translation type="unfinished"> cm</translation>
    </message>
    <message>
        <source> sq mile</source>
        <translation type="unfinished"> Quadratmeile</translation>
    </message>
    <message>
        <source> sq ft</source>
        <translation type="unfinished"> Quadratfuß</translation>
    </message>
    <message>
        <source> mile</source>
        <translation type="unfinished"> Meilen</translation>
    </message>
    <message>
        <source> foot</source>
        <translation type="unfinished"> Fuß</translation>
    </message>
    <message>
        <source> feet</source>
        <translation type="unfinished"> Fuß</translation>
    </message>
    <message>
        <source> sq.deg.</source>
        <translation type="unfinished"> sq.deg.</translation>
    </message>
    <message>
        <source> degree</source>
        <translation type="unfinished"> Grad</translation>
    </message>
    <message>
        <source> degrees</source>
        <translation type="unfinished"> Grad</translation>
    </message>
    <message>
        <source> unknown</source>
        <translation type="unfinished"> unbekannt</translation>
    </message>
    <message>
        <source>Received %1 of %2 bytes</source>
        <translation type="unfinished">%1 von %2 Bytes empfangen.</translation>
    </message>
    <message>
        <source>Received %1 bytes (total unknown)</source>
        <translation type="unfinished">%1 Bytes empfangen (Gesamtzahl unbekannt)</translation>
    </message>
    <message>
        <source>Not connected</source>
        <translation type="unfinished">Nicht verbunden</translation>
    </message>
    <message>
        <source>Looking up &apos;%1&apos;</source>
        <translation type="unfinished">Löse &apos;%1&apos; auf</translation>
    </message>
    <message>
        <source>Connecting to &apos;%1&apos;</source>
        <translation type="unfinished">Verbinde mit &apos;%1&apos;</translation>
    </message>
    <message>
        <source>Sending request &apos;%1&apos;</source>
        <translation type="unfinished">Anfrage wird an &apos;%1&apos; gesandt</translation>
    </message>
    <message>
        <source>Receiving reply</source>
        <translation type="unfinished">Emfange Antwort</translation>
    </message>
    <message>
        <source>Response is complete</source>
        <translation type="unfinished">Antwort ist vollständig</translation>
    </message>
    <message>
        <source>Closing down connection</source>
        <translation type="unfinished">Verbindung wird geschlossen</translation>
    </message>
    <message>
        <source>Unable to open </source>
        <translation type="unfinished">Fehler beim Öffnen: </translation>
    </message>
    <message>
        <source>Regular expressions on numeric values don&apos;t make sense. Use comparison instead.</source>
        <translation type="unfinished">Reguläre Ausdrücke auf numerische Werte haben keinen Sinn. Bitte benutzen Sie Vergleichsoperatoren.</translation>
    </message>
    <message>
        <source>PostgreSQL Geoprocessing</source>
        <translation>PostgreSQL-Geodatenverarbeitung</translation>
    </message>
    <message>
        <source>Geoprocessing functions for working with PostgreSQL/PostGIS layers</source>
        <translation>Geodatenverarbeitungsfunktionen für PostgreSQL-/PostGIS-Layer</translation>
    </message>
    <message>
        <source>Location: </source>
        <translation>Location: </translation>
    </message>
    <message>
        <source>&lt;br&gt;Mapset: </source>
        <translation>&lt;br&gt;Mapset: </translation>
    </message>
    <message>
        <source>&lt;b&gt;Raster&lt;/b&gt;</source>
        <translation type="unfinished">&lt;b&gt;Raster&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Cannot open raster header</source>
        <translation type="unfinished">Header des Rasters konnte nicht geöffnet werden.</translation>
    </message>
    <message>
        <source>Rows</source>
        <translation type="unfinished">Zeilen</translation>
    </message>
    <message>
        <source>Columns</source>
        <translation type="unfinished">Spalten</translation>
    </message>
    <message>
        <source>N-S resolution</source>
        <translation type="unfinished">N-S Auflösung</translation>
    </message>
    <message>
        <source>E-W resolution</source>
        <translation type="unfinished">O-W Auflösung</translation>
    </message>
    <message>
        <source>North</source>
        <translation type="unfinished">Nord</translation>
    </message>
    <message>
        <source>South</source>
        <translation type="unfinished">Süd</translation>
    </message>
    <message>
        <source>East</source>
        <translation type="unfinished">Ost</translation>
    </message>
    <message>
        <source>West</source>
        <translation type="unfinished">West</translation>
    </message>
    <message>
        <source>Format</source>
        <translation type="unfinished">Format</translation>
    </message>
    <message>
        <source>Minimum value</source>
        <translation type="unfinished">Minimalwert</translation>
    </message>
    <message>
        <source>Maximum value</source>
        <translation type="unfinished">Maximalwert</translation>
    </message>
    <message>
        <source>Data source</source>
        <translation type="unfinished">Datenquelle</translation>
    </message>
    <message>
        <source>Data description</source>
        <translation type="unfinished">Datenbeschreibung</translation>
    </message>
    <message>
        <source>Comments</source>
        <translation type="unfinished">Kommentare</translation>
    </message>
    <message>
        <source>Categories</source>
        <translation type="unfinished">Kategorien</translation>
    </message>
    <message>
        <source>&lt;b&gt;Vector&lt;/b&gt;</source>
        <translation type="unfinished">&lt;b&gt;Vektor&lt;/b;&gt;</translation>
    </message>
    <message>
        <source>Points</source>
        <translation type="unfinished">Punkte</translation>
    </message>
    <message>
        <source>Lines</source>
        <translation type="unfinished">Zeilen</translation>
    </message>
    <message>
        <source>Boundaries</source>
        <translation type="unfinished">Grenzen</translation>
    </message>
    <message>
        <source>Centroids</source>
        <translation>Zentroide</translation>
    </message>
    <message>
        <source>Faces</source>
        <translation type="unfinished">Oberflächen</translation>
    </message>
    <message>
        <source>Kernels</source>
        <translation type="unfinished">Kerne</translation>
    </message>
    <message>
        <source>Areas</source>
        <translation type="unfinished">Bereiche</translation>
    </message>
    <message>
        <source>Islands</source>
        <translation type="unfinished">Inseln</translation>
    </message>
    <message>
        <source>Top</source>
        <translation type="unfinished">Oben</translation>
    </message>
    <message>
        <source>Bottom</source>
        <translation type="unfinished">Unten</translation>
    </message>
    <message>
        <source>yes</source>
        <translation type="unfinished">ja</translation>
    </message>
    <message>
        <source>no</source>
        <translation type="unfinished">nein</translation>
    </message>
    <message>
        <source>History&lt;br&gt;</source>
        <translation type="unfinished">Geschichte&lt;br&gt;</translation>
    </message>
    <message>
        <source>&lt;b&gt;Layer&lt;/b&gt;</source>
        <translation>&lt;b&gt;Layer&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Features</source>
        <translation>Objekte</translation>
    </message>
    <message>
        <source>Driver</source>
        <translation type="unfinished">Treiber</translation>
    </message>
    <message>
        <source>Database</source>
        <translation type="unfinished">Datenbank</translation>
    </message>
    <message>
        <source>Table</source>
        <translation type="unfinished">Tabelle</translation>
    </message>
    <message>
        <source>Key column</source>
        <translation type="unfinished">Schlüsselspalte</translation>
    </message>
    <message>
        <source>GISBASE is not set.</source>
        <translation type="unfinished">GISBASE nicht gesetzt.</translation>
    </message>
    <message>
        <source> is not a GRASS mapset.</source>
        <translation> ist keine GRASS-Mapset.</translation>
    </message>
    <message>
        <source>Cannot start </source>
        <translation type="unfinished">Fehler beim Start von </translation>
    </message>
    <message>
        <source>Mapset is already in use.</source>
        <translation>Mapset wird bereits benutzt.</translation>
    </message>
    <message>
        <source>Temporary directory </source>
        <translation type="unfinished">Temporäres Verzeichnis </translation>
    </message>
    <message>
        <source> exist but is not writable</source>
        <translation type="unfinished"> existiert, ist aber nicht beschreibbar.</translation>
    </message>
    <message>
        <source>Cannot create temporary directory </source>
        <translation>Fehler beim Anlegen des temporären Verzeichnisses </translation>
    </message>
    <message>
        <source>Cannot create </source>
        <translation type="unfinished">Fehler beim Anlegen </translation>
    </message>
    <message>
        <source>Cannot remove mapset lock: </source>
        <translation>Kann Mapsetsperre nicht entfernen: </translation>
    </message>
    <message>
        <source>Warning</source>
        <translation type="unfinished">Warnung</translation>
    </message>
    <message>
        <source>Cannot read raster map region</source>
        <translation>Konnte &apos;region&apos; der Rasterkarte nicht lesen</translation>
    </message>
    <message>
        <source>Cannot read vector map region</source>
        <translation>Konnte &apos;region der Vektorkarte nicht lesen</translation>
    </message>
    <message>
        <source>Cannot read region</source>
        <translation>Konnte &apos;region&apos; nicht lesen</translation>
    </message>
    <message>
        <source>Where is &apos;</source>
        <translation type="unfinished">Wo ist &apos;</translation>
    </message>
    <message>
        <source>original location: </source>
        <translation>Original-Location: </translation>
    </message>
    <message>
        <source>To identify features, you must choose an active layer by clicking on its name in the legend</source>
        <translation>Um Objekte zu identifizieren müssen Sie einen Layer in der Legende auswählen</translation>
    </message>
    <message>
        <source>Location: </source>
        <comment>Metadata in GRASS Browser</comment>
        <translation>Location: </translation>
    </message>
    <message>
        <source>&lt;br&gt;Mapset: </source>
        <comment>Metadata in GRASS Browser</comment>
        <translation>&lt;br&gt;Mapset: </translation>
    </message>
    <message>
        <source>Could not remove polygon intersection</source>
        <translation>Konnte Polygon-Überscheidung (Intersection) nicht löschen</translation>
    </message>
    <message>
        <source>Quick Print</source>
        <translation>Schnelles Drucken</translation>
    </message>
    <message>
        <source>Quick Print is a plugin to quickly print a map with minimal effort.</source>
        <translation>Quick Print ist ein Plugin, um mal eben schnell und ohne großen Aufwandt eine Karte zu drucken.</translation>
    </message>
    <message>
        <source>Loaded default style file from </source>
        <translation type="obsolete">Standard Style geladen von </translation>
    </message>
    <message>
        <source>The directory containing your dataset needs to be writeable!</source>
        <translation>Der Ordner mit den Daten muss beschreibbar sein!</translation>
    </message>
    <message>
        <source>Created default style file as </source>
        <translation>Standard Style-Datei erstellt als </translation>
    </message>
    <message>
        <source> is not writeable.</source>
        <translation type="unfinished"> kann nicht beschrieben werden.</translation>
    </message>
    <message>
        <source>Please adjust permissions (if possible) and try again.</source>
        <translation type="unfinished">Bitte passen Sie (wenn möglich) die Berechtigung und versuchen es erneut.</translation>
    </message>
    <message>
        <source>Couldn&apos;t load SIP module.</source>
        <translation type="unfinished">Das SIP-Modul konnte nicht geladen werden.</translation>
    </message>
    <message>
        <source>Python support will be disabled.</source>
        <translation type="unfinished">Die Python-Unterstützung wird abgeschaltet.</translation>
    </message>
    <message>
        <source>Couldn&apos;t load PyQt4.</source>
        <translation type="unfinished">PyQt4 konnte nicht geladen werden.</translation>
    </message>
    <message>
        <source>Couldn&apos;t load PyQGIS.</source>
        <translation type="unfinished">PyQGIS konnte nicht geladen werden.</translation>
    </message>
    <message>
        <source>An error has occured while executing Python code:</source>
        <translation type="unfinished">Fehler bei der Ausführung von Python-Code:</translation>
    </message>
    <message>
        <source>Python version:</source>
        <translation type="unfinished">Python-Version:</translation>
    </message>
    <message>
        <source>Python path:</source>
        <translation type="unfinished">Python-Pfad:</translation>
    </message>
    <message>
        <source>An error occured during execution of following code:</source>
        <translation type="unfinished">Fehler bei der Ausführung folgenden Codes:</translation>
    </message>
    <message>
        <source>Uncatched fatal GRASS error</source>
        <translation type="unfinished">Nicht abgefangener fataler GRASS-Fehler.</translation>
    </message>
    <message>
        <source>ERROR: Failed to created default style file as %1 Check file permissions and retry.</source>
        <translation type="unfinished">FEHLER: Konnte die Datei %1 für den voreingestellten Stil nicht erzeugen.  Bitte überprüfen Sie die Zugriffrechte vor einem erneuten Versuch.</translation>
    </message>
    <message>
        <source>Coordinate Capture</source>
        <translation type="unfinished">Koordinaten abgreifen</translation>
    </message>
    <message>
        <source>Capture mouse coordinates in different CRS</source>
        <translation type="unfinished">Koordinaten in anderem KBS verfolgen</translation>
    </message>
    <message>
        <source>Legend</source>
        <translation type="unfinished">Legende</translation>
    </message>
    <message>
        <source>Dxf2Shp Converter</source>
        <translation type="unfinished">Dxf2Shp-Konverter</translation>
    </message>
    <message>
        <source>Converts from dxf to shp file format</source>
        <translation type="unfinished">Wandelt von DXF- ins Shapeformat</translation>
    </message>
    <message>
        <source>Interpolating...</source>
        <translation type="unfinished">Interpoliere...</translation>
    </message>
    <message>
        <source>Abort</source>
        <translation type="unfinished">Abbrechen</translation>
    </message>
    <message>
        <source>Interpolation plugin</source>
        <translation type="unfinished">Interpolationsplugin</translation>
    </message>
    <message>
        <source>A plugin for interpolation based on vertices of a vector layer</source>
        <translation type="unfinished">Ein Plugin für die Stützpunktinterpolation von Vektorlayern.</translation>
    </message>
    <message>
        <source>Version 0.001</source>
        <translation type="unfinished">Version 0.001</translation>
    </message>
    <message>
        <source>OGR Layer Converter</source>
        <translation type="unfinished">OGR-Layer-Konverter</translation>
    </message>
    <message>
        <source>Translates vector layers between formats supported by OGR library</source>
        <translation type="unfinished">Vektorlayer von einem in ein anderes von der OGR-Bibliothek unterstütztes Formats umwandeln</translation>
    </message>
    <message>
        <source>CRS Exception</source>
        <translation type="unfinished">KBS-Ausnahme</translation>
    </message>
    <message>
        <source>Selection extends beyond layer&apos;s coordinate system.</source>
        <translation type="unfinished">Auswahl außerhalb des Koordinatensystems des Layers.</translation>
    </message>
    <message>
        <source>Loading style file </source>
        <translation type="unfinished">Laden der Stildatei </translation>
    </message>
    <message>
        <source> failed because:</source>
        <translation type="unfinished">schlug fehl, weil:</translation>
    </message>
    <message>
        <source>Could not save symbology because:</source>
        <translation type="unfinished">Konnte Symbolik nicht speichern, weil:</translation>
    </message>
    <message>
        <source>Unable to save to file. Your project may be corrupted on disk. Try clearing some space on the volume and check file permissions before pressing save again.</source>
        <translation type="unfinished">Konnte nicht in Datei speichern. Ihr Projekt könnte auf der Festplatte defekt sein.  Versuchen Sie etwas Platz freizumachen und überprüfen Sie vor einem erneuten Versuch die Zugriffsrechte.</translation>
    </message>
</context>
<context>
    <name>QgisApp</name>
    <message>
        <source>Quantum GIS - </source>
        <translation>Quantum GIS -</translation>
    </message>
    <message>
        <source>Version</source>
        <translation>Version</translation>
    </message>
    <message>
        <source>is not a valid or recognized data source</source>
        <translation type="unfinished">ist keine zulässige oder erkannte Datenquelle</translation>
    </message>
    <message>
        <source>Invalid Data Source</source>
        <translation type="unfinished">Ungültige Datenquelle</translation>
    </message>
    <message>
        <source>No Layer Selected</source>
        <translation type="unfinished">Keinen Layer ausgewählt</translation>
    </message>
    <message>
        <source>There is a new version of QGIS available</source>
        <translation type="unfinished">Eine neue Version von QGIS ist verfügbar</translation>
    </message>
    <message>
        <source>You are running a development version of QGIS</source>
        <translation type="unfinished">Sie verwenden eine Entwicklungsversion von QGIS</translation>
    </message>
    <message>
        <source>You are running the current version of QGIS</source>
        <translation type="unfinished">Sie verwenden die aktuelle Version von QGIS</translation>
    </message>
    <message>
        <source>Would you like more information?</source>
        <translation type="unfinished">Wollen Sie mehr Information?</translation>
    </message>
    <message>
        <source>QGIS Version Information</source>
        <translation type="unfinished">QGIS Versionsinformationen</translation>
    </message>
    <message>
        <source>Unable to get current version information from server</source>
        <translation type="unfinished">Kann Informationen zu aktuellen Version nicht vom Server holen</translation>
    </message>
    <message>
        <source>Connection refused - server may be down</source>
        <translation type="unfinished">Verbunging abgelehnt - Server wahrschinlich offline</translation>
    </message>
    <message>
        <source>QGIS server was not found</source>
        <translation type="unfinished">QGIS Server nicht gefunden</translation>
    </message>
    <message>
        <source>Invalid Layer</source>
        <translation type="unfinished">Ungültiger Layer</translation>
    </message>
    <message>
        <source>%1 is an invalid layer and cannot be loaded.</source>
        <translation type="unfinished">%1 ist ein ungültiger Layer und kann nicht geladen werden.</translation>
    </message>
    <message>
        <source>Error Loading Plugin</source>
        <translation type="unfinished">Fehler beim Laden des Plugins</translation>
    </message>
    <message>
        <source>There was an error loading %1.</source>
        <translation type="obsolete">Es trat ein Fehler auf beim Laden von %1.</translation>
    </message>
    <message>
        <source>Saved map image to</source>
        <translation type="unfinished">Kartenbild gespeichert in</translation>
    </message>
    <message>
        <source>Choose a filename to save the map image as</source>
        <translation type="obsolete">Dateinamen zum Speichern des Kartenbildes wählen</translation>
    </message>
    <message>
        <source>Extents: </source>
        <translation type="unfinished">Ausmasse:</translation>
    </message>
    <message>
        <source>Problem deleting features</source>
        <translation type="unfinished">Problem beim Löschen der Objekte</translation>
    </message>
    <message>
        <source>A problem occured during deletion of features</source>
        <translation type="unfinished">Beim Löschen der Objekte ist ein Problem aufgetreten</translation>
    </message>
    <message>
        <source>No Vector Layer Selected</source>
        <translation type="unfinished">Es wurde kein Vektorlayer gewählt.</translation>
    </message>
    <message>
        <source>Deleting features only works on vector layers</source>
        <translation type="unfinished">Löschen von Objekten ist nur in Vektorlayern möglich</translation>
    </message>
    <message>
        <source>To delete features, you must select a vector layer in the legend</source>
        <translation type="unfinished">Um Objekte zu löschen, muss ein Vektorlayer in der Legende gewählt werden</translation>
    </message>
    <message>
        <source>Map legend that displays all the layers currently on the map canvas. Click on the check box to turn a layer on or off. Double click on a layer in the legend to customize its appearance and set other properties.</source>
        <translation>Legende, die alle im Kartenfester angezeigten Layer enthält. Bitte auf die Checkbox klicken, um einen Layer an- oder auszuschalten. Mit einem Doppelklick in der Legende kann die Erscheinung und sonstige Eigenschaften eines Layers festgelegt werden.</translation>
    </message>
    <message>
        <source>Map overview canvas. This canvas can be used to display a locator map that shows the current extent of the map canvas. The current extent is shown as a red rectangle. Any layer on the map can be added to the overview canvas.</source>
        <translation>Übersichtsfenster. Dieses Fenster kann benutzt werden um die momentane Ausdehnung des Kartenfensters darzustellen. Der momentane Ausschnitt ist als rotes Rechteck dargestellt. Jeder Layer in der Karte kann zum Übersichtsfenster hinzugefügt werden.</translation>
    </message>
    <message>
        <source>&amp;Plugins</source>
        <translation>&amp;Plugins</translation>
    </message>
    <message>
        <source>Displays the current map scale</source>
        <translation>Zeigt den momentanen Kartenmassstab an</translation>
    </message>
    <message>
        <source>Render</source>
        <translation>Zeichnen</translation>
    </message>
    <message>
        <source>When checked, the map layers are rendered in response to map navigation commands and other events. When not checked, no rendering is done. This allows you to add a large number of layers and symbolize them before rendering.</source>
        <translation>Wenn angewählt, werden die Kartenlayer abhängig von der Bedienung der Navigationsinstrumente, gezeichnet. Anderenfalls werden die Layer nicht gezeichnet. Dies erlaubt es, eine grosse Layeranzahl hinzuzufügen und das Aussehen der Layer vor dem Zeichnen zu setzen.</translation>
    </message>
    <message>
        <source>Choose a QGIS project file</source>
        <translation type="unfinished">Eine QGIS-Projektdatei wählen</translation>
    </message>
    <message>
        <source>Unable to save project</source>
        <translation type="unfinished">Projekt kann nicht gespeichert werden</translation>
    </message>
    <message>
        <source>Unable to save project to </source>
        <translation type="unfinished">Projekt kann nicht gespeichert werden</translation>
    </message>
    <message>
        <source>Toggle map rendering</source>
        <translation>Zeichnen der Karte einschalten</translation>
    </message>
    <message>
        <source>Open an OGR Supported Vector Layer</source>
        <translation type="unfinished">Öffnen eines OGR-Vektorlayers</translation>
    </message>
    <message>
        <source>QGIS Project Read Error</source>
        <translation type="unfinished">Fehler beim Lesen des QGIS-Projektes</translation>
    </message>
    <message>
        <source>Try to find missing layers?</source>
        <translation type="unfinished">Versuchen, fehlende Layer zu finden?</translation>
    </message>
    <message>
        <source>Open a GDAL Supported Raster Data Source</source>
        <translation type="unfinished">Öffnen einer GDAL-Rasterdatenquelle</translation>
    </message>
    <message>
        <source>Save As</source>
        <translation type="unfinished">Speichern als</translation>
    </message>
    <message>
        <source>Choose a QGIS project file to open</source>
        <translation type="unfinished">QGIS-Projektdatei zum Öffnen wählen</translation>
    </message>
    <message>
        <source>Saved project to:</source>
        <translation type="unfinished">Projekt wurde gespeichert in:</translation>
    </message>
    <message>
        <source>Reading settings</source>
        <translation>Lese Einstellungen.</translation>
    </message>
    <message>
        <source>Setting up the GUI</source>
        <translation>Richte die Oberfläche ein</translation>
    </message>
    <message>
        <source>Checking database</source>
        <translation>Überprüfe die Datenbank</translation>
    </message>
    <message>
        <source>Restoring loaded plugins</source>
        <translation>Stelle die geladenen Plugins wieder her</translation>
    </message>
    <message>
        <source>Initializing file filters</source>
        <translation>Initialisiere Dateifilter</translation>
    </message>
    <message>
        <source>Restoring window state</source>
        <translation>Stelle Fensterstatus wieder her</translation>
    </message>
    <message>
        <source>QGIS Ready!</source>
        <translation>QGIS ist startklar!</translation>
    </message>
    <message>
        <source>&amp;New Project</source>
        <translation>&amp;Neues Projekt</translation>
    </message>
    <message>
        <source>Ctrl+N</source>
        <comment>New Project</comment>
        <translation>Ctrl+N</translation>
    </message>
    <message>
        <source>New Project</source>
        <translation>Neues Projekt</translation>
    </message>
    <message>
        <source>&amp;Open Project...</source>
        <translation>Pr&amp;ojekt öffnen...</translation>
    </message>
    <message>
        <source>Ctrl+O</source>
        <comment>Open a Project</comment>
        <translation>Ctrl+O</translation>
    </message>
    <message>
        <source>Open a Project</source>
        <translation>Projekt öffnen</translation>
    </message>
    <message>
        <source>&amp;Save Project</source>
        <translation>Projekt &amp;speichern</translation>
    </message>
    <message>
        <source>Ctrl+S</source>
        <comment>Save Project</comment>
        <translation>Ctrl+S</translation>
    </message>
    <message>
        <source>Save Project</source>
        <translation>Projekt speichern</translation>
    </message>
    <message>
        <source>Save Project &amp;As...</source>
        <translation>Projekt speichern &amp;als...</translation>
    </message>
    <message>
        <source>Ctrl+A</source>
        <comment>Save Project under a new name</comment>
        <translation type="obsolete">Ctrl+A</translation>
    </message>
    <message>
        <source>Save Project under a new name</source>
        <translation>Projekt unter einem neuen Namen abspeichern.</translation>
    </message>
    <message>
        <source>&amp;Print...</source>
        <translation type="obsolete">&amp;Drucken...</translation>
    </message>
    <message>
        <source>Ctrl+P</source>
        <comment>Print</comment>
        <translation type="obsolete">Ctrl+P</translation>
    </message>
    <message>
        <source>Print</source>
        <translation type="obsolete">Drucken</translation>
    </message>
    <message>
        <source>Save as Image...</source>
        <translation>Bild speichern als...</translation>
    </message>
    <message>
        <source>Ctrl+I</source>
        <comment>Save map as image</comment>
        <translation type="obsolete">Ctrl+I</translation>
    </message>
    <message>
        <source>Save map as image</source>
        <translation>Karte als Bild speichern</translation>
    </message>
    <message>
        <source>Exit</source>
        <translation>Beenden</translation>
    </message>
    <message>
        <source>Ctrl+Q</source>
        <comment>Exit QGIS</comment>
        <translation>Ctrl+Q</translation>
    </message>
    <message>
        <source>Exit QGIS</source>
        <translation>Beende QGIS</translation>
    </message>
    <message>
        <source>Add a Vector Layer...</source>
        <translation type="obsolete">Vektorlayer hinzufügen...</translation>
    </message>
    <message>
        <source>V</source>
        <comment>Add a Vector Layer</comment>
        <translation>V</translation>
    </message>
    <message>
        <source>Add a Vector Layer</source>
        <translation>Vektorlayer hinzufügen</translation>
    </message>
    <message>
        <source>Add a Raster Layer...</source>
        <translation type="obsolete">Rasterlayer hinzufügen...</translation>
    </message>
    <message>
        <source>R</source>
        <comment>Add a Raster Layer</comment>
        <translation>R</translation>
    </message>
    <message>
        <source>Add a Raster Layer</source>
        <translation>Rasterlayer hinzufügen</translation>
    </message>
    <message>
        <source>Add a PostGIS Layer...</source>
        <translation type="obsolete">PostGIS-Layer hinzufügen...</translation>
    </message>
    <message>
        <source>D</source>
        <comment>Add a PostGIS Layer</comment>
        <translation>D</translation>
    </message>
    <message>
        <source>Add a PostGIS Layer</source>
        <translation>PostGIS-Layer hinzufügen</translation>
    </message>
    <message>
        <source>New Vector Layer...</source>
        <translation>Neuer Vektorlayer...</translation>
    </message>
    <message>
        <source>N</source>
        <comment>Create a New Vector Layer</comment>
        <translation>N</translation>
    </message>
    <message>
        <source>Create a New Vector Layer</source>
        <translation>Neuen Vektorlayer erzeugen</translation>
    </message>
    <message>
        <source>Remove Layer</source>
        <translation>Layer löschen</translation>
    </message>
    <message>
        <source>Ctrl+D</source>
        <comment>Remove a Layer</comment>
        <translation>Ctrl+D</translation>
    </message>
    <message>
        <source>Remove a Layer</source>
        <translation>Lösche einen Layer</translation>
    </message>
    <message>
        <source>Add All To Overview</source>
        <translation type="obsolete">Alle zur Übersicht hinzufügen</translation>
    </message>
    <message>
        <source>+</source>
        <comment>Show all layers in the overview map</comment>
        <translation>+</translation>
    </message>
    <message>
        <source>Show all layers in the overview map</source>
        <translation>Zeige alle Layer in der Übersichtskarte</translation>
    </message>
    <message>
        <source>Remove All From Overview</source>
        <translation>Alle aus Übersicht entfernen</translation>
    </message>
    <message>
        <source>-</source>
        <comment>Remove all layers from overview map</comment>
        <translation>-</translation>
    </message>
    <message>
        <source>Remove all layers from overview map</source>
        <translation>Alle Ebnene aus der Übersichtskarte entfernen</translation>
    </message>
    <message>
        <source>Show All Layers</source>
        <translation>Alle Layer anzeigen</translation>
    </message>
    <message>
        <source>S</source>
        <comment>Show all layers</comment>
        <translation>S</translation>
    </message>
    <message>
        <source>Show all layers</source>
        <translation>Alle Layer zeigen</translation>
    </message>
    <message>
        <source>Hide All Layers</source>
        <translation>Alle Layer ausblenden</translation>
    </message>
    <message>
        <source>H</source>
        <comment>Hide all layers</comment>
        <translation>H</translation>
    </message>
    <message>
        <source>Hide all layers</source>
        <translation>Alle Layer ausblenden</translation>
    </message>
    <message>
        <source>Project Properties...</source>
        <translation>Projekteinstellungen...</translation>
    </message>
    <message>
        <source>P</source>
        <comment>Set project properties</comment>
        <translation>P</translation>
    </message>
    <message>
        <source>Set project properties</source>
        <translation>Projekteigenschaften setzen</translation>
    </message>
    <message>
        <source>Options...</source>
        <translation>Optionen...</translation>
    </message>
    <message>
        <source>Change various QGIS options</source>
        <translation>Verschiedene QGIS-Einstellungen ändern</translation>
    </message>
    <message>
        <source>Help Contents</source>
        <translation>Hilfe-Übersicht</translation>
    </message>
    <message>
        <source>F1</source>
        <comment>Help Documentation</comment>
        <translation>F1</translation>
    </message>
    <message>
        <source>Help Documentation</source>
        <translation>Hilfe</translation>
    </message>
    <message>
        <source>Qgis Home Page</source>
        <translation type="obsolete">QGIS-Homepage</translation>
    </message>
    <message>
        <source>Ctrl+H</source>
        <comment>QGIS Home Page</comment>
        <translation>Ctrl+H</translation>
    </message>
    <message>
        <source>QGIS Home Page</source>
        <translation>QGIS-Homepage</translation>
    </message>
    <message>
        <source>About</source>
        <translation>Über</translation>
    </message>
    <message>
        <source>About QGIS</source>
        <translation>Über QGIS</translation>
    </message>
    <message>
        <source>Check Qgis Version</source>
        <translation>QGIS Version überprüfen</translation>
    </message>
    <message>
        <source>Check if your QGIS version is up to date (requires internet access)</source>
        <translation>Aktualität Ihre QGIS-Version überprüfen (erfordert Internetzugang)</translation>
    </message>
    <message>
        <source>Refresh</source>
        <translation>Erneuern</translation>
    </message>
    <message>
        <source>Ctrl+R</source>
        <comment>Refresh Map</comment>
        <translation>Ctrl+R</translation>
    </message>
    <message>
        <source>Refresh Map</source>
        <translation>Karte neu zeichnen</translation>
    </message>
    <message>
        <source>Zoom In</source>
        <translation>Hineinzoomen</translation>
    </message>
    <message>
        <source>Ctrl++</source>
        <comment>Zoom In</comment>
        <translation>Ctrl++</translation>
    </message>
    <message>
        <source>Zoom Out</source>
        <translation>Hinauszoomen</translation>
    </message>
    <message>
        <source>Ctrl+-</source>
        <comment>Zoom Out</comment>
        <translation>Ctrl+-</translation>
    </message>
    <message>
        <source>Zoom Full</source>
        <translation>Volle Ausdehnung</translation>
    </message>
    <message>
        <source>F</source>
        <comment>Zoom to Full Extents</comment>
        <translation>F</translation>
    </message>
    <message>
        <source>Zoom to Full Extents</source>
        <translation>Auf die volle Ausdehnung herauszoomen</translation>
    </message>
    <message>
        <source>Zoom To Selection</source>
        <translation type="obsolete">Auf die Auswahl zoomen</translation>
    </message>
    <message>
        <source>Ctrl+F</source>
        <comment>Zoom to selection</comment>
        <translation type="obsolete">Ctrl+F</translation>
    </message>
    <message>
        <source>Zoom to selection</source>
        <translation type="obsolete">Auf die Auswahl zoomen</translation>
    </message>
    <message>
        <source>Pan Map</source>
        <translation>Karte verschieben</translation>
    </message>
    <message>
        <source>Pan the map</source>
        <translation>Karte verschieben</translation>
    </message>
    <message>
        <source>Zoom Last</source>
        <translation>Zur vorherigen Zoomeinstellung zurückkehren</translation>
    </message>
    <message>
        <source>Zoom to Last Extent</source>
        <translation>Zur vorherigen Zoomeinstellung zurückkehren</translation>
    </message>
    <message>
        <source>Zoom To Layer</source>
        <translation type="obsolete">Auf den Layer zoomen</translation>
    </message>
    <message>
        <source>Zoom to Layer</source>
        <translation>Auf den Layer zoomen</translation>
    </message>
    <message>
        <source>Identify Features</source>
        <translation>Objekte abfragen</translation>
    </message>
    <message>
        <source>I</source>
        <comment>Click on features to identify them</comment>
        <translation>I</translation>
    </message>
    <message>
        <source>Click on features to identify them</source>
        <translation>Klicken Sie auf ein Objekt, um Informationen dazu zuerhalten</translation>
    </message>
    <message>
        <source>Select Features</source>
        <translation>Wähle Objekte aus</translation>
    </message>
    <message>
        <source>Open Table</source>
        <translation type="obsolete">Attributtabelle öffnen</translation>
    </message>
    <message>
        <source>Measure Line </source>
        <translation>Linie messen</translation>
    </message>
    <message>
        <source>Ctrl+M</source>
        <comment>Measure a Line</comment>
        <translation type="obsolete">Ctrl+M</translation>
    </message>
    <message>
        <source>Measure a Line</source>
        <translation>Linie messen</translation>
    </message>
    <message>
        <source>Measure Area</source>
        <translation>Fläche messen</translation>
    </message>
    <message>
        <source>Ctrl+J</source>
        <comment>Measure an Area</comment>
        <translation type="obsolete">Ctrl+J</translation>
    </message>
    <message>
        <source>Measure an Area</source>
        <translation>Fläche messen</translation>
    </message>
    <message>
        <source>Show Bookmarks</source>
        <translation>Lesezeichen anzeigen</translation>
    </message>
    <message>
        <source>B</source>
        <comment>Show Bookmarks</comment>
        <translation>B</translation>
    </message>
    <message>
        <source>New Bookmark...</source>
        <translation>Neues Lesezeichen...</translation>
    </message>
    <message>
        <source>Ctrl+B</source>
        <comment>New Bookmark</comment>
        <translation>Ctrl+B</translation>
    </message>
    <message>
        <source>New Bookmark</source>
        <translation>Neues Lesezeichen</translation>
    </message>
    <message>
        <source>Add WMS Layer...</source>
        <translation>WMS-Layer hinzufügen...</translation>
    </message>
    <message>
        <source>W</source>
        <comment>Add Web Mapping Server Layer</comment>
        <translation type="obsolete">W</translation>
    </message>
    <message>
        <source>Add Web Mapping Server Layer</source>
        <translation type="obsolete">WMS-Layer hinzufügen</translation>
    </message>
    <message>
        <source>In Overview</source>
        <translation type="obsolete">In Übersicht anzeigen</translation>
    </message>
    <message>
        <source>O</source>
        <comment>Add current layer to overview map</comment>
        <translation>O</translation>
    </message>
    <message>
        <source>Add current layer to overview map</source>
        <translation>Aktuellen Layer zur Übersicht hinzufügen</translation>
    </message>
    <message>
        <source>Plugin Manager...</source>
        <translation type="obsolete">Plugin Manager...</translation>
    </message>
    <message>
        <source>Open the plugin manager</source>
        <translation>Öffne den Pluginmanager</translation>
    </message>
    <message>
        <source>Capture Point</source>
        <translation>Punkt digitalisieren</translation>
    </message>
    <message>
        <source>.</source>
        <comment>Capture Points</comment>
        <translation>.</translation>
    </message>
    <message>
        <source>Capture Points</source>
        <translation>Punkte digitalisieren</translation>
    </message>
    <message>
        <source>Capture Line</source>
        <translation>Linie digitalisieren</translation>
    </message>
    <message>
        <source>/</source>
        <comment>Capture Lines</comment>
        <translation>/</translation>
    </message>
    <message>
        <source>Capture Lines</source>
        <translation>Linien digitalisieren</translation>
    </message>
    <message>
        <source>Capture Polygon</source>
        <translation>Polygon digitalisieren</translation>
    </message>
    <message>
        <source>Ctrl+/</source>
        <comment>Capture Polygons</comment>
        <translation>Ctrl+/</translation>
    </message>
    <message>
        <source>Capture Polygons</source>
        <translation>Polygon digitialisieren</translation>
    </message>
    <message>
        <source>Delete Selected</source>
        <translation>Ausgewähltes löschen</translation>
    </message>
    <message>
        <source>Add Vertex</source>
        <translation>Stützpunkt hinzufügen</translation>
    </message>
    <message>
        <source>Delete Vertex</source>
        <translation>Stützpunkt löschen</translation>
    </message>
    <message>
        <source>Move Vertex</source>
        <translation>Stützpunkt verschieben</translation>
    </message>
    <message>
        <source>&amp;File</source>
        <translation>&amp;Datei</translation>
    </message>
    <message>
        <source>&amp;Open Recent Projects</source>
        <translation>Aktuelle Pr&amp;ojekte öffnen</translation>
    </message>
    <message>
        <source>&amp;View</source>
        <translation>&amp;Ansicht</translation>
    </message>
    <message>
        <source>&amp;Layer</source>
        <translation>&amp;Layer</translation>
    </message>
    <message>
        <source>&amp;Settings</source>
        <translation>&amp;Einstellungen</translation>
    </message>
    <message>
        <source>&amp;Help</source>
        <translation>&amp;Hilfe</translation>
    </message>
    <message>
        <source>File</source>
        <translation>Datei</translation>
    </message>
    <message>
        <source>Manage Layers</source>
        <translation>Layer koordinieren</translation>
    </message>
    <message>
        <source>Help</source>
        <translation>Hilfe</translation>
    </message>
    <message>
        <source>Digitizing</source>
        <translation>Digitalisierung</translation>
    </message>
    <message>
        <source>Map Navigation</source>
        <translation>Kartennavigation</translation>
    </message>
    <message>
        <source>Attributes</source>
        <translation>Attribute</translation>
    </message>
    <message>
        <source>Plugins</source>
        <translation>Plugins</translation>
    </message>
    <message>
        <source>Ready</source>
        <translation>Fertig</translation>
    </message>
    <message>
        <source>New features</source>
        <translation>Neue Objekte</translation>
    </message>
    <message>
        <source>Unable to open project</source>
        <translation type="unfinished">Kann das Projekt nicht öffnen</translation>
    </message>
    <message>
        <source>Unable to save project </source>
        <translation type="unfinished">Kann das Projekt nicht speichern </translation>
    </message>
    <message>
        <source>Choose a filename to save the QGIS project file as</source>
        <translation type="obsolete">Bitte wählen Sie einen Dateinamen, unter dem Sie das Projekt speichern wollen.</translation>
    </message>
    <message>
        <source>QGIS: Unable to load project</source>
        <translation type="unfinished">QGIS: Kann das Projekt nicht laden.</translation>
    </message>
    <message>
        <source>Unable to load project </source>
        <translation type="unfinished">Kann das Projekt nicht laden.</translation>
    </message>
    <message>
        <source>QGIS - Changes in SVN Since Last Release</source>
        <translation type="unfinished">QGIS - Änderungen in SVN seit dem letzten Release</translation>
    </message>
    <message>
        <source>Layer is not valid</source>
        <translation type="unfinished">Layer ist ungültig</translation>
    </message>
    <message>
        <source>The layer is not a valid layer and can not be added to the map</source>
        <translation type="unfinished">Der Layer ist ungültig und kann daher nicht zum Kartenfenster hinzugefügt werden.</translation>
    </message>
    <message>
        <source>Save?</source>
        <translation type="unfinished">Speichern?</translation>
    </message>
    <message>
        <source> is not a valid or recognized raster data source</source>
        <translation type="unfinished"> ist keine gültige Rasterdatenquelle.</translation>
    </message>
    <message>
        <source> is not a supported raster data source</source>
        <translation type="unfinished"> ist kein unterstütztes Rasterdatenformat.</translation>
    </message>
    <message>
        <source>Unsupported Data Source</source>
        <translation type="unfinished">Nicht unterstütztes Datenformat</translation>
    </message>
    <message>
        <source>Enter a name for the new bookmark:</source>
        <translation type="unfinished">Bitte geben Sie einen Namen für das Lesenzeichen ein:</translation>
    </message>
    <message>
        <source>Error</source>
        <translation type="unfinished">Fehler</translation>
    </message>
    <message>
        <source>Unable to create the bookmark. Your user database may be missing or corrupted</source>
        <translation type="unfinished">Kann das Lesezeichen nicht erstellen. Ihre Datenbank scheint zu fehlen oder ist kaputt.</translation>
    </message>
    <message>
        <source>Cut Features</source>
        <translation>Ausgewählte Objekte ausschneiden</translation>
    </message>
    <message>
        <source>Cut selected features</source>
        <translation>Ausgewählte Objekte ausschneiden</translation>
    </message>
    <message>
        <source>Copy Features</source>
        <translation>Objekte kopieren</translation>
    </message>
    <message>
        <source>Copy selected features</source>
        <translation>Ausgewählte Objekte kopieren</translation>
    </message>
    <message>
        <source>Paste Features</source>
        <translation>Objekte einfügen</translation>
    </message>
    <message>
        <source>Paste selected features</source>
        <translation>Ausgewählte Objekte einfügen</translation>
    </message>
    <message>
        <source>Ctrl+?</source>
        <comment>Help Documentation (Mac)</comment>
        <translation>Ctrl+?</translation>
    </message>
    <message>
        <source>Show most toolbars</source>
        <translation type="obsolete">Die meisten Werkzeugleisten anzeigen</translation>
    </message>
    <message>
        <source>Hide most toolbars</source>
        <translation type="obsolete">Die meisten Werkzeugleisten ausblenden</translation>
    </message>
    <message>
        <source>Network error while communicating with server</source>
        <translation type="unfinished">Es trat ein Netzwerkfehler während der Kommunikation zum Server auf.</translation>
    </message>
    <message>
        <source>Unknown network socket error</source>
        <translation type="unfinished">Unbekannter Netzwerkfehler (Socketfehler)</translation>
    </message>
    <message>
        <source>Unable to communicate with QGIS Version server</source>
        <translation type="unfinished">Kann nicht mit dem QGIS-Server kommunizieren.</translation>
    </message>
    <message>
        <source>Checking provider plugins</source>
        <translation type="unfinished">Provider-Plugins werden geprüft</translation>
    </message>
    <message>
        <source>Starting Python</source>
        <translation type="unfinished">Python wird gestartet</translation>
    </message>
    <message>
        <source>Python error</source>
        <translation type="unfinished">Python-Fehler</translation>
    </message>
    <message>
        <source>Error when reading metadata of plugin </source>
        <translation type="unfinished">Fehler beim Lesen der Plugin-Metadaten</translation>
    </message>
    <message>
        <source>Provider does not support deletion</source>
        <translation type="unfinished">Provider unterstützt keine Löschoperationen</translation>
    </message>
    <message>
        <source>Data provider does not support deleting features</source>
        <translation type="unfinished">Der Provider hat nicht die Möglichkeit, Objekte zu löschen</translation>
    </message>
    <message>
        <source>Layer not editable</source>
        <translation type="unfinished">Der Layer kann nicht bearbeitet werden</translation>
    </message>
    <message>
        <source>The current layer is not editable. Choose &apos;Start editing&apos; in the digitizing toolbar.</source>
        <translation type="unfinished">Der aktuelle Layer kann nicht bearbeitet werden. Bitte wählen Sie &apos;Bearbeitungsstatus umschalten&apos; aus der Digitalisierwerkzeugleiste.</translation>
    </message>
    <message>
        <source>Toggle editing</source>
        <translation type="unfinished">Bearbeitungsstatus umschalten</translation>
    </message>
    <message>
        <source>Toggles the editing state of the current layer</source>
        <translation>Bearbeitungsstatus des aktuellen Layers umschalten</translation>
    </message>
    <message>
        <source>Add Ring</source>
        <translation type="unfinished">Ring hinzufügen</translation>
    </message>
    <message>
        <source>Add Island</source>
        <translation>Insel hinzufügen</translation>
    </message>
    <message>
        <source>Add Island to multipolygon</source>
        <translation type="unfinished">Insel in Multipolygon einfügen</translation>
    </message>
    <message>
        <source>Toolbar Visibility...</source>
        <translation type="obsolete">Werkzeugleistenanzeige...</translation>
    </message>
    <message>
        <source>Scale </source>
        <translation type="unfinished">Maßstab </translation>
    </message>
    <message>
        <source>Current map scale (formatted as x:y)</source>
        <translation type="unfinished">Aktueller Kartenmaßstab (x:y formatiert)</translation>
    </message>
    <message>
        <source>Map coordinates at mouse cursor position</source>
        <translation type="unfinished">Kartenkoordinaten beim Mauszeiger</translation>
    </message>
    <message>
        <source>Invalid scale</source>
        <translation type="unfinished">Ungültiger Maßstab</translation>
    </message>
    <message>
        <source>Do you want to save the current project?</source>
        <translation type="unfinished">Wollen Sie das aktuelle Projekt speichern?</translation>
    </message>
    <message>
        <source>Python console</source>
        <translation type="obsolete">Python-Konsole</translation>
    </message>
    <message>
        <source>Move Feature</source>
        <translation>Objekt verschieben</translation>
    </message>
    <message>
        <source>Split Features</source>
        <translation>Objekte trennen</translation>
    </message>
    <message>
        <source>Map Tips</source>
        <translation>Kartenhinweise</translation>
    </message>
    <message>
        <source>Show information about a feature when the mouse is hovered over it</source>
        <translation>Zeige Informationen zu einem Objekt, wenn die Maus darüber fährt</translation>
    </message>
    <message>
        <source>Current map scale</source>
        <translation>Aktueller Kartenmaßstab</translation>
    </message>
    <message>
        <source>Project file is older</source>
        <translation>Projektdatei ist älter</translation>
    </message>
    <message>
        <source>&lt;p&gt;This project file was saved by an older version of QGIS.</source>
        <translation>&lt;p&gt;Diese Projektdatei wurde von einer älteren QGIS Version abgespeichert.</translation>
    </message>
    <message>
        <source> When saving this project file, QGIS will update it to the latest version, possibly rendering it useless for older versions of QGIS.</source>
        <translation> Wenn Sie diese Projektdatei speichern, wird QGIS es auf die neueste Version updaten und dadurch möglicherweise  nutzlos für ältere Versionen machen.</translation>
    </message>
    <message>
        <source>&lt;p&gt;Even though QGIS developers try to maintain backwards compatibility, some of the information from the old project file might be lost.</source>
        <translation>&lt;p&gt;Obwohl QGIS-Entwickler versuchen, Rückwärtskompatibilität zu gewährleisten, können einige Informationen der Projektdatei verloren gehen.</translation>
    </message>
    <message>
        <source> To improve the quality of QGIS, we appreciate if you file a bug report at %3.</source>
        <translation> Um die Qualität von QGIS zu verbessern, möchten wir Sie bitten, einen Fehlerreport zu erstellen unter %3.</translation>
    </message>
    <message>
        <source> Be sure to include the old project file, and state the version of QGIS you used to discover the error.</source>
        <translation> Stellen Sie sicher, dass die alte Projektdatei und die QGIS Version, bei der der Fehler auftritt angegeben sind, um den Fehler zu finden.</translation>
    </message>
    <message>
        <source>&lt;p&gt;To remove this warning when opening an older project file, uncheck the box &apos;%5&apos; in the %4 menu.</source>
        <translation>&lt;p&gt;Um diese Warnung beim Öffnen einer alten Projektdatei abzustellen, deaktivieren Sie die Box &apos;%5&apos; im Menü %4.</translation>
    </message>
    <message>
        <source>&lt;p&gt;Version of the project file: %1&lt;br&gt;Current version of QGIS: %2</source>
        <translation>&lt;p&gt;Version der Projektdatei: %1&lt;br&gt;Aktueller QGIS Version: %2</translation>
    </message>
    <message>
        <source>&lt;tt&gt;Settings:Options:General&lt;/tt&gt;</source>
        <comment>Menu path to setting options</comment>
        <translation>&lt;tt&gt;Einstellungen:Optionen:Allgemein&lt;/tt&gt;</translation>
    </message>
    <message>
        <source>Warn me when opening a project file saved with an older version of QGIS</source>
        <translation>Warne mich beim Öffnen einer Projektdatei, die mit einer älteren QGIS Version erstellt wurde</translation>
    </message>
    <message>
        <source>Toggle full screen mode</source>
        <translation type="obsolete">Vollbildmodus umschalten</translation>
    </message>
    <message>
        <source>Toggle fullscreen mode</source>
        <translation type="unfinished">Vollbildmodus umschalten</translation>
    </message>
    <message>
        <source>Resource Location Error</source>
        <translation type="unfinished">Resource nicht gefunden</translation>
    </message>
    <message>
        <source>Error reading icon resources from: 
 %1
 Quitting...</source>
        <translation>Fehler beim Lesen des Icrons aus: 
 %1 
 Abbruch...</translation>
    </message>
    <message>
        <source>Overview</source>
        <translation type="unfinished">Übersicht</translation>
    </message>
    <message>
        <source>Legend</source>
        <translation type="unfinished">Legende</translation>
    </message>
    <message>
        <source>You are using QGIS version %1 built against code revision %2.</source>
        <translation type="unfinished">Sie benutzen QGIS Version %1 mit dem Codestand %2.</translation>
    </message>
    <message>
        <source> This copy of QGIS has been built with PostgreSQL support.</source>
        <translation type="unfinished">Diese QGIS-Kopie unterstützt PostgreSQL.</translation>
    </message>
    <message>
        <source> This copy of QGIS has been built without PostgreSQL support.</source>
        <translation type="unfinished">Diese QGIS-Kopie unterstützt PostgreSQL nicht.</translation>
    </message>
    <message>
        <source>
This binary was compiled against Qt %1,and is currently running against Qt %2</source>
        <translation type="unfinished">Es wurde mit Qt %1 kompiliert und läuft gerade mit Qt %2</translation>
    </message>
    <message>
        <source>T</source>
        <comment>

Show most toolbars</comment>
        <translation type="obsolete">Die meisten Werkzeugleisten einblenden</translation>
    </message>
    <message>
        <source>Ctrl+T</source>
        <comment>

Hide most toolbars</comment>
        <translation type="obsolete">Die meisten Werkzeugleisten ausblenden</translation>
    </message>
    <message>
        <source>Progress bar that displays the status of rendering layers and other time-intensive operations</source>
        <translation>Fortschrittsanzeige für das Zeichnen von Layern und andere zeitintensive Operationen.</translation>
    </message>
    <message>
        <source>Shows the map coordinates at the current cursor position. The display is continuously updated as the mouse is moved.</source>
        <translation>Zeigt die Kartenkoordinate der aktuellen Cursorposition. Die Anzeige wird während der Mausbewegung laufend aktualisiert.</translation>
    </message>
    <message>
        <source>Ctrl-F</source>
        <comment>

Toggle fullscreen mode</comment>
        <translation type="obsolete">Vollbildmodus umschalten</translation>
    </message>
    <message>
        <source>Stop map rendering</source>
        <translation type="unfinished">Zeichnen der Karte abbrechen</translation>
    </message>
    <message>
        <source>Map canvas. This is where raster and vector layers are displayed when added to the map</source>
        <translation type="unfinished">Kartenansicht.  Hier werden Raster- und Vektorlayer angezeigt, wenn sie der Karte hinzugefügt werden.</translation>
    </message>
    <message>
        <source>Custom CRS...</source>
        <translation>Benutzerkoordinatenbezugssystem...</translation>
    </message>
    <message>
        <source>Manage custom coordinate reference systems</source>
        <translation type="unfinished">Benutzerkoordinatenbezugssysteme bearbeiten</translation>
    </message>
    <message>
        <source>Toggle extents and mouse position display</source>
        <translation type="unfinished">Grenzen- und Mauspositionsanzeige umschalten</translation>
    </message>
    <message>
        <source>This icon shows whether on the fly coordinate reference system transformation is enabled or not. Click the icon to bring up the project properties dialog to alter this behaviour.</source>
        <translation type="unfinished">Diese Icon zeigt an, ob On-The-Fly-Transformation des Koordinatenbezugssystem aktiv ist. Anklicken, um dies in den Projektionseigenschaften zu ändern.</translation>
    </message>
    <message>
        <source>CRS status - Click to open coordinate reference system dialog</source>
        <translation type="unfinished">KBS-Status - Klicken um den Dialog zum Koordinatenbezugssystem zu öffnen</translation>
    </message>
    <message>
        <source>This release candidate includes over 60 bug fixes and enchancements over the QGIS 0.10.0 release. In addition we have added the following new features:</source>
        <translation type="unfinished">Dieser Release-Kandidate beinhalted über 60 Fehlerkorrekutren und Erweiterung gegenüber der Version 0.10.0. Zusätzlich haben wir folgende Funktionen ergänzt:</translation>
    </message>
    <message>
        <source>Revision of all dialogs for user interface consistancy</source>
        <translation type="unfinished">Überarbeitung aller Dialog zur Vereinheitlichung der Benutzeroberfläche.</translation>
    </message>
    <message>
        <source>Improvements to unique value renderer vector dialog</source>
        <translation type="unfinished">Verbesserungen am Dialog für Bezeichnungen nach eindeutigen Werten.</translation>
    </message>
    <message>
        <source>Symbol previews when defining vector classes</source>
        <translation type="unfinished">Symbolvorschau beim Anlegen von Vektorklassen</translation>
    </message>
    <message>
        <source>Separation of python support into its own library</source>
        <translation type="unfinished">Ausgliederung der Python-Unterstützung in eine separate Bibliothek.</translation>
    </message>
    <message>
        <source>List view and filter for GRASS toolbox to find tools more quickly</source>
        <translation type="unfinished">Filterbare Liste für den GRASS-Werkzeugkasten, um das Auffinden von Werkzeugen zu beschleunigen.</translation>
    </message>
    <message>
        <source>List view and filter for Plugin Manager to find plugins more easily</source>
        <translation type="unfinished">Filterbare Liste um das Auffinden von Plugins im Plugin-Manager zu vereinfachen.</translation>
    </message>
    <message>
        <source>Updated Spatial Reference System definitions</source>
        <translation type="unfinished">Definition der Definitionen der räumlichen Bezugssystem aktualisiert.</translation>
    </message>
    <message>
        <source>QML Style support for rasters and database layers</source>
        <translation type="unfinished">Unterstützung für QML-Stile für Raster- und Datenbanklayer.</translation>
    </message>
    <message>
        <source>There was an error loading a plugin.The following diagnostic information may help the QGIS developers resolve the issue:
%1.</source>
        <translation type="unfinished">Beim Laden eines Plugins trat ein Fehler auf.  Die folgenden Informationen könnten den QGIS-Entwicklern bei der Lösung des Problem helfen: %1.</translation>
    </message>
    <message>
        <source>Maptips require an active layer</source>
        <translation type="unfinished">Kartentipps erfordern einen aktuellen Layer.</translation>
    </message>
    <message>
        <source>Multiple Instances of QgisApp</source>
        <translation type="unfinished">Mehrere QgisApp-Instanzen</translation>
    </message>
    <message>
        <source>Multiple instances of Quantum GIS application object detected.
Please contact the developers.
</source>
        <translation type="unfinished">Mehrere Instanzen der Quantum GIS-Application wurden festgestellt.
Bitte kontaktieren Sie die Entwickler.</translation>
    </message>
    <message>
        <source>Shift+Ctrl+S</source>
        <comment>Save Project under a new name</comment>
        <translation type="unfinished">Projekt unter anderem Namen speichern</translation>
    </message>
    <message>
        <source>&amp;Print Composer</source>
        <translation type="unfinished">&amp;Druckzusammenstellung</translation>
    </message>
    <message>
        <source>Ctrl+P</source>
        <comment>Print Composer</comment>
        <translation type="unfinished">Ctrl+P</translation>
    </message>
    <message>
        <source>Print Composer</source>
        <translation type="unfinished">Druckzusammenstellung</translation>
    </message>
    <message>
        <source>&amp;Undo</source>
        <translation type="unfinished">&amp;Rückgängig</translation>
    </message>
    <message>
        <source>Ctrl+Z</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Undo the last operation</source>
        <translation type="unfinished">Die letzte Operation rückgängig machen</translation>
    </message>
    <message>
        <source>Cu&amp;t</source>
        <translation type="unfinished">&amp;Ausschneiden</translation>
    </message>
    <message>
        <source>Ctrl+X</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cut the current selection&apos;s contents to the clipboard</source>
        <translation type="unfinished">Aktuelle Auswahl in Zwischenablage verschieben</translation>
    </message>
    <message>
        <source>&amp;Copy</source>
        <translation type="unfinished">&amp;Kopieren</translation>
    </message>
    <message>
        <source>Ctrl+C</source>
        <translation type="unfinished">Ctrl+C</translation>
    </message>
    <message>
        <source>Copy the current selection&apos;s contents to the clipboard</source>
        <translation type="unfinished">Aktuelle Auswahl in die Zwischenablage kopieren</translation>
    </message>
    <message>
        <source>&amp;Paste</source>
        <translation type="unfinished">&amp;Einfügen</translation>
    </message>
    <message>
        <source>Ctrl+V</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Paste the clipboard&apos;s contents into the current selection</source>
        <translation type="unfinished">Zwischenablagen in die aktuelle Auswahl übernehmen</translation>
    </message>
    <message>
        <source>M</source>
        <comment>Measure a Line</comment>
        <translation type="unfinished">Linie messen</translation>
    </message>
    <message>
        <source>J</source>
        <comment>Measure an Area</comment>
        <translation type="unfinished">Fläche messen</translation>
    </message>
    <message>
        <source>Zoom to Selection</source>
        <translation type="unfinished">Zur Auswahl zoomen</translation>
    </message>
    <message>
        <source>Ctrl+J</source>
        <comment>Zoom to Selection</comment>
        <translation type="unfinished">Ctrl+J</translation>
    </message>
    <message>
        <source>Zoom Actual Size</source>
        <translation type="unfinished">Auf tatsächliche Größe zoomen</translation>
    </message>
    <message>
        <source>Zoom to Actual Size</source>
        <translation type="unfinished">Auf tatsächliche Größe zoomen</translation>
    </message>
    <message>
        <source>Add Vector Layer...</source>
        <translation type="unfinished">Vektorlayer hinzufügen...</translation>
    </message>
    <message>
        <source>Add Raster Layer...</source>
        <translation type="unfinished">Rasterlayer hinzufügen...</translation>
    </message>
    <message>
        <source>Add PostGIS Layer...</source>
        <translation type="unfinished">PostGIS-Layer hinzufügen...</translation>
    </message>
    <message>
        <source>W</source>
        <comment>Add a Web Mapping Server Layer</comment>
        <translation type="unfinished">W</translation>
    </message>
    <message>
        <source>Add a Web Mapping Server Layer</source>
        <translation type="unfinished">WMS-Layer hinzufügen</translation>
    </message>
    <message>
        <source>Open Attribute Table</source>
        <translation type="unfinished">Attributetabelle öffnen</translation>
    </message>
    <message>
        <source>Save as Shapefile...</source>
        <translation type="unfinished">Als Shape-Datei speichern...</translation>
    </message>
    <message>
        <source>Save the current layer as a shapefile</source>
        <translation type="unfinished">Aktuellen Layer als Shape-Datei speichern</translation>
    </message>
    <message>
        <source>Save Selection as Shapefile...</source>
        <translation type="unfinished">Auswahl als Shape-Datei speichern...</translation>
    </message>
    <message>
        <source>Save the selection as a shapefile</source>
        <translation type="unfinished">Auswahl als Shape-Datei speichern...</translation>
    </message>
    <message>
        <source>Properties...</source>
        <translation type="unfinished">Eigenschaften...</translation>
    </message>
    <message>
        <source>Set properties of the current layer</source>
        <translation type="unfinished">Eigenschaften des aktuellen Layers setzen</translation>
    </message>
    <message>
        <source>Add to Overview</source>
        <translation type="unfinished">Zur Übersicht hinzufügen</translation>
    </message>
    <message>
        <source>Add All to Overview</source>
        <translation type="unfinished">Alle zur Übersicht hinzufügen</translation>
    </message>
    <message>
        <source>Manage Plugins...</source>
        <translation type="unfinished">Plugins verwalten...</translation>
    </message>
    <message>
        <source>Toggle Full Screen Mode</source>
        <translation type="unfinished">Auf Vollbildmodus schalten</translation>
    </message>
    <message>
        <source>Ctrl-F</source>
        <comment>Toggle fullscreen mode</comment>
        <translation type="unfinished">Vollbildmodus umschalten</translation>
    </message>
    <message>
        <source>Minimize</source>
        <translation type="unfinished">Minimieren</translation>
    </message>
    <message>
        <source>Ctrl+M</source>
        <comment>Minimize Window</comment>
        <translation type="unfinished">Ctrl+M</translation>
    </message>
    <message>
        <source>Minimizes the active window to the dock</source>
        <translation type="unfinished">Minimiert das aktive Fenster ins Dock</translation>
    </message>
    <message>
        <source>Zoom</source>
        <translation type="unfinished">Zoom</translation>
    </message>
    <message>
        <source>Toggles between a predefined size and the window size set by the user</source>
        <translation type="unfinished">Schaltet zwischen voreingestellter und vom Benutzer bestimmten Fenstergröße um</translation>
    </message>
    <message>
        <source>Bring All to Front</source>
        <translation type="unfinished">Alle in den Vordergrund bringen</translation>
    </message>
    <message>
        <source>Bring forward all open windows</source>
        <translation type="unfinished">Alle geöffneten Fenster vorholen</translation>
    </message>
    <message>
        <source>&amp;Edit</source>
        <translation type="unfinished">&amp;Bearbeiten</translation>
    </message>
    <message>
        <source>Panels</source>
        <translation type="unfinished">Bedienfelder</translation>
    </message>
    <message>
        <source>Toolbars</source>
        <translation type="unfinished">Werkzeugkästen</translation>
    </message>
    <message>
        <source>&amp;Window</source>
        <translation type="unfinished">&amp;Fenster</translation>
    </message>
    <message>
        <source>Choose a file name to save the QGIS project file as</source>
        <translation type="unfinished">Name für zu speichernden QGIS-Projektdatei wählen</translation>
    </message>
    <message>
        <source>Choose a file name to save the map image as</source>
        <translation type="unfinished">Name für Datei zum Speichern des Kartenabbilds wählen</translation>
    </message>
    <message>
        <source>Start editing failed</source>
        <translation type="unfinished">Bearbeitungsbeginn schlug fehl</translation>
    </message>
    <message>
        <source>Provider cannot be opened for editing</source>
        <translation type="unfinished">Lieferant kann nicht zum Bearbeiten geöffnet werden</translation>
    </message>
    <message>
        <source>Stop editing</source>
        <translation type="unfinished">Bearbeitung beenden</translation>
    </message>
    <message>
        <source>Do you want to save the changes to layer %1?</source>
        <translation type="unfinished">Wollen Sie die Änderung am Layer %1 speichern?</translation>
    </message>
    <message>
        <source>Could not commit changes to layer %1

Errors:  %2
</source>
        <translation type="unfinished">Änderungen am Layer %1 konnten nicht gespeichern werden
<byte value="x9"/>
Fehler: %2
</translation>
    </message>
    <message>
        <source>Problems during roll back</source>
        <translation type="unfinished">Probleme beim Zurücknehmen der Änderungen</translation>
    </message>
    <message>
        <source>Python Console</source>
        <translation type="unfinished">Python-Konsole</translation>
    </message>
    <message>
        <source>Map coordinates for the current view extents</source>
        <translation type="unfinished">Kartenkoordinaten für den aktuell sichtbaren Ausschnitt</translation>
    </message>
    <message>
        <source></source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgisAppBase</name>
    <message>
        <source>MainWindow</source>
        <translation type="obsolete">Hauptfenster</translation>
    </message>
    <message>
        <source>Legend</source>
        <translation type="obsolete">Legende</translation>
    </message>
    <message>
        <source>Map View</source>
        <translation type="obsolete">Kartenansicht</translation>
    </message>
    <message>
        <source>QGIS</source>
        <translation type="unfinished">QGIS</translation>
    </message>
</context>
<context>
    <name>QgsAbout</name>
    <message>
        <source>About Quantum GIS</source>
        <translation>Über Quantum GIS</translation>
    </message>
    <message>
        <source>Ok</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>About</source>
        <translation>Über</translation>
    </message>
    <message>
        <source>Version</source>
        <translation type="unfinished">Version</translation>
    </message>
    <message>
        <source>What&apos;s New</source>
        <translation>Was ist neu</translation>
    </message>
    <message>
        <source>QGIS Home Page</source>
        <translation>QGIS Homepage</translation>
    </message>
    <message>
        <source>Providers</source>
        <translation>Datenlieferant</translation>
    </message>
    <message>
        <source>Developers</source>
        <translation>Entwickler</translation>
    </message>
    <message>
        <source>Sponsors</source>
        <translation>Sponsoren</translation>
    </message>
    <message>
        <source>Quantum GIS is licensed under the GNU General Public License</source>
        <translation type="unfinished">Quantum GIS ist unter der GNU General Public License lizenziert</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Lucida Grande&apos;; font-size:13pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:16px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:x-large; font-weight:600;&quot;&gt;&lt;span style=&quot; font-size:x-large;&quot;&gt;Quantum GIS (QGIS)&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>http://www.gnu.org/licenses</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Join our user mailing list</source>
        <translation>Abonnieren Sie unsere Mailingliste</translation>
    </message>
    <message>
        <source>&lt;p&gt;The following have sponsored QGIS by contributing money to fund development and other project costs&lt;/p&gt;</source>
        <translation type="obsolete">&lt;p&gt;QGIS wurde durch Geldspenden für Entwicklungs- und andere Projektkosten unterstützt durch&lt;/p&gt;</translation>
    </message>
    <message>
        <source>Name</source>
        <translation type="obsolete">Name</translation>
    </message>
    <message>
        <source>Available QGIS Data Provider Plugins</source>
        <translation type="obsolete">Verfügbare QGIS-Datenlieferantenplugins</translation>
    </message>
    <message>
        <source>Available Qt Database Plugins</source>
        <translation type="obsolete">Verfügbare Qt-Datenbankplugins</translation>
    </message>
    <message>
        <source>Available Qt Image Plugins</source>
        <translation type="obsolete">Verfügbare Qt-Bildformatplugins</translation>
    </message>
</context>
<context>
    <name>QgsAddAttrDialogBase</name>
    <message>
        <source>Add Attribute</source>
        <translation>Attribut hinzufügen</translation>
    </message>
    <message>
        <source>Name:</source>
        <translation type="unfinished">Name:</translation>
    </message>
    <message>
        <source>Type:</source>
        <translation>Typ:</translation>
    </message>
</context>
<context>
    <name>QgsApplication</name>
    <message>
        <source>Exception</source>
        <translation type="unfinished">Ausnahme</translation>
    </message>
</context>
<context>
    <name>QgsAttributeActionDialog</name>
    <message>
        <source>Select an action</source>
        <comment>

File dialog window title</comment>
        <translation type="obsolete">Eine Aktion wählen</translation>
    </message>
    <message>
        <source>Select an action</source>
        <comment>File dialog window title</comment>
        <translation type="unfinished">Eine Aktion wählen</translation>
    </message>
</context>
<context>
    <name>QgsAttributeActionDialogBase</name>
    <message>
        <source>This list contains all actions that have been defined for the current layer. Add actions by entering the details in the controls below and then pressing the Insert action button. Actions can be edited here by double clicking on the item.</source>
        <translation>Diese Liste beinhaltet alle Aktionen, die für aktive Layer definiert wurden. Fügen Sie durch Eingabe von Details in den untenstehenden Kontrollelementen Aktionen hinzu und drücken Sie dann den Knopf mit der Aufschrift &apos;Aktion hinzufügen&apos;. Aktionen können durch Doppelklick auf das entsprechende Element bearbeitet werden.</translation>
    </message>
    <message>
        <source>Move up</source>
        <translation>Verschiebe aufwärts</translation>
    </message>
    <message>
        <source>Move the selected action up</source>
        <translation>Gewählte Aktion aufwärts bewegen</translation>
    </message>
    <message>
        <source>Move down</source>
        <translation>Verschiebe abwärts</translation>
    </message>
    <message>
        <source>Move the selected action down</source>
        <translation>Selektierte Aktion abwärts bewegen</translation>
    </message>
    <message>
        <source>Remove</source>
        <translation>Entfernen</translation>
    </message>
    <message>
        <source>Remove the selected action</source>
        <translation>Gewählte Aktion entfernen</translation>
    </message>
    <message>
        <source>Enter the name of an action here. The name should be unique (qgis will make it unique if necessary).</source>
        <translation>Bitte Namen der Aktion eingeben. Der Name sollte eindeutig sein (QGIS macht ihn eindeutig, falls notwendig). </translation>
    </message>
    <message>
        <source>Enter the action name here</source>
        <translation>Namen der Aktion hier eingeben</translation>
    </message>
    <message>
        <source>Enter the action command here</source>
        <translation>Kommando für die Aktion hier eingeben</translation>
    </message>
    <message>
        <source>Insert action</source>
        <translation>Aktion hinzufügen</translation>
    </message>
    <message>
        <source>Inserts the action into the list above</source>
        <translation>Aktion in die obenstehende Liste einfügen</translation>
    </message>
    <message>
        <source>Update action</source>
        <translation>Aktualisiere die Aktion</translation>
    </message>
    <message>
        <source>Update the selected action</source>
        <translation>Aktualisiere die markierte Aktion</translation>
    </message>
    <message>
        <source>Insert field</source>
        <translation>Attribut einfügen</translation>
    </message>
    <message>
        <source>Inserts the selected field into the action, prepended with a %</source>
        <translation>Fügt das markierte Attribut mit vorangestelltem &apos;%&apos; in die Aktion ein</translation>
    </message>
    <message>
        <source>The valid attribute names for this layer</source>
        <translation>Die gültigen Attributnamen für diesen Layer</translation>
    </message>
    <message>
        <source>Capture output</source>
        <translation>Ausgaben aufzeichnen</translation>
    </message>
    <message>
        <source>Captures any output from the action</source>
        <translation>Ausgaben der Aktion aufzeichnen</translation>
    </message>
    <message>
        <source>Captures the standard output or error generated by the action and displays it in a dialog box</source>
        <translation>Nimmt Ausgaben ader Aktion auf Standardausgabe- oder -fehlerkanal auf und zeigt ihn in einem Dialog an</translation>
    </message>
    <message>
        <source>Enter the action here. This can be any program, script or command that is available on your system. When the action is invoked any set of characters that start with a % and then have the name of a field will be replaced by the value of that field. The special characters %% will be replaced by the value of the field that was selected. Double quote marks group text into single arguments to the program, script or command. Double quotes will be ignored if preceeded by a backslash</source>
        <translation type="unfinished">Geben Sie hier die Aktion ein. Dies kann jedes Programm, Skript oder Kommando sein, dass in Ihrem System verfügbar ist.  Wenn die Aktion ausgeführt wird jeder durch % eingeleiteter Feldname durch den Feldwert ersetzt.  Die besondere Zeichenfolge %% wird durch den Wert des gewählten Felds ersetzt.  Mit Anführungszeichen können mehrere Wörter zu einem Argument der Aktion zusammengefaßt werden. Für mit Backslash (\) eingeleitete Anführungszeichen gilt dies nicht.</translation>
    </message>
    <message>
        <source>Attribute Actions</source>
        <translation type="unfinished">Attributaktionen</translation>
    </message>
    <message>
        <source>Action properties</source>
        <translation type="unfinished">Aktionseigenschaften</translation>
    </message>
    <message>
        <source>Name</source>
        <translation type="unfinished">Name</translation>
    </message>
    <message>
        <source>Action</source>
        <translation type="unfinished">Aktion</translation>
    </message>
    <message>
        <source>Browse for action</source>
        <translation type="unfinished">Aktionen durchsuchen</translation>
    </message>
    <message>
        <source>Click to browse for an action</source>
        <translation type="unfinished">Zum Aktionen durchsuchen anklicken</translation>
    </message>
    <message>
        <source>...</source>
        <translation type="unfinished">...</translation>
    </message>
    <message>
        <source>Capture</source>
        <translation type="unfinished">Erfassen</translation>
    </message>
    <message>
        <source>Clicking the button will let you select an application to use as the action</source>
        <translation type="unfinished">Mit diesem Knopf kann man eine Applikation für diese Aktion wählen</translation>
    </message>
</context>
<context>
    <name>QgsAttributeDialog</name>
    <message>
        <source> (int)</source>
        <translation> (Ganzzahl)</translation>
    </message>
    <message>
        <source> (dbl)</source>
        <translation> (Fließkommazahl)</translation>
    </message>
    <message>
        <source> (txt)</source>
        <translation> (Text)</translation>
    </message>
    <message>
        <source>...</source>
        <translation type="unfinished">...</translation>
    </message>
    <message>
        <source>Select a file</source>
        <translation type="unfinished">Datei wählen</translation>
    </message>
</context>
<context>
    <name>QgsAttributeDialogBase</name>
    <message>
        <source>Enter Attribute Values</source>
        <translation>Attributwert eingeben</translation>
    </message>
</context>
<context>
    <name>QgsAttributeTable</name>
    <message>
        <source>Run action</source>
        <translation>Aktion starten</translation>
    </message>
    <message>
        <source>Updating selection...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Abort</source>
        <translation type="unfinished">Abbrechen</translation>
    </message>
</context>
<context>
    <name>QgsAttributeTableBase</name>
    <message>
        <source>Attribute Table</source>
        <translation>Attributtabelle</translation>
    </message>
    <message>
        <source>Ctrl+N</source>
        <translation type="obsolete">Ctrl+N</translation>
    </message>
    <message>
        <source>Ctrl+S</source>
        <translation type="unfinished">Ctrl+S</translation>
    </message>
    <message>
        <source>Invert selection</source>
        <translation>Auswahl umkehren</translation>
    </message>
    <message>
        <source>Ctrl+T</source>
        <translation type="unfinished">Die meisten Werkzeugleisten ausblenden</translation>
    </message>
    <message>
        <source>Move selected to top</source>
        <translation>Ausgewählte Objekte nach oben</translation>
    </message>
    <message>
        <source>Remove selection</source>
        <translation>Auswahl löschen</translation>
    </message>
    <message>
        <source>Copy selected rows to clipboard (Ctrl+C)</source>
        <translation>Ausgewählte Zeilen in die Zwischenablage kopieren (Ctrl+C).</translation>
    </message>
    <message>
        <source>Copies the selected rows to the clipboard</source>
        <translation>Kopiert die gewählten Zeilen in die Zwischenablage.</translation>
    </message>
    <message>
        <source>Ctrl+C</source>
        <translation>Ctrl+C</translation>
    </message>
    <message>
        <source>in</source>
        <translation>in</translation>
    </message>
    <message>
        <source>Search</source>
        <translation>Suchen</translation>
    </message>
    <message>
        <source>Adva&amp;nced...</source>
        <translation>Erw&amp;eitert...</translation>
    </message>
    <message>
        <source>Alt+N</source>
        <translation>Alt+N</translation>
    </message>
    <message>
        <source>New column</source>
        <translation type="obsolete">Neue Attributspalte</translation>
    </message>
    <message>
        <source>Delete column</source>
        <translation type="obsolete">Lösche Attributspalte</translation>
    </message>
    <message>
        <source>Zoom map to the selected rows (Ctrl-F)</source>
        <translation type="obsolete">Zoome Karte zum ausgewählten Spalteneintrag (Ctrl-F)</translation>
    </message>
    <message>
        <source>Zoom map to the selected rows</source>
        <translation>Zoome Karte zu den ausgewählten Spalteneinträgen</translation>
    </message>
    <message>
        <source>Ctrl+F</source>
        <translation type="obsolete">Ctrl+F</translation>
    </message>
    <message>
        <source>Search for</source>
        <translation type="unfinished">Suchen nach</translation>
    </message>
    <message>
        <source>Toggle editing mode</source>
        <translation type="unfinished">Bearbeitungsmodus umschalten</translation>
    </message>
    <message>
        <source>Click to toggle table editing</source>
        <translation type="unfinished">Anklicken um den Tabellenbearbeitungsmodus umzuschalten</translation>
    </message>
    <message>
        <source>Zoom map to the selected rows (Ctrl-J)</source>
        <translation type="unfinished">Zu den selektierten Zeilen zoomen</translation>
    </message>
    <message>
        <source>Ctrl+J</source>
        <translation type="unfinished">Ctrl+J</translation>
    </message>
</context>
<context>
    <name>QgsAttributeTableDisplay</name>
    <message>
        <source>select</source>
        <translation>Auswahl</translation>
    </message>
    <message>
        <source>select and bring to top</source>
        <translation>Auswählen und nach oben verschieben</translation>
    </message>
    <message>
        <source>show only matching</source>
        <translation>Nur Treffer zeigen.</translation>
    </message>
    <message>
        <source>Search string parsing error</source>
        <translation>Fehler im Suchbegriff.</translation>
    </message>
    <message>
        <source>Search results</source>
        <translation>Suchergebnisse</translation>
    </message>
    <message>
        <source>You&apos;ve supplied an empty search string.</source>
        <translation>Sie haben einen leeren Suchbegriff eingegeben.</translation>
    </message>
    <message>
        <source>Error during search</source>
        <translation>Fehler beim Suchen</translation>
    </message>
    <message>
        <source>No matching features found.</source>
        <translation>Keine Treffer gefunden.</translation>
    </message>
    <message>
        <source>Name conflict</source>
        <translation type="obsolete">Namenskonflikt</translation>
    </message>
    <message>
        <source>Stop editing</source>
        <translation type="obsolete">Bearbeitung beenden</translation>
    </message>
    <message>
        <source>Do you want to save the changes?</source>
        <translation type="obsolete">Sollen die Änderungen gespeichert werden?</translation>
    </message>
    <message>
        <source>Error</source>
        <translation type="obsolete">Fehler</translation>
    </message>
    <message>
        <source>The attribute could not be inserted. The name already exists in the table.</source>
        <translation type="obsolete">Das Attribut konnte nicht eingefügt werden, da der Name bereits vorhanden ist.</translation>
    </message>
    <message>
        <source>Could not commit changes - changes are still pending</source>
        <translation type="obsolete">Konnten Änderungen nicht speichern - sie stehen weiter an.</translation>
    </message>
    <message>
        <source>Editing not permitted</source>
        <translation type="obsolete">Bearbeitung nicht erlaubt</translation>
    </message>
    <message>
        <source>The data provider is read only, editing is not allowed.</source>
        <translation type="obsolete">Bearbeitung nicht möglich, da der Datenlieferant nur lesbar ist.</translation>
    </message>
    <message>
        <source>Start editing</source>
        <translation type="obsolete">Bearbeitun&amp;g starten</translation>
    </message>
    <message>
        <source>Attribute table - </source>
        <translation type="unfinished">Attributtabelle - </translation>
    </message>
    <message>
        <source>QGIS</source>
        <translation type="unfinished">QGIS</translation>
    </message>
    <message>
        <source>File</source>
        <translation type="unfinished">Datei</translation>
    </message>
    <message>
        <source>Close</source>
        <translation type="unfinished">Schließen</translation>
    </message>
    <message>
        <source>Ctrl+W</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Edit</source>
        <translation type="unfinished">Bearbeiten</translation>
    </message>
    <message>
        <source>&amp;Undo</source>
        <translation type="unfinished">&amp;Zurücknehmen</translation>
    </message>
    <message>
        <source>Ctrl+Z</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cu&amp;t</source>
        <translation type="unfinished">&amp;Aussschneiden</translation>
    </message>
    <message>
        <source>Ctrl+X</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Copy</source>
        <translation type="unfinished">&amp;Kopieren</translation>
    </message>
    <message>
        <source>Ctrl+C</source>
        <translation type="unfinished">Ctrl+C</translation>
    </message>
    <message>
        <source>&amp;Paste</source>
        <translation type="unfinished">&amp;Einfügen</translation>
    </message>
    <message>
        <source>Ctrl+V</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Delete</source>
        <translation type="unfinished">Löschen</translation>
    </message>
    <message>
        <source>Layer</source>
        <translation type="unfinished">Layer</translation>
    </message>
    <message>
        <source>Zoom to Selection</source>
        <translation type="unfinished">Zur Auswahl zoomen</translation>
    </message>
    <message>
        <source>Ctrl+J</source>
        <translation type="unfinished">Ctrl+J</translation>
    </message>
    <message>
        <source>Toggle Editing</source>
        <translation type="unfinished">Bearbeitungsmodus umschalten</translation>
    </message>
    <message>
        <source>Table</source>
        <translation type="unfinished">Tabelle</translation>
    </message>
    <message>
        <source>Move to Top</source>
        <translation type="unfinished">Nach Oben bringen</translation>
    </message>
    <message>
        <source>Invert</source>
        <translation type="unfinished">Umkehren</translation>
    </message>
    <message>
        <source>bad_alloc exception</source>
        <translation type="unfinished">Speicher-Fehler</translation>
    </message>
    <message>
        <source>Filling the attribute table has been stopped because there was no more virtual memory left</source>
        <translation type="unfinished">Das Auffüllen der Attributtabelle wurde beendet, da kein virtueller Speicher mehr zur Verfügung steht</translation>
    </message>
    <message>
        <source>Found %d matching features.</source>
        <translation type="obsolete">
        
        </translation>
    </message>
</context>
<context>
    <name>QgsBookmarks</name>
    <message>
        <source>Really Delete?</source>
        <translation>Wirklich löschen?</translation>
    </message>
    <message>
        <source>Are you sure you want to delete the </source>
        <translation>Sind Sie sicher, dass Sie das Lesezeichen </translation>
    </message>
    <message>
        <source> bookmark?</source>
        <translation> löschen wollen?</translation>
    </message>
    <message>
        <source>Error deleting bookmark</source>
        <translation>Fehler beim Löschen eines Lesezeichens</translation>
    </message>
    <message>
        <source>Failed to delete the </source>
        <translation>Das Löschen des </translation>
    </message>
    <message>
        <source> bookmark from the database. The database said:
</source>
        <translation> Lesezeichens aus der Datenbank schlug fehl. Die Datenbank meldete:</translation>
    </message>
    <message>
        <source>&amp;Delete</source>
        <translation type="unfinished">&amp;Löschen</translation>
    </message>
    <message>
        <source>&amp;Zoom to</source>
        <translation type="unfinished">&amp;Zoom nach</translation>
    </message>
</context>
<context>
    <name>QgsBookmarksBase</name>
    <message>
        <source>Geospatial Bookmarks</source>
        <translation>Räumliches Lesezeichen</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Name</translation>
    </message>
    <message>
        <source>Project</source>
        <translation>Projekt</translation>
    </message>
    <message>
        <source>Extent</source>
        <translation>Ausdehnung</translation>
    </message>
    <message>
        <source>Id</source>
        <translation>Id</translation>
    </message>
</context>
<context>
    <name>QgsComposer</name>
    <message>
        <source>Choose a filename to save the map image as</source>
        <translation type="obsolete">Dateinamen zum Speichern des Kartenbildes wählen</translation>
    </message>
    <message>
        <source>Choose a filename to save the map as</source>
        <translation type="obsolete">Dateinamen zum Speichern der Karte wählen</translation>
    </message>
    <message>
        <source> for read/write</source>
        <translation type="obsolete">um zu lesen/ schreiben</translation>
    </message>
    <message>
        <source>Error in Print</source>
        <translation type="obsolete">Fehler beim Drucken</translation>
    </message>
    <message>
        <source>Cannot seek</source>
        <translation type="obsolete">Kann nicht suchen</translation>
    </message>
    <message>
        <source>Cannot overwrite BoundingBox</source>
        <translation type="obsolete">Kann die Umgrenzung nicht überschreiben</translation>
    </message>
    <message>
        <source>Cannot find BoundingBox</source>
        <translation type="obsolete">Kann die Umgrenzung nicht finden.</translation>
    </message>
    <message>
        <source>Cannot overwrite translate</source>
        <translation type="obsolete">Kann Übersetzung nicht überschreiben.</translation>
    </message>
    <message>
        <source>Cannot find translate</source>
        <translation type="obsolete">Kann Übersetzung nicht finden.</translation>
    </message>
    <message>
        <source>File IO Error</source>
        <translation type="obsolete">Dateifehler</translation>
    </message>
    <message>
        <source>Paper does not match</source>
        <translation type="obsolete">Papier passt nicht</translation>
    </message>
    <message>
        <source>The selected paper size does not match the composition size</source>
        <translation type="obsolete">Die ausgewählte Papiergröße passt nicht zur Auswahl.</translation>
    </message>
    <message>
        <source>Big image</source>
        <translation>Großes Bild</translation>
    </message>
    <message>
        <source>To create image </source>
        <translation>Um ein Bild zu erzeugen </translation>
    </message>
    <message>
        <source> requires circa </source>
        <translation>werden ca. </translation>
    </message>
    <message>
        <source> MB of memory</source>
        <translation> MB Speicher benötigt.</translation>
    </message>
    <message>
        <source>QGIS - print composer</source>
        <translation>QGIS - Druckzusammenstellung</translation>
    </message>
    <message>
        <source>Map 1</source>
        <translation>Karte 1</translation>
    </message>
    <message>
        <source>Couldn&apos;t open </source>
        <translation type="obsolete">Kann nicht öffnen </translation>
    </message>
    <message>
        <source>format</source>
        <translation>Format</translation>
    </message>
    <message>
        <source>SVG warning</source>
        <translation>SVG-Warnung</translation>
    </message>
    <message>
        <source>Don&apos;t show this message again</source>
        <translation>Diese Nachricht nicht mehr anzeigen.</translation>
    </message>
    <message>
        <source>SVG Format</source>
        <translation>SVG-Format</translation>
    </message>
    <message>
        <source>Move Content</source>
        <translation type="unfinished">Inhalt verschieben</translation>
    </message>
    <message>
        <source>Move item content</source>
        <translation type="unfinished">Den Elementinhalt verschieben</translation>
    </message>
    <message>
        <source>&amp;Group</source>
        <translation type="unfinished">&amp;Gruppe</translation>
    </message>
    <message>
        <source>Group items</source>
        <translation type="unfinished">Gruppenelemente</translation>
    </message>
    <message>
        <source>&amp;Ungroup</source>
        <translation type="unfinished">&amp;Gruppe auflösen</translation>
    </message>
    <message>
        <source>Ungroup items</source>
        <translation type="unfinished">Die Gruppe auflösen</translation>
    </message>
    <message>
        <source>Raise</source>
        <translation type="unfinished">Hervorholen</translation>
    </message>
    <message>
        <source>Raise selected items</source>
        <translation type="unfinished">Ausgewählte Elemente in den Vordergrund bringen</translation>
    </message>
    <message>
        <source>Lower</source>
        <translation type="unfinished">Versenken</translation>
    </message>
    <message>
        <source>Lower selected items</source>
        <translation type="unfinished">Gewählte Elemente in den Hintergrund bringen</translation>
    </message>
    <message>
        <source>Bring to Front</source>
        <translation type="unfinished">In den Vordergrund holen</translation>
    </message>
    <message>
        <source>Move selected items to top</source>
        <translation type="unfinished">Gewählte Objekte in den Vordergrund verschieben</translation>
    </message>
    <message>
        <source>Send to Back</source>
        <translation type="unfinished">In den Hintergrund schicken</translation>
    </message>
    <message>
        <source>Move selected items to bottom</source>
        <translation type="unfinished">Die ausgewählten Elemente in den Hintergrund stellen</translation>
    </message>
    <message>
        <source>QGIS</source>
        <translation type="unfinished">QGIS</translation>
    </message>
    <message>
        <source>File</source>
        <translation type="unfinished">Datei</translation>
    </message>
    <message>
        <source>Close</source>
        <translation type="unfinished">Schließen</translation>
    </message>
    <message>
        <source>Ctrl+W</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Edit</source>
        <translation type="unfinished">Bearbeiten</translation>
    </message>
    <message>
        <source>&amp;Undo</source>
        <translation type="unfinished">&amp;Zurücknehmen</translation>
    </message>
    <message>
        <source>Ctrl+Z</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cu&amp;t</source>
        <translation type="unfinished">&amp;Ausschneiden</translation>
    </message>
    <message>
        <source>Ctrl+X</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Copy</source>
        <translation type="unfinished">&amp;Kopieren</translation>
    </message>
    <message>
        <source>Ctrl+C</source>
        <translation type="unfinished">Ctrl+C</translation>
    </message>
    <message>
        <source>&amp;Paste</source>
        <translation type="unfinished">&amp;Einfügen</translation>
    </message>
    <message>
        <source>Ctrl+V</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Delete</source>
        <translation type="unfinished">Löschen</translation>
    </message>
    <message>
        <source>View</source>
        <translation type="unfinished">Ansicht</translation>
    </message>
    <message>
        <source>Layout</source>
        <translation type="unfinished">Anordnung</translation>
    </message>
    <message>
        <source>Choose a file name to save the map image as</source>
        <translation type="unfinished">Name der Datei des zu speichernden Kartenabbild wählen</translation>
    </message>
    <message>
        <source>Choose a file name to save the map as</source>
        <translation type="unfinished">Einen Dateinamen zum Speichern des Kartenabbilds wählen</translation>
    </message>
    <message>
        <source>Project contains WMS layers</source>
        <translation type="unfinished">Projekt enthält WMS-Layer</translation>
    </message>
    <message>
        <source>Some WMS servers (e.g. UMN mapserver) have a limit for the WIDTH and HEIGHT parameter. Printing layers from such servers may exceed this limit. If this is the case, the WMS layer will not be printed</source>
        <translation type="unfinished">Einige WMS-Server (z.B. UMN Mapserver) haben Begrenzungen für die WIDTH- und HEIGHT-Parameter.  Falls diese Begrenzungen beim Ausdruck überschritten werden, werden diese WMS-Layer nicht gedruckt.</translation>
    </message>
</context>
<context>
    <name>QgsComposerBase</name>
    <message>
        <source>General</source>
        <translation>Allgemein</translation>
    </message>
    <message>
        <source>Composition</source>
        <translation>Zusammenstellung</translation>
    </message>
    <message>
        <source>Item</source>
        <translation>Eintrag</translation>
    </message>
    <message>
        <source>&amp;Open Template ...</source>
        <translation type="obsolete">&amp;Schablone öffnen...</translation>
    </message>
    <message>
        <source>Save Template &amp;As...</source>
        <translation type="obsolete">Schablone speichern &amp;als... </translation>
    </message>
    <message>
        <source>&amp;Print...</source>
        <translation>&amp;Drucken...</translation>
    </message>
    <message>
        <source>Add new map</source>
        <translation>Neue Karte hinzufügen</translation>
    </message>
    <message>
        <source>Add new label</source>
        <translation>Neue Beschriftung hinzufügen</translation>
    </message>
    <message>
        <source>Add new vect legend</source>
        <translation>Neue Vektorlegende hinzufügen</translation>
    </message>
    <message>
        <source>Select/Move item</source>
        <translation>Eintrag wählen/verschieben</translation>
    </message>
    <message>
        <source>Export as image</source>
        <translation type="obsolete">Exportiere als Bild</translation>
    </message>
    <message>
        <source>Export as SVG</source>
        <translation type="obsolete">Exportiere als SVG</translation>
    </message>
    <message>
        <source>Add new scalebar</source>
        <translation>Neuen Massstab hinzufügen</translation>
    </message>
    <message>
        <source>Refresh view</source>
        <translation>Aktualisiere Ansicht</translation>
    </message>
    <message>
        <source>MainWindow</source>
        <translation>MainWindow</translation>
    </message>
    <message>
        <source>Zoom All</source>
        <translation type="obsolete">Auf Alles zoomen</translation>
    </message>
    <message>
        <source>Zoom In</source>
        <translation>Hineinzoomen</translation>
    </message>
    <message>
        <source>Zoom Out</source>
        <translation>Hinauszoomen</translation>
    </message>
    <message>
        <source>Add Image</source>
        <translation>Bild hinzufügen</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Schließen</translation>
    </message>
    <message>
        <source>Help</source>
        <translation>Hilfe</translation>
    </message>
    <message>
        <source>&amp;Open Template...</source>
        <translation type="obsolete">&amp;Vorlage öffnen</translation>
    </message>
    <message>
        <source>Zoom Full</source>
        <translation type="unfinished">Volle Ausdehnung</translation>
    </message>
    <message>
        <source>Add Map</source>
        <translation type="unfinished">Karte hinzufügen</translation>
    </message>
    <message>
        <source>Add Label</source>
        <translation type="unfinished">Beschriftung hinzufügen</translation>
    </message>
    <message>
        <source>Add Vector Legend</source>
        <translation type="unfinished">Vektorlegende hinzufügen</translation>
    </message>
    <message>
        <source>Move Item</source>
        <translation type="unfinished">Element verschieben</translation>
    </message>
    <message>
        <source>Export as Image...</source>
        <translation type="unfinished">Speichern als Rasterbild...</translation>
    </message>
    <message>
        <source>Export as SVG...</source>
        <translation type="unfinished">Speichern als SVG...</translation>
    </message>
    <message>
        <source>Add Scalebar</source>
        <translation type="unfinished">Maßstab hinzufügen</translation>
    </message>
    <message>
        <source>Refresh</source>
        <translation type="unfinished">Auffrischen</translation>
    </message>
</context>
<context>
    <name>QgsComposerItemWidgetBase</name>
    <message>
        <source>Form</source>
        <translation type="unfinished">Formular</translation>
    </message>
    <message>
        <source>Composer item properties</source>
        <translation type="unfinished">Elementeigenschaften</translation>
    </message>
    <message>
        <source>Color:</source>
        <translation type="unfinished">Farbe:</translation>
    </message>
    <message>
        <source>Frame...</source>
        <translation type="unfinished">Rahmen...</translation>
    </message>
    <message>
        <source>Background...</source>
        <translation type="unfinished">Hintergrund...</translation>
    </message>
    <message>
        <source>Opacity:</source>
        <translation type="unfinished">Opazität:</translation>
    </message>
    <message>
        <source>Outline width: </source>
        <translation type="unfinished">Rahmenstärke:</translation>
    </message>
    <message>
        <source>Frame</source>
        <translation type="unfinished">Rahmen</translation>
    </message>
</context>
<context>
    <name>QgsComposerLabelBase</name>
    <message>
        <source>Label Options</source>
        <translation type="obsolete">Beschriftungsoption</translation>
    </message>
    <message>
        <source>Font</source>
        <translation type="obsolete">Schrift</translation>
    </message>
    <message>
        <source>Box</source>
        <translation type="obsolete">Box</translation>
    </message>
</context>
<context>
    <name>QgsComposerLabelWidgetBase</name>
    <message>
        <source>Label Options</source>
        <translation type="unfinished">Beschriftungsoption</translation>
    </message>
    <message>
        <source>Font</source>
        <translation type="unfinished">Schrift</translation>
    </message>
    <message>
        <source>Margin (mm):</source>
        <translation type="unfinished">Rand (mm)</translation>
    </message>
</context>
<context>
    <name>QgsComposerLegendItemDialogBase</name>
    <message>
        <source>Legend item properties</source>
        <translation type="unfinished">Eigenschaften des Legendenelements</translation>
    </message>
    <message>
        <source>Item text:</source>
        <translation type="unfinished">Elementtext:</translation>
    </message>
</context>
<context>
    <name>QgsComposerLegendWidgetBase</name>
    <message>
        <source>Barscale Options</source>
        <translation type="unfinished">Optionen für Massstabsbalken</translation>
    </message>
    <message>
        <source>General</source>
        <translation type="unfinished">Allgemein</translation>
    </message>
    <message>
        <source>Title:</source>
        <translation type="unfinished">Titel:</translation>
    </message>
    <message>
        <source>Font:</source>
        <translation type="unfinished">Schriftart:</translation>
    </message>
    <message>
        <source>Title...</source>
        <translation type="unfinished">Titel...</translation>
    </message>
    <message>
        <source>Layer...</source>
        <translation type="unfinished">Layer...</translation>
    </message>
    <message>
        <source>Item...</source>
        <translation type="unfinished">Element...</translation>
    </message>
    <message>
        <source>Symbol width: </source>
        <translation type="unfinished">Symbolbreite: </translation>
    </message>
    <message>
        <source>Symbol height:</source>
        <translation type="unfinished">Symbolhöhe:</translation>
    </message>
    <message>
        <source>Layer space: </source>
        <translation type="unfinished">Layerraum: </translation>
    </message>
    <message>
        <source>Symbol space:</source>
        <translation type="unfinished">Symbolraum: </translation>
    </message>
    <message>
        <source>Icon label space:</source>
        <translation type="unfinished">Iconbeschriftungsraum:</translation>
    </message>
    <message>
        <source>Box space:</source>
        <translation type="unfinished">Rahmenraum:</translation>
    </message>
    <message>
        <source>Legend items</source>
        <translation type="unfinished">Legendenelemente</translation>
    </message>
    <message>
        <source>down</source>
        <translation type="unfinished">hinunter</translation>
    </message>
    <message>
        <source>up</source>
        <translation type="unfinished">hinauf</translation>
    </message>
    <message>
        <source>remove</source>
        <translation type="unfinished">entfernen</translation>
    </message>
    <message>
        <source>edit...</source>
        <translation type="unfinished">ändern..</translation>
    </message>
    <message>
        <source>update</source>
        <translation type="unfinished">aktualisieren</translation>
    </message>
    <message>
        <source>update all</source>
        <translation type="unfinished">alle aktualisieren</translation>
    </message>
</context>
<context>
    <name>QgsComposerMap</name>
    <message>
        <source>Extent (calculate scale)</source>
        <translation type="obsolete">Ausdehnung (Massstabsberechnung)</translation>
    </message>
    <message>
        <source>Scale (calculate extent)</source>
        <translation type="obsolete">Massstab (Ausdenungsberechnung)</translation>
    </message>
    <message>
        <source>Map %1</source>
        <translation type="obsolete">Karte %1</translation>
    </message>
    <message>
        <source>Cache</source>
        <translation type="obsolete">Cache</translation>
    </message>
    <message>
        <source>Render</source>
        <translation type="obsolete">Zeichnen</translation>
    </message>
    <message>
        <source>Rectangle</source>
        <translation type="obsolete">Rechteck</translation>
    </message>
    <message>
        <source>Map</source>
        <translation type="unfinished">Karte</translation>
    </message>
    <message>
        <source>Map will be printed here</source>
        <translation type="unfinished">Karte wird hier gedruckt</translation>
    </message>
</context>
<context>
    <name>QgsComposerMapBase</name>
    <message>
        <source>Map options</source>
        <translation type="obsolete">Kartenoptionen</translation>
    </message>
    <message>
        <source>&lt;b&gt;Map&lt;/b&gt;</source>
        <translation type="obsolete">&lt;b&gt;Karte&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Set</source>
        <translation type="obsolete">Setzen</translation>
    </message>
    <message>
        <source>Width</source>
        <translation type="obsolete">Breite</translation>
    </message>
    <message>
        <source>Height</source>
        <translation type="obsolete">Höhe</translation>
    </message>
    <message>
        <source>Set Extent</source>
        <translation type="obsolete">Ausdehnung setzen</translation>
    </message>
    <message>
        <source>Set map extent to current extent in QGIS map canvas</source>
        <translation type="obsolete">Ausdehnung auf die Ausmasse des QGIS Kartenausschnitts setzen</translation>
    </message>
    <message>
        <source>Line width scale</source>
        <translation type="obsolete">Linienbreitemassstab</translation>
    </message>
    <message>
        <source>Width of one unit in millimeters</source>
        <translation type="obsolete">Breite einer Einheit in Millimeter</translation>
    </message>
    <message>
        <source>Symbol scale</source>
        <translation type="obsolete">Symbol Massstab</translation>
    </message>
    <message>
        <source>Font size scale</source>
        <translation type="obsolete">Massstab der Schriftgrösse</translation>
    </message>
    <message>
        <source>Frame</source>
        <translation type="obsolete">Rahmen</translation>
    </message>
    <message>
        <source>Preview</source>
        <translation type="obsolete">Vorschau</translation>
    </message>
    <message>
        <source>1:</source>
        <translation type="obsolete">1:</translation>
    </message>
    <message>
        <source>Scale:</source>
        <translation type="obsolete">Massstab:</translation>
    </message>
</context>
<context>
    <name>QgsComposerMapWidget</name>
    <message>
        <source>Cache</source>
        <translation type="unfinished">Cache</translation>
    </message>
    <message>
        <source>Rectangle</source>
        <translation type="unfinished">Rechteck</translation>
    </message>
    <message>
        <source>Render</source>
        <translation type="unfinished">Zeichnen</translation>
    </message>
</context>
<context>
    <name>QgsComposerMapWidgetBase</name>
    <message>
        <source>Map options</source>
        <translation type="unfinished">Kartenoptionen</translation>
    </message>
    <message>
        <source>&lt;b&gt;Map&lt;/b&gt;</source>
        <translation type="unfinished">&lt;b&gt;Karte&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Width</source>
        <translation type="unfinished">Breite</translation>
    </message>
    <message>
        <source>Height</source>
        <translation type="unfinished">Höhe</translation>
    </message>
    <message>
        <source>Scale:</source>
        <translation type="unfinished">Massstab:</translation>
    </message>
    <message>
        <source>1:</source>
        <translation type="unfinished">1:</translation>
    </message>
    <message>
        <source>Map extent</source>
        <translation type="unfinished">Kartenausmaß</translation>
    </message>
    <message>
        <source>X min:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Y min:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>X max:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Y max:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>set to map canvas extent</source>
        <translation type="unfinished">Anzeigegrenzen übernehmen</translation>
    </message>
    <message>
        <source>Preview</source>
        <translation type="unfinished">Vorschau</translation>
    </message>
    <message>
        <source>Update preview</source>
        <translation type="unfinished">Vorschau aktualisieren</translation>
    </message>
</context>
<context>
    <name>QgsComposerPicture</name>
    <message>
        <source>Warning</source>
        <translation type="obsolete">Warnung</translation>
    </message>
    <message>
        <source>Cannot load picture.</source>
        <translation type="obsolete">Kann das Bild nicht laden.</translation>
    </message>
    <message>
        <source>Choose a file</source>
        <translation type="obsolete">Wählen Sie eine Datei</translation>
    </message>
    <message>
        <source>Pictures (</source>
        <translation type="obsolete">Bilder (</translation>
    </message>
</context>
<context>
    <name>QgsComposerPictureBase</name>
    <message>
        <source>Picture Options</source>
        <translation type="obsolete">Bild-Optionen</translation>
    </message>
    <message>
        <source>Frame</source>
        <translation type="obsolete">Rahmen</translation>
    </message>
    <message>
        <source>Angle</source>
        <translation type="obsolete">Winkel</translation>
    </message>
    <message>
        <source>Width</source>
        <translation type="obsolete">Breite</translation>
    </message>
    <message>
        <source>Height</source>
        <translation type="obsolete">Höhe</translation>
    </message>
    <message>
        <source>Browse</source>
        <translation type="obsolete">Durchsuchen</translation>
    </message>
</context>
<context>
    <name>QgsComposerPictureWidget</name>
    <message>
        <source>Select svg or image file</source>
        <translation type="unfinished">SVG oder Rasterbild wählen</translation>
    </message>
</context>
<context>
    <name>QgsComposerPictureWidgetBase</name>
    <message>
        <source>Picture Options</source>
        <translation type="unfinished">Bild-Optionen</translation>
    </message>
    <message>
        <source>Browse...</source>
        <translation type="unfinished">Suchen...</translation>
    </message>
    <message>
        <source>Width:</source>
        <translation type="unfinished">Breite:</translation>
    </message>
    <message>
        <source>Height:</source>
        <translation type="unfinished">Höhe:</translation>
    </message>
    <message>
        <source>Rotation:</source>
        <translation type="unfinished">Drehung:</translation>
    </message>
</context>
<context>
    <name>QgsComposerScaleBar</name>
    <message>
        <source>Single Box</source>
        <translation type="unfinished">Einfachrahmen</translation>
    </message>
    <message>
        <source>Double Box</source>
        <translation type="unfinished">Doppelrahmen</translation>
    </message>
    <message>
        <source>Line Ticks Middle</source>
        <translation type="unfinished">Mittige Linieneinteilung</translation>
    </message>
    <message>
        <source>Line Ticks Down</source>
        <translation type="unfinished">Linieneinteilung unten</translation>
    </message>
    <message>
        <source>Line Ticks Up</source>
        <translation type="unfinished">Linieneinteilung oben</translation>
    </message>
    <message>
        <source>Numeric</source>
        <translation type="unfinished">Numerisch</translation>
    </message>
</context>
<context>
    <name>QgsComposerScaleBarWidget</name>
    <message>
        <source>Single Box</source>
        <translation type="unfinished">Einfacher Rahmen</translation>
    </message>
    <message>
        <source>Double Box</source>
        <translation type="unfinished">Doppelter Rahmen</translation>
    </message>
    <message>
        <source>Line Ticks Middle</source>
        <translation type="unfinished">Mittige Linieneinteilung</translation>
    </message>
    <message>
        <source>Line Ticks Down</source>
        <translation type="unfinished">Linieneinteilung unten</translation>
    </message>
    <message>
        <source>Line Ticks Up</source>
        <translation type="unfinished">Linieneinteilung oben</translation>
    </message>
    <message>
        <source>Numeric</source>
        <translation type="unfinished">Numerisch</translation>
    </message>
    <message>
        <source>Map </source>
        <translation type="unfinished">Karte </translation>
    </message>
</context>
<context>
    <name>QgsComposerScaleBarWidgetBase</name>
    <message>
        <source>Barscale Options</source>
        <translation type="unfinished">Optionen für Massstabsbalken</translation>
    </message>
    <message>
        <source>Segment size (map units):</source>
        <translation type="unfinished">Segmentgröße (Karteneinheiten)</translation>
    </message>
    <message>
        <source>Map units per bar unit:</source>
        <translation type="unfinished">Karteneinheiten nach Maßstab:</translation>
    </message>
    <message>
        <source>Number of segments:</source>
        <translation type="unfinished">Segmentanzahl</translation>
    </message>
    <message>
        <source>Segments left:</source>
        <translation type="unfinished">Verbleibende Segmente:</translation>
    </message>
    <message>
        <source>Style:</source>
        <translation type="unfinished">Stil:</translation>
    </message>
    <message>
        <source>Map:</source>
        <translation type="unfinished">Karte:</translation>
    </message>
    <message>
        <source>Height (mm):</source>
        <translation type="unfinished">Höhe (mm):</translation>
    </message>
    <message>
        <source>Line width:</source>
        <translation type="unfinished">Linienstärke:</translation>
    </message>
    <message>
        <source>Label space:</source>
        <translation type="unfinished">Bemaßungsraum</translation>
    </message>
    <message>
        <source>Box space:</source>
        <translation type="unfinished">Rahmenraum</translation>
    </message>
    <message>
        <source>Unit label:</source>
        <translation type="unfinished">Beschriftung der Einheit:</translation>
    </message>
    <message>
        <source>Font...</source>
        <translation type="unfinished">Schriftart...</translation>
    </message>
    <message>
        <source>Color...</source>
        <translation type="unfinished">Farbe...</translation>
    </message>
</context>
<context>
    <name>QgsComposerScalebarBase</name>
    <message>
        <source>Barscale Options</source>
        <translation type="obsolete">Optionen für Massstabsbalken</translation>
    </message>
    <message>
        <source>Segment size</source>
        <translation type="obsolete">Segmentgrösse</translation>
    </message>
    <message>
        <source>Number of segments</source>
        <translation type="obsolete">Anzahl Segmente</translation>
    </message>
    <message>
        <source>Map units per scalebar unit</source>
        <translation type="obsolete">Karteneinheiten pro Massstabsbalkeneinheit</translation>
    </message>
    <message>
        <source>Unit label</source>
        <translation type="obsolete">Beschriftung der Einheit</translation>
    </message>
    <message>
        <source>Map</source>
        <translation type="obsolete">Karte</translation>
    </message>
    <message>
        <source>Font</source>
        <translation type="obsolete">Schrift</translation>
    </message>
    <message>
        <source>Line width</source>
        <translation type="obsolete">Linienbreite</translation>
    </message>
</context>
<context>
    <name>QgsComposerVectorLegend</name>
    <message>
        <source>Combine selected layers</source>
        <translation type="obsolete">Kombiniere die ausgewählten Layer.</translation>
    </message>
    <message>
        <source>Cache</source>
        <translation type="obsolete">Cache</translation>
    </message>
    <message>
        <source>Render</source>
        <translation type="obsolete">Zeichnen</translation>
    </message>
    <message>
        <source>Rectangle</source>
        <translation type="obsolete">Rechteck</translation>
    </message>
    <message>
        <source>Legend</source>
        <translation type="obsolete">Legende</translation>
    </message>
</context>
<context>
    <name>QgsComposerVectorLegendBase</name>
    <message>
        <source>Vector Legend Options</source>
        <translation>Optionen für Vektorlegende</translation>
    </message>
    <message>
        <source>Title</source>
        <translation>Titel</translation>
    </message>
    <message>
        <source>Map</source>
        <translation>Karte</translation>
    </message>
    <message>
        <source>Font</source>
        <translation>Schrift</translation>
    </message>
    <message>
        <source>Box</source>
        <translation>Box</translation>
    </message>
    <message>
        <source>Preview</source>
        <translation>Vorschau</translation>
    </message>
    <message>
        <source>Layers</source>
        <translation type="unfinished">Layer</translation>
    </message>
    <message>
        <source>Group</source>
        <translation type="unfinished">Gruppe</translation>
    </message>
    <message>
        <source>ID</source>
        <translation type="unfinished">ID</translation>
    </message>
</context>
<context>
    <name>QgsComposition</name>
    <message>
        <source>Custom</source>
        <translation type="obsolete">Benutzerdefiniert</translation>
    </message>
    <message>
        <source>A5 (148x210 mm)</source>
        <translation type="obsolete">A5 (148x210 mm)</translation>
    </message>
    <message>
        <source>A4 (210x297 mm)</source>
        <translation type="obsolete">A4 (210x297 mm)</translation>
    </message>
    <message>
        <source>A3 (297x420 mm)</source>
        <translation type="obsolete">A3 (297x420 mm)</translation>
    </message>
    <message>
        <source>A2 (420x594 mm)</source>
        <translation type="obsolete">A2 (420x594 mm)</translation>
    </message>
    <message>
        <source>A1 (594x841 mm)</source>
        <translation type="obsolete">A1 (594x841 mm)</translation>
    </message>
    <message>
        <source>A0 (841x1189 mm)</source>
        <translation type="obsolete">A0 (841x1189 mm)</translation>
    </message>
    <message>
        <source>B5 (176 x 250 mm)</source>
        <translation type="obsolete">B5 (176 x 250 mm)</translation>
    </message>
    <message>
        <source>B4 (250 x 353 mm)</source>
        <translation type="obsolete">B4 (250 x 353 mm)</translation>
    </message>
    <message>
        <source>B3 (353 x 500 mm)</source>
        <translation type="obsolete">B3 (353 x 500 mm)</translation>
    </message>
    <message>
        <source>B2 (500 x 707 mm)</source>
        <translation type="obsolete">B2 (500 x 707 mm)</translation>
    </message>
    <message>
        <source>B1 (707 x 1000 mm)</source>
        <translation type="obsolete">B1 (707 x 1000 mm)</translation>
    </message>
    <message>
        <source>B0 (1000 x 1414 mm)</source>
        <translation type="obsolete">B0 (1000 x 1414 mm)</translation>
    </message>
    <message>
        <source>Letter (8.5x11 inches)</source>
        <translation type="obsolete">Letter (8.5x11 inches)</translation>
    </message>
    <message>
        <source>Legal (8.5x14 inches)</source>
        <translation type="obsolete">Legal (8.5x14 inches)</translation>
    </message>
    <message>
        <source>Portrait</source>
        <translation type="obsolete">Hochformat</translation>
    </message>
    <message>
        <source>Landscape</source>
        <translation type="obsolete">Querformat</translation>
    </message>
    <message>
        <source>Out of memory</source>
        <translation type="obsolete">Speicher ist am Ende (Out-of-memory).</translation>
    </message>
    <message>
        <source>Qgis is unable to resize the paper size due to insufficient memory.
 It is best that you avoid using the map composer until you restart qgis.
</source>
        <translation type="obsolete">QGIS kann die Papiergröße wegen zu wenig Speicher nicht anpassen. Am besten Sie verweden den Mapcomposer nicht mehr, so lange sie QGIS nicht neu gestartet haben.</translation>
    </message>
    <message>
        <source>Label</source>
        <translation type="obsolete">Beschriftung</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation type="obsolete">Warnung</translation>
    </message>
    <message>
        <source>Cannot load picture.</source>
        <translation type="obsolete">Kann das Bild nicht laden.</translation>
    </message>
</context>
<context>
    <name>QgsCompositionBase</name>
    <message>
        <source>Composition</source>
        <translation>Zusammenstellung</translation>
    </message>
    <message>
        <source>Paper</source>
        <translation>Papier</translation>
    </message>
    <message>
        <source>Size</source>
        <translation>Grösse</translation>
    </message>
    <message>
        <source>Units</source>
        <translation>Einheiten</translation>
    </message>
    <message>
        <source>Width</source>
        <translation>Breite</translation>
    </message>
    <message>
        <source>Height</source>
        <translation>Höhe</translation>
    </message>
    <message>
        <source>Orientation</source>
        <translation>Orientierung</translation>
    </message>
    <message>
        <source>Resolution (dpi)</source>
        <translation type="obsolete">Auflösung (dpi)</translation>
    </message>
</context>
<context>
    <name>QgsCompositionWidget</name>
    <message>
        <source>Landscape</source>
        <translation type="unfinished">Querformat</translation>
    </message>
    <message>
        <source>Portrait</source>
        <translation type="unfinished">Hochformat</translation>
    </message>
    <message>
        <source>Custom</source>
        <translation type="unfinished">Benutzerdefiniert</translation>
    </message>
    <message>
        <source>A5 (148x210 mm)</source>
        <translation type="unfinished">A5 (148x210 mm)</translation>
    </message>
    <message>
        <source>A4 (210x297 mm)</source>
        <translation type="unfinished">A4 (210x297 mm)</translation>
    </message>
    <message>
        <source>A3 (297x420 mm)</source>
        <translation type="unfinished">A3 (297x420 mm)</translation>
    </message>
    <message>
        <source>A2 (420x594 mm)</source>
        <translation type="unfinished">A2 (420x594 mm)</translation>
    </message>
    <message>
        <source>A1 (594x841 mm)</source>
        <translation type="unfinished">A1 (594x841 mm)</translation>
    </message>
    <message>
        <source>A0 (841x1189 mm)</source>
        <translation type="unfinished">A0 (841x1189 mm)</translation>
    </message>
    <message>
        <source>B5 (176 x 250 mm)</source>
        <translation type="unfinished">B5 (176 x 250 mm)</translation>
    </message>
    <message>
        <source>B4 (250 x 353 mm)</source>
        <translation type="unfinished">B4 (250 x 353 mm)</translation>
    </message>
    <message>
        <source>B3 (353 x 500 mm)</source>
        <translation type="unfinished">B3 (353 x 500 mm)</translation>
    </message>
    <message>
        <source>B2 (500 x 707 mm)</source>
        <translation type="unfinished">B2 (500 x 707 mm)</translation>
    </message>
    <message>
        <source>B1 (707 x 1000 mm)</source>
        <translation type="unfinished">B1 (707 x 1000 mm)</translation>
    </message>
    <message>
        <source>B0 (1000 x 1414 mm)</source>
        <translation type="unfinished">B0 (1000 x 1414 mm)</translation>
    </message>
    <message>
        <source>Letter (8.5x11 inches)</source>
        <translation type="unfinished">Letter (8.5x11 inches)</translation>
    </message>
    <message>
        <source>Legal (8.5x14 inches)</source>
        <translation type="unfinished">Legal (8.5x14 inches)</translation>
    </message>
</context>
<context>
    <name>QgsCompositionWidgetBase</name>
    <message>
        <source>Composition</source>
        <translation type="unfinished">Zusammenstellung</translation>
    </message>
    <message>
        <source>Paper</source>
        <translation type="unfinished">Papier</translation>
    </message>
    <message>
        <source>Orientation</source>
        <translation type="unfinished">Orientierung</translation>
    </message>
    <message>
        <source>Height</source>
        <translation type="unfinished">Höhe</translation>
    </message>
    <message>
        <source>Width</source>
        <translation type="unfinished">Breite</translation>
    </message>
    <message>
        <source>Units</source>
        <translation type="unfinished">Einheiten</translation>
    </message>
    <message>
        <source>Size</source>
        <translation type="unfinished">Grösse</translation>
    </message>
    <message>
        <source>Print quality (dpi)</source>
        <translation type="unfinished">Druckqualität (dpi)</translation>
    </message>
</context>
<context>
    <name>QgsContinuousColorDialogBase</name>
    <message>
        <source>Continuous color</source>
        <translation>Fortlaufende Farbe</translation>
    </message>
    <message>
        <source>Maximum Value:</source>
        <translation>Größter Wert:</translation>
    </message>
    <message>
        <source>Outline Width:</source>
        <translation>Umrandungsbreite:</translation>
    </message>
    <message>
        <source>Minimum Value:</source>
        <translation>Kleinster Wert:</translation>
    </message>
    <message>
        <source>Classification Field:</source>
        <translation>Klassifizierungsfeld:</translation>
    </message>
    <message>
        <source>Draw polygon outline</source>
        <translation>Polygon-Umriss zeichnen</translation>
    </message>
</context>
<context>
    <name>QgsCoordinateTransform</name>
    <message>
        <source>Failed</source>
        <translation>Fehlgeschlagen</translation>
    </message>
    <message>
        <source>transform of</source>
        <translation>Transformation von</translation>
    </message>
    <message>
        <source>with error: </source>
        <translation>mit Fehler:</translation>
    </message>
    <message>
        <source>The source spatial reference system (SRS) is not valid. </source>
        <translation type="obsolete">Das Quell-Referenzsystem (SRS) ist nicht gültig. </translation>
    </message>
    <message>
        <source>The coordinates can not be reprojected. The SRS is: </source>
        <translation type="obsolete">Die Koordinaten können nicht reprojiziert werden. Das SRS ist: </translation>
    </message>
    <message>
        <source>The destination spatial reference system (SRS) is not valid. </source>
        <translation type="obsolete">Das Zielreferenzsystem (SRS) ist nicht gültig. </translation>
    </message>
    <message>
        <source>The source spatial reference system (CRS) is not valid. </source>
        <translation type="unfinished">Das ursprüngliche Koordinatenbezugssystem (KBS) is ungültig. </translation>
    </message>
    <message>
        <source>The coordinates can not be reprojected. The CRS is: </source>
        <translation type="unfinished">Die Koordinaten können nicht projiziert werden. Das KBS ist: </translation>
    </message>
    <message>
        <source>The destination spatial reference system (CRS) is not valid. </source>
        <translation type="unfinished">Das Zielkoordinatenbezugssystem (KBS) is nicht gültig.</translation>
    </message>
</context>
<context>
    <name>QgsCopyrightLabelPlugin</name>
    <message>
        <source>Bottom Left</source>
        <translation>Unten links</translation>
    </message>
    <message>
        <source>Top Left</source>
        <translation>Oben links</translation>
    </message>
    <message>
        <source>Top Right</source>
        <translation>Oben rechts</translation>
    </message>
    <message>
        <source>Bottom Right</source>
        <translation>Unten rechts</translation>
    </message>
    <message>
        <source>&amp;Copyright Label</source>
        <translation>&amp;Urhebersrechtshinweis</translation>
    </message>
    <message>
        <source>Creates a copyright label that is displayed on the map canvas.</source>
        <translation>Erzeugt einen Urheberrechtshinweis auf dem Kartenbild.</translation>
    </message>
    <message>
        <source>&amp;Decorations</source>
        <translation>&amp;Dekorationen</translation>
    </message>
</context>
<context>
    <name>QgsCopyrightLabelPluginGuiBase</name>
    <message>
        <source>Copyright Label Plugin</source>
        <translation>Urhebersrechtsnachweis-Plugin</translation>
    </message>
    <message>
        <source>Placement</source>
        <translation>Platzierung</translation>
    </message>
    <message>
        <source>Bottom Left</source>
        <translation>Unten links</translation>
    </message>
    <message>
        <source>Top Left</source>
        <translation>Oben links</translation>
    </message>
    <message>
        <source>Bottom Right</source>
        <translation>Unten rechts</translation>
    </message>
    <message>
        <source>Top Right</source>
        <translation>Oben rechts</translation>
    </message>
    <message>
        <source>Orientation</source>
        <translation>Orientierung</translation>
    </message>
    <message>
        <source>Horizontal</source>
        <translation>Horizontal</translation>
    </message>
    <message>
        <source>Vertical</source>
        <translation>Vertikal</translation>
    </message>
    <message>
        <source>Enable Copyright Label</source>
        <translation>Urheberrechtshinweis aktivieren</translation>
    </message>
    <message>
        <source>Color</source>
        <translation type="unfinished">Farbe</translation>
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
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;&lt;span style=&quot; font-size:12pt;&quot;&gt;Description&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;Fügen Sie unten Ihr Copyright Label ein. Dieses Plugin unterstützt grundlegende html markup tags um Label zu formatieren. Z.B.:&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;&lt;span style=&quot; font-weight:600;&quot;&gt;&amp;lt;B&amp;gt; Bold text &amp;lt;/B&amp;gt; &lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:600;&quot;&gt;&lt;span style=&quot; font-weight:400; font-style:italic;&quot;&gt;&amp;lt;I&amp;gt; Italics &amp;lt;/I&amp;gt;&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-style:italic;&quot;&gt;&lt;span style=&quot; font-style:normal;&quot;&gt;(note: &amp;amp;copy; zeigt ein Copyright Symbol)&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message encoding="UTF-8">
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;© QGIS 2008&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsCustomProjectionDialog</name>
    <message>
        <source>Delete Projection Definition?</source>
        <translation>Projektdefinition löschen?</translation>
    </message>
    <message>
        <source>Deleting a projection definition is not reversable. Do you want to delete it?</source>
        <translation>Das Löschen einer Projektdefinition ist nicht umkehrbar. Wirklich löschen?</translation>
    </message>
    <message>
        <source>Abort</source>
        <translation>Abbrechen</translation>
    </message>
    <message>
        <source>New</source>
        <translation>Neu</translation>
    </message>
    <message>
        <source>QGIS Custom Projection</source>
        <translation>QGIS benutzerdefinierte Projektion</translation>
    </message>
    <message>
        <source>This proj4 projection definition is not valid. Please correct before pressing save.</source>
        <translation>Diese proj4 Definition ist ungültig. Bitte vor dem speichern korrigieren.</translation>
    </message>
    <message>
        <source>This proj4 projection definition is not valid. Please give the projection a name before pressing save.</source>
        <translation>Diese proj4 Projektionsdefinition ist ungültig. Bitte Sie einen Projektionsnamen an, bevor &apos;speichern&apos; gedrückt wird.</translation>
    </message>
    <message>
        <source>This proj4 projection definition is not valid. Please add the parameters before pressing save.</source>
        <translation>Diese proj4 Projektionsdefinition ist ungültig. Bitte geben sie Parameter an, bevor &apos;speichern&apos; gedrückt wird.</translation>
    </message>
    <message>
        <source>This proj4 projection definition is not valid. Please add a proj= clause before pressing save.</source>
        <translation>Diese proj4 Projektionsdefinition ist ungültig. Bitte fügen sie einen proj= Ausdruck hinzu, bevor &apos;speichern&apos; gedrückt wird. </translation>
    </message>
    <message>
        <source>This proj4 projection definition is not valid.</source>
        <translation>Diese proj4 Projektionsdefinition ist ungültig.</translation>
    </message>
    <message>
        <source>Northing and Easthing must be in decimal form.</source>
        <translation>Northing und Easthing muss in dezimaler Form sein.</translation>
    </message>
    <message>
        <source>Internal Error (source projection invalid?)</source>
        <translation>Interner Fehler (Quellprojektion ungültig?)</translation>
    </message>
</context>
<context>
    <name>QgsCustomProjectionDialogBase</name>
    <message>
        <source>|&lt;</source>
        <translation>|&lt;</translation>
    </message>
    <message>
        <source>&lt;</source>
        <translation>&lt;</translation>
    </message>
    <message>
        <source>1 of 1</source>
        <translation>1 von 1</translation>
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
        <source>Define</source>
        <translation>Definieren</translation>
    </message>
    <message>
        <source>Test</source>
        <translation>Testen</translation>
    </message>
    <message>
        <source>Calculate</source>
        <translation>Berechnen</translation>
    </message>
    <message>
        <source>Geographic / WGS84</source>
        <translation>Geographisch/ WGS84</translation>
    </message>
    <message>
        <source>Name</source>
        <translation type="unfinished">Name</translation>
    </message>
    <message>
        <source>Parameters</source>
        <translation type="unfinished">Parameter</translation>
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
        <translation type="unfinished">Nord</translation>
    </message>
    <message>
        <source>East</source>
        <translation type="unfinished">Ost</translation>
    </message>
    <message>
        <source>Custom Coordinate Reference System Definition</source>
        <translation type="unfinished">Definition eines Benutzerkoordinatensystems</translation>
    </message>
    <message>
        <source>You can define your own custom Coordinate Reference System (CRS) here. The definition must conform to the proj4 format for specifying a CRS.</source>
        <translation type="unfinished">Hier kann ein Benutzerkoordinatenbezugssystem (KBS) definiert werden. Die Definition muß im PROJ.4-Format erfolgen.</translation>
    </message>
    <message>
        <source>Use the text boxes below to test the CRS definition you are creating. Enter a coordinate where both the lat/long and the transformed result are known (for example by reading off a map). Then press the calculate button to see if the CRS definition you are creating is accurate.</source>
        <translation type="unfinished">Benutzen Sie das Eingabefeld unten, um die anzulegende KBS-Definition zu testen. Geben Sie eine Koordinate an zu denen Sie sowohl Breite/Länge und das transformierte Ergebnis kennen (zum Beispiel durch Ablesen von einer Karte). Mit dem &quot;Berechnen&quot;-Knopf können Sie die neue KBS-Definition überprüfen.</translation>
    </message>
    <message>
        <source>Destination CRS        </source>
        <translation type="unfinished">Ziel KBS </translation>
    </message>
</context>
<context>
    <name>QgsDbSourceSelect</name>
    <message>
        <source>Are you sure you want to remove the </source>
        <translation>Sind Sie sicher das Sie die Verbindung und </translation>
    </message>
    <message>
        <source> connection and all associated settings?</source>
        <translation> alle damit verbunden Einstellungen löschen wollen?</translation>
    </message>
    <message>
        <source>Confirm Delete</source>
        <translation>Löschen bestätigen</translation>
    </message>
    <message>
        <source>Select Table</source>
        <translation>Tabelle wählen</translation>
    </message>
    <message>
        <source>You must select a table in order to add a Layer.</source>
        <translation>Es muß eine Tabelle gewählt werden, um einen Layer hinzuzufügen.</translation>
    </message>
    <message>
        <source>Password for </source>
        <translation>Passwort für </translation>
    </message>
    <message>
        <source>Please enter your password:</source>
        <translation>Bitte Passwort eingeben:</translation>
    </message>
    <message>
        <source>Connection failed</source>
        <translation>Verbindungsfehler</translation>
    </message>
    <message>
        <source>Type</source>
        <translation>Typ</translation>
    </message>
    <message>
        <source>Sql</source>
        <translation>Sql</translation>
    </message>
    <message>
        <source>Connection to %1 on %2 failed. Either the database is down or your settings are incorrect.%3Check your username and password and try again.%4The database said:%5%6</source>
        <translation>Verbindung zu %1 auf %2 ist fehlgeschlagen. Entweder ist die Datenbank abgeschaltet oder Ihre Einstellungen sind falsch. %3Bitte überprüfen Sie den Benutzernamen und das Passwort und probieren Sie es noch einmal. %4 Die Datenbank meldete folgendes:%5%6.</translation>
    </message>
    <message>
        <source>Wildcard</source>
        <translation>Platzhalter</translation>
    </message>
    <message>
        <source>RegExp</source>
        <translation>RegAusdr</translation>
    </message>
    <message>
        <source>All</source>
        <translation type="unfinished">Alle</translation>
    </message>
    <message>
        <source>Schema</source>
        <translation type="unfinished">Schema</translation>
    </message>
    <message>
        <source>Table</source>
        <translation type="unfinished">Tabelle</translation>
    </message>
    <message>
        <source>Geometry column</source>
        <translation>Geometriespalte</translation>
    </message>
    <message>
        <source>Accessible tables could not be determined</source>
        <translation type="unfinished">Zugreifbare Tabellen konnten nicht festgestellt werden</translation>
    </message>
    <message>
        <source>Database connection was successful, but the accessible tables could not be determined.

The error message from the database was:
%1
</source>
        <translation type="unfinished">Die Datenbankverbindung war erfolgreich, jedoch konnten die zugänglichen Tabelle nicht bestimmt werden.

Die Fehlermeldung der Datenbank war:
%1
</translation>
    </message>
    <message>
        <source>No accessible tables found</source>
        <translation type="unfinished">Keine zugänglichen Tabellen gefunden</translation>
    </message>
    <message>
        <source>Database connection was successful, but no accessible tables were found.

Please verify that you have SELECT privilege on a table carrying PostGIS
geometry.</source>
        <translation type="unfinished">Die Datenbankverbindung war erfolgreich, jedoch wurden keine zugänglichen Tabellen gefunden.

Bitte stellen Sie sicher, dass Sie das SELECT-Privileg für eine Tabelle
mit PostGIS-Geometrie haben.</translation>
    </message>
</context>
<context>
    <name>QgsDbSourceSelectBase</name>
    <message>
        <source>Add PostGIS Table(s)</source>
        <translation>PostGIS-Tabelle(n) hinzufügen</translation>
    </message>
    <message>
        <source>Add</source>
        <translation>Hinzufügen</translation>
    </message>
    <message>
        <source>Help</source>
        <translation>Hilfe</translation>
    </message>
    <message>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <source>Connect</source>
        <translation>Verbinden</translation>
    </message>
    <message>
        <source>New</source>
        <translation>Neu</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation>Bearbeiten</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>Löschen</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Schließen</translation>
    </message>
    <message>
        <source>PostgreSQL Connections</source>
        <translation>PostgreSQL-Verbindungen</translation>
    </message>
    <message>
        <source>Search:</source>
        <translation>Suchen:</translation>
    </message>
    <message>
        <source>Search mode:</source>
        <translation>Suchmodus:</translation>
    </message>
    <message>
        <source>Search in columns:</source>
        <translation>Suche in Spalten:</translation>
    </message>
    <message>
        <source>Search options...</source>
        <translation>Suchoptionen...</translation>
    </message>
</context>
<context>
    <name>QgsDbTableModel</name>
    <message>
        <source>Schema</source>
        <translation type="unfinished">Schema</translation>
    </message>
    <message>
        <source>Table</source>
        <translation type="unfinished">Tabelle</translation>
    </message>
    <message>
        <source>Type</source>
        <translation type="unfinished">Typ</translation>
    </message>
    <message>
        <source>Geometry column</source>
        <translation>Geometriespalte</translation>
    </message>
    <message>
        <source>Sql</source>
        <translation type="unfinished">Sql</translation>
    </message>
    <message>
        <source>Point</source>
        <translation type="unfinished">Punkt</translation>
    </message>
    <message>
        <source>Multipoint</source>
        <translation>Multipunkt</translation>
    </message>
    <message>
        <source>Line</source>
        <translation type="unfinished">Linie</translation>
    </message>
    <message>
        <source>Multiline</source>
        <translation>Multilinie</translation>
    </message>
    <message>
        <source>Polygon</source>
        <translation type="unfinished">Polygon</translation>
    </message>
    <message>
        <source>Multipolygon</source>
        <translation>Multipolygon</translation>
    </message>
</context>
<context>
    <name>QgsDelAttrDialogBase</name>
    <message>
        <source>Delete Attributes</source>
        <translation>Attribute löschen</translation>
    </message>
</context>
<context>
    <name>QgsDelimitedTextPlugin</name>
    <message>
        <source>&amp;Add Delimited Text Layer</source>
        <translation>Getrennte Textdatei hinzufügen</translation>
    </message>
    <message>
        <source>Add a delimited text file as a map layer. </source>
        <translation>Eine Textdatei dem Kartenfenster als Layer hinzufügen.</translation>
    </message>
    <message>
        <source>The file must have a header row containing the field names. </source>
        <translation>Die Datei muss eine Kopfzeile mit Spaltennamen enthalten.</translation>
    </message>
    <message>
        <source>X and Y fields are required and must contain coordinates in decimal units.</source>
        <translation>X- und Y-Spalten mit dezimalen Koordinaten sind unbedingt erforderlich</translation>
    </message>
    <message>
        <source>&amp;Delimited text</source>
        <translation>&amp;Getrennter Text</translation>
    </message>
    <message>
        <source>DelimitedTextLayer</source>
        <translation>Layer aus Textdatei</translation>
    </message>
</context>
<context>
    <name>QgsDelimitedTextPluginGui</name>
    <message>
        <source>No layer name</source>
        <translation>Kein Layername</translation>
    </message>
    <message>
        <source>Please enter a layer name before adding the layer to the map</source>
        <translation>Bitte geben Sie einen Layernamen ein, bevor Sie den Layer zum Kartenfenster hinzufügen</translation>
    </message>
    <message>
        <source>No delimiter</source>
        <translation>Kein Trennzeichen</translation>
    </message>
    <message>
        <source>Please specify a delimiter prior to parsing the file</source>
        <translation>Es muss ein Trennzeichen eingegeben werden, damit die Datei abgearbeitet werden kann</translation>
    </message>
    <message>
        <source>Choose a delimited text file to open</source>
        <translation>Textdatei zum Öffnen wählen</translation>
    </message>
    <message>
        <source>Parse</source>
        <translation>Analysieren</translation>
    </message>
    <message>
        <source>Description</source>
        <translation>Beschreibung</translation>
    </message>
    <message>
        <source>Select a delimited text file containing a header row and one or more rows of x and y coordinates that you would like to use as a point layer and this plugin will do the job for you!</source>
        <translation>Wähle eine Textdatei mit Trennzeichen, das eine Kopfzeile, und Spalten mit X- und Y-Koordinaten enthält, die Sie gerne als Punktlayer darstellen möchten und QGIS erledigt das für Sie!</translation>
    </message>
    <message>
        <source>Use the layer name box to specify the legend name for the new layer. Use the delimiter box to specify what delimeter is used in your file (e.g. space, comma, tab or a regular expression in Perl style). After choosing a delimiter, press the parse button and select the columns containing the x and y values for the layer.</source>
        <translation>Benutzen Sie die Layername Box, um den Legendennamen des zu erstellenden Layers anzugeben. Benutzen Sie die Trennzeichen Box, um das in der Textdatei verwendete Trennzeichen anzugeben (z.B.: Leerzeichen, Kommar, Tabulator oder ein anderer regulärer Ausdruck im Perl-Stil), Nun drücken Sie den Knopf Analysieren und wählen die Spalten mit den X- und Y-Koordinaten aus.</translation>
    </message>
</context>
<context>
    <name>QgsDelimitedTextPluginGuiBase</name>
    <message>
        <source>Create a Layer from a Delimited Text File</source>
        <translation>Textdatei aus Layer erzeugen</translation>
    </message>
    <message>
        <source>&lt;p align=&quot;right&quot;&gt;X field&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;right&quot;&gt;X-Feld&lt;/p&gt;</translation>
    </message>
    <message>
        <source>Name of the field containing x values</source>
        <translation>Nennen Sie das Feld, das die X-Werte enthält</translation>
    </message>
    <message>
        <source>Name of the field containing x values. Choose a field from the list. The list is generated by parsing the header row of the delimited text file.</source>
        <translation>Nennen Sie das Feld, das die X-Werte enthält. Wähle ein Feld aus der Liste, die aus der Kopfzeile der Textdatei erzeugt wurde.</translation>
    </message>
    <message>
        <source>&lt;p align=&quot;right&quot;&gt;Y field&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;right&quot;&gt;Y Feld&lt;/p&gt;</translation>
    </message>
    <message>
        <source>Name of the field containing y values</source>
        <translation>Nennen Sie das Feld, das die y-Werte enthält</translation>
    </message>
    <message>
        <source>Name of the field containing y values. Choose a field from the list. The list is generated by parsing the header row of the delimited text file.</source>
        <translation>Nennen Sie das Feld, das die y-Werte enthält. Wähle ein Feld aus der Liste, die aus der Kopfzeile der Textdatei erzeugt wurde.</translation>
    </message>
    <message>
        <source>Layer name</source>
        <translation>Layername</translation>
    </message>
    <message>
        <source>Name to display in the map legend</source>
        <translation>Name, der in der Kartenlegende angezeigt wird</translation>
    </message>
    <message>
        <source>Name displayed in the map legend</source>
        <translation>Name, der in der Kartenlegende angezeigt wird</translation>
    </message>
    <message>
        <source>Delimiter</source>
        <translation>Trennzeichen</translation>
    </message>
    <message>
        <source>Delimiter to use when splitting fields in the text file. The delimiter can be more than one character.</source>
        <translation>Trennzeichen, das zum Aufspalten der Felder in der Textdatei verwendet wird. Das Trennzeichen kann mehr als ein Zeichen sein.</translation>
    </message>
    <message>
        <source>Delimiter to use when splitting fields in the delimited text file. The delimiter can be 1 or more characters in length.</source>
        <translation>Trennzeichen, das zum Aufspalten der Felder in der Textdatei verwendet wird. Das Trennzeichen kann ein oder mehrere Zeichen lang sein.</translation>
    </message>
    <message>
        <source>Delimited Text Layer</source>
        <translation>Layer aus Textdatei</translation>
    </message>
    <message>
        <source>Delimited text file</source>
        <translation>Textdatei</translation>
    </message>
    <message>
        <source>Full path to the delimited text file</source>
        <translation>Vollständiger Pfad zur Textdatei</translation>
    </message>
    <message>
        <source>Full path to the delimited text file. In order to properly parse the fields in the file, the delimiter must be defined prior to entering the file name. Use the Browse button to the right of this field to choose the input file.</source>
        <translation>Vollständiger Pfad zur Textdatei. Um die Felder in der Datei ordentlich verarbeiten zu können, muß das Trennzeichen vor der Eingabe des Dateinamens definiert sein. Zum Wählen der Eingabedatei den Durchsuchen- Knopf rechts des Feldes verwenden.</translation>
    </message>
    <message>
        <source>Browse to find the delimited text file to be processed</source>
        <translation>Durchsuchen zum Finden der zuverarbeitenden Textdatei</translation>
    </message>
    <message>
        <source>Use this button to browse to the location of the delimited text file. This button will not be enabled until a delimiter has been entered in the &lt;i&gt;Delimiter&lt;/i&gt; box. Once a file is chosen, the X and Y field drop-down boxes will be populated with the fields from the delimited text file.</source>
        <translation>Verwende diesen Knopf zum Durchsuchen nach dem Ort der Textdatei. Dieser Knopf ist nicht aktiv bis ein Trennzeichen in der &lt;i&gt;Trennzeichen&lt;/i&gt; Box eingegeben wurde. Wenn die Datei erst einmal gewählt wurde, werden die X- und Y-Feld Dropdownboxen mit den Feldern aus der Textdatei gefüllt.</translation>
    </message>
    <message>
        <source>Sample text</source>
        <translation>Beispieltext</translation>
    </message>
    <message>
        <source>Browse...</source>
        <translation>Suchen...</translation>
    </message>
    <message>
        <source>The delimiter is taken as is</source>
        <translation>Das Trennzeichen wurde wie vorhanden verwendet</translation>
    </message>
    <message>
        <source>Plain characters</source>
        <translation>Klartext</translation>
    </message>
    <message>
        <source>The delimiter is a regular expression</source>
        <translation>Dast Trennzeichen ist ein regulärer Ausdruck</translation>
    </message>
    <message>
        <source>Regular expression</source>
        <translation>Regulärer Ausdruck</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsDelimitedTextProvider</name>
    <message>
        <source>Note: the following lines were not loaded because Qgis was unable to determine values for the x and y coordinates:
</source>
        <translation>Beachte: Die folgenden Linien wurden nicht geladen, da QGIS die Werte für die X und Y Koordinaten nicht herausfinden konnte.</translation>
    </message>
    <message>
        <source>Error</source>
        <translation>Fehler</translation>
    </message>
</context>
<context>
    <name>QgsDetailedItemWidgetBase</name>
    <message>
        <source>Form</source>
        <translation type="unfinished">Formular</translation>
    </message>
    <message>
        <source>Heading Label</source>
        <translation type="unfinished">Kopftitel</translation>
    </message>
    <message>
        <source>Detail label</source>
        <translation type="unfinished">Detailtitel</translation>
    </message>
</context>
<context>
    <name>QgsDlgPgBufferBase</name>
    <message>
        <source>Buffer features</source>
        <translation>Puffereigenschaften</translation>
    </message>
    <message>
        <source>Buffer distance in map units:</source>
        <translation>Gepufferte Entfernung in Karteneinheiten:</translation>
    </message>
    <message>
        <source>Table name for the buffered layer:</source>
        <translation>Tabellenname für den gepufferten Layer:</translation>
    </message>
    <message>
        <source>Create unique object id</source>
        <translation>Eindeutige Objekt ID erzeugen</translation>
    </message>
    <message>
        <source>public</source>
        <translation>öffentlich</translation>
    </message>
    <message>
        <source>Geometry column:</source>
        <translation>Geometriespalte:</translation>
    </message>
    <message>
        <source>Spatial reference ID:</source>
        <translation>Räumliche Referenz ID:</translation>
    </message>
    <message>
        <source>Unique field to use as feature id:</source>
        <translation>Eindeutiges Feld zur Verwendung als Objekt-ID:</translation>
    </message>
    <message>
        <source>Schema:</source>
        <translation>Schema:</translation>
    </message>
    <message>
        <source>Add the buffered layer to the map?</source>
        <translation>Den gepufferten Layer zur Karte hinzufügen?</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Buffer the features in layer: &lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Objekte im Layer puffern: &lt;/h2&gt;</translation>
    </message>
    <message>
        <source>Parameters</source>
        <translation>Parameter</translation>
    </message>
</context>
<context>
    <name>QgsEncodingFileDialog</name>
    <message>
        <source>Encoding:</source>
        <translation>Kodierung:</translation>
    </message>
</context>
<context>
    <name>QgsGPSDeviceDialog</name>
    <message>
        <source>New device %1</source>
        <translation>Neues Gerät %1</translation>
    </message>
    <message>
        <source>Are you sure?</source>
        <translation>Sind Sie sicher?</translation>
    </message>
    <message>
        <source>Are you sure that you want to delete this device?</source>
        <translation>Sind Sie sicher, dass sie dieses Gerät löschen wollen?</translation>
    </message>
</context>
<context>
    <name>QgsGPSDeviceDialogBase</name>
    <message>
        <source>GPS Device Editor</source>
        <translation>GPS-Geräteeditor</translation>
    </message>
    <message>
        <source>This is the name of the device as it will appear in the lists</source>
        <translation>Dies ist der Name des Gerätes, so wie er in der Liste erscheinen wird</translation>
    </message>
    <message>
        <source>Update device</source>
        <translation>Gerät zum Hinaufladen</translation>
    </message>
    <message>
        <source>Delete device</source>
        <translation>Lösche Gerät</translation>
    </message>
    <message>
        <source>New device</source>
        <translation>Neues Gerät</translation>
    </message>
    <message>
        <source>Commands</source>
        <translation>Kommandos</translation>
    </message>
    <message>
        <source>Waypoint download:</source>
        <translation>Wegpunkt herunterladen:</translation>
    </message>
    <message>
        <source>Waypoint upload:</source>
        <translation>Wegpunkt hinaufladen:</translation>
    </message>
    <message>
        <source>Route download:</source>
        <translation>Route herunterladen:</translation>
    </message>
    <message>
        <source>Route upload:</source>
        <translation>Route hinaufladen:</translation>
    </message>
    <message>
        <source>Track download:</source>
        <translation>Spur herunterladen:</translation>
    </message>
    <message>
        <source>The command that is used to upload tracks to the device</source>
        <translation>Das Kommando, welches gebraucht wird, um eine Spur auf das Gerät hochzuladen</translation>
    </message>
    <message>
        <source>Track upload:</source>
        <translation>Spur hinaufladen:</translation>
    </message>
    <message>
        <source>The command that is used to download tracks from the device</source>
        <translation>Das Kommando, um Spuren vom Gerät herunterzuladen</translation>
    </message>
    <message>
        <source>The command that is used to upload routes to the device</source>
        <translation>Das Kommando zum Hinaufladen von Routen zum Gerät</translation>
    </message>
    <message>
        <source>The command that is used to download routes from the device</source>
        <translation>Das Kommando, um eine Route vom Gerät herunterzuladen</translation>
    </message>
    <message>
        <source>The command that is used to upload waypoints to the device</source>
        <translation>Das Kommando, zum Heraufladen von Wegpunkten zum Gerät</translation>
    </message>
    <message>
        <source>The command that is used to download waypoints from the device</source>
        <translation>Das Kommando, um Wegpunkte vom Gerät herunterzuladen</translation>
    </message>
    <message>
        <source>Device name</source>
        <translation type="unfinished">Gerätename</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;In the download and upload commands there can be special words that will be replaced by QGIS when the commands are used. These words are:&lt;span style=&quot; font-style:italic;&quot;&gt;%babel&lt;/span&gt; - the path to GPSBabel&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%in&lt;/span&gt; - the GPX filename when uploading or the port when downloading&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%out&lt;/span&gt; - the port when uploading or the GPX filename when downloading&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGPSPlugin</name>
    <message>
        <source>&amp;Gps Tools</source>
        <translation>&amp;GPS Werkzeuge</translation>
    </message>
    <message>
        <source>&amp;Create new GPX layer</source>
        <translation>Erstelle neuen GPX-Layer</translation>
    </message>
    <message>
        <source>Creates a new GPX layer and displays it on the map canvas</source>
        <translation>Erzeugt einen neuen GPX-Layer und zeichnet ihn in das Kartenfenster.</translation>
    </message>
    <message>
        <source>&amp;Gps</source>
        <translation>&amp;Gps</translation>
    </message>
    <message>
        <source>Save new GPX file as...</source>
        <translation>Neue GPX-Datei speichern als...</translation>
    </message>
    <message>
        <source>GPS eXchange file (*.gpx)</source>
        <translation>GPS eXchange Datei (*.gpx)</translation>
    </message>
    <message>
        <source>Could not create file</source>
        <translation>Kann die Datei nicht erzeugen.</translation>
    </message>
    <message>
        <source>Unable to create a GPX file with the given name. </source>
        <translation>Kann die GPX-Datei mit dem angegebenen Namen nicht erzeugen.</translation>
    </message>
    <message>
        <source>Try again with another name or in another </source>
        <translation>Bitte probieren Sie es mit einem anderen Namen oder in einem anderen </translation>
    </message>
    <message>
        <source>directory.</source>
        <translation>Ordner.</translation>
    </message>
    <message>
        <source>GPX Loader</source>
        <translation>GPX Lader</translation>
    </message>
    <message>
        <source>Unable to read the selected file.
</source>
        <translation>Kann die ausgewählte Datei nicht lesen.</translation>
    </message>
    <message>
        <source>Please reselect a valid file.</source>
        <translation>Bitte wählen Sie eine gültige Datei aus.</translation>
    </message>
    <message>
        <source>Could not start process</source>
        <translation>Kann den Prozess nicht starten.</translation>
    </message>
    <message>
        <source>Could not start GPSBabel!</source>
        <translation>Kann GPSBabel nicht starten!</translation>
    </message>
    <message>
        <source>Importing data...</source>
        <translation>Importiere Daten...</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Abbrechen</translation>
    </message>
    <message>
        <source>Could not import data from %1!

</source>
        <translation>Konnte die daten von %1 nicht importieren!</translation>
    </message>
    <message>
        <source>Error importing data</source>
        <translation>Fehler beim Importieren der Daten</translation>
    </message>
    <message>
        <source>Not supported</source>
        <translation>Nicht unterstützt</translation>
    </message>
    <message>
        <source>This device does not support downloading </source>
        <translation>Dieses Gerät unterstützt den Download </translation>
    </message>
    <message>
        <source>of </source>
        <translation>von nicht.</translation>
    </message>
    <message>
        <source>Downloading data...</source>
        <translation>Daten werden heruntergeladen...</translation>
    </message>
    <message>
        <source>Could not download data from GPS!

</source>
        <translation>Konnte die Daten nicht vom GPS-Gerät herunterladen.</translation>
    </message>
    <message>
        <source>Error downloading data</source>
        <translation>Fehler beim Herunterladen der Daten</translation>
    </message>
    <message>
        <source>This device does not support uploading of </source>
        <translation>Dieses Gerät unterstützt das Hochladen von </translation>
    </message>
    <message>
        <source>Uploading data...</source>
        <translation>Daten werden hochgeladen...</translation>
    </message>
    <message>
        <source>Error while uploading data to GPS!

</source>
        <translation>Fehler beim Hochladen der Daten ins GPS!</translation>
    </message>
    <message>
        <source>Error uploading data</source>
        <translation>Fehler beim Hochladen der Daten</translation>
    </message>
    <message>
        <source>Could not convert data from %1!

</source>
        <translation>Konnte Daten von %1 nicht konvertieren!</translation>
    </message>
    <message>
        <source>Error converting data</source>
        <translation>Fehler beim Datenkonvertieren</translation>
    </message>
</context>
<context>
    <name>QgsGPSPluginGui</name>
    <message>
        <source>Choose a filename to save under</source>
        <translation type="obsolete">Bitte wählen Sie einen Dateinamen</translation>
    </message>
    <message>
        <source>GPS eXchange format (*.gpx)</source>
        <translation>GPS eXchange Format (*.gpx)</translation>
    </message>
    <message>
        <source>Select GPX file</source>
        <translation>Wählen Sie GPX-Datei aus</translation>
    </message>
    <message>
        <source>Select file and format to import</source>
        <translation>Datei und Format zum Importieren wählen.</translation>
    </message>
    <message>
        <source>Waypoints</source>
        <translation type="unfinished">Wegpunkte</translation>
    </message>
    <message>
        <source>Routes</source>
        <translation type="unfinished">Routen</translation>
    </message>
    <message>
        <source>Tracks</source>
        <translation type="unfinished">Spuren</translation>
    </message>
    <message>
        <source>QGIS can perform conversions of GPX files, by using GPSBabel (%1) to perform the conversions.</source>
        <translation>QGIS kann GPX-Dateien mit Hilfe von GPSBabel (%1) konvertieren.</translation>
    </message>
    <message>
        <source>This requires that you have GPSBabel installed where QGIS can find it.</source>
        <translation>Dazu muss GPSBabel an einem Ort installiert sein, den QGIS finden kann.</translation>
    </message>
    <message>
        <source>Select a GPX input file name, the type of conversion you want to perform, a GPX filename that you want to save the converted file as, and a name for the new layer created from the result.</source>
        <translation type="obsolete">Wählen Sie eine GPX-Eingabedatei, die Konvertierung, die Sie durchführen möchten, einen Namen für die Ausgabedatei und einen Namen für den Layer, der aus den Ergebnissen erstellt werden soll.</translation>
    </message>
    <message>
        <source>GPX is the %1, which is used to store information about waypoints, routes, and tracks.</source>
        <translation>GPX ist das %1, das benutzt wird, um Informationen zu Wegpunkten, Routen und Spuren zu speichern.</translation>
    </message>
    <message>
        <source>GPS eXchange file format</source>
        <translation>GPS eXchange Dateiformat</translation>
    </message>
    <message>
        <source>Select a GPX file and then select the feature types that you want to load.</source>
        <translation>Wählen Sie eine GPX Datei und dann die Objekttypen, die Sie laden möchten.</translation>
    </message>
    <message>
        <source>This tool will help you download data from a GPS device.</source>
        <translation>Dieses Werkzeug hilft Ihnen dabei, Daten von Ihrem GPS-Gerät herunterzuladen.</translation>
    </message>
    <message>
        <source>Choose your GPS device, the port it is connected to, the feature type you want to download, a name for your new layer, and the GPX file where you want to store the data.</source>
        <translation>Wählen Sie Ihr GPS-Gerät, den Port, an den es angeschlossen ist, den Objekttyp, den Sie herunterladen möchten, einen Namen für den neuen Layer und die GPX-Datei, als die Sie die Daten speichern möchten.</translation>
    </message>
    <message>
        <source>If your device isn&apos;t listed, or if you want to change some settings, you can also edit the devices.</source>
        <translation>Wenn Ihr Gerät nicht aufgelistet ist, oder Sie Einstellungen ändern möchten, können Sie die Geräteeinstellungen editieren.</translation>
    </message>
    <message>
        <source>This tool uses the program GPSBabel (%1) to transfer the data.</source>
        <translation>Dieses Werkzeug benutzt GPSBabel (%1), um die Daten zu transferieren.</translation>
    </message>
    <message>
        <source>This tool will help you upload data from a GPX layer to a GPS device.</source>
        <translation>Dieses Werkzeug hilft Ihnen, Daten aus einem GPX-Layer auf ein GPS-Gerät zu spielen.</translation>
    </message>
    <message>
        <source>Choose the layer you want to upload, the device you want to upload it to, and the port your device is connected to.</source>
        <translation>Wählen Sie einen Layer, den sie hochladen möchten, das GPS-Gerät und den Port, über den das Gerät verbunden ist.</translation>
    </message>
    <message>
        <source>QGIS can only load GPX files by itself, but many other formats can be converted to GPX using GPSBabel (%1).</source>
        <translation>QGIS selbst kann nur GPX-Dateien laden, aber viele andere Formate können nach GPX konvertiert werden mit  (%1).</translation>
    </message>
    <message>
        <source>Select a GPS file format and the file that you want to import, the feature type that you want to use, a GPX filename that you want to save the converted file as, and a name for the new layer.</source>
        <translation type="obsolete">Wählen SIe ein GPS-Dateiformat, die Datei, die Sie importieren möchten, den Objekttyp, den Sie benutzen möchten, einen GPX-Dateinamen, unter dem Sie die konvertierten Daten speichern möchten und einen Namen für den neuen Layer.</translation>
    </message>
    <message>
        <source>All file formats can not store waypoints, routes, and tracks, so some feature types may be disabled for some file formats.</source>
        <translation>Nicht alle Dateiformate können Wegpunkte, Routen und Spuren speichern, daher können einige Objekttypen für verschiedene Formate deaktiviert sein.</translation>
    </message>
    <message>
        <source>Choose a file name to save under</source>
        <translation type="unfinished">Dateiname zum Speichern wählen</translation>
    </message>
    <message>
        <source>Select a GPS file format and the file that you want to import, the feature type that you want to use, a GPX file name that you want to save the converted file as, and a name for the new layer.</source>
        <translation type="unfinished">Das GPS-Dateiformat, die zu importierende Datei, den zu benutzenden Objekttyp, den Namen der GPX-Datei in der die konvertierten Daten gespeichert werden sollen und den Namen des neuen Layers wählen.</translation>
    </message>
    <message>
        <source>Select a GPX input file name, the type of conversion you want to perform, a GPX file name that you want to save the converted file as, and a name for the new layer created from the result.</source>
        <translation type="unfinished">GPX-Eingabedateinamen, Typ der vorzunehmenden Konvertierung, Name der zu konvertierenden GPX-Datei und Name des neuen Layers für das Ergebnis wählen.</translation>
    </message>
</context>
<context>
    <name>QgsGPSPluginGuiBase</name>
    <message>
        <source>GPS Tools</source>
        <translation>GPS Werkzeuge</translation>
    </message>
    <message>
        <source>Load GPX file</source>
        <translation>GPX-Datei laden</translation>
    </message>
    <message>
        <source>File:</source>
        <translation>Datei:</translation>
    </message>
    <message>
        <source>Feature types:</source>
        <translation>Objekttypen:</translation>
    </message>
    <message>
        <source>Waypoints</source>
        <translation>Wegpunkte</translation>
    </message>
    <message>
        <source>Routes</source>
        <translation>Routen</translation>
    </message>
    <message>
        <source>Tracks</source>
        <translation>Spuren</translation>
    </message>
    <message>
        <source>Import other file</source>
        <translation>Aus anderer Datei importieren</translation>
    </message>
    <message>
        <source>File to import:</source>
        <translation>Zu importierende Datei:</translation>
    </message>
    <message>
        <source>Feature type:</source>
        <translation>Objekttyp:</translation>
    </message>
    <message>
        <source>GPX output file:</source>
        <translation>GPX Ausgabedatei:</translation>
    </message>
    <message>
        <source>Layer name:</source>
        <translation>Layername:</translation>
    </message>
    <message>
        <source>Download from GPS</source>
        <translation>Von GPS herunterladen</translation>
    </message>
    <message>
        <source>Edit devices</source>
        <translation>Editiere Geräte</translation>
    </message>
    <message>
        <source>GPS device:</source>
        <translation>GPS Gerät:</translation>
    </message>
    <message>
        <source>Output file:</source>
        <translation>Ausgabedatei:</translation>
    </message>
    <message>
        <source>Port:</source>
        <translation>Port:</translation>
    </message>
    <message>
        <source>Upload to GPS</source>
        <translation>nach GPS hochladen</translation>
    </message>
    <message>
        <source>Data layer:</source>
        <translation>Datenlayer:</translation>
    </message>
    <message>
        <source>Browse...</source>
        <translation>Suchen...</translation>
    </message>
    <message>
        <source>Save As...</source>
        <translation type="unfinished">Speichern unter...</translation>
    </message>
    <message>
        <source>(Note: Selecting correct file type in browser dialog important!)</source>
        <translation>(Bemerkung: Die Auswahl des richtigen Datentyps ist wichtig!)</translation>
    </message>
    <message>
        <source>GPX Conversions</source>
        <translation>GPX-Konvertierung</translation>
    </message>
    <message>
        <source>Conversion:</source>
        <translation>Konvertierung:</translation>
    </message>
    <message>
        <source>GPX input file:</source>
        <translation>GPX-Eingabedatei</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Edit devices...</source>
        <translation type="unfinished">Gerät bearbeiten...</translation>
    </message>
    <message>
        <source>Refresh</source>
        <translation type="unfinished">Aktualisieren</translation>
    </message>
</context>
<context>
    <name>QgsGPXProvider</name>
    <message>
        <source>Bad URI - you need to specify the feature type.</source>
        <translation>Falsche URI - Bitte geben Sie den Featuretype an.</translation>
    </message>
    <message>
        <source>GPS eXchange file</source>
        <translation>GPS eXchange Datei</translation>
    </message>
    <message>
        <source>Digitized in QGIS</source>
        <translation>Digitalisiert mit QGIS</translation>
    </message>
</context>
<context>
    <name>QgsGenericProjectionSelector</name>
    <message>
        <source>Define this layer&apos;s projection:</source>
        <translation type="unfinished">Definiere die Projektion des Layers:</translation>
    </message>
    <message>
        <source>This layer appears to have no projection specification.</source>
        <translation type="unfinished">Dieser Layer scheint keine Projektionsangaben zu besitzen.</translation>
    </message>
    <message>
        <source>By default, this layer will now have its projection set to that of the project, but you may override this by selecting a different projection below.</source>
        <translation type="unfinished">Als standard wird die Projektion dieses Layers auf die des Projektes gesetzt, aber Sie können es durch auswählen einer anderen Projektion unten überschreiben.</translation>
    </message>
</context>
<context>
    <name>QgsGenericProjectionSelectorBase</name>
    <message>
        <source>Projection Selector</source>
        <translation type="unfinished">Projektionsauswahl</translation>
    </message>
</context>
<context>
    <name>QgsGeomTypeDialog</name>
    <message>
        <source>Real</source>
        <translation type="unfinished">Fließkommazahl</translation>
    </message>
    <message>
        <source>Integer</source>
        <translation type="unfinished">Ganzzahl</translation>
    </message>
    <message>
        <source>String</source>
        <translation type="unfinished">Zeichenkette</translation>
    </message>
</context>
<context>
    <name>QgsGeomTypeDialogBase</name>
    <message>
        <source>Type</source>
        <translation>Typ</translation>
    </message>
    <message>
        <source>Point</source>
        <translation>Punkt</translation>
    </message>
    <message>
        <source>Line</source>
        <translation>Linie</translation>
    </message>
    <message>
        <source>Polygon</source>
        <translation>Polygon</translation>
    </message>
    <message>
        <source>New Vector Layer</source>
        <translation>Neuer Vektorlayer</translation>
    </message>
    <message>
        <source>File format</source>
        <translation type="unfinished">Dateiformat</translation>
    </message>
    <message>
        <source>Attributes</source>
        <translation type="unfinished">Attribute</translation>
    </message>
    <message>
        <source>Name</source>
        <translation type="unfinished">Name</translation>
    </message>
    <message>
        <source>Remove selected row</source>
        <translation type="obsolete">Gewählte Zeilen löschen</translation>
    </message>
    <message>
        <source>...</source>
        <translation type="unfinished">...</translation>
    </message>
    <message>
        <source>Add values manually</source>
        <translation type="obsolete">Werte hinzufügen</translation>
    </message>
    <message>
        <source>Delete selected attribute</source>
        <translation type="unfinished">Gewähltes Attribut löschen</translation>
    </message>
    <message>
        <source>Add attribute</source>
        <translation type="unfinished">Attribute hinzufügen</translation>
    </message>
</context>
<context>
    <name>QgsGeorefDescriptionDialogBase</name>
    <message>
        <source>Description georeferencer</source>
        <translation>Beschreibung Georeferenzierer</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:12pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:11pt; font-weight:600;&quot;&gt;Description&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:9pt;&quot;&gt;This plugin can generate world files for rasters. You select points on the raster and give their world coordinates, and the plugin will compute the world file parameters. The more coordinates you can provide the better the result will be.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;(new line)p, li { white-space: pre-wrap; }(new line)&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:12pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:11pt; font-weight:600;&quot;&gt;Beschreibung&lt;/span&gt;&lt;/p&gt;&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:9pt;&quot;&gt;Dieses Plugin generiert World-Dateien für Raster. Es können Punkte auf dem Raster gewählt und deren Kartenkoordinaten angegeben werden. Je mehr Punkt angegeben werden desto besser wird das Resultat sein.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
</context>
<context>
    <name>QgsGeorefPlugin</name>
    <message>
        <source>&amp;Georeferencer</source>
        <translation>&amp;Georeferenzierer</translation>
    </message>
</context>
<context>
    <name>QgsGeorefPluginGui</name>
    <message>
        <source>Choose a raster file</source>
        <translation>Wähle eine Rasterkarte</translation>
    </message>
    <message>
        <source>Raster files (*.*)</source>
        <translation>Rasterkarten (*.*)</translation>
    </message>
    <message>
        <source>Error</source>
        <translation type="unfinished">Fehler</translation>
    </message>
    <message>
        <source>The selected file is not a valid raster file.</source>
        <translation>Die ausgewählte Karte ist keine gültige Rasterdatei.</translation>
    </message>
    <message>
        <source>World file exists</source>
        <translation>World file existiert</translation>
    </message>
    <message>
        <source>&lt;p&gt;The selected file already seems to have a </source>
        <translation>&lt;p&gt;Die ausgewählte Datei hat scheinbar bereits einen </translation>
    </message>
    <message>
        <source>world file! Do you want to replace it with the </source>
        <translation>World file! Möchten Sie ihn ersetzen mit dem </translation>
    </message>
    <message>
        <source>new world file?&lt;/p&gt;</source>
        <translation>neuen World file?&lt;/p&gt;</translation>
    </message>
</context>
<context>
    <name>QgsGeorefPluginGuiBase</name>
    <message>
        <source>Georeferencer</source>
        <translation>Georeferenzierer</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Schließen</translation>
    </message>
    <message>
        <source>...</source>
        <translation type="unfinished">...</translation>
    </message>
    <message>
        <source>Raster file:</source>
        <translation>Rasterdatei:</translation>
    </message>
    <message>
        <source>Arrange plugin windows</source>
        <translation>Fenster anordnen</translation>
    </message>
    <message>
        <source>Description...</source>
        <translation>Beschreibung...</translation>
    </message>
</context>
<context>
    <name>QgsGeorefWarpOptionsDialog</name>
    <message>
        <source>unstable</source>
        <translation type="obsolete">instabil</translation>
    </message>
</context>
<context>
    <name>QgsGeorefWarpOptionsDialogBase</name>
    <message>
        <source>Warp options</source>
        <translation>Verzerrungsoptionen</translation>
    </message>
    <message>
        <source>Resampling method:</source>
        <translation>Stichprobenmethode:</translation>
    </message>
    <message>
        <source>Nearest neighbour</source>
        <translation>Nächster Nachbar</translation>
    </message>
    <message>
        <source>Linear</source>
        <translation>Linear</translation>
    </message>
    <message>
        <source>Cubic</source>
        <translation>Kubisch</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>Use 0 for transparency when needed</source>
        <translation>0 für Transparanz verwenden falls benötigt</translation>
    </message>
    <message>
        <source>Compression:</source>
        <translation>Komprimierung:</translation>
    </message>
</context>
<context>
    <name>QgsGraduatedSymbolDialog</name>
    <message>
        <source>Equal Interval</source>
        <translation>Gleiches Intervall</translation>
    </message>
    <message>
        <source>Quantiles</source>
        <translation>Quantile</translation>
    </message>
    <message>
        <source>Empty</source>
        <translation>Leer</translation>
    </message>
</context>
<context>
    <name>QgsGraduatedSymbolDialogBase</name>
    <message>
        <source>graduated Symbol</source>
        <translation>abgestuftes Symbol</translation>
    </message>
    <message>
        <source>Delete class</source>
        <translation type="unfinished">Klasse löschen</translation>
    </message>
    <message>
        <source>Classify</source>
        <translation type="unfinished">Klassifizieren</translation>
    </message>
    <message>
        <source>Classification field</source>
        <translation type="unfinished">Klassifikationsfeld</translation>
    </message>
    <message>
        <source>Mode</source>
        <translation type="unfinished">Modus</translation>
    </message>
    <message>
        <source>Number of classes</source>
        <translation type="unfinished">Klassenanzahl</translation>
    </message>
</context>
<context>
    <name>QgsGrassAttributes</name>
    <message>
        <source>Warning</source>
        <translation>Warnung</translation>
    </message>
    <message>
        <source>Column</source>
        <translation type="unfinished">Spalte</translation>
    </message>
    <message>
        <source>Value</source>
        <translation type="unfinished">Wert</translation>
    </message>
    <message>
        <source>Type</source>
        <translation type="unfinished">Typ</translation>
    </message>
    <message>
        <source>ERROR</source>
        <translation type="unfinished">FEHLER</translation>
    </message>
    <message>
        <source>OK</source>
        <translation type="unfinished">OK</translation>
    </message>
    <message>
        <source>Layer</source>
        <translation type="unfinished">Layer</translation>
    </message>
</context>
<context>
    <name>QgsGrassAttributesBase</name>
    <message>
        <source>GRASS Attributes</source>
        <translation>GRASS Attribute</translation>
    </message>
    <message>
        <source>Tab 1</source>
        <translation>Tab 1</translation>
    </message>
    <message>
        <source>result</source>
        <translation>Resultat</translation>
    </message>
    <message>
        <source>Update</source>
        <translation>Aktualisieren</translation>
    </message>
    <message>
        <source>Update database record</source>
        <translation>Aktualisiere Datenbankeintrag</translation>
    </message>
    <message>
        <source>New</source>
        <translation>Neu</translation>
    </message>
    <message>
        <source>Add new category using settings in GRASS Edit toolbox</source>
        <translation>Eine neue Kategorie mit den Einstellungen der &apos;GRASS Digitalisieren&apos;-Werkzeugkiste hinzufügen</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>Löschen</translation>
    </message>
    <message>
        <source>Delete selected category</source>
        <translation>Lösche gewählte Kategorie</translation>
    </message>
</context>
<context>
    <name>QgsGrassBrowser</name>
    <message>
        <source>Tools</source>
        <translation>Werkzeuge</translation>
    </message>
    <message>
        <source>Add selected map to canvas</source>
        <translation>Ausgewählten Layer dem Kartenfenster hinzufügen</translation>
    </message>
    <message>
        <source>Copy selected map</source>
        <translation>Gewählte Karte kopieren</translation>
    </message>
    <message>
        <source>Rename selected map</source>
        <translation>Gewählte Karte umbenennen</translation>
    </message>
    <message>
        <source>Delete selected map</source>
        <translation>Gewählte Karte löschen</translation>
    </message>
    <message>
        <source>Set current region to selected map</source>
        <translation>Setze die Region auf die gewählte Karte</translation>
    </message>
    <message>
        <source>Refresh</source>
        <translation>Neu zeichnen</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Warnung</translation>
    </message>
    <message>
        <source>Cannot copy map </source>
        <translation>Kann die Karte nicht kopieren.</translation>
    </message>
    <message>
        <source>&lt;br&gt;command: </source>
        <translation>&lt;br&gt; Kommando: </translation>
    </message>
    <message>
        <source>Cannot rename map </source>
        <translation>Kann die Karte nicht umbenennen </translation>
    </message>
    <message>
        <source>Delete map &lt;b&gt;</source>
        <translation>Lösche Karte &lt;b&gt;</translation>
    </message>
    <message>
        <source>Cannot delete map </source>
        <translation>Kann die Karte nicht löschen</translation>
    </message>
    <message>
        <source>Cannot write new region</source>
        <translation>Kann die neue Region nicht schreiben.</translation>
    </message>
    <message>
        <source>New name</source>
        <translation type="unfinished">Neuer Name</translation>
    </message>
</context>
<context>
    <name>QgsGrassEdit</name>
    <message>
        <source>New point</source>
        <translation>Neuer Punkt</translation>
    </message>
    <message>
        <source>New centroid</source>
        <translation>Neues Zentroid</translation>
    </message>
    <message>
        <source>Delete vertex</source>
        <translation>Lösche Vertex</translation>
    </message>
    <message>
        <source>Left: </source>
        <translation>Links:</translation>
    </message>
    <message>
        <source>Middle: </source>
        <translation>Mitte:</translation>
    </message>
    <message>
        <source>Edit tools</source>
        <translation>Digitalisierwerkzeuge</translation>
    </message>
    <message>
        <source>New line</source>
        <translation>Neue Zeile</translation>
    </message>
    <message>
        <source>New boundary</source>
        <translation>Neue Grenze</translation>
    </message>
    <message>
        <source>Move vertex</source>
        <translation>Verschiebe Vertex</translation>
    </message>
    <message>
        <source>Add vertex</source>
        <translation>Vertex hinzufügen</translation>
    </message>
    <message>
        <source>Move element</source>
        <translation>Verschiebe Element</translation>
    </message>
    <message>
        <source>Split line</source>
        <translation>Unterteile Linie</translation>
    </message>
    <message>
        <source>Delete element</source>
        <translation>Element löschen</translation>
    </message>
    <message>
        <source>Edit attributes</source>
        <translation>Editiere Attribute</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Schließen</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Warnung</translation>
    </message>
    <message>
        <source>You are not owner of the mapset, cannot open the vector for editing.</source>
        <translation>Sie sind nicht Besitzer des Mapsets. Folglich kann der Datensatz nicht zum Editieren geöffnet werden.</translation>
    </message>
    <message>
        <source>Cannot open vector for update.</source>
        <translation>Kann die Vekordatei nicht zum Aktualisiern öffnen.</translation>
    </message>
    <message>
        <source>Info</source>
        <translation>Information</translation>
    </message>
    <message>
        <source>The table was created</source>
        <translation>Die Tabelle wurde erstellt.</translation>
    </message>
    <message>
        <source>Tool not yet implemented.</source>
        <translation>Werkzeug ist noch nicht implementiert.</translation>
    </message>
    <message>
        <source>Cannot check orphan record: </source>
        <translation>Kann den verwaisten Eintrag nicht überprüfen: </translation>
    </message>
    <message>
        <source>Orphan record was left in attribute table. &lt;br&gt;Delete the record?</source>
        <translation>Verwaister Eintrag in der Attributtabelle gefunden. &lt;br&gt; Diesen Eintrag löschen?</translation>
    </message>
    <message>
        <source>Cannot delete orphan record: </source>
        <translation>Kann den verwaisten Eintrag nicht löschen:</translation>
    </message>
    <message>
        <source>Cannot describe table for field </source>
        <translation>Kann Tabelle oder Feld nicht beschreiben</translation>
    </message>
    <message>
        <source>Background</source>
        <translation type="unfinished">Hintergrund</translation>
    </message>
    <message>
        <source>Highlight</source>
        <translation type="unfinished">Hervorheben</translation>
    </message>
    <message>
        <source>Dynamic</source>
        <translation type="unfinished">Dynamisch</translation>
    </message>
    <message>
        <source>Point</source>
        <translation type="unfinished">Punkt</translation>
    </message>
    <message>
        <source>Line</source>
        <translation type="unfinished">Linie</translation>
    </message>
    <message>
        <source>Boundary (no area)</source>
        <translation>Grenzlinie (keine Fläche)</translation>
    </message>
    <message>
        <source>Boundary (1 area)</source>
        <translation>Grenzlinie (eine Fläche)</translation>
    </message>
    <message>
        <source>Boundary (2 areas)</source>
        <translation>Grenzlinie (zwei Flächen)</translation>
    </message>
    <message>
        <source>Centroid (in area)</source>
        <translation>Zentroid (innerhalb der Fläche)</translation>
    </message>
    <message>
        <source>Centroid (outside area)</source>
        <translation>Zentroid (außerhalb der Fläche)</translation>
    </message>
    <message>
        <source>Centroid (duplicate in area)</source>
        <translation>Zentroid (Duplikat in der Fläche)</translation>
    </message>
    <message>
        <source>Node (1 line)</source>
        <translation>Knotenpunkt (1 Linie)</translation>
    </message>
    <message>
        <source>Node (2 lines)</source>
        <translation>Knotenpunkt (2 Linien)</translation>
    </message>
    <message>
        <source>Column</source>
        <translation type="obsolete">Spalte</translation>
    </message>
    <message>
        <source>Type</source>
        <translation type="obsolete">Typ</translation>
    </message>
    <message>
        <source>Length</source>
        <translation type="obsolete">Länge</translation>
    </message>
    <message>
        <source>Next not used</source>
        <translation>Nächst folgender Kategoriewert</translation>
    </message>
    <message>
        <source>Manual entry</source>
        <translation type="unfinished">Manueller Eintrag</translation>
    </message>
    <message>
        <source>No category</source>
        <translation type="unfinished">Keine Kategorie</translation>
    </message>
    <message>
        <source>Right: </source>
        <translation type="unfinished">Rechts: </translation>
    </message>
    <message>
        <source>Disp</source>
        <comment>

Column title</comment>
        <translation type="obsolete">Anz.</translation>
    </message>
    <message>
        <source>Color</source>
        <comment>

Column title</comment>
        <translation type="obsolete">Farbe</translation>
    </message>
    <message>
        <source>Type</source>
        <comment>

Column title</comment>
        <translation type="obsolete">Typ</translation>
    </message>
    <message>
        <source>Index</source>
        <comment>

Column title</comment>
        <translation type="obsolete">Index</translation>
    </message>
</context>
<context>
    <name>QgsGrassEditBase</name>
    <message>
        <source>GRASS Edit</source>
        <translation>GRASS Digitalisieren</translation>
    </message>
    <message>
        <source>Category</source>
        <translation>Kategorie</translation>
    </message>
    <message>
        <source>Mode</source>
        <translation>Modus</translation>
    </message>
    <message>
        <source>Settings</source>
        <translation>Einstellungen</translation>
    </message>
    <message>
        <source>Snapping in screen pixels</source>
        <translation>Snappingtoleranz in Bildschirmpixel</translation>
    </message>
    <message>
        <source>Symbology</source>
        <translation>Bezeichnungen</translation>
    </message>
    <message>
        <source>Column 1</source>
        <translation type="obsolete">Spalte 1</translation>
    </message>
    <message>
        <source>Table</source>
        <translation>Tabelle</translation>
    </message>
    <message>
        <source>Add Column</source>
        <translation>Attribut hinzufügen</translation>
    </message>
    <message>
        <source>Create / Alter Table</source>
        <translation>Tabelle erzeugen/verändern</translation>
    </message>
    <message>
        <source>Line width</source>
        <translation>Linienbreite</translation>
    </message>
    <message>
        <source>Marker size</source>
        <translation>Markergröße</translation>
    </message>
    <message>
        <source>Layer</source>
        <translation type="unfinished">Layer</translation>
    </message>
    <message>
        <source>Disp</source>
        <translation type="unfinished">Anz.</translation>
    </message>
    <message>
        <source>Color</source>
        <translation type="unfinished">Farbe</translation>
    </message>
    <message>
        <source>Type</source>
        <translation type="unfinished">Typ</translation>
    </message>
    <message>
        <source>Index</source>
        <translation type="unfinished">Index</translation>
    </message>
    <message>
        <source>Column</source>
        <translation type="unfinished">Spalte</translation>
    </message>
    <message>
        <source>Length</source>
        <translation type="unfinished">Länge</translation>
    </message>
</context>
<context>
    <name>QgsGrassElementDialog</name>
    <message>
        <source>Cancel</source>
        <translation type="unfinished">Abbrechen</translation>
    </message>
    <message>
        <source>Ok</source>
        <translation type="unfinished">OK</translation>
    </message>
    <message>
        <source>&lt;font color=&apos;red&apos;&gt;Enter a name!&lt;/font&gt;</source>
        <translation type="unfinished">&lt;font color=&apos;red&apos;&gt;Name eingeben!&lt;/font&gt;</translation>
    </message>
    <message>
        <source>&lt;font color=&apos;red&apos;&gt;This is name of the source!&lt;/font&gt;</source>
        <translation>&lt;font color=&apos;red&apos;&gt;Dies ist der Name der Quelle!&lt;/font&gt;</translation>
    </message>
    <message>
        <source>&lt;font color=&apos;red&apos;&gt;Exists!&lt;/font&gt;</source>
        <translation>&lt;font color=&apos;red&apos;&gt;Existiert!&lt;/font&gt;</translation>
    </message>
    <message>
        <source>Overwrite</source>
        <translation type="unfinished">Überschreiben</translation>
    </message>
</context>
<context>
    <name>QgsGrassMapcalc</name>
    <message>
        <source>Mapcalc tools</source>
        <translation>Mapcalc-Werkzeug</translation>
    </message>
    <message>
        <source>Add map</source>
        <translation>Karte hinzufügen</translation>
    </message>
    <message>
        <source>Add constant value</source>
        <translation>Konstanten Wert hinzufügen</translation>
    </message>
    <message>
        <source>Add operator or function</source>
        <translation>Operator oder Funktion hinzufügen</translation>
    </message>
    <message>
        <source>Add connection</source>
        <translation>Verbindung hinzufügen</translation>
    </message>
    <message>
        <source>Select item</source>
        <translation>Objekt wählen</translation>
    </message>
    <message>
        <source>Delete selected item</source>
        <translation>Lösche gewähltes Objekt</translation>
    </message>
    <message>
        <source>Open</source>
        <translation>Öffnen</translation>
    </message>
    <message>
        <source>Save</source>
        <translation>Speichern</translation>
    </message>
    <message>
        <source>Save as</source>
        <translation>Speichern unter</translation>
    </message>
    <message>
        <source>Addition</source>
        <translation>Addition</translation>
    </message>
    <message>
        <source>Subtraction</source>
        <translation>Subtraktion</translation>
    </message>
    <message>
        <source>Multiplication</source>
        <translation>Multiplikation</translation>
    </message>
    <message>
        <source>Division</source>
        <translation>Teilung</translation>
    </message>
    <message>
        <source>Modulus</source>
        <translation>Modus</translation>
    </message>
    <message>
        <source>Exponentiation</source>
        <translation>Exponent</translation>
    </message>
    <message>
        <source>Equal</source>
        <translation>gleich</translation>
    </message>
    <message>
        <source>Not equal</source>
        <translation>ungleich</translation>
    </message>
    <message>
        <source>Greater than</source>
        <translation>größer als</translation>
    </message>
    <message>
        <source>Greater than or equal</source>
        <translation>größer gleich</translation>
    </message>
    <message>
        <source>Less than</source>
        <translation>kleiner als</translation>
    </message>
    <message>
        <source>Less than or equal</source>
        <translation>kleiner gleich</translation>
    </message>
    <message>
        <source>And</source>
        <translation>Und</translation>
    </message>
    <message>
        <source>Or</source>
        <translation>Oder</translation>
    </message>
    <message>
        <source>Absolute value of x</source>
        <translation>Absoluter Wert für x</translation>
    </message>
    <message>
        <source>Inverse tangent of x (result is in degrees)</source>
        <translation>Inverser tangenz von X (Resultat in Grad),</translation>
    </message>
    <message>
        <source>Inverse tangent of y/x (result is in degrees)</source>
        <translation>Inverser Tangenz von y/x (Resultat in Grad).</translation>
    </message>
    <message>
        <source>Current column of moving window (starts with 1)</source>
        <translation>Aktuelle Spalte des Moving Windows (startet bei 1)</translation>
    </message>
    <message>
        <source>Cosine of x (x is in degrees)</source>
        <translation>Kosinus von X (X in Grad).</translation>
    </message>
    <message>
        <source>Convert x to double-precision floating point</source>
        <translation>Konvertiert x in doppelte Fließkommazahl</translation>
    </message>
    <message>
        <source>Current east-west resolution</source>
        <translation>Aktuelle Ost-West-Auflösung</translation>
    </message>
    <message>
        <source>Exponential function of x</source>
        <translation>Exponentielle Funktion von x</translation>
    </message>
    <message>
        <source>x to the power y</source>
        <translation>x hoch y</translation>
    </message>
    <message>
        <source>Convert x to single-precision floating point</source>
        <translation>Konvertiert x in einfache Fließkommazahl</translation>
    </message>
    <message>
        <source>Decision: 1 if x not zero, 0 otherwise</source>
        <translation>Entscheidung: 1 wenn x nicht NULL, andererseits 0</translation>
    </message>
    <message>
        <source>Decision: a if x not zero, 0 otherwise</source>
        <translation>Entscheidung: a wenn x nicht NULL, andererseits 0</translation>
    </message>
    <message>
        <source>Decision: a if x not zero, b otherwise</source>
        <translation>Entscheidung: a wenn x nicht NULL, andererseits b</translation>
    </message>
    <message>
        <source>Decision: a if x &gt; 0, b if x is zero, c if x &lt; 0</source>
        <translation>Entscheidung: a wenn x &gt; 0, c wenn x &lt; 0</translation>
    </message>
    <message>
        <source>Convert x to integer [ truncates ]</source>
        <translation>Konvertiert x zu integer [ schneidet ab ]</translation>
    </message>
    <message>
        <source>Check if x = NULL</source>
        <translation>Überprüfe, wenn x = NULL</translation>
    </message>
    <message>
        <source>Natural log of x</source>
        <translation>Natürlicher Log von x </translation>
    </message>
    <message>
        <source>Log of x base b</source>
        <translation>Log von x zur Basis b</translation>
    </message>
    <message>
        <source>Largest value</source>
        <translation>Maximum</translation>
    </message>
    <message>
        <source>Median value</source>
        <translation>Median</translation>
    </message>
    <message>
        <source>Smallest value</source>
        <translation>Minimum</translation>
    </message>
    <message>
        <source>Mode value</source>
        <translation>Modus</translation>
    </message>
    <message>
        <source>1 if x is zero, 0 otherwise</source>
        <translation>1 wenn x zero ist, ansonsten 0</translation>
    </message>
    <message>
        <source>Current north-south resolution</source>
        <translation>Aktuelle Nord-Süd-Auflösung</translation>
    </message>
    <message>
        <source>NULL value</source>
        <translation>NULL Wert</translation>
    </message>
    <message>
        <source>Random value between a and b</source>
        <translation>Zufallswert zwischen a und b</translation>
    </message>
    <message>
        <source>Round x to nearest integer</source>
        <translation>Runde x zum nächsten Integerwert</translation>
    </message>
    <message>
        <source>Current row of moving window (Starts with 1)</source>
        <translation>Aktuelle Zeile des Moving windows (startet bei 1)</translation>
    </message>
    <message>
        <source>Sine of x (x is in degrees)</source>
        <comment>sin(x)</comment>
        <translation>Sinus von X (x in Grad).</translation>
    </message>
    <message>
        <source>Square root of x</source>
        <comment>sqrt(x)</comment>
        <translation>Wurzel von x</translation>
    </message>
    <message>
        <source>Tangent of x (x is in degrees)</source>
        <comment>tan(x)</comment>
        <translation>Tangenz von X (X in Grad).</translation>
    </message>
    <message>
        <source>Current x-coordinate of moving window</source>
        <translation>Aktuelle X-Koordinate des Moving-Windows.</translation>
    </message>
    <message>
        <source>Current y-coordinate of moving window</source>
        <translation>Aktuelle Y-Koordinate des Moving-Windows.</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Warnung</translation>
    </message>
    <message>
        <source>Cannot get current region</source>
        <translation>Kann die aktuelle Region nicht ermitteln.</translation>
    </message>
    <message>
        <source>Cannot check region of map </source>
        <translation>Kann die Region der Karte nicht überprüfen</translation>
    </message>
    <message>
        <source>Cannot get region of map </source>
        <translation>Kann die Region der Karte nicht ermitteln</translation>
    </message>
    <message>
        <source>No GRASS raster maps currently in QGIS</source>
        <translation>Derzeit sind keine GRASS-Rasterkarten in QGIS geladen.</translation>
    </message>
    <message>
        <source>Cannot create &apos;mapcalc&apos; directory in current mapset.</source>
        <translation>Kann keinen &apos;Mapcalc&apos;-Odner im aktuellen Mapset erstellen.</translation>
    </message>
    <message>
        <source>New mapcalc</source>
        <translation>Neue Mapcalc</translation>
    </message>
    <message>
        <source>Enter new mapcalc name:</source>
        <translation>Einen neuen Mapcalc-Namen eingeben:</translation>
    </message>
    <message>
        <source>Enter vector name</source>
        <translation>Name für die Vektordatei eingeben.</translation>
    </message>
    <message>
        <source>The file already exists. Overwrite? </source>
        <translation>Die Datei existiert bereits. Überschreiben?</translation>
    </message>
    <message>
        <source>Save mapcalc</source>
        <translation>mapcalc speichern</translation>
    </message>
    <message>
        <source>File name empty</source>
        <translation>Dateiname leer.</translation>
    </message>
    <message>
        <source>Cannot open mapcalc file</source>
        <translation>Kann die Mapcalc-Datei nicht öffnen.</translation>
    </message>
    <message>
        <source>The mapcalc schema (</source>
        <translation>Das Mapcalc-Schema (</translation>
    </message>
    <message>
        <source>) not found.</source>
        <translation>) nicht gefunden.</translation>
    </message>
    <message>
        <source>Cannot open mapcalc schema (</source>
        <translation>Kann das Mapcalc-Schema nicht öffnen (</translation>
    </message>
    <message>
        <source>Cannot read mapcalc schema (</source>
        <translation>Kann das Mapcalc-Schema nicht lesen (</translation>
    </message>
    <message>
        <source>
at line </source>
        <translation>
bei Zeile </translation>
    </message>
    <message>
        <source> column </source>
        <translation> Spalte </translation>
    </message>
    <message>
        <source>Output</source>
        <translation type="unfinished">Ergebnis</translation>
    </message>
</context>
<context>
    <name>QgsGrassMapcalcBase</name>
    <message>
        <source>MainWindow</source>
        <translation>MainWindow</translation>
    </message>
    <message>
        <source>Output</source>
        <translation>Ergebnis</translation>
    </message>
</context>
<context>
    <name>QgsGrassModule</name>
    <message>
        <source>Run</source>
        <translation>Los</translation>
    </message>
    <message>
        <source>Stop</source>
        <translation>Stop</translation>
    </message>
    <message>
        <source>Module</source>
        <translation type="unfinished">Modul</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation type="unfinished">Warnung</translation>
    </message>
    <message>
        <source>The module file (</source>
        <translation type="unfinished">Die Moduldatei </translation>
    </message>
    <message>
        <source>) not found.</source>
        <translation type="unfinished">) nicht gefunden.</translation>
    </message>
    <message>
        <source>Cannot open module file (</source>
        <translation type="unfinished">Moduldatei nicht geöffnet (</translation>
    </message>
    <message>
        <source>)</source>
        <translation type="unfinished">)</translation>
    </message>
    <message>
        <source>Cannot read module file (</source>
        <translation type="unfinished">Konnte Moduldatei nicht lesen (</translation>
    </message>
    <message>
        <source>):
</source>
        <translation type="unfinished">):
</translation>
    </message>
    <message>
        <source>
at line </source>
        <translation type="unfinished">
bei Zeile </translation>
    </message>
    <message>
        <source>Module </source>
        <translation type="unfinished">Modul </translation>
    </message>
    <message>
        <source> not found</source>
        <translation type="unfinished"> nicht gefunden</translation>
    </message>
    <message>
        <source>Cannot find man page </source>
        <translation type="unfinished">Handbuchseite nicht gefunden: </translation>
    </message>
    <message>
        <source>Not available, description not found (</source>
        <translation type="unfinished">Nicht verfügbar, Beschreibung nicht gefunden (</translation>
    </message>
    <message>
        <source>Not available, cannot open description (</source>
        <translation type="unfinished">Nicht verfügbar, konnte Beschreibung nicht öffnen (</translation>
    </message>
    <message>
        <source> column </source>
        <translation type="unfinished"> Spalte </translation>
    </message>
    <message>
        <source>Not available, incorrect description (</source>
        <translation type="unfinished">Nicht verfügbar, falsche Beschreibung (</translation>
    </message>
    <message>
        <source>Cannot get input region</source>
        <translation>Konnte Eingabe-&apos;region&apos; nicht finden</translation>
    </message>
    <message>
        <source>Use Input Region</source>
        <translation>Eingabe-&apos;region&apos; benutzen</translation>
    </message>
    <message>
        <source>Cannot find module </source>
        <translation type="unfinished">Konnte Modul nicht finden: </translation>
    </message>
    <message>
        <source>Cannot start module: </source>
        <translation type="unfinished">Konnte Modul nicht starten: </translation>
    </message>
    <message>
        <source>&lt;B&gt;Successfully finished&lt;/B&gt;</source>
        <translation type="unfinished">&lt;B&gt;Erfolgreich beendet&lt;/B&gt;</translation>
    </message>
    <message>
        <source>&lt;B&gt;Finished with error&lt;/B&gt;</source>
        <translation type="unfinished">&lt;B&gt;Mit Fehler beendet&lt;/B&gt;</translation>
    </message>
    <message>
        <source>&lt;B&gt;Module crashed or killed&lt;/B&gt;</source>
        <translation type="unfinished">&lt;B&gt;Modul abgestürzt oder abgebrochen&lt;/B&gt;</translation>
    </message>
    <message>
        <source>Please ensure you have the GRASS documentation installed.</source>
        <translation type="unfinished">Bitte stellen Sie sicher, dass die GRASS-Dokumentation installiert ist.</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleBase</name>
    <message>
        <source>GRASS Module</source>
        <translation>GRASS Modul</translation>
    </message>
    <message>
        <source>Options</source>
        <translation>Optionen</translation>
    </message>
    <message>
        <source>Output</source>
        <translation>Ergebnis</translation>
    </message>
    <message>
        <source>Manual</source>
        <translation>Handbuch</translation>
    </message>
    <message>
        <source>Run</source>
        <translation>Starten</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Schließen</translation>
    </message>
    <message>
        <source>View output</source>
        <translation>Ergebnis visualisieren</translation>
    </message>
    <message>
        <source>TextLabel</source>
        <translation type="unfinished">Textbeschriftung</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleField</name>
    <message>
        <source>Attribute field</source>
        <translation type="unfinished">Attributfeld</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleFile</name>
    <message>
        <source>File</source>
        <translation type="unfinished">Datei</translation>
    </message>
    <message>
        <source>:&amp;nbsp;missing value</source>
        <translation type="unfinished">:&amp;nbsp;fehlender Wert</translation>
    </message>
    <message>
        <source>:&amp;nbsp;directory does not exist</source>
        <translation type="unfinished">:&amp;nbsp;Verzeichnis existiert nicht</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleGdalInput</name>
    <message>
        <source>Warning</source>
        <translation type="unfinished">Warnung</translation>
    </message>
    <message>
        <source>Cannot find layeroption </source>
        <translation type="unfinished">Kann Layeroption nicht finden: </translation>
    </message>
    <message>
        <source>PostGIS driver in OGR does not support schemas!&lt;br&gt;Only the table name will be used.&lt;br&gt;It can result in wrong input if more tables of the same name&lt;br&gt;are present in the database.</source>
        <translation type="unfinished">Der PostGIS-Treiber in OGR unterstützt keine Schemata!&lt;br&gt;Nur der Tabellenname wird benutzt.&lt;br&gt;Die kann zu falschen Eingaben führen, wenn mehrere Tabellen gleichen Namens&lt;br&gt;in der Datenbank vorkommen.</translation>
    </message>
    <message>
        <source>:&amp;nbsp;no input</source>
        <translation type="unfinished">:&amp;nbsp;keine Eingabe</translation>
    </message>
    <message>
        <source>Cannot find whereoption </source>
        <translation>Kann where Option nicht finden  </translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleInput</name>
    <message>
        <source>Warning</source>
        <translation type="unfinished">Warnung</translation>
    </message>
    <message>
        <source>Cannot find typeoption </source>
        <translation type="unfinished">Typoption nicht gefunden</translation>
    </message>
    <message>
        <source>Cannot find values for typeoption </source>
        <translation type="unfinished">Keine Werte für Typoption gefunden: </translation>
    </message>
    <message>
        <source>Cannot find layeroption </source>
        <translation type="unfinished">Layeroption nicht gefunden: </translation>
    </message>
    <message>
        <source>GRASS element </source>
        <translation type="unfinished">GRASS-Element </translation>
    </message>
    <message>
        <source> not supported</source>
        <translation type="unfinished"> nicht unterstützt</translation>
    </message>
    <message>
        <source>Use region of this map</source>
        <translation>Karten-&apos;region&apos; benutzen</translation>
    </message>
    <message>
        <source>:&amp;nbsp;no input</source>
        <translation type="unfinished">:&amp;nbsp;Keine Eingabe</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleOption</name>
    <message>
        <source>:&amp;nbsp;missing value</source>
        <translation type="unfinished">:&amp;nbsp;fehlender Wert</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleSelection</name>
    <message>
        <source>Attribute field</source>
        <translation type="unfinished">Attributfeld</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleStandardOptions</name>
    <message>
        <source>Warning</source>
        <translation type="unfinished">Warnung</translation>
    </message>
    <message>
        <source>Cannot find module </source>
        <translation type="unfinished">Kann Modul nicht finden: </translation>
    </message>
    <message>
        <source>Cannot start module </source>
        <translation type="unfinished">Kann Modul nicht starten: </translation>
    </message>
    <message>
        <source>Cannot read module description (</source>
        <translation type="unfinished">Kann Modulbeschreibung nicht lesen: </translation>
    </message>
    <message>
        <source>):
</source>
        <translation type="unfinished">):
</translation>
    </message>
    <message>
        <source>
at line </source>
        <translation type="unfinished">
bei Zeile </translation>
    </message>
    <message>
        <source> column </source>
        <translation type="unfinished"> Spalte </translation>
    </message>
    <message>
        <source>Cannot find key </source>
        <translation type="unfinished">Kann Schlüssel nicht finden: </translation>
    </message>
    <message>
        <source>Item with id </source>
        <translation type="unfinished">Element mit ID </translation>
    </message>
    <message>
        <source> not found</source>
        <translation type="unfinished"> nicht gefunden</translation>
    </message>
    <message>
        <source>Cannot get current region</source>
        <translation>Kann die aktuelle &apos;region&apos; nicht ermitteln.</translation>
    </message>
    <message>
        <source>Cannot check region of map </source>
        <translation>Kann die &apos;region&apos; der Karte nicht überprüfen</translation>
    </message>
    <message>
        <source>Cannot set region of map </source>
        <translation>Kann Karten-&apos;region&apos; nicht setzen: </translation>
    </message>
</context>
<context>
    <name>QgsGrassNewMapset</name>
    <message>
        <source>GRASS database</source>
        <translation type="obsolete">GRASS-Datenbank</translation>
    </message>
    <message>
        <source>GRASS location</source>
        <translation type="obsolete">GRASS-Location</translation>
    </message>
    <message>
        <source>Projection</source>
        <translation type="obsolete">Projektion</translation>
    </message>
    <message>
        <source>Default GRASS Region</source>
        <translation type="obsolete">Voreingestellte GRASS-&apos;region&apos;</translation>
    </message>
    <message>
        <source>Mapset</source>
        <translation type="obsolete">Mapset</translation>
    </message>
    <message>
        <source>Create New Mapset</source>
        <translation type="obsolete">Neues Mapset erzeugen</translation>
    </message>
    <message>
        <source>Tree</source>
        <translation type="obsolete">Baum</translation>
    </message>
    <message>
        <source>Comment</source>
        <translation type="obsolete">Kommentar</translation>
    </message>
    <message>
        <source>Database</source>
        <translation type="unfinished">Datenbank</translation>
    </message>
    <message>
        <source>Location 2</source>
        <translation>2. Location</translation>
    </message>
    <message>
        <source>User&apos;s mapset</source>
        <translation>Benutzer Mapset</translation>
    </message>
    <message>
        <source>System mapset</source>
        <translation>System Mapset</translation>
    </message>
    <message>
        <source>Location 1</source>
        <translation>1. Location</translation>
    </message>
    <message>
        <source>Owner</source>
        <translation type="obsolete">Besitzer</translation>
    </message>
    <message>
        <source>Enter path to GRASS database</source>
        <translation type="unfinished">Pfad zur GRASS-Datenbank angeben</translation>
    </message>
    <message>
        <source>The directory doesn&apos;t exist!</source>
        <translation type="unfinished">Das Verzeichnis existiert nicht!</translation>
    </message>
    <message>
        <source>No writable locations, the database not writable!</source>
        <translation type="unfinished">Keine </translation>
    </message>
    <message>
        <source>Enter location name!</source>
        <translation>Location-Name angeben</translation>
    </message>
    <message>
        <source>The location exists!</source>
        <translation>Die Location existiert!</translation>
    </message>
    <message>
        <source>Selected projection is not supported by GRASS!</source>
        <translation>Ausgewählte Projektion wird nicht von GRASS unterstützt!</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation type="unfinished">Warnung</translation>
    </message>
    <message>
        <source>Cannot create projection.</source>
        <translation type="unfinished">Kann Projektion nicht erzeugen</translation>
    </message>
    <message>
        <source>Cannot reproject previously set region, default region set.</source>
        <translation>Kann Region nicht reprojizieren. Voreingestellte Projektion gesetzt.</translation>
    </message>
    <message>
        <source>North must be greater than south</source>
        <translation type="unfinished">Nord muß größer als Süd sein</translation>
    </message>
    <message>
        <source>East must be greater than west</source>
        <translation type="unfinished">Ost muß größer als West sein</translation>
    </message>
    <message>
        <source>Regions file (</source>
        <translation>Region-Datei (</translation>
    </message>
    <message>
        <source>) not found.</source>
        <translation type="unfinished">) nicht gefunden.</translation>
    </message>
    <message>
        <source>Cannot open locations file (</source>
        <translation>Kann Location-Datei  nicht öffnen (</translation>
    </message>
    <message>
        <source>)</source>
        <translation>)</translation>
    </message>
    <message>
        <source>Cannot read locations file (</source>
        <translation>Kann Location-Datei nicht lesen (</translation>
    </message>
    <message>
        <source>):
</source>
        <translation type="unfinished">):
</translation>
    </message>
    <message>
        <source>
at line </source>
        <translation type="unfinished">
bei Zeile </translation>
    </message>
    <message>
        <source> column </source>
        <translation type="unfinished"> Spalte </translation>
    </message>
    <message>
        <source>Cannot create QgsSpatialRefSys</source>
        <translation type="obsolete">Kann QgsSpatialRefSys nicht erzeugen</translation>
    </message>
    <message>
        <source>Cannot reproject selected region.</source>
        <translation>Kann ausgewählte &apos;region&apos; nicht reprojizieren.</translation>
    </message>
    <message>
        <source>Cannot reproject region</source>
        <translation>Kann &apos;region&apos; nicht reprojizieren.</translation>
    </message>
    <message>
        <source>Enter mapset name.</source>
        <translation>Mapset angeben</translation>
    </message>
    <message>
        <source>The mapset already exists</source>
        <translation>Die Mapset existiert bereits</translation>
    </message>
    <message>
        <source>Database: </source>
        <translation type="unfinished">Datenbank: </translation>
    </message>
    <message>
        <source>Location: </source>
        <translation>Location: </translation>
    </message>
    <message>
        <source>Mapset: </source>
        <translation>Mapset: </translation>
    </message>
    <message>
        <source>Create location</source>
        <translation>Location anlegen</translation>
    </message>
    <message>
        <source>Cannot create new location: </source>
        <translation>Kann neue Location nicht anlegen: </translation>
    </message>
    <message>
        <source>Create mapset</source>
        <translation>Mapset anlegen</translation>
    </message>
    <message>
        <source>Cannot create new mapset directory</source>
        <translation>Kann Mapset-Verzeichnis nicht anlegen</translation>
    </message>
    <message>
        <source>Cannot open DEFAULT_WIND</source>
        <translation type="unfinished">Kann DEFAULT_WIND nicht öffnen</translation>
    </message>
    <message>
        <source>Cannot open WIND</source>
        <translation type="unfinished">Kann WIND nicht öffnen</translation>
    </message>
    <message>
        <source>New mapset</source>
        <translation>Neue Location/Mapset</translation>
    </message>
    <message>
        <source>New mapset successfully created, but cannot be opened: </source>
        <translation>Mapset erfolgreich angelegt, konnte aber nicht geöffnet werden: </translation>
    </message>
    <message>
        <source>New mapset successfully created and set as current working mapset.</source>
        <translation>Mapset erfolgreich erzeugt und als aktuelle Arbeitsumgebung eingestellt.</translation>
    </message>
    <message>
        <source>Cannot create QgsCoordinateReferenceSystem</source>
        <translation type="unfinished">Kann QgsCoordinateReferenceSystem nicht erzeugen</translation>
    </message>
</context>
<context>
    <name>QgsGrassNewMapsetBase</name>
    <message>
        <source>Column 1</source>
        <translation type="obsolete">Spalte 1</translation>
    </message>
    <message>
        <source>Example directory tree:</source>
        <translation>Beispielordnerstruktur:</translation>
    </message>
    <message>
        <source>Database Error</source>
        <translation>Datenbank Fehler</translation>
    </message>
    <message>
        <source>Database:</source>
        <translation>Datenbank:</translation>
    </message>
    <message>
        <source>...</source>
        <translation type="obsolete">...</translation>
    </message>
    <message>
        <source>Select existing directory or create a new one:</source>
        <translation>Existierenden Ordner wählen oder neuen erzeugen:</translation>
    </message>
    <message>
        <source>Location</source>
        <translation>Location</translation>
    </message>
    <message>
        <source>Select location</source>
        <translation>Wähle Location</translation>
    </message>
    <message>
        <source>Create new location</source>
        <translation>Erstelle neue Location</translation>
    </message>
    <message>
        <source>Location Error</source>
        <translation>Location Fehler</translation>
    </message>
    <message>
        <source>Projection Error</source>
        <translation>Projektionsfehler</translation>
    </message>
    <message>
        <source>Coordinate system</source>
        <translation>Koordinatensystem</translation>
    </message>
    <message>
        <source>Projection</source>
        <translation>Projektion</translation>
    </message>
    <message>
        <source>Not defined</source>
        <translation>Nicht definiert</translation>
    </message>
    <message>
        <source>Set current QGIS extent</source>
        <translation>Setze aktuelle QGIS Ausdehnung</translation>
    </message>
    <message>
        <source>Set</source>
        <translation>Setzen</translation>
    </message>
    <message>
        <source>Region Error</source>
        <translation>Region-Fehler</translation>
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
        <source>E</source>
        <translation>E</translation>
    </message>
    <message>
        <source>N</source>
        <translation>N</translation>
    </message>
    <message>
        <source>New mapset:</source>
        <translation>Neues Mapset:</translation>
    </message>
    <message>
        <source>Mapset Error</source>
        <translation>Mapset Fehler</translation>
    </message>
    <message>
        <source>&lt;p align=&quot;center&quot;&gt;Existing masets&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;center&quot;&gt; Vorhandene Mapsets&lt;/p&gt;</translation>
    </message>
    <message>
        <source>Location:</source>
        <translation>Location:</translation>
    </message>
    <message>
        <source>Mapset:</source>
        <translation>Mapset:</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;GRASS data are stored in tree directory structure.&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;The GRASS database is the top-level directory in this tree structure.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;GRASS-Daten sind als hierarchische Struktur in Ordnern abgelegt.&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;Die GRASS Datenbank (Location) bildet die oberste Ebene in dieser Struktur.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;The GRASS location is a collection of maps for a particular territory or project.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;Die GRASS location ist eine Sammlung von Karten in einer definierten Projektion und räumlichen Ausdehnung.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;The GRASS region defines a workspace for raster modules. The default region is valid for one location. It is possible to set a different region in each mapset. &lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;It is possible to change the default location region later.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;Die GRASS region definiert einen Analyseraum für Rastermodule. DIe &apos;default region&apos; ist für jeweils eine Location gültig. Es ist möglich, unterschiedliche region Einstellungen in verschiedenen Mapsets einzustellen. &lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;Es ist möglich, die &apos;default region&apos; nachträglich zu ändern.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;The GRASS mapset is a collection of maps used by one user. &lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;A user can read maps from all mapsets in the location but &lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;he can open for writing only his mapset (owned by user).&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;Eine GRASS Mapset is Sammlung von Karten, die von einem Benutzer benutzt wird.&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;Jeder Benutzer kann sämtliche Karten aller Mapsets einer Location lesen, aber&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;er hat nur innerhalb der eigenen Mapset Schreibrecht, um neue Karten zu erstellen.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>New Mapset</source>
        <translation type="unfinished">Neues Mapset</translation>
    </message>
    <message>
        <source>GRASS Database</source>
        <translation type="unfinished">GRASS-Datenbank</translation>
    </message>
    <message>
        <source>Tree</source>
        <translation type="unfinished">Baum</translation>
    </message>
    <message>
        <source>Comment</source>
        <translation type="unfinished">Kommentar</translation>
    </message>
    <message>
        <source>&lt;!DOCTYPE HTML PUBLIC &quot;-//W3C//DTD HTML 4.0//EN&quot; &quot;http://www.w3.org/TR/REC-html40/strict.dtd&quot;&gt;
&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Lucida Grande&apos;; font-size:13pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;GRASS data are stored in tree directory structure. The GRASS database is the top-level directory in this tree structure.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Browse...</source>
        <translation type="unfinished">Suchen...</translation>
    </message>
    <message>
        <source>GRASS Location</source>
        <translation type="unfinished">GRASS-Location</translation>
    </message>
    <message>
        <source>&lt;!DOCTYPE HTML PUBLIC &quot;-//W3C//DTD HTML 4.0//EN&quot; &quot;http://www.w3.org/TR/REC-html40/strict.dtd&quot;&gt;
&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Lucida Grande&apos;; font-size:13pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;The GRASS location is a collection of maps for a particular territory or project.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Default GRASS Region</source>
        <translation type="unfinished">Voreingestellte GRASS-&apos;region&apos;</translation>
    </message>
    <message>
        <source>&lt;!DOCTYPE HTML PUBLIC &quot;-//W3C//DTD HTML 4.0//EN&quot; &quot;http://www.w3.org/TR/REC-html40/strict.dtd&quot;&gt;
&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Lucida Grande&apos;; font-size:13pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;The GRASS region defines a workspace for raster modules. The default region is valid for one location. It is possible to set a different region in each mapset. It is possible to change the default location region later.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Mapset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Owner</source>
        <translation type="unfinished">Besitzer</translation>
    </message>
    <message>
        <source>&lt;!DOCTYPE HTML PUBLIC &quot;-//W3C//DTD HTML 4.0//EN&quot; &quot;http://www.w3.org/TR/REC-html40/strict.dtd&quot;&gt;
&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Lucida Grande&apos;; font-size:13pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;The GRASS mapset is a collection of maps used by one user. A user can read maps from all mapsets in the location but he can open for writing only his mapset (owned by user).&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Create New Mapset</source>
        <translation type="unfinished">Neues Mapset erzeugen</translation>
    </message>
</context>
<context>
    <name>QgsGrassPlugin</name>
    <message>
        <source>GRASS</source>
        <translation>GRASS</translation>
    </message>
    <message>
        <source>&amp;GRASS</source>
        <translation>&amp;GRASS</translation>
    </message>
    <message>
        <source>Open mapset</source>
        <translation>Mapset öffnen</translation>
    </message>
    <message>
        <source>New mapset</source>
        <translation>Neues Mapset</translation>
    </message>
    <message>
        <source>Close mapset</source>
        <translation>Schliesse Mapset</translation>
    </message>
    <message>
        <source>Add GRASS vector layer</source>
        <translation>GRASS-Vektorlayer hinzufügen</translation>
    </message>
    <message>
        <source>Add GRASS raster layer</source>
        <translation>GRASS-Rasterlayer hinzufügen</translation>
    </message>
    <message>
        <source>Open GRASS tools</source>
        <translation>GRASS-Werkzeugkiste öffnen</translation>
    </message>
    <message>
        <source>Display Current Grass Region</source>
        <translation>Aktuelle GRASS-Region darstellen</translation>
    </message>
    <message>
        <source>Edit Current Grass Region</source>
        <translation>Aktuelle GRASS-Region bearbeiten</translation>
    </message>
    <message>
        <source>Edit Grass Vector layer</source>
        <translation>GRASS-Vektorlayer bearbeiten</translation>
    </message>
    <message>
        <source>Adds a GRASS vector layer to the map canvas</source>
        <translation>Fügt dem Kartenfenster einen GRASS-Vektorlayer hinzu</translation>
    </message>
    <message>
        <source>Adds a GRASS raster layer to the map canvas</source>
        <translation>Fügt dem Kartenfenster einen GRASS-Rasterlayer hinzu</translation>
    </message>
    <message>
        <source>Displays the current GRASS region as a rectangle on the map canvas</source>
        <translation>Zeigt die aktuelle GRASS-Region als Rechteck im Kartenbild an</translation>
    </message>
    <message>
        <source>Edit the current GRASS region</source>
        <translation>Aktuelle GRASS-Region bearbeiten.</translation>
    </message>
    <message>
        <source>Edit the currently selected GRASS vector layer.</source>
        <translation>Gewählten GRASS-Vektorlayer bearbeiten.</translation>
    </message>
    <message>
        <source>GrassVector</source>
        <translation>GrassVektor</translation>
    </message>
    <message>
        <source>0.1</source>
        <translation>0.1</translation>
    </message>
    <message>
        <source>GRASS layer</source>
        <translation>GRASS-Layer</translation>
    </message>
    <message>
        <source>Create new Grass Vector</source>
        <translation>Neuen GRASS-Vektorlayer anlegen</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation type="unfinished">Warnung</translation>
    </message>
    <message>
        <source>GRASS Edit is already running.</source>
        <translation type="unfinished">GRASS-Digitalisierung läuft bereits.</translation>
    </message>
    <message>
        <source>New vector name</source>
        <translation type="unfinished">Neuer Vektorname</translation>
    </message>
    <message>
        <source>Cannot create new vector: </source>
        <translation type="unfinished">Kann Vektor nicht anlegen: </translation>
    </message>
    <message>
        <source>New vector created but cannot be opened by data provider.</source>
        <translation type="unfinished">Neuer Vektor konnte nicht durch Datenlieferant geöffnet werden.</translation>
    </message>
    <message>
        <source>Cannot start editing.</source>
        <translation type="unfinished">Konnte Digialisierung nicht beginnen.</translation>
    </message>
    <message>
        <source>GISDBASE, LOCATION_NAME or MAPSET is not set, cannot display current region.</source>
        <translation type="unfinished">GISDBASE, LOCATION_NAME oder MAPSET ist nicht gesetzt, aktuelle Region kann nicht angezeigt werden</translation>
    </message>
    <message>
        <source>Cannot read current region: </source>
        <translation type="unfinished">Kann aktuelle Region nicht lesen: </translation>
    </message>
    <message>
        <source>Cannot open the mapset. </source>
        <translation>Kann Mapset nicht öffnen: </translation>
    </message>
    <message>
        <source>Cannot close mapset. </source>
        <translation>Kann Mapset nicht schließen. </translation>
    </message>
    <message>
        <source>Cannot close current mapset. </source>
        <translation>Kann aktuellen Mapset nicht schließen. </translation>
    </message>
    <message>
        <source>Cannot open GRASS mapset. </source>
        <translation>Kann GRASS-Mapset nicht öffnen.</translation>
    </message>
</context>
<context>
    <name>QgsGrassRegion</name>
    <message>
        <source>Warning</source>
        <translation type="unfinished">Warnung</translation>
    </message>
    <message>
        <source>GISDBASE, LOCATION_NAME or MAPSET is not set, cannot display current region.</source>
        <translation type="unfinished">GISDBASE, LOCATION_NAME oder MAPSET ist nicht gesetzt, kann aktuelle Region nicht anzeigen.</translation>
    </message>
    <message>
        <source>Cannot read current region: </source>
        <translation type="unfinished">Kann aktuelle Region nicht lesen: </translation>
    </message>
    <message>
        <source>Cannot write region</source>
        <translation type="unfinished">Kann Region nicht schreiben</translation>
    </message>
</context>
<context>
    <name>QgsGrassRegionBase</name>
    <message>
        <source>GRASS Region Settings</source>
        <translation>GRASS-Regioneinstellungen</translation>
    </message>
    <message>
        <source>N</source>
        <translation>N</translation>
    </message>
    <message>
        <source>W</source>
        <translation>W</translation>
    </message>
    <message>
        <source>E</source>
        <translation>E</translation>
    </message>
    <message>
        <source>S</source>
        <translation>SS</translation>
    </message>
    <message>
        <source>N-S Res</source>
        <translation>N-S Aufl.</translation>
    </message>
    <message>
        <source>Rows</source>
        <translation>Zeilen</translation>
    </message>
    <message>
        <source>Cols</source>
        <translation>Spalten</translation>
    </message>
    <message>
        <source>E-W Res</source>
        <translation>E-W Aufl.</translation>
    </message>
    <message>
        <source>Color</source>
        <translation>Farbe</translation>
    </message>
    <message>
        <source>Width</source>
        <translation>Breite</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Abbrechen</translation>
    </message>
</context>
<context>
    <name>QgsGrassSelect</name>
    <message>
        <source>Select GRASS Vector Layer</source>
        <translation>Wählen Sie einen GRASS-Vektorlayer</translation>
    </message>
    <message>
        <source>Select GRASS Raster Layer</source>
        <translation>Wählen Sie einen GRASS-Rasterlayer</translation>
    </message>
    <message>
        <source>Select GRASS mapcalc schema</source>
        <translation>Wähen Sie ein GRASS Mapcalc-Schema</translation>
    </message>
    <message>
        <source>Select GRASS Mapset</source>
        <translation>Wählen Sie ein GRASS Mapset</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Warnung</translation>
    </message>
    <message>
        <source>Cannot open vector on level 2 (topology not available).</source>
        <translation>Kann den Vektordatensatz nicht in Level 2 öffnen (Topologie fehlt).</translation>
    </message>
    <message>
        <source>Choose existing GISDBASE</source>
        <translation>Bitte wählen Sie eine existierende GISDBASE.</translation>
    </message>
    <message>
        <source>Wrong GISDBASE, no locations available.</source>
        <translation>Falsche GISDBASE, darin sind keine Locations vorhanden.</translation>
    </message>
    <message>
        <source>Wrong GISDBASE</source>
        <translation>Falsche GISDBASE.</translation>
    </message>
    <message>
        <source>Select a map.</source>
        <translation>Wählen Sie eine Karte.</translation>
    </message>
    <message>
        <source>No map</source>
        <translation>Keine Karte</translation>
    </message>
    <message>
        <source>No layer</source>
        <translation>Kein Layer</translation>
    </message>
    <message>
        <source>No layers available in this map</source>
        <translation>Keine Layer in dieser Karte vorhanden.</translation>
    </message>
</context>
<context>
    <name>QgsGrassSelectBase</name>
    <message>
        <source>Gisdbase</source>
        <translation>Gisdbase</translation>
    </message>
    <message>
        <source>Location</source>
        <translation>Location</translation>
    </message>
    <message>
        <source>Browse</source>
        <translation>Durchsuchen</translation>
    </message>
    <message>
        <source>Mapset</source>
        <translation>Kartenset</translation>
    </message>
    <message>
        <source>Map name</source>
        <translation>Kartenname</translation>
    </message>
    <message>
        <source>Layer</source>
        <translation>Layer</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>Select or type map name (wildcards &apos;*&apos; and &apos;?&apos; accepted for rasters)</source>
        <translation>Kartenname (Wildcards &apos;*&apos; und &apos;?&apos; werden für Raster akzeptiert) wählen oder eingeben</translation>
    </message>
    <message>
        <source>Add GRASS Layer</source>
        <translation>GRASS-Layer hinzufügen</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Abbrechen</translation>
    </message>
</context>
<context>
    <name>QgsGrassShellBase</name>
    <message>
        <source>GRASS Shell</source>
        <translation>GRASS Kommandozeile</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Schließen</translation>
    </message>
</context>
<context>
    <name>QgsGrassTools</name>
    <message>
        <source>Browser</source>
        <translation>Browser</translation>
    </message>
    <message>
        <source>GRASS Tools</source>
        <translation>GRASS-Werkzeuge </translation>
    </message>
    <message>
        <source>GRASS Tools: </source>
        <translation>GRASS-Werkzeuge: </translation>
    </message>
    <message>
        <source>Warning</source>
        <translation type="unfinished">Warnung</translation>
    </message>
    <message>
        <source>Cannot find MSYS (</source>
        <translation type="unfinished">Kann MSYS nicht finden (</translation>
    </message>
    <message>
        <source>GRASS Shell is not compiled.</source>
        <translation type="unfinished">GRASS-Shell ist nicht kompiliert.</translation>
    </message>
    <message>
        <source>The config file (</source>
        <translation type="unfinished">Die Konfigurationdatei (</translation>
    </message>
    <message>
        <source>) not found.</source>
        <translation type="unfinished">) nicht gefunden.</translation>
    </message>
    <message>
        <source>Cannot open config file (</source>
        <translation type="unfinished">Kann Konfiguration nicht öffnen (</translation>
    </message>
    <message>
        <source>)</source>
        <translation type="unfinished">)</translation>
    </message>
    <message>
        <source>Cannot read config file (</source>
        <translation type="unfinished">Kann Konfiguration nicht lesen (</translation>
    </message>
    <message>
        <source>
at line </source>
        <translation type="unfinished">
bei Zeile </translation>
    </message>
    <message>
        <source> column </source>
        <translation type="unfinished"> Spalte </translation>
    </message>
</context>
<context>
    <name>QgsGrassToolsBase</name>
    <message>
        <source>Grass Tools</source>
        <translation type="unfinished">GRASS-Werkzeuge</translation>
    </message>
    <message>
        <source>Modules Tree</source>
        <translation type="unfinished">Modulbaum</translation>
    </message>
    <message>
        <source>1</source>
        <translation type="unfinished">1</translation>
    </message>
    <message>
        <source>Modules List</source>
        <translation type="unfinished">Modulliste</translation>
    </message>
</context>
<context>
    <name>QgsGridMakerPlugin</name>
    <message>
        <source>&amp;Graticule Creator</source>
        <translation>Koordinatenlinien-Generator</translation>
    </message>
    <message>
        <source>Creates a graticule (grid) and stores the result as a shapefile</source>
        <translation>Erzeugt ein Gradnetz (Grid) und speichert es in ein Shapefile.</translation>
    </message>
    <message>
        <source>&amp;Graticules</source>
        <translation>&amp;Geographisches Netz</translation>
    </message>
</context>
<context>
    <name>QgsGridMakerPluginGui</name>
    <message>
        <source>QGIS - Grid Maker</source>
        <translation>QGIS - Gitternetzbuilder</translation>
    </message>
    <message>
        <source>Please enter the file name before pressing OK!</source>
        <translation>Bitte geben Sie einen Dateinamen ein, bevor Sie auf OK drücken!</translation>
    </message>
    <message>
        <source>Choose a filename to save under</source>
        <translation type="obsolete">Bitte wählen Sie einen Dateinamen</translation>
    </message>
    <message>
        <source>ESRI Shapefile (*.shp)</source>
        <translation>ESRI Shapedatei (*.shp)</translation>
    </message>
    <message>
        <source>Please enter intervals before pressing OK!</source>
        <translation type="unfinished">Bitte Intervalle vor dem OK eingeben!</translation>
    </message>
    <message>
        <source>Choose a file name to save under</source>
        <translation type="unfinished">Dateiname zum Speichern wählen</translation>
    </message>
</context>
<context>
    <name>QgsGridMakerPluginGuiBase</name>
    <message>
        <source>Graticule Builder</source>
        <translation>Gitternetzbuilder</translation>
    </message>
    <message>
        <source>Type</source>
        <translation>Typ</translation>
    </message>
    <message>
        <source>Point</source>
        <translation>Punkt</translation>
    </message>
    <message>
        <source>Polygon</source>
        <translation>Polygon</translation>
    </message>
    <message>
        <source>Origin (lower left)</source>
        <translation>Ursprung (links unten)</translation>
    </message>
    <message>
        <source>End point (upper right)</source>
        <translation>Endpunkt (rechts oben)</translation>
    </message>
    <message>
        <source>Output (shape) file</source>
        <translation>Ergebnisdatei (Shape)</translation>
    </message>
    <message>
        <source>Save As...</source>
        <translation type="unfinished">Speichern unter...</translation>
    </message>
    <message>
        <source>QGIS Graticule Creator</source>
        <translation type="unfinished">QGIS-Gittererzeugung</translation>
    </message>
    <message>
        <source>Graticle size</source>
        <translation type="unfinished">Gittergröße</translation>
    </message>
    <message>
        <source>Y Interval:</source>
        <translation type="unfinished">Y-Interval:</translation>
    </message>
    <message>
        <source>X Interval:</source>
        <translation type="unfinished">X-Interval:</translation>
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
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:11pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;This plugin will help you to build a graticule shapefile that you can use as an overlay within your qgis map viewer.&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;;&quot;&gt;Please enter all units in decimal degrees&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:11pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;Dieses Plugin hilft Ihnen dabei, ein Shapefile mit einem Gradnetz zu erstellen, das Sie dann über ihre Layer im QGIS Kartenfenster legen können.&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;;&quot;&gt;Bitte geben Sie alle Einheiten als Dezimalgrad an.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
</context>
<context>
    <name>QgsHelpViewer</name>
    <message>
        <source>Quantum GIS Help - </source>
        <translation>Quantum GIS Hilfe -</translation>
    </message>
    <message>
        <source>Failed to get the help text from the database</source>
        <translation>Der Hilfetext konnte nicht aus der Datenbank geholt werden</translation>
    </message>
    <message>
        <source>Error</source>
        <translation>Fehler</translation>
    </message>
    <message>
        <source>The QGIS help database is not installed</source>
        <translation>Die QGIS Hilfedatenbank ist nicht installiert</translation>
    </message>
    <message>
        <source>This help file does not exist for your language</source>
        <translation>Diese Hilfedatei existiert noch nicht für Ihre Sprache.</translation>
    </message>
    <message>
        <source>If you would like to create it, contact the QGIS development team</source>
        <translation>Wenn Sie es erstellen wollen, kontaktieren Sie bitte das QGIS Entwicklungsteam.</translation>
    </message>
    <message>
        <source>Quantum GIS Help</source>
        <translation>Quantum GIS Hilfe</translation>
    </message>
</context>
<context>
    <name>QgsHelpViewerBase</name>
    <message>
        <source>QGIS Help</source>
        <translation>QGIS Hilfe</translation>
    </message>
    <message>
        <source>&amp;Home</source>
        <translation>&amp;Home</translation>
    </message>
    <message>
        <source>Alt+H</source>
        <translation>Alt+H</translation>
    </message>
    <message>
        <source>&amp;Forward</source>
        <translation>&amp;Vorwärts</translation>
    </message>
    <message>
        <source>Alt+F</source>
        <translation>Alt+F</translation>
    </message>
    <message>
        <source>&amp;Back</source>
        <translation>&amp;Rückwärts</translation>
    </message>
    <message>
        <source>Alt+B</source>
        <translation>Alt+B</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>&amp;Schließen</translation>
    </message>
    <message>
        <source>Alt+C</source>
        <translation>Alt+C</translation>
    </message>
</context>
<context>
    <name>QgsHttpTransaction</name>
    <message>
        <source>WMS Server responded unexpectedly with HTTP Status Code %1 (%2)</source>
        <translation>WMS Server hat unerwarteterweise folgenden HTTP Status Code herausgegeben: %1 (%2)</translation>
    </message>
    <message>
        <source>HTTP response completed, however there was an error: %1</source>
        <translation>HTTP Antwort beendet, es habe jedoch Fehler: %1</translation>
    </message>
    <message>
        <source>HTTP transaction completed, however there was an error: %1</source>
        <translation>HTTP Übertragung beendet, aber es trat ein Fehler auf: %1</translation>
    </message>
    <message>
        <source>Network timed out after %1 seconds of inactivity.
This may be a problem in your network connection or at the WMS server.</source>
        <translation type="obsolete">
        
        </translation>
    </message>
</context>
<context>
    <name>QgsIDWInterpolatorDialogBase</name>
    <message>
        <source>Dialog</source>
        <translation type="unfinished">Dialog</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:12pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-weight:600;&quot;&gt;Inverse Distance Weighting&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-weight:600;&quot;&gt;&lt;span style=&quot; font-weight:400;&quot;&gt;The only parameter for the IDW interpolation method is the coefficient that describes the decrease of weights with distance.&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Distance coefficient P:</source>
        <translation type="unfinished">Abstandskoeffizient P:</translation>
    </message>
</context>
<context>
    <name>QgsIdentifyResults</name>
    <message>
        <source>Identify Results - </source>
        <translation>Anfrageergebnisse - </translation>
    </message>
    <message>
        <source>Run action</source>
        <translation>Aktion starten</translation>
    </message>
    <message>
        <source>(Derived)</source>
        <translation>(abgeleitet)</translation>
    </message>
    <message>
        <source>Feature</source>
        <translation>Objekt</translation>
    </message>
    <message>
        <source>Value</source>
        <translation>Wert</translation>
    </message>
</context>
<context>
    <name>QgsIdentifyResultsBase</name>
    <message>
        <source>Identify Results</source>
        <translation>Identifikationsergebnis</translation>
    </message>
    <message>
        <source>Help</source>
        <translation>Hilfe</translation>
    </message>
    <message>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Schließen</translation>
    </message>
</context>
<context>
    <name>QgsInterpolationDialog</name>
    <message>
        <source>Triangular interpolation (TIN)</source>
        <translation type="unfinished">Unregelmäßiges Dreiecksnetz (TIN)</translation>
    </message>
    <message>
        <source>Inverse Distance Weighting (IDW)</source>
        <translation type="unfinished">Inverse Distanzwichtung (IDW)</translation>
    </message>
</context>
<context>
    <name>QgsInterpolationDialogBase</name>
    <message>
        <source>Interpolation plugin</source>
        <translation type="unfinished">Interpolationsplugin</translation>
    </message>
    <message>
        <source>Input</source>
        <translation type="unfinished">Eingabe</translation>
    </message>
    <message>
        <source>Input vector layer</source>
        <translation type="unfinished">Eingabevektorlayer</translation>
    </message>
    <message>
        <source>Use z-Coordinate for interpolation</source>
        <translation type="unfinished">Z-Koordinate für Interpolation verwenden</translation>
    </message>
    <message>
        <source>Interpolation attribute </source>
        <translation type="unfinished">Interpolationsattribut</translation>
    </message>
    <message>
        <source>Output</source>
        <translation type="unfinished">Ergebnis</translation>
    </message>
    <message>
        <source>Interpolation method</source>
        <translation type="unfinished">Interpolationsmethode</translation>
    </message>
    <message>
        <source>...</source>
        <translation type="unfinished">...</translation>
    </message>
    <message>
        <source>Number of columns</source>
        <translation type="unfinished">Spaltenanzahl</translation>
    </message>
    <message>
        <source>Number of rows</source>
        <translation type="unfinished">Zeilenanzahl</translation>
    </message>
    <message>
        <source>Output file </source>
        <translation type="unfinished">Ausgabedatei </translation>
    </message>
</context>
<context>
    <name>QgsInterpolationPlugin</name>
    <message>
        <source>&amp;Interpolation</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsLUDialogBase</name>
    <message>
        <source>Enter class bounds</source>
        <translation>Gib die Klassengrenzen ein</translation>
    </message>
    <message>
        <source>Lower value</source>
        <translation>Untere Grenze</translation>
    </message>
    <message>
        <source>-</source>
        <translation>-</translation>
    </message>
    <message>
        <source>Upper value</source>
        <translation>Obere Grenze</translation>
    </message>
</context>
<context>
    <name>QgsLabelDialogBase</name>
    <message>
        <source>Form1</source>
        <translation>Formular1</translation>
    </message>
    <message>
        <source>Preview:</source>
        <translation>Vorschau:</translation>
    </message>
    <message>
        <source>QGIS Rocks!</source>
        <translation>QGIS bringt&apos;s!</translation>
    </message>
    <message>
        <source>Font</source>
        <translation>Schrift</translation>
    </message>
    <message>
        <source>Points</source>
        <translation>Punkte</translation>
    </message>
    <message>
        <source>Map units</source>
        <translation>Karteneinheiten</translation>
    </message>
    <message>
        <source>%</source>
        <translation>%</translation>
    </message>
    <message>
        <source>Transparency:</source>
        <translation>Transparenz:</translation>
    </message>
    <message>
        <source>Position</source>
        <translation>Position</translation>
    </message>
    <message>
        <source>Size:</source>
        <translation>Grösse:</translation>
    </message>
    <message>
        <source>Size is in map units</source>
        <translation>Grösse in Karteneinheiten</translation>
    </message>
    <message>
        <source>Size is in points</source>
        <translation>Grösse in Punkten</translation>
    </message>
    <message>
        <source>Above</source>
        <translation>Oben</translation>
    </message>
    <message>
        <source>Over</source>
        <translation>Über</translation>
    </message>
    <message>
        <source>Left</source>
        <translation>Links</translation>
    </message>
    <message>
        <source>Below</source>
        <translation>Unten</translation>
    </message>
    <message>
        <source>Right</source>
        <translation>Rechts</translation>
    </message>
    <message>
        <source>Above Right</source>
        <translation>Oben rechts</translation>
    </message>
    <message>
        <source>Below Right</source>
        <translation>Unten rechts</translation>
    </message>
    <message>
        <source>Above Left</source>
        <translation>Oben links</translation>
    </message>
    <message>
        <source>Below Left</source>
        <translation>Unten links</translation>
    </message>
    <message>
        <source>Font size units</source>
        <translation>Schriftgröße</translation>
    </message>
    <message>
        <source>Placement</source>
        <translation>Platzierung</translation>
    </message>
    <message>
        <source>Buffer</source>
        <translation>Puffer</translation>
    </message>
    <message>
        <source>Buffer size units</source>
        <translation>Puffergrößeneinheiten</translation>
    </message>
    <message>
        <source>Offset units</source>
        <translation>Offset Einheiten</translation>
    </message>
    <message>
        <source>Field containing label</source>
        <translation type="unfinished">Beschreibungsfeld</translation>
    </message>
    <message>
        <source>Default label</source>
        <translation type="unfinished">Beschriftungsvorgabe</translation>
    </message>
    <message>
        <source>Data defined style</source>
        <translation type="unfinished">Datendefinierter Stil</translation>
    </message>
    <message>
        <source>Data defined alignment</source>
        <translation type="unfinished">Datendefinierte Ausrichtung</translation>
    </message>
    <message>
        <source>Data defined buffer</source>
        <translation type="unfinished">Datendefinierter Puffer</translation>
    </message>
    <message>
        <source>Data defined position</source>
        <translation type="unfinished">Datendefinierte Position</translation>
    </message>
    <message>
        <source>Font transparency</source>
        <translation type="unfinished">Schrifttransparenz</translation>
    </message>
    <message>
        <source>Color</source>
        <translation type="unfinished">Farbe</translation>
    </message>
    <message>
        <source>Angle (deg)</source>
        <translation type="unfinished">Winkel (Altgrad)</translation>
    </message>
    <message>
        <source>Buffer labels?</source>
        <translation type="unfinished">Beschriftungen freistellen</translation>
    </message>
    <message>
        <source>Buffer size</source>
        <translation type="unfinished">Puffergröße</translation>
    </message>
    <message>
        <source>Transparency</source>
        <translation type="unfinished">Transparenz</translation>
    </message>
    <message>
        <source>X Offset (pts)</source>
        <translation type="unfinished">X-Offset (Punkte)</translation>
    </message>
    <message>
        <source>Y Offset (pts)</source>
        <translation type="unfinished">Y-Offset (Punkte)</translation>
    </message>
    <message>
        <source>&amp;Font family</source>
        <translation type="unfinished">&amp;Schriftfamilie</translation>
    </message>
    <message>
        <source>&amp;Bold</source>
        <translation type="unfinished">&amp;Fett</translation>
    </message>
    <message>
        <source>&amp;Italic</source>
        <translation type="unfinished">&amp;Kursiv</translation>
    </message>
    <message>
        <source>&amp;Underline</source>
        <translation type="unfinished">&amp;Unterstrichen</translation>
    </message>
    <message>
        <source>&amp;Size</source>
        <translation type="unfinished">Größe</translation>
    </message>
    <message>
        <source>Size units</source>
        <translation type="unfinished">Größeneinheit</translation>
    </message>
    <message>
        <source>X Coordinate</source>
        <translation type="unfinished">X-Koordinate</translation>
    </message>
    <message>
        <source>Y Coordinate</source>
        <translation type="unfinished">Y-Koordinate</translation>
    </message>
    <message>
        <source>Multiline labels?</source>
        <translation type="unfinished">Mehrzeilige Beschriftungen?</translation>
    </message>
    <message encoding="UTF-8">
        <source>°</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsLegend</name>
    <message>
        <source>group</source>
        <translation>Gruppe</translation>
    </message>
    <message>
        <source>&amp;Remove</source>
        <translation>&amp;Entfernen</translation>
    </message>
    <message>
        <source>&amp;Make to toplevel item</source>
        <translation>Als Top-Level Objekt &amp;machen</translation>
    </message>
    <message>
        <source>Re&amp;name</source>
        <translation>Umbe&amp;nennen</translation>
    </message>
    <message>
        <source>&amp;Add group</source>
        <translation>Gruppe hinzufügen</translation>
    </message>
    <message>
        <source>&amp;Expand all</source>
        <translation>Alles Ausklapp&amp;en</translation>
    </message>
    <message>
        <source>&amp;Collapse all</source>
        <translation>Alles zusammenfalten</translation>
    </message>
    <message>
        <source>Show file groups</source>
        <translation>Zeige Dateigruppen</translation>
    </message>
    <message>
        <source>No Layer Selected</source>
        <translation>Keinen Layer ausgewählt</translation>
    </message>
    <message>
        <source>To open an attribute table, you must select a vector layer in the legend</source>
        <translation type="unfinished">Um eine Attributtabelle zu öffnen, müssen Sie einen Vektorlayer in der Legende auswählen</translation>
    </message>
</context>
<context>
    <name>QgsLegendLayer</name>
    <message>
        <source>&amp;Zoom to layer extent</source>
        <translation>Auf die Layerausdehnung &amp;zoomen.</translation>
    </message>
    <message>
        <source>&amp;Zoom to best scale (100%)</source>
        <translation type="unfinished">&amp;Auf besten Maßstab zoomen (100%)</translation>
    </message>
    <message>
        <source>&amp;Show in overview</source>
        <translation type="unfinished">&amp;In der Übersicht anzeigen</translation>
    </message>
    <message>
        <source>&amp;Remove</source>
        <translation type="unfinished">&amp;Entfernen</translation>
    </message>
    <message>
        <source>&amp;Open attribute table</source>
        <translation type="unfinished">&amp;Attributtabelle öffnen</translation>
    </message>
    <message>
        <source>Save as shapefile...</source>
        <translation type="unfinished">Als Shapefile abspeichern...</translation>
    </message>
    <message>
        <source>Save selection as shapefile...</source>
        <translation type="unfinished">Auswahl als Shapefile speichern...</translation>
    </message>
    <message>
        <source>&amp;Properties</source>
        <translation type="unfinished">&amp;Eigenschaften</translation>
    </message>
    <message>
        <source>More layers</source>
        <translation type="obsolete">Weitere Layer</translation>
    </message>
    <message>
        <source>This item contains more layer files. Displaying more layers in table is not supported.</source>
        <translation type="obsolete">Dieser Eintrag enthält weitere Layerdateien. Die Anzeige von mehreren Layer in einer Tabelle wird nicht unterstützt.</translation>
    </message>
    <message>
        <source>Multiple layers</source>
        <translation type="unfinished">Mehrere Layer</translation>
    </message>
    <message>
        <source>This item contains multiple layers. Displaying multiple layers in the table is not supported.</source>
        <translation type="unfinished">Dieses Element enthält mehrere Layer. Die gemeinsame Darstellung mehrerer Layer in einer Attributtabelle wird nicht unterstützt.</translation>
    </message>
</context>
<context>
    <name>QgsLegendLayerFile</name>
    <message>
        <source>Attribute table - </source>
        <translation type="obsolete">Attributtabelle - </translation>
    </message>
    <message>
        <source>Save layer as...</source>
        <translation>Layer speichern als...</translation>
    </message>
    <message>
        <source>Start editing failed</source>
        <translation type="obsolete">Beginnen der Editierung fehlgeschlagen.</translation>
    </message>
    <message>
        <source>Provider cannot be opened for editing</source>
        <translation type="obsolete">Der Provider kann nicht zum Editieren geöffnet werden.</translation>
    </message>
    <message>
        <source>Stop editing</source>
        <translation type="obsolete">Digitalisieren stoppen</translation>
    </message>
    <message>
        <source>Do you want to save the changes?</source>
        <translation type="obsolete">Sollen die Änderungen gespeichert werden?</translation>
    </message>
    <message>
        <source>Error</source>
        <translation type="unfinished">Fehler</translation>
    </message>
    <message>
        <source>Could not commit changes</source>
        <translation type="obsolete">Änderungen konnten nicht gespeichern werden.</translation>
    </message>
    <message>
        <source>Problems during roll back</source>
        <translation type="obsolete">Problem beim Rückgängigmachen (rollback).</translation>
    </message>
    <message>
        <source>Not a vector layer</source>
        <translation type="obsolete">Kein Vektorlayer</translation>
    </message>
    <message>
        <source>To open an attribute table, you must select a vector layer in the legend</source>
        <translation type="obsolete">Um eine Attributetabelle zu Öffnen müssen Sie in der Legende einen Layer auswählen.</translation>
    </message>
    <message>
        <source>Saving done</source>
        <translation type="unfinished">Speichern abgeschlossen</translation>
    </message>
    <message>
        <source>Export to Shapefile has been completed</source>
        <translation type="unfinished">Der Export in eine Shapedatei ist abgeschlossen.</translation>
    </message>
    <message>
        <source>Driver not found</source>
        <translation type="unfinished">Treiber nicht gefunden</translation>
    </message>
    <message>
        <source>ESRI Shapefile driver is not available</source>
        <translation type="unfinished">Der ESRI-Shapefile-Treiber ist nicht verfügbar</translation>
    </message>
    <message>
        <source>Error creating shapefile</source>
        <translation type="unfinished">Fehler beim Erzeugen der Shapedatei</translation>
    </message>
    <message>
        <source>The shapefile could not be created (</source>
        <translation type="unfinished">Das Shapefile konnte nicht erstellt werden (</translation>
    </message>
    <message>
        <source>Layer creation failed</source>
        <translation type="unfinished">Layererzeugung schlug fehl</translation>
    </message>
    <message>
        <source>&amp;Zoom to layer extent</source>
        <translation>Auf die Layerausdehnung &amp;zoomen.</translation>
    </message>
    <message>
        <source>&amp;Show in overview</source>
        <translation type="unfinished">&amp;In Übersicht anzeigen</translation>
    </message>
    <message>
        <source>&amp;Remove</source>
        <translation type="unfinished">&amp;Entfernen</translation>
    </message>
    <message>
        <source>&amp;Open attribute table</source>
        <translation type="unfinished">&amp;Attributtabelle öffnen</translation>
    </message>
    <message>
        <source>Save as shapefile...</source>
        <translation type="unfinished">Als Shapedatei speichern...</translation>
    </message>
    <message>
        <source>Save selection as shapefile...</source>
        <translation type="unfinished">Auswahl als Shapedatei speichern...</translation>
    </message>
    <message>
        <source>&amp;Properties</source>
        <translation type="unfinished">&amp;Eigenschaften</translation>
    </message>
    <message>
        <source>bad_alloc exception</source>
        <translation type="obsolete">Speicher-Fehler</translation>
    </message>
    <message>
        <source>Filling the attribute table has been stopped because there was no more virtual memory left</source>
        <translation type="obsolete">Das Auffüllen der Attributtabelle wurde beendet, da kein virtueller Speicher mehr zur Verfügung steht</translation>
    </message>
    <message>
        <source>Layer attribute table contains unsupported datatype(s)</source>
        <translation>Die Attributtabelle des Layers enthält nicht unterstützte Datentypen.</translation>
    </message>
    <message>
        <source>Select the coordinate reference system for the saved shapefile.</source>
        <translation type="unfinished">Koordinatenbezugssystem für die gespeicherte Shapedatei wählen</translation>
    </message>
    <message>
        <source>The data points will be transformed from the layer coordinate reference system.</source>
        <translation type="unfinished">Die Punkte werden von dem Koordinatensystem des Layers transformiert.</translation>
    </message>
</context>
<context>
    <name>QgsMapCanvas</name>
    <message>
        <source>Could not draw</source>
        <translation>Konnte nicht zeichnen</translation>
    </message>
    <message>
        <source>because</source>
        <translation>weil</translation>
    </message>
</context>
<context>
    <name>QgsMapLayer</name>
    <message>
        <source>%1 at line %2 column %3</source>
        <translation type="unfinished">%1 in Zeile %2, Spalte %3</translation>
    </message>
    <message>
        <source>User database could not be opened.</source>
        <translation type="unfinished">Benutzerdatenbank konnte nicht geöffnet werden.</translation>
    </message>
    <message>
        <source>The style table could not be created.</source>
        <translation type="unfinished">Die Stiltabelle konnte nicht angelegt werden.</translation>
    </message>
    <message>
        <source>The style %1 was saved to database</source>
        <translation type="unfinished">Der Stil %1 wurde in der Datenbank gespeichert.</translation>
    </message>
    <message>
        <source>The style %1 was updated in the database.</source>
        <translation type="unfinished">Der Stil %1 wurde in der Datenbank aktualisiert.</translation>
    </message>
    <message>
        <source>The style %1 could not be updated in the database.</source>
        <translation type="unfinished">Der Stil %1 konnte nicht in der Datenbank aktualisiert werden.</translation>
    </message>
    <message>
        <source>The style %1 could not be inserted into database.</source>
        <translation type="unfinished">Der Stil %1 konnte nicht in der Datenbank gespeichert werden.</translation>
    </message>
    <message>
        <source>style not found in database</source>
        <translation type="unfinished">Stil nicht in der Datenbank gefunden</translation>
    </message>
</context>
<context>
    <name>QgsMapToolIdentify</name>
    <message>
        <source>No features found</source>
        <translation type="obsolete">Keine Objekte gefunden</translation>
    </message>
    <message>
        <source>&lt;p&gt;No features were found within the search radius. Note that it is currently not possible to use the identify tool on unsaved features.&lt;/p&gt;</source>
        <translation type="obsolete">&lt;p&gt;Es wurden keine Objekte innerhalb des Suchradius gefunden. Beachten Sie, dass das Identifizierwerkzeug nicht mit noch nicht abgespeicherten Objekten funktioniert.&lt;/p&gt;</translation>
    </message>
    <message>
        <source>(clicked coordinate)</source>
        <translation>(Angeklickte Koordinate)</translation>
    </message>
    <message>
        <source>WMS identify result for %1
%2</source>
        <translation>WMS-Abfrageergebnis für %1
%2</translation>
    </message>
    <message>
        <source>- %1 features found</source>
        <comment>Identify results window title</comment>
        <translation type="obsolete">
        
        </translation>
    </message>
</context>
<context>
    <name>QgsMapToolSplitFeatures</name>
    <message>
        <source>Split error</source>
        <translation>Trennfehler</translation>
    </message>
    <message>
        <source>An error occured during feature splitting</source>
        <translation>Ein Fehler ist beim Objekttrennen aufgetreten</translation>
    </message>
    <message>
        <source>No feature split done</source>
        <translation type="unfinished">Keine Objekttrennung vorgenommen</translation>
    </message>
    <message>
        <source>If there are selected features, the split tool only applies to the selected ones. If you like to split all features under the split line, clear the selection</source>
        <translation type="unfinished">Wenn Objekte ausgewählt sind, wird die Objekttrennung nur auf diese angewendet. Um alle Objekte zu trennen, muß die Auswahl aufgehoben werden.</translation>
    </message>
</context>
<context>
    <name>QgsMapToolVertexEdit</name>
    <message>
        <source>Snap tolerance</source>
        <translation>Snappingtoleranz</translation>
    </message>
    <message>
        <source>Don&apos;t show this message again</source>
        <translation type="unfinished">Diese Nachricht nicht mehr anzeigen.</translation>
    </message>
    <message>
        <source>Could not snap segment.</source>
        <translation>Konnte Segment nicht schnappen.</translation>
    </message>
    <message>
        <source>Have you set the tolerance in Settings &gt; Project Properties &gt; General?</source>
        <translation>Haben Sie die Snappingtoleranz in Einstellungen &gt; Projekteinstellungen &gt; Allgemein eingestellt?</translation>
    </message>
</context>
<context>
    <name>QgsMapserverExport</name>
    <message>
        <source>Name for the map file</source>
        <translation type="unfinished">Name des Mapfile</translation>
    </message>
    <message>
        <source>Choose the QGIS project file</source>
        <translation type="unfinished">QGIS-Projektdatei wählen</translation>
    </message>
    <message>
        <source>Overwrite File?</source>
        <translation type="unfinished">Datei überschreiben?</translation>
    </message>
    <message>
        <source> exists. 
Do you want to overwrite it?</source>
        <translation type="unfinished"> existiert.
Wollen Sie sie überschreiben?</translation>
    </message>
    <message>
        <source>MapServer map files (*.map);;All files (*.*)</source>
        <comment>Filter list for selecting files from a dialog box</comment>
        <translation>MapServer map files (*.map);;Alle Dateien (*.*)</translation>
    </message>
    <message>
        <source>QGIS Project Files (*.qgs);;All files (*.*)</source>
        <comment>

Filter list for selecting files from a dialog box</comment>
        <translation type="obsolete">QGIS-Projektdatei (*.qgs);;Alle Dateien (*.*)</translation>
    </message>
    <message>
        <source> exists. 
Do you want to overwrite it?</source>
        <comment>

a filename is prepended to this text, and appears in a dialog box</comment>
        <translation type="obsolete"> vorhanden.
Wollen Sie sie überschreiben?</translation>
    </message>
    <message>
        <source>QGIS Project Files (*.qgs);;All files (*.*)</source>
        <comment>Filter list for selecting files from a dialog box</comment>
        <translation type="unfinished">QGIS-Projektdatei (*.qgs);;Alle Dateien (*.*)</translation>
    </message>
    <message>
        <source> exists. 
Do you want to overwrite it?</source>
        <comment>a fileName is prepended to this text, and appears in a dialog box</comment>
        <translation type="unfinished"> vorhanden.
Wollen Sie sie überschreiben?</translation>
    </message>
</context>
<context>
    <name>QgsMapserverExportBase</name>
    <message>
        <source>Export to Mapserver</source>
        <translation>Exportieren in MapServer</translation>
    </message>
    <message>
        <source>Map file</source>
        <translation>Kartendatei</translation>
    </message>
    <message>
        <source>Export LAYER information only</source>
        <translation>Nur die Layer-Informationen exportieren</translation>
    </message>
    <message>
        <source>Map</source>
        <translation>Karte</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Name</translation>
    </message>
    <message>
        <source>Height</source>
        <translation>Höhe</translation>
    </message>
    <message>
        <source>Width</source>
        <translation>Breite</translation>
    </message>
    <message>
        <source>dd</source>
        <translation>dd</translation>
    </message>
    <message>
        <source>feet</source>
        <translation>Fuß</translation>
    </message>
    <message>
        <source>meters</source>
        <translation>Meter</translation>
    </message>
    <message>
        <source>miles</source>
        <translation>Meilen</translation>
    </message>
    <message>
        <source>inches</source>
        <translation>Inch</translation>
    </message>
    <message>
        <source>kilometers</source>
        <translation>Kilometer</translation>
    </message>
    <message>
        <source>Units</source>
        <translation>Einheiten</translation>
    </message>
    <message>
        <source>Image type</source>
        <translation>Bildtyp</translation>
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
        <source>jpeg</source>
        <translation>jpeg</translation>
    </message>
    <message>
        <source>png</source>
        <translation>png</translation>
    </message>
    <message>
        <source>swf</source>
        <translation>swf</translation>
    </message>
    <message>
        <source>userdefined</source>
        <translation>benutzerdefiniert</translation>
    </message>
    <message>
        <source>wbmp</source>
        <translation>wbmp</translation>
    </message>
    <message>
        <source>MinScale</source>
        <translation>Minimalmassstab</translation>
    </message>
    <message>
        <source>MaxScale</source>
        <translation>Maximalmassstab</translation>
    </message>
    <message>
        <source>Prefix attached to map, scalebar and legend GIF filenames created using this MapFile. It should be kept short.</source>
        <translation>Präfix, der Name der GIF-Dateien für Karten, Maßstabsleiste und Legende, die mit diesem Mapfile erzeugt wurden.  Es sollte kurz gehalten werden.</translation>
    </message>
    <message>
        <source>Web Interface Definition</source>
        <translation>Web Interface Definition</translation>
    </message>
    <message>
        <source>Header</source>
        <translation>Kopfzeile</translation>
    </message>
    <message>
        <source>Footer</source>
        <translation>Fußzeile</translation>
    </message>
    <message>
        <source>Template</source>
        <translation>Vorlage</translation>
    </message>
    <message>
        <source>&amp;Help</source>
        <translation>&amp;Hilfe</translation>
    </message>
    <message>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <source>&amp;OK</source>
        <translation>&amp;OK</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Abbrechen</translation>
    </message>
    <message>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <source>Name for the map file to be created from the QGIS project file</source>
        <translation>Name des Mapfiles, das aus dem QGIS-Projekt erzeugt werden soll.</translation>
    </message>
    <message>
        <source>If checked, only the layer information will be processed</source>
        <translation>Wenn ausgewählt, werden nur die Layerinformationen verarbeitet.</translation>
    </message>
    <message>
        <source>Path to the MapServer template file</source>
        <translation>Pfad zur MapServer-Vorlage</translation>
    </message>
    <message>
        <source>Prefix attached to map, scalebar and legend GIF filenames created using this MapFile</source>
        <translation>Präfix, der Namen der GIF-Dateien für Karte, Massstabsleisten und Legenden, die mit diesem Mapfile erzeugt werden.</translation>
    </message>
    <message>
        <source>Full path to the QGIS project file to export to MapServer map format</source>
        <translation>Kompletter Pfad zur QGIS-Projekt-Datei, die im MapServer Map-Format exportiert werden soll.</translation>
    </message>
    <message>
        <source>QGIS project file</source>
        <translation>QGIS-Projektdatei</translation>
    </message>
    <message>
        <source>Browse...</source>
        <translation type="unfinished">Durchsuchen...</translation>
    </message>
    <message>
        <source>Save As...</source>
        <translation type="unfinished">Speichern unter...</translation>
    </message>
</context>
<context>
    <name>QgsMeasureBase</name>
    <message>
        <source>Measure</source>
        <translation>Messen</translation>
    </message>
    <message>
        <source>New</source>
        <translation>Neu</translation>
    </message>
    <message>
        <source>Help</source>
        <translation>Hilfe</translation>
    </message>
    <message>
        <source>Cl&amp;ose</source>
        <translation>Schli&amp;eßen</translation>
    </message>
    <message>
        <source>Total:</source>
        <translation>Summe:</translation>
    </message>
    <message>
        <source>Segments</source>
        <translation type="unfinished">Segmente</translation>
    </message>
</context>
<context>
    <name>QgsMeasureDialog</name>
    <message>
        <source>Segments (in meters)</source>
        <translation type="unfinished">Segmente (in Meter)</translation>
    </message>
    <message>
        <source>Segments (in feet)</source>
        <translation type="unfinished">Segmente (in Fuß)</translation>
    </message>
    <message>
        <source>Segments (in degrees)</source>
        <translation type="unfinished">Segmente (in Grad)</translation>
    </message>
    <message>
        <source>Segments</source>
        <translation type="unfinished">Segmente</translation>
    </message>
</context>
<context>
    <name>QgsMeasureTool</name>
    <message>
        <source>Incorrect measure results</source>
        <translation type="unfinished">Falsche Messergebnisse</translation>
    </message>
    <message>
        <source>&lt;p&gt;This map is defined with a geographic coordinate system (latitude/longitude) but the map extents suggests that it is actually a projected coordinate system (e.g., Mercator). If so, the results from line or area measurements will be incorrect.&lt;/p&gt;&lt;p&gt;To fix this, explicitly set an appropriate map coordinate system using the &lt;tt&gt;Settings:Project Properties&lt;/tt&gt; menu.</source>
        <translation>&lt;p&gt;Diese Karte ist mit einem geographischen Koordinatensystem definiert (latitude/longitude) aber die Kartenausdehnung zeigt, dass es tatsächlich eine projiziertes Koordinatensystem ist (z.B.: Mercator). Wenn das stimmt, sind die Ergebnisse der Strecken oder Flächenmessung falsch.&lt;/p&gt;&lt;p&gt;Um richtig messen zu können, definieren Sie bitte ein entsprechendes Koordinatensystem in dem Menü &lt;tt&gt;Einstellungen:Projekteinstellungen&lt;/tt&gt;.</translation>
    </message>
</context>
<context>
    <name>QgsMessageViewer</name>
    <message>
        <source>QGIS Message</source>
        <translation>QGIS-Nachricht</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Schließen</translation>
    </message>
    <message>
        <source>Don&apos;t show this message again</source>
        <translation>Diese Nachricht nicht mehr anzeigen.</translation>
    </message>
</context>
<context>
    <name>QgsNewConnection</name>
    <message>
        <source>Test connection</source>
        <translation>Verbindung testen</translation>
    </message>
    <message>
        <source>Connection failed - Check settings and try again.

Extended error information:
</source>
        <translation>Verbindung fehlgeschlagen - Bitte Einstellungen überprüfen und erneut versuchen.

Ausführliche Fehlerinformation:</translation>
    </message>
    <message>
        <source>Connection to %1 was successful</source>
        <translation type="unfinished">Verbindung zu %1 war erfolgreich</translation>
    </message>
</context>
<context>
    <name>QgsNewConnectionBase</name>
    <message>
        <source>Create a New PostGIS connection</source>
        <translation>Neue PostGIS-Verbindung erzeugen</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Abbrechen</translation>
    </message>
    <message>
        <source>Help</source>
        <translation>Hilfe</translation>
    </message>
    <message>
        <source>Connection Information</source>
        <translation>Verbindunginformationen</translation>
    </message>
    <message>
        <source>Host</source>
        <translation>Host</translation>
    </message>
    <message>
        <source>Database</source>
        <translation>Datenbank</translation>
    </message>
    <message>
        <source>Username</source>
        <translation>Benutzername</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Name</translation>
    </message>
    <message>
        <source>Name of the new connection</source>
        <translation>Name der neuen Verbindung</translation>
    </message>
    <message>
        <source>Password</source>
        <translation>Passwort</translation>
    </message>
    <message>
        <source>Test Connect</source>
        <translation>Verbindung testen</translation>
    </message>
    <message>
        <source>Save Password</source>
        <translation>Passwort speichern</translation>
    </message>
    <message>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <source>Port</source>
        <translation>Port</translation>
    </message>
    <message>
        <source>5432</source>
        <translation>5432</translation>
    </message>
    <message>
        <source>Only look in the geometry_columns table</source>
        <translation>Nur in geometry_columns nachschauen.</translation>
    </message>
    <message>
        <source>Only look in the &apos;public&apos; schema</source>
        <translation>Nur im &apos;public&apos; Schema nachschauen.</translation>
    </message>
    <message>
        <source>Restrict the search to the public schema for spatial tables not in the geometry_columns table</source>
        <translation>Beschränke die Suche auf das public Schema für räumliche Tabellen nicht in der geometry_columns Tabelle</translation>
    </message>
    <message>
        <source>When searching for spatial tables that are not in the geometry_columns tables, restrict the search to tables that are in the public schema (for some databases this can save lots of time)</source>
        <translation>Bei der Suche nach räumlichen Tabellen, die nicht in der Tabelle geometry_columns sind, beschränke die Suche auf Tabellen, die in dem public Schema sind (bei einigen Datenbanken kann es eine Menge Zeit sparen)</translation>
    </message>
    <message>
        <source>Restrict the displayed tables to those that are in the geometry_columns table</source>
        <translation>Beschränke angezeigte Tabellen auf jene aus der Tabelle geometry_columns</translation>
    </message>
    <message>
        <source>Restricts the displayed tables to those that are in the geometry_columns table. This can speed up the initial display of spatial tables.</source>
        <translation>Beschränke dargestellte Tabellen auf jene aus der Tabelle geometry_columns. Dies kann die Anzeige räumlicher Tabellen beschleunigen.</translation>
    </message>
</context>
<context>
    <name>QgsNewHttpConnectionBase</name>
    <message>
        <source>Name</source>
        <translation>Name</translation>
    </message>
    <message>
        <source>URL</source>
        <translation>URL</translation>
    </message>
    <message>
        <source>Name of the new connection</source>
        <translation>Name der neuen Verbindung</translation>
    </message>
    <message>
        <source>HTTP address of the Web Map Server</source>
        <translation>HTTP-Adresse des WMS-Servers.</translation>
    </message>
    <message>
        <source>Create a new WMS connection</source>
        <translation type="unfinished">WMS-Verbindung anlegen</translation>
    </message>
    <message>
        <source>Connection details</source>
        <translation type="unfinished">Verbindungsdetails</translation>
    </message>
</context>
<context>
    <name>QgsNorthArrowPlugin</name>
    <message>
        <source>Bottom Left</source>
        <translation>Unten links</translation>
    </message>
    <message>
        <source>Top Right</source>
        <translation>Oben rechts</translation>
    </message>
    <message>
        <source>Bottom Right</source>
        <translation>Unten rechts</translation>
    </message>
    <message>
        <source>Top Left</source>
        <translation>Oben links</translation>
    </message>
    <message>
        <source>&amp;North Arrow</source>
        <translation>&amp;Nordpfeil</translation>
    </message>
    <message>
        <source>Creates a north arrow that is displayed on the map canvas</source>
        <translation>Erzeugt einen Nordpfeil und stellt ihn in der Karte dar.</translation>
    </message>
    <message>
        <source>&amp;Decorations</source>
        <translation>&amp;Dekorationen</translation>
    </message>
    <message>
        <source>North arrow pixmap not found</source>
        <translation>Nordpfeil nicht gefunden.</translation>
    </message>
</context>
<context>
    <name>QgsNorthArrowPluginGui</name>
    <message>
        <source>Pixmap not found</source>
        <translation>Bild nicht gefunden</translation>
    </message>
</context>
<context>
    <name>QgsNorthArrowPluginGuiBase</name>
    <message>
        <source>North Arrow Plugin</source>
        <translation>Nordpfeil Plugin</translation>
    </message>
    <message>
        <source>Properties</source>
        <translation>Eigenschaften</translation>
    </message>
    <message>
        <source>Angle</source>
        <translation>Winkel</translation>
    </message>
    <message>
        <source>Placement</source>
        <translation>Platzierung</translation>
    </message>
    <message>
        <source>Set direction automatically</source>
        <translation>Richtung automatisch setzen</translation>
    </message>
    <message>
        <source>Enable North Arrow</source>
        <translation>Nordpfeil aktivieren</translation>
    </message>
    <message>
        <source>Top Left</source>
        <translation>Oben links</translation>
    </message>
    <message>
        <source>Top Right</source>
        <translation>Oben rechts</translation>
    </message>
    <message>
        <source>Bottom Left</source>
        <translation>Unten links</translation>
    </message>
    <message>
        <source>Bottom Right</source>
        <translation>Unten rechts</translation>
    </message>
    <message>
        <source>Placement on screen</source>
        <translation>Platzierung am Bildschirm</translation>
    </message>
    <message>
        <source>Preview of north arrow</source>
        <translation>Vorschau des Nordpfeils</translation>
    </message>
    <message>
        <source>Icon</source>
        <translation>Icon</translation>
    </message>
    <message>
        <source>Browse...</source>
        <translation type="unfinished">Durchsuchen...</translation>
    </message>
</context>
<context>
    <name>QgsOptions</name>
    <message>
        <source>Detected active locale on your system: </source>
        <translation type="unfinished">Festgestellte Spracheinstellung des Systems: </translation>
    </message>
    <message>
        <source>to vertex</source>
        <translation type="unfinished">zum Stützpunkt</translation>
    </message>
    <message>
        <source>to segment</source>
        <translation type="unfinished">zum Segment</translation>
    </message>
    <message>
        <source>to vertex and segment</source>
        <translation type="unfinished">zum Stützpunkt und Segment</translation>
    </message>
    <message>
        <source>Semi transparent circle</source>
        <translation type="unfinished">Teiltransparenter Kreis</translation>
    </message>
    <message>
        <source>Cross</source>
        <translation type="unfinished">Kreuz</translation>
    </message>
    <message>
        <source>Show all features</source>
        <translation type="obsolete">Alle Objekte anzeigen</translation>
    </message>
    <message>
        <source>Show selected features</source>
        <translation type="obsolete">Nur selektierte Objekte anzeigen</translation>
    </message>
    <message>
        <source>Show features in current canvas</source>
        <translation type="obsolete">Objekte im aktuellen Kartenausschnitt anzeigen</translation>
    </message>
</context>
<context>
    <name>QgsOptionsBase</name>
    <message>
        <source>QGIS Options</source>
        <translation>QGIS-Optionen</translation>
    </message>
    <message>
        <source>Hide splash screen at startup</source>
        <translation>Splashscreen beim Start nicht anzeigen</translation>
    </message>
    <message>
        <source>&lt;b&gt;Note: &lt;/b&gt;Theme changes take effect the next time QGIS is started</source>
        <translation>&lt;b&gt;Beachte: &lt;/b&gt;Motivänderungen werden erst beim nächsten Start von QGIS aktiv</translation>
    </message>
    <message>
        <source>&amp;Rendering</source>
        <translation>&amp;Darstellung</translation>
    </message>
    <message>
        <source>Map display will be updated (drawn) after this many features have been read from the data source</source>
        <translation>Kartenanzeige wird erneuert (gezeichnet) nachdem soviele Objekte von der Datenquelle gelesen wurden</translation>
    </message>
    <message>
        <source>Select Global Default ...</source>
        <translation>Globale Voreinstellung wählen ...</translation>
    </message>
    <message>
        <source>Make lines appear less jagged at the expense of some drawing performance</source>
        <translation>Linien auf Kosten der Zeichengeschwindigkeit weniger gezackt zeichnen.</translation>
    </message>
    <message>
        <source>By default new la&amp;yers added to the map should be displayed</source>
        <translation>Standardmäßig werden alle neuen Layer im Kartenfenster angezeigt.</translation>
    </message>
    <message>
        <source>Measure tool</source>
        <translation>Messwerkzeug</translation>
    </message>
    <message>
        <source>Search radius</source>
        <translation>Suchradius</translation>
    </message>
    <message>
        <source>Pro&amp;jection</source>
        <translation type="obsolete">Pro&amp;jektion</translation>
    </message>
    <message>
        <source>When layer is loaded that has no projection information</source>
        <translation type="obsolete">Wenn ein Layer ohne Projektionsinformationen geladen wird</translation>
    </message>
    <message>
        <source>Fix problems with incorrectly filled polygons</source>
        <translation>Problem mit falsch gefüllten Polygonen beheben.</translation>
    </message>
    <message>
        <source>%</source>
        <translation>%</translation>
    </message>
    <message>
        <source>Continuously redraw the map when dragging the legend/map divider</source>
        <translation>Karte kontinuierlich neuzeichnen, wenn der Teiler zwischen Legende und Karte verschoben wird.</translation>
    </message>
    <message>
        <source>&amp;Map tools</source>
        <translation>&amp;Kartenwerkzeuge</translation>
    </message>
    <message>
        <source>Panning and zooming</source>
        <translation>Verschieben und Zoomen</translation>
    </message>
    <message>
        <source>Zoom</source>
        <translation>Zoom</translation>
    </message>
    <message>
        <source>Zoom and recenter</source>
        <translation>Zoomen und mittig zentrieren</translation>
    </message>
    <message>
        <source>Nothing</source>
        <translation>Nichts</translation>
    </message>
    <message>
        <source>&amp;General</source>
        <translation type="unfinished">&amp;Allgemein</translation>
    </message>
    <message>
        <source>Locale</source>
        <translation type="unfinished">Sprache</translation>
    </message>
    <message>
        <source>Locale to use instead</source>
        <translation type="unfinished">Stattdessen folgende Spracheinstellungen benutzen</translation>
    </message>
    <message>
        <source>Additional Info</source>
        <translation type="unfinished">Ergänzende Informationen</translation>
    </message>
    <message>
        <source>Detected active locale on your system:</source>
        <translation type="unfinished">Festgestellte aktive Spracheinstellung:</translation>
    </message>
    <message>
        <source>Digitizing</source>
        <translation type="unfinished">Digitalisierung</translation>
    </message>
    <message>
        <source>Rubberband</source>
        <translation>Gummiband</translation>
    </message>
    <message>
        <source>Line width in pixels</source>
        <translation type="unfinished">Linienbreite in Pixel</translation>
    </message>
    <message>
        <source>Snapping</source>
        <translation>Snapping</translation>
    </message>
    <message>
        <source>Selecting this will unselect the &apos;make lines less&apos; jagged toggle</source>
        <translation>Das Auswählen deaktiviert die Option Linien weniger gezackt zeichnen</translation>
    </message>
    <message>
        <source>Zoom to mouse cursor</source>
        <translation type="unfinished">Zur Mouseposition zoomen</translation>
    </message>
    <message>
        <source>Project files</source>
        <translation type="unfinished">Projektdateien</translation>
    </message>
    <message>
        <source>Prompt to save project changes when required</source>
        <translation type="unfinished">Bei Bedarf nachfragen, ob geänderte Projekte gespeichert werden sollen</translation>
    </message>
    <message>
        <source>Warn when opening a project file saved with an older version of QGIS</source>
        <translation type="unfinished">Warnung ausgeben, wenn QGIS-Projekt einer früheren Version geöffnet wird.</translation>
    </message>
    <message>
        <source>Default Map Appearance (overridden by project properties)</source>
        <translation type="unfinished">Voreingestelle Kartenaussehen (Projekteigenschaften überschreiben)</translation>
    </message>
    <message>
        <source>Selection color</source>
        <translation type="unfinished">Farbe für Auswahlen</translation>
    </message>
    <message>
        <source>Background color</source>
        <translation type="unfinished">Hintergrundfarbe</translation>
    </message>
    <message>
        <source>&amp;Application</source>
        <translation type="unfinished">&amp;Anwendung</translation>
    </message>
    <message>
        <source>Icon theme</source>
        <translation type="unfinished">Icon-Thema</translation>
    </message>
    <message>
        <source>Capitalise layer names in legend</source>
        <translation type="unfinished">Layernamen großschreiben</translation>
    </message>
    <message>
        <source>Display classification attribute names in legend</source>
        <translation type="unfinished">Klassifikationsattributnamen in der Legende anzeigen</translation>
    </message>
    <message>
        <source>Rendering behavior</source>
        <translation type="unfinished">Zeichenverhalten</translation>
    </message>
    <message>
        <source>Number of features to draw before updating the display</source>
        <translation type="unfinished">Anzahl von Objekten nach deren Zeichnung die Anzeige aktualisiert werden soll</translation>
    </message>
    <message>
        <source>&lt;b&gt;Note:&lt;/b&gt; Use zero to prevent display updates until all features have been rendered</source>
        <translation type="unfinished">&lt;b&gt;Note:&lt;/b&gt; 0 sorgt dafür, dass erst aktualisiert wird, wenn alle Objekte gezeichnet wurden</translation>
    </message>
    <message>
        <source>Rendering quality</source>
        <translation type="unfinished">Zeichenqualität</translation>
    </message>
    <message>
        <source>Zoom factor</source>
        <translation type="unfinished">Zoomfaktor</translation>
    </message>
    <message>
        <source>Mouse wheel action</source>
        <translation type="unfinished">Mausradaktion</translation>
    </message>
    <message>
        <source>Rubberband color</source>
        <translation type="unfinished">Gummibandfarbe</translation>
    </message>
    <message>
        <source>Ellipsoid for distance calculations</source>
        <translation type="unfinished">Ellipsoid für Abstandsberechnungen</translation>
    </message>
    <message>
        <source>&lt;b&gt;Note:&lt;/b&gt; Specify the search radius as a percentage of the map width</source>
        <translation type="unfinished">&lt;b&gt;Anmerkung:&lt;/b&gt; Suchradius in Prozent der Kartenbreite angeben</translation>
    </message>
    <message>
        <source>Search radius for identifying features and displaying map tips</source>
        <translation type="unfinished">Suchradius für die Objektidentifikation und zur Maptippanzeige</translation>
    </message>
    <message>
        <source>Line width</source>
        <translation type="unfinished">Linienbreite</translation>
    </message>
    <message>
        <source>Line colour</source>
        <translation type="unfinished">Linienfarbe</translation>
    </message>
    <message>
        <source>Default snap mode</source>
        <translation type="unfinished">Voreingestellter Fangmodus</translation>
    </message>
    <message>
        <source>Default snapping tolerance in layer units</source>
        <translation type="unfinished">Voreingestellte Fangtoleranz in Layereinheiten</translation>
    </message>
    <message>
        <source>Search radius for vertex edits in layer units</source>
        <translation type="unfinished">Suchradius für Knickpunktbearbeitung in Layereinheiten</translation>
    </message>
    <message>
        <source>Vertex markers</source>
        <translation type="unfinished">Knickpunktmarken</translation>
    </message>
    <message>
        <source>Marker style</source>
        <translation type="unfinished">Markenstil</translation>
    </message>
    <message>
        <source>Prompt for projection</source>
        <translation type="obsolete">Projektion abfragen</translation>
    </message>
    <message>
        <source>Project wide default projection will be used</source>
        <translation type="obsolete">Projektweite Projektionsvoreinstellung</translation>
    </message>
    <message>
        <source>Global default projection displa&amp;yed below will be used</source>
        <translation type="obsolete">Folgende globale Projektionsvoreinstellung wird benutzt</translation>
    </message>
    <message>
        <source>Override system locale</source>
        <translation type="unfinished">System-Locale überschreiben</translation>
    </message>
    <message>
        <source>&lt;b&gt;Note:&lt;/b&gt; Enabling / changing overide on local requires an application restart</source>
        <translation type="unfinished">&lt;b&gt;Note:&lt;/b&gt; Einschalten/Änderun der Locale-Überschreibung erfordert einen Anwendungsneustart</translation>
    </message>
    <message>
        <source>Proxy</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Use proxy for web access</source>
        <translation type="unfinished">Proxy für Webzugriff benutzen</translation>
    </message>
    <message>
        <source>Host</source>
        <translation type="unfinished">Host</translation>
    </message>
    <message>
        <source>Port</source>
        <translation type="unfinished">Port</translation>
    </message>
    <message>
        <source>User</source>
        <translation type="unfinished">Benutzer</translation>
    </message>
    <message>
        <source>Leave this blank if no proxy username / password are required</source>
        <translation type="unfinished">Lassen Sie Benutzer/Passwort leer, wenn sie nicht benötigt werden.</translation>
    </message>
    <message>
        <source>Password</source>
        <translation type="unfinished">Passwort</translation>
    </message>
    <message>
        <source>Open attribute table in a dock window</source>
        <translation type="unfinished">Attributtabelle gedockt öffnen</translation>
    </message>
    <message>
        <source>Attribute table behaviour</source>
        <translation type="obsolete">Verhalten der Attributtabelle</translation>
    </message>
    <message>
        <source>CRS</source>
        <translation type="unfinished">KBS</translation>
    </message>
    <message>
        <source>When layer is loaded that has no coordinate reference system (CRS)</source>
        <translation type="unfinished">Wenn ein Layer ohne Koordinatenbezugssystem (KBS) geladen wird</translation>
    </message>
    <message>
        <source>Prompt for CRS</source>
        <translation type="unfinished">KBS abfragen</translation>
    </message>
    <message>
        <source>Project wide default CRS will be used</source>
        <translation type="unfinished">Projektweite KBS-Voreinstellung</translation>
    </message>
    <message>
        <source>Global default CRS displa&amp;yed below will be used</source>
        <translation type="unfinished">Untenstehende globale Voreinstellung wird genutzt</translation>
    </message>
</context>
<context>
    <name>QgsPasteTransformationsBase</name>
    <message>
        <source>Paste Transformations</source>
        <translation>Transformationen einfügen</translation>
    </message>
    <message>
        <source>&lt;b&gt;Note: This function is not useful yet!&lt;/b&gt;</source>
        <translation>&lt;b&gt;Bemerkung: Diese Funktion ist noch nicht nützlich!&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Source</source>
        <translation>Quelle</translation>
    </message>
    <message>
        <source>Destination</source>
        <translation>Ziel</translation>
    </message>
    <message>
        <source>&amp;Help</source>
        <translation>&amp;Hilfe</translation>
    </message>
    <message>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <source>Add New Transfer</source>
        <translation>Neuen Transfer hinzufügen</translation>
    </message>
    <message>
        <source>&amp;OK</source>
        <translation>&amp;OK</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Abbrechen</translation>
    </message>
</context>
<context>
    <name>QgsPgGeoprocessing</name>
    <message>
        <source>Buffer features in layer %1</source>
        <translation>Puffer Objekte in Layer %1</translation>
    </message>
    <message>
        <source>Error connecting to the database</source>
        <translation>Fehler beim Verbinden mit der Datenbank</translation>
    </message>
    <message>
        <source>&amp;Buffer features</source>
        <translation>O&amp;bjekte puffern.</translation>
    </message>
    <message>
        <source>A new layer is created in the database with the buffered features.</source>
        <translation>Ein neuer Layer ist in der Datenbank erstellt wurden, der die gepufferten Objekte enthält.</translation>
    </message>
    <message>
        <source>&amp;Geoprocessing</source>
        <translation>&amp;Geodatenverarbeitung</translation>
    </message>
    <message>
        <source>Unable to add geometry column</source>
        <translation>Konnte die Geometriespalte nicht hinzufügen.</translation>
    </message>
    <message>
        <source>Unable to add geometry column to the output table </source>
        <translation>Geometriespalte konnte nicht zur Ausgabetabelle hinzufügen: </translation>
    </message>
    <message>
        <source>Unable to create table</source>
        <translation>Kann die Tabelle nicht erstellen.</translation>
    </message>
    <message>
        <source>Failed to create the output table </source>
        <translation>Erstellen der Ausgabetabelle fehlgeschlagen.</translation>
    </message>
    <message>
        <source>No GEOS support</source>
        <translation>Keine GEOS-Unterstützung.</translation>
    </message>
    <message>
        <source>Buffer function requires GEOS support in PostGIS</source>
        <translation>Pufferfunktion benötigt GEOS-Unterstützung in PostGIS.</translation>
    </message>
    <message>
        <source> is not a PostgreSQL/PostGIS layer.
</source>
        <translation> ist keine PostgreSQL/PostGIS-Layer.</translation>
    </message>
    <message>
        <source>Geoprocessing functions are only available for PostgreSQL/PostGIS Layers</source>
        <translation>Geodatenverarbeitungsfunktionen sind nur für PostgreSQL/PostGIS-Layer vorgesehen.</translation>
    </message>
    <message>
        <source>No Active Layer</source>
        <translation>Kein aktiver Layer</translation>
    </message>
    <message>
        <source>You must select a layer in the legend to buffer</source>
        <translation>Wählen Sie einen Layer in der Legende, der gepuffert werden soll.</translation>
    </message>
    <message>
        <source>Not a PostgreSQL/PostGIS Layer</source>
        <translation>Kein PostgreSQL/PostGIS Layer</translation>
    </message>
    <message>
        <source>Create a buffer for a PostgreSQL layer. </source>
        <translation type="unfinished">Puffer für einen PostGIS-Layer erzeugen</translation>
    </message>
</context>
<context>
    <name>QgsPgQueryBuilder</name>
    <message>
        <source>Table &lt;b&gt;%1&lt;/b&gt; in database &lt;b&gt;%2&lt;/b&gt; on host &lt;b&gt;%3&lt;/b&gt;, user &lt;b&gt;%4&lt;/b&gt;</source>
        <translation>Tabelle &lt;b&gt;%1&lt;/b&gt; in Datenbank &lt;b&gt;%2&lt;/b&gt; auf Host &lt;b&gt;%3&lt;/b&gt;, Benutzer &lt;b&gt;%4&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Connection Failed</source>
        <translation>Verbindung fehlgeschlagen</translation>
    </message>
    <message>
        <source>Connection to the database failed:</source>
        <translation>Verbindung zur Datenbank fehlgeschlagen:</translation>
    </message>
    <message>
        <source>Database error</source>
        <translation>Datenbankfehler</translation>
    </message>
    <message>
        <source>Query Result</source>
        <translation>Erfrage Resultat</translation>
    </message>
    <message>
        <source>The where clause returned </source>
        <translation>Die WHERE-Klausel gab </translation>
    </message>
    <message>
        <source> rows.</source>
        <translation> Zeilen zurück.</translation>
    </message>
    <message>
        <source>Query Failed</source>
        <translation>Abfrage fehlgeschlagen</translation>
    </message>
    <message>
        <source>An error occurred when executing the query:</source>
        <translation>Während der Ausführung der Abfrage trat ein Fehler auf:</translation>
    </message>
    <message>
        <source>No Records</source>
        <translation>Keine Einträge</translation>
    </message>
    <message>
        <source>The query you specified results in zero records being returned. Valid PostgreSQL layers must have at least one feature.</source>
        <translation>Die Abfrage ergab keine Einträge. Gültige PostgreSQL-Layer müssen mindestens ein Objekt enthalten.</translation>
    </message>
    <message>
        <source>&lt;p&gt;Failed to get sample of field values using SQL:&lt;/p&gt;&lt;p&gt;</source>
        <translation>&lt;p&gt;Konnte keine Beispiele der Werte mit SQL holen:&lt;/p&gt;&lt;p&gt;</translation>
    </message>
    <message>
        <source>No Query</source>
        <translation type="unfinished">Keine Abfrage</translation>
    </message>
    <message>
        <source>You must create a query before you can test it</source>
        <translation type="unfinished">Sie müssen eine Anfrage erstellen bevor Sie sie testen können.</translation>
    </message>
    <message>
        <source>Error in Query</source>
        <translation type="unfinished">Fehler in Abfrage</translation>
    </message>
</context>
<context>
    <name>QgsPgQueryBuilderBase</name>
    <message>
        <source>PostgreSQL Query Builder</source>
        <translation>PostgreSQL Query Builder</translation>
    </message>
    <message>
        <source>Clear</source>
        <translation>Löschen</translation>
    </message>
    <message>
        <source>Test</source>
        <translation>Testen</translation>
    </message>
    <message>
        <source>Ok</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Abbrechen</translation>
    </message>
    <message>
        <source>Values</source>
        <translation>Werte</translation>
    </message>
    <message>
        <source>All</source>
        <translation>Alle</translation>
    </message>
    <message>
        <source>Sample</source>
        <translation>Stichprobe</translation>
    </message>
    <message>
        <source>Fields</source>
        <translation>Felder</translation>
    </message>
    <message>
        <source>Operators</source>
        <translation>Operatoren</translation>
    </message>
    <message>
        <source>=</source>
        <translation>=</translation>
    </message>
    <message>
        <source>IN</source>
        <translation>IN</translation>
    </message>
    <message>
        <source>NOT IN</source>
        <translation>NOT IN</translation>
    </message>
    <message>
        <source>&lt;</source>
        <translation>&lt;</translation>
    </message>
    <message>
        <source>&gt;</source>
        <translation>&gt;</translation>
    </message>
    <message>
        <source>%</source>
        <translation>%</translation>
    </message>
    <message>
        <source>&lt;=</source>
        <translation>&lt;=</translation>
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
        <source>LIKE</source>
        <translation>ÄHNLICH</translation>
    </message>
    <message>
        <source>AND</source>
        <translation>UND</translation>
    </message>
    <message>
        <source>ILIKE</source>
        <translation>ILIKE</translation>
    </message>
    <message>
        <source>OR</source>
        <translation>ODER</translation>
    </message>
    <message>
        <source>NOT</source>
        <translation>NICHT</translation>
    </message>
    <message>
        <source>SQL where clause</source>
        <translation>SQL where clause</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Retrieve &lt;span style=&quot; font-weight:600;&quot;&gt;all&lt;/span&gt; the record in the vector file (&lt;span style=&quot; font-style:italic;&quot;&gt;if the table is big, the operation can consume some time&lt;/span&gt;)&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-weight:600;&quot;&gt;Alle&lt;/span&gt; Datensätze einer Vektordatei laden (&lt;span style=&quot; font-style:italic;&quot;&gt;wenn die Tabelle groß ist, kann das einige Zeit dauern.&lt;/span&gt;)&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Take a &lt;span style=&quot; font-weight:600;&quot;&gt;sample&lt;/span&gt; of records in the vector file&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Eine &lt;span style=&quot; font-weight:600;&quot;&gt;Stichprobe&lt;/span&gt; von Datensätze des Vektorlayers laden.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;List of values for the current field.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Liste der Werte für das aktuelle Feld.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;List of fields in this vector file&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Liste der Felder der aktuelle Vektordatei.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Datasource</source>
        <translation>Datenquelle</translation>
    </message>
</context>
<context>
    <name>QgsPluginInstallerDialog</name>
    <message>
        <source>QGIS Python Plugin Installer</source>
        <translation type="unfinished">QGIS-Python-Plugin-Installation</translation>
    </message>
    <message>
        <source>QGIS Plugin Installer</source>
        <translation type="obsolete">QGIS Plugin Installation</translation>
    </message>
    <message>
        <source>Plugins</source>
        <translation type="unfinished">Plugins</translation>
    </message>
    <message>
        <source>List of available and installed plugins</source>
        <translation type="unfinished">Liste der verfügbaren und installierten Plugins</translation>
    </message>
    <message>
        <source>Filter:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Display only plugins containing this word in their metadata</source>
        <translation type="unfinished">Nur Plugins anzeigen deren Metadaten dieses Wort enthalten</translation>
    </message>
    <message>
        <source>Display only plugins from given repository</source>
        <translation type="unfinished">Nur Plugins des angegebenen Repository anzeigen</translation>
    </message>
    <message>
        <source>all repositories</source>
        <translation type="unfinished">Alle Repositories</translation>
    </message>
    <message>
        <source>Display only plugins with matching status</source>
        <translation type="unfinished">Nur passende Plugins anzeigen</translation>
    </message>
    <message>
        <source>Status</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Name</source>
        <translation type="unfinished">Name</translation>
    </message>
    <message>
        <source>Version</source>
        <translation type="unfinished">Version</translation>
    </message>
    <message>
        <source>Description</source>
        <translation type="unfinished">Beschreibung</translation>
    </message>
    <message>
        <source>Author</source>
        <translation type="unfinished">Autor</translation>
    </message>
    <message>
        <source>Repository</source>
        <translation type="unfinished">Repository</translation>
    </message>
    <message>
        <source>Install, reinstall or upgrade the selected plugin</source>
        <translation type="unfinished">Gewähltes Plugin installieren, neu installieren oder aktualisieren</translation>
    </message>
    <message>
        <source>Install/upgrade plugin</source>
        <translation type="unfinished">Plugin installieren/aktualisieren</translation>
    </message>
    <message>
        <source>Uninstall the selected plugin</source>
        <translation type="unfinished">Das gewählte Plugin deinstallieren</translation>
    </message>
    <message>
        <source>Uninstall plugin</source>
        <translation type="unfinished">Gewähltes Plugin deinstallieren</translation>
    </message>
    <message>
        <source>Repositories</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>List of plugin repositories</source>
        <translation type="unfinished">Liste der Plugin-Repositories</translation>
    </message>
    <message>
        <source>URL</source>
        <translation type="unfinished">URL</translation>
    </message>
    <message>
        <source>Allow the Installer to look for updates and news in enabled repositories on QGIS startup</source>
        <translation type="unfinished">Dem Installer gestatten beim QGIS-Start nach Aktualisierungen und Neuigkeiten zu suchen</translation>
    </message>
    <message>
        <source>Check for updates on startup</source>
        <translation type="unfinished">Beim Start nach Aktualisierungen suchen</translation>
    </message>
    <message>
        <source>Add third party plugin repositories to the list</source>
        <translation type="unfinished">Plugin-Repositories von Dritten zur Liste hinzufügen</translation>
    </message>
    <message>
        <source>Add 3rd party repositories</source>
        <translation type="unfinished">Plugin-Repositories hinzufügen</translation>
    </message>
    <message>
        <source>Add a new plugin repository</source>
        <translation type="unfinished">Ein neues Plugin-Repository ergänzen</translation>
    </message>
    <message>
        <source>Add...</source>
        <translation type="unfinished">Hinzufügen...</translation>
    </message>
    <message>
        <source>Edit the selected repository</source>
        <translation type="unfinished">Gewähltes Respository bearbeiten</translation>
    </message>
    <message>
        <source>Edit...</source>
        <translation type="unfinished">Bearbeiten...</translation>
    </message>
    <message>
        <source>Remove the selected repository</source>
        <translation type="unfinished">Gewähltes Repository entfernen</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation type="unfinished">Löschen</translation>
    </message>
    <message>
        <source>The plugins will be installed to ~/.qgis/python/plugins</source>
        <translation type="unfinished">Das Plugin wird nach ~/.qgis/python/plugins installiert</translation>
    </message>
    <message>
        <source>Close the Installer window</source>
        <translation type="unfinished">Das Installationsfenster schließen</translation>
    </message>
    <message>
        <source>Close</source>
        <translation type="unfinished">Schließen</translation>
    </message>
</context>
<context>
    <name>QgsPluginInstallerFetchingDialog</name>
    <message>
        <source>Fetching repositories</source>
        <translation type="unfinished">Lade Repositories</translation>
    </message>
    <message>
        <source>Overall progress:</source>
        <translation type="unfinished">Gesamtfortschritt</translation>
    </message>
    <message>
        <source>Abort fetching</source>
        <translation type="unfinished">Ladevorgang abbrechen</translation>
    </message>
    <message>
        <source>Repository</source>
        <translation type="unfinished">Repository</translation>
    </message>
    <message>
        <source>State</source>
        <translation type="unfinished">Status</translation>
    </message>
</context>
<context>
    <name>QgsPluginInstallerInstallingDialog</name>
    <message>
        <source>QGIS Python Plugin Installer</source>
        <translation type="unfinished">QGIS-Python-Plugin-Installation</translation>
    </message>
    <message>
        <source>Installing plugin:</source>
        <translation type="unfinished">Plugin wird installiert:</translation>
    </message>
    <message>
        <source>Connecting...</source>
        <translation type="unfinished">Verbinde...</translation>
    </message>
</context>
<context>
    <name>QgsPluginInstallerPluginErrorDialog</name>
    <message>
        <source>Error loading plugin</source>
        <translation type="unfinished">Fehler beim Laden des Plugins</translation>
    </message>
    <message>
        <source>The plugin seems to be invalid or have unfulfilled dependencies. It has been installed, but can&apos;t be loaded. If you really need this plugin, you can contact its author or &lt;a href=&quot;http://lists.osgeo.org/mailman/listinfo/qgis-user&quot;&gt;QGIS users group&lt;/a&gt; and try to solve the problem. If not, you can just uninstall it. Here is the error message below:</source>
        <translation type="unfinished">Das Plugin scheint ungültig zu sein oder ihm fehlen Abhängigkeiten. Es wurde installiert, aber konnte nicht geladen werden. Wenn Sie das Plugin wirklich brauchen, kontaktieren Sie den Autor oder die &lt;a href=&quot;http://lists.osgeo.org/mailman/listinfo/qgis-user&quot;&gt;QGIS-Benutzergruppe&lt;/a&gt;, um das Problem zu lösen. Anderenfalls können Sie es einfach wieder deinstallieren.  Im folgenden die Fehlermeldung:</translation>
    </message>
    <message>
        <source>Do you want to uninstall this plugin now? If you&apos;re unsure, probably you would like to do this.</source>
        <translation type="unfinished">Wollen Sie das Plugin jetzt deinstallieren? Im Zweifelsfall sollten Sie dies wahrscheinlich tun.</translation>
    </message>
</context>
<context>
    <name>QgsPluginInstallerRepositoryDetailsDialog</name>
    <message>
        <source>Repository details</source>
        <translation type="unfinished">Repository Details</translation>
    </message>
    <message>
        <source>Name:</source>
        <translation type="unfinished">Name:</translation>
    </message>
    <message>
        <source>Enter a name for the repository</source>
        <translation type="unfinished">Name des Repositories eingeben</translation>
    </message>
    <message>
        <source>URL:</source>
        <translation type="unfinished">URL:</translation>
    </message>
    <message>
        <source>Enter the repository URL, beginning with &quot;http://&quot;</source>
        <translation type="unfinished">Repository-URL beginnend mit &quot;http://&quot; eingeben</translation>
    </message>
    <message>
        <source>Enable or disable the repository (disabled repositories will be omitted)</source>
        <translation type="unfinished">Das Repository ein- oder abschalten (abgeschaltete Repositories werden nicht angesprochen)</translation>
    </message>
    <message>
        <source>Enabled</source>
        <translation type="unfinished">Eingeschaltet</translation>
    </message>
    <message>
        <source>[place for a warning message]</source>
        <translation type="obsolete">[Eine Warnung hier]</translation>
    </message>
</context>
<context>
    <name>QgsPluginManager</name>
    <message>
        <source>No Plugins</source>
        <translation>Keine Plugins</translation>
    </message>
    <message>
        <source>No QGIS plugins found in </source>
        <translation>Keine QGIS-Plugins gefunden in </translation>
    </message>
    <message>
        <source>&amp;Select All</source>
        <translation type="unfinished">&amp;Alle selektieren</translation>
    </message>
    <message>
        <source>&amp;Clear All</source>
        <translation type="unfinished">Alle &amp;deselektieren</translation>
    </message>
</context>
<context>
    <name>QgsPluginManagerBase</name>
    <message>
        <source>QGIS Plugin Manager</source>
        <translation>QGIS Plugin Manager</translation>
    </message>
    <message>
        <source>To enable / disable a plugin, click its checkbox or description</source>
        <translation type="unfinished">Checkbox oder Beschreibung anklicken, um ein Plugin zu (de-)aktivieren</translation>
    </message>
    <message>
        <source>&amp;Filter</source>
        <translation>&amp;</translation>
    </message>
    <message>
        <source>Plugin Directory:</source>
        <translation type="unfinished">Plugin-Verzeichnis:</translation>
    </message>
    <message>
        <source>Directory</source>
        <translation type="unfinished">Verzeichnis</translation>
    </message>
</context>
<context>
    <name>QgsPointDialog</name>
    <message>
        <source>Zoom In</source>
        <translation>Hineinzoomen</translation>
    </message>
    <message>
        <source>z</source>
        <translation>z</translation>
    </message>
    <message>
        <source>Zoom Out</source>
        <translation>Hinauszoomen</translation>
    </message>
    <message>
        <source>Z</source>
        <translation>Z</translation>
    </message>
    <message>
        <source>Zoom To Layer</source>
        <translation>Auf den Layer zoomen</translation>
    </message>
    <message>
        <source>Zoom to Layer</source>
        <translation>Auf den Layer zoomen</translation>
    </message>
    <message>
        <source>Pan Map</source>
        <translation>Karte verschieben</translation>
    </message>
    <message>
        <source>Pan the map</source>
        <translation>Karte verschieben</translation>
    </message>
    <message>
        <source>Add Point</source>
        <translation>Addiere Punkt</translation>
    </message>
    <message>
        <source>.</source>
        <translation>.</translation>
    </message>
    <message>
        <source>Capture Points</source>
        <translation>Punkt digitalisieren</translation>
    </message>
    <message>
        <source>Delete Point</source>
        <translation>Lösche Punkt</translation>
    </message>
    <message>
        <source>Delete Selected</source>
        <translation>Ausgewahl gelöscht.</translation>
    </message>
    <message>
        <source>Linear</source>
        <translation>Linear</translation>
    </message>
    <message>
        <source>Helmert</source>
        <translation>Helmert</translation>
    </message>
    <message>
        <source>Choose a name for the world file</source>
        <translation>Bitte einen Namen für das World-File eingeben.</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Warnung</translation>
    </message>
    <message>
        <source>&lt;p&gt;A Helmert transform requires modifications in the raster layer.&lt;/p&gt;&lt;p&gt;The modified raster will be saved in a new file and a world file will be generated for this new file instead.&lt;/p&gt;&lt;p&gt;Are you sure that this is what you want?&lt;/p&gt;</source>
        <translation>&lt;p&gt;Eine Helmert-Transformation ändert den Rasterlayer.&lt;/p&gt;&lt;p&gt;Stattdessen wird die veränderte Rasterdatei in einer neuen Datei mit einem dazu passenden Worldfile gespeichert.&lt;/p&gt;&lt;p&gt;Sind Sie sicher, dass Sie das wollen?&lt;/p&gt;</translation>
    </message>
    <message>
        <source>Affine</source>
        <translation>Affin</translation>
    </message>
    <message>
        <source>Not implemented!</source>
        <translation>Nicht implementiert!</translation>
    </message>
    <message>
        <source>&lt;p&gt;An affine transform requires changing the original raster file. This is not yet supported.&lt;/p&gt;</source>
        <translation>&lt;p&gt;Eine Affin-Transformation wird die Original-Rasterdatei verändern. Dies ist noch nicht implementiert.&lt;/p&gt;</translation>
    </message>
    <message>
        <source>&lt;p&gt;The </source>
        <translation>&lt;p&gt;Die </translation>
    </message>
    <message>
        <source> transform is not yet supported.&lt;/p&gt;</source>
        <translation>Transformation wird noch nicht unterstützt.&lt;/p&gt;</translation>
    </message>
    <message>
        <source>Error</source>
        <translation>Fehler</translation>
    </message>
    <message>
        <source>Could not write to </source>
        <translation>Kann nicht </translation>
    </message>
    <message>
        <source>Currently all modified files will be written in TIFF format.</source>
        <translation>Derzeit werden alle modifizierten Dateien im TIFF-Format geschrieben.</translation>
    </message>
    <message>
        <source>-modified</source>
        <comment>

Georeferencer:QgsPointDialog.cpp - used to modify a user given filename</comment>
        <translation type="obsolete">-modifiziert</translation>
    </message>
    <message>
        <source>-modified</source>
        <comment>Georeferencer:QgsPointDialog.cpp - used to modify a user given file name</comment>
        <translation type="unfinished">-modifiziert</translation>
    </message>
</context>
<context>
    <name>QgsPointDialogBase</name>
    <message>
        <source>Transform type:</source>
        <translation>Transformationstyp:</translation>
    </message>
    <message>
        <source>Zoom in</source>
        <translation>Hineinzoomen</translation>
    </message>
    <message>
        <source>Zoom out</source>
        <translation>Herauszoomen</translation>
    </message>
    <message>
        <source>Zoom to the raster extents</source>
        <translation>Auf die Rasterausdehnung zoomen</translation>
    </message>
    <message>
        <source>Pan</source>
        <translation>Verschieben</translation>
    </message>
    <message>
        <source>Add points</source>
        <translation>Punkte hinzufügen</translation>
    </message>
    <message>
        <source>Delete points</source>
        <translation>Punkte löschen</translation>
    </message>
    <message>
        <source>World file:</source>
        <translation>World file:</translation>
    </message>
    <message>
        <source>...</source>
        <translation type="unfinished">...</translation>
    </message>
    <message>
        <source>Modified raster:</source>
        <translation>Raster modifizieren:</translation>
    </message>
    <message>
        <source>Reference points</source>
        <translation type="unfinished">Referenzpunkte</translation>
    </message>
    <message>
        <source>Create</source>
        <translation>Erstellen</translation>
    </message>
    <message>
        <source>Create and load layer</source>
        <translation>Erstellen und Layer laden</translation>
    </message>
</context>
<context>
    <name>QgsPostgresProvider</name>
    <message>
        <source>Unable to access relation</source>
        <translation>Auf die Relation kann nicht zugegriffen werden</translation>
    </message>
    <message>
        <source>Unable to access the </source>
        <translation>Fehler beim Zugriff auf die</translation>
    </message>
    <message>
        <source> relation.
The error message from the database was:
</source>
        <translation>Relation. Die Fehlermeldung der Datenbank war:</translation>
    </message>
    <message>
        <source>No GEOS Support!</source>
        <translation type="obsolete">Keine GEOS Unterstützung!</translation>
    </message>
    <message>
        <source>Your PostGIS installation has no GEOS support.
Feature selection and identification will not work properly.
Please install PostGIS with GEOS support (http://geos.refractions.net)</source>
        <translation type="obsolete">Diese PostGISinstallation hat keine GEOS-Unterstützung.
Objektselektion und -identifizierung kann nicht sauber funktionieren.
Bitte PostGIS mit GEOSunterstützung installieren (http://geos.refractions.net)</translation>
    </message>
    <message>
        <source>No suitable key column in table</source>
        <translation>Keine passende Schlüsselspalte in der Tabelle.</translation>
    </message>
    <message>
        <source>The table has no column suitable for use as a key.

Qgis requires that the table either has a column of type
int4 with a unique constraint on it (which includes the
primary key) or has a PostgreSQL oid column.
</source>
        <translation>Die Tabelle hat keine passende Spalte, die als Schlüssel verwendet werden kann. 

QGIS benötigt benötigt eine solche Spalte (Typ int4) mit einem eindeutigen Constraint (der dann einen eindeutigen Schlüssel beinhaltet. 
Alternativ kann die oid-Spalte von PostgresSQL benutzt werden.</translation>
    </message>
    <message>
        <source>The unique index on column</source>
        <translation>Der eindeutige Index der Spalte</translation>
    </message>
    <message>
        <source>is unsuitable because Qgis does not currently support non-int4 type columns as a key into the table.
</source>
        <translation>kann nicht benutzt werden, da QGIS derzeit nur Spalten vom Typ int4 als Schlüssel einer Tabelle akzeptiert.</translation>
    </message>
    <message>
        <source>and </source>
        <translation>und </translation>
    </message>
    <message>
        <source>The unique index based on columns </source>
        <translation>Der eindeutige Index basierend auf den Spalten </translation>
    </message>
    <message>
        <source> is unsuitable because Qgis does not currently support multiple columns as a key into the table.
</source>
        <translation> ist unbrauchbar, da QGIS derzeit nicht mehrere Spalten als Schlüssel in einer Tabelle unterstützt.</translation>
    </message>
    <message>
        <source>Unable to find a key column</source>
        <translation>Kann die Schlüsselspalte nicht finden.</translation>
    </message>
    <message>
        <source> derives from </source>
        <translation> kommt von </translation>
    </message>
    <message>
        <source>and is suitable.</source>
        <translation>und is passend.</translation>
    </message>
    <message>
        <source>and is not suitable </source>
        <translation>und ist nicht passend.</translation>
    </message>
    <message>
        <source>type is </source>
        <translation>Typ ist </translation>
    </message>
    <message>
        <source> and has a suitable constraint)</source>
        <translation> und hat einen passenden Constraint).</translation>
    </message>
    <message>
        <source> and does not have a suitable constraint)</source>
        <translation> und kat keinen passenden Constraint).</translation>
    </message>
    <message>
        <source>The view you selected has the following columns, none of which satisfy the above conditions:</source>
        <translation>Das ausgewählte View hat die folgenden Spalten; keine dieser Spalten erfüllt die obigen Konditionen:</translation>
    </message>
    <message>
        <source>Qgis requires that the view has a column that can be used as a unique key. Such a column should be derived from a table column of type int4 and be a primary key, have a unique constraint on it, or be a PostgreSQL oid column. To improve performance the column should also be indexed.
</source>
        <translation>QGIS benötigt bei Views eine Spalte, die als eindeutige Schlüsselspalte verwendet werden kann. So eine Spalte (meist vom Typ int4) sollte im Datensatz vorhanden sein und als primärer Schlüssel definiert sein. Ferner sollte ein eindeutiger Constraint definiert sein. Ideal ist die von PostgreSQL unterstützte Spalte oid. Um die Geschwindigkeit zu erhöhen, sollte die Spalte auch indiziert sein.</translation>
    </message>
    <message>
        <source>The view </source>
        <translation>Das View </translation>
    </message>
    <message>
        <source>has no column suitable for use as a unique key.
</source>
        <translation>hat keine Spalte, die sich als eindeutiger Schlüssel eignet.</translation>
    </message>
    <message>
        <source>No suitable key column in view</source>
        <translation>Keine passende Schlüsselspalte im View</translation>
    </message>
    <message>
        <source>Unknown geometry type</source>
        <translation>Unbekannter Geometrietyp.</translation>
    </message>
    <message>
        <source>Column </source>
        <translation>Spalte</translation>
    </message>
    <message>
        <source> in </source>
        <translation> in </translation>
    </message>
    <message>
        <source> has a geometry type of </source>
        <translation> hat einen Geometrietyp von </translation>
    </message>
    <message>
        <source>, which Qgis does not currently support.</source>
        <translation>, den QGIS derzeit nicht unterstützt.</translation>
    </message>
    <message>
        <source>. The database communication log was:
</source>
        <translation>. Die Datenbanklogdatei sagt folgendes:</translation>
    </message>
    <message>
        <source>Unable to get feature type and srid</source>
        <translation>Kann den Fearture-Typ und die SRID nicht ermitteln.</translation>
    </message>
    <message>
        <source>Note: </source>
        <translation>Bemerkung: </translation>
    </message>
    <message>
        <source>initially appeared suitable but does not contain unique data, so is not suitable.
</source>
        <translation>anfänglich schien der Layer geeignet, allerdings enthält er keine eindeutigen Daten, insofern nicht geeignet.</translation>
    </message>
    <message>
        <source>Unable to determine table access privileges for the </source>
        <translation type="unfinished">Konnte Tabellenzugriffrechte für die Tabelle </translation>
    </message>
    <message>
        <source>Error while adding features</source>
        <translation type="unfinished">Fehler beim Hinzufügen von Objekten</translation>
    </message>
    <message>
        <source>Error while deleting features</source>
        <translation type="unfinished">Fehler beim Löschen von Objekten</translation>
    </message>
    <message>
        <source>Error while adding attributes</source>
        <translation type="unfinished">Fehler beim Hinzufügen von Attributen</translation>
    </message>
    <message>
        <source>Error while deleting attributes</source>
        <translation type="unfinished">Fehler beim Löschen von Attributen</translation>
    </message>
    <message>
        <source>Error while changing attributes</source>
        <translation type="unfinished">Fehler beim Ändern von Attributen</translation>
    </message>
    <message>
        <source>Error while changing geometry values</source>
        <translation type="unfinished">Fehler beim Ändern von Geometrien</translation>
    </message>
    <message>
        <source>Qgis was unable to determine the type and srid of column </source>
        <translation type="unfinished">QGIS konnte Typ und SRID der Spalte </translation>
    </message>
    <message>
        <source>unexpected PostgreSQL error</source>
        <translation type="unfinished">Nicht erwarteter PostgeSQL-Fehler</translation>
    </message>
</context>
<context>
    <name>QgsPostgresProvider::Conn</name>
    <message>
        <source>No GEOS Support!</source>
        <translation type="unfinished">Keine GEOS Unterstützung!</translation>
    </message>
    <message>
        <source>Your PostGIS installation has no GEOS support.
Feature selection and identification will not work properly.
Please install PostGIS with GEOS support (http://geos.refractions.net)</source>
        <translation type="unfinished">Diese PostGISinstallation hat keine GEOS-Unterstützung.
Objektselektion und -identifizierung kann nicht sauber funktionieren.
Bitte PostGIS mit GEOSunterstützung installieren (http://geos.refractions.net)</translation>
    </message>
</context>
<context>
    <name>QgsProjectPropertiesBase</name>
    <message>
        <source>Project Properties</source>
        <translation>Projekteigenschaften</translation>
    </message>
    <message>
        <source>Meters</source>
        <translation>Meter</translation>
    </message>
    <message>
        <source>Feet</source>
        <translation>Fuss</translation>
    </message>
    <message>
        <source>Decimal degrees</source>
        <translation>Dezimal Grad</translation>
    </message>
    <message>
        <source>Default project title</source>
        <translation>Default Projekttitel</translation>
    </message>
    <message>
        <source>General</source>
        <translation>Allgemein</translation>
    </message>
    <message>
        <source>Automatic</source>
        <translation>Automatisch</translation>
    </message>
    <message>
        <source>Automatically sets the number of decimal places in the mouse position display</source>
        <translation>Setzt automatisch die Anzahl Dezimalstellen in der Mauspositionsanzeige</translation>
    </message>
    <message>
        <source>The number of decimal places that are used when displaying the mouse position is automatically set to be enough so that moving the mouse by one pixel gives a change in the position display</source>
        <translation>Die Anzahl Dezimalstellen, die beim Anzeigen der Mausposition benutzt werden, wird automatisch so gesetzt, dass eine Mausbewegung um einen Pixel zu einer Änderung in der Positionsanzeige führt</translation>
    </message>
    <message>
        <source>Manual</source>
        <translation>Hilfe</translation>
    </message>
    <message>
        <source>Sets the number of decimal places to use for the mouse position display</source>
        <translation>Setzt die Anzahl Dezimalstellen für die Mauspositionsanzeige</translation>
    </message>
    <message>
        <source>The number of decimal places for the manual option</source>
        <translation>Setzt die Anzahl Dezimalstellen für die manuelle Option</translation>
    </message>
    <message>
        <source>decimal places</source>
        <translation>Dezimalstellen</translation>
    </message>
    <message>
        <source>Precision</source>
        <translation>Genauigkeit</translation>
    </message>
    <message>
        <source>Digitizing</source>
        <translation>Digitalisierung</translation>
    </message>
    <message>
        <source>Descriptive project name</source>
        <translation>Beschreibender Projektname</translation>
    </message>
    <message>
        <source>Enable topological editing</source>
        <translation>Ermögliche topologisches Editieren</translation>
    </message>
    <message>
        <source>Snapping options...</source>
        <translation>Snapping-Optionen</translation>
    </message>
    <message>
        <source>Avoid intersections of new polygons</source>
        <translation>Vermeide Überschneidung neuer Polygone</translation>
    </message>
    <message>
        <source>Title and colors</source>
        <translation type="unfinished">Titel und Farben</translation>
    </message>
    <message>
        <source>Project title</source>
        <translation type="unfinished">Projekttitel</translation>
    </message>
    <message>
        <source>Selection color</source>
        <translation type="unfinished">Selektionsfarbe</translation>
    </message>
    <message>
        <source>Background color</source>
        <translation type="unfinished">Hintergrundfarbe</translation>
    </message>
    <message>
        <source>Map units</source>
        <translation type="unfinished">Karteneinheiten</translation>
    </message>
    <message>
        <source>Coordinate Reference System (CRS)</source>
        <translation type="unfinished">Benutzerkoordinatenreferenzsystem (KBS)</translation>
    </message>
    <message>
        <source>Enable &apos;on the fly&apos; CRS transformation</source>
        <translation type="unfinished">&apos;On-The-Fly&apos;-KBS-Transformation aktivieren</translation>
    </message>
</context>
<context>
    <name>QgsProjectionSelector</name>
    <message>
        <source>User Defined Coordinate Systems</source>
        <translation type="unfinished">Benutzerdefiniertes Koordinatensystem</translation>
    </message>
    <message>
        <source>Geographic Coordinate Systems</source>
        <translation type="unfinished">Geografisches Koordinatensystem</translation>
    </message>
    <message>
        <source>Projected Coordinate Systems</source>
        <translation type="unfinished">Projeziertes Koordinatensystem</translation>
    </message>
    <message>
        <source>Resource Location Error</source>
        <translation type="unfinished">Resource nicht gefunden</translation>
    </message>
    <message>
        <source>Error reading database file from: 
 %1
Because of this the projection selector will not work...</source>
        <translation type="unfinished">Fehler beim Lesen der Datenbankdatei aus:
 %1
Daher wird die Projektionsauswahl nicht funktionieren...</translation>
    </message>
</context>
<context>
    <name>QgsProjectionSelectorBase</name>
    <message>
        <source>Search</source>
        <translation>Suchen</translation>
    </message>
    <message>
        <source>Find</source>
        <translation>Finden</translation>
    </message>
    <message>
        <source>EPSG ID</source>
        <translation>EPSG ID</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Name</translation>
    </message>
    <message>
        <source>Coordinate Reference System Selector</source>
        <translation type="unfinished">Koordinatenbezugssystem-Auswahl</translation>
    </message>
    <message>
        <source>Coordinate Reference System</source>
        <translation type="unfinished">Koordinatensystem</translation>
    </message>
    <message>
        <source>EPSG</source>
        <translation type="unfinished">EPSG</translation>
    </message>
    <message>
        <source>ID</source>
        <translation type="unfinished">ID</translation>
    </message>
</context>
<context>
    <name>QgsPythonDialog</name>
    <message>
        <source>Python console</source>
        <translation type="unfinished">Python-Konsole</translation>
    </message>
    <message>
        <source>&gt;&gt;&gt;</source>
        <translation>&gt;&gt;&gt;</translation>
    </message>
    <message>
        <source>To access Quantum GIS environment from this python console use object from global scope which is an instance of QgisInterface class.&lt;br&gt;Usage e.g.: iface.zoomFull()</source>
        <translation>Um die Quantum GIS Umgebung von dieser Python Konsole aus zu erreichen, benutzen Sie Objekte des global scope, die eine Instanz der QgisInterface Klasse sind.&lt;br&gt;Benutzung z.B.: iface.zoomFull()</translation>
    </message>
</context>
<context>
    <name>QgsQuickPrint</name>
    <message>
        <source> km</source>
        <translation type="unfinished"> km</translation>
    </message>
    <message>
        <source> mm</source>
        <translation type="unfinished"> mm</translation>
    </message>
    <message>
        <source> cm</source>
        <translation type="unfinished"> cm</translation>
    </message>
    <message>
        <source> m</source>
        <translation type="unfinished"> m</translation>
    </message>
    <message>
        <source> miles</source>
        <translation type="unfinished"> Meilen</translation>
    </message>
    <message>
        <source> mile</source>
        <translation type="unfinished"> Meile</translation>
    </message>
    <message>
        <source> inches</source>
        <translation type="unfinished"> Inches</translation>
    </message>
    <message>
        <source> foot</source>
        <translation type="unfinished">Fuss</translation>
    </message>
    <message>
        <source> feet</source>
        <translation type="unfinished">Fuß</translation>
    </message>
    <message>
        <source> degree</source>
        <translation type="unfinished">Grad</translation>
    </message>
    <message>
        <source> degrees</source>
        <translation type="unfinished">Grad</translation>
    </message>
    <message>
        <source> unknown</source>
        <translation type="unfinished">unbekannt</translation>
    </message>
</context>
<context>
    <name>QgsRasterLayer</name>
    <message>
        <source>Not Set</source>
        <translation>Nicht gesetzt.</translation>
    </message>
    <message>
        <source>Driver:</source>
        <translation>Treiber:</translation>
    </message>
    <message>
        <source>Dimensions:</source>
        <translation>Dimensionen:</translation>
    </message>
    <message>
        <source>X: </source>
        <translation>X:</translation>
    </message>
    <message>
        <source> Y: </source>
        <translation>Y:</translation>
    </message>
    <message>
        <source> Bands: </source>
        <translation>Kanäle: </translation>
    </message>
    <message>
        <source>Origin:</source>
        <translation>Ursprung:</translation>
    </message>
    <message>
        <source>Pixel Size:</source>
        <translation>Pixelgröße:</translation>
    </message>
    <message>
        <source>Raster Extent: </source>
        <translation type="obsolete">Rasterausmaß:</translation>
    </message>
    <message>
        <source>Clipped area: </source>
        <translation type="obsolete">Ausschnittsbereich:</translation>
    </message>
    <message>
        <source>Pyramid overviews:</source>
        <translation>Pyramiden Überblicke:</translation>
    </message>
    <message>
        <source>Band</source>
        <translation>Kanal</translation>
    </message>
    <message>
        <source>Band No</source>
        <translation>Kanal Nr</translation>
    </message>
    <message>
        <source>No Stats</source>
        <translation>Keine Statistik</translation>
    </message>
    <message>
        <source>No stats collected yet</source>
        <translation>Noch keine Statistik gesammelt</translation>
    </message>
    <message>
        <source>Min Val</source>
        <translation>Minimalwert</translation>
    </message>
    <message>
        <source>Max Val</source>
        <translation>Maximalwert</translation>
    </message>
    <message>
        <source>Range</source>
        <translation>Bereich</translation>
    </message>
    <message>
        <source>Mean</source>
        <translation>Durchschnitt</translation>
    </message>
    <message>
        <source>Sum of squares</source>
        <translation>Summe der Quadrate</translation>
    </message>
    <message>
        <source>Standard Deviation</source>
        <translation>Standardverteilung</translation>
    </message>
    <message>
        <source>Sum of all cells</source>
        <translation>Summe aller Zellen</translation>
    </message>
    <message>
        <source>Cell Count</source>
        <translation>Zellenanzahl</translation>
    </message>
    <message>
        <source>Data Type:</source>
        <translation>Datentyp:</translation>
    </message>
    <message>
        <source>GDT_Byte - Eight bit unsigned integer</source>
        <translation>GDT_Byte - Eight bit unsigned integer</translation>
    </message>
    <message>
        <source>GDT_UInt16 - Sixteen bit unsigned integer </source>
        <translation>GDT_UInt16 - Sixteen bit unsigned integer</translation>
    </message>
    <message>
        <source>GDT_Int16 - Sixteen bit signed integer </source>
        <translation>GDT_Int16 - Sixteen bit signed integer</translation>
    </message>
    <message>
        <source>GDT_UInt32 - Thirty two bit unsigned integer </source>
        <translation>GDT_UInt32 - Thirty two bit unsigned integer</translation>
    </message>
    <message>
        <source>GDT_Int32 - Thirty two bit signed integer </source>
        <translation>GDT_Int32 - Thirty two bit signed integer</translation>
    </message>
    <message>
        <source>GDT_Float32 - Thirty two bit floating point </source>
        <translation>GDT_Float32 - Thirty two bit floating point</translation>
    </message>
    <message>
        <source>GDT_Float64 - Sixty four bit floating point </source>
        <translation>GDT_Float64 - Sixty four bit floating point</translation>
    </message>
    <message>
        <source>GDT_CInt16 - Complex Int16 </source>
        <translation>GDT_CInt16 - Complex Int16</translation>
    </message>
    <message>
        <source>GDT_CInt32 - Complex Int32 </source>
        <translation>GDT_CInt32 - Complex Int32</translation>
    </message>
    <message>
        <source>GDT_CFloat32 - Complex Float32 </source>
        <translation>GDT_CFloat32 - Complex Float32</translation>
    </message>
    <message>
        <source>GDT_CFloat64 - Complex Float64 </source>
        <translation>GDT_CFloat64 - Complex Float64</translation>
    </message>
    <message>
        <source>Could not determine raster data type.</source>
        <translation>Konnte Rasterdatentyp nicht erkennen.</translation>
    </message>
    <message>
        <source>Average Magphase</source>
        <translation>Durchschnittliche Magphase</translation>
    </message>
    <message>
        <source>Average</source>
        <translation>Durchschnitt</translation>
    </message>
    <message>
        <source>Layer Spatial Reference System: </source>
        <translation>Referenzsystem des Layers:</translation>
    </message>
    <message>
        <source>out of extent</source>
        <translation>ausserhalb der Ausdehnung</translation>
    </message>
    <message>
        <source>null (no data)</source>
        <translation>Null (keine Daten)</translation>
    </message>
    <message>
        <source>Dataset Description</source>
        <translation>Datensatzbeschreibung</translation>
    </message>
    <message>
        <source>No Data Value</source>
        <translation>NODATA Wert</translation>
    </message>
    <message>
        <source>and all other files</source>
        <translation>und alle anderen Dateien</translation>
    </message>
    <message>
        <source>NoDataValue not set</source>
        <translation>NoDataValue nicht gesetzt</translation>
    </message>
    <message>
        <source>Band %1</source>
        <translation>Band %1</translation>
    </message>
</context>
<context>
    <name>QgsRasterLayerProperties</name>
    <message>
        <source>Grayscale</source>
        <translation>Graustufen</translation>
    </message>
    <message>
        <source>Pseudocolor</source>
        <translation>Pseudofarben</translation>
    </message>
    <message>
        <source>Freak Out</source>
        <translation>Ausgeflippt</translation>
    </message>
    <message>
        <source>Palette</source>
        <translation type="obsolete">Palette</translation>
    </message>
    <message>
        <source>Columns: </source>
        <translation>Spalten: </translation>
    </message>
    <message>
        <source>Rows: </source>
        <translation>Zeilen: </translation>
    </message>
    <message>
        <source>No-Data Value: </source>
        <translation>NODATA-Wert: </translation>
    </message>
    <message>
        <source>n/a</source>
        <translation>n/a</translation>
    </message>
    <message>
        <source>Write access denied</source>
        <translation type="unfinished">Schreibzugriff verboten</translation>
    </message>
    <message>
        <source>Write access denied. Adjust the file permissions and try again.

</source>
        <translation type="unfinished">Schreibzugriff verboten. Dateirechte ändern und erneut versuchen.
</translation>
    </message>
    <message>
        <source>Building pyramids failed.</source>
        <translation type="unfinished">Erstellung von Pyramiden fehlgeschlagen.</translation>
    </message>
    <message>
        <source>Building pyramid overviews is not supported on this type of raster.</source>
        <translation type="unfinished">Für diese Art von Raster können keine Pyramiden erstellt werden. </translation>
    </message>
    <message>
        <source>Custom Colormap</source>
        <translation type="obsolete">Individuelle Farbkarte</translation>
    </message>
    <message>
        <source>No Stretch</source>
        <translation>Kein Strecken</translation>
    </message>
    <message>
        <source>Stretch To MinMax</source>
        <translation>Strecke auf MinMax</translation>
    </message>
    <message>
        <source>Stretch And Clip To MinMax</source>
        <translation>Strecken und Zuschneiden auf MinMax</translation>
    </message>
    <message>
        <source>Clip To MinMax</source>
        <translation>Zuschneiden auf MinMax</translation>
    </message>
    <message>
        <source>Discrete</source>
        <translation>Diskret</translation>
    </message>
    <message>
        <source>Linearly</source>
        <translation type="obsolete">Linear</translation>
    </message>
    <message>
        <source>Equal interval</source>
        <translation>Gleiches Interval</translation>
    </message>
    <message>
        <source>Quantiles</source>
        <translation type="unfinished">Quantile</translation>
    </message>
    <message>
        <source>Description</source>
        <translation>Beschreibung</translation>
    </message>
    <message>
        <source>Large resolution raster layers can slow navigation in QGIS.</source>
        <translation>Hochaufgelöste Raster können das Navigieren in QGIS verlangsamen.</translation>
    </message>
    <message>
        <source>By creating lower resolution copies of the data (pyramids) performance can be considerably improved as QGIS selects the most suitable resolution to use depending on the level of zoom.</source>
        <translation>Durch das Erstellen geringer aufgelöster Kopien der Daten (Pyramiden), kann die Darstellung beschleunigt werden, da QGIS die optimale Auflösung entsprechend der gewählten Zoomeinstellung aussucht</translation>
    </message>
    <message>
        <source>You must have write access in the directory where the original data is stored to build pyramids.</source>
        <translation>Sie brauchen Schreibrecht in dem Ordner mit den Originaldaten, um Pyramiden zu erstellen.</translation>
    </message>
    <message>
        <source>Red</source>
        <translation>Rot</translation>
    </message>
    <message>
        <source>Green</source>
        <translation>Grün</translation>
    </message>
    <message>
        <source>Blue</source>
        <translation>Blau</translation>
    </message>
    <message>
        <source>Percent Transparent</source>
        <translation>Prozent Transparenz</translation>
    </message>
    <message>
        <source>Gray</source>
        <translation type="unfinished">Grau</translation>
    </message>
    <message>
        <source>Indexed Value</source>
        <translation>Indizierter Wert</translation>
    </message>
    <message>
        <source>User Defined</source>
        <translation>Benutzerdefiniert</translation>
    </message>
    <message>
        <source>No-Data Value: Not Set</source>
        <translation>No-Data Wert: Nicht gesetzt</translation>
    </message>
    <message>
        <source>Save file</source>
        <translation>Datei speichern</translation>
    </message>
    <message>
        <source>Textfile (*.txt)</source>
        <translation>Textdatei (*.txt)</translation>
    </message>
    <message>
        <source>QGIS Generated Transparent Pixel Value Export File</source>
        <translation type="unfinished">QGIS-erzeugte Export-Datei für transparente Pixelwerte</translation>
    </message>
    <message>
        <source>Open file</source>
        <translation>Datei öffnen</translation>
    </message>
    <message>
        <source>Import Error</source>
        <translation>Importfehler</translation>
    </message>
    <message>
        <source>The following lines contained errors

</source>
        <translation>Die folgenden Zeilen enthalten Fehler</translation>
    </message>
    <message>
        <source>Read access denied</source>
        <translation>Lesezugriff verweigert</translation>
    </message>
    <message>
        <source>Read access denied. Adjust the file permissions and try again.

</source>
        <translation>Lesezugriff verweigert. Passe Dateirechte an und versuche es erneut.</translation>
    </message>
    <message>
        <source>Color Ramp</source>
        <translation>Farbanstieg</translation>
    </message>
    <message>
        <source>Not Set</source>
        <translation type="unfinished">Nicht gesetzt.</translation>
    </message>
    <message>
        <source>Default Style</source>
        <translation type="unfinished">Standardstil</translation>
    </message>
    <message>
        <source>QGIS Layer Style File (*.qml)</source>
        <translation type="unfinished">QGIS Layerstil Datei (*.qml)</translation>
    </message>
    <message>
        <source>QGIS</source>
        <translation>QGIS</translation>
    </message>
    <message>
        <source>Unknown style format: </source>
        <translation type="unfinished">Unbekanntes Stilformat: </translation>
    </message>
    <message>
        <source>Please note that building internal pyramids may alter the original data file and once created they cannot be removed!</source>
        <translation type="unfinished">Bitte beachten Sie, dass der Aufbau von internen Pyramiden die Originaldatei ändern kann und einmal angelegt nicht gelöscht werden kann.</translation>
    </message>
    <message>
        <source>Please note that building internal pyramids could corrupt your image - always make a backup of your data first!</source>
        <translation type="unfinished">Bitte beachten sie, dass der Aufbau von internen Pyramiden ihr Bild beschädigen kann - bitte sichern Sie Ihre Daten zuvor.</translation>
    </message>
    <message>
        <source>Default</source>
        <translation type="unfinished">Voreinstellung</translation>
    </message>
    <message>
        <source>The file was not writeable. Some formats do not support pyramid overviews. Consult the GDAL documentation if in doubt.</source>
        <translation type="unfinished">Die Datei war nicht beschreibbar. Einige Formate unterstützen Übersichtspyramiden nicht.  Gucken Sie im Zweifel in die GDAL-Dokumentation.</translation>
    </message>
    <message>
        <source>Saved Style</source>
        <translation type="unfinished">Gespeicherter Stil</translation>
    </message>
    <message>
        <source>Colormap</source>
        <translation type="unfinished">Farbkarte</translation>
    </message>
    <message>
        <source>Linear</source>
        <translation type="unfinished">Linear</translation>
    </message>
    <message>
        <source>Exact</source>
        <translation type="unfinished">Genau</translation>
    </message>
    <message>
        <source>Custom color map entry</source>
        <translation type="unfinished">Benutzerdefinierte Farbabbildungseintrag</translation>
    </message>
    <message>
        <source>QGIS Generated Color Map Export File</source>
        <translation type="unfinished">QGIS-Farbabbildungsexportdatei</translation>
    </message>
    <message>
        <source>Load Color Map</source>
        <translation type="unfinished">QGIS-Farbabbildung laden</translation>
    </message>
    <message>
        <source>The color map for Band %n failed to load</source>
        <translation type="obsolete">
        
        </translation>
    </message>
</context>
<context>
    <name>QgsRasterLayerPropertiesBase</name>
    <message>
        <source>Raster Layer Properties</source>
        <translation>Rasterlayereigenschaften</translation>
    </message>
    <message>
        <source>General</source>
        <translation>Allgemein</translation>
    </message>
    <message>
        <source>Legend:</source>
        <translation type="obsolete">Legende:</translation>
    </message>
    <message>
        <source>No Data:</source>
        <translation>Keine Daten:</translation>
    </message>
    <message>
        <source>Symbology</source>
        <translation>Bezeichnungen</translation>
    </message>
    <message>
        <source>&lt;p align=&quot;right&quot;&gt;Full&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;right&quot;&gt;Voll&lt;/p&gt;</translation>
    </message>
    <message>
        <source>None</source>
        <translation>Keine</translation>
    </message>
    <message>
        <source>Metadata</source>
        <translation>Metadaten</translation>
    </message>
    <message>
        <source>Pyramids</source>
        <translation>Pyramiden</translation>
    </message>
    <message>
        <source>Average</source>
        <translation>Durchschnitt</translation>
    </message>
    <message>
        <source>Nearest Neighbour</source>
        <translation>Nächster Nachbar</translation>
    </message>
    <message>
        <source>Thumbnail</source>
        <translation>Miniaturbild</translation>
    </message>
    <message>
        <source>Columns:</source>
        <translation>Spalten:</translation>
    </message>
    <message>
        <source>Rows:</source>
        <translation>Zeilen:</translation>
    </message>
    <message>
        <source>Palette:</source>
        <translation type="obsolete">Palette:</translation>
    </message>
    <message>
        <source>Maximum scale at which this layer will be displayed. </source>
        <translation>Maximum Massstab, bei dem dieser Layer angezeigt wird.</translation>
    </message>
    <message>
        <source>Minimum scale at which this layer will be displayed. </source>
        <translation>Minimum Massstab, bei dem dieser Layer angezeigt wird.</translation>
    </message>
    <message>
        <source>Histogram</source>
        <translation>Histogramm</translation>
    </message>
    <message>
        <source>Options</source>
        <translation>Optionen</translation>
    </message>
    <message>
        <source>Chart Type</source>
        <translation>Diagrammtyp</translation>
    </message>
    <message>
        <source>Refresh</source>
        <translation>Erneuern</translation>
    </message>
    <message>
        <source>Change</source>
        <translation type="obsolete">Wechseln</translation>
    </message>
    <message>
        <source>Max</source>
        <translation type="unfinished">Max</translation>
    </message>
    <message>
        <source>Min</source>
        <translation type="unfinished">Min</translation>
    </message>
    <message>
        <source> 00%</source>
        <translation>00%</translation>
    </message>
    <message>
        <source>Render as</source>
        <translation>Zeige als</translation>
    </message>
    <message>
        <source>...</source>
        <translation type="unfinished">...</translation>
    </message>
    <message>
        <source>Colormap</source>
        <translation>Farbkarte</translation>
    </message>
    <message>
        <source>Delete entry</source>
        <translation>Lösche Eintrag</translation>
    </message>
    <message>
        <source>Classify</source>
        <translation>Klassifiziere</translation>
    </message>
    <message>
        <source>1</source>
        <translation type="unfinished">1</translation>
    </message>
    <message>
        <source>2</source>
        <translation type="unfinished">2</translation>
    </message>
    <message>
        <source>Single band gray</source>
        <translation>Einfaches Grauband </translation>
    </message>
    <message>
        <source>Three band color</source>
        <translation>Dreibandfarbe</translation>
    </message>
    <message>
        <source>RGB mode band selection and scaling</source>
        <translation>RGB-Band-Auswahl und Skalierung</translation>
    </message>
    <message>
        <source>Red band</source>
        <translation>Rotes Band</translation>
    </message>
    <message>
        <source>Green band</source>
        <translation>Grünes Band</translation>
    </message>
    <message>
        <source>Blue band</source>
        <translation>Blaues Band</translation>
    </message>
    <message>
        <source>Custom min / max values</source>
        <translation type="unfinished">Benutzerextrema</translation>
    </message>
    <message>
        <source>Red min</source>
        <translation>Rot-Minimum</translation>
    </message>
    <message>
        <source>Red max</source>
        <translation>Rot-Maximum</translation>
    </message>
    <message>
        <source>Green min</source>
        <translation>Grün-Minimum</translation>
    </message>
    <message>
        <source>Green max</source>
        <translation>Grün-Maximum</translation>
    </message>
    <message>
        <source>Blue min</source>
        <translation>Blau-Minimum</translation>
    </message>
    <message>
        <source>Blue max</source>
        <translation>Blau-Maximum</translation>
    </message>
    <message>
        <source>Single band properties</source>
        <translation>Einfachbandeigenschaften</translation>
    </message>
    <message>
        <source>Gray band</source>
        <translation>Grauband</translation>
    </message>
    <message>
        <source>Color map</source>
        <translation>Farbabbildung</translation>
    </message>
    <message>
        <source>Invert color map</source>
        <translation>Farbabbildung invertieren</translation>
    </message>
    <message>
        <source>Use standard deviation</source>
        <translation type="unfinished">Standardabweichung nutzen</translation>
    </message>
    <message>
        <source>Load min / max values from band</source>
        <translation>Extrema des Bandes laden</translation>
    </message>
    <message>
        <source>Estimate (faster)</source>
        <translation>Schätzung (schneller) </translation>
    </message>
    <message>
        <source>Actual (slower)</source>
        <translation>Genau (langsamer)</translation>
    </message>
    <message>
        <source>Load</source>
        <translation>Laden</translation>
    </message>
    <message>
        <source>Contrast enhancement</source>
        <translation>Konstrastverbesserung</translation>
    </message>
    <message>
        <source>Current</source>
        <translation>Aktuell</translation>
    </message>
    <message>
        <source>Save current contrast enhancement algorithm as default. This setting will be persistent between QGIS sessions.</source>
        <translation>Aktuelle Konstrastverbessungsalgorithmus voreinstellen. Diese Einstellung bleibt über verschiedene QGIS-Sitzungen erhalten.</translation>
    </message>
    <message>
        <source>Saves current contrast enhancement algorithm as a default. This setting will be persistent between QGIS sessions.</source>
        <translation>Aktuelle Konstrastverbessungsalgorithmus voreinstellen. Diese Einstellung bleibt über verschiedene QGIS-Sitzungen erhalten.</translation>
    </message>
    <message>
        <source>Default</source>
        <translation>Voreinstellung</translation>
    </message>
    <message>
        <source>TextLabel</source>
        <translation>Textbeschriftung</translation>
    </message>
    <message>
        <source>Transparency</source>
        <translation>Transparenz</translation>
    </message>
    <message>
        <source>Global transparency</source>
        <translation>Globable Transparenz</translation>
    </message>
    <message>
        <source>No data value</source>
        <translation>&apos;Ohne Wert&apos;-Wert</translation>
    </message>
    <message>
        <source>Reset no data value</source>
        <translation>&apos;Ohne Wert&apos; zurücksetzen</translation>
    </message>
    <message>
        <source>Custom transparency options</source>
        <translation>Benutzertransparenzeinstellungen</translation>
    </message>
    <message>
        <source>Transparency band</source>
        <translation>Tranzparenzband</translation>
    </message>
    <message>
        <source>Transparent pixel list</source>
        <translation>Transparentspixelliste</translation>
    </message>
    <message>
        <source>Add values manually</source>
        <translation>Werte manuell ergänzen</translation>
    </message>
    <message>
        <source>Add Values from display</source>
        <translation>Werte aus Anzeige ergänzen</translation>
    </message>
    <message>
        <source>Remove selected row</source>
        <translation>Gewählte Zeile löschen</translation>
    </message>
    <message>
        <source>Default values</source>
        <translation>Wertvoreinstellungen</translation>
    </message>
    <message>
        <source>Import from file</source>
        <translation>Datei importieren</translation>
    </message>
    <message>
        <source>Export to file</source>
        <translation>Exportieren in Datei</translation>
    </message>
    <message>
        <source>Number of entries</source>
        <translation>Eintragsanzahl</translation>
    </message>
    <message>
        <source>Color interpolation</source>
        <translation>Farbinterpolation</translation>
    </message>
    <message>
        <source>Classification mode</source>
        <translation>Klassifikationsmodus</translation>
    </message>
    <message>
        <source>Spatial reference system</source>
        <translation type="obsolete">Räumliches Bezugssystem</translation>
    </message>
    <message>
        <source>Scale dependent visibility</source>
        <translation>Maßstabsabhänge Sichtbarkeit</translation>
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
        <translation type="obsolete">Debug-Info anzeigen</translation>
    </message>
    <message>
        <source>Layer source</source>
        <translation>Layerquelle</translation>
    </message>
    <message>
        <source>Display name</source>
        <translation type="unfinished">Anzeigename</translation>
    </message>
    <message>
        <source>Pyramid resolutions</source>
        <translation>Pyramidenauflösungen</translation>
    </message>
    <message>
        <source>Resampling method</source>
        <translation>Resampling-Methode</translation>
    </message>
    <message>
        <source>Build pyramids</source>
        <translation>Pyramiden erzeugen</translation>
    </message>
    <message>
        <source>Line graph</source>
        <translation>Kurvendiagram</translation>
    </message>
    <message>
        <source>Bar chart</source>
        <translation>Balkendiagram</translation>
    </message>
    <message>
        <source>Column count</source>
        <translation>Spaltenanzahl</translation>
    </message>
    <message>
        <source>Out of range OK?</source>
        <translation>Bereichsüberschreitung erlaubt</translation>
    </message>
    <message>
        <source>Allow approximation</source>
        <translation>Approximation erlauben</translation>
    </message>
    <message>
        <source>Restore Default Style</source>
        <translation type="unfinished">Standardstil wiederherstellen</translation>
    </message>
    <message>
        <source>Save As Default</source>
        <translation type="unfinished">Als Standard speichern</translation>
    </message>
    <message>
        <source>Load Style ...</source>
        <translation type="unfinished">Stil laden...</translation>
    </message>
    <message>
        <source>Save Style ...</source>
        <translation type="unfinished">Stil speichern...</translation>
    </message>
    <message>
        <source>Note:</source>
        <translation type="unfinished">Hinweis:</translation>
    </message>
    <message>
        <source>Default R:1 G:2 B:3</source>
        <translation type="unfinished">Vorgabe R:1 G:2 B:3</translation>
    </message>
    <message>
        <source>Save current band combination as default. This setting will be persistent between QGIS sessions.</source>
        <translation type="obsolete">Aktuelle Bandkombination als Vorgabe speichern. Diese Einstellung wird zwischen Sitzungen anhalten.</translation>
    </message>
    <message>
        <source>Save current band combination as a default. This setting will be persistent between QGIS sessions.</source>
        <translation type="obsolete">Aktuelle Bandkombination als eine Vorgabe speichern. Diese Einstellung wird zwischen Sitzungen anhalten.</translation>
    </message>
    <message>
        <source>Notes</source>
        <translation type="unfinished">Anmerkungen</translation>
    </message>
    <message>
        <source>Build pyramids internally if possible</source>
        <translation type="unfinished">Wenn möglich interne Pyramiden erzeugen</translation>
    </message>
    <message>
        <source>Add entry</source>
        <translation type="unfinished">Eintrag hinzufügen</translation>
    </message>
    <message>
        <source>Sort</source>
        <translation type="unfinished">Sortieren</translation>
    </message>
    <message>
        <source>Load color map from band</source>
        <translation type="unfinished">Farbabbildung aus Band laden</translation>
    </message>
    <message>
        <source>Load color map from file</source>
        <translation type="unfinished">Farbabbildung aus Datei laden</translation>
    </message>
    <message>
        <source>Export color map to file</source>
        <translation type="unfinished">Farbabbildung in Datei exportieren</translation>
    </message>
    <message>
        <source>Generate new color map</source>
        <translation type="unfinished">Neuen Farbabbildung generieren</translation>
    </message>
    <message>
        <source>Coordinate reference system</source>
        <translation type="unfinished">Koordinatenbezugssystem</translation>
    </message>
    <message>
        <source>Change ...</source>
        <translation type="unfinished">Ändern ...</translation>
    </message>
    <message>
        <source>Legend</source>
        <translation type="unfinished">Legende</translation>
    </message>
    <message>
        <source>Palette</source>
        <translation type="unfinished">Palette</translation>
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
        <source>Unable to run command</source>
        <translation>Kommando kann nicht gestartet werden</translation>
    </message>
    <message>
        <source>Starting</source>
        <translation>Starte</translation>
    </message>
    <message>
        <source>Done</source>
        <translation>Fertig</translation>
    </message>
    <message>
        <source>Action</source>
        <translation type="unfinished">Aktion</translation>
    </message>
</context>
<context>
    <name>QgsScaleBarPlugin</name>
    <message>
        <source> metres/km</source>
        <translation>Meter/Kilometer</translation>
    </message>
    <message>
        <source> feet</source>
        <translation>Fuß</translation>
    </message>
    <message>
        <source> degrees</source>
        <translation>Grad</translation>
    </message>
    <message>
        <source> km</source>
        <translation> km</translation>
    </message>
    <message>
        <source> mm</source>
        <translation> mm</translation>
    </message>
    <message>
        <source> cm</source>
        <translation> cm</translation>
    </message>
    <message>
        <source> m</source>
        <translation> m</translation>
    </message>
    <message>
        <source> foot</source>
        <translation>Fuss</translation>
    </message>
    <message>
        <source> degree</source>
        <translation>Grad</translation>
    </message>
    <message>
        <source> unknown</source>
        <translation>unbekannt</translation>
    </message>
    <message>
        <source>Top Left</source>
        <translation>Oben links</translation>
    </message>
    <message>
        <source>Bottom Left</source>
        <translation>Unten links</translation>
    </message>
    <message>
        <source>Top Right</source>
        <translation>Oben rechts</translation>
    </message>
    <message>
        <source>Bottom Right</source>
        <translation>Unten rechts</translation>
    </message>
    <message>
        <source>Tick Down</source>
        <translation>Strich unten</translation>
    </message>
    <message>
        <source>Tick Up</source>
        <translation>Strich oben</translation>
    </message>
    <message>
        <source>Bar</source>
        <translation>Balken</translation>
    </message>
    <message>
        <source>Box</source>
        <translation>Box</translation>
    </message>
    <message>
        <source>&amp;Scale Bar</source>
        <translation>&amp;Maßstab</translation>
    </message>
    <message>
        <source>Creates a scale bar that is displayed on the map canvas</source>
        <translation>Erzeugt eine Maßstabsleiste, die im Kartenbild angezeigt wird.</translation>
    </message>
    <message>
        <source>&amp;Decorations</source>
        <translation>&amp;Dekorationen</translation>
    </message>
    <message>
        <source> feet/miles</source>
        <translation> Fuß/Meilen</translation>
    </message>
    <message>
        <source> miles</source>
        <translation> Meilen</translation>
    </message>
    <message>
        <source> mile</source>
        <translation> Meile</translation>
    </message>
    <message>
        <source> inches</source>
        <translation> Inches</translation>
    </message>
</context>
<context>
    <name>QgsScaleBarPluginGuiBase</name>
    <message>
        <source>Scale Bar Plugin</source>
        <translation>Maßstabs Plugin</translation>
    </message>
    <message>
        <source>Top Left</source>
        <translation>Oben links</translation>
    </message>
    <message>
        <source>Top Right</source>
        <translation>Oben rechts</translation>
    </message>
    <message>
        <source>Bottom Left</source>
        <translation>Unten links</translation>
    </message>
    <message>
        <source>Bottom Right</source>
        <translation>Unten rechts</translation>
    </message>
    <message>
        <source>Size of bar:</source>
        <translation>Größe des Maßstab:</translation>
    </message>
    <message>
        <source>Placement:</source>
        <translation>Platzierung:</translation>
    </message>
    <message>
        <source>Tick Down</source>
        <translation>Strich unten</translation>
    </message>
    <message>
        <source>Tick Up</source>
        <translation>Strich oben</translation>
    </message>
    <message>
        <source>Box</source>
        <translation>Box</translation>
    </message>
    <message>
        <source>Bar</source>
        <translation>Balken</translation>
    </message>
    <message>
        <source>Select the style of the scale bar</source>
        <translation>Stil des Maßstab wählen</translation>
    </message>
    <message>
        <source>Colour of bar:</source>
        <translation>Farbe des Maßstab:</translation>
    </message>
    <message>
        <source>Scale bar style:</source>
        <translation>Maßstabsstil:</translation>
    </message>
    <message>
        <source>Enable scale bar</source>
        <translation>Aktiviere Maßstab</translation>
    </message>
    <message>
        <source>Automatically snap to round number on resize</source>
        <translation>bei Grässenänderung automatisch auf runden Zahlen einstellen</translation>
    </message>
    <message>
        <source>Click to select the colour</source>
        <translation>Klick, um die Farbe auszuwählen.</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;This plugin draws a scale bar on the map. Please note the size option below is a &apos;preferred&apos; size and may have to be altered by QGIS depending on the level of zoom.  The size is measured according to the map units specified in the project properties.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Dieses Plugin zeichnet eine Maßstabsleiste auf die Karte. Bitte beachten Sie, dass die Größenoption eine &apos;bevorzugte&apos; Größe ist, die durch QGIS zoomstufenabhängig variiert wird. Die Größe wird in Karteneinheiten aus den Projektinformationen errechnet.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
</context>
<context>
    <name>QgsSearchQueryBuilder</name>
    <message>
        <source>No matching features found.</source>
        <translation>Keine Treffer gefunden.</translation>
    </message>
    <message>
        <source>Search results</source>
        <translation>Suchergebnisse:</translation>
    </message>
    <message>
        <source>Search string parsing error</source>
        <translation>Fehler im Suchstring.</translation>
    </message>
    <message>
        <source>No Records</source>
        <translation>Keine Einträge</translation>
    </message>
    <message>
        <source>The query you specified results in zero records being returned.</source>
        <translation>Die definierte Abfrage gibt keine Treffer zurück.</translation>
    </message>
    <message>
        <source>Search query builder</source>
        <translation>Suche Query Builder</translation>
    </message>
    <message>
        <source>Found %d matching features.</source>
        <translation type="obsolete">
        
        </translation>
    </message>
</context>
<context>
    <name>QgsServerSourceSelect</name>
    <message>
        <source>Are you sure you want to remove the </source>
        <translation>Sind Sie sicher dass Sie die Verbindung und </translation>
    </message>
    <message>
        <source> connection and all associated settings?</source>
        <translation> alle damit verbunden Einstellungen löschen wollen?</translation>
    </message>
    <message>
        <source>Confirm Delete</source>
        <translation>Löschen Bestätigen</translation>
    </message>
    <message>
        <source>WMS Provider</source>
        <translation>WMS Anbinder</translation>
    </message>
    <message>
        <source>Could not open the WMS Provider</source>
        <translation>Kann den WMS Provider nicht öffnen.</translation>
    </message>
    <message>
        <source>Select Layer</source>
        <translation>Wähle Layer aus.</translation>
    </message>
    <message>
        <source>You must select at least one layer first.</source>
        <translation>Es muss mindestens ein Layer ausgewählt werden.</translation>
    </message>
    <message>
        <source>Could not understand the response.  The</source>
        <translation>Kann die Antwort nicht verstehen. Der</translation>
    </message>
    <message>
        <source>provider said</source>
        <translation>Provider sagte</translation>
    </message>
    <message>
        <source>WMS proxies</source>
        <translation>WMS-Proxies</translation>
    </message>
    <message>
        <source>Coordinate Reference System</source>
        <translation>Koordinatensystem</translation>
    </message>
    <message>
        <source>There are no available coordinate reference system for the set of layers you&apos;ve selected.</source>
        <translation>Es existiert kein Koordinatensystem für den ausgewählten Layer.</translation>
    </message>
    <message>
        <source>Several WMS servers have been added to the server list. Note that if you access the internet via a web proxy, you will need to set the proxy settings in the QGIS options dialog.</source>
        <translation type="unfinished">Verschiedene WMS-Server wurden der Serverliste hinzugefügt. Beachten Sie bitte, dass Sie ggf. noch die Proxyeinstellungen in den QGIS Optionen einstellen müssen.</translation>
    </message>
    <message>
        <source>Coordinate Reference System (%1 available)</source>
        <translation type="obsolete">
        
        </translation>
    </message>
</context>
<context>
    <name>QgsServerSourceSelectBase</name>
    <message>
        <source>Add Layer(s) from a Server</source>
        <translation>Layer von einem Server hinzufügen.</translation>
    </message>
    <message>
        <source>C&amp;lose</source>
        <translation>Schließen</translation>
    </message>
    <message>
        <source>Alt+L</source>
        <translation>Alt+L</translation>
    </message>
    <message>
        <source>Help</source>
        <translation>Hilfe</translation>
    </message>
    <message>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <source>Image encoding</source>
        <translation>Bildkodierung</translation>
    </message>
    <message>
        <source>Layers</source>
        <translation>Layer</translation>
    </message>
    <message>
        <source>ID</source>
        <translation>ID</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Name</translation>
    </message>
    <message>
        <source>Title</source>
        <translation>Titel</translation>
    </message>
    <message>
        <source>Abstract</source>
        <translation>Zusammenfassung</translation>
    </message>
    <message>
        <source>&amp;Add</source>
        <translation>&amp;Hinzufügen</translation>
    </message>
    <message>
        <source>Alt+A</source>
        <translation>Alt+A</translation>
    </message>
    <message>
        <source>Server Connections</source>
        <translation>Serververbindungen</translation>
    </message>
    <message>
        <source>&amp;New</source>
        <translation>&amp;Neu</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>Löschen</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation>Bearbeiten</translation>
    </message>
    <message>
        <source>C&amp;onnect</source>
        <translation>Verbinden</translation>
    </message>
    <message>
        <source>Ready</source>
        <translation>Fertig</translation>
    </message>
    <message>
        <source>Coordinate Reference System</source>
        <translation>Koordinatensystem</translation>
    </message>
    <message>
        <source>Change ...</source>
        <translation>Verändere ...</translation>
    </message>
    <message>
        <source>Adds a few example WMS servers</source>
        <translation>Fügt einige Beispiel-WMS-Server hinzu.</translation>
    </message>
    <message>
        <source>Add default servers</source>
        <translation>Standard-Server ergänzen</translation>
    </message>
</context>
<context>
    <name>QgsShapeFile</name>
    <message>
        <source>The database gave an error while executing this SQL:</source>
        <translation type="unfinished">Datenbankfehler während der Ausführung der SQL-Anweisung: </translation>
    </message>
    <message>
        <source>The error was:</source>
        <translation type="unfinished">Fehler war:</translation>
    </message>
    <message>
        <source>... (rest of SQL trimmed)</source>
        <comment>

is appended to a truncated SQL statement</comment>
        <translation type="obsolete">... (Rest der Anweisung abgeschnitten)</translation>
    </message>
    <message>
        <source>Scanning </source>
        <translation type="unfinished">Durchsuche </translation>
    </message>
    <message>
        <source>... (rest of SQL trimmed)</source>
        <comment>is appended to a truncated SQL statement</comment>
        <translation type="unfinished">... (Rest der Anweisung abgeschnitten)</translation>
    </message>
</context>
<context>
    <name>QgsSingleSymbolDialog</name>
    <message>
        <source>Solid Line</source>
        <translation>durchgängige Linie</translation>
    </message>
    <message>
        <source>Dash Line</source>
        <translation>gestrichelte Linie</translation>
    </message>
    <message>
        <source>Dot Line</source>
        <translation>gepunktete Linie</translation>
    </message>
    <message>
        <source>Dash Dot Line</source>
        <translation>gestrichelt-gepunktete Linie</translation>
    </message>
    <message>
        <source>Dash Dot Dot Line</source>
        <translation>gestrichelt-2mal-gepunktete Linie</translation>
    </message>
    <message>
        <source>No Pen</source>
        <translation>keine Linie</translation>
    </message>
    <message>
        <source>No Brush</source>
        <translation>Keine Füllung</translation>
    </message>
    <message>
        <source>Solid</source>
        <translation type="unfinished">Ausgefüllt</translation>
    </message>
    <message>
        <source>Horizontal</source>
        <translation type="unfinished">Horizontal</translation>
    </message>
    <message>
        <source>Vertical</source>
        <translation type="unfinished">Vertikal</translation>
    </message>
    <message>
        <source>Cross</source>
        <translation type="unfinished">Kreuz</translation>
    </message>
    <message>
        <source>BDiagonal</source>
        <translation>BDiagonal</translation>
    </message>
    <message>
        <source>FDiagonal</source>
        <translation>BDiagonal</translation>
    </message>
    <message>
        <source>Diagonal X</source>
        <translation>Diagonal X</translation>
    </message>
    <message>
        <source>Dense1</source>
        <translation>Dicht1</translation>
    </message>
    <message>
        <source>Dense2</source>
        <translation>Dicht2</translation>
    </message>
    <message>
        <source>Dense3</source>
        <translation>Dicht3</translation>
    </message>
    <message>
        <source>Dense4</source>
        <translation>Dicht4</translation>
    </message>
    <message>
        <source>Dense5</source>
        <translation>Dicht5</translation>
    </message>
    <message>
        <source>Dense6</source>
        <translation>Dicht6</translation>
    </message>
    <message>
        <source>Dense7</source>
        <translation>Dicht7</translation>
    </message>
    <message>
        <source>Texture</source>
        <translation>Textur</translation>
    </message>
</context>
<context>
    <name>QgsSingleSymbolDialogBase</name>
    <message>
        <source>Single Symbol</source>
        <translation>Einfaches Symbol</translation>
    </message>
    <message>
        <source>Size</source>
        <translation>Grösse</translation>
    </message>
    <message>
        <source>Point Symbol</source>
        <translation>Punktsymbol </translation>
    </message>
    <message>
        <source>Area scale field</source>
        <translation>Flächenmaßstabs Feld</translation>
    </message>
    <message>
        <source>Rotation field</source>
        <translation>Rotationsfeld</translation>
    </message>
    <message>
        <source>Style Options</source>
        <translation>Stiloption</translation>
    </message>
    <message>
        <source>...</source>
        <translation type="unfinished">...</translation>
    </message>
    <message>
        <source>Outline style</source>
        <translation>Umrandungsstil</translation>
    </message>
    <message>
        <source>Outline color</source>
        <translation>Umrandungsfarbe
</translation>
    </message>
    <message>
        <source>Outline width</source>
        <translation>Umrandungsbreite</translation>
    </message>
    <message>
        <source>Fill color</source>
        <translation>Füllfarbe</translation>
    </message>
    <message>
        <source>Fill style</source>
        <translation>Füllstil</translation>
    </message>
    <message>
        <source>Label</source>
        <translation type="unfinished">Beschriftung</translation>
    </message>
</context>
<context>
    <name>QgsSnappingDialog</name>
    <message>
        <source>to vertex</source>
        <translation>zum Stützpunkt</translation>
    </message>
    <message>
        <source>to segment</source>
        <translation>zum Segment</translation>
    </message>
    <message>
        <source>to vertex and segment</source>
        <translation>zum Stützpunkt und Segment</translation>
    </message>
</context>
<context>
    <name>QgsSnappingDialogBase</name>
    <message>
        <source>Snapping options</source>
        <translation>Snapping Optionen</translation>
    </message>
    <message>
        <source>Layer</source>
        <translation type="unfinished">Layer</translation>
    </message>
    <message>
        <source>Mode</source>
        <translation type="unfinished">Modus</translation>
    </message>
    <message>
        <source>Tolerance</source>
        <translation>Toleranz</translation>
    </message>
</context>
<context>
    <name>QgsSpit</name>
    <message>
        <source>Are you sure you want to remove the [</source>
        <translation>Soll die [</translation>
    </message>
    <message>
        <source>] connection and all associated settings?</source>
        <translation>] Verbindung und alle zugeordneten Einstellungen gelöscht werden?</translation>
    </message>
    <message>
        <source>Confirm Delete</source>
        <translation>Löschen Bestätigen</translation>
    </message>
    <message>
        <source>The following Shapefile(s) could not be loaded:

</source>
        <translation>Die folgenden Shapedateien konnten nicht geladen werden:</translation>
    </message>
    <message>
        <source>REASON: File cannot be opened</source>
        <translation>GRUND: Datei konnte nicht geöffnet werden</translation>
    </message>
    <message>
        <source>REASON: One or both of the Shapefile files (*.dbf, *.shx) missing</source>
        <translation>GRUND: Eine oder beide Shapedateien (*.dbf, *.shx) fehlen</translation>
    </message>
    <message>
        <source>General Interface Help:</source>
        <translation>Allgemeine Hilfe Schnittstelle:</translation>
    </message>
    <message>
        <source>PostgreSQL Connections:</source>
        <translation>PostgreSQL Verbindungen:</translation>
    </message>
    <message>
        <source>[New ...] - create a new connection</source>
        <translation>[Neu...] - Verbindung erstellen</translation>
    </message>
    <message>
        <source>[Edit ...] - edit the currently selected connection</source>
        <translation>[Bearbeiten ...] - die momentan gewählte Verbindung bearbeiten</translation>
    </message>
    <message>
        <source>[Remove] - remove the currently selected connection</source>
        <translation>[Entfernen] - momentan gewählte Verbindung löschen</translation>
    </message>
    <message>
        <source>-you need to select a connection that works (connects properly) in order to import files</source>
        <translation>- es muss eine Verbindung ausgewählt werden, die funktioniert (richtig verbindet) um Dateien zu importieren</translation>
    </message>
    <message>
        <source>-when changing connections Global Schema also changes accordingly</source>
        <translation>-bei Änderungen an den Verbindungen ändern die globalen Schemas dementsprechend </translation>
    </message>
    <message>
        <source>Shapefile List:</source>
        <translation>Shapedateienliste:</translation>
    </message>
    <message>
        <source>[Add ...] - open a File dialog and browse to the desired file(s) to import</source>
        <translation>[Hinzufügen ...] - Dateidialog öffnen und die gewünschten Importdateien auswählen</translation>
    </message>
    <message>
        <source>[Remove] - remove the currently selected file(s) from the list</source>
        <translation>[Entfernen] - löscht die ausgewählten Dateien von der Liste</translation>
    </message>
    <message>
        <source>[Remove All] - remove all the files in the list</source>
        <translation>[Alles entfernen]  - löscht alle Dateien in der Liste</translation>
    </message>
    <message>
        <source>[SRID] - Reference ID for the shapefiles to be imported</source>
        <translation>[SRID] - Referenz ID für die zu importierenden Shapedateien</translation>
    </message>
    <message>
        <source>[Use Default (SRID)] - set SRID to -1</source>
        <translation>[Standart (SRID) verwenden] - setzt SRID auf -1</translation>
    </message>
    <message>
        <source>[Geometry Column Name] - name of the geometry column in the database</source>
        <translation>[Geometriespaltenname] - Name der Geometriespalte in der Datenbank</translation>
    </message>
    <message>
        <source>[Use Default (Geometry Column Name)] - set column name to &apos;the_geom&apos;</source>
        <translation>[Standard (Geometriespaltenname)] - setzt Spaltenname auf &apos;the_geom&apos;</translation>
    </message>
    <message>
        <source>[Glogal Schema] - set the schema for all files to be imported into</source>
        <translation>[Globales Schema] setzt Schema für alle zu importierenden Dateien auf</translation>
    </message>
    <message>
        <source>[Import] - import the current shapefiles in the list</source>
        <translation>[Importieren] - Shapedateien in der Liste importieren</translation>
    </message>
    <message>
        <source>[Quit] - quit the program
</source>
        <translation>[Schließen] - das Programm verlassen</translation>
    </message>
    <message>
        <source>[Help] - display this help dialog</source>
        <translation>[Hilfe] - zeigt den Hilfedialog an</translation>
    </message>
    <message>
        <source>Import Shapefiles</source>
        <translation>Shapedateien importieren</translation>
    </message>
    <message>
        <source>You need to specify a Connection first</source>
        <translation>Es muss zuerst eine Verbindung angegeben werden</translation>
    </message>
    <message>
        <source>Connection failed - Check settings and try again</source>
        <translation>Verbindung fehlgeschlagen - Bitte Einstellungen überprüfen und erneut versuchen</translation>
    </message>
    <message>
        <source>You need to add shapefiles to the list first</source>
        <translation>Es müssen zuerst Shapedateien in die Liste eingefügt werden</translation>
    </message>
    <message>
        <source>Importing files</source>
        <translation>Dateien importieren</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Abbrechen</translation>
    </message>
    <message>
        <source>Progress</source>
        <translation>Fortschritt</translation>
    </message>
    <message>
        <source>Problem inserting features from file:</source>
        <translation>Problem beim Einfügen von Objekten aus der Datei:</translation>
    </message>
    <message>
        <source>Invalid table name.</source>
        <translation>Ungültiger Tabellenname.</translation>
    </message>
    <message>
        <source>No fields detected.</source>
        <translation>Keine Spalten erkannt.</translation>
    </message>
    <message>
        <source>The following fields are duplicates:</source>
        <translation>Die folgenden Spalten kommen doppelt vor:</translation>
    </message>
    <message>
        <source>Import Shapefiles - Relation Exists</source>
        <translation>Shapedatei importieren - Relation existiert</translation>
    </message>
    <message>
        <source>The Shapefile:</source>
        <translation>Die Shapedatei:</translation>
    </message>
    <message>
        <source>will use [</source>
        <translation>wird die Relation [</translation>
    </message>
    <message>
        <source>] relation for its data,</source>
        <translation>], die bereits vorhanden ist</translation>
    </message>
    <message>
        <source>which already exists and possibly contains data.</source>
        <translation>und evtl. Daten enthält, für ihre Daten benutzen.</translation>
    </message>
    <message>
        <source>To avoid data loss change the &quot;DB Relation Name&quot;</source>
        <translation>Um Datenverlust zu vermeiden sollte in der Dateiliste</translation>
    </message>
    <message>
        <source>for this Shapefile in the main dialog file list.</source>
        <translation>der &quot;DB-Relationsname&quot; geändert werden.</translation>
    </message>
    <message>
        <source>Do you want to overwrite the [</source>
        <translation>Soll die Relation [</translation>
    </message>
    <message>
        <source>] relation?</source>
        <translation>] überschrieben werden?</translation>
    </message>
    <message>
        <source>File Name</source>
        <translation>Dateiname</translation>
    </message>
    <message>
        <source>Feature Class</source>
        <translation>Objektklasse</translation>
    </message>
    <message>
        <source>Features</source>
        <translation>Objekte</translation>
    </message>
    <message>
        <source>DB Relation Name</source>
        <translation>DB-Relationsname</translation>
    </message>
    <message>
        <source>Schema</source>
        <translation>Schema</translation>
    </message>
    <message>
        <source>Add Shapefiles</source>
        <translation>Shapedateien hinzufügen</translation>
    </message>
    <message>
        <source>Shapefiles (*.shp);;All files (*.*)</source>
        <translation>Shapefiles (*.shp);; Alle Dateien (*.*)</translation>
    </message>
    <message>
        <source>PostGIS not available</source>
        <translation>PostGIS ist nicht verfügbar</translation>
    </message>
    <message>
        <source>&lt;p&gt;The chosen database does not have PostGIS installed, but this is required for storage of spatial data.&lt;/p&gt;</source>
        <translation>&lt;p&gt;In der gewählte Datenbank ist PostGIS nicht installiert. PostGIS wird jedoch zum Speichern von räumlichen Daten benötigt.&lt;/p&gt;</translation>
    </message>
    <message>
        <source>Checking to see if </source>
        <translation type="obsolete">Schaumermal...</translation>
    </message>
    <message>
        <source>&lt;p&gt;Error while executing the SQL:&lt;/p&gt;&lt;p&gt;</source>
        <translation>&lt;p&gt;Fehler beim Ausführen des SQL:&lt;/p&gt;&lt;p&gt;</translation>
    </message>
    <message>
        <source>&lt;/p&gt;&lt;p&gt;The database said:</source>
        <translation>&lt;/p&gt;&lt;p&gt;Die Datenbank meldete:</translation>
    </message>
    <message>
        <source>%1 of %2 shapefiles could not be imported.</source>
        <translation type="unfinished">%1 von %2 Shape-Dateien konnte nicht importiert werden.</translation>
    </message>
    <message>
        <source>Password for </source>
        <translation type="unfinished">Passwort für </translation>
    </message>
    <message>
        <source>Please enter your password:</source>
        <translation type="unfinished">Bitte Passwort eingeben:</translation>
    </message>
</context>
<context>
    <name>QgsSpitBase</name>
    <message>
        <source>SPIT - Shapefile to PostGIS Import Tool</source>
        <translation>SPIT - Shapefile in PostGIS Import Tool</translation>
    </message>
    <message>
        <source>PostgreSQL Connections</source>
        <translation>PostgreSQL-Verbindungen</translation>
    </message>
    <message>
        <source>Remove</source>
        <translation>Entfernen</translation>
    </message>
    <message>
        <source>Remove All</source>
        <translation>Alles entfernen</translation>
    </message>
    <message>
        <source>Global Schema</source>
        <translation>Globales Schema</translation>
    </message>
    <message>
        <source>Add</source>
        <translation>Hinzufügen</translation>
    </message>
    <message>
        <source>Add a shapefile to the list of files to be imported</source>
        <translation>Der Liste der zu importierenden Dateien eine Shapedatei hinzufügen</translation>
    </message>
    <message>
        <source>Remove the selected shapefile from the import list</source>
        <translation>Gewähltes Shapefile aus der Importliste entfernen</translation>
    </message>
    <message>
        <source>Remove all the shapefiles from the import list</source>
        <translation>Alle Shapefiles aus der Importliste entfernen</translation>
    </message>
    <message>
        <source>Set the SRID to the default value</source>
        <translation>SRID auf den Standardwert setzen</translation>
    </message>
    <message>
        <source>Set the geometry column name to the default value</source>
        <translation>Geometriespaltenname auf den Standardwert setzen</translation>
    </message>
    <message>
        <source>New</source>
        <translation>Neu</translation>
    </message>
    <message>
        <source>Create a new PostGIS connection</source>
        <translation>Neue PostGIS Verbindung erstellen</translation>
    </message>
    <message>
        <source>Remove the current PostGIS connection</source>
        <translation>Aktuelle PostGIS Verbindung entfernen</translation>
    </message>
    <message>
        <source>Connect</source>
        <translation type="unfinished">Verbinden</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation>Bearbeiten</translation>
    </message>
    <message>
        <source>Edit the current PostGIS connection</source>
        <translation>Aktuelle PostGIS-Verbindung bearbeiten</translation>
    </message>
    <message>
        <source>Import options and shapefile list</source>
        <translation type="unfinished">Importoptionen und Shape-Dateiliste</translation>
    </message>
    <message>
        <source>Use Default SRID or specify here</source>
        <translation type="unfinished">Eingestellte SRID nutzen</translation>
    </message>
    <message>
        <source>Use Default Geometry Column Name or specify here</source>
        <translation type="unfinished">Eingestellten Spaltennamen nutzen</translation>
    </message>
    <message>
        <source>Primary Key Column Name</source>
        <translation type="unfinished">Primärschlüsselspalte</translation>
    </message>
    <message>
        <source>Connect to PostGIS</source>
        <translation type="unfinished">Mit PostGIS verbinden</translation>
    </message>
</context>
<context>
    <name>QgsSpitPlugin</name>
    <message>
        <source>&amp;Import Shapefiles to PostgreSQL</source>
        <translation>Shapefiles in PostgreSQL &amp;importieren</translation>
    </message>
    <message>
        <source>Import shapefiles into a PostGIS-enabled PostgreSQL database. The schema and field names can be customized on import</source>
        <translation>Importiert Shapefiles in eine PostgreSQL-Datenbank mit PostGIS-Aufsatz. Schema und die Feldnamen des Imports sind einstellbar.</translation>
    </message>
    <message>
        <source>&amp;Spit</source>
        <translation>&amp;Spit</translation>
    </message>
</context>
<context>
    <name>QgsTINInterpolatorDialog</name>
    <message>
        <source>Linear interpolation</source>
        <translation type="unfinished">Lineare Interpolation</translation>
    </message>
</context>
<context>
    <name>QgsTINInterpolatorDialogBase</name>
    <message>
        <source>Triangle based interpolation</source>
        <translation type="unfinished">Dreiecksinterpolation</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:12pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;This interpolator provides different methods for interpolation in a triangular irregular network (TIN).&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:12pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Diese Interpolation stellt verschiedene Methoden zur Interpolation in unregelmäßigen Dreiecksnetzen (TIN) zur Verfügung.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Interpolation method:</source>
        <translation type="unfinished">Interpolationsmethode</translation>
    </message>
</context>
<context>
    <name>QgsUniqueValueDialog</name>
    <message>
        <source>Confirm Delete</source>
        <translation type="unfinished">Löschen bestätigen</translation>
    </message>
    <message>
        <source>The classification field was changed from &apos;%1&apos; to &apos;%2&apos;.
Should the existing classes be deleted before classification?</source>
        <translation type="unfinished">Das Klassifizierungsfeld wurde von &apos;%1&apos; auf &apos;%2&apos; geändert.
Sollen die vorhandenen Klassen vor der Klassifizierung gelöscht werden?</translation>
    </message>
</context>
<context>
    <name>QgsUniqueValueDialogBase</name>
    <message>
        <source>Form1</source>
        <translation>Formular1</translation>
    </message>
    <message>
        <source>Classify</source>
        <translation type="unfinished">Klassifizieren</translation>
    </message>
    <message>
        <source>Classification field</source>
        <translation type="unfinished">Klassifizierungsfeld</translation>
    </message>
    <message>
        <source>Add class</source>
        <translation type="unfinished">Klasse hinzufügen</translation>
    </message>
    <message>
        <source>Delete classes</source>
        <translation type="unfinished">Klassen löschen</translation>
    </message>
    <message>
        <source>Randomize Colors</source>
        <translation type="unfinished">Zufällige Farben</translation>
    </message>
    <message>
        <source>Reset Colors</source>
        <translation type="unfinished">Farben zurücksetzen</translation>
    </message>
</context>
<context>
    <name>QgsVectorLayer</name>
    <message>
        <source>Could not commit the added features.</source>
        <translation type="obsolete">Hinzugefügte Objekte konnten nicht gespeichert werden.</translation>
    </message>
    <message>
        <source>No other types of changes will be committed at this time.</source>
        <translation type="obsolete">Es werden keine weiteren Änderungen gespeichert.</translation>
    </message>
    <message>
        <source>Could not commit the changed attributes.</source>
        <translation type="obsolete">Die geänderten Attribute konnte nicht gespeichert werden.</translation>
    </message>
    <message>
        <source>However, the added features were committed OK.</source>
        <translation type="obsolete">Die hinzugefügten Objekte wurden allerdings gespeichert.</translation>
    </message>
    <message>
        <source>Could not commit the changed geometries.</source>
        <translation type="obsolete">Die geänderten Geometrien konnten nicht gespeichert werden.</translation>
    </message>
    <message>
        <source>However, the changed attributes were committed OK.</source>
        <translation type="obsolete">Die geänderten Attribute wurden jedoch gespeichert.</translation>
    </message>
    <message>
        <source>Could not commit the deleted features.</source>
        <translation type="obsolete">Die Löschung von Objekte konnte nicht vollzogen werden.</translation>
    </message>
    <message>
        <source>However, the changed geometries were committed OK.</source>
        <translation type="obsolete">Die geänderten Geometrien wurden jedoch gespeichert.</translation>
    </message>
    <message>
        <source>ERROR: no provider</source>
        <translation type="unfinished">FEHLER: kein Datenlieferant</translation>
    </message>
    <message>
        <source>ERROR: layer not editable</source>
        <translation type="unfinished">FEHLER: Layer ist nicht veränderbar.</translation>
    </message>
    <message>
        <source>SUCCESS: %1 attributes added.</source>
        <translation type="unfinished">ERFOLG: %1 Attribute geändert.</translation>
    </message>
    <message>
        <source>ERROR: %1 new attributes not added</source>
        <translation type="unfinished">FEHLER: %1 neue Attribute nicht hinzugefügt.</translation>
    </message>
    <message>
        <source>SUCCESS: %1 attributes deleted.</source>
        <translation type="unfinished">ERFOLG: %1 Attribute gelöscht.</translation>
    </message>
    <message>
        <source>ERROR: %1 attributes not deleted.</source>
        <translation type="unfinished">FEHLER: %1 Attribute nicht gelöscht.</translation>
    </message>
    <message>
        <source>SUCCESS: attribute %1 was added.</source>
        <translation type="unfinished">ERFOLG: Attribut %1 wurde hinzugefügt.</translation>
    </message>
    <message>
        <source>ERROR: attribute %1 not added</source>
        <translation type="unfinished">FEHLER: Attribute %1 wurde nicht hinzugefügt.</translation>
    </message>
    <message>
        <source>SUCCESS: %1 attribute values changed.</source>
        <translation type="unfinished">ERFOLG: %1 Attributwerte geändert.</translation>
    </message>
    <message>
        <source>ERROR: %1 attribute value changes not applied.</source>
        <translation type="unfinished">FEHLER: %1 Attributwertänderung nicht angewendet.</translation>
    </message>
    <message>
        <source>SUCCESS: %1 features added.</source>
        <translation type="unfinished">ERFOLG: %1 Objekte hinzugefügt.</translation>
    </message>
    <message>
        <source>ERROR: %1 features not added.</source>
        <translation type="unfinished">FEHLER: %1 Objekte nicht hinzugefügt.</translation>
    </message>
    <message>
        <source>SUCCESS: %1 geometries were changed.</source>
        <translation type="unfinished">ERFOLG: %1 Geometrien wurden geändert.</translation>
    </message>
    <message>
        <source>ERROR: %1 geometries not changed.</source>
        <translation type="unfinished">FEHLER: %1 Geometrien nicht geändert.</translation>
    </message>
    <message>
        <source>SUCCESS: %1 features deleted.</source>
        <translation type="unfinished">ERFOLG: %1 Objekte gelöscht.</translation>
    </message>
    <message>
        <source>ERROR: %1 features not deleted.</source>
        <translation type="unfinished">FEHLER: %1 Objekte nicht gelöscht.</translation>
    </message>
    <message>
        <source>No renderer object</source>
        <translation type="unfinished">Kein Renderer-Objekt</translation>
    </message>
    <message>
        <source>Classification field not found</source>
        <translation type="unfinished">Klassifikationsfeld nicht gefunden</translation>
    </message>
</context>
<context>
    <name>QgsVectorLayerProperties</name>
    <message>
        <source>Transparency: </source>
        <translation>Transparenz: </translation>
    </message>
    <message>
        <source>Single Symbol</source>
        <translation>Einfaches Symbol</translation>
    </message>
    <message>
        <source>Graduated Symbol</source>
        <translation>Abgestuftes Symbol</translation>
    </message>
    <message>
        <source>Continuous Color</source>
        <translation>Fortlaufende Farbe</translation>
    </message>
    <message>
        <source>Unique Value</source>
        <translation>Eindeutiger Wert</translation>
    </message>
    <message>
        <source>This button opens the PostgreSQL query builder and allows you to create a subset of features to display on the map canvas rather than displaying all features in the layer</source>
        <translation>Dieser Knopf öffnet den PostgreSQL-Query-Builder und ermöglicht, statt aller Objekte, eine Untermenge der Objekte auf dem Kartenfenster darzustellen</translation>
    </message>
    <message>
        <source>The query used to limit the features in the layer is shown here. This is currently only supported for PostgreSQL layers. To enter or modify the query, click on the Query Builder button</source>
        <translation>Die Abfrage zur Begrenzung der Anzahl der Objekte wird hier angezeigt. Dies wird im Moment nur bei PostgreSQL-Layern unterstützt. Klicken Sie auf auf &apos;Query Builder&apos;, um eine Abfrage einzugeben oder zu ändern</translation>
    </message>
    <message>
        <source>Spatial Index</source>
        <translation>Räumlicher Index</translation>
    </message>
    <message>
        <source>Creation of spatial index successfull</source>
        <translation type="obsolete">Erstellung des räumlichen Indexes erfolgreich</translation>
    </message>
    <message>
        <source>Creation of spatial index failed</source>
        <translation>Erstellung des räumlichen Indexes fehlgeschlagen</translation>
    </message>
    <message>
        <source>General:</source>
        <translation>Allgemein:</translation>
    </message>
    <message>
        <source>Storage type of this layer : </source>
        <translation>Datenspeicher dieses Layers: </translation>
    </message>
    <message>
        <source>Source for this layer : </source>
        <translation>Quelle dieses Layers: </translation>
    </message>
    <message>
        <source>Geometry type of the features in this layer : </source>
        <translation>Geometrietyp der Objekte dieses Layers: </translation>
    </message>
    <message>
        <source>The number of features in this layer : </source>
        <translation>Anzahl der Objekte dieses Layers: </translation>
    </message>
    <message>
        <source>Editing capabilities of this layer : </source>
        <translation>Bearbeitungsfähigkeit dieses Layers: </translation>
    </message>
    <message>
        <source>Extents:</source>
        <translation>Ausdehnung:</translation>
    </message>
    <message>
        <source>In layer spatial reference system units : </source>
        <translation>In Einheiten des Referenzsystems dieses Layers: </translation>
    </message>
    <message>
        <source>xMin,yMin </source>
        <translation>xMin,yMin </translation>
    </message>
    <message>
        <source> : xMax,yMax </source>
        <translation> : xMax,yMax </translation>
    </message>
    <message>
        <source>In project spatial reference system units : </source>
        <translation>In Einheiten des Projektreferenzsystems: </translation>
    </message>
    <message>
        <source>Layer Spatial Reference System:</source>
        <translation>Räumliches Referenzsystem des Layers:</translation>
    </message>
    <message>
        <source>Attribute field info:</source>
        <translation>Attribut info:</translation>
    </message>
    <message>
        <source>Field</source>
        <translation>Feld</translation>
    </message>
    <message>
        <source>Type</source>
        <translation>Typ</translation>
    </message>
    <message>
        <source>Length</source>
        <translation>Länge</translation>
    </message>
    <message>
        <source>Precision</source>
        <translation>Genauigkeit</translation>
    </message>
    <message>
        <source>Layer comment: </source>
        <translation>Layerkommentar: </translation>
    </message>
    <message>
        <source>Comment</source>
        <translation type="unfinished">Kommentar</translation>
    </message>
    <message>
        <source>Default Style</source>
        <translation>Standardstil</translation>
    </message>
    <message>
        <source>QGIS Layer Style File (*.qml)</source>
        <translation>QGIS Layerstil Datei (*.qml)</translation>
    </message>
    <message>
        <source>QGIS</source>
        <translation>QGIS</translation>
    </message>
    <message>
        <source>Unknown style format: </source>
        <translation>Unbekanntes Stilformat: </translation>
    </message>
    <message>
        <source>Saved Style</source>
        <translation type="unfinished">Gespeicherter Stil</translation>
    </message>
    <message>
        <source>id</source>
        <translation type="unfinished">ID</translation>
    </message>
    <message>
        <source>name</source>
        <translation type="unfinished">Name</translation>
    </message>
    <message>
        <source>type</source>
        <translation type="unfinished">Typ</translation>
    </message>
    <message>
        <source>length</source>
        <translation type="unfinished">Länge</translation>
    </message>
    <message>
        <source>precision</source>
        <translation type="unfinished">Genauigkeit</translation>
    </message>
    <message>
        <source>comment</source>
        <translation type="unfinished">Kommentar</translation>
    </message>
    <message>
        <source>edit widget</source>
        <translation type="unfinished">Eingabefeld</translation>
    </message>
    <message>
        <source>values</source>
        <translation type="unfinished">Werte</translation>
    </message>
    <message>
        <source>line edit</source>
        <translation type="unfinished">Texteingabe</translation>
    </message>
    <message>
        <source>unique values</source>
        <translation type="unfinished">eindeutige Werte</translation>
    </message>
    <message>
        <source>unique values (editable)</source>
        <translation type="unfinished">eindeutige Werte (änderbar)</translation>
    </message>
    <message>
        <source>value map</source>
        <translation type="unfinished">Wertabbildung</translation>
    </message>
    <message>
        <source>classification</source>
        <translation type="unfinished">Klassifizierung</translation>
    </message>
    <message>
        <source>range (editable)</source>
        <translation type="unfinished">Bereich (änderbar)</translation>
    </message>
    <message>
        <source>range (slider)</source>
        <translation type="unfinished">Bereich (Schiebregler)</translation>
    </message>
    <message>
        <source>file name</source>
        <translation type="unfinished">Dateiname</translation>
    </message>
    <message>
        <source>Name conflict</source>
        <translation type="unfinished">Namenskonflikt</translation>
    </message>
    <message>
        <source>The attribute could not be inserted. The name already exists in the table.</source>
        <translation type="unfinished">Das Attribut konnte nicht eingefügt werden, da der Name bereits vorhanden ist.</translation>
    </message>
    <message>
        <source>Creation of spatial index successful</source>
        <translation type="unfinished">Räumlicher Index erfolgreich erzeugt.</translation>
    </message>
</context>
<context>
    <name>QgsVectorLayerPropertiesBase</name>
    <message>
        <source>Layer Properties</source>
        <translation>Layereigenschaften</translation>
    </message>
    <message>
        <source>Symbology</source>
        <translation>Bezeichnungen</translation>
    </message>
    <message>
        <source>General</source>
        <translation>Allgemein</translation>
    </message>
    <message>
        <source>Use scale dependent rendering</source>
        <translation>Massstabsabhängig zeichnen</translation>
    </message>
    <message>
        <source>Minimum scale at which this layer will be displayed. </source>
        <translation>Minimalmassstab, ab dem dieser Layer angezeigt wird.</translation>
    </message>
    <message>
        <source>Maximum scale at which this layer will be displayed. </source>
        <translation>Maximalmassstab, bis zu dem dieser Layer angezeigt wird.</translation>
    </message>
    <message>
        <source>Display name</source>
        <translation>Anzeigename</translation>
    </message>
    <message>
        <source>Use this control to set which field is placed at the top level of the Identify Results dialog box.</source>
        <translation>Benutze diese Kontrollelemente, um das Attribut festzulegen, das zuoberst im  &apos;Identifizierungsergebnis&apos;-Dialog steht.</translation>
    </message>
    <message>
        <source>Display field for the Identify Results dialog box</source>
        <translation>Anzeigeattribut für den &apos;Identifizierungsergebnis&apos;-Dialog</translation>
    </message>
    <message>
        <source>This sets the display field for the Identify Results dialog box</source>
        <translation>Dies setzt das Anzeigeattribut für den &apos;Identifizierungsergebnis&apos;-Dialog</translation>
    </message>
    <message>
        <source>Display field</source>
        <translation>Anzeigeattribut</translation>
    </message>
    <message>
        <source>Subset</source>
        <translation>Subset</translation>
    </message>
    <message>
        <source>Query Builder</source>
        <translation>Query Builder</translation>
    </message>
    <message>
        <source>Create Spatial Index</source>
        <translation>Räumlichen Index erstellen</translation>
    </message>
    <message>
        <source>Metadata</source>
        <translation>Metadaten</translation>
    </message>
    <message>
        <source>Labels</source>
        <translation>Beschriftungen</translation>
    </message>
    <message>
        <source>Display labels</source>
        <translation>Zeige Beschriftungen an</translation>
    </message>
    <message>
        <source>Actions</source>
        <translation>Aktionen</translation>
    </message>
    <message>
        <source>Restore Default Style</source>
        <translation>Standardstil wiederherstellen</translation>
    </message>
    <message>
        <source>Save As Default</source>
        <translation>Als Standard speichern</translation>
    </message>
    <message>
        <source>Load Style ...</source>
        <translation>Stil laden...</translation>
    </message>
    <message>
        <source>Save Style ...</source>
        <translation>Stil speichern...</translation>
    </message>
    <message>
        <source>Legend type</source>
        <translation type="unfinished">Legendentyp</translation>
    </message>
    <message>
        <source>Transparency</source>
        <translation type="unfinished">Transparenz</translation>
    </message>
    <message>
        <source>Options</source>
        <translation type="unfinished">Optionen</translation>
    </message>
    <message>
        <source>Change SRS</source>
        <translation type="obsolete">SRS ändern</translation>
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
        <translation type="unfinished">KBS ändern</translation>
    </message>
    <message>
        <source>Attributes</source>
        <translation type="unfinished">Attribute</translation>
    </message>
    <message>
        <source>New column</source>
        <translation type="unfinished">Neue Attributspalte</translation>
    </message>
    <message>
        <source>Ctrl+N</source>
        <translation type="unfinished">Ctrl+N</translation>
    </message>
    <message>
        <source>Delete column</source>
        <translation type="unfinished">Lösche Attributspalte</translation>
    </message>
    <message>
        <source>Ctrl+X</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Toggle editing mode</source>
        <translation type="unfinished">Bearbeitungsmodus umschalten</translation>
    </message>
    <message>
        <source>Click to toggle table editing</source>
        <translation type="unfinished">Zur Umschaltung des Bearbeitungsmodus klicken</translation>
    </message>
</context>
<context>
    <name>QgsWFSPlugin</name>
    <message>
        <source>&amp;Add WFS layer</source>
        <translation>&amp;WFS-Layer hinzufügen</translation>
    </message>
</context>
<context>
    <name>QgsWFSProvider</name>
    <message>
        <source>unknown</source>
        <translation>unbekannt</translation>
    </message>
    <message>
        <source>received %1 bytes from %2</source>
        <translation>%1 von %2 Bytes empfangen</translation>
    </message>
</context>
<context>
    <name>QgsWFSSourceSelect</name>
    <message>
        <source>Are you sure you want to remove the </source>
        <translation>Sind Sie sicher, dass Sie die </translation>
    </message>
    <message>
        <source> connection and all associated settings?</source>
        <translation> Verbindung und alle damit verbundenen Einstellungen löschen wollen?</translation>
    </message>
    <message>
        <source>Confirm Delete</source>
        <translation>Löschen bestätigen</translation>
    </message>
</context>
<context>
    <name>QgsWFSSourceSelectBase</name>
    <message>
        <source>Title</source>
        <translation>Titel</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Name</translation>
    </message>
    <message>
        <source>Abstract</source>
        <translation>Zusammenfassung</translation>
    </message>
    <message>
        <source>Coordinate Reference System</source>
        <translation>Koordinatensystem</translation>
    </message>
    <message>
        <source>Change ...</source>
        <translation>Ändern ...</translation>
    </message>
    <message>
        <source>Server Connections</source>
        <translation>Serververbindungen</translation>
    </message>
    <message>
        <source>&amp;New</source>
        <translation>&amp;Neu</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>Löschen</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation>Bearbeiten</translation>
    </message>
    <message>
        <source>C&amp;onnect</source>
        <translation>Verbinden</translation>
    </message>
    <message>
        <source>Add WFS Layer from a Server</source>
        <translation>WFS-Layer von Server hinzufügen</translation>
    </message>
</context>
<context>
    <name>QgsWmsProvider</name>
    <message>
        <source>Tried URL: </source>
        <translation>Versuchte URL: </translation>
    </message>
    <message>
        <source>HTTP Exception</source>
        <translation>HTTP Ausnahme</translation>
    </message>
    <message>
        <source>WMS Service Exception</source>
        <translation>WMS-Service-Ausnahme</translation>
    </message>
    <message>
        <source>DOM Exception</source>
        <translation type="obsolete">DOM-Ausnahme</translation>
    </message>
    <message>
        <source>Could not get WMS capabilities: %1 at line %2 column %3</source>
        <translation>Konnte die WMS-Capabilities nicht erfragen: %1 in Zeile %2, Spalte %3</translation>
    </message>
    <message>
        <source>This is probably due to an incorrect WMS Server URL.</source>
        <translation>Dies kommt wahrscheinlich durch eine falsche WMS-Server-URL zustande.</translation>
    </message>
    <message>
        <source>Could not get WMS capabilities in the expected format (DTD): no %1 or %2 found</source>
        <translation>Konnte die WMS-Capabilities nicht im erwarteten Format (DTD) erhalten: %1 oder %2 gefunden.</translation>
    </message>
    <message>
        <source>Could not get WMS Service Exception at %1: %2 at line %3 column %4</source>
        <translation>Konnte die WMS-Service-Ausnahme bei %1 nicht interpretieren: %2 in Zeile %3, Spalte %4</translation>
    </message>
    <message>
        <source>Request contains a Format not offered by the server.</source>
        <translation>Die Anfrage enthält ein Format, das nicht vom Server unterstützt wird.</translation>
    </message>
    <message>
        <source>Request contains a CRS not offered by the server for one or more of the Layers in the request.</source>
        <translation>Die Anfrage enthält ein Koordinatensystem (CRS), das für die angeforderten Layer nicht vom Server angeboten wird.</translation>
    </message>
    <message>
        <source>Request contains a SRS not offered by the server for one or more of the Layers in the request.</source>
        <translation>Anfrage enthält ein SRS, das nicht vom Server angeboten wird.</translation>
    </message>
    <message>
        <source>GetMap request is for a Layer not offered by the server, or GetFeatureInfo request is for a Layer not shown on the map.</source>
        <translation>GetMap-Anfrage ist für einen Layer, der nicht vom Server angeboten wird, oder GetFeatureInfo wurde für einen Layer angefordert, der nicht im Kartenfenster angezeigt wird.</translation>
    </message>
    <message>
        <source>Request is for a Layer in a Style not offered by the server.</source>
        <translation>Anfrage ist für einen Layer in einem Style, der nicht vom Server angeboten wird.</translation>
    </message>
    <message>
        <source>GetFeatureInfo request is applied to a Layer which is not declared queryable.</source>
        <translation>GetFeatureInfo wurde auf einen Layer angewendet, der als nicht abfragbar definiert ist.</translation>
    </message>
    <message>
        <source>GetFeatureInfo request contains invalid X or Y value.</source>
        <translation>Get FeatureInfo Anfrage enthält ungültige X oder Y-Werte.</translation>
    </message>
    <message>
        <source>Request does not include a sample dimension value, and the server did not declare a default value for that dimension.</source>
        <translation>Anfrage enthält keinen beispielhaften Dimensionswert, und der Server selbst definiert auch keinen.</translation>
    </message>
    <message>
        <source>Request contains an invalid sample dimension value.</source>
        <translation>Anfrage enthält einen ungültigen beispielhaften Dimensionswert.</translation>
    </message>
    <message>
        <source>Request is for an optional operation that is not supported by the server.</source>
        <translation>Anfrage ist für eine optionale Operation, die der Server nicht unterstützt.</translation>
    </message>
    <message>
        <source>(Unknown error code from a post-1.3 WMS server)</source>
        <translation>(Unbekannte Fehlermeldung eines post-1.3 WMS-Servers).</translation>
    </message>
    <message>
        <source>The WMS vendor also reported: </source>
        <translation>Der WMS-Betreiber sagt folgendes:</translation>
    </message>
    <message>
        <source>This is probably due to a bug in the QGIS program.  Please report this error.</source>
        <translation type="obsolete">Dieses Verhalten kommt wahrscheinlich durch einen Bug in QGIS. Bitte melden Sie den Fehler bei den Entwicklern.</translation>
    </message>
    <message>
        <source>Server Properties:</source>
        <translation>Server-Eigenschaften:</translation>
    </message>
    <message>
        <source>Property</source>
        <translation>Eigenschaft</translation>
    </message>
    <message>
        <source>Value</source>
        <translation>Wert</translation>
    </message>
    <message>
        <source>WMS Version</source>
        <translation>WMS Version</translation>
    </message>
    <message>
        <source>Title</source>
        <translation>Titel</translation>
    </message>
    <message>
        <source>Abstract</source>
        <translation>Zusammenfassung</translation>
    </message>
    <message>
        <source>Keywords</source>
        <translation>Schlüsselworte</translation>
    </message>
    <message>
        <source>Online Resource</source>
        <translation>Online Quelle</translation>
    </message>
    <message>
        <source>Contact Person</source>
        <translation>Kontaktperson</translation>
    </message>
    <message>
        <source>Fees</source>
        <translation>Kosten</translation>
    </message>
    <message>
        <source>Access Constraints</source>
        <translation>Zugangsbeschränkungen</translation>
    </message>
    <message>
        <source>Image Formats</source>
        <translation>Bildformate</translation>
    </message>
    <message>
        <source>Identify Formats</source>
        <translation>Abfrageformat</translation>
    </message>
    <message>
        <source>Layer Count</source>
        <translation>Anzahl der Layer</translation>
    </message>
    <message>
        <source>Layer Properties: </source>
        <translation>Layereigenschaften: </translation>
    </message>
    <message>
        <source>Selected</source>
        <translation>Ausgewählt</translation>
    </message>
    <message>
        <source>Yes</source>
        <translation>Ja</translation>
    </message>
    <message>
        <source>No</source>
        <translation>Nein</translation>
    </message>
    <message>
        <source>Visibility</source>
        <translation>Sichtbarkeit</translation>
    </message>
    <message>
        <source>Visible</source>
        <translation>Sichtbar</translation>
    </message>
    <message>
        <source>Hidden</source>
        <translation>Versteckt</translation>
    </message>
    <message>
        <source>n/a</source>
        <translation>n/a</translation>
    </message>
    <message>
        <source>Can Identify</source>
        <translation>Kann abgefragt werden</translation>
    </message>
    <message>
        <source>Can be Transparent</source>
        <translation>Kann transparent sein</translation>
    </message>
    <message>
        <source>Can Zoom In</source>
        <translation>Kann reingezoomt werden.</translation>
    </message>
    <message>
        <source>Cascade Count</source>
        <translation>Kaskadiere Anzahl</translation>
    </message>
    <message>
        <source>Fixed Width</source>
        <translation>Feste Breite</translation>
    </message>
    <message>
        <source>Fixed Height</source>
        <translation>Feste Höhe</translation>
    </message>
    <message>
        <source>WGS 84 Bounding Box</source>
        <translation>WGS 84 Bounding Box</translation>
    </message>
    <message>
        <source>Available in CRS</source>
        <translation>Verfügbar in CRS</translation>
    </message>
    <message>
        <source>Available in style</source>
        <translation>Verfügbar im Style</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Name</translation>
    </message>
    <message>
        <source>Value of (optional) UpdateSequence parameter in GetCapabilities request is equal to current value of service metadata update sequence number.</source>
        <translation>Wert in.</translation>
    </message>
    <message>
        <source>Value of (optional) UpdateSequence parameter in GetCapabilities request is greater than current value of service metadata update sequence number.</source>
        <translation>Wert in.</translation>
    </message>
    <message>
        <source>Layer cannot be queried.</source>
        <translation type="unfinished">Layer kann nicht abgefragt werden.</translation>
    </message>
    <message>
        <source>Dom Exception</source>
        <translation type="unfinished">Dom-Ausnahme</translation>
    </message>
</context>
<context>
    <name>QuickPrintGui</name>
    <message>
        <source>Portable Document Format (*.pdf)</source>
        <translation>Portable Document Format (*.pdf)</translation>
    </message>
    <message>
        <source>quickprint</source>
        <translation>Schnelldruck</translation>
    </message>
    <message>
        <source>Unknown format: </source>
        <translation>Unbekanntes Format: </translation>
    </message>
</context>
<context>
    <name>QuickPrintGuiBase</name>
    <message>
        <source>QGIS Quick Print Plugin</source>
        <translation>QGIS Schnelldruck Plugin</translation>
    </message>
    <message>
        <source>Quick Print</source>
        <translation>Schneller Druck</translation>
    </message>
    <message>
        <source>Map Title e.g. ACME inc.</source>
        <translation>Kartentitel z.B. ACME inc.</translation>
    </message>
    <message>
        <source>Map Name e.g. Water Features</source>
        <translation>Kartenname z.B. Wasserobjekte</translation>
    </message>
    <message>
        <source>Copyright</source>
        <translation>Copyright</translation>
    </message>
    <message>
        <source>Output</source>
        <translation type="unfinished">Ergebnis</translation>
    </message>
    <message>
        <source>Use last filename but incremented.</source>
        <translation>Benutze zuletzt verwendeten Dateinamen aber erhöht.</translation>
    </message>
    <message>
        <source>last used filename but incremented will be shown here</source>
        <translation>zuletzt verwendeter aber erhöhter Dateiname wrd hier gezeigt</translation>
    </message>
    <message>
        <source>Prompt for file name</source>
        <translation>Eingabeaufforderung für Dateiname</translation>
    </message>
    <message>
        <source>Note: If you want more control over the map layout please use the map composer function in QGIS.</source>
        <translation>Bemerkung: Wenn Sie mehr Kontrolle über das Layout haben wollen, benutzen Sie bitte den QGIS Map Composer.</translation>
    </message>
    <message>
        <source>Page Size</source>
        <translation type="unfinished">Seitengröße</translation>
    </message>
</context>
<context>
    <name>QuickPrintPlugin</name>
    <message>
        <source>Quick Print</source>
        <translation>Schnelldruck</translation>
    </message>
    <message>
        <source>&amp;Quick Print</source>
        <translation type="unfinished">Schnelldruck</translation>
    </message>
    <message>
        <source>Provides a way to quickly produce a map with minimal user input.</source>
        <translation type="unfinished">Bietet eine schnelle Möglichkeit Karten mit minimalen Benutzereingaben zu erzeugen</translation>
    </message>
</context>
<context>
    <name>RepositoryDetailsDialog</name>
    <message>
        <source>Repository details</source>
        <translation type="obsolete">Repository Details</translation>
    </message>
    <message>
        <source>Name:</source>
        <translation type="obsolete">Name:</translation>
    </message>
    <message>
        <source>URL:</source>
        <translation type="obsolete">URL:</translation>
    </message>
    <message>
        <source>http://</source>
        <translation type="obsolete">http://</translation>
    </message>
</context>
<context>
    <name>[pluginname]GuiBase</name>
    <message>
        <source>QGIS Plugin Template</source>
        <translation type="unfinished">QGIS Plugin Vorlage</translation>
    </message>
    <message>
        <source>Plugin Template</source>
        <translation type="unfinished">Plugin Vorlage</translation>
    </message>
</context>
<context>
    <name>dxf2shpConverter</name>
    <message>
        <source>Converts DXF files in Shapefile format</source>
        <translation type="unfinished">Konvertiert DXF-Dateien ins Shape-Format.</translation>
    </message>
    <message>
        <source>&amp;Dxf2Shp</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>dxf2shpConverterGui</name>
    <message>
        <source>Dxf Importer</source>
        <translation type="unfinished">DXF-Import</translation>
    </message>
    <message>
        <source>Input Dxf file</source>
        <translation type="unfinished">DXF-Eingabedatei</translation>
    </message>
    <message>
        <source>...</source>
        <translation type="unfinished">...</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;DejaVu Sans&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;MS Shell Dlg 2&apos;; font-size:8pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;Output file&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;DejaVu Sans&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;MS Shell Dlg 2&apos;; font-size:8pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;Ausgabedatei&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Output file type</source>
        <translation type="unfinished">Typ der Ausgabedatei</translation>
    </message>
    <message>
        <source>Polyline</source>
        <translation type="unfinished">Polylinie</translation>
    </message>
    <message>
        <source>Polygon</source>
        <translation type="unfinished">Polygon</translation>
    </message>
    <message>
        <source>Point</source>
        <translation type="unfinished">Punkt</translation>
    </message>
    <message>
        <source>Export text labels</source>
        <translation type="unfinished">Beschriftungen exportieren</translation>
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
        <translation type="obsolete">Feldbeschreibung:
* DXF-Eingabedatei: Pfad zur zu konvertierenden DXF-Datei
* Ausgabe-Shape-Datei: Gewünschter Name der zu erzeugenden Shapedatei
* Typ der Ausgabe-Shape-Datei: gibt den Typ der Ausgabe-Shape-Datei
* Beschriftungen exportieren: Es wird ein zusätzlicher Shapepunkt-Layer erzeugt, dessen zugehörige DBF-Tabelle Informationen über das &quot;TEXT&quot;-Feld der DXF-Datei und die Zeichenketten selbst enthält.

---
Entwickelt von Paolo L. Scala, Barbara Rita Barricelli, Marco Padula
CNR, Milan Unit (Information Technology), Construction Technologies Institute.
Unterstützung unter scala@itc.cnr.it
</translation>
    </message>
    <message>
        <source>Choose a DXF file to open</source>
        <translation type="obsolete">Zu öffnende DXF-Datei wählen</translation>
    </message>
    <message>
        <source>Choose a file name to save to</source>
        <translation type="obsolete">Dateiname zum Speichern wählen</translation>
    </message>
</context>
<context>
    <name>pluginname</name>
    <message>
        <source>Replace this with a short description of the what the plugin does</source>
        <translation type="obsolete">Diese Notiz mit einer kurzen Beschreibung, was das Plugin macht, ersetzen</translation>
    </message>
    <message>
        <source>[menuitemname]</source>
        <translation type="unfinished">[menuitemname]</translation>
    </message>
    <message>
        <source>&amp;[menuname]</source>
        <translation>&amp;[menuname]</translation>
    </message>
    <message>
        <source>Replace this with a short description of what the plugin does</source>
        <translation type="unfinished">Kurzbeschreibung des Plugins hier</translation>
    </message>
</context>
</TS>
