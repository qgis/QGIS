<!DOCTYPE TS><TS>
<context>
    <name>@default</name>
    <message>
        <source>OGR Driver Manager</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>unable to get OGRDriverManager</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>Dialog</name>
    <message>
        <source>QGIS Plugin Installer</source>
        <translation>Установщик модулей QGIS</translation>
    </message>
    <message>
        <source>Name of plugin to install</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Get List</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Done</source>
        <translation type="unfinished">Выполнено</translation>
    </message>
    <message>
        <source>Install Plugin</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The plugin will be installed to ~/.qgis/python/plugins</source>
        <translation>Модуль будет установлен в ~/.qgis/python/plugins</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Имя</translation>
    </message>
    <message>
        <source>Version</source>
        <translation>Версия</translation>
    </message>
    <message>
        <source>Description</source>
        <translation>Описание</translation>
    </message>
    <message>
        <source>Author</source>
        <translation>Автор</translation>
    </message>
    <message>
        <source>Select repository, retrieve the list of available plugins, select one and install it</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Repository</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Active repository:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Add</source>
        <translation type="unfinished">Добавить</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation type="unfinished">Правка</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation type="unfinished">Удалить</translation>
    </message>
</context>
<context>
    <name>Gui</name>
    <message>
        <source>Welcome to your automatically generated plugin!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>This is just a starting point. You now need to modify the code to make it do something useful....read on for a more information to get yourself started.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Documentation:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>You really need to read the QGIS API Documentation now at:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>In particular look at the following classes:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>QgsPlugin is an ABC that defines required behaviour your plugin must provide. See below for more details.</source>
        <translation type="unfinished"></translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>For Questions and Comments regarding the plugin builder template and creating your features in QGIS using the plugin interface please contact us via:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;li&gt; the QGIS developers mailing list, or &lt;/li&gt;&lt;li&gt; IRC (#qgis on freenode.net)&lt;/li&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>QGIS is distributed under the Gnu Public License. If you create a useful plugin please consider contributing it back to the community.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Have fun and thank you for choosing QGIS.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>MapCoordsDialogBase</name>
    <message>
        <source>Enter map coordinates</source>
        <translation>Введите координаты карты</translation>
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
        <translation>О&amp;тменить</translation>
    </message>
    <message>
        <source>Enter X and Y coordinates which correspond with the selected point on the image. Alternatively, click the button with icon of a pencil and then click a corresponding point on map canvas of QGIS to fill in coordinates of that point.</source>
        <translation>Введите XY-координаты, которые соответствуют выбранной точке на изображении. Либо нажмите на кнопке со значком карандаша и затем щёлкните на соответствующей точке в области вывода карты QGIS для автоматического ввода координат этой точки.</translation>
    </message>
    <message>
        <source> from map canvas</source>
        <translation> с карты</translation>
    </message>
</context>
<context>
    <name>QFileDialog</name>
    <message>
        <source>Save experiment report to portable document format (.pdf)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Load layer properties from style file (.qml)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Save layer properties as style file (.qml)</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <source>No Data Providers</source>
        <translation>Источники данных отсутствуют</translation>
    </message>
    <message>
        <source>No Data Provider Plugins</source>
        <comment>No QGIS data provider plugins found in:</comment>
        <translation>Отсутствуют модули источников данных</translation>
    </message>
    <message>
        <source>No vector layers can be loaded. Check your QGIS installation</source>
        <translation>Загрузка векторных слоёв невозможна. Проверьте вашу установку QGIS</translation>
    </message>
    <message>
        <source>No data provider plugins are available. No vector layers can be loaded</source>
        <translation>Недоступны модули источников данных. Загрузка векторных слоёв невозможна</translation>
    </message>
    <message>
        <source>Overwrite File?</source>
        <translation type="obsolete">Сохранить Поверх Файла?</translation>
    </message>
    <message>
        <source>%1 exists.%2Do you want to overwrite it?</source>
        <translation type="obsolete">%1 уже существует.%2 Сохранить поверх этого файла?</translation>
    </message>
    <message>
        <source>Yes</source>
        <translation type="obsolete">Да</translation>
    </message>
    <message>
        <source>No</source>
        <translation type="obsolete">Нет</translation>
    </message>
    <message>
        <source>QGis files (*.qgs)</source>
        <translation>Файлы QGIS (*.qgs)</translation>
    </message>
    <message>
        <source>Choose a file to open</source>
        <translation type="obsolete">Выберите файл для открытия</translation>
    </message>
    <message>
        <source>Choose a filename  to save</source>
        <translation type="obsolete">Выберите имя сохраняемого файла</translation>
    </message>
    <message>
        <source> at line </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> column </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> for file </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Unable to save to file </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Referenced column wasn&apos;t found: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Division by zero.</source>
        <translation>Деление на ноль.</translation>
    </message>
    <message>
        <source>No active layer</source>
        <translation>Нет активного слоя</translation>
    </message>
    <message>
        <source>Band</source>
        <translation>Канал</translation>
    </message>
    <message>
        <source>action</source>
        <translation>действие</translation>
    </message>
    <message>
        <source> features found</source>
        <translation> объектов найдено</translation>
    </message>
    <message>
        <source> 1 feature found</source>
        <translation> найден 1 объект</translation>
    </message>
    <message>
        <source>No features found</source>
        <translation>Объектов не найдено</translation>
    </message>
    <message>
        <source>No features were found in the active layer at the point you clicked</source>
        <translation>В активном слое не найдено объектов в точке, на которой был произведён щелчок</translation>
    </message>
    <message>
        <source>Could not identify objects on</source>
        <translation>Не удалось определить объекты на</translation>
    </message>
    <message>
        <source>because</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>New centroid</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>New point</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>New vertex</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Undo last point</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Close line</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Select vertex</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Select new position</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Select line segment</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>New vertex position</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Release</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Delete vertex</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Release vertex</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Select element</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>New location</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Release selected</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Delete selected / select next</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Select position on line</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Split the line</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Release the line</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Select point on line</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Label</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Length</source>
        <translation>Длина</translation>
    </message>
    <message>
        <source>Area</source>
        <translation>Площадь</translation>
    </message>
    <message>
        <source>Project file read error: </source>
        <translation>Ошибка чтения файла проекта: </translation>
    </message>
    <message>
        <source>Fit to a linear transform requires at least 2 points.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Fit to a Helmert transform requires at least 2 points.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Fit to an affine transform requires at least 4 points.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Couldn&apos;t open the data source: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Parse error at line </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>GPS eXchange format provider</source>
        <translation>Источник данных GPS eXchange</translation>
    </message>
    <message>
        <source>Caught a coordinate system exception while trying to transform a point. Unable to calculate line length.</source>
        <translation>Перехвачено исключение системы координат при попытке преобразования точки. Не удалось определить длину линии.</translation>
    </message>
    <message>
        <source>Caught a coordinate system exception while trying to transform a point. Unable to calculate polygon area.</source>
        <translation>Перехвачено исключение системы координат при попытке преобразования точки. Не удалось определить площадь полигона.</translation>
    </message>
    <message>
        <source>GRASS plugin</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>QGIS couldn&apos;t find your GRASS installation.
Would you like to specify path (GISBASE) to your GRASS installation?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Choose GRASS installation path (GISBASE)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>GRASS data won&apos;t be available if GISBASE is not specified.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>CopyrightLabel</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Draws copyright information</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Version 0.1</source>
        <translation>Версия 0.1</translation>
    </message>
    <message>
        <source>Version 0.2</source>
        <translation>Версия 0.2</translation>
    </message>
    <message>
        <source>Loads and displays delimited text files containing x,y coordinates</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Add Delimited Text Layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Georeferencer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Adding projection info to rasters</source>
        <translation>Добавление сведений о проекции к растрам</translation>
    </message>
    <message>
        <source>GPS Tools</source>
        <translation>GPS-инструменты</translation>
    </message>
    <message>
        <source>Tools for loading and importing GPS data</source>
        <translation>Инструменты для загрузки и импорта GPS-данных</translation>
    </message>
    <message>
        <source>GRASS</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>GRASS layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Graticule Creator</source>
        <translation>Построение сетки</translation>
    </message>
    <message>
        <source>Builds a graticule</source>
        <translation>Построение сетки</translation>
    </message>
    <message>
        <source>NorthArrow</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Displays a north arrow overlayed onto the map</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>[menuitemname]</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>[plugindescription]</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>ScaleBar</source>
        <translation>Масштабная линейка</translation>
    </message>
    <message>
        <source>Draws a scale bar</source>
        <translation>Накладывает масштабную линейку</translation>
    </message>
    <message>
        <source>SPIT</source>
        <translation></translation>
    </message>
    <message>
        <source>Shapefile to PostgreSQL/PostGIS Import Tool</source>
        <translation>Инструмент импорта shape-файлов в PostgreSQL/PostGIS</translation>
    </message>
    <message>
        <source>WFS plugin</source>
        <translation>Модуль WFS</translation>
    </message>
    <message>
        <source>Adds WFS layers to the QGIS canvas</source>
        <translation>Добавляет на карту WFS-слои</translation>
    </message>
    <message>
        <source>Version 0.0001</source>
        <translation type="obsolete">Версия 0.0001</translation>
    </message>
    <message>
        <source>Not a vector layer</source>
        <translation>Слой не является векторным</translation>
    </message>
    <message>
        <source>The current layer is not a vector layer</source>
        <translation>Текущий слой не является векторным</translation>
    </message>
    <message>
        <source>Layer cannot be added to</source>
        <translation>Слой не может быть добавлен в</translation>
    </message>
    <message>
        <source>The data provider for this layer does not support the addition of features.</source>
        <translation>Источник данных для этого слоя не поддерживает добавление объектов.</translation>
    </message>
    <message>
        <source>Layer not editable</source>
        <translation>Нередактируемый слой</translation>
    </message>
    <message>
        <source>Cannot edit the vector layer. To make it editable, go to the file item of the layer, right click and check &apos;Allow Editing&apos;.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>To select features, you must choose a vector layer by clicking on its name in the legend</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Python error</source>
        <translation>Ошибка Python</translation>
    </message>
    <message>
        <source>Couldn&apos;t load SIP module.
Python support will be disabled.</source>
        <translation>Не удалось загрузить модуль SIP.
Поддержка Python будет выключена.</translation>
    </message>
    <message>
        <source>Couldn&apos;t load PyQt bindings.
Python support will be disabled.</source>
        <translation>Не удалось загрузить библиотеки PyQt.
Поддержка Python будет выключена.</translation>
    </message>
    <message>
        <source>Couldn&apos;t load QGIS bindings.
Python support will be disabled.</source>
        <translation>Не удалось загрузить привязки QGIS.
Поддержка Python будет выключена.</translation>
    </message>
    <message>
        <source>Couldn&apos;t load plugin </source>
        <translation>Не удалось загрузить модуль </translation>
    </message>
    <message>
        <source> due an error when calling its classFactory() method</source>
        <translation> из-за ошибки вызова его метода classFactory()</translation>
    </message>
    <message>
        <source> due an error when calling its initGui() method</source>
        <translation> из-за ошибки вызова его метода initGui()</translation>
    </message>
    <message>
        <source>Error while unloading plugin </source>
        <translation>Ошибка при выгрузке модуля </translation>
    </message>
    <message>
        <source>2.5D shape type not supported</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Adding features to 2.5D shapetypes is not supported yet</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Wrong editing tool</source>
        <translation>Неверный инструмент редактирования</translation>
    </message>
    <message>
        <source>Cannot apply the &apos;capture point&apos; tool on this vector layer</source>
        <translation>Не удалось применить инструмент «захватить точку» на этот векторный слой </translation>
    </message>
    <message>
        <source>Coordinate transform error</source>
        <translation>Ошибка преобразования координат</translation>
    </message>
    <message>
        <source>Cannot transform the point to the layers coordinate system</source>
        <translation>Не удалось преобразовать точку в систему координат слоя</translation>
    </message>
    <message>
        <source>Cannot apply the &apos;capture line&apos; tool on this vector layer</source>
        <translation>Не удалось применить инструмент «захватить линию» на этот векторный слой </translation>
    </message>
    <message>
        <source>Cannot apply the &apos;capture polygon&apos; tool on this vector layer</source>
        <translation>Не удалось применить инструмент «захватить полигон» на этот векторный слой </translation>
    </message>
    <message>
        <source>Error</source>
        <translation>Ошибка</translation>
    </message>
    <message>
        <source>Cannot add feature. Unknown WKB type</source>
        <translation>Не удалось добавить объект, неизвестный тип WKB</translation>
    </message>
    <message>
        <source>Error, could not add island</source>
        <translation>Ошибка, не удалось добавить остров</translation>
    </message>
    <message>
        <source>A problem with geometry type occured</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The inserted Ring is not closed</source>
        <translation>Вставляемое кольцо не замкнуто</translation>
    </message>
    <message>
        <source>The inserted Ring is not a valid geometry</source>
        <translation>Вставляемое кольцо не является действительной геометрией</translation>
    </message>
    <message>
        <source>The inserted Ring crosses existing rings</source>
        <translation>Вставляемое кольцо пересекает существующие кольца</translation>
    </message>
    <message>
        <source>The inserted Ring is not contained in a feature</source>
        <translation>Вставляемое кольцо располагается вне границ объекта</translation>
    </message>
    <message>
        <source>An unknown error occured</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Error, could not add ring</source>
        <translation>Ошибка, не удалось добавить кольцо</translation>
    </message>
    <message>
        <source>Data provider of the current layer doesn&apos;t allow changing geometries</source>
        <translation type="obsolete">Источник данных текущего слоя не позволяет изменять геометрию</translation>
    </message>
    <message>
        <source> km2</source>
        <translation> км2</translation>
    </message>
    <message>
        <source> ha</source>
        <translation> га</translation>
    </message>
    <message>
        <source> m2</source>
        <translation> м2</translation>
    </message>
    <message>
        <source> m</source>
        <translation> м</translation>
    </message>
    <message>
        <source> km</source>
        <translation> км</translation>
    </message>
    <message>
        <source> mm</source>
        <translation> мм</translation>
    </message>
    <message>
        <source> cm</source>
        <translation> см</translation>
    </message>
    <message>
        <source> sq mile</source>
        <translation> кв. миль</translation>
    </message>
    <message>
        <source> sq ft</source>
        <translation> кв. футов</translation>
    </message>
    <message>
        <source> mile</source>
        <translation> миль</translation>
    </message>
    <message>
        <source> foot</source>
        <translation> фут</translation>
    </message>
    <message>
        <source> feet</source>
        <translation> футов</translation>
    </message>
    <message>
        <source> sq.deg.</source>
        <translation> кв. град.</translation>
    </message>
    <message>
        <source> degree</source>
        <translation> градус</translation>
    </message>
    <message>
        <source> degrees</source>
        <translation> градусов</translation>
    </message>
    <message>
        <source> unknown</source>
        <translation> неизв</translation>
    </message>
    <message>
        <source>Received %1 of %2 bytes</source>
        <translation>Получено %1 из %2 байт</translation>
    </message>
    <message>
        <source>Received %1 bytes (total unknown)</source>
        <translation>Получено %1 байт (размер неизвестен)</translation>
    </message>
    <message>
        <source>Not connected</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Looking up &apos;%1&apos;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Connecting to &apos;%1&apos;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Sending request &apos;%1&apos;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Receiving reply</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Response is complete</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Closing down connection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Unable to open </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Regular expressions on numeric values don&apos;t make sense. Use comparison instead.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Geoprocessing functions for working with PostgreSQL/PostGIS layers</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Location: </source>
        <comment>Metadata in GRASS Browser</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;br&gt;Mapset: </source>
        <comment>Metadata in GRASS Browser</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Location: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;br&gt;Mapset: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;b&gt;Raster&lt;/b&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot open raster header</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Rows</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Columns</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>N-S resolution</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>E-W resolution</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>North</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>South</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>East</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>West</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Format</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Minimum value</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Maximum value</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Data source</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Data description</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Comments</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Categories</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;b&gt;Vector&lt;/b&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Points</source>
        <translation type="unfinished">Пункты</translation>
    </message>
    <message>
        <source>Lines</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Boundaries</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Centroids</source>
        <translation type="unfinished"></translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Islands</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Top</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Bottom</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>yes</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>no</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>History&lt;br&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;b&gt;Layer&lt;/b&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Features</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Driver</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Database</source>
        <translation type="unfinished">База данных</translation>
    </message>
    <message>
        <source>Table</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Key column</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>GISBASE is not set.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> is not a GRASS mapset.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot start </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Mapset is already in use.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Temporary directory </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> exist but is not writable</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot create temporary directory </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot create </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot remove mapset lock: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Внимание</translation>
    </message>
    <message>
        <source>Cannot read raster map region</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot read vector map region</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot read region</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Where is &apos;</source>
        <translation>Где искать &apos;</translation>
    </message>
    <message>
        <source>original location: </source>
        <translation>оригинальное местоположение: </translation>
    </message>
    <message>
        <source>To identify features, you must choose an active layer by clicking on its name in the legend</source>
        <translation>Чтобы определить объекты, следует выбрать активный слой щелчком на имени слоя в легенде</translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Currently only filebased datasets are supported</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Loaded default style file from </source>
        <translation type="unfinished"></translation>
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
        <source>ERROR: Failed to created default style file as </source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgisApp</name>
    <message>
        <source>Layers</source>
        <translation type="obsolete">Слои</translation>
    </message>
    <message>
        <source>Quantum GIS - </source>
        <translation>Quantum GIS - </translation>
    </message>
    <message>
        <source>&amp;Zoom to extent of selected layer</source>
        <translation type="obsolete">&amp;Изменить вид до полной данного слоя</translation>
    </message>
    <message>
        <source>&amp;Open attribute table</source>
        <translation type="obsolete">&amp;Открыть таблицу атрибутов</translation>
    </message>
    <message>
        <source>&amp;Properties</source>
        <translation type="obsolete">&amp;Свойства</translation>
    </message>
    <message>
        <source>&amp;Remove</source>
        <translation type="obsolete">&amp;Удалить</translation>
    </message>
    <message>
        <source>Version </source>
        <translation>Версия</translation>
    </message>
    <message>
        <source> with PostgreSQL support</source>
        <translation> с поддержкой PostgreSQL</translation>
    </message>
    <message>
        <source> (no PostgreSQL support)</source>
        <translation> (без поддержки PostgreSQL)</translation>
    </message>
    <message>
        <source>Web Page: http://qgis.sourceforge.net</source>
        <translation type="obsolete">Веб Страничка: http://qgis.sourceforge.net</translation>
    </message>
    <message>
        <source>Sourceforge Project Page: http://sourceforge.net/projects/qgis</source>
        <translation type="obsolete">Веб Страничка Проекта: http://sourceforge.net/projects/qgis</translation>
    </message>
    <message>
        <source>Version</source>
        <translation>Версия</translation>
    </message>
    <message>
        <source>Available Data Provider Plugins</source>
        <translation>Доступные модули источников данных</translation>
    </message>
    <message>
        <source>Shapefiles (*.shp);;All files (*.*)</source>
        <translation type="obsolete">Shapefiles файлы (*.shp);;Все файлы (*.*)</translation>
    </message>
    <message>
        <source>Select one or more layers to add</source>
        <translation type="obsolete">Выберите один или более слоёв для добавления</translation>
    </message>
    <message>
        <source>is not a valid or recognized data source</source>
        <translation>не является действительным источником данных</translation>
    </message>
    <message>
        <source>Invalid Data Source</source>
        <translation>Недопустимый источник данных</translation>
    </message>
    <message>
        <source>No OGR Provider</source>
        <translation type="obsolete">Нет Поставщика OGR</translation>
    </message>
    <message>
        <source>No OGR data provider was found in the QGIS lib directory</source>
        <translation type="obsolete">Поставщика данных OGR в QGIS lib не найдено</translation>
    </message>
    <message>
        <source>No PostgreSQL Provider</source>
        <translation type="obsolete">Нет Поставщика PostgreSQL</translation>
    </message>
    <message>
        <source>No PostgreSQL data provider was found in the QGIS lib directory</source>
        <translation type="obsolete">Поставщика данных PostgreSQL в QGIS lib не найдено</translation>
    </message>
    <message>
        <source>Quantum GIS -- Untitled</source>
        <translation type="obsolete">Quantum GIS -- Без имени</translation>
    </message>
    <message>
        <source>Quantum GIS --</source>
        <translation type="obsolete">Quantum GIS --</translation>
    </message>
    <message>
        <source>Saved map to:</source>
        <translation type="obsolete">Карта сохранена в:</translation>
    </message>
    <message>
        <source>No Layer Selected</source>
        <translation>Слой не выбран</translation>
    </message>
    <message>
        <source>To open an attribute table, you must select a layer in the legend</source>
        <translation type="obsolete">Чтобы открыть таблицу атрибутов, выберите слой в легенде</translation>
    </message>
    <message>
        <source>No MapLayer Plugins</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>No MapLayer plugins in ../plugins/maplayer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>No Plugins</source>
        <translation type="unfinished">Модулей не найдено</translation>
    </message>
    <message>
        <source>No plugins found in ../plugins. To test plugins, start qgis from the src directory</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Name</source>
        <translation type="unfinished">Имя</translation>
    </message>
    <message>
        <source>Plugin %1 is named %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Plugin Information</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>QGis loaded the following plugin:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Name: %1</source>
        <translation>Имя: %1</translation>
    </message>
    <message>
        <source>Version: %1</source>
        <translation>Версия: %1</translation>
    </message>
    <message>
        <source>Description: %1</source>
        <translation>Описание: %1</translation>
    </message>
    <message>
        <source>Unable to Load Plugin</source>
        <translation>Не удалось загрузить модуль</translation>
    </message>
    <message>
        <source>QGIS was unable to load the plugin from: %1</source>
        <translation>QGIS не удалось загрузить модуль из: %1</translation>
    </message>
    <message>
        <source>There is a new version of QGIS available</source>
        <translation>Доступна новая версия QGIS</translation>
    </message>
    <message>
        <source>You are running a development version of QGIS</source>
        <translation>Вы используете разрабатываемую версию QGIS</translation>
    </message>
    <message>
        <source>You are running the current version of QGIS</source>
        <translation>Вы используете последнюю версию QGIS</translation>
    </message>
    <message>
        <source>Would you like more information?</source>
        <translation>Вы хотите получить дополнительную информацию?</translation>
    </message>
    <message>
        <source>QGIS Version Information</source>
        <translation>Информация о версии QGIS</translation>
    </message>
    <message>
        <source>Yes</source>
        <translation type="obsolete">Да</translation>
    </message>
    <message>
        <source>No</source>
        <translation type="obsolete">Нет</translation>
    </message>
    <message>
        <source>QGIS - Changes in CVS</source>
        <translation type="obsolete">QGIS - Изменения в CVS</translation>
    </message>
    <message>
        <source>Unable to get current version information from server</source>
        <translation>Не удалось получить информацию о версии с сервера</translation>
    </message>
    <message>
        <source>Connection refused - server may be down</source>
        <translation>В соединении отказано ‒ вероятно, сервер недоступен</translation>
    </message>
    <message>
        <source>QGIS server was not found</source>
        <translation>Сервер QGIS не найден</translation>
    </message>
    <message>
        <source>Error reading from server</source>
        <translation type="obsolete">Ошибка чтения с сервера</translation>
    </message>
    <message>
        <source>Unable to connect to the QGIS Version server</source>
        <translation type="obsolete">Контакт с QGIS сервером версий невозможен</translation>
    </message>
    <message>
        <source>Invalid Layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>%1 is an invalid layer and cannot be loaded.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Error Loading Plugin</source>
        <translation>Ошибка загрузки модуля</translation>
    </message>
    <message>
        <source>There was an error loading %1.</source>
        <translation>При загрузке модуля %1 возникла ошибка.</translation>
    </message>
    <message>
        <source>Saved map image to</source>
        <translation>Сохранить снимок карты в</translation>
    </message>
    <message>
        <source>Choose a filename to save the map image as</source>
        <translation>Выберите имя файла для сохранения снимка карты</translation>
    </message>
    <message>
        <source>Extents: </source>
        <translation>Границы: </translation>
    </message>
    <message>
        <source>Problem deleting features</source>
        <translation>Ошибка удаления объектов</translation>
    </message>
    <message>
        <source>A problem occured during deletion of features</source>
        <translation>При удалении объектов возникла ошибка</translation>
    </message>
    <message>
        <source>No Vector Layer Selected</source>
        <translation>Не выбран векторный слой</translation>
    </message>
    <message>
        <source>Deleting features only works on vector layers</source>
        <translation>Удаление объектов работает только для векторных слоёв</translation>
    </message>
    <message>
        <source>To delete features, you must select a vector layer in the legend</source>
        <translation>Для удаления объектов, следует выбрать в легенде векторный слой</translation>
    </message>
    <message>
        <source>Quantum GIS is licensed under the GNU General Public License</source>
        <translation>Quantum GIS выпущена под Стандартной Общественной Лицензией GNU</translation>
    </message>
    <message>
        <source>http://www.gnu.org/licenses</source>
        <translation>http://www.gnu.org/licenses</translation>
    </message>
    <message>
        <source>Map legend that displays all the layers currently on the map canvas. Click on the check box to turn a layer on or off. Double click on a layer in the legend to customize its appearance and set other properties.</source>
        <translation>Легенда карты, в которой перечислены все слои отображаемой карты. Щёлкните на флажке, чтобы переключить видимость соответствующего слоя. Дважды щёлкните на имени слоя, чтобы задать его отображение и другие свойства.</translation>
    </message>
    <message>
        <source>Map overview canvas. This canvas can be used to display a locator map that shows the current extent of the map canvas. The current extent is shown as a red rectangle. Any layer on the map can be added to the overview canvas.</source>
        <translation>Область обзора карты. Данная область используется для вывода обзорной карты, на которой виден текущий экстент области вывода карты. Текущий экстент нарисован в виде красного прямоугольника. Любой слой карты может быть добавлен в обзорную область.</translation>
    </message>
    <message>
        <source>Map canvas. This is where raster and vector layers are displayed when added to the map</source>
        <translation>Область карты. В этом месте отображаются добавленные на карту растровые и векторные слои</translation>
    </message>
    <message>
        <source>&amp;Plugins</source>
        <translation>&amp;Модули</translation>
    </message>
    <message>
        <source>Progress bar that displays the status of rendering layers and other time-intensive operations</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Displays the current map scale</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Render</source>
        <translation>Отрисовка</translation>
    </message>
    <message>
        <source>When checked, the map layers are rendered in response to map navigation commands and other events. When not checked, no rendering is done. This allows you to add a large number of layers and symbolize them before rendering.</source>
        <translation>Если включено, отрисовка слоёв карты выполняется сразу в ответ на команды навигации и другие события. Если выключено, отрисовка не выполняется. К примеру, это позволяет добавить большое количество слоёв и назначить им условные обозначения до их отображения.</translation>
    </message>
    <message>
        <source>QGis files (*.qgs)</source>
        <translation type="obsolete">QGIS файлы (*.qgs)</translation>
    </message>
    <message>
        <source>Choose a QGIS project file</source>
        <translation>Выберите файл проекта QGIS</translation>
    </message>
    <message>
        <source>Unable to save project</source>
        <translation>Не удалось сохранить проект</translation>
    </message>
    <message>
        <source>Unable to save project to </source>
        <translation>Не удалось сохранить проект в </translation>
    </message>
    <message>
        <source>Toggle map rendering</source>
        <translation>Переключить отрисовку карты</translation>
    </message>
    <message>
        <source>This icon shows whether on the fly projection is enabled or not. Click the icon to bring up the project properties dialog to alter this behaviour.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Projection status - Click to open projection dialog</source>
        <translation>Статус проекции ‒ щёлкните для открытия диалога проекции</translation>
    </message>
    <message>
        <source>Open an OGR Supported Vector Layer</source>
        <translation>Открыть OGR-совместимый векторный слой</translation>
    </message>
    <message>
        <source>Save As</source>
        <translation>Сохранить как</translation>
    </message>
    <message>
        <source>Choose a QGIS project file to open</source>
        <translation>Выберите открываемый файл проекта QGIS</translation>
    </message>
    <message>
        <source>QGIS Project Read Error</source>
        <translation>Ошибка чтения проекта QGIS</translation>
    </message>
    <message>
        <source>Try to find missing layers?</source>
        <translation>Попытаться найти недостающие слои?</translation>
    </message>
    <message>
        <source>Saved project to:</source>
        <translation>Проект сохранён в:</translation>
    </message>
    <message>
        <source>QGIS Browser Selection</source>
        <translation>Выбор браузера QGIS</translation>
    </message>
    <message>
        <source>Enter the name of a web browser to use (eg. konqueror).
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Enter the full path if the browser is not in your PATH.
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Open a GDAL Supported Raster Data Source</source>
        <translation>Открыть GDAL-совместимый источник растровых данных</translation>
    </message>
    <message>
        <source>Reading settings</source>
        <translation>Загрузка параметров</translation>
    </message>
    <message>
        <source>Setting up the GUI</source>
        <translation>Настройка пользовательского интерфейса</translation>
    </message>
    <message>
        <source>Checking database</source>
        <translation>Проверка базы данных</translation>
    </message>
    <message>
        <source>Restoring loaded plugins</source>
        <translation>Восстановление загруженных модулей</translation>
    </message>
    <message>
        <source>Initializing file filters</source>
        <translation>Инициализация файловых фильтров</translation>
    </message>
    <message>
        <source>Restoring window state</source>
        <translation>Восстановление состояния окна</translation>
    </message>
    <message>
        <source>QGIS Ready!</source>
        <translation>QGIS Готова!</translation>
    </message>
    <message>
        <source>&amp;New Project</source>
        <translation>&amp;Новый проект</translation>
    </message>
    <message>
        <source>Ctrl+N</source>
        <comment>New Project</comment>
        <translation></translation>
    </message>
    <message>
        <source>New Project</source>
        <translation>Новый проект</translation>
    </message>
    <message>
        <source>&amp;Open Project...</source>
        <translation>&amp;Открыть проект...</translation>
    </message>
    <message>
        <source>Ctrl+O</source>
        <comment>Open a Project</comment>
        <translation></translation>
    </message>
    <message>
        <source>Open a Project</source>
        <translation>Открыть Проект</translation>
    </message>
    <message>
        <source>&amp;Save Project</source>
        <translation>&amp;Сохранить проект</translation>
    </message>
    <message>
        <source>Ctrl+S</source>
        <comment>Save Project</comment>
        <translation></translation>
    </message>
    <message>
        <source>Save Project</source>
        <translation>Сохранить проект</translation>
    </message>
    <message>
        <source>Save Project &amp;As...</source>
        <translation>Сохранить проект &amp;как...</translation>
    </message>
    <message>
        <source>Ctrl+A</source>
        <comment>Save Project under a new name</comment>
        <translation></translation>
    </message>
    <message>
        <source>Save Project under a new name</source>
        <translation>Сохранить проект под другим именем</translation>
    </message>
    <message>
        <source>&amp;Print...</source>
        <translation>&amp;Печать...</translation>
    </message>
    <message>
        <source>Ctrl+P</source>
        <comment>Print</comment>
        <translation></translation>
    </message>
    <message>
        <source>Print</source>
        <translation>Печать</translation>
    </message>
    <message>
        <source>Save as Image...</source>
        <translation>Сохранить как изображение...</translation>
    </message>
    <message>
        <source>Ctrl+I</source>
        <comment>Save map as image</comment>
        <translation></translation>
    </message>
    <message>
        <source>Save map as image</source>
        <translation>Сохранить карту как изображение</translation>
    </message>
    <message>
        <source>Export to MapServer Map...</source>
        <translation type="obsolete">Экспорт в карту MapServer...</translation>
    </message>
    <message>
        <source>Export as MapServer .map file</source>
        <translation type="obsolete">Экспорт в формат MapServer .map</translation>
    </message>
    <message>
        <source>Exit</source>
        <translation>Выйти</translation>
    </message>
    <message>
        <source>Ctrl+Q</source>
        <comment>Exit QGIS</comment>
        <translation></translation>
    </message>
    <message>
        <source>Exit QGIS</source>
        <translation>Выйти из QGIS</translation>
    </message>
    <message>
        <source>Add a Vector Layer...</source>
        <translation>Добавить векторный слой...</translation>
    </message>
    <message>
        <source>V</source>
        <comment>Add a Vector Layer</comment>
        <translation></translation>
    </message>
    <message>
        <source>Add a Vector Layer</source>
        <translation>Добавить векторный слой</translation>
    </message>
    <message>
        <source>Add a Raster Layer...</source>
        <translation>Добавить растровый слой...</translation>
    </message>
    <message>
        <source>R</source>
        <comment>Add a Raster Layer</comment>
        <translation></translation>
    </message>
    <message>
        <source>Add a Raster Layer</source>
        <translation>Добавить растровый слой</translation>
    </message>
    <message>
        <source>Add a PostGIS Layer...</source>
        <translation>Добавить слой PostGIS...</translation>
    </message>
    <message>
        <source>D</source>
        <comment>Add a PostGIS Layer</comment>
        <translation></translation>
    </message>
    <message>
        <source>Add a PostGIS Layer</source>
        <translation>Добавить слой PostGIS</translation>
    </message>
    <message>
        <source>New Vector Layer...</source>
        <translation>Новый векторный слой...</translation>
    </message>
    <message>
        <source>N</source>
        <comment>Create a New Vector Layer</comment>
        <translation></translation>
    </message>
    <message>
        <source>Create a New Vector Layer</source>
        <translation>Создать новый векторный слой</translation>
    </message>
    <message>
        <source>Remove Layer</source>
        <translation>Удалить слой</translation>
    </message>
    <message>
        <source>Ctrl+D</source>
        <comment>Remove a Layer</comment>
        <translation></translation>
    </message>
    <message>
        <source>Remove a Layer</source>
        <translation>Удалить слой</translation>
    </message>
    <message>
        <source>Add All To Overview</source>
        <translation>Добавить все в обзор</translation>
    </message>
    <message>
        <source>+</source>
        <comment>Show all layers in the overview map</comment>
        <translation></translation>
    </message>
    <message>
        <source>Show all layers in the overview map</source>
        <translation>Показать все слои на обзорной карте</translation>
    </message>
    <message>
        <source>Remove All From Overview</source>
        <translation>Удалить все из обзора</translation>
    </message>
    <message>
        <source>-</source>
        <comment>Remove all layers from overview map</comment>
        <translation></translation>
    </message>
    <message>
        <source>Remove all layers from overview map</source>
        <translation>Удалить все слои с обзорной карты</translation>
    </message>
    <message>
        <source>Show All Layers</source>
        <translation>Показать все слои</translation>
    </message>
    <message>
        <source>S</source>
        <comment>Show all layers</comment>
        <translation></translation>
    </message>
    <message>
        <source>Show all layers</source>
        <translation>Показать все слои</translation>
    </message>
    <message>
        <source>Hide All Layers</source>
        <translation>Скрыть все слои</translation>
    </message>
    <message>
        <source>H</source>
        <comment>Hide all layers</comment>
        <translation></translation>
    </message>
    <message>
        <source>Hide all layers</source>
        <translation>Скрыть все слои</translation>
    </message>
    <message>
        <source>Project Properties...</source>
        <translation>Свойства проекта...</translation>
    </message>
    <message>
        <source>P</source>
        <comment>Set project properties</comment>
        <translation></translation>
    </message>
    <message>
        <source>Set project properties</source>
        <translation>Задать параметры проекта</translation>
    </message>
    <message>
        <source>Options...</source>
        <translation>Настройки...</translation>
    </message>
    <message>
        <source>Change various QGIS options</source>
        <translation>Изменить настройки QGIS</translation>
    </message>
    <message>
        <source>Custom Projection...</source>
        <translation>Пользовательская проекция...</translation>
    </message>
    <message>
        <source>Manage custom projections</source>
        <translation>Управление пользовательскими проекциями</translation>
    </message>
    <message>
        <source>Help Contents</source>
        <translation>Содержание</translation>
    </message>
    <message>
        <source>F1</source>
        <comment>Help Documentation</comment>
        <translation></translation>
    </message>
    <message>
        <source>Help Documentation</source>
        <translation>Открыть руководство по программе</translation>
    </message>
    <message>
        <source>Qgis Home Page</source>
        <translation>Веб-сайт Qgis</translation>
    </message>
    <message>
        <source>Ctrl+H</source>
        <comment>QGIS Home Page</comment>
        <translation></translation>
    </message>
    <message>
        <source>QGIS Home Page</source>
        <translation>Веб-сайт QGIS</translation>
    </message>
    <message>
        <source>About</source>
        <translation>О программе</translation>
    </message>
    <message>
        <source>About QGIS</source>
        <translation>О программе QGIS</translation>
    </message>
    <message>
        <source>Check Qgis Version</source>
        <translation>Проверить версию Qgis</translation>
    </message>
    <message>
        <source>Check if your QGIS version is up to date (requires internet access)</source>
        <translation>Проверить, является ли ваша версия QGIS последней (требует доступ в Интернет)</translation>
    </message>
    <message>
        <source>Refresh</source>
        <translation>Обновить</translation>
    </message>
    <message>
        <source>Ctrl+R</source>
        <comment>Refresh Map</comment>
        <translation></translation>
    </message>
    <message>
        <source>Refresh Map</source>
        <translation>Обновить карту</translation>
    </message>
    <message>
        <source>Zoom In</source>
        <translation>Увеличить</translation>
    </message>
    <message>
        <source>Ctrl++</source>
        <comment>Zoom In</comment>
        <translation></translation>
    </message>
    <message>
        <source>Zoom Out</source>
        <translation>Уменьшить</translation>
    </message>
    <message>
        <source>Ctrl+-</source>
        <comment>Zoom Out</comment>
        <translation></translation>
    </message>
    <message>
        <source>Zoom Full</source>
        <translation>Полный экстент</translation>
    </message>
    <message>
        <source>F</source>
        <comment>Zoom to Full Extents</comment>
        <translation></translation>
    </message>
    <message>
        <source>Zoom to Full Extents</source>
        <translation>Увеличить до полного экстента</translation>
    </message>
    <message>
        <source>Zoom To Selection</source>
        <translation>Увеличить до выделенного</translation>
    </message>
    <message>
        <source>Ctrl+F</source>
        <comment>Zoom to selection</comment>
        <translation></translation>
    </message>
    <message>
        <source>Zoom to selection</source>
        <translation>Увеличить до выделенного</translation>
    </message>
    <message>
        <source>Pan Map</source>
        <translation>Панорамировать карту</translation>
    </message>
    <message>
        <source>Pan the map</source>
        <translation>Панорамировать карту</translation>
    </message>
    <message>
        <source>Zoom Last</source>
        <translation>Предыдущий экстент</translation>
    </message>
    <message>
        <source>Zoom to Last Extent</source>
        <translation>Увеличить до предыдущего экстента</translation>
    </message>
    <message>
        <source>Zoom To Layer</source>
        <translation>Увеличить до слоя</translation>
    </message>
    <message>
        <source>Zoom to Layer</source>
        <translation>Увеличить до слоя</translation>
    </message>
    <message>
        <source>Identify Features</source>
        <translation>Определить объекты</translation>
    </message>
    <message>
        <source>I</source>
        <comment>Click on features to identify them</comment>
        <translation></translation>
    </message>
    <message>
        <source>Click on features to identify them</source>
        <translation>Определить объекты по щелчку мыши</translation>
    </message>
    <message>
        <source>Select Features</source>
        <translation>Выбрать объекты</translation>
    </message>
    <message>
        <source>Open Table</source>
        <translation>Открыть таблицу</translation>
    </message>
    <message>
        <source>Measure Line </source>
        <translation>Измерить линию </translation>
    </message>
    <message>
        <source>Ctrl+M</source>
        <comment>Measure a Line</comment>
        <translation></translation>
    </message>
    <message>
        <source>Measure a Line</source>
        <translation>Измерить линию</translation>
    </message>
    <message>
        <source>Measure Area</source>
        <translation>Измерить площадь</translation>
    </message>
    <message>
        <source>Ctrl+J</source>
        <comment>Measure an Area</comment>
        <translation></translation>
    </message>
    <message>
        <source>Measure an Area</source>
        <translation>Измерить площадь</translation>
    </message>
    <message>
        <source>Show Bookmarks</source>
        <translation>Показать закладки</translation>
    </message>
    <message>
        <source>B</source>
        <comment>Show Bookmarks</comment>
        <translation></translation>
    </message>
    <message>
        <source>New Bookmark...</source>
        <translation>Новая закладка...</translation>
    </message>
    <message>
        <source>Ctrl+B</source>
        <comment>New Bookmark</comment>
        <translation></translation>
    </message>
    <message>
        <source>New Bookmark</source>
        <translation>Новая закладка</translation>
    </message>
    <message>
        <source>Add WMS Layer...</source>
        <translation>Добавить WMS-слой...</translation>
    </message>
    <message>
        <source>W</source>
        <comment>Add Web Mapping Server Layer</comment>
        <translation></translation>
    </message>
    <message>
        <source>Add Web Mapping Server Layer</source>
        <translation>Добавить слой картографического веб-сервера</translation>
    </message>
    <message>
        <source>In Overview</source>
        <translation>В обзор</translation>
    </message>
    <message>
        <source>O</source>
        <comment>Add current layer to overview map</comment>
        <translation></translation>
    </message>
    <message>
        <source>Add current layer to overview map</source>
        <translation>Добавить текущий слой в обзорную карту</translation>
    </message>
    <message>
        <source>Plugin Manager...</source>
        <translation>Менеджер модулей...</translation>
    </message>
    <message>
        <source>Open the plugin manager</source>
        <translation>Открыть менеджер модулей</translation>
    </message>
    <message>
        <source>Capture Point</source>
        <translation>Захватить точку</translation>
    </message>
    <message>
        <source>.</source>
        <comment>Capture Points</comment>
        <translation>.</translation>
    </message>
    <message>
        <source>Capture Points</source>
        <translation>Захватить точки</translation>
    </message>
    <message>
        <source>Capture Line</source>
        <translation>Захватить линию</translation>
    </message>
    <message>
        <source>/</source>
        <comment>Capture Lines</comment>
        <translation></translation>
    </message>
    <message>
        <source>Capture Lines</source>
        <translation>Захватить линии</translation>
    </message>
    <message>
        <source>Capture Polygon</source>
        <translation>Захватить полигон</translation>
    </message>
    <message>
        <source>Ctrl+/</source>
        <comment>Capture Polygons</comment>
        <translation></translation>
    </message>
    <message>
        <source>Capture Polygons</source>
        <translation>Захватить полигоны</translation>
    </message>
    <message>
        <source>Delete Selected</source>
        <translation>Удалить выделенное</translation>
    </message>
    <message>
        <source>Add Vertex</source>
        <translation>Добавить вершину</translation>
    </message>
    <message>
        <source>Delete Vertex</source>
        <translation>Удалить вершину</translation>
    </message>
    <message>
        <source>Move Vertex</source>
        <translation>Переместить вершину</translation>
    </message>
    <message>
        <source>&amp;File</source>
        <translation>&amp;Файл</translation>
    </message>
    <message>
        <source>&amp;Open Recent Projects</source>
        <translation>&amp;Открыть недавние проекты</translation>
    </message>
    <message>
        <source>&amp;View</source>
        <translation>&amp;Вид</translation>
    </message>
    <message>
        <source>&amp;Layer</source>
        <translation>С&amp;лой</translation>
    </message>
    <message>
        <source>&amp;Settings</source>
        <translation>&amp;Установки</translation>
    </message>
    <message>
        <source>&amp;Help</source>
        <translation>&amp;Справка</translation>
    </message>
    <message>
        <source>File</source>
        <translation>Файл</translation>
    </message>
    <message>
        <source>Manage Layers</source>
        <translation>Управление слоями</translation>
    </message>
    <message>
        <source>Help</source>
        <translation>Справка</translation>
    </message>
    <message>
        <source>Digitizing</source>
        <translation>Оцифровка</translation>
    </message>
    <message>
        <source>Map Navigation</source>
        <translation>Навигация</translation>
    </message>
    <message>
        <source>Attributes</source>
        <translation>Атрибуты</translation>
    </message>
    <message>
        <source>Plugins</source>
        <translation>Модули</translation>
    </message>
    <message>
        <source>Ready</source>
        <translation type="unfinished">Готово</translation>
    </message>
    <message>
        <source>New features</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Unable to open project</source>
        <translation>Не удалось открыть проект</translation>
    </message>
    <message>
        <source>Unable to save project </source>
        <translation>Не удалось сохранить проект </translation>
    </message>
    <message>
        <source>Choose a filename to save the QGIS project file as</source>
        <translation>Выберите имя файла для сохранения проекта QGIS</translation>
    </message>
    <message>
        <source>QGIS: Unable to load project</source>
        <translation>QGIS: не удалось загрузить проект</translation>
    </message>
    <message>
        <source>Unable to load project </source>
        <translation>Не удалось загрузить проект </translation>
    </message>
    <message>
        <source>QGIS - Changes in SVN Since Last Release</source>
        <translation>QGIS - Изменения в SVN со времени последнего релиза</translation>
    </message>
    <message>
        <source>You can change this option later by selecting Options from the Settings menu (Help Browser tab).</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Layer is not valid</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The layer is not a valid layer and can not be added to the map</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Save?</source>
        <translation>Сохранить?</translation>
    </message>
    <message>
        <source>Clipboard contents set to: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> is not a valid or recognized raster data source</source>
        <translation> не является действительным (определяемым) источником растровых данных</translation>
    </message>
    <message>
        <source> is not a supported raster data source</source>
        <translation> не является поддерживаемым источником растровых данных</translation>
    </message>
    <message>
        <source>Unsupported Data Source</source>
        <translation>Неподдерживаемый источник данных</translation>
    </message>
    <message>
        <source>Enter a name for the new bookmark:</source>
        <translation>Введите имя для этой закладки:</translation>
    </message>
    <message>
        <source>Error</source>
        <translation>Ошибка</translation>
    </message>
    <message>
        <source>Unable to create the bookmark. Your user database may be missing or corrupted</source>
        <translation>Не удалось создать закладку. Ваша пользовательская база данных отсутствует или повреждена</translation>
    </message>
    <message>
        <source>Ctrl+?</source>
        <comment>Help Documentation (Mac)</comment>
        <translation>Ctrl+?</translation>
    </message>
    <message>
        <source>Show most toolbars</source>
        <translation>Показать все панели</translation>
    </message>
    <message>
        <source>Hide most toolbars</source>
        <translation>Скрыть все панели</translation>
    </message>
    <message>
        <source>Cut Features</source>
        <translation>Вырезать объекты</translation>
    </message>
    <message>
        <source>Cut selected features</source>
        <translation>Вырезать выделенные объекты</translation>
    </message>
    <message>
        <source>Copy Features</source>
        <translation>Копировать объекты</translation>
    </message>
    <message>
        <source>Copy selected features</source>
        <translation>Копировать выделенные объекты</translation>
    </message>
    <message>
        <source>Paste Features</source>
        <translation>Вставить объекты</translation>
    </message>
    <message>
        <source>Paste selected features</source>
        <translation>Вставить выделенные объекты</translation>
    </message>
    <message>
        <source>
Compiled against Qt </source>
        <translation>
Собрана под Qt </translation>
    </message>
    <message>
        <source>, running against Qt </source>
        <translation>. Выполняется под Qt </translation>
    </message>
    <message>
        <source>Network error while communicating with server</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Unknown network socket error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Unable to communicate with QGIS Version server</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>T</source>
        <comment>Show most toolbars</comment>
        <translation></translation>
    </message>
    <message>
        <source>Ctrl+T</source>
        <comment>Hide most toolbars</comment>
        <translation></translation>
    </message>
    <message>
        <source>Checking provider plugins</source>
        <translation>Проверка источников данных</translation>
    </message>
    <message>
        <source>Starting Python</source>
        <translation>Запуск Python</translation>
    </message>
    <message>
        <source>Python console</source>
        <translation>Консоль Python</translation>
    </message>
    <message>
        <source>Python error</source>
        <translation>Ошибка Python</translation>
    </message>
    <message>
        <source>Error when reading metadata of plugin </source>
        <translation>Ошибка чтения метаданных модуля </translation>
    </message>
    <message>
        <source>Provider does not support deletion</source>
        <translation>Источник не поддерживает удаление</translation>
    </message>
    <message>
        <source>Data provider does not support deleting features</source>
        <translation>Источник данных не поддерживает удаление объектов</translation>
    </message>
    <message>
        <source>Layer not editable</source>
        <translation>Нередактируемый слой</translation>
    </message>
    <message>
        <source>The current layer is not editable. Choose &apos;Start editing&apos; in the digitizing toolbar.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Toggle editing</source>
        <translation>Режим редактирования</translation>
    </message>
    <message>
        <source>Toggles the editing state of the current layer</source>
        <translation>Переключить текущий слой в режим редактирования</translation>
    </message>
    <message>
        <source>Add Ring</source>
        <translation>Добавить кольцо</translation>
    </message>
    <message>
        <source>Add Island</source>
        <translation>Добавить остров</translation>
    </message>
    <message>
        <source>Add Island to multipolygon</source>
        <translation>Добавить остров к мультиполигону</translation>
    </message>
    <message>
        <source>Toolbar Visibility...</source>
        <translation>Панели инструментов...</translation>
    </message>
    <message>
        <source>Scale </source>
        <translation>Масштаб </translation>
    </message>
    <message>
        <source>Current map scale (formatted as x:y)</source>
        <translation>Текущий масштаб карты (в формате x:y)</translation>
    </message>
    <message>
        <source>Map coordinates at mouse cursor position</source>
        <translation>Координаты карты в позиции курсора мыши</translation>
    </message>
    <message>
        <source>Invalid scale</source>
        <translation>Неверный масштаб</translation>
    </message>
    <message>
        <source>Do you want to save the current project?</source>
        <translation>Вы хотите сохранить текущий проект?</translation>
    </message>
    <message>
        <source>Python bindings - This is the major focus of this release it is now possible to create plugins using python. It is also possible to create GIS enabled applications written in python that use the QGIS libraries.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Removed automake build system - QGIS now needs CMake for compilation.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Many new GRASS tools added (with thanks to http://faunalia.it/)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Map Composer updates</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Crash fix for 2.5D shapefiles</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The QGIS libraries have been refactored and better organised.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Improvements to the GeoReferencer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Shows the map coordinates at the current cursor position. The display is continuously updated as the mouse is moved.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Added locale options to options dialog.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source></source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Move Feature</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Split Features</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Map Tips</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Show information about a feature when the mouse is hovered over it</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Current map scale</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Project file is older</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;p&gt;This project file was saved by an older version of QGIS.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> When saving this project file, QGIS will update it to the latest version, possibly rendering it useless for older versions of QGIS.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;p&gt;Even though QGIS developers try to maintain backwards compatibility, some of the information from the old project file might be lost.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> To improve the quality of QGIS, we appreciate if you file a bug report at %3.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> Be sure to include the old project file, and state the version of QGIS you used to discover the error.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;p&gt;To remove this warning when opening an older project file, uncheck the box &apos;%5&apos; in the %4 menu.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;p&gt;Version of the project file: %1&lt;br&gt;Current version of QGIS: %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;tt&gt;Settings:Options:General&lt;/tt&gt;</source>
        <comment>Menu path to setting options</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Warn me when opening a project file saved with an older version of QGIS</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgisAppBase</name>
    <message>
        <source>Quantum GIS</source>
        <translation type="obsolete">Quantum GIS</translation>
    </message>
    <message>
        <source>File</source>
        <translation type="obsolete">Файл</translation>
    </message>
    <message>
        <source>View</source>
        <translation type="obsolete">Вид</translation>
    </message>
    <message>
        <source>&amp;Tools</source>
        <translation type="obsolete">&amp;Инструменты</translation>
    </message>
    <message>
        <source>&amp;Help</source>
        <translation type="obsolete">&amp;Помощь</translation>
    </message>
    <message>
        <source>File Management Toolbar</source>
        <translation type="obsolete">Инструментальная Панель Управления Файлами</translation>
    </message>
    <message>
        <source>Data Toolbar</source>
        <translation type="obsolete">Инструментальная Панель Данных</translation>
    </message>
    <message>
        <source>Map Navigation Toolbar</source>
        <translation type="obsolete">Инструментальная Панель Навигации Картой</translation>
    </message>
    <message>
        <source>Attribute Data Toolbar</source>
        <translation type="obsolete">Инструментальная Панель Атрибутов Данных</translation>
    </message>
    <message>
        <source>Open Project</source>
        <translation type="obsolete">Открыть Проект</translation>
    </message>
    <message>
        <source>Exit</source>
        <translation type="obsolete">Выйти</translation>
    </message>
    <message>
        <source>E&amp;xit</source>
        <translation type="obsolete">В&amp;ыйти</translation>
    </message>
    <message>
        <source>Add a PostGIS Layer to the map</source>
        <translation type="obsolete">Добавить PostGIS слой к данной карте</translation>
    </message>
    <message>
        <source>Refresh </source>
        <translation type="obsolete">Обновить</translation>
    </message>
    <message>
        <source>Zoom to full extent</source>
        <translation type="obsolete">Изменить вид до полной</translation>
    </message>
    <message>
        <source>Map Navigation Tools</source>
        <translation type="obsolete">Инструменты Навигации Картой</translation>
    </message>
    <message>
        <source>Zoom In</source>
        <translation type="obsolete">Увеличить</translation>
    </message>
    <message>
        <source>Zoom &amp;In</source>
        <translation type="obsolete">&amp;Увеличить</translation>
    </message>
    <message>
        <source>Pan</source>
        <translation type="obsolete">Перевезти</translation>
    </message>
    <message>
        <source>&amp;Pan</source>
        <translation type="obsolete">&amp;Перевезти</translation>
    </message>
    <message>
        <source>Zoom out</source>
        <translation type="obsolete">Уменьшить</translation>
    </message>
    <message>
        <source>Zoom &amp;out</source>
        <translation type="obsolete">&amp;Уменьшить</translation>
    </message>
    <message>
        <source>Identify</source>
        <translation type="obsolete">Определить</translation>
    </message>
    <message>
        <source>Identify a feature on the active layer</source>
        <translation type="obsolete">Определить пункт на данном слое</translation>
    </message>
    <message>
        <source>select features</source>
        <translation type="obsolete">Выделить пункты</translation>
    </message>
    <message>
        <source>&amp;About Quantum GIS</source>
        <translation type="obsolete">&amp;О Quantum GIS</translation>
    </message>
    <message>
        <source>Test button</source>
        <translation type="obsolete">Тест кнопка</translation>
    </message>
    <message>
        <source>Add a vector layer</source>
        <translation type="obsolete">Добавить векторный слой</translation>
    </message>
    <message>
        <source>Add a vector layer (e.g. Shapefile)</source>
        <translation type="obsolete">Добавить векторный слой (на пример: Shapefile)</translation>
    </message>
    <message>
        <source>Attribute table</source>
        <translation type="obsolete">Таблица атрибутов</translation>
    </message>
    <message>
        <source>Open the attribute table for the selected layer</source>
        <translation type="obsolete">Открыть таблицу атрибутов для выделенного слоя</translation>
    </message>
    <message>
        <source>Action</source>
        <translation type="obsolete">Действие</translation>
    </message>
    <message>
        <source>Zoom to last extent</source>
        <translation type="obsolete">Изменить вид до предидущего</translation>
    </message>
    <message>
        <source>Test plugin functions</source>
        <translation type="obsolete">Тестировать функции компонента (плагина)</translation>
    </message>
    <message>
        <source>Options</source>
        <translation type="obsolete">Настройка</translation>
    </message>
    <message>
        <source>QGis options</source>
        <translation type="obsolete">Настройка QGIS</translation>
    </message>
    <message>
        <source>Save Project</source>
        <translation type="obsolete">Сохранить проект</translation>
    </message>
    <message>
        <source>Save Project As...</source>
        <translation type="obsolete">Сохранить Проект Как ...</translation>
    </message>
    <message>
        <source>Plugin Manager</source>
        <translation type="obsolete">Администратор компонентов (плагинов)</translation>
    </message>
    <message>
        <source>Check QGIS Version</source>
        <translation type="obsolete">Проверьте Версию QGIS</translation>
    </message>
    <message>
        <source>&amp;Properties</source>
        <translation type="obsolete">&amp;Свойства</translation>
    </message>
    <message>
        <source>MainWindow</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Legend</source>
        <translation>Легенда</translation>
    </message>
    <message>
        <source>Map View</source>
        <translation>Вид карты</translation>
    </message>
</context>
<context>
    <name>QgsAbout</name>
    <message>
        <source>About Quantum GIS</source>
        <translation>О Quantum GIS</translation>
    </message>
    <message>
        <source>Ok</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>About</source>
        <translation>О программе</translation>
    </message>
    <message>
        <source>Version</source>
        <translation>Версия</translation>
    </message>
    <message>
        <source>Quantum GIS (qgis)</source>
        <translation type="obsolete">Quantum GIS (qgis)</translation>
    </message>
    <message>
        <source>What&apos;s New</source>
        <translation>Что нового</translation>
    </message>
    <message>
        <source>License</source>
        <translation type="obsolete">Лицензия</translation>
    </message>
    <message>
        <source>http://www.gnu.org/licenses</source>
        <translation type="obsolete">http://www.gnu.org/licenses</translation>
    </message>
    <message>
        <source>Quantum GIS is licensed under the GNU General Public License</source>
        <translation type="obsolete">Данная программа лицнензирована по Универсальной Общественной Лицензии (GNU GPL)</translation>
    </message>
    <message>
        <source>Contributors</source>
        <translation type="obsolete">Основные участники</translation>
    </message>
    <message>
        <source>&lt;h3&gt;QGIS Contributors&lt;/h3&gt;
&lt;ul&gt;
&lt;li&gt;Gary E. Sherman&lt;br&gt;
&lt;li&gt;Steve Halasz&lt;br&gt;
&lt;li&gt;Tim Sutton&lt;br&gt;
&lt;li&gt;Marco Hugentobler&lt;br&gt;
&lt;li&gt;Denis Antipov&lt;br&gt;
&lt;li&gt;Mark Coletti
&lt;li&gt;Carl Anderson&lt;br&gt;
&lt;li&gt;Masaru Hoshi&lt;br&gt;
&lt;li&gt;Radim Blazek&lt;br&gt;
&lt;p&gt;
Apologies to anyone not included. Please let the project admin at qgis.sourceforge.net know if you have contributed but are not included in the acknowledgments.</source>
        <translation type="obsolete">&lt;h3&gt;QGIS Contributors&lt;/h3&gt;
&lt;ul&gt;
&lt;li&gt;Gary E. Sherman&lt;br&gt;
&lt;li&gt;Steve Halasz&lt;br&gt;
&lt;li&gt;Tim Sutton&lt;br&gt;
&lt;li&gt;Marco Hugentobler&lt;br&gt;
&lt;li&gt;Denis Antipov&lt;br&gt;
&lt;li&gt;Mark Coletti
&lt;li&gt;Carl Anderson&lt;br&gt;(new line)
&lt;li&gt;Masaru Hoshi&lt;br&gt;(new line)
&lt;li&gt;Radim Blazek&lt;br&gt;(new line)
&lt;p&gt;(new line)

Те кто не были включины в список основных участников, примите извинения. Пожалуйста дайте знать администратору проекта на qgis.sourceforge.net об участии в работе проекта но не занесении в вышеуказанный список.
</translation>
    </message>
    <message>
        <source>Plugins</source>
        <translation type="obsolete">Компоненты (Плагины)</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Quantum GIS (qgis)&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Quantum GIS (qgis)&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>QGIS Home Page</source>
        <translation>Веб-сайт QGIS</translation>
    </message>
    <message>
        <source>Subscribe to the QGIS-User mailing list</source>
        <translation>Подписатьcя на рассылку QGIS-User</translation>
    </message>
    <message>
        <source>Providers</source>
        <translation>Источники</translation>
    </message>
    <message>
        <source>Developers</source>
        <translation>Разработчики</translation>
    </message>
    <message>
        <source>&lt;h2&gt;QGIS Developers&lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Разработчики QGIS&lt;/h2&gt;</translation>
    </message>
    <message>
        <source>Name</source>
        <translation type="obsolete">Имя</translation>
    </message>
    <message>
        <source>Sponsors</source>
        <translation>Спонсоры</translation>
    </message>
    <message>
        <source>QGIS Sponsors</source>
        <translation type="obsolete">Спонсоры QGIS</translation>
    </message>
    <message>
        <source>Website</source>
        <translation type="obsolete">Веб-сайт</translation>
    </message>
    <message>
        <source>QGIS Browser Selection</source>
        <translation type="obsolete">Выбор браузера QGIS</translation>
    </message>
</context>
<context>
    <name>QgsAddAttrDialogBase</name>
    <message>
        <source>Add Attribute</source>
        <translation>Добавить атрибут</translation>
    </message>
    <message>
        <source>Name:</source>
        <translation>Имя:</translation>
    </message>
    <message>
        <source>Type:</source>
        <translation>Тип:</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Отменить</translation>
    </message>
</context>
<context>
    <name>QgsAttributeActionDialog</name>
    <message>
        <source>Name</source>
        <translation>Имя</translation>
    </message>
    <message>
        <source>Action</source>
        <translation>Действие</translation>
    </message>
    <message>
        <source>Capture</source>
        <translation>Захватывать</translation>
    </message>
    <message>
        <source>Select an action</source>
        <comment>File dialog window title</comment>
        <translation>Выберите действие</translation>
    </message>
</context>
<context>
    <name>QgsAttributeActionDialogBase</name>
    <message>
        <source>Form1</source>
        <translation></translation>
    </message>
    <message>
        <source>Name</source>
        <translation type="obsolete">Имя</translation>
    </message>
    <message>
        <source>Action</source>
        <translation type="obsolete">Действие</translation>
    </message>
    <message>
        <source>This list contains all actions that have been defined for the current layer. Add actions by entering the details in the controls below and then pressing the Insert action button. Actions can be edited here by double clicking on the item.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Move up</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Move the selected action up</source>
        <translation>Переместить выбранное действие выше</translation>
    </message>
    <message>
        <source>Move down</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Move the selected action down</source>
        <translation>Переместить выбранное действие ниже</translation>
    </message>
    <message>
        <source>Remove</source>
        <translation>Удалить</translation>
    </message>
    <message>
        <source>Remove the selected action</source>
        <translation>Удалить выбранное действие</translation>
    </message>
    <message>
        <source>Name:</source>
        <translation>Имя:</translation>
    </message>
    <message>
        <source>Enter the name of an action here. The name should be unique (qgis will make it unique if necessary).</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Enter the action name here</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Action:</source>
        <translation>Действие:</translation>
    </message>
    <message>
        <source>Enter the action command here</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Browse</source>
        <translation>Обзор</translation>
    </message>
    <message>
        <source>Browse for action commands</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Insert action</source>
        <translation>Вставить действие</translation>
    </message>
    <message>
        <source>Inserts the action into the list above</source>
        <translation>Вставить действие в список</translation>
    </message>
    <message>
        <source>Update action</source>
        <translation>Обновить действие</translation>
    </message>
    <message>
        <source>Update the selected action</source>
        <translation>Обновить выбранное действие</translation>
    </message>
    <message>
        <source>Insert field</source>
        <translation>Вставить поле</translation>
    </message>
    <message>
        <source>Inserts the selected field into the action, prepended with a %</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The valid attribute names for this layer</source>
        <translation>Действительные имена атрибутов для этого слоя</translation>
    </message>
    <message>
        <source>Capture output</source>
        <translation>Захватывать вывод</translation>
    </message>
    <message>
        <source>Captures any output from the action</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Captures the standard output or error generated by the action and displays it in a dialog box</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Enter the action here. This can be any program, script or command that is available on your system. When the action is invoked any set of characters that start with a % and then have the name of a field will be replaced by the value of that field. The special characters %% will be replaced by the value of the field that was selected. Double quote marks group text into single arguments to the program, script or command. Double quotes will be ignored if preceeded by a backslash</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsAttributeDialogBase</name>
    <message>
        <source>Enter Attribute Values</source>
        <translation>Введите значения атрибутов</translation>
    </message>
    <message>
        <source>1</source>
        <translation></translation>
    </message>
    <message>
        <source>Attribute</source>
        <translation>Атрибут</translation>
    </message>
    <message>
        <source>Value</source>
        <translation>Значение</translation>
    </message>
    <message>
        <source>&amp;OK</source>
        <translation>&amp;OK</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>О&amp;тменить</translation>
    </message>
</context>
<context>
    <name>QgsAttributeTable</name>
    <message>
        <source>Run action</source>
        <translation type="unfinished">Выполнить действие</translation>
    </message>
</context>
<context>
    <name>QgsAttributeTableBase</name>
    <message>
        <source>Attribute Table</source>
        <translation>Таблица атрибутов</translation>
    </message>
    <message>
        <source>Start editing</source>
        <translation>Начать редактирование</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>&amp;Закрыть</translation>
    </message>
    <message>
        <source>Alt+C</source>
        <translation></translation>
    </message>
    <message>
        <source>Ctrl+X</source>
        <translation></translation>
    </message>
    <message>
        <source>Ctrl+N</source>
        <translation></translation>
    </message>
    <message>
        <source>Ctrl+S</source>
        <translation></translation>
    </message>
    <message>
        <source>Invert selection</source>
        <translation>Обратить выделение</translation>
    </message>
    <message>
        <source>Ctrl+T</source>
        <translation></translation>
    </message>
    <message>
        <source>Move selected to top</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Remove selection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Copy selected rows to clipboard (Ctrl+C)</source>
        <translation>Копировать выбранные строки в буфер обмена (Ctrl+C)</translation>
    </message>
    <message>
        <source>Copies the selected rows to the clipboard</source>
        <translation>Копирует выбранные строки в буфер обмена</translation>
    </message>
    <message>
        <source>Ctrl+C</source>
        <translation></translation>
    </message>
    <message>
        <source>Stop editin&amp;g</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Alt+G</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Search for:</source>
        <translation>Искать:</translation>
    </message>
    <message>
        <source>in</source>
        <translation>в</translation>
    </message>
    <message>
        <source>Search</source>
        <translation type="unfinished">Искать</translation>
    </message>
    <message>
        <source>Adva&amp;nced...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Alt+N</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Help</source>
        <translation>&amp;Справка</translation>
    </message>
    <message>
        <source>New column</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Delete column</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Zoom map to the selected rows (Ctrl-F)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Zoom map to the selected rows</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Ctrl+F</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsAttributeTableDisplay</name>
    <message>
        <source>select</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>select and bring to top</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>show only matching</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Search string parsing error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Search results</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>You&apos;ve supplied an empty search string.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Error during search</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Found %d matching features.</source>
        <translation type="obsolete">
        </translation>
    </message>
    <message>
        <source>No matching features found.</source>
        <translation>Подходящих объектов не найдено.</translation>
    </message>
    <message>
        <source>Name conflict</source>
        <translation>Конфликт имён</translation>
    </message>
    <message>
        <source>Stop editing</source>
        <translation>Прекратить редактирование</translation>
    </message>
    <message>
        <source>Do you want to save the changes?</source>
        <translation>Вы хотите сохранить изменения?</translation>
    </message>
    <message>
        <source>Error</source>
        <translation>Ошибка</translation>
    </message>
    <message>
        <source>Could not commit changes</source>
        <translation>Не удалось внести изменения</translation>
    </message>
    <message>
        <source>The attribute could not be inserted. The name already exists in the table.</source>
        <translation>Не удалось вставить атрибут. Данное имя уже существует в таблице.</translation>
    </message>
</context>
<context>
    <name>QgsBookmarks</name>
    <message>
        <source>Really Delete?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Are you sure you want to delete the </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> bookmark?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Error deleting bookmark</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Failed to delete the </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> bookmark from the database. The database said:
</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsBookmarksBase</name>
    <message>
        <source>Geospatial Bookmarks</source>
        <translation>Пространственные закладки</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Имя</translation>
    </message>
    <message>
        <source>Project</source>
        <translation>Проект</translation>
    </message>
    <message>
        <source>Extent</source>
        <translation>Экстент</translation>
    </message>
    <message>
        <source>Id</source>
        <translation type="unfinished">ID</translation>
    </message>
    <message>
        <source>Zoom To</source>
        <translation>Увеличить до</translation>
    </message>
    <message>
        <source>Zoom to the currently selected bookmark</source>
        <translation>Увеличить до выбранной закладки</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>Удалить</translation>
    </message>
    <message>
        <source>Delete the currently selected bookmark</source>
        <translation>Удалить выбранную закладку</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
    <message>
        <source>Close the dialog</source>
        <translation>Закрыть диалог</translation>
    </message>
    <message>
        <source>Help</source>
        <translation>Справка</translation>
    </message>
</context>
<context>
    <name>QgsComposer</name>
    <message>
        <source> for read/write</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Choose a filename to save the map image as</source>
        <translation type="unfinished">Выберите имя файла для сохранения снимка карты</translation>
    </message>
    <message>
        <source>Choose a filename to save the map as</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Error in Print</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot seek</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot overwrite BoundingBox</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot find BoundingBox</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot overwrite translate</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot find translate</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>File IO Error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Paper does not match</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The selected paper size does not match the composition size</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Big image</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>To create image </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> requires circa </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> MB of memory</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>QGIS - print composer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Map 1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Couldn&apos;t open </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>format</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>SVG warning</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Don&apos;t show this message again</source>
        <translation type="unfinished">Не показывать это сообщение в дальнейшем</translation>
    </message>
    <message>
        <source>&lt;p&gt;The SVG export function in Qgis has several problems due to bugs and deficiencies in the Qt4 svg code. Of note, text does not appear in the SVG file and there are problems with the map bounding box clipping other items such as the legend or scale bar.&lt;/p&gt;If you require a vector-based output file from Qgis it is suggested that you try printing to PostScript if the SVG output is not satisfactory.&lt;/p&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>SVG Format</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsComposerBase</name>
    <message>
        <source>General</source>
        <translation type="unfinished">Общие</translation>
    </message>
    <message>
        <source>Composition</source>
        <translation type="unfinished">Композиция</translation>
    </message>
    <message>
        <source>Item</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Open Template ...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Save Template &amp;As...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Print...</source>
        <translation type="unfinished">&amp;Печать...</translation>
    </message>
    <message>
        <source>Zoom to full extent</source>
        <translation type="obsolete">Изменить вид до полной</translation>
    </message>
    <message>
        <source>Zoom out</source>
        <translation type="obsolete">Уменьшить</translation>
    </message>
    <message>
        <source>Add new map</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Add new label</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Add new vect legend</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Select/Move item</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Export as image</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Export as SVG</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Add new scalebar</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Refresh view</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>MainWindow</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Zoom All</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Zoom In</source>
        <translation>Увеличить</translation>
    </message>
    <message>
        <source>Zoom Out</source>
        <translation>Уменьшить</translation>
    </message>
    <message>
        <source>Add Image</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Close</source>
        <translation type="unfinished">Закрыть</translation>
    </message>
    <message>
        <source>Help</source>
        <translation>Справка</translation>
    </message>
</context>
<context>
    <name>QgsComposerLabelBase</name>
    <message>
        <source>Label Options</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Font</source>
        <translation type="unfinished">Шрифт</translation>
    </message>
    <message>
        <source>Box</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsComposerMap</name>
    <message>
        <source>Map %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Extent (calculate scale)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Scale (calculate extent)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cache</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Render</source>
        <translation type="unfinished">Отрисовка</translation>
    </message>
    <message>
        <source>Rectangle</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsComposerMapBase</name>
    <message>
        <source>Map options</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;b&gt;Map&lt;/b&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Set</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Width</source>
        <translation type="unfinished">Ширина</translation>
    </message>
    <message>
        <source>Height</source>
        <translation type="unfinished">Высота</translation>
    </message>
    <message>
        <source>Set Extent</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Set map extent to current extent in QGIS map canvas</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Line width scale</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Width of one unit in millimeters</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Symbol scale</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Font size scale</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Frame</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Preview</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>1:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Scale:</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsComposerPicture</name>
    <message>
        <source>Warning</source>
        <translation>Внимание</translation>
    </message>
    <message>
        <source>Cannot load picture.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Choose a file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Pictures (</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsComposerPictureBase</name>
    <message>
        <source>Picture Options</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Frame</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Angle</source>
        <translation type="unfinished">Угол</translation>
    </message>
    <message>
        <source>Width</source>
        <translation type="unfinished">Ширина</translation>
    </message>
    <message>
        <source>Height</source>
        <translation type="unfinished">Высота</translation>
    </message>
    <message>
        <source>Browse</source>
        <translation type="unfinished">Обзор</translation>
    </message>
</context>
<context>
    <name>QgsComposerScalebarBase</name>
    <message>
        <source>Barscale Options</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Segment size</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Number of segments</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Map units per scalebar unit</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Unit label</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Map</source>
        <translation type="unfinished">Карта</translation>
    </message>
    <message>
        <source>Font</source>
        <translation>Шрифт</translation>
    </message>
    <message>
        <source>Line width</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsComposerVectorLegend</name>
    <message>
        <source>Layers</source>
        <translation type="unfinished">Слои</translation>
    </message>
    <message>
        <source>Group</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Combine selected layers</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cache</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Render</source>
        <translation type="unfinished">Отрисовка</translation>
    </message>
    <message>
        <source>Rectangle</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Legend</source>
        <translation type="unfinished">Легенда</translation>
    </message>
</context>
<context>
    <name>QgsComposerVectorLegendBase</name>
    <message>
        <source>Vector Legend Options</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Title</source>
        <translation type="unfinished">Заглавие</translation>
    </message>
    <message>
        <source>Map</source>
        <translation type="unfinished">Карта</translation>
    </message>
    <message>
        <source>Font</source>
        <translation type="unfinished">Шрифт</translation>
    </message>
    <message>
        <source>Box</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Column 1</source>
        <translation type="unfinished">Столбец 1</translation>
    </message>
    <message>
        <source>Preview</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsComposition</name>
    <message>
        <source>Custom</source>
        <translation>Пользовательский</translation>
    </message>
    <message>
        <source>A5 (148x210 mm)</source>
        <translation>A5 (148x210 мм)</translation>
    </message>
    <message>
        <source>A4 (210x297 mm)</source>
        <translation>A4 (210x297 мм)</translation>
    </message>
    <message>
        <source>A3 (297x420 mm)</source>
        <translation>A3 (297x420 мм)</translation>
    </message>
    <message>
        <source>A2 (420x594 mm)</source>
        <translation>A2 (420x594 мм)</translation>
    </message>
    <message>
        <source>A1 (594x841 mm)</source>
        <translation>A1 (594x841 мм)</translation>
    </message>
    <message>
        <source>A0 (841x1189 mm)</source>
        <translation>A0 (841x1189 мм)</translation>
    </message>
    <message>
        <source>B5 (176 x 250 mm)</source>
        <translation>B5 (176 x 250 мм)</translation>
    </message>
    <message>
        <source>B4 (250 x 353 mm)</source>
        <translation>B4 (250 x 353 мм)</translation>
    </message>
    <message>
        <source>B3 (353 x 500 mm)</source>
        <translation>B3 (353 x 500 мм)</translation>
    </message>
    <message>
        <source>B2 (500 x 707 mm)</source>
        <translation>B2 (500 x 707 мм)</translation>
    </message>
    <message>
        <source>B1 (707 x 1000 mm)</source>
        <translation>B1 (707 x 1000 мм)</translation>
    </message>
    <message>
        <source>B0 (1000 x 1414 mm)</source>
        <translation>B0 (1000 x 1414 мм)</translation>
    </message>
    <message>
        <source>Letter (8.5x11 inches)</source>
        <translation>Letter (8.5x11 дюймов)</translation>
    </message>
    <message>
        <source>Legal (8.5x14 inches)</source>
        <translation>Legal (8.5x14 дюймов)</translation>
    </message>
    <message>
        <source>Portrait</source>
        <translation>Портрет</translation>
    </message>
    <message>
        <source>Landscape</source>
        <translation>Альбом</translation>
    </message>
    <message>
        <source>Out of memory</source>
        <translation>Не хватает памяти</translation>
    </message>
    <message>
        <source>Qgis is unable to resize the paper size due to insufficient memory.
 It is best that you avoid using the map composer until you restart qgis.
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Label</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Внимание</translation>
    </message>
    <message>
        <source>Cannot load picture.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsCompositionBase</name>
    <message>
        <source>Composition</source>
        <translation>Композиция</translation>
    </message>
    <message>
        <source>Paper</source>
        <translation>Бумага</translation>
    </message>
    <message>
        <source>Size</source>
        <translation>Размер</translation>
    </message>
    <message>
        <source>Units</source>
        <translation>Единицы</translation>
    </message>
    <message>
        <source>Width</source>
        <translation>Ширина</translation>
    </message>
    <message>
        <source>Height</source>
        <translation>Высота</translation>
    </message>
    <message>
        <source>Orientation</source>
        <translation>Ориентация</translation>
    </message>
    <message>
        <source>Resolution (dpi)</source>
        <translation>Разрешение (dpi)</translation>
    </message>
</context>
<context>
    <name>QgsConnectionDialog</name>
    <message>
        <source>Test connection</source>
        <translation>Проверить соединение</translation>
    </message>
    <message>
        <source>Connection to </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> was successfull</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Connection failed - Check settings and try again </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>General Interface Help:

</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsConnectionDialogBase</name>
    <message>
        <source>Connection Information</source>
        <translation>Информация о соединении</translation>
    </message>
    <message>
        <source>Host</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Database</source>
        <translation type="unfinished">База данных</translation>
    </message>
    <message>
        <source>Username</source>
        <translation type="unfinished">Имя пользователя</translation>
    </message>
    <message>
        <source>Name</source>
        <translation type="unfinished">Имя</translation>
    </message>
    <message>
        <source>Name of the new connection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Password</source>
        <translation type="unfinished">Пароль</translation>
    </message>
    <message>
        <source>Test Connect</source>
        <translation type="unfinished">Проверить соединение</translation>
    </message>
    <message>
        <source>Save Password</source>
        <translation type="unfinished">Сохранить пароль</translation>
    </message>
    <message>
        <source>Create a New PostGIS connection</source>
        <translation type="unfinished">Создать новое PostGIS-соединение</translation>
    </message>
    <message>
        <source>Port</source>
        <translation type="unfinished">Порт</translation>
    </message>
    <message>
        <source>5432</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsContinuousColorDialogBase</name>
    <message>
        <source>Continuous color</source>
        <translation>Непрерывный цвет</translation>
    </message>
    <message>
        <source>Maximum Value:</source>
        <translation>Максимальное значение:</translation>
    </message>
    <message>
        <source>Outline Width:</source>
        <translation>Ширина контура:</translation>
    </message>
    <message>
        <source>Minimum Value:</source>
        <translation>Минимальное значение:</translation>
    </message>
    <message>
        <source>Classification Field:</source>
        <translation>Поле классификации:</translation>
    </message>
    <message>
        <source>Draw polygon outline</source>
        <translation>Рисовать контуры полигонов</translation>
    </message>
</context>
<context>
    <name>QgsCoordinateTransform</name>
    <message>
        <source>Failed</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>transform of</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>with error: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The source spatial reference system (SRS) is not valid. </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The coordinates can not be reprojected. The SRS is: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The destination spatial reference system (SRS) is not valid. </source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsCopyrightLabelPlugin</name>
    <message>
        <source>Bottom Left</source>
        <translation>Внизу слева</translation>
    </message>
    <message>
        <source>Top Left</source>
        <translation>Вверху слева</translation>
    </message>
    <message>
        <source>Top Right</source>
        <translation>Вверху справа</translation>
    </message>
    <message>
        <source>Bottom Right</source>
        <translation>Внизу справа</translation>
    </message>
    <message>
        <source>&amp;Copyright Label</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Creates a copyright label that is displayed on the map canvas.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Decorations</source>
        <translation>&amp;Декораторы</translation>
    </message>
</context>
<context>
    <name>QgsCopyrightLabelPluginGuiBase</name>
    <message>
        <source>Copyright Label Plugin</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Placement</source>
        <translation>Размещение</translation>
    </message>
    <message>
        <source>Bottom Left</source>
        <translation>Внизу слева</translation>
    </message>
    <message>
        <source>Top Left</source>
        <translation>Вверху слева</translation>
    </message>
    <message>
        <source>Bottom Right</source>
        <translation>Внизу справа</translation>
    </message>
    <message>
        <source>Top Right</source>
        <translation>Вверху справа</translation>
    </message>
    <message>
        <source>Orientation</source>
        <translation>Ориентация</translation>
    </message>
    <message>
        <source>Horizontal</source>
        <translation>Горизонтальная</translation>
    </message>
    <message>
        <source>Vertical</source>
        <translation>Вертикальная</translation>
    </message>
    <message>
        <source>Enable Copyright Label</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Color</source>
        <translation>Цвет</translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message encoding="UTF-8">
        <source>© QGIS 2008</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsCustomProjectionDialog</name>
    <message>
        <source>Delete Projection Definition?</source>
        <translation>Удалить определение проекции?</translation>
    </message>
    <message>
        <source>Deleting a projection definition is not reversable. Do you want to delete it?</source>
        <translation>Удаление определения проекции ‒ необратимая операция. Вы уверены, что хотите удалить его?</translation>
    </message>
    <message>
        <source>Abort</source>
        <translation>Сбросить</translation>
    </message>
    <message>
        <source>New</source>
        <translation type="unfinished">Новое</translation>
    </message>
    <message>
        <source>QGIS Custom Projection</source>
        <translation>Пользовательская проекция QGIS</translation>
    </message>
    <message>
        <source>This proj4 projection definition is not valid. Please give the projection a name before pressing save.</source>
        <translation>Неверное определение проекции proj4. Пожалуйста, введите имя проекции перед сохранением.</translation>
    </message>
    <message>
        <source>This proj4 projection definition is not valid. Please add the parameters before pressing save.</source>
        <translation>Неверное определение проекции proj4. Пожалуйста, введите параметры проекции перед сохранением.</translation>
    </message>
    <message>
        <source>This proj4 projection definition is not valid. Please add a proj= clause before pressing save.</source>
        <translation>Неверное определение проекции proj4. Пожалуйста, введите условие proj= перед сохранением.</translation>
    </message>
    <message>
        <source>This proj4 ellipsoid definition is not valid. Please add a ellips= clause before pressing save.</source>
        <translation>Неверное определение эллипсоида proj4. Пожалуйста, введите условие ellips= перед сохранением.</translation>
    </message>
    <message>
        <source>This proj4 projection definition is not valid. Please correct before pressing save.</source>
        <translation>Неверное определение проекции proj4. Пожалуйста, исправьте его перед сохранением.</translation>
    </message>
    <message>
        <source>This proj4 projection definition is not valid.</source>
        <translation>Неверное определение проекции proj4.</translation>
    </message>
    <message>
        <source>Northing and Easthing must be in decimal form.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Internal Error (source projection invalid?)</source>
        <translation>Внутренняя ошибка (неверная исходная проекция?)</translation>
    </message>
</context>
<context>
    <name>QgsCustomProjectionDialogBase</name>
    <message>
        <source>Custom Projection Definition</source>
        <translation>Определение пользовательской проекции</translation>
    </message>
    <message>
        <source>Define</source>
        <translation>Определение</translation>
    </message>
    <message>
        <source>Parameters:</source>
        <translation>Параметры:</translation>
    </message>
    <message>
        <source>|&lt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>1 of 1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&gt;|</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>New</source>
        <translation type="unfinished">Новое</translation>
    </message>
    <message>
        <source>Save</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Delete</source>
        <translation type="unfinished">Удалить</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
    <message>
        <source>Name:</source>
        <translation>Имя:</translation>
    </message>
    <message>
        <source>Test</source>
        <translation>Проверка</translation>
    </message>
    <message>
        <source>Transform from WGS84 to the chosen projection</source>
        <translation>Преобразование в заданную проекцию из WGS84</translation>
    </message>
    <message>
        <source>Calculate</source>
        <translation>Расчитать</translation>
    </message>
    <message>
        <source>Projected Corrdinate System</source>
        <translation type="obsolete">Система координат проекции</translation>
    </message>
    <message>
        <source>Geographic / WGS84</source>
        <translation>Географическая / WGS84</translation>
    </message>
    <message>
        <source>North:</source>
        <translation>Север:</translation>
    </message>
    <message>
        <source>East:</source>
        <translation>Восток:</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:10pt;&quot;&gt;You can define your own custom projection here. The definition must conform to the proj4 format for specifying a Spatial Reference System.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:10pt;&quot;&gt;В этом окне вы можете определить свою собственную проекцию. Определение должно соответствовать формату определения координатных систем proj4.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:10pt;&quot;&gt;Use the text boxes below to test the projection definition you are creating. Enter a coordinate where both the lat/long and the projected result are known (for example by reading off a map). Then press the calculate button to see if the projection definition you are creating is accurate.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:10pt;&quot;&gt;Используйте нижеприведённые  текстовые поля для проверки создаваемого определения проекции. Введите координату для которой известны широта/долгота и результат в системе проекции (к примеру, из карты). Затем нажмите кнопку «Расчитать», и убедитесь, что созданное определение проекции является действительным.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>You can define your own custom projection here. The definition must conform to the proj4 format for specifying a Spatial Reference System.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Use the text boxes below to test the projection definition you are creating. Enter a coordinate where both the lat/long and the projected result are known (for example by reading off a map). Then press the calculate button to see if the projection definition you are creating is accurate.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Projected Coordinate System</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsDbSourceSelect</name>
    <message>
        <source>Are you sure you want to remove the </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> connection and all associated settings?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Confirm Delete</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Yes</source>
        <translation type="obsolete">Да</translation>
    </message>
    <message>
        <source>No</source>
        <translation type="obsolete">Нет</translation>
    </message>
    <message>
        <source>Select Table</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>You must select a table in order to add a Layer.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Password for </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Please enter your password:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Connection failed</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Type</source>
        <translation type="unfinished">Тип</translation>
    </message>
    <message>
        <source>Name</source>
        <translation type="obsolete">Имя</translation>
    </message>
    <message>
        <source>Sql</source>
        <translation type="unfinished">SQL</translation>
    </message>
    <message>
        <source>Connection to %1 on %2 failed. Either the database is down or your settings are incorrect.%3Check your username and password and try again.%4The database said:%5%6</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Wildcard</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>RegExp</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>All</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Schema</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Table</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Geometry column</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsDbSourceSelectBase</name>
    <message>
        <source>Add PostGIS Table(s)</source>
        <translation>Добавить таблицы PostGIS</translation>
    </message>
    <message>
        <source>Add</source>
        <translation>Добавить</translation>
    </message>
    <message>
        <source>Help</source>
        <translation>Справка</translation>
    </message>
    <message>
        <source>F1</source>
        <translation></translation>
    </message>
    <message>
        <source>Connect</source>
        <translation>Подключить</translation>
    </message>
    <message>
        <source>New</source>
        <translation>Новое</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation>Правка</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>Удалить</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
    <message>
        <source>PostgreSQL Connections</source>
        <translation>PostgreSQL-соединения</translation>
    </message>
    <message>
        <source>Tables:</source>
        <translation type="obsolete">Таблицы:</translation>
    </message>
    <message>
        <source>Type</source>
        <translation type="obsolete">Тип</translation>
    </message>
    <message>
        <source>Name</source>
        <translation type="obsolete">Имя</translation>
    </message>
    <message>
        <source>Sql</source>
        <translation type="obsolete">SQL</translation>
    </message>
    <message>
        <source>Encoding:</source>
        <translation type="obsolete">Кодировка:</translation>
    </message>
    <message>
        <source>Search:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Search mode:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Search in columns:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Search options...</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsDbTableModel</name>
    <message>
        <source>Schema</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Table</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Type</source>
        <translation type="unfinished">Тип</translation>
    </message>
    <message>
        <source>Geometry column</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Sql</source>
        <translation type="unfinished">SQL</translation>
    </message>
    <message>
        <source>Point</source>
        <translation type="unfinished">Точка</translation>
    </message>
    <message>
        <source>Multipoint</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Line</source>
        <translation type="unfinished">Линия</translation>
    </message>
    <message>
        <source>Multiline</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Polygon</source>
        <translation type="unfinished">Полигон</translation>
    </message>
    <message>
        <source>Multipolygon</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsDelAttrDialogBase</name>
    <message>
        <source>Delete Attributes</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Отменить</translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Please enter a layer name before adding the layer to the map</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>No delimiter</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Please specify a delimiter prior to parsing the file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Choose a delimited text file to open</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Parse</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Description</source>
        <translation type="unfinished">Описание</translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;p align=&quot;right&quot;&gt;X field&lt;/p&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Name of the field containing x values</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Name of the field containing x values. Choose a field from the list. The list is generated by parsing the header row of the delimited text file.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;p align=&quot;right&quot;&gt;Y field&lt;/p&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Name of the field containing y values</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Name of the field containing y values. Choose a field from the list. The list is generated by parsing the header row of the delimited text file.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Layer name</source>
        <translation>Имя слоя</translation>
    </message>
    <message>
        <source>Name to display in the map legend</source>
        <translation>Имя для отображения в легенде карты</translation>
    </message>
    <message>
        <source>Name displayed in the map legend</source>
        <translation>Имя, отображаемое в легенде карты</translation>
    </message>
    <message>
        <source>Delimiter</source>
        <translation>Разделитель</translation>
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
        <source>&amp;Help</source>
        <translation type="obsolete">&amp;Помощь</translation>
    </message>
    <message>
        <source>Sample text</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Browse...</source>
        <translation>Обзор...</translation>
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
</context>
<context>
    <name>QgsDelimitedTextProvider</name>
    <message>
        <source>Note: the following lines were not loaded because Qgis was unable to determine values for the x and y coordinates:
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Error</source>
        <translation type="unfinished">Ошибка</translation>
    </message>
</context>
<context>
    <name>QgsDlgPgBufferBase</name>
    <message>
        <source>Buffer features</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Help</source>
        <translation type="obsolete">&amp;Помощь</translation>
    </message>
    <message>
        <source>Buffer distance in map units:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Table name for the buffered layer:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Create unique object id</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>public</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Geometry column:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Spatial reference ID:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Unique field to use as feature id:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Schema:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Add the buffered layer to the map?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;h2&gt;Buffer the features in layer: &lt;/h2&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Parameters</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsEditReservedWordsBase</name>
    <message>
        <source>Edit Reserved Words</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Status</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Index</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Help</source>
        <translation type="obsolete">&amp;Помощь</translation>
    </message>
    <message>
        <source>Reserved Words</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Double click the Column Name column to change the name of the column.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Column Name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;This shapefile contains reserved words. These may affect the import into PostgreSQL. Edit the column names so none of the reserved words listed at the right are used (click on a Column Name entry to edit). You may also change any other column name if desired.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsEditReservedWordsDialog</name>
    <message>
        <source>Status</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Column Name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Index</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsEncodingFileDialog</name>
    <message>
        <source>Encoding:</source>
        <translation>Кодировка:</translation>
    </message>
</context>
<context>
    <name>QgsFillStyleWidgetBase</name>
    <message>
        <source>Form1</source>
        <translation></translation>
    </message>
    <message>
        <source>Fill Style</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>PolyStyleWidget</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Colour:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>col</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGPSDeviceDialog</name>
    <message>
        <source>New device %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Are you sure?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Are you sure that you want to delete this device?</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGPSDeviceDialogBase</name>
    <message>
        <source>GPS Device Editor</source>
        <translation>Редактор GPS-устройств</translation>
    </message>
    <message>
        <source>Device name:</source>
        <translation>Имя устройства:</translation>
    </message>
    <message>
        <source>This is the name of the device as it will appear in the lists</source>
        <translation>Имя устройства, которое отображается в списке</translation>
    </message>
    <message>
        <source>Update device</source>
        <translation>Обновить устройство</translation>
    </message>
    <message>
        <source>Delete device</source>
        <translation>Удалить устройство</translation>
    </message>
    <message>
        <source>New device</source>
        <translation>Новое устройство</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
    <message>
        <source>Commands</source>
        <translation>Команды</translation>
    </message>
    <message>
        <source>Waypoint download:</source>
        <translation>Загрузка точек:</translation>
    </message>
    <message>
        <source>Waypoint upload:</source>
        <translation>Выгрузка точек:</translation>
    </message>
    <message>
        <source>Route download:</source>
        <translation>Загрузка маршрутов:</translation>
    </message>
    <message>
        <source>Route upload:</source>
        <translation>Выгрузка маршрутов:</translation>
    </message>
    <message>
        <source>Track download:</source>
        <translation>Загрузка треков:</translation>
    </message>
    <message>
        <source>The command that is used to upload tracks to the device</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Track upload:</source>
        <translation>Выгрузка треков:</translation>
    </message>
    <message>
        <source>The command that is used to download tracks from the device</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The command that is used to upload routes to the device</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The command that is used to download routes from the device</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The command that is used to upload waypoints to the device</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The command that is used to download waypoints from the device</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;In the download and upload commands there can be special words that will be replaced by QGIS when the commands are used. These words are:&lt;span style=&quot; font-style:italic;&quot;&gt;%babel&lt;/span&gt; - the path to GPSBabel&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%in&lt;/span&gt; - the GPX filename when uploading or the port when downloading&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%out&lt;/span&gt; - the port when uploading or the GPX filename when downloading&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;В командах загрузки и выгрузки допускается ввод специальных слов, которые будут изменены QGIS при запуске команды. Этими словами являются:&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%babel&lt;/span&gt; - путь к GPSBabel&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%in&lt;/span&gt; - GPX-файл (выгрузка) или порт (загрузка)&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%out&lt;/span&gt; - порт (выгрузка) или GPX-файл (загрузка)&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
</context>
<context>
    <name>QgsGPSPlugin</name>
    <message>
        <source>&amp;Gps Tools</source>
        <translation>&amp;GPS-инструменты</translation>
    </message>
    <message>
        <source>&amp;Create new GPX layer</source>
        <translation>&amp;Создать новый GPX-слой</translation>
    </message>
    <message>
        <source>Creates a new GPX layer and displays it on the map canvas</source>
        <translation>Создать новый GPX-слой и вывести его на карте</translation>
    </message>
    <message>
        <source>&amp;Gps</source>
        <translation>&amp;GPS</translation>
    </message>
    <message>
        <source>Save new GPX file as...</source>
        <translation>Сохранить новый GPX-файл как...</translation>
    </message>
    <message>
        <source>GPS eXchange file (*.gpx)</source>
        <translation>Файлы GPS eXchange (*.gpx)</translation>
    </message>
    <message>
        <source>Could not create file</source>
        <translation>Не удалось создать файл</translation>
    </message>
    <message>
        <source>Unable to create a GPX file with the given name. </source>
        <translation>Не удалось создать GPX-файл с заданным именем. </translation>
    </message>
    <message>
        <source>Try again with another name or in another </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>directory.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>GPX Loader</source>
        <translation>Загрузчик GPX</translation>
    </message>
    <message>
        <source>Unable to read the selected file.
</source>
        <translation>Не удалось прочитать выбранный файл.
</translation>
    </message>
    <message>
        <source>Please reselect a valid file.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Could not start process</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Could not start GPSBabel!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Importing data...</source>
        <translation>Импорт данных...</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Отменить</translation>
    </message>
    <message>
        <source>Could not import data from %1!

</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Error importing data</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Not supported</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>This device does not support downloading </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>of </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Downloading data...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Could not download data from GPS!

</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Error downloading data</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>This device does not support uploading of </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Uploading data...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Error while uploading data to GPS!

</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Error uploading data</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Could not convert data from %1!

</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Error converting data</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGPSPluginGui</name>
    <message>
        <source>Choose a filename to save under</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>GPS eXchange format (*.gpx)</source>
        <translation>Формат GPS eXchange (*.gpx)</translation>
    </message>
    <message>
        <source>Select GPX file</source>
        <translation>Выберите GPX-файл</translation>
    </message>
    <message>
        <source>Select file and format to import</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Waypoints</source>
        <translation type="unfinished">Маршрутные точки</translation>
    </message>
    <message>
        <source>Routes</source>
        <translation type="unfinished">Маршруты</translation>
    </message>
    <message>
        <source>Tracks</source>
        <translation type="unfinished">Треки</translation>
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
        <source>Select a GPX input file name, the type of conversion you want to perform, a GPX filename that you want to save the converted file as, and a name for the new layer created from the result.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>GPX is the %1, which is used to store information about waypoints, routes, and tracks.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>GPS eXchange file format</source>
        <translation type="unfinished"></translation>
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
        <source>Select a GPS file format and the file that you want to import, the feature type that you want to use, a GPX filename that you want to save the converted file as, and a name for the new layer.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>All file formats can not store waypoints, routes, and tracks, so some feature types may be disabled for some file formats.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGPSPluginGuiBase</name>
    <message>
        <source>GPS Tools</source>
        <translation>GPS-инструменты</translation>
    </message>
    <message>
        <source>Load GPX file</source>
        <translation>GPX-файлы</translation>
    </message>
    <message>
        <source>File:</source>
        <translation>Файл:</translation>
    </message>
    <message>
        <source>Feature types:</source>
        <translation>Типы объектов:</translation>
    </message>
    <message>
        <source>Waypoints</source>
        <translation>Маршрутные точки</translation>
    </message>
    <message>
        <source>Routes</source>
        <translation>Маршруты</translation>
    </message>
    <message>
        <source>Tracks</source>
        <translation>Треки</translation>
    </message>
    <message>
        <source>Import other file</source>
        <translation>Прочие файлы</translation>
    </message>
    <message>
        <source>File to import:</source>
        <translation>Импортируемый файл:</translation>
    </message>
    <message>
        <source>Feature type:</source>
        <translation>Тип объектов:</translation>
    </message>
    <message>
        <source>GPX output file:</source>
        <translation>Выходной GPX-файл:</translation>
    </message>
    <message>
        <source>Layer name:</source>
        <translation>Имя слоя:</translation>
    </message>
    <message>
        <source>Download from GPS</source>
        <translation>Загрузка с GPS</translation>
    </message>
    <message>
        <source>Edit devices</source>
        <translation>Редактировать устройства</translation>
    </message>
    <message>
        <source>GPS device:</source>
        <translation>GPS-устройство:</translation>
    </message>
    <message>
        <source>Output file:</source>
        <translation>Файл вывода:</translation>
    </message>
    <message>
        <source>Port:</source>
        <translation>Порт:</translation>
    </message>
    <message>
        <source>Upload to GPS</source>
        <translation>Выгрузка в GPS</translation>
    </message>
    <message>
        <source>Data layer:</source>
        <translation>Слой данных:</translation>
    </message>
    <message>
        <source>&amp;Help</source>
        <translation type="obsolete">&amp;Помощь</translation>
    </message>
    <message>
        <source>Browse...</source>
        <translation>Обзор...</translation>
    </message>
    <message>
        <source>Save As...</source>
        <translation>Сохранить как...</translation>
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
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGPXProvider</name>
    <message>
        <source>Bad URI - you need to specify the feature type.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>GPS eXchange file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Digitized in QGIS</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGeomTypeDialog</name>
    <message>
        <source>Name</source>
        <translation>Имя</translation>
    </message>
    <message>
        <source>Type</source>
        <translation>Тип</translation>
    </message>
</context>
<context>
    <name>QgsGeomTypeDialogBase</name>
    <message>
        <source>Type</source>
        <translation>Тип</translation>
    </message>
    <message>
        <source>Point</source>
        <translation>Точка</translation>
    </message>
    <message>
        <source>Line</source>
        <translation>Линия</translation>
    </message>
    <message>
        <source>Polygon</source>
        <translation>Полигон</translation>
    </message>
    <message>
        <source>New Vector Layer</source>
        <translation>Новый векторный слой</translation>
    </message>
    <message>
        <source>Attributes:</source>
        <translation>Атрибуты:</translation>
    </message>
    <message>
        <source>Add</source>
        <translation>Добавить</translation>
    </message>
    <message>
        <source>Column 1</source>
        <translation>Столбец 1</translation>
    </message>
    <message>
        <source>Remove</source>
        <translation>Удалить</translation>
    </message>
    <message>
        <source>File Format:</source>
        <translation>Формат файла:</translation>
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
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGeorefPluginGui</name>
    <message>
        <source>Choose a raster file</source>
        <translation>Выберите растровый файл</translation>
    </message>
    <message>
        <source>Raster files (*.*)</source>
        <translation>Растровые файлы (*.*)</translation>
    </message>
    <message>
        <source>Error</source>
        <translation>Ошибка</translation>
    </message>
    <message>
        <source>The selected file is not a valid raster file.</source>
        <translation>Выбранный файл не является действительным растровым файлом.</translation>
    </message>
    <message>
        <source>World file exists</source>
        <translation>Файл привязки уже существует</translation>
    </message>
    <message>
        <source>&lt;p&gt;The selected file already seems to have a </source>
        <translation>&lt;p&gt;Судя по всему выбранный файл уже имеет </translation>
    </message>
    <message>
        <source>world file! Do you want to replace it with the </source>
        <translation>файл привязки! Вы хотите заменить его </translation>
    </message>
    <message>
        <source>new world file?&lt;/p&gt;</source>
        <translation>новым файлом привязки?&lt;/p&gt;</translation>
    </message>
</context>
<context>
    <name>QgsGeorefPluginGuiBase</name>
    <message>
        <source>Georeferencer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
    <message>
        <source>Raster file:</source>
        <translation>Растровый файл:</translation>
    </message>
    <message>
        <source>Arrange plugin windows</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>...</source>
        <translation type="unfinished">...</translation>
    </message>
    <message>
        <source>Description...</source>
        <translation>Описание...</translation>
    </message>
</context>
<context>
    <name>QgsGeorefWarpOptionsDialog</name>
    <message>
        <source>unstable</source>
        <translation type="unfinished"></translation>
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
        <translation>Метод интерполяции:</translation>
    </message>
    <message>
        <source>Nearest neighbour</source>
        <translation type="unfinished">Ближайший сосед</translation>
    </message>
    <message>
        <source>Linear</source>
        <translation>Линейная</translation>
    </message>
    <message>
        <source>Cubic</source>
        <translation>Кубическая</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>Use 0 for transparency when needed</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Compression:</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGraduatedSymbolDialog</name>
    <message>
        <source>Equal Interval</source>
        <translation>Равные интервалы</translation>
    </message>
    <message>
        <source>Quantiles</source>
        <translation>Квантили</translation>
    </message>
    <message>
        <source>Empty</source>
        <translation>Пустые</translation>
    </message>
</context>
<context>
    <name>QgsGraduatedSymbolDialogBase</name>
    <message>
        <source>graduated Symbol</source>
        <translation>градуированный знак</translation>
    </message>
    <message>
        <source>Classification Field:</source>
        <translation>Поле классификации:</translation>
    </message>
    <message>
        <source>Mode:</source>
        <translation>Режим:</translation>
    </message>
    <message>
        <source>Number of Classes:</source>
        <translation>Количество классов:</translation>
    </message>
    <message>
        <source>Delete class</source>
        <translation>Удалить класс</translation>
    </message>
    <message>
        <source>Classify</source>
        <translation>Классифицировать</translation>
    </message>
</context>
<context>
    <name>QgsGrassAttributes</name>
    <message>
        <source>Warning</source>
        <translation>Внимание</translation>
    </message>
    <message>
        <source>Column</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Value</source>
        <translation type="unfinished">Значение</translation>
    </message>
    <message>
        <source>Type</source>
        <translation type="unfinished">Тип</translation>
    </message>
    <message>
        <source>ERROR</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>Layer</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassAttributesBase</name>
    <message>
        <source>GRASS Attributes</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Tab 1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>result</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Update</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Update database record</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>New</source>
        <translation type="unfinished">Новое</translation>
    </message>
    <message>
        <source>Add new category using settings in GRASS Edit toolbox</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Delete</source>
        <translation type="unfinished">Удалить</translation>
    </message>
    <message>
        <source>Delete selected category</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassBrowser</name>
    <message>
        <source>Tools</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Add selected map to canvas</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Copy selected map</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Rename selected map</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Delete selected map</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Set current region to selected map</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Refresh</source>
        <translation type="unfinished">Обновить</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Внимание</translation>
    </message>
    <message>
        <source>Cannot copy map </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;br&gt;command: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot rename map </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Delete map &lt;b&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot delete map </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot write new region</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>New name</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassEdit</name>
    <message>
        <source>New point</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>New centroid</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Delete vertex</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Left: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Middle: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Edit tools</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>New line</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>New boundary</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Move vertex</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Add vertex</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Move element</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Split line</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Delete element</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Edit attributes</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Close</source>
        <translation type="unfinished">Закрыть</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Внимание</translation>
    </message>
    <message>
        <source>You are not owner of the mapset, cannot open the vector for editing.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot open vector for update.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Info</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The table was created</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Tool not yet implemented.</source>
        <translation type="unfinished"></translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Highlight</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Dynamic</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Point</source>
        <translation type="unfinished">Точка</translation>
    </message>
    <message>
        <source>Line</source>
        <translation type="unfinished">Линия</translation>
    </message>
    <message>
        <source>Boundary (no area)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Boundary (1 area)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Boundary (2 areas)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Centroid (in area)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Centroid (outside area)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Centroid (duplicate in area)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Node (1 line)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Node (2 lines)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Disp</source>
        <comment>Column title</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Color</source>
        <comment>Column title</comment>
        <translation type="unfinished">Цвет</translation>
    </message>
    <message>
        <source>Type</source>
        <comment>Column title</comment>
        <translation type="unfinished">Тип</translation>
    </message>
    <message>
        <source>Index</source>
        <comment>Column title</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Column</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Type</source>
        <translation type="unfinished">Тип</translation>
    </message>
    <message>
        <source>Length</source>
        <translation type="unfinished">Длина</translation>
    </message>
    <message>
        <source>Next not used</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Manual entry</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>No category</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Right: </source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassEditBase</name>
    <message>
        <source>GRASS Edit</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Category</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Mode</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Settings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Snapping in screen pixels</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Symbology</source>
        <translation type="unfinished">Символика</translation>
    </message>
    <message>
        <source>Column 1</source>
        <translation type="unfinished">Столбец 1</translation>
    </message>
    <message>
        <source>Table</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Add Column</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Create / Alter Table</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Line width</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Marker size</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Layer</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassElementDialog</name>
    <message>
        <source>Cancel</source>
        <translation>Отменить</translation>
    </message>
    <message>
        <source>Ok</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>&lt;font color=&apos;red&apos;&gt;Enter a name!&lt;/font&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;font color=&apos;red&apos;&gt;This is name of the source!&lt;/font&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;font color=&apos;red&apos;&gt;Exists!&lt;/font&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Overwrite</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassMapcalc</name>
    <message>
        <source>Mapcalc tools</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Add map</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Add constant value</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Add operator or function</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Add connection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Select item</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Delete selected item</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Open</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Save</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Save as</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Addition</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Subtraction</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Multiplication</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Division</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Modulus</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Exponentiation</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Equal</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Not equal</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Greater than</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Greater than or equal</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Less than</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Less than or equal</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>And</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Or</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Absolute value of x</source>
        <translation type="unfinished"></translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Natural log of x</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Log of x base b</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Largest value</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Median value</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Smallest value</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Mode value</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>1 if x is zero, 0 otherwise</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Current north-south resolution</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>NULL value</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Random value between a and b</source>
        <translation type="unfinished"></translation>
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
        <translation>Внимание</translation>
    </message>
    <message>
        <source>Cannot get current region</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot check region of map </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot get region of map </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>No GRASS raster maps currently in QGIS</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot create &apos;mapcalc&apos; directory in current mapset.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>New mapcalc</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Enter new mapcalc name:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Enter vector name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The file already exists. Overwrite? </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Save mapcalc</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>File name empty</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot open mapcalc file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The mapcalc schema (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>) not found.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot open mapcalc schema (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot read mapcalc schema (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>
at line </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> column </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Output</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassMapcalcBase</name>
    <message>
        <source>MainWindow</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Output</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassModule</name>
    <message>
        <source>Run</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Stop</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Module</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Warning</source>
        <translation type="unfinished">Внимание</translation>
    </message>
    <message>
        <source>The module file (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>) not found.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot open module file (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot read module file (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>):
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>
at line </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Module </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> not found</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot find man page </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Not available, cannot open description (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> column </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Not available, incorrect description (</source>
        <translation type="unfinished"></translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot start module: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;B&gt;Successfully finished&lt;/B&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;B&gt;Finished with error&lt;/B&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;B&gt;Module crashed or killed&lt;/B&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Not available, description not found (</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleBase</name>
    <message>
        <source>GRASS Module</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Options</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Output</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Manual</source>
        <translation type="unfinished">Ручная</translation>
    </message>
    <message>
        <source>Run</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Close</source>
        <translation type="unfinished">Закрыть</translation>
    </message>
    <message>
        <source>View output</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>TextLabel</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleField</name>
    <message>
        <source>Attribute field</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleFile</name>
    <message>
        <source>File</source>
        <translation type="unfinished">Файл</translation>
    </message>
    <message>
        <source>:&amp;nbsp;missing value</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>:&amp;nbsp;directory does not exist</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleGdalInput</name>
    <message>
        <source>Warning</source>
        <translation type="unfinished">Внимание</translation>
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
        <translation type="unfinished">Внимание</translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> not supported</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Use region of this map</source>
        <translation type="unfinished"></translation>
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
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleSelection</name>
    <message>
        <source>Attribute field</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleStandardOptions</name>
    <message>
        <source>Warning</source>
        <translation type="unfinished">Внимание</translation>
    </message>
    <message>
        <source>Cannot find module </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot start module </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot read module description (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>):
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>
at line </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> column </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot find key </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Item with id </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> not found</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot get current region</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot check region of map </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot set region of map </source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassNewMapset</name>
    <message>
        <source>GRASS database</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>GRASS location</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Projection</source>
        <translation>Проекция</translation>
    </message>
    <message>
        <source>Default GRASS Region</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Mapset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Create New Mapset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Tree</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Comment</source>
        <translation type="unfinished">Комментарий</translation>
    </message>
    <message>
        <source>Database</source>
        <translation type="unfinished">База данных</translation>
    </message>
    <message>
        <source>Location 2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>User&apos;s mapset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>System mapset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Location 1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Owner</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Enter path to GRASS database</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The directory doesn&apos;t exist!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>No writable locations, the database not writable!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Enter location name!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The location exists!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Selected projection is not supported by GRASS!</source>
        <translation>Выбранная проекция не поддерживается GRASS!</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Внимание</translation>
    </message>
    <message>
        <source>Cannot create projection.</source>
        <translation>Не удалось создать проекцию.</translation>
    </message>
    <message>
        <source>Cannot reproject previously set region, default region set.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>North must be greater than south</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>East must be greater than west</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Regions file (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>) not found.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot open locations file (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot read locations file (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>):
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>
at line </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> column </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot create QgsSpatialRefSys</source>
        <translation type="unfinished"></translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The mapset already exists</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Database: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Location: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Mapset: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Create location</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot create new location: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Create mapset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot open DEFAULT_WIND</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot open WIND</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>New mapset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>New mapset successfully created, but cannot be opened: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>New mapset successfully created and set as current working mapset.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot create new mapset directory</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassNewMapsetBase</name>
    <message>
        <source>Column 1</source>
        <translation type="unfinished">Столбец 1</translation>
    </message>
    <message>
        <source>Example directory tree:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;GRASS data are stored in tree directory structure.&lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;The GRASS database is the top-level directory in this tree structure.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Database Error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Database:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>...</source>
        <translation type="unfinished">...</translation>
    </message>
    <message>
        <source>Select existing directory or create a new one:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Location</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Select location</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Create new location</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Location Error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;The GRASS location is a collection of maps for a particular territory or project.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Projection Error</source>
        <translation>Ошибка проекции</translation>
    </message>
    <message>
        <source>Coordinate system</source>
        <translation>Система координат</translation>
    </message>
    <message>
        <source>Projection</source>
        <translation>Проекция</translation>
    </message>
    <message>
        <source>Not defined</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;The GRASS region defines a workspace for raster modules. The default region is valid for one location. It is possible to set a different region in each mapset. &lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;It is possible to change the default location region later.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Set current QGIS extent</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Set</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Region Error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>S</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>W</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>E</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>N</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>New mapset:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Mapset Error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;p align=&quot;center&quot;&gt;Existing masets&lt;/p&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;The GRASS mapset is a collection of maps used by one user. &lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;A user can read maps from all mapsets in the location but &lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;he can open for writing only his mapset (owned by user).&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Location:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Mapset:</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassPlugin</name>
    <message>
        <source>GRASS</source>
        <translation></translation>
    </message>
    <message>
        <source>&amp;GRASS</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Open mapset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>New mapset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Close mapset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Add GRASS vector layer</source>
        <translation>Добавить векторный слой GRASS</translation>
    </message>
    <message>
        <source>Add GRASS raster layer</source>
        <translation>Добавить растровый слой GRASS</translation>
    </message>
    <message>
        <source>Open GRASS tools</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Display Current Grass Region</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Edit Current Grass Region</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Edit Grass Vector layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Adds a GRASS vector layer to the map canvas</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Adds a GRASS raster layer to the map canvas</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Displays the current GRASS region as a rectangle on the map canvas</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Edit the current GRASS region</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Edit the currently selected GRASS vector layer.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>GrassVector</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>0.1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>GRASS layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Create new Grass Vector</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Warning</source>
        <translation type="unfinished">Внимание</translation>
    </message>
    <message>
        <source>GRASS Edit is already running.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>New vector name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot create new vector: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>New vector created but cannot be opened by data provider.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot start editing.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>GISDBASE, LOCATION_NAME or MAPSET is not set, cannot display current region.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot read current region: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot open the mapset. </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot close mapset. </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot close current mapset. </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot open GRASS mapset. </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Could not add raster layer: </source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassRegion</name>
    <message>
        <source>Warning</source>
        <translation type="unfinished">Внимание</translation>
    </message>
    <message>
        <source>GISDBASE, LOCATION_NAME or MAPSET is not set, cannot display current region.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot read current region: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot write region</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassRegionBase</name>
    <message>
        <source>GRASS Region Settings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>N</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>W</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>E</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>S</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>N-S Res</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Rows</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cols</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>E-W Res</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Color</source>
        <translation type="unfinished">Цвет</translation>
    </message>
    <message>
        <source>Width</source>
        <translation type="unfinished">Ширина</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Отменить</translation>
    </message>
</context>
<context>
    <name>QgsGrassSelect</name>
    <message>
        <source>Select GRASS Vector Layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Select GRASS Raster Layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Select GRASS mapcalc schema</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Select GRASS Mapset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Warning</source>
        <translation type="unfinished">Внимание</translation>
    </message>
    <message>
        <source>Cannot open vector on level 2 (topology not available).</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Choose existing GISDBASE</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Wrong GISDBASE, no locations available.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Wrong GISDBASE</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Select a map.</source>
        <translation type="unfinished"></translation>
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
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGrassSelectBase</name>
    <message>
        <source>Gisdbase</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Location</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Browse</source>
        <translation>Обзор</translation>
    </message>
    <message>
        <source>Mapset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Map name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>Select or type map name (wildcards &apos;*&apos; and &apos;?&apos; accepted for rasters)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Add GRASS Layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Отменить</translation>
    </message>
</context>
<context>
    <name>QgsGrassShellBase</name>
    <message>
        <source>GRASS Shell</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Close</source>
        <translation type="unfinished">Закрыть</translation>
    </message>
</context>
<context>
    <name>QgsGrassTools</name>
    <message>
        <source>Modules</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Browser</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>GRASS Tools</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>GRASS Tools: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Warning</source>
        <translation type="unfinished">Внимание</translation>
    </message>
    <message>
        <source>Cannot find MSYS (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>GRASS Shell is not compiled.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The config file (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>) not found.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot open config file (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cannot read config file (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>
at line </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> column </source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGridMakerPlugin</name>
    <message>
        <source>&amp;Graticule Creator</source>
        <translation>&amp;Построение сетки</translation>
    </message>
    <message>
        <source>Creates a graticule (grid) and stores the result as a shapefile</source>
        <translation>Построить сетку и сохранить результат в shape-файл</translation>
    </message>
    <message>
        <source>&amp;Graticules</source>
        <translation>Се&amp;тка</translation>
    </message>
</context>
<context>
    <name>QgsGridMakerPluginGui</name>
    <message>
        <source>QGIS - Grid Maker</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Please enter the file name before pressing OK!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Longitude Interval is invalid - please correct and try again.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Latitude Interval is invalid - please correct and try again.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Longitude Origin is invalid - please correct and try again..</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Latitude Origin is invalid - please correct and try again.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>End Point Longitude is invalid - please correct and try again.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>End Point Latitude is invalid - please correct and try again.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Choose a filename to save under</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>ESRI Shapefile (*.shp)</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsGridMakerPluginGuiBase</name>
    <message>
        <source>QGIS Plugin Template</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Graticule Builder</source>
        <translation>Построение сетки</translation>
    </message>
    <message>
        <source>#000.00000; </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Latitude:</source>
        <translation>Широта:</translation>
    </message>
    <message>
        <source>Longitude:</source>
        <translation>Долгота:</translation>
    </message>
    <message>
        <source>Latitude Interval:</source>
        <translation>Интервал по широте:</translation>
    </message>
    <message>
        <source>Longitude Interval:</source>
        <translation>Интервал по долготе:</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:Arial; font-size:11pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;This plugin will help you to build a graticule shapefile that you can use as an overlay within your qgis map viewer.&lt;/span&gt;&lt;/p&gt;&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:Arial; font-size:10pt;&quot;&gt;Please enter all units in decimal degrees&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:Arial; font-size:11pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;Этот модуль поможет вам построить shape-файл, содержащий сетку, которую вы можете наложить на карту.&lt;/span&gt;&lt;/p&gt;&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:Arial; font-size:10pt;&quot;&gt;Пожалуйста, вводите все значения в десятичных градусах&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Type</source>
        <translation>Тип</translation>
    </message>
    <message>
        <source>Point</source>
        <translation>Точка</translation>
    </message>
    <message>
        <source>Line</source>
        <translation>Линия</translation>
    </message>
    <message>
        <source>Polygon</source>
        <translation>Полигон</translation>
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
        <source>Graticle size (units in degrees)</source>
        <translation>Размер ячейки (единицы в градусах)</translation>
    </message>
    <message>
        <source>Output (shape) file</source>
        <translation>Выходной (shape) файл</translation>
    </message>
    <message>
        <source>Save As...</source>
        <translation>Сохранить как...</translation>
    </message>
</context>
<context>
    <name>QgsHelpViewer</name>
    <message>
        <source>Quantum GIS Help - </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Failed to get the help text from the database</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Error</source>
        <translation type="unfinished">Ошибка</translation>
    </message>
    <message>
        <source>The QGIS help database is not installed</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>This help file does not exist for your language</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>If you would like to create it, contact the QGIS development team</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Quantum GIS Help</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsHelpViewerBase</name>
    <message>
        <source>QGIS Help</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Home</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Alt+H</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Forward</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Alt+F</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Back</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Alt+B</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation type="unfinished">&amp;Закрыть</translation>
    </message>
    <message>
        <source>Alt+C</source>
        <translation type="unfinished"></translation>
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
        <source>Network timed out after %1 seconds of inactivity.
This may be a problem in your network connection or at the WMS server.</source>
        <translation type="obsolete">
        </translation>
    </message>
    <message>
        <source>HTTP transaction completed, however there was an error: %1</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsIdentifyResults</name>
    <message>
        <source>Identify Results - </source>
        <translation>Результат определения - </translation>
    </message>
    <message>
        <source>Feature</source>
        <translation>Объект</translation>
    </message>
    <message>
        <source>Value</source>
        <translation>Значение</translation>
    </message>
    <message>
        <source>Run action</source>
        <translation>Выполнить действие</translation>
    </message>
    <message>
        <source>(Derived)</source>
        <translation>(Выведенные)</translation>
    </message>
</context>
<context>
    <name>QgsIdentifyResultsBase</name>
    <message>
        <source>Identify Results</source>
        <translation>Результат определения</translation>
    </message>
    <message>
        <source>Help</source>
        <translation>Справка</translation>
    </message>
    <message>
        <source>F1</source>
        <translation></translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
</context>
<context>
    <name>QgsLUDialogBase</name>
    <message>
        <source>Enter class bounds</source>
        <translation>Введите границы класса</translation>
    </message>
    <message>
        <source>Lower value</source>
        <translation>Нижнее значение</translation>
    </message>
    <message>
        <source>-</source>
        <translation>‒</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Отменить</translation>
    </message>
    <message>
        <source>Upper value</source>
        <translation>Верхнее значение</translation>
    </message>
</context>
<context>
    <name>QgsLabelDialogBase</name>
    <message>
        <source>Form1</source>
        <translation></translation>
    </message>
    <message>
        <source>Field containing label:</source>
        <translation>Поле, содержащее подпись:</translation>
    </message>
    <message>
        <source>Default label:</source>
        <translation>Подпись по умолчанию:</translation>
    </message>
    <message>
        <source>Preview:</source>
        <translation>Предпросмотр:</translation>
    </message>
    <message>
        <source>QGIS Rocks!</source>
        <translation>QGIS работает!</translation>
    </message>
    <message>
        <source>Font Style</source>
        <translation>Стиль шрифта</translation>
    </message>
    <message>
        <source>Font</source>
        <translation>Шрифт</translation>
    </message>
    <message>
        <source>Points</source>
        <translation>Пункты</translation>
    </message>
    <message>
        <source>Map units</source>
        <translation>Единицы карты</translation>
    </message>
    <message>
        <source>%</source>
        <translation></translation>
    </message>
    <message>
        <source>Transparency:</source>
        <translation>Прозрачность:</translation>
    </message>
    <message>
        <source>Colour</source>
        <translation>Цвет</translation>
    </message>
    <message>
        <source>Position</source>
        <translation>Позиция</translation>
    </message>
    <message>
        <source>X Offset (pts):</source>
        <translation>Смещение по X (пункты):</translation>
    </message>
    <message>
        <source>Y Offset (pts):</source>
        <translation>Смещение по Y (пункты):</translation>
    </message>
    <message>
        <source>Buffer Labels?</source>
        <translation>Буферизовать подписи?</translation>
    </message>
    <message>
        <source>Size:</source>
        <translation>Размер:</translation>
    </message>
    <message>
        <source>Size is in map units</source>
        <translation>Единицы карты</translation>
    </message>
    <message>
        <source>Size is in points</source>
        <translation>Пункты</translation>
    </message>
    <message>
        <source>Above</source>
        <translation>Сверху</translation>
    </message>
    <message>
        <source>Over</source>
        <translation>Поверх</translation>
    </message>
    <message>
        <source>Left</source>
        <translation>Слева</translation>
    </message>
    <message>
        <source>Below</source>
        <translation>Внизу</translation>
    </message>
    <message>
        <source>Right</source>
        <translation>Справа</translation>
    </message>
    <message>
        <source>Above Right</source>
        <translation>Сверху справа</translation>
    </message>
    <message>
        <source>Below Right</source>
        <translation>Внизу справа</translation>
    </message>
    <message>
        <source>Above Left</source>
        <translation>Сверху слева</translation>
    </message>
    <message>
        <source>Below Left</source>
        <translation>Внизу слева</translation>
    </message>
    <message>
        <source>Angle (deg):</source>
        <translation>Угол (град):</translation>
    </message>
    <message encoding="UTF-8">
        <source>°</source>
        <translation></translation>
    </message>
    <message>
        <source>Data Defined Style</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Font family:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Italic:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Underline:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Bold:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Size:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>X Coordinate:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Y Coordinate:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Placement:</source>
        <translation>Размещение:</translation>
    </message>
    <message>
        <source>Font size units</source>
        <translation>Единицы размера шрифта</translation>
    </message>
    <message>
        <source>Font Alignment</source>
        <translation>Выравнивание</translation>
    </message>
    <message>
        <source>Placement</source>
        <translation>Размещение</translation>
    </message>
    <message>
        <source>Buffer</source>
        <translation>Буферизация</translation>
    </message>
    <message>
        <source>Buffer size units</source>
        <translation>Единицы размера буфера</translation>
    </message>
    <message>
        <source>Offset units</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Data Defined Alignment</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Data Defined Buffer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Data Defined Position</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Source</source>
        <translation>Источник</translation>
    </message>
    <message>
        <source>Size Units:</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsLayerProjectionSelector</name>
    <message>
        <source>Define this layer&apos;s projection:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>This layer appears to have no projection specification.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>By default, this layer will now have its projection set to that of the project, but you may override this by selecting a different projection below.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsLayerProjectionSelectorBase</name>
    <message>
        <source>Layer Projection Selector</source>
        <translation>Выбор проекции слоя</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:12pt; font-weight:600;&quot;&gt;Define this layer&apos;s projection:&lt;/span&gt;&lt;/p&gt;&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;This layer appears to have no projection specification. By default, this layer will now have its projection set to that of the project, but you may override this by selecting a different projection below.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:12pt; font-weight:600;&quot;&gt;Укажите проекцию данного слоя:&lt;/span&gt;&lt;/p&gt;&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Этот слой не содержит сведений о проекции. По умолчанию, для него будет назначена проекция, заданная в текущем проекте, но вы можете изменить её путём выбора иной проекции в нижеприведённом списке.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
</context>
<context>
    <name>QgsLegend</name>
    <message>
        <source>group</source>
        <translation>группа</translation>
    </message>
    <message>
        <source>&amp;Properties</source>
        <translation type="obsolete">&amp;Свойства</translation>
    </message>
    <message>
        <source>&amp;Remove</source>
        <translation>&amp;Удалить</translation>
    </message>
    <message>
        <source>&amp;Make to toplevel item</source>
        <translation>Сделать элементом &amp;первого уровня</translation>
    </message>
    <message>
        <source>Re&amp;name</source>
        <translation>Переи&amp;меновать</translation>
    </message>
    <message>
        <source>&amp;Add group</source>
        <translation>&amp;Добавить группу</translation>
    </message>
    <message>
        <source>&amp;Expand all</source>
        <translation>&amp;Развернуть все</translation>
    </message>
    <message>
        <source>&amp;Collapse all</source>
        <translation>&amp;Свернуть все</translation>
    </message>
    <message>
        <source>Show file groups</source>
        <translation>Показать группы файлов</translation>
    </message>
    <message>
        <source>&amp;Open attribute table</source>
        <translation type="obsolete">&amp;Открыть таблицу атрибутов</translation>
    </message>
    <message>
        <source>No Layer Selected</source>
        <translation>Слой не выбран</translation>
    </message>
    <message>
        <source>To open an attribute table, you must select a vector layer in the legend</source>
        <translation>Для открытия таблицы атрибутов, следует выбрать в легенде векторный слой</translation>
    </message>
</context>
<context>
    <name>QgsLegendLayer</name>
    <message>
        <source>&amp;Zoom to layer extent</source>
        <translation>&amp;Увеличить до границ слоя</translation>
    </message>
    <message>
        <source>&amp;Zoom to best scale (100%)</source>
        <translation>&amp;Увеличить до наилучшего масштаба (100%)</translation>
    </message>
    <message>
        <source>&amp;Show in overview</source>
        <translation>&amp;Показать в обзоре</translation>
    </message>
    <message>
        <source>&amp;Remove</source>
        <translation>&amp;Удалить</translation>
    </message>
    <message>
        <source>&amp;Open attribute table</source>
        <translation>&amp;Открыть таблицу атрибутов</translation>
    </message>
    <message>
        <source>Save as shapefile...</source>
        <translation>Сохранить как shape-файл...</translation>
    </message>
    <message>
        <source>Save selection as shapefile...</source>
        <translation>Сохранить выделение как shape-файл...</translation>
    </message>
    <message>
        <source>&amp;Properties</source>
        <translation>&amp;Свойства</translation>
    </message>
    <message>
        <source>More layers</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>This item contains more layer files. Displaying more layers in table is not supported.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsLegendLayerFile</name>
    <message>
        <source>Attribute table - </source>
        <translation>Таблица атрибутов - </translation>
    </message>
    <message>
        <source>Save layer as...</source>
        <translation>Сохранить слой как...</translation>
    </message>
    <message>
        <source>Start editing failed</source>
        <translation>Не удалось начать редактирование</translation>
    </message>
    <message>
        <source>Provider cannot be opened for editing</source>
        <translation>Источник не может быть открыт для редактирования</translation>
    </message>
    <message>
        <source>Stop editing</source>
        <translation type="unfinished">Прекратить редактирование</translation>
    </message>
    <message>
        <source>Do you want to save the changes?</source>
        <translation>Вы хотите сохранить изменения?</translation>
    </message>
    <message>
        <source>Error</source>
        <translation type="unfinished">Ошибка</translation>
    </message>
    <message>
        <source>Could not commit changes</source>
        <translation>Не удалось внести изменения</translation>
    </message>
    <message>
        <source>Problems during roll back</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Not a vector layer</source>
        <translation type="unfinished">Слой не является векторным</translation>
    </message>
    <message>
        <source>To open an attribute table, you must select a vector layer in the legend</source>
        <translation type="unfinished">Для открытия таблицы атрибутов, следует выбрать в легенде векторный слой</translation>
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
        <translation>Драйвер не найден</translation>
    </message>
    <message>
        <source>ESRI Shapefile driver is not available</source>
        <translation>Драйвер shape-файлов ESRI не доступен</translation>
    </message>
    <message>
        <source>Error creating shapefile</source>
        <translation>Ошибка создания shape-файла</translation>
    </message>
    <message>
        <source>The shapefile could not be created (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Layer creation failed</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Zoom to layer extent</source>
        <translation>&amp;Увеличить до границ слоя</translation>
    </message>
    <message>
        <source>&amp;Show in overview</source>
        <translation>&amp;Показать в обзоре</translation>
    </message>
    <message>
        <source>&amp;Remove</source>
        <translation>&amp;Удалить</translation>
    </message>
    <message>
        <source>&amp;Open attribute table</source>
        <translation>&amp;Открыть таблицу атрибутов</translation>
    </message>
    <message>
        <source>Save as shapefile...</source>
        <translation>Сохранить как shape-файл...</translation>
    </message>
    <message>
        <source>Save selection as shapefile...</source>
        <translation>Сохранить выделение как shape-файл...</translation>
    </message>
    <message>
        <source>&amp;Properties</source>
        <translation>&amp;Свойства</translation>
    </message>
    <message>
        <source>bad_alloc exception</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Filling the attribute table has been stopped because there was no more virtual memory left</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Layer attribute table contains unsupported datatype(s)</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsLineStyleDialogBase</name>
    <message>
        <source>Select a line style</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Styles</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Ok</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Отменить</translation>
    </message>
</context>
<context>
    <name>QgsLineStyleWidgetBase</name>
    <message>
        <source>Form2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Outline Style</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Width:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Colour:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>LineStyleWidget</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>col</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsLocationCaptureWidgetBase</name>
    <message>
        <source>Pan</source>
        <translation type="obsolete">Перевезти</translation>
    </message>
</context>
<context>
    <name>QgsMapCanvas</name>
    <message>
        <source>Could not draw</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>because</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsMapLayer</name>
    <message>
        <source>&amp;Zoom to extent of selected layer</source>
        <translation type="obsolete">&amp;Изменить вид до полной данного слоя</translation>
    </message>
    <message>
        <source>&amp;Remove</source>
        <translation type="obsolete">&amp;Удалить</translation>
    </message>
    <message>
        <source>&amp;Properties</source>
        <translation type="obsolete">&amp;Свойства</translation>
    </message>
    <message>
        <source> Check file permissions and retry.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsMapToolIdentify</name>
    <message>
        <source>No features found</source>
        <translation type="unfinished">Объектов не найдено</translation>
    </message>
    <message>
        <source>&lt;p&gt;No features were found within the search radius. Note that it is currently not possible to use the identify tool on unsaved features.&lt;/p&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>- %1 features found</source>
        <comment>Identify results window title</comment>
        <translation type="obsolete">
        </translation>
    </message>
    <message>
        <source>(clicked coordinate)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>WMS identify result for %1
%2</source>
        <translation type="unfinished"></translation>
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
</context>
<context>
    <name>QgsMapToolVertexEdit</name>
    <message>
        <source>Snap tolerance</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Don&apos;t show this message again</source>
        <translation type="unfinished">Не показывать это сообщение в дальнейшем</translation>
    </message>
    <message>
        <source>Could not snap segment.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Have you set the tolerance in Settings &gt; Project Properties &gt; General?</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsMapserverExport</name>
    <message>
        <source>Name for the map file</source>
        <translation>Имя файла карты</translation>
    </message>
    <message>
        <source>Choose the QGIS project file</source>
        <translation>Выберите файл проекта QGIS</translation>
    </message>
    <message>
        <source>QGIS Project Files (*.qgs);;All files (*.*)</source>
        <comment>Filter list for selecting files from a dialog box</comment>
        <translation>Файлы проектов QGIS (*.qgs);;Все файлы (*.*)</translation>
    </message>
    <message>
        <source>Overwrite File?</source>
        <translation>Перезаписать файл?</translation>
    </message>
    <message>
        <source> exists. 
Do you want to overwrite it?</source>
        <comment>a filename is prepended to this text, and appears in a dialog box</comment>
        <translation> уже существует. 
Вы хотите перезаписать этот файл?</translation>
    </message>
    <message>
        <source>Yes</source>
        <translation type="obsolete">Да</translation>
    </message>
    <message>
        <source>No</source>
        <translation type="obsolete">Нет</translation>
    </message>
    <message>
        <source> exists. 
Do you want to overwrite it?</source>
        <translation> уже существует. Вы хотите перезаписать этот файл?</translation>
    </message>
    <message>
        <source>MapServer map files (*.map);;All files (*.*)</source>
        <comment>Filter list for selecting files from a dialog box</comment>
        <translation>Файлы карт MapServer (*.map);;Все файлы (*.*)</translation>
    </message>
</context>
<context>
    <name>QgsMapserverExportBase</name>
    <message>
        <source>Export to Mapserver</source>
        <translation>Экспорт в Mapserver</translation>
    </message>
    <message>
        <source>Map file</source>
        <translation>Файл карты</translation>
    </message>
    <message>
        <source>Export LAYER information only</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Map</source>
        <translation>Карта</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Имя</translation>
    </message>
    <message>
        <source>Height</source>
        <translation>Высота</translation>
    </message>
    <message>
        <source>Width</source>
        <translation>Ширина</translation>
    </message>
    <message>
        <source>dd</source>
        <translation>десятичные градусы</translation>
    </message>
    <message>
        <source>feet</source>
        <translation>футы</translation>
    </message>
    <message>
        <source>meters</source>
        <translation>метры</translation>
    </message>
    <message>
        <source>miles</source>
        <translation>мили</translation>
    </message>
    <message>
        <source>inches</source>
        <translation>дюймы</translation>
    </message>
    <message>
        <source>kilometers</source>
        <translation>километры</translation>
    </message>
    <message>
        <source>Units</source>
        <translation>Единицы</translation>
    </message>
    <message>
        <source>Image type</source>
        <translation>Формат изображения</translation>
    </message>
    <message>
        <source>gif</source>
        <translation></translation>
    </message>
    <message>
        <source>gtiff</source>
        <translation></translation>
    </message>
    <message>
        <source>jpeg</source>
        <translation></translation>
    </message>
    <message>
        <source>png</source>
        <translation></translation>
    </message>
    <message>
        <source>swf</source>
        <translation></translation>
    </message>
    <message>
        <source>userdefined</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>wbmp</source>
        <translation></translation>
    </message>
    <message>
        <source>MinScale</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>MaxScale</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Prefix attached to map, scalebar and legend GIF filenames created using this MapFile. It should be kept short.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Web Interface Definition</source>
        <translation>Определение Web-интерфейса</translation>
    </message>
    <message>
        <source>Header</source>
        <translation>Верхний колонтитул</translation>
    </message>
    <message>
        <source>Footer</source>
        <translation>Нижний колонтитул</translation>
    </message>
    <message>
        <source>Template</source>
        <translation>Шаблон</translation>
    </message>
    <message>
        <source>&amp;Help</source>
        <translation>&amp;Справка</translation>
    </message>
    <message>
        <source>F1</source>
        <translation></translation>
    </message>
    <message>
        <source>&amp;OK</source>
        <translation>&amp;OK</translation>
    </message>
    <message>
        <source>&amp;Cancel</source>
        <translation>О&amp;тменить</translation>
    </message>
    <message>
        <source>...</source>
        <translation type="unfinished">...</translation>
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
        <translation>Полный путь к файлу проекта QGIS, экспортируемому в формат MapServer</translation>
    </message>
    <message>
        <source>QGIS project file</source>
        <translation>Файл проекта QGIS</translation>
    </message>
    <message>
        <source>Browse...</source>
        <translation>Обзор...</translation>
    </message>
    <message>
        <source>Save As...</source>
        <translation>Сохранить как...</translation>
    </message>
</context>
<context>
    <name>QgsMarkerDialogBase</name>
    <message>
        <source>Choose a marker symbol</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Directory</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>...</source>
        <translation type="unfinished">...</translation>
    </message>
    <message>
        <source>Ok</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Отменить</translation>
    </message>
    <message>
        <source>New Item</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsMeasureBase</name>
    <message>
        <source>Measure</source>
        <translation>Измерение</translation>
    </message>
    <message>
        <source>Help</source>
        <translation>Справка</translation>
    </message>
    <message>
        <source>New</source>
        <translation>Новое</translation>
    </message>
    <message>
        <source>Cl&amp;ose</source>
        <translation>&amp;Закрыть</translation>
    </message>
    <message>
        <source>Total:</source>
        <translation>Всего:</translation>
    </message>
</context>
<context>
    <name>QgsMeasureDialog</name>
    <message>
        <source>Segments (in meters)</source>
        <translation>Сегменты (в метрах)</translation>
    </message>
    <message>
        <source>Segments (in feet)</source>
        <translation>Сегменты (в футах)</translation>
    </message>
    <message>
        <source>Segments (in degrees)</source>
        <translation>Сегменты (в градусах)</translation>
    </message>
    <message>
        <source>Segments</source>
        <translation>Сегменты</translation>
    </message>
</context>
<context>
    <name>QgsMeasureTool</name>
    <message>
        <source>Incorrect measure results</source>
        <translation>Неверный результат измерения</translation>
    </message>
    <message>
        <source>&lt;p&gt;This map is defined with a geographic coordinate system (latitude/longitude) but the map extents suggests that it is actually a projected coordinate system (e.g., Mercator). If so, the results from line or area measurements will be incorrect.&lt;/p&gt;&lt;p&gt;To fix this, explicitly set an appropriate map coordinate system using the &lt;tt&gt;Settings:Project Properties&lt;/tt&gt; menu.</source>
        <translation>&lt;p&gt;Эта карта задана в географической системе координат (широта/долгота), но по границам карты можно предположить, что на самом деле используется прямоугольная система координат (например, Меркатора). В этом случае, результаты измерения линий и площадей будут неверными.&lt;/p&gt;&lt;p&gt;Для устранения этой ошибки следует явно задать система координат карты в меню&lt;tt&gt;Установки:Свойства проекта&lt;/tt&gt;.</translation>
    </message>
</context>
<context>
    <name>QgsMessageViewer</name>
    <message>
        <source>QGIS Message</source>
        <translation>Сообщение QGIS</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
    <message>
        <source>Don&apos;t show this message again</source>
        <translation>Не показывать это сообщение в дальнейшем</translation>
    </message>
</context>
<context>
    <name>QgsMySQLProvider</name>
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
        <source>No GEOS Support!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Your PostGIS installation has no GEOS support.
Feature selection and identification will not work properly.
Please install PostGIS with GEOS support (http://geos.refractions.net)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Save layer as...</source>
        <translation type="unfinished">Сохранить слой как...</translation>
    </message>
    <message>
        <source>Error</source>
        <translation type="unfinished">Ошибка</translation>
    </message>
    <message>
        <source>Error creating field </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Layer creation failed</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Error creating shapefile</source>
        <translation type="unfinished">Ошибка создания shape-файла</translation>
    </message>
    <message>
        <source>The shapefile could not be created (</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Driver not found</source>
        <translation>Драйвер не найден</translation>
    </message>
    <message>
        <source> driver is not available</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsNewConnection</name>
    <message>
        <source>Test connection</source>
        <translation type="unfinished">Проверить соединение</translation>
    </message>
    <message>
        <source>Connection failed - Check settings and try again.

Extended error information:
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Connection to %1 was successful</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsNewConnectionBase</name>
    <message>
        <source>Create a New PostGIS connection</source>
        <translation>Создать новое PostGIS-соединение</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Отменить</translation>
    </message>
    <message>
        <source>Help</source>
        <translation>Справка</translation>
    </message>
    <message>
        <source>Connection Information</source>
        <translation>Информация о соединении</translation>
    </message>
    <message>
        <source>Host</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Database</source>
        <translation>База данных</translation>
    </message>
    <message>
        <source>Username</source>
        <translation>Имя пользователя</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Имя</translation>
    </message>
    <message>
        <source>Name of the new connection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Password</source>
        <translation>Пароль</translation>
    </message>
    <message>
        <source>Test Connect</source>
        <translation>Проверить соединение</translation>
    </message>
    <message>
        <source>Save Password</source>
        <translation>Сохранить пароль</translation>
    </message>
    <message>
        <source>F1</source>
        <translation></translation>
    </message>
    <message>
        <source>Port</source>
        <translation>Порт</translation>
    </message>
    <message>
        <source>5432</source>
        <translation></translation>
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
        <source>Create a New WMS connection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Connection Information</source>
        <translation>Информация о соединении</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Имя</translation>
    </message>
    <message>
        <source>URL</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Proxy Host</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Proxy Port</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Proxy User</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Proxy Password</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Your user name for the HTTP proxy (optional)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Password for your HTTP proxy (optional)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Name of the new connection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>HTTP address of the Web Map Server</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Name of your HTTP proxy (optional)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Port number of your HTTP proxy (optional)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Отменить</translation>
    </message>
    <message>
        <source>Help</source>
        <translation>Справка</translation>
    </message>
    <message>
        <source>F1</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>QgsNorthArrowPlugin</name>
    <message>
        <source>Bottom Left</source>
        <translation>Внизу слева</translation>
    </message>
    <message>
        <source>Top Right</source>
        <translation>Вверху справа</translation>
    </message>
    <message>
        <source>Bottom Right</source>
        <translation>Внизу справа</translation>
    </message>
    <message>
        <source>Top Left</source>
        <translation>Вверху слева</translation>
    </message>
    <message>
        <source>&amp;North Arrow</source>
        <translation>Указатель «&amp;север-юг»</translation>
    </message>
    <message>
        <source>Creates a north arrow that is displayed on the map canvas</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Decorations</source>
        <translation>&amp;Декораторы</translation>
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
        <translation>Изображение не найдено</translation>
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
        <translation>Свойства</translation>
    </message>
    <message>
        <source>Angle</source>
        <translation>Угол</translation>
    </message>
    <message>
        <source>Placement</source>
        <translation>Размещение</translation>
    </message>
    <message>
        <source>Set direction automatically</source>
        <translation>Задать направление автоматически</translation>
    </message>
    <message>
        <source>Enable North Arrow</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Top Left</source>
        <translation>Вверху слева</translation>
    </message>
    <message>
        <source>Top Right</source>
        <translation>Вверху справа</translation>
    </message>
    <message>
        <source>Bottom Left</source>
        <translation>Внизу слева</translation>
    </message>
    <message>
        <source>Bottom Right</source>
        <translation>Внизу справа</translation>
    </message>
    <message>
        <source>Placement on screen</source>
        <translation>Размещение на экране</translation>
    </message>
    <message>
        <source>Preview of north arrow</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Icon</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>New Item</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Browse...</source>
        <translation>Обзор...</translation>
    </message>
</context>
<context>
    <name>QgsOGRFactory</name>
    <message>
        <source>Wrong Path/URI</source>
        <translation>Неверный путь/URI</translation>
    </message>
    <message>
        <source>The provided path for the dataset is not valid.</source>
        <translation>Указанный путь к набору данных недействителен.</translation>
    </message>
</context>
<context>
    <name>QgsOptions</name>
    <message>
        <source>Detected active locale on your system: </source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsOptionsBase</name>
    <message>
        <source>QGIS Options</source>
        <translation>Параметры QGIS</translation>
    </message>
    <message>
        <source>epiphany</source>
        <translation></translation>
    </message>
    <message>
        <source>galeon</source>
        <translation></translation>
    </message>
    <message>
        <source>konqueror</source>
        <translation></translation>
    </message>
    <message>
        <source>mozilla</source>
        <translation></translation>
    </message>
    <message>
        <source>opera</source>
        <translation></translation>
    </message>
    <message>
        <source>Search Radius for Identifying Features</source>
        <translation type="obsolete">Радиус поиска для определения объектов</translation>
    </message>
    <message>
        <source>Hide splash screen at startup</source>
        <translation>Не показывать заставку при запуске</translation>
    </message>
    <message>
        <source>&amp;Appearance</source>
        <translation>&amp;Внешний вид</translation>
    </message>
    <message>
        <source>&amp;Icon Theme</source>
        <translation>&amp;Тема значков</translation>
    </message>
    <message>
        <source>Theme</source>
        <translation>Тема</translation>
    </message>
    <message>
        <source>&lt;b&gt;Note: &lt;/b&gt;Theme changes take effect the next time QGIS is started</source>
        <translation>&lt;b&gt;Внимание: &lt;/b&gt;Изменение темы вступит в силу при следующем запуске QGIS</translation>
    </message>
    <message>
        <source>Help &amp;Browser</source>
        <translation>&amp;Браузер</translation>
    </message>
    <message>
        <source>Open help documents with</source>
        <translation>Открывать справочную документацию в</translation>
    </message>
    <message>
        <source>&amp;Rendering</source>
        <translation>От&amp;рисовка</translation>
    </message>
    <message>
        <source>Update display after reading</source>
        <translation>Обновлять изображение после загрузки</translation>
    </message>
    <message>
        <source>Map display will be updated (drawn) after this many features have been read from the data source</source>
        <translation>Изображение карты будет обновлено (перерисовано) после того, как это количество объектов загружено из источника данных</translation>
    </message>
    <message>
        <source>features</source>
        <translation>объектов</translation>
    </message>
    <message>
        <source>(Set to 0 to not update the display until all features have been read)</source>
        <translation>(Установите в 0, чтобы отключить обновление до загрузки всех объектов)</translation>
    </message>
    <message>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <source>Initial Visibility</source>
        <translation>Начальная видимость</translation>
    </message>
    <message>
        <source>Select Global Default ...</source>
        <translation>Выбрать глобальную проекцию по умолчанию...</translation>
    </message>
    <message>
        <source>Prompt for projection.</source>
        <translation>Запрашивать ввод проекции.</translation>
    </message>
    <message>
        <source>Project wide default projection will be used.</source>
        <translation>Использовать значение по умолчанию для данного проекта.</translation>
    </message>
    <message>
        <source>firefox</source>
        <translation></translation>
    </message>
    <message>
        <source>mozilla-firefox</source>
        <translation></translation>
    </message>
    <message>
        <source>&amp;Splash screen</source>
        <translation>Экран-&amp;заставка</translation>
    </message>
    <message>
        <source>Default Map Appearance (Overridden by project properties)</source>
        <translation>Вид карты по умолчанию (заменяется свойствами проекта)</translation>
    </message>
    <message>
        <source>Background Color:</source>
        <translation>Цвет фона:</translation>
    </message>
    <message>
        <source>Selection Color:</source>
        <translation>Цвет выделения:</translation>
    </message>
    <message>
        <source>Appearance</source>
        <translation>Внешний вид</translation>
    </message>
    <message>
        <source>Capitalise layer name</source>
        <translation>Выводить имя слоя с заглавной буквы</translation>
    </message>
    <message>
        <source>Make lines appear less jagged at the expense of some drawing performance</source>
        <translation>Рисовать сглаженные линии (снижает скорость отрисовки)</translation>
    </message>
    <message>
        <source>By default new la&amp;yers added to the map should be displayed</source>
        <translation>Добавляемые на карту слои &amp;видимы по умолчанию</translation>
    </message>
    <message>
        <source>&amp;Update during drawing</source>
        <translation>&amp;Обновление при отрисовке</translation>
    </message>
    <message>
        <source>Measure tool</source>
        <translation>Инструмент измерений</translation>
    </message>
    <message>
        <source>Ellipsoid for distance calculations:</source>
        <translation>Эллипсоид для вычисления расстояний:</translation>
    </message>
    <message>
        <source>Search radius</source>
        <translation>Радиус поиска</translation>
    </message>
    <message>
        <source>Pro&amp;jection</source>
        <translation>&amp;Проекция</translation>
    </message>
    <message>
        <source>When layer is loaded that has no projection information</source>
        <translation>Если загружаемый слой не содержит данных о проекции</translation>
    </message>
    <message>
        <source>Global default projection displa&amp;yed below will be used.</source>
        <translation>Использовать ниж&amp;еприведённую глобальную проекцию по умолчанию.</translation>
    </message>
    <message>
        <source>&lt;b&gt;Note:&lt;/b&gt; The browser must be in your PATH or you can specify the full path above</source>
        <translation>&lt;b&gt;Внимание:&lt;/b&gt; Браузер должен находится в списке каталогов PATH, в противном случае укажите полный путь к программе</translation>
    </message>
    <message>
        <source>Rendering</source>
        <translation>Отрисовка</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Selecting this will unselect the &apos;make lines less&apos; jagged toggle&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Выбор этого параметра автоматически снимает флажок «Рисовать сглаженные линии»&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Fix problems with incorrectly filled polygons</source>
        <translation>Исправлять ошибки некорректной заливки полигонов</translation>
    </message>
    <message>
        <source>Continuously redraw the map when dragging the legend/map divider</source>
        <translation>Обновлять карту при перемещении разделителя карты/легенды</translation>
    </message>
    <message>
        <source>&amp;Map tools</source>
        <translation>&amp;Инструменты</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-weight:600;&quot;&gt;Note:&lt;/span&gt; Specify the search radius as a percentage of the map width.&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-weight:600;&quot;&gt;Внимание:&lt;/span&gt; Указывайте радиус поиска в процентах от ширины видимой карты.&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>%</source>
        <translation></translation>
    </message>
    <message>
        <source>Panning and zooming</source>
        <translation>Панорамирование и масштабирование</translation>
    </message>
    <message>
        <source>Zoom</source>
        <translation>Увеличить</translation>
    </message>
    <message>
        <source>Zoom and recenter</source>
        <translation>Увеличить и центрировать</translation>
    </message>
    <message>
        <source>Nothing</source>
        <translation>Ничего</translation>
    </message>
    <message>
        <source>Zoom factor:</source>
        <translation>Фактор увеличения:</translation>
    </message>
    <message>
        <source>Mouse wheel action:</source>
        <translation>Действие при прокрутке колеса мыши:</translation>
    </message>
    <message>
        <source>&amp;General</source>
        <translation>&amp;Общие</translation>
    </message>
    <message>
        <source>General</source>
        <translation>Общие</translation>
    </message>
    <message>
        <source>Ask to save project changes when required</source>
        <translation>Запрашивать сохранение изменений в проекте, когда это необходимо</translation>
    </message>
    <message>
        <source>Rubberband color:</source>
        <translation>Цвет линии:</translation>
    </message>
    <message>
        <source>Locale</source>
        <translation>Локаль</translation>
    </message>
    <message>
        <source>Force Override System Locale</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Locale to use instead</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Note: Enabling / changing overide on local requires an application restart.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Additional Info</source>
        <translation>Дополнительная информация</translation>
    </message>
    <message>
        <source>Detected active locale on your system:</source>
        <translation>Найденная активная системная локаль:</translation>
    </message>
    <message>
        <source>Warn me when opening a project file saved with an older version of QGIS</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Selecting this will unselect the &apos;make lines less&apos; jagged toggle</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>(Specify the search radius as a percentage of the map width)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Search Radius for Identifying Features and displaying Map Tips</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Digitizing</source>
        <translation type="unfinished">Оцифровка</translation>
    </message>
    <message>
        <source>Rubberband</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Line Width:</source>
        <translation type="unfinished">Ширина линии:</translation>
    </message>
    <message>
        <source>Line width in pixels</source>
        <translation type="unfinished">Ширина линии в пикселях</translation>
    </message>
    <message>
        <source>Line Colour:</source>
        <translation type="unfinished">Цвет линии:</translation>
    </message>
    <message>
        <source>Snapping</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Default Snapping Tolerance (in layer units):</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Search radius for vertex edits (in layer units):</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsPasteTransformationsBase</name>
    <message>
        <source>Paste Transformations</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;b&gt;Note: This function is not useful yet!&lt;/b&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Source</source>
        <translation type="unfinished">Источник</translation>
    </message>
    <message>
        <source>Destination</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Help</source>
        <translation>&amp;Справка</translation>
    </message>
    <message>
        <source>F1</source>
        <translation></translation>
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
        <translation>О&amp;тменить</translation>
    </message>
</context>
<context>
    <name>QgsPatternDialogBase</name>
    <message>
        <source>Select a fill pattern</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Отменить</translation>
    </message>
    <message>
        <source>Ok</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>No Fill</source>
        <translation type="unfinished">Без заливки</translation>
    </message>
</context>
<context>
    <name>QgsPgGeoprocessing</name>
    <message>
        <source>Buffer features in layer %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Error connecting to the database</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Buffer features</source>
        <translation>&amp;Буферизация объектов</translation>
    </message>
    <message>
        <source>A new layer is created in the database with the buffered features.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Geoprocessing</source>
        <translation>&amp;Обработка данных</translation>
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
        <translation>Нет активного слоя</translation>
    </message>
    <message>
        <source>You must select a layer in the legend to buffer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Not a PostgreSQL/PostGIS Layer</source>
        <translation>Слой не в формате PostgreSQL/PostGIS</translation>
    </message>
    <message>
        <source> is not a PostgreSQL/PostGIS layer.
</source>
        <translation> не является слоем PostgreSQL/PostGIS.
</translation>
    </message>
    <message>
        <source>Geoprocessing functions are only available for PostgreSQL/PostGIS Layers</source>
        <translation>Функции обработки данных доступны только для слоёв PostgreSQL/PostGIS</translation>
    </message>
</context>
<context>
    <name>QgsPgQueryBuilder</name>
    <message>
        <source>Table &lt;b&gt;%1&lt;/b&gt; in database &lt;b&gt;%2&lt;/b&gt; on host &lt;b&gt;%3&lt;/b&gt;, user &lt;b&gt;%4&lt;/b&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Connection Failed</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Connection to the database failed:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Database error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Failed to get sample of field values</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Query Result</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The where clause returned </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> rows.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Query Failed</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>An error occurred when executing the query:</source>
        <translation>При выполнении запроса возникла ошибка:</translation>
    </message>
    <message>
        <source>No Records</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The query you specified results in zero records being returned. Valid PostgreSQL layers must have at least one feature.</source>
        <translation type="unfinished"></translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Clear</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Test</source>
        <translation type="unfinished">Проверка</translation>
    </message>
    <message>
        <source>Ok</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Отменить</translation>
    </message>
    <message>
        <source>Values</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>All</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Sample</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Fields</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Datasource:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Operators</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>=</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>IN</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>NOT IN</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>%</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;=</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&gt;=</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>!=</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>LIKE</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>AND</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>ILIKE</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>OR</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>NOT</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>SQL where clause</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsPluginManager</name>
    <message>
        <source>No Plugins</source>
        <translation>Модулей не найдено</translation>
    </message>
    <message>
        <source>No QGIS plugins found in </source>
        <translation>Модули QGIS не найдены в </translation>
    </message>
    <message>
        <source>Name</source>
        <translation type="unfinished">Имя</translation>
    </message>
    <message>
        <source>Version</source>
        <translation type="unfinished">Версия</translation>
    </message>
    <message>
        <source>Description</source>
        <translation type="unfinished">Описание</translation>
    </message>
    <message>
        <source>Library name</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsPluginManagerBase</name>
    <message>
        <source>QGIS Plugin Manger</source>
        <translation type="obsolete">Управление модулями QGIS</translation>
    </message>
    <message>
        <source>Name</source>
        <translation type="obsolete">Имя</translation>
    </message>
    <message>
        <source>Description</source>
        <translation type="obsolete">Описание</translation>
    </message>
    <message>
        <source>Library Name</source>
        <translation type="obsolete">Имя библиотеки</translation>
    </message>
    <message>
        <source>Ok</source>
        <translation type="obsolete">Да</translation>
    </message>
    <message>
        <source>Plugin Directory</source>
        <translation>Каталог модулей</translation>
    </message>
    <message>
        <source>To load a plugin, click the checkbox next to the plugin and click Ok</source>
        <translation>Для загрузки модуля, активируйте сооветствующий ему флажок и нажмите OK</translation>
    </message>
    <message>
        <source>Version</source>
        <translation type="obsolete">Версия</translation>
    </message>
    <message>
        <source>&amp;Select All</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Alt+S</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>C&amp;lear All</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Alt+L</source>
        <translation type="unfinished">Alt+P</translation>
    </message>
    <message>
        <source>&amp;Ok</source>
        <translation>&amp;OK</translation>
    </message>
    <message>
        <source>Alt+O</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation type="unfinished">&amp;Закрыть</translation>
    </message>
    <message>
        <source>Alt+C</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>QGIS Plugin Manager</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsPointDialog</name>
    <message>
        <source>Zoom In</source>
        <translation>Увеличить</translation>
    </message>
    <message>
        <source>z</source>
        <translation></translation>
    </message>
    <message>
        <source>Zoom Out</source>
        <translation>Уменьшить</translation>
    </message>
    <message>
        <source>Z</source>
        <translation></translation>
    </message>
    <message>
        <source>Zoom To Layer</source>
        <translation>Увеличить до слоя</translation>
    </message>
    <message>
        <source>Zoom to Layer</source>
        <translation>Увеличить до слоя</translation>
    </message>
    <message>
        <source>Pan Map</source>
        <translation type="unfinished">Панорамировать карту</translation>
    </message>
    <message>
        <source>Pan the map</source>
        <translation type="unfinished">Панорамировать карту</translation>
    </message>
    <message>
        <source>Add Point</source>
        <translation>Добавить точку</translation>
    </message>
    <message>
        <source>.</source>
        <translation type="unfinished">.</translation>
    </message>
    <message>
        <source>Capture Points</source>
        <translation>Захватить точки</translation>
    </message>
    <message>
        <source>Delete Point</source>
        <translation>Удалить точку</translation>
    </message>
    <message>
        <source>Delete Selected</source>
        <translation>Удалить выбранное</translation>
    </message>
    <message>
        <source>Linear</source>
        <translation>Линейное</translation>
    </message>
    <message>
        <source>Helmert</source>
        <translation>Гельмерта</translation>
    </message>
    <message>
        <source>Choose a name for the world file</source>
        <translation>Выберите имя файла привязки</translation>
    </message>
    <message>
        <source>-modified</source>
        <comment>Georeferencer:QgsPointDialog.cpp - used to modify a user given filename</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Внимание</translation>
    </message>
    <message>
        <source>Affine</source>
        <translation>Аффинное</translation>
    </message>
    <message>
        <source>Not implemented!</source>
        <translation>Функция не реализована!</translation>
    </message>
    <message>
        <source>&lt;p&gt;An affine transform requires changing the original raster file. This is not yet supported.&lt;/p&gt;</source>
        <translation>&lt;p&gt;Аффинное преобразование требует изменения оригинального растрового файла. В данный момент, эта функция не поддерживается.&lt;/p&gt;</translation>
    </message>
    <message>
        <source>&lt;p&gt;The </source>
        <translation>&lt;p&gt;</translation>
    </message>
    <message>
        <source> transform is not yet supported.&lt;/p&gt;</source>
        <translation> преобразование в настоящий момент не поддерживается.&lt;/p&gt;</translation>
    </message>
    <message>
        <source>Error</source>
        <translation>Ошибка</translation>
    </message>
    <message>
        <source>Could not write to </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Currently all modified files will be written in TIFF format.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;p&gt;A Helmert transform requires modifications in the raster layer.&lt;/p&gt;&lt;p&gt;The modified raster will be saved in a new file and a world file will be generated for this new file instead.&lt;/p&gt;&lt;p&gt;Are you sure that this is what you want?&lt;/p&gt;</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsPointDialogBase</name>
    <message>
        <source>Transform type:</source>
        <translation>Тип преобразования:</translation>
    </message>
    <message>
        <source>Zoom in</source>
        <translation>Увеличить</translation>
    </message>
    <message>
        <source>Zoom out</source>
        <translation>Уменьшить</translation>
    </message>
    <message>
        <source>Zoom to the raster extents</source>
        <translation>Увеличить до границ растра</translation>
    </message>
    <message>
        <source>Pan</source>
        <translation>Панорамировать</translation>
    </message>
    <message>
        <source>Add points</source>
        <translation>Добавить точки</translation>
    </message>
    <message>
        <source>Delete points</source>
        <translation>Удалить точки</translation>
    </message>
    <message>
        <source>World file:</source>
        <translation>Файл привязки:</translation>
    </message>
    <message>
        <source>Modified raster:</source>
        <translation>Изменённый растр:</translation>
    </message>
    <message>
        <source>Reference points</source>
        <translation>Опорные точки</translation>
    </message>
    <message>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <source>Create</source>
        <translation>Создать</translation>
    </message>
    <message>
        <source>Create and load layer</source>
        <translation>Создать и загрузить слой</translation>
    </message>
</context>
<context>
    <name>QgsPointStyleWidgetBase</name>
    <message>
        <source>Form3</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Symbol Style</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Scale</source>
        <translation type="unfinished"></translation>
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
        <source>No GEOS Support!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Your PostGIS installation has no GEOS support.
Feature selection and identification will not work properly.
Please install PostGIS with GEOS support (http://geos.refractions.net)</source>
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
        <source>INSERT error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>An error occured during feature insertion</source>
        <translation>При вставке объекта возникла ошибка</translation>
    </message>
    <message>
        <source>DELETE error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>An error occured during deletion from disk</source>
        <translation>При удалении с диска возникла ошибка</translation>
    </message>
    <message>
        <source>PostGIS error</source>
        <translation>Ошибка PostGIS</translation>
    </message>
    <message>
        <source>When trying: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>An error occured contacting the PostgreSQL database</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The PostgreSQL database returned: </source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsProjectPropertiesBase</name>
    <message>
        <source>Project Properties</source>
        <translation>Свойства проекта</translation>
    </message>
    <message>
        <source>Map Units</source>
        <translation>Картографические единицы</translation>
    </message>
    <message>
        <source>Meters</source>
        <translation>Метры</translation>
    </message>
    <message>
        <source>Feet</source>
        <translation>Футы</translation>
    </message>
    <message>
        <source>Decimal degrees</source>
        <translation>Десятичные градусы</translation>
    </message>
    <message>
        <source>&amp;Help</source>
        <translation type="obsolete">&amp;Помощь</translation>
    </message>
    <message>
        <source>Default project title</source>
        <translation>Заглавие проекта по умолчанию</translation>
    </message>
    <message>
        <source>General</source>
        <translation>Общие</translation>
    </message>
    <message>
        <source>Line Width:</source>
        <translation type="obsolete">Ширина линии:</translation>
    </message>
    <message>
        <source>Line Colour:</source>
        <translation type="obsolete">Цвет линии:</translation>
    </message>
    <message>
        <source>Automatic</source>
        <translation>Автоматическая</translation>
    </message>
    <message>
        <source>Automatically sets the number of decimal places in the mouse position display</source>
        <translation>Автоматически устанавливать число десятичных знаков в поле вывода позиции курсора мыши</translation>
    </message>
    <message>
        <source>The number of decimal places that are used when displaying the mouse position is automatically set to be enough so that moving the mouse by one pixel gives a change in the position display</source>
        <translation>Количество используемых десятичных знаков в значении позиции курсора выбирается автоматически таким образом, что перемещение мыши на один пиксель вызовет изменение в поле отображения позиции</translation>
    </message>
    <message>
        <source>Manual</source>
        <translation>Ручная</translation>
    </message>
    <message>
        <source>Sets the number of decimal places to use for the mouse position display</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The number of decimal places for the manual option</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>decimal places</source>
        <translation>десятичных знаков</translation>
    </message>
    <message>
        <source>Map Appearance</source>
        <translation>Вид карты</translation>
    </message>
    <message>
        <source>Selection Color:</source>
        <translation>Цвет выделения:</translation>
    </message>
    <message>
        <source>Project Title</source>
        <translation>Заглавие проекта</translation>
    </message>
    <message>
        <source>Projection</source>
        <translation>Проекция</translation>
    </message>
    <message>
        <source>Enable on the fly projection</source>
        <translation>Включить преобразование проекции «на лету»</translation>
    </message>
    <message>
        <source>Background Color:</source>
        <translation>Цвет фона:</translation>
    </message>
    <message>
        <source>Precision</source>
        <translation>Точность</translation>
    </message>
    <message>
        <source>Digitizing</source>
        <translation>Оцифровка</translation>
    </message>
    <message>
        <source>Descriptive project name</source>
        <translation>Описательное заглавие проекта</translation>
    </message>
    <message>
        <source>Line width in pixels</source>
        <translation type="obsolete">Ширина линии в пикселях</translation>
    </message>
    <message>
        <source>Snapping tolerance in map units</source>
        <translation type="obsolete">Порог выравнивания в единицах карты</translation>
    </message>
    <message>
        <source>Snapping Tolerance (in map units):</source>
        <translation type="obsolete">Порог выравнивания (в единицах карты):</translation>
    </message>
    <message>
        <source>Enable topological editing</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Snapping options...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Avoid intersections of new polygons</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsProjectionSelector</name>
    <message>
        <source>QGIS SRSID: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>PostGIS SRID: </source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsProjectionSelectorBase</name>
    <message>
        <source>Projection Selector</source>
        <translation>Выбор проекции</translation>
    </message>
    <message>
        <source>Projection</source>
        <translation>Проекция</translation>
    </message>
    <message>
        <source>Search</source>
        <translation>Искать</translation>
    </message>
    <message>
        <source>Find</source>
        <translation>Найти</translation>
    </message>
    <message>
        <source>Postgis SRID</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>EPSG ID</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>QGIS SRSID</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Имя</translation>
    </message>
    <message>
        <source>Spatial Reference System</source>
        <translation>Координатная система</translation>
    </message>
    <message>
        <source>Id</source>
        <translation>ID</translation>
    </message>
</context>
<context>
    <name>QgsPythonDialog</name>
    <message>
        <source>Python console</source>
        <translation>Консоль Python</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;DejaVu Sans Condensed&apos;; font-size:10pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;To access Quantum GIS environment from this python console use object &lt;span style=&quot; font-weight:600;&quot;&gt;iface&lt;/span&gt; from global scope which is an instance of QgisInterface class.&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Usage e.g.: iface.zoomFull()&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;DejaVu Sans Condensed&apos;; font-size:10pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Для получения доступа к окружению Quantum GIS из Python-консоли, используйте объект &lt;span style=&quot; font-weight:600;&quot;&gt;iface&lt;/span&gt; из глобального пространства имён (этот объект реализует интерфейс QgisInterface).&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Пример использования: iface.zoomFull()&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>&gt;&gt;&gt;</source>
        <translation></translation>
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
        <translation type="unfinished"> км</translation>
    </message>
    <message>
        <source> mm</source>
        <translation type="unfinished"> мм</translation>
    </message>
    <message>
        <source> cm</source>
        <translation type="unfinished"> см</translation>
    </message>
    <message>
        <source> m</source>
        <translation type="unfinished"> м</translation>
    </message>
    <message>
        <source> miles</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> mile</source>
        <translation type="unfinished"> миль</translation>
    </message>
    <message>
        <source> inches</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> foot</source>
        <translation type="unfinished"> фут</translation>
    </message>
    <message>
        <source> feet</source>
        <translation type="unfinished"> футов</translation>
    </message>
    <message>
        <source> degree</source>
        <translation type="unfinished"> градус</translation>
    </message>
    <message>
        <source> degrees</source>
        <translation type="unfinished"> градусов</translation>
    </message>
    <message>
        <source> unknown</source>
        <translation type="unfinished"> неизв</translation>
    </message>
</context>
<context>
    <name>QgsRasterLayer</name>
    <message>
        <source>Not Set</source>
        <translation>Не задано</translation>
    </message>
    <message>
        <source>&amp;Zoom to extent of selected layer</source>
        <translation type="obsolete">&amp;Изменить вид до полной данного слоя</translation>
    </message>
    <message>
        <source>&amp;Properties</source>
        <translation type="obsolete">&amp;Свойства</translation>
    </message>
    <message>
        <source>&amp;Remove</source>
        <translation type="obsolete">&amp;Удалить</translation>
    </message>
    <message>
        <source>Driver:</source>
        <translation>Драйвер:</translation>
    </message>
    <message>
        <source>Dimensions:</source>
        <translation>Размеры:</translation>
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
        <translation> Каналы: </translation>
    </message>
    <message>
        <source>Origin:</source>
        <translation>Базис:</translation>
    </message>
    <message>
        <source>Pixel Size:</source>
        <translation>Размер пикселя:</translation>
    </message>
    <message>
        <source>Raster Extent: </source>
        <translation>Границы растра: </translation>
    </message>
    <message>
        <source>Clipped area: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Pyramid overviews:</source>
        <translation>Обзор пирамид:</translation>
    </message>
    <message>
        <source>Property</source>
        <translation>Свойство</translation>
    </message>
    <message>
        <source>Value</source>
        <translation>Значение</translation>
    </message>
    <message>
        <source>Band</source>
        <translation>Канал</translation>
    </message>
    <message>
        <source>Band No</source>
        <translation>Канал №</translation>
    </message>
    <message>
        <source>No Stats</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>No stats collected yet</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Min Val</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Max Val</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Range</source>
        <translation>Диапазон</translation>
    </message>
    <message>
        <source>Mean</source>
        <translation>Среднее</translation>
    </message>
    <message>
        <source>Sum of squares</source>
        <translation>Сумма квадратов</translation>
    </message>
    <message>
        <source>Standard Deviation</source>
        <translation>Стандартное отклонение</translation>
    </message>
    <message>
        <source>Sum of all cells</source>
        <translation>Сумма всех ячеек</translation>
    </message>
    <message>
        <source>Cell Count</source>
        <translation>Количество ячеек</translation>
    </message>
    <message>
        <source>Data Type:</source>
        <translation>Тип данных:</translation>
    </message>
    <message>
        <source>GDT_Byte - Eight bit unsigned integer</source>
        <translation>GDT_Byte ‒ 8-битное беззнаковое целое</translation>
    </message>
    <message>
        <source>GDT_UInt16 - Sixteen bit unsigned integer </source>
        <translation>GDT_UInt16 ‒ 16-битное беззнаковое целое </translation>
    </message>
    <message>
        <source>GDT_Int16 - Sixteen bit signed integer </source>
        <translation>GDT_Int16 ‒ 16-битное целое со знаком</translation>
    </message>
    <message>
        <source>GDT_UInt32 - Thirty two bit unsigned integer </source>
        <translation>GDT_UInt32 ‒ 32-битное беззнаковое целое </translation>
    </message>
    <message>
        <source>GDT_Int32 - Thirty two bit signed integer </source>
        <translation>GDT_Int32 ‒ 32-битное целое со знаком </translation>
    </message>
    <message>
        <source>GDT_Float32 - Thirty two bit floating point </source>
        <translation>GDT_Float32 ‒ 32-битное с плавающей точкой</translation>
    </message>
    <message>
        <source>GDT_Float64 - Sixty four bit floating point </source>
        <translation>GDT_Float64 ‒ 64-битное с плавающей точкой </translation>
    </message>
    <message>
        <source>GDT_CInt16 - Complex Int16 </source>
        <translation>GDT_CInt16 ‒ Комплексное Int16 </translation>
    </message>
    <message>
        <source>GDT_CInt32 - Complex Int32 </source>
        <translation>GDT_CInt32 ‒ Комплексное Int32 </translation>
    </message>
    <message>
        <source>GDT_CFloat32 - Complex Float32 </source>
        <translation>GDT_CFloat32 - Комплексное Float32 </translation>
    </message>
    <message>
        <source>GDT_CFloat64 - Complex Float64 </source>
        <translation>GDT_CFloat64 ‒ Копмлексное Float64 </translation>
    </message>
    <message>
        <source>Could not determine raster data type.</source>
        <translation>Не удалось установить тип растровых данных.</translation>
    </message>
    <message>
        <source>Average Magphase</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Average</source>
        <translation></translation>
    </message>
    <message>
        <source>Layer Spatial Reference System: </source>
        <translation>Координатная система слоя: </translation>
    </message>
    <message>
        <source>out of extent</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>null (no data)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Dataset Description</source>
        <translation>Описание набора данных</translation>
    </message>
    <message>
        <source>No Data Value</source>
        <translation>Значение «нет данных»</translation>
    </message>
    <message>
        <source>and all other files</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>NoDataValue not set</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsRasterLayerProperties</name>
    <message>
        <source>&lt;h3&gt;Multiband Image Notes&lt;/h3&gt;&lt;p&gt;This is a multiband image. You can choose to render it as grayscale or color (RGB). For color images, you can associate bands to colors arbitarily. For example, if you have a seven band landsat image, you may choose to render it as:&lt;/p&gt;&lt;ul&gt;&lt;li&gt;Visible Blue (0.45 to 0.52 microns) - not mapped&lt;/li&gt;&lt;li&gt;Visible Green (0.52 to 0.60 microns) - not mapped&lt;/li&gt;&lt;/li&gt;Visible Red (0.63 to 0.69 microns) - mapped to red in image&lt;/li&gt;&lt;li&gt;Near Infrared (0.76 to 0.90 microns) - mapped to green in image&lt;/li&gt;&lt;li&gt;Mid Infrared (1.55 to 1.75 microns) - not mapped&lt;/li&gt;&lt;li&gt;Thermal Infrared (10.4 to 12.5 microns) - not mapped&lt;/li&gt;&lt;li&gt;Mid Infrared (2.08 to 2.35 microns) - mapped to blue in image&lt;/li&gt;&lt;/ul&gt;</source>
        <translation type="obsolete">&lt;h3&gt;Примечания для многоканального изображения&lt;/h3&gt;&lt;p&gt;Это изображение является многоканальным. Для его отрисовки вы можете выбрать как цветной режим (RGB) так и градации серого. Для цветных изображений допускается произвольное присвоение каналам цветов изображения. К примеру, семиканальное изображение Landsat можно вывести следующим образом:&lt;/p&gt;&lt;ul&gt;&lt;li&gt;Видимый синий (0.45‒0.52 мкм) ‒ не назначено&lt;/li&gt;&lt;li&gt;Видимый зелёный (0.52‒0.60 мкм) ‒ не назначено&lt;/li&gt;&lt;li&gt;Видимый красный (0.63‒0.69 мкм) ‒ красный в изображении&lt;/li&gt;&lt;li&gt;Ближний инфракрасный (0.76‒0.90 мкм) ‒ зелёный в изображении&lt;/li&gt;&lt;li&gt;Средний инфракрасный (1.55‒1.75 мкм) ‒ не назначено&lt;/li&gt;&lt;li&gt;Тепловой инфракрасный (10.4‒12.5 мкм) ‒ не назначено&lt;/li&gt;&lt;li&gt;Средний инфракрасный (2.08‒2.35 мкм) - синий в изображении&lt;/li&gt;&lt;/ul&gt;</translation>
    </message>
    <message>
        <source>Grayscale</source>
        <translation>Градации серого</translation>
    </message>
    <message>
        <source>Pseudocolor</source>
        <translation>Псевдоцвет</translation>
    </message>
    <message>
        <source>Freak Out</source>
        <translation>Кислотная</translation>
    </message>
    <message>
        <source>Palette</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Not Set</source>
        <translation type="obsolete">Не задано</translation>
    </message>
    <message>
        <source>Columns: </source>
        <translation>Столбцов: </translation>
    </message>
    <message>
        <source>Rows: </source>
        <translation>Строк: </translation>
    </message>
    <message>
        <source>No-Data Value: </source>
        <translation>Значение «нет данных»: </translation>
    </message>
    <message>
        <source>n/a</source>
        <translation>н/д</translation>
    </message>
    <message>
        <source>Write access denied</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Write access denied. Adjust the file permissions and try again.

</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Building pyramids failed.</source>
        <translation>Не удалось построить пирамиды.</translation>
    </message>
    <message>
        <source>The file was not writeable. Some formats can not be written to, only read. You can also try to check the permissions and then try again.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Building pyramid overviews is not supported on this type of raster.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Custom Colormap</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>No Stretch</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Stretch To MinMax</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Stretch And Clip To MinMax</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Clip To MinMax</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Discrete</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Linearly</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Equal interval</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Quantiles</source>
        <translation type="unfinished">Квантили</translation>
    </message>
    <message>
        <source>Description</source>
        <translation type="unfinished">Описание</translation>
    </message>
    <message>
        <source>Large resolution raster layers can slow navigation in QGIS.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>By creating lower resolution copies of the data (pyramids) performance can be considerably improved as QGIS selects the most suitable resolution to use depending on the level of zoom.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>You must have write access in the directory where the original data is stored to build pyramids.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Please note that building pyramids may alter the original data file and once created they cannot be removed!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Please note that building pyramids could corrupt your image - always make a backup of your data first!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Red</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Green</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Blue</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Percent Transparent</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Gray</source>
        <translation type="unfinished">Серый</translation>
    </message>
    <message>
        <source>Indexed Value</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>User Defined</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>No Scaling</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>No-Data Value: Not Set</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Save file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Textfile (*.txt)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>QGIS Generated Transparent Pixel Value Export File</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Open file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Import Error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The following lines contained errors

</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Read access denied</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Read access denied. Adjust the file permissions and try again.

</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Color Ramp</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsRasterLayerPropertiesBase</name>
    <message>
        <source>Raster Layer Properties</source>
        <translation>Свойства растрового слоя</translation>
    </message>
    <message>
        <source>&amp;Help</source>
        <translation type="obsolete">&amp;Помощь</translation>
    </message>
    <message>
        <source>General</source>
        <translation>Общие</translation>
    </message>
    <message>
        <source>Layer Source:</source>
        <translation>Источник слоя:</translation>
    </message>
    <message>
        <source>Display Name:</source>
        <translation>Имя в легенде:</translation>
    </message>
    <message>
        <source>Legend:</source>
        <translation>Легенда:</translation>
    </message>
    <message>
        <source>No Data:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Symbology</source>
        <translation>Символика</translation>
    </message>
    <message>
        <source>Transparency:</source>
        <translation type="obsolete">Прозрачность:</translation>
    </message>
    <message>
        <source>&lt;p align=&quot;right&quot;&gt;Full&lt;/p&gt;</source>
        <translation>Полная</translation>
    </message>
    <message>
        <source>None</source>
        <translation>Нулевая</translation>
    </message>
    <message>
        <source>Invert Color Map</source>
        <translation>Обратить цветовую карту</translation>
    </message>
    <message>
        <source>Band</source>
        <translation type="obsolete">Канал</translation>
    </message>
    <message>
        <source>&lt;b&gt;&lt;font color=&quot;#00ff00&quot;&gt;Green&lt;/font&gt;&lt;/b&gt;</source>
        <translation type="obsolete">&lt;b&gt;&lt;font color=&quot;#00ff00&quot;&gt;Зелёный&lt;/font&gt;&lt;/b&gt;</translation>
    </message>
    <message>
        <source>&lt;b&gt;&lt;font color=&quot;#ff0000&quot;&gt;Red&lt;/font&gt;&lt;/b&gt;</source>
        <translation type="obsolete">&lt;b&gt;&lt;font color=&quot;#ff0000&quot;&gt;Красный&lt;/font&gt;&lt;/b&gt;</translation>
    </message>
    <message>
        <source>&lt;b&gt;&lt;font color=&quot;#0000ff&quot;&gt;Blue&lt;/font&gt;&lt;/b&gt;</source>
        <translation type="obsolete">&lt;b&gt;&lt;font color=&quot;#0000ff&quot;&gt;Синий&lt;/font&gt;&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Color</source>
        <translation type="obsolete">Цвет</translation>
    </message>
    <message>
        <source>Gray</source>
        <translation>Серый</translation>
    </message>
    <message>
        <source>Std Deviations</source>
        <translation type="obsolete">Стд. отклонение</translation>
    </message>
    <message>
        <source>Color Map</source>
        <translation>Цветовая карта</translation>
    </message>
    <message>
        <source>Metadata</source>
        <translation>Метаданные</translation>
    </message>
    <message>
        <source>Pyramids</source>
        <translation>Пирамиды</translation>
    </message>
    <message>
        <source>Resampling Method</source>
        <translation>Метод интерполяции</translation>
    </message>
    <message>
        <source>Average</source>
        <translation>Среднее значение</translation>
    </message>
    <message>
        <source>Nearest Neighbour</source>
        <translation>Ближайшего соседа</translation>
    </message>
    <message>
        <source>Build Pyramids</source>
        <translation>Построить пирамиды</translation>
    </message>
    <message>
        <source>Pyramid Resolutions</source>
        <translation>Разрешения пирамид</translation>
    </message>
    <message>
        <source>Thumbnail</source>
        <translation>Образец</translation>
    </message>
    <message>
        <source>Columns:</source>
        <translation>Столбцы:</translation>
    </message>
    <message>
        <source>Rows:</source>
        <translation>Строки:</translation>
    </message>
    <message>
        <source>Palette:</source>
        <translation>Палитра:</translation>
    </message>
    <message>
        <source>Maximum 1:</source>
        <translation>Максимальный 1:</translation>
    </message>
    <message>
        <source>Maximum scale at which this layer will be displayed. </source>
        <translation>Максимальный масштаб, при котором виден данный слой. </translation>
    </message>
    <message>
        <source>Minimum 1:</source>
        <translation>Минимальный 1:</translation>
    </message>
    <message>
        <source>Minimum scale at which this layer will be displayed. </source>
        <translation>Минимальный масштаб, при котором виден данный слой. </translation>
    </message>
    <message>
        <source>Spatial Reference System</source>
        <translation>Координатная система</translation>
    </message>
    <message>
        <source>Change</source>
        <translation>Изменить</translation>
    </message>
    <message>
        <source>Histogram</source>
        <translation>Гистограмма</translation>
    </message>
    <message>
        <source>Options</source>
        <translation>Параметры</translation>
    </message>
    <message>
        <source>Out Of Range OK?</source>
        <translation>Разрешить значения вне диапазона?</translation>
    </message>
    <message>
        <source>Allow Approximation</source>
        <translation>Разрешить аппроксимацию</translation>
    </message>
    <message>
        <source>Chart Type</source>
        <translation>Тип диаграммы</translation>
    </message>
    <message>
        <source>Bar Chart</source>
        <translation>Столбчатая</translation>
    </message>
    <message>
        <source>Line Graph</source>
        <translation>Линейная</translation>
    </message>
    <message>
        <source>Refresh</source>
        <translation>Обновить</translation>
    </message>
    <message>
        <source>Display</source>
        <translation type="obsolete">Отображение</translation>
    </message>
    <message>
        <source>Grayscale Image</source>
        <translation type="obsolete">Градации серого</translation>
    </message>
    <message>
        <source>Color Image</source>
        <translation type="obsolete">Цветное изображение</translation>
    </message>
    <message>
        <source>DebugInfo</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Scale Dependent Visibility</source>
        <translation>Видимость в пределах масштаба</translation>
    </message>
    <message>
        <source>Column Count:</source>
        <translation>Столбцы:</translation>
    </message>
    <message>
        <source>Transparent</source>
        <translation type="obsolete">Прозрачность</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot;font-size:10pt;font-family:Sans Serif&quot;&gt;
&lt;p style=&quot;margin-top:18px&quot; dir=&quot;ltr&quot;&gt;&lt;span style=&quot;font-size:15pt;font-weight:600&quot;&gt;Description&lt;/span&gt;&lt;/p&gt;
&lt;p dir=&quot;ltr&quot;&gt;Large resolution raster layers can slow navigation in QGIS. By creating lower resolution copies of the data (pyramids) performance can be considerably improved as QGIS selects the most suitable resolution to use depending on the level of zoom. You must have write access in the directory where the original data is stored to build pyramids. &lt;/p&gt;
&lt;p dir=&quot;ltr&quot;&gt;&lt;span style=&quot;color:#ff0000&quot;&gt;Please note that building pyramids may alter the original data file and once created they cannot be removed.&lt;/span&gt;&lt;/p&gt;
&lt;p dir=&quot;ltr&quot;&gt;&lt;span style=&quot;color:#ff0000&quot;&gt;Please note that building pyramids could corrupt your image - always make a backup of your data first!&lt;/span&gt;&lt;/p&gt;
&lt;/body&gt;&lt;/html&gt;
</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot;font-size:10pt;font-family:Sans Serif&quot;&gt;
&lt;p style=&quot;margin-top:18px&quot; dir=&quot;ltr&quot;&gt;&lt;span style=&quot;font-size:15pt;font-weight:600&quot;&gt;Описание&lt;/span&gt;&lt;/p&gt;
&lt;p dir=&quot;ltr&quot;&gt;Растры высокого разрешения могут замедлить быстроту навигации в QGIS. Проивзодительность можно существенно повысить путём создания копий данных в низком разрешении (пирамид). В этом случае, QGIS автоматически выберет наиболее подходящее разрешение в зависимости от масштаба отображаемой карты. Для построения пирамид, необходимы права на запись в каталог, где хрянятся оригинальные данные &lt;/p&gt;
&lt;p dir=&quot;ltr&quot;&gt;&lt;span style=&quot;color:#ff0000&quot;&gt;Обратите внимание, что построение пирамид может изменить оригинальный файл данных и после создания их нельзя будет удалить.&lt;/span&gt;&lt;/p&gt;
&lt;p dir=&quot;ltr&quot;&gt;&lt;span style=&quot;color:#ff0000&quot;&gt;Помните, что построение пирамид может повредить ваше изображение ‒ всегда создавайте резервные копии данных!&lt;/span&gt;&lt;/p&gt;
&lt;/body&gt;&lt;/html&gt;
</translation>
    </message>
    <message>
        <source>Grayscale Band Scaling</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Max</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Std Deviation</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Custom Min Max Values:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Min</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Contrast Enhancement</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Load Min Max Values From Band(s)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>RGB Scaling</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;b&gt;&lt;font color=&apos;blue&apos;&gt;Max&lt;/font&gt;&lt;/b&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Custom Min Max Values</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;b&gt;&lt;font color=&apos;green&apos;&gt;Min&lt;/font&gt;&lt;/b&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;b&gt;&lt;font color=&apos;green&apos;&gt;Max&lt;/font&gt;&lt;/b&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;b&gt;&lt;font color=&apos;red&apos;&gt;Min&lt;/font&gt;&lt;/b&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;b&gt;&lt;font color=&apos;red&apos;&gt;Max&lt;/font&gt;&lt;/b&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;b&gt;&lt;font color=&apos;blue&apos;&gt;Min&lt;/font&gt;&lt;/b&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Grayscale Band Selection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>RGB Mode Band Selection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;b&gt;&lt;font color=&apos;blue&apos;&gt;Blue&lt;/font&gt;&lt;/b&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;b&gt;&lt;font color=&apos;green&apos;&gt;Green&lt;/font&gt;&lt;/b&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;b&gt;&lt;font color=&apos;red&apos;&gt;Red&lt;/font&gt;&lt;/b&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Global Transparency</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> 00%</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Render as</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Single Band Gray</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Three Band Color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Transparent Pixels</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Transparent Band:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Custom Transparency List</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Transparency Layer;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Add Values Manually</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>...</source>
        <translation type="unfinished">...</translation>
    </message>
    <message>
        <source>Add Values From Display</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Remove Selected Row</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Default Values</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Import From File</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Export To File</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>No Data Value:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Reset No Data Value</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Colormap</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Number of entries:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Delete entry</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Classify</source>
        <translation type="unfinished">Классифицировать</translation>
    </message>
    <message>
        <source>1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Color interpolation:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Classification mode:</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsRunProcess</name>
    <message>
        <source>Unable to run command</source>
        <translation>Не удалось выполнить команду</translation>
    </message>
    <message>
        <source>Starting</source>
        <translation>Выполняется</translation>
    </message>
    <message>
        <source>Done</source>
        <translation>Выполнено</translation>
    </message>
</context>
<context>
    <name>QgsScaleBarPlugin</name>
    <message>
        <source> metres/km</source>
        <translation> метров/км</translation>
    </message>
    <message>
        <source> feet</source>
        <translation> футов</translation>
    </message>
    <message>
        <source> degrees</source>
        <translation> градусов</translation>
    </message>
    <message>
        <source> km</source>
        <translation> км</translation>
    </message>
    <message>
        <source> mm</source>
        <translation> мм</translation>
    </message>
    <message>
        <source> cm</source>
        <translation> см</translation>
    </message>
    <message>
        <source> m</source>
        <translation> м</translation>
    </message>
    <message>
        <source> foot</source>
        <translation> фут</translation>
    </message>
    <message>
        <source> degree</source>
        <translation> градус</translation>
    </message>
    <message>
        <source> unknown</source>
        <translation> неизв</translation>
    </message>
    <message>
        <source>Top Left</source>
        <translation>Вверху слева</translation>
    </message>
    <message>
        <source>Bottom Left</source>
        <translation>Внизу слева</translation>
    </message>
    <message>
        <source>Top Right</source>
        <translation>Вверху справа</translation>
    </message>
    <message>
        <source>Bottom Right</source>
        <translation>Внизу справа</translation>
    </message>
    <message>
        <source>Tick Down</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Tick Up</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Bar</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Box</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Scale Bar</source>
        <translation>&amp;Масштабная линейка</translation>
    </message>
    <message>
        <source>Creates a scale bar that is displayed on the map canvas</source>
        <translation>Создаёт масштабную линейку в области отображения карты</translation>
    </message>
    <message>
        <source>&amp;Decorations</source>
        <translation>&amp;Декораторы</translation>
    </message>
    <message>
        <source> feet/miles</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> miles</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> mile</source>
        <translation type="unfinished"> миль</translation>
    </message>
    <message>
        <source> inches</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsScaleBarPluginGuiBase</name>
    <message>
        <source>Scale Bar Plugin</source>
        <translation>Модуль масштабной линейки</translation>
    </message>
    <message>
        <source>Top Left</source>
        <translation>Вверху слева</translation>
    </message>
    <message>
        <source>Top Right</source>
        <translation>Вверху справа</translation>
    </message>
    <message>
        <source>Bottom Left</source>
        <translation>Внизу слева</translation>
    </message>
    <message>
        <source>Bottom Right</source>
        <translation>Внизу справа</translation>
    </message>
    <message>
        <source>Size of bar:</source>
        <translation>Размер линейки:</translation>
    </message>
    <message>
        <source>Placement:</source>
        <translation>Размещение:</translation>
    </message>
    <message>
        <source>Tick Down</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Tick Up</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Box</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Bar</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Select the style of the scale bar</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Colour of bar:</source>
        <translation>Цвет линейки:</translation>
    </message>
    <message>
        <source>Scale bar style:</source>
        <translation>Стиль линейки:</translation>
    </message>
    <message>
        <source>Enable scale bar</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Automatically snap to round number on resize</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Click to select the colour</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;This plugin draws a scale bar on the map. Please note the size option below is a &apos;preferred&apos; size and may have to be altered by QGIS depending on the level of zoom.  The size is measured according to the map units specified in the project properties.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsSearchQueryBuilder</name>
    <message>
        <source>Found %d matching features.</source>
        <translation type="obsolete">
        </translation>
    </message>
    <message>
        <source>No matching features found.</source>
        <translation type="unfinished">Подходящих объектов не найдено.</translation>
    </message>
    <message>
        <source>Search results</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Search string parsing error</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>No Records</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The query you specified results in zero records being returned.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Search query builder</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsServerSourceSelect</name>
    <message>
        <source>Are you sure you want to remove the </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> connection and all associated settings?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Confirm Delete</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Yes</source>
        <translation type="obsolete">Да</translation>
    </message>
    <message>
        <source>No</source>
        <translation type="obsolete">Нет</translation>
    </message>
    <message>
        <source>WMS Provider</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Could not open the WMS Provider</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Select Layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>You must select at least one layer first.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Coordinate Reference System (%1 available)</source>
        <translation type="obsolete">
        </translation>
    </message>
    <message>
        <source>Could not understand the response.  The</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>provider said</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>WMS proxies</source>
        <translation>WMS-прокси</translation>
    </message>
    <message>
        <source>&lt;p&gt;Several WMS servers have been added to the server list. Note that the proxy fields have been left blank and if you access the internet via a web proxy, you will need to individually set the proxy fields with appropriate values.&lt;/p&gt;</source>
        <translation>&lt;p&gt;Некоторые WMS-сервера были добавлены в список. Обратите внимание, что поля прокси были оставлены пустыми, и если вы выходите в интернет, используя прокси-сервер, вам следует задать параметры прокси в соответствующих полях.&lt;/p&gt;</translation>
    </message>
    <message>
        <source>Coordinate Reference System</source>
        <translation type="unfinished">Система координат</translation>
    </message>
    <message>
        <source>There are no available coordinate reference system for the set of layers you&apos;ve selected.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsServerSourceSelectBase</name>
    <message>
        <source>Add Layer(s) from a Server</source>
        <translation>Добавить слои с сервера</translation>
    </message>
    <message>
        <source>C&amp;lose</source>
        <translation>&amp;Закрыть</translation>
    </message>
    <message>
        <source>Alt+L</source>
        <translation>Alt+P</translation>
    </message>
    <message>
        <source>Help</source>
        <translation>Справка</translation>
    </message>
    <message>
        <source>F1</source>
        <translation></translation>
    </message>
    <message>
        <source>Image encoding</source>
        <translation>формат изображения</translation>
    </message>
    <message>
        <source>Layers</source>
        <translation>Слои</translation>
    </message>
    <message>
        <source>ID</source>
        <translation>ID</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Имя</translation>
    </message>
    <message>
        <source>Title</source>
        <translation>Заглавие</translation>
    </message>
    <message>
        <source>Abstract</source>
        <translation>Описание</translation>
    </message>
    <message>
        <source>&amp;Add</source>
        <translation>&amp;Добавить</translation>
    </message>
    <message>
        <source>Alt+A</source>
        <translation>Alt+L</translation>
    </message>
    <message>
        <source>Server Connections</source>
        <translation>Соединения с серверами</translation>
    </message>
    <message>
        <source>&amp;New</source>
        <translation>&amp;Новое</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>Удалить</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation>Правка</translation>
    </message>
    <message>
        <source>C&amp;onnect</source>
        <translation>&amp;Подключить</translation>
    </message>
    <message>
        <source>Ready</source>
        <translation>Готово</translation>
    </message>
    <message>
        <source>Coordinate Reference System</source>
        <translation>Система координат</translation>
    </message>
    <message>
        <source>Change ...</source>
        <translation>Изменить...</translation>
    </message>
    <message>
        <source>Adds a few example WMS servers</source>
        <translation>Добавить несколько известных WMS-серверов</translation>
    </message>
    <message>
        <source>Add default servers</source>
        <translation>Добавить сервера по умолчанию</translation>
    </message>
</context>
<context>
    <name>QgsShapeFile</name>
    <message>
        <source>The database gave an error while executing this SQL:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The error was:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>... (rest of SQL trimmed)</source>
        <comment>is appended to a truncated SQL statement</comment>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsSingleSymbolDialog</name>
    <message>
        <source>Solid Line</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Dash Line</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Dot Line</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Dash Dot Line</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Dash Dot Dot Line</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>No Pen</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Solid Pattern</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Hor Pattern</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Ver Pattern</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Cross Pattern</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>BDiag Pattern</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>FDiag Pattern</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Diag Cross Pattern</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Dense1 Pattern</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Dense2 Pattern</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Dense3 Pattern</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Dense4 Pattern</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Dense5 Pattern</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Dense6 Pattern</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Dense7 Pattern</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>No Brush</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Texture Pattern</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsSingleSymbolDialogBase</name>
    <message>
        <source>Single Symbol</source>
        <translation type="unfinished">Обычный знак</translation>
    </message>
    <message>
        <source>Fill Patterns:</source>
        <translation type="obsolete">Шаблоны заливки:</translation>
    </message>
    <message>
        <source>Point</source>
        <translation type="obsolete">Точка</translation>
    </message>
    <message>
        <source>Size</source>
        <translation>Размер</translation>
    </message>
    <message>
        <source>Symbol</source>
        <translation type="obsolete">Знак</translation>
    </message>
    <message>
        <source>Outline Width:</source>
        <translation type="obsolete">Ширина контура:</translation>
    </message>
    <message>
        <source>Fill Color:</source>
        <translation type="obsolete">Цвет заливки:</translation>
    </message>
    <message>
        <source>Outline color:</source>
        <translation type="obsolete">Цвет контура:</translation>
    </message>
    <message>
        <source>Outline Style:</source>
        <translation type="obsolete">Стиль контура:</translation>
    </message>
    <message>
        <source>Label:</source>
        <translation type="obsolete">Метка:</translation>
    </message>
    <message>
        <source>No Fill</source>
        <translation type="obsolete">Без заливки</translation>
    </message>
    <message>
        <source>Browse:</source>
        <translation type="obsolete">Обзор:</translation>
    </message>
    <message>
        <source>Point Symbol</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Area scale field</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Rotation field</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Style Options</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>...</source>
        <translation type="unfinished">...</translation>
    </message>
    <message>
        <source>Outline style</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Outline color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Outline width</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Fill color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Fill style</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Label</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsSnappingDialog</name>
    <message>
        <source>to vertex</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>to segment</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>to vertex and segment</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsSnappingDialogBase</name>
    <message>
        <source>Snapping options</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Mode</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Tolerance</source>
        <translation type="unfinished"></translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Yes</source>
        <translation type="obsolete">Да</translation>
    </message>
    <message>
        <source>No</source>
        <translation type="obsolete">Нет</translation>
    </message>
    <message>
        <source> - Edit Column Names</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The following Shapefile(s) could not be loaded:

</source>
        <translation>Не удалось загрузить следующие shape-файлы:

</translation>
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
        <translation>Отменить</translation>
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
        <source>Use the table below to edit column names. Make sure that none of the columns are named using a PostgreSQL reserved word</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>File Name</source>
        <translation>Имя файла</translation>
    </message>
    <message>
        <source>Feature Class</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Features</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>DB Relation Name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Schema</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>New Connection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Add Shapefiles</source>
        <translation>Добавить shape-файлы</translation>
    </message>
    <message>
        <source>Shapefiles (*.shp);;All files (*.*)</source>
        <translation>Shape-файлы (*.shp);;Все файлы (*.*)</translation>
    </message>
    <message>
        <source>PostGIS not available</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;p&gt;The chosen database does not have PostGIS installed, but this is required for storage of spatial data.&lt;/p&gt;</source>
        <translation>&lt;p&gt;PostGIS не установлен в выбранной БД, что делает невозможным хранение пространственных данных.&lt;/p&gt;</translation>
    </message>
    <message>
        <source>Checking to see if </source>
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
</context>
<context>
    <name>QgsSpitBase</name>
    <message>
        <source>SPIT - Shapefile to PostGIS Import Tool</source>
        <translation type="unfinished">SPIT - инструмент импорта shape-файлов в PostGIS</translation>
    </message>
    <message>
        <source>PostgreSQL Connections</source>
        <translation>Соединения PostgreSQL</translation>
    </message>
    <message>
        <source>Remove</source>
        <translation>Удалить</translation>
    </message>
    <message>
        <source>Shapefile List</source>
        <translation>Список shape-файлов</translation>
    </message>
    <message>
        <source>Geometry Column Name</source>
        <translation>Имя поля геометрии</translation>
    </message>
    <message>
        <source>SRID</source>
        <translation></translation>
    </message>
    <message>
        <source>Remove All</source>
        <translation>Удалить все</translation>
    </message>
    <message>
        <source>Global Schema</source>
        <translation>Общая схема</translation>
    </message>
    <message>
        <source>Shapefile to PostGIS Import Tool</source>
        <translation>Импорт shape-файлов в PostGIS</translation>
    </message>
    <message>
        <source>Add</source>
        <translation>Добавить</translation>
    </message>
    <message>
        <source>Add a shapefile to the list of files to be imported</source>
        <translation>Добавить shape-файл к списку импортируемых</translation>
    </message>
    <message>
        <source>Remove the selected shapefile from the import list</source>
        <translation>Удалить выбранный shape-файл из списка</translation>
    </message>
    <message>
        <source>Remove all the shapefiles from the import list</source>
        <translation>Удалить все shape-файлы из списка</translation>
    </message>
    <message>
        <source>Use Default SRID</source>
        <translation>Использовать SRID по умолчанию</translation>
    </message>
    <message>
        <source>Set the SRID to the default value</source>
        <translation>Заполнить SRID значением по умолчанию</translation>
    </message>
    <message>
        <source>Use Default Geometry Column Name</source>
        <translation>Использовать имя поля геометрии по умолчанию</translation>
    </message>
    <message>
        <source>Set the geometry column name to the default value</source>
        <translation>Задать имя поля геометрии в соответствии со значением по умолчанию</translation>
    </message>
    <message>
        <source>New</source>
        <translation>Новое</translation>
    </message>
    <message>
        <source>Create a new PostGIS connection</source>
        <translation>Создать новое PostGIS-соединение</translation>
    </message>
    <message>
        <source>Remove the current PostGIS connection</source>
        <translation>Удалить текущее PostGIS-соединение</translation>
    </message>
    <message>
        <source>Connect</source>
        <translation>Подключить</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation>Правка</translation>
    </message>
    <message>
        <source>Edit the current PostGIS connection</source>
        <translation>Редактировать текущее PostGIS-соединение</translation>
    </message>
</context>
<context>
    <name>QgsSpitPlugin</name>
    <message>
        <source>&amp;Import Shapefiles to PostgreSQL</source>
        <translation>&amp;Импорт Shape-файлов в PostgreSQL</translation>
    </message>
    <message>
        <source>Import shapefiles into a PostGIS-enabled PostgreSQL database. The schema and field names can be customized on import</source>
        <translation>Импорт shape-файлов в базу данных PostgreSQL с поддержкой PostGIS. В процессе импорта допускается изменение схемы и имён полей</translation>
    </message>
    <message>
        <source>&amp;Spit</source>
        <translation>&amp;Spit</translation>
    </message>
</context>
<context>
    <name>QgsUniqueValueDialogBase</name>
    <message>
        <source>Form1</source>
        <translation></translation>
    </message>
    <message>
        <source>Classification Field:</source>
        <translation>Поле классификации:</translation>
    </message>
    <message>
        <source>Delete class</source>
        <translation>Удалить класс</translation>
    </message>
    <message>
        <source>Classify</source>
        <translation>Классифицировать</translation>
    </message>
</context>
<context>
    <name>QgsVectorLayer</name>
    <message>
        <source>&amp;Zoom to extent of selected layer</source>
        <translation type="obsolete">&amp;Изменить вид до полной данного слоя</translation>
    </message>
    <message>
        <source>&amp;Open attribute table</source>
        <translation type="obsolete">&amp;Открыть таблицу атрибутов</translation>
    </message>
    <message>
        <source>&amp;Properties</source>
        <translation type="obsolete">&amp;Свойства</translation>
    </message>
    <message>
        <source>&amp;Remove</source>
        <translation type="obsolete">&amp;Удалить</translation>
    </message>
    <message>
        <source>Could not commit the added features.</source>
        <translation>Не удалось внести добавленные объекты.</translation>
    </message>
    <message>
        <source>No other types of changes will be committed at this time.</source>
        <translation>Другие виды изменений не будут внесены в данный момент.</translation>
    </message>
    <message>
        <source>Could not commit the changed attributes.</source>
        <translation>Не удалось внести изменённые атрибуты.</translation>
    </message>
    <message>
        <source>However, the added features were committed OK.</source>
        <translation>Тем не менее, добавленные объекты были успешно внесены.</translation>
    </message>
    <message>
        <source>Could not commit the changed geometries.</source>
        <translation>Не удалось внести изменённую геометрию объектов.</translation>
    </message>
    <message>
        <source>However, the changed attributes were committed OK.</source>
        <translation>Тем не менее, изменённые атрибуты были успешно внесены.</translation>
    </message>
    <message>
        <source>Could not commit the deleted features.</source>
        <translation>Не удалось внести удалённые объекты.</translation>
    </message>
    <message>
        <source>However, the changed geometries were committed OK.</source>
        <translation>Тем не менее, изменённия геометрии были успешно внесены.</translation>
    </message>
</context>
<context>
    <name>QgsVectorLayerProperties</name>
    <message>
        <source>Transparency: </source>
        <translation>Прозрачность: </translation>
    </message>
    <message>
        <source>Single Symbol</source>
        <translation>Обычный знак</translation>
    </message>
    <message>
        <source>Graduated Symbol</source>
        <translation>Градуированный знак</translation>
    </message>
    <message>
        <source>Continuous Color</source>
        <translation>Непрерывный цвет</translation>
    </message>
    <message>
        <source>Unique Value</source>
        <translation>Уникальное значение</translation>
    </message>
    <message>
        <source>This button opens the PostgreSQL query builder and allows you to create a subset of features to display on the map canvas rather than displaying all features in the layer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The query used to limit the features in the layer is shown here. This is currently only supported for PostgreSQL layers. To enter or modify the query, click on the Query Builder button</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Spatial Index</source>
        <translation>Пространственный индекс</translation>
    </message>
    <message>
        <source>Creation of spatial index successfull</source>
        <translation> Пространственный индекс успешно создан</translation>
    </message>
    <message>
        <source>Creation of spatial index failed</source>
        <translation>Не удалось создать пространственный индекс</translation>
    </message>
    <message>
        <source>General:</source>
        <translation>Общее:</translation>
    </message>
    <message>
        <source>Storage type of this layer : </source>
        <translation>Тип хранилища этого слоя: </translation>
    </message>
    <message>
        <source>Source for this layer : </source>
        <translation>Источник этого слоя: </translation>
    </message>
    <message>
        <source>Geometry type of the features in this layer : </source>
        <translation>Тип геометрии объектов в этом слое: </translation>
    </message>
    <message>
        <source>The number of features in this layer : </source>
        <translation>Количество объектов в слое: </translation>
    </message>
    <message>
        <source>Editing capabilities of this layer : </source>
        <translation>Возможности редактирования данного слоя: </translation>
    </message>
    <message>
        <source>Extents:</source>
        <translation>Границы: </translation>
    </message>
    <message>
        <source>In layer spatial reference system units : </source>
        <translation>В единицах координатной системы слоя: </translation>
    </message>
    <message>
        <source>xMin,yMin </source>
        <translation></translation>
    </message>
    <message>
        <source> : xMax,yMax </source>
        <translation></translation>
    </message>
    <message>
        <source>In project spatial reference system units : </source>
        <translation>В единицах координатной системы проекта: </translation>
    </message>
    <message>
        <source>Layer Spatial Reference System:</source>
        <translation>Система координат слоя:</translation>
    </message>
    <message>
        <source>Attribute field info:</source>
        <translation>Поля атрибутов:</translation>
    </message>
    <message>
        <source>Field</source>
        <translation>Поле</translation>
    </message>
    <message>
        <source>Type</source>
        <translation>Тип</translation>
    </message>
    <message>
        <source>Length</source>
        <translation>Длина</translation>
    </message>
    <message>
        <source>Precision</source>
        <translation>Точность</translation>
    </message>
    <message>
        <source>Layer comment: </source>
        <translation>Комментарий слоя: </translation>
    </message>
    <message>
        <source>Comment</source>
        <translation>Комментарий</translation>
    </message>
    <message>
        <source>Default Style</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>QGIS Layer Style File (*.qml)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>QGIS</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Unknown style format: </source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsVectorLayerPropertiesBase</name>
    <message>
        <source>Layer Properties</source>
        <translation>Свойства слоя</translation>
    </message>
    <message>
        <source>Legend type:</source>
        <translation>Тип легенды:</translation>
    </message>
    <message>
        <source>Symbology</source>
        <translation>Символика</translation>
    </message>
    <message>
        <source>Transparency:</source>
        <translation>Прозрачность:</translation>
    </message>
    <message>
        <source>General</source>
        <translation>Общие</translation>
    </message>
    <message>
        <source>Use scale dependent rendering</source>
        <translation>Видимость в пределах масштаба</translation>
    </message>
    <message>
        <source>Maximum 1:</source>
        <translation>Максимальный 1:</translation>
    </message>
    <message>
        <source>Minimum 1:</source>
        <translation>Минимальный 1:</translation>
    </message>
    <message>
        <source>Minimum scale at which this layer will be displayed. </source>
        <translation>Минимальный масштаб, при котором виден данный слой. </translation>
    </message>
    <message>
        <source>Maximum scale at which this layer will be displayed. </source>
        <translation>Максимальный масштаб, при котором виден данный слой. </translation>
    </message>
    <message>
        <source>Display name</source>
        <translation>Имя в легенде</translation>
    </message>
    <message>
        <source>Use this control to set which field is placed at the top level of the Identify Results dialog box.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Spatial Reference System</source>
        <translation>Координатная система</translation>
    </message>
    <message>
        <source>Change</source>
        <translation>Изменить</translation>
    </message>
    <message>
        <source>Display field for the Identify Results dialog box</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>This sets the display field for the Identify Results dialog box</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Display field</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Subset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Query Builder</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Spatial Index</source>
        <translation>Пространственный индекс</translation>
    </message>
    <message>
        <source>Create Spatial Index</source>
        <translation>Создать пространственный индекс</translation>
    </message>
    <message>
        <source>Create</source>
        <translation>Создать</translation>
    </message>
    <message>
        <source>Metadata</source>
        <translation>Метаданные</translation>
    </message>
    <message>
        <source>Labels</source>
        <translation>Подписи</translation>
    </message>
    <message>
        <source>Display labels</source>
        <translation>Включить подписи</translation>
    </message>
    <message>
        <source>Actions</source>
        <translation>Действия</translation>
    </message>
    <message>
        <source>Restore Default Style</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Save As Default</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Load Style ...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Save Style ...</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsVectorSymbologyWidgetBase</name>
    <message>
        <source>Form2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Label</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Min</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Max</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Symbol Classes:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Count:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Mode:</source>
        <translation type="unfinished">Режим:</translation>
    </message>
    <message>
        <source>Field:</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsWFSPlugin</name>
    <message>
        <source>&amp;Add WFS layer</source>
        <translation>&amp;Добавить WFS-слой</translation>
    </message>
</context>
<context>
    <name>QgsWFSProvider</name>
    <message>
        <source>unknown</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>received %1 bytes from %2</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsWFSSourceSelect</name>
    <message>
        <source>Are you sure you want to remove the </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> connection and all associated settings?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Confirm Delete</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Yes</source>
        <translation type="obsolete">Да</translation>
    </message>
    <message>
        <source>No</source>
        <translation type="obsolete">Нет</translation>
    </message>
</context>
<context>
    <name>QgsWFSSourceSelectBase</name>
    <message>
        <source>Title</source>
        <translation>Заглавие</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Имя</translation>
    </message>
    <message>
        <source>Abstract</source>
        <translation>Описание</translation>
    </message>
    <message>
        <source>Coordinate Reference System</source>
        <translation>Система координат</translation>
    </message>
    <message>
        <source>Change ...</source>
        <translation>Изменить...</translation>
    </message>
    <message>
        <source>Server Connections</source>
        <translation>Соединения с серверами</translation>
    </message>
    <message>
        <source>&amp;New</source>
        <translation>&amp;Новое</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>Удалить</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation>Правка</translation>
    </message>
    <message>
        <source>C&amp;onnect</source>
        <translation>&amp;Подключить</translation>
    </message>
    <message>
        <source>Add WFS Layer from a Server</source>
        <translation type="unfinished"></translation>
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
        <source>DOM Exception</source>
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
        <source>This is probably due to a bug in the QGIS program.  Please report this error.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Server Properties:</source>
        <translation>Свойства сервера:</translation>
    </message>
    <message>
        <source>Property</source>
        <translation type="unfinished">Свойство</translation>
    </message>
    <message>
        <source>Value</source>
        <translation type="unfinished">Значение</translation>
    </message>
    <message>
        <source>WMS Version</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Title</source>
        <translation type="unfinished">Заглавие</translation>
    </message>
    <message>
        <source>Abstract</source>
        <translation type="unfinished">Описание</translation>
    </message>
    <message>
        <source>Keywords</source>
        <translation type="unfinished"></translation>
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
        <translation>Свойства слоя: </translation>
    </message>
    <message>
        <source>Selected</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Yes</source>
        <translation>Да</translation>
    </message>
    <message>
        <source>No</source>
        <translation>Нет</translation>
    </message>
    <message>
        <source>Visibility</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Visible</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Hidden</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>n/a</source>
        <translation type="unfinished">н/д</translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Available in style</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Name</source>
        <translation type="unfinished">Имя</translation>
    </message>
    <message>
        <source>Layer cannot be queried.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QuickPrint</name>
    <message>
        <source>Quick Print</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Replace this with a short description of the what the plugin does</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Quick Print</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QuickPrintGui</name>
    <message>
        <source>Welcome to your automatically generated plugin!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>This is just a starting point. You now need to modify the code to make it do something useful....read on for a more information to get yourself started.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Documentation:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>You really need to read the QGIS API Documentation now at:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>In particular look at the following classes:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>QgsPlugin is an ABC that defines required behaviour your plugin must provide. See below for more details.</source>
        <translation type="unfinished"></translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>For Questions and Comments regarding the plugin builder template and creating your features in QGIS using the plugin interface please contact us via:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&lt;li&gt; the QGIS developers mailing list, or &lt;/li&gt;&lt;li&gt; IRC (#qgis on freenode.net)&lt;/li&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>QGIS is distributed under the Gnu Public License. If you create a useful plugin please consider contributing it back to the community.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Have fun and thank you for choosing QGIS.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Portable Document Format (*.pdf)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>quickprint</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Unknown format: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> km</source>
        <translation type="unfinished"> км</translation>
    </message>
    <message>
        <source> mm</source>
        <translation type="unfinished"> мм</translation>
    </message>
    <message>
        <source> cm</source>
        <translation type="unfinished"> см</translation>
    </message>
    <message>
        <source> m</source>
        <translation type="unfinished"> м</translation>
    </message>
    <message>
        <source> miles</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> mile</source>
        <translation type="unfinished"> миль</translation>
    </message>
    <message>
        <source> inches</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source> foot</source>
        <translation type="unfinished"> фут</translation>
    </message>
    <message>
        <source> feet</source>
        <translation type="unfinished"> футов</translation>
    </message>
    <message>
        <source> degree</source>
        <translation type="unfinished"> градус</translation>
    </message>
    <message>
        <source> degrees</source>
        <translation type="unfinished"> градусов</translation>
    </message>
    <message>
        <source> unknown</source>
        <translation type="unfinished"> неизв</translation>
    </message>
</context>
<context>
    <name>QuickPrintGuiBase</name>
    <message>
        <source>QGIS Plugin Template</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Plugin Template</source>
        <translation type="unfinished"></translation>
    </message>
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
        <translation type="unfinished"></translation>
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
</context>
<context>
    <name>QuickPrintPlugin</name>
    <message>
        <source>Quick Print</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Replace this with a short description of the what the plugin does</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;Quick Print</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>RepositoryDetailsDialog</name>
    <message>
        <source>Repository details</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Name:</source>
        <translation type="unfinished">Имя:</translation>
    </message>
    <message>
        <source>URL:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>http://</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>SplashScreen</name>
    <message>
        <source>Quantum GIS - </source>
        <translation type="obsolete">Quantum GIS - </translation>
    </message>
    <message>
        <source>Version </source>
        <translation type="obsolete">Версия</translation>
    </message>
</context>
<context>
    <name>[pluginname]Gui</name>
</context>
<context>
    <name>[pluginname]GuiBase</name>
    <message>
        <source>QGIS Plugin Template</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Plugin Template</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>gpsPage</name>
    <message>
        <source>No</source>
        <translation type="obsolete">Нет</translation>
    </message>
</context>
<context>
    <name>pluginname</name>
    <message>
        <source>Replace this with a short description of the what the plugin does</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>[menuitemname]</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>&amp;[menuname]</source>
        <translation type="unfinished"></translation>
    </message>
</context>
</TS>
