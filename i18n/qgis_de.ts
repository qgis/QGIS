<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS><TS version="1.1">
<context>
    <name>CoordinateCapture</name>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapture.cpp" line="142"/>
        <source>Coordinate Capture</source>
        <translation>Koordinaten abgreifen</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapture.cpp" line="87"/>
        <source>Click on the map to view coordinates and capture to clipboard.</source>
        <translation>Klicken Sie auf die Karte um Koordinaten anzuzeigen und in die Zwischenanlage zu übertragen</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapture.cpp" line="92"/>
        <source>&amp;Coordinate Capture</source>
        <translation>&amp;Koordinaten abgreifen</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapture.cpp" line="108"/>
        <source>Click to select the CRS to use for coordinate display</source>
        <translation>Klicken Sie um das KBS zur Koordinatenanzeige auszuwählen</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapture.cpp" line="117"/>
        <source>Coordinate in your selected CRS</source>
        <translation>Koordinate im gewählten Koordinatenbezugssystem</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapture.cpp" line="121"/>
        <source>Coordinate in map canvas coordinate reference system</source>
        <translation>Koordinate im Koordinatenbezugssystem des Kartenbereichs</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapture.cpp" line="124"/>
        <source>Copy to clipboard</source>
        <translation>In die Zwischenablage kopieren</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapture.cpp" line="129"/>
        <source>Click to enable mouse tracking. Click the canvas to stop</source>
        <translation>Klicken um die Mausverfolgung zu aktivieren.  Zum Beenden in den Kartenbereich klicken</translation>
    </message>
</context>
<context>
    <name>CoordinateCaptureGui</name>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapturegui.cpp" line="55"/>
        <source>Welcome to your automatically generated plugin!</source>
        <translation type="unfinished">Willkommen zu Ihrem automatisch erzeugten Plugin!</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapturegui.cpp" line="56"/>
        <source>This is just a starting point. You now need to modify the code to make it do something useful....read on for a more information to get yourself started.</source>
        <translation type="unfinished">Dies ist erst der erste Schritt. Sie müssen nun den Quellcode anpassen, damit es etwas sinnvolles macht ... lesen Sie dazu weiter.</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapturegui.cpp" line="57"/>
        <source>Documentation:</source>
        <translation type="unfinished">Dokumentation:</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapturegui.cpp" line="58"/>
        <source>You really need to read the QGIS API Documentation now at:</source>
        <translation type="unfinished">Sie sollten nun unbedingt die QGIS API-Dokumentation lesen unter:</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapturegui.cpp" line="59"/>
        <source>In particular look at the following classes:</source>
        <translation type="unfinished">Schauen Sie insbesondere nach den folgenden Klassen:</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapturegui.cpp" line="62"/>
        <source>QgsPlugin is an ABC that defines required behaviour your plugin must provide. See below for more details.</source>
        <translation type="unfinished">QgsPlugin ist eine Grundlage, die das notwendige Verhalten Ihres Plugins definiert und bereitstellt. Lesen Sie weiter für mehr Details.</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapturegui.cpp" line="63"/>
        <source>What are all the files in my generated plugin directory for?</source>
        <translation type="unfinished">Wozu dienen die ganzen Dateien in dem gerade erstellten Plugin-Ordner?</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapturegui.cpp" line="64"/>
        <source>This is the generated CMake file that builds the plugin. You should add you application specific dependencies and source files to this file.</source>
        <translation type="unfinished">Dies ist die CMake-Datei, die den Plugin erstellt. Sie sollten die anwendungsspezifischen Abhängigkeiten und die Quelldateien in der CMake-Datei ergänzen.</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapturegui.cpp" line="65"/>
        <source>This is the class that provides the &apos;glue&apos; between your custom application logic and the QGIS application. You will see that a number of methods are already implemented for you - including some examples of how to add a raster or vector layer to the main application map canvas. This class is a concrete instance of the QgisPlugin interface which defines required behaviour for a plugin. In particular, a plugin has a number of static methods and members so that the QgsPluginManager and plugin loader logic can identify each plugin, create an appropriate menu entry for it etc. Note there is nothing stopping you creating multiple toolbar icons and menu entries for a single plugin. By default though a single menu entry and toolbar button is created and its pre-configured to call the run() method in this class when selected. This default implementation provided for you by the plugin builder is well documented, so please refer to the code for further advice.</source>
        <translation type="unfinished">Dies ist die Klasse, die Ihre Applikation mit QGIS verbindet. Sie werden sehen, dass bereits eine Vielzahl an Methoden bereitstehen, inklusive einiger Beispiele, etwa wie Raster- oder Vektorlayer in das Kartenfenster integriert werden können. Diese Klasse ist eine feste Instanz des QGIS Plugin-Interfaces, welche notwendiges Verhalten des Plugins definiert. Genau gesagt, enthält ein Plugin eine Reihe statischer Methoden und Klassenmitglieder, damit der QGIS Pluginmanager und der Pluginlader jedes Plugin identifizieren, einen passenden Menüeintrag erstellen kann usw. Beachten Sie, dass Sie auch mehrere Icons für die Werkzeugleiste sowie mehere Menüeinträge für ein einzelnes Plugin erstellen können. Standardmässig wird jedoch ein einzelnes Icon und ein Menüeintrag erstellt und so vorkonfiguriert, dass die Methode run() dieser Klasse bei ihrer Auswahl gestarted wird. Diese durch den Pluginbuilder bereitgestellte Standardimplementierung ist sehr gut dokumentiert. Beziehen Sie sich daher bitte auf den Quellcode für weitere Hinweise.</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapturegui.cpp" line="66"/>
        <source>This is a Qt designer &apos;ui&apos; file. It defines the look of the default plugin dialog without implementing any application logic. You can modify this form to suite your needs or completely remove it if your plugin does not need to display a user form (e.g. for custom MapTools).</source>
        <translation type="unfinished">Dies ist eine Qt Designer &apos;ui&apos; Datei. Sie definiert das Aussehen des Standard Plugindialogs ohne irgendeine Anwendungsfunktion. Sie können die Vorlage an Ihre Bedürfnisse anpassen oder auch löschen, wenn Ihr Plugin keinen Benutzerdialog braucht (z.B. für angepasste MapTools).</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapturegui.cpp" line="67"/>
        <source>This is the concrete class where application logic for the above mentioned dialog should go. The world is your oyster here really....</source>
        <translation type="unfinished">Dies ist eine feste Klasse in welche die Applikationstechnologie des oben beschriebenen Dialogs,eingefügt werden sollte Die Welt steht Ihnen an dieser Stelle absolut offen....</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapturegui.cpp" line="68"/>
        <source>This is the Qt4 resources file for your plugin. The Makefile generated for your plugin is all set up to compile the resource file so all you need to do is add your additional icons etc using the simple xml file format. Note the namespace used for all your resources e.g. (&apos;:/Homann/&apos;). It is important to use this prefix for all your resources. We suggest you include any other images and run time data in this resurce file too.</source>
        <translation type="unfinished">Dies ist die Qt4-Resourcendatei für Ihren plugin. Der für Ihr Plugin erstellte Makefile ist erstellt, um die Resourcendatei zu kompilieren. Alles was Sie hier tun müssen, ist die zusätzlichen Icons usw. mit Hilfe des einfachen XML-Formates zu ergänzen. Beachten Sie, die Namensräume für Ihre Resourcen z.B. (&apos;:/Homann/&apos;). Es ist wichtig diesen Prefix für all Ihre Resourcen zu verwenden. Wir schlagen vor, Sie bauen ein irgendwelche anderen Bilder und Laufzeitdaten in die Resourcendatei ein.</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapturegui.cpp" line="69"/>
        <source>This is the icon that will be used for your plugin menu entry and toolbar icon. Simply replace this icon with your own icon to make your plugin disctinctive from the rest.</source>
        <translation type="unfinished">Dies ist das Icon, dass für Ihr Pluginmenü und die Werkzeugleiste benutzt wird. Ersetzen Sie das Icon einfach durch ihr eigenes, um Ihr Plugin von den anderen zu unterscheiden.</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapturegui.cpp" line="70"/>
        <source>This file contains the documentation you are reading now!</source>
        <translation type="unfinished">Diese Datei enthält die Dokumentation, die Sie gerade lesen!</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapturegui.cpp" line="71"/>
        <source>Getting developer help:</source>
        <translation type="unfinished">Entwickler-Hilfe bekommen:</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapturegui.cpp" line="72"/>
        <source>For Questions and Comments regarding the plugin builder template and creating your features in QGIS using the plugin interface please contact us via:</source>
        <translation type="unfinished">Für Fragen und Kommentare in Bezug auf das &apos;Plugin-Builder&apos; Template und die Erstellung eigener Funktionen in QGIS mit Hilfe des Plugin-Interfaces kontaktieren Sie uns bitter unter:</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapturegui.cpp" line="73"/>
        <source>&lt;li&gt; the QGIS developers mailing list, or &lt;/li&gt;&lt;li&gt; IRC (#qgis on freenode.net)&lt;/li&gt;</source>
        <translation type="unfinished">&lt;li&gt; Die QGIS Entwickler-Mailingliste, oder &lt;/li&gt;&lt;li&gt; IRC (#qgis on freenode.net)&lt;/li&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapturegui.cpp" line="74"/>
        <source>QGIS is distributed under the Gnu Public License. If you create a useful plugin please consider contributing it back to the community.</source>
        <translation type="unfinished">QGIS ist veröffentlicht unter der GNU General Public License. Wenn Sie ein nützliches Plugin erstellt haben, überlegen Sie bitte, es der Community bereitzustellen.</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapturegui.cpp" line="75"/>
        <source>Have fun and thank you for choosing QGIS.</source>
        <translation type="unfinished">Viel Spass und danke, dass Sie sich für QGIS entschieden haben.</translation>
    </message>
</context>
<context>
    <name>CoordinateCaptureGuiBase</name>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecaptureguibase.ui" line="13"/>
        <source>QGIS Plugin Template</source>
        <translation type="unfinished">QGIS Plugin-Vorlage</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecaptureguibase.ui" line="47"/>
        <source>Plugin Template</source>
        <translation type="unfinished">Plugin-Vorlage</translation>
    </message>
</context>
<context>
    <name>Dialog</name>
    <message>
        <location filename="../src/plugins/ogr_converter/dialog.cpp" line="115"/>
        <source>Connect</source>
        <translation type="unfinished">Verbinden</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/dialog.cpp" line="119"/>
        <source>Browse</source>
        <translation type="unfinished">Durchsuchen</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/dialog.cpp" line="424"/>
        <source>OGR Converter</source>
        <translation type="unfinished">OGR-Konverter</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/dialog.cpp" line="421"/>
        <source>Could not establish connection to: &apos;</source>
        <translation type="unfinished">Konnte keine Verbindung herstellen zu: &apos;</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/dialog.cpp" line="216"/>
        <source>Open OGR file</source>
        <translation type="unfinished">OGR-Datei öffnen</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/dialog.cpp" line="435"/>
        <source>OGR File Data Source (*.*)</source>
        <translation type="unfinished">OGR-Quelldatei (*.*)</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/dialog.cpp" line="226"/>
        <source>Open Directory</source>
        <translation type="unfinished">Verzeichnis öffnen</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/dialog.cpp" line="241"/>
        <source>Input OGR dataset is missing!</source>
        <translation type="unfinished">OGR-Quelle fehlt!</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/dialog.cpp" line="248"/>
        <source>Input OGR layer name is missing!</source>
        <translation type="unfinished">OGR-Layername fehlt!</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/dialog.cpp" line="262"/>
        <source>Target OGR format not selected!</source>
        <translation type="unfinished">OGR-Zielformat nicht gewählt</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/dialog.cpp" line="269"/>
        <source>Output OGR dataset is missing!</source>
        <translation type="unfinished">OGR-Ziel fehlt!</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/dialog.cpp" line="276"/>
        <source>Output OGR layer name is missing!</source>
        <translation type="unfinished">Name des OGR-Ausgabelayers fehlt!</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/dialog.cpp" line="298"/>
        <source>Successfully translated layer &apos;</source>
        <translation type="unfinished">Layer wurde erfolgreich konvertiert: &apos;</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/dialog.cpp" line="303"/>
        <source>Failed to translate layer &apos;</source>
        <translation type="unfinished">Konvertierung des Layer gescheitert: &apos;</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/dialog.cpp" line="417"/>
        <source>Successfully connected to: &apos;</source>
        <translation type="unfinished">Erfolgreich verbunden mit &apos;</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/dialog.cpp" line="434"/>
        <source>Choose a file name to save to</source>
        <translation type="unfinished">Dateiname zum Speichern wählen</translation>
    </message>
</context>
<context>
    <name>Gui</name>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="55"/>
        <source>Welcome to your automatically generated plugin!</source>
        <translation>Willkommen zu Ihrem automatisch installierten Plugin!</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="56"/>
        <source>This is just a starting point. You now need to modify the code to make it do something useful....read on for a more information to get yourself started.</source>
        <translation>Dies ist erst der erste Schritt. Sie müssen nun den Quellcode anpassen, damit es etwas sinnvolles macht ... lesen Sie dazu weiter.</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="57"/>
        <source>Documentation:</source>
        <translation>Dokumentation:</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="58"/>
        <source>You really need to read the QGIS API Documentation now at:</source>
        <translation>Sie sollten nun unbedingt die QGIS API-Dokumentation lesen unter:</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="59"/>
        <source>In particular look at the following classes:</source>
        <translation>Schauen Sie insbesondere nach den folgenden Klassen:</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="62"/>
        <source>QgsPlugin is an ABC that defines required behaviour your plugin must provide. See below for more details.</source>
        <translation>QgsPlugin ist eine Grundlage, die das notwendige Verhalten Ihres Plugins definiert und bereitstellt. Lesen Sie weiter für mehr Details.</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="63"/>
        <source>What are all the files in my generated plugin directory for?</source>
        <translation>Wozu sind die ganzen Dateien in dem gerade erstellten Plugin-Ordner nützlich?</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="64"/>
        <source>This is the generated CMake file that builds the plugin. You should add you application specific dependencies and source files to this file.</source>
        <translation>Dies ist die CMake-Datei, die den Plugin erstellt. Sie sollten die anwendungsspezifischen Abhängigkeiten und die Quelldateien in der CMake-Datei ergänzen.</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="65"/>
        <source>This is the class that provides the &apos;glue&apos; between your custom application logic and the QGIS application. You will see that a number of methods are already implemented for you - including some examples of how to add a raster or vector layer to the main application map canvas. This class is a concrete instance of the QgisPlugin interface which defines required behaviour for a plugin. In particular, a plugin has a number of static methods and members so that the QgsPluginManager and plugin loader logic can identify each plugin, create an appropriate menu entry for it etc. Note there is nothing stopping you creating multiple toolbar icons and menu entries for a single plugin. By default though a single menu entry and toolbar button is created and its pre-configured to call the run() method in this class when selected. This default implementation provided for you by the plugin builder is well documented, so please refer to the code for further advice.</source>
        <translation>Dies ist die Klasse, die Ihre Applikation mit QGIS verbindet. Sie werden sehen, dass bereits eine Vielzahl an Methoden bereitstehen, inklusive einiger Beispiele, etwa wie Raster- oder Vektorlayer in das Kartenfenster integriert werden können. Diese Klasse ist eine feste Instanz des QGIS Plugin-Interfaces, welche notwendiges Verhalten des Plugins definiert. Genau gesagt, enthält ein Plugin eine Reihe statischer Methoden und Klassenmitglieder, damit der QGIS Pluginmanager und der Pluginlader jedes Plugin identifizieren, einen passenden Menüeintrag erstellen kann usw. Beachten Sie, dass Sie auch mehrere Icons für die Werkzeugleiste sowie mehere Menüeinträge für ein einzelnes Plugin erstellen können. Standardmässig wird jedoch ein einzelnes Icon und ein Menüeintrag erstellt und so vorkonfiguriert, dass die Methode run() dieser Klasse bei ihrer Auswahl gestarted wird. Diese durch den Pluginbuilder bereitgestellte Standardimplementierung ist sehr gut dokumentiert. Beziehen Sie sich daher bitte auf den Quellcode für weitere Hinweise.</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="66"/>
        <source>This is a Qt designer &apos;ui&apos; file. It defines the look of the default plugin dialog without implementing any application logic. You can modify this form to suite your needs or completely remove it if your plugin does not need to display a user form (e.g. for custom MapTools).</source>
        <translation>Dies ist eine Qt Designer &apos;ui&apos; Datei. Sie definiert das Aussehen des Standard Plugindialogs ohne irgendeine Anwendungsfunktion. Sie können die Vorlage an Ihre Bedürfnisse anpassen oder auch löschen, wenn Ihr Plugin keinen Benutzerdialog braucht (z.B. für angepasste MapTools).</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="67"/>
        <source>This is the concrete class where application logic for the above mentioned dialog should go. The world is your oyster here really....</source>
        <translation>Dies ist eine feste Klasse in welche die Applikationstechnologie des oben beschriebenen Dialogs,eingefügt werden sollte Die Welt steht Ihnen an dieser Stelle absolut offen....</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="68"/>
        <source>This is the Qt4 resources file for your plugin. The Makefile generated for your plugin is all set up to compile the resource file so all you need to do is add your additional icons etc using the simple xml file format. Note the namespace used for all your resources e.g. (&apos;:/Homann/&apos;). It is important to use this prefix for all your resources. We suggest you include any other images and run time data in this resurce file too.</source>
        <translation>Dies ist die Qt4-Resourcendatei für Ihren plugin. Der für Ihr Plugin erstellte Makefile ist erstellt, um die Resourcendatei zu kompilieren. Alles was Sie hier tun müssen, ist die zusätzlichen Icons usw. mit Hilfe des einfachen XML-Formates zu ergänzen. Beachten Sie, die Namensräume für Ihre Resourcen z.B. (&apos;:/Homann/&apos;). Es ist wichtig diesen Prefix für all Ihre Resourcen zu verwenden. Wir schlagen vor, Sie bauen ein irgendwelche anderen Bilder und Laufzeitdaten in die Resourcendatei ein.</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="69"/>
        <source>This is the icon that will be used for your plugin menu entry and toolbar icon. Simply replace this icon with your own icon to make your plugin disctinctive from the rest.</source>
        <translation>Dies ist das Icon, dass für Ihr Pluginmenü und die Werkzeugleiste benutzt wird. Ersetzen Sie das Icon einfach durch ihr eigenes, um Ihr Plugin von den anderen zu unterscheiden.</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="70"/>
        <source>This file contains the documentation you are reading now!</source>
        <translation>Diese Datei enthält die Dokumentation, die Sie gerade lesen!</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="71"/>
        <source>Getting developer help:</source>
        <translation>Entwickler-Hilfe bekommen:</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="72"/>
        <source>For Questions and Comments regarding the plugin builder template and creating your features in QGIS using the plugin interface please contact us via:</source>
        <translation>Für Fragen und Kommentare in Bezug auf das &apos;Plugin-Builder&apos; Template und die Erstellung eigener Funktionen in QGIS mit Hilfe des Plugin-Interfaces kontaktieren Sie uns bitter unter:</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="73"/>
        <source>&lt;li&gt; the QGIS developers mailing list, or &lt;/li&gt;&lt;li&gt; IRC (#qgis on freenode.net)&lt;/li&gt;</source>
        <translation>&lt;li&gt; Die QGIS Entwickler-Mailingliste, oder &lt;/li&gt;&lt;li&gt; IRC (#qgis on freenode.net)&lt;/li&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="74"/>
        <source>QGIS is distributed under the Gnu Public License. If you create a useful plugin please consider contributing it back to the community.</source>
        <translation>QGIS ist veröffentlicht unter der GNU General Public License. Wenn Sie ein nützliches Plugin erstellt haben, überlegen Sie bitte, es der Community bereitzustellen.</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="75"/>
        <source>Have fun and thank you for choosing QGIS.</source>
        <translation>Viel Spass und danke, dass Sie sich für QGIS entschieden haben.</translation>
    </message>
</context>
<context>
    <name>MapCoordsDialogBase</name>
    <message>
        <location filename="../src/plugins/georeferencer/mapcoordsdialogbase.ui" line="13"/>
        <source>Enter map coordinates</source>
        <translation>Kartenkoordinaten eingeben</translation>
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
        <translation>&amp;OK</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/mapcoordsdialogbase.ui" line="172"/>
        <source>&amp;Cancel</source>
        <translation>&amp;Abbrechen</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/mapcoordsdialogbase.ui" line="28"/>
        <source>Enter X and Y coordinates which correspond with the selected point on the image. Alternatively, click the button with icon of a pencil and then click a corresponding point on map canvas of QGIS to fill in coordinates of that point.</source>
        <translation>Klicken Sie auf den &apos;aus Karte&apos; Knopf und wählen Sie dann den korrespondierenden Punkt in der Hauptkarte per Klick aus, um die Koordinaten von dort zu übernehmen. Alternativ, können Sie die Koordinaten auch manuell eingeben.</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/mapcoordsdialogbase.ui" line="137"/>
        <source> from map canvas</source>
        <translation> aus Karte</translation>
    </message>
</context>
<context>
    <name>OgrConverterGuiBase</name>
    <message>
        <location filename="../src/plugins/ogr_converter/ogrconverterguibase.ui" line="25"/>
        <source>OGR Layer Converter</source>
        <translation type="unfinished">OGR-Layer-Konverter</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/ogrconverterguibase.ui" line="40"/>
        <source>Source</source>
        <translation type="unfinished">Quelle</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/ogrconverterguibase.ui" line="183"/>
        <source>Format</source>
        <translation type="unfinished">Format</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/ogrconverterguibase.ui" line="88"/>
        <source>File</source>
        <translation type="unfinished">Datei</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/ogrconverterguibase.ui" line="95"/>
        <source>Directory</source>
        <translation type="unfinished">Verzeichnis</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/ogrconverterguibase.ui" line="102"/>
        <source>Remote source</source>
        <translation type="unfinished">Entfernte Quelle</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/ogrconverterguibase.ui" line="206"/>
        <source>Dataset</source>
        <translation type="unfinished">Datensatz</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/ogrconverterguibase.ui" line="226"/>
        <source>Browse</source>
        <translation type="unfinished">Durchsuchen</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/ogrconverterguibase.ui" line="239"/>
        <source>Layer</source>
        <translation type="unfinished">Layer</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/ogrconverterguibase.ui" line="171"/>
        <source>Target</source>
        <translation type="unfinished">Ziel</translation>
    </message>
</context>
<context>
    <name>OgrPlugin</name>
    <message>
        <location filename="../src/plugins/ogr_converter/plugin.cpp" line="57"/>
        <source>Run OGR Layer Converter</source>
        <translation type="unfinished">OGR-Layer-Konverter starten</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/plugin.cpp" line="67"/>
        <source>OG&amp;R Converter</source>
        <translation type="unfinished">OG&amp;R-Konverter</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/plugin.cpp" line="60"/>
        <source>Translates vector layers between formats supported by OGR library</source>
        <translation type="unfinished">Vektorlayer zwischen von der OGR-Bibliothek unterstützten Formaten umwandeln</translation>
    </message>
</context>
<context>
    <name>QFileDialog</name>
    <message>
        <location filename="../src/plugins/quick_print/quickprintgui.cpp" line="108"/>
        <source>Save experiment report to portable document format (.pdf)</source>
        <translation>Vorläufigen Bericht im &apos;Portable Document Format&apos; (.pdf) speichern</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="935"/>
        <source>Load layer properties from style file (.qml)</source>
        <translation>Layereigenschaften aus der Style-Datei (.qml) laden</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="1000"/>
        <source>Save layer properties as style file (.qml)</source>
        <translation>Layereigenschaften als Style-Datei (.qml) speichern</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="../src/core/qgsproviderregistry.cpp" line="92"/>
        <source>No Data Providers</source>
        <translation>Keine Datenlieferanten</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproviderregistry.cpp" line="87"/>
        <source>No Data Provider Plugins</source>
        <comment>No QGIS data provider plugins found in:</comment>
        <translation>Keine Datenlieferanten Plugins</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproviderregistry.cpp" line="89"/>
        <source>No vector layers can be loaded. Check your QGIS installation</source>
        <translation>Es können keine Vektorlayer geladen werden. Bitte QGIS Installation überprüfen</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproviderregistry.cpp" line="253"/>
        <source>No data provider plugins are available. No vector layers can be loaded</source>
        <translation>Keine Datenlieferanten-Plugins verfügbar. Es können keine Vektorlayer geladen werden</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3103"/>
        <source>QGis files (*.qgs)</source>
        <translation>QGIS Dateien (*.qgs)</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="770"/>
        <source> at line </source>
        <translation> in Zeile </translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="771"/>
        <source> column </source>
        <translation> Spalte </translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="777"/>
        <source> for file </source>
        <translation>Für Datei</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="931"/>
        <source>Unable to save to file </source>
        <translation>Datei kann nicht gespeichert werden</translation>
    </message>
    <message>
        <location filename="../src/core/qgssearchtreenode.cpp" line="287"/>
        <source>Referenced column wasn&apos;t found: </source>
        <translation>Die Referenzspalte wurde nicht gefunden: </translation>
    </message>
    <message>
        <location filename="../src/core/qgssearchtreenode.cpp" line="291"/>
        <source>Division by zero.</source>
        <translation>Division durch Null.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolselect.cpp" line="71"/>
        <source>No active layer</source>
        <translation>Keine aktiven Layer</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="154"/>
        <source>Band</source>
        <translation>Kanal</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="381"/>
        <source>action</source>
        <translation>Aktion</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="389"/>
        <source> features found</source>
        <translation> Objekte gefunden.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="393"/>
        <source> 1 feature found</source>
        <translation> 1 Objekt gefunden.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="399"/>
        <source>No features found</source>
        <translation>Keine Objekte gefunden</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="399"/>
        <source>No features were found in the active layer at the point you clicked</source>
        <translation>Keine Objekte im aktiven Layer am gewählten Punkt gefunden</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="424"/>
        <source>Could not identify objects on</source>
        <translation>Konnte Objekte nicht identifizieren auf</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="424"/>
        <source>because</source>
        <translation>weil</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="71"/>
        <source>New centroid</source>
        <translation>Neues Zentroid</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="238"/>
        <source>New point</source>
        <translation>Neuer Punkt</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="135"/>
        <source>New vertex</source>
        <translation>Neuer Vertex</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="238"/>
        <source>Undo last point</source>
        <translation>Undo letzter Punkt</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="238"/>
        <source>Close line</source>
        <translation>Linie schliessen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="601"/>
        <source>Select vertex</source>
        <translation>Vertex wählen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="317"/>
        <source>Select new position</source>
        <translation>Neue Position wählen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="471"/>
        <source>Select line segment</source>
        <translation>Liniensegment wählen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="456"/>
        <source>New vertex position</source>
        <translation>Neue Vertexposition</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="456"/>
        <source>Release</source>
        <translation>Freigeben</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="586"/>
        <source>Delete vertex</source>
        <translation>Lösche Vertex</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="586"/>
        <source>Release vertex</source>
        <translation>Vertex freigeben</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="870"/>
        <source>Select element</source>
        <translation>Element wählen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="662"/>
        <source>New location</source>
        <translation>Neue Location</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="747"/>
        <source>Release selected</source>
        <translation>Ausgewähltes freigeben</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="747"/>
        <source>Delete selected / select next</source>
        <translation>Auswahl löschen / nächstes wählen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="816"/>
        <source>Select position on line</source>
        <translation>Position auf Linie wählen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="837"/>
        <source>Split the line</source>
        <translation>Linie auftrennen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="837"/>
        <source>Release the line</source>
        <translation>Linie freigeben</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="853"/>
        <source>Select point on line</source>
        <translation>Punkt in der Mitte wählen</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="347"/>
        <source>Length</source>
        <translation>Länge</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="365"/>
        <source>Area</source>
        <translation>Fläche</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="770"/>
        <source>Project file read error: </source>
        <translation>Fehler beim Lesen der Projektdatei: </translation>
    </message>
    <message>
        <location filename="../src/core/qgslabelattributes.cpp" line="61"/>
        <source>Label</source>
        <translation>Beschriftung</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsleastsquares.cpp" line="34"/>
        <source>Fit to a linear transform requires at least 2 points.</source>
        <translation>Anpassung an eine lineare Transformation benötigt mindestens 2 Punkte.</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsleastsquares.cpp" line="76"/>
        <source>Fit to a Helmert transform requires at least 2 points.</source>
        <translation>Eine Helmert-Transformation benötigt mindestens 2 Punkte.</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsleastsquares.cpp" line="132"/>
        <source>Fit to an affine transform requires at least 4 points.</source>
        <translation>Anpassung an eine affine Transformation benötigt mindestens 4 Punkte.</translation>
    </message>
    <message>
        <location filename="../src/providers/gpx/gpsdata.cpp" line="375"/>
        <source>Couldn&apos;t open the data source: </source>
        <translation>Kann die Datenquelle nicht öffnen.</translation>
    </message>
    <message>
        <location filename="../src/providers/gpx/gpsdata.cpp" line="399"/>
        <source>Parse error at line </source>
        <translation>Interpretationsfehler in Linie</translation>
    </message>
    <message>
        <location filename="../src/providers/gpx/qgsgpxprovider.cpp" line="56"/>
        <source>GPS eXchange format provider</source>
        <translation>GPS eXchange Format Provider</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="310"/>
        <source>Caught a coordinate system exception while trying to transform a point. Unable to calculate line length.</source>
        <translation>Ein Problem beim Versuch einer Koordinatentransformation eines Punktes aus. Konnte daher die Linienlänge nicht berechnen.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="401"/>
        <source>Caught a coordinate system exception while trying to transform a point. Unable to calculate polygon area.</source>
        <translation>Ein Problem beim Versuch einer Koordinatentransformation eines Punktes aus. Konnte daher die Fläches des Polygons nicht berechnen.</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="162"/>
        <source>GRASS plugin</source>
        <translation>GRASS plugin</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="132"/>
        <source>QGIS couldn&apos;t find your GRASS installation.
Would you like to specify path (GISBASE) to your GRASS installation?</source>
        <translation>QGIS konnte Ihre GRASS-Installation nicht finden. Wollen Sie den Pfad zu Ihrer GRASS-Installation (GISBASE) angeben?</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="146"/>
        <source>Choose GRASS installation path (GISBASE)</source>
        <translation>Bitte wählen Sie einen GRASS-Installationspfad (GISBASE)</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="163"/>
        <source>GRASS data won&apos;t be available if GISBASE is not specified.</source>
        <translation>GRASS-Daten können nicht benutzt werden, wenn keine GISBASE definiert ist.</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="49"/>
        <source>CopyrightLabel</source>
        <translation>CopyrightLabel</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="50"/>
        <source>Draws copyright information</source>
        <translation>Zeichnet Urhebersrechtsinformationen</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfsplugin.cpp" line="31"/>
        <source>Version 0.1</source>
        <translation>Version 0.1</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="42"/>
        <source>Version 0.2</source>
        <translation>Version 0.2</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="43"/>
        <source>Loads and displays delimited text files containing x,y coordinates</source>
        <translation>Lädt und stellt Textdateien in CSV-Format, die x und y-Koordinaten haben, dar.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="159"/>
        <source>Add Delimited Text Layer</source>
        <translation>Layer aus Textdatei laden</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugin.cpp" line="57"/>
        <source>Georeferencer</source>
        <translation>Georeferenzierer</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugin.cpp" line="58"/>
        <source>Adding projection info to rasters</source>
        <translation>Fügt Projektionsinformationen zu Rasterdateien hinzu.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="52"/>
        <source>GPS Tools</source>
        <translation>GPS Werkzeuge</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="54"/>
        <source>Tools for loading and importing GPS data</source>
        <translation>Werkzeuge zum Laden und Importieren von GPS-Daten.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="831"/>
        <source>GRASS</source>
        <translation>GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="837"/>
        <source>GRASS layer</source>
        <translation>GRASS-Layer anzeigen und GRASS-Module auf Daten in GRASS Locations anwenden.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugin.cpp" line="41"/>
        <source>Graticule Creator</source>
        <translation>Gradnetz Generator</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugin.cpp" line="42"/>
        <source>Builds a graticule</source>
        <translation>Erstellt ein Gradnetz</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="58"/>
        <source>NorthArrow</source>
        <translation>Nordpfeil</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="59"/>
        <source>Displays a north arrow overlayed onto the map</source>
        <translation>Stelle einen Nordpfeil auf der Kartendarstellung dar.</translation>
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
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="60"/>
        <source>ScaleBar</source>
        <translation>Maßstab</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="61"/>
        <source>Draws a scale bar</source>
        <translation>Zeichnet einen Maßstab</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitplugin.cpp" line="37"/>
        <source>SPIT</source>
        <translation>SPIT</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitplugin.cpp" line="38"/>
        <source>Shapefile to PostgreSQL/PostGIS Import Tool</source>
        <translation>Werkzeug zum Importieren von Shapes in PostgreSQL/PostGIS</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfsplugin.cpp" line="29"/>
        <source>WFS plugin</source>
        <translation>WFS-Plugin</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfsplugin.cpp" line="30"/>
        <source>Adds WFS layers to the QGIS canvas</source>
        <translation>Fügt einen WFS-Layer zur Kartendarstellung hinzu.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="42"/>
        <source>Not a vector layer</source>
        <translation>Keine Vektorlayer</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="43"/>
        <source>The current layer is not a vector layer</source>
        <translation>Der aktuelle Layer ist kein Vektorlayer</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="72"/>
        <source>Layer cannot be added to</source>
        <translation>Der Layer kann nicht hinzugefügt werden zu</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="73"/>
        <source>The data provider for this layer does not support the addition of features.</source>
        <translation>Der Datenlieferant dieses Layers unterstützt das Hinzufügen von neuen Objekten nicht.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="49"/>
        <source>Layer not editable</source>
        <translation>Der Layer kann nicht bearbeitet werden</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="51"/>
        <source>Cannot edit the vector layer. To make it editable, go to the file item of the layer, right click and check &apos;Allow Editing&apos;.</source>
        <translation>Der Vektorlayer kann nicht geändert werden. Um ihn zu bearbeiten, klicken Sie bitte erst mit der rechten Maustaste auf den Dateieintrag des Layers und dann auf &apos;Bearbeitungsstatus umschalten&apos;.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolselect.cpp" line="74"/>
        <source>To select features, you must choose a vector layer by clicking on its name in the legend</source>
        <translation>Um Objekte zu selektieren, müssen Sie einen Vektorlayer durch anklicken in der Legende auswählen.</translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="199"/>
        <source>Python error</source>
        <translation type="unfinished">Python-Fehler</translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="497"/>
        <source>Couldn&apos;t load plugin </source>
        <translation>Plugin konnte nicht geladen werden</translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="501"/>
        <source> due an error when calling its classFactory() method</source>
        <translation> konnte durch einen Fehler beim Aufruf dessen classFactory()-Methode nicht laden</translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="505"/>
        <source> due an error when calling its initGui() method</source>
        <translation> konnte durch einen Fehler beim Aufruf dessen initGui()-Methode nicht laden.</translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="517"/>
        <source>Error while unloading plugin </source>
        <translation type="unfinished">Fehler beim Entladen des Plugins </translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="59"/>
        <source>2.5D shape type not supported</source>
        <translation>2,5D-Shapetyp wird nicht unterstützt</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="59"/>
        <source>Adding features to 2.5D shapetypes is not supported yet</source>
        <translation>Das Hinzufügen von 2,5D-Shape-Informationen wird zur Zeit nicht unterstützt</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="207"/>
        <source>Wrong editing tool</source>
        <translation type="unfinished">Falsches Bearbeitungswerkzeug</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="92"/>
        <source>Cannot apply the &apos;capture point&apos; tool on this vector layer</source>
        <translation>Das &apos;Punkt digitalisieren&apos;-Werkzeug kann nicht auf diesen Vektorlayer angewendet werden</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="65"/>
        <source>Coordinate transform error</source>
        <translation type="unfinished">Koordinatentransformationsfehler</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="66"/>
        <source>Cannot transform the point to the layers coordinate system</source>
        <translation>Konnte den Punkt nicht auf das Koordinatensystem des Layers transformieren.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="200"/>
        <source>Cannot apply the &apos;capture line&apos; tool on this vector layer</source>
        <translation>Das &apos;Linie digitalisieren&apos;-Werkzeug kann nicht auf diesen Vektorlayer angewendet werden</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="208"/>
        <source>Cannot apply the &apos;capture polygon&apos; tool on this vector layer</source>
        <translation>Das &apos;Polygon digitalisieren&apos;-Werkzeug kann nicht auf diesen Vektorlayer angewendet werden.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="435"/>
        <source>Error</source>
        <translation type="unfinished">Fehler</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="420"/>
        <source>Cannot add feature. Unknown WKB type</source>
        <translation type="unfinished">Konnte Objekt nicht hinzufügen. Unbekannter WKB-Typ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdisland.cpp" line="134"/>
        <source>Error, could not add island</source>
        <translation>Fehler beim Hinzufügen des Insel-Polygons</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="91"/>
        <source>A problem with geometry type occured</source>
        <translation>Es ist ein Problem mit dem Geometrietyp aufgetreten</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="95"/>
        <source>The inserted Ring is not closed</source>
        <translation type="unfinished">Der eingefügte Ring ist nicht geschlossen</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="99"/>
        <source>The inserted Ring is not a valid geometry</source>
        <translation type="unfinished">Der eingefügte Ring hat keine gültige Geometrie</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="103"/>
        <source>The inserted Ring crosses existing rings</source>
        <translation type="unfinished">Der eingefügte Ring überschneidet sich mit vorhandenen Ringen</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="107"/>
        <source>The inserted Ring is not contained in a feature</source>
        <translation type="unfinished">Der eingefügte Ring befindet sich nicht innerhalb des Objekts.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="111"/>
        <source>An unknown error occured</source>
        <translation type="unfinished">Ein unbekannter Fehler trat auf.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="113"/>
        <source>Error, could not add ring</source>
        <translation type="unfinished">Es ist ein Fehler beim Einfügen des Rings aufgetreten.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="647"/>
        <source> km2</source>
        <translation type="unfinished"> km2</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="652"/>
        <source> ha</source>
        <translation type="unfinished"> ha</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="657"/>
        <source> m2</source>
        <translation type="unfinished"> m2</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="685"/>
        <source> m</source>
        <translation type="unfinished"> m</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="670"/>
        <source> km</source>
        <translation type="unfinished"> km</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="675"/>
        <source> mm</source>
        <translation type="unfinished"> mm</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="680"/>
        <source> cm</source>
        <translation type="unfinished"> cm</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="694"/>
        <source> sq mile</source>
        <translation type="unfinished"> Quadratmeile</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="699"/>
        <source> sq ft</source>
        <translation type="unfinished"> Quadratfuß</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="706"/>
        <source> mile</source>
        <translation type="unfinished"> Meilen</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="712"/>
        <source> foot</source>
        <translation type="unfinished"> Fuß</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="714"/>
        <source> feet</source>
        <translation type="unfinished"> Fuß</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="721"/>
        <source> sq.deg.</source>
        <translation type="unfinished"> sq.deg.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="726"/>
        <source> degree</source>
        <translation type="unfinished"> Grad</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="728"/>
        <source> degrees</source>
        <translation type="unfinished"> Grad</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="732"/>
        <source> unknown</source>
        <translation type="unfinished"> unbekannt</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="294"/>
        <source>Received %1 of %2 bytes</source>
        <translation type="unfinished">%1 von %2 Bytes empfangen.</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="300"/>
        <source>Received %1 bytes (total unknown)</source>
        <translation type="unfinished">%1 Bytes empfangen (Gesamtzahl unbekannt)</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="412"/>
        <source>Not connected</source>
        <translation type="unfinished">Nicht verbunden</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="418"/>
        <source>Looking up &apos;%1&apos;</source>
        <translation type="unfinished">Löse &apos;%1&apos; auf</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="425"/>
        <source>Connecting to &apos;%1&apos;</source>
        <translation type="unfinished">Verbinde mit &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="432"/>
        <source>Sending request &apos;%1&apos;</source>
        <translation type="unfinished">Anfrage wird an &apos;%1&apos; gesandt</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="439"/>
        <source>Receiving reply</source>
        <translation type="unfinished">Emfange Antwort</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="445"/>
        <source>Response is complete</source>
        <translation type="unfinished">Antwort ist vollständig</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="451"/>
        <source>Closing down connection</source>
        <translation type="unfinished">Verbindung wird geschlossen</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="755"/>
        <source>Unable to open </source>
        <translation type="unfinished">Fehler beim Öffnen: </translation>
    </message>
    <message>
        <location filename="../src/core/qgssearchtreenode.cpp" line="253"/>
        <source>Regular expressions on numeric values don&apos;t make sense. Use comparison instead.</source>
        <translation type="unfinished">Reguläre Ausdrücke auf numerische Werte haben keinen Sinn. Bitte benutzen Sie Vergleichsoperatoren.</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="45"/>
        <source>PostgreSQL Geoprocessing</source>
        <translation>PostgreSQL-Geodatenverarbeitung</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="46"/>
        <source>Geoprocessing functions for working with PostgreSQL/PostGIS layers</source>
        <translation>Geodatenverarbeitungsfunktionen für PostgreSQL-/PostGIS-Layer</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="121"/>
        <source>Location: </source>
        <translation>Location: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="121"/>
        <source>&lt;br&gt;Mapset: </source>
        <translation>&lt;br&gt;Mapset: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="126"/>
        <source>&lt;b&gt;Raster&lt;/b&gt;</source>
        <translation type="unfinished">&lt;b&gt;Raster&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="135"/>
        <source>Cannot open raster header</source>
        <translation type="unfinished">Header des Rasters konnte nicht geöffnet werden.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="139"/>
        <source>Rows</source>
        <translation type="unfinished">Zeilen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="140"/>
        <source>Columns</source>
        <translation type="unfinished">Spalten</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="141"/>
        <source>N-S resolution</source>
        <translation type="unfinished">N-S Auflösung</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="142"/>
        <source>E-W resolution</source>
        <translation type="unfinished">O-W Auflösung</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="280"/>
        <source>North</source>
        <translation type="unfinished">Nord</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="282"/>
        <source>South</source>
        <translation type="unfinished">Süd</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="284"/>
        <source>East</source>
        <translation type="unfinished">Ost</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="286"/>
        <source>West</source>
        <translation type="unfinished">West</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="169"/>
        <source>Format</source>
        <translation type="unfinished">Format</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="180"/>
        <source>Minimum value</source>
        <translation type="unfinished">Minimalwert</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="181"/>
        <source>Maximum value</source>
        <translation type="unfinished">Maximalwert</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="192"/>
        <source>Data source</source>
        <translation type="unfinished">Datenquelle</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="197"/>
        <source>Data description</source>
        <translation type="unfinished">Datenbeschreibung</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="206"/>
        <source>Comments</source>
        <translation type="unfinished">Kommentare</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="221"/>
        <source>Categories</source>
        <translation type="unfinished">Kategorien</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="328"/>
        <source>&lt;b&gt;Vector&lt;/b&gt;</source>
        <translation type="unfinished">&lt;b&gt;Vektor&lt;/b;&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="254"/>
        <source>Points</source>
        <translation type="unfinished">Punkte</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="255"/>
        <source>Lines</source>
        <translation type="unfinished">Zeilen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="256"/>
        <source>Boundaries</source>
        <translation type="unfinished">Grenzen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="257"/>
        <source>Centroids</source>
        <translation>Zentroide</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="260"/>
        <source>Faces</source>
        <translation type="unfinished">Oberflächen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="261"/>
        <source>Kernels</source>
        <translation type="unfinished">Kerne</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="264"/>
        <source>Areas</source>
        <translation type="unfinished">Bereiche</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="265"/>
        <source>Islands</source>
        <translation type="unfinished">Inseln</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="289"/>
        <source>Top</source>
        <translation type="unfinished">Oben</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="290"/>
        <source>Bottom</source>
        <translation type="unfinished">Unten</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="293"/>
        <source>yes</source>
        <translation type="unfinished">ja</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="293"/>
        <source>no</source>
        <translation type="unfinished">nein</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="300"/>
        <source>History&lt;br&gt;</source>
        <translation type="unfinished">Geschichte&lt;br&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="329"/>
        <source>&lt;b&gt;Layer&lt;/b&gt;</source>
        <translation>&lt;b&gt;Layer&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="348"/>
        <source>Features</source>
        <translation>Objekte</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="357"/>
        <source>Driver</source>
        <translation type="unfinished">Treiber</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="358"/>
        <source>Database</source>
        <translation type="unfinished">Datenbank</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="359"/>
        <source>Table</source>
        <translation type="unfinished">Tabelle</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="360"/>
        <source>Key column</source>
        <translation type="unfinished">Schlüsselspalte</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="436"/>
        <source>GISBASE is not set.</source>
        <translation type="unfinished">GISBASE nicht gesetzt.</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="441"/>
        <source> is not a GRASS mapset.</source>
        <translation> ist keine GRASS-Mapset.</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="475"/>
        <source>Cannot start </source>
        <translation type="unfinished">Fehler beim Start von </translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="485"/>
        <source>Mapset is already in use.</source>
        <translation>Mapset wird bereits benutzt.</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="500"/>
        <source>Temporary directory </source>
        <translation type="unfinished">Temporäres Verzeichnis </translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="500"/>
        <source> exist but is not writable</source>
        <translation type="unfinished"> existiert, ist aber nicht beschreibbar.</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="506"/>
        <source>Cannot create temporary directory </source>
        <translation>Fehler beim Anlegen des temporären Verzeichnisses </translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="520"/>
        <source>Cannot create </source>
        <translation type="unfinished">Fehler beim Anlegen </translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="595"/>
        <source>Cannot remove mapset lock: </source>
        <translation>Kann Mapsetsperre nicht entfernen: </translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="1016"/>
        <source>Warning</source>
        <translation type="unfinished">Warnung</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="964"/>
        <source>Cannot read raster map region</source>
        <translation>Konnte &apos;region&apos; der Rasterkarte nicht lesen</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="981"/>
        <source>Cannot read vector map region</source>
        <translation>Konnte &apos;region der Vektorkarte nicht lesen</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="1017"/>
        <source>Cannot read region</source>
        <translation>Konnte &apos;region&apos; nicht lesen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2520"/>
        <source>Where is &apos;</source>
        <translation type="unfinished">Wo ist &apos;</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2520"/>
        <source>original location: </source>
        <translation>Original-Location: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="121"/>
        <source>To identify features, you must choose an active layer by clicking on its name in the legend</source>
        <translation>Um Objekte zu identifizieren müssen Sie einen Layer in der Legende auswählen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="117"/>
        <source>Location: </source>
        <comment>Metadata in GRASS Browser</comment>
        <translation>Location: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="117"/>
        <source>&lt;br&gt;Mapset: </source>
        <comment>Metadata in GRASS Browser</comment>
        <translation>&lt;br&gt;Mapset: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="435"/>
        <source>Could not remove polygon intersection</source>
        <translation>Konnte Polygon-Überscheidung (Intersection) nicht löschen</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintplugin.cpp" line="38"/>
        <source>Quick Print</source>
        <translation>Schnelles Drucken</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintplugin.cpp" line="40"/>
        <source>Quick Print is a plugin to quickly print a map with minimal effort.</source>
        <translation>Quick Print ist ein Plugin, um mal eben schnell und ohne großen Aufwandt eine Karte zu drucken.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="585"/>
        <source>The directory containing your dataset needs to be writeable!</source>
        <translation>Der Ordner mit den Daten muss beschreibbar sein!</translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="599"/>
        <source>Created default style file as </source>
        <translation>Standard Style-Datei erstellt als </translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="940"/>
        <source> is not writeable.</source>
        <translation type="unfinished"> kann nicht beschrieben werden.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="940"/>
        <source>Please adjust permissions (if possible) and try again.</source>
        <translation type="unfinished">Bitte passen Sie (wenn möglich) die Berechtigung und versuchen es erneut.</translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="64"/>
        <source>Couldn&apos;t load SIP module.</source>
        <translation type="unfinished">Das SIP-Modul konnte nicht geladen werden.</translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="79"/>
        <source>Python support will be disabled.</source>
        <translation type="unfinished">Die Python-Unterstützung wird abgeschaltet.</translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="72"/>
        <source>Couldn&apos;t load PyQt4.</source>
        <translation type="unfinished">PyQt4 konnte nicht geladen werden.</translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="79"/>
        <source>Couldn&apos;t load PyQGIS.</source>
        <translation type="unfinished">PyQGIS konnte nicht geladen werden.</translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="90"/>
        <source>An error has occured while executing Python code:</source>
        <translation type="unfinished">Fehler bei der Ausführung von Python-Code:</translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="194"/>
        <source>Python version:</source>
        <translation type="unfinished">Python-Version:</translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="195"/>
        <source>Python path:</source>
        <translation type="unfinished">Python-Pfad:</translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="185"/>
        <source>An error occured during execution of following code:</source>
        <translation type="unfinished">Fehler bei der Ausführung folgenden Codes:</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="386"/>
        <source>Uncatched fatal GRASS error</source>
        <translation type="unfinished">Nicht abgefangener fataler GRASS-Fehler.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="604"/>
        <source>ERROR: Failed to created default style file as %1 Check file permissions and retry.</source>
        <translation type="unfinished">FEHLER: Konnte die Datei %1 für den voreingestellten Stil nicht erzeugen.  Bitte überprüfen Sie die Zugriffrechte vor einem erneuten Versuch.</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapture.cpp" line="50"/>
        <source>Coordinate Capture</source>
        <translation type="unfinished">Koordinaten abgreifen</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapture.cpp" line="51"/>
        <source>Capture mouse coordinates in different CRS</source>
        <translation type="unfinished">Koordinaten in anderem KBS verfolgen</translation>
    </message>
    <message>
        <location filename="../src/core/composer/qgscomposerlegend.cpp" line="27"/>
        <source>Legend</source>
        <translation type="unfinished">Legende</translation>
    </message>
    <message>
        <location filename="../src/plugins/dxf2shp_converter/dxf2shpconverter.cpp" line="37"/>
        <source>Dxf2Shp Converter</source>
        <translation type="unfinished">Dxf2Shp-Konverter</translation>
    </message>
    <message>
        <location filename="../src/plugins/dxf2shp_converter/dxf2shpconverter.cpp" line="39"/>
        <source>Converts from dxf to shp file format</source>
        <translation type="unfinished">Wandelt von DXF- ins Shapeformat</translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsgridfilewriter.cpp" line="65"/>
        <source>Interpolating...</source>
        <translation type="unfinished">Interpoliere...</translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsgridfilewriter.cpp" line="65"/>
        <source>Abort</source>
        <translation type="unfinished">Abbrechen</translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsinterpolationplugin.cpp" line="22"/>
        <source>Interpolation plugin</source>
        <translation type="unfinished">Interpolationsplugin</translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsinterpolationplugin.cpp" line="23"/>
        <source>A plugin for interpolation based on vertices of a vector layer</source>
        <translation type="unfinished">Ein Plugin für die Stützpunktinterpolation von Vektorlayern.</translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsinterpolationplugin.cpp" line="24"/>
        <source>Version 0.001</source>
        <translation type="unfinished">Version 0.001</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/plugin.cpp" line="33"/>
        <source>OGR Layer Converter</source>
        <translation type="unfinished">OGR-Layer-Konverter</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/plugin.cpp" line="34"/>
        <source>Translates vector layers between formats supported by OGR library</source>
        <translation type="unfinished">Vektorlayer von einem in ein anderes von der OGR-Bibliothek unterstütztes Formats umwandeln</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolselect.cpp" line="133"/>
        <source>CRS Exception</source>
        <translation type="unfinished">KBS-Ausnahme</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolselect.cpp" line="134"/>
        <source>Selection extends beyond layer&apos;s coordinate system.</source>
        <translation type="unfinished">Auswahl außerhalb des Koordinatensystems des Layers.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="545"/>
        <source>Loading style file </source>
        <translation type="unfinished">Laden der Stildatei </translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="545"/>
        <source> failed because:</source>
        <translation type="unfinished">schlug fehl, weil:</translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="573"/>
        <source>Could not save symbology because:</source>
        <translation type="unfinished">Konnte Symbolik nicht speichern, weil:</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="1027"/>
        <source>Unable to save to file. Your project may be corrupted on disk. Try clearing some space on the volume and check file permissions before pressing save again.</source>
        <translation type="unfinished">Konnte nicht in Datei speichern. Ihr Projekt könnte auf der Festplatte defekt sein.  Versuchen Sie etwas Platz freizumachen und überprüfen Sie vor einem erneuten Versuch die Zugriffsrechte.</translation>
    </message>
    <message>
        <location filename="../src/app/qgspluginregistry.cpp" line="289"/>
        <source>Error Loading Plugin</source>
        <translation type="unfinished">Fehler beim Laden des Plugins</translation>
    </message>
    <message>
        <location filename="../src/app/qgspluginregistry.cpp" line="291"/>
        <source>There was an error loading a plugin.The following diagnostic information may help the QGIS developers resolve the issue:
%1.</source>
        <translation type="unfinished">Beim Laden eines Plugins trat ein Fehler auf.  Die folgenden Informationen könnten den QGIS-Entwicklern bei der Lösung des Problem helfen: %1.</translation>
    </message>
    <message>
        <location filename="../src/app/qgspluginregistry.cpp" line="411"/>
        <source>Error when reading metadata of plugin </source>
        <translation type="unfinished">Fehler beim Lesen der Plugin-Metadaten</translation>
    </message>
</context>
<context>
    <name>QgisApp</name>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="370"/>
        <source>Quantum GIS - </source>
        <translation>Quantum GIS -</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1801"/>
        <source>Version</source>
        <translation>Version</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2178"/>
        <source>is not a valid or recognized data source</source>
        <translation type="unfinished">ist keine zulässige oder erkannte Datenquelle</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5187"/>
        <source>Invalid Data Source</source>
        <translation type="unfinished">Ungültige Datenquelle</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3655"/>
        <source>No Layer Selected</source>
        <translation type="unfinished">Keinen Layer ausgewählt</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4194"/>
        <source>There is a new version of QGIS available</source>
        <translation type="unfinished">Eine neue Version von QGIS ist verfügbar</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4200"/>
        <source>You are running a development version of QGIS</source>
        <translation type="unfinished">Sie verwenden eine Entwicklungsversion von QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4204"/>
        <source>You are running the current version of QGIS</source>
        <translation type="unfinished">Sie verwenden die aktuelle Version von QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4209"/>
        <source>Would you like more information?</source>
        <translation type="unfinished">Wollen Sie mehr Information?</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4258"/>
        <source>QGIS Version Information</source>
        <translation type="unfinished">QGIS-Versionsinformationen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4230"/>
        <source>Unable to get current version information from server</source>
        <translation type="unfinished">Kann Informationen zu aktuellen Version nicht vom Server holen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4244"/>
        <source>Connection refused - server may be down</source>
        <translation type="unfinished">Verbindung abgelehnt - Server vielleicht heruntergefahren</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4247"/>
        <source>QGIS server was not found</source>
        <translation type="unfinished">QGIS Server nicht gefunden</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2271"/>
        <source>Invalid Layer</source>
        <translation type="unfinished">Ungültiger Layer</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2271"/>
        <source>%1 is an invalid layer and cannot be loaded.</source>
        <translation type="unfinished">%1 ist ein ungültiger Layer und kann nicht geladen werden.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3385"/>
        <source>Saved map image to</source>
        <translation type="unfinished">Kartenbild gespeichert in</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4696"/>
        <source>Extents: </source>
        <translation type="unfinished">Ausmasse:</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3684"/>
        <source>Problem deleting features</source>
        <translation type="unfinished">Problem beim Löschen der Objekte</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3685"/>
        <source>A problem occured during deletion of features</source>
        <translation type="unfinished">Beim Löschen der Objekte ist ein Problem aufgetreten</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3663"/>
        <source>No Vector Layer Selected</source>
        <translation type="unfinished">Es wurde kein Vektorlayer gewählt.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3664"/>
        <source>Deleting features only works on vector layers</source>
        <translation type="unfinished">Löschen von Objekten ist nur in Vektorlayern möglich</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3656"/>
        <source>To delete features, you must select a vector layer in the legend</source>
        <translation type="unfinished">Um Objekte zu löschen, muss ein Vektorlayer in der Legende gewählt werden</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1644"/>
        <source>Map legend that displays all the layers currently on the map canvas. Click on the check box to turn a layer on or off. Double click on a layer in the legend to customize its appearance and set other properties.</source>
        <translation>Legende, die alle im Kartenfester angezeigten Layer enthält. Bitte auf die Checkbox klicken, um einen Layer an- oder auszuschalten. Mit einem Doppelklick in der Legende kann die Erscheinung und sonstige Eigenschaften eines Layers festgelegt werden.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1584"/>
        <source>Map overview canvas. This canvas can be used to display a locator map that shows the current extent of the map canvas. The current extent is shown as a red rectangle. Any layer on the map can be added to the overview canvas.</source>
        <translation>Übersichtsfenster. Dieses Fenster kann benutzt werden um die momentane Ausdehnung des Kartenfensters darzustellen. Der momentane Ausschnitt ist als rotes Rechteck dargestellt. Jeder Layer in der Karte kann zum Übersichtsfenster hinzugefügt werden.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1155"/>
        <source>&amp;Plugins</source>
        <translation>&amp;Plugins</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1350"/>
        <source>Displays the current map scale</source>
        <translation>Zeigt den momentanen Kartenmassstab an</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1363"/>
        <source>Render</source>
        <translation>Zeichnen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1369"/>
        <source>When checked, the map layers are rendered in response to map navigation commands and other events. When not checked, no rendering is done. This allows you to add a large number of layers and symbolize them before rendering.</source>
        <translation>Wenn angewählt, werden die Kartenlayer abhängig von der Bedienung der Navigationsinstrumente, gezeichnet. Anderenfalls werden die Layer nicht gezeichnet. Dies erlaubt es, eine grosse Layeranzahl hinzuzufügen und das Aussehen der Layer vor dem Zeichnen zu setzen.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3027"/>
        <source>Choose a QGIS project file</source>
        <translation type="unfinished">Eine QGIS-Projektdatei wählen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3153"/>
        <source>Unable to save project</source>
        <translation type="unfinished">Projekt kann nicht gespeichert werden</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3154"/>
        <source>Unable to save project to </source>
        <translation type="unfinished">Projekt kann nicht gespeichert werden</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1370"/>
        <source>Toggle map rendering</source>
        <translation>Zeichnen der Karte einschalten</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2122"/>
        <source>Open an OGR Supported Vector Layer</source>
        <translation type="unfinished">Öffnen eines OGR-Vektorlayers</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2970"/>
        <source>QGIS Project Read Error</source>
        <translation type="unfinished">Fehler beim Lesen des QGIS-Projektes</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2972"/>
        <source>Try to find missing layers?</source>
        <translation type="unfinished">Versuchen, fehlende Layer zu finden?</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5093"/>
        <source>Open a GDAL Supported Raster Data Source</source>
        <translation type="unfinished">Öffnen einer GDAL-Rasterdatenquelle</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2749"/>
        <source>Save As</source>
        <translation type="unfinished">Speichern als</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2849"/>
        <source>Choose a QGIS project file to open</source>
        <translation type="unfinished">QGIS-Projektdatei zum Öffnen wählen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3146"/>
        <source>Saved project to:</source>
        <translation type="unfinished">Projekt wurde gespeichert in:</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="331"/>
        <source>Reading settings</source>
        <translation>Lese Einstellungen.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="334"/>
        <source>Setting up the GUI</source>
        <translation>Richte die Oberfläche ein</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="325"/>
        <source>Checking database</source>
        <translation>Überprüfe die Datenbank</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="394"/>
        <source>Restoring loaded plugins</source>
        <translation>Stelle die geladenen Plugins wieder her</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="399"/>
        <source>Initializing file filters</source>
        <translation>Initialisiere Dateifilter</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="424"/>
        <source>Restoring window state</source>
        <translation>Stelle Fensterstatus wieder her</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="428"/>
        <source>QGIS Ready!</source>
        <translation>QGIS ist startklar!</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="533"/>
        <source>&amp;New Project</source>
        <translation>&amp;Neues Projekt</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="534"/>
        <source>Ctrl+N</source>
        <comment>New Project</comment>
        <translation>Ctrl+N</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="535"/>
        <source>New Project</source>
        <translation>Neues Projekt</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="538"/>
        <source>&amp;Open Project...</source>
        <translation>Pr&amp;ojekt öffnen...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="539"/>
        <source>Ctrl+O</source>
        <comment>Open a Project</comment>
        <translation>Ctrl+O</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="540"/>
        <source>Open a Project</source>
        <translation>Projekt öffnen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="543"/>
        <source>&amp;Save Project</source>
        <translation>Projekt &amp;speichern</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="544"/>
        <source>Ctrl+S</source>
        <comment>Save Project</comment>
        <translation>Ctrl+S</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="545"/>
        <source>Save Project</source>
        <translation>Projekt speichern</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="548"/>
        <source>Save Project &amp;As...</source>
        <translation>Projekt speichern &amp;als...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="550"/>
        <source>Save Project under a new name</source>
        <translation>Projekt unter einem neuen Namen abspeichern.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="553"/>
        <source>Save as Image...</source>
        <translation>Bild speichern als...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="555"/>
        <source>Save map as image</source>
        <translation>Karte als Bild speichern</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="563"/>
        <source>Exit</source>
        <translation>Beenden</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="564"/>
        <source>Ctrl+Q</source>
        <comment>Exit QGIS</comment>
        <translation>Ctrl+Q</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="565"/>
        <source>Exit QGIS</source>
        <translation>Beende QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="759"/>
        <source>V</source>
        <comment>Add a Vector Layer</comment>
        <translation>V</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="760"/>
        <source>Add a Vector Layer</source>
        <translation>Vektorlayer hinzufügen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="764"/>
        <source>R</source>
        <comment>Add a Raster Layer</comment>
        <translation>R</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="765"/>
        <source>Add a Raster Layer</source>
        <translation>Rasterlayer hinzufügen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="769"/>
        <source>D</source>
        <comment>Add a PostGIS Layer</comment>
        <translation>D</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="770"/>
        <source>Add a PostGIS Layer</source>
        <translation>PostGIS-Layer hinzufügen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="753"/>
        <source>New Vector Layer...</source>
        <translation>Neuer Vektorlayer...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="754"/>
        <source>N</source>
        <comment>Create a New Vector Layer</comment>
        <translation>N</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="755"/>
        <source>Create a New Vector Layer</source>
        <translation>Neuen Vektorlayer erzeugen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="807"/>
        <source>Remove Layer</source>
        <translation>Layer löschen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="808"/>
        <source>Ctrl+D</source>
        <comment>Remove a Layer</comment>
        <translation>Ctrl+D</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="809"/>
        <source>Remove a Layer</source>
        <translation>Lösche einen Layer</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="825"/>
        <source>+</source>
        <comment>Show all layers in the overview map</comment>
        <translation>+</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="826"/>
        <source>Show all layers in the overview map</source>
        <translation>Zeige alle Layer in der Übersichtskarte</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="829"/>
        <source>Remove All From Overview</source>
        <translation>Alle aus Übersicht entfernen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="830"/>
        <source>-</source>
        <comment>Remove all layers from overview map</comment>
        <translation>-</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="831"/>
        <source>Remove all layers from overview map</source>
        <translation>Alle Ebnene aus der Übersichtskarte entfernen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="834"/>
        <source>Show All Layers</source>
        <translation>Alle Layer anzeigen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="835"/>
        <source>S</source>
        <comment>Show all layers</comment>
        <translation>S</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="836"/>
        <source>Show all layers</source>
        <translation>Alle Layer zeigen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="839"/>
        <source>Hide All Layers</source>
        <translation>Alle Layer ausblenden</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="840"/>
        <source>H</source>
        <comment>Hide all layers</comment>
        <translation>H</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="841"/>
        <source>Hide all layers</source>
        <translation>Alle Layer ausblenden</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="858"/>
        <source>Project Properties...</source>
        <translation>Projekteinstellungen...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="859"/>
        <source>P</source>
        <comment>Set project properties</comment>
        <translation>P</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="860"/>
        <source>Set project properties</source>
        <translation>Projekteigenschaften setzen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="863"/>
        <source>Options...</source>
        <translation>Optionen...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="865"/>
        <source>Change various QGIS options</source>
        <translation>Verschiedene QGIS-Einstellungen ändern</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="897"/>
        <source>Help Contents</source>
        <translation>Hilfe-Übersicht</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="901"/>
        <source>F1</source>
        <comment>Help Documentation</comment>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="903"/>
        <source>Help Documentation</source>
        <translation>Hilfe</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="908"/>
        <source>Ctrl+H</source>
        <comment>QGIS Home Page</comment>
        <translation>Ctrl+H</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="910"/>
        <source>QGIS Home Page</source>
        <translation>QGIS-Homepage</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="917"/>
        <source>About</source>
        <translation>Über</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="918"/>
        <source>About QGIS</source>
        <translation>Über QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="913"/>
        <source>Check Qgis Version</source>
        <translation>QGIS Version überprüfen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="914"/>
        <source>Check if your QGIS version is up to date (requires internet access)</source>
        <translation>Aktualität Ihre QGIS-Version überprüfen (erfordert Internetzugang)</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="746"/>
        <source>Refresh</source>
        <translation>Erneuern</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="747"/>
        <source>Ctrl+R</source>
        <comment>Refresh Map</comment>
        <translation>Ctrl+R</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="748"/>
        <source>Refresh Map</source>
        <translation>Karte neu zeichnen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="677"/>
        <source>Zoom In</source>
        <translation>Hineinzoomen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="676"/>
        <source>Ctrl++</source>
        <comment>Zoom In</comment>
        <translation>Ctrl++</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="682"/>
        <source>Zoom Out</source>
        <translation>Hinauszoomen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="681"/>
        <source>Ctrl+-</source>
        <comment>Zoom Out</comment>
        <translation>Ctrl+-</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="706"/>
        <source>Zoom Full</source>
        <translation>Volle Ausdehnung</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="707"/>
        <source>F</source>
        <comment>Zoom to Full Extents</comment>
        <translation>F</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="708"/>
        <source>Zoom to Full Extents</source>
        <translation>Auf die volle Ausdehnung herauszoomen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="671"/>
        <source>Pan Map</source>
        <translation>Karte verschieben</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="672"/>
        <source>Pan the map</source>
        <translation>Karte verschieben</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="721"/>
        <source>Zoom Last</source>
        <translation>Zur vorherigen Zoomeinstellung zurückkehren</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="723"/>
        <source>Zoom to Last Extent</source>
        <translation>Zur vorherigen Zoomeinstellung zurückkehren</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="713"/>
        <source>Zoom to Layer</source>
        <translation>Auf den Layer zoomen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="690"/>
        <source>Identify Features</source>
        <translation>Objekte abfragen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="691"/>
        <source>I</source>
        <comment>Click on features to identify them</comment>
        <translation>I</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="692"/>
        <source>Click on features to identify them</source>
        <translation>Klicken Sie auf ein Objekt, um Informationen dazu zuerhalten</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="686"/>
        <source>Select Features</source>
        <translation>Wähle Objekte aus</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="696"/>
        <source>Measure Line </source>
        <translation>Linie messen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="698"/>
        <source>Measure a Line</source>
        <translation>Strecke messen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="701"/>
        <source>Measure Area</source>
        <translation>Fläche messen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="703"/>
        <source>Measure an Area</source>
        <translation>Fläche messen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="743"/>
        <source>Show Bookmarks</source>
        <translation>Lesezeichen anzeigen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="742"/>
        <source>B</source>
        <comment>Show Bookmarks</comment>
        <translation>B</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="736"/>
        <source>New Bookmark...</source>
        <translation>Neues Lesezeichen...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="737"/>
        <source>Ctrl+B</source>
        <comment>New Bookmark</comment>
        <translation>Ctrl+B</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5450"/>
        <source>New Bookmark</source>
        <translation>Neues Lesezeichen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="780"/>
        <source>Add WMS Layer...</source>
        <translation>WMS-Layer hinzufügen...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="819"/>
        <source>O</source>
        <comment>Add current layer to overview map</comment>
        <translation>O</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="820"/>
        <source>Add current layer to overview map</source>
        <translation>Aktuellen Layer zur Übersicht hinzufügen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="848"/>
        <source>Open the plugin manager</source>
        <translation>Öffne den Pluginmanager</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="611"/>
        <source>Capture Point</source>
        <translation>Punkt digitalisieren</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="612"/>
        <source>.</source>
        <comment>Capture Points</comment>
        <translation>.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="613"/>
        <source>Capture Points</source>
        <translation>Punkte digitalisieren</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="617"/>
        <source>Capture Line</source>
        <translation>Linie digitalisieren</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="618"/>
        <source>/</source>
        <comment>Capture Lines</comment>
        <translation>/</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="619"/>
        <source>Capture Lines</source>
        <translation>Linien digitalisieren</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="623"/>
        <source>Capture Polygon</source>
        <translation>Polygon digitalisieren</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="624"/>
        <source>Ctrl+/</source>
        <comment>Capture Polygons</comment>
        <translation>Ctrl+/</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="625"/>
        <source>Capture Polygons</source>
        <translation>Polygon digitialisieren</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="640"/>
        <source>Delete Selected</source>
        <translation>Ausgewähltes löschen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="645"/>
        <source>Add Vertex</source>
        <translation>Stützpunkt hinzufügen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="655"/>
        <source>Delete Vertex</source>
        <translation>Stützpunkt löschen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="650"/>
        <source>Move Vertex</source>
        <translation>Stützpunkt verschieben</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1006"/>
        <source>&amp;File</source>
        <translation>&amp;Datei</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1010"/>
        <source>&amp;Open Recent Projects</source>
        <translation>Aktuelle Pr&amp;ojekte öffnen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1073"/>
        <source>&amp;View</source>
        <translation>&amp;Ansicht</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1106"/>
        <source>&amp;Layer</source>
        <translation>&amp;Layer</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1138"/>
        <source>&amp;Settings</source>
        <translation>&amp;Einstellungen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1177"/>
        <source>&amp;Help</source>
        <translation>&amp;Hilfe</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1199"/>
        <source>File</source>
        <translation>Datei</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1216"/>
        <source>Manage Layers</source>
        <translation>Layer koordinieren</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1282"/>
        <source>Help</source>
        <translation>Hilfe</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1227"/>
        <source>Digitizing</source>
        <translation>Digitalisierung</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1248"/>
        <source>Map Navigation</source>
        <translation>Kartennavigation</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1262"/>
        <source>Attributes</source>
        <translation>Attribute</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1276"/>
        <source>Plugins</source>
        <translation>Plugins</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1398"/>
        <source>Ready</source>
        <translation>Fertig</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1803"/>
        <source>New features</source>
        <translation>Neue Objekte</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2988"/>
        <source>Unable to open project</source>
        <translation type="unfinished">Kann das Projekt nicht öffnen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3160"/>
        <source>Unable to save project </source>
        <translation type="unfinished">Kann das Projekt nicht speichern </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3218"/>
        <source>QGIS: Unable to load project</source>
        <translation type="unfinished">QGIS: Kann das Projekt nicht laden.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3219"/>
        <source>Unable to load project </source>
        <translation type="unfinished">Kann das Projekt nicht laden.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4218"/>
        <source>QGIS - Changes in SVN Since Last Release</source>
        <translation type="unfinished">QGIS - Änderungen in SVN seit dem letzten Release</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5267"/>
        <source>Layer is not valid</source>
        <translation type="unfinished">Layer ist ungültig</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5268"/>
        <source>The layer is not a valid layer and can not be added to the map</source>
        <translation type="unfinished">Der Layer ist ungültig und kann daher nicht zum Kartenfenster hinzugefügt werden.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4476"/>
        <source>Save?</source>
        <translation type="unfinished">Speichern?</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5186"/>
        <source> is not a valid or recognized raster data source</source>
        <translation type="unfinished"> ist keine gültige Rasterdatenquelle.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5349"/>
        <source> is not a supported raster data source</source>
        <translation type="unfinished"> ist kein unterstütztes Rasterdatenformat.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5354"/>
        <source>Unsupported Data Source</source>
        <translation type="unfinished">Nicht unterstütztes Datenformat</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5451"/>
        <source>Enter a name for the new bookmark:</source>
        <translation type="unfinished">Bitte geben Sie einen Namen für das Lesenzeichen ein:</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5468"/>
        <source>Error</source>
        <translation type="unfinished">Fehler</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5468"/>
        <source>Unable to create the bookmark. Your user database may be missing or corrupted</source>
        <translation type="unfinished">Kann das Lesezeichen nicht erstellen. Ihre Datenbank scheint zu fehlen oder ist kaputt.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="593"/>
        <source>Cut Features</source>
        <translation>Ausgewählte Objekte ausschneiden</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="595"/>
        <source>Cut selected features</source>
        <translation>Ausgewählte Objekte ausschneiden</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="599"/>
        <source>Copy Features</source>
        <translation>Objekte kopieren</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="601"/>
        <source>Copy selected features</source>
        <translation>Ausgewählte Objekte kopieren</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="605"/>
        <source>Paste Features</source>
        <translation>Objekte einfügen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="607"/>
        <source>Paste selected features</source>
        <translation>Ausgewählte Objekte einfügen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="899"/>
        <source>Ctrl+?</source>
        <comment>Help Documentation (Mac)</comment>
        <translation>Ctrl+?</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4250"/>
        <source>Network error while communicating with server</source>
        <translation type="unfinished">Es trat ein Netzwerkfehler während der Kommunikation zum Server auf.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4253"/>
        <source>Unknown network socket error</source>
        <translation type="unfinished">Unbekannter Netzwerkfehler (Socketfehler)</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4258"/>
        <source>Unable to communicate with QGIS Version server</source>
        <translation type="unfinished">Kann nicht mit dem QGIS-Server kommunizieren.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="383"/>
        <source>Checking provider plugins</source>
        <translation type="unfinished">Provider-Plugins werden geprüft</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="387"/>
        <source>Starting Python</source>
        <translation type="unfinished">Python wird gestartet</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3670"/>
        <source>Provider does not support deletion</source>
        <translation type="unfinished">Provider unterstützt keine Löschoperationen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3671"/>
        <source>Data provider does not support deleting features</source>
        <translation type="unfinished">Der Provider hat nicht die Möglichkeit, Objekte zu löschen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3677"/>
        <source>Layer not editable</source>
        <translation type="unfinished">Der Layer kann nicht bearbeitet werden</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3678"/>
        <source>The current layer is not editable. Choose &apos;Start editing&apos; in the digitizing toolbar.</source>
        <translation type="unfinished">Der aktuelle Layer kann nicht bearbeitet werden. Bitte wählen Sie &apos;Bearbeitungsstatus umschalten&apos; aus der Digitalisierwerkzeugleiste.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="791"/>
        <source>Toggle editing</source>
        <translation type="unfinished">Bearbeitungsstatus umschalten</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="792"/>
        <source>Toggles the editing state of the current layer</source>
        <translation>Bearbeitungsstatus des aktuellen Layers umschalten</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="660"/>
        <source>Add Ring</source>
        <translation type="unfinished">Ring hinzufügen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="664"/>
        <source>Add Island</source>
        <translation>Insel hinzufügen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="665"/>
        <source>Add Island to multipolygon</source>
        <translation type="unfinished">Insel in Multipolygon einfügen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1336"/>
        <source>Scale </source>
        <translation type="unfinished">Maßstab </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1351"/>
        <source>Current map scale (formatted as x:y)</source>
        <translation type="unfinished">Aktueller Kartenmaßstab (x:y formatiert)</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4683"/>
        <source>Map coordinates at mouse cursor position</source>
        <translation type="unfinished">Kartenkoordinaten beim Mauszeiger</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4008"/>
        <source>Invalid scale</source>
        <translation type="unfinished">Ungültiger Maßstab</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4477"/>
        <source>Do you want to save the current project?</source>
        <translation type="unfinished">Wollen Sie das aktuelle Projekt speichern?</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="630"/>
        <source>Move Feature</source>
        <translation>Objekt verschieben</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="635"/>
        <source>Split Features</source>
        <translation>Objekte trennen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="731"/>
        <source>Map Tips</source>
        <translation>Kartenhinweise</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="732"/>
        <source>Show information about a feature when the mouse is hovered over it</source>
        <translation>Zeige Informationen zu einem Objekt, wenn die Maus darüber fährt</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1337"/>
        <source>Current map scale</source>
        <translation>Aktueller Kartenmaßstab</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5482"/>
        <source>Project file is older</source>
        <translation>Projektdatei ist älter</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5484"/>
        <source>&lt;p&gt;This project file was saved by an older version of QGIS.</source>
        <translation>&lt;p&gt;Diese Projektdatei wurde von einer älteren QGIS Version abgespeichert.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5486"/>
        <source> When saving this project file, QGIS will update it to the latest version, possibly rendering it useless for older versions of QGIS.</source>
        <translation> Wenn Sie diese Projektdatei speichern, wird QGIS es auf die neueste Version updaten und dadurch möglicherweise  nutzlos für ältere Versionen machen.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5489"/>
        <source>&lt;p&gt;Even though QGIS developers try to maintain backwards compatibility, some of the information from the old project file might be lost.</source>
        <translation>&lt;p&gt;Obwohl QGIS-Entwickler versuchen, Rückwärtskompatibilität zu gewährleisten, können einige Informationen der Projektdatei verloren gehen.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5491"/>
        <source> To improve the quality of QGIS, we appreciate if you file a bug report at %3.</source>
        <translation> Um die Qualität von QGIS zu verbessern, möchten wir Sie bitten, einen Fehlerreport zu erstellen unter %3.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5493"/>
        <source> Be sure to include the old project file, and state the version of QGIS you used to discover the error.</source>
        <translation> Stellen Sie sicher, dass die alte Projektdatei und die QGIS Version, bei der der Fehler auftritt angegeben sind, um den Fehler zu finden.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5495"/>
        <source>&lt;p&gt;To remove this warning when opening an older project file, uncheck the box &apos;%5&apos; in the %4 menu.</source>
        <translation>&lt;p&gt;Um diese Warnung beim Öffnen einer alten Projektdatei abzustellen, deaktivieren Sie die Box &apos;%5&apos; im Menü %4.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5495"/>
        <source>&lt;p&gt;Version of the project file: %1&lt;br&gt;Current version of QGIS: %2</source>
        <translation>&lt;p&gt;Version der Projektdatei: %1&lt;br&gt;Aktueller QGIS Version: %2</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5499"/>
        <source>&lt;tt&gt;Settings:Options:General&lt;/tt&gt;</source>
        <comment>Menu path to setting options</comment>
        <translation>&lt;tt&gt;Einstellungen:Optionen:Allgemein&lt;/tt&gt;</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5500"/>
        <source>Warn me when opening a project file saved with an older version of QGIS</source>
        <translation>Warne mich beim Öffnen einer Projektdatei, die mit einer älteren QGIS Version erstellt wurde</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="855"/>
        <source>Toggle fullscreen mode</source>
        <translation type="unfinished">Vollbildmodus umschalten</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1383"/>
        <source>Resource Location Error</source>
        <translation type="unfinished">Resource nicht gefunden</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1384"/>
        <source>Error reading icon resources from: 
 %1
 Quitting...</source>
        <translation>Fehler beim Lesen des Icrons aus: 
 %1 
 Abbruch...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1593"/>
        <source>Overview</source>
        <translation type="unfinished">Übersicht</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1645"/>
        <source>Legend</source>
        <translation type="unfinished">Legende</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1781"/>
        <source>You are using QGIS version %1 built against code revision %2.</source>
        <translation type="unfinished">Sie benutzen QGIS Version %1 mit dem Codestand %2.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1785"/>
        <source> This copy of QGIS has been built with PostgreSQL support.</source>
        <translation type="unfinished">Diese QGIS-Kopie unterstützt PostgreSQL.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1788"/>
        <source> This copy of QGIS has been built without PostgreSQL support.</source>
        <translation type="unfinished">Diese QGIS-Kopie unterstützt PostgreSQL nicht.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1792"/>
        <source>
This binary was compiled against Qt %1,and is currently running against Qt %2</source>
        <translation type="unfinished">Es wurde mit Qt %1 kompiliert und läuft gerade mit Qt %2</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1300"/>
        <source>Progress bar that displays the status of rendering layers and other time-intensive operations</source>
        <translation>Fortschrittsanzeige für das Zeichnen von Layern und andere zeitintensive Operationen.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1326"/>
        <source>Shows the map coordinates at the current cursor position. The display is continuously updated as the mouse is moved.</source>
        <translation>Zeigt die Kartenkoordinate der aktuellen Cursorposition. Die Anzeige wird während der Mausbewegung laufend aktualisiert.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1360"/>
        <source>Stop map rendering</source>
        <translation type="unfinished">Zeichnen der Karte abbrechen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1536"/>
        <source>Map canvas. This is where raster and vector layers are displayed when added to the map</source>
        <translation type="unfinished">Kartenansicht.  Hier werden Raster- und Vektorlayer angezeigt, wenn sie der Karte hinzugefügt werden.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="869"/>
        <source>Custom CRS...</source>
        <translation>Benutzerkoordinatenbezugssystem...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="871"/>
        <source>Manage custom coordinate reference systems</source>
        <translation type="unfinished">Benutzerkoordinatenbezugssysteme bearbeiten</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1313"/>
        <source>Toggle extents and mouse position display</source>
        <translation type="unfinished">Grenzen- und Mauspositionsanzeige umschalten</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1392"/>
        <source>This icon shows whether on the fly coordinate reference system transformation is enabled or not. Click the icon to bring up the project properties dialog to alter this behaviour.</source>
        <translation type="unfinished">Diese Icon zeigt an, ob On-The-Fly-Transformation des Koordinatenbezugssystem aktiv ist. Anklicken, um dies in den Projektionseigenschaften zu ändern.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1394"/>
        <source>CRS status - Click to open coordinate reference system dialog</source>
        <translation type="unfinished">KBS-Status - Klicken um den Dialog zum Koordinatenbezugssystem zu öffnen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4768"/>
        <source>Maptips require an active layer</source>
        <translation type="unfinished">Kartentipps erfordern einen aktuellen Layer.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="315"/>
        <source>Multiple Instances of QgisApp</source>
        <translation type="unfinished">Mehrere QgisApp-Instanzen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="316"/>
        <source>Multiple instances of Quantum GIS application object detected.
Please contact the developers.
</source>
        <translation type="unfinished">Mehrere Instanzen der Quantum GIS-Application wurden festgestellt.
Bitte kontaktieren Sie die Entwickler.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="549"/>
        <source>Shift+Ctrl+S</source>
        <comment>Save Project under a new name</comment>
        <translation type="unfinished">Projekt unter anderem Namen speichern</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="558"/>
        <source>&amp;Print Composer</source>
        <translation type="unfinished">&amp;Druckzusammenstellung</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="559"/>
        <source>Ctrl+P</source>
        <comment>Print Composer</comment>
        <translation type="unfinished">Ctrl+P</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="560"/>
        <source>Print Composer</source>
        <translation type="unfinished">Druckzusammenstellung</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="572"/>
        <source>&amp;Undo</source>
        <translation type="unfinished">&amp;Rückgängig</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="573"/>
        <source>Ctrl+Z</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="574"/>
        <source>Undo the last operation</source>
        <translation type="unfinished">Die letzte Operation rückgängig machen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="577"/>
        <source>Cu&amp;t</source>
        <translation type="unfinished">&amp;Ausschneiden</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="594"/>
        <source>Ctrl+X</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="579"/>
        <source>Cut the current selection&apos;s contents to the clipboard</source>
        <translation type="unfinished">Aktuelle Auswahl in Zwischenablage verschieben</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="582"/>
        <source>&amp;Copy</source>
        <translation type="unfinished">&amp;Kopieren</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="600"/>
        <source>Ctrl+C</source>
        <translation type="unfinished">Ctrl+C</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="584"/>
        <source>Copy the current selection&apos;s contents to the clipboard</source>
        <translation type="unfinished">Aktuelle Auswahl in die Zwischenablage kopieren</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="587"/>
        <source>&amp;Paste</source>
        <translation type="unfinished">&amp;Einfügen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="606"/>
        <source>Ctrl+V</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="589"/>
        <source>Paste the clipboard&apos;s contents into the current selection</source>
        <translation type="unfinished">Zwischenablagen in die aktuelle Auswahl übernehmen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="697"/>
        <source>M</source>
        <comment>Measure a Line</comment>
        <translation type="unfinished">Linie messen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="702"/>
        <source>J</source>
        <comment>Measure an Area</comment>
        <translation type="unfinished">Fläche messen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="718"/>
        <source>Zoom to Selection</source>
        <translation type="unfinished">Zur Auswahl zoomen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="717"/>
        <source>Ctrl+J</source>
        <comment>Zoom to Selection</comment>
        <translation type="unfinished">Ctrl+J</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="726"/>
        <source>Zoom Actual Size</source>
        <translation type="unfinished">Auf tatsächliche Größe zoomen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="727"/>
        <source>Zoom to Actual Size</source>
        <translation type="unfinished">Auf tatsächliche Größe zoomen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="758"/>
        <source>Add Vector Layer...</source>
        <translation type="unfinished">Vektorlayer hinzufügen...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="763"/>
        <source>Add Raster Layer...</source>
        <translation type="unfinished">Rasterlayer hinzufügen...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="768"/>
        <source>Add PostGIS Layer...</source>
        <translation type="unfinished">PostGIS-Layer hinzufügen...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="781"/>
        <source>W</source>
        <comment>Add a Web Mapping Server Layer</comment>
        <translation type="unfinished">W</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="782"/>
        <source>Add a Web Mapping Server Layer</source>
        <translation type="unfinished">WMS-Layer hinzufügen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="787"/>
        <source>Open Attribute Table</source>
        <translation type="unfinished">Attributetabelle öffnen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="797"/>
        <source>Save as Shapefile...</source>
        <translation type="unfinished">Als Shape-Datei speichern...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="798"/>
        <source>Save the current layer as a shapefile</source>
        <translation type="unfinished">Aktuellen Layer als Shape-Datei speichern</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="802"/>
        <source>Save Selection as Shapefile...</source>
        <translation type="unfinished">Auswahl als Shape-Datei speichern...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="803"/>
        <source>Save the selection as a shapefile</source>
        <translation type="unfinished">Auswahl als Shape-Datei speichern...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="813"/>
        <source>Properties...</source>
        <translation type="unfinished">Eigenschaften...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="814"/>
        <source>Set properties of the current layer</source>
        <translation type="unfinished">Eigenschaften des aktuellen Layers setzen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="818"/>
        <source>Add to Overview</source>
        <translation type="unfinished">Zur Übersicht hinzufügen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="824"/>
        <source>Add All to Overview</source>
        <translation type="unfinished">Alle zur Übersicht hinzufügen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="846"/>
        <source>Manage Plugins...</source>
        <translation type="unfinished">Plugins verwalten...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="853"/>
        <source>Toggle Full Screen Mode</source>
        <translation type="unfinished">Auf Vollbildmodus schalten</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="854"/>
        <source>Ctrl-F</source>
        <comment>Toggle fullscreen mode</comment>
        <translation type="unfinished">Vollbildmodus umschalten</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="878"/>
        <source>Minimize</source>
        <translation type="unfinished">Minimieren</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="879"/>
        <source>Ctrl+M</source>
        <comment>Minimize Window</comment>
        <translation type="unfinished">Ctrl+M</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="880"/>
        <source>Minimizes the active window to the dock</source>
        <translation type="unfinished">Minimiert das aktive Fenster ins Dock</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="883"/>
        <source>Zoom</source>
        <translation type="unfinished">Zoom</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="884"/>
        <source>Toggles between a predefined size and the window size set by the user</source>
        <translation type="unfinished">Schaltet zwischen voreingestellter und vom Benutzer bestimmten Fenstergröße um</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="887"/>
        <source>Bring All to Front</source>
        <translation type="unfinished">Alle in den Vordergrund bringen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="888"/>
        <source>Bring forward all open windows</source>
        <translation type="unfinished">Alle geöffneten Fenster vorholen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1034"/>
        <source>&amp;Edit</source>
        <translation type="unfinished">&amp;Bearbeiten</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1068"/>
        <source>Panels</source>
        <translation type="unfinished">Bedienfelder</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1069"/>
        <source>Toolbars</source>
        <translation type="unfinished">Werkzeugkästen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1164"/>
        <source>&amp;Window</source>
        <translation type="unfinished">&amp;Fenster</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3102"/>
        <source>Choose a file name to save the QGIS project file as</source>
        <translation type="unfinished">Name für zu speichernden QGIS-Projektdatei wählen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3343"/>
        <source>Choose a file name to save the map image as</source>
        <translation type="unfinished">Name für Datei zum Speichern des Kartenabbilds wählen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3917"/>
        <source>Start editing failed</source>
        <translation type="unfinished">Bearbeitungsbeginn schlug fehl</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3917"/>
        <source>Provider cannot be opened for editing</source>
        <translation type="unfinished">Lieferant kann nicht zum Bearbeiten geöffnet werden</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3925"/>
        <source>Stop editing</source>
        <translation type="unfinished">Bearbeitung beenden</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3926"/>
        <source>Do you want to save the changes to layer %1?</source>
        <translation type="unfinished">Wollen Sie die Änderung am Layer %1 speichern?</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3936"/>
        <source>Could not commit changes to layer %1

Errors:  %2
</source>
        <translation type="unfinished">Änderungen am Layer %1 konnten nicht gespeichern werden
<byte value="x9"/>
Fehler: %2
</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3947"/>
        <source>Problems during roll back</source>
        <translation type="unfinished">Probleme beim Zurücknehmen der Änderungen</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4135"/>
        <source>Python Console</source>
        <translation type="unfinished">Python-Konsole</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4676"/>
        <source>Map coordinates for the current view extents</source>
        <translation type="unfinished">Kartenkoordinaten für den aktuell sichtbaren Ausschnitt</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1806"/>
        <source>This release candidate includes over 265 bug fixes and enchancements over the QGIS 0.11.0 release. In addition we have added the following new features:</source>
        <translation type="unfinished">Dieser Release-Kandidate beinhalted über 265 Fehlerkorrekutren und Erweiterung gegenüber der Version 0.11.0. Zusätzlich haben wir folgende Funktionen ergänzt:</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1809"/>
        <source>HIG Compliance improvements for Windows / Mac OS X / KDE / Gnome</source>
        <translation type="unfinished">Verbesserung der Einhaltung der Richtlinien für Benutzerschnittstellen von Windows / Mac OS X / KDE / GNOME</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1812"/>
        <source>Saving a vector layer or subset of that layer to disk with a different Coordinate Reference System to the original.</source>
        <translation type="unfinished">Ganze oder Ausschnitte von Vektorlayern mit einem geänderten Koordinatenreferenzsystem speichern.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1815"/>
        <source>Advanced topological editing of vector data.</source>
        <translation type="unfinished">Erweiterte topologische Bearbeitung von Vektordaten.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1818"/>
        <source>Single click selection of vector features.</source>
        <translation type="unfinished">Auswahl von Vektorobjekten mit einem Klick.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1821"/>
        <source>Many improvements to raster rendering and support for building pyramids external to the raster file.</source>
        <translation type="unfinished">Viele Verbesserungen in der Rasterdarstellung und Unterstützung für externen Pyramiden.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1824"/>
        <source>Overhaul of the map composer for much improved printing support.</source>
        <translation type="unfinished">Überholung der Druckzusammenstellung für stark verbesserte Druckunterstützung.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1827"/>
        <source>A new &apos;coordinate capture&apos; plugin was added that lets you click on the map and then cut &amp; paste the coordinates to and from the clipboard.</source>
        <translation type="unfinished">Ein neues Plugin zum &apos;Koordinaten abgreifen&apos; wurde hinzugefügt mit dem man auf die Karte klicken und dann die Koordinate in die oder aus der Zwischenablage übernehmen kann.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1830"/>
        <source>A new plugin for converting between OGR supported formats was added.</source>
        <translation type="unfinished">Ein neues Plugin wurde hinzugefügt, das zwischen durch OGR-unterstützten Formaten konvertieren kann.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1833"/>
        <source>A new plugin for converting from DXF files to shapefiles was added.</source>
        <translation type="unfinished">Ein neues Plugin wurde hinzugefügt, das DXF- in Shapedatei konvertiert.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1836"/>
        <source>A new plugin was added for interpolating point features into ASCII grid layers.</source>
        <translation type="unfinished">Ein neues Plugin wurde hinzugefügt, dass Punktobjekte in ASCII-Gridlayern interpoliert.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1842"/>
        <source>Plugin toolbar positions are now correctly saved when the application is closed.</source>
        <translation type="unfinished">Die Toolbarpositionen werden nun bei Programmende richtig gespeichert.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1845"/>
        <source>In the WMS client, WMS standards support has been improved.</source>
        <translation type="unfinished">Im WMS-Client wurde die Standardkonformität verbessert.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1848"/>
        <source>Complete API revision - we now have a stable API following well defined naming conventions.</source>
        <translation type="unfinished">Komplette Überarbeitung der API - wir haben nun eine stabile API, die wohldefinierten Namenskonventionen folgt.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1851"/>
        <source>Ported all GDAL/OGR and GEOS usage to use C APIs only.</source>
        <translation type="unfinished">GDAL/OGR und GEOS werden nur noch über die C-APIs angesprochen.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1839"/>
        <source>The python plugin installer was completely overhauled, the new version having many improvements, including checking that the version of QGIS running will support a plugin that is being installed.</source>
        <translation type="unfinished">Das Plugin zur Installation von Python-Plugins wurde komplett überholt.  Die neue Version hat viele Verbesserungen, u.a. prüft es, ob die laufende QGIS-Version ein Plugin unterstützt, dass installiert werden soll.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1854"/>
        <source>Vector editing overhaul - handling of geometry and attribute edit transactions is now handled transparently in one place.</source>
        <translation type="unfinished">Vektorbearbeitung überholt - Bearbeitungstransaktionen werden nun transparent an einer Stelle behandelt.</translation>
    </message>
</context>
<context>
    <name>QgisAppBase</name>
    <message>
        <location filename="../src/ui/qgisappbase.ui" line="13"/>
        <source>QGIS</source>
        <translation type="unfinished">QGIS</translation>
    </message>
</context>
<context>
    <name>QgsAbout</name>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="13"/>
        <source>About Quantum GIS</source>
        <translation>Über Quantum GIS</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="230"/>
        <source>Ok</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="26"/>
        <source>About</source>
        <translation>Über</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="65"/>
        <source>Version</source>
        <translation type="unfinished">Version</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="152"/>
        <source>What&apos;s New</source>
        <translation>Was ist neu</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="129"/>
        <source>QGIS Home Page</source>
        <translation>QGIS Homepage</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="189"/>
        <source>Providers</source>
        <translation>Datenlieferant</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="175"/>
        <source>Developers</source>
        <translation>Entwickler</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="199"/>
        <source>Sponsors</source>
        <translation>Sponsoren</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="91"/>
        <source>Quantum GIS is licensed under the GNU General Public License</source>
        <translation type="unfinished">Quantum GIS ist unter der GNU General Public License lizenziert</translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="139"/>
        <source>Join our user mailing list</source>
        <translation>Abonnieren Sie unsere Mailingliste</translation>
    </message>
    <message>
        <location filename="../src/app/qgsabout.cpp" line="114"/>
        <source>&lt;p&gt;The following have sponsored QGIS by contributing money to fund development and other project costs&lt;/p&gt;</source>
        <translation type="unfinished">&lt;p&gt;QGIS wurde durch Geldspenden für Entwicklungs- und andere Projektkosten unterstützt durch&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/app/qgsabout.cpp" line="116"/>
        <source>Name</source>
        <translation type="unfinished">Name</translation>
    </message>
    <message>
        <location filename="../src/app/qgsabout.cpp" line="170"/>
        <source>Available QGIS Data Provider Plugins</source>
        <translation type="unfinished">Verfügbare QGIS-Datenlieferantenplugins</translation>
    </message>
    <message>
        <location filename="../src/app/qgsabout.cpp" line="173"/>
        <source>Available Qt Database Plugins</source>
        <translation type="unfinished">Verfügbare Qt-Datenbankplugins</translation>
    </message>
    <message>
        <location filename="../src/app/qgsabout.cpp" line="179"/>
        <source>Available Qt Image Plugins</source>
        <translation type="unfinished">Verfügbare Qt-Bildformatplugins</translation>
    </message>
    <message>
        <location filename="../src/app/qgsabout.cpp" line="117"/>
        <source>Website</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsAddAttrDialogBase</name>
    <message>
        <location filename="../src/ui/qgsaddattrdialogbase.ui" line="13"/>
        <source>Add Attribute</source>
        <translation>Attribut hinzufügen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsaddattrdialogbase.ui" line="22"/>
        <source>Name:</source>
        <translation type="unfinished">Name:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsaddattrdialogbase.ui" line="35"/>
        <source>Type:</source>
        <translation>Typ:</translation>
    </message>
</context>
<context>
    <name>QgsApplication</name>
    <message>
        <location filename="../src/core/qgsapplication.cpp" line="82"/>
        <source>Exception</source>
        <translation type="unfinished">Ausnahme</translation>
    </message>
</context>
<context>
    <name>QgsAttributeActionDialog</name>
    <message>
        <location filename="../src/app/qgsattributeactiondialog.cpp" line="150"/>
        <source>Select an action</source>
        <comment>File dialog window title</comment>
        <translation type="unfinished">Eine Aktion wählen</translation>
    </message>
</context>
<context>
    <name>QgsAttributeActionDialogBase</name>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="144"/>
        <source>This list contains all actions that have been defined for the current layer. Add actions by entering the details in the controls below and then pressing the Insert action button. Actions can be edited here by double clicking on the item.</source>
        <translation>Diese Liste beinhaltet alle Aktionen, die für aktive Layer definiert wurden. Fügen Sie durch Eingabe von Details in den untenstehenden Kontrollelementen Aktionen hinzu und drücken Sie dann den Knopf mit der Aufschrift &apos;Aktion hinzufügen&apos;. Aktionen können durch Doppelklick auf das entsprechende Element bearbeitet werden.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="214"/>
        <source>Move up</source>
        <translation>Verschiebe aufwärts</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="211"/>
        <source>Move the selected action up</source>
        <translation>Gewählte Aktion aufwärts bewegen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="224"/>
        <source>Move down</source>
        <translation>Verschiebe abwärts</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="221"/>
        <source>Move the selected action down</source>
        <translation>Selektierte Aktion abwärts bewegen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="250"/>
        <source>Remove</source>
        <translation>Entfernen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="247"/>
        <source>Remove the selected action</source>
        <translation>Gewählte Aktion entfernen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="59"/>
        <source>Enter the name of an action here. The name should be unique (qgis will make it unique if necessary).</source>
        <translation>Bitte Namen der Aktion eingeben. Der Name sollte eindeutig sein (QGIS macht ihn eindeutig, falls notwendig). </translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="56"/>
        <source>Enter the action name here</source>
        <translation>Namen der Aktion hier eingeben</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="79"/>
        <source>Enter the action command here</source>
        <translation>Kommando für die Aktion hier eingeben</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="178"/>
        <source>Insert action</source>
        <translation>Aktion hinzufügen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="175"/>
        <source>Inserts the action into the list above</source>
        <translation>Aktion in die obenstehende Liste einfügen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="188"/>
        <source>Update action</source>
        <translation>Aktualisiere die Aktion</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="185"/>
        <source>Update the selected action</source>
        <translation>Aktualisiere die markierte Aktion</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="115"/>
        <source>Insert field</source>
        <translation>Attribut einfügen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="112"/>
        <source>Inserts the selected field into the action, prepended with a %</source>
        <translation>Fügt das markierte Attribut mit vorangestelltem &apos;%&apos; in die Aktion ein</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="105"/>
        <source>The valid attribute names for this layer</source>
        <translation>Die gültigen Attributnamen für diesen Layer</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="128"/>
        <source>Capture output</source>
        <translation>Ausgaben aufzeichnen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="122"/>
        <source>Captures any output from the action</source>
        <translation>Ausgaben der Aktion aufzeichnen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="125"/>
        <source>Captures the standard output or error generated by the action and displays it in a dialog box</source>
        <translation>Nimmt Ausgaben ader Aktion auf Standardausgabe- oder -fehlerkanal auf und zeigt ihn in einem Dialog an</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="82"/>
        <source>Enter the action here. This can be any program, script or command that is available on your system. When the action is invoked any set of characters that start with a % and then have the name of a field will be replaced by the value of that field. The special characters %% will be replaced by the value of the field that was selected. Double quote marks group text into single arguments to the program, script or command. Double quotes will be ignored if preceeded by a backslash</source>
        <translation type="unfinished">Geben Sie hier die Aktion ein. Dies kann jedes Programm, Skript oder Kommando sein, dass in Ihrem System verfügbar ist.  Wenn die Aktion ausgeführt wird jeder durch % eingeleiteter Feldname durch den Feldwert ersetzt.  Die besondere Zeichenfolge %% wird durch den Wert des gewählten Felds ersetzt.  Mit Anführungszeichen können mehrere Wörter zu einem Argument der Aktion zusammengefaßt werden. Für mit Backslash (\) eingeleitete Anführungszeichen gilt dies nicht.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="19"/>
        <source>Attribute Actions</source>
        <translation type="unfinished">Attributaktionen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="37"/>
        <source>Action properties</source>
        <translation type="unfinished">Aktionseigenschaften</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="157"/>
        <source>Name</source>
        <translation type="unfinished">Name</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="162"/>
        <source>Action</source>
        <translation type="unfinished">Aktion</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="89"/>
        <source>Browse for action</source>
        <translation type="unfinished">Aktionen durchsuchen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="92"/>
        <source>Click to browse for an action</source>
        <translation type="unfinished">Zum Aktionen durchsuchen anklicken</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="98"/>
        <source>...</source>
        <translation type="unfinished">...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="167"/>
        <source>Capture</source>
        <translation type="unfinished">Erfassen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="95"/>
        <source>Clicking the button will let you select an application to use as the action</source>
        <translation type="unfinished">Mit diesem Knopf kann man eine Applikation für diese Aktion wählen</translation>
    </message>
</context>
<context>
    <name>QgsAttributeDialog</name>
    <message>
        <location filename="../src/app/qgsattributedialog.cpp" line="271"/>
        <source> (int)</source>
        <translation> (Ganzzahl)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributedialog.cpp" line="275"/>
        <source> (dbl)</source>
        <translation> (Fließkommazahl)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributedialog.cpp" line="280"/>
        <source> (txt)</source>
        <translation> (Text)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributedialog.cpp" line="256"/>
        <source>...</source>
        <translation type="unfinished">...</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributedialog.cpp" line="315"/>
        <source>Select a file</source>
        <translation type="unfinished">Datei wählen</translation>
    </message>
</context>
<context>
    <name>QgsAttributeDialogBase</name>
    <message>
        <location filename="../src/ui/qgsattributedialogbase.ui" line="13"/>
        <source>Enter Attribute Values</source>
        <translation>Attributwert eingeben</translation>
    </message>
</context>
<context>
    <name>QgsAttributeTable</name>
    <message>
        <location filename="../src/app/qgsattributetable.cpp" line="356"/>
        <source>Run action</source>
        <translation>Aktion starten</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetable.cpp" line="630"/>
        <source>Updating selection...</source>
        <translation type="unfinished">Auswahl wird aktualisiert...</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetable.cpp" line="630"/>
        <source>Abort</source>
        <translation type="unfinished">Abbrechen</translation>
    </message>
</context>
<context>
    <name>QgsAttributeTableBase</name>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="13"/>
        <source>Attribute Table</source>
        <translation>Attributtabelle</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="77"/>
        <source>Ctrl+S</source>
        <translation type="unfinished">Ctrl+S</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="67"/>
        <source>Invert selection</source>
        <translation>Auswahl umkehren</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="60"/>
        <source>Ctrl+T</source>
        <translation type="unfinished">Die meisten Werkzeugleisten ausblenden</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="50"/>
        <source>Move selected to top</source>
        <translation>Ausgewählte Objekte nach oben</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="33"/>
        <source>Remove selection</source>
        <translation>Auswahl löschen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="84"/>
        <source>Copy selected rows to clipboard (Ctrl+C)</source>
        <translation>Ausgewählte Zeilen in die Zwischenablage kopieren (Ctrl+C).</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="87"/>
        <source>Copies the selected rows to the clipboard</source>
        <translation>Kopiert die gewählten Zeilen in die Zwischenablage.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="97"/>
        <source>Ctrl+C</source>
        <translation>Ctrl+C</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="148"/>
        <source>in</source>
        <translation>in</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="168"/>
        <source>Search</source>
        <translation>Suchen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="178"/>
        <source>Adva&amp;nced...</source>
        <translation>Erw&amp;eitert...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="181"/>
        <source>Alt+N</source>
        <translation>Alt+N</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="107"/>
        <source>Zoom map to the selected rows</source>
        <translation>Zoome Karte zu den ausgewählten Spalteneinträgen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="135"/>
        <source>Search for</source>
        <translation type="unfinished">Suchen nach</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="207"/>
        <source>Toggle editing mode</source>
        <translation type="unfinished">Bearbeitungsmodus umschalten</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="210"/>
        <source>Click to toggle table editing</source>
        <translation type="unfinished">Anklicken um den Tabellenbearbeitungsmodus umzuschalten</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="104"/>
        <source>Zoom map to the selected rows (Ctrl-J)</source>
        <translation type="unfinished">Zu den selektierten Zeilen zoomen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="113"/>
        <source>Ctrl+J</source>
        <translation type="unfinished">Ctrl+J</translation>
    </message>
</context>
<context>
    <name>QgsAttributeTableDisplay</name>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="117"/>
        <source>select</source>
        <translation>Auswahl</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="118"/>
        <source>select and bring to top</source>
        <translation>Auswählen und nach oben verschieben</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="119"/>
        <source>show only matching</source>
        <translation>Nur Treffer zeigen.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="358"/>
        <source>Search string parsing error</source>
        <translation>Fehler im Suchbegriff.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="406"/>
        <source>Search results</source>
        <translation>Suchergebnisse</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="365"/>
        <source>You&apos;ve supplied an empty search string.</source>
        <translation>Sie haben einen leeren Suchbegriff eingegeben.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="394"/>
        <source>Error during search</source>
        <translation>Fehler beim Suchen</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="405"/>
        <source>No matching features found.</source>
        <translation>Keine Treffer gefunden.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="132"/>
        <source>Attribute table - </source>
        <translation type="unfinished">Attributtabelle - </translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="139"/>
        <source>QGIS</source>
        <translation type="unfinished">QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="143"/>
        <source>File</source>
        <translation type="unfinished">Datei</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="144"/>
        <source>Close</source>
        <translation type="unfinished">Schließen</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="144"/>
        <source>Ctrl+W</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="146"/>
        <source>Edit</source>
        <translation type="unfinished">Bearbeiten</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="147"/>
        <source>&amp;Undo</source>
        <translation type="unfinished">&amp;Zurücknehmen</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="147"/>
        <source>Ctrl+Z</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="150"/>
        <source>Cu&amp;t</source>
        <translation type="unfinished">&amp;Aussschneiden</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="150"/>
        <source>Ctrl+X</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="153"/>
        <source>&amp;Copy</source>
        <translation type="unfinished">&amp;Kopieren</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="153"/>
        <source>Ctrl+C</source>
        <translation type="unfinished">Ctrl+C</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="154"/>
        <source>&amp;Paste</source>
        <translation type="unfinished">&amp;Einfügen</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="154"/>
        <source>Ctrl+V</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="157"/>
        <source>Delete</source>
        <translation type="unfinished">Löschen</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="159"/>
        <source>Layer</source>
        <translation type="unfinished">Layer</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="161"/>
        <source>Zoom to Selection</source>
        <translation type="unfinished">Zur Auswahl zoomen</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="161"/>
        <source>Ctrl+J</source>
        <translation type="unfinished">Ctrl+J</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="164"/>
        <source>Toggle Editing</source>
        <translation type="unfinished">Bearbeitungsmodus umschalten</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="170"/>
        <source>Table</source>
        <translation type="unfinished">Tabelle</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="172"/>
        <source>Move to Top</source>
        <translation type="unfinished">Nach Oben bringen</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="174"/>
        <source>Invert</source>
        <translation type="unfinished">Umkehren</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="472"/>
        <source>bad_alloc exception</source>
        <translation type="unfinished">Speicher-Fehler</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="472"/>
        <source>Filling the attribute table has been stopped because there was no more virtual memory left</source>
        <translation type="unfinished">Das Auffüllen der Attributtabelle wurde beendet, da kein virtueller Speicher mehr zur Verfügung steht</translation>
    </message>
    <message numerus="yes">
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="403"/>
        <source>Found %1 matching features.</source>
        <translation type="unfinished">
            <numerusform>Keine passenden Objekte gefunden.</numerusform>
        </translation>
    </message>
</context>
<context>
    <name>QgsBookmarks</name>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="158"/>
        <source>Really Delete?</source>
        <translation>Wirklich löschen?</translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="159"/>
        <source>Are you sure you want to delete the </source>
        <translation>Sind Sie sicher, dass Sie das Lesezeichen </translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="159"/>
        <source> bookmark?</source>
        <translation> löschen wollen?</translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="175"/>
        <source>Error deleting bookmark</source>
        <translation>Fehler beim Löschen eines Lesezeichens</translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="177"/>
        <source>Failed to delete the </source>
        <translation>Das Löschen des </translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="179"/>
        <source> bookmark from the database. The database said:
</source>
        <translation> Lesezeichens aus der Datenbank schlug fehl. Die Datenbank meldete:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="58"/>
        <source>&amp;Delete</source>
        <translation type="unfinished">&amp;Löschen</translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="59"/>
        <source>&amp;Zoom to</source>
        <translation type="unfinished">&amp;Zoom nach</translation>
    </message>
</context>
<context>
    <name>QgsBookmarksBase</name>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="13"/>
        <source>Geospatial Bookmarks</source>
        <translation>Räumliches Lesezeichen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="29"/>
        <source>Name</source>
        <translation>Name</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="34"/>
        <source>Project</source>
        <translation>Projekt</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="39"/>
        <source>Extent</source>
        <translation>Ausdehnung</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="44"/>
        <source>Id</source>
        <translation>Id</translation>
    </message>
</context>
<context>
    <name>QgsComposer</name>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="473"/>
        <source>Big image</source>
        <translation>Großes Bild</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="474"/>
        <source>To create image </source>
        <translation>Um ein Bild zu erzeugen </translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="477"/>
        <source> requires circa </source>
        <translation>werden ca. </translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="477"/>
        <source> MB of memory</source>
        <translation> MB Speicher benötigt.</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="83"/>
        <source>QGIS - print composer</source>
        <translation>QGIS - Druckzusammenstellung</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="181"/>
        <source>Map 1</source>
        <translation>Karte 1</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="504"/>
        <source>format</source>
        <translation>Format</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="597"/>
        <source>SVG warning</source>
        <translation>SVG-Warnung</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="1094"/>
        <source>Don&apos;t show this message again</source>
        <translation>Diese Nachricht nicht mehr anzeigen.</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="624"/>
        <source>SVG Format</source>
        <translation>SVG-Format</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="94"/>
        <source>QGIS</source>
        <translation type="unfinished">QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="98"/>
        <source>File</source>
        <translation type="unfinished">Datei</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="99"/>
        <source>Close</source>
        <translation type="unfinished">Schließen</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="99"/>
        <source>Ctrl+W</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="105"/>
        <source>Edit</source>
        <translation type="unfinished">Bearbeiten</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="106"/>
        <source>&amp;Undo</source>
        <translation type="unfinished">&amp;Zurücknehmen</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="106"/>
        <source>Ctrl+Z</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="109"/>
        <source>Cu&amp;t</source>
        <translation type="unfinished">&amp;Ausschneiden</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="109"/>
        <source>Ctrl+X</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="111"/>
        <source>&amp;Copy</source>
        <translation type="unfinished">&amp;Kopieren</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="111"/>
        <source>Ctrl+C</source>
        <translation type="unfinished">Ctrl+C</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="113"/>
        <source>&amp;Paste</source>
        <translation type="unfinished">&amp;Einfügen</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="113"/>
        <source>Ctrl+V</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="115"/>
        <source>Delete</source>
        <translation type="unfinished">Löschen</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="118"/>
        <source>View</source>
        <translation type="unfinished">Ansicht</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="125"/>
        <source>Layout</source>
        <translation type="unfinished">Anordnung</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="527"/>
        <source>Choose a file name to save the map image as</source>
        <translation type="unfinished">Name der Datei des zu speichernden Kartenabbild wählen</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="623"/>
        <source>Choose a file name to save the map as</source>
        <translation type="unfinished">Einen Dateinamen zum Speichern des Kartenabbilds wählen</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="1092"/>
        <source>Project contains WMS layers</source>
        <translation type="unfinished">Projekt enthält WMS-Layer</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="1093"/>
        <source>Some WMS servers (e.g. UMN mapserver) have a limit for the WIDTH and HEIGHT parameter. Printing layers from such servers may exceed this limit. If this is the case, the WMS layer will not be printed</source>
        <translation type="unfinished">Einige WMS-Server (z.B. UMN Mapserver) haben Begrenzungen für die WIDTH- und HEIGHT-Parameter.  Falls diese Begrenzungen beim Ausdruck überschritten werden, werden diese WMS-Layer nicht gedruckt.</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="604"/>
        <source>&lt;p&gt;The SVG export function in Qgis has several problems due to bugs and deficiencies in the </source>
        <translation type="unfinished">&lt;p&gt;Die SVG-Exportfunktion in QGIS hat einige Probleme durch Fehler und Einschränkungen im </translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="610"/>
        <source>Qt4 svg code. Of note, text does not appear in the SVG file and there are problems with the map bounding box clipping other items such as the legend or scale bar.&lt;/p&gt;</source>
        <translation type="unfinished">Qt4-SVG-Code. Erwähnenswert ist, dass kein Text in der SVG-Datei erscheint und es Probleme mit der Ausgabeabgrenzung von Karte und anderen Elementen wie Legende und Maßstabsleiste gibt.</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="614"/>
        <source>Qt4 svg code. In particular, there are problems with layers not being clipped to the map bounding box.&lt;/p&gt;</source>
        <translation type="unfinished">Qt4-SVG-Code. Genauergesagt ist die Ausgabebegrenzung der Layer auf die Kartengrenzen ein Problem&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="618"/>
        <source>If you require a vector-based output file from Qgis it is suggested that you try printing to PostScript if the SVG output is not satisfactory.&lt;/p&gt;</source>
        <translation type="unfinished">Wenn Sie vektorbasierte QGIS-Ausgabedateien benötigen, sei der Ausdruck als PostScript empfohlen, wenn Ihnen die SVG-Ausgabe nicht ausreicht.&lt;/p&gt;</translation>
    </message>
</context>
<context>
    <name>QgsComposerBase</name>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="81"/>
        <source>General</source>
        <translation>Allgemein</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="134"/>
        <source>Composition</source>
        <translation>Zusammenstellung</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="142"/>
        <source>Item</source>
        <translation>Eintrag</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="260"/>
        <source>&amp;Print...</source>
        <translation>&amp;Drucken...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="295"/>
        <source>Add new map</source>
        <translation>Neue Karte hinzufügen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="306"/>
        <source>Add new label</source>
        <translation>Neue Beschriftung hinzufügen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="317"/>
        <source>Add new vect legend</source>
        <translation>Neue Vektorlegende hinzufügen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="328"/>
        <source>Select/Move item</source>
        <translation>Eintrag wählen/verschieben</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="355"/>
        <source>Add new scalebar</source>
        <translation>Neuen Massstab hinzufügen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="366"/>
        <source>Refresh view</source>
        <translation>Aktualisiere Ansicht</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="16"/>
        <source>MainWindow</source>
        <translation>MainWindow</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="276"/>
        <source>Zoom In</source>
        <translation>Hineinzoomen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="284"/>
        <source>Zoom Out</source>
        <translation>Hinauszoomen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="374"/>
        <source>Add Image</source>
        <translation>Bild hinzufügen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="214"/>
        <source>Close</source>
        <translation>Schließen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="194"/>
        <source>Help</source>
        <translation>Hilfe</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="268"/>
        <source>Zoom Full</source>
        <translation type="unfinished">Volle Ausdehnung</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="292"/>
        <source>Add Map</source>
        <translation type="unfinished">Karte hinzufügen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="303"/>
        <source>Add Label</source>
        <translation type="unfinished">Beschriftung hinzufügen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="314"/>
        <source>Add Vector Legend</source>
        <translation type="unfinished">Vektorlegende hinzufügen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="325"/>
        <source>Move Item</source>
        <translation type="unfinished">Element verschieben</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="336"/>
        <source>Export as Image...</source>
        <translation type="unfinished">Speichern als Rasterbild...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="344"/>
        <source>Export as SVG...</source>
        <translation type="unfinished">Speichern als SVG...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="352"/>
        <source>Add Scalebar</source>
        <translation type="unfinished">Maßstab hinzufügen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="363"/>
        <source>Refresh</source>
        <translation type="unfinished">Auffrischen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="379"/>
        <source>Move Content</source>
        <translation type="unfinished">Inhalt verschieben</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="382"/>
        <source>Move item content</source>
        <translation type="unfinished">Den Elementinhalt verschieben</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="387"/>
        <source>Group</source>
        <translation type="unfinished">Gruppieren</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="390"/>
        <source>Group items</source>
        <translation type="unfinished">Gruppenelemente</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="395"/>
        <source>Ungroup</source>
        <translation type="unfinished">Auflösen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="398"/>
        <source>Ungroup items</source>
        <translation type="unfinished">Die Gruppe auflösen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="403"/>
        <source>Raise</source>
        <translation type="unfinished">Hervorholen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="406"/>
        <source>Raise selected items</source>
        <translation type="unfinished">Ausgewählte Elemente in den Vordergrund bringen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="411"/>
        <source>Lower</source>
        <translation type="unfinished">Versenken</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="414"/>
        <source>Lower selected items</source>
        <translation type="unfinished">Gewählte Elemente in den Hintergrund bringen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="419"/>
        <source>Bring to Front</source>
        <translation type="unfinished">In den Vordergrund holen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="422"/>
        <source>Move selected items to top</source>
        <translation type="unfinished">Gewählte Objekte in den Vordergrund verschieben</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="427"/>
        <source>Send to Back</source>
        <translation type="unfinished">In den Hintergrund schicken</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="430"/>
        <source>Move selected items to bottom</source>
        <translation type="unfinished">Die ausgewählten Elemente in den Hintergrund stellen</translation>
    </message>
</context>
<context>
    <name>QgsComposerItemWidgetBase</name>
    <message>
        <location filename="../src/ui/qgscomposeritemwidgetbase.ui" line="13"/>
        <source>Form</source>
        <translation type="unfinished">Formular</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposeritemwidgetbase.ui" line="19"/>
        <source>Composer item properties</source>
        <translation type="unfinished">Elementeigenschaften</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposeritemwidgetbase.ui" line="25"/>
        <source>Color:</source>
        <translation type="unfinished">Farbe:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposeritemwidgetbase.ui" line="32"/>
        <source>Frame...</source>
        <translation type="unfinished">Rahmen...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposeritemwidgetbase.ui" line="39"/>
        <source>Background...</source>
        <translation type="unfinished">Hintergrund...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposeritemwidgetbase.ui" line="46"/>
        <source>Opacity:</source>
        <translation type="unfinished">Opazität:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposeritemwidgetbase.ui" line="63"/>
        <source>Outline width: </source>
        <translation type="unfinished">Rahmenstärke:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposeritemwidgetbase.ui" line="73"/>
        <source>Frame</source>
        <translation type="unfinished">Rahmen</translation>
    </message>
</context>
<context>
    <name>QgsComposerLabelWidgetBase</name>
    <message>
        <location filename="../src/ui/qgscomposerlabelwidgetbase.ui" line="19"/>
        <source>Label Options</source>
        <translation type="unfinished">Beschriftungsoption</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlabelwidgetbase.ui" line="38"/>
        <source>Font</source>
        <translation type="unfinished">Schrift</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlabelwidgetbase.ui" line="45"/>
        <source>Margin (mm):</source>
        <translation type="unfinished">Rand (mm)</translation>
    </message>
</context>
<context>
    <name>QgsComposerLegendItemDialogBase</name>
    <message>
        <location filename="../src/ui/qgscomposerlegenditemdialogbase.ui" line="13"/>
        <source>Legend item properties</source>
        <translation type="unfinished">Eigenschaften des Legendenelements</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlegenditemdialogbase.ui" line="19"/>
        <source>Item text:</source>
        <translation type="unfinished">Elementtext:</translation>
    </message>
</context>
<context>
    <name>QgsComposerLegendWidgetBase</name>
    <message>
        <location filename="../src/ui/qgscomposerlegendwidgetbase.ui" line="19"/>
        <source>Barscale Options</source>
        <translation type="unfinished">Optionen für Massstabsbalken</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlegendwidgetbase.ui" line="25"/>
        <source>General</source>
        <translation type="unfinished">Allgemein</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlegendwidgetbase.ui" line="33"/>
        <source>Title:</source>
        <translation type="unfinished">Titel:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlegendwidgetbase.ui" line="47"/>
        <source>Font:</source>
        <translation type="unfinished">Schriftart:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlegendwidgetbase.ui" line="60"/>
        <source>Title...</source>
        <translation type="unfinished">Titel...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlegendwidgetbase.ui" line="67"/>
        <source>Layer...</source>
        <translation type="unfinished">Layer...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlegendwidgetbase.ui" line="74"/>
        <source>Item...</source>
        <translation type="unfinished">Element...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlegendwidgetbase.ui" line="85"/>
        <source>Symbol width: </source>
        <translation type="unfinished">Symbolbreite: </translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlegendwidgetbase.ui" line="95"/>
        <source>Symbol height:</source>
        <translation type="unfinished">Symbolhöhe:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlegendwidgetbase.ui" line="105"/>
        <source>Layer space: </source>
        <translation type="unfinished">Layerraum: </translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlegendwidgetbase.ui" line="115"/>
        <source>Symbol space:</source>
        <translation type="unfinished">Symbolraum: </translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlegendwidgetbase.ui" line="129"/>
        <source>Icon label space:</source>
        <translation type="unfinished">Iconbeschriftungsraum:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlegendwidgetbase.ui" line="139"/>
        <source>Box space:</source>
        <translation type="unfinished">Rahmenraum:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlegendwidgetbase.ui" line="154"/>
        <source>Legend items</source>
        <translation type="unfinished">Legendenelemente</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlegendwidgetbase.ui" line="160"/>
        <source>down</source>
        <translation type="unfinished">hinunter</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlegendwidgetbase.ui" line="167"/>
        <source>up</source>
        <translation type="unfinished">hinauf</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlegendwidgetbase.ui" line="174"/>
        <source>remove</source>
        <translation type="unfinished">entfernen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlegendwidgetbase.ui" line="181"/>
        <source>edit...</source>
        <translation type="unfinished">ändern..</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlegendwidgetbase.ui" line="188"/>
        <source>update</source>
        <translation type="unfinished">aktualisieren</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlegendwidgetbase.ui" line="195"/>
        <source>update all</source>
        <translation type="unfinished">alle aktualisieren</translation>
    </message>
</context>
<context>
    <name>QgsComposerMap</name>
    <message>
        <location filename="../src/core/composer/qgscomposermap.cpp" line="83"/>
        <source>Map</source>
        <translation type="unfinished">Karte</translation>
    </message>
    <message>
        <location filename="../src/core/composer/qgscomposermap.cpp" line="191"/>
        <source>Map will be printed here</source>
        <translation type="unfinished">Karte wird hier gedruckt</translation>
    </message>
</context>
<context>
    <name>QgsComposerMapWidget</name>
    <message>
        <location filename="../src/app/composer/qgscomposermapwidget.cpp" line="215"/>
        <source>Cache</source>
        <translation type="unfinished">Cache</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposermapwidget.cpp" line="223"/>
        <source>Rectangle</source>
        <translation type="unfinished">Rechteck</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposermapwidget.cpp" line="219"/>
        <source>Render</source>
        <translation type="unfinished">Zeichnen</translation>
    </message>
</context>
<context>
    <name>QgsComposerMapWidgetBase</name>
    <message>
        <location filename="../src/ui/qgscomposermapwidgetbase.ui" line="19"/>
        <source>Map options</source>
        <translation type="unfinished">Kartenoptionen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapwidgetbase.ui" line="25"/>
        <source>&lt;b&gt;Map&lt;/b&gt;</source>
        <translation type="unfinished">&lt;b&gt;Karte&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapwidgetbase.ui" line="45"/>
        <source>Width</source>
        <translation type="unfinished">Breite</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapwidgetbase.ui" line="61"/>
        <source>Height</source>
        <translation type="unfinished">Höhe</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapwidgetbase.ui" line="83"/>
        <source>Scale:</source>
        <translation type="unfinished">Massstab:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapwidgetbase.ui" line="93"/>
        <source>1:</source>
        <translation type="unfinished">1:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapwidgetbase.ui" line="106"/>
        <source>Map extent</source>
        <translation type="unfinished">Kartenausmaß</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapwidgetbase.ui" line="117"/>
        <source>X min:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapwidgetbase.ui" line="124"/>
        <source>Y min:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapwidgetbase.ui" line="145"/>
        <source>X max:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapwidgetbase.ui" line="155"/>
        <source>Y max:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapwidgetbase.ui" line="174"/>
        <source>set to map canvas extent</source>
        <translation type="unfinished">Anzeigegrenzen übernehmen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapwidgetbase.ui" line="197"/>
        <source>Preview</source>
        <translation type="unfinished">Vorschau</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapwidgetbase.ui" line="220"/>
        <source>Update preview</source>
        <translation type="unfinished">Vorschau aktualisieren</translation>
    </message>
</context>
<context>
    <name>QgsComposerPictureWidget</name>
    <message>
        <location filename="../src/app/composer/qgscomposerpicturewidget.cpp" line="59"/>
        <source>Select svg or image file</source>
        <translation type="unfinished">SVG oder Rasterbild wählen</translation>
    </message>
</context>
<context>
    <name>QgsComposerPictureWidgetBase</name>
    <message>
        <location filename="../src/ui/qgscomposerpicturewidgetbase.ui" line="19"/>
        <source>Picture Options</source>
        <translation type="unfinished">Bild-Optionen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerpicturewidgetbase.ui" line="57"/>
        <source>Browse...</source>
        <translation type="unfinished">Suchen...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerpicturewidgetbase.ui" line="74"/>
        <source>Width:</source>
        <translation type="unfinished">Breite:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerpicturewidgetbase.ui" line="93"/>
        <source>Height:</source>
        <translation type="unfinished">Höhe:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerpicturewidgetbase.ui" line="112"/>
        <source>Rotation:</source>
        <translation type="unfinished">Drehung:</translation>
    </message>
</context>
<context>
    <name>QgsComposerScaleBarWidget</name>
    <message>
        <location filename="../src/app/composer/qgscomposerscalebarwidget.cpp" line="286"/>
        <source>Single Box</source>
        <translation type="unfinished">Einfacher Rahmen</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposerscalebarwidget.cpp" line="290"/>
        <source>Double Box</source>
        <translation type="unfinished">Doppelter Rahmen</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposerscalebarwidget.cpp" line="298"/>
        <source>Line Ticks Middle</source>
        <translation type="unfinished">Mittige Linieneinteilung</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposerscalebarwidget.cpp" line="302"/>
        <source>Line Ticks Down</source>
        <translation type="unfinished">Linieneinteilung unten</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposerscalebarwidget.cpp" line="306"/>
        <source>Line Ticks Up</source>
        <translation type="unfinished">Linieneinteilung oben</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposerscalebarwidget.cpp" line="310"/>
        <source>Numeric</source>
        <translation type="unfinished">Numerisch</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposerscalebarwidget.cpp" line="148"/>
        <source>Map </source>
        <translation type="unfinished">Karte </translation>
    </message>
</context>
<context>
    <name>QgsComposerScaleBarWidgetBase</name>
    <message>
        <location filename="../src/ui/qgscomposerscalebarwidgetbase.ui" line="19"/>
        <source>Barscale Options</source>
        <translation type="unfinished">Optionen für Massstabsbalken</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarwidgetbase.ui" line="25"/>
        <source>Segment size (map units):</source>
        <translation type="unfinished">Segmentgröße (Karteneinheiten)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarwidgetbase.ui" line="45"/>
        <source>Map units per bar unit:</source>
        <translation type="unfinished">Karteneinheiten pro Massstabseinheit:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarwidgetbase.ui" line="62"/>
        <source>Number of segments:</source>
        <translation type="unfinished">Anzahl Segmente:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarwidgetbase.ui" line="75"/>
        <source>Segments left:</source>
        <translation type="unfinished">Segmente links:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarwidgetbase.ui" line="85"/>
        <source>Style:</source>
        <translation type="unfinished">Stil:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarwidgetbase.ui" line="101"/>
        <source>Map:</source>
        <translation type="unfinished">Karte:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarwidgetbase.ui" line="127"/>
        <source>Height (mm):</source>
        <translation type="unfinished">Höhe (mm):</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarwidgetbase.ui" line="137"/>
        <source>Line width:</source>
        <translation type="unfinished">Linienstärke:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarwidgetbase.ui" line="169"/>
        <source>Label space:</source>
        <translation type="unfinished">Beschriftungsabstand</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarwidgetbase.ui" line="179"/>
        <source>Box space:</source>
        <translation type="unfinished">Rahmabstand</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarwidgetbase.ui" line="189"/>
        <source>Unit label:</source>
        <translation type="unfinished">Beschriftung der Einheit:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarwidgetbase.ui" line="211"/>
        <source>Font...</source>
        <translation type="unfinished">Schriftart...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarwidgetbase.ui" line="218"/>
        <source>Color...</source>
        <translation type="unfinished">Farbe...</translation>
    </message>
</context>
<context>
    <name>QgsComposerVectorLegendBase</name>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="21"/>
        <source>Vector Legend Options</source>
        <translation>Optionen für Vektorlegende</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="113"/>
        <source>Title</source>
        <translation>Titel</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="92"/>
        <source>Map</source>
        <translation>Karte</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="182"/>
        <source>Font</source>
        <translation>Schrift</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="167"/>
        <source>Box</source>
        <translation>Box</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="53"/>
        <source>Preview</source>
        <translation>Vorschau</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="149"/>
        <source>Layers</source>
        <translation type="unfinished">Layer</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="154"/>
        <source>Group</source>
        <translation type="unfinished">Gruppe</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="159"/>
        <source>ID</source>
        <translation type="unfinished">ID</translation>
    </message>
</context>
<context>
    <name>QgsCompositionBase</name>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="19"/>
        <source>Composition</source>
        <translation>Zusammenstellung</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="43"/>
        <source>Paper</source>
        <translation>Papier</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="182"/>
        <source>Size</source>
        <translation>Grösse</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="166"/>
        <source>Units</source>
        <translation>Einheiten</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="150"/>
        <source>Width</source>
        <translation>Breite</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="134"/>
        <source>Height</source>
        <translation>Höhe</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="118"/>
        <source>Orientation</source>
        <translation>Orientierung</translation>
    </message>
</context>
<context>
    <name>QgsCompositionWidget</name>
    <message>
        <location filename="../src/app/composer/qgscompositionwidget.cpp" line="241"/>
        <source>Landscape</source>
        <translation type="unfinished">Querformat</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscompositionwidget.cpp" line="245"/>
        <source>Portrait</source>
        <translation type="unfinished">Hochformat</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscompositionwidget.cpp" line="110"/>
        <source>Custom</source>
        <translation type="unfinished">Benutzerdefiniert</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscompositionwidget.cpp" line="63"/>
        <source>A5 (148x210 mm)</source>
        <translation type="unfinished">A5 (148x210 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscompositionwidget.cpp" line="65"/>
        <source>A4 (210x297 mm)</source>
        <translation type="unfinished">A4 (210x297 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscompositionwidget.cpp" line="67"/>
        <source>A3 (297x420 mm)</source>
        <translation type="unfinished">A3 (297x420 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscompositionwidget.cpp" line="69"/>
        <source>A2 (420x594 mm)</source>
        <translation type="unfinished">A2 (420x594 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscompositionwidget.cpp" line="71"/>
        <source>A1 (594x841 mm)</source>
        <translation type="unfinished">A1 (594x841 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscompositionwidget.cpp" line="73"/>
        <source>A0 (841x1189 mm)</source>
        <translation type="unfinished">A0 (841x1189 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscompositionwidget.cpp" line="75"/>
        <source>B5 (176 x 250 mm)</source>
        <translation type="unfinished">B5 (176 x 250 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscompositionwidget.cpp" line="77"/>
        <source>B4 (250 x 353 mm)</source>
        <translation type="unfinished">B4 (250 x 353 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscompositionwidget.cpp" line="79"/>
        <source>B3 (353 x 500 mm)</source>
        <translation type="unfinished">B3 (353 x 500 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscompositionwidget.cpp" line="81"/>
        <source>B2 (500 x 707 mm)</source>
        <translation type="unfinished">B2 (500 x 707 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscompositionwidget.cpp" line="83"/>
        <source>B1 (707 x 1000 mm)</source>
        <translation type="unfinished">B1 (707 x 1000 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscompositionwidget.cpp" line="85"/>
        <source>B0 (1000 x 1414 mm)</source>
        <translation type="unfinished">B0 (1000 x 1414 mm)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscompositionwidget.cpp" line="87"/>
        <source>Letter (8.5x11 inches)</source>
        <translation type="unfinished">Letter (8.5x11 inches)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscompositionwidget.cpp" line="89"/>
        <source>Legal (8.5x14 inches)</source>
        <translation type="unfinished">Legal (8.5x14 inches)</translation>
    </message>
</context>
<context>
    <name>QgsCompositionWidgetBase</name>
    <message>
        <location filename="../src/ui/qgscompositionwidgetbase.ui" line="19"/>
        <source>Composition</source>
        <translation type="unfinished">Zusammenstellung</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionwidgetbase.ui" line="43"/>
        <source>Paper</source>
        <translation type="unfinished">Papier</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionwidgetbase.ui" line="118"/>
        <source>Orientation</source>
        <translation type="unfinished">Orientierung</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionwidgetbase.ui" line="134"/>
        <source>Height</source>
        <translation type="unfinished">Höhe</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionwidgetbase.ui" line="150"/>
        <source>Width</source>
        <translation type="unfinished">Breite</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionwidgetbase.ui" line="166"/>
        <source>Units</source>
        <translation type="unfinished">Einheiten</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionwidgetbase.ui" line="182"/>
        <source>Size</source>
        <translation type="unfinished">Grösse</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionwidgetbase.ui" line="217"/>
        <source>Print quality (dpi)</source>
        <translation type="unfinished">Druckqualität (dpi)</translation>
    </message>
</context>
<context>
    <name>QgsContinuousColorDialogBase</name>
    <message>
        <location filename="../src/ui/qgscontinuouscolordialogbase.ui" line="13"/>
        <source>Continuous color</source>
        <translation>Fortlaufende Farbe</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscontinuouscolordialogbase.ui" line="54"/>
        <source>Maximum Value:</source>
        <translation>Größter Wert:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscontinuouscolordialogbase.ui" line="80"/>
        <source>Outline Width:</source>
        <translation>Umrandungsbreite:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscontinuouscolordialogbase.ui" line="93"/>
        <source>Minimum Value:</source>
        <translation>Kleinster Wert:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscontinuouscolordialogbase.ui" line="109"/>
        <source>Classification Field:</source>
        <translation>Klassifizierungsfeld:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscontinuouscolordialogbase.ui" line="119"/>
        <source>Draw polygon outline</source>
        <translation>Polygon-Umriss zeichnen</translation>
    </message>
</context>
<context>
    <name>QgsCoordinateTransform</name>
    <message>
        <location filename="../src/core/qgscoordinatetransform.cpp" line="483"/>
        <source>Failed</source>
        <translation>Fehlgeschlagen</translation>
    </message>
    <message>
        <location filename="../src/core/qgscoordinatetransform.cpp" line="483"/>
        <source>transform of</source>
        <translation>Transformation von</translation>
    </message>
    <message>
        <location filename="../src/core/qgscoordinatetransform.cpp" line="496"/>
        <source>with error: </source>
        <translation>mit Fehler:</translation>
    </message>
    <message>
        <location filename="../src/core/qgscoordinatetransform.cpp" line="418"/>
        <source>The source spatial reference system (CRS) is not valid. </source>
        <translation type="unfinished">Das ursprüngliche Koordinatenbezugssystem (KBS) is ungültig. </translation>
    </message>
    <message>
        <location filename="../src/core/qgscoordinatetransform.cpp" line="426"/>
        <source>The coordinates can not be reprojected. The CRS is: </source>
        <translation type="unfinished">Die Koordinaten können nicht projiziert werden. Das KBS ist: </translation>
    </message>
    <message>
        <location filename="../src/core/qgscoordinatetransform.cpp" line="425"/>
        <source>The destination spatial reference system (CRS) is not valid. </source>
        <translation type="unfinished">Das Zielkoordinatenbezugssystem (KBS) is nicht gültig.</translation>
    </message>
</context>
<context>
    <name>QgsCopyrightLabelPlugin</name>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="64"/>
        <source>Bottom Left</source>
        <translation>Unten links</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="65"/>
        <source>Top Left</source>
        <translation>Oben links</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="65"/>
        <source>Top Right</source>
        <translation>Oben rechts</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="65"/>
        <source>Bottom Right</source>
        <translation>Unten rechts</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="77"/>
        <source>&amp;Copyright Label</source>
        <translation>&amp;Urhebersrechtshinweis</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="78"/>
        <source>Creates a copyright label that is displayed on the map canvas.</source>
        <translation>Erzeugt einen Urheberrechtshinweis auf dem Kartenbild.</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="202"/>
        <source>&amp;Decorations</source>
        <translation>&amp;Dekorationen</translation>
    </message>
</context>
<context>
    <name>QgsCopyrightLabelPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="13"/>
        <source>Copyright Label Plugin</source>
        <translation>Urhebersrechtsnachweis-Plugin</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="185"/>
        <source>Placement</source>
        <translation>Platzierung</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="193"/>
        <source>Bottom Left</source>
        <translation>Unten links</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="198"/>
        <source>Top Left</source>
        <translation>Oben links</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="203"/>
        <source>Bottom Right</source>
        <translation>Unten rechts</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="208"/>
        <source>Top Right</source>
        <translation>Oben rechts</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="216"/>
        <source>Orientation</source>
        <translation>Orientierung</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="224"/>
        <source>Horizontal</source>
        <translation>Horizontal</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="229"/>
        <source>Vertical</source>
        <translation>Vertikal</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="146"/>
        <source>Enable Copyright Label</source>
        <translation>Urheberrechtshinweis aktivieren</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="57"/>
        <source>Color</source>
        <translation type="unfinished">Farbe</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="98"/>
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
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="158"/>
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
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="170"/>
        <source>Delete Projection Definition?</source>
        <translation>Projektdefinition löschen?</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="171"/>
        <source>Deleting a projection definition is not reversable. Do you want to delete it?</source>
        <translation>Das Löschen einer Projektdefinition ist nicht umkehrbar. Wirklich löschen?</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="847"/>
        <source>Abort</source>
        <translation>Abbrechen</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="849"/>
        <source>New</source>
        <translation>Neu</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="903"/>
        <source>QGIS Custom Projection</source>
        <translation>QGIS benutzerdefinierte Projektion</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="772"/>
        <source>This proj4 projection definition is not valid. Please correct before pressing save.</source>
        <translation>Diese proj4 Definition ist ungültig. Bitte vor dem speichern korrigieren.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="720"/>
        <source>This proj4 projection definition is not valid. Please give the projection a name before pressing save.</source>
        <translation>Diese proj4 Projektionsdefinition ist ungültig. Bitte Sie einen Projektionsnamen an, bevor &apos;speichern&apos; gedrückt wird.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="726"/>
        <source>This proj4 projection definition is not valid. Please add the parameters before pressing save.</source>
        <translation>Diese proj4 Projektionsdefinition ist ungültig. Bitte geben sie Parameter an, bevor &apos;speichern&apos; gedrückt wird.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="741"/>
        <source>This proj4 projection definition is not valid. Please add a proj= clause before pressing save.</source>
        <translation>Diese proj4 Projektionsdefinition ist ungültig. Bitte fügen sie einen proj= Ausdruck hinzu, bevor &apos;speichern&apos; gedrückt wird. </translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="877"/>
        <source>This proj4 projection definition is not valid.</source>
        <translation>Diese proj4 Projektionsdefinition ist ungültig.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="892"/>
        <source>Northing and Easthing must be in decimal form.</source>
        <translation>Northing und Easthing muss in dezimaler Form sein.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="904"/>
        <source>Internal Error (source projection invalid?)</source>
        <translation>Interner Fehler (Quellprojektion ungültig?)</translation>
    </message>
</context>
<context>
    <name>QgsCustomProjectionDialogBase</name>
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
        <translation>1 von 1</translation>
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
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="22"/>
        <source>Define</source>
        <translation>Definieren</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="151"/>
        <source>Test</source>
        <translation>Testen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="253"/>
        <source>Calculate</source>
        <translation>Berechnen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="180"/>
        <source>Geographic / WGS84</source>
        <translation>Geographisch/ WGS84</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="38"/>
        <source>Name</source>
        <translation type="unfinished">Name</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="167"/>
        <source>Parameters</source>
        <translation type="unfinished">Parameter</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="116"/>
        <source>*</source>
        <translation>*</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="126"/>
        <source>S</source>
        <translation>S</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="136"/>
        <source>X</source>
        <translation>X</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="194"/>
        <source>North</source>
        <translation type="unfinished">Nord</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="224"/>
        <source>East</source>
        <translation type="unfinished">Ost</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="13"/>
        <source>Custom Coordinate Reference System Definition</source>
        <translation type="unfinished">Definition eines Benutzerkoordinatensystems</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="28"/>
        <source>You can define your own custom Coordinate Reference System (CRS) here. The definition must conform to the proj4 format for specifying a CRS.</source>
        <translation type="unfinished">Hier kann ein Benutzerkoordinatenbezugssystem (KBS) definiert werden. Die Definition muß im PROJ.4-Format erfolgen.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="157"/>
        <source>Use the text boxes below to test the CRS definition you are creating. Enter a coordinate where both the lat/long and the transformed result are known (for example by reading off a map). Then press the calculate button to see if the CRS definition you are creating is accurate.</source>
        <translation type="unfinished">Benutzen Sie das Eingabefeld unten, um die anzulegende KBS-Definition zu testen. Geben Sie eine Koordinate an zu denen Sie sowohl Breite/Länge und das transformierte Ergebnis kennen (zum Beispiel durch Ablesen von einer Karte). Mit dem &quot;Berechnen&quot;-Knopf können Sie die neue KBS-Definition überprüfen.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="187"/>
        <source>Destination CRS        </source>
        <translation type="unfinished">Ziel KBS </translation>
    </message>
</context>
<context>
    <name>QgsDbSourceSelect</name>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="232"/>
        <source>Are you sure you want to remove the </source>
        <translation>Sind Sie sicher das Sie die Verbindung und </translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="232"/>
        <source> connection and all associated settings?</source>
        <translation> alle damit verbunden Einstellungen löschen wollen?</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="233"/>
        <source>Confirm Delete</source>
        <translation>Löschen bestätigen</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="340"/>
        <source>Select Table</source>
        <translation>Tabelle wählen</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="340"/>
        <source>You must select a table in order to add a Layer.</source>
        <translation>Es muß eine Tabelle gewählt werden, um einen Layer hinzuzufügen.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="401"/>
        <source>Password for </source>
        <translation>Passwort für </translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="402"/>
        <source>Please enter your password:</source>
        <translation>Bitte Passwort eingeben:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="453"/>
        <source>Connection failed</source>
        <translation>Verbindungsfehler</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="149"/>
        <source>Type</source>
        <translation>Typ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="157"/>
        <source>Sql</source>
        <translation>Sql</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="455"/>
        <source>Connection to %1 on %2 failed. Either the database is down or your settings are incorrect.%3Check your username and password and try again.%4The database said:%5%6</source>
        <translation>Verbindung zu %1 auf %2 ist fehlgeschlagen. Entweder ist die Datenbank abgeschaltet oder Ihre Einstellungen sind falsch. %3Bitte überprüfen Sie den Benutzernamen und das Passwort und probieren Sie es noch einmal. %4 Die Datenbank meldete folgendes:%5%6.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="125"/>
        <source>Wildcard</source>
        <translation>Platzhalter</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="129"/>
        <source>RegExp</source>
        <translation>RegAusdr</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="137"/>
        <source>All</source>
        <translation type="unfinished">Alle</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="141"/>
        <source>Schema</source>
        <translation type="unfinished">Schema</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="145"/>
        <source>Table</source>
        <translation type="unfinished">Tabelle</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="153"/>
        <source>Geometry column</source>
        <translation>Geometriespalte</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="637"/>
        <source>Accessible tables could not be determined</source>
        <translation type="unfinished">Zugreifbare Tabellen konnten nicht festgestellt werden</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="640"/>
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
        <location filename="../src/app/qgsdbsourceselect.cpp" line="673"/>
        <source>No accessible tables found</source>
        <translation type="unfinished">Keine zugänglichen Tabellen gefunden</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="676"/>
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
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="13"/>
        <source>Add PostGIS Table(s)</source>
        <translation>PostGIS-Tabelle(n) hinzufügen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="140"/>
        <source>Add</source>
        <translation>Hinzufügen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="111"/>
        <source>Help</source>
        <translation>Hilfe</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="114"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="87"/>
        <source>Connect</source>
        <translation>Verbinden</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="80"/>
        <source>New</source>
        <translation>Neu</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="73"/>
        <source>Edit</source>
        <translation>Bearbeiten</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="66"/>
        <source>Delete</source>
        <translation>Löschen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="156"/>
        <source>Close</source>
        <translation>Schließen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="54"/>
        <source>PostgreSQL Connections</source>
        <translation>PostgreSQL-Verbindungen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="183"/>
        <source>Search:</source>
        <translation>Suchen:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="190"/>
        <source>Search mode:</source>
        <translation>Suchmodus:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="200"/>
        <source>Search in columns:</source>
        <translation>Suche in Spalten:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="216"/>
        <source>Search options...</source>
        <translation>Suchoptionen...</translation>
    </message>
</context>
<context>
    <name>QgsDbTableModel</name>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="25"/>
        <source>Schema</source>
        <translation type="unfinished">Schema</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="26"/>
        <source>Table</source>
        <translation type="unfinished">Tabelle</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="27"/>
        <source>Type</source>
        <translation type="unfinished">Typ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="28"/>
        <source>Geometry column</source>
        <translation>Geometriespalte</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="29"/>
        <source>Sql</source>
        <translation type="unfinished">Sql</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="237"/>
        <source>Point</source>
        <translation type="unfinished">Punkt</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="241"/>
        <source>Multipoint</source>
        <translation>Multipunkt</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="245"/>
        <source>Line</source>
        <translation type="unfinished">Linie</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="249"/>
        <source>Multiline</source>
        <translation>Multilinie</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="253"/>
        <source>Polygon</source>
        <translation type="unfinished">Polygon</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="257"/>
        <source>Multipolygon</source>
        <translation>Multipolygon</translation>
    </message>
</context>
<context>
    <name>QgsDelAttrDialogBase</name>
    <message>
        <location filename="../src/ui/qgsdelattrdialogbase.ui" line="13"/>
        <source>Delete Attributes</source>
        <translation>Attribute löschen</translation>
    </message>
</context>
<context>
    <name>QgsDelimitedTextPlugin</name>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="99"/>
        <source>&amp;Add Delimited Text Layer</source>
        <translation>Getrennte Textdatei hinzufügen</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="102"/>
        <source>Add a delimited text file as a map layer. </source>
        <translation>Eine Textdatei dem Kartenfenster als Layer hinzufügen.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="103"/>
        <source>The file must have a header row containing the field names. </source>
        <translation>Die Datei muss eine Kopfzeile mit Spaltennamen enthalten.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="103"/>
        <source>X and Y fields are required and must contain coordinates in decimal units.</source>
        <translation>X- und Y-Spalten mit dezimalen Koordinaten sind unbedingt erforderlich</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="140"/>
        <source>&amp;Delimited text</source>
        <translation>&amp;Getrennter Text</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="54"/>
        <source>DelimitedTextLayer</source>
        <translation>Layer aus Textdatei</translation>
    </message>
</context>
<context>
    <name>QgsDelimitedTextPluginGui</name>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="125"/>
        <source>No layer name</source>
        <translation>Kein Layername</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="125"/>
        <source>Please enter a layer name before adding the layer to the map</source>
        <translation>Bitte geben Sie einen Layernamen ein, bevor Sie den Layer zum Kartenfenster hinzufügen</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="207"/>
        <source>No delimiter</source>
        <translation>Kein Trennzeichen</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="207"/>
        <source>Please specify a delimiter prior to parsing the file</source>
        <translation>Es muss ein Trennzeichen eingegeben werden, damit die Datei abgearbeitet werden kann</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="241"/>
        <source>Choose a delimited text file to open</source>
        <translation>Textdatei zum Öffnen wählen</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="35"/>
        <source>Parse</source>
        <translation>Analysieren</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="60"/>
        <source>Description</source>
        <translation>Beschreibung</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="63"/>
        <source>Select a delimited text file containing a header row and one or more rows of x and y coordinates that you would like to use as a point layer and this plugin will do the job for you!</source>
        <translation>Wähle eine Textdatei mit Trennzeichen, das eine Kopfzeile, und Spalten mit X- und Y-Koordinaten enthält, die Sie gerne als Punktlayer darstellen möchten und QGIS erledigt das für Sie!</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="67"/>
        <source>Use the layer name box to specify the legend name for the new layer. Use the delimiter box to specify what delimeter is used in your file (e.g. space, comma, tab or a regular expression in Perl style). After choosing a delimiter, press the parse button and select the columns containing the x and y values for the layer.</source>
        <translation>Benutzen Sie die Layername Box, um den Legendennamen des zu erstellenden Layers anzugeben. Benutzen Sie die Trennzeichen Box, um das in der Textdatei verwendete Trennzeichen anzugeben (z.B.: Leerzeichen, Kommar, Tabulator oder ein anderer regulärer Ausdruck im Perl-Stil), Nun drücken Sie den Knopf Analysieren und wählen die Spalten mit den X- und Y-Koordinaten aus.</translation>
    </message>
</context>
<context>
    <name>QgsDelimitedTextPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="13"/>
        <source>Create a Layer from a Delimited Text File</source>
        <translation>Textdatei aus Layer erzeugen</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="91"/>
        <source>&lt;p align=&quot;right&quot;&gt;X field&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;right&quot;&gt;X-Feld&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="110"/>
        <source>Name of the field containing x values</source>
        <translation>Nennen Sie das Feld, das die X-Werte enthält</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="113"/>
        <source>Name of the field containing x values. Choose a field from the list. The list is generated by parsing the header row of the delimited text file.</source>
        <translation>Nennen Sie das Feld, das die X-Werte enthält. Wähle ein Feld aus der Liste, die aus der Kopfzeile der Textdatei erzeugt wurde.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="123"/>
        <source>&lt;p align=&quot;right&quot;&gt;Y field&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;right&quot;&gt;Y Feld&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="142"/>
        <source>Name of the field containing y values</source>
        <translation>Nennen Sie das Feld, das die y-Werte enthält</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="145"/>
        <source>Name of the field containing y values. Choose a field from the list. The list is generated by parsing the header row of the delimited text file.</source>
        <translation>Nennen Sie das Feld, das die y-Werte enthält. Wähle ein Feld aus der Liste, die aus der Kopfzeile der Textdatei erzeugt wurde.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="229"/>
        <source>Layer name</source>
        <translation>Layername</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="236"/>
        <source>Name to display in the map legend</source>
        <translation>Name, der in der Kartenlegende angezeigt wird</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="239"/>
        <source>Name displayed in the map legend</source>
        <translation>Name, der in der Kartenlegende angezeigt wird</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="354"/>
        <source>Delimiter</source>
        <translation>Trennzeichen</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="373"/>
        <source>Delimiter to use when splitting fields in the text file. The delimiter can be more than one character.</source>
        <translation>Trennzeichen, das zum Aufspalten der Felder in der Textdatei verwendet wird. Das Trennzeichen kann mehr als ein Zeichen sein.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="376"/>
        <source>Delimiter to use when splitting fields in the delimited text file. The delimiter can be 1 or more characters in length.</source>
        <translation>Trennzeichen, das zum Aufspalten der Felder in der Textdatei verwendet wird. Das Trennzeichen kann ein oder mehrere Zeichen lang sein.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="51"/>
        <source>Delimited Text Layer</source>
        <translation>Layer aus Textdatei</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="174"/>
        <source>Delimited text file</source>
        <translation>Textdatei</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="181"/>
        <source>Full path to the delimited text file</source>
        <translation>Vollständiger Pfad zur Textdatei</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="184"/>
        <source>Full path to the delimited text file. In order to properly parse the fields in the file, the delimiter must be defined prior to entering the file name. Use the Browse button to the right of this field to choose the input file.</source>
        <translation>Vollständiger Pfad zur Textdatei. Um die Felder in der Datei ordentlich verarbeiten zu können, muß das Trennzeichen vor der Eingabe des Dateinamens definiert sein. Zum Wählen der Eingabedatei den Durchsuchen- Knopf rechts des Feldes verwenden.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="197"/>
        <source>Browse to find the delimited text file to be processed</source>
        <translation>Durchsuchen zum Finden der zuverarbeitenden Textdatei</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="200"/>
        <source>Use this button to browse to the location of the delimited text file. This button will not be enabled until a delimiter has been entered in the &lt;i&gt;Delimiter&lt;/i&gt; box. Once a file is chosen, the X and Y field drop-down boxes will be populated with the fields from the delimited text file.</source>
        <translation>Verwende diesen Knopf zum Durchsuchen nach dem Ort der Textdatei. Dieser Knopf ist nicht aktiv bis ein Trennzeichen in der &lt;i&gt;Trennzeichen&lt;/i&gt; Box eingegeben wurde. Wenn die Datei erst einmal gewählt wurde, werden die X- und Y-Feld Dropdownboxen mit den Feldern aus der Textdatei gefüllt.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="284"/>
        <source>Sample text</source>
        <translation>Beispieltext</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="203"/>
        <source>Browse...</source>
        <translation>Suchen...</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="386"/>
        <source>The delimiter is taken as is</source>
        <translation>Das Trennzeichen wurde wie vorhanden verwendet</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="389"/>
        <source>Plain characters</source>
        <translation>Klartext</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="399"/>
        <source>The delimiter is a regular expression</source>
        <translation>Dast Trennzeichen ist ein regulärer Ausdruck</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="402"/>
        <source>Regular expression</source>
        <translation>Regulärer Ausdruck</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="64"/>
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
        <location filename="../src/providers/delimitedtext/qgsdelimitedtextprovider.cpp" line="402"/>
        <source>Note: the following lines were not loaded because Qgis was unable to determine values for the x and y coordinates:
</source>
        <translation>Beachte: Die folgenden Linien wurden nicht geladen, da QGIS die Werte für die X und Y Koordinaten nicht herausfinden konnte.</translation>
    </message>
    <message>
        <location filename="../src/providers/delimitedtext/qgsdelimitedtextprovider.cpp" line="400"/>
        <source>Error</source>
        <translation>Fehler</translation>
    </message>
</context>
<context>
    <name>QgsDetailedItemWidgetBase</name>
    <message>
        <location filename="../src/ui/qgsdetaileditemwidgetbase.ui" line="13"/>
        <source>Form</source>
        <translation type="unfinished">Formular</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdetaileditemwidgetbase.ui" line="96"/>
        <source>Heading Label</source>
        <translation type="unfinished">Kopftitel</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdetaileditemwidgetbase.ui" line="117"/>
        <source>Detail label</source>
        <translation type="unfinished">Detailtitel</translation>
    </message>
</context>
<context>
    <name>QgsDlgPgBufferBase</name>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="13"/>
        <source>Buffer features</source>
        <translation>Puffereigenschaften</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="224"/>
        <source>Buffer distance in map units:</source>
        <translation>Gepufferte Entfernung in Karteneinheiten:</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="132"/>
        <source>Table name for the buffered layer:</source>
        <translation>Tabellenname für den gepufferten Layer:</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="172"/>
        <source>Create unique object id</source>
        <translation>Eindeutige Objekt ID erzeugen</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="216"/>
        <source>public</source>
        <translation>öffentlich</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="59"/>
        <source>Geometry column:</source>
        <translation>Geometriespalte:</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="92"/>
        <source>Spatial reference ID:</source>
        <translation>Räumliche Referenz ID:</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="125"/>
        <source>Unique field to use as feature id:</source>
        <translation>Eindeutiges Feld zur Verwendung als Objekt-ID:</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="118"/>
        <source>Schema:</source>
        <translation>Schema:</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="66"/>
        <source>Add the buffered layer to the map?</source>
        <translation>Den gepufferten Layer zur Karte hinzufügen?</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="234"/>
        <source>&lt;h2&gt;Buffer the features in layer: &lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Objekte im Layer puffern: &lt;/h2&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="28"/>
        <source>Parameters</source>
        <translation>Parameter</translation>
    </message>
</context>
<context>
    <name>QgsEncodingFileDialog</name>
    <message>
        <location filename="../src/gui/qgsencodingfiledialog.cpp" line="29"/>
        <source>Encoding:</source>
        <translation>Kodierung:</translation>
    </message>
</context>
<context>
    <name>QgsGPSDeviceDialog</name>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialog.cpp" line="45"/>
        <source>New device %1</source>
        <translation>Neues Gerät %1</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialog.cpp" line="59"/>
        <source>Are you sure?</source>
        <translation>Sind Sie sicher?</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialog.cpp" line="60"/>
        <source>Are you sure that you want to delete this device?</source>
        <translation>Sind Sie sicher, dass sie dieses Gerät löschen wollen?</translation>
    </message>
</context>
<context>
    <name>QgsGPSDeviceDialogBase</name>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="19"/>
        <source>GPS Device Editor</source>
        <translation>GPS-Geräteeditor</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="154"/>
        <source>This is the name of the device as it will appear in the lists</source>
        <translation>Dies ist der Name des Gerätes, so wie er in der Liste erscheinen wird</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="92"/>
        <source>Update device</source>
        <translation>Gerät zum Hinaufladen</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="79"/>
        <source>Delete device</source>
        <translation>Lösche Gerät</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="66"/>
        <source>New device</source>
        <translation>Neues Gerät</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="163"/>
        <source>Commands</source>
        <translation>Kommandos</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="204"/>
        <source>Waypoint download:</source>
        <translation>Wegpunkt herunterladen:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="267"/>
        <source>Waypoint upload:</source>
        <translation>Wegpunkt hinaufladen:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="218"/>
        <source>Route download:</source>
        <translation>Route herunterladen:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="197"/>
        <source>Route upload:</source>
        <translation>Route hinaufladen:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="190"/>
        <source>Track download:</source>
        <translation>Spur herunterladen:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="260"/>
        <source>The command that is used to upload tracks to the device</source>
        <translation>Das Kommando, welches gebraucht wird, um eine Spur auf das Gerät hochzuladen</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="232"/>
        <source>Track upload:</source>
        <translation>Spur hinaufladen:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="239"/>
        <source>The command that is used to download tracks from the device</source>
        <translation>Das Kommando, um Spuren vom Gerät herunterzuladen</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="246"/>
        <source>The command that is used to upload routes to the device</source>
        <translation>Das Kommando zum Hinaufladen von Routen zum Gerät</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="211"/>
        <source>The command that is used to download routes from the device</source>
        <translation>Das Kommando, um eine Route vom Gerät herunterzuladen</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="225"/>
        <source>The command that is used to upload waypoints to the device</source>
        <translation>Das Kommando, zum Heraufladen von Wegpunkten zum Gerät</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="253"/>
        <source>The command that is used to download waypoints from the device</source>
        <translation>Das Kommando, um Wegpunkte vom Gerät herunterzuladen</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="141"/>
        <source>Device name</source>
        <translation type="unfinished">Gerätename</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="283"/>
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
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="90"/>
        <source>&amp;Gps Tools</source>
        <translation>&amp;GPS Werkzeuge</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="91"/>
        <source>&amp;Create new GPX layer</source>
        <translation>Erstelle neuen GPX-Layer</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="94"/>
        <source>Creates a new GPX layer and displays it on the map canvas</source>
        <translation>Erzeugt einen neuen GPX-Layer und zeichnet ihn in das Kartenfenster.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="200"/>
        <source>&amp;Gps</source>
        <translation>&amp;Gps</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="160"/>
        <source>Save new GPX file as...</source>
        <translation>Neue GPX-Datei speichern als...</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="162"/>
        <source>GPS eXchange file (*.gpx)</source>
        <translation>GPS eXchange Datei (*.gpx)</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="169"/>
        <source>Could not create file</source>
        <translation>Kann die Datei nicht erzeugen.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="171"/>
        <source>Unable to create a GPX file with the given name. </source>
        <translation>Kann die GPX-Datei mit dem angegebenen Namen nicht erzeugen.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="172"/>
        <source>Try again with another name or in another </source>
        <translation>Bitte probieren Sie es mit einem anderen Namen oder in einem anderen </translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="172"/>
        <source>directory.</source>
        <translation>Ordner.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="213"/>
        <source>GPX Loader</source>
        <translation>GPX Lader</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="215"/>
        <source>Unable to read the selected file.
</source>
        <translation>Kann die ausgewählte Datei nicht lesen.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="215"/>
        <source>Please reselect a valid file.</source>
        <translation>Bitte wählen Sie eine gültige Datei aus.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="516"/>
        <source>Could not start process</source>
        <translation>Kann den Prozess nicht starten.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="517"/>
        <source>Could not start GPSBabel!</source>
        <translation>Kann GPSBabel nicht starten!</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="342"/>
        <source>Importing data...</source>
        <translation>Importiere Daten...</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="522"/>
        <source>Cancel</source>
        <translation>Abbrechen</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="284"/>
        <source>Could not import data from %1!

</source>
        <translation>Konnte die daten von %1 nicht importieren!</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="286"/>
        <source>Error importing data</source>
        <translation>Fehler beim Importieren der Daten</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="504"/>
        <source>Not supported</source>
        <translation>Nicht unterstützt</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="413"/>
        <source>This device does not support downloading </source>
        <translation>Dieses Gerät unterstützt den Download </translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="413"/>
        <source>of </source>
        <translation>von nicht.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="430"/>
        <source>Downloading data...</source>
        <translation>Daten werden heruntergeladen...</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="443"/>
        <source>Could not download data from GPS!

</source>
        <translation>Konnte die Daten nicht vom GPS-Gerät herunterladen.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="445"/>
        <source>Error downloading data</source>
        <translation>Fehler beim Herunterladen der Daten</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="505"/>
        <source>This device does not support uploading of </source>
        <translation>Dieses Gerät unterstützt das Hochladen von </translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="522"/>
        <source>Uploading data...</source>
        <translation>Daten werden hochgeladen...</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="535"/>
        <source>Error while uploading data to GPS!

</source>
        <translation>Fehler beim Hochladen der Daten ins GPS!</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="537"/>
        <source>Error uploading data</source>
        <translation>Fehler beim Hochladen der Daten</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="356"/>
        <source>Could not convert data from %1!

</source>
        <translation>Konnte Daten von %1 nicht konvertieren!</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="358"/>
        <source>Error converting data</source>
        <translation>Fehler beim Datenkonvertieren</translation>
    </message>
</context>
<context>
    <name>QgsGPSPluginGui</name>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="573"/>
        <source>GPS eXchange format (*.gpx)</source>
        <translation>GPS eXchange Format (*.gpx)</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="559"/>
        <source>Select GPX file</source>
        <translation>Wählen Sie GPX-Datei aus</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="261"/>
        <source>Select file and format to import</source>
        <translation>Datei und Format zum Importieren wählen.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="534"/>
        <source>Waypoints</source>
        <translation type="unfinished">Wegpunkte</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="534"/>
        <source>Routes</source>
        <translation type="unfinished">Routen</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="285"/>
        <source>Tracks</source>
        <translation type="unfinished">Spuren</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="539"/>
        <source>QGIS can perform conversions of GPX files, by using GPSBabel (%1) to perform the conversions.</source>
        <translation>QGIS kann GPX-Dateien mit Hilfe von GPSBabel (%1) konvertieren.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="540"/>
        <source>This requires that you have GPSBabel installed where QGIS can find it.</source>
        <translation>Dazu muss GPSBabel an einem Ort installiert sein, den QGIS finden kann.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="466"/>
        <source>GPX is the %1, which is used to store information about waypoints, routes, and tracks.</source>
        <translation>GPX ist das %1, das benutzt wird, um Informationen zu Wegpunkten, Routen und Spuren zu speichern.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="466"/>
        <source>GPS eXchange file format</source>
        <translation>GPS eXchange Dateiformat</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="467"/>
        <source>Select a GPX file and then select the feature types that you want to load.</source>
        <translation>Wählen Sie eine GPX Datei und dann die Objekttypen, die Sie laden möchten.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="480"/>
        <source>This tool will help you download data from a GPS device.</source>
        <translation>Dieses Werkzeug hilft Ihnen dabei, Daten von Ihrem GPS-Gerät herunterzuladen.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="481"/>
        <source>Choose your GPS device, the port it is connected to, the feature type you want to download, a name for your new layer, and the GPX file where you want to store the data.</source>
        <translation>Wählen Sie Ihr GPS-Gerät, den Port, an den es angeschlossen ist, den Objekttyp, den Sie herunterladen möchten, einen Namen für den neuen Layer und die GPX-Datei, als die Sie die Daten speichern möchten.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="501"/>
        <source>If your device isn&apos;t listed, or if you want to change some settings, you can also edit the devices.</source>
        <translation>Wenn Ihr Gerät nicht aufgelistet ist, oder Sie Einstellungen ändern möchten, können Sie die Geräteeinstellungen editieren.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="503"/>
        <source>This tool uses the program GPSBabel (%1) to transfer the data.</source>
        <translation>Dieses Werkzeug benutzt GPSBabel (%1), um die Daten zu transferieren.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="499"/>
        <source>This tool will help you upload data from a GPX layer to a GPS device.</source>
        <translation>Dieses Werkzeug hilft Ihnen, Daten aus einem GPX-Layer auf ein GPS-Gerät zu spielen.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="500"/>
        <source>Choose the layer you want to upload, the device you want to upload it to, and the port your device is connected to.</source>
        <translation>Wählen Sie einen Layer, den sie hochladen möchten, das GPS-Gerät und den Port, über den das Gerät verbunden ist.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="519"/>
        <source>QGIS can only load GPX files by itself, but many other formats can be converted to GPX using GPSBabel (%1).</source>
        <translation>QGIS selbst kann nur GPX-Dateien laden, aber viele andere Formate können nach GPX konvertiert werden mit  (%1).</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="522"/>
        <source>All file formats can not store waypoints, routes, and tracks, so some feature types may be disabled for some file formats.</source>
        <translation>Nicht alle Dateiformate können Wegpunkte, Routen und Spuren speichern, daher können einige Objekttypen für verschiedene Formate deaktiviert sein.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="571"/>
        <source>Choose a file name to save under</source>
        <translation type="unfinished">Dateiname zum Speichern wählen</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="521"/>
        <source>Select a GPS file format and the file that you want to import, the feature type that you want to use, a GPX file name that you want to save the converted file as, and a name for the new layer.</source>
        <translation type="unfinished">Das GPS-Dateiformat, die zu importierende Datei, den zu benutzenden Objekttyp, den Namen der GPX-Datei in der die konvertierten Daten gespeichert werden sollen und den Namen des neuen Layers wählen.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="541"/>
        <source>Select a GPX input file name, the type of conversion you want to perform, a GPX file name that you want to save the converted file as, and a name for the new layer created from the result.</source>
        <translation type="unfinished">GPX-Eingabedateinamen, Typ der vorzunehmenden Konvertierung, Name der zu konvertierenden GPX-Datei und Name des neuen Layers für das Ergebnis wählen.</translation>
    </message>
</context>
<context>
    <name>QgsGPSPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="13"/>
        <source>GPS Tools</source>
        <translation>GPS Werkzeuge</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="70"/>
        <source>Load GPX file</source>
        <translation>GPX-Datei laden</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="116"/>
        <source>File:</source>
        <translation>Datei:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="133"/>
        <source>Feature types:</source>
        <translation>Objekttypen:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="419"/>
        <source>Waypoints</source>
        <translation>Wegpunkte</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="424"/>
        <source>Routes</source>
        <translation>Routen</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="429"/>
        <source>Tracks</source>
        <translation>Spuren</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="180"/>
        <source>Import other file</source>
        <translation>Aus anderer Datei importieren</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="298"/>
        <source>File to import:</source>
        <translation>Zu importierende Datei:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="405"/>
        <source>Feature type:</source>
        <translation>Objekttyp:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="641"/>
        <source>GPX output file:</source>
        <translation>GPX Ausgabedatei:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="617"/>
        <source>Layer name:</source>
        <translation>Layername:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="306"/>
        <source>Download from GPS</source>
        <translation>Vom GPS herunterladen</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="555"/>
        <source>Edit devices</source>
        <translation>Editiere Geräte</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="562"/>
        <source>GPS device:</source>
        <translation>GPS Gerät:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="437"/>
        <source>Output file:</source>
        <translation>Ausgabedatei:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="528"/>
        <source>Port:</source>
        <translation>Port:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="465"/>
        <source>Upload to GPS</source>
        <translation>Zum GPS hochladen</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="569"/>
        <source>Data layer:</source>
        <translation>Datenlayer:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="631"/>
        <source>Browse...</source>
        <translation>Suchen...</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="624"/>
        <source>Save As...</source>
        <translation type="unfinished">Speichern unter...</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="284"/>
        <source>(Note: Selecting correct file type in browser dialog important!)</source>
        <translation>(Bemerkung: Die Auswahl des richtigen Datentyps ist wichtig!)</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="577"/>
        <source>GPX Conversions</source>
        <translation>GPX-Konvertierung</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="674"/>
        <source>Conversion:</source>
        <translation>Konvertierung:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="688"/>
        <source>GPX input file:</source>
        <translation>GPX-Eingabedatei</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="604"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="342"/>
        <source>Edit devices...</source>
        <translation type="unfinished">Gerät bearbeiten...</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="382"/>
        <source>Refresh</source>
        <translation type="unfinished">Aktualisieren</translation>
    </message>
</context>
<context>
    <name>QgsGPXProvider</name>
    <message>
        <location filename="../src/providers/gpx/qgsgpxprovider.cpp" line="72"/>
        <source>Bad URI - you need to specify the feature type.</source>
        <translation>Falsche URI - Bitte geben Sie den Featuretype an.</translation>
    </message>
    <message>
        <location filename="../src/providers/gpx/qgsgpxprovider.cpp" line="120"/>
        <source>GPS eXchange file</source>
        <translation>GPS eXchange Datei</translation>
    </message>
    <message>
        <location filename="../src/providers/gpx/qgsgpxprovider.cpp" line="815"/>
        <source>Digitized in QGIS</source>
        <translation>Digitalisiert mit QGIS</translation>
    </message>
</context>
<context>
    <name>QgsGenericProjectionSelector</name>
    <message>
        <location filename="../src/gui/qgsgenericprojectionselector.cpp" line="43"/>
        <source>Define this layer&apos;s projection:</source>
        <translation type="unfinished">Definiere die Projektion des Layers:</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsgenericprojectionselector.cpp" line="44"/>
        <source>This layer appears to have no projection specification.</source>
        <translation type="unfinished">Dieser Layer scheint keine Projektionsangaben zu besitzen.</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsgenericprojectionselector.cpp" line="46"/>
        <source>By default, this layer will now have its projection set to that of the project, but you may override this by selecting a different projection below.</source>
        <translation type="unfinished">Als standard wird die Projektion dieses Layers auf die des Projektes gesetzt, aber Sie können es durch auswählen einer anderen Projektion unten überschreiben.</translation>
    </message>
</context>
<context>
    <name>QgsGenericProjectionSelectorBase</name>
    <message>
        <location filename="../src/ui/qgsgenericprojectionselectorbase.ui" line="13"/>
        <source>Projection Selector</source>
        <translation type="unfinished">Projektionsauswahl</translation>
    </message>
</context>
<context>
    <name>QgsGeomTypeDialog</name>
    <message>
        <location filename="../src/app/qgsgeomtypedialog.cpp" line="30"/>
        <source>Real</source>
        <translation type="unfinished">Fließkommazahl</translation>
    </message>
    <message>
        <location filename="../src/app/qgsgeomtypedialog.cpp" line="31"/>
        <source>Integer</source>
        <translation type="unfinished">Ganzzahl</translation>
    </message>
    <message>
        <location filename="../src/app/qgsgeomtypedialog.cpp" line="32"/>
        <source>String</source>
        <translation type="unfinished">Zeichenkette</translation>
    </message>
</context>
<context>
    <name>QgsGeomTypeDialogBase</name>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="155"/>
        <source>Type</source>
        <translation>Typ</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="41"/>
        <source>Point</source>
        <translation>Punkt</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="48"/>
        <source>Line</source>
        <translation>Linie</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="55"/>
        <source>Polygon</source>
        <translation>Polygon</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="13"/>
        <source>New Vector Layer</source>
        <translation>Neuer Vektorlayer</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="22"/>
        <source>File format</source>
        <translation type="unfinished">Dateiformat</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="65"/>
        <source>Attributes</source>
        <translation type="unfinished">Attribute</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="150"/>
        <source>Name</source>
        <translation type="unfinished">Name</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="127"/>
        <source>...</source>
        <translation type="unfinished">...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="111"/>
        <source>Delete selected attribute</source>
        <translation type="unfinished">Gewähltes Attribut löschen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="124"/>
        <source>Add attribute</source>
        <translation type="unfinished">Attribute hinzufügen</translation>
    </message>
</context>
<context>
    <name>QgsGeorefDescriptionDialogBase</name>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefdescriptiondialogbase.ui" line="13"/>
        <source>Description georeferencer</source>
        <translation>Beschreibung Georeferenzierer</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefdescriptiondialogbase.ui" line="44"/>
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
        <location filename="../src/plugins/georeferencer/plugin.cpp" line="119"/>
        <source>&amp;Georeferencer</source>
        <translation>&amp;Georeferenzierer</translation>
    </message>
</context>
<context>
    <name>QgsGeorefPluginGui</name>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="86"/>
        <source>Choose a raster file</source>
        <translation>Wähle eine Rasterkarte</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="88"/>
        <source>Raster files (*.*)</source>
        <translation>Rasterkarten (*.*)</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="99"/>
        <source>Error</source>
        <translation type="unfinished">Fehler</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="100"/>
        <source>The selected file is not a valid raster file.</source>
        <translation>Die ausgewählte Karte ist keine gültige Rasterdatei.</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="127"/>
        <source>World file exists</source>
        <translation>World file existiert</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="129"/>
        <source>&lt;p&gt;The selected file already seems to have a </source>
        <translation>&lt;p&gt;Die ausgewählte Datei hat scheinbar bereits einen </translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="130"/>
        <source>world file! Do you want to replace it with the </source>
        <translation>World file! Möchten Sie ihn ersetzen mit dem </translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="130"/>
        <source>new world file?&lt;/p&gt;</source>
        <translation>neuen World file?&lt;/p&gt;</translation>
    </message>
</context>
<context>
    <name>QgsGeorefPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/georeferencer/pluginguibase.ui" line="13"/>
        <source>Georeferencer</source>
        <translation>Georeferenzierer</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/pluginguibase.ui" line="99"/>
        <source>Close</source>
        <translation>Schließen</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/pluginguibase.ui" line="45"/>
        <source>...</source>
        <translation type="unfinished">...</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/pluginguibase.ui" line="22"/>
        <source>Raster file:</source>
        <translation>Rasterdatei:</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/pluginguibase.ui" line="76"/>
        <source>Arrange plugin windows</source>
        <translation>Fenster anordnen</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/pluginguibase.ui" line="69"/>
        <source>Description...</source>
        <translation>Beschreibung...</translation>
    </message>
</context>
<context>
    <name>QgsGeorefWarpOptionsDialogBase</name>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="13"/>
        <source>Warp options</source>
        <translation>Verzerrungsoptionen</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="35"/>
        <source>Resampling method:</source>
        <translation>Stichprobenmethode:</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="46"/>
        <source>Nearest neighbour</source>
        <translation>Nächster Nachbar</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="51"/>
        <source>Linear</source>
        <translation>Linear</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="56"/>
        <source>Cubic</source>
        <translation>Kubisch</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="74"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="64"/>
        <source>Use 0 for transparency when needed</source>
        <translation>0 für Transparanz verwenden falls benötigt</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="28"/>
        <source>Compression:</source>
        <translation>Komprimierung:</translation>
    </message>
</context>
<context>
    <name>QgsGraduatedSymbolDialog</name>
    <message>
        <location filename="../src/app/qgsgraduatedsymboldialog.cpp" line="368"/>
        <source>Equal Interval</source>
        <translation>Gleiches Intervall</translation>
    </message>
    <message>
        <location filename="../src/app/qgsgraduatedsymboldialog.cpp" line="343"/>
        <source>Quantiles</source>
        <translation>Quantile</translation>
    </message>
    <message>
        <location filename="../src/app/qgsgraduatedsymboldialog.cpp" line="394"/>
        <source>Empty</source>
        <translation>Leer</translation>
    </message>
</context>
<context>
    <name>QgsGraduatedSymbolDialogBase</name>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="25"/>
        <source>graduated Symbol</source>
        <translation>abgestuftes Symbol</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="188"/>
        <source>Delete class</source>
        <translation type="unfinished">Klasse löschen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="181"/>
        <source>Classify</source>
        <translation type="unfinished">Klassifizieren</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="55"/>
        <source>Classification field</source>
        <translation type="unfinished">Klassifikationsfeld</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="93"/>
        <source>Mode</source>
        <translation type="unfinished">Modus</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="131"/>
        <source>Number of classes</source>
        <translation type="unfinished">Klassenanzahl</translation>
    </message>
</context>
<context>
    <name>QgsGrassAttributes</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="266"/>
        <source>Warning</source>
        <translation>Warnung</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="122"/>
        <source>Column</source>
        <translation type="unfinished">Spalte</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="123"/>
        <source>Value</source>
        <translation type="unfinished">Wert</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="123"/>
        <source>Type</source>
        <translation type="unfinished">Typ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="267"/>
        <source>ERROR</source>
        <translation type="unfinished">FEHLER</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="271"/>
        <source>OK</source>
        <translation type="unfinished">OK</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="157"/>
        <source>Layer</source>
        <translation type="unfinished">Layer</translation>
    </message>
</context>
<context>
    <name>QgsGrassAttributesBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="43"/>
        <source>GRASS Attributes</source>
        <translation>GRASS Attribute</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="80"/>
        <source>Tab 1</source>
        <translation>Tab 1</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="112"/>
        <source>result</source>
        <translation>Resultat</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="185"/>
        <source>Update</source>
        <translation>Aktualisieren</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="182"/>
        <source>Update database record</source>
        <translation>Aktualisiere Datenbankeintrag</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="213"/>
        <source>New</source>
        <translation>Neu</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="210"/>
        <source>Add new category using settings in GRASS Edit toolbox</source>
        <translation>Eine neue Kategorie mit den Einstellungen der &apos;GRASS Digitalisieren&apos;-Werkzeugkiste hinzufügen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="241"/>
        <source>Delete</source>
        <translation>Löschen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="238"/>
        <source>Delete selected category</source>
        <translation>Lösche gewählte Kategorie</translation>
    </message>
</context>
<context>
    <name>QgsGrassBrowser</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="43"/>
        <source>Tools</source>
        <translation>Werkzeuge</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="47"/>
        <source>Add selected map to canvas</source>
        <translation>Ausgewählten Layer dem Kartenfenster hinzufügen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="55"/>
        <source>Copy selected map</source>
        <translation>Gewählte Karte kopieren</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="63"/>
        <source>Rename selected map</source>
        <translation>Gewählte Karte umbenennen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="71"/>
        <source>Delete selected map</source>
        <translation>Gewählte Karte löschen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="79"/>
        <source>Set current region to selected map</source>
        <translation>Setze die Region auf die gewählte Karte</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="87"/>
        <source>Refresh</source>
        <translation>Neu zeichnen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="412"/>
        <source>Warning</source>
        <translation>Warnung</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="256"/>
        <source>Cannot copy map </source>
        <translation>Kann die Karte nicht kopieren.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="374"/>
        <source>&lt;br&gt;command: </source>
        <translation>&lt;br&gt; Kommando: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="320"/>
        <source>Cannot rename map </source>
        <translation>Kann die Karte nicht umbenennen </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="356"/>
        <source>Delete map &lt;b&gt;</source>
        <translation>Lösche Karte &lt;b&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="373"/>
        <source>Cannot delete map </source>
        <translation>Kann die Karte nicht löschen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="413"/>
        <source>Cannot write new region</source>
        <translation>Kann die neue Region nicht schreiben.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="304"/>
        <source>New name</source>
        <translation type="unfinished">Neuer Name</translation>
    </message>
</context>
<context>
    <name>QgsGrassEdit</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="259"/>
        <source>New point</source>
        <translation>Neuer Punkt</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="277"/>
        <source>New centroid</source>
        <translation>Neues Zentroid</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="295"/>
        <source>Delete vertex</source>
        <translation>Lösche Vertex</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1885"/>
        <source>Left: </source>
        <translation>Links:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1886"/>
        <source>Middle: </source>
        <translation>Mitte:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="256"/>
        <source>Edit tools</source>
        <translation>Digitalisierwerkzeuge</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="265"/>
        <source>New line</source>
        <translation>Neue Zeile</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="271"/>
        <source>New boundary</source>
        <translation>Neue Grenze</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="283"/>
        <source>Move vertex</source>
        <translation>Verschiebe Vertex</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="289"/>
        <source>Add vertex</source>
        <translation>Vertex hinzufügen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="301"/>
        <source>Move element</source>
        <translation>Verschiebe Element</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="307"/>
        <source>Split line</source>
        <translation>Unterteile Linie</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="313"/>
        <source>Delete element</source>
        <translation>Element löschen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="319"/>
        <source>Edit attributes</source>
        <translation>Editiere Attribute</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="324"/>
        <source>Close</source>
        <translation>Schließen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1522"/>
        <source>Warning</source>
        <translation>Warnung</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="238"/>
        <source>You are not owner of the mapset, cannot open the vector for editing.</source>
        <translation>Sie sind nicht Besitzer des Mapsets. Folglich kann der Datensatz nicht zum Editieren geöffnet werden.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="244"/>
        <source>Cannot open vector for update.</source>
        <translation>Kann die Vekordatei nicht zum Aktualisiern öffnen.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="699"/>
        <source>Info</source>
        <translation>Information</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="699"/>
        <source>The table was created</source>
        <translation>Die Tabelle wurde erstellt.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1382"/>
        <source>Tool not yet implemented.</source>
        <translation>Werkzeug ist noch nicht implementiert.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1404"/>
        <source>Cannot check orphan record: </source>
        <translation>Kann den verwaisten Eintrag nicht überprüfen: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1411"/>
        <source>Orphan record was left in attribute table. &lt;br&gt;Delete the record?</source>
        <translation>Verwaister Eintrag in der Attributtabelle gefunden. &lt;br&gt; Diesen Eintrag löschen?</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1421"/>
        <source>Cannot delete orphan record: </source>
        <translation>Kann den verwaisten Eintrag nicht löschen:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1454"/>
        <source>Cannot describe table for field </source>
        <translation>Kann Tabelle oder Feld nicht beschreiben</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="407"/>
        <source>Background</source>
        <translation type="unfinished">Hintergrund</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="408"/>
        <source>Highlight</source>
        <translation type="unfinished">Hervorheben</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="409"/>
        <source>Dynamic</source>
        <translation type="unfinished">Dynamisch</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="410"/>
        <source>Point</source>
        <translation type="unfinished">Punkt</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="411"/>
        <source>Line</source>
        <translation type="unfinished">Linie</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="412"/>
        <source>Boundary (no area)</source>
        <translation>Grenzlinie (keine Fläche)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="413"/>
        <source>Boundary (1 area)</source>
        <translation>Grenzlinie (eine Fläche)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="414"/>
        <source>Boundary (2 areas)</source>
        <translation>Grenzlinie (zwei Flächen)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="415"/>
        <source>Centroid (in area)</source>
        <translation>Zentroid (innerhalb der Fläche)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="416"/>
        <source>Centroid (outside area)</source>
        <translation>Zentroid (außerhalb der Fläche)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="417"/>
        <source>Centroid (duplicate in area)</source>
        <translation>Zentroid (Duplikat in der Fläche)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="418"/>
        <source>Node (1 line)</source>
        <translation>Knotenpunkt (1 Linie)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="419"/>
        <source>Node (2 lines)</source>
        <translation>Knotenpunkt (2 Linien)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="548"/>
        <source>Next not used</source>
        <translation>Nächst folgender Kategoriewert</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="549"/>
        <source>Manual entry</source>
        <translation type="unfinished">Manueller Eintrag</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="550"/>
        <source>No category</source>
        <translation type="unfinished">Keine Kategorie</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1887"/>
        <source>Right: </source>
        <translation type="unfinished">Rechts: </translation>
    </message>
</context>
<context>
    <name>QgsGrassEditBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="13"/>
        <source>GRASS Edit</source>
        <translation>GRASS Digitalisieren</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="84"/>
        <source>Category</source>
        <translation>Kategorie</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="54"/>
        <source>Mode</source>
        <translation>Modus</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="154"/>
        <source>Settings</source>
        <translation>Einstellungen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="162"/>
        <source>Snapping in screen pixels</source>
        <translation>Snappingtoleranz in Bildschirmpixel</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="213"/>
        <source>Symbology</source>
        <translation>Darstellung</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="327"/>
        <source>Table</source>
        <translation>Tabelle</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="416"/>
        <source>Add Column</source>
        <translation>Attribut hinzufügen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="423"/>
        <source>Create / Alter Table</source>
        <translation>Tabelle erzeugen/verändern</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="227"/>
        <source>Line width</source>
        <translation>Linienbreite</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="250"/>
        <source>Marker size</source>
        <translation>Markergröße</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="335"/>
        <source>Layer</source>
        <translation type="unfinished">Layer</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="295"/>
        <source>Disp</source>
        <translation type="unfinished">Anz.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="300"/>
        <source>Color</source>
        <translation type="unfinished">Farbe</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="388"/>
        <source>Type</source>
        <translation type="unfinished">Typ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="310"/>
        <source>Index</source>
        <translation type="unfinished">Index</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="383"/>
        <source>Column</source>
        <translation type="unfinished">Spalte</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="393"/>
        <source>Length</source>
        <translation type="unfinished">Länge</translation>
    </message>
</context>
<context>
    <name>QgsGrassElementDialog</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="114"/>
        <source>Cancel</source>
        <translation type="unfinished">Abbrechen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="143"/>
        <source>Ok</source>
        <translation type="unfinished">OK</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="148"/>
        <source>&lt;font color=&apos;red&apos;&gt;Enter a name!&lt;/font&gt;</source>
        <translation type="unfinished">&lt;font color=&apos;red&apos;&gt;Name eingeben!&lt;/font&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="159"/>
        <source>&lt;font color=&apos;red&apos;&gt;This is name of the source!&lt;/font&gt;</source>
        <translation>&lt;font color=&apos;red&apos;&gt;Dies ist der Name der Quelle!&lt;/font&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="165"/>
        <source>&lt;font color=&apos;red&apos;&gt;Exists!&lt;/font&gt;</source>
        <translation>&lt;font color=&apos;red&apos;&gt;Existiert!&lt;/font&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="166"/>
        <source>Overwrite</source>
        <translation type="unfinished">Überschreiben</translation>
    </message>
</context>
<context>
    <name>QgsGrassMapcalc</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="77"/>
        <source>Mapcalc tools</source>
        <translation>Mapcalc-Werkzeug</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="80"/>
        <source>Add map</source>
        <translation>Karte hinzufügen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="87"/>
        <source>Add constant value</source>
        <translation>Konstanten Wert hinzufügen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="94"/>
        <source>Add operator or function</source>
        <translation>Operator oder Funktion hinzufügen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="101"/>
        <source>Add connection</source>
        <translation>Verbindung hinzufügen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="108"/>
        <source>Select item</source>
        <translation>Objekt wählen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="115"/>
        <source>Delete selected item</source>
        <translation>Lösche gewähltes Objekt</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="125"/>
        <source>Open</source>
        <translation>Öffnen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="130"/>
        <source>Save</source>
        <translation>Speichern</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="136"/>
        <source>Save as</source>
        <translation>Speichern unter</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="144"/>
        <source>Addition</source>
        <translation>Addition</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="145"/>
        <source>Subtraction</source>
        <translation>Subtraktion</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="146"/>
        <source>Multiplication</source>
        <translation>Multiplikation</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="147"/>
        <source>Division</source>
        <translation>Teilung</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="148"/>
        <source>Modulus</source>
        <translation>Modus</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="149"/>
        <source>Exponentiation</source>
        <translation>Exponent</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="152"/>
        <source>Equal</source>
        <translation>gleich</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="153"/>
        <source>Not equal</source>
        <translation>ungleich</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="154"/>
        <source>Greater than</source>
        <translation>größer als</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="155"/>
        <source>Greater than or equal</source>
        <translation>größer gleich</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="156"/>
        <source>Less than</source>
        <translation>kleiner als</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="157"/>
        <source>Less than or equal</source>
        <translation>kleiner gleich</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="158"/>
        <source>And</source>
        <translation>Und</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="159"/>
        <source>Or</source>
        <translation>Oder</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="162"/>
        <source>Absolute value of x</source>
        <translation>Absoluter Wert für x</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="163"/>
        <source>Inverse tangent of x (result is in degrees)</source>
        <translation>Inverser tangenz von X (Resultat in Grad),</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="164"/>
        <source>Inverse tangent of y/x (result is in degrees)</source>
        <translation>Inverser Tangenz von y/x (Resultat in Grad).</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="165"/>
        <source>Current column of moving window (starts with 1)</source>
        <translation>Aktuelle Spalte des Moving Windows (startet bei 1)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="166"/>
        <source>Cosine of x (x is in degrees)</source>
        <translation>Kosinus von X (X in Grad).</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="167"/>
        <source>Convert x to double-precision floating point</source>
        <translation>Konvertiert x in doppelte Fließkommazahl</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="168"/>
        <source>Current east-west resolution</source>
        <translation>Aktuelle Ost-West-Auflösung</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="169"/>
        <source>Exponential function of x</source>
        <translation>Exponentielle Funktion von x</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="170"/>
        <source>x to the power y</source>
        <translation>x hoch y</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="171"/>
        <source>Convert x to single-precision floating point</source>
        <translation>Konvertiert x in einfache Fließkommazahl</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="172"/>
        <source>Decision: 1 if x not zero, 0 otherwise</source>
        <translation>Entscheidung: 1 wenn x nicht NULL, andererseits 0</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="173"/>
        <source>Decision: a if x not zero, 0 otherwise</source>
        <translation>Entscheidung: a wenn x nicht NULL, andererseits 0</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="174"/>
        <source>Decision: a if x not zero, b otherwise</source>
        <translation>Entscheidung: a wenn x nicht NULL, andererseits b</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="175"/>
        <source>Decision: a if x &gt; 0, b if x is zero, c if x &lt; 0</source>
        <translation>Entscheidung: a wenn x &gt; 0, c wenn x &lt; 0</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="176"/>
        <source>Convert x to integer [ truncates ]</source>
        <translation>Konvertiert x zu integer [ schneidet ab ]</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="177"/>
        <source>Check if x = NULL</source>
        <translation>Überprüfe, wenn x = NULL</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="178"/>
        <source>Natural log of x</source>
        <translation>Natürlicher Log von x </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="179"/>
        <source>Log of x base b</source>
        <translation>Log von x zur Basis b</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="181"/>
        <source>Largest value</source>
        <translation>Maximum</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="183"/>
        <source>Median value</source>
        <translation>Median</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="185"/>
        <source>Smallest value</source>
        <translation>Minimum</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="187"/>
        <source>Mode value</source>
        <translation>Modus</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="188"/>
        <source>1 if x is zero, 0 otherwise</source>
        <translation>1 wenn x zero ist, ansonsten 0</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="189"/>
        <source>Current north-south resolution</source>
        <translation>Aktuelle Nord-Süd-Auflösung</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="190"/>
        <source>NULL value</source>
        <translation>NULL Wert</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="191"/>
        <source>Random value between a and b</source>
        <translation>Zufallswert zwischen a und b</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="192"/>
        <source>Round x to nearest integer</source>
        <translation>Runde x zum nächsten Integerwert</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="193"/>
        <source>Current row of moving window (Starts with 1)</source>
        <translation>Aktuelle Zeile des Moving windows (startet bei 1)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="194"/>
        <source>Sine of x (x is in degrees)</source>
        <comment>sin(x)</comment>
        <translation>Sinus von X (x in Grad).</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="195"/>
        <source>Square root of x</source>
        <comment>sqrt(x)</comment>
        <translation>Wurzel von x</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="196"/>
        <source>Tangent of x (x is in degrees)</source>
        <comment>tan(x)</comment>
        <translation>Tangenz von X (X in Grad).</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="197"/>
        <source>Current x-coordinate of moving window</source>
        <translation>Aktuelle X-Koordinate des Moving-Windows.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="198"/>
        <source>Current y-coordinate of moving window</source>
        <translation>Aktuelle Y-Koordinate des Moving-Windows.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1259"/>
        <source>Warning</source>
        <translation>Warnung</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="538"/>
        <source>Cannot get current region</source>
        <translation>Kann die aktuelle Region nicht ermitteln.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="517"/>
        <source>Cannot check region of map </source>
        <translation>Kann die Region der Karte nicht überprüfen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="572"/>
        <source>Cannot get region of map </source>
        <translation>Kann die Region der Karte nicht ermitteln</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="769"/>
        <source>No GRASS raster maps currently in QGIS</source>
        <translation>Derzeit sind keine GRASS-Rasterkarten in QGIS geladen.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1045"/>
        <source>Cannot create &apos;mapcalc&apos; directory in current mapset.</source>
        <translation>Kann keinen &apos;Mapcalc&apos;-Odner im aktuellen Mapset erstellen.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1055"/>
        <source>New mapcalc</source>
        <translation>Neue Mapcalc</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1056"/>
        <source>Enter new mapcalc name:</source>
        <translation>Einen neuen Mapcalc-Namen eingeben:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1062"/>
        <source>Enter vector name</source>
        <translation>Name für die Vektordatei eingeben.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1070"/>
        <source>The file already exists. Overwrite? </source>
        <translation>Die Datei existiert bereits. Überschreiben?</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1106"/>
        <source>Save mapcalc</source>
        <translation>mapcalc speichern</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1088"/>
        <source>File name empty</source>
        <translation>Dateiname leer.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1107"/>
        <source>Cannot open mapcalc file</source>
        <translation>Kann die Mapcalc-Datei nicht öffnen.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1236"/>
        <source>The mapcalc schema (</source>
        <translation>Das Mapcalc-Schema (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1236"/>
        <source>) not found.</source>
        <translation>) nicht gefunden.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1243"/>
        <source>Cannot open mapcalc schema (</source>
        <translation>Kann das Mapcalc-Schema nicht öffnen (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1255"/>
        <source>Cannot read mapcalc schema (</source>
        <translation>Kann das Mapcalc-Schema nicht lesen (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1256"/>
        <source>
at line </source>
        <translation>
bei Zeile </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1257"/>
        <source> column </source>
        <translation> Spalte </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1330"/>
        <source>Output</source>
        <translation type="unfinished">Ergebnis</translation>
    </message>
</context>
<context>
    <name>QgsGrassMapcalcBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalcbase.ui" line="13"/>
        <source>MainWindow</source>
        <translation>MainWindow</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalcbase.ui" line="22"/>
        <source>Output</source>
        <translation>Ergebnis</translation>
    </message>
</context>
<context>
    <name>QgsGrassModule</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1382"/>
        <source>Run</source>
        <translation>Los</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1354"/>
        <source>Stop</source>
        <translation>Stop</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="163"/>
        <source>Module</source>
        <translation type="unfinished">Modul</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1348"/>
        <source>Warning</source>
        <translation type="unfinished">Warnung</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="177"/>
        <source>The module file (</source>
        <translation type="unfinished">Die Moduldatei </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="177"/>
        <source>) not found.</source>
        <translation type="unfinished">) nicht gefunden.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="182"/>
        <source>Cannot open module file (</source>
        <translation type="unfinished">Moduldatei nicht geöffnet (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="984"/>
        <source>)</source>
        <translation type="unfinished">)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="979"/>
        <source>Cannot read module file (</source>
        <translation type="unfinished">Konnte Moduldatei nicht lesen (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="979"/>
        <source>):
</source>
        <translation type="unfinished">):
</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="980"/>
        <source>
at line </source>
        <translation type="unfinished">
bei Zeile </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="219"/>
        <source>Module </source>
        <translation type="unfinished">Modul </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="219"/>
        <source> not found</source>
        <translation type="unfinished"> nicht gefunden</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="263"/>
        <source>Cannot find man page </source>
        <translation type="unfinished">Handbuchseite nicht gefunden: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="968"/>
        <source>Not available, description not found (</source>
        <translation type="unfinished">Nicht verfügbar, Beschreibung nicht gefunden (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="972"/>
        <source>Not available, cannot open description (</source>
        <translation type="unfinished">Nicht verfügbar, konnte Beschreibung nicht öffnen (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="980"/>
        <source> column </source>
        <translation type="unfinished"> Spalte </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="984"/>
        <source>Not available, incorrect description (</source>
        <translation type="unfinished">Nicht verfügbar, falsche Beschreibung (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1174"/>
        <source>Cannot get input region</source>
        <translation>Konnte Eingabe-&apos;region&apos; nicht finden</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1162"/>
        <source>Use Input Region</source>
        <translation>Eingabe-&apos;region&apos; benutzen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1277"/>
        <source>Cannot find module </source>
        <translation type="unfinished">Konnte Modul nicht finden: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1349"/>
        <source>Cannot start module: </source>
        <translation type="unfinished">Konnte Modul nicht starten: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1367"/>
        <source>&lt;B&gt;Successfully finished&lt;/B&gt;</source>
        <translation type="unfinished">&lt;B&gt;Erfolgreich beendet&lt;/B&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1375"/>
        <source>&lt;B&gt;Finished with error&lt;/B&gt;</source>
        <translation type="unfinished">&lt;B&gt;Mit Fehler beendet&lt;/B&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1380"/>
        <source>&lt;B&gt;Module crashed or killed&lt;/B&gt;</source>
        <translation type="unfinished">&lt;B&gt;Modul abgestürzt oder abgebrochen&lt;/B&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="264"/>
        <source>Please ensure you have the GRASS documentation installed.</source>
        <translation type="unfinished">Bitte stellen Sie sicher, dass die GRASS-Dokumentation installiert ist.</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="13"/>
        <source>GRASS Module</source>
        <translation>GRASS Modul</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="20"/>
        <source>Options</source>
        <translation>Optionen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="25"/>
        <source>Output</source>
        <translation>Ergebnis</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="47"/>
        <source>Manual</source>
        <translation>Handbuch</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="116"/>
        <source>Run</source>
        <translation>Starten</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="159"/>
        <source>Close</source>
        <translation>Schließen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="139"/>
        <source>View output</source>
        <translation>Ergebnis visualisieren</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="78"/>
        <source>TextLabel</source>
        <translation type="unfinished">Textbeschriftung</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleField</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2812"/>
        <source>Attribute field</source>
        <translation type="unfinished">Attributfeld</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleFile</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="3020"/>
        <source>File</source>
        <translation type="unfinished">Datei</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="3136"/>
        <source>:&amp;nbsp;missing value</source>
        <translation type="unfinished">:&amp;nbsp;fehlender Wert</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="3143"/>
        <source>:&amp;nbsp;directory does not exist</source>
        <translation type="unfinished">:&amp;nbsp;Verzeichnis existiert nicht</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleGdalInput</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2762"/>
        <source>Warning</source>
        <translation type="unfinished">Warnung</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2617"/>
        <source>Cannot find layeroption </source>
        <translation type="unfinished">Kann Layeroption nicht finden: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2766"/>
        <source>PostGIS driver in OGR does not support schemas!&lt;br&gt;Only the table name will be used.&lt;br&gt;It can result in wrong input if more tables of the same name&lt;br&gt;are present in the database.</source>
        <translation type="unfinished">Der PostGIS-Treiber in OGR unterstützt keine Schemata!&lt;br&gt;Nur der Tabellenname wird benutzt.&lt;br&gt;Die kann zu falschen Eingaben führen, wenn mehrere Tabellen gleichen Namens&lt;br&gt;in der Datenbank vorkommen.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2790"/>
        <source>:&amp;nbsp;no input</source>
        <translation type="unfinished">:&amp;nbsp;keine Eingabe</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2632"/>
        <source>Cannot find whereoption </source>
        <translation>Kann where Option nicht finden  </translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleInput</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2144"/>
        <source>Warning</source>
        <translation type="unfinished">Warnung</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2045"/>
        <source>Cannot find typeoption </source>
        <translation type="unfinished">Typoption nicht gefunden</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2054"/>
        <source>Cannot find values for typeoption </source>
        <translation type="unfinished">Keine Werte für Typoption gefunden: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2127"/>
        <source>Cannot find layeroption </source>
        <translation type="unfinished">Layeroption nicht gefunden: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2144"/>
        <source>GRASS element </source>
        <translation type="unfinished">GRASS-Element </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2144"/>
        <source> not supported</source>
        <translation type="unfinished"> nicht unterstützt</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2168"/>
        <source>Use region of this map</source>
        <translation>Karten-&apos;region&apos; benutzen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2514"/>
        <source>:&amp;nbsp;no input</source>
        <translation type="unfinished">:&amp;nbsp;Keine Eingabe</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleOption</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1953"/>
        <source>:&amp;nbsp;missing value</source>
        <translation type="unfinished">:&amp;nbsp;fehlender Wert</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleSelection</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2904"/>
        <source>Attribute field</source>
        <translation type="unfinished">Attributfeld</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleStandardOptions</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="874"/>
        <source>Warning</source>
        <translation type="unfinished">Warnung</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="329"/>
        <source>Cannot find module </source>
        <translation type="unfinished">Kann Modul nicht finden: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="346"/>
        <source>Cannot start module </source>
        <translation type="unfinished">Kann Modul nicht starten: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="360"/>
        <source>Cannot read module description (</source>
        <translation type="unfinished">Kann Modulbeschreibung nicht lesen: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="360"/>
        <source>):
</source>
        <translation type="unfinished">):
</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="361"/>
        <source>
at line </source>
        <translation type="unfinished">
bei Zeile </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="361"/>
        <source> column </source>
        <translation type="unfinished"> Spalte </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="401"/>
        <source>Cannot find key </source>
        <translation type="unfinished">Kann Schlüssel nicht finden: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="541"/>
        <source>Item with id </source>
        <translation type="unfinished">Element mit ID </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="541"/>
        <source> not found</source>
        <translation type="unfinished"> nicht gefunden</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="834"/>
        <source>Cannot get current region</source>
        <translation>Kann die aktuelle &apos;region&apos; nicht ermitteln.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="811"/>
        <source>Cannot check region of map </source>
        <translation>Kann die &apos;region&apos; der Karte nicht überprüfen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="875"/>
        <source>Cannot set region of map </source>
        <translation>Kann Karten-&apos;region&apos; nicht setzen: </translation>
    </message>
</context>
<context>
    <name>QgsGrassNewMapset</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="137"/>
        <source>Database</source>
        <translation type="unfinished">Datenbank</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="149"/>
        <source>Location 2</source>
        <translation>2. Location</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="153"/>
        <source>User&apos;s mapset</source>
        <translation>Benutzer Mapset</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="151"/>
        <source>System mapset</source>
        <translation>System Mapset</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="140"/>
        <source>Location 1</source>
        <translation>1. Location</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="202"/>
        <source>Enter path to GRASS database</source>
        <translation type="unfinished">Pfad zur GRASS-Datenbank angeben</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="210"/>
        <source>The directory doesn&apos;t exist!</source>
        <translation type="unfinished">Das Verzeichnis existiert nicht!</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="240"/>
        <source>No writable locations, the database not writable!</source>
        <translation type="unfinished">Keine </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="356"/>
        <source>Enter location name!</source>
        <translation>Location-Name angeben</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="363"/>
        <source>The location exists!</source>
        <translation>Die Location existiert!</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="494"/>
        <source>Selected projection is not supported by GRASS!</source>
        <translation>Ausgewählte Projektion wird nicht von GRASS unterstützt!</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1086"/>
        <source>Warning</source>
        <translation type="unfinished">Warnung</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="530"/>
        <source>Cannot create projection.</source>
        <translation type="unfinished">Kann Projektion nicht erzeugen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="579"/>
        <source>Cannot reproject previously set region, default region set.</source>
        <translation>Kann Region nicht reprojizieren. Voreingestellte Region gesetzt.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="706"/>
        <source>North must be greater than south</source>
        <translation type="unfinished">Nord muß größer als Süd sein</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="711"/>
        <source>East must be greater than west</source>
        <translation type="unfinished">Ost muß größer als West sein</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="755"/>
        <source>Regions file (</source>
        <translation>Region-Datei (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="755"/>
        <source>) not found.</source>
        <translation type="unfinished">) nicht gefunden.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="761"/>
        <source>Cannot open locations file (</source>
        <translation>Kann Location-Datei  nicht öffnen (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="761"/>
        <source>)</source>
        <translation>)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="771"/>
        <source>Cannot read locations file (</source>
        <translation>Kann Location-Datei nicht lesen (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="772"/>
        <source>):
</source>
        <translation type="unfinished">):
</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="772"/>
        <source>
at line </source>
        <translation type="unfinished">
bei Zeile </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="773"/>
        <source> column </source>
        <translation type="unfinished"> Spalte </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="904"/>
        <source>Cannot reproject selected region.</source>
        <translation>Kann ausgewählte &apos;region&apos; nicht reprojizieren.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="989"/>
        <source>Cannot reproject region</source>
        <translation>Kann &apos;region&apos; nicht reprojizieren.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1207"/>
        <source>Enter mapset name.</source>
        <translation>Mapset angeben</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1217"/>
        <source>The mapset already exists</source>
        <translation>Die Mapset existiert bereits</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1235"/>
        <source>Database: </source>
        <translation type="unfinished">Datenbank: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1246"/>
        <source>Location: </source>
        <translation>Location: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1248"/>
        <source>Mapset: </source>
        <translation>Mapset: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1279"/>
        <source>Create location</source>
        <translation>Location anlegen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1281"/>
        <source>Cannot create new location: </source>
        <translation>Kann neue Location nicht anlegen: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1328"/>
        <source>Create mapset</source>
        <translation>Mapset anlegen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1311"/>
        <source>Cannot create new mapset directory</source>
        <translation>Kann Mapset-Verzeichnis nicht anlegen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1321"/>
        <source>Cannot open DEFAULT_WIND</source>
        <translation type="unfinished">Kann DEFAULT_WIND nicht öffnen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1328"/>
        <source>Cannot open WIND</source>
        <translation type="unfinished">Kann WIND nicht öffnen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1356"/>
        <source>New mapset</source>
        <translation>Neue Location/Mapset</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1352"/>
        <source>New mapset successfully created, but cannot be opened: </source>
        <translation>Mapset erfolgreich angelegt, konnte aber nicht geöffnet werden: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1358"/>
        <source>New mapset successfully created and set as current working mapset.</source>
        <translation>Mapset erfolgreich erzeugt und als aktuelle Arbeitsumgebung eingestellt.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1087"/>
        <source>Cannot create QgsCoordinateReferenceSystem</source>
        <translation type="unfinished">Kann QgsCoordinateReferenceSystem nicht erzeugen</translation>
    </message>
</context>
<context>
    <name>QgsGrassNewMapsetBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="52"/>
        <source>Example directory tree:</source>
        <translation>Beispielordnerstruktur:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="561"/>
        <source>Database Error</source>
        <translation>Datenbank Fehler</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2668"/>
        <source>Database:</source>
        <translation>Datenbank:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="607"/>
        <source>Select existing directory or create a new one:</source>
        <translation>Existierenden Ordner wählen oder neuen erzeugen:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="639"/>
        <source>Location</source>
        <translation>Location</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="654"/>
        <source>Select location</source>
        <translation>Wähle Location</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="671"/>
        <source>Create new location</source>
        <translation>Erstelle neue Location</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1147"/>
        <source>Location Error</source>
        <translation>Location Fehler</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1669"/>
        <source>Projection Error</source>
        <translation>Projektionsfehler</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1676"/>
        <source>Coordinate system</source>
        <translation>Koordinatensystem</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1688"/>
        <source>Projection</source>
        <translation>Projektion</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1695"/>
        <source>Not defined</source>
        <translation>Nicht definiert</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1798"/>
        <source>Set current QGIS extent</source>
        <translation>Setze aktuelle QGIS Ausdehnung</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1827"/>
        <source>Set</source>
        <translation>Setzen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1839"/>
        <source>Region Error</source>
        <translation>Region-Fehler</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1870"/>
        <source>S</source>
        <translation>S</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1906"/>
        <source>W</source>
        <translation>W</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1945"/>
        <source>E</source>
        <translation>E</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1981"/>
        <source>N</source>
        <translation>N</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2041"/>
        <source>New mapset:</source>
        <translation>Neues Mapset:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2526"/>
        <source>Mapset Error</source>
        <translation>Mapset Fehler</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2567"/>
        <source>&lt;p align=&quot;center&quot;&gt;Existing masets&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;center&quot;&gt; Vorhandene Mapsets&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2687"/>
        <source>Location:</source>
        <translation>Location:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2706"/>
        <source>Mapset:</source>
        <translation>Mapset:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="13"/>
        <source>New Mapset</source>
        <translation type="unfinished">Neues Mapset</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="17"/>
        <source>GRASS Database</source>
        <translation type="unfinished">GRASS-Datenbank</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="39"/>
        <source>Tree</source>
        <translation type="unfinished">Baum</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="44"/>
        <source>Comment</source>
        <translation type="unfinished">Kommentar</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="59"/>
        <source>&lt;!DOCTYPE HTML PUBLIC &quot;-//W3C//DTD HTML 4.0//EN&quot; &quot;http://www.w3.org/TR/REC-html40/strict.dtd&quot;&gt;
&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Lucida Grande&apos;; font-size:13pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;GRASS data are stored in tree directory structure. The GRASS database is the top-level directory in this tree structure.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished">&lt;!DOCTYPE HTML PUBLIC &quot;-//W3C//DTD HTML 4.0//EN&quot; &quot;http://www.w3.org/TR/REC-html40/strict.dtd&quot;&gt;
&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Lucida Grande&apos;; font-size:13pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;GRASS-Daten werden in einer baumförmigen Verzeichnisstuktur gespeichert. Die GRASS-Datenbank ist die Verzeichnis auf oberste Ebene der Baumstruktur.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="596"/>
        <source>Browse...</source>
        <translation type="unfinished">Suchen...</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="633"/>
        <source>GRASS Location</source>
        <translation type="unfinished">GRASS-Location</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1163"/>
        <source>&lt;!DOCTYPE HTML PUBLIC &quot;-//W3C//DTD HTML 4.0//EN&quot; &quot;http://www.w3.org/TR/REC-html40/strict.dtd&quot;&gt;
&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Lucida Grande&apos;; font-size:13pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;The GRASS location is a collection of maps for a particular territory or project.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished">&lt;!DOCTYPE HTML PUBLIC &quot;-//W3C//DTD HTML 4.0//EN&quot; &quot;http://www.w3.org/TR/REC-html40/strict.dtd&quot;&gt;
&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Lucida Grande&apos;; font-size:13pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;Die GRASS-Location ist eine Zusammenstellung von Karten für ein bestimmtes Gebiet oder Projekt.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1716"/>
        <source>Default GRASS Region</source>
        <translation type="unfinished">Voreingestellte GRASS-&apos;region&apos;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1747"/>
        <source>&lt;!DOCTYPE HTML PUBLIC &quot;-//W3C//DTD HTML 4.0//EN&quot; &quot;http://www.w3.org/TR/REC-html40/strict.dtd&quot;&gt;
&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Lucida Grande&apos;; font-size:13pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;The GRASS region defines a workspace for raster modules. The default region is valid for one location. It is possible to set a different region in each mapset. It is possible to change the default location region later.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished">&lt;!DOCTYPE HTML PUBLIC &quot;-//W3C//DTD HTML 4.0//EN&quot; &quot;http://www.w3.org/TR/REC-html40/strict.dtd&quot;&gt;
&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Lucida Grande&apos;; font-size:13pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;Die GRASS-Region definiert einen Arbeitsbereich für Rastermodule. Die voreingestellte Region gilt für eine Location. In jedem Mapset kann eine andere Region gesetzt werden. Die voreingestellte Region der Location kann auch später geändert werden.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2587"/>
        <source>Mapset</source>
        <translation type="unfinished">Kartenset</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2592"/>
        <source>Owner</source>
        <translation type="unfinished">Besitzer</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2625"/>
        <source>&lt;!DOCTYPE HTML PUBLIC &quot;-//W3C//DTD HTML 4.0//EN&quot; &quot;http://www.w3.org/TR/REC-html40/strict.dtd&quot;&gt;
&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Lucida Grande&apos;; font-size:13pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;The GRASS mapset is a collection of maps used by one user. A user can read maps from all mapsets in the location but he can open for writing only his mapset (owned by user).&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished">&lt;!DOCTYPE HTML PUBLIC &quot;-//W3C//DTD HTML 4.0//EN&quot; &quot;http://www.w3.org/TR/REC-html40/strict.dtd&quot;&gt;
&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Lucida Grande&apos;; font-size:13pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;Ein GRASS-Mapset ist ein Zusammenstellung von Karten, die von einem Benutzer genutzt werden. Ein Benutzer kann Karten aus allen Mapsets der Location lesen, aber nur die seines eigenen zum Schreiben öffnen (deren Besitzer er ist).&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2650"/>
        <source>Create New Mapset</source>
        <translation type="unfinished">Neues Mapset erzeugen</translation>
    </message>
</context>
<context>
    <name>QgsGrassPlugin</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="180"/>
        <source>GRASS</source>
        <translation>GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="771"/>
        <source>&amp;GRASS</source>
        <translation>&amp;GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="127"/>
        <source>Open mapset</source>
        <translation>Mapset öffnen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="128"/>
        <source>New mapset</source>
        <translation>Neues Mapset</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="129"/>
        <source>Close mapset</source>
        <translation>Schliesse Mapset</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="132"/>
        <source>Add GRASS vector layer</source>
        <translation>GRASS-Vektorlayer hinzufügen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="134"/>
        <source>Add GRASS raster layer</source>
        <translation>GRASS-Rasterlayer hinzufügen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="150"/>
        <source>Open GRASS tools</source>
        <translation>GRASS-Werkzeugkiste öffnen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="139"/>
        <source>Display Current Grass Region</source>
        <translation>Aktuelle GRASS-Region darstellen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="143"/>
        <source>Edit Current Grass Region</source>
        <translation>Aktuelle GRASS-Region bearbeiten</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="145"/>
        <source>Edit Grass Vector layer</source>
        <translation>GRASS-Vektorlayer bearbeiten</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="148"/>
        <source>Adds a GRASS vector layer to the map canvas</source>
        <translation>Fügt dem Kartenfenster einen GRASS-Vektorlayer hinzu</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="149"/>
        <source>Adds a GRASS raster layer to the map canvas</source>
        <translation>Fügt dem Kartenfenster einen GRASS-Rasterlayer hinzu</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="151"/>
        <source>Displays the current GRASS region as a rectangle on the map canvas</source>
        <translation>Zeigt die aktuelle GRASS-Region als Rechteck im Kartenbild an</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="152"/>
        <source>Edit the current GRASS region</source>
        <translation>Aktuelle GRASS-Region bearbeiten.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="153"/>
        <source>Edit the currently selected GRASS vector layer.</source>
        <translation>Gewählten GRASS-Vektorlayer bearbeiten.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="64"/>
        <source>GrassVector</source>
        <translation>GrassVektor</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="65"/>
        <source>0.1</source>
        <translation>0.1</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="66"/>
        <source>GRASS layer</source>
        <translation>GRASS-Layer</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="146"/>
        <source>Create new Grass Vector</source>
        <translation>Neuen GRASS-Vektorlayer anlegen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="743"/>
        <source>Warning</source>
        <translation type="unfinished">Warnung</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="455"/>
        <source>GRASS Edit is already running.</source>
        <translation type="unfinished">GRASS-Digitalisierung läuft bereits.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="464"/>
        <source>New vector name</source>
        <translation type="unfinished">Neuer Vektorname</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="480"/>
        <source>Cannot create new vector: </source>
        <translation type="unfinished">Kann Vektor nicht anlegen: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="505"/>
        <source>New vector created but cannot be opened by data provider.</source>
        <translation type="unfinished">Neuer Vektor konnte nicht durch Datenlieferant geöffnet werden.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="519"/>
        <source>Cannot start editing.</source>
        <translation type="unfinished">Konnte Digialisierung nicht beginnen.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="551"/>
        <source>GISDBASE, LOCATION_NAME or MAPSET is not set, cannot display current region.</source>
        <translation type="unfinished">GISDBASE, LOCATION_NAME oder MAPSET ist nicht gesetzt, aktuelle Region kann nicht angezeigt werden</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="562"/>
        <source>Cannot read current region: </source>
        <translation type="unfinished">Kann aktuelle Region nicht lesen: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="663"/>
        <source>Cannot open the mapset. </source>
        <translation>Kann Mapset nicht öffnen: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="679"/>
        <source>Cannot close mapset. </source>
        <translation>Kann Mapset nicht schließen. </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="734"/>
        <source>Cannot close current mapset. </source>
        <translation>Kann aktuellen Mapset nicht schließen. </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="743"/>
        <source>Cannot open GRASS mapset. </source>
        <translation>Kann GRASS-Mapset nicht öffnen.</translation>
    </message>
</context>
<context>
    <name>QgsGrassRegion</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregion.cpp" line="433"/>
        <source>Warning</source>
        <translation type="unfinished">Warnung</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregion.cpp" line="164"/>
        <source>GISDBASE, LOCATION_NAME or MAPSET is not set, cannot display current region.</source>
        <translation type="unfinished">GISDBASE, LOCATION_NAME oder MAPSET ist nicht gesetzt, kann aktuelle Region nicht anzeigen.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregion.cpp" line="172"/>
        <source>Cannot read current region: </source>
        <translation type="unfinished">Kann aktuelle Region nicht lesen: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregion.cpp" line="433"/>
        <source>Cannot write region</source>
        <translation type="unfinished">Kann Region nicht schreiben</translation>
    </message>
</context>
<context>
    <name>QgsGrassRegionBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="13"/>
        <source>GRASS Region Settings</source>
        <translation>GRASS-Regioneinstellungen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="92"/>
        <source>N</source>
        <translation>N</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="169"/>
        <source>W</source>
        <translation>W</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="195"/>
        <source>E</source>
        <translation>E</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="266"/>
        <source>S</source>
        <translation>SS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="316"/>
        <source>N-S Res</source>
        <translation>N-S Aufl.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="329"/>
        <source>Rows</source>
        <translation>Zeilen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="339"/>
        <source>Cols</source>
        <translation>Spalten</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="352"/>
        <source>E-W Res</source>
        <translation>E-W Aufl.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="407"/>
        <source>Color</source>
        <translation>Farbe</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="427"/>
        <source>Width</source>
        <translation>Breite</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="514"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="537"/>
        <source>Cancel</source>
        <translation>Abbrechen</translation>
    </message>
</context>
<context>
    <name>QgsGrassSelect</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="64"/>
        <source>Select GRASS Vector Layer</source>
        <translation>Wählen Sie einen GRASS-Vektorlayer</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="71"/>
        <source>Select GRASS Raster Layer</source>
        <translation>Wählen Sie einen GRASS-Rasterlayer</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="78"/>
        <source>Select GRASS mapcalc schema</source>
        <translation>Wähen Sie ein GRASS Mapcalc-Schema</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="86"/>
        <source>Select GRASS Mapset</source>
        <translation>Wählen Sie ein GRASS Mapset</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="428"/>
        <source>Warning</source>
        <translation>Warnung</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="428"/>
        <source>Cannot open vector on level 2 (topology not available).</source>
        <translation>Kann den Vektordatensatz nicht in Level 2 öffnen (Topologie fehlt).</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="485"/>
        <source>Choose existing GISDBASE</source>
        <translation>Bitte wählen Sie eine existierende GISDBASE.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="502"/>
        <source>Wrong GISDBASE, no locations available.</source>
        <translation>Falsche GISDBASE, darin sind keine Locations vorhanden.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="503"/>
        <source>Wrong GISDBASE</source>
        <translation>Falsche GISDBASE.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="521"/>
        <source>Select a map.</source>
        <translation>Wählen Sie eine Karte.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="522"/>
        <source>No map</source>
        <translation>Keine Karte</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="530"/>
        <source>No layer</source>
        <translation>Kein Layer</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="531"/>
        <source>No layers available in this map</source>
        <translation>Keine Layer in dieser Karte vorhanden.</translation>
    </message>
</context>
<context>
    <name>QgsGrassSelectBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="82"/>
        <source>Gisdbase</source>
        <translation>Gisdbase</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="95"/>
        <source>Location</source>
        <translation>Location</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="185"/>
        <source>Browse</source>
        <translation>Durchsuchen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="102"/>
        <source>Mapset</source>
        <translation>Kartenset</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="135"/>
        <source>Map name</source>
        <translation>Kartenname</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="142"/>
        <source>Layer</source>
        <translation>Layer</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="199"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="119"/>
        <source>Select or type map name (wildcards &apos;*&apos; and &apos;?&apos; accepted for rasters)</source>
        <translation>Kartenname (Wildcards &apos;*&apos; und &apos;?&apos; werden für Raster akzeptiert) wählen oder eingeben</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="19"/>
        <source>Add GRASS Layer</source>
        <translation>GRASS-Layer hinzufügen</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="192"/>
        <source>Cancel</source>
        <translation>Abbrechen</translation>
    </message>
</context>
<context>
    <name>QgsGrassShellBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassshellbase.ui" line="19"/>
        <source>GRASS Shell</source>
        <translation>GRASS Kommandozeile</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassshellbase.ui" line="49"/>
        <source>Close</source>
        <translation>Schließen</translation>
    </message>
</context>
<context>
    <name>QgsGrassTools</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="125"/>
        <source>Browser</source>
        <translation>Browser</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="66"/>
        <source>GRASS Tools</source>
        <translation>GRASS-Werkzeuge </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="383"/>
        <source>GRASS Tools: </source>
        <translation>GRASS-Werkzeuge: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="272"/>
        <source>Warning</source>
        <translation type="unfinished">Warnung</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="185"/>
        <source>Cannot find MSYS (</source>
        <translation type="unfinished">Kann MSYS nicht finden (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="207"/>
        <source>GRASS Shell is not compiled.</source>
        <translation type="unfinished">GRASS-Shell ist nicht kompiliert.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="255"/>
        <source>The config file (</source>
        <translation type="unfinished">Die Konfigurationdatei (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="255"/>
        <source>) not found.</source>
        <translation type="unfinished">) nicht gefunden.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="260"/>
        <source>Cannot open config file (</source>
        <translation type="unfinished">Kann Konfiguration nicht öffnen (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="260"/>
        <source>)</source>
        <translation type="unfinished">)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="269"/>
        <source>Cannot read config file (</source>
        <translation type="unfinished">Kann Konfiguration nicht lesen (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="270"/>
        <source>
at line </source>
        <translation type="unfinished">
bei Zeile </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="270"/>
        <source> column </source>
        <translation type="unfinished"> Spalte </translation>
    </message>
</context>
<context>
    <name>QgsGrassToolsBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstoolsbase.ui" line="13"/>
        <source>Grass Tools</source>
        <translation type="unfinished">GRASS-Werkzeuge</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstoolsbase.ui" line="23"/>
        <source>Modules Tree</source>
        <translation type="unfinished">Modulbaum</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstoolsbase.ui" line="42"/>
        <source>1</source>
        <translation type="unfinished">1</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstoolsbase.ui" line="51"/>
        <source>Modules List</source>
        <translation type="unfinished">Modulliste</translation>
    </message>
</context>
<context>
    <name>QgsGridMakerPlugin</name>
    <message>
        <location filename="../src/plugins/grid_maker/plugin.cpp" line="91"/>
        <source>&amp;Graticule Creator</source>
        <translation>Koordinatenlinien-Generator</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugin.cpp" line="92"/>
        <source>Creates a graticule (grid) and stores the result as a shapefile</source>
        <translation>Erzeugt ein Gradnetz (Grid) und speichert es in ein Shapefile.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugin.cpp" line="134"/>
        <source>&amp;Graticules</source>
        <translation>&amp;Geographisches Netz</translation>
    </message>
</context>
<context>
    <name>QgsGridMakerPluginGui</name>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="62"/>
        <source>QGIS - Grid Maker</source>
        <translation>QGIS - Gitternetzbuilder</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="52"/>
        <source>Please enter the file name before pressing OK!</source>
        <translation>Bitte geben Sie einen Dateinamen ein, bevor Sie auf OK drücken!</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="118"/>
        <source>ESRI Shapefile (*.shp)</source>
        <translation>ESRI Shapedatei (*.shp)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="63"/>
        <source>Please enter intervals before pressing OK!</source>
        <translation type="unfinished">Bitte Intervalle vor dem OK eingeben!</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="116"/>
        <source>Choose a file name to save under</source>
        <translation type="unfinished">Dateiname zum Speichern wählen</translation>
    </message>
</context>
<context>
    <name>QgsGridMakerPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="322"/>
        <source>Graticule Builder</source>
        <translation>Gitternetzbuilder</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="243"/>
        <source>Type</source>
        <translation>Typ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="264"/>
        <source>Point</source>
        <translation>Punkt</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="274"/>
        <source>Polygon</source>
        <translation>Polygon</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="146"/>
        <source>Origin (lower left)</source>
        <translation>Ursprung (links unten)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="93"/>
        <source>End point (upper right)</source>
        <translation>Endpunkt (rechts oben)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="209"/>
        <source>Output (shape) file</source>
        <translation>Ergebnisdatei (Shape)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="233"/>
        <source>Save As...</source>
        <translation type="unfinished">Speichern unter...</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="13"/>
        <source>QGIS Graticule Creator</source>
        <translation type="unfinished">QGIS-Gittererzeugung</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="40"/>
        <source>Graticle size</source>
        <translation type="unfinished">Gittergröße</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="70"/>
        <source>Y Interval:</source>
        <translation type="unfinished">Y-Interval:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="80"/>
        <source>X Interval:</source>
        <translation type="unfinished">X-Interval:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="176"/>
        <source>Y</source>
        <translation>Y</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="186"/>
        <source>X</source>
        <translation>X</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="287"/>
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
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="187"/>
        <source>Quantum GIS Help - </source>
        <translation>Quantum GIS Hilfe -</translation>
    </message>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="193"/>
        <source>Failed to get the help text from the database</source>
        <translation>Der Hilfetext konnte nicht aus der Datenbank geholt werden</translation>
    </message>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="217"/>
        <source>Error</source>
        <translation>Fehler</translation>
    </message>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="218"/>
        <source>The QGIS help database is not installed</source>
        <translation>Die QGIS Hilfedatenbank ist nicht installiert</translation>
    </message>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="139"/>
        <source>This help file does not exist for your language</source>
        <translation>Diese Hilfedatei existiert noch nicht für Ihre Sprache.</translation>
    </message>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="142"/>
        <source>If you would like to create it, contact the QGIS development team</source>
        <translation>Wenn Sie es erstellen wollen, kontaktieren Sie bitte das QGIS Entwicklungsteam.</translation>
    </message>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="158"/>
        <source>Quantum GIS Help</source>
        <translation>Quantum GIS Hilfe</translation>
    </message>
</context>
<context>
    <name>QgsHelpViewerBase</name>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="13"/>
        <source>QGIS Help</source>
        <translation>QGIS Hilfe</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="39"/>
        <source>&amp;Home</source>
        <translation>&amp;Home</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="42"/>
        <source>Alt+H</source>
        <translation>Alt+H</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="52"/>
        <source>&amp;Forward</source>
        <translation>&amp;Vorwärts</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="55"/>
        <source>Alt+F</source>
        <translation>Alt+F</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="65"/>
        <source>&amp;Back</source>
        <translation>&amp;Rückwärts</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="68"/>
        <source>Alt+B</source>
        <translation>Alt+B</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="78"/>
        <source>&amp;Close</source>
        <translation>&amp;Schließen</translation>
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
        <location filename="../src/core/qgshttptransaction.cpp" line="255"/>
        <source>WMS Server responded unexpectedly with HTTP Status Code %1 (%2)</source>
        <translation>WMS Server hat unerwarteterweise folgenden HTTP Status Code herausgegeben: %1 (%2)</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="334"/>
        <source>HTTP response completed, however there was an error: %1</source>
        <translation>HTTP Antwort beendet, es habe jedoch Fehler: %1</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="384"/>
        <source>HTTP transaction completed, however there was an error: %1</source>
        <translation>HTTP Übertragung beendet, aber es trat ein Fehler auf: %1</translation>
    </message>
    <message numerus="yes">
        <location filename="../src/core/qgshttptransaction.cpp" line="463"/>
        <source>Network timed out after %1 seconds of inactivity.
This may be a problem in your network connection or at the WMS server.</source>
        <translation type="unfinished">
            <numerusform>
        
        
        </numerusform>
        </translation>
    </message>
</context>
<context>
    <name>QgsIDWInterpolatorDialogBase</name>
    <message>
        <location filename="../src/plugins/interpolation/qgsidwinterpolatordialogbase.ui" line="13"/>
        <source>Dialog</source>
        <translation type="unfinished">Dialog</translation>
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
        <translation type="unfinished">Abstandskoeffizient P:</translation>
    </message>
</context>
<context>
    <name>QgsIdentifyResults</name>
    <message>
        <location filename="../src/app/qgsidentifyresults.cpp" line="214"/>
        <source>Identify Results - </source>
        <translation>Anfrageergebnisse - </translation>
    </message>
    <message>
        <location filename="../src/app/qgsidentifyresults.cpp" line="110"/>
        <source>Run action</source>
        <translation>Aktion starten</translation>
    </message>
    <message>
        <location filename="../src/app/qgsidentifyresults.cpp" line="59"/>
        <source>(Derived)</source>
        <translation>(abgeleitet)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsidentifyresults.cpp" line="45"/>
        <source>Feature</source>
        <translation>Objekt</translation>
    </message>
    <message>
        <location filename="../src/app/qgsidentifyresults.cpp" line="46"/>
        <source>Value</source>
        <translation>Wert</translation>
    </message>
</context>
<context>
    <name>QgsIdentifyResultsBase</name>
    <message>
        <location filename="../src/ui/qgsidentifyresultsbase.ui" line="13"/>
        <source>Identify Results</source>
        <translation>Identifikationsergebnis</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsidentifyresultsbase.ui" line="43"/>
        <source>Help</source>
        <translation>Hilfe</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsidentifyresultsbase.ui" line="46"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsidentifyresultsbase.ui" line="72"/>
        <source>Close</source>
        <translation>Schließen</translation>
    </message>
</context>
<context>
    <name>QgsInterpolationDialog</name>
    <message>
        <location filename="../src/plugins/interpolation/qgsinterpolationdialog.cpp" line="210"/>
        <source>Triangular interpolation (TIN)</source>
        <translation type="unfinished">Unregelmäßiges Dreiecksnetz (TIN)</translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsinterpolationdialog.cpp" line="206"/>
        <source>Inverse Distance Weighting (IDW)</source>
        <translation type="unfinished">Inverse Distanzwichtung (IDW)</translation>
    </message>
</context>
<context>
    <name>QgsInterpolationDialogBase</name>
    <message>
        <location filename="../src/plugins/interpolation/qgsinterpolationdialogbase.ui" line="19"/>
        <source>Interpolation plugin</source>
        <translation type="unfinished">Interpolationsplugin</translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsinterpolationdialogbase.ui" line="31"/>
        <source>Input</source>
        <translation type="unfinished">Eingabe</translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsinterpolationdialogbase.ui" line="37"/>
        <source>Input vector layer</source>
        <translation type="unfinished">Eingabevektorlayer</translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsinterpolationdialogbase.ui" line="64"/>
        <source>Use z-Coordinate for interpolation</source>
        <translation type="unfinished">Z-Koordinate für Interpolation verwenden</translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsinterpolationdialogbase.ui" line="71"/>
        <source>Interpolation attribute </source>
        <translation type="unfinished">Interpolationsattribut</translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsinterpolationdialogbase.ui" line="100"/>
        <source>Output</source>
        <translation type="unfinished">Ergebnis</translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsinterpolationdialogbase.ui" line="106"/>
        <source>Interpolation method</source>
        <translation type="unfinished">Interpolationsmethode</translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsinterpolationdialogbase.ui" line="176"/>
        <source>...</source>
        <translation type="unfinished">...</translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsinterpolationdialogbase.ui" line="129"/>
        <source>Number of columns</source>
        <translation type="unfinished">Spaltenanzahl</translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsinterpolationdialogbase.ui" line="146"/>
        <source>Number of rows</source>
        <translation type="unfinished">Zeilenanzahl</translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsinterpolationdialogbase.ui" line="163"/>
        <source>Output file </source>
        <translation type="unfinished">Ausgabedatei </translation>
    </message>
</context>
<context>
    <name>QgsInterpolationPlugin</name>
    <message>
        <location filename="../src/plugins/interpolation/qgsinterpolationplugin.cpp" line="49"/>
        <source>&amp;Interpolation</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsLUDialogBase</name>
    <message>
        <location filename="../src/ui/qgsludialogbase.ui" line="13"/>
        <source>Enter class bounds</source>
        <translation>Gib die Klassengrenzen ein</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsludialogbase.ui" line="40"/>
        <source>Lower value</source>
        <translation>Untere Grenze</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsludialogbase.ui" line="79"/>
        <source>-</source>
        <translation>-</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsludialogbase.ui" line="66"/>
        <source>Upper value</source>
        <translation>Obere Grenze</translation>
    </message>
</context>
<context>
    <name>QgsLabelDialog</name>
    <message>
        <location filename="../src/app/qgslabeldialog.cpp" line="206"/>
        <source>Auto</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsLabelDialogBase</name>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="19"/>
        <source>Form1</source>
        <translation>Formular1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="999"/>
        <source>Preview:</source>
        <translation>Vorschau:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="1008"/>
        <source>QGIS Rocks!</source>
        <translation>QGIS bringt&apos;s!</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="152"/>
        <source>Font</source>
        <translation>Schrift</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="582"/>
        <source>Points</source>
        <translation>Punkte</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="589"/>
        <source>Map units</source>
        <translation>Karteneinheiten</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="480"/>
        <source>%</source>
        <translation>%</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="877"/>
        <source>Transparency:</source>
        <translation>Transparenz:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="95"/>
        <source>Position</source>
        <translation>Position</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="900"/>
        <source>Size:</source>
        <translation>Grösse:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="513"/>
        <source>Size is in map units</source>
        <translation>Grösse in Karteneinheiten</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="506"/>
        <source>Size is in points</source>
        <translation>Grösse in Punkten</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="275"/>
        <source>Above</source>
        <translation>Oben</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="265"/>
        <source>Over</source>
        <translation>Über</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="282"/>
        <source>Left</source>
        <translation>Links</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="258"/>
        <source>Below</source>
        <translation>Unten</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="251"/>
        <source>Right</source>
        <translation>Rechts</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="296"/>
        <source>Above Right</source>
        <translation>Oben rechts</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="244"/>
        <source>Below Right</source>
        <translation>Unten rechts</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="303"/>
        <source>Above Left</source>
        <translation>Oben links</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="289"/>
        <source>Below Left</source>
        <translation>Unten links</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="313"/>
        <source>Font size units</source>
        <translation>Schriftgröße</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="824"/>
        <source>Placement</source>
        <translation>Platzierung</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="90"/>
        <source>Buffer</source>
        <translation>Puffer</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="500"/>
        <source>Buffer size units</source>
        <translation>Puffergrößeneinheiten</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="576"/>
        <source>Offset units</source>
        <translation>Offset Einheiten</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="30"/>
        <source>Field containing label</source>
        <translation type="unfinished">Beschreibungsfeld</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="53"/>
        <source>Default label</source>
        <translation type="unfinished">Beschriftungsvorgabe</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="100"/>
        <source>Data defined style</source>
        <translation type="unfinished">Datendefinierter Stil</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="105"/>
        <source>Data defined alignment</source>
        <translation type="unfinished">Datendefinierte Ausrichtung</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="110"/>
        <source>Data defined buffer</source>
        <translation type="unfinished">Datendefinierter Puffer</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="115"/>
        <source>Data defined position</source>
        <translation type="unfinished">Datendefinierte Position</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="162"/>
        <source>Font transparency</source>
        <translation type="unfinished">Schrifttransparenz</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="493"/>
        <source>Color</source>
        <translation type="unfinished">Farbe</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="840"/>
        <source>Angle (deg)</source>
        <translation type="unfinished">Winkel (Altgrad)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="432"/>
        <source>Buffer labels?</source>
        <translation type="unfinished">Beschriftungen freistellen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="442"/>
        <source>Buffer size</source>
        <translation type="unfinished">Puffergröße</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="786"/>
        <source>Transparency</source>
        <translation type="unfinished">Transparenz</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="963"/>
        <source>X Offset (pts)</source>
        <translation type="unfinished">X-Offset (Punkte)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="979"/>
        <source>Y Offset (pts)</source>
        <translation type="unfinished">Y-Offset (Punkte)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="630"/>
        <source>&amp;Font family</source>
        <translation type="unfinished">&amp;Schriftfamilie</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="656"/>
        <source>&amp;Bold</source>
        <translation type="unfinished">&amp;Fett</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="682"/>
        <source>&amp;Italic</source>
        <translation type="unfinished">&amp;Kursiv</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="708"/>
        <source>&amp;Underline</source>
        <translation type="unfinished">&amp;Unterstrichen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="734"/>
        <source>&amp;Size</source>
        <translation type="unfinished">Größe</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="760"/>
        <source>Size units</source>
        <translation type="unfinished">Größeneinheit</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="931"/>
        <source>X Coordinate</source>
        <translation type="unfinished">X-Koordinate</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="947"/>
        <source>Y Coordinate</source>
        <translation type="unfinished">Y-Koordinate</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="410"/>
        <source>Multiline labels?</source>
        <translation type="unfinished">Mehrzeilige Beschriftungen?</translation>
    </message>
    <message encoding="UTF-8">
        <location filename="../src/ui/qgslabeldialogbase.ui" line="220"/>
        <source>°</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="85"/>
        <source>General</source>
        <translation type="unfinished">Allgemein</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="349"/>
        <source>Use scale dependent rendering</source>
        <translation type="unfinished">Massstabsabhängig zeichnen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="361"/>
        <source>Maximum</source>
        <translation type="unfinished">Maximum</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="371"/>
        <source>Minimum</source>
        <translation type="unfinished">Minimum</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="381"/>
        <source>Minimum scale at which this layer will be displayed. </source>
        <translation>Minimaler Massstab ab dem dieser Layer angezeigt wird.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="394"/>
        <source>Maximum scale at which this layer will be displayed. </source>
        <translation>Maximaler Massstab bis zu dem dieser Layer angezeigt wird.</translation>
    </message>
</context>
<context>
    <name>QgsLegend</name>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="114"/>
        <source>group</source>
        <translation>Gruppe</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="452"/>
        <source>&amp;Remove</source>
        <translation>&amp;Entfernen</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="445"/>
        <source>&amp;Make to toplevel item</source>
        <translation>Als Top-Level Objekt &amp;machen</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="457"/>
        <source>Re&amp;name</source>
        <translation>Umbe&amp;nennen</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="462"/>
        <source>&amp;Add group</source>
        <translation>Gruppe hinzufügen</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="463"/>
        <source>&amp;Expand all</source>
        <translation>Alles Ausklapp&amp;en</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="464"/>
        <source>&amp;Collapse all</source>
        <translation>Alles zusammenfalten</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="466"/>
        <source>Show file groups</source>
        <translation>Zeige Dateigruppen</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="1868"/>
        <source>No Layer Selected</source>
        <translation>Keinen Layer ausgewählt</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="1869"/>
        <source>To open an attribute table, you must select a vector layer in the legend</source>
        <translation type="unfinished">Um eine Attributtabelle zu öffnen, müssen Sie einen Vektorlayer in der Legende auswählen</translation>
    </message>
</context>
<context>
    <name>QgsLegendLayer</name>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="481"/>
        <source>&amp;Zoom to layer extent</source>
        <translation>Auf die Layerausdehnung &amp;zoomen.</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="484"/>
        <source>&amp;Zoom to best scale (100%)</source>
        <translation type="unfinished">&amp;Auf besten Maßstab zoomen (100%)</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="488"/>
        <source>&amp;Show in overview</source>
        <translation type="unfinished">&amp;In der Übersicht anzeigen</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="494"/>
        <source>&amp;Remove</source>
        <translation type="unfinished">&amp;Entfernen</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="501"/>
        <source>&amp;Open attribute table</source>
        <translation type="unfinished">&amp;Attributtabelle öffnen</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="525"/>
        <source>Save as shapefile...</source>
        <translation type="unfinished">Als Shapefile abspeichern...</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="532"/>
        <source>Save selection as shapefile...</source>
        <translation type="unfinished">Auswahl als Shapefile speichern...</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="542"/>
        <source>&amp;Properties</source>
        <translation type="unfinished">&amp;Eigenschaften</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="591"/>
        <source>Multiple layers</source>
        <translation type="unfinished">Mehrere Layer</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="592"/>
        <source>This item contains multiple layers. Displaying multiple layers in the table is not supported.</source>
        <translation type="unfinished">Dieses Element enthält mehrere Layer. Die gemeinsame Darstellung mehrerer Layer in einer Attributtabelle wird nicht unterstützt.</translation>
    </message>
</context>
<context>
    <name>QgsLegendLayerFile</name>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="244"/>
        <source>Save layer as...</source>
        <translation>Layer speichern als...</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="335"/>
        <source>Error</source>
        <translation type="unfinished">Fehler</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="319"/>
        <source>Saving done</source>
        <translation type="unfinished">Speichern abgeschlossen</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="319"/>
        <source>Export to Shapefile has been completed</source>
        <translation type="unfinished">Der Export in eine Shapedatei ist abgeschlossen.</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="323"/>
        <source>Driver not found</source>
        <translation type="unfinished">Treiber nicht gefunden</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="323"/>
        <source>ESRI Shapefile driver is not available</source>
        <translation type="unfinished">Der ESRI-Shapefile-Treiber ist nicht verfügbar</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="327"/>
        <source>Error creating shapefile</source>
        <translation type="unfinished">Fehler beim Erzeugen der Shapedatei</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="328"/>
        <source>The shapefile could not be created (</source>
        <translation type="unfinished">Das Shapefile konnte nicht erstellt werden (</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="332"/>
        <source>Layer creation failed</source>
        <translation type="unfinished">Layererzeugung schlug fehl</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="362"/>
        <source>&amp;Zoom to layer extent</source>
        <translation>Auf die Layerausdehnung &amp;zoomen.</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="365"/>
        <source>&amp;Show in overview</source>
        <translation type="unfinished">&amp;In Übersicht anzeigen</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="373"/>
        <source>&amp;Remove</source>
        <translation type="unfinished">&amp;Entfernen</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="382"/>
        <source>&amp;Open attribute table</source>
        <translation type="unfinished">&amp;Attributtabelle öffnen</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="395"/>
        <source>Save as shapefile...</source>
        <translation type="unfinished">Als Shapedatei speichern...</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="397"/>
        <source>Save selection as shapefile...</source>
        <translation type="unfinished">Auswahl als Shapedatei speichern...</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="414"/>
        <source>&amp;Properties</source>
        <translation type="unfinished">&amp;Eigenschaften</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="336"/>
        <source>Layer attribute table contains unsupported datatype(s)</source>
        <translation>Die Attributtabelle des Layers enthält nicht unterstützte Datentypen.</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="281"/>
        <source>Select the coordinate reference system for the saved shapefile.</source>
        <translation type="unfinished">Koordinatenbezugssystem für die gespeicherte Shapedatei wählen</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="281"/>
        <source>The data points will be transformed from the layer coordinate reference system.</source>
        <translation type="unfinished">Die Punkte werden von dem Koordinatensystem des Layers transformiert.</translation>
    </message>
</context>
<context>
    <name>QgsMapCanvas</name>
    <message>
        <location filename="../src/gui/qgsmapcanvas.cpp" line="1223"/>
        <source>Could not draw</source>
        <translation>Konnte nicht zeichnen</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsmapcanvas.cpp" line="1223"/>
        <source>because</source>
        <translation>weil</translation>
    </message>
</context>
<context>
    <name>QgsMapLayer</name>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="517"/>
        <source>%1 at line %2 column %3</source>
        <translation type="unfinished">%1 in Zeile %2, Spalte %3</translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="620"/>
        <source>User database could not be opened.</source>
        <translation type="unfinished">Benutzerdatenbank konnte nicht geöffnet werden.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="635"/>
        <source>The style table could not be created.</source>
        <translation type="unfinished">Die Stiltabelle konnte nicht angelegt werden.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="650"/>
        <source>The style %1 was saved to database</source>
        <translation type="unfinished">Der Stil %1 wurde in der Datenbank gespeichert.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="667"/>
        <source>The style %1 was updated in the database.</source>
        <translation type="unfinished">Der Stil %1 wurde in der Datenbank aktualisiert.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="672"/>
        <source>The style %1 could not be updated in the database.</source>
        <translation type="unfinished">Der Stil %1 konnte nicht in der Datenbank aktualisiert werden.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="679"/>
        <source>The style %1 could not be inserted into database.</source>
        <translation type="unfinished">Der Stil %1 konnte nicht in der Datenbank gespeichert werden.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="522"/>
        <source>style not found in database</source>
        <translation type="unfinished">Stil nicht in der Datenbank gefunden</translation>
    </message>
</context>
<context>
    <name>QgsMapToolIdentify</name>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="162"/>
        <source>(clicked coordinate)</source>
        <translation>(Angeklickte Koordinate)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="224"/>
        <source>WMS identify result for %1
%2</source>
        <translation>WMS-Abfrageergebnis für %1
%2</translation>
    </message>
    <message numerus="yes">
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="404"/>
        <source>- %1 features found</source>
        <comment>Identify results window title</comment>
        <translation type="unfinished">
            <numerusform>
        
        
        </numerusform>
        </translation>
    </message>
</context>
<context>
    <name>QgsMapToolSplitFeatures</name>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="90"/>
        <source>Split error</source>
        <translation>Trennfehler</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="90"/>
        <source>An error occured during feature splitting</source>
        <translation>Ein Fehler ist beim Objekttrennen aufgetreten</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="85"/>
        <source>No feature split done</source>
        <translation type="unfinished">Keine Objekttrennung vorgenommen</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="85"/>
        <source>If there are selected features, the split tool only applies to the selected ones. If you like to split all features under the split line, clear the selection</source>
        <translation type="unfinished">Wenn Objekte ausgewählt sind, wird die Objekttrennung nur auf diese angewendet. Um alle Objekte zu trennen, muß die Auswahl aufgehoben werden.</translation>
    </message>
</context>
<context>
    <name>QgsMapToolVertexEdit</name>
    <message>
        <location filename="../src/app/qgsmaptoolvertexedit.cpp" line="51"/>
        <source>Snap tolerance</source>
        <translation>Snappingtoleranz</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolvertexedit.cpp" line="52"/>
        <source>Don&apos;t show this message again</source>
        <translation type="unfinished">Diese Nachricht nicht mehr anzeigen.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolvertexedit.cpp" line="57"/>
        <source>Could not snap segment.</source>
        <translation>Konnte Segment nicht schnappen.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolvertexedit.cpp" line="60"/>
        <source>Have you set the tolerance in Settings &gt; Project Properties &gt; General?</source>
        <translation>Haben Sie die Snappingtoleranz in Einstellungen &gt; Projekteinstellungen &gt; Allgemein eingestellt?</translation>
    </message>
</context>
<context>
    <name>QgsMapserverExport</name>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexport.cpp" line="76"/>
        <source>Name for the map file</source>
        <translation type="unfinished">Name des Mapfile</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexport.cpp" line="84"/>
        <source>Choose the QGIS project file</source>
        <translation type="unfinished">QGIS-Projektdatei wählen</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexport.cpp" line="201"/>
        <source>Overwrite File?</source>
        <translation type="unfinished">Datei überschreiben?</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmapserverexport.cpp" line="74"/>
        <source> exists. 
Do you want to overwrite it?</source>
        <translation type="unfinished"> existiert.
Wollen Sie sie überschreiben?</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexport.cpp" line="77"/>
        <source>MapServer map files (*.map);;All files (*.*)</source>
        <comment>Filter list for selecting files from a dialog box</comment>
        <translation>MapServer map files (*.map);;Alle Dateien (*.*)</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexport.cpp" line="85"/>
        <source>QGIS Project Files (*.qgs);;All files (*.*)</source>
        <comment>Filter list for selecting files from a dialog box</comment>
        <translation type="unfinished">QGIS-Projektdatei (*.qgs);;Alle Dateien (*.*)</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexport.cpp" line="203"/>
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
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="14"/>
        <source>Export to Mapserver</source>
        <translation>Exportieren in MapServer</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="352"/>
        <source>Map file</source>
        <translation>Kartendatei</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="403"/>
        <source>Export LAYER information only</source>
        <translation>Nur die Layer-Informationen exportieren</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="159"/>
        <source>Map</source>
        <translation>Karte</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="330"/>
        <source>Name</source>
        <translation>Name</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="304"/>
        <source>Height</source>
        <translation>Höhe</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="291"/>
        <source>Width</source>
        <translation>Breite</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="196"/>
        <source>dd</source>
        <translation>dd</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="201"/>
        <source>feet</source>
        <translation>Fuß</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="206"/>
        <source>meters</source>
        <translation>Meter</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="211"/>
        <source>miles</source>
        <translation>Meilen</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="216"/>
        <source>inches</source>
        <translation>Inch</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="221"/>
        <source>kilometers</source>
        <translation>Kilometer</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="182"/>
        <source>Units</source>
        <translation>Einheiten</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="229"/>
        <source>Image type</source>
        <translation>Bildtyp</translation>
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
        <translation>benutzerdefiniert</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="273"/>
        <source>wbmp</source>
        <translation>wbmp</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmapserverexportbase.ui" line="226"/>
        <source>MinScale</source>
        <translation>Minimalmassstab</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmapserverexportbase.ui" line="236"/>
        <source>MaxScale</source>
        <translation>Maximalmassstab</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmapserverexportbase.ui" line="252"/>
        <source>Prefix attached to map, scalebar and legend GIF filenames created using this MapFile. It should be kept short.</source>
        <translation>Präfix, der Name der GIF-Dateien für Karten, Maßstabsleiste und Legende, die mit diesem Mapfile erzeugt wurden.  Es sollte kurz gehalten werden.</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="29"/>
        <source>Web Interface Definition</source>
        <translation>Web Interface Definition</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="98"/>
        <source>Header</source>
        <translation>Kopfzeile</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="134"/>
        <source>Footer</source>
        <translation>Fußzeile</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="58"/>
        <source>Template</source>
        <translation>Vorlage</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmapserverexportbase.ui" line="56"/>
        <source>&amp;Help</source>
        <translation>&amp;Hilfe</translation>
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
        <translation>&amp;Abbrechen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmapserverexportbase.ui" line="116"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="362"/>
        <source>Name for the map file to be created from the QGIS project file</source>
        <translation>Name des Mapfiles, das aus dem QGIS-Projekt erzeugt werden soll.</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="400"/>
        <source>If checked, only the layer information will be processed</source>
        <translation>Wenn ausgewählt, werden nur die Layerinformationen verarbeitet.</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="68"/>
        <source>Path to the MapServer template file</source>
        <translation>Pfad zur MapServer-Vorlage</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="340"/>
        <source>Prefix attached to map, scalebar and legend GIF filenames created using this MapFile</source>
        <translation>Präfix, der Namen der GIF-Dateien für Karte, Massstabsleisten und Legenden, die mit diesem Mapfile erzeugt werden.</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="376"/>
        <source>Full path to the QGIS project file to export to MapServer map format</source>
        <translation>Kompletter Pfad zur QGIS-Projekt-Datei, die im MapServer Map-Format exportiert werden soll.</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="383"/>
        <source>QGIS project file</source>
        <translation>QGIS-Projektdatei</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="369"/>
        <source>Browse...</source>
        <translation type="unfinished">Durchsuchen...</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="393"/>
        <source>Save As...</source>
        <translation type="unfinished">Speichern unter...</translation>
    </message>
</context>
<context>
    <name>QgsMeasureBase</name>
    <message>
        <location filename="../src/ui/qgsmeasurebase.ui" line="19"/>
        <source>Measure</source>
        <translation>Messen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmeasurebase.ui" line="125"/>
        <source>New</source>
        <translation>Neu</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmeasurebase.ui" line="102"/>
        <source>Help</source>
        <translation>Hilfe</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmeasurebase.ui" line="132"/>
        <source>Cl&amp;ose</source>
        <translation>Schli&amp;eßen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmeasurebase.ui" line="66"/>
        <source>Total:</source>
        <translation>Summe:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmeasurebase.ui" line="86"/>
        <source>Segments</source>
        <translation type="unfinished">Segmente</translation>
    </message>
</context>
<context>
    <name>QgsMeasureDialog</name>
    <message>
        <location filename="../src/app/qgsmeasuredialog.cpp" line="196"/>
        <source>Segments (in meters)</source>
        <translation type="unfinished">Segmente (in Meter)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmeasuredialog.cpp" line="199"/>
        <source>Segments (in feet)</source>
        <translation type="unfinished">Segmente (in Fuß)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmeasuredialog.cpp" line="202"/>
        <source>Segments (in degrees)</source>
        <translation type="unfinished">Segmente (in Grad)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmeasuredialog.cpp" line="205"/>
        <source>Segments</source>
        <translation type="unfinished">Segmente</translation>
    </message>
</context>
<context>
    <name>QgsMeasureTool</name>
    <message>
        <location filename="../src/app/qgsmeasuretool.cpp" line="74"/>
        <source>Incorrect measure results</source>
        <translation type="unfinished">Falsche Messergebnisse</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmeasuretool.cpp" line="82"/>
        <source>&lt;p&gt;This map is defined with a geographic coordinate system (latitude/longitude) but the map extents suggests that it is actually a projected coordinate system (e.g., Mercator). If so, the results from line or area measurements will be incorrect.&lt;/p&gt;&lt;p&gt;To fix this, explicitly set an appropriate map coordinate system using the &lt;tt&gt;Settings:Project Properties&lt;/tt&gt; menu.</source>
        <translation>&lt;p&gt;Diese Karte ist mit einem geographischen Koordinatensystem definiert (latitude/longitude) aber die Kartenausdehnung zeigt, dass es tatsächlich eine projiziertes Koordinatensystem ist (z.B.: Mercator). Wenn das stimmt, sind die Ergebnisse der Strecken oder Flächenmessung falsch.&lt;/p&gt;&lt;p&gt;Um richtig messen zu können, definieren Sie bitte ein entsprechendes Koordinatensystem in dem Menü &lt;tt&gt;Einstellungen:Projekteinstellungen&lt;/tt&gt;.</translation>
    </message>
</context>
<context>
    <name>QgsMessageViewer</name>
    <message>
        <location filename="../src/ui/qgsmessageviewer.ui" line="13"/>
        <source>QGIS Message</source>
        <translation>QGIS-Nachricht</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmessageviewer.ui" line="48"/>
        <source>Close</source>
        <translation>Schließen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmessageviewer.ui" line="28"/>
        <source>Don&apos;t show this message again</source>
        <translation>Diese Nachricht nicht mehr anzeigen.</translation>
    </message>
</context>
<context>
    <name>QgsNewConnection</name>
    <message>
        <location filename="../src/app/qgsnewconnection.cpp" line="123"/>
        <source>Test connection</source>
        <translation>Verbindung testen</translation>
    </message>
    <message>
        <location filename="../src/app/qgsnewconnection.cpp" line="123"/>
        <source>Connection failed - Check settings and try again.

Extended error information:
</source>
        <translation>Verbindung fehlgeschlagen - Bitte Einstellungen überprüfen und erneut versuchen.

Ausführliche Fehlerinformation:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsnewconnection.cpp" line="119"/>
        <source>Connection to %1 was successful</source>
        <translation type="unfinished">Verbindung zu %1 war erfolgreich</translation>
    </message>
</context>
<context>
    <name>QgsNewConnectionBase</name>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="21"/>
        <source>Create a New PostGIS connection</source>
        <translation>Neue PostGIS-Verbindung erzeugen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="252"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="268"/>
        <source>Cancel</source>
        <translation>Abbrechen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="284"/>
        <source>Help</source>
        <translation>Hilfe</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="39"/>
        <source>Connection Information</source>
        <translation>Verbindunginformationen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="147"/>
        <source>Host</source>
        <translation>Host</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="157"/>
        <source>Database</source>
        <translation>Datenbank</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="177"/>
        <source>Username</source>
        <translation>Benutzername</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="137"/>
        <source>Name</source>
        <translation>Name</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="207"/>
        <source>Name of the new connection</source>
        <translation>Name der neuen Verbindung</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="187"/>
        <source>Password</source>
        <translation>Passwort</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="112"/>
        <source>Test Connect</source>
        <translation>Verbindung testen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="105"/>
        <source>Save Password</source>
        <translation>Passwort speichern</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="287"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="167"/>
        <source>Port</source>
        <translation>Port</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="220"/>
        <source>5432</source>
        <translation>5432</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="90"/>
        <source>Only look in the geometry_columns table</source>
        <translation>Nur in geometry_columns nachschauen.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="67"/>
        <source>Only look in the &apos;public&apos; schema</source>
        <translation>Nur im &apos;public&apos; Schema nachschauen.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="61"/>
        <source>Restrict the search to the public schema for spatial tables not in the geometry_columns table</source>
        <translation>Beschränke die Suche auf das public Schema für räumliche Tabellen nicht in der geometry_columns Tabelle</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="64"/>
        <source>When searching for spatial tables that are not in the geometry_columns tables, restrict the search to tables that are in the public schema (for some databases this can save lots of time)</source>
        <translation>Bei der Suche nach räumlichen Tabellen, die nicht in der Tabelle geometry_columns sind, beschränke die Suche auf Tabellen, die in dem public Schema sind (bei einigen Datenbanken kann es eine Menge Zeit sparen)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="84"/>
        <source>Restrict the displayed tables to those that are in the geometry_columns table</source>
        <translation>Beschränke angezeigte Tabellen auf jene aus der Tabelle geometry_columns</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="87"/>
        <source>Restricts the displayed tables to those that are in the geometry_columns table. This can speed up the initial display of spatial tables.</source>
        <translation>Beschränke dargestellte Tabellen auf jene aus der Tabelle geometry_columns. Dies kann die Anzeige räumlicher Tabellen beschleunigen.</translation>
    </message>
</context>
<context>
    <name>QgsNewHttpConnectionBase</name>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="31"/>
        <source>Name</source>
        <translation>Name</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="60"/>
        <source>URL</source>
        <translation>URL</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="50"/>
        <source>Name of the new connection</source>
        <translation>Name der neuen Verbindung</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="73"/>
        <source>HTTP address of the Web Map Server</source>
        <translation>HTTP-Adresse des WMS-Servers.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="13"/>
        <source>Create a new WMS connection</source>
        <translation type="unfinished">WMS-Verbindung anlegen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="25"/>
        <source>Connection details</source>
        <translation type="unfinished">Verbindungsdetails</translation>
    </message>
</context>
<context>
    <name>QgsNorthArrowPlugin</name>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="80"/>
        <source>Bottom Left</source>
        <translation>Unten links</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="81"/>
        <source>Top Right</source>
        <translation>Oben rechts</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="81"/>
        <source>Bottom Right</source>
        <translation>Unten rechts</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="81"/>
        <source>Top Left</source>
        <translation>Oben links</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="94"/>
        <source>&amp;North Arrow</source>
        <translation>&amp;Nordpfeil</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="95"/>
        <source>Creates a north arrow that is displayed on the map canvas</source>
        <translation>Erzeugt einen Nordpfeil und stellt ihn in der Karte dar.</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="254"/>
        <source>&amp;Decorations</source>
        <translation>&amp;Dekorationen</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="245"/>
        <source>North arrow pixmap not found</source>
        <translation>Nordpfeil nicht gefunden.</translation>
    </message>
</context>
<context>
    <name>QgsNorthArrowPluginGui</name>
    <message>
        <location filename="../src/plugins/north_arrow/plugingui.cpp" line="157"/>
        <source>Pixmap not found</source>
        <translation>Bild nicht gefunden</translation>
    </message>
</context>
<context>
    <name>QgsNorthArrowPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="235"/>
        <source>North Arrow Plugin</source>
        <translation>Nordpfeil Plugin</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="35"/>
        <source>Properties</source>
        <translation>Eigenschaften</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="55"/>
        <source>Angle</source>
        <translation>Winkel</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="62"/>
        <source>Placement</source>
        <translation>Platzierung</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="75"/>
        <source>Set direction automatically</source>
        <translation>Richtung automatisch setzen</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="85"/>
        <source>Enable North Arrow</source>
        <translation>Nordpfeil aktivieren</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="141"/>
        <source>Top Left</source>
        <translation>Oben links</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="146"/>
        <source>Top Right</source>
        <translation>Oben rechts</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="151"/>
        <source>Bottom Left</source>
        <translation>Unten links</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="156"/>
        <source>Bottom Right</source>
        <translation>Unten rechts</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="137"/>
        <source>Placement on screen</source>
        <translation>Platzierung am Bildschirm</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="164"/>
        <source>Preview of north arrow</source>
        <translation>Vorschau des Nordpfeils</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="183"/>
        <source>Icon</source>
        <translation>Icon</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="198"/>
        <source>Browse...</source>
        <translation type="unfinished">Durchsuchen...</translation>
    </message>
</context>
<context>
    <name>QgsOptions</name>
    <message>
        <location filename="../src/app/qgsoptions.cpp" line="156"/>
        <source>Detected active locale on your system: </source>
        <translation type="unfinished">Festgestellte Spracheinstellung des Systems: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsoptions.cpp" line="176"/>
        <source>to vertex</source>
        <translation type="unfinished">zum Stützpunkt</translation>
    </message>
    <message>
        <location filename="../src/app/qgsoptions.cpp" line="177"/>
        <source>to segment</source>
        <translation type="unfinished">zum Segment</translation>
    </message>
    <message>
        <location filename="../src/app/qgsoptions.cpp" line="178"/>
        <source>to vertex and segment</source>
        <translation type="unfinished">zum Stützpunkt und Segment</translation>
    </message>
    <message>
        <location filename="../src/app/qgsoptions.cpp" line="345"/>
        <source>Semi transparent circle</source>
        <translation type="unfinished">Teiltransparenter Kreis</translation>
    </message>
    <message>
        <location filename="../src/app/qgsoptions.cpp" line="349"/>
        <source>Cross</source>
        <translation type="unfinished">Kreuz</translation>
    </message>
    <message>
        <location filename="../src/app/qgsoptions.cpp" line="70"/>
        <source>Show all features</source>
        <translation type="unfinished">Alle Objekte anzeigen</translation>
    </message>
    <message>
        <location filename="../src/app/qgsoptions.cpp" line="71"/>
        <source>Show selected features</source>
        <translation type="unfinished">Nur selektierte Objekte anzeigen</translation>
    </message>
    <message>
        <location filename="../src/app/qgsoptions.cpp" line="72"/>
        <source>Show features in current canvas</source>
        <translation type="unfinished">Objekte im aktuellen Kartenausschnitt anzeigen</translation>
    </message>
</context>
<context>
    <name>QgsOptionsBase</name>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="13"/>
        <source>QGIS Options</source>
        <translation>QGIS-Optionen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="212"/>
        <source>Hide splash screen at startup</source>
        <translation>Splashscreen beim Start nicht anzeigen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="188"/>
        <source>&lt;b&gt;Note: &lt;/b&gt;Theme changes take effect the next time QGIS is started</source>
        <translation>&lt;b&gt;Beachte: &lt;/b&gt;Motivänderungen werden erst beim nächsten Start von QGIS aktiv</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="271"/>
        <source>&amp;Rendering</source>
        <translation>&amp;Darstellung</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="300"/>
        <source>Map display will be updated (drawn) after this many features have been read from the data source</source>
        <translation>Kartenanzeige wird erneuert (gezeichnet) nachdem soviele Objekte von der Datenquelle gelesen wurden</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="854"/>
        <source>Select Global Default ...</source>
        <translation>Globale Voreinstellung wählen ...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="341"/>
        <source>Make lines appear less jagged at the expense of some drawing performance</source>
        <translation>Linien auf Kosten der Zeichengeschwindigkeit weniger gezackt zeichnen.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="283"/>
        <source>By default new la&amp;yers added to the map should be displayed</source>
        <translation>Standardmäßig werden alle neuen Layer im Kartenfenster angezeigt.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="472"/>
        <source>Measure tool</source>
        <translation>Messwerkzeug</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="542"/>
        <source>Search radius</source>
        <translation>Suchradius</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="351"/>
        <source>Fix problems with incorrectly filled polygons</source>
        <translation>Problem mit falsch gefüllten Polygonen beheben.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="580"/>
        <source>%</source>
        <translation>%</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="358"/>
        <source>Continuously redraw the map when dragging the legend/map divider</source>
        <translation>Karte kontinuierlich neuzeichnen, wenn der Teiler zwischen Legende und Karte verschoben wird.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="382"/>
        <source>&amp;Map tools</source>
        <translation>&amp;Kartenwerkzeuge</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="400"/>
        <source>Panning and zooming</source>
        <translation>Verschieben und Zoomen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="419"/>
        <source>Zoom</source>
        <translation>Zoom</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="424"/>
        <source>Zoom and recenter</source>
        <translation>Zoomen und mittig zentrieren</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="434"/>
        <source>Nothing</source>
        <translation>Nichts</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="32"/>
        <source>&amp;General</source>
        <translation type="unfinished">&amp;Allgemein</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="907"/>
        <source>Locale</source>
        <translation type="unfinished">Sprache</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="922"/>
        <source>Locale to use instead</source>
        <translation type="unfinished">Stattdessen folgende Spracheinstellungen benutzen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="961"/>
        <source>Additional Info</source>
        <translation type="unfinished">Ergänzende Informationen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="967"/>
        <source>Detected active locale on your system:</source>
        <translation type="unfinished">Festgestellte aktive Spracheinstellung:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="613"/>
        <source>Digitizing</source>
        <translation type="unfinished">Digitalisierung</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="619"/>
        <source>Rubberband</source>
        <translation>Gummiband</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="635"/>
        <source>Line width in pixels</source>
        <translation type="unfinished">Linienbreite in Pixel</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="671"/>
        <source>Snapping</source>
        <translation>Snapping</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="348"/>
        <source>Selecting this will unselect the &apos;make lines less&apos; jagged toggle</source>
        <translation>Das Auswählen deaktiviert die Option Linien weniger gezackt zeichnen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="429"/>
        <source>Zoom to mouse cursor</source>
        <translation type="unfinished">Zur Mouseposition zoomen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="38"/>
        <source>Project files</source>
        <translation type="unfinished">Projektdateien</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="56"/>
        <source>Prompt to save project changes when required</source>
        <translation type="unfinished">Bei Bedarf nachfragen, ob geänderte Projekte gespeichert werden sollen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="63"/>
        <source>Warn when opening a project file saved with an older version of QGIS</source>
        <translation type="unfinished">Warnung ausgeben, wenn QGIS-Projekt einer früheren Version geöffnet wird.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="73"/>
        <source>Default Map Appearance (overridden by project properties)</source>
        <translation type="unfinished">Voreingestelle Kartenaussehen (Projekteigenschaften überschreiben)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="79"/>
        <source>Selection color</source>
        <translation type="unfinished">Farbe für Auswahlen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="115"/>
        <source>Background color</source>
        <translation type="unfinished">Hintergrundfarbe</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="154"/>
        <source>&amp;Application</source>
        <translation type="unfinished">&amp;Anwendung</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="166"/>
        <source>Icon theme</source>
        <translation type="unfinished">Icon-Thema</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="198"/>
        <source>Capitalise layer names in legend</source>
        <translation type="unfinished">Layernamen großschreiben</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="205"/>
        <source>Display classification attribute names in legend</source>
        <translation type="unfinished">Klassifikationsattributnamen in der Legende anzeigen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="277"/>
        <source>Rendering behavior</source>
        <translation type="unfinished">Zeichenverhalten</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="290"/>
        <source>Number of features to draw before updating the display</source>
        <translation type="unfinished">Anzahl von Objekten nach deren Zeichnung die Anzeige aktualisiert werden soll</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="313"/>
        <source>&lt;b&gt;Note:&lt;/b&gt; Use zero to prevent display updates until all features have been rendered</source>
        <translation type="unfinished">&lt;b&gt;Note:&lt;/b&gt; 0 sorgt dafür, dass erst aktualisiert wird, wenn alle Objekte gezeichnet wurden</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="323"/>
        <source>Rendering quality</source>
        <translation type="unfinished">Zeichenqualität</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="442"/>
        <source>Zoom factor</source>
        <translation type="unfinished">Zoomfaktor</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="449"/>
        <source>Mouse wheel action</source>
        <translation type="unfinished">Mausradaktion</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="519"/>
        <source>Rubberband color</source>
        <translation type="unfinished">Gummibandfarbe</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="529"/>
        <source>Ellipsoid for distance calculations</source>
        <translation type="unfinished">Ellipsoid für Abstandsberechnungen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="560"/>
        <source>&lt;b&gt;Note:&lt;/b&gt; Specify the search radius as a percentage of the map width</source>
        <translation type="unfinished">&lt;b&gt;Anmerkung:&lt;/b&gt; Suchradius in Prozent der Kartenbreite angeben</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="570"/>
        <source>Search radius for identifying features and displaying map tips</source>
        <translation type="unfinished">Suchradius für die Objektidentifikation und zur Maptippanzeige</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="625"/>
        <source>Line width</source>
        <translation type="unfinished">Linienbreite</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="645"/>
        <source>Line colour</source>
        <translation type="unfinished">Linienfarbe</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="677"/>
        <source>Default snap mode</source>
        <translation type="unfinished">Voreingestellter Fangmodus</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="707"/>
        <source>Default snapping tolerance in layer units</source>
        <translation type="unfinished">Voreingestellte Fangtoleranz in Layereinheiten</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="737"/>
        <source>Search radius for vertex edits in layer units</source>
        <translation type="unfinished">Suchradius für Knickpunktbearbeitung in Layereinheiten</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="770"/>
        <source>Vertex markers</source>
        <translation type="unfinished">Knickpunktmarken</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="776"/>
        <source>Marker style</source>
        <translation type="unfinished">Markenstil</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="913"/>
        <source>Override system locale</source>
        <translation type="unfinished">System-Locale überschreiben</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="935"/>
        <source>&lt;b&gt;Note:&lt;/b&gt; Enabling / changing overide on local requires an application restart</source>
        <translation type="unfinished">&lt;b&gt;Note:&lt;/b&gt; Einschalten/Änderun der Locale-Überschreibung erfordert einen Anwendungsneustart</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="978"/>
        <source>Proxy</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="984"/>
        <source>Use proxy for web access</source>
        <translation type="unfinished">Proxy für Webzugriff benutzen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="996"/>
        <source>Host</source>
        <translation type="unfinished">Host</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="1009"/>
        <source>Port</source>
        <translation type="unfinished">Port</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="1022"/>
        <source>User</source>
        <translation type="unfinished">Benutzer</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="1049"/>
        <source>Leave this blank if no proxy username / password are required</source>
        <translation type="unfinished">Lassen Sie Benutzer/Passwort leer, wenn sie nicht benötigt werden.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="1039"/>
        <source>Password</source>
        <translation type="unfinished">Passwort</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="219"/>
        <source>Open attribute table in a dock window</source>
        <translation type="unfinished">Attributtabelle gedockt öffnen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="232"/>
        <source>Attribute table behaviour</source>
        <translation type="unfinished">Verhalten der Attributtabelle</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="823"/>
        <source>CRS</source>
        <translation type="unfinished">KBS</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="864"/>
        <source>When layer is loaded that has no coordinate reference system (CRS)</source>
        <translation type="unfinished">Wenn ein Layer ohne Koordinatenbezugssystem (KBS) geladen wird</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="882"/>
        <source>Prompt for CRS</source>
        <translation type="unfinished">KBS abfragen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="889"/>
        <source>Project wide default CRS will be used</source>
        <translation type="unfinished">Projektweite KBS-Voreinstellung</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="896"/>
        <source>Global default CRS displa&amp;yed below will be used</source>
        <translation type="unfinished">Untenstehende globale Voreinstellung wird genutzt</translation>
    </message>
</context>
<context>
    <name>QgsPasteTransformationsBase</name>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="16"/>
        <source>Paste Transformations</source>
        <translation>Transformationen einfügen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="39"/>
        <source>&lt;b&gt;Note: This function is not useful yet!&lt;/b&gt;</source>
        <translation>&lt;b&gt;Bemerkung: Diese Funktion ist noch nicht nützlich!&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="62"/>
        <source>Source</source>
        <translation>Quelle</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="83"/>
        <source>Destination</source>
        <translation>Ziel</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="122"/>
        <source>&amp;Help</source>
        <translation>&amp;Hilfe</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="125"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="151"/>
        <source>Add New Transfer</source>
        <translation>Neuen Transfer hinzufügen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="158"/>
        <source>&amp;OK</source>
        <translation>&amp;OK</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="174"/>
        <source>&amp;Cancel</source>
        <translation>&amp;Abbrechen</translation>
    </message>
</context>
<context>
    <name>QgsPgGeoprocessing</name>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="119"/>
        <source>Buffer features in layer %1</source>
        <translation>Puffer Objekte in Layer %1</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="364"/>
        <source>Error connecting to the database</source>
        <translation>Fehler beim Verbinden mit der Datenbank</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="76"/>
        <source>&amp;Buffer features</source>
        <translation>O&amp;bjekte puffern.</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="78"/>
        <source>A new layer is created in the database with the buffered features.</source>
        <translation>Ein neuer Layer ist in der Datenbank erstellt wurden, der die gepufferten Objekte enthält.</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="445"/>
        <source>&amp;Geoprocessing</source>
        <translation>&amp;Geodatenverarbeitung</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="345"/>
        <source>Unable to add geometry column</source>
        <translation>Konnte die Geometriespalte nicht hinzufügen.</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="347"/>
        <source>Unable to add geometry column to the output table </source>
        <translation>Geometriespalte konnte nicht zur Ausgabetabelle hinzufügen: </translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="353"/>
        <source>Unable to create table</source>
        <translation>Kann die Tabelle nicht erstellen.</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="355"/>
        <source>Failed to create the output table </source>
        <translation>Erstellen der Ausgabetabelle fehlgeschlagen.</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="370"/>
        <source>No GEOS support</source>
        <translation>Keine GEOS-Unterstützung.</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="371"/>
        <source>Buffer function requires GEOS support in PostGIS</source>
        <translation>Pufferfunktion benötigt GEOS-Unterstützung in PostGIS.</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="379"/>
        <source> is not a PostgreSQL/PostGIS layer.
</source>
        <translation> ist keine PostgreSQL/PostGIS-Layer.</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="379"/>
        <source>Geoprocessing functions are only available for PostgreSQL/PostGIS Layers</source>
        <translation>Geodatenverarbeitungsfunktionen sind nur für PostgreSQL/PostGIS-Layer vorgesehen.</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="384"/>
        <source>No Active Layer</source>
        <translation>Kein aktiver Layer</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="385"/>
        <source>You must select a layer in the legend to buffer</source>
        <translation>Wählen Sie einen Layer in der Legende, der gepuffert werden soll.</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="376"/>
        <source>Not a PostgreSQL/PostGIS Layer</source>
        <translation>Kein PostgreSQL/PostGIS Layer</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="78"/>
        <source>Create a buffer for a PostgreSQL layer. </source>
        <translation type="unfinished">Puffer für einen PostGIS-Layer erzeugen</translation>
    </message>
</context>
<context>
    <name>QgsPgQueryBuilder</name>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="84"/>
        <source>Table &lt;b&gt;%1&lt;/b&gt; in database &lt;b&gt;%2&lt;/b&gt; on host &lt;b&gt;%3&lt;/b&gt;, user &lt;b&gt;%4&lt;/b&gt;</source>
        <translation>Tabelle &lt;b&gt;%1&lt;/b&gt; in Datenbank &lt;b&gt;%2&lt;/b&gt; auf Host &lt;b&gt;%3&lt;/b&gt;, Benutzer &lt;b&gt;%4&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="68"/>
        <source>Connection Failed</source>
        <translation>Verbindung fehlgeschlagen</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="68"/>
        <source>Connection to the database failed:</source>
        <translation>Verbindung zur Datenbank fehlgeschlagen:</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="207"/>
        <source>Database error</source>
        <translation>Datenbankfehler</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="282"/>
        <source>Query Result</source>
        <translation>Erfrage Resultat</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="284"/>
        <source>The where clause returned </source>
        <translation>Die WHERE-Klausel gab </translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="284"/>
        <source> rows.</source>
        <translation> Zeilen zurück.</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="288"/>
        <source>Query Failed</source>
        <translation>Abfrage fehlgeschlagen</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="290"/>
        <source>An error occurred when executing the query:</source>
        <translation>Während der Ausführung der Abfrage trat ein Fehler auf:</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="344"/>
        <source>No Records</source>
        <translation>Keine Einträge</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="344"/>
        <source>The query you specified results in zero records being returned. Valid PostgreSQL layers must have at least one feature.</source>
        <translation>Die Abfrage ergab keine Einträge. Gültige PostgreSQL-Layer müssen mindestens ein Objekt enthalten.</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="207"/>
        <source>&lt;p&gt;Failed to get sample of field values using SQL:&lt;/p&gt;&lt;p&gt;</source>
        <translation>&lt;p&gt;Konnte keine Beispiele der Werte mit SQL holen:&lt;/p&gt;&lt;p&gt;</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="271"/>
        <source>No Query</source>
        <translation type="unfinished">Keine Abfrage</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="271"/>
        <source>You must create a query before you can test it</source>
        <translation type="unfinished">Sie müssen eine Anfrage erstellen bevor Sie sie testen können.</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="337"/>
        <source>Error in Query</source>
        <translation type="unfinished">Fehler in Abfrage</translation>
    </message>
</context>
<context>
    <name>QgsPgQueryBuilderBase</name>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="21"/>
        <source>PostgreSQL Query Builder</source>
        <translation>PostgreSQL Query Builder</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="328"/>
        <source>Clear</source>
        <translation>Löschen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="338"/>
        <source>Test</source>
        <translation>Testen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="348"/>
        <source>Ok</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="358"/>
        <source>Cancel</source>
        <translation>Abbrechen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="83"/>
        <source>Values</source>
        <translation>Werte</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="139"/>
        <source>All</source>
        <translation>Alle</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="126"/>
        <source>Sample</source>
        <translation>Stichprobe</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="46"/>
        <source>Fields</source>
        <translation>Felder</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="149"/>
        <source>Operators</source>
        <translation>Operatoren</translation>
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
        <translation>ÄHNLICH</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="195"/>
        <source>AND</source>
        <translation>UND</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="244"/>
        <source>ILIKE</source>
        <translation>ILIKE</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="188"/>
        <source>OR</source>
        <translation>ODER</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="181"/>
        <source>NOT</source>
        <translation>NICHT</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="274"/>
        <source>SQL where clause</source>
        <translation>SQL where clause</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="133"/>
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
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="120"/>
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
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="101"/>
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
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="64"/>
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
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="33"/>
        <source>Datasource</source>
        <translation>Datenquelle</translation>
    </message>
</context>
<context>
    <name>QgsPluginInstaller</name>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="33"/>
        <source>Couldn&apos;t parse output from the repository</source>
        <translation type="unfinished">Konnte Ausgabe des Repositorys nicht interpretieren</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="34"/>
        <source>Couldn&apos;t open the system plugin directory</source>
        <translation type="unfinished">Konnte Systempluginverzeichnis nicht öffnen</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="39"/>
        <source>Couldn&apos;t open the local plugin directory</source>
        <translation type="unfinished">Konnte lokales Pluginverzeichnis nicht öffnen</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="45"/>
        <source>Fetch Python Plugins...</source>
        <translation type="unfinished">Python-Plugins herunterladen...</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="47"/>
        <source>Install more plugins from remote repositories</source>
        <translation type="unfinished">Mehr Plugins von entfernten Repositorys installieren</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="50"/>
        <source>Looking for new plugins...</source>
        <translation type="unfinished">Neue Plugins werden gesucht...</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="51"/>
        <source>There is a new plugin available</source>
        <translation type="unfinished">Es gibt ein neues Plugin</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="54"/>
        <source>There is a plugin update available</source>
        <translation type="unfinished">Es gibt ein Plugin-Update</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="55"/>
        <source>QGIS Python Plugin Installer</source>
        <translation type="unfinished">QGIS-Python-Plugin-Installation</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="60"/>
        <source>Error reading repository:</source>
        <translation type="unfinished">Fehler beim Lesen des Repositorys</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="66"/>
        <source>Nothing to remove! Plugin directory doesn&apos;t exist:</source>
        <translation type="unfinished">Nichts zu entfernen! Pluginverzeichnis existiert nicht:</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="67"/>
        <source>Failed to remove the directory:</source>
        <translation type="unfinished">Das Verzeichnis konnte nicht gelöscht werden:</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="69"/>
        <source>Check permissions or remove it manually</source>
        <translation type="unfinished">Überprüfen Sie die Zugriffsrechte oder entfernen Sie es manuell</translation>
    </message>
</context>
<context>
    <name>QgsPluginInstallerDialog</name>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="221"/>
        <source>QGIS Python Plugin Installer</source>
        <translation type="unfinished">QGIS-Python-Plugin-Installation</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="117"/>
        <source>Error reading repository:</source>
        <translation type="unfinished">Fehler beim Lesen des Repositorys:</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="118"/>
        <source>all repositories</source>
        <translation type="unfinished">Alle Repositorys</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="119"/>
        <source>connected</source>
        <translation type="unfinished">verbunden</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="120"/>
        <source>This repository is connected</source>
        <translation type="unfinished">Repository ist verbunden</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="121"/>
        <source>unavailable</source>
        <translation type="unfinished">nicht verfügbar</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="122"/>
        <source>This repository is enabled, but unavailable</source>
        <translation type="unfinished">Diese Repository ist aktiv, aber nicht verfügbar</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="123"/>
        <source>disabled</source>
        <translation type="unfinished">deaktiviert</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="124"/>
        <source>This repository is disabled</source>
        <translation type="unfinished">Dieses Repository ist deaktiviert</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="212"/>
        <source>This repository is blocked due to incompatibility with your Quantum GIS version</source>
        <translation type="unfinished">Diese Repository wurde wegen Inkompatibilität zu Ihrer Quantum GIS-Version blockiert</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="138"/>
        <source>orphans</source>
        <translation type="unfinished">Waisen</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="127"/>
        <source>any status</source>
        <translation type="unfinished">Alle Zustände</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="128"/>
        <source>not installed</source>
        <comment>plural</comment>
        <translation type="unfinished">nicht installiert</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="129"/>
        <source>installed</source>
        <comment>plural</comment>
        <translation type="unfinished">installiert</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="134"/>
        <source>upgradeable and news</source>
        <translation type="unfinished">Aktualisierungen und Neuigkeiten</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="139"/>
        <source>This plugin is not installed</source>
        <translation type="unfinished">Dieses Plugin ist nicht installiert</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="140"/>
        <source>This plugin is installed</source>
        <translation type="unfinished">Dieses Plugin ist installiert</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="141"/>
        <source>This plugin is installed, but there is an updated version available</source>
        <translation type="unfinished">Dieses Plugin ist installiert, aber es gibt eine neuere Version</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="142"/>
        <source>This plugin is installed, but I can&apos;t find it in any enabled repository</source>
        <translation type="unfinished">Dieses Plugin ist installiert, aber in keinem aktivierten Repository zu finden</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="143"/>
        <source>This plugin is not installed and is seen for the first time</source>
        <translation type="unfinished">Dieses Plugin ist nicht installiert und wurde zum ersten Mal entdeckt.</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="144"/>
        <source>This plugin is installed and is newer than its version available in a repository</source>
        <translation type="unfinished">Dieses Plugin ist installiert und ist aktueller als die Version in einem Repository</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="148"/>
        <source>not installed</source>
        <comment>singular</comment>
        <translation type="unfinished">nicht installiert</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="153"/>
        <source>installed</source>
        <comment>singular</comment>
        <translation type="unfinished">installiert</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="150"/>
        <source>upgradeable</source>
        <comment>singular</comment>
        <translation type="unfinished">aktualisierbar</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="152"/>
        <source>new!</source>
        <comment>singular</comment>
        <translation type="unfinished">neu!</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="154"/>
        <source>invalid</source>
        <comment>singular</comment>
        <translation type="unfinished">ungültig</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="161"/>
        <source>installed version</source>
        <translation type="unfinished">installierte Version</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="158"/>
        <source>available version</source>
        <translation type="unfinished">verfügbare Version</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="160"/>
        <source>That&apos;s the newest available version</source>
        <translation type="unfinished">Dies ist die neuste verfügbare Version</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="162"/>
        <source>There is no version available for download</source>
        <translation type="unfinished">Keine Version zum Download verfügbar</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="168"/>
        <source>only locally available</source>
        <translation type="unfinished">nur lokal verfügbar</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="173"/>
        <source>Install plugin</source>
        <translation type="unfinished">Plugin installieren</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="175"/>
        <source>Reinstall plugin</source>
        <translation type="unfinished">Plugin erneut installieren</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="171"/>
        <source>Upgrade plugin</source>
        <translation type="unfinished">Plugin aktualisieren</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="178"/>
        <source>Install/upgrade plugin</source>
        <translation type="unfinished">Plugin installieren/aktualisieren</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="174"/>
        <source>Downgrade plugin</source>
        <translation type="unfinished">Frühere Version des Plugins installieren</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="180"/>
        <source>Are you sure you want to downgrade the plugin to the latest available version? The installed one is newer!</source>
        <translation type="unfinished">Sind Sie sicher, dass sie eine früherer Version des Plugins installieren wollen?  Die installierte ist aktueller!</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="181"/>
        <source>Plugin installation failed</source>
        <translation type="unfinished">Plugin-Installation schlug fehl</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="182"/>
        <source>Plugin has disappeared</source>
        <translation type="unfinished">Plugin ist verschwunden</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="183"/>
        <source>The plugin seems to have been installed but I don&apos;t know where. Probably the plugin package contained a wrong named directory.
Please search the list of installed plugins. I&apos;m nearly sure you&apos;ll find the plugin there, but I just can&apos;t determine which of them it is. It also means that I won&apos;t be able to determine if this plugin is installed and inform you about available updates. However the plugin may work. Please contact the plugin author and submit this issue.</source>
        <translation type="unfinished">Das Plugin scheint installiert worden zu sein, aber der Ort konnte nicht festgestellt werden.  Wahrscheinlich enthielt das Plugin-Paket ein falsch benanntes Verzeichnis.
Bitte durchsuchen Sie die Liste der installierten Plugins. Dies bedeutet ausserdem, dass es nicht möglich ist festzustellen, dass das Plugin installiert ist und ob es Updates gibt. Nichtdestotrotz könnte das Plugin funktionieren.  Bitte kontaktieren Sie den Plugin-Autor und setzen Ihn davon in Kenntnis.</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="184"/>
        <source>Plugin installed successfully</source>
        <translation type="unfinished">Plugin erfolgreich installiert</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="197"/>
        <source>Plugin uninstall failed</source>
        <translation type="unfinished">Plugin-Deinstallation gescheitert</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="194"/>
        <source>Are you sure you want to uninstall the following plugin?</source>
        <translation type="unfinished">Sind Sie sicher, dass sie folgendes Plugin deinstallieren wollen?</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="195"/>
        <source>Warning: this plugin isn&apos;t available in any accessible repository!</source>
        <translation type="unfinished">Warnung: dieses Plugin ist in keinem verfügbaren Repository enthalten.</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="199"/>
        <source>Plugin uninstalled successfully</source>
        <translation type="unfinished">Plugin erfolgreich deinstalliert</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="204"/>
        <source>You are going to add some plugin repositories neither authorized nor supported by the Quantum GIS team, however provided by folks associated with us. Plugin authors generally make efforts to make their works useful and safe, but we can&apos;t assume any responsibility for them. FEEL WARNED!</source>
        <translation type="unfinished">Sie sind dabei Plugin-Repositorys hinzufügen, die zwar vom Quantum GIS-Team weder autorisiert noch unterstützt werden, aber von uns verbundenen Leuten zur Verfügung gestellt werden. Plugin-Autoren versuchen grundsätzlich ihre Arbeite nützlich und sicher zu machen, aber wir können uns auf deren Verantwortungsgefühl nicht verlassen.  Seien Sie gewarnt!</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="216"/>
        <source>Unable to add another repository with the same URL!</source>
        <translation type="unfinished">Ein Repository einer bereits vorhandene URL kann nicht hinzugefügt werden.</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="217"/>
        <source>Are you sure you want to remove the following repository?</source>
        <translation type="unfinished">Sind Sie sicher, dass die folgendes Repository entfernen wollen?</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="145"/>
        <source>This plugin is incompatible with your Quantum GIS version and probably won&apos;t work.</source>
        <translation type="unfinished">Dieses Plugin ist inkompatibel mit Ihrer Quantum GIS-Version und wird wahrscheinlich nicht funktionieren.</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="147"/>
        <source>This plugin seems to be broken.
It has been installed but can&apos;t be loaded.
Here is the error message:</source>
        <translation type="unfinished">Dieses Plugin scheint defekt zu sein.
Es wurde installiert, konnte aber nicht geladen werden.
Hier die Fehlermeldung:</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="155"/>
        <source>Note that it&apos;s an uninstallable core plugin</source>
        <translation type="unfinished">Beachten Sie, dass es ein nicht installierbares Core-Plugin ist.</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="163"/>
        <source>This plugin is broken</source>
        <translation type="unfinished">Dieses Plugin ist defekt.</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="164"/>
        <source>This plugin requires a newer version of Quantum GIS</source>
        <translation type="unfinished">Diese Plugin benötigt eine neuere Quantum GIS-Version</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="165"/>
        <source>This plugin requires a missing module</source>
        <translation type="unfinished">Dieses Plugin benötigt ein fehlendes Modul.</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="186"/>
        <source>Plugin reinstalled successfully</source>
        <translation type="unfinished">Plugin erfolgreich neuinstalliert.</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="188"/>
        <source>The plugin is designed for a newer version of Quantum GIS. The minimum required version is:</source>
        <translation type="unfinished">Dieses Plugin ist für eine neuere Version von Quantum GIS bestimmt.  Die minimal erforderliche Version ist:</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="189"/>
        <source>The plugin depends on some components missing on your system. You need to install the following Python module in order to enable it:</source>
        <translation type="unfinished">Dieses Plugin hängt von einigen auf Ihrem System fehlenden Komponenten ab. Sie müssen folgende Pythonmodule installieren, um es zu aktivieren:</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="190"/>
        <source>The plugin is broken. Python said:</source>
        <translation type="unfinished">Das Plugin ist defekt.  Python meldete:</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="146"/>
        <source>The required Python module is not installed.
For more information, please visit its homepage and Quantum GIS wiki.</source>
        <translation type="unfinished">Das benötigte Python-Modul ist nicht installiert.
Für weitere Informationen besuchen Sie bitte dessen Homepage und das Quantum GIS-Wiki</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="185"/>
        <source>Python plugin installed.
Now you need to enable it in Plugin Manager.</source>
        <translation type="unfinished">Python-Plugin installiert.
Sie müssen es noch im Plugin-Manager aktivieren.</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="187"/>
        <source>Python plugin reinstalled.
You need to restart Quantum GIS in order to reload it.</source>
        <translation type="unfinished">Python plugin neuinstalliert.
Sie müssen Quantum GIS neustartet, um es neuzuladen.</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="203"/>
        <source>Python plugin uninstalled. Note that you may need to restart Quantum GIS in order to remove it completely.</source>
        <translation type="unfinished">Python-Plugin deinstalliert.  Beachten Sie, dass Sie Quantum GIS u.U. neustarten müssen, um es komplett zu entfernen.</translation>
    </message>
</context>
<context>
    <name>QgsPluginInstallerDialogBase</name>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="20"/>
        <source>QGIS Python Plugin Installer</source>
        <translation type="unfinished">QGIS-Python-Plugin-Installation</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="36"/>
        <source>Plugins</source>
        <translation type="unfinished">Plugins</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="39"/>
        <source>List of available and installed plugins</source>
        <translation type="unfinished">Liste der verfügbaren und installierten Plugins</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="50"/>
        <source>Filter:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="63"/>
        <source>Display only plugins containing this word in their metadata</source>
        <translation type="unfinished">Nur Plugins anzeigen deren Metadaten dieses Wort enthalten</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="85"/>
        <source>Display only plugins from given repository</source>
        <translation type="unfinished">Nur Plugins des angegebenen Repository anzeigen</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="89"/>
        <source>all repositories</source>
        <translation type="unfinished">Alle Repositories</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="109"/>
        <source>Display only plugins with matching status</source>
        <translation type="unfinished">Nur passende Plugins anzeigen</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="242"/>
        <source>Status</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="247"/>
        <source>Name</source>
        <translation type="unfinished">Name</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="144"/>
        <source>Version</source>
        <translation type="unfinished">Version</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="149"/>
        <source>Description</source>
        <translation type="unfinished">Beschreibung</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="154"/>
        <source>Author</source>
        <translation type="unfinished">Autor</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="159"/>
        <source>Repository</source>
        <translation type="unfinished">Repository</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="197"/>
        <source>Install, reinstall or upgrade the selected plugin</source>
        <translation type="unfinished">Gewähltes Plugin installieren, neu installieren oder aktualisieren</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="200"/>
        <source>Install/upgrade plugin</source>
        <translation type="unfinished">Plugin installieren/aktualisieren</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="213"/>
        <source>Uninstall the selected plugin</source>
        <translation type="unfinished">Das gewählte Plugin deinstallieren</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="216"/>
        <source>Uninstall plugin</source>
        <translation type="unfinished">Gewähltes Plugin deinstallieren</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="226"/>
        <source>Repositories</source>
        <translation type="unfinished">Repositorys</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="229"/>
        <source>List of plugin repositories</source>
        <translation type="unfinished">Liste der Plugin-Repositories</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="252"/>
        <source>URL</source>
        <translation type="unfinished">URL</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="266"/>
        <source>Allow the Installer to look for updates and news in enabled repositories on QGIS startup</source>
        <translation type="unfinished">Dem Installer gestatten beim QGIS-Start nach Aktualisierungen und Neuigkeiten zu suchen</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="269"/>
        <source>Check for updates on startup</source>
        <translation type="unfinished">Beim Start nach Aktualisierungen suchen</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="298"/>
        <source>Add third party plugin repositories to the list</source>
        <translation type="unfinished">Plugin-Repositories von Dritten zur Liste hinzufügen</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="301"/>
        <source>Add 3rd party repositories</source>
        <translation type="unfinished">Plugin-Repositories hinzufügen</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="324"/>
        <source>Add a new plugin repository</source>
        <translation type="unfinished">Ein neues Plugin-Repository ergänzen</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="327"/>
        <source>Add...</source>
        <translation type="unfinished">Hinzufügen...</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="337"/>
        <source>Edit the selected repository</source>
        <translation type="unfinished">Gewähltes Respository bearbeiten</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="340"/>
        <source>Edit...</source>
        <translation type="unfinished">Bearbeiten...</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="350"/>
        <source>Remove the selected repository</source>
        <translation type="unfinished">Gewähltes Repository entfernen</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="353"/>
        <source>Delete</source>
        <translation type="unfinished">Löschen</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="369"/>
        <source>The plugins will be installed to ~/.qgis/python/plugins</source>
        <translation type="unfinished">Das Plugin wird nach ~/.qgis/python/plugins installiert</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="385"/>
        <source>Close the Installer window</source>
        <translation type="unfinished">Das Installationsfenster schließen</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="388"/>
        <source>Close</source>
        <translation type="unfinished">Schließen</translation>
    </message>
</context>
<context>
    <name>QgsPluginInstallerFetchingDialog</name>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="73"/>
        <source>Success</source>
        <translation type="unfinished">Erfolg</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="74"/>
        <source>Resolving host name...</source>
        <translation type="unfinished">Löse Hostname auf...</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="75"/>
        <source>Connecting...</source>
        <translation type="unfinished">Verbinde...</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="76"/>
        <source>Host connected. Sending request...</source>
        <translation type="unfinished">Verbunden. Sende Anfrage...</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="77"/>
        <source>Downloading data...</source>
        <translation type="unfinished">Daten werden heruntergeladen...</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="78"/>
        <source>Idle</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="79"/>
        <source>Closing connection...</source>
        <translation type="unfinished">Verbindung wird geschlossen...</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="80"/>
        <source>Error</source>
        <translation type="unfinished">Fehler</translation>
    </message>
</context>
<context>
    <name>QgsPluginInstallerFetchingDialogBase</name>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerfetchingbase.ui" line="14"/>
        <source>Fetching repositories</source>
        <translation type="unfinished">Lade Repositories</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerfetchingbase.ui" line="39"/>
        <source>Overall progress:</source>
        <translation type="unfinished">Gesamtfortschritt</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerfetchingbase.ui" line="111"/>
        <source>Abort fetching</source>
        <translation type="unfinished">Ladevorgang abbrechen</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerfetchingbase.ui" line="167"/>
        <source>Repository</source>
        <translation type="unfinished">Repository</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerfetchingbase.ui" line="172"/>
        <source>State</source>
        <translation type="unfinished">Status</translation>
    </message>
</context>
<context>
    <name>QgsPluginInstallerInstallingDialog</name>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="86"/>
        <source>Installing...</source>
        <translation type="unfinished">Installiere...</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="87"/>
        <source>Resolving host name...</source>
        <translation type="unfinished">Löse Hostname auf...</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="88"/>
        <source>Connecting...</source>
        <translation type="unfinished">Verbinde...</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="89"/>
        <source>Host connected. Sending request...</source>
        <translation type="unfinished">Verbunden. Sende Anfrage...</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="90"/>
        <source>Downloading data...</source>
        <translation type="unfinished">Daten werden heruntergeladen...</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="91"/>
        <source>Idle</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="92"/>
        <source>Closing connection...</source>
        <translation type="unfinished">Verbindung wird geschlossen...</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="95"/>
        <source>Error</source>
        <translation type="unfinished">Fehler</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="98"/>
        <source>Failed to unzip the plugin package. Probably it&apos;s broken or missing from the repository. You may also want to make sure that you have write permission to the plugin directory:</source>
        <translation type="unfinished">Plugin-Paket konnte nicht ausgepackt werden. Es ist wahrscheinlich defekt oder fehlt im Repository. Sie sollten sicherstellen, dass Sie Schreibrechte im Plugin-Verzeichnis haben:</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="99"/>
        <source>Aborted by user</source>
        <translation type="unfinished">Durch Benutzer abgebrochen</translation>
    </message>
</context>
<context>
    <name>QgsPluginInstallerInstallingDialogBase</name>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerinstallingbase.ui" line="14"/>
        <source>QGIS Python Plugin Installer</source>
        <translation type="unfinished">QGIS-Python-Plugin-Installation</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerinstallingbase.ui" line="41"/>
        <source>Installing plugin:</source>
        <translation type="unfinished">Plugin wird installiert:</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerinstallingbase.ui" line="63"/>
        <source>Connecting...</source>
        <translation type="unfinished">Verbinde...</translation>
    </message>
</context>
<context>
    <name>QgsPluginInstallerPluginErrorDialog</name>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="106"/>
        <source>no error message received</source>
        <translation type="unfinished">keine Fehlermeldung empfangen</translation>
    </message>
</context>
<context>
    <name>QgsPluginInstallerPluginErrorDialogBase</name>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerpluginerrorbase.ui" line="20"/>
        <source>Error loading plugin</source>
        <translation type="unfinished">Fehler beim Laden des Plugins</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerpluginerrorbase.ui" line="35"/>
        <source>The plugin seems to be invalid or have unfulfilled dependencies. It has been installed, but can&apos;t be loaded. If you really need this plugin, you can contact its author or &lt;a href=&quot;http://lists.osgeo.org/mailman/listinfo/qgis-user&quot;&gt;QGIS users group&lt;/a&gt; and try to solve the problem. If not, you can just uninstall it. Here is the error message below:</source>
        <translation type="unfinished">Das Plugin scheint ungültig zu sein oder ihm fehlen Abhängigkeiten. Es wurde installiert, aber konnte nicht geladen werden. Wenn Sie das Plugin wirklich brauchen, kontaktieren Sie den Autor oder die &lt;a href=&quot;http://lists.osgeo.org/mailman/listinfo/qgis-user&quot;&gt;QGIS-Benutzergruppe&lt;/a&gt;, um das Problem zu lösen. Anderenfalls können Sie es einfach wieder deinstallieren.  Im folgenden die Fehlermeldung:</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerpluginerrorbase.ui" line="83"/>
        <source>Do you want to uninstall this plugin now? If you&apos;re unsure, probably you would like to do this.</source>
        <translation type="unfinished">Wollen Sie das Plugin jetzt deinstallieren? Im Zweifelsfall sollten Sie dies wahrscheinlich tun.</translation>
    </message>
</context>
<context>
    <name>QgsPluginInstallerRepositoryDetailsDialogBase</name>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerrepositorybase.ui" line="20"/>
        <source>Repository details</source>
        <translation type="unfinished">Repository Details</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerrepositorybase.ui" line="41"/>
        <source>Name:</source>
        <translation type="unfinished">Name:</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerrepositorybase.ui" line="67"/>
        <source>Enter a name for the repository</source>
        <translation type="unfinished">Name des Repositories eingeben</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerrepositorybase.ui" line="74"/>
        <source>URL:</source>
        <translation type="unfinished">URL:</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerrepositorybase.ui" line="84"/>
        <source>Enter the repository URL, beginning with &quot;http://&quot;</source>
        <translation type="unfinished">Repository-URL beginnend mit &quot;http://&quot; eingeben</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerrepositorybase.ui" line="106"/>
        <source>Enable or disable the repository (disabled repositories will be omitted)</source>
        <translation type="unfinished">Das Repository ein- oder abschalten (abgeschaltete Repositories werden nicht angesprochen)</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerrepositorybase.ui" line="109"/>
        <source>Enabled</source>
        <translation type="unfinished">Eingeschaltet</translation>
    </message>
</context>
<context>
    <name>QgsPluginManager</name>
    <message>
        <location filename="../src/app/qgspluginmanager.cpp" line="209"/>
        <source>No Plugins</source>
        <translation>Keine Plugins</translation>
    </message>
    <message>
        <location filename="../src/app/qgspluginmanager.cpp" line="209"/>
        <source>No QGIS plugins found in </source>
        <translation>Keine QGIS-Plugins gefunden in </translation>
    </message>
    <message>
        <location filename="../src/app/qgspluginmanager.cpp" line="85"/>
        <source>&amp;Select All</source>
        <translation type="unfinished">&amp;Alle selektieren</translation>
    </message>
    <message>
        <location filename="../src/app/qgspluginmanager.cpp" line="86"/>
        <source>&amp;Clear All</source>
        <translation type="unfinished">Alle &amp;deselektieren</translation>
    </message>
    <message>
        <location filename="../src/app/qgspluginmanager.cpp" line="154"/>
        <source>[ incompatible ]</source>
        <translation type="unfinished">[ inkompatibel ]</translation>
    </message>
</context>
<context>
    <name>QgsPluginManagerBase</name>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="16"/>
        <source>QGIS Plugin Manager</source>
        <translation>QGIS Plugin Manager</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="25"/>
        <source>To enable / disable a plugin, click its checkbox or description</source>
        <translation type="unfinished">Checkbox oder Beschreibung anklicken, um ein Plugin zu (de-)aktivieren</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="45"/>
        <source>&amp;Filter</source>
        <translation>&amp;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="58"/>
        <source>Plugin Directory:</source>
        <translation type="unfinished">Plugin-Verzeichnis:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="71"/>
        <source>Directory</source>
        <translation type="unfinished">Verzeichnis</translation>
    </message>
</context>
<context>
    <name>QgsPointDialog</name>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="497"/>
        <source>Zoom In</source>
        <translation>Hineinzoomen</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="496"/>
        <source>z</source>
        <translation>z</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="502"/>
        <source>Zoom Out</source>
        <translation>Hinauszoomen</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="501"/>
        <source>Z</source>
        <translation>Z</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="505"/>
        <source>Zoom To Layer</source>
        <translation>Auf den Layer zoomen</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="507"/>
        <source>Zoom to Layer</source>
        <translation>Auf den Layer zoomen</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="510"/>
        <source>Pan Map</source>
        <translation>Karte verschieben</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="511"/>
        <source>Pan the map</source>
        <translation>Karte verschieben</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="514"/>
        <source>Add Point</source>
        <translation>Addiere Punkt</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="515"/>
        <source>.</source>
        <translation>.</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="516"/>
        <source>Capture Points</source>
        <translation>Punkt digitalisieren</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="519"/>
        <source>Delete Point</source>
        <translation>Lösche Punkt</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="520"/>
        <source>Delete Selected</source>
        <translation>Ausgewahl gelöscht.</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="573"/>
        <source>Linear</source>
        <translation>Linear</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="574"/>
        <source>Helmert</source>
        <translation>Helmert</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="206"/>
        <source>Choose a name for the world file</source>
        <translation>Bitte einen Namen für das World-File eingeben.</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="273"/>
        <source>Warning</source>
        <translation>Warnung</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="279"/>
        <source>&lt;p&gt;A Helmert transform requires modifications in the raster layer.&lt;/p&gt;&lt;p&gt;The modified raster will be saved in a new file and a world file will be generated for this new file instead.&lt;/p&gt;&lt;p&gt;Are you sure that this is what you want?&lt;/p&gt;</source>
        <translation>&lt;p&gt;Eine Helmert-Transformation ändert den Rasterlayer.&lt;/p&gt;&lt;p&gt;Stattdessen wird die veränderte Rasterdatei in einer neuen Datei mit einem dazu passenden Worldfile gespeichert.&lt;/p&gt;&lt;p&gt;Sind Sie sicher, dass Sie das wollen?&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="288"/>
        <source>Affine</source>
        <translation>Affin</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="298"/>
        <source>Not implemented!</source>
        <translation>Nicht implementiert!</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="293"/>
        <source>&lt;p&gt;An affine transform requires changing the original raster file. This is not yet supported.&lt;/p&gt;</source>
        <translation>&lt;p&gt;Eine Affin-Transformation wird die Original-Rasterdatei verändern. Dies ist noch nicht implementiert.&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="300"/>
        <source>&lt;p&gt;The </source>
        <translation>&lt;p&gt;Die </translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="301"/>
        <source> transform is not yet supported.&lt;/p&gt;</source>
        <translation>Transformation wird noch nicht unterstützt.&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="339"/>
        <source>Error</source>
        <translation>Fehler</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="340"/>
        <source>Could not write to </source>
        <translation>Kann nicht </translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="280"/>
        <source>Currently all modified files will be written in TIFF format.</source>
        <translation>Derzeit werden alle modifizierten Dateien im TIFF-Format geschrieben.</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="224"/>
        <source>-modified</source>
        <comment>Georeferencer:QgsPointDialog.cpp - used to modify a user given file name</comment>
        <translation type="unfinished">-modifiziert</translation>
    </message>
</context>
<context>
    <name>QgsPointDialogBase</name>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="65"/>
        <source>Transform type:</source>
        <translation>Transformationstyp:</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="178"/>
        <source>Zoom in</source>
        <translation>Hineinzoomen</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="200"/>
        <source>Zoom out</source>
        <translation>Herauszoomen</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="222"/>
        <source>Zoom to the raster extents</source>
        <translation>Auf die Rasterausdehnung zoomen</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="244"/>
        <source>Pan</source>
        <translation>Verschieben</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="105"/>
        <source>Add points</source>
        <translation>Punkte hinzufügen</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="130"/>
        <source>Delete points</source>
        <translation>Punkte löschen</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="52"/>
        <source>World file:</source>
        <translation>World file:</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="38"/>
        <source>...</source>
        <translation type="unfinished">...</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="45"/>
        <source>Modified raster:</source>
        <translation>Raster modifizieren:</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="13"/>
        <source>Reference points</source>
        <translation type="unfinished">Referenzpunkte</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="75"/>
        <source>Create</source>
        <translation>Erstellen</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="282"/>
        <source>Create and load layer</source>
        <translation>Erstellen und Layer laden</translation>
    </message>
</context>
<context>
    <name>QgsPostgresProvider</name>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="125"/>
        <source>Unable to access relation</source>
        <translation>Auf die Relation kann nicht zugegriffen werden</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="106"/>
        <source>Unable to access the </source>
        <translation>Fehler beim Zugriff auf die</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="128"/>
        <source> relation.
The error message from the database was:
</source>
        <translation>Relation. Die Fehlermeldung der Datenbank war:</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="920"/>
        <source>No suitable key column in table</source>
        <translation>Keine passende Schlüsselspalte in der Tabelle.</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="924"/>
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
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="964"/>
        <source>The unique index on column</source>
        <translation>Der eindeutige Index der Spalte</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="966"/>
        <source>is unsuitable because Qgis does not currently support non-int4 type columns as a key into the table.
</source>
        <translation>kann nicht benutzt werden, da QGIS derzeit nur Spalten vom Typ int4 als Schlüssel einer Tabelle akzeptiert.</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="987"/>
        <source>and </source>
        <translation>und </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="993"/>
        <source>The unique index based on columns </source>
        <translation>Der eindeutige Index basierend auf den Spalten </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="995"/>
        <source> is unsuitable because Qgis does not currently support multiple columns as a key into the table.
</source>
        <translation> ist unbrauchbar, da QGIS derzeit nicht mehrere Spalten als Schlüssel in einer Tabelle unterstützt.</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1037"/>
        <source>Unable to find a key column</source>
        <translation>Kann die Schlüsselspalte nicht finden.</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1117"/>
        <source> derives from </source>
        <translation> kommt von </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1121"/>
        <source>and is suitable.</source>
        <translation>und is passend.</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1125"/>
        <source>and is not suitable </source>
        <translation>und ist nicht passend.</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1126"/>
        <source>type is </source>
        <translation>Typ ist </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1128"/>
        <source> and has a suitable constraint)</source>
        <translation> und hat einen passenden Constraint).</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1130"/>
        <source> and does not have a suitable constraint)</source>
        <translation> und hat keinen passenden Constraint).</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1228"/>
        <source>The view you selected has the following columns, none of which satisfy the above conditions:</source>
        <translation>Der ausgewählte View hat folgende Spalten von denen keine die obigen Bedingungen erfüllt:</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1234"/>
        <source>Qgis requires that the view has a column that can be used as a unique key. Such a column should be derived from a table column of type int4 and be a primary key, have a unique constraint on it, or be a PostgreSQL oid column. To improve performance the column should also be indexed.
</source>
        <translation>QGIS benötigt bei Views eine Spalte, die als eindeutige Schlüsselspalte verwendet werden kann. Eine solche Spalte (meist vom Typ int4) muß im Datensatz vorhanden sein und als primärer Schlüssel definiert sein. Ferner sollte ein eindeutiger Constraint definiert sein. Ideal ist die von PostgreSQL unterstützte Spalte oid. Um die Geschwindigkeit zu erhöhen, sollte die Spalte auch indiziert sein.</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1235"/>
        <source>The view </source>
        <translation>Das View </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1236"/>
        <source>has no column suitable for use as a unique key.
</source>
        <translation>hat keine Spalte, die sich als eindeutiger Schlüssel eignet.</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1237"/>
        <source>No suitable key column in view</source>
        <translation>Keine passende Schlüsselspalte im View</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2553"/>
        <source>Unknown geometry type</source>
        <translation>Unbekannter Geometrietyp.</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2554"/>
        <source>Column </source>
        <translation>Spalte</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2564"/>
        <source> in </source>
        <translation> in </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2556"/>
        <source> has a geometry type of </source>
        <translation> hat einen Geometrietyp von </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2556"/>
        <source>, which Qgis does not currently support.</source>
        <translation>, den QGIS derzeit nicht unterstützt.</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2565"/>
        <source>. The database communication log was:
</source>
        <translation>. Die Datenbanklogdatei sagt folgendes:</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2566"/>
        <source>Unable to get feature type and srid</source>
        <translation>Kann den Fearture-Typ und die SRID nicht ermitteln.</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1214"/>
        <source>Note: </source>
        <translation>Bemerkung: </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1216"/>
        <source>initially appeared suitable but does not contain unique data, so is not suitable.
</source>
        <translation>anfänglich schien der Layer geeignet, allerdings enthält er keine eindeutigen Daten, insofern nicht geeignet.</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="126"/>
        <source>Unable to determine table access privileges for the </source>
        <translation type="unfinished">Konnte Tabellenzugriffrechte für die Tabelle </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1888"/>
        <source>Error while adding features</source>
        <translation type="unfinished">Fehler beim Hinzufügen von Objekten</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1928"/>
        <source>Error while deleting features</source>
        <translation type="unfinished">Fehler beim Löschen von Objekten</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1966"/>
        <source>Error while adding attributes</source>
        <translation type="unfinished">Fehler beim Hinzufügen von Attributen</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2011"/>
        <source>Error while deleting attributes</source>
        <translation type="unfinished">Fehler beim Löschen von Attributen</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2081"/>
        <source>Error while changing attributes</source>
        <translation type="unfinished">Fehler beim Ändern von Attributen</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2160"/>
        <source>Error while changing geometry values</source>
        <translation type="unfinished">Fehler beim Ändern von Geometrien</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2563"/>
        <source>Qgis was unable to determine the type and srid of column </source>
        <translation type="unfinished">QGIS konnte Typ und SRID der Spalte </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.h" line="482"/>
        <source>unexpected PostgreSQL error</source>
        <translation type="unfinished">Nicht erwarteter PostgeSQL-Fehler</translation>
    </message>
</context>
<context>
    <name>QgsPostgresProvider::Conn</name>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="323"/>
        <source>No GEOS Support!</source>
        <translation type="unfinished">Keine GEOS Unterstützung!</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="327"/>
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
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="13"/>
        <source>Project Properties</source>
        <translation>Projekteigenschaften</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="157"/>
        <source>Meters</source>
        <translation>Meter</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="167"/>
        <source>Feet</source>
        <translation>Fuss</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="174"/>
        <source>Decimal degrees</source>
        <translation>Dezimal Grad</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="54"/>
        <source>Default project title</source>
        <translation>Default Projekttitel</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="32"/>
        <source>General</source>
        <translation>Allgemein</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="196"/>
        <source>Automatic</source>
        <translation>Automatisch</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="190"/>
        <source>Automatically sets the number of decimal places in the mouse position display</source>
        <translation>Setzt automatisch die Anzahl Dezimalstellen in der Mauspositionsanzeige</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="193"/>
        <source>The number of decimal places that are used when displaying the mouse position is automatically set to be enough so that moving the mouse by one pixel gives a change in the position display</source>
        <translation>Die Anzahl Dezimalstellen, die beim Anzeigen der Mausposition benutzt werden, wird automatisch so gesetzt, dass eine Mausbewegung um einen Pixel zu einer Änderung in der Positionsanzeige führt</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="212"/>
        <source>Manual</source>
        <translation>Hilfe</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="209"/>
        <source>Sets the number of decimal places to use for the mouse position display</source>
        <translation>Setzt die Anzahl Dezimalstellen für die Mauspositionsanzeige</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="222"/>
        <source>The number of decimal places for the manual option</source>
        <translation>Setzt die Anzahl Dezimalstellen für die manuelle Option</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="235"/>
        <source>decimal places</source>
        <translation>Dezimalstellen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="184"/>
        <source>Precision</source>
        <translation>Genauigkeit</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="251"/>
        <source>Digitizing</source>
        <translation>Digitalisierung</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="51"/>
        <source>Descriptive project name</source>
        <translation>Beschreibender Projektname</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="257"/>
        <source>Enable topological editing</source>
        <translation>Ermögliche topologisches Editieren</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="271"/>
        <source>Snapping options...</source>
        <translation>Snapping-Optionen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="264"/>
        <source>Avoid intersections of new polygons</source>
        <translation>Vermeide Überschneidung neuer Polygone</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="38"/>
        <source>Title and colors</source>
        <translation type="unfinished">Titel und Farben</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="44"/>
        <source>Project title</source>
        <translation type="unfinished">Projekttitel</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="61"/>
        <source>Selection color</source>
        <translation type="unfinished">Selektionsfarbe</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="100"/>
        <source>Background color</source>
        <translation type="unfinished">Hintergrundfarbe</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="139"/>
        <source>Map units</source>
        <translation type="unfinished">Karteneinheiten</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="282"/>
        <source>Coordinate Reference System (CRS)</source>
        <translation type="unfinished">Benutzerkoordinatenreferenzsystem (KBS)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="300"/>
        <source>Enable &apos;on the fly&apos; CRS transformation</source>
        <translation type="unfinished">&apos;On-The-Fly&apos;-KBS-Transformation aktivieren</translation>
    </message>
</context>
<context>
    <name>QgsProjectionSelector</name>
    <message>
        <location filename="../src/gui/qgsprojectionselector.cpp" line="491"/>
        <source>User Defined Coordinate Systems</source>
        <translation type="unfinished">Benutzerdefiniertes Koordinatensystem</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsprojectionselector.cpp" line="568"/>
        <source>Geographic Coordinate Systems</source>
        <translation type="unfinished">Geografisches Koordinatensystem</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsprojectionselector.cpp" line="577"/>
        <source>Projected Coordinate Systems</source>
        <translation type="unfinished">Projeziertes Koordinatensystem</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsprojectionselector.cpp" line="932"/>
        <source>Resource Location Error</source>
        <translation type="unfinished">Resource nicht gefunden</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsprojectionselector.cpp" line="935"/>
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
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="111"/>
        <source>Search</source>
        <translation>Suchen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="155"/>
        <source>Find</source>
        <translation>Finden</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="117"/>
        <source>EPSG ID</source>
        <translation>EPSG ID</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="133"/>
        <source>Name</source>
        <translation>Name</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="19"/>
        <source>Coordinate Reference System Selector</source>
        <translation type="unfinished">Koordinatenbezugssystem-Auswahl</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="53"/>
        <source>Coordinate Reference System</source>
        <translation type="unfinished">Koordinatensystem</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="58"/>
        <source>EPSG</source>
        <translation type="unfinished">EPSG</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="63"/>
        <source>ID</source>
        <translation type="unfinished">ID</translation>
    </message>
</context>
<context>
    <name>QgsPythonDialog</name>
    <message>
        <location filename="../src/ui/qgspythondialog.ui" line="13"/>
        <source>Python console</source>
        <translation type="unfinished">Python-Konsole</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspythondialog.ui" line="58"/>
        <source>&gt;&gt;&gt;</source>
        <translation>&gt;&gt;&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspythondialog.ui" line="33"/>
        <source>To access Quantum GIS environment from this python console use object from global scope which is an instance of QgisInterface class.&lt;br&gt;Usage e.g.: iface.zoomFull()</source>
        <translation>Um die Quantum GIS Umgebung von dieser Python Konsole aus zu erreichen, benutzen Sie Objekte des global scope, die eine Instanz der QgisInterface Klasse sind.&lt;br&gt;Benutzung z.B.: iface.zoomFull()</translation>
    </message>
</context>
<context>
    <name>QgsQuickPrint</name>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="832"/>
        <source> km</source>
        <translation type="unfinished"> km</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="837"/>
        <source> mm</source>
        <translation type="unfinished"> mm</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="842"/>
        <source> cm</source>
        <translation type="unfinished"> cm</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="846"/>
        <source> m</source>
        <translation type="unfinished"> m</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="851"/>
        <source> miles</source>
        <translation type="unfinished"> Meilen</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="856"/>
        <source> mile</source>
        <translation type="unfinished"> Meile</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="861"/>
        <source> inches</source>
        <translation type="unfinished"> Inches</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="866"/>
        <source> foot</source>
        <translation type="unfinished">Fuss</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="870"/>
        <source> feet</source>
        <translation type="unfinished">Fuß</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="875"/>
        <source> degree</source>
        <translation type="unfinished">Grad</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="877"/>
        <source> degrees</source>
        <translation type="unfinished">Grad</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="880"/>
        <source> unknown</source>
        <translation type="unfinished">unbekannt</translation>
    </message>
</context>
<context>
    <name>QgsRasterLayer</name>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3344"/>
        <source>Not Set</source>
        <translation>Nicht gesetzt.</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2424"/>
        <source>Driver:</source>
        <translation>Treiber:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2501"/>
        <source>Dimensions:</source>
        <translation>Dimensionen:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2504"/>
        <source>X: </source>
        <translation>X:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2505"/>
        <source> Y: </source>
        <translation>Y:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2505"/>
        <source> Bands: </source>
        <translation>Kanäle: </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2620"/>
        <source>Origin:</source>
        <translation>Ursprung:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2629"/>
        <source>Pixel Size:</source>
        <translation>Pixelgröße:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2571"/>
        <source>Pyramid overviews:</source>
        <translation>Pyramiden Überblicke:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2647"/>
        <source>Band</source>
        <translation>Kanal</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2654"/>
        <source>Band No</source>
        <translation>Kanal Nr</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2666"/>
        <source>No Stats</source>
        <translation>Keine Statistik</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2669"/>
        <source>No stats collected yet</source>
        <translation>Noch keine Statistik gesammelt</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2679"/>
        <source>Min Val</source>
        <translation>Minimalwert</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2687"/>
        <source>Max Val</source>
        <translation>Maximalwert</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2695"/>
        <source>Range</source>
        <translation>Bereich</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2703"/>
        <source>Mean</source>
        <translation>Durchschnitt</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2711"/>
        <source>Sum of squares</source>
        <translation>Summe der Quadrate</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2719"/>
        <source>Standard Deviation</source>
        <translation>Standardverteilung</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2727"/>
        <source>Sum of all cells</source>
        <translation>Summe aller Zellen</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2735"/>
        <source>Cell Count</source>
        <translation>Zellenanzahl</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2527"/>
        <source>Data Type:</source>
        <translation>Datentyp:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2533"/>
        <source>GDT_Byte - Eight bit unsigned integer</source>
        <translation>GDT_Byte - Eight bit unsigned integer</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2536"/>
        <source>GDT_UInt16 - Sixteen bit unsigned integer </source>
        <translation>GDT_UInt16 - Sixteen bit unsigned integer</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2539"/>
        <source>GDT_Int16 - Sixteen bit signed integer </source>
        <translation>GDT_Int16 - Sixteen bit signed integer</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2542"/>
        <source>GDT_UInt32 - Thirty two bit unsigned integer </source>
        <translation>GDT_UInt32 - Thirty two bit unsigned integer</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2545"/>
        <source>GDT_Int32 - Thirty two bit signed integer </source>
        <translation>GDT_Int32 - Thirty two bit signed integer</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2548"/>
        <source>GDT_Float32 - Thirty two bit floating point </source>
        <translation>GDT_Float32 - Thirty two bit floating point</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2551"/>
        <source>GDT_Float64 - Sixty four bit floating point </source>
        <translation>GDT_Float64 - Sixty four bit floating point</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2554"/>
        <source>GDT_CInt16 - Complex Int16 </source>
        <translation>GDT_CInt16 - Complex Int16</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2557"/>
        <source>GDT_CInt32 - Complex Int32 </source>
        <translation>GDT_CInt32 - Complex Int32</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2560"/>
        <source>GDT_CFloat32 - Complex Float32 </source>
        <translation>GDT_CFloat32 - Complex Float32</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2563"/>
        <source>GDT_CFloat64 - Complex Float64 </source>
        <translation>GDT_CFloat64 - Complex Float64</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2566"/>
        <source>Could not determine raster data type.</source>
        <translation>Konnte Rasterdatentyp nicht erkennen.</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="1045"/>
        <source>Average Magphase</source>
        <translation>Durchschnittliche Magphase</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="1050"/>
        <source>Average</source>
        <translation>Durchschnitt</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2592"/>
        <source>Layer Spatial Reference System: </source>
        <translation>Referenzsystem des Layers:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="1792"/>
        <source>out of extent</source>
        <translation>ausserhalb der Ausdehnung</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="1830"/>
        <source>null (no data)</source>
        <translation>Null (keine Daten)</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2449"/>
        <source>Dataset Description</source>
        <translation>Datensatzbeschreibung</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2512"/>
        <source>No Data Value</source>
        <translation>NODATA Wert</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="380"/>
        <source>and all other files</source>
        <translation>und alle anderen Dateien</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2521"/>
        <source>NoDataValue not set</source>
        <translation>NoDataValue nicht gesetzt</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2470"/>
        <source>Band %1</source>
        <translation>Band %1</translation>
    </message>
</context>
<context>
    <name>QgsRasterLayerProperties</name>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1001"/>
        <source>Grayscale</source>
        <translation>Graustufen</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2373"/>
        <source>Pseudocolor</source>
        <translation>Pseudofarben</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2373"/>
        <source>Freak Out</source>
        <translation>Ausgeflippt</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="810"/>
        <source>Columns: </source>
        <translation>Spalten: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="811"/>
        <source>Rows: </source>
        <translation>Zeilen: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="812"/>
        <source>No-Data Value: </source>
        <translation>NODATA-Wert: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="812"/>
        <source>n/a</source>
        <translation>n/a</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2734"/>
        <source>Write access denied</source>
        <translation type="unfinished">Schreibzugriff verboten</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2734"/>
        <source>Write access denied. Adjust the file permissions and try again.

</source>
        <translation type="unfinished">Schreibzugriff verboten. Dateirechte ändern und erneut versuchen.
</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1597"/>
        <source>Building pyramids failed.</source>
        <translation type="unfinished">Erstellung von Pyramiden fehlgeschlagen.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1598"/>
        <source>Building pyramid overviews is not supported on this type of raster.</source>
        <translation type="unfinished">Für diese Art von Raster können keine Pyramiden erstellt werden. </translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2912"/>
        <source>No Stretch</source>
        <translation>Kein Strecken</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2917"/>
        <source>Stretch To MinMax</source>
        <translation>Strecke auf MinMax</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2922"/>
        <source>Stretch And Clip To MinMax</source>
        <translation>Strecken und Zuschneiden auf MinMax</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2927"/>
        <source>Clip To MinMax</source>
        <translation>Zuschneiden auf MinMax</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2791"/>
        <source>Discrete</source>
        <translation>Diskret</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2583"/>
        <source>Equal interval</source>
        <translation>Gleiches Interval</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2606"/>
        <source>Quantiles</source>
        <translation type="unfinished">Quantile</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="306"/>
        <source>Description</source>
        <translation>Beschreibung</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="307"/>
        <source>Large resolution raster layers can slow navigation in QGIS.</source>
        <translation>Hochaufgelöste Raster können das Navigieren in QGIS verlangsamen.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="308"/>
        <source>By creating lower resolution copies of the data (pyramids) performance can be considerably improved as QGIS selects the most suitable resolution to use depending on the level of zoom.</source>
        <translation>Durch das Erstellen geringer aufgelöster Kopien der Daten (Pyramiden), kann die Darstellung beschleunigt werden, da QGIS die optimale Auflösung entsprechend der gewählten Zoomeinstellung aussucht</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="309"/>
        <source>You must have write access in the directory where the original data is stored to build pyramids.</source>
        <translation>Sie brauchen Schreibrecht in dem Ordner mit den Originaldaten, um Pyramiden zu erstellen.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1826"/>
        <source>Red</source>
        <translation>Rot</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1826"/>
        <source>Green</source>
        <translation>Grün</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1826"/>
        <source>Blue</source>
        <translation>Blau</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1843"/>
        <source>Percent Transparent</source>
        <translation>Prozent Transparenz</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1839"/>
        <source>Gray</source>
        <translation type="unfinished">Grau</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1843"/>
        <source>Indexed Value</source>
        <translation>Indizierter Wert</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2910"/>
        <source>User Defined</source>
        <translation>Benutzerdefiniert</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="804"/>
        <source>No-Data Value: Not Set</source>
        <translation>No-Data Wert: Nicht gesetzt</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2680"/>
        <source>Save file</source>
        <translation>Datei speichern</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2760"/>
        <source>Textfile (*.txt)</source>
        <translation>Textdatei (*.txt)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1823"/>
        <source>QGIS Generated Transparent Pixel Value Export File</source>
        <translation type="unfinished">QGIS-erzeugte Export-Datei für transparente Pixelwerte</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2760"/>
        <source>Open file</source>
        <translation>Datei öffnen</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2828"/>
        <source>Import Error</source>
        <translation>Importfehler</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2828"/>
        <source>The following lines contained errors

</source>
        <translation>Die folgenden Zeilen enthalten Fehler</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2833"/>
        <source>Read access denied</source>
        <translation>Lesezugriff verweigert</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2833"/>
        <source>Read access denied. Adjust the file permissions and try again.

</source>
        <translation>Lesezugriff verweigert. Passe Dateirechte an und versuche es erneut.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2373"/>
        <source>Color Ramp</source>
        <translation>Farbanstieg</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="56"/>
        <source>Not Set</source>
        <translation type="unfinished">Nicht gesetzt.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="3076"/>
        <source>Default Style</source>
        <translation type="unfinished">Standardstil</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="3180"/>
        <source>QGIS Layer Style File (*.qml)</source>
        <translation type="unfinished">QGIS Layerstil Datei (*.qml)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="3206"/>
        <source>QGIS</source>
        <translation>QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="3207"/>
        <source>Unknown style format: </source>
        <translation type="unfinished">Unbekanntes Stilformat: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="310"/>
        <source>Please note that building internal pyramids may alter the original data file and once created they cannot be removed!</source>
        <translation type="unfinished">Bitte beachten Sie, dass der Aufbau von internen Pyramiden die Originaldatei ändern kann und einmal angelegt nicht gelöscht werden kann.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="311"/>
        <source>Please note that building internal pyramids could corrupt your image - always make a backup of your data first!</source>
        <translation type="unfinished">Bitte beachten sie, dass der Aufbau von internen Pyramiden ihr Bild beschädigen kann - bitte sichern Sie Ihre Daten zuvor.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2905"/>
        <source>Default</source>
        <translation type="unfinished">Voreinstellung</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1583"/>
        <source>The file was not writeable. Some formats do not support pyramid overviews. Consult the GDAL documentation if in doubt.</source>
        <translation type="unfinished">Die Datei war nicht beschreibbar. Einige Formate unterstützen Übersichtspyramiden nicht.  Gucken Sie im Zweifel in die GDAL-Dokumentation.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="3199"/>
        <source>Saved Style</source>
        <translation type="unfinished">Gespeicherter Stil</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1719"/>
        <source>Colormap</source>
        <translation type="unfinished">Farbkarte</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2787"/>
        <source>Linear</source>
        <translation type="unfinished">Linear</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2795"/>
        <source>Exact</source>
        <translation type="unfinished">Genau</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2675"/>
        <source>Custom color map entry</source>
        <translation type="unfinished">Benutzerdefinierte Farbabbildungseintrag</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2692"/>
        <source>QGIS Generated Color Map Export File</source>
        <translation type="unfinished">QGIS-Farbabbildungsexportdatei</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2750"/>
        <source>Load Color Map</source>
        <translation type="unfinished">QGIS-Farbabbildung laden</translation>
    </message>
    <message numerus="yes">
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2750"/>
        <source>The color map for Band %n failed to load</source>
        <translation type="unfinished">
            <numerusform>
        
        
        </numerusform>
        </translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1593"/>
        <source>Building internal pyramid overviews is not supported on raster layers with JPEG compression.</source>
        <translation type="unfinished">Interne Pyramiden-Übersichten für Rasterlayer mit JPEG-Kompression nicht unterstützt</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="437"/>
        <source>Note: Minimum Maximum values are estimates or user defined</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="441"/>
        <source>Note: Minimum Maximum values are actual values computed from the band(s)</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsRasterLayerPropertiesBase</name>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="13"/>
        <source>Raster Layer Properties</source>
        <translation>Rasterlayereigenschaften</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1327"/>
        <source>General</source>
        <translation>Allgemein</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1397"/>
        <source>No Data:</source>
        <translation>Keine Daten:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="34"/>
        <source>Symbology</source>
        <translation>Darstellung</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="930"/>
        <source>&lt;p align=&quot;right&quot;&gt;Full&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;right&quot;&gt;Voll&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="884"/>
        <source>None</source>
        <translation>Keine</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1631"/>
        <source>Metadata</source>
        <translation>Metadaten</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1644"/>
        <source>Pyramids</source>
        <translation>Pyramiden</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1718"/>
        <source>Average</source>
        <translation>Durchschnitt</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1723"/>
        <source>Nearest Neighbour</source>
        <translation>Nächster Nachbar</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1512"/>
        <source>Thumbnail</source>
        <translation>Miniaturbild</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1371"/>
        <source>Columns:</source>
        <translation>Spalten:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1384"/>
        <source>Rows:</source>
        <translation>Zeilen:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1418"/>
        <source>Maximum scale at which this layer will be displayed. </source>
        <translation>Maximaler Massstab bis zu dem dieser Layer angezeigt wird.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1441"/>
        <source>Minimum scale at which this layer will be displayed. </source>
        <translation>Minimaler Massstab ab dem dieser Layer angezeigt wird.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1746"/>
        <source>Histogram</source>
        <translation>Histogramm</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1812"/>
        <source>Options</source>
        <translation>Optionen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1783"/>
        <source>Chart Type</source>
        <translation>Diagrammtyp</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1894"/>
        <source>Refresh</source>
        <translation>Erneuern</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="635"/>
        <source>Max</source>
        <translation type="unfinished">Max</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="597"/>
        <source>Min</source>
        <translation type="unfinished">Min</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="910"/>
        <source> 00%</source>
        <translation>00%</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="40"/>
        <source>Render as</source>
        <translation>Zeige als</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1247"/>
        <source>...</source>
        <translation type="unfinished">...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1134"/>
        <source>Colormap</source>
        <translation>Farbkarte</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1170"/>
        <source>Delete entry</source>
        <translation>Lösche Eintrag</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1316"/>
        <source>Classify</source>
        <translation>Klassifiziere</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1267"/>
        <source>1</source>
        <translation type="unfinished">1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1272"/>
        <source>2</source>
        <translation type="unfinished">2</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="46"/>
        <source>Single band gray</source>
        <translation>Einfaches Grauband </translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="66"/>
        <source>Three band color</source>
        <translation>Dreibandfarbe</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="118"/>
        <source>RGB mode band selection and scaling</source>
        <translation>RGB-Band-Auswahl und Skalierung</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="124"/>
        <source>Red band</source>
        <translation>Rotes Band</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="156"/>
        <source>Green band</source>
        <translation>Grünes Band</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="188"/>
        <source>Blue band</source>
        <translation>Blaues Band</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="584"/>
        <source>Custom min / max values</source>
        <translation type="unfinished">Benutzerextrema</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="255"/>
        <source>Red min</source>
        <translation>Rot-Minimum</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="306"/>
        <source>Red max</source>
        <translation>Rot-Maximum</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="344"/>
        <source>Green min</source>
        <translation>Grün-Minimum</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="382"/>
        <source>Green max</source>
        <translation>Grün-Maximum</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="420"/>
        <source>Blue min</source>
        <translation>Blau-Minimum</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="458"/>
        <source>Blue max</source>
        <translation>Blau-Maximum</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="526"/>
        <source>Single band properties</source>
        <translation>Einfachbandeigenschaften</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="538"/>
        <source>Gray band</source>
        <translation>Grauband</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="561"/>
        <source>Color map</source>
        <translation>Farbabbildung</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="92"/>
        <source>Invert color map</source>
        <translation>Farbabbildung invertieren</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="667"/>
        <source>Use standard deviation</source>
        <translation type="unfinished">Standardabweichung nutzen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="721"/>
        <source>Load min / max values from band</source>
        <translation>Extrema des Bandes laden</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="727"/>
        <source>Estimate (faster)</source>
        <translation>Schätzung (schneller) </translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="747"/>
        <source>Actual (slower)</source>
        <translation>Genau (langsamer)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="767"/>
        <source>Load</source>
        <translation>Laden</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="777"/>
        <source>Contrast enhancement</source>
        <translation>Konstrastverbesserung</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="795"/>
        <source>Current</source>
        <translation>Aktuell</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="818"/>
        <source>Save current contrast enhancement algorithm as default. This setting will be persistent between QGIS sessions.</source>
        <translation>Aktuelle Konstrastverbessungsalgorithmus voreinstellen. Diese Einstellung bleibt über verschiedene QGIS-Sitzungen erhalten.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="821"/>
        <source>Saves current contrast enhancement algorithm as a default. This setting will be persistent between QGIS sessions.</source>
        <translation>Aktuelle Konstrastverbessungsalgorithmus voreinstellen. Diese Einstellung bleibt über verschiedene QGIS-Sitzungen erhalten.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="835"/>
        <source>Default</source>
        <translation>Voreinstellung</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="842"/>
        <source>TextLabel</source>
        <translation>Textbeschriftung</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="853"/>
        <source>Transparency</source>
        <translation>Transparenz</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="859"/>
        <source>Global transparency</source>
        <translation>Globable Transparenz</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="940"/>
        <source>No data value</source>
        <translation>&apos;Ohne Wert&apos;-Wert</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="949"/>
        <source>Reset no data value</source>
        <translation>&apos;Ohne Wert&apos; zurücksetzen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="959"/>
        <source>Custom transparency options</source>
        <translation>Benutzertransparenzeinstellungen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="965"/>
        <source>Transparency band</source>
        <translation>Tranzparenzband</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="991"/>
        <source>Transparent pixel list</source>
        <translation>Transparentspixelliste</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1028"/>
        <source>Add values manually</source>
        <translation>Werte manuell ergänzen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1045"/>
        <source>Add Values from display</source>
        <translation>Werte aus Anzeige ergänzen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1059"/>
        <source>Remove selected row</source>
        <translation>Gewählte Zeile löschen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1073"/>
        <source>Default values</source>
        <translation>Wertvoreinstellungen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1100"/>
        <source>Import from file</source>
        <translation>Datei importieren</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1114"/>
        <source>Export to file</source>
        <translation>Exportieren in Datei</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1286"/>
        <source>Number of entries</source>
        <translation>Eintragsanzahl</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1140"/>
        <source>Color interpolation</source>
        <translation>Farbinterpolation</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1306"/>
        <source>Classification mode</source>
        <translation>Klassifikationsmodus</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1406"/>
        <source>Scale dependent visibility</source>
        <translation>Maßstabsabhänge Sichtbarkeit</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1431"/>
        <source>Maximum</source>
        <translation>Maximum</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1454"/>
        <source>Minimum</source>
        <translation>Minimum</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1346"/>
        <source>Layer source</source>
        <translation>Layerquelle</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1333"/>
        <source>Display name</source>
        <translation type="unfinished">Anzeigename</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1657"/>
        <source>Pyramid resolutions</source>
        <translation>Pyramidenauflösungen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1707"/>
        <source>Resampling method</source>
        <translation>Resampling-Methode</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1738"/>
        <source>Build pyramids</source>
        <translation>Pyramiden erzeugen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1792"/>
        <source>Line graph</source>
        <translation>Kurvendiagram</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1802"/>
        <source>Bar chart</source>
        <translation>Balkendiagram</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1818"/>
        <source>Column count</source>
        <translation>Spaltenanzahl</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1841"/>
        <source>Out of range OK?</source>
        <translation>Bereichsüberschreitung erlaubt</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1848"/>
        <source>Allow approximation</source>
        <translation>Approximation erlauben</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1905"/>
        <source>Restore Default Style</source>
        <translation type="unfinished">Standardstil wiederherstellen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1912"/>
        <source>Save As Default</source>
        <translation type="unfinished">Als Standard speichern</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1919"/>
        <source>Load Style ...</source>
        <translation type="unfinished">Stil laden...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1926"/>
        <source>Save Style ...</source>
        <translation type="unfinished">Stil speichern...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="707"/>
        <source>Note:</source>
        <translation type="unfinished">Hinweis:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="233"/>
        <source>Default R:1 G:2 B:3</source>
        <translation type="unfinished">Vorgabe R:1 G:2 B:3</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1650"/>
        <source>Notes</source>
        <translation type="unfinished">Anmerkungen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1700"/>
        <source>Build pyramids internally if possible</source>
        <translation type="unfinished">Wenn möglich interne Pyramiden erzeugen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1163"/>
        <source>Add entry</source>
        <translation type="unfinished">Eintrag hinzufügen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1177"/>
        <source>Sort</source>
        <translation type="unfinished">Sortieren</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1200"/>
        <source>Load color map from band</source>
        <translation type="unfinished">Farbabbildung aus Band laden</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1230"/>
        <source>Load color map from file</source>
        <translation type="unfinished">Farbabbildung aus Datei laden</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1244"/>
        <source>Export color map to file</source>
        <translation type="unfinished">Farbabbildung in Datei exportieren</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1280"/>
        <source>Generate new color map</source>
        <translation type="unfinished">Neuen Farbabbildung generieren</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1467"/>
        <source>Coordinate reference system</source>
        <translation type="unfinished">Koordinatenbezugssystem</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1476"/>
        <source>Change ...</source>
        <translation type="unfinished">Ändern ...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1549"/>
        <source>Legend</source>
        <translation type="unfinished">Legende</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1586"/>
        <source>Palette</source>
        <translation type="unfinished">Palette</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1670"/>
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
        <location filename="../src/core/qgsrunprocess.cpp" line="150"/>
        <source>Unable to run command</source>
        <translation>Kommando kann nicht gestartet werden</translation>
    </message>
    <message>
        <location filename="../src/core/qgsrunprocess.cpp" line="57"/>
        <source>Starting</source>
        <translation>Starte</translation>
    </message>
    <message>
        <location filename="../src/core/qgsrunprocess.cpp" line="119"/>
        <source>Done</source>
        <translation>Fertig</translation>
    </message>
    <message>
        <location filename="../src/core/qgsrunprocess.cpp" line="72"/>
        <source>Action</source>
        <translation type="unfinished">Aktion</translation>
    </message>
</context>
<context>
    <name>QgsScaleBarPlugin</name>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="160"/>
        <source> metres/km</source>
        <translation>Meter/Kilometer</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="278"/>
        <source> feet</source>
        <translation>Fuß</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="285"/>
        <source> degrees</source>
        <translation>Grad</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="240"/>
        <source> km</source>
        <translation> km</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="245"/>
        <source> mm</source>
        <translation> mm</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="250"/>
        <source> cm</source>
        <translation> cm</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="254"/>
        <source> m</source>
        <translation> m</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="274"/>
        <source> foot</source>
        <translation>Fuss</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="283"/>
        <source> degree</source>
        <translation>Grad</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="288"/>
        <source> unknown</source>
        <translation>unbekannt</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="77"/>
        <source>Top Left</source>
        <translation>Oben links</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="76"/>
        <source>Bottom Left</source>
        <translation>Unten links</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="77"/>
        <source>Top Right</source>
        <translation>Oben rechts</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="77"/>
        <source>Bottom Right</source>
        <translation>Unten rechts</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="79"/>
        <source>Tick Down</source>
        <translation>Strich unten</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="80"/>
        <source>Tick Up</source>
        <translation>Strich oben</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="80"/>
        <source>Bar</source>
        <translation>Balken</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="80"/>
        <source>Box</source>
        <translation>Box</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="100"/>
        <source>&amp;Scale Bar</source>
        <translation>&amp;Maßstab</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="101"/>
        <source>Creates a scale bar that is displayed on the map canvas</source>
        <translation>Erzeugt eine Maßstabsleiste, die im Kartenbild angezeigt wird.</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="539"/>
        <source>&amp;Decorations</source>
        <translation>&amp;Dekorationen</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="161"/>
        <source> feet/miles</source>
        <translation> Fuß/Meilen</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="259"/>
        <source> miles</source>
        <translation> Meilen</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="264"/>
        <source> mile</source>
        <translation> Meile</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="269"/>
        <source> inches</source>
        <translation> Inches</translation>
    </message>
</context>
<context>
    <name>QgsScaleBarPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="300"/>
        <source>Scale Bar Plugin</source>
        <translation>Maßstabs Plugin</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="143"/>
        <source>Top Left</source>
        <translation>Oben links</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="148"/>
        <source>Top Right</source>
        <translation>Oben rechts</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="153"/>
        <source>Bottom Left</source>
        <translation>Unten links</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="158"/>
        <source>Bottom Right</source>
        <translation>Unten rechts</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="94"/>
        <source>Size of bar:</source>
        <translation>Größe des Maßstab:</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="254"/>
        <source>Placement:</source>
        <translation>Platzierung:</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="211"/>
        <source>Tick Down</source>
        <translation>Strich unten</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="216"/>
        <source>Tick Up</source>
        <translation>Strich oben</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="221"/>
        <source>Box</source>
        <translation>Box</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="226"/>
        <source>Bar</source>
        <translation>Balken</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="207"/>
        <source>Select the style of the scale bar</source>
        <translation>Stil des Maßstab wählen</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="127"/>
        <source>Colour of bar:</source>
        <translation>Farbe des Maßstab:</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="192"/>
        <source>Scale bar style:</source>
        <translation>Maßstabsstil:</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="174"/>
        <source>Enable scale bar</source>
        <translation>Aktiviere Maßstab</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="109"/>
        <source>Automatically snap to round number on resize</source>
        <translation>bei Grässenänderung automatisch auf runden Zahlen einstellen</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="76"/>
        <source>Click to select the colour</source>
        <translation>Klick, um die Farbe auszuwählen.</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="274"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;This plugin draws a scale bar on the map. Please note the size option below is a &apos;preferred&apos; size and may have to be altered by QGIS depending on the level of zoom.  The size is measured according to the map units specified in the project properties.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Dieses Plugin zeichnet eine Maßstabsleiste auf die Karte. Bitte beachten Sie, dass die Größenoption eine &apos;bevorzugte&apos; Größe ist, die durch QGIS zoomstufenabhängig variiert wird. Die Größe wird in Karteneinheiten aus den Projektinformationen errechnet.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
</context>
<context>
    <name>QgsSearchQueryBuilder</name>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="169"/>
        <source>No matching features found.</source>
        <translation>Keine Treffer gefunden.</translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="170"/>
        <source>Search results</source>
        <translation>Suchergebnisse:</translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="179"/>
        <source>Search string parsing error</source>
        <translation>Fehler im Suchstring.</translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="235"/>
        <source>No Records</source>
        <translation>Keine Einträge</translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="235"/>
        <source>The query you specified results in zero records being returned.</source>
        <translation>Die definierte Abfrage gibt keine Treffer zurück.</translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="41"/>
        <source>Search query builder</source>
        <translation>Suche Query Builder</translation>
    </message>
    <message numerus="yes">
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="167"/>
        <source>Found %1 matching features.</source>
        <translation type="unfinished">
            <numerusform>Keine passenden Objekte gefunden.</numerusform>
        </translation>
    </message>
</context>
<context>
    <name>QgsServerSourceSelect</name>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="172"/>
        <source>Are you sure you want to remove the </source>
        <translation>Sind Sie sicher dass Sie die Verbindung und </translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="172"/>
        <source> connection and all associated settings?</source>
        <translation> alle damit verbunden Einstellungen löschen wollen?</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="173"/>
        <source>Confirm Delete</source>
        <translation>Löschen Bestätigen</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="352"/>
        <source>WMS Provider</source>
        <translation>WMS Anbinder</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="354"/>
        <source>Could not open the WMS Provider</source>
        <translation>Kann den WMS Provider nicht öffnen.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="363"/>
        <source>Select Layer</source>
        <translation>Wähle Layer aus.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="363"/>
        <source>You must select at least one layer first.</source>
        <translation>Es muss mindestens ein Layer ausgewählt werden.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="634"/>
        <source>Could not understand the response.  The</source>
        <translation>Kann die Antwort nicht verstehen. Der</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="635"/>
        <source>provider said</source>
        <translation>Provider sagte</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="686"/>
        <source>WMS proxies</source>
        <translation>WMS-Proxies</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="367"/>
        <source>Coordinate Reference System</source>
        <translation>Koordinatensystem</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="367"/>
        <source>There are no available coordinate reference system for the set of layers you&apos;ve selected.</source>
        <translation>Es existiert kein Koordinatensystem für den ausgewählten Layer.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="689"/>
        <source>Several WMS servers have been added to the server list. Note that if you access the internet via a web proxy, you will need to set the proxy settings in the QGIS options dialog.</source>
        <translation type="unfinished">Verschiedene WMS-Server wurden der Serverliste hinzugefügt. Beachten Sie bitte, dass Sie ggf. noch die Proxyeinstellungen in den QGIS Optionen einstellen müssen.</translation>
    </message>
    <message numerus="yes">
        <location filename="../src/app/qgsserversourceselect.cpp" line="475"/>
        <source>Coordinate Reference System (%1 available)</source>
        <translation type="unfinished">
            <numerusform>
        
        
        </numerusform>
        </translation>
    </message>
</context>
<context>
    <name>QgsServerSourceSelectBase</name>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="13"/>
        <source>Add Layer(s) from a Server</source>
        <translation>Layer von einem Server hinzufügen.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="309"/>
        <source>C&amp;lose</source>
        <translation>Schließen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="312"/>
        <source>Alt+L</source>
        <translation>Alt+L</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="296"/>
        <source>Help</source>
        <translation>Hilfe</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="299"/>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="270"/>
        <source>Image encoding</source>
        <translation>Bildkodierung</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="203"/>
        <source>Layers</source>
        <translation>Layer</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="230"/>
        <source>ID</source>
        <translation>ID</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="235"/>
        <source>Name</source>
        <translation>Name</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="240"/>
        <source>Title</source>
        <translation>Titel</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="245"/>
        <source>Abstract</source>
        <translation>Zusammenfassung</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="187"/>
        <source>&amp;Add</source>
        <translation>&amp;Hinzufügen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="190"/>
        <source>Alt+A</source>
        <translation>Alt+A</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="34"/>
        <source>Server Connections</source>
        <translation>Serververbindungen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="108"/>
        <source>&amp;New</source>
        <translation>&amp;Neu</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="101"/>
        <source>Delete</source>
        <translation>Löschen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="91"/>
        <source>Edit</source>
        <translation>Bearbeiten</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="81"/>
        <source>C&amp;onnect</source>
        <translation>Verbinden</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="174"/>
        <source>Ready</source>
        <translation>Fertig</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="118"/>
        <source>Coordinate Reference System</source>
        <translation>Koordinatensystem</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="156"/>
        <source>Change ...</source>
        <translation>Verändere ...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="46"/>
        <source>Adds a few example WMS servers</source>
        <translation>Fügt einige Beispiel-WMS-Server hinzu.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="52"/>
        <source>Add default servers</source>
        <translation>Standard-Server ergänzen</translation>
    </message>
</context>
<context>
    <name>QgsShapeFile</name>
    <message>
        <location filename="../src/plugins/spit/qgsshapefile.cpp" line="448"/>
        <source>The database gave an error while executing this SQL:</source>
        <translation type="unfinished">Datenbankfehler während der Ausführung der SQL-Anweisung: </translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsshapefile.cpp" line="456"/>
        <source>The error was:</source>
        <translation type="unfinished">Fehler war:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsshapefile.cpp" line="89"/>
        <source>Scanning </source>
        <translation type="unfinished">Durchsuche </translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsshapefile.cpp" line="453"/>
        <source>... (rest of SQL trimmed)</source>
        <comment>is appended to a truncated SQL statement</comment>
        <translation type="unfinished">... (Rest der Anweisung abgeschnitten)</translation>
    </message>
</context>
<context>
    <name>QgsSingleSymbolDialog</name>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="114"/>
        <source>Solid Line</source>
        <translation>durchgängige Linie</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="115"/>
        <source>Dash Line</source>
        <translation>gestrichelte Linie</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="116"/>
        <source>Dot Line</source>
        <translation>gepunktete Linie</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="117"/>
        <source>Dash Dot Line</source>
        <translation>gestrichelt-gepunktete Linie</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="118"/>
        <source>Dash Dot Dot Line</source>
        <translation>gestrichelt-2mal-gepunktete Linie</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="119"/>
        <source>No Pen</source>
        <translation>keine Linie</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="138"/>
        <source>No Brush</source>
        <translation>Keine Füllung</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="124"/>
        <source>Solid</source>
        <translation type="unfinished">Ausgefüllt</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="125"/>
        <source>Horizontal</source>
        <translation type="unfinished">Horizontal</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="126"/>
        <source>Vertical</source>
        <translation type="unfinished">Vertikal</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="127"/>
        <source>Cross</source>
        <translation type="unfinished">Kreuz</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="128"/>
        <source>BDiagonal</source>
        <translation>BDiagonal</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="129"/>
        <source>FDiagonal</source>
        <translation>BDiagonal</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="130"/>
        <source>Diagonal X</source>
        <translation>Diagonal X</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="131"/>
        <source>Dense1</source>
        <translation>Dicht1</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="132"/>
        <source>Dense2</source>
        <translation>Dicht2</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="133"/>
        <source>Dense3</source>
        <translation>Dicht3</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="134"/>
        <source>Dense4</source>
        <translation>Dicht4</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="135"/>
        <source>Dense5</source>
        <translation>Dicht5</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="136"/>
        <source>Dense6</source>
        <translation>Dicht6</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="137"/>
        <source>Dense7</source>
        <translation>Dicht7</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="139"/>
        <source>Texture</source>
        <translation>Textur</translation>
    </message>
</context>
<context>
    <name>QgsSingleSymbolDialogBase</name>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="19"/>
        <source>Single Symbol</source>
        <translation>Einfaches Symbol</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="130"/>
        <source>Size</source>
        <translation>Grösse</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="76"/>
        <source>Point Symbol</source>
        <translation>Punktsymbol </translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="120"/>
        <source>Area scale field</source>
        <translation>Flächenmaßstabsfeld</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="110"/>
        <source>Rotation field</source>
        <translation>Rotationsfeld</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="176"/>
        <source>Style Options</source>
        <translation>Stiloption</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="338"/>
        <source>...</source>
        <translation type="unfinished">...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="182"/>
        <source>Outline style</source>
        <translation>Umrandungsstil</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="214"/>
        <source>Outline color</source>
        <translation>Umrandungsfarbe
</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="249"/>
        <source>Outline width</source>
        <translation>Umrandungsbreite</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="275"/>
        <source>Fill color</source>
        <translation>Füllfarbe</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="310"/>
        <source>Fill style</source>
        <translation>Füllstil</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="48"/>
        <source>Label</source>
        <translation type="unfinished">Beschriftung</translation>
    </message>
</context>
<context>
    <name>QgsSnappingDialog</name>
    <message>
        <location filename="../src/app/qgssnappingdialog.cpp" line="147"/>
        <source>to vertex</source>
        <translation>zum Stützpunkt</translation>
    </message>
    <message>
        <location filename="../src/app/qgssnappingdialog.cpp" line="151"/>
        <source>to segment</source>
        <translation>zum Segment</translation>
    </message>
    <message>
        <location filename="../src/app/qgssnappingdialog.cpp" line="89"/>
        <source>to vertex and segment</source>
        <translation>zum Stützpunkt und Segment</translation>
    </message>
</context>
<context>
    <name>QgsSnappingDialogBase</name>
    <message>
        <location filename="../src/ui/qgssnappingdialogbase.ui" line="13"/>
        <source>Snapping options</source>
        <translation>Snapping Optionen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssnappingdialogbase.ui" line="26"/>
        <source>Layer</source>
        <translation type="unfinished">Layer</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssnappingdialogbase.ui" line="31"/>
        <source>Mode</source>
        <translation type="unfinished">Modus</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssnappingdialogbase.ui" line="36"/>
        <source>Tolerance</source>
        <translation>Toleranz</translation>
    </message>
</context>
<context>
    <name>QgsSpit</name>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="148"/>
        <source>Are you sure you want to remove the [</source>
        <translation>Soll die [</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="148"/>
        <source>] connection and all associated settings?</source>
        <translation>] Verbindung und alle zugeordneten Einstellungen gelöscht werden?</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="149"/>
        <source>Confirm Delete</source>
        <translation>Löschen Bestätigen</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="268"/>
        <source>The following Shapefile(s) could not be loaded:

</source>
        <translation>Die folgenden Shapedateien konnten nicht geladen werden:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="272"/>
        <source>REASON: File cannot be opened</source>
        <translation>GRUND: Datei konnte nicht geöffnet werden</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="277"/>
        <source>REASON: One or both of the Shapefile files (*.dbf, *.shx) missing</source>
        <translation>GRUND: Eine oder beide Shapedateien (*.dbf, *.shx) fehlen</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="354"/>
        <source>General Interface Help:</source>
        <translation>Allgemeine Hilfe Schnittstelle:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="356"/>
        <source>PostgreSQL Connections:</source>
        <translation>PostgreSQL Verbindungen:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="358"/>
        <source>[New ...] - create a new connection</source>
        <translation>[Neu...] - Verbindung erstellen</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="359"/>
        <source>[Edit ...] - edit the currently selected connection</source>
        <translation>[Bearbeiten ...] - die momentan gewählte Verbindung bearbeiten</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="360"/>
        <source>[Remove] - remove the currently selected connection</source>
        <translation>[Entfernen] - momentan gewählte Verbindung löschen</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="361"/>
        <source>-you need to select a connection that works (connects properly) in order to import files</source>
        <translation>- es muss eine Verbindung ausgewählt werden, die funktioniert (richtig verbindet) um Dateien zu importieren</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="362"/>
        <source>-when changing connections Global Schema also changes accordingly</source>
        <translation>-bei Änderungen an den Verbindungen ändern die globalen Schemas dementsprechend </translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="363"/>
        <source>Shapefile List:</source>
        <translation>Shapedateienliste:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="365"/>
        <source>[Add ...] - open a File dialog and browse to the desired file(s) to import</source>
        <translation>[Hinzufügen ...] - Dateidialog öffnen und die gewünschten Importdateien auswählen</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="366"/>
        <source>[Remove] - remove the currently selected file(s) from the list</source>
        <translation>[Entfernen] - löscht die ausgewählten Dateien von der Liste</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="367"/>
        <source>[Remove All] - remove all the files in the list</source>
        <translation>[Alles entfernen]  - löscht alle Dateien in der Liste</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="368"/>
        <source>[SRID] - Reference ID for the shapefiles to be imported</source>
        <translation>[SRID] - Referenz ID für die zu importierenden Shapedateien</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="369"/>
        <source>[Use Default (SRID)] - set SRID to -1</source>
        <translation>[Standart (SRID) verwenden] - setzt SRID auf -1</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="370"/>
        <source>[Geometry Column Name] - name of the geometry column in the database</source>
        <translation>[Geometriespaltenname] - Name der Geometriespalte in der Datenbank</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="371"/>
        <source>[Use Default (Geometry Column Name)] - set column name to &apos;the_geom&apos;</source>
        <translation>[Standard (Geometriespaltenname)] - setzt Spaltenname auf &apos;the_geom&apos;</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="372"/>
        <source>[Glogal Schema] - set the schema for all files to be imported into</source>
        <translation>[Globales Schema] setzt Schema für alle zu importierenden Dateien auf</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="374"/>
        <source>[Import] - import the current shapefiles in the list</source>
        <translation>[Importieren] - Shapedateien in der Liste importieren</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="375"/>
        <source>[Quit] - quit the program
</source>
        <translation>[Schließen] - das Programm verlassen</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="376"/>
        <source>[Help] - display this help dialog</source>
        <translation>[Hilfe] - zeigt den Hilfedialog an</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="835"/>
        <source>Import Shapefiles</source>
        <translation>Shapedateien importieren</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="835"/>
        <source>You need to specify a Connection first</source>
        <translation>Es muss zuerst eine Verbindung angegeben werden</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="428"/>
        <source>Connection failed - Check settings and try again</source>
        <translation>Verbindung fehlgeschlagen - Bitte Einstellungen überprüfen und erneut versuchen</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="514"/>
        <source>You need to add shapefiles to the list first</source>
        <translation>Es müssen zuerst Shapedateien in die Liste eingefügt werden</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="581"/>
        <source>Importing files</source>
        <translation>Dateien importieren</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="519"/>
        <source>Cancel</source>
        <translation>Abbrechen</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="523"/>
        <source>Progress</source>
        <translation>Fortschritt</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="532"/>
        <source>Problem inserting features from file:</source>
        <translation>Problem beim Einfügen von Objekten aus der Datei:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="539"/>
        <source>Invalid table name.</source>
        <translation>Ungültiger Tabellenname.</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="549"/>
        <source>No fields detected.</source>
        <translation>Keine Spalten erkannt.</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="574"/>
        <source>The following fields are duplicates:</source>
        <translation>Die folgenden Spalten kommen doppelt vor:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="674"/>
        <source>Import Shapefiles - Relation Exists</source>
        <translation>Shapedatei importieren - Relation existiert</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="675"/>
        <source>The Shapefile:</source>
        <translation>Die Shapedatei:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="676"/>
        <source>will use [</source>
        <translation>wird die Relation [</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="676"/>
        <source>] relation for its data,</source>
        <translation>], die bereits vorhanden ist</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="676"/>
        <source>which already exists and possibly contains data.</source>
        <translation>und evtl. Daten enthält, für ihre Daten benutzen.</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="677"/>
        <source>To avoid data loss change the &quot;DB Relation Name&quot;</source>
        <translation>Um Datenverlust zu vermeiden sollte in der Dateiliste</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="677"/>
        <source>for this Shapefile in the main dialog file list.</source>
        <translation>der &quot;DB-Relationsname&quot; geändert werden.</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="678"/>
        <source>Do you want to overwrite the [</source>
        <translation>Soll die Relation [</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="678"/>
        <source>] relation?</source>
        <translation>] überschrieben werden?</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="70"/>
        <source>File Name</source>
        <translation>Dateiname</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="70"/>
        <source>Feature Class</source>
        <translation>Objektklasse</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="71"/>
        <source>Features</source>
        <translation>Objekte</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="71"/>
        <source>DB Relation Name</source>
        <translation>DB-Relationsname</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="71"/>
        <source>Schema</source>
        <translation>Schema</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="172"/>
        <source>Add Shapefiles</source>
        <translation>Shapedateien hinzufügen</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="174"/>
        <source>Shapefiles (*.shp);;All files (*.*)</source>
        <translation>Shapefiles (*.shp);; Alle Dateien (*.*)</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="468"/>
        <source>PostGIS not available</source>
        <translation>PostGIS ist nicht verfügbar</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="470"/>
        <source>&lt;p&gt;The chosen database does not have PostGIS installed, but this is required for storage of spatial data.&lt;/p&gt;</source>
        <translation>&lt;p&gt;In der gewählte Datenbank ist PostGIS nicht installiert. PostGIS wird jedoch zum Speichern von räumlichen Daten benötigt.&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="816"/>
        <source>&lt;p&gt;Error while executing the SQL:&lt;/p&gt;&lt;p&gt;</source>
        <translation>&lt;p&gt;Fehler beim Ausführen des SQL:&lt;/p&gt;&lt;p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="817"/>
        <source>&lt;/p&gt;&lt;p&gt;The database said:</source>
        <translation>&lt;/p&gt;&lt;p&gt;Die Datenbank meldete:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="831"/>
        <source>%1 of %2 shapefiles could not be imported.</source>
        <translation type="unfinished">%1 von %2 Shape-Dateien konnte nicht importiert werden.</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="408"/>
        <source>Password for </source>
        <translation type="unfinished">Passwort für </translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="409"/>
        <source>Please enter your password:</source>
        <translation type="unfinished">Bitte Passwort eingeben:</translation>
    </message>
</context>
<context>
    <name>QgsSpitBase</name>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="19"/>
        <source>SPIT - Shapefile to PostGIS Import Tool</source>
        <translation>SPIT - Shapefile in PostGIS Import Tool</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="54"/>
        <source>PostgreSQL Connections</source>
        <translation>PostgreSQL-Verbindungen</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="188"/>
        <source>Remove</source>
        <translation>Entfernen</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="201"/>
        <source>Remove All</source>
        <translation>Alles entfernen</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="273"/>
        <source>Global Schema</source>
        <translation>Globales Schema</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="175"/>
        <source>Add</source>
        <translation>Hinzufügen</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="172"/>
        <source>Add a shapefile to the list of files to be imported</source>
        <translation>Der Liste der zu importierenden Dateien eine Shapedatei hinzufügen</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="185"/>
        <source>Remove the selected shapefile from the import list</source>
        <translation>Gewähltes Shapefile aus der Importliste entfernen</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="198"/>
        <source>Remove all the shapefiles from the import list</source>
        <translation>Alle Shapefiles aus der Importliste entfernen</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="224"/>
        <source>Set the SRID to the default value</source>
        <translation>SRID auf den Standardwert setzen</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="237"/>
        <source>Set the geometry column name to the default value</source>
        <translation>Geometriespaltenname auf den Standardwert setzen</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="121"/>
        <source>New</source>
        <translation>Neu</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="118"/>
        <source>Create a new PostGIS connection</source>
        <translation>Neue PostGIS Verbindung erstellen</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="105"/>
        <source>Remove the current PostGIS connection</source>
        <translation>Aktuelle PostGIS Verbindung entfernen</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="134"/>
        <source>Connect</source>
        <translation type="unfinished">Verbinden</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="95"/>
        <source>Edit</source>
        <translation>Bearbeiten</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="92"/>
        <source>Edit the current PostGIS connection</source>
        <translation>Aktuelle PostGIS-Verbindung bearbeiten</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="144"/>
        <source>Import options and shapefile list</source>
        <translation type="unfinished">Importoptionen und Shape-Dateiliste</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="227"/>
        <source>Use Default SRID or specify here</source>
        <translation type="unfinished">Eingestellte SRID nutzen</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="240"/>
        <source>Use Default Geometry Column Name or specify here</source>
        <translation type="unfinished">Eingestellten Spaltennamen nutzen</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="263"/>
        <source>Primary Key Column Name</source>
        <translation type="unfinished">Primärschlüsselspalte</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="131"/>
        <source>Connect to PostGIS</source>
        <translation type="unfinished">Mit PostGIS verbinden</translation>
    </message>
</context>
<context>
    <name>QgsSpitPlugin</name>
    <message>
        <location filename="../src/plugins/spit/qgsspitplugin.cpp" line="68"/>
        <source>&amp;Import Shapefiles to PostgreSQL</source>
        <translation>Shapefiles in PostgreSQL &amp;importieren</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitplugin.cpp" line="70"/>
        <source>Import shapefiles into a PostGIS-enabled PostgreSQL database. The schema and field names can be customized on import</source>
        <translation>Importiert Shapefiles in eine PostgreSQL-Datenbank mit PostGIS-Aufsatz. Schema und die Feldnamen des Imports sind einstellbar.</translation>
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
        <translation type="unfinished">Lineare Interpolation</translation>
    </message>
</context>
<context>
    <name>QgsTINInterpolatorDialogBase</name>
    <message>
        <location filename="../src/plugins/interpolation/qgstininterpolatordialogbase.ui" line="13"/>
        <source>Triangle based interpolation</source>
        <translation type="unfinished">Dreiecksinterpolation</translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgstininterpolatordialogbase.ui" line="19"/>
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
        <location filename="../src/plugins/interpolation/qgstininterpolatordialogbase.ui" line="31"/>
        <source>Interpolation method:</source>
        <translation type="unfinished">Interpolationsmethode</translation>
    </message>
</context>
<context>
    <name>QgsUniqueValueDialog</name>
    <message>
        <location filename="../src/app/qgsuniquevaluedialog.cpp" line="282"/>
        <source>Confirm Delete</source>
        <translation type="unfinished">Löschen bestätigen</translation>
    </message>
    <message>
        <location filename="../src/app/qgsuniquevaluedialog.cpp" line="285"/>
        <source>The classification field was changed from &apos;%1&apos; to &apos;%2&apos;.
Should the existing classes be deleted before classification?</source>
        <translation type="unfinished">Das Klassifizierungsfeld wurde von &apos;%1&apos; auf &apos;%2&apos; geändert.
Sollen die vorhandenen Klassen vor der Klassifizierung gelöscht werden?</translation>
    </message>
</context>
<context>
    <name>QgsUniqueValueDialogBase</name>
    <message>
        <location filename="../src/ui/qgsuniquevaluedialogbase.ui" line="19"/>
        <source>Form1</source>
        <translation>Formular1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsuniquevaluedialogbase.ui" line="93"/>
        <source>Classify</source>
        <translation type="unfinished">Klassifizieren</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsuniquevaluedialogbase.ui" line="49"/>
        <source>Classification field</source>
        <translation type="unfinished">Klassifizierungsfeld</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsuniquevaluedialogbase.ui" line="100"/>
        <source>Add class</source>
        <translation type="unfinished">Klasse hinzufügen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsuniquevaluedialogbase.ui" line="107"/>
        <source>Delete classes</source>
        <translation type="unfinished">Klassen löschen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsuniquevaluedialogbase.ui" line="114"/>
        <source>Randomize Colors</source>
        <translation type="unfinished">Zufällige Farben</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsuniquevaluedialogbase.ui" line="121"/>
        <source>Reset Colors</source>
        <translation type="unfinished">Farben zurücksetzen</translation>
    </message>
</context>
<context>
    <name>QgsVectorLayer</name>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2610"/>
        <source>ERROR: no provider</source>
        <translation type="unfinished">FEHLER: kein Datenlieferant</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2616"/>
        <source>ERROR: layer not editable</source>
        <translation type="unfinished">FEHLER: Layer ist nicht veränderbar.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2652"/>
        <source>SUCCESS: %1 attributes added.</source>
        <translation type="unfinished">ERFOLG: %1 Attribute geändert.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2658"/>
        <source>ERROR: %1 new attributes not added</source>
        <translation type="unfinished">FEHLER: %1 neue Attribute nicht hinzugefügt.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2630"/>
        <source>SUCCESS: %1 attributes deleted.</source>
        <translation type="unfinished">ERFOLG: %1 Attribute gelöscht.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2636"/>
        <source>ERROR: %1 attributes not deleted.</source>
        <translation type="unfinished">FEHLER: %1 Attribute nicht gelöscht.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2699"/>
        <source>SUCCESS: attribute %1 was added.</source>
        <translation type="unfinished">ERFOLG: Attribut %1 wurde hinzugefügt.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2706"/>
        <source>ERROR: attribute %1 not added</source>
        <translation type="unfinished">FEHLER: Attribute %1 wurde nicht hinzugefügt.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2765"/>
        <source>SUCCESS: %1 attribute values changed.</source>
        <translation type="unfinished">ERFOLG: %1 Attributwerte geändert.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2770"/>
        <source>ERROR: %1 attribute value changes not applied.</source>
        <translation type="unfinished">FEHLER: %1 Attributwertänderung nicht angewendet.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2802"/>
        <source>SUCCESS: %1 features added.</source>
        <translation type="unfinished">ERFOLG: %1 Objekte hinzugefügt.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2807"/>
        <source>ERROR: %1 features not added.</source>
        <translation type="unfinished">FEHLER: %1 Objekte nicht hinzugefügt.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2820"/>
        <source>SUCCESS: %1 geometries were changed.</source>
        <translation type="unfinished">ERFOLG: %1 Geometrien wurden geändert.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2825"/>
        <source>ERROR: %1 geometries not changed.</source>
        <translation type="unfinished">FEHLER: %1 Geometrien nicht geändert.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2837"/>
        <source>SUCCESS: %1 features deleted.</source>
        <translation type="unfinished">ERFOLG: %1 Objekte gelöscht.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2847"/>
        <source>ERROR: %1 features not deleted.</source>
        <translation type="unfinished">FEHLER: %1 Objekte nicht gelöscht.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2324"/>
        <source>No renderer object</source>
        <translation type="unfinished">Kein Renderer-Objekt</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2328"/>
        <source>Classification field not found</source>
        <translation type="unfinished">Klassifikationsfeld nicht gefunden</translation>
    </message>
</context>
<context>
    <name>QgsVectorLayerProperties</name>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="304"/>
        <source>Transparency: </source>
        <translation>Transparenz: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="395"/>
        <source>Single Symbol</source>
        <translation>Einfaches Symbol</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="398"/>
        <source>Graduated Symbol</source>
        <translation>Abgestuftes Symbol</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="399"/>
        <source>Continuous Color</source>
        <translation>Fortlaufende Farbe</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="400"/>
        <source>Unique Value</source>
        <translation>Eindeutiger Wert</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="355"/>
        <source>This button opens the PostgreSQL query builder and allows you to create a subset of features to display on the map canvas rather than displaying all features in the layer</source>
        <translation>Dieser Knopf öffnet den PostgreSQL-Query-Builder und ermöglicht, statt aller Objekte, eine Untermenge der Objekte auf dem Kartenfenster darzustellen</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="358"/>
        <source>The query used to limit the features in the layer is shown here. This is currently only supported for PostgreSQL layers. To enter or modify the query, click on the Query Builder button</source>
        <translation>Die Abfrage zur Begrenzung der Anzahl der Objekte wird hier angezeigt. Dies wird im Moment nur bei PostgreSQL-Layern unterstützt. Klicken Sie auf auf &apos;Query Builder&apos;, um eine Abfrage einzugeben oder zu ändern</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="656"/>
        <source>Spatial Index</source>
        <translation>Räumlicher Index</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="656"/>
        <source>Creation of spatial index failed</source>
        <translation>Erstellung des räumlichen Indexes fehlgeschlagen</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="669"/>
        <source>General:</source>
        <translation>Allgemein:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="684"/>
        <source>Storage type of this layer : </source>
        <translation>Datenspeicher dieses Layers: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="690"/>
        <source>Source for this layer : </source>
        <translation>Quelle dieses Layers: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="707"/>
        <source>Geometry type of the features in this layer : </source>
        <translation>Geometrietyp der Objekte dieses Layers: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="715"/>
        <source>The number of features in this layer : </source>
        <translation>Anzahl der Objekte dieses Layers: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="720"/>
        <source>Editing capabilities of this layer : </source>
        <translation>Bearbeitungsfähigkeit dieses Layers: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="727"/>
        <source>Extents:</source>
        <translation>Ausdehnung:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="732"/>
        <source>In layer spatial reference system units : </source>
        <translation>In Einheiten des Referenzsystems dieses Layers: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="733"/>
        <source>xMin,yMin </source>
        <translation>xMin,yMin </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="737"/>
        <source> : xMax,yMax </source>
        <translation> : xMax,yMax </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="792"/>
        <source>In project spatial reference system units : </source>
        <translation>In Einheiten des Projektreferenzsystems: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="766"/>
        <source>Layer Spatial Reference System:</source>
        <translation>Räumliches Referenzsystem des Layers:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="803"/>
        <source>Attribute field info:</source>
        <translation>Attribut info:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="810"/>
        <source>Field</source>
        <translation>Feld</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="813"/>
        <source>Type</source>
        <translation>Typ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="816"/>
        <source>Length</source>
        <translation>Länge</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="819"/>
        <source>Precision</source>
        <translation>Genauigkeit</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="677"/>
        <source>Layer comment: </source>
        <translation>Layerkommentar: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="822"/>
        <source>Comment</source>
        <translation type="unfinished">Kommentar</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="918"/>
        <source>Default Style</source>
        <translation>Standardstil</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="1021"/>
        <source>QGIS Layer Style File (*.qml)</source>
        <translation>QGIS Layerstil Datei (*.qml)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="1049"/>
        <source>QGIS</source>
        <translation>QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="1050"/>
        <source>Unknown style format: </source>
        <translation>Unbekanntes Stilformat: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="1042"/>
        <source>Saved Style</source>
        <translation type="unfinished">Gespeicherter Stil</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="134"/>
        <source>id</source>
        <translation type="unfinished">ID</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="135"/>
        <source>name</source>
        <translation type="unfinished">Name</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="136"/>
        <source>type</source>
        <translation type="unfinished">Typ</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="137"/>
        <source>length</source>
        <translation type="unfinished">Länge</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="138"/>
        <source>precision</source>
        <translation type="unfinished">Genauigkeit</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="139"/>
        <source>comment</source>
        <translation type="unfinished">Kommentar</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="140"/>
        <source>edit widget</source>
        <translation type="unfinished">Eingabefeld</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="141"/>
        <source>values</source>
        <translation type="unfinished">Werte</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="166"/>
        <source>line edit</source>
        <translation type="unfinished">Texteingabe</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="167"/>
        <source>unique values</source>
        <translation type="unfinished">eindeutige Werte</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="168"/>
        <source>unique values (editable)</source>
        <translation type="unfinished">eindeutige Werte (änderbar)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="169"/>
        <source>value map</source>
        <translation type="unfinished">Wertabbildung</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="170"/>
        <source>classification</source>
        <translation type="unfinished">Klassifizierung</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="171"/>
        <source>range (editable)</source>
        <translation type="unfinished">Bereich (änderbar)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="172"/>
        <source>range (slider)</source>
        <translation type="unfinished">Bereich (Schiebregler)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="173"/>
        <source>file name</source>
        <translation type="unfinished">Dateiname</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="249"/>
        <source>Name conflict</source>
        <translation type="unfinished">Namenskonflikt</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="249"/>
        <source>The attribute could not be inserted. The name already exists in the table.</source>
        <translation type="unfinished">Das Attribut konnte nicht eingefügt werden, da der Name bereits vorhanden ist.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="651"/>
        <source>Creation of spatial index successful</source>
        <translation type="unfinished">Räumlicher Index erfolgreich erzeugt.</translation>
    </message>
</context>
<context>
    <name>QgsVectorLayerPropertiesBase</name>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="19"/>
        <source>Layer Properties</source>
        <translation>Layereigenschaften</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="304"/>
        <source>Symbology</source>
        <translation>Darstellung</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="108"/>
        <source>General</source>
        <translation>Allgemein</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="180"/>
        <source>Use scale dependent rendering</source>
        <translation>Massstabsabhängig zeichnen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="212"/>
        <source>Minimum scale at which this layer will be displayed. </source>
        <translation>Minimaler Massstab ab dem dieser Layer angezeigt wird.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="225"/>
        <source>Maximum scale at which this layer will be displayed. </source>
        <translation>Maximaler Massstab bis zu dem dieser Layer angezeigt wird.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="120"/>
        <source>Display name</source>
        <translation>Anzeigename</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="149"/>
        <source>Use this control to set which field is placed at the top level of the Identify Results dialog box.</source>
        <translation>Benutze diese Kontrollelemente, um das Attribut festzulegen, das zuoberst im  &apos;Identifizierungsergebnis&apos;-Dialog steht.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="133"/>
        <source>Display field for the Identify Results dialog box</source>
        <translation>Anzeigeattribut für den &apos;Identifizierungsergebnis&apos;-Dialog</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="136"/>
        <source>This sets the display field for the Identify Results dialog box</source>
        <translation>Dies setzt das Anzeigeattribut für den &apos;Identifizierungsergebnis&apos;-Dialog</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="139"/>
        <source>Display field</source>
        <translation>Anzeigeattribut</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="241"/>
        <source>Subset</source>
        <translation>Subset</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="279"/>
        <source>Query Builder</source>
        <translation>Query Builder</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="163"/>
        <source>Create Spatial Index</source>
        <translation>Räumlichen Index erstellen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="437"/>
        <source>Metadata</source>
        <translation>Metadaten</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="465"/>
        <source>Labels</source>
        <translation>Beschriftungen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="477"/>
        <source>Display labels</source>
        <translation>Zeige Beschriftungen an</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="509"/>
        <source>Actions</source>
        <translation>Aktionen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="41"/>
        <source>Restore Default Style</source>
        <translation>Standardstil wiederherstellen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="48"/>
        <source>Save As Default</source>
        <translation>Als Standard speichern</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="55"/>
        <source>Load Style ...</source>
        <translation>Stil laden...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="62"/>
        <source>Save Style ...</source>
        <translation>Stil speichern...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="322"/>
        <source>Legend type</source>
        <translation type="unfinished">Legendentyp</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="360"/>
        <source>Transparency</source>
        <translation type="unfinished">Transparenz</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="114"/>
        <source>Options</source>
        <translation type="unfinished">Optionen</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="192"/>
        <source>Maximum</source>
        <translation>Maximum</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="202"/>
        <source>Minimum</source>
        <translation>Minimum</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="170"/>
        <source>Change CRS</source>
        <translation type="unfinished">KBS ändern</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="540"/>
        <source>Attributes</source>
        <translation type="unfinished">Attribute</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="557"/>
        <source>New column</source>
        <translation type="unfinished">Neue Attributspalte</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="567"/>
        <source>Ctrl+N</source>
        <translation type="unfinished">Ctrl+N</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="574"/>
        <source>Delete column</source>
        <translation type="unfinished">Lösche Attributspalte</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="584"/>
        <source>Ctrl+X</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="591"/>
        <source>Toggle editing mode</source>
        <translation type="unfinished">Bearbeitungsmodus umschalten</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="594"/>
        <source>Click to toggle table editing</source>
        <translation type="unfinished">Zur Umschaltung des Bearbeitungsmodus klicken</translation>
    </message>
</context>
<context>
    <name>QgsWFSPlugin</name>
    <message>
        <location filename="../src/plugins/wfs/qgswfsplugin.cpp" line="59"/>
        <source>&amp;Add WFS layer</source>
        <translation>&amp;WFS-Layer hinzufügen</translation>
    </message>
</context>
<context>
    <name>QgsWFSProvider</name>
    <message>
        <location filename="../src/providers/wfs/qgswfsprovider.cpp" line="1371"/>
        <source>unknown</source>
        <translation>unbekannt</translation>
    </message>
    <message>
        <location filename="../src/providers/wfs/qgswfsprovider.cpp" line="1377"/>
        <source>received %1 bytes from %2</source>
        <translation>%1 von %2 Bytes empfangen</translation>
    </message>
</context>
<context>
    <name>QgsWFSSourceSelect</name>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselect.cpp" line="257"/>
        <source>Are you sure you want to remove the </source>
        <translation>Sind Sie sicher, dass Sie die </translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselect.cpp" line="257"/>
        <source> connection and all associated settings?</source>
        <translation> Verbindung und alle damit verbundenen Einstellungen löschen wollen?</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselect.cpp" line="258"/>
        <source>Confirm Delete</source>
        <translation>Löschen bestätigen</translation>
    </message>
</context>
<context>
    <name>QgsWFSSourceSelectBase</name>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="29"/>
        <source>Title</source>
        <translation>Titel</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="34"/>
        <source>Name</source>
        <translation>Name</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="39"/>
        <source>Abstract</source>
        <translation>Zusammenfassung</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="47"/>
        <source>Coordinate Reference System</source>
        <translation>Koordinatensystem</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="85"/>
        <source>Change ...</source>
        <translation>Ändern ...</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="95"/>
        <source>Server Connections</source>
        <translation>Serververbindungen</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="107"/>
        <source>&amp;New</source>
        <translation>&amp;Neu</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="117"/>
        <source>Delete</source>
        <translation>Löschen</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="127"/>
        <source>Edit</source>
        <translation>Bearbeiten</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="153"/>
        <source>C&amp;onnect</source>
        <translation>Verbinden</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="13"/>
        <source>Add WFS Layer from a Server</source>
        <translation>WFS-Layer von Server hinzufügen</translation>
    </message>
</context>
<context>
    <name>QgsWmsProvider</name>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="719"/>
        <source>Tried URL: </source>
        <translation>Versuchte URL: </translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="699"/>
        <source>HTTP Exception</source>
        <translation>HTTP Ausnahme</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="662"/>
        <source>WMS Service Exception</source>
        <translation>WMS-Service-Ausnahme</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="773"/>
        <source>Could not get WMS capabilities: %1 at line %2 column %3</source>
        <translation>Konnte die WMS-Capabilities nicht erfragen: %1 in Zeile %2, Spalte %3</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="804"/>
        <source>This is probably due to an incorrect WMS Server URL.</source>
        <translation>Dies kommt wahrscheinlich durch eine falsche WMS-Server-URL zustande.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="800"/>
        <source>Could not get WMS capabilities in the expected format (DTD): no %1 or %2 found</source>
        <translation>Konnte die WMS-Capabilities nicht im erwarteten Format (DTD) erhalten: %1 oder %2 gefunden.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1548"/>
        <source>Could not get WMS Service Exception at %1: %2 at line %3 column %4</source>
        <translation>Konnte die WMS-Service-Ausnahme bei %1 nicht interpretieren: %2 in Zeile %3, Spalte %4</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1600"/>
        <source>Request contains a Format not offered by the server.</source>
        <translation>Die Anfrage enthält ein Format, das nicht vom Server unterstützt wird.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1604"/>
        <source>Request contains a CRS not offered by the server for one or more of the Layers in the request.</source>
        <translation>Die Anfrage enthält ein Koordinatensystem (CRS), das für die angeforderten Layer nicht vom Server angeboten wird.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1608"/>
        <source>Request contains a SRS not offered by the server for one or more of the Layers in the request.</source>
        <translation>Anfrage enthält ein SRS, das nicht vom Server angeboten wird.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1613"/>
        <source>GetMap request is for a Layer not offered by the server, or GetFeatureInfo request is for a Layer not shown on the map.</source>
        <translation>GetMap-Anfrage ist für einen Layer, der nicht vom Server angeboten wird, oder GetFeatureInfo wurde für einen Layer angefordert, der nicht im Kartenfenster angezeigt wird.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1617"/>
        <source>Request is for a Layer in a Style not offered by the server.</source>
        <translation>Anfrage ist für einen Layer in einem Style, der nicht vom Server angeboten wird.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1621"/>
        <source>GetFeatureInfo request is applied to a Layer which is not declared queryable.</source>
        <translation>GetFeatureInfo wurde auf einen Layer angewendet, der als nicht abfragbar definiert ist.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1625"/>
        <source>GetFeatureInfo request contains invalid X or Y value.</source>
        <translation>Get FeatureInfo Anfrage enthält ungültige X oder Y-Werte.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1640"/>
        <source>Request does not include a sample dimension value, and the server did not declare a default value for that dimension.</source>
        <translation>Anfrage enthält keinen beispielhaften Dimensionswert, und der Server selbst definiert auch keinen.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1644"/>
        <source>Request contains an invalid sample dimension value.</source>
        <translation>Anfrage enthält einen ungültigen beispielhaften Dimensionswert.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1648"/>
        <source>Request is for an optional operation that is not supported by the server.</source>
        <translation>Anfrage ist für eine optionale Operation, die der Server nicht unterstützt.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1652"/>
        <source>(Unknown error code from a post-1.3 WMS server)</source>
        <translation>(Unbekannte Fehlermeldung eines post-1.3 WMS-Servers).</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1655"/>
        <source>The WMS vendor also reported: </source>
        <translation>Der WMS-Betreiber sagt folgendes:</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1840"/>
        <source>Server Properties:</source>
        <translation>Server-Eigenschaften:</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1971"/>
        <source>Property</source>
        <translation>Eigenschaft</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1974"/>
        <source>Value</source>
        <translation>Wert</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1857"/>
        <source>WMS Version</source>
        <translation>WMS Version</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2103"/>
        <source>Title</source>
        <translation>Titel</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2111"/>
        <source>Abstract</source>
        <translation>Zusammenfassung</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1881"/>
        <source>Keywords</source>
        <translation>Schlüsselworte</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1889"/>
        <source>Online Resource</source>
        <translation>Online Quelle</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1897"/>
        <source>Contact Person</source>
        <translation>Kontaktperson</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1909"/>
        <source>Fees</source>
        <translation>Kosten</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1917"/>
        <source>Access Constraints</source>
        <translation>Zugangsbeschränkungen</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1925"/>
        <source>Image Formats</source>
        <translation>Bildformate</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1933"/>
        <source>Identify Formats</source>
        <translation>Abfrageformat</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1941"/>
        <source>Layer Count</source>
        <translation>Anzahl der Layer</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1961"/>
        <source>Layer Properties: </source>
        <translation>Layereigenschaften: </translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1979"/>
        <source>Selected</source>
        <translation>Ausgewählt</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2036"/>
        <source>Yes</source>
        <translation>Ja</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2036"/>
        <source>No</source>
        <translation>Nein</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1988"/>
        <source>Visibility</source>
        <translation>Sichtbarkeit</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1994"/>
        <source>Visible</source>
        <translation>Sichtbar</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1995"/>
        <source>Hidden</source>
        <translation>Versteckt</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1996"/>
        <source>n/a</source>
        <translation>n/a</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2017"/>
        <source>Can Identify</source>
        <translation>Kann abgefragt werden</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2025"/>
        <source>Can be Transparent</source>
        <translation>Kann transparent sein</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2033"/>
        <source>Can Zoom In</source>
        <translation>Kann reingezoomt werden.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2041"/>
        <source>Cascade Count</source>
        <translation>Kaskadiere Anzahl</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2049"/>
        <source>Fixed Width</source>
        <translation>Feste Breite</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2057"/>
        <source>Fixed Height</source>
        <translation>Feste Höhe</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2065"/>
        <source>WGS 84 Bounding Box</source>
        <translation>WGS 84 Bounding Box</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2075"/>
        <source>Available in CRS</source>
        <translation>Verfügbar in CRS</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2086"/>
        <source>Available in style</source>
        <translation>Verfügbar im Style</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2095"/>
        <source>Name</source>
        <translation>Name</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1630"/>
        <source>Value of (optional) UpdateSequence parameter in GetCapabilities request is equal to current value of service metadata update sequence number.</source>
        <translation>Wert in.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1635"/>
        <source>Value of (optional) UpdateSequence parameter in GetCapabilities request is greater than current value of service metadata update sequence number.</source>
        <translation>Wert in.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2196"/>
        <source>Layer cannot be queried.</source>
        <translation type="unfinished">Layer kann nicht abgefragt werden.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1546"/>
        <source>Dom Exception</source>
        <translation type="unfinished">Dom-Ausnahme</translation>
    </message>
</context>
<context>
    <name>QuickPrintGui</name>
    <message>
        <location filename="../src/plugins/quick_print/quickprintgui.cpp" line="129"/>
        <source>Portable Document Format (*.pdf)</source>
        <translation>Portable Document Format (*.pdf)</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintgui.cpp" line="154"/>
        <source>quickprint</source>
        <translation>Schnelldruck</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintgui.cpp" line="155"/>
        <source>Unknown format: </source>
        <translation>Unbekanntes Format: </translation>
    </message>
</context>
<context>
    <name>QuickPrintGuiBase</name>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="13"/>
        <source>QGIS Quick Print Plugin</source>
        <translation>QGIS Schnelldruck Plugin</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="158"/>
        <source>Quick Print</source>
        <translation>Schneller Druck</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="129"/>
        <source>Map Title e.g. ACME inc.</source>
        <translation>Kartentitel z.B. ACME inc.</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="116"/>
        <source>Map Name e.g. Water Features</source>
        <translation>Kartenname z.B. Wasserobjekte</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="103"/>
        <source>Copyright</source>
        <translation>Copyright</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="48"/>
        <source>Output</source>
        <translation type="unfinished">Ergebnis</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="60"/>
        <source>Use last filename but incremented.</source>
        <translation>Benutze zuletzt verwendeten Dateinamen aber erhöht.</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="67"/>
        <source>last used filename but incremented will be shown here</source>
        <translation>zuletzt verwendeter aber erhöhter Dateiname wrd hier gezeigt</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="77"/>
        <source>Prompt for file name</source>
        <translation>Eingabeaufforderung für Dateiname</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="38"/>
        <source>Note: If you want more control over the map layout please use the map composer function in QGIS.</source>
        <translation>Bemerkung: Wenn Sie mehr Kontrolle über das Layout haben wollen, benutzen Sie bitte den QGIS Map Composer.</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="93"/>
        <source>Page Size</source>
        <translation type="unfinished">Seitengröße</translation>
    </message>
</context>
<context>
    <name>QuickPrintPlugin</name>
    <message>
        <location filename="../src/plugins/quick_print/quickprintplugin.cpp" line="75"/>
        <source>Quick Print</source>
        <translation>Schnelldruck</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintplugin.cpp" line="82"/>
        <source>&amp;Quick Print</source>
        <translation type="unfinished">Schnelldruck</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintplugin.cpp" line="77"/>
        <source>Provides a way to quickly produce a map with minimal user input.</source>
        <translation type="unfinished">Bietet eine schnelle Möglichkeit Karten mit minimalen Benutzereingaben zu erzeugen</translation>
    </message>
</context>
<context>
    <name>[pluginname]GuiBase</name>
    <message>
        <location filename="../src/plugins/plugin_template/pluginguibase.ui" line="13"/>
        <source>QGIS Plugin Template</source>
        <translation type="unfinished">QGIS Plugin Vorlage</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/pluginguibase.ui" line="47"/>
        <source>Plugin Template</source>
        <translation type="unfinished">Plugin Vorlage</translation>
    </message>
</context>
<context>
    <name>dxf2shpConverter</name>
    <message>
        <location filename="../src/plugins/dxf2shp_converter/dxf2shpconverter.cpp" line="73"/>
        <source>Converts DXF files in Shapefile format</source>
        <translation type="unfinished">Konvertiert DXF-Dateien ins Shape-Format.</translation>
    </message>
    <message>
        <location filename="../src/plugins/dxf2shp_converter/dxf2shpconverter.cpp" line="109"/>
        <source>&amp;Dxf2Shp</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>dxf2shpConverterGui</name>
    <message>
        <location filename="../src/plugins/dxf2shp_converter/dxf2shpconvertergui.ui" line="25"/>
        <source>Dxf Importer</source>
        <translation type="unfinished">DXF-Import</translation>
    </message>
    <message>
        <location filename="../src/plugins/dxf2shp_converter/dxf2shpconvertergui.ui" line="34"/>
        <source>Input Dxf file</source>
        <translation type="unfinished">DXF-Eingabedatei</translation>
    </message>
    <message>
        <location filename="../src/plugins/dxf2shp_converter/dxf2shpconvertergui.ui" line="64"/>
        <source>...</source>
        <translation type="unfinished">...</translation>
    </message>
    <message>
        <location filename="../src/plugins/dxf2shp_converter/dxf2shpconvertergui.ui" line="51"/>
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
        <location filename="../src/plugins/dxf2shp_converter/dxf2shpconvertergui.ui" line="83"/>
        <source>Output file type</source>
        <translation type="unfinished">Typ der Ausgabedatei</translation>
    </message>
    <message>
        <location filename="../src/plugins/dxf2shp_converter/dxf2shpconvertergui.ui" line="89"/>
        <source>Polyline</source>
        <translation type="unfinished">Polylinie</translation>
    </message>
    <message>
        <location filename="../src/plugins/dxf2shp_converter/dxf2shpconvertergui.ui" line="99"/>
        <source>Polygon</source>
        <translation type="unfinished">Polygon</translation>
    </message>
    <message>
        <location filename="../src/plugins/dxf2shp_converter/dxf2shpconvertergui.ui" line="106"/>
        <source>Point</source>
        <translation type="unfinished">Punkt</translation>
    </message>
    <message>
        <location filename="../src/plugins/dxf2shp_converter/dxf2shpconvertergui.ui" line="116"/>
        <source>Export text labels</source>
        <translation type="unfinished">Beschriftungen exportieren</translation>
    </message>
    <message>
        <location filename="../src/plugins/dxf2shp_converter/dxf2shpconvertergui.cpp" line="130"/>
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
        <translation type="unfinished">Feldbeschreibung:
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
        <location filename="../src/plugins/dxf2shp_converter/dxf2shpconvertergui.cpp" line="152"/>
        <source>Choose a DXF file to open</source>
        <translation type="unfinished">Zu öffnende DXF-Datei wählen</translation>
    </message>
    <message>
        <location filename="../src/plugins/dxf2shp_converter/dxf2shpconvertergui.cpp" line="162"/>
        <source>Choose a file name to save to</source>
        <translation type="unfinished">Dateiname zum Speichern wählen</translation>
    </message>
</context>
<context>
    <name>pluginname</name>
    <message>
        <location filename="../src/plugins/plugin_template/plugin.cpp" line="73"/>
        <source>[menuitemname]</source>
        <translation type="unfinished">[menuitemname]</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugin.cpp" line="80"/>
        <source>&amp;[menuname]</source>
        <translation>&amp;[menuname]</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugin.cpp" line="75"/>
        <source>Replace this with a short description of what the plugin does</source>
        <translation type="unfinished">Kurzbeschreibung des Plugins hier</translation>
    </message>
</context>
</TS>
