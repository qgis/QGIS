<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS><TS version="1.1" language="ru">
<context>
    <name>@default</name>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1710"/>
        <source>OGR Driver Manager</source>
        <translation type="obsolete">Менеджер драйверов OGR</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1710"/>
        <source>unable to get OGRDriverManager</source>
        <translation type="obsolete">не удалось получить OGRDriverManager</translation>
    </message>
</context>
<context>
    <name>Dialog</name>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="13"/>
        <source>QGIS Plugin Installer</source>
        <translation>Установка модулей QGIS</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="142"/>
        <source>Name of plugin to install</source>
        <translation>Имя модуля для установки</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="61"/>
        <source>Get List</source>
        <translation>Получить список</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="176"/>
        <source>Done</source>
        <translation>Готово</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="152"/>
        <source>Install Plugin</source>
        <translation>Установить модуль</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="163"/>
        <source>The plugin will be installed to ~/.qgis/python/plugins</source>
        <translation>Модуль будет установлен в ~/.qgis/python/plugins</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="117"/>
        <source>Name</source>
        <translation>Имя</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="122"/>
        <source>Version</source>
        <translation>Версия</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="127"/>
        <source>Description</source>
        <translation>Описание</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="132"/>
        <source>Author</source>
        <translation>Автор</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="19"/>
        <source>Select repository, retrieve the list of available plugins, select one and install it</source>
        <translation>Выберите репозиторий, загрузите список доступных модулей и выберите модуль для установки</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="26"/>
        <source>Repository</source>
        <translation>Репозиторий</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="34"/>
        <source>Active repository:</source>
        <translation>Активный репозиторий:</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="81"/>
        <source>Add</source>
        <translation>Добавить</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="88"/>
        <source>Edit</source>
        <translation>Изменить</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/gui.ui" line="95"/>
        <source>Delete</source>
        <translation>Удалить</translation>
    </message>
</context>
<context>
    <name>Gui</name>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="55"/>
        <source>Welcome to your automatically generated plugin!</source>
        <translation>Welcome to your automatically generated plugin!</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="56"/>
        <source>This is just a starting point. You now need to modify the code to make it do something useful....read on for a more information to get yourself started.</source>
        <translation>This is just a starting point. You now need to modify the code to make it do something useful....read on for a more information to get yourself started.</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="57"/>
        <source>Documentation:</source>
        <translation>Documentation:</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="58"/>
        <source>You really need to read the QGIS API Documentation now at:</source>
        <translation>You really need to read the QGIS API Documentation now at:</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="59"/>
        <source>In particular look at the following classes:</source>
        <translation>In particular look at the following classes:</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="62"/>
        <source>QgsPlugin is an ABC that defines required behaviour your plugin must provide. See below for more details.</source>
        <translation>QgsPlugin is an ABC that defines required behaviour your plugin must provide. See below for more details.</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="63"/>
        <source>What are all the files in my generated plugin directory for?</source>
        <translation>What are all the files in my generated plugin directory for?</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="64"/>
        <source>This is the generated CMake file that builds the plugin. You should add you application specific dependencies and source files to this file.</source>
        <translation>This is the generated CMake file that builds the plugin. You should add you application specific dependencies and source files to this file.</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="65"/>
        <source>This is the class that provides the &apos;glue&apos; between your custom application logic and the QGIS application. You will see that a number of methods are already implemented for you - including some examples of how to add a raster or vector layer to the main application map canvas. This class is a concrete instance of the QgisPlugin interface which defines required behaviour for a plugin. In particular, a plugin has a number of static methods and members so that the QgsPluginManager and plugin loader logic can identify each plugin, create an appropriate menu entry for it etc. Note there is nothing stopping you creating multiple toolbar icons and menu entries for a single plugin. By default though a single menu entry and toolbar button is created and its pre-configured to call the run() method in this class when selected. This default implementation provided for you by the plugin builder is well documented, so please refer to the code for further advice.</source>
        <translation>This is the class that provides the &apos;glue&apos; between your custom application logic and the QGIS application. You will see that a number of methods are already implemented for you - including some examples of how to add a raster or vector layer to the main application map canvas. This class is a concrete instance of the QgisPlugin interface which defines required behaviour for a plugin. In particular, a plugin has a number of static methods and members so that the QgsPluginManager and plugin loader logic can identify each plugin, create an appropriate menu entry for it etc. Note there is nothing stopping you creating multiple toolbar icons and menu entries for a single plugin. By default though a single menu entry and toolbar button is created and its pre-configured to call the run() method in this class when selected. This default implementation provided for you by the plugin builder is well documented, so please refer to the code for further advice.</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="66"/>
        <source>This is a Qt designer &apos;ui&apos; file. It defines the look of the default plugin dialog without implementing any application logic. You can modify this form to suite your needs or completely remove it if your plugin does not need to display a user form (e.g. for custom MapTools).</source>
        <translation>This is a Qt designer &apos;ui&apos; file. It defines the look of the default plugin dialog without implementing any application logic. You can modify this form to suite your needs or completely remove it if your plugin does not need to display a user form (e.g. for custom MapTools).</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="67"/>
        <source>This is the concrete class where application logic for the above mentioned dialog should go. The world is your oyster here really....</source>
        <translation>This is the concrete class where application logic for the above mentioned dialog should go. The world is your oyster here really....</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="68"/>
        <source>This is the Qt4 resources file for your plugin. The Makefile generated for your plugin is all set up to compile the resource file so all you need to do is add your additional icons etc using the simple xml file format. Note the namespace used for all your resources e.g. (&apos;:/Homann/&apos;). It is important to use this prefix for all your resources. We suggest you include any other images and run time data in this resurce file too.</source>
        <translation>This is the Qt4 resources file for your plugin. The Makefile generated for your plugin is all set up to compile the resource file so all you need to do is add your additional icons etc using the simple xml file format. Note the namespace used for all your resources e.g. (&apos;:/Homann/&apos;). It is important to use this prefix for all your resources. We suggest you include any other images and run time data in this resurce file too.</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="69"/>
        <source>This is the icon that will be used for your plugin menu entry and toolbar icon. Simply replace this icon with your own icon to make your plugin disctinctive from the rest.</source>
        <translation>This is the icon that will be used for your plugin menu entry and toolbar icon. Simply replace this icon with your own icon to make your plugin disctinctive from the rest.</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="70"/>
        <source>This file contains the documentation you are reading now!</source>
        <translation>This file contains the documentation you are reading now!</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="71"/>
        <source>Getting developer help:</source>
        <translation>Getting developer help:</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="72"/>
        <source>For Questions and Comments regarding the plugin builder template and creating your features in QGIS using the plugin interface please contact us via:</source>
        <translation>For Questions and Comments regarding the plugin builder template and creating your features in QGIS using the plugin interface please contact us via:</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="73"/>
        <source>&lt;li&gt; the QGIS developers mailing list, or &lt;/li&gt;&lt;li&gt; IRC (#qgis on freenode.net)&lt;/li&gt;</source>
        <translation>&lt;li&gt; the QGIS developers mailing list, or &lt;/li&gt;&lt;li&gt; IRC (#qgis on freenode.net)&lt;/li&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="74"/>
        <source>QGIS is distributed under the Gnu Public License. If you create a useful plugin please consider contributing it back to the community.</source>
        <translation>QGIS is distributed under the Gnu Public License. If you create a useful plugin please consider contributing it back to the community.</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugingui.cpp" line="75"/>
        <source>Have fun and thank you for choosing QGIS.</source>
        <translation>Have fun and thank you for choosing QGIS.</translation>
    </message>
</context>
<context>
    <name>MapCoordsDialogBase</name>
    <message>
        <location filename="../src/plugins/georeferencer/mapcoordsdialogbase.ui" line="13"/>
        <source>Enter map coordinates</source>
        <translation>Введите координаты карты</translation>
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
        <translation>О&amp;тменить</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/mapcoordsdialogbase.ui" line="28"/>
        <source>Enter X and Y coordinates which correspond with the selected point on the image. Alternatively, click the button with icon of a pencil and then click a corresponding point on map canvas of QGIS to fill in coordinates of that point.</source>
        <translation>Введите XY-координаты, которые соответствуют выбранной точке на изображении. Либо нажмите на кнопке со значком карандаша и затем щёлкните на соответствующей точке в области карты QGIS для автоматического ввода координат этой точки.</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/mapcoordsdialogbase.ui" line="137"/>
        <source> from map canvas</source>
        <translation> с карты</translation>
    </message>
</context>
<context>
    <name>QFileDialog</name>
    <message>
        <location filename="../src/plugins/quick_print/quickprintgui.cpp" line="108"/>
        <source>Save experiment report to portable document format (.pdf)</source>
        <translation>Сохранить вывод в формате PDF</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="666"/>
        <source>Load layer properties from style file (.qml)</source>
        <translation>Загрузить свойства слоя из файла стиля (.qml)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="727"/>
        <source>Save layer properties as style file (.qml)</source>
        <translation>Сохранить свойства слоя в файл стиля (.qml)</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="../src/core/qgsproviderregistry.cpp" line="92"/>
        <source>No Data Providers</source>
        <translation>Источники данных отсутствуют</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproviderregistry.cpp" line="87"/>
        <source>No Data Provider Plugins</source>
        <comment>No QGIS data provider plugins found in:</comment>
        <translation>Отсутствуют модули источников данных</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproviderregistry.cpp" line="89"/>
        <source>No vector layers can be loaded. Check your QGIS installation</source>
        <translation>Загрузка векторных слоёв невозможна. Проверьте вашу установку QGIS</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproviderregistry.cpp" line="251"/>
        <source>No data provider plugins are available. No vector layers can be loaded</source>
        <translation>Недоступны модули источников данных. Загрузка векторных слоёв невозможна</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Overwrite File?</source>
        <translation type="obsolete">Сохранить Поверх Файла?</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>%1 exists.%2Do you want to overwrite it?</source>
        <translation type="obsolete">%1 уже существует.%2 Сохранить поверх этого файла?</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Yes</source>
        <translation type="obsolete">Да</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>No</source>
        <translation type="obsolete">Нет</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2995"/>
        <source>QGis files (*.qgs)</source>
        <translation>Файлы QGIS (*.qgs)</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Choose a file to open</source>
        <translation type="obsolete">Выберите файл для открытия</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Choose a filename  to save</source>
        <translation type="obsolete">Выберите имя сохраняемого файла</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="771"/>
        <source> at line </source>
        <translation> в строке </translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="772"/>
        <source> column </source>
        <translation>, столбец </translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="778"/>
        <source> for file </source>
        <translation> в файле </translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="934"/>
        <source>Unable to save to file </source>
        <translation>Не удалось сохранить в файл  </translation>
    </message>
    <message>
        <location filename="../src/core/qgssearchtreenode.cpp" line="289"/>
        <source>Referenced column wasn&apos;t found: </source>
        <translation>Упоминаемое поле не найдено: </translation>
    </message>
    <message>
        <location filename="../src/core/qgssearchtreenode.cpp" line="293"/>
        <source>Division by zero.</source>
        <translation>Деление на ноль.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolselect.cpp" line="75"/>
        <source>No active layer</source>
        <translation>Нет активного слоя</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="154"/>
        <source>Band</source>
        <translation>Канал</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="467"/>
        <source>action</source>
        <translation>действие</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="476"/>
        <source> features found</source>
        <translation> объектов найдено</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="480"/>
        <source> 1 feature found</source>
        <translation> найден 1 объект</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="486"/>
        <source>No features found</source>
        <translation>Объектов не найдено</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="486"/>
        <source>No features were found in the active layer at the point you clicked</source>
        <translation>В активном слое не найдено объектов в точке, на которой был произведён щелчок</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="587"/>
        <source>Could not identify objects on</source>
        <translation>Не удалось определить объекты на</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="587"/>
        <source>because</source>
        <translation>потому что</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="72"/>
        <source>New centroid</source>
        <translation>Новый центроид</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="223"/>
        <source>New point</source>
        <translation>Новая точка</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="134"/>
        <source>New vertex</source>
        <translation>Новая вершина</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="223"/>
        <source>Undo last point</source>
        <translation>Отменить последнюю точку</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="223"/>
        <source>Close line</source>
        <translation>Закрыть линию</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="543"/>
        <source>Select vertex</source>
        <translation>Выбрать вершину</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="296"/>
        <source>Select new position</source>
        <translation>Выбрать новую позицию</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="427"/>
        <source>Select line segment</source>
        <translation>Выбрать сегмент линии</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="414"/>
        <source>New vertex position</source>
        <translation>Новая позиция вершины</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="414"/>
        <source>Release</source>
        <translation>Освободить</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="530"/>
        <source>Delete vertex</source>
        <translation>Удалить вершину</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="530"/>
        <source>Release vertex</source>
        <translation>Освободить вершину</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="784"/>
        <source>Select element</source>
        <translation>Выбрать элемент</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="597"/>
        <source>New location</source>
        <translation>Новое положение</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="673"/>
        <source>Release selected</source>
        <translation>Освободить выделение</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="673"/>
        <source>Delete selected / select next</source>
        <translation>Удалить выделение / выбрать следующий</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="736"/>
        <source>Select position on line</source>
        <translation>Выбрать позицию на линии</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="754"/>
        <source>Split the line</source>
        <translation>Разделить линию</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="754"/>
        <source>Release the line</source>
        <translation>Освободить линию</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="768"/>
        <source>Select point on line</source>
        <translation>Выбрать точку на линии</translation>
    </message>
    <message>
        <location filename="../src/core/qgslabelattributes.cpp" line="58"/>
        <source>Label</source>
        <translation>Подпись</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="454"/>
        <source>Length</source>
        <translation>Длина</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="460"/>
        <source>Area</source>
        <translation>Площадь</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="771"/>
        <source>Project file read error: </source>
        <translation>Ошибка чтения файла проекта: </translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsleastsquares.cpp" line="32"/>
        <source>Fit to a linear transform requires at least 2 points.</source>
        <translation>Для привязки с линейным преобразованием необходимо как минимум 2 точки.</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsleastsquares.cpp" line="71"/>
        <source>Fit to a Helmert transform requires at least 2 points.</source>
        <translation>Для привязки с преобразованием Гельмерта необходимо как минимум 2 точки.</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsleastsquares.cpp" line="123"/>
        <source>Fit to an affine transform requires at least 4 points.</source>
        <translation>Для привязки с аффинным преобразованием необходимо как минимум 4 точки.</translation>
    </message>
    <message>
        <location filename="../src/providers/gpx/gpsdata.cpp" line="332"/>
        <source>Couldn&apos;t open the data source: </source>
        <translation>Не удалось открыть источник данных: </translation>
    </message>
    <message>
        <location filename="../src/providers/gpx/gpsdata.cpp" line="354"/>
        <source>Parse error at line </source>
        <translation>Ошибка разбора в строке </translation>
    </message>
    <message>
        <location filename="../src/providers/gpx/qgsgpxprovider.cpp" line="55"/>
        <source>GPS eXchange format provider</source>
        <translation>Источник данных GPS eXchange</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="303"/>
        <source>Caught a coordinate system exception while trying to transform a point. Unable to calculate line length.</source>
        <translation>Перехвачено исключение системы координат при попытке преобразования точки. Не удалось определить длину линии.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="394"/>
        <source>Caught a coordinate system exception while trying to transform a point. Unable to calculate polygon area.</source>
        <translation>Перехвачено исключение системы координат при попытке преобразования точки. Не удалось определить площадь полигона.</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="166"/>
        <source>GRASS plugin</source>
        <translation>Модуль GRASS</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="136"/>
        <source>QGIS couldn&apos;t find your GRASS installation.
Would you like to specify path (GISBASE) to your GRASS installation?</source>
        <translation>Не удалось найти установленную систему GRASS.
Вы хотите указать путь (GISBASE) к вашей установке GRASS?</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="150"/>
        <source>Choose GRASS installation path (GISBASE)</source>
        <translation>Выберите путь установки GRASS (GISBASE)</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="167"/>
        <source>GRASS data won&apos;t be available if GISBASE is not specified.</source>
        <translation>Данные GRASS будут недоступны, если не задано значение GISBASE.</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="51"/>
        <source>CopyrightLabel</source>
        <translation>Метка авторского права</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="52"/>
        <source>Draws copyright information</source>
        <translation>Добавляет значок авторского права</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfsplugin.cpp" line="31"/>
        <source>Version 0.1</source>
        <translation>Версия 0.1</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="44"/>
        <source>Version 0.2</source>
        <translation>Версия 0.2</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="45"/>
        <source>Loads and displays delimited text files containing x,y coordinates</source>
        <translation>Загружает и показывает текстовые файлы, содержащие x,y-координаты</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="161"/>
        <source>Add Delimited Text Layer</source>
        <translation>Текст с разделителями</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugin.cpp" line="57"/>
        <source>Georeferencer</source>
        <translation>Привязка растров</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugin.cpp" line="58"/>
        <source>Adding projection info to rasters</source>
        <translation>Добавление сведений о проекции к растрам</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="55"/>
        <source>GPS Tools</source>
        <translation>GPS-инструменты</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="57"/>
        <source>Tools for loading and importing GPS data</source>
        <translation>Инструменты для загрузки и импорта GPS-данных</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="829"/>
        <source>GRASS</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="835"/>
        <source>GRASS layer</source>
        <translation>Поддержка GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugin.cpp" line="43"/>
        <source>Graticule Creator</source>
        <translation>Конструктор сетки</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugin.cpp" line="44"/>
        <source>Builds a graticule</source>
        <translation>Модуль построения сетки</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="60"/>
        <source>NorthArrow</source>
        <translation>Указатель «север-юг»</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="61"/>
        <source>Displays a north arrow overlayed onto the map</source>
        <translation>Добавляет на карту указатель «север-юг»</translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugin.cpp" line="38"/>
        <source>[menuitemname]</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugin.cpp" line="39"/>
        <source>[plugindescription]</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="62"/>
        <source>ScaleBar</source>
        <translation>Масштабная линейка</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="63"/>
        <source>Draws a scale bar</source>
        <translation>Накладывает масштабную линейку</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitplugin.cpp" line="37"/>
        <source>SPIT</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitplugin.cpp" line="38"/>
        <source>Shapefile to PostgreSQL/PostGIS Import Tool</source>
        <translation>Инструмент импорта shape-файлов в PostgreSQL/PostGIS</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfsplugin.cpp" line="29"/>
        <source>WFS plugin</source>
        <translation>Модуль WFS</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfsplugin.cpp" line="30"/>
        <source>Adds WFS layers to the QGIS canvas</source>
        <translation>Добавляет на карту WFS-слои</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Version 0.0001</source>
        <translation type="obsolete">Версия 0.0001</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="42"/>
        <source>Not a vector layer</source>
        <translation>Слой не является векторным</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="43"/>
        <source>The current layer is not a vector layer</source>
        <translation>Текущий слой не является векторным</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="72"/>
        <source>Layer cannot be added to</source>
        <translation>Слой не может быть добавлен в</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="73"/>
        <source>The data provider for this layer does not support the addition of features.</source>
        <translation>Источник данных для этого слоя не поддерживает добавление объектов.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="49"/>
        <source>Layer not editable</source>
        <translation>Нередактируемый слой</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="51"/>
        <source>Cannot edit the vector layer. To make it editable, go to the file item of the layer, right click and check &apos;Allow Editing&apos;.</source>
        <translation>Не удалось внести изменения в векторный слой. Для редактирования слоя, щёлкните на его имени в легенде правой кнопкой мыши и выберите «Режим редактирования».</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolselect.cpp" line="76"/>
        <source>To select features, you must choose a vector layer by clicking on its name in the legend</source>
        <translation>Для выделения объектов необходимо выбрать векторный слой щелчком мыши на его имени в легенде</translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="193"/>
        <source>Python error</source>
        <translation>Ошибка Python</translation>
    </message>
    <message>
        <location filename="../src/app/qgspythonutils.cpp" line="57"/>
        <source>Couldn&apos;t load SIP module.
Python support will be disabled.</source>
        <translation type="obsolete">Не удалось загрузить модуль SIP.
Поддержка Python будет выключена.</translation>
    </message>
    <message>
        <location filename="../src/app/qgspythonutils.cpp" line="67"/>
        <source>Couldn&apos;t load PyQt bindings.
Python support will be disabled.</source>
        <translation type="obsolete">Не удалось загрузить библиотеки PyQt.
Поддержка Python будет выключена.</translation>
    </message>
    <message>
        <location filename="../src/app/qgspythonutils.cpp" line="78"/>
        <source>Couldn&apos;t load QGIS bindings.
Python support will be disabled.</source>
        <translation type="obsolete">Не удалось загрузить привязки QGIS.
Поддержка Python будет выключена.</translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="477"/>
        <source>Couldn&apos;t load plugin </source>
        <translation>Не удалось загрузить модуль </translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="481"/>
        <source> due an error when calling its classFactory() method</source>
        <translation> из-за ошибки вызова его метода classFactory()</translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="485"/>
        <source> due an error when calling its initGui() method</source>
        <translation> из-за ошибки вызова его метода initGui()</translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="497"/>
        <source>Error while unloading plugin </source>
        <translation>Ошибка при выгрузке модуля </translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="59"/>
        <source>2.5D shape type not supported</source>
        <translation>2.5-мерные данные не поддерживаются</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="59"/>
        <source>Adding features to 2.5D shapetypes is not supported yet</source>
        <translation>Добавление 2.5-мерных объектов в данный момент не поддерживается</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="207"/>
        <source>Wrong editing tool</source>
        <translation>Неверный инструмент редактирования</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="92"/>
        <source>Cannot apply the &apos;capture point&apos; tool on this vector layer</source>
        <translation>Не удалось применить инструмент «захватить точку» на этот векторный слой </translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="65"/>
        <source>Coordinate transform error</source>
        <translation>Ошибка преобразования координат</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="66"/>
        <source>Cannot transform the point to the layers coordinate system</source>
        <translation>Не удалось преобразовать точку в систему координат слоя</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="200"/>
        <source>Cannot apply the &apos;capture line&apos; tool on this vector layer</source>
        <translation>Не удалось применить инструмент «захватить линию» на этот векторный слой </translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="208"/>
        <source>Cannot apply the &apos;capture polygon&apos; tool on this vector layer</source>
        <translation>Не удалось применить инструмент «захватить полигон» на этот векторный слой </translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="427"/>
        <source>Error</source>
        <translation>Ошибка</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="416"/>
        <source>Cannot add feature. Unknown WKB type</source>
        <translation>Не удалось добавить объект, неизвестный тип WKB</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdisland.cpp" line="113"/>
        <source>Error, could not add island</source>
        <translation>Ошибка, не удалось добавить остров</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="91"/>
        <source>A problem with geometry type occured</source>
        <translation>Ошибка типа геометрии</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="95"/>
        <source>The inserted Ring is not closed</source>
        <translation>Вставляемое кольцо не замкнуто</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="99"/>
        <source>The inserted Ring is not a valid geometry</source>
        <translation>Вставляемое кольцо не является действительной геометрией</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="103"/>
        <source>The inserted Ring crosses existing rings</source>
        <translation>Вставляемое кольцо пересекает существующие кольца</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="107"/>
        <source>The inserted Ring is not contained in a feature</source>
        <translation>Вставляемое кольцо располагается вне границ объекта</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="111"/>
        <source>An unknown error occured</source>
        <translation>Неизвестная ошибка</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdring.cpp" line="113"/>
        <source>Error, could not add ring</source>
        <translation>Ошибка, не удалось добавить кольцо</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Data provider of the current layer doesn&apos;t allow changing geometries</source>
        <translation type="obsolete">Источник данных текущего слоя не позволяет изменять геометрию</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="640"/>
        <source> km2</source>
        <translation> км2</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="645"/>
        <source> ha</source>
        <translation> га</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="650"/>
        <source> m2</source>
        <translation> м2</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="678"/>
        <source> m</source>
        <translation> м</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="663"/>
        <source> km</source>
        <translation> км</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="668"/>
        <source> mm</source>
        <translation> мм</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="673"/>
        <source> cm</source>
        <translation> см</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="687"/>
        <source> sq mile</source>
        <translation> кв. миль</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="692"/>
        <source> sq ft</source>
        <translation> кв. футов</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="699"/>
        <source> mile</source>
        <translation> миль</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="705"/>
        <source> foot</source>
        <translation> фут</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="707"/>
        <source> feet</source>
        <translation> футов</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="714"/>
        <source> sq.deg.</source>
        <translation> кв. град.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="719"/>
        <source> degree</source>
        <translation> градус</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="721"/>
        <source> degrees</source>
        <translation> градусов</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="725"/>
        <source> unknown</source>
        <translation> неизв</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="273"/>
        <source>Received %1 of %2 bytes</source>
        <translation>Получено %1 из %2 байт</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="279"/>
        <source>Received %1 bytes (total unknown)</source>
        <translation>Получено %1 байт (размер неизвестен)</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="390"/>
        <source>Not connected</source>
        <translation>Нет соединения</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="396"/>
        <source>Looking up &apos;%1&apos;</source>
        <translation>Поиск «%1»</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="403"/>
        <source>Connecting to &apos;%1&apos;</source>
        <translation>Соединение с «%1»</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="410"/>
        <source>Sending request &apos;%1&apos;</source>
        <translation>Отправка запроса «%1»</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="417"/>
        <source>Receiving reply</source>
        <translation>Получение ответа</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="423"/>
        <source>Response is complete</source>
        <translation>Ответ получен</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="429"/>
        <source>Closing down connection</source>
        <translation>Соединение закрыто</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="753"/>
        <source>Unable to open </source>
        <translation>Не удалось открыть </translation>
    </message>
    <message>
        <location filename="../src/core/qgssearchtreenode.cpp" line="253"/>
        <source>Regular expressions on numeric values don&apos;t make sense. Use comparison instead.</source>
        <translation>Регулярные выражения не имеют смысла для числовых значений. Используйте сравнение.</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="48"/>
        <source>Geoprocessing functions for working with PostgreSQL/PostGIS layers</source>
        <translation>Функции пространственной обработки для слоёв PostgreSQL/PostGIS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="137"/>
        <source>Location: </source>
        <comment>Metadata in GRASS Browser</comment>
        <translation>Район: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="137"/>
        <source>&lt;br&gt;Mapset: </source>
        <comment>Metadata in GRASS Browser</comment>
        <translation>&lt;br&gt;Набор: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="141"/>
        <source>Location: </source>
        <translation>Район: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="141"/>
        <source>&lt;br&gt;Mapset: </source>
        <translation>&lt;br&gt;Набор: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="146"/>
        <source>&lt;b&gt;Raster&lt;/b&gt;</source>
        <translation>&lt;b&gt;Растр&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="155"/>
        <source>Cannot open raster header</source>
        <translation>Не удалось открыть заголовок растра</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="159"/>
        <source>Rows</source>
        <translation>Строк</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="160"/>
        <source>Columns</source>
        <translation>Столбцов</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="161"/>
        <source>N-S resolution</source>
        <translation>Вертикальное разрешение</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="162"/>
        <source>E-W resolution</source>
        <translation>Горизонтальное разрешение</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="300"/>
        <source>North</source>
        <translation>Север</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="302"/>
        <source>South</source>
        <translation>Юг</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="304"/>
        <source>East</source>
        <translation>Восток</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="306"/>
        <source>West</source>
        <translation>Запад</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="189"/>
        <source>Format</source>
        <translation>Формат</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="200"/>
        <source>Minimum value</source>
        <translation>Мин. значение</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="201"/>
        <source>Maximum value</source>
        <translation>Макс. значение</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="212"/>
        <source>Data source</source>
        <translation>Источник данных</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="217"/>
        <source>Data description</source>
        <translation>Описание данных</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="226"/>
        <source>Comments</source>
        <translation>Комментарии</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="241"/>
        <source>Categories</source>
        <translation>Категории</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="347"/>
        <source>&lt;b&gt;Vector&lt;/b&gt;</source>
        <translation>&lt;b&gt;Вектор&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="274"/>
        <source>Points</source>
        <translation>Точки</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="275"/>
        <source>Lines</source>
        <translation>Линии</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="276"/>
        <source>Boundaries</source>
        <translation>Границы</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="277"/>
        <source>Centroids</source>
        <translation>Центроиды</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="280"/>
        <source>Faces</source>
        <translation>Грани</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="281"/>
        <source>Kernels</source>
        <translation>Ядра</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="284"/>
        <source>Areas</source>
        <translation>Площади</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="285"/>
        <source>Islands</source>
        <translation>Острова</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="309"/>
        <source>Top</source>
        <translation>Верх</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="310"/>
        <source>Bottom</source>
        <translation>Низ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="313"/>
        <source>yes</source>
        <translation>да</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="313"/>
        <source>no</source>
        <translation>нет</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="320"/>
        <source>History&lt;br&gt;</source>
        <translation>История&lt;br&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="348"/>
        <source>&lt;b&gt;Layer&lt;/b&gt;</source>
        <translation>&lt;b&gt;Слой&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="367"/>
        <source>Features</source>
        <translation>Объектов</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="376"/>
        <source>Driver</source>
        <translation>Драйвер</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="377"/>
        <source>Database</source>
        <translation>База данных</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="378"/>
        <source>Table</source>
        <translation>Таблица</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="379"/>
        <source>Key column</source>
        <translation>Ключевое поле</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="453"/>
        <source>GISBASE is not set.</source>
        <translation>GISBASE не задана.</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="458"/>
        <source> is not a GRASS mapset.</source>
        <translation> не является набором GRASS.</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="480"/>
        <source>Cannot start </source>
        <translation>Не удалось запустить </translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="497"/>
        <source>Mapset is already in use.</source>
        <translation>Набор уже используется.</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="512"/>
        <source>Temporary directory </source>
        <translation>Временный каталог </translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="512"/>
        <source> exist but is not writable</source>
        <translation> существует, но закрыт для записи</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="518"/>
        <source>Cannot create temporary directory </source>
        <translation>Не удаётся создать временный каталог </translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="534"/>
        <source>Cannot create </source>
        <translation>Не удаётся создать </translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="611"/>
        <source>Cannot remove mapset lock: </source>
        <translation>Не удалось снять блокировку набора: </translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="1051"/>
        <source>Warning</source>
        <translation>Внимание</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="999"/>
        <source>Cannot read raster map region</source>
        <translation>Не удаётся прочесть границы растровой карты</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="1016"/>
        <source>Cannot read vector map region</source>
        <translation>Не удаётся прочесть границы векторной карты</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="1052"/>
        <source>Cannot read region</source>
        <translation>Не удаётся прочесть регион</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2422"/>
        <source>Where is &apos;</source>
        <translation>Где искать &apos;</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2422"/>
        <source>original location: </source>
        <translation>оригинальное местоположение: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="123"/>
        <source>To identify features, you must choose an active layer by clicking on its name in the legend</source>
        <translation>Чтобы определить объекты, следует выбрать активный слой щелчком на имени слоя в легенде</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="47"/>
        <source>PostgreSQL Geoprocessing</source>
        <translation>Пространственная обработка PostgreSQL</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintplugin.cpp" line="38"/>
        <source>Quick Print</source>
        <translation>Быстрая печать</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintplugin.cpp" line="40"/>
        <source>Quick Print is a plugin to quickly print a map with minimal effort.</source>
        <translation>Модуль для быстрой печати карт с минимумом параметров.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="427"/>
        <source>Could not remove polygon intersection</source>
        <translation>Не удалось удалить пересечение полигонов</translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="428"/>
        <source>Currently only filebased datasets are supported</source>
        <translation type="obsolete">Данная функция пока доступна только для файловых данных</translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="521"/>
        <source>Loaded default style file from </source>
        <translation>Стиль по умолчанию загружен из </translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="552"/>
        <source>The directory containing your dataset needs to be writeable!</source>
        <translation>Необходимы права на запись в каталог, содержащий ваши данные!</translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="565"/>
        <source>Created default style file as </source>
        <translation>Файл стиля по умолчанию создан в </translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="536"/>
        <source>ERROR: Failed to created default style file as </source>
        <translation type="obsolete">ОШИБКА: Не удалось сохранить файл стиля по умолчанию как </translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="433"/>
        <source>File could not been opened.</source>
        <translation type="obsolete">Файл не может быть открыт.</translation>
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
        <translation type="unfinished">Отменить</translation>
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
        <translation type="unfinished">Версия 0.001</translation>
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
        <location filename="" line="0"/>
        <source>Layers</source>
        <translation type="obsolete">Слои</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="345"/>
        <source>Quantum GIS - </source>
        <translation>Quantum GIS — </translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Zoom to extent of selected layer</source>
        <translation type="obsolete">&amp;Изменить вид до полной данного слоя</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Open attribute table</source>
        <translation type="obsolete">&amp;Открыть таблицу атрибутов</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Properties</source>
        <translation type="obsolete">&amp;Свойства</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Remove</source>
        <translation type="obsolete">&amp;Удалить</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1529"/>
        <source>Version </source>
        <translation type="obsolete">Версия</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1536"/>
        <source> with PostgreSQL support</source>
        <translation type="obsolete"> с поддержкой PostgreSQL</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1539"/>
        <source> (no PostgreSQL support)</source>
        <translation type="obsolete"> (без поддержки PostgreSQL)</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Web Page: http://qgis.sourceforge.net</source>
        <translation type="obsolete">Веб Страничка: http://qgis.sourceforge.net</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Sourceforge Project Page: http://sourceforge.net/projects/qgis</source>
        <translation type="obsolete">Веб Страничка Проекта: http://sourceforge.net/projects/qgis</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1625"/>
        <source>Version</source>
        <translation>Версия</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1586"/>
        <source>Available Data Provider Plugins</source>
        <translation type="obsolete">Доступные модули источников данных</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Shapefiles (*.shp);;All files (*.*)</source>
        <translation type="obsolete">Shapefiles файлы (*.shp);;Все файлы (*.*)</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Select one or more layers to add</source>
        <translation type="obsolete">Выберите один или более слоёв для добавления</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2081"/>
        <source>is not a valid or recognized data source</source>
        <translation>не является действительным источником данных</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5184"/>
        <source>Invalid Data Source</source>
        <translation>Недопустимый источник данных</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>No OGR Provider</source>
        <translation type="obsolete">Нет Поставщика OGR</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>No OGR data provider was found in the QGIS lib directory</source>
        <translation type="obsolete">Поставщика данных OGR в QGIS lib не найдено</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>No PostgreSQL Provider</source>
        <translation type="obsolete">Нет Поставщика PostgreSQL</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>No PostgreSQL data provider was found in the QGIS lib directory</source>
        <translation type="obsolete">Поставщика данных PostgreSQL в QGIS lib не найдено</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Quantum GIS -- Untitled</source>
        <translation type="obsolete">Quantum GIS -- Без имени</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Quantum GIS --</source>
        <translation type="obsolete">Quantum GIS --</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Saved map to:</source>
        <translation type="obsolete">Карта сохранена в:</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3487"/>
        <source>No Layer Selected</source>
        <translation>Слой не выбран</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>To open an attribute table, you must select a layer in the legend</source>
        <translation type="obsolete">Чтобы открыть таблицу атрибутов, выберите слой в легенде</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4048"/>
        <source>No MapLayer Plugins</source>
        <translation>Не найдены модули MapLayer</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4048"/>
        <source>No MapLayer plugins in ../plugins/maplayer</source>
        <translation>Модули MapLayer не найдены в ../plugins/maplayer</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4125"/>
        <source>No Plugins</source>
        <translation>Модулей не найдено</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4126"/>
        <source>No plugins found in ../plugins. To test plugins, start qgis from the src directory</source>
        <translation>Модули в каталоге ../plugins не найдены. Для проверки модулей, запустите qgis из каталога src</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4160"/>
        <source>Name</source>
        <translation>Имя</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4160"/>
        <source>Plugin %1 is named %2</source>
        <translation>Модуль %1 назван %2</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4177"/>
        <source>Plugin Information</source>
        <translation>Сведения о модуле</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4178"/>
        <source>QGis loaded the following plugin:</source>
        <translation>Следующий модуль загружен в QGIS:</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4178"/>
        <source>Name: %1</source>
        <translation>Имя: %1</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4178"/>
        <source>Version: %1</source>
        <translation>Версия: %1</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4179"/>
        <source>Description: %1</source>
        <translation>Описание: %1</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4197"/>
        <source>Unable to Load Plugin</source>
        <translation>Не удалось загрузить модуль</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4198"/>
        <source>QGIS was unable to load the plugin from: %1</source>
        <translation>QGIS не удалось загрузить модуль из: %1</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4254"/>
        <source>There is a new version of QGIS available</source>
        <translation>Доступна новая версия QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4260"/>
        <source>You are running a development version of QGIS</source>
        <translation>Вы используете разрабатываемую версию QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4264"/>
        <source>You are running the current version of QGIS</source>
        <translation>Вы используете последнюю версию QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4269"/>
        <source>Would you like more information?</source>
        <translation>Вы хотите получить дополнительную информацию?</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4316"/>
        <source>QGIS Version Information</source>
        <translation>Информация о версии QGIS</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Yes</source>
        <translation type="obsolete">Да</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>No</source>
        <translation type="obsolete">Нет</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>QGIS - Changes in CVS</source>
        <translation type="obsolete">QGIS - Изменения в CVS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4288"/>
        <source>Unable to get current version information from server</source>
        <translation>Не удалось получить информацию о версии с сервера</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4302"/>
        <source>Connection refused - server may be down</source>
        <translation>В соединении отказано — вероятно, сервер недоступен</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4305"/>
        <source>QGIS server was not found</source>
        <translation>Сервер QGIS не найден</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Error reading from server</source>
        <translation type="obsolete">Ошибка чтения с сервера</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Unable to connect to the QGIS Version server</source>
        <translation type="obsolete">Контакт с QGIS сервером версий невозможен</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2173"/>
        <source>Invalid Layer</source>
        <translation>Недействительный слой</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2173"/>
        <source>%1 is an invalid layer and cannot be loaded.</source>
        <translation>Слой %1 не является действительным и не может быть загружен.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4012"/>
        <source>Error Loading Plugin</source>
        <translation>Ошибка загрузки модуля</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4012"/>
        <source>There was an error loading %1.</source>
        <translation>При загрузке модуля %1 возникла ошибка.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3276"/>
        <source>Saved map image to</source>
        <translation>Сохранить снимок карты в</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3234"/>
        <source>Choose a filename to save the map image as</source>
        <translation>Выберите имя файла для сохранения снимка карты</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4684"/>
        <source>Extents: </source>
        <translation>Границы: </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3517"/>
        <source>Problem deleting features</source>
        <translation>Ошибка удаления объектов</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3518"/>
        <source>A problem occured during deletion of features</source>
        <translation>При удалении объектов возникла ошибка</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3495"/>
        <source>No Vector Layer Selected</source>
        <translation>Не выбран векторный слой</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3496"/>
        <source>Deleting features only works on vector layers</source>
        <translation>Удаление объектов работает только для векторных слоёв</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3488"/>
        <source>To delete features, you must select a vector layer in the legend</source>
        <translation>Для удаления объектов, следует выбрать в легенде векторный слой</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1552"/>
        <source>Quantum GIS is licensed under the GNU General Public License</source>
        <translation type="obsolete">Quantum GIS выпущена под Стандартной Общественной Лицензией GNU</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1554"/>
        <source>http://www.gnu.org/licenses</source>
        <translation type="obsolete">http://www.gnu.org/licenses</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1464"/>
        <source>Map legend that displays all the layers currently on the map canvas. Click on the check box to turn a layer on or off. Double click on a layer in the legend to customize its appearance and set other properties.</source>
        <translation>Легенда карты, в которой перечислены все слои отображаемой карты. Щёлкните на флажке, чтобы переключить видимость соответствующего слоя. Дважды щёлкните на имени слоя, чтобы задать его отображение и другие свойства.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1425"/>
        <source>Map overview canvas. This canvas can be used to display a locator map that shows the current extent of the map canvas. The current extent is shown as a red rectangle. Any layer on the map can be added to the overview canvas.</source>
        <translation>Область обзора карты. Данная область используется для вывода обзорной карты, на которой виден текущий экстент области карты QGIS. Текущий экстент нарисован в виде красного прямоугольника. Любой слой карты может быть добавлен в обзорную область.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1314"/>
        <source>Map canvas. This is where raster and vector layers are displayed when added to the map</source>
        <translation type="obsolete">Область карты. В этом месте отображаются добавленные на карту растровые и векторные слои</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1023"/>
        <source>&amp;Plugins</source>
        <translation>&amp;Модули</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1160"/>
        <source>Progress bar that displays the status of rendering layers and other time-intensive operations</source>
        <translation>Индикатор хода процесса отрисовки слоёв и других долговременных операций</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1189"/>
        <source>Displays the current map scale</source>
        <translation>Показывает текущий масштаб карты</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1215"/>
        <source>Render</source>
        <translation>Отрисовка</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1221"/>
        <source>When checked, the map layers are rendered in response to map navigation commands and other events. When not checked, no rendering is done. This allows you to add a large number of layers and symbolize them before rendering.</source>
        <translation>Если включено, отрисовка слоёв карты выполняется сразу в ответ на команды навигации и другие события. Если выключено, отрисовка не выполняется. К примеру, это позволяет добавить большое количество слоёв и назначить им условные обозначения до их отображения.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>QGis files (*.qgs)</source>
        <translation type="obsolete">QGIS файлы (*.qgs)</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2921"/>
        <source>Choose a QGIS project file</source>
        <translation>Выберите файл проекта QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3045"/>
        <source>Unable to save project</source>
        <translation>Не удалось сохранить проект</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3046"/>
        <source>Unable to save project to </source>
        <translation>Не удалось сохранить проект в </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1222"/>
        <source>Toggle map rendering</source>
        <translation>Переключить отрисовку карты</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1245"/>
        <source>This icon shows whether on the fly projection is enabled or not. Click the icon to bring up the project properties dialog to alter this behaviour.</source>
        <translation>Этот значок сигнализирует о включенном преообразовании проекции «на лету». Щёлкните на нём для открытия диалога свойств проекта, в котором можно изменить этот параметр.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1247"/>
        <source>Projection status - Click to open projection dialog</source>
        <translation>Статус проекции — щёлкните для открытия диалога проекции</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2027"/>
        <source>Open an OGR Supported Vector Layer</source>
        <translation>Открыть OGR-совместимый векторный слой</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2649"/>
        <source>Save As</source>
        <translation>Сохранить как</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2749"/>
        <source>Choose a QGIS project file to open</source>
        <translation>Выберите открываемый файл проекта QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2864"/>
        <source>QGIS Project Read Error</source>
        <translation>Ошибка чтения проекта QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2866"/>
        <source>Try to find missing layers?</source>
        <translation>Попытаться найти недостающие слои?</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3038"/>
        <source>Saved project to:</source>
        <translation>Проект сохранён в:</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4265"/>
        <source>QGIS Browser Selection</source>
        <translation type="obsolete">Выбор браузера QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4266"/>
        <source>Enter the name of a web browser to use (eg. konqueror).
</source>
        <translation type="obsolete">Введите имя используемого веб-браузера (напр. konqueror).
</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4267"/>
        <source>Enter the full path if the browser is not in your PATH.
</source>
        <translation type="obsolete">Введите полный путь, если браузер установлен вне PATH.
</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5092"/>
        <source>Open a GDAL Supported Raster Data Source</source>
        <translation>Открыть GDAL-совместимый источник растровых данных</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="318"/>
        <source>Reading settings</source>
        <translation>Загрузка параметров</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="322"/>
        <source>Setting up the GUI</source>
        <translation>Настройка пользовательского интерфейса</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="312"/>
        <source>Checking database</source>
        <translation>Проверка базы данных</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="405"/>
        <source>Restoring loaded plugins</source>
        <translation>Восстановление загруженных модулей</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="409"/>
        <source>Initializing file filters</source>
        <translation>Инициализация файловых фильтров</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="434"/>
        <source>Restoring window state</source>
        <translation>Восстановление состояния окна</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="438"/>
        <source>QGIS Ready!</source>
        <translation>QGIS Готова!</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="548"/>
        <source>&amp;New Project</source>
        <translation>&amp;Новый проект</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="549"/>
        <source>Ctrl+N</source>
        <comment>New Project</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="550"/>
        <source>New Project</source>
        <translation>Новый проект</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="553"/>
        <source>&amp;Open Project...</source>
        <translation>&amp;Открыть проект...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="554"/>
        <source>Ctrl+O</source>
        <comment>Open a Project</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="555"/>
        <source>Open a Project</source>
        <translation>Открыть проект</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="558"/>
        <source>&amp;Save Project</source>
        <translation>&amp;Сохранить проект</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="559"/>
        <source>Ctrl+S</source>
        <comment>Save Project</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="560"/>
        <source>Save Project</source>
        <translation>Сохранить проект</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="563"/>
        <source>Save Project &amp;As...</source>
        <translation>Сохранить проект &amp;как...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="564"/>
        <source>Ctrl+A</source>
        <comment>Save Project under a new name</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="565"/>
        <source>Save Project under a new name</source>
        <translation>Сохранить проект под другим именем</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="568"/>
        <source>&amp;Print...</source>
        <translation>&amp;Печать...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="569"/>
        <source>Ctrl+P</source>
        <comment>Print</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="570"/>
        <source>Print</source>
        <translation>Печать</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="573"/>
        <source>Save as Image...</source>
        <translation>Сохранить как изображение...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="574"/>
        <source>Ctrl+I</source>
        <comment>Save map as image</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="575"/>
        <source>Save map as image</source>
        <translation>Сохранить карту как изображение</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Export to MapServer Map...</source>
        <translation type="obsolete">Экспорт в карту MapServer...</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Export as MapServer .map file</source>
        <translation type="obsolete">Экспорт в формат MapServer .map</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="586"/>
        <source>Exit</source>
        <translation>Выйти</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="587"/>
        <source>Ctrl+Q</source>
        <comment>Exit QGIS</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="588"/>
        <source>Exit QGIS</source>
        <translation>Выйти из QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="593"/>
        <source>Add a Vector Layer...</source>
        <translation>Добавить векторный слой...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="594"/>
        <source>V</source>
        <comment>Add a Vector Layer</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="595"/>
        <source>Add a Vector Layer</source>
        <translation>Добавить векторный слой</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="598"/>
        <source>Add a Raster Layer...</source>
        <translation>Добавить растровый слой...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="599"/>
        <source>R</source>
        <comment>Add a Raster Layer</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="600"/>
        <source>Add a Raster Layer</source>
        <translation>Добавить растровый слой</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="603"/>
        <source>Add a PostGIS Layer...</source>
        <translation>Добавить слой PostGIS...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="604"/>
        <source>D</source>
        <comment>Add a PostGIS Layer</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="605"/>
        <source>Add a PostGIS Layer</source>
        <translation>Добавить слой PostGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="615"/>
        <source>New Vector Layer...</source>
        <translation>Новый векторный слой...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="616"/>
        <source>N</source>
        <comment>Create a New Vector Layer</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="617"/>
        <source>Create a New Vector Layer</source>
        <translation>Создать новый векторный слой</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="620"/>
        <source>Remove Layer</source>
        <translation>Удалить слой</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="621"/>
        <source>Ctrl+D</source>
        <comment>Remove a Layer</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="622"/>
        <source>Remove a Layer</source>
        <translation>Удалить слой</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="626"/>
        <source>Add All To Overview</source>
        <translation>Добавить все в обзор</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="627"/>
        <source>+</source>
        <comment>Show all layers in the overview map</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="628"/>
        <source>Show all layers in the overview map</source>
        <translation>Показать все слои на обзорной карте</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="631"/>
        <source>Remove All From Overview</source>
        <translation>Удалить все из обзора</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="632"/>
        <source>-</source>
        <comment>Remove all layers from overview map</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="633"/>
        <source>Remove all layers from overview map</source>
        <translation>Удалить все слои с обзорной карты</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="642"/>
        <source>Show All Layers</source>
        <translation>Показать все слои</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="643"/>
        <source>S</source>
        <comment>Show all layers</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="644"/>
        <source>Show all layers</source>
        <translation>Показать все слои</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="647"/>
        <source>Hide All Layers</source>
        <translation>Скрыть все слои</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="648"/>
        <source>H</source>
        <comment>Hide all layers</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="649"/>
        <source>Hide all layers</source>
        <translation>Скрыть все слои</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="654"/>
        <source>Project Properties...</source>
        <translation>Свойства проекта...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="655"/>
        <source>P</source>
        <comment>Set project properties</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="656"/>
        <source>Set project properties</source>
        <translation>Задать параметры проекта</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="659"/>
        <source>Options...</source>
        <translation>Настройки...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="661"/>
        <source>Change various QGIS options</source>
        <translation>Изменить настройки QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="664"/>
        <source>Custom Projection...</source>
        <translation>Пользовательская проекция...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="666"/>
        <source>Manage custom projections</source>
        <translation>Управление пользовательскими проекциями</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="671"/>
        <source>Help Contents</source>
        <translation>Содержание</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="675"/>
        <source>F1</source>
        <comment>Help Documentation</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="677"/>
        <source>Help Documentation</source>
        <translation>Открыть руководство по программе</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="680"/>
        <source>Qgis Home Page</source>
        <translation>Веб-сайт Qgis</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="682"/>
        <source>Ctrl+H</source>
        <comment>QGIS Home Page</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="684"/>
        <source>QGIS Home Page</source>
        <translation>Веб-сайт QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="687"/>
        <source>About</source>
        <translation>О программе</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="688"/>
        <source>About QGIS</source>
        <translation>О программе QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="691"/>
        <source>Check Qgis Version</source>
        <translation>Проверить версию Qgis</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="692"/>
        <source>Check if your QGIS version is up to date (requires internet access)</source>
        <translation>Проверить, является ли ваша версия QGIS последней (требует доступ в Интернет)</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="697"/>
        <source>Refresh</source>
        <translation>Обновить</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="698"/>
        <source>Ctrl+R</source>
        <comment>Refresh Map</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="699"/>
        <source>Refresh Map</source>
        <translation>Обновить карту</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="704"/>
        <source>Zoom In</source>
        <translation>Увеличить</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="703"/>
        <source>Ctrl++</source>
        <comment>Zoom In</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="709"/>
        <source>Zoom Out</source>
        <translation>Уменьшить</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="708"/>
        <source>Ctrl+-</source>
        <comment>Zoom Out</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="712"/>
        <source>Zoom Full</source>
        <translation>Полный экстент</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="713"/>
        <source>F</source>
        <comment>Zoom to Full Extents</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="714"/>
        <source>Zoom to Full Extents</source>
        <translation>Увеличить до полного экстента</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="717"/>
        <source>Zoom To Selection</source>
        <translation>Увеличить до выделенного</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="718"/>
        <source>Ctrl+F</source>
        <comment>Zoom to selection</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="719"/>
        <source>Zoom to selection</source>
        <translation>Увеличить до выделенного</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="722"/>
        <source>Pan Map</source>
        <translation>Панорамировать карту</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="723"/>
        <source>Pan the map</source>
        <translation>Панорамировать карту</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="726"/>
        <source>Zoom Last</source>
        <translation>Предыдущий экстент</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="728"/>
        <source>Zoom to Last Extent</source>
        <translation>Увеличить до предыдущего экстента</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="731"/>
        <source>Zoom To Layer</source>
        <translation>Увеличить до слоя</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="733"/>
        <source>Zoom to Layer</source>
        <translation>Увеличить до слоя</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="736"/>
        <source>Identify Features</source>
        <translation>Определить объекты</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="737"/>
        <source>I</source>
        <comment>Click on features to identify them</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="738"/>
        <source>Click on features to identify them</source>
        <translation>Определить объекты по щелчку мыши</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="742"/>
        <source>Select Features</source>
        <translation>Выбрать объекты</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="748"/>
        <source>Open Table</source>
        <translation>Открыть таблицу</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="752"/>
        <source>Measure Line </source>
        <translation>Измерить линию </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="753"/>
        <source>Ctrl+M</source>
        <comment>Measure a Line</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="754"/>
        <source>Measure a Line</source>
        <translation>Измерить линию</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="757"/>
        <source>Measure Area</source>
        <translation>Измерить площадь</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="758"/>
        <source>Ctrl+J</source>
        <comment>Measure an Area</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="759"/>
        <source>Measure an Area</source>
        <translation>Измерить площадь</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="764"/>
        <source>Show Bookmarks</source>
        <translation>Показать закладки</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="763"/>
        <source>B</source>
        <comment>Show Bookmarks</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="779"/>
        <source>New Bookmark...</source>
        <translation>Новая закладка...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="780"/>
        <source>Ctrl+B</source>
        <comment>New Bookmark</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5440"/>
        <source>New Bookmark</source>
        <translation>Новая закладка</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="784"/>
        <source>Add WMS Layer...</source>
        <translation>Добавить WMS-слой...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="785"/>
        <source>W</source>
        <comment>Add Web Mapping Server Layer</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="786"/>
        <source>Add Web Mapping Server Layer</source>
        <translation>Добавить слой картографического веб-сервера</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="789"/>
        <source>In Overview</source>
        <translation>В обзор</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="790"/>
        <source>O</source>
        <comment>Add current layer to overview map</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="791"/>
        <source>Add current layer to overview map</source>
        <translation>Добавить текущий слой в обзорную карту</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="797"/>
        <source>Plugin Manager...</source>
        <translation>Менеджер модулей...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="799"/>
        <source>Open the plugin manager</source>
        <translation>Открыть менеджер модулей</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="817"/>
        <source>Capture Point</source>
        <translation>Захватить точку</translation>
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
        <translation>Захватить точки</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="823"/>
        <source>Capture Line</source>
        <translation>Захватить линию</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="824"/>
        <source>/</source>
        <comment>Capture Lines</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="825"/>
        <source>Capture Lines</source>
        <translation>Захватить линии</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="829"/>
        <source>Capture Polygon</source>
        <translation>Захватить полигон</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="830"/>
        <source>Ctrl+/</source>
        <comment>Capture Polygons</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="831"/>
        <source>Capture Polygons</source>
        <translation>Захватить полигоны</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="836"/>
        <source>Delete Selected</source>
        <translation>Удалить выделенное</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="851"/>
        <source>Add Vertex</source>
        <translation>Добавить вершину</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="856"/>
        <source>Delete Vertex</source>
        <translation>Удалить вершину</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="861"/>
        <source>Move Vertex</source>
        <translation>Переместить вершину</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="954"/>
        <source>&amp;File</source>
        <translation>&amp;Файл</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="957"/>
        <source>&amp;Open Recent Projects</source>
        <translation>&amp;Открыть недавние проекты</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="977"/>
        <source>&amp;View</source>
        <translation>&amp;Вид</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="996"/>
        <source>&amp;Layer</source>
        <translation>С&amp;лой</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1016"/>
        <source>&amp;Settings</source>
        <translation>&amp;Установки</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1036"/>
        <source>&amp;Help</source>
        <translation>&amp;Справка</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1055"/>
        <source>File</source>
        <translation>Файл</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1072"/>
        <source>Manage Layers</source>
        <translation>Управление слоями</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1133"/>
        <source>Help</source>
        <translation>Справка</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1082"/>
        <source>Digitizing</source>
        <translation>Оцифровка</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1102"/>
        <source>Map Navigation</source>
        <translation>Навигация</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1115"/>
        <source>Attributes</source>
        <translation>Атрибуты</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1128"/>
        <source>Plugins</source>
        <translation>Модули</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1251"/>
        <source>Ready</source>
        <translation>Готово</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1627"/>
        <source>New features</source>
        <translation>Новые возможности</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2882"/>
        <source>Unable to open project</source>
        <translation>Не удалось открыть проект</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3052"/>
        <source>Unable to save project </source>
        <translation>Не удалось сохранить проект </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2994"/>
        <source>Choose a filename to save the QGIS project file as</source>
        <translation>Выберите имя файла для сохранения проекта QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3110"/>
        <source>QGIS: Unable to load project</source>
        <translation>QGIS: не удалось загрузить проект</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3111"/>
        <source>Unable to load project </source>
        <translation>Не удалось загрузить проект </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4276"/>
        <source>QGIS - Changes in SVN Since Last Release</source>
        <translation>QGIS — Изменения в SVN со времени последнего релиза</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4268"/>
        <source>You can change this option later by selecting Options from the Settings menu (Help Browser tab).</source>
        <translation type="obsolete">Впоследствии вы сможете изменить этот параметр выбрав пункт Настройки в меню Установки (вкладка «Браузер»).</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5264"/>
        <source>Layer is not valid</source>
        <translation>Неверный слой</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5265"/>
        <source>The layer is not a valid layer and can not be added to the map</source>
        <translation>Слой не является действительным и не может быть добавлен на карту</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4533"/>
        <source>Save?</source>
        <translation>Сохранить?</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5047"/>
        <source>Clipboard contents set to: </source>
        <translation>Содержимое буфера обмена установлено в: </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5183"/>
        <source> is not a valid or recognized raster data source</source>
        <translation> не является действительным (определяемым) источником растровых данных</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5344"/>
        <source> is not a supported raster data source</source>
        <translation> не является поддерживаемым источником растровых данных</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5345"/>
        <source>Unsupported Data Source</source>
        <translation>Неподдерживаемый источник данных</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5441"/>
        <source>Enter a name for the new bookmark:</source>
        <translation>Введите имя для этой закладки:</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5458"/>
        <source>Error</source>
        <translation>Ошибка</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5458"/>
        <source>Unable to create the bookmark. Your user database may be missing or corrupted</source>
        <translation>Не удалось создать закладку. Ваша пользовательская база данных отсутствует или повреждена</translation>
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
        <translation>Показать все панели</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="775"/>
        <source>Hide most toolbars</source>
        <translation>Скрыть все панели</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="875"/>
        <source>Cut Features</source>
        <translation>Вырезать объекты</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="876"/>
        <source>Cut selected features</source>
        <translation>Вырезать выделенные объекты</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="880"/>
        <source>Copy Features</source>
        <translation>Копировать объекты</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="881"/>
        <source>Copy selected features</source>
        <translation>Копировать выделенные объекты</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="885"/>
        <source>Paste Features</source>
        <translation>Вставить объекты</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="886"/>
        <source>Paste selected features</source>
        <translation>Вставить выделенные объекты</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1541"/>
        <source>
Compiled against Qt </source>
        <translation type="obsolete">
Собрана под Qt </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1542"/>
        <source>, running against Qt </source>
        <translation type="obsolete">. Выполняется под Qt </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4308"/>
        <source>Network error while communicating with server</source>
        <translation>Ошибка сети во время соединения с сервером</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4311"/>
        <source>Unknown network socket error</source>
        <translation>Неизвестная ошибка сетевого соединения</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4316"/>
        <source>Unable to communicate with QGIS Version server</source>
        <translation>Не удалось связаться с сервером версии QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="768"/>
        <source>T</source>
        <comment>Show most toolbars</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="774"/>
        <source>Ctrl+T</source>
        <comment>Hide most toolbars</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="358"/>
        <source>Checking provider plugins</source>
        <translation>Проверка источников данных</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="362"/>
        <source>Starting Python</source>
        <translation>Запуск Python</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="396"/>
        <source>Python console</source>
        <translation>Консоль Python</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1762"/>
        <source>Python error</source>
        <translation>Ошибка Python</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1762"/>
        <source>Error when reading metadata of plugin </source>
        <translation>Ошибка чтения метаданных модуля </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3502"/>
        <source>Provider does not support deletion</source>
        <translation>Источник не поддерживает удаление</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3503"/>
        <source>Data provider does not support deleting features</source>
        <translation>Источник данных не поддерживает удаление объектов</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3509"/>
        <source>Layer not editable</source>
        <translation>Нередактируемый слой</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3510"/>
        <source>The current layer is not editable. Choose &apos;Start editing&apos; in the digitizing toolbar.</source>
        <translation>Текущий слой нередактируем. Выберите «Режим редактирования» на панели инструментов оцифровки.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="810"/>
        <source>Toggle editing</source>
        <translation>Режим редактирования</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="811"/>
        <source>Toggles the editing state of the current layer</source>
        <translation>Переключить текущий слой в режим редактирования</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="866"/>
        <source>Add Ring</source>
        <translation>Добавить кольцо</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="870"/>
        <source>Add Island</source>
        <translation>Добавить остров</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="871"/>
        <source>Add Island to multipolygon</source>
        <translation>Добавить остров к мультиполигону</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1145"/>
        <source>Toolbar Visibility...</source>
        <translation>Панели инструментов...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1175"/>
        <source>Scale </source>
        <translation>Масштаб </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1190"/>
        <source>Current map scale (formatted as x:y)</source>
        <translation>Текущий масштаб карты (в формате x:y)</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1204"/>
        <source>Map coordinates at mouse cursor position</source>
        <translation>Координаты карты в позиции курсора мыши</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3780"/>
        <source>Invalid scale</source>
        <translation>Неверный масштаб</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4534"/>
        <source>Do you want to save the current project?</source>
        <translation>Вы хотите сохранить текущий проект?</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Python bindings - This is the major focus of this release it is now possible to create plugins using python. It is also possible to create GIS enabled applications written in python that use the QGIS libraries.</source>
        <translation type="obsolete">Интерфейс к Python — ключевой особенностью данного релиза стала возможность создавать модули на языке Python. Кроме этого, вы можете создавать на Python самостоятельные ГИС-приложения, использующие библиотеки QGIS.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Removed automake build system - QGIS now needs CMake for compilation.</source>
        <translation type="obsolete">Удалена система сборки automake — для сборки QGIS теперь требуется CMake.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Many new GRASS tools added (with thanks to http://faunalia.it/)</source>
        <translation type="obsolete">Добавлено множество новых инструментов GRASS (спасибо http://faunalia.it/)</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Map Composer updates</source>
        <translation type="obsolete">Обновлён компоновщик карт</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Crash fix for 2.5D shapefiles</source>
        <translation type="obsolete">Исправлена ошибка с 2.5-мерными shape-файлами</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>The QGIS libraries have been refactored and better organised.</source>
        <translation type="obsolete">Библиотеки QGIS были переработаны и лучше организованы.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Improvements to the GeoReferencer</source>
        <translation type="obsolete">Улучшен модуль привязки растров</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1203"/>
        <source>Shows the map coordinates at the current cursor position. The display is continuously updated as the mouse is moved.</source>
        <translation>Здесь показаны координаты карты в позиции курсора. Эти значения постоянно обновляются при движении мыши.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Added locale options to options dialog.</source>
        <translation type="obsolete">В диалог настроек добавлен выбор языка.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="841"/>
        <source>Move Feature</source>
        <translation>Переместить объект</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="846"/>
        <source>Split Features</source>
        <translation>Разделить объекты</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="891"/>
        <source>Map Tips</source>
        <translation>Всплывающие описания</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="892"/>
        <source>Show information about a feature when the mouse is hovered over it</source>
        <translation>Показать информацию об объекте при перемещении над ним курсора мыши</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1176"/>
        <source>Current map scale</source>
        <translation>Текущий масштаб карты</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5493"/>
        <source>Project file is older</source>
        <translation>Устаревший файл проекта</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5495"/>
        <source>&lt;p&gt;This project file was saved by an older version of QGIS.</source>
        <translation>&lt;p&gt;Этот файл проекта был создан старой версией QGIS.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5497"/>
        <source> When saving this project file, QGIS will update it to the latest version, possibly rendering it useless for older versions of QGIS.</source>
        <translation> При сохранении, этот файл будет обновлён, что может повлечь за собой несовместимость с предыдущими версиями QGIS.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5500"/>
        <source>&lt;p&gt;Even though QGIS developers try to maintain backwards compatibility, some of the information from the old project file might be lost.</source>
        <translation>&lt;p&gt;Несмотря на то, что разработчики QGIS стремятся к максимальной обратной совместимости, часть информации из старого проекта может быть потеряна.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5502"/>
        <source> To improve the quality of QGIS, we appreciate if you file a bug report at %3.</source>
        <translation> Вы могли бы помочь нам улучшить QGIS, отправив сообщение об ошибке по адресу: %3.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5504"/>
        <source> Be sure to include the old project file, and state the version of QGIS you used to discover the error.</source>
        <translation> Пожалуйста, приложите старый файл проекта и укажите версию QGIS, в которой была обнаружена ошибка.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5506"/>
        <source>&lt;p&gt;To remove this warning when opening an older project file, uncheck the box &apos;%5&apos; in the %4 menu.</source>
        <translation>&lt;p&gt;Если вы не хотите видеть это сообщение в дальнейшем, снимите флажок «%5» в меню «%4».</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5506"/>
        <source>&lt;p&gt;Version of the project file: %1&lt;br&gt;Current version of QGIS: %2</source>
        <translation>&lt;p&gt;Версия файла проекта: %1&lt;br&gt;Текущая версия QGIS: %2</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5510"/>
        <source>&lt;tt&gt;Settings:Options:General&lt;/tt&gt;</source>
        <comment>Menu path to setting options</comment>
        <translation>&lt;tt&gt;Установки:Настройки:Общие&lt;/tt&gt;</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5511"/>
        <source>Warn me when opening a project file saved with an older version of QGIS</source>
        <translation>Предупреждать при попытке открытия файлов проекта старых версий QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="636"/>
        <source>Toggle full screen mode</source>
        <translation>Полноэкранный режим</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="637"/>
        <source>Ctrl-F</source>
        <comment>Toggle fullscreen mode</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="638"/>
        <source>Toggle fullscreen mode</source>
        <translation>Полноэкранный режим</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1561"/>
        <source>This release candidate includes over 40 bug fixes and enchancements over the QGIS 0.9.1 release. In addition we have added the following new features:</source>
        <translation type="obsolete">Эта версия включает более 40 исправлений ошибок и обновлений по сравнению с QGIS 0.9.1. Кроме того, мы добавили ряд новых возможностей:</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1633"/>
        <source>Imrovements to digitising capabilities.</source>
        <translation>Улучшены инструменты оцифровки.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1639"/>
        <source>Supporting default and defined styles (.qml) files for file based vector layers. With styles you can save the symbolisation and other settings associated with a vector layer and they will be loaded whenever you load that layer.</source>
        <translation>Поддержка сохранения пользовательских стилей и стилей по умолчанию для векторных слоёв в файлах .qml. Используя стили, можно сохранять символику и другие параметры слоя, которые будут загружены всякий раз при открытии этого слоя.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1645"/>
        <source>Improved support for transparency and contrast stretching in raster layers. Support for color ramps in raster layers. Support for non-north up rasters. Many other raster improvements &apos;under the hood&apos;.</source>
        <translation>Улучшены средства прозрачности и улучшения контраста растровых слоёв. Добавлена поддержка градиентов и поддержка растров с ориентацией, отличной от «север вверху». Множество улучшений в алгоритмах обработки растров.</translation>
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
        <translation type="unfinished">Легенда</translation>
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
        <translation type="unfinished">Эта версия включает более 40 исправлений ошибок и обновлений по сравнению с QGIS 0.9.1. Кроме того, мы добавили ряд новых возможностей: {120 ?} {0.9.1 ?}</translation>
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
        <location filename="" line="0"/>
        <source>Quantum GIS</source>
        <translation type="obsolete">Quantum GIS</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>File</source>
        <translation type="obsolete">Файл</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>View</source>
        <translation type="obsolete">Вид</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Tools</source>
        <translation type="obsolete">&amp;Инструменты</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Help</source>
        <translation type="obsolete">&amp;Помощь</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>File Management Toolbar</source>
        <translation type="obsolete">Инструментальная Панель Управления Файлами</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Data Toolbar</source>
        <translation type="obsolete">Инструментальная Панель Данных</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Map Navigation Toolbar</source>
        <translation type="obsolete">Инструментальная Панель Навигации Картой</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Attribute Data Toolbar</source>
        <translation type="obsolete">Инструментальная Панель Атрибутов Данных</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Open Project</source>
        <translation type="obsolete">Открыть Проект</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Exit</source>
        <translation type="obsolete">Выйти</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>E&amp;xit</source>
        <translation type="obsolete">В&amp;ыйти</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Add a PostGIS Layer to the map</source>
        <translation type="obsolete">Добавить PostGIS слой к данной карте</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Refresh </source>
        <translation type="obsolete">Обновить</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Zoom to full extent</source>
        <translation type="obsolete">Изменить вид до полной</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Map Navigation Tools</source>
        <translation type="obsolete">Инструменты Навигации Картой</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Zoom In</source>
        <translation type="obsolete">Увеличить</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Zoom &amp;In</source>
        <translation type="obsolete">&amp;Увеличить</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Pan</source>
        <translation type="obsolete">Перевезти</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Pan</source>
        <translation type="obsolete">&amp;Перевезти</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Zoom out</source>
        <translation type="obsolete">Уменьшить</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Zoom &amp;out</source>
        <translation type="obsolete">&amp;Уменьшить</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Identify</source>
        <translation type="obsolete">Определить</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Identify a feature on the active layer</source>
        <translation type="obsolete">Определить пункт на данном слое</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>select features</source>
        <translation type="obsolete">Выделить пункты</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;About Quantum GIS</source>
        <translation type="obsolete">&amp;О Quantum GIS</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Test button</source>
        <translation type="obsolete">Тест кнопка</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Add a vector layer</source>
        <translation type="obsolete">Добавить векторный слой</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Add a vector layer (e.g. Shapefile)</source>
        <translation type="obsolete">Добавить векторный слой (на пример: Shapefile)</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Attribute table</source>
        <translation type="obsolete">Таблица атрибутов</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Open the attribute table for the selected layer</source>
        <translation type="obsolete">Открыть таблицу атрибутов для выделенного слоя</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Action</source>
        <translation type="obsolete">Действие</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Zoom to last extent</source>
        <translation type="obsolete">Изменить вид до предидущего</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Test plugin functions</source>
        <translation type="obsolete">Тестировать функции компонента (плагина)</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Options</source>
        <translation type="obsolete">Настройка</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>QGis options</source>
        <translation type="obsolete">Настройка QGIS</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Save Project</source>
        <translation type="obsolete">Сохранить проект</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Save Project As...</source>
        <translation type="obsolete">Сохранить Проект Как ...</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Plugin Manager</source>
        <translation type="obsolete">Администратор компонентов (плагинов)</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Check QGIS Version</source>
        <translation type="obsolete">Проверьте Версию QGIS</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Properties</source>
        <translation type="obsolete">&amp;Свойства</translation>
    </message>
    <message>
        <location filename="../src/ui/qgisappbase.ui" line="13"/>
        <source>MainWindow</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgisappbase.ui" line="102"/>
        <source>Legend</source>
        <translation>Легенда</translation>
    </message>
    <message>
        <location filename="../src/ui/qgisappbase.ui" line="135"/>
        <source>Map View</source>
        <translation>Вид карты</translation>
    </message>
</context>
<context>
    <name>QgsAbout</name>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="13"/>
        <source>About Quantum GIS</source>
        <translation>О Quantum GIS</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="230"/>
        <source>Ok</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="26"/>
        <source>About</source>
        <translation>О программе</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="65"/>
        <source>Version</source>
        <translation>Версия</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Quantum GIS (qgis)</source>
        <translation type="obsolete">Quantum GIS (qgis)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="152"/>
        <source>What&apos;s New</source>
        <translation>Что нового</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>License</source>
        <translation type="obsolete">Лицензия</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="104"/>
        <source>http://www.gnu.org/licenses</source>
        <translation type="unfinished">http://www.gnu.org/licenses</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="91"/>
        <source>Quantum GIS is licensed under the GNU General Public License</source>
        <translation>Quantum GIS выпускается под Стандартной Общественной Лицензией GNU</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Contributors</source>
        <translation type="obsolete">Основные участники</translation>
    </message>
    <message>
        <location filename="" line="0"/>
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
        <location filename="" line="0"/>
        <source>Plugins</source>
        <translation type="obsolete">Компоненты (Плагины)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="76"/>
        <source>&lt;h2&gt;Quantum GIS (qgis)&lt;/h2&gt;</source>
        <translation type="obsolete">&lt;h2&gt;Quantum GIS (qgis)&lt;/h2&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="129"/>
        <source>QGIS Home Page</source>
        <translation>Веб-сайт QGIS</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="117"/>
        <source>Subscribe to the QGIS-User mailing list</source>
        <translation type="obsolete">Подписатьcя на рассылку QGIS-User</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="189"/>
        <source>Providers</source>
        <translation>Источники</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="175"/>
        <source>Developers</source>
        <translation>Разработчики</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="225"/>
        <source>&lt;h2&gt;QGIS Developers&lt;/h2&gt;</source>
        <translation type="obsolete">&lt;h2&gt;Разработчики QGIS&lt;/h2&gt;</translation>
    </message>
    <message>
        <location filename="../src/app/qgsabout.cpp" line="114"/>
        <source>Name</source>
        <translation>Имя</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="199"/>
        <source>Sponsors</source>
        <translation>Спонсоры</translation>
    </message>
    <message>
        <location filename="../src/app/qgsabout.cpp" line="118"/>
        <source>QGIS Sponsors</source>
        <translation type="obsolete">Спонсоры QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgsabout.cpp" line="115"/>
        <source>Website</source>
        <translation>Веб-сайт</translation>
    </message>
    <message>
        <location filename="../src/app/qgsabout.cpp" line="255"/>
        <source>QGIS Browser Selection</source>
        <translation type="obsolete">Выбор браузера QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgsabout.cpp" line="121"/>
        <source>The following have sponsored QGIS by contributing money to fund development and other project costs</source>
        <translation type="obsolete">Эти люди спонсировали QGIS, вкладывая деньги в разработку и покрытие иных расходов проекта</translation>
    </message>
    <message>
        <location filename="../src/app/qgsabout.cpp" line="259"/>
        <source>Enter the name of a web browser to use (eg. konqueror).
Enter the full path if the browser is not in your PATH.
You can change this option later by selection Options from the Settings menu (Help Browser tab).</source>
        <translation type="obsolete">Введите имя используемого веб-браузера (напр. konqueror).
Если браузер отсутствует в пути поиска PATH, укажите его полный путь.
Этот параметр можно изменить впоследствии, выбрав «Настройки» в меню «Установки» (на вкладке «Браузер»).</translation>
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
        <translation>Добавить атрибут</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsaddattrdialogbase.ui" line="22"/>
        <source>Name:</source>
        <translation>Имя:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsaddattrdialogbase.ui" line="35"/>
        <source>Type:</source>
        <translation>Тип:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsaddattrdialogbase.ui" line="52"/>
        <source>OK</source>
        <translation type="obsolete">OK</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsaddattrdialogbase.ui" line="59"/>
        <source>Cancel</source>
        <translation type="obsolete">Отменить</translation>
    </message>
</context>
<context>
    <name>QgsAttributeActionDialog</name>
    <message>
        <location filename="../src/app/qgsattributeactiondialog.cpp" line="57"/>
        <source>Name</source>
        <translation type="obsolete">Имя</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributeactiondialog.cpp" line="58"/>
        <source>Action</source>
        <translation type="obsolete">Действие</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributeactiondialog.cpp" line="59"/>
        <source>Capture</source>
        <translation type="obsolete">Захватывать</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributeactiondialog.cpp" line="150"/>
        <source>Select an action</source>
        <comment>File dialog window title</comment>
        <translation>Выберите действие</translation>
    </message>
</context>
<context>
    <name>QgsAttributeActionDialogBase</name>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="157"/>
        <source>Name</source>
        <translation type="unfinished">Имя</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="162"/>
        <source>Action</source>
        <translation type="unfinished">Действие</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="144"/>
        <source>This list contains all actions that have been defined for the current layer. Add actions by entering the details in the controls below and then pressing the Insert action button. Actions can be edited here by double clicking on the item.</source>
        <translation>Этот список содержит действия, определённые для текущего слоя. Чтобы добавить новое действие, заполните соответствующие поля и нажмите «Вставить действие». Чтобы изменить действие, дважды щёлкните на нём в этом списке.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="214"/>
        <source>Move up</source>
        <translation>Передвинуть вверх</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="211"/>
        <source>Move the selected action up</source>
        <translation>Переместить выбранное действие выше</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="224"/>
        <source>Move down</source>
        <translation>Передвинуть вниз</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="221"/>
        <source>Move the selected action down</source>
        <translation>Переместить выбранное действие ниже</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="250"/>
        <source>Remove</source>
        <translation>Удалить</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="247"/>
        <source>Remove the selected action</source>
        <translation>Удалить выбранное действие</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="231"/>
        <source>Name:</source>
        <translation type="obsolete">Имя:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="59"/>
        <source>Enter the name of an action here. The name should be unique (qgis will make it unique if necessary).</source>
        <translation>Поле ввода имени действия. Имя должно быть уникальным (qgis сделает его уникальным при необходимости).</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="56"/>
        <source>Enter the action name here</source>
        <translation>Введите имя действия</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="208"/>
        <source>Action:</source>
        <translation type="obsolete">Действие:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="79"/>
        <source>Enter the action command here</source>
        <translation>Введите команду действия</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="129"/>
        <source>Browse</source>
        <translation type="obsolete">Обзор</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="126"/>
        <source>Browse for action commands</source>
        <translation type="obsolete">Поиск команд действия</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="178"/>
        <source>Insert action</source>
        <translation>Вставить действие</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="175"/>
        <source>Inserts the action into the list above</source>
        <translation>Вставить действие в список</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="188"/>
        <source>Update action</source>
        <translation>Обновить действие</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="185"/>
        <source>Update the selected action</source>
        <translation>Обновить выбранное действие</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="115"/>
        <source>Insert field</source>
        <translation>Вставить поле</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="112"/>
        <source>Inserts the selected field into the action, prepended with a %</source>
        <translation>Вставить в действие выбранное поле с предшествующим %</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="105"/>
        <source>The valid attribute names for this layer</source>
        <translation>Действительные имена атрибутов для этого слоя</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="128"/>
        <source>Capture output</source>
        <translation>Захватывать вывод</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="122"/>
        <source>Captures any output from the action</source>
        <translation>Захватывать вывод команды действия</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="125"/>
        <source>Captures the standard output or error generated by the action and displays it in a dialog box</source>
        <translation>Захватывать вывод или ошибки действия и выводить их в диалоговом окне</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="82"/>
        <source>Enter the action here. This can be any program, script or command that is available on your system. When the action is invoked any set of characters that start with a % and then have the name of a field will be replaced by the value of that field. The special characters %% will be replaced by the value of the field that was selected. Double quote marks group text into single arguments to the program, script or command. Double quotes will be ignored if preceeded by a backslash</source>
        <translation>Данное поле предназначено для ввода действия. Действием может быть любая программа, сценарий или команда, доступная в вашей системе. Когда действие выполняется, любые последовательности, начинающиеся со знака % и следующим за ним именем поля, будут заменены на значение этого поля. Специальные символы %% будут заменены на значение выбранного поля. Двойные кавычки позволяют группировать текст в единый аргумент программы, сценария или команды. Двойные кавычки, перед которыми следует \, будут проигнорированы</translation>
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
        <translation type="unfinished">Захватывать</translation>
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
        <translation>Введите значения атрибутов</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributedialogbase.ui" line="37"/>
        <source>Attribute</source>
        <translation type="obsolete">Атрибут</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributedialogbase.ui" line="42"/>
        <source>Value</source>
        <translation type="obsolete">Значение</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributedialogbase.ui" line="50"/>
        <source>&amp;OK</source>
        <translation type="obsolete">&amp;OK</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributedialogbase.ui" line="57"/>
        <source>&amp;Cancel</source>
        <translation type="obsolete">О&amp;тменить</translation>
    </message>
</context>
<context>
    <name>QgsAttributeTable</name>
    <message>
        <location filename="../src/app/qgsattributetable.cpp" line="340"/>
        <source>Run action</source>
        <translation>Выполнить действие</translation>
    </message>
</context>
<context>
    <name>QgsAttributeTableBase</name>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="13"/>
        <source>Attribute Table</source>
        <translation>Таблица атрибутов</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="167"/>
        <source>Start editing</source>
        <translation>Начать редактирование</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="279"/>
        <source>&amp;Close</source>
        <translation type="obsolete">&amp;Закрыть</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="144"/>
        <source>Ctrl+X</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="128"/>
        <source>Ctrl+N</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="71"/>
        <source>Ctrl+S</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="62"/>
        <source>Invert selection</source>
        <translation>Обратить выделение</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="55"/>
        <source>Ctrl+T</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="46"/>
        <source>Move selected to top</source>
        <translation>Переместить выделенное в начало</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="30"/>
        <source>Remove selection</source>
        <translation>Удалить выделение</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="78"/>
        <source>Copy selected rows to clipboard (Ctrl+C)</source>
        <translation>Копировать выбранные строки в буфер обмена (Ctrl+C)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="81"/>
        <source>Copies the selected rows to the clipboard</source>
        <translation>Копирует выбранные строки в буфер обмена</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="90"/>
        <source>Ctrl+C</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="174"/>
        <source>Stop editin&amp;g</source>
        <translation>Прекратить &amp;редактирование</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="177"/>
        <source>Alt+G</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="224"/>
        <source>Search for:</source>
        <translation type="obsolete">Искать:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="211"/>
        <source>in</source>
        <translation>в</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="231"/>
        <source>Search</source>
        <translation>Поиск</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="241"/>
        <source>Adva&amp;nced...</source>
        <translation>&amp;Дополнительно...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="244"/>
        <source>Alt+N</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="214"/>
        <source>&amp;Help</source>
        <translation type="obsolete">&amp;Справка</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="119"/>
        <source>New column</source>
        <translation>Новое поле</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="135"/>
        <source>Delete column</source>
        <translation>Удалить поле</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="97"/>
        <source>Zoom map to the selected rows (Ctrl-F)</source>
        <translation>Увеличить карту до выбранных строк (Ctrl+F)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="100"/>
        <source>Zoom map to the selected rows</source>
        <translation>Увеличить карту до выбранных строк</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="106"/>
        <source>Ctrl+F</source>
        <translation></translation>
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
        <translation>выбрать</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="99"/>
        <source>select and bring to top</source>
        <translation>выбрать и переместить в начало</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="100"/>
        <source>show only matching</source>
        <translation>только показать соответствия</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="330"/>
        <source>Search string parsing error</source>
        <translation>Ошибка разбора поискового запроса</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="382"/>
        <source>Search results</source>
        <translation>Результаты поиска</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="336"/>
        <source>You&apos;ve supplied an empty search string.</source>
        <translation>Вы ввели пустой поисковый запрос.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="370"/>
        <source>Error during search</source>
        <translation>Ошибка во время поиска</translation>
    </message>
    <message numerus="yes">
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="379"/>
        <source>Found %d matching features.</source>
        <translation>
            <numerusform>Найден %d подходящий объект.</numerusform>
            <numerusform>Найдено %d подходящих объекта.</numerusform>
            <numerusform>Найдено %d подходящих объектов.</numerusform>
        </translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="381"/>
        <source>No matching features found.</source>
        <translation>Подходящих объектов не найдено.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="148"/>
        <source>Name conflict</source>
        <translation>Конфликт имён</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="197"/>
        <source>Stop editing</source>
        <translation>Прекратить редактирование</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="198"/>
        <source>Do you want to save the changes?</source>
        <translation>Вы хотите сохранить изменения?</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="204"/>
        <source>Error</source>
        <translation>Ошибка</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="200"/>
        <source>Could not commit changes</source>
        <translation type="obsolete">Не удалось внести изменения</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="148"/>
        <source>The attribute could not be inserted. The name already exists in the table.</source>
        <translation>Не удалось вставить атрибут. Данное имя уже существует в таблице.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="204"/>
        <source>Could not commit changes - changes are still pending</source>
        <translation>Не удалось внести изменения — поставлено в очередь</translation>
    </message>
</context>
<context>
    <name>QgsBookmarks</name>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="160"/>
        <source>Really Delete?</source>
        <translation>Действительно удалить?</translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="161"/>
        <source>Are you sure you want to delete the </source>
        <translation>Вы уверены, что хотите удалить закладку </translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="161"/>
        <source> bookmark?</source>
        <translation>?</translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="177"/>
        <source>Error deleting bookmark</source>
        <translation>Ошибка удаления закладки</translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="179"/>
        <source>Failed to delete the </source>
        <translation>Не удалось удалить закладку </translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="181"/>
        <source> bookmark from the database. The database said:
</source>
        <translation> из базы данных. Сообщение базы данных:
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
        <translation>Пространственные закладки</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="29"/>
        <source>Name</source>
        <translation>Имя</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="34"/>
        <source>Project</source>
        <translation>Проект</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="39"/>
        <source>Extent</source>
        <translation>Экстент</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="44"/>
        <source>Id</source>
        <translation>ID</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="98"/>
        <source>Zoom To</source>
        <translation type="obsolete">Увеличить до</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="95"/>
        <source>Zoom to the currently selected bookmark</source>
        <translation type="obsolete">Увеличить до выбранной закладки</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="88"/>
        <source>Delete</source>
        <translation type="obsolete">Удалить</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="85"/>
        <source>Delete the currently selected bookmark</source>
        <translation type="obsolete">Удалить выбранную закладку</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="78"/>
        <source>Close</source>
        <translation type="obsolete">Закрыть</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="75"/>
        <source>Close the dialog</source>
        <translation type="obsolete">Закрыть диалог</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="68"/>
        <source>Help</source>
        <translation type="obsolete">Справка</translation>
    </message>
</context>
<context>
    <name>QgsComposer</name>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="472"/>
        <source> for read/write</source>
        <translation> для чтения/записи</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="688"/>
        <source>Choose a filename to save the map image as</source>
        <translation>Выберите имя файла для сохранения снимка карты</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="783"/>
        <source>Choose a filename to save the map as</source>
        <translation>Выберите имя файла для сохранения карты</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="551"/>
        <source>Error in Print</source>
        <translation>Ошибка печати</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="529"/>
        <source>Cannot seek</source>
        <translation>Ошибка позиционирования</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="454"/>
        <source>Cannot overwrite BoundingBox</source>
        <translation>Не удаётся перезаписать границы</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="462"/>
        <source>Cannot find BoundingBox</source>
        <translation>Не удаётся найти границы</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="545"/>
        <source>Cannot overwrite translate</source>
        <translation>Не удаётся перезаписать преобразование</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="551"/>
        <source>Cannot find translate</source>
        <translation>Не удаётся найти преобразование</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="559"/>
        <source>File IO Error</source>
        <translation>Ошибка чтения/записи файла</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="569"/>
        <source>Paper does not match</source>
        <translation>Несовпадение размера бумаги</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="570"/>
        <source>The selected paper size does not match the composition size</source>
        <translation>Выбранный размер бумаги не совпадает с размером композиции</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="634"/>
        <source>Big image</source>
        <translation>Большое изображение</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="635"/>
        <source>To create image </source>
        <translation>Для создания изображения </translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="638"/>
        <source> requires circa </source>
        <translation> требуется приблизительно </translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="638"/>
        <source> MB of memory</source>
        <translation> МБ памяти</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="56"/>
        <source>QGIS - print composer</source>
        <translation>QGIS — компоновка карты</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="81"/>
        <source>Map 1</source>
        <translation>Карта 1</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="472"/>
        <source>Couldn&apos;t open </source>
        <translation>Не удалось открыть </translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="665"/>
        <source>format</source>
        <translation>формат</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="757"/>
        <source>SVG warning</source>
        <translation>Предупреждение SVG</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="758"/>
        <source>Don&apos;t show this message again</source>
        <translation>Не показывать это сообщение в дальнейшем</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="802"/>
        <source>&lt;p&gt;The SVG export function in Qgis has several problems due to bugs and deficiencies in the Qt4 svg code. Of note, text does not appear in the SVG file and there are problems with the map bounding box clipping other items such as the legend or scale bar.&lt;/p&gt;If you require a vector-based output file from Qgis it is suggested that you try printing to PostScript if the SVG output is not satisfactory.&lt;/p&gt;</source>
        <translation type="obsolete">&lt;p&gt;Функция SVG-экспорта в QGIS содержит ошибки из-за проблем в svg-коде Qt4. К примеру, в SVG-файлах не отображается текст и замечены проблемы с рамкой карты, отсекающей другие элементы, такие как масштабная линейка и легенда.&lt;/p&gt;Если вам необходим вывод в векторном формате и SVG-вывод оказывается неудовлетворительным, попробуйте печать в файл PostScript.&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="784"/>
        <source>SVG Format</source>
        <translation>Формат SVG</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="764"/>
        <source>&lt;p&gt;The SVG export function in Qgis has several problems due to bugs and deficiencies in the </source>
        <translation>&lt;p&gt;Функция SVG-экспорта в QGIS может работать неправильно из-за ошибок в </translation>
    </message>
</context>
<context>
    <name>QgsComposerBase</name>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="62"/>
        <source>General</source>
        <translation>Общие</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="107"/>
        <source>Composition</source>
        <translation>Композиция</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="115"/>
        <source>Item</source>
        <translation>Элемент</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="208"/>
        <source>&amp;Open Template ...</source>
        <translation>&amp;Открыть шаблон...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="216"/>
        <source>Save Template &amp;As...</source>
        <translation>Сохранить шаблон &amp;как...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="224"/>
        <source>&amp;Print...</source>
        <translation>&amp;Печать...</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Zoom to full extent</source>
        <translation type="obsolete">Изменить вид до полной</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Zoom out</source>
        <translation type="obsolete">Уменьшить</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="256"/>
        <source>Add new map</source>
        <translation>Добавить карту</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="264"/>
        <source>Add new label</source>
        <translation>Добавить текст</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="272"/>
        <source>Add new vect legend</source>
        <translation>Добавить легенду</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="280"/>
        <source>Select/Move item</source>
        <translation>Выбрать/переместить элемент</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="288"/>
        <source>Export as image</source>
        <translation>Экспорт в изображение</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="296"/>
        <source>Export as SVG</source>
        <translation>Экспорт в SVG</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="304"/>
        <source>Add new scalebar</source>
        <translation>Добавить масштабную линейку</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="312"/>
        <source>Refresh view</source>
        <translation>Обновить</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="13"/>
        <source>MainWindow</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="232"/>
        <source>Zoom All</source>
        <translation>Показать всё</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="240"/>
        <source>Zoom In</source>
        <translation>Увеличить</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="248"/>
        <source>Zoom Out</source>
        <translation>Уменьшить</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="320"/>
        <source>Add Image</source>
        <translation>Добавить изображение</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="170"/>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="150"/>
        <source>Help</source>
        <translation>Справка</translation>
    </message>
</context>
<context>
    <name>QgsComposerLabelBase</name>
    <message>
        <location filename="../src/ui/qgscomposerlabelbase.ui" line="21"/>
        <source>Label Options</source>
        <translation>Параметры текста</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlabelbase.ui" line="48"/>
        <source>Font</source>
        <translation>Шрифт</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlabelbase.ui" line="55"/>
        <source>Box</source>
        <translation>Рамка</translation>
    </message>
</context>
<context>
    <name>QgsComposerMap</name>
    <message>
        <location filename="../src/app/composer/qgscomposermap.cpp" line="77"/>
        <source>Map %1</source>
        <translation>Карта %1</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposermap.cpp" line="99"/>
        <source>Extent (calculate scale)</source>
        <translation>Границы (авт. масштаб)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposermap.cpp" line="100"/>
        <source>Scale (calculate extent)</source>
        <translation>Масштаб (авт. границы)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposermap.cpp" line="107"/>
        <source>Cache</source>
        <translation>Кэш</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposermap.cpp" line="108"/>
        <source>Render</source>
        <translation>Отрисовка</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposermap.cpp" line="109"/>
        <source>Rectangle</source>
        <translation>Прямоугольник</translation>
    </message>
</context>
<context>
    <name>QgsComposerMapBase</name>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="21"/>
        <source>Map options</source>
        <translation>Параметры карты</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="173"/>
        <source>&lt;b&gt;Map&lt;/b&gt;</source>
        <translation>&lt;b&gt;Карта&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="147"/>
        <source>Set</source>
        <translation>Задать</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="196"/>
        <source>Width</source>
        <translation>Ширина</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="180"/>
        <source>Height</source>
        <translation>Высота</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="72"/>
        <source>Set Extent</source>
        <translation>Задать границы</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="69"/>
        <source>Set map extent to current extent in QGIS map canvas</source>
        <translation>Задать границы карты из текущих границ области карты QGIS</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="212"/>
        <source>Line width scale</source>
        <translation>Масштаб ширины контуров</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="116"/>
        <source>Width of one unit in millimeters</source>
        <translation>Ширина одной единицы в миллиметрах</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="225"/>
        <source>Symbol scale</source>
        <translation>Масштаб значков</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="238"/>
        <source>Font size scale</source>
        <translation>Масштаб шрифтов</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="251"/>
        <source>Frame</source>
        <translation>Рамка</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="258"/>
        <source>Preview</source>
        <translation>Предпросмотр</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="79"/>
        <source>1:</source>
        <translation>1:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapbase.ui" line="97"/>
        <source>Scale:</source>
        <translation>Масштаб:</translation>
    </message>
</context>
<context>
    <name>QgsComposerPicture</name>
    <message>
        <location filename="../src/app/composer/qgscomposerpicture.cpp" line="399"/>
        <source>Warning</source>
        <translation>Внимание</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposerpicture.cpp" line="400"/>
        <source>Cannot load picture.</source>
        <translation>Не удалось загрузить изображение.</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposerpicture.cpp" line="483"/>
        <source>Choose a file</source>
        <translation>Выберите файл</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposerpicture.cpp" line="466"/>
        <source>Pictures (</source>
        <translation>Изображения (</translation>
    </message>
</context>
<context>
    <name>QgsComposerPictureBase</name>
    <message>
        <location filename="../src/ui/qgscomposerpicturebase.ui" line="21"/>
        <source>Picture Options</source>
        <translation>Параметры изображения</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerpicturebase.ui" line="197"/>
        <source>Frame</source>
        <translation>Рамка</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerpicturebase.ui" line="161"/>
        <source>Angle</source>
        <translation>Угол</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerpicturebase.ui" line="119"/>
        <source>Width</source>
        <translation>Ширина</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerpicturebase.ui" line="140"/>
        <source>Height</source>
        <translation>Высота</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerpicturebase.ui" line="58"/>
        <source>Browse</source>
        <translation>Обзор</translation>
    </message>
</context>
<context>
    <name>QgsComposerScalebarBase</name>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="21"/>
        <source>Barscale Options</source>
        <translation>Параметры масштабной линейки</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="140"/>
        <source>Segment size</source>
        <translation>Размер сегмента</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="172"/>
        <source>Number of segments</source>
        <translation>Число сегментов</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="159"/>
        <source>Map units per scalebar unit</source>
        <translation>Единиц карты в делении</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="88"/>
        <source>Unit label</source>
        <translation>Обозначение единиц</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="127"/>
        <source>Map</source>
        <translation>Карта</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="195"/>
        <source>Font</source>
        <translation>Шрифт</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarbase.ui" line="41"/>
        <source>Line width</source>
        <translation>Ширина линии</translation>
    </message>
</context>
<context>
    <name>QgsComposerVectorLegend</name>
    <message>
        <location filename="../src/app/composer/qgscomposervectorlegend.cpp" line="117"/>
        <source>Layers</source>
        <translation type="obsolete">Слои</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposervectorlegend.cpp" line="118"/>
        <source>Group</source>
        <translation type="obsolete">Группа</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposervectorlegend.cpp" line="718"/>
        <source>Combine selected layers</source>
        <translation>Объединить выбранные слои</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposervectorlegend.cpp" line="134"/>
        <source>Cache</source>
        <translation>Кэш</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposervectorlegend.cpp" line="135"/>
        <source>Render</source>
        <translation>Отрисовка</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposervectorlegend.cpp" line="136"/>
        <source>Rectangle</source>
        <translation>Прямоугольник</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposervectorlegend.cpp" line="110"/>
        <source>Legend</source>
        <translation>Легенда</translation>
    </message>
</context>
<context>
    <name>QgsComposerVectorLegendBase</name>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="21"/>
        <source>Vector Legend Options</source>
        <translation>Параметры векторной легенды</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="113"/>
        <source>Title</source>
        <translation>Заглавие</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="92"/>
        <source>Map</source>
        <translation>Карта</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="182"/>
        <source>Font</source>
        <translation>Шрифт</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="167"/>
        <source>Box</source>
        <translation>Рамка</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="140"/>
        <source>Column 1</source>
        <translation type="obsolete">Столбец 1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="53"/>
        <source>Preview</source>
        <translation>Предпросмотр</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="149"/>
        <source>Layers</source>
        <translation type="unfinished">Слои</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="154"/>
        <source>Group</source>
        <translation type="unfinished">Группа</translation>
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
        <translation>Пользовательский</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="89"/>
        <source>A5 (148x210 mm)</source>
        <translation>A5 (148x210 мм)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="90"/>
        <source>A4 (210x297 mm)</source>
        <translation>A4 (210x297 мм)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="91"/>
        <source>A3 (297x420 mm)</source>
        <translation>A3 (297x420 мм)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="92"/>
        <source>A2 (420x594 mm)</source>
        <translation>A2 (420x594 мм)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="93"/>
        <source>A1 (594x841 mm)</source>
        <translation>A1 (594x841 мм)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="94"/>
        <source>A0 (841x1189 mm)</source>
        <translation>A0 (841x1189 мм)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="95"/>
        <source>B5 (176 x 250 mm)</source>
        <translation>B5 (176 x 250 мм)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="96"/>
        <source>B4 (250 x 353 mm)</source>
        <translation>B4 (250 x 353 мм)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="97"/>
        <source>B3 (353 x 500 mm)</source>
        <translation>B3 (353 x 500 мм)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="98"/>
        <source>B2 (500 x 707 mm)</source>
        <translation>B2 (500 x 707 мм)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="99"/>
        <source>B1 (707 x 1000 mm)</source>
        <translation>B1 (707 x 1000 мм)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="100"/>
        <source>B0 (1000 x 1414 mm)</source>
        <translation>B0 (1000 x 1414 мм)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="101"/>
        <source>Letter (8.5x11 inches)</source>
        <translation>Letter (8.5x11 дюймов)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="102"/>
        <source>Legal (8.5x14 inches)</source>
        <translation>Legal (8.5x14 дюймов)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="115"/>
        <source>Portrait</source>
        <translation>Портрет</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="116"/>
        <source>Landscape</source>
        <translation>Альбом</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="632"/>
        <source>Out of memory</source>
        <translation>Не хватает памяти</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="635"/>
        <source>Qgis is unable to resize the paper size due to insufficient memory.
 It is best that you avoid using the map composer until you restart qgis.
</source>
        <translation>Не хватает памяти для изменения размера бумаги.
 Во избежание ошибок, не используйте компоновщик карт до перезапуска QGIS.
</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="779"/>
        <source>Label</source>
        <translation>Метка</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="829"/>
        <source>Warning</source>
        <translation>Внимание</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposition.cpp" line="830"/>
        <source>Cannot load picture.</source>
        <translation>Не удалось загрузить изображение.</translation>
    </message>
</context>
<context>
    <name>QgsCompositionBase</name>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="21"/>
        <source>Composition</source>
        <translation>Композиция</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="33"/>
        <source>Paper</source>
        <translation>Бумага</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="176"/>
        <source>Size</source>
        <translation>Размер</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="158"/>
        <source>Units</source>
        <translation>Единицы</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="140"/>
        <source>Width</source>
        <translation>Ширина</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="122"/>
        <source>Height</source>
        <translation>Высота</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="104"/>
        <source>Orientation</source>
        <translation>Ориентация</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="213"/>
        <source>Resolution (dpi)</source>
        <translation>Разрешение (dpi)</translation>
    </message>
</context>
<context>
    <name>QgsConnectionDialog</name>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialog.cpp" line="74"/>
        <source>Test connection</source>
        <translation type="obsolete">Проверка соединения</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialog.cpp" line="72"/>
        <source>Connection to </source>
        <translation type="obsolete">Соединение с </translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialog.cpp" line="72"/>
        <source> was successfull</source>
        <translation type="obsolete"> прошло успешно</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialog.cpp" line="74"/>
        <source>Connection failed - Check settings and try again </source>
        <translation type="obsolete">Не удалось соединиться — проверьте параметры и попробуйте ещё раз </translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialog.cpp" line="96"/>
        <source>General Interface Help:

</source>
        <translation type="obsolete">Общая справка по интерфейсу:

</translation>
    </message>
</context>
<context>
    <name>QgsConnectionDialogBase</name>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="31"/>
        <source>Connection Information</source>
        <translation type="obsolete">Информация о соединении</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="90"/>
        <source>Host</source>
        <translation type="obsolete">Узел</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="97"/>
        <source>Database</source>
        <translation type="obsolete">База данных</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="111"/>
        <source>Username</source>
        <translation type="obsolete">Имя пользователя</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="83"/>
        <source>Name</source>
        <translation type="obsolete">Имя</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="135"/>
        <source>Name of the new connection</source>
        <translation type="obsolete">Имя нового соединения</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="118"/>
        <source>Password</source>
        <translation type="obsolete">Пароль</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="58"/>
        <source>Test Connect</source>
        <translation type="obsolete">Проверить соединение</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="51"/>
        <source>Save Password</source>
        <translation type="obsolete">Сохранить пароль</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="13"/>
        <source>Create a New PostGIS connection</source>
        <translation type="obsolete">Создать новое PostGIS-соединение</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsconnectiondialogbase.ui" line="104"/>
        <source>Port</source>
        <translation type="obsolete">Порт</translation>
    </message>
</context>
<context>
    <name>QgsContinuousColorDialogBase</name>
    <message>
        <location filename="../src/ui/qgscontinuouscolordialogbase.ui" line="13"/>
        <source>Continuous color</source>
        <translation>Непрерывный цвет</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscontinuouscolordialogbase.ui" line="54"/>
        <source>Maximum Value:</source>
        <translation>Максимальное значение:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscontinuouscolordialogbase.ui" line="80"/>
        <source>Outline Width:</source>
        <translation>Ширина контура:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscontinuouscolordialogbase.ui" line="93"/>
        <source>Minimum Value:</source>
        <translation>Минимальное значение:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscontinuouscolordialogbase.ui" line="109"/>
        <source>Classification Field:</source>
        <translation>Поле классификации:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscontinuouscolordialogbase.ui" line="119"/>
        <source>Draw polygon outline</source>
        <translation>Рисовать контуры полигонов</translation>
    </message>
</context>
<context>
    <name>QgsCoordinateTransform</name>
    <message>
        <location filename="../src/core/qgscoordinatetransform.cpp" line="483"/>
        <source>Failed</source>
        <translation>Не удалось выполнить</translation>
    </message>
    <message>
        <location filename="../src/core/qgscoordinatetransform.cpp" line="483"/>
        <source>transform of</source>
        <translation>преобразование</translation>
    </message>
    <message>
        <location filename="../src/core/qgscoordinatetransform.cpp" line="496"/>
        <source>with error: </source>
        <translation>по причине: </translation>
    </message>
    <message>
        <location filename="../src/core/qgscoordinatetransform.cpp" line="418"/>
        <source>The source spatial reference system (SRS) is not valid. </source>
        <translation>Исходная система координат (SRS) неверна. </translation>
    </message>
    <message>
        <location filename="../src/core/qgscoordinatetransform.cpp" line="426"/>
        <source>The coordinates can not be reprojected. The SRS is: </source>
        <translation>Не удалось спроецировать координаты. SRS: </translation>
    </message>
    <message>
        <location filename="../src/core/qgscoordinatetransform.cpp" line="425"/>
        <source>The destination spatial reference system (SRS) is not valid. </source>
        <translation>Целевая система координат (SRS) неверна. </translation>
    </message>
</context>
<context>
    <name>QgsCopyrightLabelPlugin</name>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="66"/>
        <source>Bottom Left</source>
        <translation>Внизу слева</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="67"/>
        <source>Top Left</source>
        <translation>Вверху слева</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="67"/>
        <source>Top Right</source>
        <translation>Вверху справа</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="67"/>
        <source>Bottom Right</source>
        <translation>Внизу справа</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="79"/>
        <source>&amp;Copyright Label</source>
        <translation>&amp;Метка авторского права</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="80"/>
        <source>Creates a copyright label that is displayed on the map canvas.</source>
        <translation>Создаёт значок авторского права в области карты.</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="204"/>
        <source>&amp;Decorations</source>
        <translation>&amp;Оформление</translation>
    </message>
</context>
<context>
    <name>QgsCopyrightLabelPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="13"/>
        <source>Copyright Label Plugin</source>
        <translation>Модуль метки авторского права</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="145"/>
        <source>Placement</source>
        <translation>Размещение</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="153"/>
        <source>Bottom Left</source>
        <translation>Внизу слева</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="158"/>
        <source>Top Left</source>
        <translation>Вверху слева</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="163"/>
        <source>Bottom Right</source>
        <translation>Внизу справа</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="168"/>
        <source>Top Right</source>
        <translation>Вверху справа</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="176"/>
        <source>Orientation</source>
        <translation>Ориентация</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="184"/>
        <source>Horizontal</source>
        <translation>Горизонтальная</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="189"/>
        <source>Vertical</source>
        <translation>Вертикальная</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="118"/>
        <source>Enable Copyright Label</source>
        <translation>Включить метку авторского права</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="36"/>
        <source>Color</source>
        <translation>Цвет</translation>
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
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:12pt;&quot;&gt;Описание&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Введите вашу метку авторского права в следующем поле. Для форматирования метки разрешается использовать базовую HTML-разметку. Например:&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-weight:600;&quot;&gt;&amp;lt;B&amp;gt;Полужирный шрифт&amp;lt;/B&amp;gt; &lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-weight:600;&quot;&gt;&lt;span style=&quot; font-weight:400; font-style:italic;&quot;&gt;&amp;lt;I&amp;gt;Курсив&amp;lt;/I&amp;gt;&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-style:italic;&quot;&gt;&lt;span style=&quot; font-style:normal;&quot;&gt;(значок &amp;copy; задаётся последовательностью «&amp;amp;copy;»)&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message encoding="UTF-8">
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="130"/>
        <source>© QGIS 2008</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>QgsCustomProjectionDialog</name>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="175"/>
        <source>Delete Projection Definition?</source>
        <translation>Удалить определение проекции?</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="176"/>
        <source>Deleting a projection definition is not reversable. Do you want to delete it?</source>
        <translation>Удаление определения проекции ‒ необратимая операция. Вы уверены, что хотите удалить его?</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="882"/>
        <source>Abort</source>
        <translation>Отменить</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="884"/>
        <source>New</source>
        <translation>Создать</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="939"/>
        <source>QGIS Custom Projection</source>
        <translation>Пользовательская проекция QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="756"/>
        <source>This proj4 projection definition is not valid. Please give the projection a name before pressing save.</source>
        <translation>Неверное определение проекции proj4. Пожалуйста, введите имя проекции перед сохранением.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="762"/>
        <source>This proj4 projection definition is not valid. Please add the parameters before pressing save.</source>
        <translation>Неверное определение проекции proj4. Пожалуйста, введите параметры проекции перед сохранением.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="777"/>
        <source>This proj4 projection definition is not valid. Please add a proj= clause before pressing save.</source>
        <translation>Неверное определение проекции proj4. Пожалуйста, введите условие proj= перед сохранением.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="784"/>
        <source>This proj4 ellipsoid definition is not valid. Please add a ellips= clause before pressing save.</source>
        <translation>Неверное определение эллипсоида proj4. Пожалуйста, введите условие ellips= перед сохранением.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="800"/>
        <source>This proj4 projection definition is not valid. Please correct before pressing save.</source>
        <translation>Неверное определение проекции proj4. Пожалуйста, исправьте его перед сохранением.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="913"/>
        <source>This proj4 projection definition is not valid.</source>
        <translation>Неверное определение проекции proj4.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="928"/>
        <source>Northing and Easthing must be in decimal form.</source>
        <translation>Север и восток следует вводить в десятичной форме.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="940"/>
        <source>Internal Error (source projection invalid?)</source>
        <translation>Внутренняя ошибка (неверная исходная проекция?)</translation>
    </message>
</context>
<context>
    <name>QgsCustomProjectionDialogBase</name>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="13"/>
        <source>Custom Projection Definition</source>
        <translation>Определение пользовательской проекции</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="22"/>
        <source>Define</source>
        <translation>Определение</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="203"/>
        <source>Parameters:</source>
        <translation type="obsolete">Параметры:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="66"/>
        <source>|&lt;</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="76"/>
        <source>&lt;</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="86"/>
        <source>1 of 1</source>
        <translation>1 из 1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="96"/>
        <source>&gt;</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="106"/>
        <source>&gt;|</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="125"/>
        <source>New</source>
        <translation type="obsolete">НовоеСоздать</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="132"/>
        <source>Save</source>
        <translation type="obsolete">Сохранить</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="139"/>
        <source>Delete</source>
        <translation type="obsolete">Удалить</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="146"/>
        <source>Close</source>
        <translation type="obsolete">Закрыть</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="155"/>
        <source>Name:</source>
        <translation type="obsolete">Имя:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="151"/>
        <source>Test</source>
        <translation>Проверка</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="189"/>
        <source>Transform from WGS84 to the chosen projection</source>
        <translation type="obsolete">Преобразование в заданную проекцию из WGS84</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="253"/>
        <source>Calculate</source>
        <translation>Расчитать</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Projected Corrdinate System</source>
        <translation type="obsolete">Система координат проекции</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="180"/>
        <source>Geographic / WGS84</source>
        <translation>Географическая / WGS84</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="275"/>
        <source>North:</source>
        <translation type="obsolete">Север:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="265"/>
        <source>East:</source>
        <translation type="obsolete">Восток:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
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
        <location filename="" line="0"/>
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
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="28"/>
        <source>You can define your own custom projection here. The definition must conform to the proj4 format for specifying a Spatial Reference System.</source>
        <translation>В этом диалоге вы можете определить вашу собственную проекцию. Определение проекции должно быть задано в формате координатных систем PROJ4.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="157"/>
        <source>Use the text boxes below to test the projection definition you are creating. Enter a coordinate where both the lat/long and the projected result are known (for example by reading off a map). Then press the calculate button to see if the projection definition you are creating is accurate.</source>
        <translation>Используйте данные поля для проверки вновь созданной проекции. Введите точку для которой известны широта/долгота и прямоугольные координаты (например, с карты). После этого нажмите кнопку «Расчитать» и проверьте, верно ли задана ваша проекция.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="187"/>
        <source>Projected Coordinate System</source>
        <translation>Прямоугольная система координат</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="38"/>
        <source>Name</source>
        <translation type="unfinished">Имя</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="167"/>
        <source>Parameters</source>
        <translation type="unfinished">Параметры</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="116"/>
        <source>*</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="126"/>
        <source>S</source>
        <translation type="unfinished">Ю</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="136"/>
        <source>X</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="194"/>
        <source>North</source>
        <translation type="unfinished">Север</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="224"/>
        <source>East</source>
        <translation type="unfinished">Восток</translation>
    </message>
</context>
<context>
    <name>QgsDbSourceSelect</name>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="230"/>
        <source>Are you sure you want to remove the </source>
        <translation>Вы уверены, что хотите удалить соединение </translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="230"/>
        <source> connection and all associated settings?</source>
        <translation> и все связанные с ним параметры?</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="231"/>
        <source>Confirm Delete</source>
        <translation>Потвердите удаление</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Yes</source>
        <translation type="obsolete">Да</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>No</source>
        <translation type="obsolete">Нет</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="338"/>
        <source>Select Table</source>
        <translation>Выберите таблицу</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="338"/>
        <source>You must select a table in order to add a Layer.</source>
        <translation>Для добавления слоя необходимо выбрать таблицу.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="371"/>
        <source>Password for </source>
        <translation>Пароль для </translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="372"/>
        <source>Please enter your password:</source>
        <translation>Пожалуйста, введите ваш пароль:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="437"/>
        <source>Connection failed</source>
        <translation>Не удалось соединиться</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="147"/>
        <source>Type</source>
        <translation>Тип</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Name</source>
        <translation type="obsolete">Имя</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="155"/>
        <source>Sql</source>
        <translation>SQL</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="440"/>
        <source>Connection to %1 on %2 failed. Either the database is down or your settings are incorrect.%3Check your username and password and try again.%4The database said:%5%6</source>
        <translation>Не удалось подключиться к %1 на %2. Вероятно, база данных выключена, или же вы указали неверные параметры.%3Проверьте имя пользователя и пароль и попытайтесь снова.%4Сообщение БД:%5%6</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="123"/>
        <source>Wildcard</source>
        <translation>Шаблон</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="127"/>
        <source>RegExp</source>
        <translation>Рег. выражение</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="135"/>
        <source>All</source>
        <translation>Все</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="139"/>
        <source>Schema</source>
        <translation>Схема</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="143"/>
        <source>Table</source>
        <translation>Таблица</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="151"/>
        <source>Geometry column</source>
        <translation>Поле геометрии</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="547"/>
        <source>Accessible tables could not be determined</source>
        <translation>Не удалось распознать доступные таблицы</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="549"/>
        <source>Database connection was successful, but the accessible tables could not be determined.

The error message from the database was:
%1
</source>
        <translation>Соединение с базой данных установлено, но доступные таблицы не были распознаны.

Сообщение базы данных:
%1
</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="554"/>
        <source>No accessible tables found</source>
        <translation>Доступные таблицы не найдены</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="558"/>
        <source>Database connection was successful, but no accessible tables were found.

Please verify that you have SELECT privilege on a table carrying PostGIS
geometry.</source>
        <translation>Соединение с базой данных установлено, но доступные таблицы не были найдены.

Пожалуйста, проверьте, что у вас есть права на выполнение SELECT для таблиц, содержащих PostGIS-геометрию.</translation>
    </message>
</context>
<context>
    <name>QgsDbSourceSelectBase</name>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="13"/>
        <source>Add PostGIS Table(s)</source>
        <translation>Добавить таблицы PostGIS</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="140"/>
        <source>Add</source>
        <translation>Добавить</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="111"/>
        <source>Help</source>
        <translation>Справка</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="114"/>
        <source>F1</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="87"/>
        <source>Connect</source>
        <translation>Подключить</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="80"/>
        <source>New</source>
        <translation>Новое</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="73"/>
        <source>Edit</source>
        <translation>Изменить</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="66"/>
        <source>Delete</source>
        <translation>Удалить</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="156"/>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="54"/>
        <source>PostgreSQL Connections</source>
        <translation>PostgreSQL-соединения</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Tables:</source>
        <translation type="obsolete">Таблицы:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Type</source>
        <translation type="obsolete">Тип</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Name</source>
        <translation type="obsolete">Имя</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Sql</source>
        <translation type="obsolete">SQL</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Encoding:</source>
        <translation type="obsolete">Кодировка:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="183"/>
        <source>Search:</source>
        <translation>Поиск:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="190"/>
        <source>Search mode:</source>
        <translation>Режим поиска:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="200"/>
        <source>Search in columns:</source>
        <translation>Искать в полях:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdbsourceselectbase.ui" line="216"/>
        <source>Search options...</source>
        <translation>Поиск...</translation>
    </message>
</context>
<context>
    <name>QgsDbTableModel</name>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="24"/>
        <source>Schema</source>
        <translation>Схема</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="25"/>
        <source>Table</source>
        <translation>Таблица</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="26"/>
        <source>Type</source>
        <translation>Тип</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="27"/>
        <source>Geometry column</source>
        <translation>Поле геометрии</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="28"/>
        <source>Sql</source>
        <translation>SQL</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="229"/>
        <source>Point</source>
        <translation>Точка</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="233"/>
        <source>Multipoint</source>
        <translation>Мультиточка</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="237"/>
        <source>Line</source>
        <translation>Линия</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="241"/>
        <source>Multiline</source>
        <translation>Мультилиния</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="245"/>
        <source>Polygon</source>
        <translation>Полигон</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="249"/>
        <source>Multipolygon</source>
        <translation>Мультиполигон</translation>
    </message>
</context>
<context>
    <name>QgsDelAttrDialogBase</name>
    <message>
        <location filename="../src/ui/qgsdelattrdialogbase.ui" line="13"/>
        <source>Delete Attributes</source>
        <translation>Удалить атрибуты</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdelattrdialogbase.ui" line="52"/>
        <source>OK</source>
        <translation type="obsolete">OK</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdelattrdialogbase.ui" line="59"/>
        <source>Cancel</source>
        <translation type="obsolete">Отменить</translation>
    </message>
</context>
<context>
    <name>QgsDelimitedTextPlugin</name>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="101"/>
        <source>&amp;Add Delimited Text Layer</source>
        <translation>Добавить слой из &amp;текста с разделителями</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="104"/>
        <source>Add a delimited text file as a map layer. </source>
        <translation>Добавить текстовый файл с разделителями как слой карты.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="105"/>
        <source>The file must have a header row containing the field names. </source>
        <translation>Файл должен включать строку заголовка, содержащую имена полей.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="105"/>
        <source>X and Y fields are required and must contain coordinates in decimal units.</source>
        <translation>Поля X и Y обязательны и должны содержать координаты в десятичных единицах.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="142"/>
        <source>&amp;Delimited text</source>
        <translation>&amp;Текст с разделителями</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="56"/>
        <source>DelimitedTextLayer</source>
        <translation>Слой текста с разделителями</translation>
    </message>
</context>
<context>
    <name>QgsDelimitedTextPluginGui</name>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="125"/>
        <source>No layer name</source>
        <translation>Не указано имя слоя</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="125"/>
        <source>Please enter a layer name before adding the layer to the map</source>
        <translation>Пожалуйста, введите имя слоя перед его добавлением к карте</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="211"/>
        <source>No delimiter</source>
        <translation>Разделитель не указан</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="211"/>
        <source>Please specify a delimiter prior to parsing the file</source>
        <translation>Пожалуйста, укажите разделитель до начала загрузки файла</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="245"/>
        <source>Choose a delimited text file to open</source>
        <translation>Выберите текстовый файл с разделителями</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="35"/>
        <source>Parse</source>
        <translation>Анализ</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="60"/>
        <source>Description</source>
        <translation>Описание</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="63"/>
        <source>Select a delimited text file containing a header row and one or more rows of x and y coordinates that you would like to use as a point layer and this plugin will do the job for you!</source>
        <translation>Выберите текстовый файл с разделителями, содержащий строку заголовка и строки, содержащие XY-координаты и этот модуль преобразует его в точечный слой!</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="67"/>
        <source>Use the layer name box to specify the legend name for the new layer. Use the delimiter box to specify what delimeter is used in your file (e.g. space, comma, tab or a regular expression in Perl style). After choosing a delimiter, press the parse button and select the columns containing the x and y values for the layer.</source>
        <translation>Введите имя слоя в легенде в поле «имя слоя». В поле «разделитель» введите разделитель, используемый в вашем файле (пробел, запятая, TAB или регулярное выражение в стиле Python). После выбора разделителя, нажмите кнопку «Анализ» и выберите поля, содержащие координаты X и Y.</translation>
    </message>
</context>
<context>
    <name>QgsDelimitedTextPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="13"/>
        <source>Create a Layer from a Delimited Text File</source>
        <translation>Создать слой из текста с разделителями</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="67"/>
        <source>&lt;p align=&quot;right&quot;&gt;X field&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;right&quot;&gt;X-поле&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="88"/>
        <source>Name of the field containing x values</source>
        <translation>Имя поля, содержащего X-значения</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="91"/>
        <source>Name of the field containing x values. Choose a field from the list. The list is generated by parsing the header row of the delimited text file.</source>
        <translation>Имя поля, содержащего X-значения. Выберите поле из списка, создаваемого анализом строки заголовка в текстовом файле.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="101"/>
        <source>&lt;p align=&quot;right&quot;&gt;Y field&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;right&quot;&gt;Y-поле&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="122"/>
        <source>Name of the field containing y values</source>
        <translation>Имя поля, содержащего Y-значения</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="125"/>
        <source>Name of the field containing y values. Choose a field from the list. The list is generated by parsing the header row of the delimited text file.</source>
        <translation>Имя поля, содержащего Y-значения. Выберите поле из списка, создаваемого анализом строки заголовка в текстовом файле.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="191"/>
        <source>Layer name</source>
        <translation>Имя слоя</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="198"/>
        <source>Name to display in the map legend</source>
        <translation>Имя для отображения в легенде карты</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="201"/>
        <source>Name displayed in the map legend</source>
        <translation>Имя, отображаемое в легенде карты</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="297"/>
        <source>Delimiter</source>
        <translation>Разделитель</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="318"/>
        <source>Delimiter to use when splitting fields in the text file. The delimiter can be more than one character.</source>
        <translation>Разделитель полей в текстовом файле. Разделитель может состоять из более чем одного символа.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="321"/>
        <source>Delimiter to use when splitting fields in the delimited text file. The delimiter can be 1 or more characters in length.</source>
        <translation>Разделитель полей в текстовом файле. Разделитель может состоять из одного и более символов.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="39"/>
        <source>Delimited Text Layer</source>
        <translation>Слой текста с разделителями</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="145"/>
        <source>Delimited text file</source>
        <translation>Текстовый файл</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="152"/>
        <source>Full path to the delimited text file</source>
        <translation>Полный путь к текстовому файлу</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="155"/>
        <source>Full path to the delimited text file. In order to properly parse the fields in the file, the delimiter must be defined prior to entering the file name. Use the Browse button to the right of this field to choose the input file.</source>
        <translation>Полный путь к текстовому файлу. Для обеспечения правильности анализа файла, разделитель следует указывать перед вводом имени файла. Нажмите кнопку «Обзор» для интерактивного выбора файла.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="168"/>
        <source>Browse to find the delimited text file to be processed</source>
        <translation>Выбор текстового файла для обработки</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="171"/>
        <source>Use this button to browse to the location of the delimited text file. This button will not be enabled until a delimiter has been entered in the &lt;i&gt;Delimiter&lt;/i&gt; box. Once a file is chosen, the X and Y field drop-down boxes will be populated with the fields from the delimited text file.</source>
        <translation>Используйте эту кнопку для выбора текстового файла. Кнопка не будет активирована, пока разделитель не будет введён в поле &lt;i&gt;Разделитель&lt;/i&gt;. После того, как файл будет выбран, списки X и Y-полей будут заполнены именами полей из текстового файла.</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Help</source>
        <translation type="obsolete">&amp;Помощь</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="246"/>
        <source>Sample text</source>
        <translation>Образец</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="174"/>
        <source>Browse...</source>
        <translation>Обзор...</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="331"/>
        <source>The delimiter is taken as is</source>
        <translation>Разделитель используется как есть</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="334"/>
        <source>Plain characters</source>
        <translation>Простой текст</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="344"/>
        <source>The delimiter is a regular expression</source>
        <translation>Разделитель является регулярным выражением</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="347"/>
        <source>Regular expression</source>
        <translation>Регулярное выражение</translation>
    </message>
</context>
<context>
    <name>QgsDelimitedTextProvider</name>
    <message>
        <location filename="../src/providers/delimitedtext/qgsdelimitedtextprovider.cpp" line="402"/>
        <source>Note: the following lines were not loaded because Qgis was unable to determine values for the x and y coordinates:
</source>
        <translation>Внимание: следующие строки не были загружены, потому что не удалось определить значения XY-координат:
</translation>
    </message>
    <message>
        <location filename="../src/providers/delimitedtext/qgsdelimitedtextprovider.cpp" line="400"/>
        <source>Error</source>
        <translation>Ошибка</translation>
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
        <translation>Буферизация объектов</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Help</source>
        <translation type="obsolete">&amp;Помощь</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="224"/>
        <source>Buffer distance in map units:</source>
        <translation>Зона буфера в единицах карты:</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="132"/>
        <source>Table name for the buffered layer:</source>
        <translation>Имя таблицы для слоя буферных зон:</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="172"/>
        <source>Create unique object id</source>
        <translation>Создавать уникальные ID объектов</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="216"/>
        <source>public</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="59"/>
        <source>Geometry column:</source>
        <translation>Поле геометрии:</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="92"/>
        <source>Spatial reference ID:</source>
        <translation>ID системы координат (SRID):</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="125"/>
        <source>Unique field to use as feature id:</source>
        <translation>Уникальное поле для ID объектов:</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="118"/>
        <source>Schema:</source>
        <translation>Схема:</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="66"/>
        <source>Add the buffered layer to the map?</source>
        <translation>Добавить на карту слой буферных зон?</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="234"/>
        <source>&lt;h2&gt;Buffer the features in layer: &lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Буферизация объектов слоя: &lt;/h2&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgsdlgpgbufferbase.ui" line="28"/>
        <source>Parameters</source>
        <translation>Параметры</translation>
    </message>
</context>
<context>
    <name>QgsEditReservedWordsBase</name>
    <message>
        <location filename="../src/plugins/spit/qgseditreservedwordsbase.ui" line="13"/>
        <source>Edit Reserved Words</source>
        <translation type="obsolete">Зарезервированные слова</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgseditreservedwordsbase.ui" line="47"/>
        <source>Status</source>
        <translation type="obsolete">Состояние</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgseditreservedwordsbase.ui" line="57"/>
        <source>Index</source>
        <translation type="obsolete">Индекс</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Help</source>
        <translation type="obsolete">&amp;Помощь</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgseditreservedwordsbase.ui" line="89"/>
        <source>Reserved Words</source>
        <translation type="obsolete">Зарезервированные слова</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgseditreservedwordsbase.ui" line="37"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Double click the Column Name column to change the name of the column.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Для изменения имени поля, дважды щёлкните в столбце «Имя поля».&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgseditreservedwordsbase.ui" line="52"/>
        <source>Column Name</source>
        <translation type="obsolete">Имя поля</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgseditreservedwordsbase.ui" line="82"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;This shapefile contains reserved words. These may affect the import into PostgreSQL. Edit the column names so none of the reserved words listed at the right are used (click on a Column Name entry to edit). You may also change any other column name if desired.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Данный shape-файл содержит зарезервированные слова, которые могут повлиять на импорт в PostgreSQL. Исправьте имена полей так, чтобы они не содержали зарезервированных слов, перечисленных справа (для изменения щёлкните в столбце «Имя поля»). По желанию вы можете изменить и имена других полей.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
</context>
<context>
    <name>QgsEditReservedWordsDialog</name>
    <message>
        <location filename="../src/plugins/spit/qgseditreservedwordsdialog.cpp" line="34"/>
        <source>Status</source>
        <translation type="obsolete">Состояние</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgseditreservedwordsdialog.cpp" line="34"/>
        <source>Column Name</source>
        <translation type="obsolete">Имя поля</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgseditreservedwordsdialog.cpp" line="34"/>
        <source>Index</source>
        <translation type="obsolete">Индекс</translation>
    </message>
</context>
<context>
    <name>QgsEncodingFileDialog</name>
    <message>
        <location filename="../src/gui/qgsencodingfiledialog.cpp" line="29"/>
        <source>Encoding:</source>
        <translation>Кодировка:</translation>
    </message>
</context>
<context>
    <name>QgsFillStyleWidgetBase</name>
    <message>
        <location filename="../build/src/ui/ui_qgsfillstylewidgetbase.h" line="83"/>
        <source>Form1</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../build/src/ui/ui_qgsfillstylewidgetbase.h" line="84"/>
        <source>Fill Style</source>
        <translation>Стиль заливки</translation>
    </message>
    <message>
        <location filename="../build/src/ui/ui_qgsfillstylewidgetbase.h" line="90"/>
        <source>PolyStyleWidget</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../build/src/ui/ui_qgsfillstylewidgetbase.h" line="86"/>
        <source>Colour:</source>
        <translation>Цвет:</translation>
    </message>
    <message>
        <location filename="../build/src/ui/ui_qgsfillstylewidgetbase.h" line="85"/>
        <source>col</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>QgsGPSDeviceDialog</name>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialog.cpp" line="44"/>
        <source>New device %1</source>
        <translation>Новое устройство %1</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialog.cpp" line="57"/>
        <source>Are you sure?</source>
        <translation>Вы уверены?</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialog.cpp" line="58"/>
        <source>Are you sure that you want to delete this device?</source>
        <translation>Вы уверены, что хотите удалить это устройство?</translation>
    </message>
</context>
<context>
    <name>QgsGPSDeviceDialogBase</name>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="21"/>
        <source>GPS Device Editor</source>
        <translation>Редактор GPS-устройств</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="132"/>
        <source>Device name:</source>
        <translation>Имя устройства:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="147"/>
        <source>This is the name of the device as it will appear in the lists</source>
        <translation>Имя устройства, которое отображается в списке</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="99"/>
        <source>Update device</source>
        <translation>Обновить устройство</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="84"/>
        <source>Delete device</source>
        <translation>Удалить устройство</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="69"/>
        <source>New device</source>
        <translation>Новое устройство</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="298"/>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="156"/>
        <source>Commands</source>
        <translation>Команды</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="185"/>
        <source>Waypoint download:</source>
        <translation>Загрузка точек:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="248"/>
        <source>Waypoint upload:</source>
        <translation>Выгрузка точек:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="199"/>
        <source>Route download:</source>
        <translation>Загрузка маршрутов:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="178"/>
        <source>Route upload:</source>
        <translation>Выгрузка маршрутов:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="171"/>
        <source>Track download:</source>
        <translation>Загрузка треков:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="241"/>
        <source>The command that is used to upload tracks to the device</source>
        <translation>Команда, используемая для выгрузки треков в устройство</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="213"/>
        <source>Track upload:</source>
        <translation>Выгрузка треков:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="220"/>
        <source>The command that is used to download tracks from the device</source>
        <translation>Команда, используемая для загрузки треков из устройства</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="227"/>
        <source>The command that is used to upload routes to the device</source>
        <translation>Команда, используемая для выгрузки маршрутов в устройство</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="192"/>
        <source>The command that is used to download routes from the device</source>
        <translation>Команда, используемая для загрузки маршрутов из устройства</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="206"/>
        <source>The command that is used to upload waypoints to the device</source>
        <translation>Команда, используемая для выгрузки точек в устройство</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="234"/>
        <source>The command that is used to download waypoints from the device</source>
        <translation>Команда, используемая для загрузки точек из устройства</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="266"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;In the download and upload commands there can be special words that will be replaced by QGIS when the commands are used. These words are:&lt;span style=&quot; font-style:italic;&quot;&gt;%babel&lt;/span&gt; - the path to GPSBabel&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%in&lt;/span&gt; - the GPX filename when uploading or the port when downloading&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%out&lt;/span&gt; - the port when uploading or the GPX filename when downloading&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;В командах загрузки и выгрузки допускается ввод специальных слов, которые будут изменены QGIS при запуске команды. Этими словами являются:&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%babel&lt;/span&gt; — путь к GPSBabel&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%in&lt;/span&gt; — GPX-файл (выгрузка) или порт (загрузка)&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%out&lt;/span&gt; — порт (выгрузка) или GPX-файл (загрузка)&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
</context>
<context>
    <name>QgsGPSPlugin</name>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="93"/>
        <source>&amp;Gps Tools</source>
        <translation>&amp;GPS-инструменты</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="94"/>
        <source>&amp;Create new GPX layer</source>
        <translation>&amp;Создать новый GPX-слой</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="97"/>
        <source>Creates a new GPX layer and displays it on the map canvas</source>
        <translation>Создать новый GPX-слой и вывести его на карте</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="196"/>
        <source>&amp;Gps</source>
        <translation>&amp;GPS</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="160"/>
        <source>Save new GPX file as...</source>
        <translation>Сохранить новый GPX-файл как...</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="160"/>
        <source>GPS eXchange file (*.gpx)</source>
        <translation>Файлы GPS eXchange (*.gpx)</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="165"/>
        <source>Could not create file</source>
        <translation>Не удалось создать файл</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="167"/>
        <source>Unable to create a GPX file with the given name. </source>
        <translation>Не удалось создать GPX-файл с заданным именем. </translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="168"/>
        <source>Try again with another name or in another </source>
        <translation>Попробуйте ещё раз с другим именем или в другом </translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="168"/>
        <source>directory.</source>
        <translation>каталоге.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="207"/>
        <source>GPX Loader</source>
        <translation>Загрузчик GPX</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="209"/>
        <source>Unable to read the selected file.
</source>
        <translation>Не удалось прочитать выбранный файл.
</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="209"/>
        <source>Please reselect a valid file.</source>
        <translation>Пожалуйста, выберите правильный файл.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="487"/>
        <source>Could not start process</source>
        <translation>Не удалось запустить процесс</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="488"/>
        <source>Could not start GPSBabel!</source>
        <translation>Не удалось запустить GPSBabel!</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="330"/>
        <source>Importing data...</source>
        <translation>Импорт данных...</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="493"/>
        <source>Cancel</source>
        <translation>Отменить</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="274"/>
        <source>Could not import data from %1!

</source>
        <translation>Ошибка импорта данных из %1!

</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="276"/>
        <source>Error importing data</source>
        <translation>Ошибка импорта данных</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="476"/>
        <source>Not supported</source>
        <translation>Функция не поддерживается</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="394"/>
        <source>This device does not support downloading </source>
        <translation>Это устройство не поддерживает загрузку </translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="394"/>
        <source>of </source>
        <translation>данных типа </translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="410"/>
        <source>Downloading data...</source>
        <translation>Загрузка данных...</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="421"/>
        <source>Could not download data from GPS!

</source>
        <translation>Ошибка загрузки данных из GPS!

</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="423"/>
        <source>Error downloading data</source>
        <translation>Ошибка загрузки данных</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="477"/>
        <source>This device does not support uploading of </source>
        <translation>Это устройство не поддерживает выгрузку данных типа  </translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="493"/>
        <source>Uploading data...</source>
        <translation>Выгрузка данных...</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="504"/>
        <source>Error while uploading data to GPS!

</source>
        <translation>Ошибка выгрузки данных в GPS!

</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="506"/>
        <source>Error uploading data</source>
        <translation>Ошибка выгрузки данных</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="342"/>
        <source>Could not convert data from %1!

</source>
        <translation>Не удалось преобразовать данные из %1!</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="344"/>
        <source>Error converting data</source>
        <translation>Ошибка преобразования данных</translation>
    </message>
</context>
<context>
    <name>QgsGPSPluginGui</name>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="522"/>
        <source>Choose a filename to save under</source>
        <translation>Выберите имя сохраняемого файла</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="524"/>
        <source>GPS eXchange format (*.gpx)</source>
        <translation>Формат GPS eXchange (*.gpx)</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="511"/>
        <source>Select GPX file</source>
        <translation>Выберите GPX-файл</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="250"/>
        <source>Select file and format to import</source>
        <translation>Выберите файл и формат для импорта</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="486"/>
        <source>Waypoints</source>
        <translation>Маршрутные точки</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="486"/>
        <source>Routes</source>
        <translation>Маршруты</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="272"/>
        <source>Tracks</source>
        <translation>Треки</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="491"/>
        <source>QGIS can perform conversions of GPX files, by using GPSBabel (%1) to perform the conversions.</source>
        <translation>QGIS может выполнять преобразование GPX-файлов при помощи пакета GPSBabel (%1).</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="492"/>
        <source>This requires that you have GPSBabel installed where QGIS can find it.</source>
        <translation>Для этого требуется установить пакет GPSBabel так, чтобы он мог быть найден QGIS.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="493"/>
        <source>Select a GPX input file name, the type of conversion you want to perform, a GPX filename that you want to save the converted file as, and a name for the new layer created from the result.</source>
        <translation>Выберите исходный GPX-файл, тип преобразования, которое вы хотели бы осуществить, а также имя файла, в котором будет сохранён результат и имя нового слоя.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="422"/>
        <source>GPX is the %1, which is used to store information about waypoints, routes, and tracks.</source>
        <translation>GPX (%1) — это формат, используемый для хранения маршрутных точек, маршрутов и треков.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="422"/>
        <source>GPS eXchange file format</source>
        <translation>Формат GPS eXchange</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="423"/>
        <source>Select a GPX file and then select the feature types that you want to load.</source>
        <translation>Выберите GPX-файл и типы объектов, которые вы хотели бы загрузить.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="435"/>
        <source>This tool will help you download data from a GPS device.</source>
        <translation>Этот инстумент поможет вам загрузить данные с GPS-устройства.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="436"/>
        <source>Choose your GPS device, the port it is connected to, the feature type you want to download, a name for your new layer, and the GPX file where you want to store the data.</source>
        <translation>Выберите ваше GPS-устройство и порт, к которому оно подключено, а также тип объектов, которые вы хотите загрузить, имя нового слоя и GPX-файл для сохранения данных.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="455"/>
        <source>If your device isn&apos;t listed, or if you want to change some settings, you can also edit the devices.</source>
        <translation>Если вашего устройства нет в списке или вы хотите изменить его параметры, нажмите «Редактировать устройства».</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="457"/>
        <source>This tool uses the program GPSBabel (%1) to transfer the data.</source>
        <translation>Этот инструмент использует GPSBabel (%1) для передачи данных.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="453"/>
        <source>This tool will help you upload data from a GPX layer to a GPS device.</source>
        <translation>Этот инстумент поможет вам выгрузить данные в GPS-устройство из существующего GPX-слоя.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="454"/>
        <source>Choose the layer you want to upload, the device you want to upload it to, and the port your device is connected to.</source>
        <translation>Выберите слой, который вы желаете выгрузить, устройство для выгрузки и порт, к которому оно подключено.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="472"/>
        <source>QGIS can only load GPX files by itself, but many other formats can be converted to GPX using GPSBabel (%1).</source>
        <translation>QGIS может загружать только GPX-файлы, но прочие форматы могут быть преобразованы в GPX при помощи GPSBabel (%1).</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="474"/>
        <source>Select a GPS file format and the file that you want to import, the feature type that you want to use, a GPX filename that you want to save the converted file as, and a name for the new layer.</source>
        <translation>Выберите формат GPS-данных и файл для импорта, а также тип загружаемых объектов, имя GPX-файла, в который будет сохранён результат и имя нового слоя.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="475"/>
        <source>All file formats can not store waypoints, routes, and tracks, so some feature types may be disabled for some file formats.</source>
        <translation>Не все форматы могут содержать маршрутные точки, маршруты и треки, поэтому для некоторых форматов часть типов данных будет выключена.</translation>
    </message>
</context>
<context>
    <name>QgsGPSPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="13"/>
        <source>GPS Tools</source>
        <translation>GPS-инструменты</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="58"/>
        <source>Load GPX file</source>
        <translation>GPX-файлы</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="94"/>
        <source>File:</source>
        <translation>Файл:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="111"/>
        <source>Feature types:</source>
        <translation>Типы объектов:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="314"/>
        <source>Waypoints</source>
        <translation>Маршрутные точки</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="319"/>
        <source>Routes</source>
        <translation>Маршруты</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="324"/>
        <source>Tracks</source>
        <translation>Треки</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="158"/>
        <source>Import other file</source>
        <translation>Прочие файлы</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="264"/>
        <source>File to import:</source>
        <translation>Импортируемый файл:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="339"/>
        <source>Feature type:</source>
        <translation>Тип объектов:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="582"/>
        <source>GPX output file:</source>
        <translation>Выходной GPX-файл:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="558"/>
        <source>Layer name:</source>
        <translation>Имя слоя:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="272"/>
        <source>Download from GPS</source>
        <translation>Загрузка с GPS</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="508"/>
        <source>Edit devices</source>
        <translation>Редактировать устройства</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="515"/>
        <source>GPS device:</source>
        <translation>GPS-устройство:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="356"/>
        <source>Output file:</source>
        <translation>Файл вывода:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="481"/>
        <source>Port:</source>
        <translation>Порт:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="430"/>
        <source>Upload to GPS</source>
        <translation>Выгрузка в GPS</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="522"/>
        <source>Data layer:</source>
        <translation>Слой данных:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Help</source>
        <translation type="obsolete">&amp;Помощь</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="572"/>
        <source>Browse...</source>
        <translation>Обзор...</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="565"/>
        <source>Save As...</source>
        <translation>Сохранить как...</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="250"/>
        <source>(Note: Selecting correct file type in browser dialog important!)</source>
        <translation>(Внимание: важно выбрать правильный тип файла в диалоге выбора файлов!)</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="530"/>
        <source>GPX Conversions</source>
        <translation>Конвертеры GPX</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="615"/>
        <source>Conversion:</source>
        <translation>Преобразование:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="629"/>
        <source>GPX input file:</source>
        <translation>Исходный GPX-файл:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="545"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>QgsGPXProvider</name>
    <message>
        <location filename="../src/providers/gpx/qgsgpxprovider.cpp" line="70"/>
        <source>Bad URI - you need to specify the feature type.</source>
        <translation>Неверный URI — необходимо указать тип объектов.</translation>
    </message>
    <message>
        <location filename="../src/providers/gpx/qgsgpxprovider.cpp" line="114"/>
        <source>GPS eXchange file</source>
        <translation>Файлы GPS eXchange (*.gpx)</translation>
    </message>
    <message>
        <location filename="../src/providers/gpx/qgsgpxprovider.cpp" line="731"/>
        <source>Digitized in QGIS</source>
        <translation>Оцифрован в QGIS</translation>
    </message>
</context>
<context>
    <name>QgsGeomTypeDialog</name>
    <message>
        <location filename="../src/app/qgsgeomtypedialog.cpp" line="31"/>
        <source>Name</source>
        <translation type="obsolete">Имя</translation>
    </message>
    <message>
        <location filename="../src/app/qgsgeomtypedialog.cpp" line="32"/>
        <source>Type</source>
        <translation type="obsolete">Тип</translation>
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
        <translation>Тип</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="41"/>
        <source>Point</source>
        <translation>Точка</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="48"/>
        <source>Line</source>
        <translation>Линия</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="55"/>
        <source>Polygon</source>
        <translation>Полигон</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="13"/>
        <source>New Vector Layer</source>
        <translation>Новый векторный слой</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="64"/>
        <source>Attributes:</source>
        <translation type="obsolete">Атрибуты:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="74"/>
        <source>Add</source>
        <translation type="obsolete">Добавить</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="118"/>
        <source>Column 1</source>
        <translation type="obsolete">Столбец 1</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="41"/>
        <source>Remove</source>
        <translation type="obsolete">Удалить</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="28"/>
        <source>File Format:</source>
        <translation type="obsolete">Формат файла:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="22"/>
        <source>File format</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="65"/>
        <source>Attributes</source>
        <translation type="unfinished">Атрибуты</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="150"/>
        <source>Name</source>
        <translation type="unfinished">Имя</translation>
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
        <translation>Описание модуля привязки</translation>
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
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:11pt; font-weight:600;&quot;&gt;Описание&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:9pt;&quot;&gt;Этот модуль позволяет создать файлы привязки для растров. После того, как вы укажете точки на растре и введёте их географические координаты, модуль сможет расчитать параметры файла привязки. Чем больше точек будет указано, тем лучше будет результат.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
</context>
<context>
    <name>QgsGeorefPlugin</name>
    <message>
        <location filename="../src/plugins/georeferencer/plugin.cpp" line="119"/>
        <source>&amp;Georeferencer</source>
        <translation>&amp;Привязка растров</translation>
    </message>
</context>
<context>
    <name>QgsGeorefPluginGui</name>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="85"/>
        <source>Choose a raster file</source>
        <translation>Выберите растровый файл</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="87"/>
        <source>Raster files (*.*)</source>
        <translation>Растровые файлы (*.*)</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="97"/>
        <source>Error</source>
        <translation>Ошибка</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="98"/>
        <source>The selected file is not a valid raster file.</source>
        <translation>Выбранный файл не является действительным растровым файлом.</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="122"/>
        <source>World file exists</source>
        <translation>Файл привязки уже существует</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="124"/>
        <source>&lt;p&gt;The selected file already seems to have a </source>
        <translation>&lt;p&gt;Судя по всему выбранный файл уже имеет </translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="125"/>
        <source>world file! Do you want to replace it with the </source>
        <translation>файл привязки! Вы хотите заменить его </translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="125"/>
        <source>new world file?&lt;/p&gt;</source>
        <translation>новым файлом привязки?&lt;/p&gt;</translation>
    </message>
</context>
<context>
    <name>QgsGeorefPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/georeferencer/pluginguibase.ui" line="13"/>
        <source>Georeferencer</source>
        <translation>Привязка растров</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/pluginguibase.ui" line="100"/>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/pluginguibase.ui" line="62"/>
        <source>Raster file:</source>
        <translation>Растровый файл:</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/pluginguibase.ui" line="28"/>
        <source>Arrange plugin windows</source>
        <translation>Выровнять окна модуля</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/pluginguibase.ui" line="43"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/pluginguibase.ui" line="77"/>
        <source>Description...</source>
        <translation>Описание...</translation>
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
        <translation>Параметры преобразования</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="35"/>
        <source>Resampling method:</source>
        <translation>Метод интерполяции:</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="46"/>
        <source>Nearest neighbour</source>
        <translation>Ближайший сосед</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="51"/>
        <source>Linear</source>
        <translation>Линейная</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="56"/>
        <source>Cubic</source>
        <translation>Кубическая</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="74"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="64"/>
        <source>Use 0 for transparency when needed</source>
        <translation>Использовать 0 для прозрачности при необходимости</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsgeorefwarpoptionsdialogbase.ui" line="28"/>
        <source>Compression:</source>
        <translation>Сжатие:</translation>
    </message>
</context>
<context>
    <name>QgsGraduatedSymbolDialog</name>
    <message>
        <location filename="../src/app/qgsgraduatedsymboldialog.cpp" line="324"/>
        <source>Equal Interval</source>
        <translation>Равные интервалы</translation>
    </message>
    <message>
        <location filename="../src/app/qgsgraduatedsymboldialog.cpp" line="301"/>
        <source>Quantiles</source>
        <translation>Квантили</translation>
    </message>
    <message>
        <location filename="../src/app/qgsgraduatedsymboldialog.cpp" line="350"/>
        <source>Empty</source>
        <translation>Пустые</translation>
    </message>
</context>
<context>
    <name>QgsGraduatedSymbolDialogBase</name>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="25"/>
        <source>graduated Symbol</source>
        <translation>градуированный знак</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="73"/>
        <source>Classification Field:</source>
        <translation type="obsolete">Поле классификации:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="89"/>
        <source>Mode:</source>
        <translation type="obsolete">Режим:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="105"/>
        <source>Number of Classes:</source>
        <translation type="obsolete">Количество классов:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="188"/>
        <source>Delete class</source>
        <translation>Удалить класс</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="181"/>
        <source>Classify</source>
        <translation>Классифицировать</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="55"/>
        <source>Classification field</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="93"/>
        <source>Mode</source>
        <translation type="unfinished">Режим</translation>
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
        <translation>Внимание</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="152"/>
        <source>Column</source>
        <translation>Поле</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="153"/>
        <source>Value</source>
        <translation>Значение</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="154"/>
        <source>Type</source>
        <translation>Тип</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="301"/>
        <source>ERROR</source>
        <translation>ОШИБКА</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="303"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="158"/>
        <source>Layer</source>
        <translation>Слой</translation>
    </message>
</context>
<context>
    <name>QgsGrassAttributesBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="48"/>
        <source>GRASS Attributes</source>
        <translation>Атрибуты GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="78"/>
        <source>Tab 1</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="112"/>
        <source>result</source>
        <translation>результат</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="180"/>
        <source>Update</source>
        <translation>Обновить</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="177"/>
        <source>Update database record</source>
        <translation>Обновить запись базы данных</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="210"/>
        <source>New</source>
        <translation>Новая</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="207"/>
        <source>Add new category using settings in GRASS Edit toolbox</source>
        <translation>Добавить новую категорию, используя параметры редактора GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="240"/>
        <source>Delete</source>
        <translation>Удалить</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="237"/>
        <source>Delete selected category</source>
        <translation>Удалить выбранную категорию</translation>
    </message>
</context>
<context>
    <name>QgsGrassBrowser</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="66"/>
        <source>Tools</source>
        <translation>Инструменты</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="71"/>
        <source>Add selected map to canvas</source>
        <translation>Добавить выбранную карту в область QGIS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="79"/>
        <source>Copy selected map</source>
        <translation>Копировать выбранную карту</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="87"/>
        <source>Rename selected map</source>
        <translation>Переименовать выбранную карту</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="95"/>
        <source>Delete selected map</source>
        <translation>Удалить выбранную карту</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="103"/>
        <source>Set current region to selected map</source>
        <translation>Установить регион по границам выбранной карты</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="111"/>
        <source>Refresh</source>
        <translation>Обновить</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="453"/>
        <source>Warning</source>
        <translation>Внимание</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="289"/>
        <source>Cannot copy map </source>
        <translation>Не удалось скопировать карту </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="411"/>
        <source>&lt;br&gt;command: </source>
        <translation>&lt;br&gt;команда: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="355"/>
        <source>Cannot rename map </source>
        <translation>Не удалось переименовать карту </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="393"/>
        <source>Delete map &lt;b&gt;</source>
        <translation>Удалить карту &lt;b&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="410"/>
        <source>Cannot delete map </source>
        <translation>Не удалось удалить карту </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="454"/>
        <source>Cannot write new region</source>
        <translation>Не удалось сохранить новый регион</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="339"/>
        <source>New name</source>
        <translation>Новое имя</translation>
    </message>
</context>
<context>
    <name>QgsGrassEdit</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="217"/>
        <source>New point</source>
        <translation>Новая точка</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="235"/>
        <source>New centroid</source>
        <translation>Новый центроид</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="253"/>
        <source>Delete vertex</source>
        <translation>Удалить вершину</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1821"/>
        <source>Left: </source>
        <translation>Левая: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1822"/>
        <source>Middle: </source>
        <translation>Средняя: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="214"/>
        <source>Edit tools</source>
        <translation>Инструменты редактора</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="223"/>
        <source>New line</source>
        <translation>Новая линия</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="229"/>
        <source>New boundary</source>
        <translation>Новая граница</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="241"/>
        <source>Move vertex</source>
        <translation>Переместить вершину</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="247"/>
        <source>Add vertex</source>
        <translation>Добавить вершину</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="259"/>
        <source>Move element</source>
        <translation>Переместить элемент</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="265"/>
        <source>Split line</source>
        <translation>Разделить линию</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="271"/>
        <source>Delete element</source>
        <translation>Удалить элемент</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="277"/>
        <source>Edit attributes</source>
        <translation>Изменить атрибуты</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="282"/>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1458"/>
        <source>Warning</source>
        <translation>Внимание</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="201"/>
        <source>You are not owner of the mapset, cannot open the vector for editing.</source>
        <translation>Вы не являетесь владельцем набора, невозможно изменить векторный слой.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="206"/>
        <source>Cannot open vector for update.</source>
        <translation>Не удалось открыть векторный слой для обновления.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="665"/>
        <source>Info</source>
        <translation>Информация</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="665"/>
        <source>The table was created</source>
        <translation>Таблица была создана</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1333"/>
        <source>Tool not yet implemented.</source>
        <translation>Инструмент не реализован.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1357"/>
        <source>Cannot check orphan record: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1364"/>
        <source>Orphan record was left in attribute table. &lt;br&gt;Delete the record?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1373"/>
        <source>Cannot delete orphan record: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1401"/>
        <source>Cannot describe table for field </source>
        <translation>Не удаётся описать таблицу для поля </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="363"/>
        <source>Background</source>
        <translation>Фон</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="364"/>
        <source>Highlight</source>
        <translation>Подсветка</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="365"/>
        <source>Dynamic</source>
        <translation>Изменяемое</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="366"/>
        <source>Point</source>
        <translation>Точка</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="367"/>
        <source>Line</source>
        <translation>Линия</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="368"/>
        <source>Boundary (no area)</source>
        <translation>Граница (нет площади)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="369"/>
        <source>Boundary (1 area)</source>
        <translation>Граница (1 площадь)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="370"/>
        <source>Boundary (2 areas)</source>
        <translation>Граница (2 площади)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="371"/>
        <source>Centroid (in area)</source>
        <translation>Центроид (в площади)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="372"/>
        <source>Centroid (outside area)</source>
        <translation>Центроид (за площадью)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="373"/>
        <source>Centroid (duplicate in area)</source>
        <translation>Центроид (дублированный)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="374"/>
        <source>Node (1 line)</source>
        <translation>Узел (1 линия)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="375"/>
        <source>Node (2 lines)</source>
        <translation>Узел (2 линии)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="408"/>
        <source>Disp</source>
        <comment>Column title</comment>
        <translation>Видимость</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="410"/>
        <source>Color</source>
        <comment>Column title</comment>
        <translation>Цвет</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="412"/>
        <source>Type</source>
        <comment>Column title</comment>
        <translation>Тип</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="414"/>
        <source>Index</source>
        <comment>Column title</comment>
        <translation>Индекс</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="444"/>
        <source>Column</source>
        <translation>Поле</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="445"/>
        <source>Type</source>
        <translation>Тип</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="446"/>
        <source>Length</source>
        <translation>Длина</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="501"/>
        <source>Next not used</source>
        <translation>Следующая неиспользуемая</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="502"/>
        <source>Manual entry</source>
        <translation>Ручной ввод</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="503"/>
        <source>No category</source>
        <translation>Без категории</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1823"/>
        <source>Right: </source>
        <translation>Правая: </translation>
    </message>
</context>
<context>
    <name>QgsGrassEditBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="16"/>
        <source>GRASS Edit</source>
        <translation>Редактор GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="106"/>
        <source>Category</source>
        <translation>Категории</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="66"/>
        <source>Mode</source>
        <translation>Режим</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="170"/>
        <source>Settings</source>
        <translation>Параметры</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="190"/>
        <source>Snapping in screen pixels</source>
        <translation>Прилипание в пикселях экрана</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="241"/>
        <source>Symbology</source>
        <translation>Символика</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="274"/>
        <source>Column 1</source>
        <translation>Столбец 1</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="360"/>
        <source>Table</source>
        <translation>Таблица</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="496"/>
        <source>Add Column</source>
        <translation>Добавить столбец</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="511"/>
        <source>Create / Alter Table</source>
        <translation>Создать / обновить таблицу</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="298"/>
        <source>Line width</source>
        <translation>Ширина линии</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="325"/>
        <source>Marker size</source>
        <translation>Размер маркера</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="410"/>
        <source>Layer</source>
        <translation>Слой</translation>
    </message>
</context>
<context>
    <name>QgsGrassElementDialog</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="132"/>
        <source>Cancel</source>
        <translation>Отменить</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="163"/>
        <source>Ok</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="168"/>
        <source>&lt;font color=&apos;red&apos;&gt;Enter a name!&lt;/font&gt;</source>
        <translation>&lt;font color=&apos;red&apos;&gt;Введите имя!&lt;/font&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="179"/>
        <source>&lt;font color=&apos;red&apos;&gt;This is name of the source!&lt;/font&gt;</source>
        <translation>&lt;font color=&apos;red&apos;&gt;Это имя источника!&lt;/font&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="185"/>
        <source>&lt;font color=&apos;red&apos;&gt;Exists!&lt;/font&gt;</source>
        <translation>&lt;font color=&apos;red&apos;&gt;Уже существует!&lt;/font&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="186"/>
        <source>Overwrite</source>
        <translation>Перезаписать</translation>
    </message>
</context>
<context>
    <name>QgsGrassMapcalc</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="111"/>
        <source>Mapcalc tools</source>
        <translation>Инструменты Mapcalc</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="114"/>
        <source>Add map</source>
        <translation>Добавить карту</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="121"/>
        <source>Add constant value</source>
        <translation>Добавить постоянное значение</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="128"/>
        <source>Add operator or function</source>
        <translation>Добавить оператор или функцию</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="135"/>
        <source>Add connection</source>
        <translation>Добавить соединение</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="142"/>
        <source>Select item</source>
        <translation>Выбрать элемент</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="149"/>
        <source>Delete selected item</source>
        <translation>Удалить выбранный элемент</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="159"/>
        <source>Open</source>
        <translation>Открыть</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="164"/>
        <source>Save</source>
        <translation>Сохранить</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="170"/>
        <source>Save as</source>
        <translation>Сохранить как</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="178"/>
        <source>Addition</source>
        <translation>Сложение</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="179"/>
        <source>Subtraction</source>
        <translation>Вычитание</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="180"/>
        <source>Multiplication</source>
        <translation>Умножение</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="181"/>
        <source>Division</source>
        <translation>Деление</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="182"/>
        <source>Modulus</source>
        <translation>Остаток</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="183"/>
        <source>Exponentiation</source>
        <translation>Возведение в степень</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="186"/>
        <source>Equal</source>
        <translation>Равно</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="187"/>
        <source>Not equal</source>
        <translation>Не равно</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="188"/>
        <source>Greater than</source>
        <translation>Больше чем</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="189"/>
        <source>Greater than or equal</source>
        <translation>Больше или равно</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="190"/>
        <source>Less than</source>
        <translation>Меньше чем</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="191"/>
        <source>Less than or equal</source>
        <translation>Меньше или равно</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="192"/>
        <source>And</source>
        <translation>И</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="193"/>
        <source>Or</source>
        <translation>Или</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="196"/>
        <source>Absolute value of x</source>
        <translation>Абсолютное значение x</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="197"/>
        <source>Inverse tangent of x (result is in degrees)</source>
        <translation>Арктангенс x (результат в градусах)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="198"/>
        <source>Inverse tangent of y/x (result is in degrees)</source>
        <translation>Арктангенс у/x (результат в градусах)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="199"/>
        <source>Current column of moving window (starts with 1)</source>
        <translation>Текущий столбец подвижного окна (начиная с 1)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="200"/>
        <source>Cosine of x (x is in degrees)</source>
        <translation>Косинус x (x в градусах)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="201"/>
        <source>Convert x to double-precision floating point</source>
        <translation>Преобразование x в число с двойной точностью</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="202"/>
        <source>Current east-west resolution</source>
        <translation>Текущее горизонтальное разрешение</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="203"/>
        <source>Exponential function of x</source>
        <translation>Экспонента от x</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="204"/>
        <source>x to the power y</source>
        <translation>x в степени y</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="205"/>
        <source>Convert x to single-precision floating point</source>
        <translation>Преобразование x в число с одинарной точностью</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="206"/>
        <source>Decision: 1 if x not zero, 0 otherwise</source>
        <translation>Решение: 1 если x не равно нулю, иначе 0</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="207"/>
        <source>Decision: a if x not zero, 0 otherwise</source>
        <translation>Решение: a если x не равно нулю, иначе 0</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="208"/>
        <source>Decision: a if x not zero, b otherwise</source>
        <translation>Решение: a если x не равно нулю, иначе b</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="209"/>
        <source>Decision: a if x &gt; 0, b if x is zero, c if x &lt; 0</source>
        <translation>Решение: a если x &gt; 0, b если x = 0, c если x &lt; 0</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="210"/>
        <source>Convert x to integer [ truncates ]</source>
        <translation>Преобразование x в целое [отсечение]</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="211"/>
        <source>Check if x = NULL</source>
        <translation>Проверка, равен ли x значению NULL</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="212"/>
        <source>Natural log of x</source>
        <translation>Натуральный логарифм x</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="213"/>
        <source>Log of x base b</source>
        <translation>Логарифм x по основанию b</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="215"/>
        <source>Largest value</source>
        <translation>Наибольшее значение</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="217"/>
        <source>Median value</source>
        <translation>Медиана</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="219"/>
        <source>Smallest value</source>
        <translation>Наименьшее значение</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="221"/>
        <source>Mode value</source>
        <translation>Мода</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="222"/>
        <source>1 if x is zero, 0 otherwise</source>
        <translation>1, если x равен нулю, иначе 0</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="223"/>
        <source>Current north-south resolution</source>
        <translation>Текущее вертикальное разрешение</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="224"/>
        <source>NULL value</source>
        <translation>Значение NULL</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="225"/>
        <source>Random value between a and b</source>
        <translation>Случайное значение между a и b</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="226"/>
        <source>Round x to nearest integer</source>
        <translation>Округление x до ближайшего целого</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="227"/>
        <source>Current row of moving window (Starts with 1)</source>
        <translation>Текущая строка подвижного окна (начиная с 1)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="228"/>
        <source>Sine of x (x is in degrees)</source>
        <comment>sin(x)</comment>
        <translation>Синус x (x в градусах)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="229"/>
        <source>Square root of x</source>
        <comment>sqrt(x)</comment>
        <translation>Квадратный корень из x</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="230"/>
        <source>Tangent of x (x is in degrees)</source>
        <comment>tan(x)</comment>
        <translation>Тангенс x (x в градусах)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="231"/>
        <source>Current x-coordinate of moving window</source>
        <translation>Текущая x-координата подвижного окна</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="232"/>
        <source>Current y-coordinate of moving window</source>
        <translation>Текущая y-координата подвижного окна</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1317"/>
        <source>Warning</source>
        <translation>Внимание</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="584"/>
        <source>Cannot get current region</source>
        <translation>Не удалось получить регион</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="561"/>
        <source>Cannot check region of map </source>
        <translation>Не удалось проверить регион карты </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="617"/>
        <source>Cannot get region of map </source>
        <translation>Не удалось получить регион карты </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="813"/>
        <source>No GRASS raster maps currently in QGIS</source>
        <translation>Не найдено доступных для QGIS растровых карт GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1102"/>
        <source>Cannot create &apos;mapcalc&apos; directory in current mapset.</source>
        <translation>Не удалось создать каталог «mapcalc» в текущем наборе.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1112"/>
        <source>New mapcalc</source>
        <translation>Новая схема mapcalc</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1113"/>
        <source>Enter new mapcalc name:</source>
        <translation>Введите имя новой схемы mapcalc:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1118"/>
        <source>Enter vector name</source>
        <translation>Введите имя файла</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1126"/>
        <source>The file already exists. Overwrite? </source>
        <translation>Файл уже существует. Перезаписать? </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1164"/>
        <source>Save mapcalc</source>
        <translation>Сохранить схему mapcalc</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1146"/>
        <source>File name empty</source>
        <translation>Пустое имя файла</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1165"/>
        <source>Cannot open mapcalc file</source>
        <translation>Не удаётся открыть файл mapcalc</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1295"/>
        <source>The mapcalc schema (</source>
        <translation>Схема mapcalc (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1295"/>
        <source>) not found.</source>
        <translation>) не найдена.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1302"/>
        <source>Cannot open mapcalc schema (</source>
        <translation>Не удаётся открыть схему mapcalc (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1313"/>
        <source>Cannot read mapcalc schema (</source>
        <translation>Не удаётся прочесть схему mapcalc (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1314"/>
        <source>
at line </source>
        <translation>
в строке </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1315"/>
        <source> column </source>
        <translation>, столбец </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1388"/>
        <source>Output</source>
        <translation>Вывод</translation>
    </message>
</context>
<context>
    <name>QgsGrassMapcalcBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalcbase.ui" line="16"/>
        <source>MainWindow</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalcbase.ui" line="37"/>
        <source>Output</source>
        <translation>Вывод</translation>
    </message>
</context>
<context>
    <name>QgsGrassModule</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1380"/>
        <source>Run</source>
        <translation>Выполнить</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1358"/>
        <source>Stop</source>
        <translation>Остановить</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="205"/>
        <source>Module</source>
        <translation>Модуль</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1352"/>
        <source>Warning</source>
        <translation>Внимание</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="220"/>
        <source>The module file (</source>
        <translation>Файл модуля (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="220"/>
        <source>) not found.</source>
        <translation>) не найден.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="224"/>
        <source>Cannot open module file (</source>
        <translation>Не удалось открыть файл модуля (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="992"/>
        <source>)</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="987"/>
        <source>Cannot read module file (</source>
        <translation>Не удалось прочесть файл модуля (</translation>
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
в строке </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="260"/>
        <source>Module </source>
        <translation>Модуль </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="260"/>
        <source> not found</source>
        <translation> не найден</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="301"/>
        <source>Cannot find man page </source>
        <translation>Не удалось найти страницу руководства </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="981"/>
        <source>Not available, cannot open description (</source>
        <translation>Модуль недоступен, не удалось открыть описание (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="988"/>
        <source> column </source>
        <translation>, столбец </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="992"/>
        <source>Not available, incorrect description (</source>
        <translation>Модуль недоступен, неверное описание (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1179"/>
        <source>Cannot get input region</source>
        <translation>Не удаётся получить исходный регион</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1167"/>
        <source>Use Input Region</source>
        <translation>Использовать исходный регион</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1281"/>
        <source>Cannot find module </source>
        <translation>Не удалось найти модуль </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1353"/>
        <source>Cannot start module: </source>
        <translation>Не удалось запустить модуль: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1369"/>
        <source>&lt;B&gt;Successfully finished&lt;/B&gt;</source>
        <translation>&lt;B&gt;Успешное завершение&lt;/B&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1375"/>
        <source>&lt;B&gt;Finished with error&lt;/B&gt;</source>
        <translation>&lt;B&gt;Завершено с ошибкой&lt;/B&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1378"/>
        <source>&lt;B&gt;Module crashed or killed&lt;/B&gt;</source>
        <translation>&lt;B&gt;Модуль рухнул или был убит&lt;/B&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="978"/>
        <source>Not available, description not found (</source>
        <translation>Модуль недоступен, описание не найдено (</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="13"/>
        <source>GRASS Module</source>
        <translation>Модуль GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="26"/>
        <source>Options</source>
        <translation>Параметры</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="31"/>
        <source>Output</source>
        <translation>Вывод</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="47"/>
        <source>Manual</source>
        <translation>Справка</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="118"/>
        <source>Run</source>
        <translation>Выполнить</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="161"/>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="141"/>
        <source>View output</source>
        <translation>Открыть вывод</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="74"/>
        <source>TextLabel</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleField</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2714"/>
        <source>Attribute field</source>
        <translation>Поле атрибута</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleFile</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2916"/>
        <source>File</source>
        <translation>Файл</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="3029"/>
        <source>:&amp;nbsp;missing value</source>
        <translation>:&amp;nbsp;значение не задано</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="3036"/>
        <source>:&amp;nbsp;directory does not exist</source>
        <translation>:&amp;nbsp;каталог не существует</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleGdalInput</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2665"/>
        <source>Warning</source>
        <translation>Внимание</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2527"/>
        <source>Cannot find layeroption </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2669"/>
        <source>PostGIS driver in OGR does not support schemas!&lt;br&gt;Only the table name will be used.&lt;br&gt;It can result in wrong input if more tables of the same name&lt;br&gt;are present in the database.</source>
        <translation>OGR-драйвер PostGIS не поддерживает схемы!&lt;br&gt;Будет использоваться только имя таблицы.&lt;br&gt;Это может повлиять на правильность ввода,&lt;br&gt;если в базе данных есть более одной таблицы&lt;br&gt;с одинаковыми именами.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2692"/>
        <source>:&amp;nbsp;no input</source>
        <translation>:&amp;nbsp;параметр не задан</translation>
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
        <translation>Внимание</translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2071"/>
        <source>GRASS element </source>
        <translation>Элемент GRASS </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2071"/>
        <source> not supported</source>
        <translation> не поддерживается</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2095"/>
        <source>Use region of this map</source>
        <translation>Использовать регион этой карты</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2431"/>
        <source>:&amp;nbsp;no input</source>
        <translation>:&amp;nbsp;параметр не задан</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleOption</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1900"/>
        <source>:&amp;nbsp;missing value</source>
        <translation>:&amp;nbsp;значение не задано</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleSelection</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2803"/>
        <source>Attribute field</source>
        <translation>Поле атрибута</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleStandardOptions</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="886"/>
        <source>Warning</source>
        <translation>Внимание</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="369"/>
        <source>Cannot find module </source>
        <translation>Не удалось найти модуль </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="386"/>
        <source>Cannot start module </source>
        <translation>Не удалось запустить модуль </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="399"/>
        <source>Cannot read module description (</source>
        <translation>Не удалось прочесть описание модуля (</translation>
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
в строке </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="400"/>
        <source> column </source>
        <translation>, столбец </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="424"/>
        <source>Cannot find key </source>
        <translation>Не удаётся найти ключ </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="560"/>
        <source>Item with id </source>
        <translation>Элемент с ID </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="560"/>
        <source> not found</source>
        <translation> не найден</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="848"/>
        <source>Cannot get current region</source>
        <translation>Не удалось получить текущий регион</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="825"/>
        <source>Cannot check region of map </source>
        <translation>Не удалось проверить регион карты </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="887"/>
        <source>Cannot set region of map </source>
        <translation>Не удалось задать регион карты </translation>
    </message>
</context>
<context>
    <name>QgsGrassNewMapset</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="124"/>
        <source>GRASS database</source>
        <translation>База данных GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="125"/>
        <source>GRASS location</source>
        <translation>Район GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="126"/>
        <source>Projection</source>
        <translation>Проекция</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="127"/>
        <source>Default GRASS Region</source>
        <translation>Регион GRASS по умолчанию</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="184"/>
        <source>Mapset</source>
        <translation>Набор</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="129"/>
        <source>Create New Mapset</source>
        <translation>Создать новый набор</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="158"/>
        <source>Tree</source>
        <translation>Дерево</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="159"/>
        <source>Comment</source>
        <translation>Комментарий</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="160"/>
        <source>Database</source>
        <translation>База данных</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="164"/>
        <source>Location 2</source>
        <translation>Район 2</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="175"/>
        <source>User&apos;s mapset</source>
        <translation>Пользовательский набор</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="177"/>
        <source>System mapset</source>
        <translation>Системный набор</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="171"/>
        <source>Location 1</source>
        <translation>Район 1</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="185"/>
        <source>Owner</source>
        <translation>Владелец</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="233"/>
        <source>Enter path to GRASS database</source>
        <translation>Введите путь к базе данных GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="241"/>
        <source>The directory doesn&apos;t exist!</source>
        <translation>Каталог не существует!</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="271"/>
        <source>No writable locations, the database not writable!</source>
        <translation>Не найдено доступных для записи районов, база данных не изменяема!</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="369"/>
        <source>Enter location name!</source>
        <translation>Введите имя района!</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="382"/>
        <source>The location exists!</source>
        <translation>Район уже существует!</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="533"/>
        <source>Selected projection is not supported by GRASS!</source>
        <translation>Выбранная проекция не поддерживается GRASS!</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1160"/>
        <source>Warning</source>
        <translation>Внимание</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="580"/>
        <source>Cannot create projection.</source>
        <translation>Не удалось создать проекцию.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="629"/>
        <source>Cannot reproject previously set region, default region set.</source>
        <translation>Не удалось спроецировать ранее заданный регион, выбран регион по умолчанию.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="764"/>
        <source>North must be greater than south</source>
        <translation>Значение севера должно быть больше значения юга</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="769"/>
        <source>East must be greater than west</source>
        <translation>Значение востока должно быть больше значения запада</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="816"/>
        <source>Regions file (</source>
        <translation>Файл областей (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="816"/>
        <source>) not found.</source>
        <translation>) не найден.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="821"/>
        <source>Cannot open locations file (</source>
        <translation>Не удаётся открыть файл районов (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="821"/>
        <source>)</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="830"/>
        <source>Cannot read locations file (</source>
        <translation>Не удаётся прочесть файл районов (</translation>
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
в строке </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="832"/>
        <source> column </source>
        <translation>, столбец </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1161"/>
        <source>Cannot create QgsSpatialRefSys</source>
        <translation>Не удалось создать QgsSpatialRefSys</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="968"/>
        <source>Cannot reproject selected region.</source>
        <translation>Не удаётся спроецировать выбранный регион.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1057"/>
        <source>Cannot reproject region</source>
        <translation>Не удаётся спроецировать регион</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1289"/>
        <source>Enter mapset name.</source>
        <translation>Введите имя набора.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1306"/>
        <source>The mapset already exists</source>
        <translation>Этот набор уже существует</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1330"/>
        <source>Database: </source>
        <translation>База данных:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1341"/>
        <source>Location: </source>
        <translation>Район: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1343"/>
        <source>Mapset: </source>
        <translation>Набор:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1378"/>
        <source>Create location</source>
        <translation>Создать район</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1380"/>
        <source>Cannot create new location: </source>
        <translation>Не удалось создать новый район: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1427"/>
        <source>Create mapset</source>
        <translation>Создать набор</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1420"/>
        <source>Cannot open DEFAULT_WIND</source>
        <translation>Не удалось открыть DEFAULT_WIND</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1427"/>
        <source>Cannot open WIND</source>
        <translation>Не удалось открыть WIND</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1454"/>
        <source>New mapset</source>
        <translation>Новый набор</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1450"/>
        <source>New mapset successfully created, but cannot be opened: </source>
        <translation>Новый набор успешно создан, но не может быть открыт: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1456"/>
        <source>New mapset successfully created and set as current working mapset.</source>
        <translation>Новый набор успешно создан и открыт как текущий рабочий набор.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1410"/>
        <source>Cannot create new mapset directory</source>
        <translation>Не удалось создать каталог для нового набора</translation>
    </message>
</context>
<context>
    <name>QgsGrassNewMapsetBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2068"/>
        <source>Column 1</source>
        <translation>Столбец 1</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="88"/>
        <source>Example directory tree:</source>
        <translation>Пример структуры каталогов:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="95"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;GRASS data are stored in tree directory structure.&lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;The GRASS database is the top-level directory in this tree structure.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Данные GRASS хранятся в виде дерева каталогов.&lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Базой данных GRASS называется верхний каталог в этой структуре.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="399"/>
        <source>Database Error</source>
        <translation>Ошибка базы данных</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2153"/>
        <source>Database:</source>
        <translation>База данных:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="440"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="457"/>
        <source>Select existing directory or create a new one:</source>
        <translation>Выберите существующий каталог или содайте новый:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="508"/>
        <source>Location</source>
        <translation>Район</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="535"/>
        <source>Select location</source>
        <translation>Выбрать район</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="552"/>
        <source>Create new location</source>
        <translation>Создать новый район</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="832"/>
        <source>Location Error</source>
        <translation>Ошибка района</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="848"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;The GRASS location is a collection of maps for a particular territory or project.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Районом GRASS называется коллекция карт для определённой территории или проекта.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1159"/>
        <source>Projection Error</source>
        <translation>Ошибка проекции</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1174"/>
        <source>Coordinate system</source>
        <translation>Система координат</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1186"/>
        <source>Projection</source>
        <translation>Проекция</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1193"/>
        <source>Not defined</source>
        <translation>Не определена</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1273"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;The GRASS region defines a workspace for raster modules. The default region is valid for one location. It is possible to set a different region in each mapset. &lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;It is possible to change the default location region later.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Регион GRASS определяет границы для обработки растровых данных. Для каждого района существует регион по умолчанию. Регион может быть задан отдельно для каждого набора. &lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Регион по умолчанию можно изменить впоследствии.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1334"/>
        <source>Set current QGIS extent</source>
        <translation>Установить текущие границы QGIS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1376"/>
        <source>Set</source>
        <translation>Установить</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1396"/>
        <source>Region Error</source>
        <translation>Ошибка области</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1441"/>
        <source>S</source>
        <translation>Ю</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1500"/>
        <source>W</source>
        <translation>З</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1555"/>
        <source>E</source>
        <translation>В</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1614"/>
        <source>N</source>
        <translation>С</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1699"/>
        <source>New mapset:</source>
        <translation>Новый набор:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1988"/>
        <source>Mapset Error</source>
        <translation>Ошибка набора</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2045"/>
        <source>&lt;p align=&quot;center&quot;&gt;Existing masets&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;center&quot;&gt;Существующие наборы&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2101"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;The GRASS mapset is a collection of maps used by one user. &lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;A user can read maps from all mapsets in the location but &lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;he can open for writing only his mapset (owned by user).&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Набором GRASS называется коллекция карт, используемых одним пользователем. &lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Пользователь может читать карты из любых наборов в районе, но осуществлять запись он может только в свой набор (владельцем которого он является).&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2174"/>
        <source>Location:</source>
        <translation>Район: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2195"/>
        <source>Mapset:</source>
        <translation>Набор:</translation>
    </message>
</context>
<context>
    <name>QgsGrassPlugin</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="198"/>
        <source>GRASS</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="788"/>
        <source>&amp;GRASS</source>
        <translation>&amp;GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="145"/>
        <source>Open mapset</source>
        <translation>Открыть набор</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="146"/>
        <source>New mapset</source>
        <translation>Новый набор</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="147"/>
        <source>Close mapset</source>
        <translation>Закрыть набор</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="150"/>
        <source>Add GRASS vector layer</source>
        <translation>Добавить векторный слой GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="152"/>
        <source>Add GRASS raster layer</source>
        <translation>Добавить растровый слой GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="168"/>
        <source>Open GRASS tools</source>
        <translation>Открыть инструменты GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="157"/>
        <source>Display Current Grass Region</source>
        <translation>Показать текущий регион GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="161"/>
        <source>Edit Current Grass Region</source>
        <translation>Изменить текущий регион GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="163"/>
        <source>Edit Grass Vector layer</source>
        <translation>Редактировать векторный слой GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="166"/>
        <source>Adds a GRASS vector layer to the map canvas</source>
        <translation>Добавить на карту векторый слой GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="167"/>
        <source>Adds a GRASS raster layer to the map canvas</source>
        <translation>Добавить на карту растровый слой GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="169"/>
        <source>Displays the current GRASS region as a rectangle on the map canvas</source>
        <translation>Показать текущий регион GRASS в виде прямоугольника на карте</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="170"/>
        <source>Edit the current GRASS region</source>
        <translation>Изменить текущий регион GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="171"/>
        <source>Edit the currently selected GRASS vector layer.</source>
        <translation>Изменить выбранный векторный слой GRASS.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="82"/>
        <source>GrassVector</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="83"/>
        <source>0.1</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="84"/>
        <source>GRASS layer</source>
        <translation>Слой GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="164"/>
        <source>Create new Grass Vector</source>
        <translation>Создать новый векторный слой GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="758"/>
        <source>Warning</source>
        <translation>Внимание</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="470"/>
        <source>GRASS Edit is already running.</source>
        <translation>Редактор GRASS уже запущен.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="479"/>
        <source>New vector name</source>
        <translation>Имя нового слоя</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="495"/>
        <source>Cannot create new vector: </source>
        <translation>Не удалось создать новый векторный слой: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="515"/>
        <source>New vector created but cannot be opened by data provider.</source>
        <translation>Новый слой создан, но не может быть открыт поставщиком данных.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="526"/>
        <source>Cannot start editing.</source>
        <translation>Не удалось начать редактирование.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="561"/>
        <source>GISDBASE, LOCATION_NAME or MAPSET is not set, cannot display current region.</source>
        <translation>Не заданы GISDBASE, LOCATION_NAME или MAPSET, невозможно вывести текущий регион.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="571"/>
        <source>Cannot read current region: </source>
        <translation>Не удалось прочесть текущий регион: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="675"/>
        <source>Cannot open the mapset. </source>
        <translation>Не удаётся открыть набор. </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="693"/>
        <source>Cannot close mapset. </source>
        <translation>Не удаётся закрыть набор. </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="749"/>
        <source>Cannot close current mapset. </source>
        <translation>Не удаётся закрыть текущий набор. </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="758"/>
        <source>Cannot open GRASS mapset. </source>
        <translation>Не удаётся открыть набор GRASS. </translation>
    </message>
</context>
<context>
    <name>QgsGrassRegion</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregion.cpp" line="459"/>
        <source>Warning</source>
        <translation>Внимание</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregion.cpp" line="195"/>
        <source>GISDBASE, LOCATION_NAME or MAPSET is not set, cannot display current region.</source>
        <translation>Не заданы GISDBASE, LOCATION_NAME или MAPSET, невозможно вывести текущий регион.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregion.cpp" line="202"/>
        <source>Cannot read current region: </source>
        <translation>Не удаётся прочесть текущий регион: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregion.cpp" line="459"/>
        <source>Cannot write region</source>
        <translation>Не удаётся сохранить регион</translation>
    </message>
</context>
<context>
    <name>QgsGrassRegionBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="13"/>
        <source>GRASS Region Settings</source>
        <translation>Параметры региона GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="76"/>
        <source>N</source>
        <translation>С</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="146"/>
        <source>W</source>
        <translation>З</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="172"/>
        <source>E</source>
        <translation>В</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="236"/>
        <source>S</source>
        <translation>Ю</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="280"/>
        <source>N-S Res</source>
        <translation>Вертикальное разрешение</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="293"/>
        <source>Rows</source>
        <translation>Строк</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="303"/>
        <source>Cols</source>
        <translation>Столбцов</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="316"/>
        <source>E-W Res</source>
        <translation>Горизонтальное разрешение</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="364"/>
        <source>Color</source>
        <translation>Цвет</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="384"/>
        <source>Width</source>
        <translation>Ширина</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="464"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="487"/>
        <source>Cancel</source>
        <translation>Отменить</translation>
    </message>
</context>
<context>
    <name>QgsGrassSelect</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="68"/>
        <source>Select GRASS Vector Layer</source>
        <translation>Выберите векторный слой GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="75"/>
        <source>Select GRASS Raster Layer</source>
        <translation>Выберите растровый слой GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="82"/>
        <source>Select GRASS mapcalc schema</source>
        <translation>Выберите схему GRASS mapcalc</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="90"/>
        <source>Select GRASS Mapset</source>
        <translation>Выберите набор GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="409"/>
        <source>Warning</source>
        <translation>Внимание</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="409"/>
        <source>Cannot open vector on level 2 (topology not available).</source>
        <translation>Не удаётся открыть вектор уровня 2 (топология недоступна).</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="466"/>
        <source>Choose existing GISDBASE</source>
        <translation>Выберите существующую GISDBASE</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="482"/>
        <source>Wrong GISDBASE, no locations available.</source>
        <translation>Неверная GISDBASE, доступных районов не найдено.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="483"/>
        <source>Wrong GISDBASE</source>
        <translation>Неверная GISDBASE</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="500"/>
        <source>Select a map.</source>
        <translation>Выберите карту.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="501"/>
        <source>No map</source>
        <translation>Нет карты</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="509"/>
        <source>No layer</source>
        <translation>Нет слоя</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="510"/>
        <source>No layers available in this map</source>
        <translation>В этой карте нет доступных слоёв</translation>
    </message>
</context>
<context>
    <name>QgsGrassSelectBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="65"/>
        <source>Gisdbase</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="78"/>
        <source>Location</source>
        <translation>Район</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="161"/>
        <source>Browse</source>
        <translation>Обзор</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="85"/>
        <source>Mapset</source>
        <translation>Набор</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="118"/>
        <source>Map name</source>
        <translation>Имя карты</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="125"/>
        <source>Layer</source>
        <translation>Слой</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="175"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="102"/>
        <source>Select or type map name (wildcards &apos;*&apos; and &apos;?&apos; accepted for rasters)</source>
        <translation>Выберите или введите имя карты (шаблоны * и ? принимаются для растров)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="21"/>
        <source>Add GRASS Layer</source>
        <translation>Добавить слой GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="168"/>
        <source>Cancel</source>
        <translation>Отменить</translation>
    </message>
</context>
<context>
    <name>QgsGrassShellBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassshellbase.ui" line="19"/>
        <source>GRASS Shell</source>
        <translation>Оболочка GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassshellbase.ui" line="49"/>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
</context>
<context>
    <name>QgsGrassTools</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="146"/>
        <source>Modules</source>
        <translation>Модули</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="205"/>
        <source>Browser</source>
        <translation>Браузер</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="123"/>
        <source>GRASS Tools</source>
        <translation>Инструменты GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="470"/>
        <source>GRASS Tools: </source>
        <translation>Инструменты GRASS: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="358"/>
        <source>Warning</source>
        <translation>Внимание</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="271"/>
        <source>Cannot find MSYS (</source>
        <translation>Не удаётся найти MSYS (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="293"/>
        <source>GRASS Shell is not compiled.</source>
        <translation>Оболочка GRASS не была скомпилирована.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="343"/>
        <source>The config file (</source>
        <translation>Файл настроек (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="343"/>
        <source>) not found.</source>
        <translation>) не найден.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="347"/>
        <source>Cannot open config file (</source>
        <translation>Не удаётся открыть файл конфигурации (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="347"/>
        <source>)</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="355"/>
        <source>Cannot read config file (</source>
        <translation>Не удаётся прочесть файл конфигурации (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="356"/>
        <source>
at line </source>
        <translation>
в строке </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="356"/>
        <source> column </source>
        <translation>, столбец </translation>
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
        <translation>&amp;Конструктор сетки</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugin.cpp" line="94"/>
        <source>Creates a graticule (grid) and stores the result as a shapefile</source>
        <translation>Построить сетку и сохранить результат в shape-файл</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugin.cpp" line="136"/>
        <source>&amp;Graticules</source>
        <translation>Се&amp;тка</translation>
    </message>
</context>
<context>
    <name>QgsGridMakerPluginGui</name>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="51"/>
        <source>QGIS - Grid Maker</source>
        <translation>QGIS — Конструктор сетки</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="52"/>
        <source>Please enter the file name before pressing OK!</source>
        <translation>Пожалуйста, введите имя файла прежде чем нажимать OK!</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="108"/>
        <source>Choose a filename to save under</source>
        <translation>Выберите имя сохраняемого файла</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="110"/>
        <source>ESRI Shapefile (*.shp)</source>
        <translation>Shape-файлы (*.shp)</translation>
    </message>
</context>
<context>
    <name>QgsGridMakerPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="13"/>
        <source>QGIS Plugin Template</source>
        <translation type="obsolete">Шаблон модуля QGIS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="256"/>
        <source>Graticule Builder</source>
        <translation>Конструктор сетки</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Latitude:</source>
        <translation type="obsolete">Широта:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Longitude:</source>
        <translation type="obsolete">Долгота:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Latitude Interval:</source>
        <translation type="obsolete">Интервал по широте:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Longitude Interval:</source>
        <translation type="obsolete">Интервал по долготе:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:Arial; font-size:11pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;This plugin will help you to build a graticule shapefile that you can use as an overlay within your qgis map viewer.&lt;/span&gt;&lt;/p&gt;&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:Arial; font-size:10pt;&quot;&gt;Please enter all units in decimal degrees&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:Arial; font-size:11pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;Этот модуль поможет вам построить shape-файл, содержащий сетку, которую можно наложить на карту.&lt;/span&gt;&lt;/p&gt;&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:Arial; font-size:10pt;&quot;&gt;Пожалуйста, вводите все значения в десятичных градусах&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="186"/>
        <source>Type</source>
        <translation>Тип</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="198"/>
        <source>Point</source>
        <translation>Точка</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Line</source>
        <translation type="obsolete">Линия</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="208"/>
        <source>Polygon</source>
        <translation>Полигон</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="110"/>
        <source>Origin (lower left)</source>
        <translation>Начальная точка (нижняя левая)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="69"/>
        <source>End point (upper right)</source>
        <translation>Конечная точка (верхняя правая)</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Graticle size (units in degrees)</source>
        <translation type="obsolete">Размер ячейки (единицы в градусах)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="161"/>
        <source>Output (shape) file</source>
        <translation>Выходной (shape) файл</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="176"/>
        <source>Save As...</source>
        <translation>Сохранить как...</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="59"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Lucida Grande&apos;; font-size:13pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:11pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;This plugin will help you to build a graticule shapefile that you can use as an overlay within your qgis map viewer.&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:10pt;&quot;&gt;Please enter all units in decimal degrees&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Lucida Grande&apos;; font-size:13pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:11pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;Этот модуль позволяет построить shape-файл, содержащий сетку, которую впоследствии можно наложить на карту QGIS.&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:10pt;&quot;&gt;Пожалуйста, вводите все значения в десятичных градусах.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="138"/>
        <source>X</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="128"/>
        <source>Y</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="28"/>
        <source>Graticle size</source>
        <translation>Размеры сетки</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="56"/>
        <source>X Interval:</source>
        <translation>Интервал по X:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="46"/>
        <source>Y Interval:</source>
        <translation>Интервал по Y:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="13"/>
        <source>QGIS Graticule Creator</source>
        <translation>Конструктор сетки QGIS</translation>
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
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:11pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;Этот модуль поможет вам создать shape-файл, содержащий сетку и пригодный для последующего наложения на вашу карту QGIS.&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:10pt;&quot;&gt;Пожалуйста, вводите все значения в десятичных градусах.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
</context>
<context>
    <name>QgsHelpViewer</name>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="185"/>
        <source>Quantum GIS Help - </source>
        <translation>Справка Quantum GIS — </translation>
    </message>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="191"/>
        <source>Failed to get the help text from the database</source>
        <translation>Не удалось получить текст справки из базы данных</translation>
    </message>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="214"/>
        <source>Error</source>
        <translation>Ошибка</translation>
    </message>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="215"/>
        <source>The QGIS help database is not installed</source>
        <translation>База справки QGIS не установлена</translation>
    </message>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="139"/>
        <source>This help file does not exist for your language</source>
        <translation>Данный файл справки недоступен на вашем языке</translation>
    </message>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="142"/>
        <source>If you would like to create it, contact the QGIS development team</source>
        <translation>Если вы хотите создать его, свяжитесь с командой разработки QGIS</translation>
    </message>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="157"/>
        <source>Quantum GIS Help</source>
        <translation>Справка Quantum GIS</translation>
    </message>
</context>
<context>
    <name>QgsHelpViewerBase</name>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="13"/>
        <source>QGIS Help</source>
        <translation>Справка QGIS</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="39"/>
        <source>&amp;Home</source>
        <translation>&amp;Главная</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="42"/>
        <source>Alt+H</source>
        <translation>Alt+U</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="52"/>
        <source>&amp;Forward</source>
        <translation>&amp;Вперёд</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="55"/>
        <source>Alt+F</source>
        <translation>Alt+D</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="65"/>
        <source>&amp;Back</source>
        <translation>&amp;Назад</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="68"/>
        <source>Alt+B</source>
        <translation>Alt+Y</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="78"/>
        <source>&amp;Close</source>
        <translation>&amp;Закрыть</translation>
    </message>
    <message>
        <location filename="../src/ui/qgshelpviewerbase.ui" line="81"/>
        <source>Alt+C</source>
        <translation>Alt+P</translation>
    </message>
</context>
<context>
    <name>QgsHttpTransaction</name>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="234"/>
        <source>WMS Server responded unexpectedly with HTTP Status Code %1 (%2)</source>
        <translation>Неожиданный ответ WMS-сервера с HTTP-кодом %1 (%2)</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="313"/>
        <source>HTTP response completed, however there was an error: %1</source>
        <translation>HTTP-ответ получен  с ошибкой: %1</translation>
    </message>
    <message numerus="yes">
        <location filename="../src/core/qgshttptransaction.cpp" line="441"/>
        <source>Network timed out after %1 seconds of inactivity.
This may be a problem in your network connection or at the WMS server.</source>
        <translation>
            <numerusform>Соединение сброшено после %1 секунды бездействия.
Это может означать проблему с вашим соединением или с WMS-сервером.</numerusform>
            <numerusform>Соединение сброшено после %1 секунд бездействия.
Это может означать проблему с вашим соединением или с WMS-сервером.</numerusform>
            <numerusform>Соединение сброшено после %1 секунд бездействия.
Это может означать проблему с вашим соединением или с WMS-сервером.</numerusform>
        </translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="362"/>
        <source>HTTP transaction completed, however there was an error: %1</source>
        <translation>HTTP-транзакция завершена с ошибкой: %1</translation>
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
        <translation>Результат определения — </translation>
    </message>
    <message>
        <location filename="../src/app/qgsidentifyresults.cpp" line="44"/>
        <source>Feature</source>
        <translation>Объект</translation>
    </message>
    <message>
        <location filename="../src/app/qgsidentifyresults.cpp" line="45"/>
        <source>Value</source>
        <translation>Значение</translation>
    </message>
    <message>
        <location filename="../src/app/qgsidentifyresults.cpp" line="106"/>
        <source>Run action</source>
        <translation>Выполнить действие</translation>
    </message>
    <message>
        <location filename="../src/app/qgsidentifyresults.cpp" line="196"/>
        <source>(Derived)</source>
        <translation>(Выведенные)</translation>
    </message>
</context>
<context>
    <name>QgsIdentifyResultsBase</name>
    <message>
        <location filename="../src/ui/qgsidentifyresultsbase.ui" line="13"/>
        <source>Identify Results</source>
        <translation>Результат определения</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsidentifyresultsbase.ui" line="43"/>
        <source>Help</source>
        <translation>Справка</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsidentifyresultsbase.ui" line="46"/>
        <source>F1</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsidentifyresultsbase.ui" line="72"/>
        <source>Close</source>
        <translation>Закрыть</translation>
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
        <translation type="unfinished">Вывод</translation>
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
        <translation>Введите границы класса</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsludialogbase.ui" line="40"/>
        <source>Lower value</source>
        <translation>Нижнее значение</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsludialogbase.ui" line="79"/>
        <source>-</source>
        <translation>‒</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsludialogbase.ui" line="94"/>
        <source>OK</source>
        <translation type="obsolete">OK</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsludialogbase.ui" line="101"/>
        <source>Cancel</source>
        <translation type="obsolete">Отменить</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsludialogbase.ui" line="66"/>
        <source>Upper value</source>
        <translation>Верхнее значение</translation>
    </message>
</context>
<context>
    <name>QgsLabelDialogBase</name>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="19"/>
        <source>Form1</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="1148"/>
        <source>Field containing label:</source>
        <translation type="obsolete">Поле, содержащее подпись:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="1191"/>
        <source>Default label:</source>
        <translation type="obsolete">Подпись по умолчанию:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="890"/>
        <source>Preview:</source>
        <translation>Предпросмотр:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="908"/>
        <source>QGIS Rocks!</source>
        <translation>QGIS работает!</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="78"/>
        <source>Font Style</source>
        <translation type="obsolete">Стиль шрифта</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="153"/>
        <source>Font</source>
        <translation>Шрифт</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="505"/>
        <source>Points</source>
        <translation>Пункты</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="512"/>
        <source>Map units</source>
        <translation>Единицы карты</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="411"/>
        <source>%</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="776"/>
        <source>Transparency:</source>
        <translation>Прозрачность:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="534"/>
        <source>Colour</source>
        <translation type="obsolete">Цвет</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="104"/>
        <source>Position</source>
        <translation>Позиция</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="1080"/>
        <source>X Offset (pts):</source>
        <translation type="obsolete">Смещение по X (пункты):</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="1116"/>
        <source>Y Offset (pts):</source>
        <translation type="obsolete">Смещение по Y (пункты):</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="451"/>
        <source>Buffer Labels?</source>
        <translation type="obsolete">Буферизовать подписи?</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="799"/>
        <source>Size:</source>
        <translation>Размер:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="444"/>
        <source>Size is in map units</source>
        <translation>Единицы карты</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="437"/>
        <source>Size is in points</source>
        <translation>Пункты</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="285"/>
        <source>Above</source>
        <translation>Сверху</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="275"/>
        <source>Over</source>
        <translation>Поверх</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="292"/>
        <source>Left</source>
        <translation>Слева</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="268"/>
        <source>Below</source>
        <translation>Внизу</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="261"/>
        <source>Right</source>
        <translation>Справа</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="306"/>
        <source>Above Right</source>
        <translation>Сверху справа</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="254"/>
        <source>Below Right</source>
        <translation>Внизу справа</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="313"/>
        <source>Above Left</source>
        <translation>Сверху слева</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="299"/>
        <source>Below Left</source>
        <translation>Внизу слева</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="893"/>
        <source>Angle (deg):</source>
        <translation type="obsolete">Угол (град):</translation>
    </message>
    <message encoding="UTF-8">
        <location filename="../src/ui/qgslabeldialogbase.ui" line="221"/>
        <source>°</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="625"/>
        <source>Data Defined Style</source>
        <translation type="obsolete">Данные стиля</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="813"/>
        <source>&amp;Font family:</source>
        <translation type="obsolete">&amp;Шрифт:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="645"/>
        <source>&amp;Italic:</source>
        <translation type="obsolete">&amp;Курсив:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="719"/>
        <source>&amp;Underline:</source>
        <translation type="obsolete">&amp;Подчёркивание:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="681"/>
        <source>&amp;Bold:</source>
        <translation type="obsolete">&amp;Полужирный:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="663"/>
        <source>&amp;Size:</source>
        <translation type="obsolete">&amp;Размер:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="1062"/>
        <source>X Coordinate:</source>
        <translation type="obsolete">X-координата:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="1101"/>
        <source>Y Coordinate:</source>
        <translation type="obsolete">Y-координата:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="914"/>
        <source>Placement:</source>
        <translation type="obsolete">Размещение:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="323"/>
        <source>Font size units</source>
        <translation>Единицы размера шрифта</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="236"/>
        <source>Font Alignment</source>
        <translation type="obsolete">Выравнивание</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="731"/>
        <source>Placement</source>
        <translation>Размещение</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="99"/>
        <source>Buffer</source>
        <translation>Буферизация</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="431"/>
        <source>Buffer size units</source>
        <translation>Единицы размера буфера</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="499"/>
        <source>Offset units</source>
        <translation>Единицы смещения</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="847"/>
        <source>Data Defined Alignment</source>
        <translation type="obsolete">Данные выравнивания</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="922"/>
        <source>Data Defined Buffer</source>
        <translation type="obsolete">Данные буфера</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="1016"/>
        <source>Data Defined Position</source>
        <translation type="obsolete">Данные позиции</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="1136"/>
        <source>Source</source>
        <translation type="obsolete">Источник</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="767"/>
        <source>Size Units:</source>
        <translation type="obsolete">Единицы размера:</translation>
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
        <translation type="unfinished">Цвет</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="747"/>
        <source>Angle (deg)</source>
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
        <translation>Введите проекцию слоя:</translation>
    </message>
    <message>
        <location filename="../src/gui/qgslayerprojectionselector.cpp" line="35"/>
        <source>This layer appears to have no projection specification.</source>
        <translation>Этот слой не содержит сведений о проекции.</translation>
    </message>
    <message>
        <location filename="../src/gui/qgslayerprojectionselector.cpp" line="37"/>
        <source>By default, this layer will now have its projection set to that of the project, but you may override this by selecting a different projection below.</source>
        <translation>По умолчанию, для этого слоя будет выбрана проекция текущего проекта, но вы можете переопределить её, выбрав другую проекцию ниже.</translation>
    </message>
</context>
<context>
    <name>QgsLayerProjectionSelectorBase</name>
    <message>
        <location filename="../src/ui/qgslayerprojectionselectorbase.ui" line="13"/>
        <source>Layer Projection Selector</source>
        <translation>Выбор проекции слоя</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslayerprojectionselectorbase.ui" line="80"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:12pt; font-weight:600;&quot;&gt;Define this layer&apos;s projection:&lt;/span&gt;&lt;/p&gt;&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;This layer appears to have no projection specification. By default, this layer will now have its projection set to that of the project, but you may override this by selecting a different projection below.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-size:12pt; font-weight:600;&quot;&gt;Укажите проекцию данного слоя:&lt;/span&gt;&lt;/p&gt;&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Этот слой не содержит сведений о проекции. По умолчанию, для него будет назначена проекция, заданная в текущем проекте, но вы можете изменить её путём выбора иной проекции в нижеприведённом списке.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
</context>
<context>
    <name>QgsLegend</name>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="113"/>
        <source>group</source>
        <translation>группа</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Properties</source>
        <translation type="obsolete">&amp;Свойства</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="426"/>
        <source>&amp;Remove</source>
        <translation>&amp;Удалить</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="419"/>
        <source>&amp;Make to toplevel item</source>
        <translation>Сделать элементом &amp;первого уровня</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="431"/>
        <source>Re&amp;name</source>
        <translation>Переи&amp;меновать</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="436"/>
        <source>&amp;Add group</source>
        <translation>&amp;Добавить группу</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="437"/>
        <source>&amp;Expand all</source>
        <translation>&amp;Развернуть все</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="438"/>
        <source>&amp;Collapse all</source>
        <translation>&amp;Свернуть все</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="440"/>
        <source>Show file groups</source>
        <translation>Показать группы файлов</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Open attribute table</source>
        <translation type="obsolete">&amp;Открыть таблицу атрибутов</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="1827"/>
        <source>No Layer Selected</source>
        <translation>Слой не выбран</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="1828"/>
        <source>To open an attribute table, you must select a vector layer in the legend</source>
        <translation>Для открытия таблицы атрибутов, следует выбрать в легенде векторный слой</translation>
    </message>
</context>
<context>
    <name>QgsLegendLayer</name>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="490"/>
        <source>&amp;Zoom to layer extent</source>
        <translation>&amp;Увеличить до границ слоя</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="493"/>
        <source>&amp;Zoom to best scale (100%)</source>
        <translation>&amp;Увеличить до наилучшего масштаба (100%)</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="497"/>
        <source>&amp;Show in overview</source>
        <translation>&amp;Показать в обзоре</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="503"/>
        <source>&amp;Remove</source>
        <translation>&amp;Удалить</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="510"/>
        <source>&amp;Open attribute table</source>
        <translation>&amp;Открыть таблицу атрибутов</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="534"/>
        <source>Save as shapefile...</source>
        <translation>Сохранить как shape-файл...</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="541"/>
        <source>Save selection as shapefile...</source>
        <translation>Сохранить выделение как shape-файл...</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="551"/>
        <source>&amp;Properties</source>
        <translation>&amp;Свойства</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="600"/>
        <source>More layers</source>
        <translation>Дополнительные слои</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="601"/>
        <source>This item contains more layer files. Displaying more layers in table is not supported.</source>
        <translation>Этот элемент содержит более одного слоя. Вывод дополнительных слоёв в таблице невозможен.</translation>
    </message>
</context>
<context>
    <name>QgsLegendLayerFile</name>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="276"/>
        <source>Attribute table - </source>
        <translation>Таблица атрибутов — </translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="346"/>
        <source>Save layer as...</source>
        <translation>Сохранить слой как...</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="426"/>
        <source>Start editing failed</source>
        <translation>Не удалось начать редактирование</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="427"/>
        <source>Provider cannot be opened for editing</source>
        <translation>Источник не может быть открыт для редактирования</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="440"/>
        <source>Stop editing</source>
        <translation>Прекратить редактирование</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="440"/>
        <source>Do you want to save the changes?</source>
        <translation>Вы хотите сохранить изменения?</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="457"/>
        <source>Error</source>
        <translation>Ошибка</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="446"/>
        <source>Could not commit changes</source>
        <translation>Не удалось внести изменения</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="458"/>
        <source>Problems during roll back</source>
        <translation>Ошибка во время отката</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="227"/>
        <source>Not a vector layer</source>
        <translation>Слой не является векторным</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="228"/>
        <source>To open an attribute table, you must select a vector layer in the legend</source>
        <translation>Для открытия таблицы атрибутов, следует выбрать в легенде векторный слой</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="393"/>
        <source>Saving done</source>
        <translation>Сохранение выполнено</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="393"/>
        <source>Export to Shapefile has been completed</source>
        <translation>Экспорт в shape-файл завершён</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="397"/>
        <source>Driver not found</source>
        <translation>Драйвер не найден</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="397"/>
        <source>ESRI Shapefile driver is not available</source>
        <translation>Драйвер shape-файлов ESRI не доступен</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="401"/>
        <source>Error creating shapefile</source>
        <translation>Ошибка создания shape-файла</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="402"/>
        <source>The shapefile could not be created (</source>
        <translation>Не удалось создать shape-файл (</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="406"/>
        <source>Layer creation failed</source>
        <translation>Не удалось создать слой</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="501"/>
        <source>&amp;Zoom to layer extent</source>
        <translation>&amp;Увеличить до границ слоя</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="504"/>
        <source>&amp;Show in overview</source>
        <translation>&amp;Показать в обзоре</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="512"/>
        <source>&amp;Remove</source>
        <translation>&amp;Удалить</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="521"/>
        <source>&amp;Open attribute table</source>
        <translation>&amp;Открыть таблицу атрибутов</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="535"/>
        <source>Save as shapefile...</source>
        <translation>Сохранить как shape-файл...</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="537"/>
        <source>Save selection as shapefile...</source>
        <translation>Сохранить выделение как shape-файл...</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="554"/>
        <source>&amp;Properties</source>
        <translation>&amp;Свойства</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="271"/>
        <source>bad_alloc exception</source>
        <translation>Исключение bad_alloc</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="271"/>
        <source>Filling the attribute table has been stopped because there was no more virtual memory left</source>
        <translation>Заполнение таблицы атрибутов остановлено, посколько закончилась виртуальная память</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="410"/>
        <source>Layer attribute table contains unsupported datatype(s)</source>
        <translation>Таблица атрибутов слоя включает неподдерживаемые типы данных</translation>
    </message>
</context>
<context>
    <name>QgsLineStyleDialogBase</name>
    <message>
        <location filename="../src/ui/qgslinestyledialogbase.ui" line="13"/>
        <source>Select a line style</source>
        <translation type="obsolete">Выберите стиль линии</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslinestyledialogbase.ui" line="28"/>
        <source>Styles</source>
        <translation type="obsolete">Стили</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslinestyledialogbase.ui" line="177"/>
        <source>Ok</source>
        <translation type="obsolete">OK</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslinestyledialogbase.ui" line="184"/>
        <source>Cancel</source>
        <translation type="obsolete">Отменить</translation>
    </message>
</context>
<context>
    <name>QgsLineStyleWidgetBase</name>
    <message>
        <location filename="../src/ui/qgslinestylewidgetbase.ui" line="36"/>
        <source>Outline Style</source>
        <translation type="obsolete">Стиль линии</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslinestylewidgetbase.ui" line="61"/>
        <source>Width:</source>
        <translation type="obsolete">Ширина:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslinestylewidgetbase.ui" line="87"/>
        <source>Colour:</source>
        <translation type="obsolete">Цвет:</translation>
    </message>
</context>
<context>
    <name>QgsLocationCaptureWidgetBase</name>
    <message>
        <location filename="" line="0"/>
        <source>Pan</source>
        <translation type="obsolete">Перевезти</translation>
    </message>
</context>
<context>
    <name>QgsMapCanvas</name>
    <message>
        <location filename="../src/gui/qgsmapcanvas.cpp" line="1224"/>
        <source>Could not draw</source>
        <translation>Ошибка отрисовки</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsmapcanvas.cpp" line="1224"/>
        <source>because</source>
        <translation>по причине</translation>
    </message>
</context>
<context>
    <name>QgsMapLayer</name>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Zoom to extent of selected layer</source>
        <translation type="obsolete">&amp;Изменить вид до полной данного слоя</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Remove</source>
        <translation type="obsolete">&amp;Удалить</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Properties</source>
        <translation type="obsolete">&amp;Свойства</translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="537"/>
        <source> Check file permissions and retry.</source>
        <translation type="obsolete"> Проверьте права доступа к файлу и попробуйте ещё раз.</translation>
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
        <translation>Объектов не найдено</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="568"/>
        <source>&lt;p&gt;No features were found within the search radius. Note that it is currently not possible to use the identify tool on unsaved features.&lt;/p&gt;</source>
        <translation>&lt;p&gt;Не удалось определить объекты в заданном радиусе поиска. Обратите внимание, что инструмент определения пока не поддерживает несохранённые объекты.&lt;/p&gt;</translation>
    </message>
    <message numerus="yes">
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="491"/>
        <source>- %1 features found</source>
        <comment>Identify results window title</comment>
        <translation>
            <numerusform>— найден %1 объект</numerusform>
            <numerusform>— найдено %1 объекта</numerusform>
            <numerusform>— найдено %1 объектов</numerusform>
        </translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="424"/>
        <source>(clicked coordinate)</source>
        <translation>(координаты щелчка)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="223"/>
        <source>WMS identify result for %1
%2</source>
        <translation>Результат WMS-определения для %1
%2</translation>
    </message>
</context>
<context>
    <name>QgsMapToolSplitFeatures</name>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="86"/>
        <source>Split error</source>
        <translation>Ошибка разделения</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="86"/>
        <source>An error occured during feature splitting</source>
        <translation>При разделении объектов возникла ошибка</translation>
    </message>
</context>
<context>
    <name>QgsMapToolVertexEdit</name>
    <message>
        <location filename="../src/app/qgsmaptoolvertexedit.cpp" line="51"/>
        <source>Snap tolerance</source>
        <translation>Порог прилипания</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolvertexedit.cpp" line="52"/>
        <source>Don&apos;t show this message again</source>
        <translation>Не показывать это сообщение в дальнейшем</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolvertexedit.cpp" line="57"/>
        <source>Could not snap segment.</source>
        <translation>Ошибка выравнивания сегмента.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolvertexedit.cpp" line="60"/>
        <source>Have you set the tolerance in Settings &gt; Project Properties &gt; General?</source>
        <translation>Проверьте, был ли задан порог прилипания в меню Установки &gt; Свойства проекта &gt; Общие?</translation>
    </message>
</context>
<context>
    <name>QgsMapserverExport</name>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexport.cpp" line="76"/>
        <source>Name for the map file</source>
        <translation>Имя файла карты</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexport.cpp" line="84"/>
        <source>Choose the QGIS project file</source>
        <translation>Выберите файл проекта QGIS</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexport.cpp" line="85"/>
        <source>QGIS Project Files (*.qgs);;All files (*.*)</source>
        <comment>Filter list for selecting files from a dialog box</comment>
        <translation>Файлы проектов QGIS (*.qgs);;Все файлы (*.*)</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexport.cpp" line="197"/>
        <source>Overwrite File?</source>
        <translation>Перезаписать файл?</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexport.cpp" line="199"/>
        <source> exists. 
Do you want to overwrite it?</source>
        <comment>a filename is prepended to this text, and appears in a dialog box</comment>
        <translation> уже существует. 
Вы хотите перезаписать этот файл?</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Yes</source>
        <translation type="obsolete">Да</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>No</source>
        <translation type="obsolete">Нет</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmapserverexport.cpp" line="74"/>
        <source> exists. 
Do you want to overwrite it?</source>
        <translation> уже существует. Вы хотите перезаписать этот файл?</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexport.cpp" line="77"/>
        <source>MapServer map files (*.map);;All files (*.*)</source>
        <comment>Filter list for selecting files from a dialog box</comment>
        <translation>Файлы карт MapServer (*.map);;Все файлы (*.*)</translation>
    </message>
</context>
<context>
    <name>QgsMapserverExportBase</name>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="14"/>
        <source>Export to Mapserver</source>
        <translation>Экспорт в Mapserver</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="352"/>
        <source>Map file</source>
        <translation>Файл карты</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="403"/>
        <source>Export LAYER information only</source>
        <translation>Экспортировать только данные слоёв (LAYER)</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="159"/>
        <source>Map</source>
        <translation>Карта</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="330"/>
        <source>Name</source>
        <translation>Имя</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="304"/>
        <source>Height</source>
        <translation>Высота</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="291"/>
        <source>Width</source>
        <translation>Ширина</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="196"/>
        <source>dd</source>
        <translation>десятичные градусы</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="201"/>
        <source>feet</source>
        <translation>футы</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="206"/>
        <source>meters</source>
        <translation>метры</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="211"/>
        <source>miles</source>
        <translation>мили</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="216"/>
        <source>inches</source>
        <translation>дюймы</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="221"/>
        <source>kilometers</source>
        <translation>километры</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="182"/>
        <source>Units</source>
        <translation>Единицы</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="229"/>
        <source>Image type</source>
        <translation>Формат изображения</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="243"/>
        <source>gif</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="248"/>
        <source>gtiff</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="253"/>
        <source>jpeg</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="258"/>
        <source>png</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="263"/>
        <source>swf</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="268"/>
        <source>userdefined</source>
        <translation>пользовательский</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="273"/>
        <source>wbmp</source>
        <translation></translation>
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
        <translation>Префикс для карты, масштабной линейки и GIF-файла легенды, созданных для этого map-файла. Имя должно быть кратким.</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="29"/>
        <source>Web Interface Definition</source>
        <translation>Определение Web-интерфейса</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="98"/>
        <source>Header</source>
        <translation>Верхний колонтитул</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="134"/>
        <source>Footer</source>
        <translation>Нижний колонтитул</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="58"/>
        <source>Template</source>
        <translation>Шаблон</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmapserverexportbase.ui" line="56"/>
        <source>&amp;Help</source>
        <translation>&amp;Справка</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmapserverexportbase.ui" line="59"/>
        <source>F1</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmapserverexportbase.ui" line="85"/>
        <source>&amp;OK</source>
        <translation>&amp;OK</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmapserverexportbase.ui" line="101"/>
        <source>&amp;Cancel</source>
        <translation>О&amp;тменить</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmapserverexportbase.ui" line="116"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="362"/>
        <source>Name for the map file to be created from the QGIS project file</source>
        <translation>Имя map-файла, создаваемого из файла проекта QGIS</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="400"/>
        <source>If checked, only the layer information will be processed</source>
        <translation>Обрабатывать только сведения о слоях</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="68"/>
        <source>Path to the MapServer template file</source>
        <translation>Путь к файлу шаблона MapServer</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="340"/>
        <source>Prefix attached to map, scalebar and legend GIF filenames created using this MapFile</source>
        <translation>Префикс для карты, масштабной линейки и GIF-файла легенды, созданных для этого map-файла</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="376"/>
        <source>Full path to the QGIS project file to export to MapServer map format</source>
        <translation>Полный путь к файлу проекта QGIS, экспортируемому в формат MapServer</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="383"/>
        <source>QGIS project file</source>
        <translation>Файл проекта QGIS</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="369"/>
        <source>Browse...</source>
        <translation>Обзор...</translation>
    </message>
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexportbase.ui" line="393"/>
        <source>Save As...</source>
        <translation>Сохранить как...</translation>
    </message>
</context>
<context>
    <name>QgsMarkerDialogBase</name>
    <message>
        <location filename="../src/ui/qgsmarkerdialogbase.ui" line="16"/>
        <source>Choose a marker symbol</source>
        <translation type="obsolete">Выберите знак маркера</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmarkerdialogbase.ui" line="28"/>
        <source>Directory</source>
        <translation type="obsolete">Каталог</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmarkerdialogbase.ui" line="38"/>
        <source>...</source>
        <translation type="obsolete">...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmarkerdialogbase.ui" line="71"/>
        <source>Ok</source>
        <translation type="obsolete">OK</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmarkerdialogbase.ui" line="81"/>
        <source>Cancel</source>
        <translation type="obsolete">Отменить</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmarkerdialogbase.ui" line="108"/>
        <source>New Item</source>
        <translation type="obsolete">Новый элемент</translation>
    </message>
</context>
<context>
    <name>QgsMeasureBase</name>
    <message>
        <location filename="../src/ui/qgsmeasurebase.ui" line="19"/>
        <source>Measure</source>
        <translation>Измерение</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmeasurebase.ui" line="102"/>
        <source>Help</source>
        <translation>Справка</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmeasurebase.ui" line="125"/>
        <source>New</source>
        <translation>Новое</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmeasurebase.ui" line="132"/>
        <source>Cl&amp;ose</source>
        <translation>&amp;Закрыть</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmeasurebase.ui" line="66"/>
        <source>Total:</source>
        <translation>Всего:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmeasurebase.ui" line="86"/>
        <source>Segments</source>
        <translation type="unfinished">Сегменты</translation>
    </message>
</context>
<context>
    <name>QgsMeasureDialog</name>
    <message>
        <location filename="../src/app/qgsmeasuredialog.cpp" line="198"/>
        <source>Segments (in meters)</source>
        <translation>Сегменты (в метрах)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmeasuredialog.cpp" line="201"/>
        <source>Segments (in feet)</source>
        <translation>Сегменты (в футах)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmeasuredialog.cpp" line="204"/>
        <source>Segments (in degrees)</source>
        <translation>Сегменты (в градусах)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmeasuredialog.cpp" line="207"/>
        <source>Segments</source>
        <translation>Сегменты</translation>
    </message>
</context>
<context>
    <name>QgsMeasureTool</name>
    <message>
        <location filename="../src/app/qgsmeasuretool.cpp" line="74"/>
        <source>Incorrect measure results</source>
        <translation>Неверный результат измерения</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmeasuretool.cpp" line="82"/>
        <source>&lt;p&gt;This map is defined with a geographic coordinate system (latitude/longitude) but the map extents suggests that it is actually a projected coordinate system (e.g., Mercator). If so, the results from line or area measurements will be incorrect.&lt;/p&gt;&lt;p&gt;To fix this, explicitly set an appropriate map coordinate system using the &lt;tt&gt;Settings:Project Properties&lt;/tt&gt; menu.</source>
        <translation>&lt;p&gt;Эта карта задана в географической системе координат (широта/долгота), но по границам карты можно предположить, что на самом деле используется прямоугольная система координат (например, Меркатора). В этом случае, результаты измерения линий и площадей будут неверными.&lt;/p&gt;&lt;p&gt;Для устранения этой ошибки следует явно задать система координат карты в меню&lt;tt&gt;Установки:Свойства проекта&lt;/tt&gt;.</translation>
    </message>
</context>
<context>
    <name>QgsMessageViewer</name>
    <message>
        <location filename="../src/ui/qgsmessageviewer.ui" line="13"/>
        <source>QGIS Message</source>
        <translation>Сообщение QGIS</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmessageviewer.ui" line="48"/>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmessageviewer.ui" line="28"/>
        <source>Don&apos;t show this message again</source>
        <translation>Не показывать это сообщение в дальнейшем</translation>
    </message>
</context>
<context>
    <name>QgsMySQLProvider</name>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="164"/>
        <source>Unable to access relation</source>
        <translation type="obsolete">Не удаётся открыть реляцию</translation>
    </message>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="165"/>
        <source>Unable to access the </source>
        <translation type="obsolete">Не удаётся открыть реляцию </translation>
    </message>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="167"/>
        <source> relation.
The error message from the database was:
</source>
        <translation type="obsolete">.
Сообщение базы данных:
</translation>
    </message>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="184"/>
        <source>No GEOS Support!</source>
        <translation type="obsolete">Поддержка GEOS не установлена!</translation>
    </message>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="187"/>
        <source>Your PostGIS installation has no GEOS support.
Feature selection and identification will not work properly.
Please install PostGIS with GEOS support (http://geos.refractions.net)</source>
        <translation type="obsolete">Ваша версия PostGIS не поддерживает GEOS.
Выбор и определение объектов будут невозможны.
Пожалуйста, установите PostGIS с поддержкой GEOS (http://geos.refractions.net)</translation>
    </message>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="813"/>
        <source>Save layer as...</source>
        <translation type="obsolete">Сохранить слой как...</translation>
    </message>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="965"/>
        <source>Error</source>
        <translation type="obsolete">Ошибка</translation>
    </message>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="886"/>
        <source>Error creating field </source>
        <translation type="obsolete">Ошибка создания поля</translation>
    </message>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="965"/>
        <source>Layer creation failed</source>
        <translation type="obsolete">Не удалось создать слой</translation>
    </message>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="971"/>
        <source>Error creating shapefile</source>
        <translation type="obsolete">Ошибка создания shape-файла</translation>
    </message>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="973"/>
        <source>The shapefile could not be created (</source>
        <translation type="obsolete">Shape-файл не может быть создан (</translation>
    </message>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="981"/>
        <source>Driver not found</source>
        <translation type="obsolete">Драйвер не найден</translation>
    </message>
    <message>
        <location filename="../src/providers/mysql/qgsmysqlprovider.cpp" line="982"/>
        <source> driver is not available</source>
        <translation type="obsolete">-драйвер недоступен</translation>
    </message>
</context>
<context>
    <name>QgsNewConnection</name>
    <message>
        <location filename="../src/app/qgsnewconnection.cpp" line="121"/>
        <source>Test connection</source>
        <translation>Проверка соединения</translation>
    </message>
    <message>
        <location filename="../src/app/qgsnewconnection.cpp" line="121"/>
        <source>Connection failed - Check settings and try again.

Extended error information:
</source>
        <translation>Не удалось соединиться — проверьте параметры и попробуйте ещё раз.

Дополнительная информация:
</translation>
    </message>
    <message>
        <location filename="../src/app/qgsnewconnection.cpp" line="118"/>
        <source>Connection to %1 was successful</source>
        <translation>Успешное соединение с %1</translation>
    </message>
</context>
<context>
    <name>QgsNewConnectionBase</name>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="21"/>
        <source>Create a New PostGIS connection</source>
        <translation>Создать новое PostGIS-соединение</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="252"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="268"/>
        <source>Cancel</source>
        <translation>Отменить</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="284"/>
        <source>Help</source>
        <translation>Справка</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="39"/>
        <source>Connection Information</source>
        <translation>Информация о соединении</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="147"/>
        <source>Host</source>
        <translation>Узел</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="157"/>
        <source>Database</source>
        <translation>База данных</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="177"/>
        <source>Username</source>
        <translation>Имя пользователя</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="137"/>
        <source>Name</source>
        <translation>Имя</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="207"/>
        <source>Name of the new connection</source>
        <translation>Имя нового соединения</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="187"/>
        <source>Password</source>
        <translation>Пароль</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="112"/>
        <source>Test Connect</source>
        <translation>Проверить соединение</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="105"/>
        <source>Save Password</source>
        <translation>Сохранить пароль</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="287"/>
        <source>F1</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="167"/>
        <source>Port</source>
        <translation>Порт</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="220"/>
        <source>5432</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="67"/>
        <source>Only look in the &apos;public&apos; schema</source>
        <translation>Искать только в схеме «public»</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="90"/>
        <source>Only look in the geometry_columns table</source>
        <translation>Искать только в таблице geometry_columns</translation>
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
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="13"/>
        <source>Create a New WMS connection</source>
        <translation type="obsolete">Создать новое WMS-соединение</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="31"/>
        <source>Connection Information</source>
        <translation type="obsolete">Информация о соединении</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="31"/>
        <source>Name</source>
        <translation>Имя</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="60"/>
        <source>URL</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="98"/>
        <source>Proxy Host</source>
        <translation type="obsolete">Прокси-сервер</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="111"/>
        <source>Proxy Port</source>
        <translation type="obsolete">Порт</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="124"/>
        <source>Proxy User</source>
        <translation type="obsolete">Пользователь</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="137"/>
        <source>Proxy Password</source>
        <translation type="obsolete">Пароль</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="158"/>
        <source>Your user name for the HTTP proxy (optional)</source>
        <translation type="obsolete">Имя пользователя HTTP-прокси (необязательно)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="173"/>
        <source>Password for your HTTP proxy (optional)</source>
        <translation type="obsolete">Пароль HTTP-прокси (необязательно)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="50"/>
        <source>Name of the new connection</source>
        <translation>Имя нового соединения</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="73"/>
        <source>HTTP address of the Web Map Server</source>
        <translation>HTTP-адрес WMS-сервера</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="190"/>
        <source>Name of your HTTP proxy (optional)</source>
        <translation type="obsolete">Имя вашего HTTP-прокси (необязательно)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="205"/>
        <source>Port number of your HTTP proxy (optional)</source>
        <translation type="obsolete">Номер порта вашего HTTP-прокси (необязательно)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="226"/>
        <source>OK</source>
        <translation type="obsolete">OK</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="242"/>
        <source>Cancel</source>
        <translation type="obsolete">Отменить</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="258"/>
        <source>Help</source>
        <translation type="obsolete">Справка</translation>
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
        <translation>Внизу слева</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="83"/>
        <source>Top Right</source>
        <translation>Вверху справа</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="83"/>
        <source>Bottom Right</source>
        <translation>Внизу справа</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="83"/>
        <source>Top Left</source>
        <translation>Вверху слева</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="96"/>
        <source>&amp;North Arrow</source>
        <translation>Указатель «&amp;север-юг»</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="97"/>
        <source>Creates a north arrow that is displayed on the map canvas</source>
        <translation>Добавляет на карту указатель «север-юг»</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="255"/>
        <source>&amp;Decorations</source>
        <translation>&amp;Оформление</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="246"/>
        <source>North arrow pixmap not found</source>
        <translation>Не найдено изображение указателя «север-юг»</translation>
    </message>
</context>
<context>
    <name>QgsNorthArrowPluginGui</name>
    <message>
        <location filename="../src/plugins/north_arrow/plugingui.cpp" line="157"/>
        <source>Pixmap not found</source>
        <translation>Изображение не найдено</translation>
    </message>
</context>
<context>
    <name>QgsNorthArrowPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="235"/>
        <source>North Arrow Plugin</source>
        <translation>Указатель «север-юг»</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="35"/>
        <source>Properties</source>
        <translation>Свойства</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="55"/>
        <source>Angle</source>
        <translation>Угол</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="62"/>
        <source>Placement</source>
        <translation>Размещение</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="75"/>
        <source>Set direction automatically</source>
        <translation>Выбирать направление автоматически</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="85"/>
        <source>Enable North Arrow</source>
        <translation>Включить указатель «север-юг»</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="141"/>
        <source>Top Left</source>
        <translation>Вверху слева</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="146"/>
        <source>Top Right</source>
        <translation>Вверху справа</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="151"/>
        <source>Bottom Left</source>
        <translation>Внизу слева</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="156"/>
        <source>Bottom Right</source>
        <translation>Внизу справа</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="137"/>
        <source>Placement on screen</source>
        <translation>Размещение на экране</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="164"/>
        <source>Preview of north arrow</source>
        <translation>Предпросмотр указателя</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="183"/>
        <source>Icon</source>
        <translation>Значок</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="206"/>
        <source>New Item</source>
        <translation type="obsolete">Новый элемент</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="198"/>
        <source>Browse...</source>
        <translation>Обзор...</translation>
    </message>
</context>
<context>
    <name>QgsOGRFactory</name>
    <message>
        <location filename="../src/providers/ogr/qgsogrfactory.cpp" line="63"/>
        <source>Wrong Path/URI</source>
        <translation type="obsolete">Неверный путь/URI</translation>
    </message>
    <message>
        <location filename="../src/providers/ogr/qgsogrfactory.cpp" line="63"/>
        <source>The provided path for the dataset is not valid.</source>
        <translation type="obsolete">Указанный путь к набору данных недействителен.</translation>
    </message>
</context>
<context>
    <name>QgsOptions</name>
    <message>
        <location filename="../src/app/qgsoptions.cpp" line="154"/>
        <source>Detected active locale on your system: </source>
        <translation>Обнаруженный системный язык: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsoptions.cpp" line="332"/>
        <source>to vertex</source>
        <translation>к вершинам</translation>
    </message>
    <message>
        <location filename="../src/app/qgsoptions.cpp" line="336"/>
        <source>to segment</source>
        <translation>к сегментам</translation>
    </message>
    <message>
        <location filename="../src/app/qgsoptions.cpp" line="340"/>
        <source>to vertex and segment</source>
        <translation>к вершинам и сегментам</translation>
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
        <translation>Параметры QGIS</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Search Radius for Identifying Features</source>
        <translation type="obsolete">Радиус поиска для определения объектов</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="212"/>
        <source>Hide splash screen at startup</source>
        <translation>Не показывать заставку при запуске</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="93"/>
        <source>&amp;Appearance</source>
        <translation type="obsolete">&amp;Внешний вид</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="127"/>
        <source>&amp;Icon Theme</source>
        <translation type="obsolete">&amp;Тема значков</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="157"/>
        <source>Theme</source>
        <translation type="obsolete">Тема</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="188"/>
        <source>&lt;b&gt;Note: &lt;/b&gt;Theme changes take effect the next time QGIS is started</source>
        <translation>&lt;b&gt;Внимание: &lt;/b&gt;Изменение темы вступит в силу при следующем запуске QGIS</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="1026"/>
        <source>Help &amp;Browser</source>
        <translation type="obsolete">&amp;Браузер</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="1116"/>
        <source>Open help documents with</source>
        <translation type="obsolete">Открывать справочную документацию в</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="236"/>
        <source>&amp;Rendering</source>
        <translation>От&amp;рисовка</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="362"/>
        <source>Update display after reading</source>
        <translation type="obsolete">Обновлять изображение после загрузки</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="265"/>
        <source>Map display will be updated (drawn) after this many features have been read from the data source</source>
        <translation>Изображение карты будет обновлено (перерисовано) после того, как это количество объектов загружено из источника данных</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="342"/>
        <source>features</source>
        <translation type="obsolete">объектов</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="372"/>
        <source>(Set to 0 to not update the display until all features have been read)</source>
        <translation type="obsolete">(Установите в 0, чтобы отключить обновление до загрузки всех объектов)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="1067"/>
        <source>...</source>
        <translation type="obsolete">...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="382"/>
        <source>Initial Visibility</source>
        <translation type="obsolete">Начальная видимость</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="819"/>
        <source>Select Global Default ...</source>
        <translation>Выбрать глобальную проекцию по умолчанию...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="949"/>
        <source>Prompt for projection.</source>
        <translation type="obsolete">Запрашивать ввод проекции.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="956"/>
        <source>Project wide default projection will be used.</source>
        <translation type="obsolete">Использовать значение по умолчанию для данного проекта.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="105"/>
        <source>&amp;Splash screen</source>
        <translation type="obsolete">Экран-&amp;заставка</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="195"/>
        <source>Default Map Appearance (Overridden by project properties)</source>
        <translation type="obsolete">Вид карты по умолчанию (заменяется свойствами проекта)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="243"/>
        <source>Background Color:</source>
        <translation type="obsolete">Цвет фона:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="207"/>
        <source>Selection Color:</source>
        <translation type="obsolete">Цвет выделения:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="282"/>
        <source>Appearance</source>
        <translation type="obsolete">Внешний вид</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="294"/>
        <source>Capitalise layer name</source>
        <translation type="obsolete">Выводить имя слоя с заглавной буквы</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="306"/>
        <source>Make lines appear less jagged at the expense of some drawing performance</source>
        <translation>Рисовать сглаженные линии (снижает скорость отрисовки)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="248"/>
        <source>By default new la&amp;yers added to the map should be displayed</source>
        <translation>Добавляемые на карту слои &amp;видимы по умолчанию</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="330"/>
        <source>&amp;Update during drawing</source>
        <translation type="obsolete">&amp;Обновление при отрисовке</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="437"/>
        <source>Measure tool</source>
        <translation>Инструмент измерений</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="586"/>
        <source>Ellipsoid for distance calculations:</source>
        <translation type="obsolete">Эллипсоид для вычисления расстояний:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="507"/>
        <source>Search radius</source>
        <translation>Радиус поиска</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="788"/>
        <source>Pro&amp;jection</source>
        <translation>&amp;Проекция</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="829"/>
        <source>When layer is loaded that has no projection information</source>
        <translation>Если загружаемый слой не содержит данных о проекции</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="963"/>
        <source>Global default projection displa&amp;yed below will be used.</source>
        <translation type="obsolete">Использовать ниж&amp;еприведённую глобальную проекцию по умолчанию.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="1051"/>
        <source>&lt;b&gt;Note:&lt;/b&gt; The browser must be in your PATH or you can specify the full path above</source>
        <translation type="obsolete">&lt;b&gt;Внимание:&lt;/b&gt; Браузер должен находится в списке каталогов PATH, в противном случае укажите полный путь к программе</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="404"/>
        <source>Rendering</source>
        <translation type="obsolete">Отрисовка</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Selecting this will unselect the &apos;make lines less&apos; jagged toggle&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Выбор этого параметра автоматически снимает флажок «Рисовать сглаженные линии»&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="316"/>
        <source>Fix problems with incorrectly filled polygons</source>
        <translation>Исправлять ошибки заливки полигонов</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="323"/>
        <source>Continuously redraw the map when dragging the legend/map divider</source>
        <translation>Обновлять карту при перемещении разделителя карты/легенды</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="347"/>
        <source>&amp;Map tools</source>
        <translation>&amp;Инструменты</translation>
    </message>
    <message>
        <location filename="" line="0"/>
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
        <location filename="../src/ui/qgsoptionsbase.ui" line="545"/>
        <source>%</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="365"/>
        <source>Panning and zooming</source>
        <translation>Панорамирование и масштабирование</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="384"/>
        <source>Zoom</source>
        <translation>Увеличить</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="389"/>
        <source>Zoom and recenter</source>
        <translation>Увеличить и центрировать</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="399"/>
        <source>Nothing</source>
        <translation>Ничего</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="505"/>
        <source>Zoom factor:</source>
        <translation type="obsolete">Фактор увеличения:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="512"/>
        <source>Mouse wheel action:</source>
        <translation type="obsolete">Действие при прокрутке колеса мыши:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="32"/>
        <source>&amp;General</source>
        <translation>&amp;Общие</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="50"/>
        <source>General</source>
        <translation type="obsolete">Общие</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="62"/>
        <source>Ask to save project changes when required</source>
        <translation type="obsolete">Запрашивать сохранение изменений в проекте, когда это необходимо</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="576"/>
        <source>Rubberband color:</source>
        <translation type="obsolete">Цвет линии:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="872"/>
        <source>Locale</source>
        <translation>Язык</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="986"/>
        <source>Force Override System Locale</source>
        <translation type="obsolete">Переопределить язык системы</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="887"/>
        <source>Locale to use instead</source>
        <translation>Язык, используемый вместо системного</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="1014"/>
        <source>Note: Enabling / changing overide on local requires an application restart.</source>
        <translation type="obsolete">Внимание: для переопределения настройки языка необходим перезапуск приложения.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="926"/>
        <source>Additional Info</source>
        <translation>Дополнительная информация</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="932"/>
        <source>Detected active locale on your system:</source>
        <translation>Обнаруженный системный язык:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="69"/>
        <source>Warn me when opening a project file saved with an older version of QGIS</source>
        <translation type="obsolete">Предупреждать при попытке открытия файлов проекта старых версий QGIS</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="313"/>
        <source>Selecting this will unselect the &apos;make lines less&apos; jagged toggle</source>
        <translation>Активация этого параметра выключит флажок «Рисовать сглаженные линии»</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="611"/>
        <source>(Specify the search radius as a percentage of the map width)</source>
        <translation type="obsolete">(Радиус поиска задаётся в процентах от ширины видимой карты)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="621"/>
        <source>Search Radius for Identifying Features and displaying Map Tips</source>
        <translation type="obsolete">Радиус поиска для определения объектов и всплывающих описаний</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="578"/>
        <source>Digitizing</source>
        <translation>Оцифровка</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="584"/>
        <source>Rubberband</source>
        <translation>Резиновая нить</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="840"/>
        <source>Line Width:</source>
        <translation type="obsolete">Ширина линии:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="600"/>
        <source>Line width in pixels</source>
        <translation>Ширина линии в пикселях</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="873"/>
        <source>Line Colour:</source>
        <translation type="obsolete">Цвет линии:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="636"/>
        <source>Snapping</source>
        <translation>Прилипание</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="788"/>
        <source>Default Snapping Tolerance (in layer units):</source>
        <translation type="obsolete">Порог прилипания по умолчанию (в единицах слоя):</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="751"/>
        <source>Search radius for vertex edits (in layer units):</source>
        <translation type="obsolete">Радиус поиска для редактирования вершин (в единицах слоя):</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="394"/>
        <source>Zoom to mouse cursor</source>
        <translation>Увеличить в положении курсора</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="709"/>
        <source>Default Snap Mode:</source>
        <translation type="obsolete">Режим прилипания по умолчанию:</translation>
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
        <translation type="unfinished">Ширина линии</translation>
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
        <translation type="unfinished">Узел</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="974"/>
        <source>Port</source>
        <translation type="unfinished">Порт</translation>
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
        <translation type="unfinished">Пароль</translation>
    </message>
</context>
<context>
    <name>QgsPasteTransformationsBase</name>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="16"/>
        <source>Paste Transformations</source>
        <translation>Вставить преобразования</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="39"/>
        <source>&lt;b&gt;Note: This function is not useful yet!&lt;/b&gt;</source>
        <translation>&lt;b&gt;Внимание: Эта функция пока бесполезна!&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="62"/>
        <source>Source</source>
        <translation>Источник</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="83"/>
        <source>Destination</source>
        <translation>Приёмник</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="122"/>
        <source>&amp;Help</source>
        <translation>&amp;Справка</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="125"/>
        <source>F1</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="151"/>
        <source>Add New Transfer</source>
        <translation>Добавить новое преобразование</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="158"/>
        <source>&amp;OK</source>
        <translation>&amp;OK</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspastetransformationsbase.ui" line="174"/>
        <source>&amp;Cancel</source>
        <translation>О&amp;тменить</translation>
    </message>
</context>
<context>
    <name>QgsPatternDialogBase</name>
    <message>
        <location filename="../src/ui/qgspatterndialogbase.ui" line="13"/>
        <source>Select a fill pattern</source>
        <translation type="obsolete">Выберите шаблон заливки</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspatterndialogbase.ui" line="205"/>
        <source>Cancel</source>
        <translation type="obsolete">Отменить</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspatterndialogbase.ui" line="212"/>
        <source>Ok</source>
        <translation type="obsolete">OK</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspatterndialogbase.ui" line="189"/>
        <source>No Fill</source>
        <translation type="obsolete">Без заливки</translation>
    </message>
</context>
<context>
    <name>QgsPgGeoprocessing</name>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="117"/>
        <source>Buffer features in layer %1</source>
        <translation>Буферизация объектов слоя %1</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="340"/>
        <source>Error connecting to the database</source>
        <translation>Ошибка подключения к базе данных</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="78"/>
        <source>&amp;Buffer features</source>
        <translation>&amp;Буферизация объектов</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="80"/>
        <source>A new layer is created in the database with the buffered features.</source>
        <translation>В базе данных создан новый слой буферных зон.</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="410"/>
        <source>&amp;Geoprocessing</source>
        <translation>&amp;Обработка данных</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="325"/>
        <source>Unable to add geometry column</source>
        <translation>Не удалось добавить поле геометрии</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="327"/>
        <source>Unable to add geometry column to the output table </source>
        <translation>Не удалось добавить поле геометрии в выходную таблицу </translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="331"/>
        <source>Unable to create table</source>
        <translation>Не удалось создать таблицу</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="333"/>
        <source>Failed to create the output table </source>
        <translation>Не удалось создать выходную таблицу </translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="346"/>
        <source>No GEOS support</source>
        <translation>Поддержка GEOS не установлена</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="347"/>
        <source>Buffer function requires GEOS support in PostGIS</source>
        <translation>Буферизация объектов PostGIS требует поддержки GEOS</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="356"/>
        <source>No Active Layer</source>
        <translation>Нет активного слоя</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="357"/>
        <source>You must select a layer in the legend to buffer</source>
        <translation>Для буферизации необходимо выбрать слой в легенде</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="350"/>
        <source>Not a PostgreSQL/PostGIS Layer</source>
        <translation>Слой не в формате PostgreSQL/PostGIS</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="353"/>
        <source> is not a PostgreSQL/PostGIS layer.
</source>
        <translation> не является слоем PostgreSQL/PostGIS.
</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="353"/>
        <source>Geoprocessing functions are only available for PostgreSQL/PostGIS Layers</source>
        <translation>Функции обработки данных доступны только для слоёв PostgreSQL/PostGIS</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="80"/>
        <source>Create a buffer for a PostgreSQL layer. </source>
        <translation>Создание буферных зон для слоя PostgreSQL. </translation>
    </message>
</context>
<context>
    <name>QgsPgQueryBuilder</name>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="83"/>
        <source>Table &lt;b&gt;%1&lt;/b&gt; in database &lt;b&gt;%2&lt;/b&gt; on host &lt;b&gt;%3&lt;/b&gt;, user &lt;b&gt;%4&lt;/b&gt;</source>
        <translation>Таблица &lt;b&gt;%1&lt;/b&gt; в базе данных &lt;b&gt;%2&lt;/b&gt; на сервере &lt;b&gt;%3&lt;/b&gt;, пользователь &lt;b&gt;%4&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="67"/>
        <source>Connection Failed</source>
        <translation>Не удалось соединиться</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="67"/>
        <source>Connection to the database failed:</source>
        <translation>Не удалось подключиться к базу данных:</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="204"/>
        <source>Database error</source>
        <translation>Ошибка базы данных</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Failed to get sample of field values</source>
        <translation type="obsolete">Не удалось получить образец значений поля</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="279"/>
        <source>Query Result</source>
        <translation>Результат запроса</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="281"/>
        <source>The where clause returned </source>
        <translation>По условию WHERE получено </translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="281"/>
        <source> rows.</source>
        <translation> строк.</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="285"/>
        <source>Query Failed</source>
        <translation>Ошибка запроса</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="287"/>
        <source>An error occurred when executing the query:</source>
        <translation>При выполнении запроса возникла ошибка:</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="340"/>
        <source>No Records</source>
        <translation>Нет записей</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="340"/>
        <source>The query you specified results in zero records being returned. Valid PostgreSQL layers must have at least one feature.</source>
        <translation>По указанному запросу не было получено ни одной записи. Действительные слои PostgreSQL должны содержать как минимум один объект.</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="204"/>
        <source>&lt;p&gt;Failed to get sample of field values using SQL:&lt;/p&gt;&lt;p&gt;</source>
        <translation>&lt;p&gt;Ошибка получения образцов значений по SQL-запросу:&lt;/p&gt;&lt;p&gt;</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="268"/>
        <source>No Query</source>
        <translation>Нет запроса</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="268"/>
        <source>You must create a query before you can test it</source>
        <translation>Следует создать запрос, прежде чем он сможет быть проверен</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="334"/>
        <source>Error in Query</source>
        <translation>Ошибка запроса</translation>
    </message>
</context>
<context>
    <name>QgsPgQueryBuilderBase</name>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="21"/>
        <source>PostgreSQL Query Builder</source>
        <translation>Конструктор запросов PostgreSQL</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="328"/>
        <source>Clear</source>
        <translation>Очистить</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="338"/>
        <source>Test</source>
        <translation>Проверка</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="348"/>
        <source>Ok</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="358"/>
        <source>Cancel</source>
        <translation>Отменить</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="83"/>
        <source>Values</source>
        <translation>Значения</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="139"/>
        <source>All</source>
        <translation>Все</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="126"/>
        <source>Sample</source>
        <translation>Образец</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="46"/>
        <source>Fields</source>
        <translation>Поля</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="334"/>
        <source>Datasource:</source>
        <translation type="obsolete">Источник данных:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="149"/>
        <source>Operators</source>
        <translation>Операторы</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="167"/>
        <source>=</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="209"/>
        <source>IN</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="216"/>
        <source>NOT IN</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="174"/>
        <source>&lt;</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="230"/>
        <source>&gt;</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="202"/>
        <source>%</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="258"/>
        <source>&lt;=</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="251"/>
        <source>&gt;=</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="223"/>
        <source>!=</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="237"/>
        <source>LIKE</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="195"/>
        <source>AND</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="244"/>
        <source>ILIKE</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="188"/>
        <source>OR</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="181"/>
        <source>NOT</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspgquerybuilderbase.ui" line="274"/>
        <source>SQL where clause</source>
        <translation>SQL-условие WHERE</translation>
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
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Загрузить &lt;span style=&quot; font-weight:600;&quot;&gt;все&lt;/span&gt; записи векторного слоя (&lt;span style=&quot; font-style:italic;&quot;&gt;для больших таблиц эта операция может занять много времени&lt;/span&gt;)&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
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
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Загрузить &lt;span style=&quot; font-weight:600;&quot;&gt;образцы&lt;/span&gt; записей векторного слоя&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
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
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Список значений для текущего поля.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
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
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Список полей в текущем слое&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
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
        <translation>Модулей не найдено</translation>
    </message>
    <message>
        <location filename="../src/app/qgspluginmanager.cpp" line="203"/>
        <source>No QGIS plugins found in </source>
        <translation>Модули QGIS не найдены в </translation>
    </message>
    <message>
        <location filename="../src/app/qgspluginmanager.cpp" line="76"/>
        <source>Name</source>
        <translation type="obsolete">Имя</translation>
    </message>
    <message>
        <location filename="../src/app/qgspluginmanager.cpp" line="77"/>
        <source>Version</source>
        <translation type="obsolete">Версия</translation>
    </message>
    <message>
        <location filename="../src/app/qgspluginmanager.cpp" line="78"/>
        <source>Description</source>
        <translation type="obsolete">Описание</translation>
    </message>
    <message>
        <location filename="../src/app/qgspluginmanager.cpp" line="79"/>
        <source>Library name</source>
        <translation type="obsolete">Имя библиотеки</translation>
    </message>
    <message>
        <location filename="../src/app/qgspluginmanager.cpp" line="84"/>
        <source>&amp;Select All</source>
        <translation type="unfinished">В&amp;ыбрать все</translation>
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
        <location filename="" line="0"/>
        <source>QGIS Plugin Manger</source>
        <translation type="obsolete">Управление модулями QGIS</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Name</source>
        <translation type="obsolete">Имя</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Description</source>
        <translation type="obsolete">Описание</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Library Name</source>
        <translation type="obsolete">Имя библиотеки</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Ok</source>
        <translation type="obsolete">Да</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="39"/>
        <source>Plugin Directory</source>
        <translation type="obsolete">Каталог модулей</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="61"/>
        <source>To load a plugin, click the checkbox next to the plugin and click Ok</source>
        <translation type="obsolete">Для загрузки модуля, активируйте сооветствующий ему флажок и нажмите OK</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Version</source>
        <translation type="obsolete">Версия</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="92"/>
        <source>&amp;Select All</source>
        <translation type="obsolete">В&amp;ыбрать все</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="102"/>
        <source>C&amp;lear All</source>
        <translation type="obsolete">&amp;Отключить все</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="105"/>
        <source>Alt+L</source>
        <translation type="obsolete">Alt+J</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="112"/>
        <source>&amp;Ok</source>
        <translation type="obsolete">&amp;OK</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="122"/>
        <source>&amp;Close</source>
        <translation type="obsolete">&amp;Закрыть</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="16"/>
        <source>QGIS Plugin Manager</source>
        <translation>Менеджер модулей QGIS</translation>
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
        <translation type="unfinished">Каталог</translation>
    </message>
</context>
<context>
    <name>QgsPointDialog</name>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="488"/>
        <source>Zoom In</source>
        <translation>Увеличить</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="487"/>
        <source>z</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="493"/>
        <source>Zoom Out</source>
        <translation>Уменьшить</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="492"/>
        <source>Z</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="496"/>
        <source>Zoom To Layer</source>
        <translation>Увеличить до слоя</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="498"/>
        <source>Zoom to Layer</source>
        <translation>Увеличить до слоя</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="501"/>
        <source>Pan Map</source>
        <translation>Панорамировать карту</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="502"/>
        <source>Pan the map</source>
        <translation>Панорамировать карту</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="505"/>
        <source>Add Point</source>
        <translation>Добавить точку</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="506"/>
        <source>.</source>
        <translation>.</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="507"/>
        <source>Capture Points</source>
        <translation>Захватить точки</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="510"/>
        <source>Delete Point</source>
        <translation>Удалить точку</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="511"/>
        <source>Delete Selected</source>
        <translation>Удалить выбранное</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="559"/>
        <source>Linear</source>
        <translation>Линейное</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="560"/>
        <source>Helmert</source>
        <translation>Гельмерта</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="198"/>
        <source>Choose a name for the world file</source>
        <translation>Выберите имя файла привязки</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="216"/>
        <source>-modified</source>
        <comment>Georeferencer:QgsPointDialog.cpp - used to modify a user given filename</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="265"/>
        <source>Warning</source>
        <translation>Внимание</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="280"/>
        <source>Affine</source>
        <translation>Аффинное</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="290"/>
        <source>Not implemented!</source>
        <translation>Функция не реализована!</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="285"/>
        <source>&lt;p&gt;An affine transform requires changing the original raster file. This is not yet supported.&lt;/p&gt;</source>
        <translation>&lt;p&gt;Аффинное преобразование требует изменения оригинального растрового файла. В данный момент, эта функция не поддерживается.&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="292"/>
        <source>&lt;p&gt;The </source>
        <translation>&lt;p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="293"/>
        <source> transform is not yet supported.&lt;/p&gt;</source>
        <translation> преобразование в настоящий момент не поддерживается.&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="324"/>
        <source>Error</source>
        <translation>Ошибка</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="325"/>
        <source>Could not write to </source>
        <translation>Не удалось сохранить в </translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="272"/>
        <source>Currently all modified files will be written in TIFF format.</source>
        <translation>В данный момент все изменённые файлы сохраняются в формате TIFF.</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="271"/>
        <source>&lt;p&gt;A Helmert transform requires modifications in the raster layer.&lt;/p&gt;&lt;p&gt;The modified raster will be saved in a new file and a world file will be generated for this new file instead.&lt;/p&gt;&lt;p&gt;Are you sure that this is what you want?&lt;/p&gt;</source>
        <translation>&lt;p&gt;Преобразование Гельмерта требует изменения исходного слоя.&lt;/p&gt;&lt;p&gt;Изменённый растр будет сохранён в новом файле и файл привязки будет создан уже для нового файла.&lt;/p&gt;&lt;p&gt;Вы уверены, что хотите продолжить?&lt;/p&gt;</translation>
    </message>
</context>
<context>
    <name>QgsPointDialogBase</name>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="65"/>
        <source>Transform type:</source>
        <translation>Тип преобразования:</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="178"/>
        <source>Zoom in</source>
        <translation>Увеличить</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="200"/>
        <source>Zoom out</source>
        <translation>Уменьшить</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="222"/>
        <source>Zoom to the raster extents</source>
        <translation>Увеличить до границ растра</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="244"/>
        <source>Pan</source>
        <translation>Панорамировать</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="105"/>
        <source>Add points</source>
        <translation>Добавить точки</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="130"/>
        <source>Delete points</source>
        <translation>Удалить точки</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="52"/>
        <source>World file:</source>
        <translation>Файл привязки:</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="45"/>
        <source>Modified raster:</source>
        <translation>Изменённый растр:</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="13"/>
        <source>Reference points</source>
        <translation>Опорные точки</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="38"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="75"/>
        <source>Create</source>
        <translation>Создать</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialogbase.ui" line="282"/>
        <source>Create and load layer</source>
        <translation>Создать и загрузить слой</translation>
    </message>
</context>
<context>
    <name>QgsPointStyleWidgetBase</name>
    <message>
        <location filename="../src/ui/qgspointstylewidgetbase.ui" line="36"/>
        <source>Symbol Style</source>
        <translation type="obsolete">Стиль знака</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspointstylewidgetbase.ui" line="51"/>
        <source>Scale</source>
        <translation type="obsolete">Масштаб </translation>
    </message>
</context>
<context>
    <name>QgsPostgresProvider</name>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="141"/>
        <source>Unable to access relation</source>
        <translation>Не удаётся открыть реляцию</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="121"/>
        <source>Unable to access the </source>
        <translation>Не удаётся открыть реляцию </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="144"/>
        <source> relation.
The error message from the database was:
</source>
        <translation>.Сообщение базы данных:</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="105"/>
        <source>No GEOS Support!</source>
        <translation>Поддержка GEOS не установлена!</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="109"/>
        <source>Your PostGIS installation has no GEOS support.
Feature selection and identification will not work properly.
Please install PostGIS with GEOS support (http://geos.refractions.net)</source>
        <translation>Ваша версия PostGIS не поддерживает GEOS.
Выбор и определение объектов будут невозможны.
Пожалуйста, установите PostGIS с поддержкой GEOS (http://geos.refractions.net)</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="907"/>
        <source>No suitable key column in table</source>
        <translation>В таблице нет подходящего ключевого поля</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="911"/>
        <source>The table has no column suitable for use as a key.

Qgis requires that the table either has a column of type
int4 with a unique constraint on it (which includes the
primary key) or has a PostgreSQL oid column.
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="952"/>
        <source>The unique index on column</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="954"/>
        <source>is unsuitable because Qgis does not currently support non-int4 type columns as a key into the table.
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="977"/>
        <source>and </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="983"/>
        <source>The unique index based on columns </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="985"/>
        <source> is unsuitable because Qgis does not currently support multiple columns as a key into the table.
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1028"/>
        <source>Unable to find a key column</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1108"/>
        <source> derives from </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1112"/>
        <source>and is suitable.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1116"/>
        <source>and is not suitable </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1117"/>
        <source>type is </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1119"/>
        <source> and has a suitable constraint)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1121"/>
        <source> and does not have a suitable constraint)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1221"/>
        <source>The view you selected has the following columns, none of which satisfy the above conditions:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1227"/>
        <source>Qgis requires that the view has a column that can be used as a unique key. Such a column should be derived from a table column of type int4 and be a primary key, have a unique constraint on it, or be a PostgreSQL oid column. To improve performance the column should also be indexed.
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1228"/>
        <source>The view </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1229"/>
        <source>has no column suitable for use as a unique key.
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1230"/>
        <source>No suitable key column in view</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2530"/>
        <source>Unknown geometry type</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2531"/>
        <source>Column </source>
        <translation>Поле </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2541"/>
        <source> in </source>
        <translation> в </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2533"/>
        <source> has a geometry type of </source>
        <translation> имеет тип геометрии </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2533"/>
        <source>, which Qgis does not currently support.</source>
        <translation>, который QGIS не поддерживает в данный момент.</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2542"/>
        <source>. The database communication log was:
</source>
        <translation>. История операций с базой данных:
</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2543"/>
        <source>Unable to get feature type and srid</source>
        <translation>Не удалось получить тип объекта и SRID</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1207"/>
        <source>Note: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1209"/>
        <source>initially appeared suitable but does not contain unique data, so is not suitable.
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1817"/>
        <source>INSERT error</source>
        <translation type="obsolete">Ошибка выполнения INSERT</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1811"/>
        <source>An error occured during feature insertion</source>
        <translation type="obsolete">При вставке объекта возникла ошибка</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1870"/>
        <source>DELETE error</source>
        <translation type="obsolete">Ошибка выполнения DELETE</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1864"/>
        <source>An error occured during deletion from disk</source>
        <translation type="obsolete">При удалении с диска возникла ошибка</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2184"/>
        <source>PostGIS error</source>
        <translation type="obsolete">Ошибка PostGIS</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2186"/>
        <source>When trying: </source>
        <translation type="obsolete">При попытке: </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2178"/>
        <source>An error occured contacting the PostgreSQL database</source>
        <translation type="obsolete">Обнаружена ошибка при попытке соединения с базой PostgreSQL</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2185"/>
        <source>The PostgreSQL database returned: </source>
        <translation type="obsolete">Сообщение базы данных PostgreSQL: </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2540"/>
        <source>Qgis was unable to determine the type and srid of column </source>
        <translation>Не удалось определить тип и SRID поля </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="142"/>
        <source>Unable to determine table access privileges for the </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1869"/>
        <source>Error while adding features</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1903"/>
        <source>Error while deleting features</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1935"/>
        <source>Error while adding attributes</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1974"/>
        <source>Error while deleting attributes</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2038"/>
        <source>Error while changing attributes</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2117"/>
        <source>Error while changing geometry values</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.h" line="495"/>
        <source>unexpected PostgreSQL error</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsProjectPropertiesBase</name>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="13"/>
        <source>Project Properties</source>
        <translation>Свойства проекта</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="301"/>
        <source>Map Units</source>
        <translation type="obsolete">Картографические единицы</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="157"/>
        <source>Meters</source>
        <translation>Метры</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="167"/>
        <source>Feet</source>
        <translation>Футы</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="174"/>
        <source>Decimal degrees</source>
        <translation>Десятичные градусы</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Help</source>
        <translation type="obsolete">&amp;Помощь</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="54"/>
        <source>Default project title</source>
        <translation>Заглавие проекта по умолчанию</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="32"/>
        <source>General</source>
        <translation>Общие</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Line Width:</source>
        <translation type="obsolete">Ширина линии:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Line Colour:</source>
        <translation type="obsolete">Цвет линии:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="196"/>
        <source>Automatic</source>
        <translation>Автоматически</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="190"/>
        <source>Automatically sets the number of decimal places in the mouse position display</source>
        <translation>Автоматически устанавливать число десятичных знаков в поле вывода позиции курсора мыши</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="193"/>
        <source>The number of decimal places that are used when displaying the mouse position is automatically set to be enough so that moving the mouse by one pixel gives a change in the position display</source>
        <translation>Количество используемых десятичных знаков в значении позиции курсора выбирается автоматически таким образом, что перемещение мыши на один пиксель вызовет изменение в поле отображения позиции</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="212"/>
        <source>Manual</source>
        <translation>Вручную</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="209"/>
        <source>Sets the number of decimal places to use for the mouse position display</source>
        <translation>Число десятичных знаков в поле вывода позиции курсора мыши</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="222"/>
        <source>The number of decimal places for the manual option</source>
        <translation>Количество десятичных знаков для параметра «Вручную»</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="235"/>
        <source>decimal places</source>
        <translation>десятичных знаков</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="157"/>
        <source>Map Appearance</source>
        <translation type="obsolete">Вид карты</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="169"/>
        <source>Selection Color:</source>
        <translation type="obsolete">Цвет выделения:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="50"/>
        <source>Project Title</source>
        <translation type="obsolete">Заглавие проекта</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="282"/>
        <source>Projection</source>
        <translation>Проекция</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="300"/>
        <source>Enable on the fly projection</source>
        <translation>Включить преобразование проекции «на лету»</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="208"/>
        <source>Background Color:</source>
        <translation type="obsolete">Цвет фона:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="184"/>
        <source>Precision</source>
        <translation>Точность</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="251"/>
        <source>Digitizing</source>
        <translation>Оцифровка</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="51"/>
        <source>Descriptive project name</source>
        <translation>Описательное заглавие проекта</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Line width in pixels</source>
        <translation type="obsolete">Ширина линии в пикселях</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Snapping tolerance in map units</source>
        <translation type="obsolete">Порог выравнивания в единицах карты</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Snapping Tolerance (in map units):</source>
        <translation type="obsolete">Порог выравнивания (в единицах карты):</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="257"/>
        <source>Enable topological editing</source>
        <translation>Включить топологическое редактирование</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="271"/>
        <source>Snapping options...</source>
        <translation>Параметры прилипания...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="264"/>
        <source>Avoid intersections of new polygons</source>
        <translation>Предотвращать пересечение новых полигонов</translation>
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
        <translation type="unfinished">Единицы карты</translation>
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
        <translation>Выбор проекции</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="52"/>
        <source>Projection</source>
        <translation>Проекция</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="82"/>
        <source>Search</source>
        <translation>Поиск</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="112"/>
        <source>Find</source>
        <translation>Найти</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="164"/>
        <source>Postgis SRID</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="151"/>
        <source>EPSG ID</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="138"/>
        <source>QGIS SRSID</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="125"/>
        <source>Name</source>
        <translation>Имя</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="221"/>
        <source>Spatial Reference System</source>
        <translation>Система координат</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="226"/>
        <source>Id</source>
        <translation>ID</translation>
    </message>
</context>
<context>
    <name>QgsPythonDialog</name>
    <message>
        <location filename="../src/ui/qgspythondialog.ui" line="13"/>
        <source>Python console</source>
        <translation>Консоль Python</translation>
    </message>
    <message>
        <location filename="" line="0"/>
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
        <location filename="../src/ui/qgspythondialog.ui" line="58"/>
        <source>&gt;&gt;&gt;</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgspythondialog.ui" line="33"/>
        <source>To access Quantum GIS environment from this python console use object from global scope which is an instance of QgisInterface class.&lt;br&gt;Usage e.g.: iface.zoomFull()</source>
        <translation>Для доступа к окружению Quantum GIS из консоли Python, используйте объект из глобального пространства имен, который является экземпляром класса QgisInterface.&lt;br&gt;Например: iface.zoomFull()</translation>
    </message>
</context>
<context>
    <name>QgsQuickPrint</name>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="845"/>
        <source> km</source>
        <translation> км</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="850"/>
        <source> mm</source>
        <translation> мм</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="855"/>
        <source> cm</source>
        <translation> см</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="859"/>
        <source> m</source>
        <translation> м</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="864"/>
        <source> miles</source>
        <translation> миль</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="869"/>
        <source> mile</source>
        <translation> миля</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="874"/>
        <source> inches</source>
        <translation>дюймов</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="879"/>
        <source> foot</source>
        <translation> фут</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="883"/>
        <source> feet</source>
        <translation> футов</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="888"/>
        <source> degree</source>
        <translation> градус</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="890"/>
        <source> degrees</source>
        <translation> градусов</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="893"/>
        <source> unknown</source>
        <translation> неизв</translation>
    </message>
</context>
<context>
    <name>QgsRasterLayer</name>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.h" line="513"/>
        <source>Not Set</source>
        <translation>Не задано</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Zoom to extent of selected layer</source>
        <translation type="obsolete">&amp;Изменить вид до полной данного слоя</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Properties</source>
        <translation type="obsolete">&amp;Свойства</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Remove</source>
        <translation type="obsolete">&amp;Удалить</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3425"/>
        <source>Driver:</source>
        <translation>Драйвер:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3502"/>
        <source>Dimensions:</source>
        <translation>Размеры:</translation>
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
        <translation> Каналы: </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3621"/>
        <source>Origin:</source>
        <translation>Базис:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3630"/>
        <source>Pixel Size:</source>
        <translation>Размер пикселя:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2202"/>
        <source>Raster Extent: </source>
        <translation>Границы растра: </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2205"/>
        <source>Clipped area: </source>
        <translation>Вырезанная область: </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3572"/>
        <source>Pyramid overviews:</source>
        <translation>Обзор пирамид:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3722"/>
        <source>Property</source>
        <translation type="obsolete">Свойство</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3725"/>
        <source>Value</source>
        <translation type="obsolete">Значение</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4793"/>
        <source>Band</source>
        <translation>Канал</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3655"/>
        <source>Band No</source>
        <translation>Канал №</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3667"/>
        <source>No Stats</source>
        <translation>Нет статистики</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3670"/>
        <source>No stats collected yet</source>
        <translation>Сбор статистики не производился</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3680"/>
        <source>Min Val</source>
        <translation>Мин. значение</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3688"/>
        <source>Max Val</source>
        <translation>Макс. значение</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3696"/>
        <source>Range</source>
        <translation>Диапазон</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3704"/>
        <source>Mean</source>
        <translation>Среднее</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3712"/>
        <source>Sum of squares</source>
        <translation>Сумма квадратов</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3720"/>
        <source>Standard Deviation</source>
        <translation>Стандартное отклонение</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3728"/>
        <source>Sum of all cells</source>
        <translation>Сумма всех ячеек</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3736"/>
        <source>Cell Count</source>
        <translation>Количество ячеек</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3528"/>
        <source>Data Type:</source>
        <translation>Тип данных:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3534"/>
        <source>GDT_Byte - Eight bit unsigned integer</source>
        <translation>GDT_Byte — 8-битное беззнаковое целое</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3537"/>
        <source>GDT_UInt16 - Sixteen bit unsigned integer </source>
        <translation>GDT_UInt16 — 16-битное беззнаковое целое </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3540"/>
        <source>GDT_Int16 - Sixteen bit signed integer </source>
        <translation>GDT_Int16 — 16-битное целое со знаком</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3543"/>
        <source>GDT_UInt32 - Thirty two bit unsigned integer </source>
        <translation>GDT_UInt32 — 32-битное беззнаковое целое </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3546"/>
        <source>GDT_Int32 - Thirty two bit signed integer </source>
        <translation>GDT_Int32 — 32-битное целое со знаком </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3549"/>
        <source>GDT_Float32 - Thirty two bit floating point </source>
        <translation>GDT_Float32 — 32-битное с плавающей точкой</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3552"/>
        <source>GDT_Float64 - Sixty four bit floating point </source>
        <translation>GDT_Float64 — 64-битное с плавающей точкой </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3555"/>
        <source>GDT_CInt16 - Complex Int16 </source>
        <translation>GDT_CInt16 — Комплексное Int16 </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3558"/>
        <source>GDT_CInt32 - Complex Int32 </source>
        <translation>GDT_CInt32 — Комплексное Int32 </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3561"/>
        <source>GDT_CFloat32 - Complex Float32 </source>
        <translation>GDT_CFloat32 — Комплексное Float32 </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3564"/>
        <source>GDT_CFloat64 - Complex Float64 </source>
        <translation>GDT_CFloat64 — Копмлексное Float64 </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3567"/>
        <source>Could not determine raster data type.</source>
        <translation>Не удалось установить тип растровых данных.</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3822"/>
        <source>Average Magphase</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3827"/>
        <source>Average</source>
        <translation>Среднее</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3593"/>
        <source>Layer Spatial Reference System: </source>
        <translation>Система координат слоя: </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4749"/>
        <source>out of extent</source>
        <translation>за границами</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="4787"/>
        <source>null (no data)</source>
        <translation>null (нет данных)</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3450"/>
        <source>Dataset Description</source>
        <translation>Описание набора данных</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3513"/>
        <source>No Data Value</source>
        <translation>Значение «нет данных»</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="242"/>
        <source>and all other files</source>
        <translation>и прочие файлы</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3522"/>
        <source>NoDataValue not set</source>
        <translation>Значение «нет данных» не задано</translation>
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
        <location filename="" line="0"/>
        <source>&lt;h3&gt;Multiband Image Notes&lt;/h3&gt;&lt;p&gt;This is a multiband image. You can choose to render it as grayscale or color (RGB). For color images, you can associate bands to colors arbitarily. For example, if you have a seven band landsat image, you may choose to render it as:&lt;/p&gt;&lt;ul&gt;&lt;li&gt;Visible Blue (0.45 to 0.52 microns) - not mapped&lt;/li&gt;&lt;li&gt;Visible Green (0.52 to 0.60 microns) - not mapped&lt;/li&gt;&lt;/li&gt;Visible Red (0.63 to 0.69 microns) - mapped to red in image&lt;/li&gt;&lt;li&gt;Near Infrared (0.76 to 0.90 microns) - mapped to green in image&lt;/li&gt;&lt;li&gt;Mid Infrared (1.55 to 1.75 microns) - not mapped&lt;/li&gt;&lt;li&gt;Thermal Infrared (10.4 to 12.5 microns) - not mapped&lt;/li&gt;&lt;li&gt;Mid Infrared (2.08 to 2.35 microns) - mapped to blue in image&lt;/li&gt;&lt;/ul&gt;</source>
        <translation type="obsolete">&lt;h3&gt;Примечания для многоканального изображения&lt;/h3&gt;&lt;p&gt;Это изображение является многоканальным. Для его отрисовки вы можете выбрать как цветной режим (RGB) так и градации серого. Для цветных изображений допускается произвольное присвоение каналам цветов изображения. К примеру, семиканальное изображение Landsat можно вывести следующим образом:&lt;/p&gt;&lt;ul&gt;&lt;li&gt;Видимый синий (0.45‒0.52 мкм) ‒ не назначено&lt;/li&gt;&lt;li&gt;Видимый зелёный (0.52‒0.60 мкм) ‒ не назначено&lt;/li&gt;&lt;li&gt;Видимый красный (0.63‒0.69 мкм) ‒ красный в изображении&lt;/li&gt;&lt;li&gt;Ближний инфракрасный (0.76‒0.90 мкм) ‒ зелёный в изображении&lt;/li&gt;&lt;li&gt;Средний инфракрасный (1.55‒1.75 мкм) ‒ не назначено&lt;/li&gt;&lt;li&gt;Тепловой инфракрасный (10.4‒12.5 мкм) ‒ не назначено&lt;/li&gt;&lt;li&gt;Средний инфракрасный (2.08‒2.35 мкм) - синий в изображении&lt;/li&gt;&lt;/ul&gt;</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1032"/>
        <source>Grayscale</source>
        <translation>Градации серого</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2412"/>
        <source>Pseudocolor</source>
        <translation>Псевдоцвет</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2412"/>
        <source>Freak Out</source>
        <translation>Кислотная</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="177"/>
        <source>Palette</source>
        <translation>Палитра</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="56"/>
        <source>Not Set</source>
        <translation type="unfinished">Не задано</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="825"/>
        <source>Columns: </source>
        <translation>Столбцов: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="826"/>
        <source>Rows: </source>
        <translation>Строк: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="827"/>
        <source>No-Data Value: </source>
        <translation>Значение «нет данных»: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="827"/>
        <source>n/a</source>
        <translation>н/д</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1849"/>
        <source>Write access denied</source>
        <translation>Закрыт доступ на запись</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1849"/>
        <source>Write access denied. Adjust the file permissions and try again.

</source>
        <translation>Закрыт доступ на запись. Исправьте права доступа к файлу и попробуйте ещё раз.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1594"/>
        <source>Building pyramids failed.</source>
        <translation>Не удалось построить пирамиды.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1590"/>
        <source>The file was not writeable. Some formats can not be written to, only read. You can also try to check the permissions and then try again.</source>
        <translation>Запись в файл невозможна. Некоторые форматы имеют ограничение на запись и доступны только для чтения. Кроме того, проверьте права доступа к файлу и попробуйте ещё раз.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1595"/>
        <source>Building pyramid overviews is not supported on this type of raster.</source>
        <translation>Построение пирамид не поддерживается для данного типа растра.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1714"/>
        <source>Custom Colormap</source>
        <translation>Пользовательская</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2777"/>
        <source>No Stretch</source>
        <translation>Без растяжения</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2782"/>
        <source>Stretch To MinMax</source>
        <translation>Растяжение до мин/макс</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2787"/>
        <source>Stretch And Clip To MinMax</source>
        <translation>Растяжение и отсечение по мин/макс</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2792"/>
        <source>Clip To MinMax</source>
        <translation>Отсечение по мин/макс</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1427"/>
        <source>Discrete</source>
        <translation>Дискретная</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="907"/>
        <source>Linearly</source>
        <translation>Линейная</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2622"/>
        <source>Equal interval</source>
        <translation>Равные интервалы</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2644"/>
        <source>Quantiles</source>
        <translation>Квантили</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="332"/>
        <source>Description</source>
        <translation>Описание</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="333"/>
        <source>Large resolution raster layers can slow navigation in QGIS.</source>
        <translation>Растры выского разрешения могут замедлить навигацию в QGIS.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="334"/>
        <source>By creating lower resolution copies of the data (pyramids) performance can be considerably improved as QGIS selects the most suitable resolution to use depending on the level of zoom.</source>
        <translation>Создание копий данных низкого разрешения (пирамид) позволяет существенно повысить скорость, поскольку QGIS будет автоматически выбирать оптимальное разрешение в зависимости от текущего масштаба.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="335"/>
        <source>You must have write access in the directory where the original data is stored to build pyramids.</source>
        <translation>Для сохранения пирамид необходимы права на запись в каталог, в котором хранятся оригинальные данные.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="336"/>
        <source>Please note that building pyramids may alter the original data file and once created they cannot be removed!</source>
        <translation>Помните, что при построении пирамид в оригинальные файлы данных могут быть внесены изменения, и пирамиды нельзя удалить после создания!</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="337"/>
        <source>Please note that building pyramids could corrupt your image - always make a backup of your data first!</source>
        <translation>Помните, что при построении пирамид ваши изображения могут быть повреждены — создавайте резервные копии данных!</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1821"/>
        <source>Red</source>
        <translation>Красный</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1821"/>
        <source>Green</source>
        <translation>Зелёный</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1821"/>
        <source>Blue</source>
        <translation>Синий</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1838"/>
        <source>Percent Transparent</source>
        <translation>Процент прозрачности</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1834"/>
        <source>Gray</source>
        <translation>Серый</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1838"/>
        <source>Indexed Value</source>
        <translation>Индексированное значение</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2774"/>
        <source>User Defined</source>
        <translation>Пользовательское</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="713"/>
        <source>No Scaling</source>
        <translation type="obsolete">Без масштабирования</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="819"/>
        <source>No-Data Value: Not Set</source>
        <translation>Значение «нет данных»: не задано</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1806"/>
        <source>Save file</source>
        <translation>Сохранить файл</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2308"/>
        <source>Textfile (*.txt)</source>
        <translation>Текстовые файлы (*.txt)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1818"/>
        <source>QGIS Generated Transparent Pixel Value Export File</source>
        <translation>Файл экспорта значений прозрачности пикселей, созданный QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2308"/>
        <source>Open file</source>
        <translation>Открыть файл</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2381"/>
        <source>Import Error</source>
        <translation>Ошибка импорта</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2381"/>
        <source>The following lines contained errors

</source>
        <translation>Следующие строки содержат ошибки

</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2386"/>
        <source>Read access denied</source>
        <translation>Закрыт доступ на чтение</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2386"/>
        <source>Read access denied. Adjust the file permissions and try again.

</source>
        <translation>Закрыт доступ на чтение. Исправьте права доступа к файлу и попробуйте ещё раз.

</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2412"/>
        <source>Color Ramp</source>
        <translation>Градиент</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2999"/>
        <source>Default Style</source>
        <translation type="unfinished">Стиль по умолчанию</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2984"/>
        <source>QGIS Layer Style File (*.qml)</source>
        <translation type="unfinished">Файл стиля слоя QGIS (*.qml)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="3005"/>
        <source>QGIS</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="3006"/>
        <source>Unknown style format: </source>
        <translation type="unfinished">Неизвестный формат стиля: </translation>
    </message>
</context>
<context>
    <name>QgsRasterLayerPropertiesBase</name>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="13"/>
        <source>Raster Layer Properties</source>
        <translation>Свойства растрового слоя</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Help</source>
        <translation type="obsolete">&amp;Помощь</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1306"/>
        <source>General</source>
        <translation>Общие</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1514"/>
        <source>Layer Source:</source>
        <translation type="obsolete">Источник слоя:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1534"/>
        <source>Display Name:</source>
        <translation type="obsolete">Имя в легенде:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1570"/>
        <source>Legend:</source>
        <translation>Легенда:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1464"/>
        <source>No Data:</source>
        <translation>Нет данных:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="32"/>
        <source>Symbology</source>
        <translation>Символика</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Transparency:</source>
        <translation type="obsolete">Прозрачность:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="881"/>
        <source>&lt;p align=&quot;right&quot;&gt;Full&lt;/p&gt;</source>
        <translation>Полная</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="835"/>
        <source>None</source>
        <translation>Нулевая</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="846"/>
        <source>Invert Color Map</source>
        <translation type="obsolete">Обратить цветовую карту</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Band</source>
        <translation type="obsolete">Канал</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&lt;b&gt;&lt;font color=&quot;#00ff00&quot;&gt;Green&lt;/font&gt;&lt;/b&gt;</source>
        <translation type="obsolete">&lt;b&gt;&lt;font color=&quot;#00ff00&quot;&gt;Зелёный&lt;/font&gt;&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&lt;b&gt;&lt;font color=&quot;#ff0000&quot;&gt;Red&lt;/font&gt;&lt;/b&gt;</source>
        <translation type="obsolete">&lt;b&gt;&lt;font color=&quot;#ff0000&quot;&gt;Красный&lt;/font&gt;&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&lt;b&gt;&lt;font color=&quot;#0000ff&quot;&gt;Blue&lt;/font&gt;&lt;/b&gt;</source>
        <translation type="obsolete">&lt;b&gt;&lt;font color=&quot;#0000ff&quot;&gt;Синий&lt;/font&gt;&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Color</source>
        <translation type="obsolete">Цвет</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="671"/>
        <source>Gray</source>
        <translation type="obsolete">Серый</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Std Deviations</source>
        <translation type="obsolete">Стд. отклонение</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="816"/>
        <source>Color Map</source>
        <translation type="obsolete">Цветовая карта</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1679"/>
        <source>Metadata</source>
        <translation>Метаданные</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1701"/>
        <source>Pyramids</source>
        <translation>Пирамиды</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1800"/>
        <source>Resampling Method</source>
        <translation type="obsolete">Метод интерполяции</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1790"/>
        <source>Average</source>
        <translation>Среднее значение</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1795"/>
        <source>Nearest Neighbour</source>
        <translation>Ближайший сосед</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1824"/>
        <source>Build Pyramids</source>
        <translation type="obsolete">Построить пирамиды</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1748"/>
        <source>Pyramid Resolutions</source>
        <translation type="obsolete">Разрешения пирамид</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1517"/>
        <source>Thumbnail</source>
        <translation>Образец</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1450"/>
        <source>Columns:</source>
        <translation>Столбцы:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1457"/>
        <source>Rows:</source>
        <translation>Строки:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1623"/>
        <source>Palette:</source>
        <translation>Палитра:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1440"/>
        <source>Maximum 1:</source>
        <translation type="obsolete">Максимальный 1:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1380"/>
        <source>Maximum scale at which this layer will be displayed. </source>
        <translation>Максимальный масштаб, при котором виден данный слой. </translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1463"/>
        <source>Minimum 1:</source>
        <translation type="obsolete">Минимальный 1:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1403"/>
        <source>Minimum scale at which this layer will be displayed. </source>
        <translation>Минимальный масштаб, при котором виден данный слой. </translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1383"/>
        <source>Spatial Reference System</source>
        <translation type="obsolete">Система координат</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1342"/>
        <source>Change</source>
        <translation>Изменить</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1813"/>
        <source>Histogram</source>
        <translation>Гистограмма</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1907"/>
        <source>Options</source>
        <translation>Параметры</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1939"/>
        <source>Out Of Range OK?</source>
        <translation type="obsolete">Разрешить значения вне диапазона?</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1946"/>
        <source>Allow Approximation</source>
        <translation type="obsolete">Разрешить аппроксимацию</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1862"/>
        <source>Chart Type</source>
        <translation>Тип диаграммы</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1903"/>
        <source>Bar Chart</source>
        <translation type="obsolete">Столбчатая</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1893"/>
        <source>Line Graph</source>
        <translation type="obsolete">Линейная</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1900"/>
        <source>Refresh</source>
        <translation>Обновить</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Display</source>
        <translation type="obsolete">Отображение</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Grayscale Image</source>
        <translation type="obsolete">Градации серого</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Color Image</source>
        <translation type="obsolete">Цветное изображение</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1412"/>
        <source>Scale Dependent Visibility</source>
        <translation type="obsolete">Видимость в пределах масштаба</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1932"/>
        <source>Column Count:</source>
        <translation type="obsolete">Столбцы:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Transparent</source>
        <translation type="obsolete">Прозрачность</translation>
    </message>
    <message>
        <location filename="" line="0"/>
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
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="50"/>
        <source>Grayscale Band Scaling</source>
        <translation type="obsolete">Уровень серого</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="584"/>
        <source>Max</source>
        <translation>Макс</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="332"/>
        <source>Std Deviation</source>
        <translation type="obsolete">Стд. отклонение</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="179"/>
        <source>Custom Min Max Values:</source>
        <translation type="obsolete">Пользовательские значения мин/макс:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="546"/>
        <source>Min</source>
        <translation>Мин</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="249"/>
        <source>Contrast Enhancement</source>
        <translation type="obsolete">Повышение контраста</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="291"/>
        <source>Load Min Max Values From Band(s)</source>
        <translation type="obsolete">Загрузить мин./макс. значения каналов</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="313"/>
        <source>RGB Scaling</source>
        <translation type="obsolete">Уровни RGB</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="325"/>
        <source>&lt;b&gt;&lt;font color=&apos;blue&apos;&gt;Max&lt;/font&gt;&lt;/b&gt;</source>
        <translation type="obsolete">&lt;b&gt;&lt;font color=&apos;blue&apos;&gt;Макс.&lt;/font&gt;&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="339"/>
        <source>Custom Min Max Values</source>
        <translation type="obsolete">Пользовательские значения мин/макс</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="381"/>
        <source>&lt;b&gt;&lt;font color=&apos;green&apos;&gt;Min&lt;/font&gt;&lt;/b&gt;</source>
        <translation type="obsolete">&lt;b&gt;&lt;font color=&apos;green&apos;&gt;Мин.&lt;/font&gt;&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="396"/>
        <source>&lt;b&gt;&lt;font color=&apos;green&apos;&gt;Max&lt;/font&gt;&lt;/b&gt;</source>
        <translation type="obsolete">&lt;b&gt;&lt;font color=&apos;green&apos;&gt;Макс.&lt;/font&gt;&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="411"/>
        <source>&lt;b&gt;&lt;font color=&apos;red&apos;&gt;Min&lt;/font&gt;&lt;/b&gt;</source>
        <translation type="obsolete">&lt;b&gt;&lt;font color=&apos;red&apos;&gt;Мин.&lt;/font&gt;&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="466"/>
        <source>&lt;b&gt;&lt;font color=&apos;red&apos;&gt;Max&lt;/font&gt;&lt;/b&gt;</source>
        <translation type="obsolete">&lt;b&gt;&lt;font color=&apos;red&apos;&gt;Макс.&lt;/font&gt;&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="548"/>
        <source>&lt;b&gt;&lt;font color=&apos;blue&apos;&gt;Min&lt;/font&gt;&lt;/b&gt;</source>
        <translation type="obsolete">&lt;b&gt;&lt;font color=&apos;blue&apos;&gt;Мин.&lt;/font&gt;&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="651"/>
        <source>Grayscale Band Selection</source>
        <translation type="obsolete">Выбор серого канала</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="699"/>
        <source>RGB Mode Band Selection</source>
        <translation type="obsolete">Выбор каналов RGB</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="711"/>
        <source>&lt;b&gt;&lt;font color=&apos;blue&apos;&gt;Blue&lt;/font&gt;&lt;/b&gt;</source>
        <translation type="obsolete">&lt;b&gt;&lt;font color=&apos;blue&apos;&gt;Синий&lt;/font&gt;&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="721"/>
        <source>&lt;b&gt;&lt;font color=&apos;green&apos;&gt;Green&lt;/font&gt;&lt;/b&gt;</source>
        <translation type="obsolete">&lt;b&gt;&lt;font color=&apos;green&apos;&gt;Зелёный&lt;/font&gt;&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="731"/>
        <source>&lt;b&gt;&lt;font color=&apos;red&apos;&gt;Red&lt;/font&gt;&lt;/b&gt;</source>
        <translation type="obsolete">&lt;b&gt;&lt;font color=&apos;red&apos;&gt;Красный&lt;/font&gt;&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="856"/>
        <source>Global Transparency</source>
        <translation type="obsolete">Общая прозрачность</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="861"/>
        <source> 00%</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="38"/>
        <source>Render as</source>
        <translation>Отображать как</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="972"/>
        <source>Single Band Gray</source>
        <translation type="obsolete">Одноканальное серое</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="979"/>
        <source>Three Band Color</source>
        <translation type="obsolete">Трёхканальное цветное</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="990"/>
        <source>Transparent Pixels</source>
        <translation type="obsolete">Прозрачность</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1036"/>
        <source>Transparent Band:</source>
        <translation type="obsolete">Канал прозрачности:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1050"/>
        <source>Custom Transparency List</source>
        <translation type="obsolete">Пользовательские значения прозрачности</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1057"/>
        <source>Transparency Layer;</source>
        <translation type="obsolete">Слой прозрачности:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1100"/>
        <source>Add Values Manually</source>
        <translation type="obsolete">Добавить значение вручную</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1098"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1116"/>
        <source>Add Values From Display</source>
        <translation type="obsolete">Добавить значение с экрана</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1129"/>
        <source>Remove Selected Row</source>
        <translation type="obsolete">Удалить выбранную строку</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1142"/>
        <source>Default Values</source>
        <translation type="obsolete">Значения по умолчанию</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1168"/>
        <source>Import From File</source>
        <translation type="obsolete">Импорт из файла</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1181"/>
        <source>Export To File</source>
        <translation type="obsolete">Экспорт в файл</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1196"/>
        <source>No Data Value:</source>
        <translation type="obsolete">Значение «нет данных»: </translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1206"/>
        <source>Reset No Data Value</source>
        <translation type="obsolete">Сбросить значение «нет данных»</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1127"/>
        <source>Colormap</source>
        <translation>Цветовая карта</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1260"/>
        <source>Number of entries:</source>
        <translation type="obsolete">Количество значений:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1217"/>
        <source>Delete entry</source>
        <translation>Удалить значение</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1224"/>
        <source>Classify</source>
        <translation>Классифицировать</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1240"/>
        <source>1</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1245"/>
        <source>2</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1338"/>
        <source>Color interpolation:</source>
        <translation type="obsolete">Интерполяция цветов:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1358"/>
        <source>Classification mode:</source>
        <translation type="obsolete">Режим классификации:</translation>
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
        <translation type="unfinished"></translation>
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
        <translation type="unfinished">Имя в легенде</translation>
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
        <translation type="unfinished">Восстановить по умолчанию</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="2009"/>
        <source>Save As Default</source>
        <translation type="unfinished">Сохранить по умолчанию</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="2016"/>
        <source>Load Style ...</source>
        <translation type="unfinished">Загрузить стиль...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="2023"/>
        <source>Save Style ...</source>
        <translation type="unfinished">Сохранить стиль...</translation>
    </message>
</context>
<context>
    <name>QgsRunProcess</name>
    <message>
        <location filename="../src/core/qgsrunprocess.cpp" line="146"/>
        <source>Unable to run command</source>
        <translation>Не удалось выполнить команду</translation>
    </message>
    <message>
        <location filename="../src/core/qgsrunprocess.cpp" line="59"/>
        <source>Starting</source>
        <translation>Выполняется</translation>
    </message>
    <message>
        <location filename="../src/core/qgsrunprocess.cpp" line="115"/>
        <source>Done</source>
        <translation>Выполнено</translation>
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
        <translation type="unfinished">Вы уверены, что хотите удалить соединение </translation>
    </message>
    <message>
        <location filename="../src/plugins/sos/qgssossourceselect.cpp" line="219"/>
        <source> connection and all associated settings?</source>
        <translation type="unfinished"> и все связанные с ним параметры?</translation>
    </message>
    <message>
        <location filename="../src/plugins/sos/qgssossourceselect.cpp" line="220"/>
        <source>Confirm Delete</source>
        <translation type="unfinished">Потвердите удаление</translation>
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
        <translation type="unfinished">Соединения с серверами</translation>
    </message>
    <message>
        <location filename="../src/plugins/sos/qgssossourceselectbase.ui" line="49"/>
        <source>&amp;New</source>
        <translation type="unfinished">&amp;Новое</translation>
    </message>
    <message>
        <location filename="../src/plugins/sos/qgssossourceselectbase.ui" line="59"/>
        <source>Delete</source>
        <translation type="unfinished">Удалить</translation>
    </message>
    <message>
        <location filename="../src/plugins/sos/qgssossourceselectbase.ui" line="69"/>
        <source>Edit</source>
        <translation type="unfinished">Изменить</translation>
    </message>
    <message>
        <location filename="../src/plugins/sos/qgssossourceselectbase.ui" line="95"/>
        <source>C&amp;onnect</source>
        <translation type="unfinished">&amp;Подключить</translation>
    </message>
    <message>
        <location filename="../src/plugins/sos/qgssossourceselectbase.ui" line="108"/>
        <source>Offerings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/sos/qgssossourceselectbase.ui" line="118"/>
        <source>Name</source>
        <translation type="unfinished">Имя</translation>
    </message>
    <message>
        <location filename="../src/plugins/sos/qgssossourceselectbase.ui" line="123"/>
        <source>Id</source>
        <translation type="unfinished">ID</translation>
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
        <translation> метров/км</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="281"/>
        <source> feet</source>
        <translation> футов</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="288"/>
        <source> degrees</source>
        <translation> градусов</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="243"/>
        <source> km</source>
        <translation> км</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="248"/>
        <source> mm</source>
        <translation> мм</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="253"/>
        <source> cm</source>
        <translation> см</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="257"/>
        <source> m</source>
        <translation> м</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="277"/>
        <source> foot</source>
        <translation> фут</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="286"/>
        <source> degree</source>
        <translation> градус</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="291"/>
        <source> unknown</source>
        <translation> неизв</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="79"/>
        <source>Top Left</source>
        <translation>Вверху слева</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="78"/>
        <source>Bottom Left</source>
        <translation>Внизу слева</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="79"/>
        <source>Top Right</source>
        <translation>Вверху справа</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="79"/>
        <source>Bottom Right</source>
        <translation>Внизу справа</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="81"/>
        <source>Tick Down</source>
        <translation>Штрих вниз</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="82"/>
        <source>Tick Up</source>
        <translation>Штрих вверх</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="82"/>
        <source>Bar</source>
        <translation>Линия</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="82"/>
        <source>Box</source>
        <translation>Рамка</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="102"/>
        <source>&amp;Scale Bar</source>
        <translation>&amp;Масштабная линейка</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="103"/>
        <source>Creates a scale bar that is displayed on the map canvas</source>
        <translation>Создаёт масштабную линейку в области отображения карты</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="543"/>
        <source>&amp;Decorations</source>
        <translation>&amp;Оформление</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="165"/>
        <source> feet/miles</source>
        <translation> футов/миль</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="262"/>
        <source> miles</source>
        <translation> миль</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="267"/>
        <source> mile</source>
        <translation> миля</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="272"/>
        <source> inches</source>
        <translation>дюймов</translation>
    </message>
</context>
<context>
    <name>QgsScaleBarPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="300"/>
        <source>Scale Bar Plugin</source>
        <translation>Модуль масштабной линейки</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="143"/>
        <source>Top Left</source>
        <translation>Вверху слева</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="148"/>
        <source>Top Right</source>
        <translation>Вверху справа</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="153"/>
        <source>Bottom Left</source>
        <translation>Внизу слева</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="158"/>
        <source>Bottom Right</source>
        <translation>Внизу справа</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="94"/>
        <source>Size of bar:</source>
        <translation>Размер линейки:</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="254"/>
        <source>Placement:</source>
        <translation>Размещение:</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="211"/>
        <source>Tick Down</source>
        <translation>Штрих вниз</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="216"/>
        <source>Tick Up</source>
        <translation>Штрих вверх</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="221"/>
        <source>Box</source>
        <translation>Рамка</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="226"/>
        <source>Bar</source>
        <translation>Линия</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="207"/>
        <source>Select the style of the scale bar</source>
        <translation>Выберите стиль масштабной линейки</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="127"/>
        <source>Colour of bar:</source>
        <translation>Цвет линейки:</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="192"/>
        <source>Scale bar style:</source>
        <translation>Стиль линейки:</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="174"/>
        <source>Enable scale bar</source>
        <translation>Включить масштабную линейку</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="109"/>
        <source>Automatically snap to round number on resize</source>
        <translation>Автоматически изменять размер для округления показателя</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="76"/>
        <source>Click to select the colour</source>
        <translation>Щелкните для выбора цвета</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/pluginguibase.ui" line="274"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;This plugin draws a scale bar on the map. Please note the size option below is a &apos;preferred&apos; size and may have to be altered by QGIS depending on the level of zoom.  The size is measured according to the map units specified in the project properties.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Этот модуль добавляет к карте масштабную линейку. Обратите внимание, что параметр размера ниже является «предпочтительным» и может быть изменён в зависимости от текущего масштаба.  Размер задаётся в соответствии с единицами карты, указанными в свойствах проекта.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
</context>
<context>
    <name>QgsSearchQueryBuilder</name>
    <message numerus="yes">
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="171"/>
        <source>Found %d matching features.</source>
        <translation>
            <numerusform>Найден %d подходящий объект.</numerusform>
            <numerusform>Найдено %d подходящих объекта.</numerusform>
            <numerusform>Найдено %d подходящих объектов.</numerusform>
        </translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="173"/>
        <source>No matching features found.</source>
        <translation>Подходящих объектов не найдено.</translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="174"/>
        <source>Search results</source>
        <translation>Результаты поиска</translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="183"/>
        <source>Search string parsing error</source>
        <translation>Ошибка обработки строки запроса</translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="239"/>
        <source>No Records</source>
        <translation>Нет записей</translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="239"/>
        <source>The query you specified results in zero records being returned.</source>
        <translation>В результате указанного запроса найдено 0 записей.</translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="41"/>
        <source>Search query builder</source>
        <translation>Конструктор поисковых запросов</translation>
    </message>
</context>
<context>
    <name>QgsServerSourceSelect</name>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="173"/>
        <source>Are you sure you want to remove the </source>
        <translation>Вы уверены, что хотите удалить соединение </translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="173"/>
        <source> connection and all associated settings?</source>
        <translation> и все связанные с ним параметры?</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="174"/>
        <source>Confirm Delete</source>
        <translation>Потвердите удаление</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Yes</source>
        <translation type="obsolete">Да</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>No</source>
        <translation type="obsolete">Нет</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="399"/>
        <source>WMS Provider</source>
        <translation>WMS-источник</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="401"/>
        <source>Could not open the WMS Provider</source>
        <translation>Не удалось открыть WMS-источник</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="410"/>
        <source>Select Layer</source>
        <translation>Выберите слой</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="410"/>
        <source>You must select at least one layer first.</source>
        <translation>Для добавления следует выбрать хотя бы один слой.</translation>
    </message>
    <message numerus="yes">
        <location filename="../src/app/qgsserversourceselect.cpp" line="520"/>
        <source>Coordinate Reference System (%1 available)</source>
        <translation>
            <numerusform>Система координат (доступна %1)</numerusform>
            <numerusform>Система координат (доступно %1)</numerusform>
            <numerusform>Система координат (доступно %1)</numerusform>
        </translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="679"/>
        <source>Could not understand the response.  The</source>
        <translation>Ошибка обработки ответа. Источник</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="680"/>
        <source>provider said</source>
        <translation>сообщил</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="731"/>
        <source>WMS proxies</source>
        <translation>WMS-прокси</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="822"/>
        <source>&lt;p&gt;Several WMS servers have been added to the server list. Note that the proxy fields have been left blank and if you access the internet via a web proxy, you will need to individually set the proxy fields with appropriate values.&lt;/p&gt;</source>
        <translation type="obsolete">&lt;p&gt;Некоторые WMS-сервера были добавлены в список. Обратите внимание, что поля прокси были оставлены пустыми, и если вы выходите в интернет, используя прокси-сервер, вам следует задать соответствующие параметры прокси.&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="414"/>
        <source>Coordinate Reference System</source>
        <translation>Система координат</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="414"/>
        <source>There are no available coordinate reference system for the set of layers you&apos;ve selected.</source>
        <translation>Для выбранных слоёв не найдено доступных систем координат.</translation>
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
        <translation>Добавить слои с сервера</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="309"/>
        <source>C&amp;lose</source>
        <translation>&amp;Закрыть</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="312"/>
        <source>Alt+L</source>
        <translation>Alt+P</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="296"/>
        <source>Help</source>
        <translation>Справка</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="299"/>
        <source>F1</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="270"/>
        <source>Image encoding</source>
        <translation>формат изображения</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="203"/>
        <source>Layers</source>
        <translation>Слои</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="230"/>
        <source>ID</source>
        <translation>ID</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="235"/>
        <source>Name</source>
        <translation>Имя</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="240"/>
        <source>Title</source>
        <translation>Заглавие</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="245"/>
        <source>Abstract</source>
        <translation>Описание</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="187"/>
        <source>&amp;Add</source>
        <translation>&amp;Добавить</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="190"/>
        <source>Alt+A</source>
        <translation>Alt+L</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="34"/>
        <source>Server Connections</source>
        <translation>Соединения с серверами</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="108"/>
        <source>&amp;New</source>
        <translation>&amp;Новое</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="101"/>
        <source>Delete</source>
        <translation>Удалить</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="91"/>
        <source>Edit</source>
        <translation>Изменить</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="81"/>
        <source>C&amp;onnect</source>
        <translation>&amp;Подключить</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="174"/>
        <source>Ready</source>
        <translation>Готово</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="118"/>
        <source>Coordinate Reference System</source>
        <translation>Система координат</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="156"/>
        <source>Change ...</source>
        <translation>Изменить...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="46"/>
        <source>Adds a few example WMS servers</source>
        <translation>Добавить несколько известных WMS-серверов</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsserversourceselectbase.ui" line="52"/>
        <source>Add default servers</source>
        <translation>Добавить сервера по умолчанию</translation>
    </message>
</context>
<context>
    <name>QgsShapeFile</name>
    <message>
        <location filename="../src/plugins/spit/qgsshapefile.cpp" line="419"/>
        <source>The database gave an error while executing this SQL:</source>
        <translation>База данных вернула ошибку при выполнении SQL:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsshapefile.cpp" line="427"/>
        <source>The error was:</source>
        <translation>Сообщение об ошибке:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsshapefile.cpp" line="424"/>
        <source>... (rest of SQL trimmed)</source>
        <comment>is appended to a truncated SQL statement</comment>
        <translation>...(остаток SQL проигнорирован)</translation>
    </message>
</context>
<context>
    <name>QgsSingleSymbolDialog</name>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="121"/>
        <source>Solid Line</source>
        <translation>Сплошная линия</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="122"/>
        <source>Dash Line</source>
        <translation>Штриховой пунктир</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="123"/>
        <source>Dot Line</source>
        <translation>Точечный пунктир</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="124"/>
        <source>Dash Dot Line</source>
        <translation>Штрихпунктир</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="125"/>
        <source>Dash Dot Dot Line</source>
        <translation>Штрихпунктир с двумя точками</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="126"/>
        <source>No Pen</source>
        <translation>Без линии</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="131"/>
        <source>Solid Pattern</source>
        <translation type="obsolete">Сплошная заливка</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="132"/>
        <source>Hor Pattern</source>
        <translation type="obsolete">Горизонтальный шаблон</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="133"/>
        <source>Ver Pattern</source>
        <translation type="obsolete">Вертикальный шаблон</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="134"/>
        <source>Cross Pattern</source>
        <translation type="obsolete">Сетка</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="135"/>
        <source>BDiag Pattern</source>
        <translation type="obsolete">Диагональный 1</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="136"/>
        <source>FDiag Pattern</source>
        <translation type="obsolete">Диагональный 2</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="137"/>
        <source>Diag Cross Pattern</source>
        <translation type="obsolete">Диагональная сетка</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="138"/>
        <source>Dense1 Pattern</source>
        <translation type="obsolete">Штриховка 1</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="139"/>
        <source>Dense2 Pattern</source>
        <translation type="obsolete">Штриховка 2</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="140"/>
        <source>Dense3 Pattern</source>
        <translation type="obsolete">Штриховка 3</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="141"/>
        <source>Dense4 Pattern</source>
        <translation type="obsolete">Штриховка 4</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="142"/>
        <source>Dense5 Pattern</source>
        <translation type="obsolete">Штриховка 5</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="143"/>
        <source>Dense6 Pattern</source>
        <translation type="obsolete">Штриховка 6</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="144"/>
        <source>Dense7 Pattern</source>
        <translation type="obsolete">Штриховка 7</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="145"/>
        <source>No Brush</source>
        <translation>Без заливки</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="146"/>
        <source>Texture Pattern</source>
        <translation type="obsolete">Заливка текстурой</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="131"/>
        <source>Solid</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="132"/>
        <source>Horizontal</source>
        <translation type="unfinished">Горизонтальная</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="133"/>
        <source>Vertical</source>
        <translation type="unfinished">Вертикальная</translation>
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
        <translation>Обычный знак</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Fill Patterns:</source>
        <translation type="obsolete">Шаблоны заливки:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Point</source>
        <translation type="obsolete">Точка</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="95"/>
        <source>Size</source>
        <translation>Размер</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Symbol</source>
        <translation type="obsolete">Знак</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Outline Width:</source>
        <translation type="obsolete">Ширина контура:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Fill Color:</source>
        <translation type="obsolete">Цвет заливки:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Outline color:</source>
        <translation type="obsolete">Цвет контура:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Outline Style:</source>
        <translation type="obsolete">Стиль контура:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Label:</source>
        <translation type="obsolete">Метка:</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>No Fill</source>
        <translation type="obsolete">Без заливки</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Browse:</source>
        <translation type="obsolete">Обзор:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="73"/>
        <source>Point Symbol</source>
        <translation>Значок</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="105"/>
        <source>Area scale field</source>
        <translation>Поле масштаба</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="115"/>
        <source>Rotation field</source>
        <translation>Поле вращения</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="182"/>
        <source>Style Options</source>
        <translation>Параметры стиля</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="341"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="188"/>
        <source>Outline style</source>
        <translation>Стиль линии</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="220"/>
        <source>Outline color</source>
        <translation>Цвет линии</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="255"/>
        <source>Outline width</source>
        <translation>Ширина линии</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="278"/>
        <source>Fill color</source>
        <translation>Цвет заливки</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="313"/>
        <source>Fill style</source>
        <translation>Стиль заливки</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="45"/>
        <source>Label</source>
        <translation>Метка</translation>
    </message>
</context>
<context>
    <name>QgsSnappingDialog</name>
    <message>
        <location filename="../src/app/qgssnappingdialog.cpp" line="147"/>
        <source>to vertex</source>
        <translation>к вершинам</translation>
    </message>
    <message>
        <location filename="../src/app/qgssnappingdialog.cpp" line="151"/>
        <source>to segment</source>
        <translation>к сегментам</translation>
    </message>
    <message>
        <location filename="../src/app/qgssnappingdialog.cpp" line="89"/>
        <source>to vertex and segment</source>
        <translation>к вершинам и сегментам</translation>
    </message>
</context>
<context>
    <name>QgsSnappingDialogBase</name>
    <message>
        <location filename="../src/ui/qgssnappingdialogbase.ui" line="13"/>
        <source>Snapping options</source>
        <translation>Параметры прилипания</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssnappingdialogbase.ui" line="26"/>
        <source>Layer</source>
        <translation>Слой</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssnappingdialogbase.ui" line="31"/>
        <source>Mode</source>
        <translation>Режим</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssnappingdialogbase.ui" line="36"/>
        <source>Tolerance</source>
        <translation>Порог</translation>
    </message>
</context>
<context>
    <name>QgsSpit</name>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="148"/>
        <source>Are you sure you want to remove the [</source>
        <translation>Вы уверены, что хотите удалить соединение [</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="148"/>
        <source>] connection and all associated settings?</source>
        <translation>] и все связанные с ним параметры?</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="149"/>
        <source>Confirm Delete</source>
        <translation>Потвердите удаление</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Yes</source>
        <translation type="obsolete">Да</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>No</source>
        <translation type="obsolete">Нет</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="866"/>
        <source> - Edit Column Names</source>
        <translation type="obsolete"> — Правка имён полей</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="268"/>
        <source>The following Shapefile(s) could not be loaded:

</source>
        <translation>Не удалось загрузить следующие shape-файлы:

</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="272"/>
        <source>REASON: File cannot be opened</source>
        <translation>ПРИЧИНА: Файл не может быть открыт</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="277"/>
        <source>REASON: One or both of the Shapefile files (*.dbf, *.shx) missing</source>
        <translation>ПРИЧИНА: Один или оба дополнительных файла (*.dbf, *.shx) отсутствуют</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="354"/>
        <source>General Interface Help:</source>
        <translation>Общая справка по интерфейсу:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="356"/>
        <source>PostgreSQL Connections:</source>
        <translation>Соединения PostgreSQL:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="358"/>
        <source>[New ...] - create a new connection</source>
        <translation>[Новое...] — создать новое соединение</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="359"/>
        <source>[Edit ...] - edit the currently selected connection</source>
        <translation>[Изменить...] — редактировать выбранное соединение</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="360"/>
        <source>[Remove] - remove the currently selected connection</source>
        <translation>[Удалить] — удалить выбранное соединение</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="361"/>
        <source>-you need to select a connection that works (connects properly) in order to import files</source>
        <translation>- для успешного импорта необходимо выбрать рабочее соединение</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="362"/>
        <source>-when changing connections Global Schema also changes accordingly</source>
        <translation>- при изменении соединения соответственно меняется общая схема</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="363"/>
        <source>Shapefile List:</source>
        <translation>Список shape-файлов:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="365"/>
        <source>[Add ...] - open a File dialog and browse to the desired file(s) to import</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="366"/>
        <source>[Remove] - remove the currently selected file(s) from the list</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="367"/>
        <source>[Remove All] - remove all the files in the list</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="368"/>
        <source>[SRID] - Reference ID for the shapefiles to be imported</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="369"/>
        <source>[Use Default (SRID)] - set SRID to -1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="370"/>
        <source>[Geometry Column Name] - name of the geometry column in the database</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="371"/>
        <source>[Use Default (Geometry Column Name)] - set column name to &apos;the_geom&apos;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="372"/>
        <source>[Glogal Schema] - set the schema for all files to be imported into</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="374"/>
        <source>[Import] - import the current shapefiles in the list</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="375"/>
        <source>[Quit] - quit the program
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="376"/>
        <source>[Help] - display this help dialog</source>
        <translation>[Справка] — вывести этот диалог справки</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="833"/>
        <source>Import Shapefiles</source>
        <translation>Импорт shape-файлов</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="833"/>
        <source>You need to specify a Connection first</source>
        <translation>Необходимо указать соединение</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="428"/>
        <source>Connection failed - Check settings and try again</source>
        <translation>Не удалось соединиться — проверьте параметры и попробуйте ещё раз </translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="513"/>
        <source>You need to add shapefiles to the list first</source>
        <translation>Необходимо добавить shape-файлы в список</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="580"/>
        <source>Importing files</source>
        <translation>Импорт файлов</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="518"/>
        <source>Cancel</source>
        <translation>Отменить</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="522"/>
        <source>Progress</source>
        <translation>Прогресс</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="531"/>
        <source>Problem inserting features from file:</source>
        <translation>Проблема при вставке объектов из файла:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="538"/>
        <source>Invalid table name.</source>
        <translation>Неверное имя таблицы.</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="548"/>
        <source>No fields detected.</source>
        <translation>Поля не выбраны.</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="573"/>
        <source>The following fields are duplicates:</source>
        <translation>Следующие поля являются дубликатами:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="672"/>
        <source>Import Shapefiles - Relation Exists</source>
        <translation>Импорт shape-файлов — реляция существует</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="673"/>
        <source>The Shapefile:</source>
        <translation>Shape-файл:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="674"/>
        <source>will use [</source>
        <translation>будет загружен в реляцию [</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="674"/>
        <source>] relation for its data,</source>
        <translation>],</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="674"/>
        <source>which already exists and possibly contains data.</source>
        <translation>которая уже существует и возможно содержит данные.</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="675"/>
        <source>To avoid data loss change the &quot;DB Relation Name&quot;</source>
        <translation>Во избежание потери данных, измените «Имя реляции БД»</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="675"/>
        <source>for this Shapefile in the main dialog file list.</source>
        <translation>для этого shape-файла в списке файлов главного диалога.</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="676"/>
        <source>Do you want to overwrite the [</source>
        <translation>Вы хотите перезаписать реляцию [</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="676"/>
        <source>] relation?</source>
        <translation>]?</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="70"/>
        <source>File Name</source>
        <translation>Имя файла</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="70"/>
        <source>Feature Class</source>
        <translation>Класс объектов</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="71"/>
        <source>Features</source>
        <translation>Объекты</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="71"/>
        <source>DB Relation Name</source>
        <translation>Имя реляции БД</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="71"/>
        <source>Schema</source>
        <translation>Схема</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="122"/>
        <source>New Connection</source>
        <translation type="obsolete">Новое соединение</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="172"/>
        <source>Add Shapefiles</source>
        <translation>Добавить shape-файлы</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="174"/>
        <source>Shapefiles (*.shp);;All files (*.*)</source>
        <translation>Shape-файлы (*.shp);;Все файлы (*.*)</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="467"/>
        <source>PostGIS not available</source>
        <translation>PostGIS недоступна</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="469"/>
        <source>&lt;p&gt;The chosen database does not have PostGIS installed, but this is required for storage of spatial data.&lt;/p&gt;</source>
        <translation>&lt;p&gt;PostGIS не установлен в выбранной БД, что делает невозможным хранение пространственных данных.&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="564"/>
        <source>Checking to see if </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="814"/>
        <source>&lt;p&gt;Error while executing the SQL:&lt;/p&gt;&lt;p&gt;</source>
        <translation>&lt;p&gt;Ошибка при выполнении SQL:&lt;/p&gt;&lt;p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="815"/>
        <source>&lt;/p&gt;&lt;p&gt;The database said:</source>
        <translation>&lt;/p&gt;&lt;p&gt;Сообщение БД:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="829"/>
        <source>%1 of %2 shapefiles could not be imported.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="408"/>
        <source>Password for </source>
        <translation type="unfinished">Пароль для </translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="409"/>
        <source>Please enter your password:</source>
        <translation type="unfinished">Пожалуйста, введите ваш пароль:</translation>
    </message>
</context>
<context>
    <name>QgsSpitBase</name>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="19"/>
        <source>SPIT - Shapefile to PostGIS Import Tool</source>
        <translation>SPIT — инструмент импорта shape-файлов в PostGIS</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="54"/>
        <source>PostgreSQL Connections</source>
        <translation>Соединения PostgreSQL</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="188"/>
        <source>Remove</source>
        <translation>Удалить</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="63"/>
        <source>Shapefile List</source>
        <translation type="obsolete">Список shape-файлов</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="269"/>
        <source>Geometry Column Name</source>
        <translation type="obsolete">Имя поля геометрии</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="201"/>
        <source>Remove All</source>
        <translation>Удалить все</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="273"/>
        <source>Global Schema</source>
        <translation>Общая схема</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="53"/>
        <source>Shapefile to PostGIS Import Tool</source>
        <translation type="obsolete">Импорт shape-файлов в PostGIS</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="175"/>
        <source>Add</source>
        <translation>Добавить</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="172"/>
        <source>Add a shapefile to the list of files to be imported</source>
        <translation>Добавить shape-файл к списку импортируемых</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="185"/>
        <source>Remove the selected shapefile from the import list</source>
        <translation>Удалить выбранный shape-файл из списка</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="198"/>
        <source>Remove all the shapefiles from the import list</source>
        <translation>Удалить все shape-файлы из списка</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="215"/>
        <source>Use Default SRID</source>
        <translation type="obsolete">Использовать SRID по умолчанию</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="224"/>
        <source>Set the SRID to the default value</source>
        <translation>Заполнить SRID значением по умолчанию</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="254"/>
        <source>Use Default Geometry Column Name</source>
        <translation type="obsolete">Использовать имя поля геометрии по умолчанию</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="237"/>
        <source>Set the geometry column name to the default value</source>
        <translation>Задать имя поля геометрии в соответствии со значением по умолчанию</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="121"/>
        <source>New</source>
        <translation>Новое</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="118"/>
        <source>Create a new PostGIS connection</source>
        <translation>Создать новое PostGIS-соединение</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="105"/>
        <source>Remove the current PostGIS connection</source>
        <translation>Удалить текущее PostGIS-соединение</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="134"/>
        <source>Connect</source>
        <translation type="unfinished">Подключить</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="95"/>
        <source>Edit</source>
        <translation>Изменить</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="92"/>
        <source>Edit the current PostGIS connection</source>
        <translation>Редактировать текущее PostGIS-соединение</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="144"/>
        <source>Import options and shapefile list</source>
        <translation>Параметры импорта и список shape-файлов</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="227"/>
        <source>Use Default SRID or specify here</source>
        <translation>Использовать SRID по умолчанию или указанный</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="240"/>
        <source>Use Default Geometry Column Name or specify here</source>
        <translation>Использовать поле геометрии по умолчанию или указанное</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="263"/>
        <source>Primary Key Column Name</source>
        <translation>Имя первичного ключевого поля</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="131"/>
        <source>Connect to PostGIS</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsSpitPlugin</name>
    <message>
        <location filename="../src/plugins/spit/qgsspitplugin.cpp" line="68"/>
        <source>&amp;Import Shapefiles to PostgreSQL</source>
        <translation>&amp;Импорт Shape-файлов в PostgreSQL</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitplugin.cpp" line="70"/>
        <source>Import shapefiles into a PostGIS-enabled PostgreSQL database. The schema and field names can be customized on import</source>
        <translation>Импорт shape-файлов в базу данных PostgreSQL с поддержкой PostGIS. В процессе импорта допускается изменение схемы и имён полей</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitplugin.cpp" line="93"/>
        <source>&amp;Spit</source>
        <translation>&amp;SPIT</translation>
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
        <translation type="unfinished">Потвердите удаление</translation>
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
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsuniquevaluedialogbase.ui" line="37"/>
        <source>Classification Field:</source>
        <translation type="obsolete">Поле классификации:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsuniquevaluedialogbase.ui" line="47"/>
        <source>Delete class</source>
        <translation type="obsolete">Удалить класс</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsuniquevaluedialogbase.ui" line="93"/>
        <source>Classify</source>
        <translation>Классифицировать</translation>
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
        <location filename="" line="0"/>
        <source>&amp;Zoom to extent of selected layer</source>
        <translation type="obsolete">&amp;Изменить вид до полной данного слоя</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Open attribute table</source>
        <translation type="obsolete">&amp;Открыть таблицу атрибутов</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Properties</source>
        <translation type="obsolete">&amp;Свойства</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Remove</source>
        <translation type="obsolete">&amp;Удалить</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2470"/>
        <source>Could not commit the added features.</source>
        <translation>Не удалось внести добавленные объекты.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2561"/>
        <source>No other types of changes will be committed at this time.</source>
        <translation>Другие виды изменений не будут внесены в данный момент.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2492"/>
        <source>Could not commit the changed attributes.</source>
        <translation>Не удалось внести изменённые атрибуты.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2551"/>
        <source>However, the added features were committed OK.</source>
        <translation>Тем не менее, добавленные объекты были успешно внесены.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2518"/>
        <source>Could not commit the changed geometries.</source>
        <translation>Не удалось внести изменённую геометрию объектов.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2555"/>
        <source>However, the changed attributes were committed OK.</source>
        <translation>Тем не менее, изменённые атрибуты были успешно внесены.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2548"/>
        <source>Could not commit the deleted features.</source>
        <translation>Не удалось внести удалённые объекты.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2559"/>
        <source>However, the changed geometries were committed OK.</source>
        <translation>Тем не менее, изменённия геометрии были успешно внесены.</translation>
    </message>
</context>
<context>
    <name>QgsVectorLayerProperties</name>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="113"/>
        <source>Transparency: </source>
        <translation>Прозрачность: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="205"/>
        <source>Single Symbol</source>
        <translation>Обычный знак</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="208"/>
        <source>Graduated Symbol</source>
        <translation>Градуированный знак</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="209"/>
        <source>Continuous Color</source>
        <translation>Непрерывный цвет</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="210"/>
        <source>Unique Value</source>
        <translation>Уникальное значение</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="164"/>
        <source>This button opens the PostgreSQL query builder and allows you to create a subset of features to display on the map canvas rather than displaying all features in the layer</source>
        <translation>Эта кнопка открывает конструктор запросов PostgreSQL, при помощи которого можно выбрать подмножество объектов для отображения на карте, иначе все объекты будут видимы</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="167"/>
        <source>The query used to limit the features in the layer is shown here. This is currently only supported for PostgreSQL layers. To enter or modify the query, click on the Query Builder button</source>
        <translation>Этот запрос используется для ограничения доступа к объектам слоя. В данный момент поддерживаются только слои PostgreSQL. Для создания или изменения запроса кликните на кнопке «Конструктор запросов»</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="398"/>
        <source>Spatial Index</source>
        <translation>Пространственный индекс</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="393"/>
        <source>Creation of spatial index successfull</source>
        <translation> Пространственный индекс успешно создан</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="398"/>
        <source>Creation of spatial index failed</source>
        <translation>Не удалось создать пространственный индекс</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="411"/>
        <source>General:</source>
        <translation>Общее:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="426"/>
        <source>Storage type of this layer : </source>
        <translation>Тип хранилища этого слоя: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="432"/>
        <source>Source for this layer : </source>
        <translation>Источник этого слоя: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="449"/>
        <source>Geometry type of the features in this layer : </source>
        <translation>Тип геометрии объектов в этом слое: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="457"/>
        <source>The number of features in this layer : </source>
        <translation>Количество объектов в слое: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="462"/>
        <source>Editing capabilities of this layer : </source>
        <translation>Возможности редактирования данного слоя: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="469"/>
        <source>Extents:</source>
        <translation>Границы: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="474"/>
        <source>In layer spatial reference system units : </source>
        <translation>В единицах координатной системы слоя: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="475"/>
        <source>xMin,yMin </source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="479"/>
        <source> : xMax,yMax </source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="534"/>
        <source>In project spatial reference system units : </source>
        <translation>В единицах координатной системы проекта: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="508"/>
        <source>Layer Spatial Reference System:</source>
        <translation>Система координат слоя:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="545"/>
        <source>Attribute field info:</source>
        <translation>Поля атрибутов:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="552"/>
        <source>Field</source>
        <translation>Поле</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="555"/>
        <source>Type</source>
        <translation>Тип</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="558"/>
        <source>Length</source>
        <translation>Длина</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="561"/>
        <source>Precision</source>
        <translation>Точность</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="419"/>
        <source>Layer comment: </source>
        <translation>Комментарий слоя: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="564"/>
        <source>Comment</source>
        <translation>Комментарий</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="766"/>
        <source>Default Style</source>
        <translation>Стиль по умолчанию</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="748"/>
        <source>QGIS Layer Style File (*.qml)</source>
        <translation>Файл стиля слоя QGIS (*.qml)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="772"/>
        <source>QGIS</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="773"/>
        <source>Unknown style format: </source>
        <translation>Неизвестный формат стиля: </translation>
    </message>
</context>
<context>
    <name>QgsVectorLayerPropertiesBase</name>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="19"/>
        <source>Layer Properties</source>
        <translation>Свойства слоя</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="85"/>
        <source>Legend type:</source>
        <translation type="obsolete">Тип легенды:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="38"/>
        <source>Symbology</source>
        <translation>Символика</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="69"/>
        <source>Transparency:</source>
        <translation type="obsolete">Прозрачность:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="145"/>
        <source>General</source>
        <translation>Общие</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="217"/>
        <source>Use scale dependent rendering</source>
        <translation>Видимость в пределах масштаба</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="192"/>
        <source>Maximum 1:</source>
        <translation type="obsolete">Максимальный 1:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="202"/>
        <source>Minimum 1:</source>
        <translation type="obsolete">Минимальный 1:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="258"/>
        <source>Minimum scale at which this layer will be displayed. </source>
        <translation>Минимальный масштаб, при котором виден данный слой. </translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="271"/>
        <source>Maximum scale at which this layer will be displayed. </source>
        <translation>Максимальный масштаб, при котором виден данный слой. </translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="157"/>
        <source>Display name</source>
        <translation>Имя в легенде</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="186"/>
        <source>Use this control to set which field is placed at the top level of the Identify Results dialog box.</source>
        <translation>Используйте этот список для выбора поля, помещаемого в верхний уровень дерева в диалоге результатов определения.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="289"/>
        <source>Spatial Reference System</source>
        <translation type="obsolete">Система координат</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="308"/>
        <source>Change</source>
        <translation type="obsolete">Изменить</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="170"/>
        <source>Display field for the Identify Results dialog box</source>
        <translation>Отображаемое поле для диалога результатов определения</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="173"/>
        <source>This sets the display field for the Identify Results dialog box</source>
        <translation>Отображаемое поле для диалога результатов определения</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="176"/>
        <source>Display field</source>
        <translation>Отображаемое поле</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="287"/>
        <source>Subset</source>
        <translation>Подмножество</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="334"/>
        <source>Query Builder</source>
        <translation>Конструктор запросов</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="241"/>
        <source>Spatial Index</source>
        <translation type="obsolete">Пространственный индекс</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="200"/>
        <source>Create Spatial Index</source>
        <translation>Создать пространственный индекс</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="263"/>
        <source>Create</source>
        <translation type="obsolete">Создать</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="345"/>
        <source>Metadata</source>
        <translation>Метаданные</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="374"/>
        <source>Labels</source>
        <translation>Подписи</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="386"/>
        <source>Display labels</source>
        <translation>Включить подписи</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="410"/>
        <source>Actions</source>
        <translation>Действия</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="453"/>
        <source>Restore Default Style</source>
        <translation>Восстановить по умолчанию</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="460"/>
        <source>Save As Default</source>
        <translation>Сохранить по умолчанию</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="467"/>
        <source>Load Style ...</source>
        <translation>Загрузить стиль...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="474"/>
        <source>Save Style ...</source>
        <translation>Сохранить стиль...</translation>
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
        <translation type="unfinished"></translation>
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
        <location filename="../src/ui/qgsvectorsymbologywidgetbase.ui" line="44"/>
        <source>Label</source>
        <translation type="obsolete">Метка</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorsymbologywidgetbase.ui" line="49"/>
        <source>Min</source>
        <translation type="obsolete">Мин</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorsymbologywidgetbase.ui" line="54"/>
        <source>Max</source>
        <translation type="obsolete">Макс</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorsymbologywidgetbase.ui" line="62"/>
        <source>Symbol Classes:</source>
        <translation type="obsolete">Классы знака:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorsymbologywidgetbase.ui" line="77"/>
        <source>Count:</source>
        <translation type="obsolete">Количество:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorsymbologywidgetbase.ui" line="90"/>
        <source>Mode:</source>
        <translation type="obsolete">Режим:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorsymbologywidgetbase.ui" line="100"/>
        <source>Field:</source>
        <translation type="obsolete">Поле:</translation>
    </message>
</context>
<context>
    <name>QgsWFSPlugin</name>
    <message>
        <location filename="../src/plugins/wfs/qgswfsplugin.cpp" line="59"/>
        <source>&amp;Add WFS layer</source>
        <translation>&amp;Добавить WFS-слой</translation>
    </message>
</context>
<context>
    <name>QgsWFSProvider</name>
    <message>
        <location filename="../src/providers/wfs/qgswfsprovider.cpp" line="1391"/>
        <source>unknown</source>
        <translation> неизвестно</translation>
    </message>
    <message>
        <location filename="../src/providers/wfs/qgswfsprovider.cpp" line="1397"/>
        <source>received %1 bytes from %2</source>
        <translation>получено %1 из %2 байт</translation>
    </message>
</context>
<context>
    <name>QgsWFSSourceSelect</name>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselect.cpp" line="260"/>
        <source>Are you sure you want to remove the </source>
        <translation>Вы уверены, что хотите удалить соединение </translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselect.cpp" line="260"/>
        <source> connection and all associated settings?</source>
        <translation> и все связанные с ним параметры?</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselect.cpp" line="261"/>
        <source>Confirm Delete</source>
        <translation>Потвердите удаление</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Yes</source>
        <translation type="obsolete">Да</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>No</source>
        <translation type="obsolete">Нет</translation>
    </message>
</context>
<context>
    <name>QgsWFSSourceSelectBase</name>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="29"/>
        <source>Title</source>
        <translation>Заглавие</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="34"/>
        <source>Name</source>
        <translation>Имя</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="39"/>
        <source>Abstract</source>
        <translation>Описание</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="47"/>
        <source>Coordinate Reference System</source>
        <translation>Система координат</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="85"/>
        <source>Change ...</source>
        <translation>Изменить...</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="95"/>
        <source>Server Connections</source>
        <translation>Соединения с серверами</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="107"/>
        <source>&amp;New</source>
        <translation>&amp;Новое</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="117"/>
        <source>Delete</source>
        <translation>Удалить</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="127"/>
        <source>Edit</source>
        <translation>Изменить</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="153"/>
        <source>C&amp;onnect</source>
        <translation>&amp;Подключить</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselectbase.ui" line="13"/>
        <source>Add WFS Layer from a Server</source>
        <translation>Добавить WFS-слой</translation>
    </message>
</context>
<context>
    <name>QgsWmsProvider</name>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="706"/>
        <source>Tried URL: </source>
        <translation>Используемый URL: </translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="686"/>
        <source>HTTP Exception</source>
        <translation>HTTP-исключение</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="649"/>
        <source>WMS Service Exception</source>
        <translation>Исключение WMS-службы</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1500"/>
        <source>DOM Exception</source>
        <translation>DOM-исключение</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="760"/>
        <source>Could not get WMS capabilities: %1 at line %2 column %3</source>
        <translation>Не удалось получить возможности WMS: %1 в строке %2, столбец %3</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="791"/>
        <source>This is probably due to an incorrect WMS Server URL.</source>
        <translation>Вероятнее всего, адрес WMS-сервера неверен.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="787"/>
        <source>Could not get WMS capabilities in the expected format (DTD): no %1 or %2 found</source>
        <translation>Не удалось получить возможности WMS в ожидаемом формате (DTD): %1 или %2 не найдены</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1502"/>
        <source>Could not get WMS Service Exception at %1: %2 at line %3 column %4</source>
        <translation>Не удалось получить ошибку WMS из %1: %2 в строке %3, столбец %4</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1552"/>
        <source>Request contains a Format not offered by the server.</source>
        <translation>Запрос требует формата, который не поддерживается сервером.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1556"/>
        <source>Request contains a CRS not offered by the server for one or more of the Layers in the request.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1560"/>
        <source>Request contains a SRS not offered by the server for one or more of the Layers in the request.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1565"/>
        <source>GetMap request is for a Layer not offered by the server, or GetFeatureInfo request is for a Layer not shown on the map.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1569"/>
        <source>Request is for a Layer in a Style not offered by the server.</source>
        <translation>Запрос требует слой в стиле, который недоступен на сервере.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1573"/>
        <source>GetFeatureInfo request is applied to a Layer which is not declared queryable.</source>
        <translation>Попытка запроса GetFeatureInfo для слоя, который не поддерживает запросов.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1577"/>
        <source>GetFeatureInfo request contains invalid X or Y value.</source>
        <translation>Запрос GetFeatureInfo содержит недействительные значения X или Y.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1582"/>
        <source>Value of (optional) UpdateSequence parameter in GetCapabilities request is equal to current value of service metadata update sequence number.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1587"/>
        <source>Value of (optional) UpdateSequence parameter in GetCapabilities request is greater than current value of service metadata update sequence number.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1592"/>
        <source>Request does not include a sample dimension value, and the server did not declare a default value for that dimension.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1596"/>
        <source>Request contains an invalid sample dimension value.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1600"/>
        <source>Request is for an optional operation that is not supported by the server.</source>
        <translation>Запрос необязательной операции, которая не поддерживается сервером.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1604"/>
        <source>(Unknown error code from a post-1.3 WMS server)</source>
        <translation>(Неизвестный код ошибки от WMS-сервера &gt; 1.3)</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1607"/>
        <source>The WMS vendor also reported: </source>
        <translation>Дополнительное сообщение WMS-провайдера: </translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1610"/>
        <source>This is probably due to a bug in the QGIS program.  Please report this error.</source>
        <translation>Вероятно, это внутренняя ошибка QGIS.  Пожалуйста, сообщите об этой ошибке.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1794"/>
        <source>Server Properties:</source>
        <translation>Свойства сервера:</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1925"/>
        <source>Property</source>
        <translation>Свойство</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1928"/>
        <source>Value</source>
        <translation>Значение</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1811"/>
        <source>WMS Version</source>
        <translation>Версия WMS</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2057"/>
        <source>Title</source>
        <translation>Заглавие</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2065"/>
        <source>Abstract</source>
        <translation>Описание</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1835"/>
        <source>Keywords</source>
        <translation>Ключевые слова</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1843"/>
        <source>Online Resource</source>
        <translation>Онлайн-ресурс</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1851"/>
        <source>Contact Person</source>
        <translation>Контактное лицо</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1863"/>
        <source>Fees</source>
        <translation>Плата</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1871"/>
        <source>Access Constraints</source>
        <translation>Ограничения доступа</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1879"/>
        <source>Image Formats</source>
        <translation>Форматы изображения</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1887"/>
        <source>Identify Formats</source>
        <translation>Форматы запроса</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1895"/>
        <source>Layer Count</source>
        <translation>Количество слоёв</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1915"/>
        <source>Layer Properties: </source>
        <translation>Свойства слоя: </translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1933"/>
        <source>Selected</source>
        <translation>Выбран</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1990"/>
        <source>Yes</source>
        <translation>Да</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1990"/>
        <source>No</source>
        <translation>Нет</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1942"/>
        <source>Visibility</source>
        <translation>Видимость</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1948"/>
        <source>Visible</source>
        <translation>Видимый</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1949"/>
        <source>Hidden</source>
        <translation>Скрытый</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1950"/>
        <source>n/a</source>
        <translation>н/д</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1971"/>
        <source>Can Identify</source>
        <translation>Можно определять</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1979"/>
        <source>Can be Transparent</source>
        <translation>Может быть прозрачным</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1987"/>
        <source>Can Zoom In</source>
        <translation>Можно увеличивать</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1995"/>
        <source>Cascade Count</source>
        <translation>Количество каскадов</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2003"/>
        <source>Fixed Width</source>
        <translation>Фикс. ширина</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2011"/>
        <source>Fixed Height</source>
        <translation>Фикс. высота</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2019"/>
        <source>WGS 84 Bounding Box</source>
        <translation>Рамка WGS 84</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2029"/>
        <source>Available in CRS</source>
        <translation>Доступен в CRS</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2040"/>
        <source>Available in style</source>
        <translation>Доступен в стиле</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2049"/>
        <source>Name</source>
        <translation>Имя</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2150"/>
        <source>Layer cannot be queried.</source>
        <translation>Не удаётся опросить слой.</translation>
    </message>
</context>
<context>
    <name>QuickPrint</name>
    <message>
        <location filename="" line="0"/>
        <source>Quick Print</source>
        <translation type="obsolete">Быстрая печать</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Replace this with a short description of the what the plugin does</source>
        <translation type="obsolete">Замените это кратким описанием функций модуля</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>&amp;Quick Print</source>
        <translation type="obsolete">&amp;Быстрая печать</translation>
    </message>
</context>
<context>
    <name>QuickPrintGui</name>
    <message>
        <location filename="../src/plugins/quick_print/quickprintgui.cpp" line="129"/>
        <source>Portable Document Format (*.pdf)</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintgui.cpp" line="154"/>
        <source>quickprint</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintgui.cpp" line="155"/>
        <source>Unknown format: </source>
        <translation>Неизвестный формат: </translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source> km</source>
        <translation type="obsolete"> км</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source> mm</source>
        <translation type="obsolete"> мм</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source> cm</source>
        <translation type="obsolete"> см</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source> m</source>
        <translation type="obsolete"> м</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source> miles</source>
        <translation type="obsolete"> миль</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source> mile</source>
        <translation type="obsolete"> миля</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source> inches</source>
        <translation type="obsolete">дюймов</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source> foot</source>
        <translation type="obsolete"> фут</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source> feet</source>
        <translation type="obsolete"> футов</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source> degree</source>
        <translation type="obsolete"> градус</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source> degrees</source>
        <translation type="obsolete"> градусов</translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source> unknown</source>
        <translation type="obsolete"> неизв</translation>
    </message>
</context>
<context>
    <name>QuickPrintGuiBase</name>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="13"/>
        <source>QGIS Quick Print Plugin</source>
        <translation>Модуль быстрой печати QGIS</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="158"/>
        <source>Quick Print</source>
        <translation>Быстрая печать</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="129"/>
        <source>Map Title e.g. ACME inc.</source>
        <translation>Заголовок карты (напр. «ACME inc.»).</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="116"/>
        <source>Map Name e.g. Water Features</source>
        <translation>Имя карты (напр. «Водные объекты»)</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="103"/>
        <source>Copyright</source>
        <translation>Авторское право</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="48"/>
        <source>Output</source>
        <translation>Вывод</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="60"/>
        <source>Use last filename but incremented.</source>
        <translation>Использовать предыдущее имя файла с приращением.</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="67"/>
        <source>last used filename but incremented will be shown here</source>
        <translation>Здесь будет выведено предыдущее имя файла с приращением</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="77"/>
        <source>Prompt for file name</source>
        <translation>Запрашивать имя файла</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="38"/>
        <source>Note: If you want more control over the map layout please use the map composer function in QGIS.</source>
        <translation>Внимание: для полного контроля над макетом карты рекомендуется использовать компоновщик карт.</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintguibase.ui" line="93"/>
        <source>Page Size</source>
        <translation>Размер страницы</translation>
    </message>
</context>
<context>
    <name>QuickPrintPlugin</name>
    <message>
        <location filename="../src/plugins/quick_print/quickprintplugin.cpp" line="75"/>
        <source>Quick Print</source>
        <translation>Быстрая печать</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintplugin.cpp" line="77"/>
        <source>Replace this with a short description of the what the plugin does</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintplugin.cpp" line="82"/>
        <source>&amp;Quick Print</source>
        <translation>&amp;Быстрая печать</translation>
    </message>
</context>
<context>
    <name>RepositoryDetailsDialog</name>
    <message>
        <location filename="../python/plugins/plugin_installer/repository.ui" line="13"/>
        <source>Repository details</source>
        <translation>Данные репозитория</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/repository.ui" line="19"/>
        <source>Name:</source>
        <translation>Имя:</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/repository.ui" line="29"/>
        <source>URL:</source>
        <translation>URL:</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/repository.ui" line="36"/>
        <source>http://</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>SplashScreen</name>
    <message>
        <location filename="" line="0"/>
        <source>Quantum GIS - </source>
        <translation type="obsolete">Quantum GIS - </translation>
    </message>
    <message>
        <location filename="" line="0"/>
        <source>Version </source>
        <translation type="obsolete">Версия</translation>
    </message>
</context>
<context>
    <name>[pluginname]GuiBase</name>
    <message>
        <location filename="../src/plugins/plugin_template/pluginguibase.ui" line="13"/>
        <source>QGIS Plugin Template</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/pluginguibase.ui" line="47"/>
        <source>Plugin Template</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>gpsPage</name>
    <message>
        <location filename="" line="0"/>
        <source>No</source>
        <translation type="obsolete">Нет</translation>
    </message>
</context>
<context>
    <name>pluginname</name>
    <message>
        <location filename="../src/plugins/plugin_template/plugin.cpp" line="75"/>
        <source>Replace this with a short description of the what the plugin does</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugin.cpp" line="73"/>
        <source>[menuitemname]</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugins/plugin_template/plugin.cpp" line="80"/>
        <source>&amp;[menuname]</source>
        <translation>&amp;[menuname]</translation>
    </message>
</context>
</TS>
