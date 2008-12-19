<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS><TS version="1.1" language="pl_PL">
<defaultcodec></defaultcodec>
<context>
    <name>CoordinateCapture</name>
    <message>
        <source>Coordinate Capture</source>
        <translation>Przechwyć współrzędne</translation>
    </message>
    <message>
        <source>Click on the map to view coordinates and capture to clipboard.</source>
        <translation>Kliknij na mapie aby wyświetlić współrzędne i przechwycić je do schowka.</translation>
    </message>
    <message>
        <source>&amp;Coordinate Capture</source>
        <translation>&amp;Przechwyć współrzędne</translation>
    </message>
    <message>
        <source>Click to select the CRS to use for coordinate display</source>
        <translation>Kliknij aby wybrać SOW do wyświetlania współrzędnych</translation>
    </message>
    <message>
        <source>Coordinate in your selected CRS</source>
        <translation>Współrzędne w wybranym SOW</translation>
    </message>
    <message>
        <source>Coordinate in map canvas coordinate reference system</source>
        <translation>Współrzędne w układzie współrzędnych obszaru mapy</translation>
    </message>
    <message>
        <source>Copy to clipboard</source>
        <translation>Kopiuj do schowka</translation>
    </message>
    <message>
        <source>Click to enable mouse tracking. Click the canvas to stop</source>
        <translation>Kliknij aby włączyć śledzenie myszy. Kliknij ponownie w obszarze mapy aby zatrzymać</translation>
    </message>
</context>
<context>
    <name>CoordinateCaptureGui</name>
    <message>
        <source>Welcome to your automatically generated plugin!</source>
        <translation>Witaj w Twojej automatycznie generowanej wtyczce!</translation>
    </message>
    <message>
        <source>This is just a starting point. You now need to modify the code to make it do something useful....read on for a more information to get yourself started.</source>
        <translation>To dopiero początek. Teraz należy zmodyfikować kod źródłowy aby odpowiednio go dostosować ...  przeczytaj więcej aby dowiedzieć się jak zacząć.</translation>
    </message>
    <message>
        <source>Documentation:</source>
        <translation>Dokumentacja:</translation>
    </message>
    <message>
        <source>You really need to read the QGIS API Documentation now at:</source>
        <translation>Teraz należy przeczytać dokumentację QGIS API:</translation>
    </message>
    <message>
        <source>In particular look at the following classes:</source>
        <translation>Zwróć szczególną uwagę na następujące klasy:</translation>
    </message>
    <message>
        <source>QgsPlugin is an ABC that defines required behaviour your plugin must provide. See below for more details.</source>
        <translation>QgsPlugin to ABC które definiuje funkcje które twoja wtyczka musi udostępniać. Szczegóły przeczytaj poniżej.</translation>
    </message>
    <message>
        <source>What are all the files in my generated plugin directory for?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>This is the generated CMake file that builds the plugin. You should add you application specific dependencies and source files to this file.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>This is the class that provides the &apos;glue&apos; between your custom application logic and the QGIS application. You will see that a number of methods are already implemented for you - including some examples of how to add a raster or vector layer to the main application map canvas. This class is a concrete instance of the QgisPlugin interface which defines required behaviour for a plugin. In particular, a plugin has a number of static methods and members so that the QgsPluginManager and plugin loader logic can identify each plugin, create an appropriate menu entry for it etc. Note there is nothing stopping you creating multiple toolbar icons and menu entries for a single plugin. By default though a single menu entry and toolbar button is created and its pre-configured to call the run() method in this class when selected. This default implementation provided for you by the plugin builder is well documented, so please refer to the code for further advice.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>This is a Qt designer &apos;ui&apos; file. It defines the look of the default plugin dialog without implementing any application logic. You can modify this form to suite your needs or completely remove it if your plugin does not need to display a user form (e.g. for custom MapTools).</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>This is the concrete class where application logic for the above mentioned dialog should go. The world is your oyster here really....</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>This is the Qt4 resources file for your plugin. The Makefile generated for your plugin is all set up to compile the resource file so all you need to do is add your additional icons etc using the simple xml file format. Note the namespace used for all your resources e.g. (&apos;:/Homann/&apos;). It is important to use this prefix for all your resources. We suggest you include any other images and run time data in this resurce file too.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>This is the icon that will be used for your plugin menu entry and toolbar icon. Simply replace this icon with your own icon to make your plugin disctinctive from the rest.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>This file contains the documentation you are reading now!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Getting developer help:</source>
        <translation>Pomoc programistów:</translation>
    </message>
    <message>
        <source>For Questions and Comments regarding the plugin builder template and creating your features in QGIS using the plugin interface please contact us via:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;li&gt; the QGIS developers mailing list, or &lt;/li&gt;&lt;li&gt; IRC (#qgis on freenode.net)&lt;/li&gt;</source>
        <translation>&lt;li&gt; lista dyskusyjna programistów QGIS, lub &lt;/li&gt;&lt;li&gt; IRC (#qgis na freenode.net)&lt;/li&gt;</translation>
    </message>
    <message>
        <source>QGIS is distributed under the Gnu Public License. If you create a useful plugin please consider contributing it back to the community.</source>
        <translation>QGIS jest rozpowszechniany na zasadach Powszechnej Licencji Publicznej GNU. Jeżeli stworzysz użyteczną wtyczkę, rozważ proszę jej udostępnienie społeczności użytkowników QGIS.</translation>
    </message>
    <message>
        <source>Have fun and thank you for choosing QGIS.</source>
        <translation>Dziękujemy za wybranie QGIS i życzymy miłej pracy.</translation>
    </message>
</context>
<context>
    <name>CoordinateCaptureGuiBase</name>
    <message>
        <source>QGIS Plugin Template</source>
        <translation>Szablon wtyczki QGIS</translation>
    </message>
    <message>
        <source>Plugin Template</source>
        <translation>Szablon wtyczki</translation>
    </message>
</context>
<context>
    <name>Dialog</name>
    <message>
        <source>Point Symbol</source>
        <translation type="obsolete">Symbol punktu</translation>
    </message>
    <message>
        <source>Size</source>
        <translation type="obsolete">Rozmiar</translation>
    </message>
    <message>
        <source>Area scale field</source>
        <translation type="obsolete">Skaluj wielkość wg pola</translation>
    </message>
    <message>
        <source>Rotation field</source>
        <translation type="obsolete">Obrót wg pola</translation>
    </message>
    <message>
        <source>Style Options</source>
        <translation type="obsolete">Opcje stylu</translation>
    </message>
    <message>
        <source>Outline style</source>
        <translation type="obsolete">Styl obrysu</translation>
    </message>
    <message>
        <source>Outline color</source>
        <translation type="obsolete">Kolor obrysu</translation>
    </message>
    <message>
        <source>Outline width</source>
        <translation type="obsolete">Szerokość obrysu</translation>
    </message>
    <message>
        <source>Fill color</source>
        <translation type="obsolete">Kolor wypełnienia</translation>
    </message>
    <message>
        <source>Fill style</source>
        <translation type="obsolete">Styl wypełnienia</translation>
    </message>
    <message>
        <source>...</source>
        <translation type="obsolete">...</translation>
    </message>
    <message>
        <source>Connect</source>
        <translation>Połącz</translation>
    </message>
    <message>
        <source>Browse</source>
        <translation>Przeglądaj</translation>
    </message>
    <message>
        <source>OGR Converter</source>
        <translation>Konwerter OGR</translation>
    </message>
    <message>
        <source>Could not establish connection to: &apos;</source>
        <translation>Nie można ustanowić połączenia do:&apos;</translation>
    </message>
    <message>
        <source>Open OGR file</source>
        <translation>Otwórz plik OGR</translation>
    </message>
    <message>
        <source>OGR File Data Source (*.*)</source>
        <translation>Plik OGR (*.*)</translation>
    </message>
    <message>
        <source>Open Directory</source>
        <translation>Otwórz katalog</translation>
    </message>
    <message>
        <source>Input OGR dataset is missing!</source>
        <translation>Brak wejściowego dataset OGR!</translation>
    </message>
    <message>
        <source>Input OGR layer name is missing!</source>
        <translation>Brak wejściowej nazwy warstwy OGR!</translation>
    </message>
    <message>
        <source>Target OGR format not selected!</source>
        <translation>Nie wybrano docelowego formatu OGR!</translation>
    </message>
    <message>
        <source>Output OGR dataset is missing!</source>
        <translation>Brak wyjściowego dataset OGR!</translation>
    </message>
    <message>
        <source>Output OGR layer name is missing!</source>
        <translation>Brak wyjściowej nazwy warstwy OGR!</translation>
    </message>
    <message>
        <source>Successfully translated layer &apos;</source>
        <translation>Pomyślnie przekonwertowano warstwę &apos;</translation>
    </message>
    <message>
        <source>Failed to translate layer &apos;</source>
        <translation>Nie udało się przekonwertować warstwy &apos;</translation>
    </message>
    <message>
        <source>Successfully connected to: &apos;</source>
        <translation>Pomyślnie połączono z: &apos;</translation>
    </message>
    <message>
        <source>Choose a file name to save to</source>
        <translation>Podaj nazwę pliku do którego należy zapisać</translation>
    </message>
</context>
<context>
    <name>Gui</name>
    <message>
        <source>Welcome to your automatically generated plugin!</source>
        <translation>Witaj w Twojej automatycznie generowanej wtyczce!</translation>
    </message>
    <message>
        <source>This is just a starting point. You now need to modify the code to make it do something useful....read on for a more information to get yourself started.</source>
        <translation>To dopiero początek. Teraz należy zmodyfikować kod źródłowy aby odpowiednio go dostosować ...  przeczytaj więcej aby rozpocząć samodzielnie pracę.</translation>
    </message>
    <message>
        <source>Documentation:</source>
        <translation>Dokumentacja:</translation>
    </message>
    <message>
        <source>You really need to read the QGIS API Documentation now at:</source>
        <translation>Teraz należy przeczytać dokumentację QGIS API:</translation>
    </message>
    <message>
        <source>In particular look at the following classes:</source>
        <translation>Zwróć szczególną uwagę na następujące klasy:</translation>
    </message>
    <message>
        <source>QgsPlugin is an ABC that defines required behaviour your plugin must provide. See below for more details.</source>
        <translation>QgsPlugin to ABC które definiuje funkcje które twoja wtyczka musi udostępniać. Szczegóły przeczytaj poniżej.</translation>
    </message>
    <message>
        <source>What are all the files in my generated plugin directory for?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>This is the generated CMake file that builds the plugin. You should add you application specific dependencies and source files to this file.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>This is the class that provides the &apos;glue&apos; between your custom application logic and the QGIS application. You will see that a number of methods are already implemented for you - including some examples of how to add a raster or vector layer to the main application map canvas. This class is a concrete instance of the QgisPlugin interface which defines required behaviour for a plugin. In particular, a plugin has a number of static methods and members so that the QgsPluginManager and plugin loader logic can identify each plugin, create an appropriate menu entry for it etc. Note there is nothing stopping you creating multiple toolbar icons and menu entries for a single plugin. By default though a single menu entry and toolbar button is created and its pre-configured to call the run() method in this class when selected. This default implementation provided for you by the plugin builder is well documented, so please refer to the code for further advice.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>This is a Qt designer &apos;ui&apos; file. It defines the look of the default plugin dialog without implementing any application logic. You can modify this form to suite your needs or completely remove it if your plugin does not need to display a user form (e.g. for custom MapTools).</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>This is the concrete class where application logic for the above mentioned dialog should go. The world is your oyster here really....</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>This is the Qt4 resources file for your plugin. The Makefile generated for your plugin is all set up to compile the resource file so all you need to do is add your additional icons etc using the simple xml file format. Note the namespace used for all your resources e.g. (&apos;:/Homann/&apos;). It is important to use this prefix for all your resources. We suggest you include any other images and run time data in this resurce file too.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>This is the icon that will be used for your plugin menu entry and toolbar icon. Simply replace this icon with your own icon to make your plugin disctinctive from the rest.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>This file contains the documentation you are reading now!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Getting developer help:</source>
        <translation>Pomoc dla programistów:</translation>
    </message>
    <message>
        <source>For Questions and Comments regarding the plugin builder template and creating your features in QGIS using the plugin interface please contact us via:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;li&gt; the QGIS developers mailing list, or &lt;/li&gt;&lt;li&gt; IRC (#qgis on freenode.net)&lt;/li&gt;</source>
        <translation>&lt;li&gt; lista dyskusyjna programistów QGIS, lub &lt;/li&gt;&lt;li&gt; IRC (#qgis na freenode.net)&lt;/li&gt;</translation>
    </message>
    <message>
        <source>QGIS is distributed under the Gnu Public License. If you create a useful plugin please consider contributing it back to the community.</source>
        <translation>QGIS jest rozpowszechniany na zasadach Powszechnej Licencji Publicznej GNU. Jeżeli stworzysz użyteczną wtyczkę, rozważ proszę jej udostępnienie społeczności użytkowników QGIS.</translation>
    </message>
    <message>
        <source>Have fun and thank you for choosing QGIS.</source>
        <translation>Dziękujemy za wybranie QGIS i życzymy miłej pracy.</translation>
    </message>
</context>
<context>
    <name>MapCoordsDialogBase</name>
    <message>
        <source>Enter map coordinates</source>
        <translation>Wpisz współrzędne mapy</translation>
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
        <translation>&amp;Anuluj</translation>
    </message>
    <message>
        <source>Enter X and Y coordinates which correspond with the selected point on the image. Alternatively, click the button with icon of a pencil and then click a corresponding point on map canvas of QGIS to fill in coordinates of that point.</source>
        <translation>Wpisz współrzędne XY, odpowiadające wskazanemu punktowi na obrazie. Ewentualnie możesz kliknąć na przycisku z ikoną ołówka, a następnie na odpowiadającym punkcie w obszarze mapy, aby pobrać współrzędne tego punktu.</translation>
    </message>
    <message>
        <source> from map canvas</source>
        <translation>z obszaru mapy</translation>
    </message>
</context>
<context>
    <name>OgrConverterGuiBase</name>
    <message>
        <source>OGR Layer Converter</source>
        <translation>Konwerter warstw OGR</translation>
    </message>
    <message>
        <source>Source</source>
        <translation>Źródło</translation>
    </message>
    <message>
        <source>Format</source>
        <translation>Format</translation>
    </message>
    <message>
        <source>File</source>
        <translation>Plik</translation>
    </message>
    <message>
        <source>Directory</source>
        <translation>Katalog</translation>
    </message>
    <message>
        <source>Remote source</source>
        <translation>Źródło zdalne</translation>
    </message>
    <message>
        <source>Dataset</source>
        <translation>Dataset</translation>
    </message>
    <message>
        <source>Browse</source>
        <translation>Przeglądaj</translation>
    </message>
    <message>
        <source>Layer</source>
        <translation>Warstwa</translation>
    </message>
    <message>
        <source>Target</source>
        <translation>Cel</translation>
    </message>
</context>
<context>
    <name>OgrPlugin</name>
    <message>
        <source>Run OGR Layer Converter</source>
        <translation>Uruchom konwerter warstw OGR</translation>
    </message>
    <message>
        <source>Replace this with a short description of the what the plugin does</source>
        <translation type="obsolete">Zastąp to krótkim opisem działania wtyczki</translation>
    </message>
    <message>
        <source>OG&amp;R Converter</source>
        <translation>OG&amp;R konwerter</translation>
    </message>
    <message>
        <source>Translates vector layers between formats supported by OGR library</source>
        <translation>Konwertuje warstwy wektorowe pomiędzy formatami wspieranymi przez bibliotekę OGR</translation>
    </message>
</context>
<context>
    <name>QFileDialog</name>
    <message>
        <source>Save experiment report to portable document format (.pdf)</source>
        <translation>Zapisz raport próbny w formacie pdf (.pdf)</translation>
    </message>
    <message>
        <source>Load layer properties from style file (.qml)</source>
        <translation>Wczytaj właściwości warstwy z pliku stylu (.qml)</translation>
    </message>
    <message>
        <source>Save layer properties as style file (.qml)</source>
        <translation>Zapisz właściwości warstwy do pliku stylu (.qml)</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <source>QGis files (*.qgs)</source>
        <translation>Pliki QGIS (*.qgs)</translation>
    </message>
    <message>
        <source>No Data Provider Plugins</source>
        <comment>No QGIS data provider plugins found in:</comment>
        <translation>Brak wtyczek źródeł danych</translation>
    </message>
    <message>
        <source>No vector layers can be loaded. Check your QGIS installation</source>
        <translation>Warstwy wektorowe nie mogą zostać wczytane. Spawdź swoją instalację QGIS</translation>
    </message>
    <message>
        <source>No Data Providers</source>
        <translation>Brak źródeł danych</translation>
    </message>
    <message>
        <source>No data provider plugins are available. No vector layers can be loaded</source>
        <translation>Nie są dostępne żadne wtyczki źródeł danych. Nie można wczytać wartstw wektorowych</translation>
    </message>
    <message>
        <source> at line </source>
        <translation>w wierszu </translation>
    </message>
    <message>
        <source> column </source>
        <translation>kolumna</translation>
    </message>
    <message>
        <source> for file </source>
        <translation>dla pliku</translation>
    </message>
    <message>
        <source>Unable to save to file </source>
        <translation>Nie mogę zapisać pliku</translation>
    </message>
    <message>
        <source>Referenced column wasn&apos;t found: </source>
        <translation>Kolumna referencyjna nie została odnaleziona: </translation>
    </message>
    <message>
        <source>Division by zero.</source>
        <translation>Dzielenie przez zero.</translation>
    </message>
    <message>
        <source>No active layer</source>
        <translation>Brak aktywnej warstwy</translation>
    </message>
    <message>
        <source>Band</source>
        <translation>Kanał</translation>
    </message>
    <message>
        <source>action</source>
        <translation>akcja</translation>
    </message>
    <message>
        <source> features found</source>
        <translation>znalezione obiekty</translation>
    </message>
    <message>
        <source> 1 feature found</source>
        <translation>1 znaleziony obiekt</translation>
    </message>
    <message>
        <source>No features found</source>
        <translation>Nie znaleziono obiektów</translation>
    </message>
    <message>
        <source>No features were found in the active layer at the point you clicked</source>
        <translation>Nie znaleziono żadnych obiektów na aktywnej warstwie w miejscu kliknięcia</translation>
    </message>
    <message>
        <source>Could not identify objects on</source>
        <translation>Nie mogę zidentyfikować obiektu</translation>
    </message>
    <message>
        <source>because</source>
        <translation> ponieważ</translation>
    </message>
    <message>
        <source>New centroid</source>
        <translation>Nowy centroid</translation>
    </message>
    <message>
        <source>New point</source>
        <translation>Nowy punkt</translation>
    </message>
    <message>
        <source>New vertex</source>
        <translation>Nowy wierzchołek</translation>
    </message>
    <message>
        <source>Undo last point</source>
        <translation>Cofnij ostatni punkt</translation>
    </message>
    <message>
        <source>Close line</source>
        <translation>Zakończ linię</translation>
    </message>
    <message>
        <source>Select vertex</source>
        <translation>Wybierz wierzchołek</translation>
    </message>
    <message>
        <source>Select new position</source>
        <translation>Wybierz nową pozycję</translation>
    </message>
    <message>
        <source>Select line segment</source>
        <translation>Wybierz segment linii</translation>
    </message>
    <message>
        <source>New vertex position</source>
        <translation>Nowa pozycja wierzchołka</translation>
    </message>
    <message>
        <source>Release</source>
        <translation>Zwolnij</translation>
    </message>
    <message>
        <source>Delete vertex</source>
        <translation>Usuń wierzchołek</translation>
    </message>
    <message>
        <source>Release vertex</source>
        <translation>Zwolnij wierzchołek</translation>
    </message>
    <message>
        <source>Select element</source>
        <translation>Zaznacz element</translation>
    </message>
    <message>
        <source>New location</source>
        <translation>Nowa lokacja</translation>
    </message>
    <message>
        <source>Release selected</source>
        <translation>Wybrano zwolnienie</translation>
    </message>
    <message>
        <source>Delete selected / select next</source>
        <translation>Usuń zaznaczone / zaznacz następne</translation>
    </message>
    <message>
        <source>Select position on line</source>
        <translation>Zaznacz pozycję na linii</translation>
    </message>
    <message>
        <source>Split the line</source>
        <translation>Podziel linię</translation>
    </message>
    <message>
        <source>Release the line</source>
        <translation>Zwolnij linię</translation>
    </message>
    <message>
        <source>Select point on line</source>
        <translation>Zaznacz punkt na linii</translation>
    </message>
    <message>
        <source>Label</source>
        <translation>Etykieta</translation>
    </message>
    <message>
        <source>Length</source>
        <translation>Długość</translation>
    </message>
    <message>
        <source>Area</source>
        <translation>Powierzchnia</translation>
    </message>
    <message>
        <source>Project file read error: </source>
        <translation>Błąd odczytu pliku projektu:</translation>
    </message>
    <message>
        <source>Fit to a linear transform requires at least 2 points.</source>
        <translation>Transformacja liniowa wymaga wskazania minimum 2 punktów.</translation>
    </message>
    <message>
        <source>Fit to a Helmert transform requires at least 2 points.</source>
        <translation>Transformacja Helmerta wymaga wskazania minimum 2 punktów.</translation>
    </message>
    <message>
        <source>Fit to an affine transform requires at least 4 points.</source>
        <translation>Transformacja afiniczna wymaga wskazania minimum 4 punktów.</translation>
    </message>
    <message>
        <source>Couldn&apos;t open the data source: </source>
        <translation>Nie można otworzyć źródła danych: </translation>
    </message>
    <message>
        <source>Parse error at line </source>
        <translation>Analizuj błąd w wierszu</translation>
    </message>
    <message>
        <source>GPS eXchange format provider</source>
        <translation>Format pliku GPS eXchange</translation>
    </message>
    <message>
        <source>Caught a coordinate system exception while trying to transform a point. Unable to calculate line length.</source>
        <translation>Przechwycono wyjątek układu współrzędnych podczas transformacji punktu. Nie można obliczyć długości odcinka.</translation>
    </message>
    <message>
        <source>Caught a coordinate system exception while trying to transform a point. Unable to calculate polygon area.</source>
        <translation>Przechwycono wyjątek układu współrzędnych podczas transformacji punktu. Nie można obliczyć powierzchni poligonu.</translation>
    </message>
    <message>
        <source>GRASS plugin</source>
        <translation>Wtyczka GRASS</translation>
    </message>
    <message>
        <source>QGIS couldn&apos;t find your GRASS installation.
Would you like to specify path (GISBASE) to your GRASS installation?</source>
        <translation>QGIS nie może znaleźć instalacji GRASS.
Czy chcesz wskazać ścieżkę (GISBASE) do Twojej instalacji GRASS?</translation>
    </message>
    <message>
        <source>Choose GRASS installation path (GISBASE)</source>
        <translation>Wskaż ściężkę do instalacji GRASS (GISBASE)</translation>
    </message>
    <message>
        <source>GRASS data won&apos;t be available if GISBASE is not specified.</source>
        <translation>Dane GRASS nie będą dostępne, jeśli nie wskażesz GISBASE.</translation>
    </message>
    <message>
        <source>CopyrightLabel</source>
        <translation>Prawa autorskie</translation>
    </message>
    <message>
        <source>Draws copyright information</source>
        <translation>Wyświetla informacje o prawach autorskich</translation>
    </message>
    <message>
        <source>Version 0.1</source>
        <translation>Wersja 0.1</translation>
    </message>
    <message>
        <source>Version 0.2</source>
        <translation>Wersja 0.2</translation>
    </message>
    <message>
        <source>Loads and displays delimited text files containing x,y coordinates</source>
        <translation>Załaduj i wyświetl plik CSV zawierający współrzędne x,y</translation>
    </message>
    <message>
        <source>Add Delimited Text Layer</source>
        <translation>Dodaj warstwę tekstową CSV</translation>
    </message>
    <message>
        <source>Georeferencer</source>
        <translation>Georeferencer</translation>
    </message>
    <message>
        <source>Adding projection info to rasters</source>
        <translation>Dodaje informację o odwzorowaniu do rastra</translation>
    </message>
    <message>
        <source>GPS Tools</source>
        <translation>Narzędzia GPS</translation>
    </message>
    <message>
        <source>Tools for loading and importing GPS data</source>
        <translation>Narzędzia do importu i eksportu danych GPS</translation>
    </message>
    <message>
        <source>GRASS</source>
        <translation>GRASS</translation>
    </message>
    <message>
        <source>GRASS layer</source>
        <translation>Warstwa GRASS</translation>
    </message>
    <message>
        <source>Graticule Creator</source>
        <translation>Generator siatki kartograficznej</translation>
    </message>
    <message>
        <source>Builds a graticule</source>
        <translation>Generuje siatkę kartograficzną</translation>
    </message>
    <message>
        <source>NorthArrow</source>
        <translation>Strzałka północy</translation>
    </message>
    <message>
        <source>Displays a north arrow overlayed onto the map</source>
        <translation>Wyświetla strzałkę północy w oknie mapy</translation>
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
        <translation>Podziałka</translation>
    </message>
    <message>
        <source>Draws a scale bar</source>
        <translation>Wyświetla podziałkę</translation>
    </message>
    <message>
        <source>SPIT</source>
        <translation>SPIT</translation>
    </message>
    <message>
        <source>Shapefile to PostgreSQL/PostGIS Import Tool</source>
        <translation>Narzędzie importu formatu shape do PostgreSQL/PostGIS (SPIT)</translation>
    </message>
    <message>
        <source>WFS plugin</source>
        <translation>Wtyczka WFS</translation>
    </message>
    <message>
        <source>Adds WFS layers to the QGIS canvas</source>
        <translation>Dodaje warstwy WFS do obszaru mapy QGIS</translation>
    </message>
    <message>
        <source>Not a vector layer</source>
        <translation>Nie warstwa wektorowa</translation>
    </message>
    <message>
        <source>The current layer is not a vector layer</source>
        <translation>Aktywna warstwa nie jest warstwą wektorową</translation>
    </message>
    <message>
        <source>Layer cannot be added to</source>
        <translation>Nie można dodać warstwy do</translation>
    </message>
    <message>
        <source>The data provider for this layer does not support the addition of features.</source>
        <translation>Źródło danych dla tej warstwy nie wspiera dodawania obiektów.</translation>
    </message>
    <message>
        <source>Layer not editable</source>
        <translation>Warstwa nie jest w trybie edycji</translation>
    </message>
    <message>
        <source>Cannot edit the vector layer. To make it editable, go to the file item of the layer, right click and check &apos;Allow Editing&apos;.</source>
        <translation>Nie można edytować warstwy wektorowej. Aby rozpocząć edycję, kliknij prawym przyciskiem myszy na jej nazwie w legendzie i wybierz &apos;Tryb edycji&apos;.</translation>
    </message>
    <message>
        <source>To select features, you must choose a vector layer by clicking on its name in the legend</source>
        <translation>Aby wskazać obiekty, najpierw należy wybrać warstwę wektorową z legendy</translation>
    </message>
    <message>
        <source>Python error</source>
        <translation>Błąd Pythona</translation>
    </message>
    <message>
        <source>Couldn&apos;t load plugin </source>
        <translation>Nie można wczytać wtyczki</translation>
    </message>
    <message>
        <source> due an error when calling its classFactory() method</source>
        <translation>z powodu błędu wywołania jej metody classFactory()</translation>
    </message>
    <message>
        <source> due an error when calling its initGui() method</source>
        <translation>z powodu błędu wywołania jej metody initGui()</translation>
    </message>
    <message>
        <source>Error while unloading plugin </source>
        <translation>Błąd podczas usuwania wtyczki</translation>
    </message>
    <message>
        <source>2.5D shape type not supported</source>
        <translation>Obiekt typu 2.5D nie jest wspierany</translation>
    </message>
    <message>
        <source>Adding features to 2.5D shapetypes is not supported yet</source>
        <translation>Dodawanie obiektów do typów 2.5D nie jest jeszcze wspierane</translation>
    </message>
    <message>
        <source>Wrong editing tool</source>
        <translation>Nieprawidłowe narzędzie do edycji</translation>
    </message>
    <message>
        <source>Cannot apply the &apos;capture point&apos; tool on this vector layer</source>
        <translation>Nie można użyć narzędzia &apos;dodaj punkt&apos; na tej warstwie wektorowej</translation>
    </message>
    <message>
        <source>Coordinate transform error</source>
        <translation>Błąd transformacji współrzędnych</translation>
    </message>
    <message>
        <source>Cannot transform the point to the layers coordinate system</source>
        <translation>Nie można przetransformować punktu do układdu współrzędnych warstwy</translation>
    </message>
    <message>
        <source>Cannot apply the &apos;capture line&apos; tool on this vector layer</source>
        <translation>Nie można użyć narzędzia &apos;dodaj linię&apos; na tej warstwie wektorowej</translation>
    </message>
    <message>
        <source>Cannot apply the &apos;capture polygon&apos; tool on this vector layer</source>
        <translation>Nie można użyć narzędzia &apos;dodaj poligon&apos; na tej warstwie wektorowej</translation>
    </message>
    <message>
        <source>Error</source>
        <translation>Błąd</translation>
    </message>
    <message>
        <source>Cannot add feature. Unknown WKB type</source>
        <translation>Nie można dodać obiektu. Nieznany typ WKB</translation>
    </message>
    <message>
        <source>Error, could not add island</source>
        <translation>Błąd, nie można dodać wyspy</translation>
    </message>
    <message>
        <source>A problem with geometry type occured</source>
        <translation>Wystąpił problem z geometrią</translation>
    </message>
    <message>
        <source>The inserted Ring is not closed</source>
        <translation>Dodany pierścień nie jest zamknięty</translation>
    </message>
    <message>
        <source>The inserted Ring is not a valid geometry</source>
        <translation>Dodany pierścień ma niewłaściwą geometrię</translation>
    </message>
    <message>
        <source>The inserted Ring crosses existing rings</source>
        <translation>Dodany pierścień przecina istniejące pierścienie</translation>
    </message>
    <message>
        <source>The inserted Ring is not contained in a feature</source>
        <translation>Dodany pierścień nie zawiera się w obiekcie</translation>
    </message>
    <message>
        <source>An unknown error occured</source>
        <translation>Wystąpił nieznany błąd</translation>
    </message>
    <message>
        <source>Error, could not add ring</source>
        <translation>Błąd, nie można dodać pierścienia</translation>
    </message>
    <message>
        <source> km2</source>
        <translation>km2</translation>
    </message>
    <message>
        <source> ha</source>
        <translation> ha</translation>
    </message>
    <message>
        <source> m2</source>
        <translation>m2</translation>
    </message>
    <message>
        <source> m</source>
        <translation> m</translation>
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
        <source> sq mile</source>
        <translation>mil2</translation>
    </message>
    <message>
        <source> sq ft</source>
        <translation>stóp2</translation>
    </message>
    <message>
        <source> mile</source>
        <translation>mila</translation>
    </message>
    <message>
        <source> foot</source>
        <translation> stopa</translation>
    </message>
    <message>
        <source> feet</source>
        <translation>stopy</translation>
    </message>
    <message>
        <source> sq.deg.</source>
        <translation>stopni2.</translation>
    </message>
    <message>
        <source> degree</source>
        <translation>stopień</translation>
    </message>
    <message>
        <source> degrees</source>
        <translation>stopnie</translation>
    </message>
    <message>
        <source> unknown</source>
        <translation>nieznany</translation>
    </message>
    <message>
        <source>Received %1 of %2 bytes</source>
        <translation>Odebrano %1 z %2 bajtów</translation>
    </message>
    <message>
        <source>Received %1 bytes (total unknown)</source>
        <translation>Odebrano %1 bajtów (całkowity rozmiar nieznany)</translation>
    </message>
    <message>
        <source>Not connected</source>
        <translation>Nie podłączony</translation>
    </message>
    <message>
        <source>Looking up &apos;%1&apos;</source>
        <translation>Wyszukiwanie &apos;%1&apos;</translation>
    </message>
    <message>
        <source>Connecting to &apos;%1&apos;</source>
        <translation>Podłączanie do &apos;%1&apos;</translation>
    </message>
    <message>
        <source>Sending request &apos;%1&apos;</source>
        <translation>Wysyłanie żądania &apos;%1&apos;</translation>
    </message>
    <message>
        <source>Receiving reply</source>
        <translation>Otrzymywanie odpowiedzi</translation>
    </message>
    <message>
        <source>Response is complete</source>
        <translation>Odpowiedź jest pełna</translation>
    </message>
    <message>
        <source>Closing down connection</source>
        <translation>Zamykanie połączenia</translation>
    </message>
    <message>
        <source>Unable to open </source>
        <translation>Nie można otworzyć</translation>
    </message>
    <message>
        <source>Regular expressions on numeric values don&apos;t make sense. Use comparison instead.</source>
        <translation>Wyrażenia regularne na wartościach numerycznych nie mają sensu. W zamian użyj porównania.</translation>
    </message>
    <message>
        <source>Geoprocessing functions for working with PostgreSQL/PostGIS layers</source>
        <translation>Funkcje geoprzestrzenne do pracy z warstwami PostgreSQL/PostGIS</translation>
    </message>
    <message>
        <source>Location: </source>
        <comment>Metadata in GRASS Browser</comment>
        <translation>Lokacja: </translation>
    </message>
    <message>
        <source>&lt;br&gt;Mapset: </source>
        <comment>Metadata in GRASS Browser</comment>
        <translation>&lt;br&gt;Mapset: </translation>
    </message>
    <message>
        <source>Location: </source>
        <translation>Lokacja: </translation>
    </message>
    <message>
        <source>&lt;br&gt;Mapset: </source>
        <translation>&lt;br&gt;Mapset: </translation>
    </message>
    <message>
        <source>&lt;b&gt;Raster&lt;/b&gt;</source>
        <translation>&lt;b&gt;Raster&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Cannot open raster header</source>
        <translation>Nie można otworzyć nagłówka pliku rastrowego</translation>
    </message>
    <message>
        <source>Rows</source>
        <translation>Wiersze</translation>
    </message>
    <message>
        <source>Columns</source>
        <translation>Kolumny</translation>
    </message>
    <message>
        <source>N-S resolution</source>
        <translation>Rozdzielczość N-S</translation>
    </message>
    <message>
        <source>E-W resolution</source>
        <translation>Rozdzielczość E-W</translation>
    </message>
    <message>
        <source>North</source>
        <translation>Północ</translation>
    </message>
    <message>
        <source>South</source>
        <translation>Południe</translation>
    </message>
    <message>
        <source>East</source>
        <translation>Wschód</translation>
    </message>
    <message>
        <source>West</source>
        <translation>Zachód</translation>
    </message>
    <message>
        <source>Format</source>
        <translation>Format</translation>
    </message>
    <message>
        <source>Minimum value</source>
        <translation>Wartość minimalna</translation>
    </message>
    <message>
        <source>Maximum value</source>
        <translation>Wartość maksymalna</translation>
    </message>
    <message>
        <source>Data source</source>
        <translation>Źródło danych</translation>
    </message>
    <message>
        <source>Data description</source>
        <translation>Opis danych</translation>
    </message>
    <message>
        <source>Comments</source>
        <translation>Komentarze</translation>
    </message>
    <message>
        <source>Categories</source>
        <translation>Kategorie</translation>
    </message>
    <message>
        <source>&lt;b&gt;Vector&lt;/b&gt;</source>
        <translation>&lt;b&gt;Wektor&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Points</source>
        <translation>Punkty</translation>
    </message>
    <message>
        <source>Lines</source>
        <translation>Linie</translation>
    </message>
    <message>
        <source>Boundaries</source>
        <translation>Granice</translation>
    </message>
    <message>
        <source>Centroids</source>
        <translation>Centroidy</translation>
    </message>
    <message>
        <source>Faces</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Kernels</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Areas</source>
        <translation>Powierzchnie</translation>
    </message>
    <message>
        <source>Islands</source>
        <translation>Wyspy</translation>
    </message>
    <message>
        <source>Top</source>
        <translation>Góra</translation>
    </message>
    <message>
        <source>Bottom</source>
        <translation>Dół</translation>
    </message>
    <message>
        <source>yes</source>
        <translation>tak</translation>
    </message>
    <message>
        <source>no</source>
        <translation>nie</translation>
    </message>
    <message>
        <source>History&lt;br&gt;</source>
        <translation>Historia&lt;br&gt;</translation>
    </message>
    <message>
        <source>&lt;b&gt;Layer&lt;/b&gt;</source>
        <translation>&lt;b&gt;Warstwa&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Features</source>
        <translation>Obiekty</translation>
    </message>
    <message>
        <source>Driver</source>
        <translation>Sterownik</translation>
    </message>
    <message>
        <source>Database</source>
        <translation>Baza danych</translation>
    </message>
    <message>
        <source>Table</source>
        <translation>Tabela</translation>
    </message>
    <message>
        <source>Key column</source>
        <translation>Kolumna z kluczem</translation>
    </message>
    <message>
        <source>GISBASE is not set.</source>
        <translation>GISBASE nie jest zdefiniowany.</translation>
    </message>
    <message>
        <source> is not a GRASS mapset.</source>
        <translation>nie jest mapsetem GRASS.</translation>
    </message>
    <message>
        <source>Cannot start </source>
        <translation>Nie można uruchomić</translation>
    </message>
    <message>
        <source>Mapset is already in use.</source>
        <translation>Mapset jest już w użyciu.</translation>
    </message>
    <message>
        <source>Temporary directory </source>
        <translation>Katalog tymczasowy</translation>
    </message>
    <message>
        <source> exist but is not writable</source>
        <translation>istnieje ale niemożliwy jest zapis</translation>
    </message>
    <message>
        <source>Cannot create temporary directory </source>
        <translation>Nie można utworzyć katalogu tymczasowego</translation>
    </message>
    <message>
        <source>Cannot create </source>
        <translation>Nie można utworzyć</translation>
    </message>
    <message>
        <source>Cannot remove mapset lock: </source>
        <translation>Nie można usunąć blokady mapsetu:</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Ostrzeżenie</translation>
    </message>
    <message>
        <source>Cannot read raster map region</source>
        <translation>Nie można odczytać regionu mapy rastrowej</translation>
    </message>
    <message>
        <source>Cannot read vector map region</source>
        <translation>Nie można odczytać regionu mapy wektorowej</translation>
    </message>
    <message>
        <source>Cannot read region</source>
        <translation>Nie można odczytać regionu</translation>
    </message>
    <message>
        <source>Where is &apos;</source>
        <translation>Gdzie jest &apos;</translation>
    </message>
    <message>
        <source>original location: </source>
        <translation>oryginalna lokacja:</translation>
    </message>
    <message>
        <source>To identify features, you must choose an active layer by clicking on its name in the legend</source>
        <translation>Aby wybrać obiekty, najpierw należy wybrać warstwę wektorową z legendy</translation>
    </message>
    <message>
        <source>PostgreSQL Geoprocessing</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Quick Print</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Quick Print is a plugin to quickly print a map with minimal effort.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Could not remove polygon intersection</source>
        <translation>Nie można zlikwidować przecięcia poligonów</translation>
    </message>
    <message>
        <source>The directory containing your dataset needs to be writeable!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Created default style file as </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Couldn&apos;t load SIP module.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Python support will be disabled.</source>
        <translation>Wsparcie dla Pythona zostanie wyłączone.</translation>
    </message>
    <message>
        <source>Couldn&apos;t load PyQt4.</source>
        <translation>Nie mogę załadować PyQt4.</translation>
    </message>
    <message>
        <source>Couldn&apos;t load PyQGIS.</source>
        <translation>Nie mogę załadować PyQGIS.</translation>
    </message>
    <message>
        <source>An error occured during execution of following code:</source>
        <translation>Wystąpił błąd podczas wykonywania następującego kodu:</translation>
    </message>
    <message>
        <source> is not writeable.</source>
        <translation>nie umożliwia zapisu.</translation>
    </message>
    <message>
        <source>Please adjust permissions (if possible) and try again.</source>
        <translation>Dostosuj uprawnienia (jeśli to możliwe) i spróbuj ponownie.</translation>
    </message>
    <message>
        <source>Uncatched fatal GRASS error</source>
        <translation>Nieobsługiwany błąd krytyczny GRASSa</translation>
    </message>
    <message>
        <source>ERROR: Failed to created default style file as %1 Check file permissions and retry.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>An error has occured while executing Python code:</source>
        <translation>Wystąpił błąd podczas wykonywania kodu Pythona:</translation>
    </message>
    <message>
        <source>Python version:</source>
        <translation>Wersja Pythona:</translation>
    </message>
    <message>
        <source>Python path:</source>
        <translation>Lokalizacja Pythona:</translation>
    </message>
    <message>
        <source>Legend</source>
        <translation>Legenda</translation>
    </message>
    <message>
        <source>Coordinate Capture</source>
        <translation>Przechwyć współrzędne</translation>
    </message>
    <message>
        <source>Capture mouse coordinates in different CRS</source>
        <translation>Przechwyć współrzędne myszy w innym SOW</translation>
    </message>
    <message>
        <source>Dxf2Shp Converter</source>
        <translation>Konwerter Dxf2Shp</translation>
    </message>
    <message>
        <source>Converts from dxf to shp file format</source>
        <translation>Konwertuje plik DXF do formatu shape</translation>
    </message>
    <message>
        <source>Interpolating...</source>
        <translation>Interpoluję...</translation>
    </message>
    <message>
        <source>Abort</source>
        <translation>Anuluj</translation>
    </message>
    <message>
        <source>Interpolation plugin</source>
        <translation>Wtyczka interpolacji</translation>
    </message>
    <message>
        <source>A plugin for interpolation based on vertices of a vector layer</source>
        <translation>Wtyczka do interpolacji w oparciu o wierzchołki z warstwy wektorowej</translation>
    </message>
    <message>
        <source>Version 0.001</source>
        <translation>Wersja 0.001</translation>
    </message>
    <message>
        <source>OGR Layer Converter</source>
        <translation>Konwerter warstw OGR</translation>
    </message>
    <message>
        <source>Translates vector layers between formats supported by OGR library</source>
        <translation>Konwertuje warstwy wektorowe pomiędzy formatami wspieranymi przez bibliotekę OGR</translation>
    </message>
    <message>
        <source>CRS Exception</source>
        <translation>Wyjątek SOW</translation>
    </message>
    <message>
        <source>Selection extends beyond layer&apos;s coordinate system.</source>
        <translation>Wybór wykracza poza układ współrzędnych warstwy.</translation>
    </message>
    <message>
        <source>Loading style file </source>
        <translation>Wczytywanie pliku stylów</translation>
    </message>
    <message>
        <source> failed because:</source>
        <translation>nie powiodło się ponieważ:</translation>
    </message>
    <message>
        <source>Could not save symbology because:</source>
        <translation>Nie można zapisać symboliki ponieważ:</translation>
    </message>
    <message>
        <source>Unable to save to file. Your project may be corrupted on disk. Try clearing some space on the volume and check file permissions before pressing save again.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Error Loading Plugin</source>
        <translation>Błąd podczas wczytywania wtyczki</translation>
    </message>
    <message>
        <source>There was an error loading a plugin.The following diagnostic information may help the QGIS developers resolve the issue:
%1.</source>
        <translation>Wystąpił błąd podczas ładowania wtyczki. Poniższa informacja diagnostyczna może pomóc programistom QGIS rozwiązać ten problem:
%1.</translation>
    </message>
    <message>
        <source>Error when reading metadata of plugin </source>
        <translation>Błąd podczas odczytywania metadanych wtyczki </translation>
    </message>
</context>
<context>
    <name>QgisApp</name>
    <message>
        <source>Quantum GIS - </source>
        <translation>Quantum GIS - </translation>
    </message>
    <message>
        <source>Map legend that displays all the layers currently on the map canvas. Click on the check box to turn a layer on or off. Double click on a layer in the legend to customize its appearance and set other properties.</source>
        <translation>Legenda mapy zawiera wszystkie aktualnie wyświetlane warstwy mapy. Zaznacz pole wyboru przy nazwie, aby włączyć lub wyłączyć warstwę. Podwójne kliknięcie otwiera okno do edycji parametrów wyświetlania i innych właściwości warstwy.</translation>
    </message>
    <message>
        <source>Map overview canvas. This canvas can be used to display a locator map that shows the current extent of the map canvas. The current extent is shown as a red rectangle. Any layer on the map can be added to the overview canvas.</source>
        <translation>Obszar podglądu mapy. W tym obszarze wyświetlana jest lokalizacja i zasięg aktualnie wyświetlanego obszaru mapy. Bieżący zasięg zaznaczony jest czerwonym prostokątem. Do obszaru podglądu można dodać każdą warstwę z mapy.</translation>
    </message>
    <message>
        <source>Map canvas. This is where raster and vector layers are displayed when added to the map</source>
        <translation>Obszar mapy. Tutaj wyświetlane są warstwy rastrowe i wektorowe, które zostały dodane do mapy</translation>
    </message>
    <message>
        <source>Progress bar that displays the status of rendering layers and other time-intensive operations</source>
        <translation>Pasek postępu informuje o postępie rysowania warstw oraz innych czasochłonnych operacji</translation>
    </message>
    <message>
        <source>Displays the current map scale</source>
        <translation>Wyświetla bieżącą skalę mapy</translation>
    </message>
    <message>
        <source>Render</source>
        <translation>Renderuj</translation>
    </message>
    <message>
        <source>When checked, the map layers are rendered in response to map navigation commands and other events. When not checked, no rendering is done. This allows you to add a large number of layers and symbolize them before rendering.</source>
        <translation>Jeśli pole jest zaznaczone, to warstwy są odrysowywane na bieżąco w odpowiedzi na użycie narzędzi nawigacyjnych i inne zdarzenia. Umożliwia to dodanie wielu warstw i określenie dla nich symboli przed odrysowaniem.</translation>
    </message>
    <message>
        <source>Version</source>
        <translation>Wersja</translation>
    </message>
    <message>
        <source>is not a valid or recognized data source</source>
        <translation>nie jest poprawnym lub rozpoznawanym źródłem danych</translation>
    </message>
    <message>
        <source>Invalid Data Source</source>
        <translation>Niepoprawne źródło danych</translation>
    </message>
    <message>
        <source>Invalid Layer</source>
        <translation>Niepoprawna warstwa</translation>
    </message>
    <message>
        <source>%1 is an invalid layer and cannot be loaded.</source>
        <translation>%1 nie jest poprawną warstwą i nie może zostać wczytana.</translation>
    </message>
    <message>
        <source>Choose a QGIS project file</source>
        <translation>Podaj nazwę pliku projektu QGIS</translation>
    </message>
    <message>
        <source>Unable to save project</source>
        <translation>Nie można zapisać projektu</translation>
    </message>
    <message>
        <source>Unable to save project to </source>
        <translation>Nie można zapisać projektu w </translation>
    </message>
    <message>
        <source>Choose a filename to save the map image as</source>
        <translation type="obsolete">Podaj nazwę pliku do którego zapisać obraz mapy</translation>
    </message>
    <message>
        <source>Saved map image to</source>
        <translation>Obraz mapy zapisany do</translation>
    </message>
    <message>
        <source>No Layer Selected</source>
        <translation>Nie wybrano warstwy</translation>
    </message>
    <message>
        <source>Problem deleting features</source>
        <translation>Problem podczas usuwania obiektów</translation>
    </message>
    <message>
        <source>A problem occured during deletion of features</source>
        <translation>Wystąpił problem podczas usuwania obiektów</translation>
    </message>
    <message>
        <source>No Vector Layer Selected</source>
        <translation>Nie wybrano warstwy wektorowej</translation>
    </message>
    <message>
        <source>Deleting features only works on vector layers</source>
        <translation>Usuwanie obiektów jest możliwe tylko z warstw wektorowych</translation>
    </message>
    <message>
        <source>To delete features, you must select a vector layer in the legend</source>
        <translation>Aby usunąć obiekt, najpierw należy wybrać warstwę wektorową z legendy</translation>
    </message>
    <message>
        <source>Error Loading Plugin</source>
        <translation type="obsolete">Błąd podczas wczytywania wtyczki</translation>
    </message>
    <message>
        <source>There was an error loading %1.</source>
        <translation type="obsolete">Wystąpił błąd podczas wczytywania %1.</translation>
    </message>
    <message>
        <source>No MapLayer Plugins</source>
        <translation type="obsolete">Brak wtyczek MapLayers</translation>
    </message>
    <message>
        <source>No MapLayer plugins in ../plugins/maplayer</source>
        <translation type="obsolete">Nie znaleziono wtyczek MapLayer w ../plugins/maplayer</translation>
    </message>
    <message>
        <source>No Plugins</source>
        <translation type="obsolete">Brak wtyczek</translation>
    </message>
    <message>
        <source>No plugins found in ../plugins. To test plugins, start qgis from the src directory</source>
        <translation type="obsolete">Nie znaleziono wtyczek w ../plugins. Aby przetestować wtyczki, uruchom qgis z katalogu src</translation>
    </message>
    <message>
        <source>Name</source>
        <translation type="obsolete">Nazwa</translation>
    </message>
    <message>
        <source>Plugin %1 is named %2</source>
        <translation type="obsolete">Wtyczka %1 jest nazwana %2</translation>
    </message>
    <message>
        <source>Plugin Information</source>
        <translation type="obsolete">Informacje o wtyczce</translation>
    </message>
    <message>
        <source>QGis loaded the following plugin:</source>
        <translation type="obsolete">QGIS wczytał następującą wtyczkę:</translation>
    </message>
    <message>
        <source>Name: %1</source>
        <translation type="obsolete">Nazwa: %1</translation>
    </message>
    <message>
        <source>Version: %1</source>
        <translation type="obsolete">Wersja: %1</translation>
    </message>
    <message>
        <source>Description: %1</source>
        <translation type="obsolete">Opis: %1</translation>
    </message>
    <message>
        <source>Unable to Load Plugin</source>
        <translation type="obsolete">Nie można wczytać wtyczki</translation>
    </message>
    <message>
        <source>QGIS was unable to load the plugin from: %1</source>
        <translation type="obsolete">QGIS nie może wczytać wtyczki z: %1</translation>
    </message>
    <message>
        <source>There is a new version of QGIS available</source>
        <translation>Dostępna jest nowa wersja QGIS</translation>
    </message>
    <message>
        <source>You are running a development version of QGIS</source>
        <translation>Pracujesz z wersją rozwojową QGIS</translation>
    </message>
    <message>
        <source>You are running the current version of QGIS</source>
        <translation>Pracujesz z aktualną wersją QGIS</translation>
    </message>
    <message>
        <source>Would you like more information?</source>
        <translation>Czy chcesz otrzymać więcej informacji?</translation>
    </message>
    <message>
        <source>QGIS Version Information</source>
        <translation>Informacje o wersji QGIS</translation>
    </message>
    <message>
        <source>Unable to get current version information from server</source>
        <translation>Nie można pobrać z serwera informacji o aktualnej wersji</translation>
    </message>
    <message>
        <source>Connection refused - server may be down</source>
        <translation>Połączenie odrzucone - serwer może być wyłączony</translation>
    </message>
    <message>
        <source>QGIS server was not found</source>
        <translation>Nie można odnaleźć serwera QGIS</translation>
    </message>
    <message>
        <source>Extents: </source>
        <translation>Zasięg: </translation>
    </message>
    <message>
        <source>&amp;Plugins</source>
        <translation>&amp;Wtyczki</translation>
    </message>
    <message>
        <source>Toggle map rendering</source>
        <translation>Przełącz renderowanie mapy</translation>
    </message>
    <message>
        <source>Open an OGR Supported Vector Layer</source>
        <translation>Otwórz warstwę wektorową OGR</translation>
    </message>
    <message>
        <source>Save As</source>
        <translation>Zapisz jako</translation>
    </message>
    <message>
        <source>Choose a QGIS project file to open</source>
        <translation>Wybierz plik projektu QGIS do otwarcia</translation>
    </message>
    <message>
        <source>QGIS Project Read Error</source>
        <translation>Błąd wczytywania projektu QGIS</translation>
    </message>
    <message>
        <source>Try to find missing layers?</source>
        <translation>Próbować znaleźć brakujące warstwy?</translation>
    </message>
    <message>
        <source>Open a GDAL Supported Raster Data Source</source>
        <translation>Otwórz rastrowe źródło danych GDAL</translation>
    </message>
    <message>
        <source>Reading settings</source>
        <translation>Czytam ustawienia</translation>
    </message>
    <message>
        <source>Setting up the GUI</source>
        <translation>Ustawienia GUI</translation>
    </message>
    <message>
        <source>Checking database</source>
        <translation>Sprawdzanie bazy danych</translation>
    </message>
    <message>
        <source>Restoring loaded plugins</source>
        <translation>Przywracanie wczytanych wtyczek</translation>
    </message>
    <message>
        <source>Initializing file filters</source>
        <translation>Inicjalizacja filtrów plików</translation>
    </message>
    <message>
        <source>Restoring window state</source>
        <translation>Przywracanie stanu okna</translation>
    </message>
    <message>
        <source>QGIS Ready!</source>
        <translation>QGIS gotowy!</translation>
    </message>
    <message>
        <source>&amp;New Project</source>
        <translation>&amp;Nowy projekt</translation>
    </message>
    <message>
        <source>Ctrl+N</source>
        <comment>New Project</comment>
        <translation>Ctrl+N</translation>
    </message>
    <message>
        <source>New Project</source>
        <translation>Nowy projekt</translation>
    </message>
    <message>
        <source>&amp;Open Project...</source>
        <translation>&amp;Otwórz projekt...</translation>
    </message>
    <message>
        <source>Ctrl+O</source>
        <comment>Open a Project</comment>
        <translation>Ctrl+O</translation>
    </message>
    <message>
        <source>Open a Project</source>
        <translation>Otwórz projekt</translation>
    </message>
    <message>
        <source>&amp;Save Project</source>
        <translation>&amp;Zapisz projekt</translation>
    </message>
    <message>
        <source>Ctrl+S</source>
        <comment>Save Project</comment>
        <translation>Ctrl+S</translation>
    </message>
    <message>
        <source>Save Project</source>
        <translation>Zapisz projekt</translation>
    </message>
    <message>
        <source>Save Project &amp;As...</source>
        <translation>Z&amp;apisz projekt jako...</translation>
    </message>
    <message>
        <source>Ctrl+A</source>
        <comment>Save Project under a new name</comment>
        <translation type="obsolete">Ctrl+A

Zapisz projekt pod nową nazwą</translation>
    </message>
    <message>
        <source>Save Project under a new name</source>
        <translation>Zapisz projekt pod nową nazwą</translation>
    </message>
    <message>
        <source>&amp;Print...</source>
        <translation type="obsolete">&amp;Drukuj...</translation>
    </message>
    <message>
        <source>Ctrl+P</source>
        <comment>Print</comment>
        <translation type="obsolete">Ctrl+P</translation>
    </message>
    <message>
        <source>Print</source>
        <translation type="obsolete">Drukuj</translation>
    </message>
    <message>
        <source>Save as Image...</source>
        <translation>Zapisz jako obraz...</translation>
    </message>
    <message>
        <source>Ctrl+I</source>
        <comment>Save map as image</comment>
        <translation type="obsolete">Ctrl+I</translation>
    </message>
    <message>
        <source>Save map as image</source>
        <translation>Zapisz mapę jako obraz</translation>
    </message>
    <message>
        <source>Exit</source>
        <translation>Zakończ</translation>
    </message>
    <message>
        <source>Ctrl+Q</source>
        <comment>Exit QGIS</comment>
        <translation>Ctrl+Q</translation>
    </message>
    <message>
        <source>Exit QGIS</source>
        <translation>Wyjdź z QGIS</translation>
    </message>
    <message>
        <source>Add a Vector Layer...</source>
        <translation type="obsolete">Dodaj warstwę wektorową...</translation>
    </message>
    <message>
        <source>V</source>
        <comment>Add a Vector Layer</comment>
        <translation>V</translation>
    </message>
    <message>
        <source>Add a Vector Layer</source>
        <translation>Dodaj warstwę wektorową</translation>
    </message>
    <message>
        <source>Add a Raster Layer...</source>
        <translation type="obsolete">Dodaj warstwę rastrową...</translation>
    </message>
    <message>
        <source>R</source>
        <comment>Add a Raster Layer</comment>
        <translation>R</translation>
    </message>
    <message>
        <source>Add a Raster Layer</source>
        <translation>Dodaj warstwę rastrową</translation>
    </message>
    <message>
        <source>Add a PostGIS Layer...</source>
        <translation type="obsolete">Dodaj warstwę PostGIS...</translation>
    </message>
    <message>
        <source>D</source>
        <comment>Add a PostGIS Layer</comment>
        <translation>D</translation>
    </message>
    <message>
        <source>Add a PostGIS Layer</source>
        <translation>Dodaj warstwę PostGIS</translation>
    </message>
    <message>
        <source>New Vector Layer...</source>
        <translation>Nowa warstwa wektorowa...</translation>
    </message>
    <message>
        <source>N</source>
        <comment>Create a New Vector Layer</comment>
        <translation>N</translation>
    </message>
    <message>
        <source>Create a New Vector Layer</source>
        <translation>Utwórz nową warstwę wektorową</translation>
    </message>
    <message>
        <source>Remove Layer</source>
        <translation>Usuń warstwę</translation>
    </message>
    <message>
        <source>Ctrl+D</source>
        <comment>Remove a Layer</comment>
        <translation>Ctrl+D</translation>
    </message>
    <message>
        <source>Remove a Layer</source>
        <translation>Usuń warstwę</translation>
    </message>
    <message>
        <source>Add All To Overview</source>
        <translation type="obsolete">Dodaj wszystkie do podglądu</translation>
    </message>
    <message>
        <source>+</source>
        <comment>Show all layers in the overview map</comment>
        <translation>+</translation>
    </message>
    <message>
        <source>Show all layers in the overview map</source>
        <translation>Pokaż wszystkie warstwy w podglądzie</translation>
    </message>
    <message>
        <source>Remove All From Overview</source>
        <translation>Usuń wszystkie z podglądu</translation>
    </message>
    <message>
        <source>-</source>
        <comment>Remove all layers from overview map</comment>
        <translation>-</translation>
    </message>
    <message>
        <source>Remove all layers from overview map</source>
        <translation>Usuń wszystkie warstwy z podglądu</translation>
    </message>
    <message>
        <source>Show All Layers</source>
        <translation>Pokaż wszystkie</translation>
    </message>
    <message>
        <source>S</source>
        <comment>Show all layers</comment>
        <translation>S</translation>
    </message>
    <message>
        <source>Show all layers</source>
        <translation>Pokaż wszystkie</translation>
    </message>
    <message>
        <source>Hide All Layers</source>
        <translation>Ukryj wszystkie</translation>
    </message>
    <message>
        <source>H</source>
        <comment>Hide all layers</comment>
        <translation>H</translation>
    </message>
    <message>
        <source>Hide all layers</source>
        <translation>Ukryj wszystkie</translation>
    </message>
    <message>
        <source>Project Properties...</source>
        <translation>Właściwości projektu...</translation>
    </message>
    <message>
        <source>P</source>
        <comment>Set project properties</comment>
        <translation>P</translation>
    </message>
    <message>
        <source>Set project properties</source>
        <translation>Ustaw właściwości projektu</translation>
    </message>
    <message>
        <source>Options...</source>
        <translation>Opcje...</translation>
    </message>
    <message>
        <source>Change various QGIS options</source>
        <translation>Zmień opcje QGIS</translation>
    </message>
    <message>
        <source>Custom Projection...</source>
        <translation type="obsolete">Własny układ współrzędnych...</translation>
    </message>
    <message>
        <source>Manage custom projections</source>
        <translation type="obsolete">Zarządzanie własnymi układami współrzędnych</translation>
    </message>
    <message>
        <source>Help Contents</source>
        <translation>Zawartość pliku pomocy</translation>
    </message>
    <message>
        <source>F1</source>
        <comment>Help Documentation</comment>
        <translation>F1</translation>
    </message>
    <message>
        <source>Help Documentation</source>
        <translation>Pomoc</translation>
    </message>
    <message>
        <source>Qgis Home Page</source>
        <translation type="obsolete">Strona domowa QGIS</translation>
    </message>
    <message>
        <source>Ctrl+H</source>
        <comment>QGIS Home Page</comment>
        <translation>Ctrl+H</translation>
    </message>
    <message>
        <source>QGIS Home Page</source>
        <translation>Strona domowa QGIS</translation>
    </message>
    <message>
        <source>About</source>
        <translation>O QGIS</translation>
    </message>
    <message>
        <source>About QGIS</source>
        <translation>O QGIS</translation>
    </message>
    <message>
        <source>Check Qgis Version</source>
        <translation>Sprawdź wersję Qgis</translation>
    </message>
    <message>
        <source>Check if your QGIS version is up to date (requires internet access)</source>
        <translation>Sprawdź aktualność Twojej wersji QGIS (wymagane połączenie internetowe)</translation>
    </message>
    <message>
        <source>Refresh</source>
        <translation>Odśwież</translation>
    </message>
    <message>
        <source>Ctrl+R</source>
        <comment>Refresh Map</comment>
        <translation>Ctrl+R</translation>
    </message>
    <message>
        <source>Refresh Map</source>
        <translation>Odśwież mapę</translation>
    </message>
    <message>
        <source>Zoom In</source>
        <translation>Powiększ</translation>
    </message>
    <message>
        <source>Ctrl++</source>
        <comment>Zoom In</comment>
        <translation>Ctrl++</translation>
    </message>
    <message>
        <source>Zoom Out</source>
        <translation>Pomniejsz</translation>
    </message>
    <message>
        <source>Ctrl+-</source>
        <comment>Zoom Out</comment>
        <translation>Ctrl+-</translation>
    </message>
    <message>
        <source>Zoom Full</source>
        <translation>Cały zasięg</translation>
    </message>
    <message>
        <source>F</source>
        <comment>Zoom to Full Extents</comment>
        <translation>F</translation>
    </message>
    <message>
        <source>Zoom to Full Extents</source>
        <translation>Powiększ do pełnego zasięgu</translation>
    </message>
    <message>
        <source>Zoom To Selection</source>
        <translation type="obsolete">Powiększ do zaznaczonych</translation>
    </message>
    <message>
        <source>Ctrl+F</source>
        <comment>Zoom to selection</comment>
        <translation type="obsolete">Ctrl+F</translation>
    </message>
    <message>
        <source>Zoom to selection</source>
        <translation type="obsolete">Powiększ do zaznaczonych</translation>
    </message>
    <message>
        <source>Pan Map</source>
        <translation>Przesuwanie</translation>
    </message>
    <message>
        <source>Pan the map</source>
        <translation>Przesuwanie mapy</translation>
    </message>
    <message>
        <source>Zoom Last</source>
        <translation>Poprzedni widok</translation>
    </message>
    <message>
        <source>Zoom to Last Extent</source>
        <translation>Przejdź do ostatniego widoku</translation>
    </message>
    <message>
        <source>Zoom To Layer</source>
        <translation type="obsolete">Powiększ do zasięgu warstwy</translation>
    </message>
    <message>
        <source>Zoom to Layer</source>
        <translation>Powiększ do zasięgu warstwy</translation>
    </message>
    <message>
        <source>Identify Features</source>
        <translation>Informacje o obiekcie</translation>
    </message>
    <message>
        <source>I</source>
        <comment>Click on features to identify them</comment>
        <translation>I</translation>
    </message>
    <message>
        <source>Click on features to identify them</source>
        <translation>Kliknij na obiekcie by uzyskać o nim informacje</translation>
    </message>
    <message>
        <source>Select Features</source>
        <translation>Wybierz obiekty</translation>
    </message>
    <message>
        <source>Open Table</source>
        <translation type="obsolete">Otwórz tabelę</translation>
    </message>
    <message>
        <source>Measure Line </source>
        <translation>Pomiar odległości</translation>
    </message>
    <message>
        <source>Ctrl+M</source>
        <comment>Measure a Line</comment>
        <translation type="obsolete">Ctrl+M</translation>
    </message>
    <message>
        <source>Measure a Line</source>
        <translation>Pomiar odległości</translation>
    </message>
    <message>
        <source>Measure Area</source>
        <translation>Pomiar powierzchni</translation>
    </message>
    <message>
        <source>Ctrl+J</source>
        <comment>Measure an Area</comment>
        <translation type="obsolete">Ctrl+J</translation>
    </message>
    <message>
        <source>Measure an Area</source>
        <translation>Pomiar powierzchni</translation>
    </message>
    <message>
        <source>Show Bookmarks</source>
        <translation>Pokaż zakładki</translation>
    </message>
    <message>
        <source>B</source>
        <comment>Show Bookmarks</comment>
        <translation>B</translation>
    </message>
    <message>
        <source>New Bookmark...</source>
        <translation>Nowa zakładka...</translation>
    </message>
    <message>
        <source>Ctrl+B</source>
        <comment>New Bookmark</comment>
        <translation>Ctrl+B</translation>
    </message>
    <message>
        <source>New Bookmark</source>
        <translation>Nowa zakładka</translation>
    </message>
    <message>
        <source>Add WMS Layer...</source>
        <translation>Dodaj warstwę WMS...</translation>
    </message>
    <message>
        <source>W</source>
        <comment>Add Web Mapping Server Layer</comment>
        <translation type="obsolete">W</translation>
    </message>
    <message>
        <source>Add Web Mapping Server Layer</source>
        <translation type="obsolete">Dodaj warstwę Web Mapping Server</translation>
    </message>
    <message>
        <source>In Overview</source>
        <translation type="obsolete">Podgląd</translation>
    </message>
    <message>
        <source>O</source>
        <comment>Add current layer to overview map</comment>
        <translation>O</translation>
    </message>
    <message>
        <source>Add current layer to overview map</source>
        <translation>Dodaj aktualną warstwę do podglądu</translation>
    </message>
    <message>
        <source>Plugin Manager...</source>
        <translation type="obsolete">Menedżer wtyczek...</translation>
    </message>
    <message>
        <source>Open the plugin manager</source>
        <translation>Otwórz menedżer wtyczek</translation>
    </message>
    <message>
        <source>Capture Point</source>
        <translation>Dodaj punkt</translation>
    </message>
    <message>
        <source>.</source>
        <comment>Capture Points</comment>
        <translation>.</translation>
    </message>
    <message>
        <source>Capture Points</source>
        <translation>Dodaj punkty</translation>
    </message>
    <message>
        <source>Capture Line</source>
        <translation>Dodaj linię</translation>
    </message>
    <message>
        <source>/</source>
        <comment>Capture Lines</comment>
        <translation>/</translation>
    </message>
    <message>
        <source>Capture Lines</source>
        <translation>Dodaj linie</translation>
    </message>
    <message>
        <source>Capture Polygon</source>
        <translation>Dodaj poligon</translation>
    </message>
    <message>
        <source>Ctrl+/</source>
        <comment>Capture Polygons</comment>
        <translation>Ctrl+/</translation>
    </message>
    <message>
        <source>Capture Polygons</source>
        <translation>Dodaj poligony</translation>
    </message>
    <message>
        <source>Delete Selected</source>
        <translation>Usuń zaznaczone</translation>
    </message>
    <message>
        <source>Add Vertex</source>
        <translation>Dodaj wierzchołek</translation>
    </message>
    <message>
        <source>Delete Vertex</source>
        <translation>Usuń wierzchołek</translation>
    </message>
    <message>
        <source>Move Vertex</source>
        <translation>Przesuń wierzchołek</translation>
    </message>
    <message>
        <source>&amp;File</source>
        <translation>&amp;Plik</translation>
    </message>
    <message>
        <source>&amp;Open Recent Projects</source>
        <translation>&amp;Otwórz ostatnie projekty</translation>
    </message>
    <message>
        <source>&amp;View</source>
        <translation>&amp;Widok</translation>
    </message>
    <message>
        <source>&amp;Layer</source>
        <translation>W&amp;arstwa</translation>
    </message>
    <message>
        <source>&amp;Settings</source>
        <translation>&amp;Ustawienia</translation>
    </message>
    <message>
        <source>&amp;Help</source>
        <translation>&amp;Pomoc</translation>
    </message>
    <message>
        <source>File</source>
        <translation>Plik</translation>
    </message>
    <message>
        <source>Manage Layers</source>
        <translation>Zarządzaj warstwami</translation>
    </message>
    <message>
        <source>Help</source>
        <translation>Pomoc</translation>
    </message>
    <message>
        <source>Digitizing</source>
        <translation>Digitalizacja</translation>
    </message>
    <message>
        <source>Map Navigation</source>
        <translation>Nawigacja mapy</translation>
    </message>
    <message>
        <source>Attributes</source>
        <translation>Atrybuty</translation>
    </message>
    <message>
        <source>Plugins</source>
        <translation>Wtyczki</translation>
    </message>
    <message>
        <source>Ready</source>
        <translation>Gotowe</translation>
    </message>
    <message>
        <source>New features</source>
        <translation>Nowości</translation>
    </message>
    <message>
        <source>Unable to open project</source>
        <translation>Nie można otworzyć projektu</translation>
    </message>
    <message>
        <source>Unable to save project </source>
        <translation>Nie można zapisać projektu</translation>
    </message>
    <message>
        <source>Choose a filename to save the QGIS project file as</source>
        <translation type="obsolete">Wybierz nazwę by zapisać projekt QGIS jako</translation>
    </message>
    <message>
        <source>QGIS: Unable to load project</source>
        <translation>QGIS: Nie mogę wczytać projektu</translation>
    </message>
    <message>
        <source>Unable to load project </source>
        <translation>Nie mogę wczytać projektu</translation>
    </message>
    <message>
        <source>QGIS - Changes in SVN Since Last Release</source>
        <translation>QGIS - zmiany w SVN od ostatniego wydania</translation>
    </message>
    <message>
        <source>Layer is not valid</source>
        <translation>Warstwa jest nieobsługiwana</translation>
    </message>
    <message>
        <source>The layer is not a valid layer and can not be added to the map</source>
        <translation>Warstwa jest nieobsługiwana i nie może być dodana do mapy</translation>
    </message>
    <message>
        <source>Save?</source>
        <translation>Zapisać?</translation>
    </message>
    <message>
        <source>Clipboard contents set to: </source>
        <translation type="obsolete">Zawartość schowka ustawiona na: </translation>
    </message>
    <message>
        <source> is not a valid or recognized raster data source</source>
        <translation> jest nieprawidłowym lub nierozpoznanym rastrowym źródłem danych</translation>
    </message>
    <message>
        <source> is not a supported raster data source</source>
        <translation> jest nieobsługiwanym rastrowym źródłem danych</translation>
    </message>
    <message>
        <source>Unsupported Data Source</source>
        <translation>Nieobsługiwane źródło danych</translation>
    </message>
    <message>
        <source>Enter a name for the new bookmark:</source>
        <translation>Wpisz nazwę nowej zakładki:</translation>
    </message>
    <message>
        <source>Error</source>
        <translation>Błąd</translation>
    </message>
    <message>
        <source>Unable to create the bookmark. Your user database may be missing or corrupted</source>
        <translation>Nie mogę stworzyć zakładki. Twoja baza danych użytkownika nie istnieje lub jest uszkodzona</translation>
    </message>
    <message>
        <source>Ctrl+?</source>
        <comment>Help Documentation (Mac)</comment>
        <translation>Ctrl+?</translation>
    </message>
    <message>
        <source>Show most toolbars</source>
        <translation type="obsolete">Pokaż paski narzędzi</translation>
    </message>
    <message>
        <source>Hide most toolbars</source>
        <translation type="obsolete">Ukryj paski narzędzi</translation>
    </message>
    <message>
        <source>Cut Features</source>
        <translation>Wytnij obiekty</translation>
    </message>
    <message>
        <source>Cut selected features</source>
        <translation>Wytnij zaznaczone obiekty</translation>
    </message>
    <message>
        <source>Copy Features</source>
        <translation>Kopiuj obiekty</translation>
    </message>
    <message>
        <source>Copy selected features</source>
        <translation>Kopiuj zaznaczone obiekty</translation>
    </message>
    <message>
        <source>Paste Features</source>
        <translation>Wklej obiekty</translation>
    </message>
    <message>
        <source>Paste selected features</source>
        <translation>Wklej zaznaczone obiekty</translation>
    </message>
    <message>
        <source>T</source>
        <comment>Show most toolbars</comment>
        <translation type="obsolete">T</translation>
    </message>
    <message>
        <source>Checking provider plugins</source>
        <translation>Sprawdzanie źródeł wtyczek</translation>
    </message>
    <message>
        <source>Starting Python</source>
        <translation>Uruchamianie Pythona</translation>
    </message>
    <message>
        <source>Python console</source>
        <translation type="obsolete">Konsola Python</translation>
    </message>
    <message>
        <source>Python error</source>
        <translation type="obsolete">Błąd Pythona</translation>
    </message>
    <message>
        <source>Error when reading metadata of plugin </source>
        <translation type="obsolete">Błąd podczas odczytywania metadanych wtyczki </translation>
    </message>
    <message>
        <source>Provider does not support deletion</source>
        <translation>Źródło danych nie obsługuje operacji usuwania</translation>
    </message>
    <message>
        <source>Data provider does not support deleting features</source>
        <translation>Źródło danych nie obsługuje usuwania</translation>
    </message>
    <message>
        <source>Layer not editable</source>
        <translation>Warstwa nie jest w trybie edycji</translation>
    </message>
    <message>
        <source>Toggle editing</source>
        <translation>Tryb edycji</translation>
    </message>
    <message>
        <source>Toggles the editing state of the current layer</source>
        <translation>Włącza/wyłącza tryb edycji aktywnej warstwy</translation>
    </message>
    <message>
        <source>Add Ring</source>
        <translation>Dodaj pierścień</translation>
    </message>
    <message>
        <source>Add Island</source>
        <translation>Dodaj wyspę</translation>
    </message>
    <message>
        <source>Add Island to multipolygon</source>
        <translation>Dodaj wyspę do multipoligonu</translation>
    </message>
    <message>
        <source>Toolbar Visibility...</source>
        <translation type="obsolete">Paski narzędzi...</translation>
    </message>
    <message>
        <source>Scale </source>
        <translation>Skala </translation>
    </message>
    <message>
        <source>Current map scale (formatted as x:y)</source>
        <translation>Obecna skala mapy (zapisana jako x:y)</translation>
    </message>
    <message>
        <source>Map coordinates at mouse cursor position</source>
        <translation>Współrzędne mapy w miejscu kursora</translation>
    </message>
    <message>
        <source>Saved project to:</source>
        <translation>Projekt zapisano w:</translation>
    </message>
    <message>
        <source>The current layer is not editable. Choose &apos;Start editing&apos; in the digitizing toolbar.</source>
        <translation>Aktywna warstwa nie jest edytowalna. Wybierz &apos;Rozpocznij edycję&apos; z belki narzędzi digitizacji.</translation>
    </message>
    <message>
        <source>Invalid scale</source>
        <translation>Skala nieprawidłowa</translation>
    </message>
    <message>
        <source>Network error while communicating with server</source>
        <translation>Błąd połączenia sieciowego podczas komunikacji z serwerem</translation>
    </message>
    <message>
        <source>Unknown network socket error</source>
        <translation>Nieznany błąd połączenia sieciowego</translation>
    </message>
    <message>
        <source>Unable to communicate with QGIS Version server</source>
        <translation>Nie można połączyć się z serwerem wersji QGIS</translation>
    </message>
    <message>
        <source>Do you want to save the current project?</source>
        <translation>Czy chcesz zapisać aktywny projekt?</translation>
    </message>
    <message>
        <source>Move Feature</source>
        <translation>Przesuń obiekt</translation>
    </message>
    <message>
        <source>Split Features</source>
        <translation>Rozdziel obiekty</translation>
    </message>
    <message>
        <source>Map Tips</source>
        <translation>Podpowiedzi na mapie</translation>
    </message>
    <message>
        <source>Show information about a feature when the mouse is hovered over it</source>
        <translation>Wyświetl informacje o obiekcie po najechaniu na niego kursorem</translation>
    </message>
    <message>
        <source>Current map scale</source>
        <translation>Aktualna skala mapy</translation>
    </message>
    <message>
        <source>Project file is older</source>
        <translation>Plik projektu jest starszy</translation>
    </message>
    <message>
        <source>&lt;p&gt;This project file was saved by an older version of QGIS.</source>
        <translation>&lt;p&gt;Plik projektu został zapisany w starszej wersji QGIS.</translation>
    </message>
    <message>
        <source> When saving this project file, QGIS will update it to the latest version, possibly rendering it useless for older versions of QGIS.</source>
        <translation> Podczas zapisywania tego pliku projektu, QGIS zaktualizuje go do najnowszej wersji, co najprawdopodobniej spowoduje jego niekompatybilność ze starszymi wersjami QGIS.</translation>
    </message>
    <message>
        <source>&lt;p&gt;Even though QGIS developers try to maintain backwards compatibility, some of the information from the old project file might be lost.</source>
        <translation>&lt;p&gt;Pomimo, że autorzy QGIS starają się zachować wsteczną kompatybilność, niektóre informacje ze starego pliku projektu mogą być utracone.</translation>
    </message>
    <message>
        <source> To improve the quality of QGIS, we appreciate if you file a bug report at %3.</source>
        <translation> Aby poprawić jakość programu QGIS, prosimy o przesłanie raportu o błędzie %3.</translation>
    </message>
    <message>
        <source> Be sure to include the old project file, and state the version of QGIS you used to discover the error.</source>
        <translation>Upewnij się że załączyła(e)ś plik projektu oraz podała(e)ś wersję QGIS przy której wystąpił błąd.</translation>
    </message>
    <message>
        <source>&lt;p&gt;To remove this warning when opening an older project file, uncheck the box &apos;%5&apos; in the %4 menu.</source>
        <translation>&lt;p&gt;Aby wyłączyć to ostrzeżenie podczas wczytywania starczych wersji plików projektów, odznacz pole &apos;%5&apos; w menu %4.</translation>
    </message>
    <message>
        <source>&lt;p&gt;Version of the project file: %1&lt;br&gt;Current version of QGIS: %2</source>
        <translation>&lt;p&gt;Wersja pliku projektu: %1&lt;br&gt;Aktualna wersja QGIS: %2</translation>
    </message>
    <message>
        <source>&lt;tt&gt;Settings:Options:General&lt;/tt&gt;</source>
        <comment>Menu path to setting options</comment>
        <translation>&lt;tt&gt;Ustawienia:Opcje:Ogólne&lt;/tt&gt;</translation>
    </message>
    <message>
        <source>Warn me when opening a project file saved with an older version of QGIS</source>
        <translation>Ostrzegaj przy otwieraniu projektów zapisanych w starszych wersjach QGIS</translation>
    </message>
    <message>
        <source>Toggle full screen mode</source>
        <translation type="obsolete">Przełącz tryb pełnoekranowy</translation>
    </message>
    <message>
        <source>Ctrl-F</source>
        <comment>Toggle fullscreen mode</comment>
        <translation>Ctrl-F</translation>
    </message>
    <message>
        <source>Toggle fullscreen mode</source>
        <translation>Przełącz tryb pełnoekranowy</translation>
    </message>
    <message>
        <source>Ctrl+T</source>
        <comment>
Hide most toolbars</comment>
        <translation type="obsolete">Ctrl+T</translation>
    </message>
    <message>
        <source>Resource Location Error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Error reading icon resources from: 
 %1
 Quitting...</source>
        <translation>Błąd przy dostępie do zasobów ikony z: 
 %1
 Kończę...</translation>
    </message>
    <message>
        <source>Overview</source>
        <translation>Podgląd</translation>
    </message>
    <message>
        <source>Legend</source>
        <translation>Legenda</translation>
    </message>
    <message>
        <source>You are using QGIS version %1 built against code revision %2.</source>
        <translation>Używasz QGIS w wersji %1 przygotowanej dla rewizji %2.</translation>
    </message>
    <message>
        <source> This copy of QGIS has been built with PostgreSQL support.</source>
        <translation>Ta wersja QGIS została skompilowana ze wsparciem PostgreSQL.</translation>
    </message>
    <message>
        <source> This copy of QGIS has been built without PostgreSQL support.</source>
        <translation>Ta wersja QGIS została skompilowana bez wsparcia PostgreSQL.</translation>
    </message>
    <message>
        <source>
This binary was compiled against Qt %1,and is currently running against Qt %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Shows the map coordinates at the current cursor position. The display is continuously updated as the mouse is moved.</source>
        <translation>Wyświetla aktualizowane na bieżąco współrzędne mapy dla aktualnej pozycji kursora.</translation>
    </message>
    <message>
        <source>Stop map rendering</source>
        <translation>Przerwij renderowanie mapy</translation>
    </message>
    <message>
        <source>Multiple Instances of QgisApp</source>
        <translation>Wiele instancji QgisApp</translation>
    </message>
    <message>
        <source>Multiple instances of Quantum GIS application object detected.
Please contact the developers.
</source>
        <translation>Wykryto wiele instancji Quantum GIS.
Prosimy o kontakt z autorami.
</translation>
    </message>
    <message>
        <source>Shift+Ctrl+S</source>
        <comment>Save Project under a new name</comment>
        <translation>Shift+Ctrl+S</translation>
    </message>
    <message>
        <source>&amp;Print Composer</source>
        <translation>&amp;Asystent wydruku</translation>
    </message>
    <message>
        <source>Ctrl+P</source>
        <comment>Print Composer</comment>
        <translation>Ctrl+P</translation>
    </message>
    <message>
        <source>Print Composer</source>
        <translation>Asystent wydruku</translation>
    </message>
    <message>
        <source>&amp;Undo</source>
        <translation>&amp;Cofnij</translation>
    </message>
    <message>
        <source>Ctrl+Z</source>
        <translation>Ctrl+Z</translation>
    </message>
    <message>
        <source>Undo the last operation</source>
        <translation>Cofnij ostatnią operację</translation>
    </message>
    <message>
        <source>Cu&amp;t</source>
        <translation>Wy&amp;tnij</translation>
    </message>
    <message>
        <source>Ctrl+X</source>
        <translation>Ctrl+X</translation>
    </message>
    <message>
        <source>Cut the current selection&apos;s contents to the clipboard</source>
        <translation>Wytnij aktualnie wybraną zawartość do schowka</translation>
    </message>
    <message>
        <source>&amp;Copy</source>
        <translation>Kopiuj</translation>
    </message>
    <message>
        <source>Ctrl+C</source>
        <translation>Ctrl+C</translation>
    </message>
    <message>
        <source>Copy the current selection&apos;s contents to the clipboard</source>
        <translation>Kopiuj aktualnie wybraną zawartość do schowka</translation>
    </message>
    <message>
        <source>&amp;Paste</source>
        <translation>Wklej</translation>
    </message>
    <message>
        <source>Ctrl+V</source>
        <translation>Ctrl+V</translation>
    </message>
    <message>
        <source>Paste the clipboard&apos;s contents into the current selection</source>
        <translation>Wklej zawartość schowka do aktualnie wybranych</translation>
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
        <translation>Powiększ do zaznaczonych</translation>
    </message>
    <message>
        <source>Ctrl+J</source>
        <comment>Zoom to Selection</comment>
        <translation>Ctrl+J</translation>
    </message>
    <message>
        <source>Zoom Actual Size</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Zoom to Actual Size</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Add Vector Layer...</source>
        <translation>Dodaj warstwę wektorową...</translation>
    </message>
    <message>
        <source>Add Raster Layer...</source>
        <translation>Dodaj warstwę rastrową...</translation>
    </message>
    <message>
        <source>Add PostGIS Layer...</source>
        <translation>Dodaj warstwę PostGIS...</translation>
    </message>
    <message>
        <source>W</source>
        <comment>Add a Web Mapping Server Layer</comment>
        <translation>W</translation>
    </message>
    <message>
        <source>Add a Web Mapping Server Layer</source>
        <translation>Dodaj warstwę Web Mapping Server</translation>
    </message>
    <message>
        <source>Open Attribute Table</source>
        <translation>Otwórz tabelę atrybutów</translation>
    </message>
    <message>
        <source>Save as Shapefile...</source>
        <translation>Zapisz jako shape...</translation>
    </message>
    <message>
        <source>Save the current layer as a shapefile</source>
        <translation>Zapisz aktualną warstwę jako plik shape</translation>
    </message>
    <message>
        <source>Save Selection as Shapefile...</source>
        <translation>Zapisz wybrane jako shape...</translation>
    </message>
    <message>
        <source>Save the selection as a shapefile</source>
        <translation>Zapisz wybrane jako shape</translation>
    </message>
    <message>
        <source>Properties...</source>
        <translation>Właściwości...</translation>
    </message>
    <message>
        <source>Set properties of the current layer</source>
        <translation>Ustaw właściwości aktualnej warstwy</translation>
    </message>
    <message>
        <source>Add to Overview</source>
        <translation>Dodaj do podglądu</translation>
    </message>
    <message>
        <source>Add All to Overview</source>
        <translation>Dodaj wszystkie do podglądu</translation>
    </message>
    <message>
        <source>Manage Plugins...</source>
        <translation>Zarządzaj wtyczkami...</translation>
    </message>
    <message>
        <source>Toggle Full Screen Mode</source>
        <translation>Przełącz tryb pełnoekranowy</translation>
    </message>
    <message>
        <source>Custom CRS...</source>
        <translation>SOW użytkownika...</translation>
    </message>
    <message>
        <source>Manage custom coordinate reference systems</source>
        <translation>Zarządzaj system odniesienia opartym na współrzędnych</translation>
    </message>
    <message>
        <source>Minimize</source>
        <translation>Minimalizuj</translation>
    </message>
    <message>
        <source>Ctrl+M</source>
        <comment>Minimize Window</comment>
        <translation>Ctrl+M</translation>
    </message>
    <message>
        <source>Minimizes the active window to the dock</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Zoom</source>
        <translation>Powiększ</translation>
    </message>
    <message>
        <source>Toggles between a predefined size and the window size set by the user</source>
        <translation>Przełącza pomiędzy predefiniowanym rozmiarem okna a ustalonym przez użytkownika </translation>
    </message>
    <message>
        <source>Bring All to Front</source>
        <translation>Przenieś wszystkie na wierzch</translation>
    </message>
    <message>
        <source>Bring forward all open windows</source>
        <translation>Przenieś do przodu wszystkie otwarte okna</translation>
    </message>
    <message>
        <source>&amp;Edit</source>
        <translation>&amp;Edycja</translation>
    </message>
    <message>
        <source>Panels</source>
        <translation>Panele</translation>
    </message>
    <message>
        <source>Toolbars</source>
        <translation>Paski narzędzi</translation>
    </message>
    <message>
        <source>&amp;Window</source>
        <translation>Okno</translation>
    </message>
    <message>
        <source>Toggle extents and mouse position display</source>
        <translation>Przełącza wyświetlanie zakresu i pozycji myszy</translation>
    </message>
    <message>
        <source>This icon shows whether on the fly coordinate reference system transformation is enabled or not. Click the icon to bring up the project properties dialog to alter this behaviour.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>CRS status - Click to open coordinate reference system dialog</source>
        <translation>Stan SOW - Kliknij aby otworzyć okno dialogowe systemu odniesienia opartego na współrzędnych</translation>
    </message>
    <message>
        <source>Choose a file name to save the QGIS project file as</source>
        <translation>Podaj nazwę pliku do zapisania projektu QGIS</translation>
    </message>
    <message>
        <source>Choose a file name to save the map image as</source>
        <translation>Podaj nazwę pliku do zapisania obrazu mapy</translation>
    </message>
    <message>
        <source>Start editing failed</source>
        <translation>Rozpocznij edycję pola</translation>
    </message>
    <message>
        <source>Provider cannot be opened for editing</source>
        <translation>Źródło nie może zostać otwarte do edycji</translation>
    </message>
    <message>
        <source>Stop editing</source>
        <translation>Zakończ edycję</translation>
    </message>
    <message>
        <source>Do you want to save the changes to layer %1?</source>
        <translation>Czy chcesz zapisać zmiany na warstwie %1?</translation>
    </message>
    <message>
        <source>Could not commit changes to layer %1

Errors:  %2
</source>
        <translation>Nie można zatwierdzić zmian dla warstwy %1

Błędy:  %2
</translation>
    </message>
    <message>
        <source>Problems during roll back</source>
        <translation>Wystąpiły problemy przy cofaniu zmian</translation>
    </message>
    <message>
        <source>Python Console</source>
        <translation>Konsola Pythona</translation>
    </message>
    <message>
        <source>Map coordinates for the current view extents</source>
        <translation>Współrzędne mapy dla bieżącego zasięgu widoku</translation>
    </message>
    <message>
        <source>Maptips require an active layer</source>
        <translation>Podpowiedzi mapy wymagają aktywnej warstwy</translation>
    </message>
    <message>
        <source>This release candidate includes over 265 bug fixes and enchancements over the QGIS 0.11.0 release. In addition we have added the following new features:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>HIG Compliance improvements for Windows / Mac OS X / KDE / Gnome</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Saving a vector layer or subset of that layer to disk with a different Coordinate Reference System to the original.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Advanced topological editing of vector data.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Single click selection of vector features.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Many improvements to raster rendering and support for building pyramids external to the raster file.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Overhaul of the map composer for much improved printing support.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>A new &apos;coordinate capture&apos; plugin was added that lets you click on the map and then cut &amp; paste the coordinates to and from the clipboard.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>A new plugin for converting between OGR supported formats was added.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>A new plugin for converting from DXF files to shapefiles was added.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>A new plugin was added for interpolating point features into ASCII grid layers.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The python plugin manager was completely overhauled, the new version having many improvements, including checking that the version of QGIS running will support a plugin that is being installed.</source>
        <translation>Instalator wtyczek został całkowicie zmodernizowany. Nowa wersja zawiera wiele ulepszeń, w tym automatyczne aktualizacje i testowanie kompatybilności instalowanych wtyczek.</translation>
    </message>
    <message>
        <source>Plugin toolbar positions are now correctly saved when the application is closed.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>In the WMS client, WMS standards support has been improved.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Complete API revision - we now have a stable API following well defined naming conventions.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Ported all GDAL/OGR and GEOS usage to use C APIs only.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The python plugin installer was completely overhauled, the new version having many improvements, including checking that the version of QGIS running will support a plugin that is being installed.</source>
        <translation>Instalator wtyczek został całkowicie zmodernizowany. Nowa wersja zawiera wiele ulepszeń, w tym automatyczne aktualizacje i testowanie kompatybilności instalowanych wtyczek.</translation>
    </message>
    <message>
        <source>Vector editing overhaul - handling of geometry and attribute edit transactions is now handled transparently in one place.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgisAppBase</name>
    <message>
        <source>MainWindow</source>
        <translation type="obsolete">Główne okno</translation>
    </message>
    <message>
        <source>Legend</source>
        <translation type="obsolete">Legenda</translation>
    </message>
    <message>
        <source>Map View</source>
        <translation type="obsolete">Mapa</translation>
    </message>
    <message>
        <source>QGIS</source>
        <translation>QGIS</translation>
    </message>
</context>
<context>
    <name>QgsAbout</name>
    <message>
        <source>About Quantum GIS</source>
        <translation>O Quantum GIS</translation>
    </message>
    <message>
        <source>Ok</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>About</source>
        <translation>O QGIS</translation>
    </message>
    <message>
        <source>QGIS Home Page</source>
        <translation>Strona domowa QGIS</translation>
    </message>
    <message>
        <source>Version</source>
        <translation>Wersja</translation>
    </message>
    <message>
        <source>What&apos;s New</source>
        <translation>Co nowego</translation>
    </message>
    <message>
        <source>Providers</source>
        <translation>Źródła danych</translation>
    </message>
    <message>
        <source>Developers</source>
        <translation>Programiści</translation>
    </message>
    <message>
        <source>Sponsors</source>
        <translation>Sponsorzy</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Lucida Grande&apos;; font-size:13pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:16px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:x-large; font-weight:600;&quot;&gt;&lt;span style=&quot; font-size:x-large;&quot;&gt;Quantum GIS (QGIS)&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;p, li { white-space: pre-wrap; }&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Lucida Grande&apos;; font-size:13pt; font-weight:400; font-style:normal;&quot;&gt;&lt;p style=&quot; margin-top:16px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:x-large; font-weight:600;&quot;&gt;&lt;span style=&quot; font-size:x-large;&quot;&gt;Quantum GIS (QGIS)&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Quantum GIS is licensed under the GNU General Public License</source>
        <translation>Quantum GIS jest rozpowszechniany na zasadach określonych w Powszechnej Licencji Publicznej GNU (GPL GNU)</translation>
    </message>
    <message>
        <source>http://www.gnu.org/licenses</source>
        <translation>http://www.gnu.org/licenses</translation>
    </message>
    <message>
        <source>Join our user mailing list</source>
        <translation>Dołącz do naszej grupy dyskusyjnej dla użytkowników</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Osoba</translation>
    </message>
    <message>
        <source>Website</source>
        <translation>Strona WWW</translation>
    </message>
    <message>
        <source>&lt;p&gt;The following have sponsored QGIS by contributing money to fund development and other project costs&lt;/p&gt;</source>
        <translation>&lt;p&gt;Sponsorzy QGIS&lt;/p&gt;</translation>
    </message>
    <message>
        <source>Available QGIS Data Provider Plugins</source>
        <translation>Dostępne wtyczki źródeł danych QGIS</translation>
    </message>
    <message>
        <source>Available Qt Database Plugins</source>
        <translation>Dostępne wtyczki baz danych Qt</translation>
    </message>
    <message>
        <source>Available Qt Image Plugins</source>
        <translation>Dostępne wtyczki obrazów Qt</translation>
    </message>
</context>
<context>
    <name>QgsAddAttrDialogBase</name>
    <message>
        <source>Add Attribute</source>
        <translation>Dodaj atrybut</translation>
    </message>
    <message>
        <source>Name:</source>
        <translation>Nazwa:</translation>
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
        <translation>Wyjątek</translation>
    </message>
</context>
<context>
    <name>QgsAttributeActionDialog</name>
    <message>
        <source>Select an action</source>
        <comment>File dialog window title</comment>
        <translation>Wybierz operację</translation>
    </message>
</context>
<context>
    <name>QgsAttributeActionDialogBase</name>
    <message>
        <source>This list contains all actions that have been defined for the current layer. Add actions by entering the details in the controls below and then pressing the Insert action button. Actions can be edited here by double clicking on the item.</source>
        <translation>Lista zawiera wszystkie akcje zdefiniowane dla bieżącej warstwy. Dodaj akcje wprowadzając szczegóły w kontrolkach poniżej, a następnie kliknij przycisk Dodaj. Akcje mogą być edytowane po dwukrotnym kliknięciu na jedną z nich.</translation>
    </message>
    <message>
        <source>Move up</source>
        <translation>Przesuń w górę</translation>
    </message>
    <message>
        <source>Move the selected action up</source>
        <translation>Przesuń wybraną akcję wyżej</translation>
    </message>
    <message>
        <source>Move down</source>
        <translation>Przesuń w dół</translation>
    </message>
    <message>
        <source>Move the selected action down</source>
        <translation>Przesuń wybraną akcję niżej</translation>
    </message>
    <message>
        <source>Remove</source>
        <translation>Usuń</translation>
    </message>
    <message>
        <source>Remove the selected action</source>
        <translation>Usuń wybraną akcję</translation>
    </message>
    <message>
        <source>Enter the action name here</source>
        <translation>Wpisz tutaj nazwę akcji</translation>
    </message>
    <message>
        <source>Enter the action command here</source>
        <translation>Wpisz tutaj polecenie akcji</translation>
    </message>
    <message>
        <source>Insert action</source>
        <translation>Wstaw akcję</translation>
    </message>
    <message>
        <source>Insert field</source>
        <translation>Dodaj pole</translation>
    </message>
    <message>
        <source>The valid attribute names for this layer</source>
        <translation>Poprawne nazwy dla atrybutu warstwy</translation>
    </message>
    <message>
        <source>Inserts the selected field into the action, prepended with a %</source>
        <translation>Wstawia wybrane pole do akcji, poprzedzone znakiem %</translation>
    </message>
    <message>
        <source>Enter the name of an action here. The name should be unique (qgis will make it unique if necessary).</source>
        <translation>Tutaj wpisz nazwę akcji. Nazwa powinna być unikalna (qgis zmieni ją na taką jeśli będzie to konieczne).</translation>
    </message>
    <message>
        <source>Inserts the action into the list above</source>
        <translation>Wstawia akcję na listę</translation>
    </message>
    <message>
        <source>Update action</source>
        <translation>Aktualizuj akcję</translation>
    </message>
    <message>
        <source>Update the selected action</source>
        <translation>Aktualizuj wybraną akcję</translation>
    </message>
    <message>
        <source>Capture output</source>
        <translation>Przechwyć wynik</translation>
    </message>
    <message>
        <source>Captures any output from the action</source>
        <translation>Przechwytuje wynik działania akcji</translation>
    </message>
    <message>
        <source>Captures the standard output or error generated by the action and displays it in a dialog box</source>
        <translation>Przechwytuje standardowe wyjście lub błąd wygenerowany przez akację i wyświetla je w oknie dialogowym</translation>
    </message>
    <message>
        <source>Enter the action here. This can be any program, script or command that is available on your system. When the action is invoked any set of characters that start with a % and then have the name of a field will be replaced by the value of that field. The special characters %% will be replaced by the value of the field that was selected. Double quote marks group text into single arguments to the program, script or command. Double quotes will be ignored if preceeded by a backslash</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Attribute Actions</source>
        <translation>Atrybuty akcji</translation>
    </message>
    <message>
        <source>Action properties</source>
        <translation>Właściwości akcji</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Nazwa</translation>
    </message>
    <message>
        <source>Action</source>
        <translation>Akcja</translation>
    </message>
    <message>
        <source>Browse for action</source>
        <translation>Wyszukaj akcję</translation>
    </message>
    <message>
        <source>Click to browse for an action</source>
        <translation>Kliknij aby wyszukać akcję</translation>
    </message>
    <message>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <source>Capture</source>
        <translation>Przechwyć</translation>
    </message>
    <message>
        <source>Clicking the button will let you select an application to use as the action</source>
        <translation>Kliknięcie przycisku umożliwi wybór aplikacji, która zostanie wykorzystana jako akcja</translation>
    </message>
</context>
<context>
    <name>QgsAttributeDialog</name>
    <message>
        <source> (int)</source>
        <translation>(int)</translation>
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
        <translation>Wybierz plik</translation>
    </message>
</context>
<context>
    <name>QgsAttributeDialogBase</name>
    <message>
        <source>Enter Attribute Values</source>
        <translation>Wprowadź wartość atrybutu</translation>
    </message>
</context>
<context>
    <name>QgsAttributeTable</name>
    <message>
        <source>Run action</source>
        <translation>Uruchom akcję</translation>
    </message>
    <message>
        <source>Updating selection...</source>
        <translation>Aktualizuję wybór...</translation>
    </message>
    <message>
        <source>Abort</source>
        <translation>Przerwij</translation>
    </message>
</context>
<context>
    <name>QgsAttributeTableBase</name>
    <message>
        <source>Attribute Table</source>
        <translation>Tabela atrybutów</translation>
    </message>
    <message>
        <source>Start editing</source>
        <translation type="obsolete">Rozpocznij edycję</translation>
    </message>
    <message>
        <source>Ctrl+X</source>
        <translation type="obsolete">Ctrl+X</translation>
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
        <source>Invert selection</source>
        <translation>Odwróć zaznaczenie</translation>
    </message>
    <message>
        <source>Ctrl+T</source>
        <translation>Ctrl+T</translation>
    </message>
    <message>
        <source>Move selected to top</source>
        <translation>Przesuń zaznaczone na górę</translation>
    </message>
    <message>
        <source>Remove selection</source>
        <translation>Usuń zaznaczenie</translation>
    </message>
    <message>
        <source>Copy selected rows to clipboard (Ctrl+C)</source>
        <translation>Kopiuj zaznaczone wiersze do schowka (Ctrl+C)</translation>
    </message>
    <message>
        <source>Copies the selected rows to the clipboard</source>
        <translation>Kopiuje zaznaczone wiersze do schowka</translation>
    </message>
    <message>
        <source>Ctrl+C</source>
        <translation>Ctrl+C</translation>
    </message>
    <message>
        <source>Stop editin&amp;g</source>
        <translation type="obsolete">Zatrzymaj edycję</translation>
    </message>
    <message>
        <source>Alt+G</source>
        <translation type="obsolete">Alt+G</translation>
    </message>
    <message>
        <source>in</source>
        <translation> w</translation>
    </message>
    <message>
        <source>Search</source>
        <translation>Szukaj</translation>
    </message>
    <message>
        <source>Adva&amp;nced...</source>
        <translation>Zaawansowa&amp;ne...</translation>
    </message>
    <message>
        <source>Alt+N</source>
        <translation>Alt+N</translation>
    </message>
    <message>
        <source>New column</source>
        <translation type="obsolete">Nowa kolumna</translation>
    </message>
    <message>
        <source>Delete column</source>
        <translation type="obsolete">Usuń kolumnę</translation>
    </message>
    <message>
        <source>Zoom map to the selected rows (Ctrl-F)</source>
        <translation type="obsolete">Kadruj mapę do zasięgu wybranych wierszy (Ctrl-F)</translation>
    </message>
    <message>
        <source>Zoom map to the selected rows</source>
        <translation>Kadruj mapę do zasięgu wybranych wierszy</translation>
    </message>
    <message>
        <source>Ctrl+F</source>
        <translation type="obsolete">Ctrl+F</translation>
    </message>
    <message>
        <source>Search for</source>
        <translation>Szukaj</translation>
    </message>
    <message>
        <source>Zoom map to the selected rows (Ctrl-J)</source>
        <translation>Powiększ mapę do wybranych wierszy (Ctrl-J)</translation>
    </message>
    <message>
        <source>Ctrl+J</source>
        <translation>Ctrl+J</translation>
    </message>
    <message>
        <source>Toggle editing mode</source>
        <translation>Przełącz tryb edycji</translation>
    </message>
    <message>
        <source>Click to toggle table editing</source>
        <translation>Przełącz edycję tabeli</translation>
    </message>
</context>
<context>
    <name>QgsAttributeTableDisplay</name>
    <message>
        <source>select</source>
        <translation>zaznacz</translation>
    </message>
    <message>
        <source>select and bring to top</source>
        <translation>zaznacz i przenieś na górę</translation>
    </message>
    <message>
        <source>show only matching</source>
        <translation>pokaż tylko zaznaczone</translation>
    </message>
    <message>
        <source>Search string parsing error</source>
        <translation>Błąd parsowania tekstu wyszukiwania</translation>
    </message>
    <message>
        <source>Search results</source>
        <translation>Rezultaty wyszukiwania</translation>
    </message>
    <message>
        <source>You&apos;ve supplied an empty search string.</source>
        <translation>Podano pusty warunek wyszukiwania.</translation>
    </message>
    <message>
        <source>Error during search</source>
        <translation>Błąd wyszukiwania</translation>
    </message>
    <message>
        <source>No matching features found.</source>
        <translation>Nie znaleziono pasujących obiektów.</translation>
    </message>
    <message>
        <source>Name conflict</source>
        <translation type="obsolete">Konflikt nazw</translation>
    </message>
    <message>
        <source>Stop editing</source>
        <translation type="obsolete">Zakończ edycję</translation>
    </message>
    <message>
        <source>Do you want to save the changes?</source>
        <translation type="obsolete">Czy chcesz zapisać zmiany?</translation>
    </message>
    <message>
        <source>Error</source>
        <translation type="obsolete">Błąd</translation>
    </message>
    <message>
        <source>The attribute could not be inserted. The name already exists in the table.</source>
        <translation type="obsolete">Nie można dodać atrybutu. Atrybut o tej nazwie już istnieje w tabeli.</translation>
    </message>
    <message>
        <source>Attribute table - </source>
        <translation>Tabela atrybutów - </translation>
    </message>
    <message>
        <source>QGIS</source>
        <translation>QGIS</translation>
    </message>
    <message>
        <source>File</source>
        <translation>Plik</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Zamknij</translation>
    </message>
    <message>
        <source>Ctrl+W</source>
        <translation>Ctrl+W</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation>Edytuj</translation>
    </message>
    <message>
        <source>&amp;Undo</source>
        <translation>&amp;Cofnij</translation>
    </message>
    <message>
        <source>Ctrl+Z</source>
        <translation>Ctrl+Z</translation>
    </message>
    <message>
        <source>Cu&amp;t</source>
        <translation>Wy&amp;tnij</translation>
    </message>
    <message>
        <source>Ctrl+X</source>
        <translation>Ctrl+X</translation>
    </message>
    <message>
        <source>&amp;Copy</source>
        <translation>&amp;Kopiuj</translation>
    </message>
    <message>
        <source>Ctrl+C</source>
        <translation>Ctrl+C</translation>
    </message>
    <message>
        <source>&amp;Paste</source>
        <translation>&amp;Wklej</translation>
    </message>
    <message>
        <source>Ctrl+V</source>
        <translation>Ctrl+V</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>Usuń</translation>
    </message>
    <message>
        <source>Layer</source>
        <translation>Warstwa</translation>
    </message>
    <message>
        <source>Zoom to Selection</source>
        <translation>Powiększ do zaznaczonych</translation>
    </message>
    <message>
        <source>Ctrl+J</source>
        <translation>Ctrl+J</translation>
    </message>
    <message>
        <source>Toggle Editing</source>
        <translation>Tryb edycji</translation>
    </message>
    <message>
        <source>Table</source>
        <translation>Tabela</translation>
    </message>
    <message>
        <source>Move to Top</source>
        <translation>Przesuń na górę</translation>
    </message>
    <message>
        <source>Invert</source>
        <translation>Odwróć</translation>
    </message>
    <message>
        <source>bad_alloc exception</source>
        <translation>wyjątek bad_alloc</translation>
    </message>
    <message>
        <source>Filling the attribute table has been stopped because there was no more virtual memory left</source>
        <translation>Wypełnianie tabeli atrybutów zostało wstrzymane ponieważ zaczyna brakować pamięci wirtualnej</translation>
    </message>
    <message numerus="yes">
        <source>Found %1 matching features.</source>
        <translation>
            <numerusform>Znaleziono %1 pasujący obiekt.</numerusform>
            <numerusform>Znaleziono %1 pasujące obiekty.</numerusform>
            <numerusform>Znaleziono %1 pasujących obiektów.</numerusform>
        </translation>
    </message>
</context>
<context>
    <name>QgsBookmarks</name>
    <message>
        <source>Really Delete?</source>
        <translation>Czy na pewno usunąć?</translation>
    </message>
    <message>
        <source>Are you sure you want to delete the </source>
        <translation>Czy jesteś pewien, że mam usunąć zakładkę </translation>
    </message>
    <message>
        <source> bookmark?</source>
        <translation> ?</translation>
    </message>
    <message>
        <source>Error deleting bookmark</source>
        <translation>Błąd podczas usuwania zakładki</translation>
    </message>
    <message>
        <source>Failed to delete the </source>
        <translation>Nie powiodło się usunięcie zakładki </translation>
    </message>
    <message>
        <source> bookmark from the database. The database said:
</source>
        <translation> z bazy danych. Komunikat bazy danych:</translation>
    </message>
    <message>
        <source>&amp;Delete</source>
        <translation>&amp;Usuń</translation>
    </message>
    <message>
        <source>&amp;Zoom to</source>
        <translation>Powięks&amp;z do</translation>
    </message>
</context>
<context>
    <name>QgsBookmarksBase</name>
    <message>
        <source>Geospatial Bookmarks</source>
        <translation>Zakładki</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Nazwa</translation>
    </message>
    <message>
        <source>Project</source>
        <translation>Projekt</translation>
    </message>
    <message>
        <source>Extent</source>
        <translation>Zasięg</translation>
    </message>
    <message>
        <source>Id</source>
        <translation>Id</translation>
    </message>
</context>
<context>
    <name>QgsComposer</name>
    <message>
        <source> for read/write</source>
        <translation type="obsolete">do odczytu/zapisu</translation>
    </message>
    <message>
        <source>Choose a filename to save the map image as</source>
        <translation type="obsolete">Podaj nazwę pliku do którego zapisać obraz mapy</translation>
    </message>
    <message>
        <source>Choose a filename to save the map as</source>
        <translation type="obsolete">Podaj nazwę pliku do którego zapisać mapę</translation>
    </message>
    <message>
        <source>Error in Print</source>
        <translation type="obsolete">Błąd druku</translation>
    </message>
    <message>
        <source>Cannot seek</source>
        <translation type="obsolete">Nie mogę wyszukać</translation>
    </message>
    <message>
        <source>Cannot overwrite BoundingBox</source>
        <translation type="obsolete">Nie mogę nadpisać BoundingBox</translation>
    </message>
    <message>
        <source>Cannot find BoundingBox</source>
        <translation type="obsolete">Nie mogę znaleźć BoundingBox</translation>
    </message>
    <message>
        <source>Cannot overwrite translate</source>
        <translation type="obsolete">Nie mogę nadpisać tłumaczenia</translation>
    </message>
    <message>
        <source>Cannot find translate</source>
        <translation type="obsolete">Nie mogę znaleźć tłumaczenia</translation>
    </message>
    <message>
        <source>File IO Error</source>
        <translation type="obsolete">Błąd wejścia/wyjścia</translation>
    </message>
    <message>
        <source>Paper does not match</source>
        <translation type="obsolete">Niewłaściwy papier </translation>
    </message>
    <message>
        <source>The selected paper size does not match the composition size</source>
        <translation type="obsolete">Wybrany rozmiar papieru nie odpowiada rozmiarowi kompozycji</translation>
    </message>
    <message>
        <source>Big image</source>
        <translation>Duży obraz</translation>
    </message>
    <message>
        <source>To create image </source>
        <translation>Aby stworzyć obraz</translation>
    </message>
    <message>
        <source> requires circa </source>
        <translation>potrzeba około</translation>
    </message>
    <message>
        <source> MB of memory</source>
        <translation>MB pamięci</translation>
    </message>
    <message>
        <source>QGIS - print composer</source>
        <translation>QGIS - asystent wydruku</translation>
    </message>
    <message>
        <source>Map 1</source>
        <translation>Mapa 1</translation>
    </message>
    <message>
        <source>Couldn&apos;t open </source>
        <translation type="obsolete">Nie mogę otworzyć</translation>
    </message>
    <message>
        <source>format</source>
        <translation>format</translation>
    </message>
    <message>
        <source>SVG warning</source>
        <translation>Ostrzeżenie SVG</translation>
    </message>
    <message>
        <source>Don&apos;t show this message again</source>
        <translation>Nie pokazuj tego komunikatu ponownie</translation>
    </message>
    <message>
        <source>SVG Format</source>
        <translation>Format SVG</translation>
    </message>
    <message>
        <source>Move Content</source>
        <translation type="obsolete">Przesuń zawartość</translation>
    </message>
    <message>
        <source>Move item content</source>
        <translation type="obsolete">Przesuń zawartość obiektu</translation>
    </message>
    <message>
        <source>&amp;Group</source>
        <translation type="obsolete">&amp;Grupuj</translation>
    </message>
    <message>
        <source>Group items</source>
        <translation type="obsolete">Grupuj obiekty</translation>
    </message>
    <message>
        <source>&amp;Ungroup</source>
        <translation type="obsolete">&amp;Rozdziel</translation>
    </message>
    <message>
        <source>Ungroup items</source>
        <translation type="obsolete">Rozdziel obiekty</translation>
    </message>
    <message>
        <source>Raise</source>
        <translation type="obsolete">Podnieś</translation>
    </message>
    <message>
        <source>Raise selected items</source>
        <translation type="obsolete">Wybrany wyżej</translation>
    </message>
    <message>
        <source>Lower</source>
        <translation type="obsolete">Obniż</translation>
    </message>
    <message>
        <source>Lower selected items</source>
        <translation type="obsolete">Wybrany niżej</translation>
    </message>
    <message>
        <source>Bring to Front</source>
        <translation type="obsolete">Przenieś na wierzch</translation>
    </message>
    <message>
        <source>Move selected items to top</source>
        <translation type="obsolete">Wybrane obiekty na wierzch</translation>
    </message>
    <message>
        <source>Send to Back</source>
        <translation type="obsolete">Przenieś na tył</translation>
    </message>
    <message>
        <source>Move selected items to bottom</source>
        <translation type="obsolete">Wybrane obiekty na tył</translation>
    </message>
    <message>
        <source>QGIS</source>
        <translation>QGIS</translation>
    </message>
    <message>
        <source>File</source>
        <translation>Plik</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Zamknij</translation>
    </message>
    <message>
        <source>Ctrl+W</source>
        <translation>Ctrl+W</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation>Edytuj</translation>
    </message>
    <message>
        <source>&amp;Undo</source>
        <translation>&amp;Cofnij</translation>
    </message>
    <message>
        <source>Ctrl+Z</source>
        <translation>Ctrl+Z</translation>
    </message>
    <message>
        <source>Cu&amp;t</source>
        <translation>Wy&amp;tnij</translation>
    </message>
    <message>
        <source>Ctrl+X</source>
        <translation>Ctrl+X</translation>
    </message>
    <message>
        <source>&amp;Copy</source>
        <translation>&amp;Kopiuj</translation>
    </message>
    <message>
        <source>Ctrl+C</source>
        <translation>Ctrl+C</translation>
    </message>
    <message>
        <source>&amp;Paste</source>
        <translation>&amp;Wklej</translation>
    </message>
    <message>
        <source>Ctrl+V</source>
        <translation>Ctrl+V</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>Usuń</translation>
    </message>
    <message>
        <source>View</source>
        <translation>Widok</translation>
    </message>
    <message>
        <source>Layout</source>
        <translation>Układ</translation>
    </message>
    <message>
        <source>Choose a file name to save the map image as</source>
        <translation>Podaj nazwę pliku do zapisania obrazu mapy</translation>
    </message>
    <message>
        <source>Choose a file name to save the map as</source>
        <translation>Podaj nazwę pliku do zapisania mapy</translation>
    </message>
    <message>
        <source>Project contains WMS layers</source>
        <translation>Projekt zawiera warstwy WMS</translation>
    </message>
    <message>
        <source>Some WMS servers (e.g. UMN mapserver) have a limit for the WIDTH and HEIGHT parameter. Printing layers from such servers may exceed this limit. If this is the case, the WMS layer will not be printed</source>
        <translation>Niektóre serwery WMS (np. UMN Mapserver) mają ograniczenia parametrów WIDTH i HEIGHT. Drukowanie warstw z takich serwerów może przekroczyć te limity. W takim przypadku warstwa WMS nie zostanie wydrukowana</translation>
    </message>
    <message>
        <source>&lt;p&gt;The SVG export function in Qgis has several problems due to bugs and deficiencies in the </source>
        <translation>&lt;p&gt;Funkcja eksportu SVG napotkała problemy z powodu błędów i braków w </translation>
    </message>
</context>
<context>
    <name>QgsComposerBase</name>
    <message>
        <source>General</source>
        <translation>Ogólne</translation>
    </message>
    <message>
        <source>Composition</source>
        <translation>Kompozycja</translation>
    </message>
    <message>
        <source>Item</source>
        <translation>Obiekt</translation>
    </message>
    <message>
        <source>&amp;Open Template ...</source>
        <translation type="obsolete">&amp;Otwórz szablon ...</translation>
    </message>
    <message>
        <source>Save Template &amp;As...</source>
        <translation type="obsolete">Zapisz szablon j&amp;ako...</translation>
    </message>
    <message>
        <source>&amp;Print...</source>
        <translation>&amp;Drukuj...</translation>
    </message>
    <message>
        <source>Add new map</source>
        <translation>Dodaj nową mapę</translation>
    </message>
    <message>
        <source>Add new label</source>
        <translation>Dodaj nową etykietę</translation>
    </message>
    <message>
        <source>Add new vect legend</source>
        <translation>Dodaj nową legendę</translation>
    </message>
    <message>
        <source>Select/Move item</source>
        <translation>Wybierz/przesuń obiekt</translation>
    </message>
    <message>
        <source>Export as image</source>
        <translation type="obsolete">Eksportuj jako obraz</translation>
    </message>
    <message>
        <source>Export as SVG</source>
        <translation type="obsolete">Eksportuj jako SVG</translation>
    </message>
    <message>
        <source>Add new scalebar</source>
        <translation>Dodaj podziałkę</translation>
    </message>
    <message>
        <source>Refresh view</source>
        <translation>Odśwież widok</translation>
    </message>
    <message>
        <source>MainWindow</source>
        <translation>Okno główne</translation>
    </message>
    <message>
        <source>Zoom All</source>
        <translation type="obsolete">Pełny zasięg</translation>
    </message>
    <message>
        <source>Zoom In</source>
        <translation>Powiększ</translation>
    </message>
    <message>
        <source>Zoom Out</source>
        <translation>Pomniejsz</translation>
    </message>
    <message>
        <source>Add Image</source>
        <translation>Dodaj obraz</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Zamknij</translation>
    </message>
    <message>
        <source>Help</source>
        <translation>Pomoc</translation>
    </message>
    <message>
        <source>&amp;Open Template...</source>
        <translation type="obsolete">&amp;Otwórz szablon ...</translation>
    </message>
    <message>
        <source>Zoom Full</source>
        <translation>Cały zasięg</translation>
    </message>
    <message>
        <source>Add Map</source>
        <translation>Dodaj mapę</translation>
    </message>
    <message>
        <source>Add Label</source>
        <translation>Dodaj etykietę</translation>
    </message>
    <message>
        <source>Add Vector Legend</source>
        <translation>Dodaj legendę wektorową</translation>
    </message>
    <message>
        <source>Move Item</source>
        <translation>Przesuń obiekt</translation>
    </message>
    <message>
        <source>Export as Image...</source>
        <translation>Eksportuj jako obraz ...</translation>
    </message>
    <message>
        <source>Export as SVG...</source>
        <translation>Eksportuj jako SVG...</translation>
    </message>
    <message>
        <source>Add Scalebar</source>
        <translation>Dodaj skalę</translation>
    </message>
    <message>
        <source>Refresh</source>
        <translation>Odśwież</translation>
    </message>
    <message>
        <source>Move Content</source>
        <translation>Przesuń zawartość</translation>
    </message>
    <message>
        <source>Move item content</source>
        <translation>Przesuń zawartość obiektu</translation>
    </message>
    <message>
        <source>Group</source>
        <translation>Grupuj</translation>
    </message>
    <message>
        <source>Group items</source>
        <translation>Grupuj obiekty</translation>
    </message>
    <message>
        <source>Ungroup</source>
        <translation>Rozdziel</translation>
    </message>
    <message>
        <source>Ungroup items</source>
        <translation>Rozdziel obiekty</translation>
    </message>
    <message>
        <source>Raise</source>
        <translation>Podnieś</translation>
    </message>
    <message>
        <source>Raise selected items</source>
        <translation>Wybrane wyżej</translation>
    </message>
    <message>
        <source>Lower</source>
        <translation>Obniż</translation>
    </message>
    <message>
        <source>Lower selected items</source>
        <translation>Wybrane niżej</translation>
    </message>
    <message>
        <source>Bring to Front</source>
        <translation>Przesuń na wierzch</translation>
    </message>
    <message>
        <source>Move selected items to top</source>
        <translation>Wybrane obiekty na wierzch</translation>
    </message>
    <message>
        <source>Send to Back</source>
        <translation>Przesuń na spód</translation>
    </message>
    <message>
        <source>Move selected items to bottom</source>
        <translation>Wybrane obiekty na spód</translation>
    </message>
</context>
<context>
    <name>QgsComposerItemWidgetBase</name>
    <message>
        <source>Form</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Composer item properties</source>
        <translation>Właściwości obiektu</translation>
    </message>
    <message>
        <source>Color:</source>
        <translation>Kolor:</translation>
    </message>
    <message>
        <source>Frame...</source>
        <translation>Ramka...</translation>
    </message>
    <message>
        <source>Background...</source>
        <translation>Tło...</translation>
    </message>
    <message>
        <source>Opacity:</source>
        <translation>Przeźroczystość:</translation>
    </message>
    <message>
        <source>Outline width: </source>
        <translation>Szerokość obrysu:</translation>
    </message>
    <message>
        <source>Frame</source>
        <translation>Ramka</translation>
    </message>
</context>
<context>
    <name>QgsComposerLabelWidgetBase</name>
    <message>
        <source>Label Options</source>
        <translation>Opcje etykiety</translation>
    </message>
    <message>
        <source>Font</source>
        <translation>Czcionka</translation>
    </message>
    <message>
        <source>Margin (mm):</source>
        <translation>Margines (mm):</translation>
    </message>
</context>
<context>
    <name>QgsComposerLegendItemDialogBase</name>
    <message>
        <source>Legend item properties</source>
        <translation>Właściwości elementu legendy</translation>
    </message>
    <message>
        <source>Item text:</source>
        <translation type="unfinished">Tekst obiektu:</translation>
    </message>
</context>
<context>
    <name>QgsComposerLegendWidgetBase</name>
    <message>
        <source>Barscale Options</source>
        <translation>Opcje podziałki</translation>
    </message>
    <message>
        <source>General</source>
        <translation>Ogólne</translation>
    </message>
    <message>
        <source>Title:</source>
        <translation>Tytuł:</translation>
    </message>
    <message>
        <source>Font:</source>
        <translation>Czcionka:</translation>
    </message>
    <message>
        <source>Title...</source>
        <translation>Tytuł...</translation>
    </message>
    <message>
        <source>Layer...</source>
        <translation>Warstwa...</translation>
    </message>
    <message>
        <source>Item...</source>
        <translation>Obiekt...</translation>
    </message>
    <message>
        <source>Symbol width: </source>
        <translation>Szerokość symbolu:</translation>
    </message>
    <message>
        <source>Symbol height:</source>
        <translation>Wysokość symbolu:</translation>
    </message>
    <message>
        <source>Layer space: </source>
        <translation>Odstęp warstwy:</translation>
    </message>
    <message>
        <source>Symbol space:</source>
        <translation>Odstęp symbolu:</translation>
    </message>
    <message>
        <source>Icon label space:</source>
        <translation>Odstęp etykiety ikony:</translation>
    </message>
    <message>
        <source>Box space:</source>
        <translation>Odstęp pojemnika:</translation>
    </message>
    <message>
        <source>Legend items</source>
        <translation>Legenda obiektów</translation>
    </message>
    <message>
        <source>down</source>
        <translation>niżej</translation>
    </message>
    <message>
        <source>up</source>
        <translation>wyżej</translation>
    </message>
    <message>
        <source>remove</source>
        <translation>usuń</translation>
    </message>
    <message>
        <source>edit...</source>
        <translation>edytuj...</translation>
    </message>
    <message>
        <source>update</source>
        <translation>aktualizuj</translation>
    </message>
    <message>
        <source>update all</source>
        <translation>aktualizuj wszystkie</translation>
    </message>
</context>
<context>
    <name>QgsComposerMap</name>
    <message>
        <source>Map %1</source>
        <translation type="obsolete">Mapa %1</translation>
    </message>
    <message>
        <source>Extent (calculate scale)</source>
        <translation type="obsolete">Zasięg (przelicz skalę)</translation>
    </message>
    <message>
        <source>Scale (calculate extent)</source>
        <translation type="obsolete">Skala (przelicz powiększenie)</translation>
    </message>
    <message>
        <source>Cache</source>
        <translation type="obsolete">Cache</translation>
    </message>
    <message>
        <source>Render</source>
        <translation type="obsolete">Renderuj</translation>
    </message>
    <message>
        <source>Rectangle</source>
        <translation type="obsolete">Prostokąt</translation>
    </message>
    <message>
        <source>Map</source>
        <translation>Mapa</translation>
    </message>
    <message>
        <source>Map will be printed here</source>
        <translation>Tutaj zostanie wydrukowana mapa</translation>
    </message>
</context>
<context>
    <name>QgsComposerMapWidget</name>
    <message>
        <source>Cache</source>
        <translation>Cache</translation>
    </message>
    <message>
        <source>Rectangle</source>
        <translation>Prostokąt</translation>
    </message>
    <message>
        <source>Render</source>
        <translation>Renderuj</translation>
    </message>
</context>
<context>
    <name>QgsComposerMapWidgetBase</name>
    <message>
        <source>Map options</source>
        <translation>Opcje mapy</translation>
    </message>
    <message>
        <source>&lt;b&gt;Map&lt;/b&gt;</source>
        <translation>&lt;b&gt;Mapa&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Width</source>
        <translation>Szerokość</translation>
    </message>
    <message>
        <source>Height</source>
        <translation>Wysokość</translation>
    </message>
    <message>
        <source>Scale:</source>
        <translation>Skala:</translation>
    </message>
    <message>
        <source>1:</source>
        <translation>1:</translation>
    </message>
    <message>
        <source>Map extent</source>
        <translation>Zakres mapy</translation>
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
        <translation>ustaw do zakresu mapy</translation>
    </message>
    <message>
        <source>Preview</source>
        <translation>Podgląd</translation>
    </message>
    <message>
        <source>Update preview</source>
        <translation>Aktualizuj podgląd</translation>
    </message>
</context>
<context>
    <name>QgsComposerPictureWidget</name>
    <message>
        <source>Select svg or image file</source>
        <translation>Wybierz plik SVG lub mapę bitową</translation>
    </message>
</context>
<context>
    <name>QgsComposerPictureWidgetBase</name>
    <message>
        <source>Picture Options</source>
        <translation>Opcje obrazu</translation>
    </message>
    <message>
        <source>Browse...</source>
        <translation>Przeglądaj...</translation>
    </message>
    <message>
        <source>Width:</source>
        <translation>Szerokość:</translation>
    </message>
    <message>
        <source>Height:</source>
        <translation>Wysokość:</translation>
    </message>
    <message>
        <source>Rotation:</source>
        <translation>Obrót:</translation>
    </message>
</context>
<context>
    <name>QgsComposerScaleBar</name>
    <message>
        <source>Single Box</source>
        <translation type="obsolete">Pojedynczy pojemnik</translation>
    </message>
    <message>
        <source>Double Box</source>
        <translation type="obsolete">Podwójny pojemnik</translation>
    </message>
    <message>
        <source>Line Ticks Middle</source>
        <translation type="obsolete">Znaczniki pośrodku</translation>
    </message>
    <message>
        <source>Line Ticks Down</source>
        <translation type="obsolete">Znaczniki na dole</translation>
    </message>
    <message>
        <source>Line Ticks Up</source>
        <translation type="obsolete">Znaczniki u góry</translation>
    </message>
    <message>
        <source>Numeric</source>
        <translation type="obsolete">Numeryczna</translation>
    </message>
</context>
<context>
    <name>QgsComposerScaleBarWidget</name>
    <message>
        <source>Single Box</source>
        <translation>Pojedynczy pojemnik</translation>
    </message>
    <message>
        <source>Double Box</source>
        <translation>Podwójny pojemnik</translation>
    </message>
    <message>
        <source>Line Ticks Middle</source>
        <translation>Znaczniki pośrodku</translation>
    </message>
    <message>
        <source>Line Ticks Down</source>
        <translation>Znaczniki na dole</translation>
    </message>
    <message>
        <source>Line Ticks Up</source>
        <translation>Znaczniki u góry</translation>
    </message>
    <message>
        <source>Numeric</source>
        <translation>Numeryczna</translation>
    </message>
    <message>
        <source>Map </source>
        <translation>Mapa</translation>
    </message>
</context>
<context>
    <name>QgsComposerScaleBarWidgetBase</name>
    <message>
        <source>Barscale Options</source>
        <translation>Opcje podziałki</translation>
    </message>
    <message>
        <source>Segment size (map units):</source>
        <translation>Rozmiar segmentu (w jednostkach mapy):</translation>
    </message>
    <message>
        <source>Map units per bar unit:</source>
        <translation>Jednostek mapy na jednostkę podziałki:</translation>
    </message>
    <message>
        <source>Number of segments:</source>
        <translation>Liczba segmentów:</translation>
    </message>
    <message>
        <source>Segments left:</source>
        <translation>Pozostałych segmentów:</translation>
    </message>
    <message>
        <source>Style:</source>
        <translation>Styl:</translation>
    </message>
    <message>
        <source>Map:</source>
        <translation>Mapa:</translation>
    </message>
    <message>
        <source>Height (mm):</source>
        <translation>Wysokość (mm):</translation>
    </message>
    <message>
        <source>Line width:</source>
        <translation>Szerokość linii:</translation>
    </message>
    <message>
        <source>Label space:</source>
        <translation>Odstęp etykiety:</translation>
    </message>
    <message>
        <source>Box space:</source>
        <translation>Odstęp pojemnika:</translation>
    </message>
    <message>
        <source>Unit label:</source>
        <translation>Etykieta jednostki:</translation>
    </message>
    <message>
        <source>Font...</source>
        <translation>Czcionka...</translation>
    </message>
    <message>
        <source>Color...</source>
        <translation>Kolor...</translation>
    </message>
</context>
<context>
    <name>QgsComposerVectorLegendBase</name>
    <message>
        <source>Vector Legend Options</source>
        <translation>Opcje legendy</translation>
    </message>
    <message>
        <source>Title</source>
        <translation>Tytuł</translation>
    </message>
    <message>
        <source>Map</source>
        <translation>Mapa</translation>
    </message>
    <message>
        <source>Font</source>
        <translation>Czcionka</translation>
    </message>
    <message>
        <source>Box</source>
        <translation>Prostokąt</translation>
    </message>
    <message>
        <source>Preview</source>
        <translation>Podgląd</translation>
    </message>
    <message>
        <source>Layers</source>
        <translation>Warstwy</translation>
    </message>
    <message>
        <source>Group</source>
        <translation>Grupa</translation>
    </message>
    <message>
        <source>ID</source>
        <translation>ID</translation>
    </message>
</context>
<context>
    <name>QgsComposition</name>
    <message>
        <source>Custom</source>
        <translation type="obsolete">Własny</translation>
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
        <translation type="obsolete">Pionowo</translation>
    </message>
    <message>
        <source>Landscape</source>
        <translation type="obsolete">Poziomo</translation>
    </message>
    <message>
        <source>Out of memory</source>
        <translation type="obsolete">Brak pamięci</translation>
    </message>
    <message>
        <source>Qgis is unable to resize the paper size due to insufficient memory.
 It is best that you avoid using the map composer until you restart qgis.
</source>
        <translation type="obsolete">Qgis nie może zmienić rozmiaru papieru z powodu braku pamięci.
Przed ponownym użyciem kompozytora mapy zaleca się restart qgis.
  </translation>
    </message>
    <message>
        <source>Label</source>
        <translation type="obsolete">Etykieta</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation type="obsolete">Ostrzeżenie</translation>
    </message>
    <message>
        <source>Cannot load picture.</source>
        <translation type="obsolete">Nie można wczytać obrazu.</translation>
    </message>
</context>
<context>
    <name>QgsCompositionBase</name>
    <message>
        <source>Composition</source>
        <translation>Kompozycja</translation>
    </message>
    <message>
        <source>Paper</source>
        <translation>Papier</translation>
    </message>
    <message>
        <source>Size</source>
        <translation>Rozmiar</translation>
    </message>
    <message>
        <source>Units</source>
        <translation>Jednostki</translation>
    </message>
    <message>
        <source>Width</source>
        <translation>Szerokość</translation>
    </message>
    <message>
        <source>Height</source>
        <translation>Wysokość</translation>
    </message>
    <message>
        <source>Orientation</source>
        <translation>Orientacja</translation>
    </message>
    <message>
        <source>Resolution (dpi)</source>
        <translation type="obsolete">Rozdzielczość (dpi)</translation>
    </message>
</context>
<context>
    <name>QgsCompositionWidget</name>
    <message>
        <source>Landscape</source>
        <translation>Poziomo</translation>
    </message>
    <message>
        <source>Portrait</source>
        <translation>Pionowo</translation>
    </message>
    <message>
        <source>Custom</source>
        <translation>Użytkownika</translation>
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
        <translation>Kompozycja</translation>
    </message>
    <message>
        <source>Paper</source>
        <translation>Papier</translation>
    </message>
    <message>
        <source>Orientation</source>
        <translation>Orientacja</translation>
    </message>
    <message>
        <source>Height</source>
        <translation>Wysokość</translation>
    </message>
    <message>
        <source>Width</source>
        <translation>Szerokość</translation>
    </message>
    <message>
        <source>Units</source>
        <translation>Jednostki</translation>
    </message>
    <message>
        <source>Size</source>
        <translation>Rozmiar</translation>
    </message>
    <message>
        <source>Print quality (dpi)</source>
        <translation>Rozdzielczość wydruku (dpi)</translation>
    </message>
</context>
<context>
    <name>QgsContinuousColorDialogBase</name>
    <message>
        <source>Continuous color</source>
        <translation>Ciągły kolor</translation>
    </message>
    <message>
        <source>Maximum Value:</source>
        <translation>Wartość maksymalna:</translation>
    </message>
    <message>
        <source>Outline Width:</source>
        <translation>Szerokość obrysu:</translation>
    </message>
    <message>
        <source>Minimum Value:</source>
        <translation>Wartość minimalna:</translation>
    </message>
    <message>
        <source>Classification Field:</source>
        <translation>Pole klasyfikacji:</translation>
    </message>
    <message>
        <source>Draw polygon outline</source>
        <translation>Rysuj obrys poligonu</translation>
    </message>
</context>
<context>
    <name>QgsCoordinateTransform</name>
    <message>
        <source>Failed</source>
        <translation>Nieudana</translation>
    </message>
    <message>
        <source>transform of</source>
        <translation>transformacja</translation>
    </message>
    <message>
        <source>with error: </source>
        <translation>z błędem: </translation>
    </message>
    <message>
        <source>The source spatial reference system (SRS) is not valid. </source>
        <translation type="obsolete">Źródłowy system referencyjny (SRS) jest nieprawidłowy. </translation>
    </message>
    <message>
        <source>The coordinates can not be reprojected. The SRS is: </source>
        <translation type="obsolete">Współrzędne nie mogą być przeliczone. Aktualny SRS: </translation>
    </message>
    <message>
        <source>The destination spatial reference system (SRS) is not valid. </source>
        <translation type="obsolete">Docelowy system referencyjny (SRS) jest nieprawidłowy. </translation>
    </message>
    <message>
        <source>The source spatial reference system (CRS) is not valid. </source>
        <translation>Źródłowy system odniesienia oparty na współrzędnych (SOW) jest nieprawidłowy.</translation>
    </message>
    <message>
        <source>The coordinates can not be reprojected. The CRS is: </source>
        <translation>Współrzędne nie mogą być przeliczone. SOW to:</translation>
    </message>
    <message>
        <source>The destination spatial reference system (CRS) is not valid. </source>
        <translation>Docelowy system odniesienia oparty na współrzędnych (SOW) jest nieprawidłowy.</translation>
    </message>
</context>
<context>
    <name>QgsCopyrightLabelPlugin</name>
    <message>
        <source>Bottom Left</source>
        <translation>Lewy dolny</translation>
    </message>
    <message>
        <source>Top Left</source>
        <translation>Lewy górny</translation>
    </message>
    <message>
        <source>Top Right</source>
        <translation>Prawy górny</translation>
    </message>
    <message>
        <source>Bottom Right</source>
        <translation>Prawy dolny</translation>
    </message>
    <message>
        <source>&amp;Copyright Label</source>
        <translation>Informa&amp;cja o prawach autorskich</translation>
    </message>
    <message>
        <source>Creates a copyright label that is displayed on the map canvas.</source>
        <translation>Tworzy etykietę o prawach autorskich wyświetlaną w obszarze mapie.</translation>
    </message>
    <message>
        <source>&amp;Decorations</source>
        <translation>&amp;Dekoracje</translation>
    </message>
</context>
<context>
    <name>QgsCopyrightLabelPluginGuiBase</name>
    <message>
        <source>Copyright Label Plugin</source>
        <translation>Informacja o prawach autorskich</translation>
    </message>
    <message>
        <source>Placement</source>
        <translation>Umiejscowienie</translation>
    </message>
    <message>
        <source>Bottom Left</source>
        <translation>Lewy dolny</translation>
    </message>
    <message>
        <source>Top Left</source>
        <translation>Lewy górny</translation>
    </message>
    <message>
        <source>Bottom Right</source>
        <translation>Prawy dolny</translation>
    </message>
    <message>
        <source>Top Right</source>
        <translation>Prawy górny</translation>
    </message>
    <message>
        <source>Orientation</source>
        <translation>Orientacja</translation>
    </message>
    <message>
        <source>Horizontal</source>
        <translation>Poziomo</translation>
    </message>
    <message>
        <source>Vertical</source>
        <translation>Pionowo</translation>
    </message>
    <message>
        <source>Enable Copyright Label</source>
        <translation>Włącz etykietę o prawach autorskich</translation>
    </message>
    <message>
        <source>Color</source>
        <translation>Kolor</translation>
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
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;p, li { white-space: pre-wrap; }&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:12pt;&quot;&gt;Opis&lt;/span&gt;&lt;/p&gt;&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Wpisz poniżej informację o prawach autorskich. Wtyczka obsługuje podstawowe znaczniki html do formatowania tekstu. Na przykład:&lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-weight:600;&quot;&gt;&amp;lt;B&amp;gt; pogrubiony &amp;lt;/B&amp;gt; &lt;/span&gt;&lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-weight:600;&quot;&gt;&lt;span style=&quot; font-weight:400; font-style:italic;&quot;&gt;&amp;lt;I&amp;gt; pochylony &amp;lt;/I&amp;gt;&lt;/span&gt;&lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-style:italic;&quot;&gt;&lt;span style=&quot; font-style:normal;&quot;&gt;(uwaga: &amp;amp;copy; wyświetla symbol copyright)&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>&#xa9; QGIS 2008</source>
        <translation type="obsolete">© QGIS 2008</translation>
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
        <translation type="unfinished"></translation>
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
        <translation>Usunąć definicję odwzorowania?</translation>
    </message>
    <message>
        <source>Deleting a projection definition is not reversable. Do you want to delete it?</source>
        <translation>Usunięcie definicji odwzorowania jest nieodwracalne. Czy chcesz ją usunąć?</translation>
    </message>
    <message>
        <source>Abort</source>
        <translation>Anuluj</translation>
    </message>
    <message>
        <source>New</source>
        <translation>Nowy</translation>
    </message>
    <message>
        <source>QGIS Custom Projection</source>
        <translation>Odwzorowanie użytkownika</translation>
    </message>
    <message>
        <source>This proj4 projection definition is not valid. Please give the projection a name before pressing save.</source>
        <translation>Ta definicja odwzorowania proj4 jest niepoprawna. Odwzorowanie powinno posiadać nazwę.</translation>
    </message>
    <message>
        <source>This proj4 projection definition is not valid. Please add the parameters before pressing save.</source>
        <translation>Ta definicja odwzorowania proj4 jest niepoprawna. Proszę uzupełnić parametery przed zapisem.</translation>
    </message>
    <message>
        <source>This proj4 projection definition is not valid. Please add a proj= clause before pressing save.</source>
        <translation>Ta definicja odwzorowania proj4 jest niepoprawna. Proszę dodać człon proj= przed zapisem.</translation>
    </message>
    <message>
        <source>This proj4 projection definition is not valid. Please correct before pressing save.</source>
        <translation>Ta definicja odwzorowania proj4 jest niepoprawna. Proszę poprawić przed zapisem.</translation>
    </message>
    <message>
        <source>This proj4 projection definition is not valid.</source>
        <translation>Ta definicja odwzorowania proj4 jest niepoprawna.</translation>
    </message>
    <message>
        <source>Northing and Easthing must be in decimal form.</source>
        <translation>Północ i Wschód muszą być zapisane w formacie dziesiętnym.</translation>
    </message>
    <message>
        <source>Internal Error (source projection invalid?)</source>
        <translation>Błąd wewnętrzny (nieprawidłowe odwzorowanie źródłowe?)</translation>
    </message>
</context>
<context>
    <name>QgsCustomProjectionDialogBase</name>
    <message>
        <source>Custom Projection Definition</source>
        <translation type="obsolete">Definicja własnego układu współrzędnych</translation>
    </message>
    <message>
        <source>Define</source>
        <translation>Definiuj</translation>
    </message>
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
        <translation>1 z 1</translation>
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
        <source>Test</source>
        <translation>Testuj</translation>
    </message>
    <message>
        <source>Calculate</source>
        <translation>Przelicz</translation>
    </message>
    <message>
        <source>Geographic / WGS84</source>
        <translation>Geograficzny /WGS84</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Nazwa</translation>
    </message>
    <message>
        <source>Parameters</source>
        <translation>Parametry</translation>
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
        <translation>Północ</translation>
    </message>
    <message>
        <source>East</source>
        <translation>Wschód</translation>
    </message>
    <message>
        <source>Custom Coordinate Reference System Definition</source>
        <translation>Definicja użytkownika Systemu Odniesienia opartego na Współrzędnych (SOW)</translation>
    </message>
    <message>
        <source>You can define your own custom Coordinate Reference System (CRS) here. The definition must conform to the proj4 format for specifying a CRS.</source>
        <translation>Możesz zdefiniować tutaj swój własny System Odniesienia oparty na Współrzędnych (SOW). Definicja SOW musi być zgodna z formatem proj4.</translation>
    </message>
    <message>
        <source>Use the text boxes below to test the CRS definition you are creating. Enter a coordinate where both the lat/long and the transformed result are known (for example by reading off a map). Then press the calculate button to see if the CRS definition you are creating is accurate.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Destination CRS        </source>
        <translation>Docelowy SOW</translation>
    </message>
</context>
<context>
    <name>QgsDbSourceSelect</name>
    <message>
        <source>Are you sure you want to remove the </source>
        <translation>Czy na pewno chcesz usunąć </translation>
    </message>
    <message>
        <source> connection and all associated settings?</source>
        <translation>połączenie i wszystkie związane z nim ustawienia?</translation>
    </message>
    <message>
        <source>Confirm Delete</source>
        <translation>Zatwierdź usunięcie</translation>
    </message>
    <message>
        <source>Select Table</source>
        <translation>Wybierz Tabelę</translation>
    </message>
    <message>
        <source>You must select a table in order to add a Layer.</source>
        <translation>Musisz wybrać tabelę aby dodać warstwę.</translation>
    </message>
    <message>
        <source>Password for </source>
        <translation>Hasło dla</translation>
    </message>
    <message>
        <source>Please enter your password:</source>
        <translation>Proszę podać hasło:</translation>
    </message>
    <message>
        <source>Connection failed</source>
        <translation>Połączenie nie powiodło się</translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Wildcard</source>
        <translation>Maska</translation>
    </message>
    <message>
        <source>RegExp</source>
        <translation>RegExp</translation>
    </message>
    <message>
        <source>All</source>
        <translation>Wszystko</translation>
    </message>
    <message>
        <source>Schema</source>
        <translation>Schemat</translation>
    </message>
    <message>
        <source>Table</source>
        <translation>Tabela</translation>
    </message>
    <message>
        <source>Geometry column</source>
        <translation>Kolumna geometrii</translation>
    </message>
    <message>
        <source>Accessible tables could not be determined</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Database connection was successful, but the accessible tables could not be determined.

The error message from the database was:
%1
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>No accessible tables found</source>
        <translation>Nie znaleziono dostępnych tabel</translation>
    </message>
    <message>
        <source>Database connection was successful, but no accessible tables were found.

Please verify that you have SELECT privilege on a table carrying PostGIS
geometry.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsDbSourceSelectBase</name>
    <message>
        <source>Add PostGIS Table(s)</source>
        <translation>Dodaj tabelę PostGIS</translation>
    </message>
    <message>
        <source>PostgreSQL Connections</source>
        <translation>Połączenie PostgreSQL</translation>
    </message>
    <message>
        <source>Connect</source>
        <translation>Połącz</translation>
    </message>
    <message>
        <source>New</source>
        <translation>Nowy</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation>Edytuj</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>Usuń</translation>
    </message>
    <message>
        <source>Help</source>
        <translation>Pomoc</translation>
    </message>
    <message>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <source>Add</source>
        <translation>Dodaj</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Zamknij</translation>
    </message>
    <message>
        <source>Search:</source>
        <translation>Szukaj:</translation>
    </message>
    <message>
        <source>Search mode:</source>
        <translation>Tryb poszukiwań:</translation>
    </message>
    <message>
        <source>Search in columns:</source>
        <translation>Szukaj w kolumnach:</translation>
    </message>
    <message>
        <source>Search options...</source>
        <translation>Opcje poszukiwań...</translation>
    </message>
</context>
<context>
    <name>QgsDbTableModel</name>
    <message>
        <source>Schema</source>
        <translation>Schemat</translation>
    </message>
    <message>
        <source>Table</source>
        <translation>Tabela</translation>
    </message>
    <message>
        <source>Type</source>
        <translation>Typ</translation>
    </message>
    <message>
        <source>Geometry column</source>
        <translation>Kolumna geometrii</translation>
    </message>
    <message>
        <source>Sql</source>
        <translation>Sql</translation>
    </message>
    <message>
        <source>Point</source>
        <translation>Punkt</translation>
    </message>
    <message>
        <source>Multipoint</source>
        <translation>Wielopunkt</translation>
    </message>
    <message>
        <source>Line</source>
        <translation>Linia</translation>
    </message>
    <message>
        <source>Multiline</source>
        <translation>Wielolinia</translation>
    </message>
    <message>
        <source>Polygon</source>
        <translation>Poligon</translation>
    </message>
    <message>
        <source>Multipolygon</source>
        <translation>Wielopoligon</translation>
    </message>
</context>
<context>
    <name>QgsDelAttrDialogBase</name>
    <message>
        <source>Delete Attributes</source>
        <translation>Usuń atrybuty</translation>
    </message>
</context>
<context>
    <name>QgsDelimitedTextPlugin</name>
    <message>
        <source>&amp;Add Delimited Text Layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Add a delimited text file as a map layer. </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The file must have a header row containing the field names. </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>X and Y fields are required and must contain coordinates in decimal units.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Delimited text</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>DelimitedTextLayer</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsDelimitedTextPluginGui</name>
    <message>
        <source>No layer name</source>
        <translation>Brak nazwy warstwy</translation>
    </message>
    <message>
        <source>Please enter a layer name before adding the layer to the map</source>
        <translation>Proszę wprowadzić nazwę warstwy przed dodaniem jej do mapy</translation>
    </message>
    <message>
        <source>No delimiter</source>
        <translation>Brak separatora</translation>
    </message>
    <message>
        <source>Please specify a delimiter prior to parsing the file</source>
        <translation>Proszę zdefiniować separator przez rozpoczęciem przetwarzania pliku</translation>
    </message>
    <message>
        <source>Choose a delimited text file to open</source>
        <translation>Wybierz do otwarcia plik rozdzielany separatorem</translation>
    </message>
    <message>
        <source>Parse</source>
        <translation>Parsuj</translation>
    </message>
    <message>
        <source>Description</source>
        <translation>Opis</translation>
    </message>
    <message>
        <source>Select a delimited text file containing a header row and one or more rows of x and y coordinates that you would like to use as a point layer and this plugin will do the job for you!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Use the layer name box to specify the legend name for the new layer. Use the delimiter box to specify what delimeter is used in your file (e.g. space, comma, tab or a regular expression in Perl style). After choosing a delimiter, press the parse button and select the columns containing the x and y values for the layer.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsDelimitedTextPluginGuiBase</name>
    <message>
        <source>Create a Layer from a Delimited Text File</source>
        <translation>Utwórz warstwę na podstawie pliku rozdzielanego separatorem</translation>
    </message>
    <message>
        <source>&lt;p align=&quot;right&quot;&gt;X field&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;right&quot;&gt;X pole&lt;/p&gt;</translation>
    </message>
    <message>
        <source>Name of the field containing x values</source>
        <translation>Nazwa pola zawierającego wartość x</translation>
    </message>
    <message>
        <source>Name of the field containing x values. Choose a field from the list. The list is generated by parsing the header row of the delimited text file.</source>
        <translation>Nazwa pola zawierającego wartość x. Wybierz pole z listy. Lista generowana jest na podstawie pierwszego wiersza pliku rozdzielanego separatorem.</translation>
    </message>
    <message>
        <source>&lt;p align=&quot;right&quot;&gt;Y field&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;right&quot;&gt;Y pole&lt;/p&gt;</translation>
    </message>
    <message>
        <source>Name of the field containing y values</source>
        <translation>Nazwa pola zawierającego wartość y</translation>
    </message>
    <message>
        <source>Name of the field containing y values. Choose a field from the list. The list is generated by parsing the header row of the delimited text file.</source>
        <translation>Nazwa pola zawierającego wartość y. Wybierz pole z listy. Lista generowana jest na podstawie pierwszego wiersza pliku rozdzielanego separatorem.</translation>
    </message>
    <message>
        <source>Sample text</source>
        <translation>Przykładowy tekst</translation>
    </message>
    <message>
        <source>Delimited Text Layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Delimited text file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Full path to the delimited text file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Full path to the delimited text file. In order to properly parse the fields in the file, the delimiter must be defined prior to entering the file name. Use the Browse button to the right of this field to choose the input file.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Browse to find the delimited text file to be processed</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Use this button to browse to the location of the delimited text file. This button will not be enabled until a delimiter has been entered in the &lt;i&gt;Delimiter&lt;/i&gt; box. Once a file is chosen, the X and Y field drop-down boxes will be populated with the fields from the delimited text file.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Layer name</source>
        <translation>Nazwa warstwy</translation>
    </message>
    <message>
        <source>Name to display in the map legend</source>
        <translation>Nazwa do wyświetlenia w legendzie mapy</translation>
    </message>
    <message>
        <source>Name displayed in the map legend</source>
        <translation>Nazwa wyświetlana w legendzie mapy</translation>
    </message>
    <message>
        <source>Delimiter</source>
        <translation>Separator</translation>
    </message>
    <message>
        <source>Delimiter to use when splitting fields in the text file. The delimiter can be more than one character.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Delimiter to use when splitting fields in the delimited text file. The delimiter can be 1 or more characters in length.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Browse...</source>
        <translation>Przeglądaj...</translation>
    </message>
    <message>
        <source>The delimiter is taken as is</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Plain characters</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The delimiter is a regular expression</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Regular expression</source>
        <translation type="unfinished"></translation>
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
        <translation>Uwaga: następujące wiersze nie zostały wczytane, ponieważ Qgis nie mógł określić wartości dla współrzędnych x oraz y:
</translation>
    </message>
    <message>
        <source>Error</source>
        <translation>Błąd</translation>
    </message>
</context>
<context>
    <name>QgsDetailedItemWidgetBase</name>
    <message>
        <source>Form</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Heading Label</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Detail label</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsDlgPgBufferBase</name>
    <message>
        <source>Buffer features</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Buffer distance in map units:</source>
        <translation>Odległość bufora w jednostkach mapy:</translation>
    </message>
    <message>
        <source>Table name for the buffered layer:</source>
        <translation>Nazwa tabeli dla buforowanej warstwy:</translation>
    </message>
    <message>
        <source>Create unique object id</source>
        <translation>Utwórz unikalny id obiektu</translation>
    </message>
    <message>
        <source>public</source>
        <translation>publiczny</translation>
    </message>
    <message>
        <source>Geometry column:</source>
        <translation>Kolumna geometrii:</translation>
    </message>
    <message>
        <source>Spatial reference ID:</source>
        <translation>ID powiązania przestrzennego:</translation>
    </message>
    <message>
        <source>Unique field to use as feature id:</source>
        <translation>Pole unikalne do użycia jako id obiektu:</translation>
    </message>
    <message>
        <source>Schema:</source>
        <translation>Schemat:</translation>
    </message>
    <message>
        <source>Add the buffered layer to the map?</source>
        <translation>Czy dodać buforowaną warstwę do mapy?</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Buffer the features in layer: &lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Buforuj obiekty na warstwie: &lt;/h2&gt;</translation>
    </message>
    <message>
        <source>Parameters</source>
        <translation>Parametry</translation>
    </message>
</context>
<context>
    <name>QgsEncodingFileDialog</name>
    <message>
        <source>Encoding:</source>
        <translation>Kodowanie:</translation>
    </message>
</context>
<context>
    <name>QgsGPSDeviceDialog</name>
    <message>
        <source>New device %1</source>
        <translation>Nowe urządzenie %1</translation>
    </message>
    <message>
        <source>Are you sure?</source>
        <translation>Czy jesteś pewien?</translation>
    </message>
    <message>
        <source>Are you sure that you want to delete this device?</source>
        <translation>Czy jesteś pewien, że chcesz usunąć to urządzenie?</translation>
    </message>
</context>
<context>
    <name>QgsGPSDeviceDialogBase</name>
    <message>
        <source>GPS Device Editor</source>
        <translation>Edytor urządzenia GPS</translation>
    </message>
    <message>
        <source>Device name:</source>
        <translation type="obsolete">Nazwa urządzenia:</translation>
    </message>
    <message>
        <source>This is the name of the device as it will appear in the lists</source>
        <translation>Nazwa urządzenia, która będzie wyświetlana w liście</translation>
    </message>
    <message>
        <source>Update device</source>
        <translation>Aktualizuj urządzenie</translation>
    </message>
    <message>
        <source>Delete device</source>
        <translation>Usuń urządzenie</translation>
    </message>
    <message>
        <source>New device</source>
        <translation>Nowe urządzenie</translation>
    </message>
    <message>
        <source>Close</source>
        <translation type="obsolete">Zamknij</translation>
    </message>
    <message>
        <source>Commands</source>
        <translation>Komendy</translation>
    </message>
    <message>
        <source>Waypoint download:</source>
        <translation>Import punktów nawigacyjnych:</translation>
    </message>
    <message>
        <source>Waypoint upload:</source>
        <translation>Eksport punktów nawigacyjnych:</translation>
    </message>
    <message>
        <source>Route download:</source>
        <translation>Import tras (route):</translation>
    </message>
    <message>
        <source>Route upload:</source>
        <translation>Eksport tras (route):</translation>
    </message>
    <message>
        <source>Track download:</source>
        <translation>Import śladów (tracków):</translation>
    </message>
    <message>
        <source>The command that is used to upload tracks to the device</source>
        <translation>Komenda używana do eksportu śladów (tracków) do urządzenia</translation>
    </message>
    <message>
        <source>Track upload:</source>
        <translation>Eksport śladów (tracków):</translation>
    </message>
    <message>
        <source>The command that is used to download tracks from the device</source>
        <translation>Polecenie wykorzystywane do importu śladów z urządzenia</translation>
    </message>
    <message>
        <source>The command that is used to upload routes to the device</source>
        <translation>Polecenie wykorzystywane do eksportu tras do urządzenia</translation>
    </message>
    <message>
        <source>The command that is used to download routes from the device</source>
        <translation>Polecenie wykorzystywane do importu tras z urządzenia</translation>
    </message>
    <message>
        <source>The command that is used to upload waypoints to the device</source>
        <translation>Polecenie wykorzystywane do eksportu punktów nawigacyjnych do urządzenia</translation>
    </message>
    <message>
        <source>The command that is used to download waypoints from the device</source>
        <translation>Polecenie wykorzystywane do importu punktów nawigacyjnych z urządzenia</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;In the download and upload commands there can be special words that will be replaced by QGIS when the commands are used. These words are:&lt;span style=&quot; font-style:italic;&quot;&gt;%babel&lt;/span&gt; - the path to GPSBabel&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%in&lt;/span&gt; - the GPX filename when uploading or the port when downloading&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%out&lt;/span&gt; - the port when uploading or the GPX filename when downloading&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;W poleceniach do wysyłania i pobierania można użyć specjalnych wyrażeń, które zostaną odpowiednio zastąpione podczas użycia tych poleceń. Te wyrażenia to:&lt;span style=&quot; font-style:italic;&quot;&gt;%babel&lt;/span&gt; - ścieżka do GPSBabel&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%in&lt;/span&gt; - nazwa pliku GPX podczas eksportu lub port podczas importu&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%out&lt;/span&gt; - port podczas eksportu lub nazwa pliku GPX podczas importu&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Device name</source>
        <translation>Nazwa urządzenia</translation>
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
        <translation>Narzędzie &amp;GPS</translation>
    </message>
    <message>
        <source>&amp;Create new GPX layer</source>
        <translation>&amp;Utwórz nową warstwę GPX</translation>
    </message>
    <message>
        <source>Creates a new GPX layer and displays it on the map canvas</source>
        <translation>Tworzy nową warstwę GPX i wyświetla ją w obszarze mapy</translation>
    </message>
    <message>
        <source>&amp;Gps</source>
        <translation>&amp;GPS</translation>
    </message>
    <message>
        <source>Save new GPX file as...</source>
        <translation>Zapisz nowy plik GPX jako...</translation>
    </message>
    <message>
        <source>GPS eXchange file (*.gpx)</source>
        <translation>plik wymiany z GPS (*.gpx)</translation>
    </message>
    <message>
        <source>Could not create file</source>
        <translation>Nie mogę utworzyć pliku</translation>
    </message>
    <message>
        <source>Unable to create a GPX file with the given name. </source>
        <translation>Nie mogę utworzyć plku GPX z nadaną nazwą. </translation>
    </message>
    <message>
        <source>Try again with another name or in another </source>
        <translation>Spróbuj ponownie z inną nazwą lub w innym</translation>
    </message>
    <message>
        <source>directory.</source>
        <translation>katalogu.</translation>
    </message>
    <message>
        <source>GPX Loader</source>
        <translation>Import GPX</translation>
    </message>
    <message>
        <source>Unable to read the selected file.
</source>
        <translation>Nie mogę odczytać zaznaczonego pliku.</translation>
    </message>
    <message>
        <source>Please reselect a valid file.</source>
        <translation>Zaznacz proszę prawidłowy plik.</translation>
    </message>
    <message>
        <source>Could not start process</source>
        <translation>Nie mogę uruchomić procesu</translation>
    </message>
    <message>
        <source>Could not start GPSBabel!</source>
        <translation>Nie mogę uruchomić GPSBabel!</translation>
    </message>
    <message>
        <source>Importing data...</source>
        <translation>Importuję dane...</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Anuluj</translation>
    </message>
    <message>
        <source>Could not import data from %1!

</source>
        <translation>Nie mogę zaimportować danych z %1!</translation>
    </message>
    <message>
        <source>Error importing data</source>
        <translation>Błąd pobierania danych</translation>
    </message>
    <message>
        <source>Not supported</source>
        <translation>Nieobsługiwane</translation>
    </message>
    <message>
        <source>This device does not support downloading </source>
        <translation>To urządzenie nie umożliwia pobierania danych</translation>
    </message>
    <message>
        <source>of </source>
        <translation>z</translation>
    </message>
    <message>
        <source>Downloading data...</source>
        <translation>Pobieram dane...</translation>
    </message>
    <message>
        <source>Could not download data from GPS!

</source>
        <translation>Nie mogę pobrać danych z GPS!

</translation>
    </message>
    <message>
        <source>Error downloading data</source>
        <translation>Błąd pobierania danych</translation>
    </message>
    <message>
        <source>This device does not support uploading of </source>
        <translation>To urządzenie nie umożliwia wgrywania </translation>
    </message>
    <message>
        <source>Uploading data...</source>
        <translation>Eksportuję dane...</translation>
    </message>
    <message>
        <source>Error while uploading data to GPS!

</source>
        <translation>Błąd podczas eksportowania danych do GPS!

</translation>
    </message>
    <message>
        <source>Error uploading data</source>
        <translation>Błąd eksportu danych</translation>
    </message>
    <message>
        <source>Could not convert data from %1!

</source>
        <translation>Nie mogę przekonwertować danych z %1!

</translation>
    </message>
    <message>
        <source>Error converting data</source>
        <translation>Błąd podczas konwersji danych</translation>
    </message>
</context>
<context>
    <name>QgsGPSPluginGui</name>
    <message>
        <source>Choose a filename to save under</source>
        <translation type="obsolete">Wybierz nazwę pliku pod jaką mam zapisać</translation>
    </message>
    <message>
        <source>GPS eXchange format (*.gpx)</source>
        <translation>Format wymiany z GPS (*.gpx)</translation>
    </message>
    <message>
        <source>Select GPX file</source>
        <translation>Wybierz plik GPX</translation>
    </message>
    <message>
        <source>Select file and format to import</source>
        <translation>Wybierz plik i format do zaimportowania </translation>
    </message>
    <message>
        <source>Waypoints</source>
        <translation>Punkty nawigacyjne</translation>
    </message>
    <message>
        <source>Routes</source>
        <translation>Trasy</translation>
    </message>
    <message>
        <source>Tracks</source>
        <translation>Ślady</translation>
    </message>
    <message>
        <source>QGIS can perform conversions of GPX files, by using GPSBabel (%1) to perform the conversions.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>This requires that you have GPSBabel installed where QGIS can find it.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>GPX is the %1, which is used to store information about waypoints, routes, and tracks.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>GPS eXchange file format</source>
        <translation>Format pliku GPS eXchange</translation>
    </message>
    <message>
        <source>Select a GPX file and then select the feature types that you want to load.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>This tool will help you download data from a GPS device.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Choose your GPS device, the port it is connected to, the feature type you want to download, a name for your new layer, and the GPX file where you want to store the data.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>If your device isn&apos;t listed, or if you want to change some settings, you can also edit the devices.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>This tool uses the program GPSBabel (%1) to transfer the data.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>This tool will help you upload data from a GPX layer to a GPS device.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Choose the layer you want to upload, the device you want to upload it to, and the port your device is connected to.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>QGIS can only load GPX files by itself, but many other formats can be converted to GPX using GPSBabel (%1).</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>All file formats can not store waypoints, routes, and tracks, so some feature types may be disabled for some file formats.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Choose a file name to save under</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Select a GPS file format and the file that you want to import, the feature type that you want to use, a GPX file name that you want to save the converted file as, and a name for the new layer.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Select a GPX input file name, the type of conversion you want to perform, a GPX file name that you want to save the converted file as, and a name for the new layer created from the result.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGPSPluginGuiBase</name>
    <message>
        <source>GPS Tools</source>
        <translation>Narzędzia GPS</translation>
    </message>
    <message>
        <source>Load GPX file</source>
        <translation>Załaduj plik GPX</translation>
    </message>
    <message>
        <source>File:</source>
        <translation>Plik:</translation>
    </message>
    <message>
        <source>Feature types:</source>
        <translation>Typy obiektów:</translation>
    </message>
    <message>
        <source>Waypoints</source>
        <translation>Punkty nawigacyjne</translation>
    </message>
    <message>
        <source>Routes</source>
        <translation>Trasy</translation>
    </message>
    <message>
        <source>Tracks</source>
        <translation>Ślady</translation>
    </message>
    <message>
        <source>Import other file</source>
        <translation>Zaimportuj inny plik</translation>
    </message>
    <message>
        <source>File to import:</source>
        <translation>Plik do zaimportowania:</translation>
    </message>
    <message>
        <source>Feature type:</source>
        <translation>Typ obiektu:</translation>
    </message>
    <message>
        <source>GPX output file:</source>
        <translation>Wyjściowy plik GPX:</translation>
    </message>
    <message>
        <source>Layer name:</source>
        <translation>Nazwa warstwy:</translation>
    </message>
    <message>
        <source>Download from GPS</source>
        <translation>Pobierz z GPS</translation>
    </message>
    <message>
        <source>Edit devices</source>
        <translation>Edytuj urządzenia</translation>
    </message>
    <message>
        <source>GPS device:</source>
        <translation>Urządzenie GPS:</translation>
    </message>
    <message>
        <source>Output file:</source>
        <translation>Plik wyjściowy:</translation>
    </message>
    <message>
        <source>Port:</source>
        <translation>Port:</translation>
    </message>
    <message>
        <source>Upload to GPS</source>
        <translation>Załaduj do GPS</translation>
    </message>
    <message>
        <source>Data layer:</source>
        <translation>Warstwa danych:</translation>
    </message>
    <message>
        <source>Browse...</source>
        <translation>Przeglądaj...</translation>
    </message>
    <message>
        <source>Save As...</source>
        <translation>Zapisz jako...</translation>
    </message>
    <message>
        <source>(Note: Selecting correct file type in browser dialog important!)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>GPX Conversions</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Conversion:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>GPX input file:</source>
        <translation>Wejściowy plik GPX:</translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Refresh</source>
        <translation>Odśwież</translation>
    </message>
</context>
<context>
    <name>QgsGPXProvider</name>
    <message>
        <source>Bad URI - you need to specify the feature type.</source>
        <translation>Błąd - musisz określić typ danych.</translation>
    </message>
    <message>
        <source>GPS eXchange file</source>
        <translation>Plik wymiany GPS</translation>
    </message>
    <message>
        <source>Digitized in QGIS</source>
        <translation>Zdigitalizowane w QGIS</translation>
    </message>
</context>
<context>
    <name>QgsGenericProjectionSelector</name>
    <message>
        <source>Define this layer&apos;s projection:</source>
        <translation>Zdefiniuj odwzorowanie tej warstwy:</translation>
    </message>
    <message>
        <source>This layer appears to have no projection specification.</source>
        <translation>Warstwa wydaje się nie posiadać określnego odwzorowania.</translation>
    </message>
    <message>
        <source>By default, this layer will now have its projection set to that of the project, but you may override this by selecting a different projection below.</source>
        <translation>Domyślnie ta warstwa będzie miała teraz odwzorowanie tożsame z projektem, ale możesz to zmienić wybierając inne odwzorowanie poniżej.</translation>
    </message>
</context>
<context>
    <name>QgsGenericProjectionSelectorBase</name>
    <message>
        <source>Projection Selector</source>
        <translation>Wybór odwzorowania</translation>
    </message>
</context>
<context>
    <name>QgsGeomTypeDialog</name>
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
        <source>Type</source>
        <translation>Typ</translation>
    </message>
    <message>
        <source>Point</source>
        <translation>Punkt</translation>
    </message>
    <message>
        <source>Line</source>
        <translation>Linia</translation>
    </message>
    <message>
        <source>Polygon</source>
        <translation>Poligon</translation>
    </message>
    <message>
        <source>New Vector Layer</source>
        <translation>Nowa warstwa wektorowa</translation>
    </message>
    <message>
        <source>File format</source>
        <translation>Format pliku</translation>
    </message>
    <message>
        <source>Attributes</source>
        <translation>Atrybuty</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Nazwa</translation>
    </message>
    <message>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <source>Delete selected attribute</source>
        <translation>Usuń wybrany atrybut</translation>
    </message>
    <message>
        <source>Add attribute</source>
        <translation>Dodaj atrybut</translation>
    </message>
</context>
<context>
    <name>QgsGeorefDescriptionDialogBase</name>
    <message>
        <source>Description georeferencer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
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
        <source>&amp;Georeferencer</source>
        <translation>&amp;Georeferencer</translation>
    </message>
</context>
<context>
    <name>QgsGeorefPluginGui</name>
    <message>
        <source>Choose a raster file</source>
        <translation>Wybierz plik rastrowy</translation>
    </message>
    <message>
        <source>Raster files (*.*)</source>
        <translation>Rastry (*.*)</translation>
    </message>
    <message>
        <source>Error</source>
        <translation>Błąd</translation>
    </message>
    <message>
        <source>The selected file is not a valid raster file.</source>
        <translation>Wybrany plik nie jest prawidłowym plikiem rastrowym.</translation>
    </message>
    <message>
        <source>World file exists</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;p&gt;The selected file already seems to have a </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>world file! Do you want to replace it with the </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>new world file?&lt;/p&gt;</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGeorefPluginGuiBase</name>
    <message>
        <source>Georeferencer</source>
        <translation>Georeferencer</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Zamknij</translation>
    </message>
    <message>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <source>Raster file:</source>
        <translation>Raster:</translation>
    </message>
    <message>
        <source>Arrange plugin windows</source>
        <translation>Rozmieść okna wtyczki</translation>
    </message>
    <message>
        <source>Description...</source>
        <translation>Opis...</translation>
    </message>
</context>
<context>
    <name>QgsGeorefWarpOptionsDialogBase</name>
    <message>
        <source>Warp options</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Resampling method:</source>
        <translation>Metoda interpolacji:</translation>
    </message>
    <message>
        <source>Nearest neighbour</source>
        <translation>Najbliższe sąsiedztwo</translation>
    </message>
    <message>
        <source>Linear</source>
        <translation>Liniowa</translation>
    </message>
    <message>
        <source>Cubic</source>
        <translation>Sześcienna</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>Use 0 for transparency when needed</source>
        <translation>Użyj 0 dla przeźroczystości jeżeli wymagane</translation>
    </message>
    <message>
        <source>Compression:</source>
        <translation>Kompresja:</translation>
    </message>
</context>
<context>
    <name>QgsGraduatedSymbolDialog</name>
    <message>
        <source>Equal Interval</source>
        <translation>Równe przedziały</translation>
    </message>
    <message>
        <source>Quantiles</source>
        <translation>Kwantyle</translation>
    </message>
    <message>
        <source>Empty</source>
        <translation>Pusty</translation>
    </message>
</context>
<context>
    <name>QgsGraduatedSymbolDialogBase</name>
    <message>
        <source>graduated Symbol</source>
        <translation>Symbol stopniowy</translation>
    </message>
    <message>
        <source>Delete class</source>
        <translation>Usuń klasę</translation>
    </message>
    <message>
        <source>Classify</source>
        <translation>Klasyfikuj</translation>
    </message>
    <message>
        <source>Classification field</source>
        <translation>Pole klasyfikacji</translation>
    </message>
    <message>
        <source>Mode</source>
        <translation>Tryb</translation>
    </message>
    <message>
        <source>Number of classes</source>
        <translation>Liczba klas</translation>
    </message>
</context>
<context>
    <name>QgsGrassAttributes</name>
    <message>
        <source>Warning</source>
        <translation>Ostrzeżenie</translation>
    </message>
    <message>
        <source>Column</source>
        <translation>Kolumna</translation>
    </message>
    <message>
        <source>Value</source>
        <translation>Wartość</translation>
    </message>
    <message>
        <source>Type</source>
        <translation>Typ</translation>
    </message>
    <message>
        <source>ERROR</source>
        <translation>BŁĄD</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>Layer</source>
        <translation>Warstwa</translation>
    </message>
</context>
<context>
    <name>QgsGrassAttributesBase</name>
    <message>
        <source>GRASS Attributes</source>
        <translation>Atrybuty GRASS</translation>
    </message>
    <message>
        <source>Tab 1</source>
        <translation>Zakładka 1</translation>
    </message>
    <message>
        <source>result</source>
        <translation>wynik</translation>
    </message>
    <message>
        <source>Update</source>
        <translation>Aktualizuj</translation>
    </message>
    <message>
        <source>Update database record</source>
        <translation>Aktualizuj rekord w bazie danych</translation>
    </message>
    <message>
        <source>New</source>
        <translation>Nowy</translation>
    </message>
    <message>
        <source>Add new category using settings in GRASS Edit toolbox</source>
        <translation>Dodaj nową kategorię używając ustawień w GRASS Edit toolbox</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>Usuń</translation>
    </message>
    <message>
        <source>Delete selected category</source>
        <translation>Usuń wybraną kategorię</translation>
    </message>
</context>
<context>
    <name>QgsGrassBrowser</name>
    <message>
        <source>Tools</source>
        <translation>Narzędzia</translation>
    </message>
    <message>
        <source>Add selected map to canvas</source>
        <translation>Dodaj wybraną mapę do obszaru mapy</translation>
    </message>
    <message>
        <source>Copy selected map</source>
        <translation>Kopiuj zaznaczoną mapę</translation>
    </message>
    <message>
        <source>Rename selected map</source>
        <translation>Zmień nazwę zaznaczonej mapy</translation>
    </message>
    <message>
        <source>Delete selected map</source>
        <translation>Usuń zaznaczoną mapę</translation>
    </message>
    <message>
        <source>Set current region to selected map</source>
        <translation>Ustaw bieżący obszar do wybranej mapy</translation>
    </message>
    <message>
        <source>Refresh</source>
        <translation>Odśwież</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Ostrzeżenie</translation>
    </message>
    <message>
        <source>Cannot copy map </source>
        <translation>Nie mogę skopiować mapy</translation>
    </message>
    <message>
        <source>&lt;br&gt;command: </source>
        <translation>&lt;br&gt;komenda: </translation>
    </message>
    <message>
        <source>Cannot rename map </source>
        <translation>Nie mogę zmienić nazwy mapy</translation>
    </message>
    <message>
        <source>Delete map &lt;b&gt;</source>
        <translation>Usuń mapę &lt;b&gt;</translation>
    </message>
    <message>
        <source>Cannot delete map </source>
        <translation>Nie mogę usunąć mapy</translation>
    </message>
    <message>
        <source>Cannot write new region</source>
        <translation>Nie można zapisać nowego regionu</translation>
    </message>
    <message>
        <source>New name</source>
        <translation>Nowa nazwa</translation>
    </message>
</context>
<context>
    <name>QgsGrassEdit</name>
    <message>
        <source>New point</source>
        <translation>Nowy punkt</translation>
    </message>
    <message>
        <source>New centroid</source>
        <translation>Nowy centroid</translation>
    </message>
    <message>
        <source>Delete vertex</source>
        <translation>Usuń wierzchołek</translation>
    </message>
    <message>
        <source>Left: </source>
        <translation>Lewy: </translation>
    </message>
    <message>
        <source>Middle: </source>
        <translation>Środkowy: </translation>
    </message>
    <message>
        <source>Edit tools</source>
        <translation>Narzędzia edycji</translation>
    </message>
    <message>
        <source>New line</source>
        <translation>Nowa linia</translation>
    </message>
    <message>
        <source>New boundary</source>
        <translation>Nowy zasięg</translation>
    </message>
    <message>
        <source>Move vertex</source>
        <translation>Przesuń wierzchołek</translation>
    </message>
    <message>
        <source>Add vertex</source>
        <translation>Dodaj wierzchołek</translation>
    </message>
    <message>
        <source>Move element</source>
        <translation>Przesuń element</translation>
    </message>
    <message>
        <source>Split line</source>
        <translation>Podziel linię</translation>
    </message>
    <message>
        <source>Delete element</source>
        <translation>Usuń element</translation>
    </message>
    <message>
        <source>Edit attributes</source>
        <translation>Edytuj atrybuty</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Zamknij</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Ostrzeżenie</translation>
    </message>
    <message>
        <source>You are not owner of the mapset, cannot open the vector for editing.</source>
        <translation>Nie jesteś właścicielem mapsetu, nie możesz otworzyć wektora do edycji.</translation>
    </message>
    <message>
        <source>Cannot open vector for update.</source>
        <translation>Nie można otworzyć wektora do aktualizacji.</translation>
    </message>
    <message>
        <source>Info</source>
        <translation>Informacje</translation>
    </message>
    <message>
        <source>The table was created</source>
        <translation>Tabela została stworzona</translation>
    </message>
    <message>
        <source>Tool not yet implemented.</source>
        <translation>Narzędzie jeszcze niedostępne.</translation>
    </message>
    <message>
        <source>Cannot check orphan record: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Orphan record was left in attribute table. &lt;br&gt;Delete the record?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot delete orphan record: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot describe table for field </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Background</source>
        <translation>Tło</translation>
    </message>
    <message>
        <source>Highlight</source>
        <translation>Wybrane</translation>
    </message>
    <message>
        <source>Dynamic</source>
        <translation>Dynamiczne</translation>
    </message>
    <message>
        <source>Point</source>
        <translation>Punkt</translation>
    </message>
    <message>
        <source>Line</source>
        <translation>Linia</translation>
    </message>
    <message>
        <source>Boundary (no area)</source>
        <translation>Granica (bez obszaru)</translation>
    </message>
    <message>
        <source>Boundary (1 area)</source>
        <translation>Granica (1 obszar)</translation>
    </message>
    <message>
        <source>Boundary (2 areas)</source>
        <translation>Granica (2 obszary)</translation>
    </message>
    <message>
        <source>Centroid (in area)</source>
        <translation>Centroid (w obrysie)</translation>
    </message>
    <message>
        <source>Centroid (outside area)</source>
        <translation>Centroid (poza obrysem)</translation>
    </message>
    <message>
        <source>Centroid (duplicate in area)</source>
        <translation>Centroid (duplikat w obrysie)</translation>
    </message>
    <message>
        <source>Node (1 line)</source>
        <translation>Węzeł (1 linia)</translation>
    </message>
    <message>
        <source>Node (2 lines)</source>
        <translation>Węzeł (2 linie)</translation>
    </message>
    <message>
        <source>Type</source>
        <translation type="obsolete">Typ</translation>
    </message>
    <message>
        <source>Length</source>
        <translation type="obsolete">Długość</translation>
    </message>
    <message>
        <source>Next not used</source>
        <translation>Najbliższa wolna</translation>
    </message>
    <message>
        <source>Manual entry</source>
        <translation>Wprowadzanie ręczne</translation>
    </message>
    <message>
        <source>No category</source>
        <translation>Bez kategorii</translation>
    </message>
    <message>
        <source>Right: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Color</source>
        <comment>
Column title</comment>
        <translation type="obsolete">Kolor</translation>
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
        <translation type="obsolete">Indeks</translation>
    </message>
</context>
<context>
    <name>QgsGrassEditBase</name>
    <message>
        <source>GRASS Edit</source>
        <translation>Edytor GRASS</translation>
    </message>
    <message>
        <source>Category</source>
        <translation>Kategoria</translation>
    </message>
    <message>
        <source>Mode</source>
        <translation>Tryb</translation>
    </message>
    <message>
        <source>Settings</source>
        <translation>Ustawienia</translation>
    </message>
    <message>
        <source>Snapping in screen pixels</source>
        <translation>Dociąganie w pikselach ekranowych</translation>
    </message>
    <message>
        <source>Symbology</source>
        <translation>Symbolika</translation>
    </message>
    <message>
        <source>Column 1</source>
        <translation type="obsolete">Kolumna 1</translation>
    </message>
    <message>
        <source>Table</source>
        <translation>Tabela</translation>
    </message>
    <message>
        <source>Add Column</source>
        <translation>Dodaj kolumnę</translation>
    </message>
    <message>
        <source>Create / Alter Table</source>
        <translation>Utwórz / Zmień tabelę</translation>
    </message>
    <message>
        <source>Line width</source>
        <translation>Szerokość linii</translation>
    </message>
    <message>
        <source>Marker size</source>
        <translation>Rozmiar markera</translation>
    </message>
    <message>
        <source>Layer</source>
        <translation>Warstwa</translation>
    </message>
    <message>
        <source>Disp</source>
        <translation>Widoczne</translation>
    </message>
    <message>
        <source>Color</source>
        <translation>Kolor</translation>
    </message>
    <message>
        <source>Type</source>
        <translation>Typ</translation>
    </message>
    <message>
        <source>Index</source>
        <translation>Indeks</translation>
    </message>
    <message>
        <source>Column</source>
        <translation>Kolumna</translation>
    </message>
    <message>
        <source>Length</source>
        <translation>Długość</translation>
    </message>
</context>
<context>
    <name>QgsGrassElementDialog</name>
    <message>
        <source>Cancel</source>
        <translation>Anuluj</translation>
    </message>
    <message>
        <source>Ok</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>&lt;font color=&apos;red&apos;&gt;Enter a name!&lt;/font&gt;</source>
        <translation>&lt;font color=&apos;red&apos;&gt;Wpisz nazwę!&lt;/font&gt;</translation>
    </message>
    <message>
        <source>&lt;font color=&apos;red&apos;&gt;This is name of the source!&lt;/font&gt;</source>
        <translation>&lt;font color=&apos;red&apos;&gt;To jest nazwa źródła!&lt;/font&gt;</translation>
    </message>
    <message>
        <source>&lt;font color=&apos;red&apos;&gt;Exists!&lt;/font&gt;</source>
        <translation>&lt;font color=&apos;red&apos;&gt;Istnieje!&lt;/font&gt;</translation>
    </message>
    <message>
        <source>Overwrite</source>
        <translation>Nadpisz</translation>
    </message>
</context>
<context>
    <name>QgsGrassMapcalc</name>
    <message>
        <source>Mapcalc tools</source>
        <translation>Narzędzia mapcalc (algebra mapowa)</translation>
    </message>
    <message>
        <source>Add map</source>
        <translation>Dodaj mapę</translation>
    </message>
    <message>
        <source>Add constant value</source>
        <translation>Dodaj stałą wartość</translation>
    </message>
    <message>
        <source>Add operator or function</source>
        <translation>Dodaj operator lub funkcję</translation>
    </message>
    <message>
        <source>Add connection</source>
        <translation>Dodaj połączenie</translation>
    </message>
    <message>
        <source>Select item</source>
        <translation>Zaznacz obiekt</translation>
    </message>
    <message>
        <source>Delete selected item</source>
        <translation>Usuń zaznaczony obiekt</translation>
    </message>
    <message>
        <source>Open</source>
        <translation>Otwórz</translation>
    </message>
    <message>
        <source>Save</source>
        <translation>Zapisz</translation>
    </message>
    <message>
        <source>Save as</source>
        <translation>Zapisz jako</translation>
    </message>
    <message>
        <source>Addition</source>
        <translation>Dodawanie</translation>
    </message>
    <message>
        <source>Subtraction</source>
        <translation>Odejmowanie</translation>
    </message>
    <message>
        <source>Multiplication</source>
        <translation>Mnożenie</translation>
    </message>
    <message>
        <source>Division</source>
        <translation>Dzielenie</translation>
    </message>
    <message>
        <source>Modulus</source>
        <translation>Moduł</translation>
    </message>
    <message>
        <source>Exponentiation</source>
        <translation>Potęgowanie</translation>
    </message>
    <message>
        <source>Equal</source>
        <translation>Równe</translation>
    </message>
    <message>
        <source>Not equal</source>
        <translation>Nie równe</translation>
    </message>
    <message>
        <source>Greater than</source>
        <translation>Większy niż</translation>
    </message>
    <message>
        <source>Greater than or equal</source>
        <translation>Większy niż lub równy</translation>
    </message>
    <message>
        <source>Less than</source>
        <translation>Mniejszy niż</translation>
    </message>
    <message>
        <source>Less than or equal</source>
        <translation>Mniejszy niż lub równy</translation>
    </message>
    <message>
        <source>And</source>
        <translation>I</translation>
    </message>
    <message>
        <source>Or</source>
        <translation>Lub</translation>
    </message>
    <message>
        <source>Absolute value of x</source>
        <translation>Wartość bezwzględna z x</translation>
    </message>
    <message>
        <source>Inverse tangent of x (result is in degrees)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Inverse tangent of y/x (result is in degrees)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Current column of moving window (starts with 1)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cosine of x (x is in degrees)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Convert x to double-precision floating point</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Current east-west resolution</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Exponential function of x</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>x to the power y</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Convert x to single-precision floating point</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Decision: 1 if x not zero, 0 otherwise</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Decision: a if x not zero, 0 otherwise</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Decision: a if x not zero, b otherwise</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Decision: a if x &gt; 0, b if x is zero, c if x &lt; 0</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Convert x to integer [ truncates ]</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Check if x = NULL</source>
        <translation>Sprawdź czy x = NULL</translation>
    </message>
    <message>
        <source>Natural log of x</source>
        <translation>Logarytm naturalny z x</translation>
    </message>
    <message>
        <source>Log of x base b</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Largest value</source>
        <translation>Największa wartość</translation>
    </message>
    <message>
        <source>Median value</source>
        <translation>Średnia wartość</translation>
    </message>
    <message>
        <source>Smallest value</source>
        <translation>Najniższa wartość</translation>
    </message>
    <message>
        <source>Mode value</source>
        <translation>Wartość modalna</translation>
    </message>
    <message>
        <source>1 if x is zero, 0 otherwise</source>
        <translation>1 jeśli x jest zerem, w przeciwnym razie 0</translation>
    </message>
    <message>
        <source>Current north-south resolution</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>NULL value</source>
        <translation>wartość NULL</translation>
    </message>
    <message>
        <source>Random value between a and b</source>
        <translation>Losowa wartość między a i b</translation>
    </message>
    <message>
        <source>Round x to nearest integer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Current row of moving window (Starts with 1)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Sine of x (x is in degrees)</source>
        <comment>sin(x)</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Square root of x</source>
        <comment>sqrt(x)</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Tangent of x (x is in degrees)</source>
        <comment>tan(x)</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Current x-coordinate of moving window</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Current y-coordinate of moving window</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Ostrzeżenie</translation>
    </message>
    <message>
        <source>Cannot get current region</source>
        <translation>Nie można ustalić bieżącego regionu</translation>
    </message>
    <message>
        <source>Cannot check region of map </source>
        <translation>Nie można sprawdzić regionu mapy</translation>
    </message>
    <message>
        <source>Cannot get region of map </source>
        <translation>Nie można ustalić regionu mapy</translation>
    </message>
    <message>
        <source>No GRASS raster maps currently in QGIS</source>
        <translation>Brak map rastrowych GRASS w QGIS</translation>
    </message>
    <message>
        <source>Cannot create &apos;mapcalc&apos; directory in current mapset.</source>
        <translation>Nie można utworzyć kartoteki &apos;mapcalc&apos; w bieżącym mapsecie.</translation>
    </message>
    <message>
        <source>New mapcalc</source>
        <translation>Nowy mapcalc</translation>
    </message>
    <message>
        <source>Enter new mapcalc name:</source>
        <translation>Wpisz nazwę nowego mapcalc:</translation>
    </message>
    <message>
        <source>Enter vector name</source>
        <translation>Podaj nazwę wektora</translation>
    </message>
    <message>
        <source>The file already exists. Overwrite? </source>
        <translation>Plik istnieje. Nadpisać? </translation>
    </message>
    <message>
        <source>Save mapcalc</source>
        <translation>Zapisz mapcalc</translation>
    </message>
    <message>
        <source>File name empty</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot open mapcalc file</source>
        <translation>Nie można otworzyć pliku mapcalc (</translation>
    </message>
    <message>
        <source>The mapcalc schema (</source>
        <translation>Schemat mapcalc (</translation>
    </message>
    <message>
        <source>) not found.</source>
        <translation>) nie znaleziony.</translation>
    </message>
    <message>
        <source>Cannot open mapcalc schema (</source>
        <translation>Nie można otworzyć schematu mapcalc (</translation>
    </message>
    <message>
        <source>Cannot read mapcalc schema (</source>
        <translation>Nie można odczytać schematu mapcalc (</translation>
    </message>
    <message>
        <source>
at line </source>
        <translation>
w wierszu </translation>
    </message>
    <message>
        <source> column </source>
        <translation>kolumna</translation>
    </message>
    <message>
        <source>Output</source>
        <translation>Wyjście</translation>
    </message>
</context>
<context>
    <name>QgsGrassMapcalcBase</name>
    <message>
        <source>MainWindow</source>
        <translation>Okno główne</translation>
    </message>
    <message>
        <source>Output</source>
        <translation>Wyjście</translation>
    </message>
</context>
<context>
    <name>QgsGrassModule</name>
    <message>
        <source>Run</source>
        <translation>Uruchom</translation>
    </message>
    <message>
        <source>Stop</source>
        <translation>Zatrzymaj</translation>
    </message>
    <message>
        <source>Module</source>
        <translation>Moduł</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Ostrzeżenie</translation>
    </message>
    <message>
        <source>The module file (</source>
        <translation>Plik modułu (</translation>
    </message>
    <message>
        <source>) not found.</source>
        <translation>) nie znaleziony.</translation>
    </message>
    <message>
        <source>Cannot open module file (</source>
        <translation>Nie można otworzyć pliku modułu (</translation>
    </message>
    <message>
        <source>)</source>
        <translation>)</translation>
    </message>
    <message>
        <source>Cannot read module file (</source>
        <translation>Nie można odczytać pliku modułu (</translation>
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
w wierszu </translation>
    </message>
    <message>
        <source>Module </source>
        <translation>Moduł</translation>
    </message>
    <message>
        <source> not found</source>
        <translation>nie znaleziony</translation>
    </message>
    <message>
        <source>Cannot find man page </source>
        <translation>Nie można odnaleźć strony podręcznika</translation>
    </message>
    <message>
        <source>Not available, cannot open description (</source>
        <translation>Niedostępne, nie można otworzyć opisu (</translation>
    </message>
    <message>
        <source> column </source>
        <translation>kolumna</translation>
    </message>
    <message>
        <source>Not available, incorrect description (</source>
        <translation>Niedostępne, błędny opis (</translation>
    </message>
    <message>
        <source>Cannot get input region</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Use Input Region</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot find module </source>
        <translation>Nie można odnaleźć modułu</translation>
    </message>
    <message>
        <source>Cannot start module: </source>
        <translation>Nie można uruchomić modułu:</translation>
    </message>
    <message>
        <source>&lt;B&gt;Successfully finished&lt;/B&gt;</source>
        <translation>&lt;B&gt;Pomyślnie zakończono&lt;/B&gt;</translation>
    </message>
    <message>
        <source>&lt;B&gt;Finished with error&lt;/B&gt;</source>
        <translation>&lt;B&gt;Zakończono z błędem&lt;/B&gt;</translation>
    </message>
    <message>
        <source>&lt;B&gt;Module crashed or killed&lt;/B&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Not available, description not found (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Please ensure you have the GRASS documentation installed.</source>
        <translation>Upewnij się, że masz zainstalowaną dokumentację GRASS.</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleBase</name>
    <message>
        <source>GRASS Module</source>
        <translation>Moduł GRASS</translation>
    </message>
    <message>
        <source>Options</source>
        <translation>Opcje</translation>
    </message>
    <message>
        <source>Output</source>
        <translation>Wyjście</translation>
    </message>
    <message>
        <source>Manual</source>
        <translation>Podręcznik</translation>
    </message>
    <message>
        <source>Run</source>
        <translation>Uruchom</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Zamknij</translation>
    </message>
    <message>
        <source>View output</source>
        <translation>Zobacz wynik</translation>
    </message>
    <message>
        <source>TextLabel</source>
        <translation>Etykieta tekstowa</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleField</name>
    <message>
        <source>Attribute field</source>
        <translation>Pole atrybutu</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleFile</name>
    <message>
        <source>File</source>
        <translation>Plik</translation>
    </message>
    <message>
        <source>:&amp;nbsp;missing value</source>
        <translation>:&amp;nbsp;brakująca wartość</translation>
    </message>
    <message>
        <source>:&amp;nbsp;directory does not exist</source>
        <translation>:&amp;nbsp;katalog nie istnieje</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleGdalInput</name>
    <message>
        <source>Warning</source>
        <translation>Ostrzeżenie</translation>
    </message>
    <message>
        <source>Cannot find layeroption </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>PostGIS driver in OGR does not support schemas!&lt;br&gt;Only the table name will be used.&lt;br&gt;It can result in wrong input if more tables of the same name&lt;br&gt;are present in the database.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>:&amp;nbsp;no input</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot find whereoption </source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleInput</name>
    <message>
        <source>Warning</source>
        <translation>Ostrzeżenie</translation>
    </message>
    <message>
        <source>Cannot find typeoption </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot find values for typeoption </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot find layeroption </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>GRASS element </source>
        <translation>element GRASS</translation>
    </message>
    <message>
        <source> not supported</source>
        <translation>nieobsługiwane</translation>
    </message>
    <message>
        <source>Use region of this map</source>
        <translation>Użyj regionu tej mapy</translation>
    </message>
    <message>
        <source>:&amp;nbsp;no input</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleOption</name>
    <message>
        <source>:&amp;nbsp;missing value</source>
        <translation>:&amp;nbsp;brakująca wartość</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleSelection</name>
    <message>
        <source>Attribute field</source>
        <translation>Pole atrybutu</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleStandardOptions</name>
    <message>
        <source>Warning</source>
        <translation>Ostrzeżenie</translation>
    </message>
    <message>
        <source>Cannot find module </source>
        <translation>Nie można odnaleźć modułu</translation>
    </message>
    <message>
        <source>Cannot start module </source>
        <translation>Nie można uruchomić modułu</translation>
    </message>
    <message>
        <source>Cannot read module description (</source>
        <translation>Nie można odczytać opisu modułu (</translation>
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
w wierszu </translation>
    </message>
    <message>
        <source> column </source>
        <translation>kolumna</translation>
    </message>
    <message>
        <source>Cannot find key </source>
        <translation>Nie można odnaleźć klucza</translation>
    </message>
    <message>
        <source>Item with id </source>
        <translation>Obiekt z id</translation>
    </message>
    <message>
        <source> not found</source>
        <translation>nie znaleziony</translation>
    </message>
    <message>
        <source>Cannot get current region</source>
        <translation>Nie można ustalić bieżącego regionu</translation>
    </message>
    <message>
        <source>Cannot check region of map </source>
        <translation>Nie można sprawdzić regionu mapy</translation>
    </message>
    <message>
        <source>Cannot set region of map </source>
        <translation>Nie można ustalić regionu mapy</translation>
    </message>
</context>
<context>
    <name>QgsGrassNewMapset</name>
    <message>
        <source>Projection</source>
        <translation type="obsolete">Układ współrzędnych</translation>
    </message>
    <message>
        <source>Database</source>
        <translation>Baza danych</translation>
    </message>
    <message>
        <source>Location 2</source>
        <translation>Lokacja 2</translation>
    </message>
    <message>
        <source>User&apos;s mapset</source>
        <translation>Mapset użytkownika</translation>
    </message>
    <message>
        <source>System mapset</source>
        <translation>Mapset systemowy</translation>
    </message>
    <message>
        <source>Location 1</source>
        <translation>Lokacja 1</translation>
    </message>
    <message>
        <source>Enter path to GRASS database</source>
        <translation>Podaj ścieżkę do bazy danych GRASS</translation>
    </message>
    <message>
        <source>The directory doesn&apos;t exist!</source>
        <translation>Kartoteka nie istnieje!</translation>
    </message>
    <message>
        <source>No writable locations, the database not writable!</source>
        <translation type="unfinished">Lokacje i baza danych bez praw do zapisu!</translation>
    </message>
    <message>
        <source>Enter location name!</source>
        <translation>Wpisz nazwę lokacji!</translation>
    </message>
    <message>
        <source>The location exists!</source>
        <translation>Taka lokacja już istnieje!</translation>
    </message>
    <message>
        <source>Selected projection is not supported by GRASS!</source>
        <translation>Wybrane odwzorowanie nie jest wpierane przez GRASS!</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Ostrzeżenie</translation>
    </message>
    <message>
        <source>Cannot create projection.</source>
        <translation>Nie można utworzyć odwzorowania.</translation>
    </message>
    <message>
        <source>Cannot reproject previously set region, default region set.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>North must be greater than south</source>
        <translation>Wartość północy musi być większa niż południa</translation>
    </message>
    <message>
        <source>East must be greater than west</source>
        <translation>Wartość wschodu musi być większa niż zachodu</translation>
    </message>
    <message>
        <source>Regions file (</source>
        <translation>Plik regionów (</translation>
    </message>
    <message>
        <source>) not found.</source>
        <translation>) nie znaleziony.</translation>
    </message>
    <message>
        <source>Cannot open locations file (</source>
        <translation>Nie można otworzyć pliku lokacji (</translation>
    </message>
    <message>
        <source>)</source>
        <translation>)</translation>
    </message>
    <message>
        <source>Cannot read locations file (</source>
        <translation>Nie można odczytać pliku lokacji (</translation>
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
w wierszu </translation>
    </message>
    <message>
        <source> column </source>
        <translation>kolumna</translation>
    </message>
    <message>
        <source>Cannot reproject selected region.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot reproject region</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Enter mapset name.</source>
        <translation>Podaj nazwę mapsetu.</translation>
    </message>
    <message>
        <source>The mapset already exists</source>
        <translation>Taki mapset już istnieje</translation>
    </message>
    <message>
        <source>Database: </source>
        <translation>Baza danych:</translation>
    </message>
    <message>
        <source>Location: </source>
        <translation>Lokacja: </translation>
    </message>
    <message>
        <source>Mapset: </source>
        <translation>Mapset:</translation>
    </message>
    <message>
        <source>Create location</source>
        <translation>Twórz lokację</translation>
    </message>
    <message>
        <source>Cannot create new location: </source>
        <translation>Nie można utworzyć nowej lokacji:</translation>
    </message>
    <message>
        <source>Create mapset</source>
        <translation>Twórz mapset</translation>
    </message>
    <message>
        <source>Cannot open DEFAULT_WIND</source>
        <translation>Nie można otworzyć DEFAULT_WIND</translation>
    </message>
    <message>
        <source>Cannot open WIND</source>
        <translation>Nie można otworzyć WIND</translation>
    </message>
    <message>
        <source>New mapset</source>
        <translation>Nowy mapset</translation>
    </message>
    <message>
        <source>New mapset successfully created, but cannot be opened: </source>
        <translation>Pomyślnie utworzono nowy mapset, ale nie można otworzyć:</translation>
    </message>
    <message>
        <source>New mapset successfully created and set as current working mapset.</source>
        <translation>Pomyślnie utworzono nowy mapset i ustawiono jako bieżący roboczy.</translation>
    </message>
    <message>
        <source>Cannot create new mapset directory</source>
        <translation>Nie można utworzyć kartoteki nowego mapsetu</translation>
    </message>
    <message>
        <source>Cannot create QgsCoordinateReferenceSystem</source>
        <translation>Nie można utworzyć QgsCoordinateReferenceSystem</translation>
    </message>
</context>
<context>
    <name>QgsGrassNewMapsetBase</name>
    <message>
        <source>Column 1</source>
        <translation type="obsolete">Kolumna 1</translation>
    </message>
    <message>
        <source>Example directory tree:</source>
        <translation>Przykładowa struktura kartotek:</translation>
    </message>
    <message>
        <source>Database Error</source>
        <translation>Błąd bazy danych</translation>
    </message>
    <message>
        <source>Database:</source>
        <translation>Baza danych:</translation>
    </message>
    <message>
        <source>...</source>
        <translation type="obsolete">...</translation>
    </message>
    <message>
        <source>Select existing directory or create a new one:</source>
        <translation>Wybierz istniejący katalog lub utwórz nowy:</translation>
    </message>
    <message>
        <source>Location</source>
        <translation>Lokacja</translation>
    </message>
    <message>
        <source>Select location</source>
        <translation>Wybierz lokację</translation>
    </message>
    <message>
        <source>Create new location</source>
        <translation>Twórz nową lokację</translation>
    </message>
    <message>
        <source>Location Error</source>
        <translation>Błąd lokacji</translation>
    </message>
    <message>
        <source>Projection Error</source>
        <translation>Błąd odwzorowania</translation>
    </message>
    <message>
        <source>Coordinate system</source>
        <translation>Układ współrzędnych</translation>
    </message>
    <message>
        <source>Projection</source>
        <translation>Odwzorowanie</translation>
    </message>
    <message>
        <source>Not defined</source>
        <translation>Nie zdefiniowane</translation>
    </message>
    <message>
        <source>Set current QGIS extent</source>
        <translation>Ustal bieżący zakres QGIS</translation>
    </message>
    <message>
        <source>Set</source>
        <translation>Ustaw</translation>
    </message>
    <message>
        <source>Region Error</source>
        <translation>Błąd regionu</translation>
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
        <translation>Nowy mapset:</translation>
    </message>
    <message>
        <source>Mapset Error</source>
        <translation>Błąd mapsetu</translation>
    </message>
    <message>
        <source>&lt;p align=&quot;center&quot;&gt;Existing masets&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;center&quot;&gt;Istniejące mapsety&lt;/p&gt;</translation>
    </message>
    <message>
        <source>Location:</source>
        <translation>Lokacja: </translation>
    </message>
    <message>
        <source>Mapset:</source>
        <translation>Mapset:</translation>
    </message>
    <message>
        <source>New Mapset</source>
        <translation>Nowy mapset</translation>
    </message>
    <message>
        <source>GRASS Database</source>
        <translation>Baza danych GRASS</translation>
    </message>
    <message>
        <source>Tree</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Comment</source>
        <translation>Komentarz</translation>
    </message>
    <message>
        <source>&lt;!DOCTYPE HTML PUBLIC &quot;-//W3C//DTD HTML 4.0//EN&quot; &quot;http://www.w3.org/TR/REC-html40/strict.dtd&quot;&gt;
&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Lucida Grande&apos;; font-size:13pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;GRASS data are stored in tree directory structure. The GRASS database is the top-level directory in this tree structure.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;!DOCTYPE HTML PUBLIC &quot;-//W3C//DTD HTML 4.0//EN&quot; &quot;http://www.w3.org/TR/REC-html40/strict.dtd&quot;&gt;
&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Lucida Grande&apos;; font-size:13pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;Dane GRASS są zgromadzone w strukturze kartotek. Baza danych GRASS jest na samej górze tej struktury.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Browse...</source>
        <translation>Przeglądaj...</translation>
    </message>
    <message>
        <source>GRASS Location</source>
        <translation>Lokacja GRASS</translation>
    </message>
    <message>
        <source>&lt;!DOCTYPE HTML PUBLIC &quot;-//W3C//DTD HTML 4.0//EN&quot; &quot;http://www.w3.org/TR/REC-html40/strict.dtd&quot;&gt;
&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Lucida Grande&apos;; font-size:13pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;The GRASS location is a collection of maps for a particular territory or project.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;!DOCTYPE HTML PUBLIC &quot;-//W3C//DTD HTML 4.0//EN&quot; &quot;http://www.w3.org/TR/REC-html40/strict.dtd&quot;&gt;
&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Lucida Grande&apos;; font-size:13pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;Lokacja GRASS jest kolekcją map dla określonego terytorium lub projektu.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Default GRASS Region</source>
        <translation>Domyślny region GRASS</translation>
    </message>
    <message>
        <source>&lt;!DOCTYPE HTML PUBLIC &quot;-//W3C//DTD HTML 4.0//EN&quot; &quot;http://www.w3.org/TR/REC-html40/strict.dtd&quot;&gt;
&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Lucida Grande&apos;; font-size:13pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;The GRASS region defines a workspace for raster modules. The default region is valid for one location. It is possible to set a different region in each mapset. It is possible to change the default location region later.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;!DOCTYPE HTML PUBLIC &quot;-//W3C//DTD HTML 4.0//EN&quot; &quot;http://www.w3.org/TR/REC-html40/strict.dtd&quot;&gt;
&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Lucida Grande&apos;; font-size:13pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;Region GRASS definiuje obszar roboczy dla modułów rastrowych. Domyślny region jest obowiązujący dla jednej lokacji. Dla każdego mapsetu można zdefiniować oddzielny region. Region domyślnej lokacji można później zmienić.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Mapset</source>
        <translation>Mapset</translation>
    </message>
    <message>
        <source>Owner</source>
        <translation>Właściciel</translation>
    </message>
    <message>
        <source>&lt;!DOCTYPE HTML PUBLIC &quot;-//W3C//DTD HTML 4.0//EN&quot; &quot;http://www.w3.org/TR/REC-html40/strict.dtd&quot;&gt;
&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Lucida Grande&apos;; font-size:13pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;The GRASS mapset is a collection of maps used by one user. A user can read maps from all mapsets in the location but he can open for writing only his mapset (owned by user).&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;!DOCTYPE HTML PUBLIC &quot;-//W3C//DTD HTML 4.0//EN&quot; &quot;http://www.w3.org/TR/REC-html40/strict.dtd&quot;&gt;
&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Lucida Grande&apos;; font-size:13pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;Mapset GRASS jest kolekcją map używanych przez jedengo użytkownika. Użytkownik może czytać mapy ze wszystkich mapsetów w lokacji, ale może otwierać do zapisu wyłącznie własny mapset (którego jest właścicielem).&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Create New Mapset</source>
        <translation>Twórz nowy mapset</translation>
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
        <translation>Otwórz mapset</translation>
    </message>
    <message>
        <source>New mapset</source>
        <translation>Nowy mapset</translation>
    </message>
    <message>
        <source>Close mapset</source>
        <translation>Zamknij mapset</translation>
    </message>
    <message>
        <source>Add GRASS vector layer</source>
        <translation>Dodaj warstwę wektorową GRASS</translation>
    </message>
    <message>
        <source>Add GRASS raster layer</source>
        <translation>Dodaj warstwę rastrową GRASS</translation>
    </message>
    <message>
        <source>Open GRASS tools</source>
        <translation>Otwórz narzędzia GRASS</translation>
    </message>
    <message>
        <source>Display Current Grass Region</source>
        <translation>Wyświetl bieżący region GRASS</translation>
    </message>
    <message>
        <source>Edit Current Grass Region</source>
        <translation>Edytuj bieżący region GRASS</translation>
    </message>
    <message>
        <source>Edit Grass Vector layer</source>
        <translation>Edytuj warstwę wektorową GRASS</translation>
    </message>
    <message>
        <source>Adds a GRASS vector layer to the map canvas</source>
        <translation>Dodaj warstwę wektorową GRASSa do obszaru mapy</translation>
    </message>
    <message>
        <source>Adds a GRASS raster layer to the map canvas</source>
        <translation>Dodaj warstwę rastrową GRASSa do obszaru mapy</translation>
    </message>
    <message>
        <source>Displays the current GRASS region as a rectangle on the map canvas</source>
        <translation>Wyświetla aktulny region GRASSa jako prostokąt w obszarze mapy</translation>
    </message>
    <message>
        <source>Edit the current GRASS region</source>
        <translation>Edytuj bieżący region GRASS</translation>
    </message>
    <message>
        <source>Edit the currently selected GRASS vector layer.</source>
        <translation>Edytuj aktualnie wybraną warstwę wektorową GRASS.</translation>
    </message>
    <message>
        <source>GrassVector</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>0.1</source>
        <translation>0.1</translation>
    </message>
    <message>
        <source>GRASS layer</source>
        <translation>Warstwa GRASS</translation>
    </message>
    <message>
        <source>Create new Grass Vector</source>
        <translation>Utwórz nową warstwę wektorową GRASS</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Ostrzeżenie</translation>
    </message>
    <message>
        <source>GRASS Edit is already running.</source>
        <translation>Edytor GRASS jest już uruchomiony.</translation>
    </message>
    <message>
        <source>New vector name</source>
        <translation>Nazwa nowej warstwy wektorowej</translation>
    </message>
    <message>
        <source>Cannot create new vector: </source>
        <translation>Nie można utworzyć nowej warstwy wektorowej:</translation>
    </message>
    <message>
        <source>New vector created but cannot be opened by data provider.</source>
        <translation>Nowa warstwa wektorowa została utworzona, ale nie może być otwarta przez źródło danych.</translation>
    </message>
    <message>
        <source>Cannot start editing.</source>
        <translation>Nie można rozpocząć edycji.</translation>
    </message>
    <message>
        <source>GISDBASE, LOCATION_NAME or MAPSET is not set, cannot display current region.</source>
        <translation>GISDBASE, LOCATION_NAME lub MAPSET nie są ustalone, nie można wyświetlić bieżącego regionu.</translation>
    </message>
    <message>
        <source>Cannot read current region: </source>
        <translation>Nie można odczytać bieżącego regionu:</translation>
    </message>
    <message>
        <source>Cannot open the mapset. </source>
        <translation>Nie można otworzyć mapsetu.</translation>
    </message>
    <message>
        <source>Cannot close mapset. </source>
        <translation>Nie można zamknąć mapsetu.</translation>
    </message>
    <message>
        <source>Cannot close current mapset. </source>
        <translation>Nie można zamknąć bieżącego mapsetu.</translation>
    </message>
    <message>
        <source>Cannot open GRASS mapset. </source>
        <translation>Nie można otworzyć mapsetu GRASS.</translation>
    </message>
</context>
<context>
    <name>QgsGrassRegion</name>
    <message>
        <source>Warning</source>
        <translation>Ostrzeżenie</translation>
    </message>
    <message>
        <source>GISDBASE, LOCATION_NAME or MAPSET is not set, cannot display current region.</source>
        <translation>GISDBASE, LOCATION_NAME lub MAPSET nie są ustalone, nie można wyświetlić bieżącego regionu.</translation>
    </message>
    <message>
        <source>Cannot read current region: </source>
        <translation>Nie można odczytać bieżącego regionu:</translation>
    </message>
    <message>
        <source>Cannot write region</source>
        <translation>Nie można zapisać regionu</translation>
    </message>
</context>
<context>
    <name>QgsGrassRegionBase</name>
    <message>
        <source>GRASS Region Settings</source>
        <translation>Ustawienia regionu GRASS</translation>
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
        <translation>S</translation>
    </message>
    <message>
        <source>N-S Res</source>
        <translation>rozdz. N-S</translation>
    </message>
    <message>
        <source>Rows</source>
        <translation>Rekordy</translation>
    </message>
    <message>
        <source>Cols</source>
        <translation>Kolumny</translation>
    </message>
    <message>
        <source>E-W Res</source>
        <translation>rozdz. E-W</translation>
    </message>
    <message>
        <source>Color</source>
        <translation>Kolor</translation>
    </message>
    <message>
        <source>Width</source>
        <translation>Szerokość</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Anuluj</translation>
    </message>
</context>
<context>
    <name>QgsGrassSelect</name>
    <message>
        <source>Select GRASS Vector Layer</source>
        <translation>Wybierz warstwę wektorową GRASS</translation>
    </message>
    <message>
        <source>Select GRASS Raster Layer</source>
        <translation>Wybierz warstwę rastrową GRASS</translation>
    </message>
    <message>
        <source>Select GRASS mapcalc schema</source>
        <translation>Wybierz schemat GRASS mapcalc</translation>
    </message>
    <message>
        <source>Select GRASS Mapset</source>
        <translation>Wybierz mapset GRASS</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Ostrzeżenie</translation>
    </message>
    <message>
        <source>Cannot open vector on level 2 (topology not available).</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Choose existing GISDBASE</source>
        <translation>Wybierz istniejący GISDBASE</translation>
    </message>
    <message>
        <source>Wrong GISDBASE, no locations available.</source>
        <translation>Błędny GISDBASE, brak dostępnych lokacji.</translation>
    </message>
    <message>
        <source>Wrong GISDBASE</source>
        <translation>Błędny GISDBASE</translation>
    </message>
    <message>
        <source>Select a map.</source>
        <translation>Wybierz mapę.</translation>
    </message>
    <message>
        <source>No map</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>No layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>No layers available in this map</source>
        <translation>Brak warstw dostępnych dla tej mapy</translation>
    </message>
</context>
<context>
    <name>QgsGrassSelectBase</name>
    <message>
        <source>Add GRASS Layer</source>
        <translation>Dodaj warstwę GRASS</translation>
    </message>
    <message>
        <source>Gisdbase</source>
        <translation>Gisdbase</translation>
    </message>
    <message>
        <source>Location</source>
        <translation>Lokacja</translation>
    </message>
    <message>
        <source>Mapset</source>
        <translation>Mapset</translation>
    </message>
    <message>
        <source>Select or type map name (wildcards &apos;*&apos; and &apos;?&apos; accepted for rasters)</source>
        <translation>Wybierz lub wpisz nazwę mapy (znaki wieloznaczne &apos;*&apos; i &apos;?&apos; akceptowane są dla rastrów)</translation>
    </message>
    <message>
        <source>Map name</source>
        <translation>Nazwa mapy</translation>
    </message>
    <message>
        <source>Layer</source>
        <translation>Warstwa</translation>
    </message>
    <message>
        <source>Browse</source>
        <translation>Przeglądaj</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Anuluj</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>OK</translation>
    </message>
</context>
<context>
    <name>QgsGrassShellBase</name>
    <message>
        <source>GRASS Shell</source>
        <translation>GRASS Shell</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Zamknij</translation>
    </message>
</context>
<context>
    <name>QgsGrassTools</name>
    <message>
        <source>Browser</source>
        <translation>Przeglądarka</translation>
    </message>
    <message>
        <source>GRASS Tools</source>
        <translation>Narzędzia GRASS</translation>
    </message>
    <message>
        <source>GRASS Tools: </source>
        <translation>Narzędzia GRASS:</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Ostrzeżenie</translation>
    </message>
    <message>
        <source>Cannot find MSYS (</source>
        <translation>Nie można odnaleźć MSYS (</translation>
    </message>
    <message>
        <source>GRASS Shell is not compiled.</source>
        <translation>GRASS Shell nie jest skompilowany.</translation>
    </message>
    <message>
        <source>The config file (</source>
        <translation>Plik konfiguracyjny (</translation>
    </message>
    <message>
        <source>) not found.</source>
        <translation>) nie znaleziony.</translation>
    </message>
    <message>
        <source>Cannot open config file (</source>
        <translation>Nie można otworzyć pliku konfiguracyjnego (</translation>
    </message>
    <message>
        <source>)</source>
        <translation>)</translation>
    </message>
    <message>
        <source>Cannot read config file (</source>
        <translation>Nie można odczytać pliku konfiguracyjnego (</translation>
    </message>
    <message>
        <source>
at line </source>
        <translation>
w wierszu </translation>
    </message>
    <message>
        <source> column </source>
        <translation>kolumna</translation>
    </message>
</context>
<context>
    <name>QgsGrassToolsBase</name>
    <message>
        <source>Grass Tools</source>
        <translation>Narzędzia GRASS</translation>
    </message>
    <message>
        <source>Modules Tree</source>
        <translation>Struktura modułów</translation>
    </message>
    <message>
        <source>1</source>
        <translation>1</translation>
    </message>
    <message>
        <source>Modules List</source>
        <translation>Lista modułów</translation>
    </message>
</context>
<context>
    <name>QgsGridMakerPlugin</name>
    <message>
        <source>&amp;Graticule Creator</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Creates a graticule (grid) and stores the result as a shapefile</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Graticules</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGridMakerPluginGui</name>
    <message>
        <source>Choose a filename to save under</source>
        <translation type="obsolete">Wybierz nazwę pliku pod jaką mam zapisać</translation>
    </message>
    <message>
        <source>ESRI Shapefile (*.shp)</source>
        <translation>Pliki ESRI shape (*.shp)</translation>
    </message>
    <message>
        <source>QGIS - Grid Maker</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Please enter the file name before pressing OK!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Please enter intervals before pressing OK!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Choose a file name to save under</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGridMakerPluginGuiBase</name>
    <message>
        <source>Graticule Builder</source>
        <translation>Generator siatki kartograficznej</translation>
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
        <translation>Poligon</translation>
    </message>
    <message>
        <source>Origin (lower left)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>End point (upper right)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Output (shape) file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Save As...</source>
        <translation>Zapisz jako...</translation>
    </message>
    <message>
        <source>QGIS Graticule Creator</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Graticle size</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Y Interval:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>X Interval:</source>
        <translation type="unfinished"></translation>
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
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsHelpViewer</name>
    <message>
        <source>Quantum GIS Help - </source>
        <translation>Pomoc Quantum GIS - </translation>
    </message>
    <message>
        <source>Failed to get the help text from the database</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Error</source>
        <translation>Błąd</translation>
    </message>
    <message>
        <source>The QGIS help database is not installed</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>This help file does not exist for your language</source>
        <translation>Plik pomocy dla Twojego języka nie istnieje</translation>
    </message>
    <message>
        <source>If you would like to create it, contact the QGIS development team</source>
        <translation>Jeśli chciałbyś go stworzyć skontaktuj się z zespołem twórców QGIS</translation>
    </message>
    <message>
        <source>Quantum GIS Help</source>
        <translation>Pomoc Quantum GIS</translation>
    </message>
</context>
<context>
    <name>QgsHelpViewerBase</name>
    <message>
        <source>QGIS Help</source>
        <translation>Pomoc QGIS</translation>
    </message>
    <message>
        <source>&amp;Home</source>
        <translation>&amp;Początek</translation>
    </message>
    <message>
        <source>Alt+H</source>
        <translation>Alt+H</translation>
    </message>
    <message>
        <source>&amp;Forward</source>
        <translation>&amp;Naprzód</translation>
    </message>
    <message>
        <source>Alt+F</source>
        <translation>Alt+F</translation>
    </message>
    <message>
        <source>&amp;Back</source>
        <translation>&amp;Wstecz</translation>
    </message>
    <message>
        <source>Alt+B</source>
        <translation>Alt+B</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>&amp;Zamknij</translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>HTTP response completed, however there was an error: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>HTTP transaction completed, however there was an error: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message numerus="yes">
        <source>Network timed out after %1 seconds of inactivity.
        This may be a problem in your network connection or at the WMS server.</source>
        <translation type="unfinished">
            <numerusform></numerusform>
            <numerusform></numerusform>
            <numerusform></numerusform>
        </translation>
    </message>
</context>
<context>
    <name>QgsIDWInterpolatorDialogBase</name>
    <message>
        <source>Dialog</source>
        <translation type="unfinished"></translation>
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
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsIdentifyResults</name>
    <message>
        <source>Identify Results - </source>
        <translation>Wynik wskazania - </translation>
    </message>
    <message>
        <source>Feature</source>
        <translation>Obiekt</translation>
    </message>
    <message>
        <source>Value</source>
        <translation>Wartość</translation>
    </message>
    <message>
        <source>Run action</source>
        <translation>Uruchom akcję</translation>
    </message>
    <message>
        <source>(Derived)</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsIdentifyResultsBase</name>
    <message>
        <source>Identify Results</source>
        <translation>Wynik identyfikacji</translation>
    </message>
    <message>
        <source>Help</source>
        <translation>Pomoc</translation>
    </message>
    <message>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Zamknij</translation>
    </message>
</context>
<context>
    <name>QgsInterpolationDialog</name>
    <message>
        <source>Triangular interpolation (TIN)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Inverse Distance Weighting (IDW)</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsInterpolationDialogBase</name>
    <message>
        <source>Interpolation plugin</source>
        <translation>Wtyczka interpolacji</translation>
    </message>
    <message>
        <source>Input</source>
        <translation>Wejście</translation>
    </message>
    <message>
        <source>Input vector layer</source>
        <translation>Wejściowa warstwa wektorowa</translation>
    </message>
    <message>
        <source>Use z-Coordinate for interpolation</source>
        <translation>Wykorzystaj współrzędną Z do interpolacji</translation>
    </message>
    <message>
        <source>Interpolation attribute </source>
        <translation>Atrybuty interpolacji</translation>
    </message>
    <message>
        <source>Output</source>
        <translation>Wyjście</translation>
    </message>
    <message>
        <source>Interpolation method</source>
        <translation>Metoda interpolacji</translation>
    </message>
    <message>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <source>Number of columns</source>
        <translation>Liczba kolumn</translation>
    </message>
    <message>
        <source>Number of rows</source>
        <translation>Liczba wierszy</translation>
    </message>
    <message>
        <source>Output file </source>
        <translation>Plik wyjściowy</translation>
    </message>
</context>
<context>
    <name>QgsInterpolationPlugin</name>
    <message>
        <source>&amp;Interpolation</source>
        <translation>&amp;Interpolacja</translation>
    </message>
</context>
<context>
    <name>QgsLUDialogBase</name>
    <message>
        <source>Enter class bounds</source>
        <translation>Podaj przedział klasy</translation>
    </message>
    <message>
        <source>Lower value</source>
        <translation>Niższa wartość</translation>
    </message>
    <message>
        <source>-</source>
        <translation>-</translation>
    </message>
    <message>
        <source>Upper value</source>
        <translation>Wyższa wartość</translation>
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
    <message>
        <source>Form1</source>
        <translation>Form1</translation>
    </message>
    <message>
        <source>Font</source>
        <translation>Czcionka</translation>
    </message>
    <message>
        <source>Points</source>
        <translation>Punkty</translation>
    </message>
    <message>
        <source>Map units</source>
        <translation>Jednostki mapy</translation>
    </message>
    <message>
        <source>%</source>
        <translation>%</translation>
    </message>
    <message>
        <source>Transparency:</source>
        <translation>Przezroczystość:</translation>
    </message>
    <message>
        <source>Above</source>
        <translation>Powyżej</translation>
    </message>
    <message>
        <source>Over</source>
        <translation>Na</translation>
    </message>
    <message>
        <source>Left</source>
        <translation>Z lewej</translation>
    </message>
    <message>
        <source>Below</source>
        <translation>Poniżej</translation>
    </message>
    <message>
        <source>Right</source>
        <translation>Z prawej</translation>
    </message>
    <message>
        <source>Above Right</source>
        <translation>Powyżej z prawej</translation>
    </message>
    <message>
        <source>Below Right</source>
        <translation>Poniżej z prawej</translation>
    </message>
    <message>
        <source>Above Left</source>
        <translation>Powyżej z lewej</translation>
    </message>
    <message>
        <source>Below Left</source>
        <translation>Poniżej z lewej</translation>
    </message>
    <message>
        <source>Size:</source>
        <translation>Rozmiar:</translation>
    </message>
    <message>
        <source>Size is in map units</source>
        <translation>Rozmiar jest w jednostkach mapy</translation>
    </message>
    <message>
        <source>Size is in points</source>
        <translation>Rozmiar jest w punktach</translation>
    </message>
    <message>
        <source>Position</source>
        <translation>Pozycja</translation>
    </message>
    <message>
        <source>Preview:</source>
        <translation>Podgląd:</translation>
    </message>
    <message>
        <source>QGIS Rocks!</source>
        <translation>QGIS rządzi!</translation>
    </message>
    <message>
        <source>Font size units</source>
        <translation>Jednostki rozmiaru czcionek</translation>
    </message>
    <message>
        <source>Placement</source>
        <translation>Umiejscowienie</translation>
    </message>
    <message>
        <source>Buffer</source>
        <translation>Bufor</translation>
    </message>
    <message>
        <source>Buffer size units</source>
        <translation>Jednostki rozmiaru bufora</translation>
    </message>
    <message>
        <source>Offset units</source>
        <translation>Jednostki przesunięcia</translation>
    </message>
    <message>
        <source>Field containing label</source>
        <translation>Pole zawierające etykiety</translation>
    </message>
    <message>
        <source>Default label</source>
        <translation>Domyślna etykieta</translation>
    </message>
    <message>
        <source>Data defined style</source>
        <translation>Styl oparty na danych</translation>
    </message>
    <message>
        <source>Data defined alignment</source>
        <translation>Wyrównanie oparte na danych</translation>
    </message>
    <message>
        <source>Data defined buffer</source>
        <translation>Bufor oparty na danych</translation>
    </message>
    <message>
        <source>Data defined position</source>
        <translation>Lokalizacja oparta na danych</translation>
    </message>
    <message>
        <source>Font transparency</source>
        <translation>Przeźroczystość czcionki</translation>
    </message>
    <message>
        <source>Color</source>
        <translation>Kolor</translation>
    </message>
    <message>
        <source>Angle (deg)</source>
        <translation>Kąt</translation>
    </message>
    <message>
        <source>Buffer labels?</source>
        <translation>Bufor etykiet?</translation>
    </message>
    <message>
        <source>Buffer size</source>
        <translation>Rozmiar bufora</translation>
    </message>
    <message>
        <source>Transparency</source>
        <translation>Przeźroczystość</translation>
    </message>
    <message>
        <source>X Offset (pts)</source>
        <translation>Przesunięcie X (punkty)</translation>
    </message>
    <message>
        <source>Y Offset (pts)</source>
        <translation>Przesunięcie Y (punkty)</translation>
    </message>
    <message>
        <source>&amp;Font family</source>
        <translation>Rodzaj czcionki</translation>
    </message>
    <message>
        <source>&amp;Bold</source>
        <translation>Pogrubione</translation>
    </message>
    <message>
        <source>&amp;Italic</source>
        <translation>Pochylone</translation>
    </message>
    <message>
        <source>&amp;Underline</source>
        <translation>Podkreślone</translation>
    </message>
    <message>
        <source>&amp;Size</source>
        <translation>Rozmiar</translation>
    </message>
    <message>
        <source>Size units</source>
        <translation>Jednostki rozmiaru</translation>
    </message>
    <message>
        <source>X Coordinate</source>
        <translation>Współrzędna X</translation>
    </message>
    <message>
        <source>Y Coordinate</source>
        <translation>Współrzędna Y</translation>
    </message>
    <message encoding="UTF-8">
        <source>°</source>
        <translation>°</translation>
    </message>
    <message>
        <source>Multiline labels?</source>
        <translation>Etykiety wielowierszowe?</translation>
    </message>
    <message>
        <source>General</source>
        <translation>Ogólne</translation>
    </message>
    <message>
        <source>Use scale dependent rendering</source>
        <translation>Użyj rysowania zależnego od skali</translation>
    </message>
    <message>
        <source>Maximum</source>
        <translation>Maksimum</translation>
    </message>
    <message>
        <source>Minimum</source>
        <translation>Minimum</translation>
    </message>
    <message>
        <source>Minimum scale at which this layer will be displayed. </source>
        <translation>Minimalna skala, dla której wartstwa będzie wyświetlana.</translation>
    </message>
    <message>
        <source>Maximum scale at which this layer will be displayed. </source>
        <translation>Maksymalna skala, dla której wartstwa będzie wyświetlana.</translation>
    </message>
</context>
<context>
    <name>QgsLegend</name>
    <message>
        <source>group</source>
        <translation>grupa</translation>
    </message>
    <message>
        <source>&amp;Remove</source>
        <translation>&amp;Usuń</translation>
    </message>
    <message>
        <source>&amp;Make to toplevel item</source>
        <translation>Przenieś na główny poziom </translation>
    </message>
    <message>
        <source>Re&amp;name</source>
        <translation>Zmień &amp;nazwę</translation>
    </message>
    <message>
        <source>&amp;Add group</source>
        <translation>Dod&amp;aj grupę</translation>
    </message>
    <message>
        <source>&amp;Expand all</source>
        <translation>Rozwiń wszystki&amp;e</translation>
    </message>
    <message>
        <source>&amp;Collapse all</source>
        <translation>&amp;Zwiń wszystkie</translation>
    </message>
    <message>
        <source>Show file groups</source>
        <translation>Pokaż grupy plików</translation>
    </message>
    <message>
        <source>No Layer Selected</source>
        <translation>Nie wybrano warstwy</translation>
    </message>
    <message>
        <source>To open an attribute table, you must select a vector layer in the legend</source>
        <translation>Aby otworzyć tabelę atrybutów musisz wybrać warstwę wektorową z legendy</translation>
    </message>
</context>
<context>
    <name>QgsLegendLayer</name>
    <message>
        <source>&amp;Zoom to layer extent</source>
        <translation>Powięks&amp;z do zasięgu warstwy</translation>
    </message>
    <message>
        <source>&amp;Zoom to best scale (100%)</source>
        <translation>&amp;Powiększ do najlepszej skali (100%)</translation>
    </message>
    <message>
        <source>&amp;Show in overview</source>
        <translation>Pokaż w podglądzie</translation>
    </message>
    <message>
        <source>&amp;Remove</source>
        <translation>&amp;Usuń</translation>
    </message>
    <message>
        <source>&amp;Open attribute table</source>
        <translation>&amp;Otwórz tabelę atrybutów</translation>
    </message>
    <message>
        <source>Save as shapefile...</source>
        <translation>Zapisz jako shape...</translation>
    </message>
    <message>
        <source>Save selection as shapefile...</source>
        <translation>Zapisz wybrane jako shape...</translation>
    </message>
    <message>
        <source>&amp;Properties</source>
        <translation>&amp;Właściwości</translation>
    </message>
    <message>
        <source>More layers</source>
        <translation type="obsolete">Więcej warstw</translation>
    </message>
    <message>
        <source>Multiple layers</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>This item contains multiple layers. Displaying multiple layers in the table is not supported.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsLegendLayerFile</name>
    <message>
        <source>Attribute table - </source>
        <translation type="obsolete">Tabela atrybutów - </translation>
    </message>
    <message>
        <source>Save layer as...</source>
        <translation>Zapisz warstwę jako...</translation>
    </message>
    <message>
        <source>Stop editing</source>
        <translation type="obsolete">Zakończ edycję</translation>
    </message>
    <message>
        <source>Do you want to save the changes?</source>
        <translation type="obsolete">Czy chcesz zapisać zmiany?</translation>
    </message>
    <message>
        <source>Error</source>
        <translation>Błąd</translation>
    </message>
    <message>
        <source>Saving done</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Export to Shapefile has been completed</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Driver not found</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>ESRI Shapefile driver is not available</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Error creating shapefile</source>
        <translation>Błąd tworzenia pliku shape</translation>
    </message>
    <message>
        <source>The shapefile could not be created (</source>
        <translation>Plik shape nie może być utworzony (</translation>
    </message>
    <message>
        <source>Layer creation failed</source>
        <translation>Błąd tworzenia warstwy</translation>
    </message>
    <message>
        <source>&amp;Zoom to layer extent</source>
        <translation>Powięks&amp;z do zasięgu warstwy</translation>
    </message>
    <message>
        <source>&amp;Show in overview</source>
        <translation>Pokaż w podglądzie</translation>
    </message>
    <message>
        <source>&amp;Remove</source>
        <translation>&amp;Usuń</translation>
    </message>
    <message>
        <source>&amp;Open attribute table</source>
        <translation>&amp;Otwórz tabelę atrybutów</translation>
    </message>
    <message>
        <source>Save as shapefile...</source>
        <translation>Zapisz jako shape...</translation>
    </message>
    <message>
        <source>Save selection as shapefile...</source>
        <translation>Zapisz wybrane jako shape...</translation>
    </message>
    <message>
        <source>&amp;Properties</source>
        <translation>&amp;Właściwości</translation>
    </message>
    <message>
        <source>Layer attribute table contains unsupported datatype(s)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Select the coordinate reference system for the saved shapefile.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The data points will be transformed from the layer coordinate reference system.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsMapCanvas</name>
    <message>
        <source>Could not draw</source>
        <translation>Nie mogę narysować</translation>
    </message>
    <message>
        <source>because</source>
        <translation> ponieważ</translation>
    </message>
</context>
<context>
    <name>QgsMapLayer</name>
    <message>
        <source>%1 at line %2 column %3</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>User database could not be opened.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The style table could not be created.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The style %1 was saved to database</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The style %1 was updated in the database.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The style %1 could not be updated in the database.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The style %1 could not be inserted into database.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>style not found in database</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsMapToolIdentify</name>
    <message>
        <source>No features found</source>
        <translation type="obsolete">Nie znaleziono obiektów</translation>
    </message>
    <message>
        <source>(clicked coordinate)</source>
        <translation>(wskazane współrzędne)</translation>
    </message>
    <message>
        <source>WMS identify result for %1
%2</source>
        <translation type="unfinished"></translation>
    </message>
    <message numerus="yes">
        <source>- %1 features found</source>
        <comment>Identify results window title</comment>
        <translation>
            <numerusform>- %1 znaleziony obiekt</numerusform>
            <numerusform>- %1 znalezione obiekty</numerusform>
            <numerusform>- %1 znalezionych obiektów</numerusform>
        </translation>
    </message>
</context>
<context>
    <name>QgsMapToolSplitFeatures</name>
    <message>
        <source>Split error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>An error occured during feature splitting</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>No feature split done</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>If there are selected features, the split tool only applies to the selected ones. If you like to split all features under the split line, clear the selection</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsMapToolVertexEdit</name>
    <message>
        <source>Snap tolerance</source>
        <translation>Tolerancja przyciągania</translation>
    </message>
    <message>
        <source>Don&apos;t show this message again</source>
        <translation>Nie pokazuj tego komunikatu ponownie</translation>
    </message>
    <message>
        <source>Could not snap segment.</source>
        <translation>Nie mogę przyciągnąć segmentu.</translation>
    </message>
    <message>
        <source>Have you set the tolerance in Settings &gt; Project Properties &gt; General?</source>
        <translation>Czy ustawiono tolerancje w Ustawienia &gt; Właściwości projektu &gt; Ogólne?</translation>
    </message>
</context>
<context>
    <name>QgsMapserverExport</name>
    <message>
        <source>Name for the map file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Choose the QGIS project file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>QGIS Project Files (*.qgs);;All files (*.*)</source>
        <comment>Filter list for selecting files from a dialog box</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Overwrite File?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> exists. 
Do you want to overwrite it?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>MapServer map files (*.map);;All files (*.*)</source>
        <comment>Filter list for selecting files from a dialog box</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> exists. 
Do you want to overwrite it?</source>
        <comment>a fileName is prepended to this text, and appears in a dialog box</comment>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsMapserverExportBase</name>
    <message>
        <source>Export to Mapserver</source>
        <translation>Eksport do formatu Mapserver</translation>
    </message>
    <message>
        <source>Map file</source>
        <translation>Plik mapy</translation>
    </message>
    <message>
        <source>Export LAYER information only</source>
        <translation>Eksportuj tylko informację o warstwie</translation>
    </message>
    <message>
        <source>Map</source>
        <translation>Mapa</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Nazwa</translation>
    </message>
    <message>
        <source>Height</source>
        <translation>Wysokość</translation>
    </message>
    <message>
        <source>Width</source>
        <translation>Szerokość</translation>
    </message>
    <message>
        <source>dd</source>
        <translation>stopnie</translation>
    </message>
    <message>
        <source>feet</source>
        <translation>stopy</translation>
    </message>
    <message>
        <source>meters</source>
        <translation>metry</translation>
    </message>
    <message>
        <source>miles</source>
        <translation>mile</translation>
    </message>
    <message>
        <source>inches</source>
        <translation>cale</translation>
    </message>
    <message>
        <source>kilometers</source>
        <translation>kilometry</translation>
    </message>
    <message>
        <source>Units</source>
        <translation>Jednostki</translation>
    </message>
    <message>
        <source>Image type</source>
        <translation>Obraz</translation>
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
        <translation>użytkownika</translation>
    </message>
    <message>
        <source>wbmp</source>
        <translation>wbmp</translation>
    </message>
    <message>
        <source>MinScale</source>
        <translation>Skala min</translation>
    </message>
    <message>
        <source>MaxScale</source>
        <translation>Skala maks</translation>
    </message>
    <message>
        <source>Prefix attached to map, scalebar and legend GIF filenames created using this MapFile. It should be kept short.</source>
        <translation>Prefiks dodany do nazw plików mapy, podziałki i legendy tworzonych podczas eksportu. Powinien być krótki.</translation>
    </message>
    <message>
        <source>Web Interface Definition</source>
        <translation>Web Interface Definition</translation>
    </message>
    <message>
        <source>Header</source>
        <translation>Nagłówek</translation>
    </message>
    <message>
        <source>Footer</source>
        <translation>Stopka</translation>
    </message>
    <message>
        <source>Template</source>
        <translation>Szablon</translation>
    </message>
    <message>
        <source>&amp;Help</source>
        <translation>&amp;Pomoc</translation>
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
        <translation>&amp;Anuluj</translation>
    </message>
    <message>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <source>Name for the map file to be created from the QGIS project file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>If checked, only the layer information will be processed</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Path to the MapServer template file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Prefix attached to map, scalebar and legend GIF filenames created using this MapFile</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Full path to the QGIS project file to export to MapServer map format</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>QGIS project file</source>
        <translation>Plik projektu QGIS</translation>
    </message>
    <message>
        <source>Browse...</source>
        <translation>Przeglądaj...</translation>
    </message>
    <message>
        <source>Save As...</source>
        <translation>Zapisz jako...</translation>
    </message>
</context>
<context>
    <name>QgsMeasureBase</name>
    <message>
        <source>Measure</source>
        <translation>Miara</translation>
    </message>
    <message>
        <source>Help</source>
        <translation>Pomoc</translation>
    </message>
    <message>
        <source>New</source>
        <translation>Nowy</translation>
    </message>
    <message>
        <source>Cl&amp;ose</source>
        <translation>&amp;Zamknij</translation>
    </message>
    <message>
        <source>Total:</source>
        <translation>Ogółem:</translation>
    </message>
    <message>
        <source>Segments</source>
        <translation type="unfinished">Segmenty</translation>
    </message>
</context>
<context>
    <name>QgsMeasureDialog</name>
    <message>
        <source>Segments (in meters)</source>
        <translation>Segmenty (w metrach)</translation>
    </message>
    <message>
        <source>Segments (in feet)</source>
        <translation>Segmenty (w stopach)</translation>
    </message>
    <message>
        <source>Segments (in degrees)</source>
        <translation>Segmenty (w stopniach)</translation>
    </message>
    <message>
        <source>Segments</source>
        <translation>Segmenty</translation>
    </message>
</context>
<context>
    <name>QgsMeasureTool</name>
    <message>
        <source>Incorrect measure results</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;p&gt;This map is defined with a geographic coordinate system (latitude/longitude) but the map extents suggests that it is actually a projected coordinate system (e.g., Mercator). If so, the results from line or area measurements will be incorrect.&lt;/p&gt;&lt;p&gt;To fix this, explicitly set an appropriate map coordinate system using the &lt;tt&gt;Settings:Project Properties&lt;/tt&gt; menu.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsMessageViewer</name>
    <message>
        <source>QGIS Message</source>
        <translation>Wiadomość QGIS</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Zamknij</translation>
    </message>
    <message>
        <source>Don&apos;t show this message again</source>
        <translation>Nie pokazuj tego komunikatu ponownie</translation>
    </message>
</context>
<context>
    <name>QgsNewConnection</name>
    <message>
        <source>Test connection</source>
        <translation>Test połączenia</translation>
    </message>
    <message>
        <source>Connection failed - Check settings and try again.

Extended error information:
</source>
        <translation>Połączenie nie powiodło się. Sprawdź ustawienia i spróbuj ponownie.

Rozszerzona informacja o błędzie:
</translation>
    </message>
    <message>
        <source>Connection to %1 was successful</source>
        <translation>Połączenie do %1 zakończone pomyślnie</translation>
    </message>
</context>
<context>
    <name>QgsNewConnectionBase</name>
    <message>
        <source>Create a New PostGIS connection</source>
        <translation>Utwórz nowe połączenie dla PostGIS</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Anuluj</translation>
    </message>
    <message>
        <source>Help</source>
        <translation>Pomoc</translation>
    </message>
    <message>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <source>Connection Information</source>
        <translation>Informacja o połączeniu</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Nazwa</translation>
    </message>
    <message>
        <source>Host</source>
        <translation>Host</translation>
    </message>
    <message>
        <source>Database</source>
        <translation>Baza danych</translation>
    </message>
    <message>
        <source>Port</source>
        <translation>Port</translation>
    </message>
    <message>
        <source>Username</source>
        <translation>Użytkownik</translation>
    </message>
    <message>
        <source>Password</source>
        <translation>Hasło</translation>
    </message>
    <message>
        <source>Name of the new connection</source>
        <translation>Nazwa dla nowego połączenia</translation>
    </message>
    <message>
        <source>5432</source>
        <translation>5432</translation>
    </message>
    <message>
        <source>Save Password</source>
        <translation>Zapisz hasło</translation>
    </message>
    <message>
        <source>Test Connect</source>
        <translation>Test połączenia</translation>
    </message>
    <message>
        <source>Only look in the &apos;public&apos; schema</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Only look in the geometry_columns table</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Restrict the search to the public schema for spatial tables not in the geometry_columns table</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>When searching for spatial tables that are not in the geometry_columns tables, restrict the search to tables that are in the public schema (for some databases this can save lots of time)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Restrict the displayed tables to those that are in the geometry_columns table</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Restricts the displayed tables to those that are in the geometry_columns table. This can speed up the initial display of spatial tables.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsNewHttpConnectionBase</name>
    <message>
        <source>Name</source>
        <translation>Nazwa</translation>
    </message>
    <message>
        <source>URL</source>
        <translation>URL</translation>
    </message>
    <message>
        <source>Name of the new connection</source>
        <translation>Nazwa dla nowego połączenia</translation>
    </message>
    <message>
        <source>HTTP address of the Web Map Server</source>
        <translation>Adres HTTP serwera WMS (Web Map Server)</translation>
    </message>
    <message>
        <source>Create a new WMS connection</source>
        <translation>Utwórz nowe połączenie WMS</translation>
    </message>
    <message>
        <source>Connection details</source>
        <translation>Szczegóły połączenia</translation>
    </message>
</context>
<context>
    <name>QgsNorthArrowPlugin</name>
    <message>
        <source>Bottom Left</source>
        <translation>Lewy dolny</translation>
    </message>
    <message>
        <source>Top Right</source>
        <translation>Prawy górny</translation>
    </message>
    <message>
        <source>Bottom Right</source>
        <translation>Prawy dolny</translation>
    </message>
    <message>
        <source>Top Left</source>
        <translation>Lewy górny</translation>
    </message>
    <message>
        <source>&amp;North Arrow</source>
        <translation>Strzałka pół&amp;nocy</translation>
    </message>
    <message>
        <source>Creates a north arrow that is displayed on the map canvas</source>
        <translation>Tworzy strzałkę północy, która jest wyświetlana w obszarze mapy</translation>
    </message>
    <message>
        <source>&amp;Decorations</source>
        <translation>&amp;Dekoracje</translation>
    </message>
    <message>
        <source>North arrow pixmap not found</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsNorthArrowPluginGui</name>
    <message>
        <source>Pixmap not found</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsNorthArrowPluginGuiBase</name>
    <message>
        <source>North Arrow Plugin</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Properties</source>
        <translation>Właściwości</translation>
    </message>
    <message>
        <source>Angle</source>
        <translation>Kąt</translation>
    </message>
    <message>
        <source>Placement</source>
        <translation>Umiejscowienie</translation>
    </message>
    <message>
        <source>Set direction automatically</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Enable North Arrow</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Top Left</source>
        <translation>Lewy górny</translation>
    </message>
    <message>
        <source>Top Right</source>
        <translation>Prawy górny</translation>
    </message>
    <message>
        <source>Bottom Left</source>
        <translation>Lewy dolny</translation>
    </message>
    <message>
        <source>Bottom Right</source>
        <translation>Prawy dolny</translation>
    </message>
    <message>
        <source>Placement on screen</source>
        <translation>Umiejscowienie na ekranie</translation>
    </message>
    <message>
        <source>Preview of north arrow</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Icon</source>
        <translation>Ikona</translation>
    </message>
    <message>
        <source>Browse...</source>
        <translation>Przeglądaj...</translation>
    </message>
</context>
<context>
    <name>QgsOptions</name>
    <message>
        <source>Detected active locale on your system: </source>
        <translation>Wykryte ustawienia lokalne w twoim systemie:</translation>
    </message>
    <message>
        <source>to vertex</source>
        <translation>do wierzchołka</translation>
    </message>
    <message>
        <source>to segment</source>
        <translation>do segmentu</translation>
    </message>
    <message>
        <source>to vertex and segment</source>
        <translation>do wierzchołka i segmentu</translation>
    </message>
    <message>
        <source>Semi transparent circle</source>
        <translation>Półprzeźroczyste koło</translation>
    </message>
    <message>
        <source>Cross</source>
        <translation>Krzyż</translation>
    </message>
    <message>
        <source>Show all features</source>
        <translation>Pokaż wszystkie obiekty</translation>
    </message>
    <message>
        <source>Show selected features</source>
        <translation>Pokaż wybrane obiekty</translation>
    </message>
    <message>
        <source>Show features in current canvas</source>
        <translation>Pokaż obiekty w bieżącym oknie</translation>
    </message>
</context>
<context>
    <name>QgsOptionsBase</name>
    <message>
        <source>QGIS Options</source>
        <translation>Ustawienia QGIS</translation>
    </message>
    <message>
        <source>Hide splash screen at startup</source>
        <translation>Nie pokazuj ekranu powitalnego przy starcie</translation>
    </message>
    <message>
        <source>&lt;b&gt;Note: &lt;/b&gt;Theme changes take effect the next time QGIS is started</source>
        <translation>&lt;b&gt;Uwaga: &lt;/b&gt;Zmiana motywu zostanie zastosowana po ponownym uruchomieniu QGIS</translation>
    </message>
    <message>
        <source>&amp;Rendering</source>
        <translation>&amp;Wyświetlanie</translation>
    </message>
    <message>
        <source>Map display will be updated (drawn) after this many features have been read from the data source</source>
        <translation>Ekran mapy będzie odświeżany po wczytaniu podanej tu ilości obiektów</translation>
    </message>
    <message>
        <source>Select Global Default ...</source>
        <translation>Wskaż domyślne globalnie...</translation>
    </message>
    <message>
        <source>Make lines appear less jagged at the expense of some drawing performance</source>
        <translation>Pokaż linie jako mniej postrzępione kosztem wydajności wyświetlania</translation>
    </message>
    <message>
        <source>By default new la&amp;yers added to the map should be displayed</source>
        <translation>Domyślnie nowo dodawane warstw&amp;y są wyświetlane</translation>
    </message>
    <message>
        <source>Measure tool</source>
        <translation>Narzędzie pomiaru</translation>
    </message>
    <message>
        <source>Search radius</source>
        <translation>Promień wyszukiwania</translation>
    </message>
    <message>
        <source>Pro&amp;jection</source>
        <translation type="obsolete">Układ &amp;współrzędnych</translation>
    </message>
    <message>
        <source>When layer is loaded that has no projection information</source>
        <translation type="obsolete">Gdy dodawana warstwa nie posiada informacji o układzie współrzędnych</translation>
    </message>
    <message>
        <source>Fix problems with incorrectly filled polygons</source>
        <translation>Napraw problemy związane z nieprawidłowo wypełnionymi poligonami</translation>
    </message>
    <message>
        <source>Continuously redraw the map when dragging the legend/map divider</source>
        <translation>Płynne odświeżanie mapy w trakcie przeciągania legendy/mapy</translation>
    </message>
    <message>
        <source>&amp;Map tools</source>
        <translation>&amp;Narzędzia</translation>
    </message>
    <message>
        <source>%</source>
        <translation>%</translation>
    </message>
    <message>
        <source>Panning and zooming</source>
        <translation>Przesuwanie i powiększanie</translation>
    </message>
    <message>
        <source>Zoom</source>
        <translation>Powiększ</translation>
    </message>
    <message>
        <source>Zoom and recenter</source>
        <translation>Powiększ i wycentruj</translation>
    </message>
    <message>
        <source>Nothing</source>
        <translation>Brak</translation>
    </message>
    <message>
        <source>&amp;General</source>
        <translation>&amp;Ogólne</translation>
    </message>
    <message>
        <source>Locale</source>
        <translation>Język</translation>
    </message>
    <message>
        <source>Locale to use instead</source>
        <translation>Zastąp ustawieniami lokalnymi</translation>
    </message>
    <message>
        <source>Additional Info</source>
        <translation>Informacje dodatkowe</translation>
    </message>
    <message>
        <source>Detected active locale on your system:</source>
        <translation>Wykryte ustawienia lokalne w twoim systemie:</translation>
    </message>
    <message>
        <source>Selecting this will unselect the &apos;make lines less&apos; jagged toggle</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Digitizing</source>
        <translation>Digitalizacja</translation>
    </message>
    <message>
        <source>Rubberband</source>
        <translation>Linijka</translation>
    </message>
    <message>
        <source>Line width in pixels</source>
        <translation>Szerokość linii w pikselach</translation>
    </message>
    <message>
        <source>Snapping</source>
        <translation>Przyciąganie</translation>
    </message>
    <message>
        <source>Zoom to mouse cursor</source>
        <translation>Powiększ do kursora</translation>
    </message>
    <message>
        <source>Vertex markers</source>
        <translation>Znaczniki wierzchołków</translation>
    </message>
    <message>
        <source>Project files</source>
        <translation>Pliki projektu</translation>
    </message>
    <message>
        <source>Prompt to save project changes when required</source>
        <translation>Informuj o konieczności zapisu gdy projekt ulegnie zmianie</translation>
    </message>
    <message>
        <source>Warn when opening a project file saved with an older version of QGIS</source>
        <translation>Ostrzegaj przy otwieraniu projektów zapisanych w starszych wersjach QGIS</translation>
    </message>
    <message>
        <source>Default Map Appearance (overridden by project properties)</source>
        <translation>Domyślny wygląd mapy (nadpisywane przez właściwości projektu)</translation>
    </message>
    <message>
        <source>Selection color</source>
        <translation>Kolor obiektów wybranych</translation>
    </message>
    <message>
        <source>Background color</source>
        <translation>Kolor tła</translation>
    </message>
    <message>
        <source>&amp;Application</source>
        <translation>&amp;Program</translation>
    </message>
    <message>
        <source>Icon theme</source>
        <translation>Motyw ikon</translation>
    </message>
    <message>
        <source>Capitalise layer names in legend</source>
        <translation>Nazwy warstw w legendzie dużymi literami</translation>
    </message>
    <message>
        <source>Rendering behavior</source>
        <translation>Właściwości wyświetlania</translation>
    </message>
    <message>
        <source>Number of features to draw before updating the display</source>
        <translation>Liczba obiektów koniecznych do narysowania przed odświeżeniem widoku</translation>
    </message>
    <message>
        <source>Rendering quality</source>
        <translation>Jakość wyświetlania</translation>
    </message>
    <message>
        <source>Zoom factor</source>
        <translation>Współczynnik powiększenia</translation>
    </message>
    <message>
        <source>Mouse wheel action</source>
        <translation>Działanie kółka na myszce</translation>
    </message>
    <message>
        <source>Rubberband color</source>
        <translation>Kolor linijki</translation>
    </message>
    <message>
        <source>Ellipsoid for distance calculations</source>
        <translation>Elipsoida do pomiaru odległości</translation>
    </message>
    <message>
        <source>Search radius for identifying features and displaying map tips</source>
        <translation>Promień wyszukiwania przy wskazywaniu obiektów i wyświetlaniu podpowiedzi</translation>
    </message>
    <message>
        <source>Line width</source>
        <translation>Szerokość linii</translation>
    </message>
    <message>
        <source>Line colour</source>
        <translation>Kolor linii</translation>
    </message>
    <message>
        <source>Default snap mode</source>
        <translation>Domyślny tryb przyciągania</translation>
    </message>
    <message>
        <source>Default snapping tolerance in layer units</source>
        <translation>Domyślna tolerancja przyciągania w jednostkach warstwy</translation>
    </message>
    <message>
        <source>Search radius for vertex edits in layer units</source>
        <translation>Promień wyszukiwania przy edycji wierzchołków w jednostkach warstwy</translation>
    </message>
    <message>
        <source>Marker style</source>
        <translation>Styl znacznika</translation>
    </message>
    <message>
        <source>Override system locale</source>
        <translation>Nadpisz ustawienia lokalne</translation>
    </message>
    <message>
        <source>Display classification attribute names in legend</source>
        <translation>Wyświetl nazwy atrybutów w legendzie</translation>
    </message>
    <message>
        <source>&lt;b&gt;Note:&lt;/b&gt; Use zero to prevent display updates until all features have been rendered</source>
        <translation>&lt;b&gt;Uwaga:&lt;/b&gt; Wybierz zero aby zapobiec odświeżaniu przed wyrenderowaniem wszystkich obiektów</translation>
    </message>
    <message>
        <source>&lt;b&gt;Note:&lt;/b&gt; Specify the search radius as a percentage of the map width</source>
        <translation>&lt;b&gt;Uwaga:&lt;/b&gt; Podaj promień poszukiwań jako procent szerokości mapy</translation>
    </message>
    <message>
        <source>&lt;b&gt;Note:&lt;/b&gt; Enabling / changing overide on local requires an application restart</source>
        <translation>&lt;b&gt;Uwaga:&lt;/b&gt; Nadpisanie lub zmiana ustawień lokalnych wymaga ponownego uruchomienia programu</translation>
    </message>
    <message>
        <source>Proxy</source>
        <translation>Serwer proxy</translation>
    </message>
    <message>
        <source>Use proxy for web access</source>
        <translation>Użyj serwera proxy przy dostępie do sieci</translation>
    </message>
    <message>
        <source>Host</source>
        <translation>Host</translation>
    </message>
    <message>
        <source>Port</source>
        <translation>Port</translation>
    </message>
    <message>
        <source>User</source>
        <translation>Użytkownik</translation>
    </message>
    <message>
        <source>Leave this blank if no proxy username / password are required</source>
        <translation>Pozostaw puste jeśli nawa użytkownika / hasło nie są wymagane</translation>
    </message>
    <message>
        <source>Password</source>
        <translation>Hasło</translation>
    </message>
    <message>
        <source>Open attribute table in a dock window</source>
        <translation>Otwórz tabelę atrybutów w oknie dokowanym</translation>
    </message>
    <message>
        <source>Attribute table behaviour</source>
        <translation>Zachowanie tabeli atrybutów</translation>
    </message>
    <message>
        <source>CRS</source>
        <translation>SOW</translation>
    </message>
    <message>
        <source>When layer is loaded that has no coordinate reference system (CRS)</source>
        <translation>Wgrywana warstwa nie zawiera informacji o systemie odniesienia opartym na współrzędnych (SOW)</translation>
    </message>
    <message>
        <source>Prompt for CRS</source>
        <translation>Pytaj o SOW</translation>
    </message>
    <message>
        <source>Project wide default CRS will be used</source>
        <translation>Zostanie zastosowany domyślny SOW projektu</translation>
    </message>
    <message>
        <source>Global default CRS displa&amp;yed below will be used</source>
        <translation>Zostanie zastosowany &amp;globalny domyślny SOW wyświetlony poniżej </translation>
    </message>
</context>
<context>
    <name>QgsPasteTransformationsBase</name>
    <message>
        <source>Paste Transformations</source>
        <translation>Wstaw transformację</translation>
    </message>
    <message>
        <source>&lt;b&gt;Note: This function is not useful yet!&lt;/b&gt;</source>
        <translation>&lt;b&gt;Uwaga: Ta funkcja jeszcze nie działa!&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Source</source>
        <translation>Źródło</translation>
    </message>
    <message>
        <source>Destination</source>
        <translation>Cel</translation>
    </message>
    <message>
        <source>&amp;Help</source>
        <translation>&amp;Pomoc</translation>
    </message>
    <message>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <source>Add New Transfer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;OK</source>
        <translation>&amp;OK</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>&amp;Anuluj</translation>
    </message>
</context>
<context>
    <name>QgsPgGeoprocessing</name>
    <message>
        <source>Buffer features in layer %1</source>
        <translation>Buforuj obiekty w warstwie %1</translation>
    </message>
    <message>
        <source>Error connecting to the database</source>
        <translation>Błąd podczas połączenia z bazą danych</translation>
    </message>
    <message>
        <source>&amp;Buffer features</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>A new layer is created in the database with the buffered features.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Geoprocessing</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Unable to add geometry column</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Unable to add geometry column to the output table </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Unable to create table</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Failed to create the output table </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>No GEOS support</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Buffer function requires GEOS support in PostGIS</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>No Active Layer</source>
        <translation>Brak aktywnej warstwy</translation>
    </message>
    <message>
        <source>You must select a layer in the legend to buffer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Not a PostgreSQL/PostGIS Layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> is not a PostgreSQL/PostGIS layer.
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Geoprocessing functions are only available for PostgreSQL/PostGIS Layers</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Create a buffer for a PostgreSQL layer. </source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsPgQueryBuilder</name>
    <message>
        <source>Table &lt;b&gt;%1&lt;/b&gt; in database &lt;b&gt;%2&lt;/b&gt; on host &lt;b&gt;%3&lt;/b&gt;, user &lt;b&gt;%4&lt;/b&gt;</source>
        <translation>Tabela &lt;b&gt;%1&lt;/b&gt; w bazie danych &lt;b&gt;%2&lt;/b&gt; na serwerze &lt;b&gt;%3&lt;/b&gt;, użytkownik &lt;b&gt;%4&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Connection Failed</source>
        <translation>Połączenie nie powiodło się</translation>
    </message>
    <message>
        <source>Connection to the database failed:</source>
        <translation>Połączenie z bazą danych nie powiodło się:</translation>
    </message>
    <message>
        <source>Database error</source>
        <translation>Błąd bazy danych</translation>
    </message>
    <message>
        <source>Query Result</source>
        <translation>Wyniki zapytania</translation>
    </message>
    <message>
        <source>The where clause returned </source>
        <translation>Klauzula where zwróciła</translation>
    </message>
    <message>
        <source> rows.</source>
        <translation>rekordów.</translation>
    </message>
    <message>
        <source>Query Failed</source>
        <translation>Zapytanie nie powiodło się</translation>
    </message>
    <message>
        <source>An error occurred when executing the query:</source>
        <translation>Wystąpił błąd podczas wykonywania zapytania:</translation>
    </message>
    <message>
        <source>No Records</source>
        <translation>Brak rekordów</translation>
    </message>
    <message>
        <source>The query you specified results in zero records being returned. Valid PostgreSQL layers must have at least one feature.</source>
        <translation>Zapytanie nie zwróciło rekordów. Poprawna warstwa w PostgreSQL musi posiadać przynajmniej jeden obiekt.</translation>
    </message>
    <message>
        <source>&lt;p&gt;Failed to get sample of field values using SQL:&lt;/p&gt;&lt;p&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>No Query</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>You must create a query before you can test it</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Error in Query</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsPgQueryBuilderBase</name>
    <message>
        <source>PostgreSQL Query Builder</source>
        <translation>Kreator zapytań PostgreSQL</translation>
    </message>
    <message>
        <source>Clear</source>
        <translation>Wyczyść</translation>
    </message>
    <message>
        <source>Test</source>
        <translation>Testuj</translation>
    </message>
    <message>
        <source>Ok</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Anuluj</translation>
    </message>
    <message>
        <source>Values</source>
        <translation>Wartości</translation>
    </message>
    <message>
        <source>All</source>
        <translation>Wszystko</translation>
    </message>
    <message>
        <source>Sample</source>
        <translation>Przykład</translation>
    </message>
    <message>
        <source>Fields</source>
        <translation>Pola</translation>
    </message>
    <message>
        <source>Operators</source>
        <translation>Operatory</translation>
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
        <translation>LIKE</translation>
    </message>
    <message>
        <source>AND</source>
        <translation>AND</translation>
    </message>
    <message>
        <source>ILIKE</source>
        <translation>ILIKE</translation>
    </message>
    <message>
        <source>OR</source>
        <translation>OR</translation>
    </message>
    <message>
        <source>NOT</source>
        <translation>NOT</translation>
    </message>
    <message>
        <source>SQL where clause</source>
        <translation>Klauzula where</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Retrieve &lt;span style=&quot; font-weight:600;&quot;&gt;all&lt;/span&gt; the record in the vector file (&lt;span style=&quot; font-style:italic;&quot;&gt;if the table is big, the operation can consume some time&lt;/span&gt;)&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Take a &lt;span style=&quot; font-weight:600;&quot;&gt;sample&lt;/span&gt; of records in the vector file&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;List of values for the current field.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;List of fields in this vector file&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Datasource</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsPluginInstaller</name>
    <message>
        <source>Couldn&apos;t parse output from the repository</source>
        <translation>Nie mogę rozczytać zawartości repozytorium </translation>
    </message>
    <message>
        <source>Couldn&apos;t open the system plugin directory</source>
        <translation>Nie mogę otworzyć katalogu wtyczek systemowych </translation>
    </message>
    <message>
        <source>Couldn&apos;t open the local plugin directory</source>
        <translation>Nie mogę otworzyć katalogu wtyczek użytkownika </translation>
    </message>
    <message>
        <source>Check permissions or remove it manually</source>
        <translation>Sprawdź uprawnienia lub usuń go ręcznie</translation>
    </message>
    <message>
        <source>Fetch Python Plugins...</source>
        <translation>Pobierz więcej wtyczek...</translation>
    </message>
    <message>
        <source>Install more plugins from remote repositories</source>
        <translation>Zainstaluj więcej wtyczek z repozytoriów sieciowych</translation>
    </message>
    <message>
        <source>Looking for new plugins...</source>
        <translation>Szukam nowych wtyczek...</translation>
    </message>
    <message>
        <source>There is a new plugin available</source>
        <translation>Dostępna jest nowa wtyczka</translation>
    </message>
    <message>
        <source>There is a plugin update available</source>
        <translation>Dostępna jest aktualizacja wtyczki</translation>
    </message>
    <message>
        <source>QGIS Python Plugin Installer</source>
        <translation>Instalator pytonowych wtyczek Quantum GISa</translation>
    </message>
    <message>
        <source>Error reading repository:</source>
        <translation>Błąd odczytu repozytorium: </translation>
    </message>
    <message>
        <source>Failed to remove the directory:</source>
        <translation>Nie mogę usunąć katalogu:</translation>
    </message>
    <message>
        <source>Nothing to remove! Plugin directory doesn&apos;t exist:</source>
        <translation>Nie ma czego usunąć! Katalog z wtyczką nie istnieje:</translation>
    </message>
</context>
<context>
    <name>QgsPluginInstallerDialog</name>
    <message>
        <source>QGIS Python Plugin Installer</source>
        <translation>Instalator pytonowych wtyczek Quantum GISa</translation>
    </message>
    <message>
        <source>all repositories</source>
        <translation>wszystkie repozytoria</translation>
    </message>
    <message>
        <source>Install/upgrade plugin</source>
        <translation>Instaluj/aktualizuj wtyczkę</translation>
    </message>
    <message>
        <source>connected</source>
        <translation>podłączone</translation>
    </message>
    <message>
        <source>This repository is connected</source>
        <translation>To repozytorium jest podłączone</translation>
    </message>
    <message>
        <source>unavailable</source>
        <translation>niedostępne</translation>
    </message>
    <message>
        <source>This repository is enabled, but unavailable</source>
        <translation>To repozytorium jest włączone, ale nie jest aktualnie dostępne</translation>
    </message>
    <message>
        <source>disabled</source>
        <translation>wyłączone</translation>
    </message>
    <message>
        <source>This repository is disabled</source>
        <translation>To repozytorium jest wyłączone</translation>
    </message>
    <message>
        <source>orphans</source>
        <translation>sieroty</translation>
    </message>
    <message>
        <source>any status</source>
        <translation>dowolny stan</translation>
    </message>
    <message>
        <source>upgradeable and news</source>
        <translation>aktualizowalne i nowości</translation>
    </message>
    <message>
        <source>This plugin is not installed</source>
        <translation>Ta wtyczka nie jest zainstalowana</translation>
    </message>
    <message>
        <source>This plugin is installed</source>
        <translation>Ta wtyczka jest zainstalowana</translation>
    </message>
    <message>
        <source>This plugin is installed, but there is an updated version available</source>
        <translation>Ta wtyczka jest zainstalowana, ale dostępna jest nowsza wersja</translation>
    </message>
    <message>
        <source>This plugin is installed, but I can&apos;t find it in any enabled repository</source>
        <translation>Ta wtyczka jest zainstalowana, ale nie widzę jej w żadnym repozytorium</translation>
    </message>
    <message>
        <source>This plugin is not installed and is seen for the first time</source>
        <translation>Ta wtyczka nie jest zainstalowana i widzę ją pierwszy raz</translation>
    </message>
    <message>
        <source>This plugin is installed and is newer than its version available in a repository</source>
        <translation>Ta wtyczka jest zainstalowana i jest nawet nowsza niż wersja dostępna w repozytorium</translation>
    </message>
    <message>
        <source>installed version</source>
        <translation>wersja zainstalowana</translation>
    </message>
    <message>
        <source>available version</source>
        <translation>wersja dostępna</translation>
    </message>
    <message>
        <source>That&apos;s the newest available version</source>
        <translation>To jest najnowsza dostępna wersja</translation>
    </message>
    <message>
        <source>There is no version available for download</source>
        <translation>Wtyczka nie jest dostępna w repozytoriach</translation>
    </message>
    <message>
        <source>only locally available</source>
        <translation>dostępna tylko lokalnie</translation>
    </message>
    <message>
        <source>Install plugin</source>
        <translation>Zainstaluj wtyczkę</translation>
    </message>
    <message>
        <source>Reinstall plugin</source>
        <translation>Przeinstaluj wtyczkę</translation>
    </message>
    <message>
        <source>Upgrade plugin</source>
        <translation>Aktualizuj wtyczkę</translation>
    </message>
    <message>
        <source>Downgrade plugin</source>
        <translation>Cofnij wtyczkę</translation>
    </message>
    <message>
        <source>Plugin installation failed</source>
        <translation>Instalacja wtyczki nie powiodła się</translation>
    </message>
    <message>
        <source>Plugin installed successfully</source>
        <translation>Wtyczka zainstalowana pomyślnie</translation>
    </message>
    <message>
        <source>Plugin uninstall failed</source>
        <translation>Odinstalowanie wtyczki nie powiodło się</translation>
    </message>
    <message>
        <source>Warning: this plugin isn&apos;t available in any accessible repository!</source>
        <translation>Uwaga: ta wtyczka jest niedostępna w żadnym z włączonych repozytoriów!</translation>
    </message>
    <message>
        <source>Plugin uninstalled successfully</source>
        <translation>Wtyczka odinstalowana pomyślnie</translation>
    </message>
    <message>
        <source>Unable to add another repository with the same URL!</source>
        <translation>Nie można dodać drugiego repozytorium pod tym samym adresem!</translation>
    </message>
    <message>
        <source>not installed</source>
        <comment>plural</comment>
        <translation>niezainstalowane</translation>
    </message>
    <message>
        <source>installed</source>
        <comment>plural</comment>
        <translation>zainstalowane</translation>
    </message>
    <message>
        <source>not installed</source>
        <comment>singular</comment>
        <translation>niezainstalowana</translation>
    </message>
    <message>
        <source>installed</source>
        <comment>singular</comment>
        <translation>zainstalowana</translation>
    </message>
    <message>
        <source>upgradeable</source>
        <comment>singular</comment>
        <translation>aktualizowalna</translation>
    </message>
    <message>
        <source>new!</source>
        <comment>singular</comment>
        <translation>nowość!</translation>
    </message>
    <message>
        <source>invalid</source>
        <comment>singular</comment>
        <translation>niesprawna</translation>
    </message>
    <message>
        <source>Error reading repository:</source>
        <translation>Błąd odczytu repozytorium:</translation>
    </message>
    <message>
        <source>This repository is blocked due to incompatibility with your Quantum GIS version</source>
        <translation>To repozytorium jest zablokowane z powodu niekompatybilności z Twoją wersją Quantum GISa</translation>
    </message>
    <message>
        <source>Are you sure you want to uninstall the following plugin?</source>
        <translation>Na pewno chcesz odinstalować tę wtyczkę?</translation>
    </message>
    <message>
        <source>You are going to add some plugin repositories neither authorized nor supported by the Quantum GIS team, however provided by folks associated with us. Plugin authors generally make efforts to make their works useful and safe, but we can&apos;t assume any responsibility for them. FEEL WARNED!</source>
        <translation>Zamierzasz dodać repozytoria nieautoryzowane i niewspierane przez zespół Quantum GISa, jednak prowadzone przez ludzi związanych z nami. Autorzy generalnie dokładają starań, by ich wtyczki były użyteczne i bezpieczne, lecz nie możemy brać za nich żadnej odpowiedzialności. ZOSTAŁAŚ LUB ZOSTAŁEŚ OSTRZEŻONY!</translation>
    </message>
    <message>
        <source>Are you sure you want to remove the following repository?</source>
        <translation>Na pewno chcesz usunąć to repozytorium?</translation>
    </message>
    <message>
        <source>Are you sure you want to downgrade the plugin to the latest available version? The installed one is newer!</source>
        <translation>Na pewno chcesz cofnąć wersję wtyczki do ostatniej dostępnej? Zainstalowana wtyczka jest nowsza!</translation>
    </message>
    <message>
        <source>Plugin has disappeared</source>
        <translation>Wtyczka zniknęła</translation>
    </message>
    <message>
        <source>The plugin seems to have been installed but I don&apos;t know where. Probably the plugin package contained a wrong named directory.
Please search the list of installed plugins. I&apos;m nearly sure you&apos;ll find the plugin there, but I just can&apos;t determine which of them it is. It also means that I won&apos;t be able to determine if this plugin is installed and inform you about available updates. However the plugin may work. Please contact the plugin author and submit this issue.</source>
        <translation>Wszystko wskazuje na to, że zainstalowałem wtyczkę, lecz nie mam pojęcia gdzie. Prawdopodobnie paczka zip była źle przygotowana i zawierała błędnie nazwany katalog.
Przejrzyj proszę listę zainstalowanych wtyczek. Jestem prawie pewien, że znajdziesz ją tam, lecz ja po prostu nie umiem ustalić, która to. Oznacza to także, że nie będę w stanie określić, czy ta wtyczka jest zainstalowana ani powiadamiać o dostępnych uaktualnieniach. Mimo to może będzie działać. Proszę skontaktuj się z jej autorem i zgłoś ten problem.</translation>
    </message>
    <message>
        <source>This plugin is broken</source>
        <translation>Ta wtyczka jest zepsuta</translation>
    </message>
    <message>
        <source>This plugin requires a newer version of Quantum GIS</source>
        <translation>Ta wtyczka wymaga nowszej wersji programu Quantum GIS</translation>
    </message>
    <message>
        <source>This plugin requires a missing module</source>
        <translation>Ta wtyczka wymaga brakującego modułu</translation>
    </message>
    <message>
        <source>Plugin reinstalled successfully</source>
        <translation>Wtyczka przeinstalowana pomyślnie</translation>
    </message>
    <message>
        <source>The plugin is designed for a newer version of Quantum GIS. The minimum required version is:</source>
        <translation>Wtyczka jest przeznaczona do nowszej wersji programu Quantum GIS. Minimalna wymagana wersja to:</translation>
    </message>
    <message>
        <source>The plugin is broken. Python said:</source>
        <translation>Wtyczka jest uszkodzona. Python odpowiedział:</translation>
    </message>
    <message>
        <source>This plugin is incompatible with your Quantum GIS version and probably won&apos;t work.</source>
        <translation>Ta wtyczka jest niekompatybilna z tą wersją programu Quantum GIS i prawdopodobnie nie będzie działać.</translation>
    </message>
    <message>
        <source>This plugin seems to be broken.
It has been installed but can&apos;t be loaded.
Here is the error message:</source>
        <translation>Ta wtyczka wygląda na uszkodzoną.
Jest zainstalowana, ale nie mogę jej załadować.
Oto komunikat błędu:</translation>
    </message>
    <message>
        <source>The plugin depends on some components missing on your system. You need to install the following Python module in order to enable it:</source>
        <translation>Ta wtyczka jest zależna od komponentów, które nie są zainstalowane na tym komputerze. Żeby działała, musisz zainstalować następujący moduł Pythona:</translation>
    </message>
    <message>
        <source>Note that it&apos;s an uninstallable core plugin</source>
        <translation>To jest nieusuwalna wtyczka systemowa</translation>
    </message>
    <message>
        <source>The required Python module is not installed.
For more information, please visit its homepage and Quantum GIS wiki.</source>
        <translation>Wymagany moduł Pythona nie jest zainstalowany. Dla dalszych informacji proszę odwiedzić jego stronę domową i wiki QGISa.</translation>
    </message>
    <message>
        <source>Python plugin installed.
Now you need to enable it in Plugin Manager.</source>
        <translation>Wtyczka zainstalowana pomyślnie.
Musisz ją teraz włączyć w Menedżerze Wtyczek.</translation>
    </message>
    <message>
        <source>Python plugin reinstalled.
You need to restart Quantum GIS in order to reload it.</source>
        <translation>Wtyczka przeinstalowana pomyślnie.
Musisz włączyć ponownie Quantum GISa, żeby ją przeładować.</translation>
    </message>
    <message>
        <source>Python plugin uninstalled. Note that you may need to restart Quantum GIS in order to remove it completely.</source>
        <translation>Wtyczka odinstalowana pomyślnie. W niektórych przypadkach pełne usunięcie nastąpi po ponownym uruchomieniu programu.</translation>
    </message>
</context>
<context>
    <name>QgsPluginInstallerDialogBase</name>
    <message>
        <source>QGIS Python Plugin Installer</source>
        <translation>Instalator pytonowych wtyczek Quantum GISa</translation>
    </message>
    <message>
        <source>Plugins</source>
        <translation>Wtyczki</translation>
    </message>
    <message>
        <source>List of available and installed plugins</source>
        <translation>Lista dostępnych i zainstalowanych wtyczek</translation>
    </message>
    <message>
        <source>Filter:</source>
        <translation>Filtr:</translation>
    </message>
    <message>
        <source>Display only plugins containing this word in their metadata</source>
        <translation>Wyświetlaj tylko wtyczki zawierające w metadanych to wyrażenie</translation>
    </message>
    <message>
        <source>Display only plugins from given repository</source>
        <translation>Wyświetlaj tylko wtyczki z tego repozytorium</translation>
    </message>
    <message>
        <source>all repositories</source>
        <translation>wszystkie repozytoria</translation>
    </message>
    <message>
        <source>Display only plugins with matching status</source>
        <translation>Wyświetlaj tylko wtyczki w tym stanie</translation>
    </message>
    <message>
        <source>Status</source>
        <translation>Stan</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Nazwa</translation>
    </message>
    <message>
        <source>Version</source>
        <translation>Wersja</translation>
    </message>
    <message>
        <source>Description</source>
        <translation>Opis</translation>
    </message>
    <message>
        <source>Author</source>
        <translation>Autor</translation>
    </message>
    <message>
        <source>Repository</source>
        <translation>Repozytorium</translation>
    </message>
    <message>
        <source>Install, reinstall or upgrade the selected plugin</source>
        <translation>Zainstaluj, przeinstaluj lub zaktualizuj wybraną wtyczkę</translation>
    </message>
    <message>
        <source>Install/upgrade plugin</source>
        <translation>Instaluj/aktualizuj wtyczkę</translation>
    </message>
    <message>
        <source>Uninstall the selected plugin</source>
        <translation>Odinstaluj wybraną wtyczkę</translation>
    </message>
    <message>
        <source>Uninstall plugin</source>
        <translation>Odinstaluj wtyczkę</translation>
    </message>
    <message>
        <source>Repositories</source>
        <translation>Repozytoria</translation>
    </message>
    <message>
        <source>List of plugin repositories</source>
        <translation>Lista repozytoriów wtyczek</translation>
    </message>
    <message>
        <source>URL</source>
        <translation>URL</translation>
    </message>
    <message>
        <source>Allow the Installer to look for updates and news in enabled repositories on QGIS startup</source>
        <translation>Zezwól instalatorowi szukać aktualizacji i nowości podczas startu programu Quantum GIS</translation>
    </message>
    <message>
        <source>Check for updates on startup</source>
        <translation>Poszukuj aktualizacji przy starcie</translation>
    </message>
    <message>
        <source>Add third party plugin repositories to the list</source>
        <translation>Dodaj do listy niezależne repozytoria wtyczek</translation>
    </message>
    <message>
        <source>Add 3rd party repositories</source>
        <translation>Dodaj niezależne repozytoria</translation>
    </message>
    <message>
        <source>Add a new plugin repository</source>
        <translation>Dodaj nowe repozytorium wtyczek</translation>
    </message>
    <message>
        <source>Add...</source>
        <translation>Dodaj...</translation>
    </message>
    <message>
        <source>Edit the selected repository</source>
        <translation>Zmień szczegóły wybranego repozytorium</translation>
    </message>
    <message>
        <source>Edit...</source>
        <translation>Zmień...</translation>
    </message>
    <message>
        <source>Remove the selected repository</source>
        <translation>Usuń wybrane repozytorium</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>Usuń</translation>
    </message>
    <message>
        <source>The plugins will be installed to ~/.qgis/python/plugins</source>
        <translation>Wtyczki będą zainstalowane w katalogu ~/.qgis/python/plugins</translation>
    </message>
    <message>
        <source>Close the Installer window</source>
        <translation>Zamknij okno Instalatora</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Zamknij</translation>
    </message>
</context>
<context>
    <name>QgsPluginInstallerFetchingDialog</name>
    <message>
        <source>Success</source>
        <translation>Gotowe</translation>
    </message>
    <message>
        <source>Resolving host name...</source>
        <translation>Rozwiązywanie nazwy serwera...</translation>
    </message>
    <message>
        <source>Connecting...</source>
        <translation>Łączenie...</translation>
    </message>
    <message>
        <source>Host connected. Sending request...</source>
        <translation>Podłączono do serwera. Wysyłanie żądania...</translation>
    </message>
    <message>
        <source>Downloading data...</source>
        <translation>Pobieranie danych...</translation>
    </message>
    <message>
        <source>Idle</source>
        <translation>Bezczynność</translation>
    </message>
    <message>
        <source>Closing connection...</source>
        <translation>Zamykanie połączenia...</translation>
    </message>
    <message>
        <source>Error</source>
        <translation>Błąd</translation>
    </message>
</context>
<context>
    <name>QgsPluginInstallerFetchingDialogBase</name>
    <message>
        <source>Fetching repositories</source>
        <translation>Pobieranie zawartości repozytoriów</translation>
    </message>
    <message>
        <source>Overall progress:</source>
        <translation>Całkowity postęp:</translation>
    </message>
    <message>
        <source>Abort fetching</source>
        <translation>Przerwij</translation>
    </message>
    <message>
        <source>Repository</source>
        <translation>Repozytorium</translation>
    </message>
    <message>
        <source>State</source>
        <translation>Stan</translation>
    </message>
</context>
<context>
    <name>QgsPluginInstallerInstallingDialog</name>
    <message>
        <source>Connecting...</source>
        <translation>Łączenie...</translation>
    </message>
    <message>
        <source>Installing...</source>
        <translation>Instalacja...</translation>
    </message>
    <message>
        <source>Resolving host name...</source>
        <translation>Rozwiązywanie nazwy serwera...</translation>
    </message>
    <message>
        <source>Host connected. Sending request...</source>
        <translation>Podłączono do serwera. Wysyłanie żądania...</translation>
    </message>
    <message>
        <source>Downloading data...</source>
        <translation>Pobieranie danych...</translation>
    </message>
    <message>
        <source>Idle</source>
        <translation>Bezczynność</translation>
    </message>
    <message>
        <source>Closing connection...</source>
        <translation>Zamykanie połączenia...</translation>
    </message>
    <message>
        <source>Error</source>
        <translation>Błąd</translation>
    </message>
    <message>
        <source>Aborted by user</source>
        <translation>Przerwano przez użytkownika</translation>
    </message>
    <message>
        <source>Failed to unzip the plugin package. Probably it&apos;s broken or missing from the repository. You may also want to make sure that you have write permission to the plugin directory:</source>
        <translation>Nie mogę rozpakować paczki z wtyczką. Prawdopodobnie jest uszkodzona lub brakowało jej na serwerze. Możesz także upewnić się, czy masz prawa do zapisu w katalogu wtyczek:</translation>
    </message>
</context>
<context>
    <name>QgsPluginInstallerInstallingDialogBase</name>
    <message>
        <source>QGIS Python Plugin Installer</source>
        <translation>Instalator pytonowych wtyczek Quantum GISa</translation>
    </message>
    <message>
        <source>Installing plugin:</source>
        <translation>Instalacja wtyczki: </translation>
    </message>
    <message>
        <source>Connecting...</source>
        <translation>Łączenie...</translation>
    </message>
</context>
<context>
    <name>QgsPluginInstallerPluginErrorDialog</name>
    <message>
        <source>no error message received</source>
        <translation>brak komunikatu błędu</translation>
    </message>
</context>
<context>
    <name>QgsPluginInstallerPluginErrorDialogBase</name>
    <message>
        <source>Error loading plugin</source>
        <translation>Błąd wczytania wtyczki </translation>
    </message>
    <message>
        <source>The plugin seems to be invalid or have unfulfilled dependencies. It has been installed, but can&apos;t be loaded. If you really need this plugin, you can contact its author or &lt;a href=&quot;http://lists.osgeo.org/mailman/listinfo/qgis-user&quot;&gt;QGIS users group&lt;/a&gt; and try to solve the problem. If not, you can just uninstall it. Here is the error message below:</source>
        <translation>Wtyczka wydaje się być uszkodzona albo jest zależna od modułów, które nie są zainstalowane. Zainstalowała się, ale nie mogę jej załadować. Jeśli jest Ci potrzebna, możesz skontaktować się z jej autorem lub &lt;a href=&quot;http://lists.osgeo.org/mailman/listinfo/qgis-user&quot;&gt;listą dyskusyjną użytkowników programu Quantum GIS&lt;/a&gt; i spróbować rozwiązać problem. Jeśli nie, możesz ją po prostu odinstalować. Poniżej znajduje się dokładny komunikat błędu:</translation>
    </message>
    <message>
        <source>Do you want to uninstall this plugin now? If you&apos;re unsure, probably you would like to do this.</source>
        <translation>Czy chcesz od razu odinstalować tę wtyczkę? Jeśli nie masz pewności, co odpowiedzieć, prawdopodobnie chcesz :)</translation>
    </message>
</context>
<context>
    <name>QgsPluginInstallerRepositoryDetailsDialogBase</name>
    <message>
        <source>Repository details</source>
        <translation>Szczegóły repozytorium</translation>
    </message>
    <message>
        <source>Name:</source>
        <translation>Nazwa:</translation>
    </message>
    <message>
        <source>Enter a name for the repository</source>
        <translation>Podaj nazwę dla repozytorium</translation>
    </message>
    <message>
        <source>URL:</source>
        <translation>Adres:</translation>
    </message>
    <message>
        <source>Enter the repository URL, beginning with &quot;http://&quot;</source>
        <translation>Podaj adres repozytorium, zaczynając od &quot;http://&quot;</translation>
    </message>
    <message>
        <source>Enable or disable the repository (disabled repositories will be omitted)</source>
        <translation>Włącz lub wyłącz repozytorium (wyłączone repozytoria będą pomijane)</translation>
    </message>
    <message>
        <source>Enabled</source>
        <translation>Włączone</translation>
    </message>
</context>
<context>
    <name>QgsPluginManager</name>
    <message>
        <source>No Plugins</source>
        <translation>Brak wtyczek</translation>
    </message>
    <message>
        <source>No QGIS plugins found in </source>
        <translation>Nie znaleziono wtyczek QGIS</translation>
    </message>
    <message>
        <source>&amp;Select All</source>
        <translation>&amp;Zaznacz wszystko</translation>
    </message>
    <message>
        <source>&amp;Clear All</source>
        <translation>&amp;Wyczyść wszystko</translation>
    </message>
    <message>
        <source>[ incompatible ]</source>
        <translation>[ niekompatybilna ]</translation>
    </message>
</context>
<context>
    <name>QgsPluginManagerBase</name>
    <message>
        <source>QGIS Plugin Manager</source>
        <translation>Menedżer wtyczek QGIS</translation>
    </message>
    <message>
        <source>To enable / disable a plugin, click its checkbox or description</source>
        <translation>Aby włączyć / wyłączyć wtyczkę, kliknij na jej pole wyboru lub opis</translation>
    </message>
    <message>
        <source>&amp;Filter</source>
        <translation>&amp;Filtr</translation>
    </message>
    <message>
        <source>Plugin Directory:</source>
        <translation>Katalog wtyczki:</translation>
    </message>
    <message>
        <source>Directory</source>
        <translation>Katalog</translation>
    </message>
</context>
<context>
    <name>QgsPointDialog</name>
    <message>
        <source>Zoom In</source>
        <translation>Powiększ</translation>
    </message>
    <message>
        <source>z</source>
        <translation>z</translation>
    </message>
    <message>
        <source>Zoom Out</source>
        <translation>Pomniejsz</translation>
    </message>
    <message>
        <source>Z</source>
        <translation>Z</translation>
    </message>
    <message>
        <source>Zoom To Layer</source>
        <translation>Powiększ do warstwy</translation>
    </message>
    <message>
        <source>Zoom to Layer</source>
        <translation>Powiększ do warstwy</translation>
    </message>
    <message>
        <source>Pan Map</source>
        <translation>Przesuń mapę</translation>
    </message>
    <message>
        <source>Pan the map</source>
        <translation>Przesuń mapę</translation>
    </message>
    <message>
        <source>Add Point</source>
        <translation>Dodaj punkt</translation>
    </message>
    <message>
        <source>.</source>
        <translation>.</translation>
    </message>
    <message>
        <source>Capture Points</source>
        <translation>Dodaj punkty</translation>
    </message>
    <message>
        <source>Delete Point</source>
        <translation>Usuń punkt</translation>
    </message>
    <message>
        <source>Delete Selected</source>
        <translation>Usuń zaznaczony</translation>
    </message>
    <message>
        <source>Linear</source>
        <translation>Liniowa</translation>
    </message>
    <message>
        <source>Helmert</source>
        <translation>Helmerta</translation>
    </message>
    <message>
        <source>Choose a name for the world file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Ostrzeżenie</translation>
    </message>
    <message>
        <source>Affine</source>
        <translation>Afiniczna</translation>
    </message>
    <message>
        <source>Not implemented!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;p&gt;An affine transform requires changing the original raster file. This is not yet supported.&lt;/p&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;p&gt;The </source>
        <translation>&lt;p&gt;</translation>
    </message>
    <message>
        <source> transform is not yet supported.&lt;/p&gt;</source>
        <translation>transformacja nie jest jeszcze obsługiwana.&lt;/p&gt;</translation>
    </message>
    <message>
        <source>Error</source>
        <translation>Błąd</translation>
    </message>
    <message>
        <source>Could not write to </source>
        <translation>Nie mogę zapisać do</translation>
    </message>
    <message>
        <source>Currently all modified files will be written in TIFF format.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;p&gt;A Helmert transform requires modifications in the raster layer.&lt;/p&gt;&lt;p&gt;The modified raster will be saved in a new file and a world file will be generated for this new file instead.&lt;/p&gt;&lt;p&gt;Are you sure that this is what you want?&lt;/p&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>-modified</source>
        <comment>Georeferencer:QgsPointDialog.cpp - used to modify a user given file name</comment>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsPointDialogBase</name>
    <message>
        <source>Transform type:</source>
        <translation>Rodzaj transformacji:</translation>
    </message>
    <message>
        <source>Zoom in</source>
        <translation>Powiększ</translation>
    </message>
    <message>
        <source>Zoom out</source>
        <translation>Pomniejsz</translation>
    </message>
    <message>
        <source>Zoom to the raster extents</source>
        <translation>Powiększ do zasięgu rastra</translation>
    </message>
    <message>
        <source>Pan</source>
        <translation>Przesuń</translation>
    </message>
    <message>
        <source>Add points</source>
        <translation>Dodaj punkty</translation>
    </message>
    <message>
        <source>Delete points</source>
        <translation>Usuń punkty</translation>
    </message>
    <message>
        <source>World file:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <source>Modified raster:</source>
        <translation>Zmodyfikowany raster:</translation>
    </message>
    <message>
        <source>Reference points</source>
        <translation>Punkty referencyjne</translation>
    </message>
    <message>
        <source>Create</source>
        <translation>Stwórz</translation>
    </message>
    <message>
        <source>Create and load layer</source>
        <translation>Stwórz i wczytaj warstwę</translation>
    </message>
</context>
<context>
    <name>QgsPostgresProvider</name>
    <message>
        <source>Unable to access relation</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Unable to access the </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> relation.
The error message from the database was:
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>No suitable key column in table</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The table has no column suitable for use as a key.

Qgis requires that the table either has a column of type
int4 with a unique constraint on it (which includes the
primary key) or has a PostgreSQL oid column.
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The unique index on column</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>is unsuitable because Qgis does not currently support non-int4 type columns as a key into the table.
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>and </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The unique index based on columns </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> is unsuitable because Qgis does not currently support multiple columns as a key into the table.
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Unable to find a key column</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> derives from </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>and is suitable.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>and is not suitable </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>type is </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> and has a suitable constraint)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> and does not have a suitable constraint)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The view you selected has the following columns, none of which satisfy the above conditions:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Qgis requires that the view has a column that can be used as a unique key. Such a column should be derived from a table column of type int4 and be a primary key, have a unique constraint on it, or be a PostgreSQL oid column. To improve performance the column should also be indexed.
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The view </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>has no column suitable for use as a unique key.
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>No suitable key column in view</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Unknown geometry type</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Column </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> in </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> has a geometry type of </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>, which Qgis does not currently support.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>. The database communication log was:
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Unable to get feature type and srid</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Note: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>initially appeared suitable but does not contain unique data, so is not suitable.
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Unable to determine table access privileges for the </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Error while adding features</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Error while deleting features</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Error while adding attributes</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Error while deleting attributes</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Error while changing attributes</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Error while changing geometry values</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>unexpected PostgreSQL error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Qgis was unable to determine the type and srid of column </source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsPostgresProvider::Conn</name>
    <message>
        <source>No GEOS Support!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Your PostGIS installation has no GEOS support.
Feature selection and identification will not work properly.
Please install PostGIS with GEOS support (http://geos.refractions.net)</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsProjectPropertiesBase</name>
    <message>
        <source>Project Properties</source>
        <translation>Właściwości projektu</translation>
    </message>
    <message>
        <source>Default project title</source>
        <translation>Domyślny tytuł projektu</translation>
    </message>
    <message>
        <source>Meters</source>
        <translation>Metry</translation>
    </message>
    <message>
        <source>Feet</source>
        <translation>Stopy</translation>
    </message>
    <message>
        <source>Decimal degrees</source>
        <translation>Stopnie dziesiętne</translation>
    </message>
    <message>
        <source>General</source>
        <translation>Ogólne</translation>
    </message>
    <message>
        <source>Automatic</source>
        <translation>Automatyczna</translation>
    </message>
    <message>
        <source>Automatically sets the number of decimal places in the mouse position display</source>
        <translation>Automatycznie ustala liczbę miejsc dziesiętnych do wyświetlania pozycji myszy</translation>
    </message>
    <message>
        <source>The number of decimal places that are used when displaying the mouse position is automatically set to be enough so that moving the mouse by one pixel gives a change in the position display</source>
        <translation>Liczba miejsc dziesiętnych do wyświetlania pozycji myszy jest ustalana automatycznie, aby móc wyświetlić zmianę pozycji o jeden piksel</translation>
    </message>
    <message>
        <source>Manual</source>
        <translation>Ręczna</translation>
    </message>
    <message>
        <source>Sets the number of decimal places to use for the mouse position display</source>
        <translation>Ustala liczbę miejsc dziesiętnych do wyświetlania pozycji myszy</translation>
    </message>
    <message>
        <source>The number of decimal places for the manual option</source>
        <translation>Liczba miejsc dziesiętnych wyświetlanych dla opcji ręcznej</translation>
    </message>
    <message>
        <source>decimal places</source>
        <translation>miejsca dziesiętne</translation>
    </message>
    <message>
        <source>Projection</source>
        <translation type="obsolete">Układ współrzędnych</translation>
    </message>
    <message>
        <source>Precision</source>
        <translation>Dokładność</translation>
    </message>
    <message>
        <source>Digitizing</source>
        <translation>Digitalizacja</translation>
    </message>
    <message>
        <source>Descriptive project name</source>
        <translation>Opisowy tytuł projektu</translation>
    </message>
    <message>
        <source>Enable topological editing</source>
        <translation>Włącz edycję topologiczną</translation>
    </message>
    <message>
        <source>Snapping options...</source>
        <translation>Opcje przyciągania...</translation>
    </message>
    <message>
        <source>Avoid intersections of new polygons</source>
        <translation>Unikaj przecinania sie nowych poligonów</translation>
    </message>
    <message>
        <source>Title and colors</source>
        <translation>Tytuł i kolory</translation>
    </message>
    <message>
        <source>Project title</source>
        <translation>Tytuł projektu</translation>
    </message>
    <message>
        <source>Selection color</source>
        <translation>Kolor obiektów wybranych</translation>
    </message>
    <message>
        <source>Background color</source>
        <translation>Kolor tła</translation>
    </message>
    <message>
        <source>Map units</source>
        <translation>Jednostki mapy</translation>
    </message>
    <message>
        <source>Coordinate Reference System (CRS)</source>
        <translation>System Odniesienia oparty na Współrzędnych (SOW)</translation>
    </message>
    <message>
        <source>Enable &apos;on the fly&apos; CRS transformation</source>
        <translation>Transformuj SOW w locie</translation>
    </message>
</context>
<context>
    <name>QgsProjectionSelector</name>
    <message>
        <source>User Defined Coordinate Systems</source>
        <translation>Układ współrzędnych użytkownika</translation>
    </message>
    <message>
        <source>Geographic Coordinate Systems</source>
        <translation>Geograficzny układ współrzędnych</translation>
    </message>
    <message>
        <source>Projected Coordinate Systems</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Resource Location Error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Error reading database file from: 
 %1
Because of this the projection selector will not work...</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsProjectionSelectorBase</name>
    <message>
        <source>Projection</source>
        <translation type="obsolete">Układ współrzędnych</translation>
    </message>
    <message>
        <source>Search</source>
        <translation>Szukaj</translation>
    </message>
    <message>
        <source>Find</source>
        <translation>Znajdź</translation>
    </message>
    <message>
        <source>EPSG ID</source>
        <translation>EPSG ID</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Nazwa</translation>
    </message>
    <message>
        <source>Id</source>
        <translation type="obsolete">Id</translation>
    </message>
    <message>
        <source>Coordinate Reference System Selector</source>
        <translation>Wybór Systemu Odniesienia opartego na Współrzędnych</translation>
    </message>
    <message>
        <source>Coordinate Reference System</source>
        <translation>System Odniesienia oparty na Współrzędnych</translation>
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
        <source>Python console</source>
        <translation>Konsola Pythona</translation>
    </message>
    <message>
        <source>&gt;&gt;&gt;</source>
        <translation>&gt;&gt;&gt;</translation>
    </message>
    <message>
        <source>To access Quantum GIS environment from this python console use object from global scope which is an instance of QgisInterface class.&lt;br&gt;Usage e.g.: iface.zoomFull()</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsQuickPrint</name>
    <message>
        <source> km</source>
        <translation>km</translation>
    </message>
    <message>
        <source> mm</source>
        <translation>mm</translation>
    </message>
    <message>
        <source> cm</source>
        <translation>cm</translation>
    </message>
    <message>
        <source> m</source>
        <translation>m</translation>
    </message>
    <message>
        <source> miles</source>
        <translation>mile</translation>
    </message>
    <message>
        <source> mile</source>
        <translation>mila</translation>
    </message>
    <message>
        <source> inches</source>
        <translation>cale</translation>
    </message>
    <message>
        <source> foot</source>
        <translation>stopa</translation>
    </message>
    <message>
        <source> feet</source>
        <translation>stopy</translation>
    </message>
    <message>
        <source> degree</source>
        <translation>stopień</translation>
    </message>
    <message>
        <source> degrees</source>
        <translation>stopnie</translation>
    </message>
    <message>
        <source> unknown</source>
        <translation>nieznany</translation>
    </message>
</context>
<context>
    <name>QgsRasterLayer</name>
    <message>
        <source>Not Set</source>
        <translation>Brak</translation>
    </message>
    <message>
        <source>Raster Extent: </source>
        <translation type="obsolete">Zasięg rastra:</translation>
    </message>
    <message>
        <source>Clipped area: </source>
        <translation type="obsolete">Obszar przycięcia:</translation>
    </message>
    <message>
        <source>Driver:</source>
        <translation>Sterownik:</translation>
    </message>
    <message>
        <source>Dimensions:</source>
        <translation>Wymiar:</translation>
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
        <source> Bands: </source>
        <translation>Kanały:</translation>
    </message>
    <message>
        <source>Data Type:</source>
        <translation>Typ danych:</translation>
    </message>
    <message>
        <source>GDT_Byte - Eight bit unsigned integer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>GDT_UInt16 - Sixteen bit unsigned integer </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>GDT_Int16 - Sixteen bit signed integer </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>GDT_UInt32 - Thirty two bit unsigned integer </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>GDT_Int32 - Thirty two bit signed integer </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>GDT_Float32 - Thirty two bit floating point </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>GDT_Float64 - Sixty four bit floating point </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>GDT_CInt16 - Complex Int16 </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>GDT_CInt32 - Complex Int32 </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>GDT_CFloat32 - Complex Float32 </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>GDT_CFloat64 - Complex Float64 </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Could not determine raster data type.</source>
        <translation>Nie można rozpoznać typu rastra.</translation>
    </message>
    <message>
        <source>Pyramid overviews:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Origin:</source>
        <translation>Początek:</translation>
    </message>
    <message>
        <source>Pixel Size:</source>
        <translation>Rozmiar piksela:</translation>
    </message>
    <message>
        <source>Band</source>
        <translation>Kanał</translation>
    </message>
    <message>
        <source>Band No</source>
        <translation>Kanał nr</translation>
    </message>
    <message>
        <source>No Stats</source>
        <translation>Brak statystyk</translation>
    </message>
    <message>
        <source>No stats collected yet</source>
        <translation>Statystyki jeszcze nie zostały zebrane</translation>
    </message>
    <message>
        <source>Min Val</source>
        <translation>Minimum</translation>
    </message>
    <message>
        <source>Max Val</source>
        <translation>Maksimum</translation>
    </message>
    <message>
        <source>Range</source>
        <translation>Zakres</translation>
    </message>
    <message>
        <source>Mean</source>
        <translation>Średnia</translation>
    </message>
    <message>
        <source>Sum of squares</source>
        <translation>Suma kwadratów</translation>
    </message>
    <message>
        <source>Standard Deviation</source>
        <translation>Odchylenie standardowe</translation>
    </message>
    <message>
        <source>Sum of all cells</source>
        <translation>Suma wszystkich cel</translation>
    </message>
    <message>
        <source>Cell Count</source>
        <translation>Liczba cel</translation>
    </message>
    <message>
        <source>Average Magphase</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Average</source>
        <translation>Średnia</translation>
    </message>
    <message>
        <source>Layer Spatial Reference System: </source>
        <translation>Przestrzenny układ odniesienia warstwy:</translation>
    </message>
    <message>
        <source>out of extent</source>
        <translation>poza obszarem</translation>
    </message>
    <message>
        <source>null (no data)</source>
        <translation>null (brak danych)</translation>
    </message>
    <message>
        <source>Dataset Description</source>
        <translation>Opis danych</translation>
    </message>
    <message>
        <source>No Data Value</source>
        <translation>Wartość oznaczająca brak danych</translation>
    </message>
    <message>
        <source>and all other files</source>
        <translation>i wszystkie inne pliki</translation>
    </message>
    <message>
        <source>NoDataValue not set</source>
        <translation>Nie podano wartości braku danych</translation>
    </message>
    <message>
        <source>Band %1</source>
        <translation>Kanał %1</translation>
    </message>
</context>
<context>
    <name>QgsRasterLayerProperties</name>
    <message>
        <source>Grayscale</source>
        <translation>Skala szarości</translation>
    </message>
    <message>
        <source>Pseudocolor</source>
        <translation>Pseudokolor</translation>
    </message>
    <message>
        <source>Freak Out</source>
        <translation>Szalone</translation>
    </message>
    <message>
        <source>Palette</source>
        <translation type="obsolete">Paleta</translation>
    </message>
    <message>
        <source>Columns: </source>
        <translation>Kolumny:</translation>
    </message>
    <message>
        <source>Rows: </source>
        <translation>Wiersze:</translation>
    </message>
    <message>
        <source>No-Data Value: </source>
        <translation>Wartość oznaczająca brak danych:</translation>
    </message>
    <message>
        <source>n/a</source>
        <translation>n/a</translation>
    </message>
    <message>
        <source>Write access denied</source>
        <translation>Brak dostępu do zapisu</translation>
    </message>
    <message>
        <source>Write access denied. Adjust the file permissions and try again.

</source>
        <translation>Brak dostępu do zapisu. Dostosuj uprawnienia do pliku i spróbuj ponownie.

</translation>
    </message>
    <message>
        <source>Building pyramids failed.</source>
        <translation>Budowa piramidy nie powiodłą się.</translation>
    </message>
    <message>
        <source>Building pyramid overviews is not supported on this type of raster.</source>
        <translation>Budow podglądu piramidy nie jes wspierana dla tego typu rastra.</translation>
    </message>
    <message>
        <source>No Stretch</source>
        <translation>Bez rozciągania</translation>
    </message>
    <message>
        <source>Stretch To MinMax</source>
        <translation>Rozciągnij do MinMax</translation>
    </message>
    <message>
        <source>Stretch And Clip To MinMax</source>
        <translation>Rozciągnij i przytnij do MinMax</translation>
    </message>
    <message>
        <source>Clip To MinMax</source>
        <translation>Przytnij do MinMax</translation>
    </message>
    <message>
        <source>Discrete</source>
        <translation>Dyskretna</translation>
    </message>
    <message>
        <source>Equal interval</source>
        <translation>Równe przedziały</translation>
    </message>
    <message>
        <source>Quantiles</source>
        <translation>Kwantyle</translation>
    </message>
    <message>
        <source>Description</source>
        <translation>Opis</translation>
    </message>
    <message>
        <source>Large resolution raster layers can slow navigation in QGIS.</source>
        <translation>Warstwy rastrowe o wysokiej rozdzielczości mogą spowolnić pracę QGIS.</translation>
    </message>
    <message>
        <source>By creating lower resolution copies of the data (pyramids) performance can be considerably improved as QGIS selects the most suitable resolution to use depending on the level of zoom.</source>
        <translation>Tworząc kopie danych w niższych rozdzielczościach (piramidy) wydajność pracy z QGIS może ulec znaczącej poprawie, gdyż wybierana jest wtedy rozdzielczość dostosowana do powiększenia.</translation>
    </message>
    <message>
        <source>You must have write access in the directory where the original data is stored to build pyramids.</source>
        <translation>Musisz mieć prawo zapisu w kartotece gdzie znajdują się oryginalne dane do budowy piramid.</translation>
    </message>
    <message>
        <source>Red</source>
        <translation>Czerwony</translation>
    </message>
    <message>
        <source>Green</source>
        <translation>Zielony</translation>
    </message>
    <message>
        <source>Blue</source>
        <translation>Niebieski</translation>
    </message>
    <message>
        <source>Percent Transparent</source>
        <translation>Procent przeźroczystości</translation>
    </message>
    <message>
        <source>Gray</source>
        <translation>Szary</translation>
    </message>
    <message>
        <source>Indexed Value</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>User Defined</source>
        <translation>Użytkownika</translation>
    </message>
    <message>
        <source>No-Data Value: Not Set</source>
        <translation>Wartość oznaczająca brak danych: brak</translation>
    </message>
    <message>
        <source>Save file</source>
        <translation>Zapisz plik</translation>
    </message>
    <message>
        <source>Textfile (*.txt)</source>
        <translation>Plik tekstowy (*.txt)</translation>
    </message>
    <message>
        <source>QGIS Generated Transparent Pixel Value Export File</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Open file</source>
        <translation>Otwórz plik</translation>
    </message>
    <message>
        <source>Import Error</source>
        <translation>Błąd importu</translation>
    </message>
    <message>
        <source>The following lines contained errors

</source>
        <translation>Następujące wiersze zawierają błędy

</translation>
    </message>
    <message>
        <source>Read access denied</source>
        <translation>Brak praw odczytu</translation>
    </message>
    <message>
        <source>Read access denied. Adjust the file permissions and try again.

</source>
        <translation>Brak praw odczytu. Dostosuj uprawnienia do pliku i spróbuj ponownie.

</translation>
    </message>
    <message>
        <source>Color Ramp</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Not Set</source>
        <translation>Brak</translation>
    </message>
    <message>
        <source>Default Style</source>
        <translation>DOmyślny styl</translation>
    </message>
    <message>
        <source>QGIS Layer Style File (*.qml)</source>
        <translation>Plik stylu warstwy QGIS (*.qml)</translation>
    </message>
    <message>
        <source>QGIS</source>
        <translation>QGIS</translation>
    </message>
    <message>
        <source>Unknown style format: </source>
        <translation>Nieznany format stylu:</translation>
    </message>
    <message>
        <source>Colormap</source>
        <translation>Mapa kolorów</translation>
    </message>
    <message>
        <source>Linear</source>
        <translation>Liniowa</translation>
    </message>
    <message>
        <source>Exact</source>
        <translation>Szczegółowa</translation>
    </message>
    <message>
        <source>Please note that building internal pyramids may alter the original data file and once created they cannot be removed!</source>
        <translation>Zważ, że budując piramidy zmieniasz oryginalne dane i dokonane zmiany nie mogą być cofnięte!</translation>
    </message>
    <message>
        <source>Please note that building internal pyramids could corrupt your image - always make a backup of your data first!</source>
        <translation>Zważ, że budując wewnętrzne piramidy możesz uszkodzić obraz - zawsze na początku utwórz kopię danych!</translation>
    </message>
    <message>
        <source>Default</source>
        <translation>Domyślny</translation>
    </message>
    <message>
        <source>The file was not writeable. Some formats do not support pyramid overviews. Consult the GDAL documentation if in doubt.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Custom color map entry</source>
        <translation>Mapa kolorów użytkownika</translation>
    </message>
    <message>
        <source>QGIS Generated Color Map Export File</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Load Color Map</source>
        <translation>Wczytaj mapę kolorów</translation>
    </message>
    <message>
        <source>Saved Style</source>
        <translation>Zapisany styl</translation>
    </message>
    <message numerus="yes">
        <source>The color map for Band %n failed to load</source>
        <translation type="unfinished">
            <numerusform></numerusform>
            <numerusform></numerusform>
            <numerusform></numerusform>
        </translation>
    </message>
    <message>
        <source>Building internal pyramid overviews is not supported on raster layers with JPEG compression.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Note: Minimum Maximum values are estimates or user defined</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Note: Minimum Maximum values are actual values computed from the band(s)</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsRasterLayerPropertiesBase</name>
    <message>
        <source>Raster Layer Properties</source>
        <translation>Właściwości warstwy rastrowej</translation>
    </message>
    <message>
        <source>Symbology</source>
        <translation>Symbolika</translation>
    </message>
    <message>
        <source>&lt;p align=&quot;right&quot;&gt;Full&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;right&quot;&gt;Pełny&lt;/p&gt;</translation>
    </message>
    <message>
        <source>None</source>
        <translation>Brak</translation>
    </message>
    <message>
        <source>General</source>
        <translation>Ogólne</translation>
    </message>
    <message>
        <source>Thumbnail</source>
        <translation>Miniatura</translation>
    </message>
    <message>
        <source>Columns:</source>
        <translation>Kolumny:</translation>
    </message>
    <message>
        <source>No Data:</source>
        <translation>Brak danych:</translation>
    </message>
    <message>
        <source>Rows:</source>
        <translation>Rekordy:</translation>
    </message>
    <message>
        <source>Legend:</source>
        <translation type="obsolete">Legenda:</translation>
    </message>
    <message>
        <source>Palette:</source>
        <translation type="obsolete">Paleta:</translation>
    </message>
    <message>
        <source>Maximum scale at which this layer will be displayed. </source>
        <translation>Maksymalna skala, dla której wartstwa będzie wyświetlana.</translation>
    </message>
    <message>
        <source>Minimum scale at which this layer will be displayed. </source>
        <translation>Minimalna skala, dla której wartstwa będzie wyświetlana.</translation>
    </message>
    <message>
        <source>Metadata</source>
        <translation>Metadane</translation>
    </message>
    <message>
        <source>Pyramids</source>
        <translation>Piramidy</translation>
    </message>
    <message>
        <source>Average</source>
        <translation>Średnia</translation>
    </message>
    <message>
        <source>Nearest Neighbour</source>
        <translation>Najbliższe sąsiedztwo</translation>
    </message>
    <message>
        <source>Change</source>
        <translation type="obsolete">Zmień</translation>
    </message>
    <message>
        <source>Histogram</source>
        <translation>Histogram</translation>
    </message>
    <message>
        <source>Options</source>
        <translation>Opcje</translation>
    </message>
    <message>
        <source>Chart Type</source>
        <translation>Typ wykresu</translation>
    </message>
    <message>
        <source>Refresh</source>
        <translation>Odśwież</translation>
    </message>
    <message>
        <source>Max</source>
        <translation>Max</translation>
    </message>
    <message>
        <source>Min</source>
        <translation>Min</translation>
    </message>
    <message>
        <source> 00%</source>
        <translation>00%</translation>
    </message>
    <message>
        <source>Render as</source>
        <translation>Renderuj jako</translation>
    </message>
    <message>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <source>Colormap</source>
        <translation>Paleta</translation>
    </message>
    <message>
        <source>Delete entry</source>
        <translation>Usuń wpis</translation>
    </message>
    <message>
        <source>Classify</source>
        <translation>Klasyfikuj</translation>
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
        <source>Three band color</source>
        <translation>Kompozycja barwna (trzy kanały)</translation>
    </message>
    <message>
        <source>Single band gray</source>
        <translation>Jeden kanał</translation>
    </message>
    <message>
        <source>Color map</source>
        <translation>Paleta</translation>
    </message>
    <message>
        <source>Invert color map</source>
        <translation>Odwróć kolory mapy</translation>
    </message>
    <message>
        <source>Std. deviation</source>
        <translation type="obsolete">Odch. standardowe</translation>
    </message>
    <message>
        <source>Custom min / max values</source>
        <translation>Wartości min / max użytkownika</translation>
    </message>
    <message>
        <source>Load min / max values from band</source>
        <translation>Wczytaj wartości min / max z kanału</translation>
    </message>
    <message>
        <source>Estimate (faster)</source>
        <translation>Oszacowane (szybciej)</translation>
    </message>
    <message>
        <source>Load</source>
        <translation>Wczytaj</translation>
    </message>
    <message>
        <source>Actual (slower)</source>
        <translation>Rzeczywiste (wolniej)</translation>
    </message>
    <message>
        <source>Contrast enhancement</source>
        <translation>Wzmocnienie kontrastu</translation>
    </message>
    <message>
        <source>Current</source>
        <translation>Bieżący</translation>
    </message>
    <message>
        <source>Save current contrast enhancement algorithm as default. This setting will be persistent between QGIS sessions.</source>
        <translation>Zapisz wybrany algorytm wzmacniania kontrastu jako domyślny. Te ustatwienia zostaną zachowane pomiędzy sesjami QGIS.</translation>
    </message>
    <message>
        <source>Saves current contrast enhancement algorithm as a default. This setting will be persistent between QGIS sessions.</source>
        <translation>Zapisuje wybrany algorytm wzmacniania kontrastu jako domyślny. Te ustatwienia zostaną zachowane pomiędzy sesjami QGIS.</translation>
    </message>
    <message>
        <source>Default</source>
        <translation>Domyślny</translation>
    </message>
    <message>
        <source>TextLabel</source>
        <translation>Etykieta tekstowa</translation>
    </message>
    <message>
        <source>Transparency</source>
        <translation>Przeźroczystość</translation>
    </message>
    <message>
        <source>Global transparency</source>
        <translation>Ogólna przeźroczystość</translation>
    </message>
    <message>
        <source>No data value</source>
        <translation>Wartość oznaczająca brak danych</translation>
    </message>
    <message>
        <source>Reset no data value</source>
        <translation>Wyczyść wartość oznaczającą brak danych</translation>
    </message>
    <message>
        <source>Custom transparency options</source>
        <translation>Opcje przeźroczystości użytkownika</translation>
    </message>
    <message>
        <source>Transparency band</source>
        <translation>Kanał przeźroczystości</translation>
    </message>
    <message>
        <source>Transparent pixel list</source>
        <translation>Lista pikseli przeźroczystości</translation>
    </message>
    <message>
        <source>Add values manually</source>
        <translation>Dodaj wartości ręcznie</translation>
    </message>
    <message>
        <source>Add Values from display</source>
        <translation>Dodaj wartości z ekranu</translation>
    </message>
    <message>
        <source>Remove selected row</source>
        <translation>Usuń wybrany wiersz</translation>
    </message>
    <message>
        <source>Default values</source>
        <translation>Wartości domyślne</translation>
    </message>
    <message>
        <source>Import from file</source>
        <translation>Importuj z pliku</translation>
    </message>
    <message>
        <source>Export to file</source>
        <translation>Eksportuj do pliku</translation>
    </message>
    <message>
        <source>Number of entries</source>
        <translation>Liczba klas</translation>
    </message>
    <message>
        <source>Color interpolation</source>
        <translation>Interpolacja koloru</translation>
    </message>
    <message>
        <source>Classification mode</source>
        <translation>Tryb klasyfikacji</translation>
    </message>
    <message>
        <source>Scale dependent visibility</source>
        <translation>Widoczność zależna od skali</translation>
    </message>
    <message>
        <source>Maximum</source>
        <translation>Maksimum</translation>
    </message>
    <message>
        <source>Minimum</source>
        <translation>Minimum</translation>
    </message>
    <message>
        <source>Layer source</source>
        <translation>Źródło warstwy</translation>
    </message>
    <message>
        <source>Display name</source>
        <translation>Nazwa wyświetlana</translation>
    </message>
    <message>
        <source>Pyramid resolutions</source>
        <translation>Rozdzielczość piramidy</translation>
    </message>
    <message>
        <source>Resampling method</source>
        <translation>Metoda interpolacji</translation>
    </message>
    <message>
        <source>Build pyramids</source>
        <translation>Twórz piramidy</translation>
    </message>
    <message>
        <source>Line graph</source>
        <translation>Wykres liniowy</translation>
    </message>
    <message>
        <source>Bar chart</source>
        <translation>Wykres słupkowy</translation>
    </message>
    <message>
        <source>Column count</source>
        <translation>Liczba kolumn</translation>
    </message>
    <message>
        <source>Out of range OK?</source>
        <translation>Dopuszczać z poza zakresu?</translation>
    </message>
    <message>
        <source>Allow approximation</source>
        <translation>Zezwól na aproksymację</translation>
    </message>
    <message>
        <source>RGB mode band selection and scaling</source>
        <translation>Tryb RGB wyboru kanału i skalowania</translation>
    </message>
    <message>
        <source>Red band</source>
        <translation>Kanał czerwony</translation>
    </message>
    <message>
        <source>Green band</source>
        <translation>Kanał zielony</translation>
    </message>
    <message>
        <source>Blue band</source>
        <translation>Kanał niebieski</translation>
    </message>
    <message>
        <source>Red min</source>
        <translation>Czerwony min</translation>
    </message>
    <message>
        <source>Red max</source>
        <translation>Czerwony max</translation>
    </message>
    <message>
        <source>Green min</source>
        <translation>Zielony min</translation>
    </message>
    <message>
        <source>Green max</source>
        <translation>Zielony max</translation>
    </message>
    <message>
        <source>Blue min</source>
        <translation>Niebieski min</translation>
    </message>
    <message>
        <source>Blue max</source>
        <translation>Niebieski max</translation>
    </message>
    <message>
        <source>Single band properties</source>
        <translation>Właściwości kanału</translation>
    </message>
    <message>
        <source>Gray band</source>
        <translation>Kanał </translation>
    </message>
    <message>
        <source>Use standard deviation</source>
        <translation>Użyj odchylenia standardowego</translation>
    </message>
    <message>
        <source>Restore Default Style</source>
        <translation>Przywróć domyślny styl</translation>
    </message>
    <message>
        <source>Save As Default</source>
        <translation>Zapisz jako domyślny</translation>
    </message>
    <message>
        <source>Load Style ...</source>
        <translation>Wczytaj styl ...</translation>
    </message>
    <message>
        <source>Save Style ...</source>
        <translation>Zapisz styl ...</translation>
    </message>
    <message>
        <source>Note:</source>
        <translation>Uwaga:</translation>
    </message>
    <message>
        <source>Default R:1 G:2 B:3</source>
        <translation>Domyślnie R:1 G:2 B:3</translation>
    </message>
    <message>
        <source>Add entry</source>
        <translation>Dodaj wpis</translation>
    </message>
    <message>
        <source>Sort</source>
        <translation>Sortuj</translation>
    </message>
    <message>
        <source>Load color map from band</source>
        <translation>Wczytaj mapę kolorów z kanału</translation>
    </message>
    <message>
        <source>Load color map from file</source>
        <translation>Wczytaj mapę kolorów z pliku</translation>
    </message>
    <message>
        <source>Export color map to file</source>
        <translation>Eksportuj mapę kolorów do pliku</translation>
    </message>
    <message>
        <source>Generate new color map</source>
        <translation>Generuj nową mapę kolorów</translation>
    </message>
    <message>
        <source>Coordinate reference system</source>
        <translation>System Odniesienia oparty na Współrzędnych</translation>
    </message>
    <message>
        <source>Change ...</source>
        <translation>Zmień ...</translation>
    </message>
    <message>
        <source>Legend</source>
        <translation>Legenda</translation>
    </message>
    <message>
        <source>Palette</source>
        <translation>Paleta</translation>
    </message>
    <message>
        <source>Notes</source>
        <translation>Uwagi</translation>
    </message>
    <message>
        <source>Build pyramids internally if possible</source>
        <translation>Zbuduj piramidę wewnętrznie jeśli to możliwe</translation>
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
        <translation>Nie można uruchomić komendy</translation>
    </message>
    <message>
        <source>Starting</source>
        <translation>Uruchamiam </translation>
    </message>
    <message>
        <source>Done</source>
        <translation>Zamknij</translation>
    </message>
    <message>
        <source>Action</source>
        <translation>Akcja</translation>
    </message>
</context>
<context>
    <name>QgsScaleBarPlugin</name>
    <message>
        <source> metres/km</source>
        <translation> metry/km</translation>
    </message>
    <message>
        <source> feet</source>
        <translation>stopy</translation>
    </message>
    <message>
        <source> degrees</source>
        <translation>stopnie</translation>
    </message>
    <message>
        <source> km</source>
        <translation>km</translation>
    </message>
    <message>
        <source> mm</source>
        <translation>mm</translation>
    </message>
    <message>
        <source> cm</source>
        <translation>cm</translation>
    </message>
    <message>
        <source> m</source>
        <translation>m</translation>
    </message>
    <message>
        <source> foot</source>
        <translation>stopa</translation>
    </message>
    <message>
        <source> degree</source>
        <translation>stopień</translation>
    </message>
    <message>
        <source> unknown</source>
        <translation>nieznany</translation>
    </message>
    <message>
        <source>Top Left</source>
        <translation>Lewy górny</translation>
    </message>
    <message>
        <source>Bottom Left</source>
        <translation>Lewy dolny</translation>
    </message>
    <message>
        <source>Top Right</source>
        <translation>Prawy górny</translation>
    </message>
    <message>
        <source>Bottom Right</source>
        <translation>Prawy dolny</translation>
    </message>
    <message>
        <source>Tick Down</source>
        <translation>Dolny</translation>
    </message>
    <message>
        <source>Tick Up</source>
        <translation>Górny</translation>
    </message>
    <message>
        <source>Bar</source>
        <translation>Pasek</translation>
    </message>
    <message>
        <source>Box</source>
        <translation>Prostokąt</translation>
    </message>
    <message>
        <source>&amp;Scale Bar</source>
        <translation>Podziałka</translation>
    </message>
    <message>
        <source>Creates a scale bar that is displayed on the map canvas</source>
        <translation>Tworzy podziałkę, która będzie widoczna w obszarze mapy</translation>
    </message>
    <message>
        <source>&amp;Decorations</source>
        <translation>&amp;Dekoracje</translation>
    </message>
    <message>
        <source> feet/miles</source>
        <translation>stopy/mile</translation>
    </message>
    <message>
        <source> miles</source>
        <translation>mile</translation>
    </message>
    <message>
        <source> mile</source>
        <translation>mila</translation>
    </message>
    <message>
        <source> inches</source>
        <translation>cale</translation>
    </message>
</context>
<context>
    <name>QgsScaleBarPluginGuiBase</name>
    <message>
        <source>Scale Bar Plugin</source>
        <translation>Podziałka</translation>
    </message>
    <message>
        <source>Top Left</source>
        <translation>Lewy górny</translation>
    </message>
    <message>
        <source>Top Right</source>
        <translation>Prawy górny</translation>
    </message>
    <message>
        <source>Bottom Left</source>
        <translation>Lewy dolny</translation>
    </message>
    <message>
        <source>Bottom Right</source>
        <translation>Prawy dolny</translation>
    </message>
    <message>
        <source>Size of bar:</source>
        <translation>Rozmiar paska:</translation>
    </message>
    <message>
        <source>Placement:</source>
        <translation>Umiejscowienie:</translation>
    </message>
    <message>
        <source>Tick Down</source>
        <translation>Dolny</translation>
    </message>
    <message>
        <source>Tick Up</source>
        <translation>Górny</translation>
    </message>
    <message>
        <source>Box</source>
        <translation>Prostokąt</translation>
    </message>
    <message>
        <source>Bar</source>
        <translation>Pasek</translation>
    </message>
    <message>
        <source>Select the style of the scale bar</source>
        <translation>Wybierz styl dla podziałki</translation>
    </message>
    <message>
        <source>Colour of bar:</source>
        <translation>Kolor podziałki:</translation>
    </message>
    <message>
        <source>Scale bar style:</source>
        <translation>Styl podziałki:</translation>
    </message>
    <message>
        <source>Enable scale bar</source>
        <translation>Włącz podziałkę</translation>
    </message>
    <message>
        <source>Automatically snap to round number on resize</source>
        <translation>Automatycznie zaokrąglaj przy zmianie wielkości okna</translation>
    </message>
    <message>
        <source>Click to select the colour</source>
        <translation>Kliknij by wybrać kolor</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;This plugin draws a scale bar on the map. Please note the size option below is a &apos;preferred&apos; size and may have to be altered by QGIS depending on the level of zoom.  The size is measured according to the map units specified in the project properties.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsSearchQueryBuilder</name>
    <message>
        <source>No matching features found.</source>
        <translation>Nie znaleziono pasujących obiektów.</translation>
    </message>
    <message>
        <source>Search results</source>
        <translation>Rezultaty wyszukiwania</translation>
    </message>
    <message>
        <source>Search string parsing error</source>
        <translation>Błąd parsowania tekstu wyszukiwania</translation>
    </message>
    <message>
        <source>No Records</source>
        <translation>Brak rekordów</translation>
    </message>
    <message>
        <source>The query you specified results in zero records being returned.</source>
        <translation>Podane zapytanie nie zwraca żadnych rekordów.</translation>
    </message>
    <message>
        <source>Search query builder</source>
        <translation>Kreator zapytań wyboru</translation>
    </message>
    <message numerus="yes">
        <source>Found %1 matching features.</source>
        <translation>
            <numerusform>Znaleziono %1 pasujący obiekt.</numerusform>
            <numerusform>Znaleziono %1 pasujące obiekty.</numerusform>
            <numerusform>Znaleziono %1 pasujących obiektów.</numerusform>
        </translation>
    </message>
</context>
<context>
    <name>QgsServerSourceSelect</name>
    <message>
        <source>Are you sure you want to remove the </source>
        <translation>Czy na pewno chcesz usunąć </translation>
    </message>
    <message>
        <source> connection and all associated settings?</source>
        <translation>połączenie i wszystkie związane z nim ustawienia?</translation>
    </message>
    <message>
        <source>Confirm Delete</source>
        <translation>Zatwierdź usunięcie</translation>
    </message>
    <message>
        <source>WMS Provider</source>
        <translation>Źródło danych WMS</translation>
    </message>
    <message>
        <source>Could not open the WMS Provider</source>
        <translation>Nie moge otworzyć źródła danych WMS</translation>
    </message>
    <message>
        <source>Select Layer</source>
        <translation>Wybierz warstwę</translation>
    </message>
    <message>
        <source>You must select at least one layer first.</source>
        <translation>Musisz najpierw wybrać co najmniej jedną warstwę.</translation>
    </message>
    <message>
        <source>Could not understand the response.  The</source>
        <translation>Niezrozumiała odpowiedź serwera.</translation>
    </message>
    <message>
        <source>provider said</source>
        <translation>źródło danych odpowiedziało</translation>
    </message>
    <message>
        <source>WMS proxies</source>
        <translation>MWS proxy</translation>
    </message>
    <message>
        <source>Coordinate Reference System</source>
        <translation>System Odniesienia oparty na Współrzędnych</translation>
    </message>
    <message>
        <source>There are no available coordinate reference system for the set of layers you&apos;ve selected.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Several WMS servers have been added to the server list. Note that if you access the internet via a web proxy, you will need to set the proxy settings in the QGIS options dialog.</source>
        <translation type="unfinished"></translation>
    </message>
    <message numerus="yes">
        <source>Coordinate Reference System (%1 available)</source>
        <translation>
            <numerusform>System Odniesienia oparty na Współrzędnych (%1 dostępny)</numerusform>
            <numerusform>System Odniesienia oparty na Współrzędnych (%1 dostępne)</numerusform>
            <numerusform>System Odniesienia oparty na Współrzędnych (%1 dostępnych)</numerusform>
        </translation>
    </message>
</context>
<context>
    <name>QgsServerSourceSelectBase</name>
    <message>
        <source>Add Layer(s) from a Server</source>
        <translation>Dodaj warstwę z serwera</translation>
    </message>
    <message>
        <source>C&amp;lose</source>
        <translation>Zamknij</translation>
    </message>
    <message>
        <source>Alt+L</source>
        <translation>Alt+L</translation>
    </message>
    <message>
        <source>Help</source>
        <translation>Pomoc</translation>
    </message>
    <message>
        <source>F1</source>
        <translation>F1</translation>
    </message>
    <message>
        <source>Image encoding</source>
        <translation>Kodowanie obrazu</translation>
    </message>
    <message>
        <source>Layers</source>
        <translation>Warstwy</translation>
    </message>
    <message>
        <source>ID</source>
        <translation>ID</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Nazwa</translation>
    </message>
    <message>
        <source>Title</source>
        <translation>Tytuł</translation>
    </message>
    <message>
        <source>Abstract</source>
        <translation>Abstrakt</translation>
    </message>
    <message>
        <source>&amp;Add</source>
        <translation>Dod&amp;aj</translation>
    </message>
    <message>
        <source>Alt+A</source>
        <translation>Alt+A</translation>
    </message>
    <message>
        <source>Server Connections</source>
        <translation>Połączenia z serwerem</translation>
    </message>
    <message>
        <source>&amp;New</source>
        <translation>&amp;Nowy</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>Usuń</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation>Edytuj</translation>
    </message>
    <message>
        <source>C&amp;onnect</source>
        <translation>P&amp;ołącz</translation>
    </message>
    <message>
        <source>Ready</source>
        <translation>Gotowe</translation>
    </message>
    <message>
        <source>Coordinate Reference System</source>
        <translation>System Odniesienia oparty na Współrzędnych</translation>
    </message>
    <message>
        <source>Change ...</source>
        <translation>Zmień ...</translation>
    </message>
    <message>
        <source>Adds a few example WMS servers</source>
        <translation>Dodaj kilka przykładowych serwerów WMS</translation>
    </message>
    <message>
        <source>Add default servers</source>
        <translation>Dodaj domyślne serwery</translation>
    </message>
</context>
<context>
    <name>QgsShapeFile</name>
    <message>
        <source>The database gave an error while executing this SQL:</source>
        <translation>Baza danych zwróciła komunikat o błędzie przy zapytaniu SQL:</translation>
    </message>
    <message>
        <source>The error was:</source>
        <translation>Błąd:</translation>
    </message>
    <message>
        <source>... (rest of SQL trimmed)</source>
        <comment>is appended to a truncated SQL statement</comment>
        <translation>... (resztę zapytania SQL pominięto)</translation>
    </message>
    <message>
        <source>Scanning </source>
        <translation>Przeszukuję</translation>
    </message>
</context>
<context>
    <name>QgsSingleSymbolDialog</name>
    <message>
        <source>Solid Line</source>
        <translation>Linia ciągła</translation>
    </message>
    <message>
        <source>Dash Line</source>
        <translation>Linia kreskowa</translation>
    </message>
    <message>
        <source>Dot Line</source>
        <translation>Linia kropkowa</translation>
    </message>
    <message>
        <source>Dash Dot Line</source>
        <translation>Linia kropka-kreska</translation>
    </message>
    <message>
        <source>Dash Dot Dot Line</source>
        <translation>Linia kropka-kreska-kropka</translation>
    </message>
    <message>
        <source>No Pen</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>No Brush</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Solid</source>
        <translation>Wypełniony</translation>
    </message>
    <message>
        <source>Horizontal</source>
        <translation>Poziomo</translation>
    </message>
    <message>
        <source>Vertical</source>
        <translation>Pionowo</translation>
    </message>
    <message>
        <source>Cross</source>
        <translation>Krzyż</translation>
    </message>
    <message>
        <source>BDiagonal</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>FDiagonal</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Diagonal X</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Dense1</source>
        <translation>Szrafura1</translation>
    </message>
    <message>
        <source>Dense2</source>
        <translation>Szrafura2</translation>
    </message>
    <message>
        <source>Dense3</source>
        <translation>Szrafura3</translation>
    </message>
    <message>
        <source>Dense4</source>
        <translation>Szrafura4</translation>
    </message>
    <message>
        <source>Dense5</source>
        <translation>Szrafura5</translation>
    </message>
    <message>
        <source>Dense6</source>
        <translation>Szrafura6</translation>
    </message>
    <message>
        <source>Dense7</source>
        <translation>Szrafura7</translation>
    </message>
    <message>
        <source>Texture</source>
        <translation>Tekstura</translation>
    </message>
</context>
<context>
    <name>QgsSingleSymbolDialogBase</name>
    <message>
        <source>Single Symbol</source>
        <translation>Symbol pojedynczy</translation>
    </message>
    <message>
        <source>Size</source>
        <translation>Rozmiar</translation>
    </message>
    <message>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <source>Point Symbol</source>
        <translation>Symbol punktu</translation>
    </message>
    <message>
        <source>Area scale field</source>
        <translation>Skaluj wielkość wg pola</translation>
    </message>
    <message>
        <source>Rotation field</source>
        <translation>Obrót wg pola</translation>
    </message>
    <message>
        <source>Style Options</source>
        <translation>Opcje stylu</translation>
    </message>
    <message>
        <source>Outline style</source>
        <translation>Styl obrysu</translation>
    </message>
    <message>
        <source>Outline color</source>
        <translation>Kolor obrysu</translation>
    </message>
    <message>
        <source>Outline width</source>
        <translation>Szerokośc obrysu</translation>
    </message>
    <message>
        <source>Fill color</source>
        <translation>Kolor wypełnienia</translation>
    </message>
    <message>
        <source>Fill style</source>
        <translation>Styl wypełnienia</translation>
    </message>
    <message>
        <source>Label</source>
        <translation>Etykieta</translation>
    </message>
</context>
<context>
    <name>QgsSnappingDialog</name>
    <message>
        <source>to vertex</source>
        <translation>do wierzchołka</translation>
    </message>
    <message>
        <source>to segment</source>
        <translation>do segmentu</translation>
    </message>
    <message>
        <source>to vertex and segment</source>
        <translation>do wierzchołka i segmentu</translation>
    </message>
</context>
<context>
    <name>QgsSnappingDialogBase</name>
    <message>
        <source>Snapping options</source>
        <translation>Opcje przyciągania</translation>
    </message>
    <message>
        <source>Layer</source>
        <translation>Warstwa</translation>
    </message>
    <message>
        <source>Mode</source>
        <translation>Tryb</translation>
    </message>
    <message>
        <source>Tolerance</source>
        <translation>Tolerancja</translation>
    </message>
</context>
<context>
    <name>QgsSpit</name>
    <message>
        <source>Are you sure you want to remove the [</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>] connection and all associated settings?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Confirm Delete</source>
        <translation>Zatwierdź usunięcie</translation>
    </message>
    <message>
        <source>The following Shapefile(s) could not be loaded:

</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>REASON: File cannot be opened</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>REASON: One or both of the Shapefile files (*.dbf, *.shx) missing</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>General Interface Help:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>PostgreSQL Connections:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>[New ...] - create a new connection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>[Edit ...] - edit the currently selected connection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>[Remove] - remove the currently selected connection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>-you need to select a connection that works (connects properly) in order to import files</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>-when changing connections Global Schema also changes accordingly</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Shapefile List:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>[Add ...] - open a File dialog and browse to the desired file(s) to import</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>[Remove] - remove the currently selected file(s) from the list</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>[Remove All] - remove all the files in the list</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>[SRID] - Reference ID for the shapefiles to be imported</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>[Use Default (SRID)] - set SRID to -1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>[Geometry Column Name] - name of the geometry column in the database</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>[Use Default (Geometry Column Name)] - set column name to &apos;the_geom&apos;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>[Glogal Schema] - set the schema for all files to be imported into</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>[Import] - import the current shapefiles in the list</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>[Quit] - quit the program
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>[Help] - display this help dialog</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Import Shapefiles</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>You need to specify a Connection first</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Connection failed - Check settings and try again</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>You need to add shapefiles to the list first</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Importing files</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Anuluj</translation>
    </message>
    <message>
        <source>Progress</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Problem inserting features from file:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Invalid table name.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>No fields detected.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The following fields are duplicates:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Import Shapefiles - Relation Exists</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The Shapefile:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>will use [</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>] relation for its data,</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>which already exists and possibly contains data.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>To avoid data loss change the &quot;DB Relation Name&quot;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>for this Shapefile in the main dialog file list.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Do you want to overwrite the [</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>] relation?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>File Name</source>
        <translation>Nazwa pliku</translation>
    </message>
    <message>
        <source>Feature Class</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Features</source>
        <translation>Obiekty</translation>
    </message>
    <message>
        <source>DB Relation Name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Schema</source>
        <translation>Schemat</translation>
    </message>
    <message>
        <source>Add Shapefiles</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Shapefiles (*.shp);;All files (*.*)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>PostGIS not available</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;p&gt;The chosen database does not have PostGIS installed, but this is required for storage of spatial data.&lt;/p&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;p&gt;Error while executing the SQL:&lt;/p&gt;&lt;p&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;/p&gt;&lt;p&gt;The database said:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>%1 of %2 shapefiles could not be imported.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Password for </source>
        <translation>Hasło dla</translation>
    </message>
    <message>
        <source>Please enter your password:</source>
        <translation>Proszę podać hasło:</translation>
    </message>
</context>
<context>
    <name>QgsSpitBase</name>
    <message>
        <source>SPIT - Shapefile to PostGIS Import Tool</source>
        <translation>SPIT - Narzędzie importu z pliku shape do PostGIS</translation>
    </message>
    <message>
        <source>Add</source>
        <translation>Dodaj</translation>
    </message>
    <message>
        <source>Add a shapefile to the list of files to be imported</source>
        <translation>Dodaj plik shapefile do listy importowanych plików</translation>
    </message>
    <message>
        <source>Remove</source>
        <translation>Usuń</translation>
    </message>
    <message>
        <source>Remove the selected shapefile from the import list</source>
        <translation>Usuń plik shapefile z listy importowanych plików</translation>
    </message>
    <message>
        <source>Remove All</source>
        <translation>Usuń wszystko</translation>
    </message>
    <message>
        <source>Remove all the shapefiles from the import list</source>
        <translation>Usuń wszystkie pliki shapefile z listy</translation>
    </message>
    <message>
        <source>Global Schema</source>
        <translation>Główny schemat</translation>
    </message>
    <message>
        <source>Set the SRID to the default value</source>
        <translation>Ustaw domyślną wartość dla SRID</translation>
    </message>
    <message>
        <source>Set the geometry column name to the default value</source>
        <translation>Ustaw domyślną wartość dla nazwy kolumny z geometrią</translation>
    </message>
    <message>
        <source>PostgreSQL Connections</source>
        <translation>Połączenia PostgreSQL</translation>
    </message>
    <message>
        <source>New</source>
        <translation>Nowy</translation>
    </message>
    <message>
        <source>Create a new PostGIS connection</source>
        <translation>Utwórz nowe połączenie z PostGIS</translation>
    </message>
    <message>
        <source>Remove the current PostGIS connection</source>
        <translation>Usuń bieżące połączenie z PostGIS</translation>
    </message>
    <message>
        <source>Connect</source>
        <translation>Połącz</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation>Edytuj</translation>
    </message>
    <message>
        <source>Edit the current PostGIS connection</source>
        <translation>Edytuj bieżące połączenie z PostGIS</translation>
    </message>
    <message>
        <source>Import options and shapefile list</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Use Default SRID or specify here</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Use Default Geometry Column Name or specify here</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Primary Key Column Name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Connect to PostGIS</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsSpitPlugin</name>
    <message>
        <source>&amp;Import Shapefiles to PostgreSQL</source>
        <translation>&amp;Importuj format shape do PostgreSQL</translation>
    </message>
    <message>
        <source>Import shapefiles into a PostGIS-enabled PostgreSQL database. The schema and field names can be customized on import</source>
        <translation>Importuj pliki shape do bazy PostgreSQL/PostGIS. Struktura oraz nazwy pól mogą być modyfikowane w trakcie importu</translation>
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
        <translation>Interpolacja liniowa</translation>
    </message>
</context>
<context>
    <name>QgsTINInterpolatorDialogBase</name>
    <message>
        <source>Triangle based interpolation</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:12pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;This interpolator provides different methods for interpolation in a triangular irregular network (TIN).&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Interpolation method:</source>
        <translation>Metoda interpolacji:</translation>
    </message>
</context>
<context>
    <name>QgsUniqueValueDialog</name>
    <message>
        <source>Confirm Delete</source>
        <translation>Zatwierdź usunięcie</translation>
    </message>
    <message>
        <source>The classification field was changed from &apos;%1&apos; to &apos;%2&apos;.
Should the existing classes be deleted before classification?</source>
        <translation>Pole klasyfikacji zostało zmienione z  &apos;%1&apos; na &apos;%2&apos;.
Czy aktualne klasy powiny zostać usunięte przed klasyfikacją?</translation>
    </message>
</context>
<context>
    <name>QgsUniqueValueDialogBase</name>
    <message>
        <source>Form1</source>
        <translation>Form1</translation>
    </message>
    <message>
        <source>Classify</source>
        <translation>Klasyfikuj</translation>
    </message>
    <message>
        <source>Classification field</source>
        <translation>Pole klasyfikacji</translation>
    </message>
    <message>
        <source>Add class</source>
        <translation>Dodaj klasę</translation>
    </message>
    <message>
        <source>Delete classes</source>
        <translation>Usuń klasy</translation>
    </message>
    <message>
        <source>Randomize Colors</source>
        <translation>Losowe kolory</translation>
    </message>
    <message>
        <source>Reset Colors</source>
        <translation>Zresetuj kolory</translation>
    </message>
</context>
<context>
    <name>QgsVectorLayer</name>
    <message>
        <source>ERROR: no provider</source>
        <translation>BŁĄD: brak źródła</translation>
    </message>
    <message>
        <source>ERROR: layer not editable</source>
        <translation>BŁĄD: warstwa nie edytowalna</translation>
    </message>
    <message>
        <source>SUCCESS: %1 attributes added.</source>
        <translation>SUKCES: dodano %1 atrybutów.</translation>
    </message>
    <message>
        <source>ERROR: %1 new attributes not added</source>
        <translation>BŁĄD: nie dodano %1 nowych atrybutów</translation>
    </message>
    <message>
        <source>SUCCESS: %1 attributes deleted.</source>
        <translation>SUKCES: usunięto %1 atrybutów.</translation>
    </message>
    <message>
        <source>ERROR: %1 attributes not deleted.</source>
        <translation>BŁĄD: nie usunięto %1 atrybutów.</translation>
    </message>
    <message>
        <source>SUCCESS: attribute %1 was added.</source>
        <translation>SUKCES: atrybut %1 został dodany.</translation>
    </message>
    <message>
        <source>ERROR: attribute %1 not added</source>
        <translation>BŁĄD: atrybut %1 nie został dodany</translation>
    </message>
    <message>
        <source>SUCCESS: %1 attribute values changed.</source>
        <translation>SUKCES: zmieniono wartości %1 atrybutów.</translation>
    </message>
    <message>
        <source>ERROR: %1 attribute value changes not applied.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>SUCCESS: %1 features added.</source>
        <translation>SUKCES: dodano %1 obiektów.</translation>
    </message>
    <message>
        <source>ERROR: %1 features not added.</source>
        <translation>BŁĄD: nie dodano %1 obiektów.</translation>
    </message>
    <message>
        <source>SUCCESS: %1 geometries were changed.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>ERROR: %1 geometries not changed.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>SUCCESS: %1 features deleted.</source>
        <translation>SUKCES: usunięto %1 obiektów.</translation>
    </message>
    <message>
        <source>ERROR: %1 features not deleted.</source>
        <translation>BŁĄD: nie usunięto %1 obiektów.</translation>
    </message>
    <message>
        <source>No renderer object</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Classification field not found</source>
        <translation>Nie znaleziono pola klasyfikacji</translation>
    </message>
</context>
<context>
    <name>QgsVectorLayerProperties</name>
    <message>
        <source>Transparency: </source>
        <translation>Przeźroczystość:</translation>
    </message>
    <message>
        <source>Single Symbol</source>
        <translation>Symbol pojedynczy</translation>
    </message>
    <message>
        <source>Graduated Symbol</source>
        <translation>Symbol stopniowy</translation>
    </message>
    <message>
        <source>Continuous Color</source>
        <translation>Kolor ciągły</translation>
    </message>
    <message>
        <source>Unique Value</source>
        <translation>Wartość unikalna</translation>
    </message>
    <message>
        <source>This button opens the PostgreSQL query builder and allows you to create a subset of features to display on the map canvas rather than displaying all features in the layer</source>
        <translation>Otwiera kreator zapytań PostgreSQL oraz pozwala utworzyć podzbiór obiektów do wyświetlenia na mapie, zamiast wyświetlania wszystkich obiektów</translation>
    </message>
    <message>
        <source>The query used to limit the features in the layer is shown here. This is currently only supported for PostgreSQL layers. To enter or modify the query, click on the Query Builder button</source>
        <translation>Zobacz zapytanie użyte do wyboru obiektów na warstwie. Aktualnie jest ono wspierane tylko przez warstwy PostgrSQL. Aby wprowadzić lub zmienić zapytanie, kliknij na Query Builder</translation>
    </message>
    <message>
        <source>Spatial Index</source>
        <translation>Indeks przestrzenny</translation>
    </message>
    <message>
        <source>Creation of spatial index failed</source>
        <translation>Tworzenie indeksu przestrzennego nie powiodło się</translation>
    </message>
    <message>
        <source>General:</source>
        <translation>Ogólne:</translation>
    </message>
    <message>
        <source>Storage type of this layer : </source>
        <translation>Format zapisu tej warstwy : </translation>
    </message>
    <message>
        <source>Source for this layer : </source>
        <translation>Źródło tej warstwy : </translation>
    </message>
    <message>
        <source>Geometry type of the features in this layer : </source>
        <translation>Rodzaj obiektów geometrycznych na tej warstwie : </translation>
    </message>
    <message>
        <source>The number of features in this layer : </source>
        <translation>Liczba obiektów na tej warstwie : </translation>
    </message>
    <message>
        <source>Editing capabilities of this layer : </source>
        <translation>Możliwości edycyjne tej warstwy : </translation>
    </message>
    <message>
        <source>Extents:</source>
        <translation>Zasięg: </translation>
    </message>
    <message>
        <source>In layer spatial reference system units : </source>
        <translation>W jednostkach  układu odniesienia warstwy : </translation>
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
        <translation>W jednostkach układu odniesienia projektu : </translation>
    </message>
    <message>
        <source>Layer Spatial Reference System:</source>
        <translation>Przestrzenny układ odniesienia warstwy:</translation>
    </message>
    <message>
        <source>Attribute field info:</source>
        <translation>Informacja o atrybutach pola:</translation>
    </message>
    <message>
        <source>Field</source>
        <translation>Pole</translation>
    </message>
    <message>
        <source>Type</source>
        <translation>Typ</translation>
    </message>
    <message>
        <source>Length</source>
        <translation>Długość</translation>
    </message>
    <message>
        <source>Precision</source>
        <translation>Dokładność</translation>
    </message>
    <message>
        <source>Layer comment: </source>
        <translation>Komentarz warstwy:</translation>
    </message>
    <message>
        <source>Comment</source>
        <translation>Komentarz</translation>
    </message>
    <message>
        <source>Default Style</source>
        <translation>Domyślny styl</translation>
    </message>
    <message>
        <source>QGIS Layer Style File (*.qml)</source>
        <translation>Plik stylu warstwy QGIS (*.qml)</translation>
    </message>
    <message>
        <source>QGIS</source>
        <translation>QGIS</translation>
    </message>
    <message>
        <source>Unknown style format: </source>
        <translation>Nieznany format stylu:</translation>
    </message>
    <message>
        <source>id</source>
        <translation>id</translation>
    </message>
    <message>
        <source>name</source>
        <translation>nazwa</translation>
    </message>
    <message>
        <source>type</source>
        <translation>typ</translation>
    </message>
    <message>
        <source>length</source>
        <translation>długość</translation>
    </message>
    <message>
        <source>precision</source>
        <translation>dokładność</translation>
    </message>
    <message>
        <source>comment</source>
        <translation>komentarz</translation>
    </message>
    <message>
        <source>edit widget</source>
        <translation>edytuj widżet</translation>
    </message>
    <message>
        <source>values</source>
        <translation>wartości</translation>
    </message>
    <message>
        <source>line edit</source>
        <translation>edycja wiersza</translation>
    </message>
    <message>
        <source>unique values</source>
        <translation>unikalne wartości</translation>
    </message>
    <message>
        <source>unique values (editable)</source>
        <translation>unikalne wartości (edytowalne)</translation>
    </message>
    <message>
        <source>value map</source>
        <translation>mapa wartości</translation>
    </message>
    <message>
        <source>classification</source>
        <translation>klasyfikacja</translation>
    </message>
    <message>
        <source>range (editable)</source>
        <translation>zakres (edytowalny)</translation>
    </message>
    <message>
        <source>range (slider)</source>
        <translation>zakres (suwak)</translation>
    </message>
    <message>
        <source>file name</source>
        <translation>nazwa pliku</translation>
    </message>
    <message>
        <source>Name conflict</source>
        <translation>Konflikt nazw</translation>
    </message>
    <message>
        <source>The attribute could not be inserted. The name already exists in the table.</source>
        <translation>Nie można dodać atrybutu. Atrybut o tej nazwie już istnieje w tabeli.</translation>
    </message>
    <message>
        <source>Creation of spatial index successful</source>
        <translation>Tworzenie indeksu przestrzennego zakończone pomyślnie</translation>
    </message>
    <message>
        <source>Saved Style</source>
        <translation>Zapisany styl</translation>
    </message>
</context>
<context>
    <name>QgsVectorLayerPropertiesBase</name>
    <message>
        <source>Layer Properties</source>
        <translation>Właściwości warstwy</translation>
    </message>
    <message>
        <source>Symbology</source>
        <translation>Symbolika</translation>
    </message>
    <message>
        <source>General</source>
        <translation>Ogólne</translation>
    </message>
    <message>
        <source>Use scale dependent rendering</source>
        <translation>Użyj rysowania zależnego od skali</translation>
    </message>
    <message>
        <source>Minimum scale at which this layer will be displayed. </source>
        <translation>Minimalna skala, dla której wartstwa będzie wyświetlana.</translation>
    </message>
    <message>
        <source>Maximum scale at which this layer will be displayed. </source>
        <translation>Maksymalna skala, dla której wartstwa będzie wyświetlana.</translation>
    </message>
    <message>
        <source>Display name</source>
        <translation>Nazwa wyświetlana</translation>
    </message>
    <message>
        <source>Use this control to set which field is placed at the top level of the Identify Results dialog box.</source>
        <translation>Użyj tego narzędzia aby ustawić pola wyświetlane na górze w oknie wyników identyfikacji.</translation>
    </message>
    <message>
        <source>Display field for the Identify Results dialog box</source>
        <translation>Wyświetlaj pole w oknie wyników identyfikacji</translation>
    </message>
    <message>
        <source>This sets the display field for the Identify Results dialog box</source>
        <translation>Ustawia wyświetlanie pola w oknie wyników identyfikacji</translation>
    </message>
    <message>
        <source>Display field</source>
        <translation>Wyświetlaj pole</translation>
    </message>
    <message>
        <source>Subset</source>
        <translation>Podzbiór</translation>
    </message>
    <message>
        <source>Query Builder</source>
        <translation>Kreator zapytań</translation>
    </message>
    <message>
        <source>Create Spatial Index</source>
        <translation>Twórz indeks przestrzenny</translation>
    </message>
    <message>
        <source>Metadata</source>
        <translation>Metadane</translation>
    </message>
    <message>
        <source>Labels</source>
        <translation>Etykiety</translation>
    </message>
    <message>
        <source>Display labels</source>
        <translation>Wyświetlaj etykiety</translation>
    </message>
    <message>
        <source>Actions</source>
        <translation>Akcje</translation>
    </message>
    <message>
        <source>Restore Default Style</source>
        <translation>Przywróć domyślny styl</translation>
    </message>
    <message>
        <source>Save As Default</source>
        <translation>Zapisz jako domyślny</translation>
    </message>
    <message>
        <source>Load Style ...</source>
        <translation>Wczytaj styl ...</translation>
    </message>
    <message>
        <source>Save Style ...</source>
        <translation>Zapisz styl ...</translation>
    </message>
    <message>
        <source>Legend type</source>
        <translation>Typ legendy</translation>
    </message>
    <message>
        <source>Transparency</source>
        <translation>Przeźroczystość</translation>
    </message>
    <message>
        <source>Options</source>
        <translation>Opcje</translation>
    </message>
    <message>
        <source>Maximum</source>
        <translation>Maksimum</translation>
    </message>
    <message>
        <source>Minimum</source>
        <translation>Minimum</translation>
    </message>
    <message>
        <source>Change CRS</source>
        <translation>Zmień SOW</translation>
    </message>
    <message>
        <source>Attributes</source>
        <translation>Atrybuty</translation>
    </message>
    <message>
        <source>New column</source>
        <translation>Nowa kolumna</translation>
    </message>
    <message>
        <source>Ctrl+N</source>
        <translation>Ctrl+N</translation>
    </message>
    <message>
        <source>Delete column</source>
        <translation>Usuń kolumnę</translation>
    </message>
    <message>
        <source>Ctrl+X</source>
        <translation>Ctrl+X</translation>
    </message>
    <message>
        <source>Toggle editing mode</source>
        <translation>Przełącz tryb edycji</translation>
    </message>
    <message>
        <source>Click to toggle table editing</source>
        <translation>Kliknij aby przełączyć edycję tabeli</translation>
    </message>
</context>
<context>
    <name>QgsWFSPlugin</name>
    <message>
        <source>&amp;Add WFS layer</source>
        <translation>Dod&amp;aj warstwę WFS</translation>
    </message>
</context>
<context>
    <name>QgsWFSProvider</name>
    <message>
        <source>unknown</source>
        <translation>nieznany</translation>
    </message>
    <message>
        <source>received %1 bytes from %2</source>
        <translation>odebrano %1 bajtów z %2</translation>
    </message>
</context>
<context>
    <name>QgsWFSSourceSelect</name>
    <message>
        <source>Are you sure you want to remove the </source>
        <translation>Czy na pewno chcesz usunąć </translation>
    </message>
    <message>
        <source> connection and all associated settings?</source>
        <translation>połączenie i wszystkie związane z nim ustawienia?</translation>
    </message>
    <message>
        <source>Confirm Delete</source>
        <translation>Zatwierdź usunięcie</translation>
    </message>
</context>
<context>
    <name>QgsWFSSourceSelectBase</name>
    <message>
        <source>Title</source>
        <translation>Tytuł</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Nazwa</translation>
    </message>
    <message>
        <source>Abstract</source>
        <translation>Abstrakt</translation>
    </message>
    <message>
        <source>Coordinate Reference System</source>
        <translation>System Odniesienia oparty na Współrzędnych</translation>
    </message>
    <message>
        <source>Change ...</source>
        <translation>Zmień ...</translation>
    </message>
    <message>
        <source>Server Connections</source>
        <translation>Połączenia z serwerem</translation>
    </message>
    <message>
        <source>&amp;New</source>
        <translation>&amp;Nowy</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>Usuń</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation>Edytuj</translation>
    </message>
    <message>
        <source>C&amp;onnect</source>
        <translation>P&amp;ołącz</translation>
    </message>
    <message>
        <source>Add WFS Layer from a Server</source>
        <translation>Dodaj warstwę WFS z serwera</translation>
    </message>
</context>
<context>
    <name>QgsWmsProvider</name>
    <message>
        <source>Tried URL: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>HTTP Exception</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>WMS Service Exception</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Could not get WMS capabilities: %1 at line %2 column %3</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>This is probably due to an incorrect WMS Server URL.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Could not get WMS capabilities in the expected format (DTD): no %1 or %2 found</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Could not get WMS Service Exception at %1: %2 at line %3 column %4</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Request contains a Format not offered by the server.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Request contains a CRS not offered by the server for one or more of the Layers in the request.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Request contains a SRS not offered by the server for one or more of the Layers in the request.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>GetMap request is for a Layer not offered by the server, or GetFeatureInfo request is for a Layer not shown on the map.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Request is for a Layer in a Style not offered by the server.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>GetFeatureInfo request is applied to a Layer which is not declared queryable.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>GetFeatureInfo request contains invalid X or Y value.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Value of (optional) UpdateSequence parameter in GetCapabilities request is equal to current value of service metadata update sequence number.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Value of (optional) UpdateSequence parameter in GetCapabilities request is greater than current value of service metadata update sequence number.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Request does not include a sample dimension value, and the server did not declare a default value for that dimension.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Request contains an invalid sample dimension value.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Request is for an optional operation that is not supported by the server.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>(Unknown error code from a post-1.3 WMS server)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The WMS vendor also reported: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Server Properties:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Property</source>
        <translation>Właściwość</translation>
    </message>
    <message>
        <source>Value</source>
        <translation>Wartość</translation>
    </message>
    <message>
        <source>WMS Version</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Title</source>
        <translation>Tytuł</translation>
    </message>
    <message>
        <source>Abstract</source>
        <translation type="unfinished">Abstrakt</translation>
    </message>
    <message>
        <source>Keywords</source>
        <translation>Słowa kluczowe</translation>
    </message>
    <message>
        <source>Online Resource</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Contact Person</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Fees</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Access Constraints</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Image Formats</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Identify Formats</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Layer Count</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Layer Properties: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Selected</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Yes</source>
        <translation>Tak</translation>
    </message>
    <message>
        <source>No</source>
        <translation>Nie</translation>
    </message>
    <message>
        <source>Visibility</source>
        <translation>Widoczność</translation>
    </message>
    <message>
        <source>Visible</source>
        <translation>Widoczny</translation>
    </message>
    <message>
        <source>Hidden</source>
        <translation>Ukryty</translation>
    </message>
    <message>
        <source>n/a</source>
        <translation>n/a</translation>
    </message>
    <message>
        <source>Can Identify</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Can be Transparent</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Can Zoom In</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cascade Count</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Fixed Width</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Fixed Height</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>WGS 84 Bounding Box</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Available in CRS</source>
        <translation>Dostępny w SOW</translation>
    </message>
    <message>
        <source>Available in style</source>
        <translation>Dostępny w stylu</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Nazwa</translation>
    </message>
    <message>
        <source>Layer cannot be queried.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Dom Exception</source>
        <translation type="unfinished"></translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Unknown format: </source>
        <translation>Nieznany format:</translation>
    </message>
</context>
<context>
    <name>QuickPrintGuiBase</name>
    <message>
        <source>QGIS Quick Print Plugin</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Quick Print</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Map Title e.g. ACME inc.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Map Name e.g. Water Features</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Copyright</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Output</source>
        <translation>Wyjście</translation>
    </message>
    <message>
        <source>Use last filename but incremented.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>last used filename but incremented will be shown here</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Prompt for file name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Note: If you want more control over the map layout please use the map composer function in QGIS.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Page Size</source>
        <translation>Rozmiar strony</translation>
    </message>
</context>
<context>
    <name>QuickPrintPlugin</name>
    <message>
        <source>Quick Print</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Quick Print</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Provides a way to quickly produce a map with minimal user input.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>[pluginname]GuiBase</name>
    <message>
        <source>QGIS Plugin Template</source>
        <translation>Szablon wtyczki QGIS</translation>
    </message>
    <message>
        <source>Plugin Template</source>
        <translation>Szablon wtyczki</translation>
    </message>
</context>
<context>
    <name>dxf2shpConverter</name>
    <message>
        <source>Converts DXF files in Shapefile format</source>
        <translation>Konwertuje plik DXF do formatu shape</translation>
    </message>
    <message>
        <source>&amp;Dxf2Shp</source>
        <translation>&amp;Dxf2Shp</translation>
    </message>
</context>
<context>
    <name>dxf2shpConverterGui</name>
    <message>
        <source>Dxf Importer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Input Dxf file</source>
        <translation type="unfinished"></translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Output file type</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Polyline</source>
        <translation>Poliliinia</translation>
    </message>
    <message>
        <source>Polygon</source>
        <translation>Poligon</translation>
    </message>
    <message>
        <source>Point</source>
        <translation>Punkt</translation>
    </message>
    <message>
        <source>Export text labels</source>
        <translation>Eksportuj etykiety tekstowe</translation>
    </message>
    <message>
        <source>Choose a file name to save to</source>
        <translation>Podaj nazwę pliku do którego należy zapisać</translation>
    </message>
    <message>
        <source>Choose a DXF file to open</source>
        <translation>Wybierz plik DXF do otwarcia</translation>
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
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>pluginname</name>
    <message>
        <source>[menuitemname]</source>
        <translation type="unfinished">[menuitemname]</translation>
    </message>
    <message>
        <source>&amp;[menuname]</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Replace this with a short description of what the plugin does</source>
        <translation type="unfinished"></translation>
    </message>
</context>
</TS>
