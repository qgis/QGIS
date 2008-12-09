<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS><TS version="1.1" language="ru">
<defaultcodec></defaultcodec>
<context>
    <name>CoordinateCapture</name>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapture.cpp" line="142"/>
        <source>Coordinate Capture</source>
        <translation>Захват координат</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapture.cpp" line="87"/>
        <source>Click on the map to view coordinates and capture to clipboard.</source>
        <translation>Вывод координат в месте щелчка и копирование их в буфер обмена.</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapture.cpp" line="92"/>
        <source>&amp;Coordinate Capture</source>
        <translation>&amp;Захват координат</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapture.cpp" line="108"/>
        <source>Click to select the CRS to use for coordinate display</source>
        <translation>Щёлкните для выбора системы координат, используемой для вывода</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapture.cpp" line="117"/>
        <source>Coordinate in your selected CRS</source>
        <translation>Координаты в выбранной системе</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapture.cpp" line="121"/>
        <source>Coordinate in map canvas coordinate reference system</source>
        <translation>Координаты в системе координат проекта</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapture.cpp" line="124"/>
        <source>Copy to clipboard</source>
        <translation>Копировать в буфер обмена</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapture.cpp" line="129"/>
        <source>Click to enable mouse tracking. Click the canvas to stop</source>
        <translation>Щёлкните для активации слежения за курсором. Щёлкните на карте для прекращения слежения</translation>
    </message>
</context>
<context>
    <name>CoordinateCaptureGui</name>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapturegui.cpp" line="55"/>
        <source>Welcome to your automatically generated plugin!</source>
        <translation>Welcome to your automatically generated plugin!</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapturegui.cpp" line="56"/>
        <source>This is just a starting point. You now need to modify the code to make it do something useful....read on for a more information to get yourself started.</source>
        <translation>This is just a starting point. You now need to modify the code to make it do something useful....read on for a more information to get yourself started.</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapturegui.cpp" line="57"/>
        <source>Documentation:</source>
        <translation>Documentation:</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapturegui.cpp" line="58"/>
        <source>You really need to read the QGIS API Documentation now at:</source>
        <translation>You really need to read the QGIS API Documentation now at:</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapturegui.cpp" line="59"/>
        <source>In particular look at the following classes:</source>
        <translation>In particular look at the following classes:</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapturegui.cpp" line="62"/>
        <source>QgsPlugin is an ABC that defines required behaviour your plugin must provide. See below for more details.</source>
        <translation>QgsPlugin is an ABC that defines required behaviour your plugin must provide. See below for more details.</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapturegui.cpp" line="63"/>
        <source>What are all the files in my generated plugin directory for?</source>
        <translation>What are all the files in my generated plugin directory for?</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapturegui.cpp" line="64"/>
        <source>This is the generated CMake file that builds the plugin. You should add you application specific dependencies and source files to this file.</source>
        <translation>This is the generated CMake file that builds the plugin. You should add you application specific dependencies and source files to this file.</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapturegui.cpp" line="65"/>
        <source>This is the class that provides the &apos;glue&apos; between your custom application logic and the QGIS application. You will see that a number of methods are already implemented for you - including some examples of how to add a raster or vector layer to the main application map canvas. This class is a concrete instance of the QgisPlugin interface which defines required behaviour for a plugin. In particular, a plugin has a number of static methods and members so that the QgsPluginManager and plugin loader logic can identify each plugin, create an appropriate menu entry for it etc. Note there is nothing stopping you creating multiple toolbar icons and menu entries for a single plugin. By default though a single menu entry and toolbar button is created and its pre-configured to call the run() method in this class when selected. This default implementation provided for you by the plugin builder is well documented, so please refer to the code for further advice.</source>
        <translation>This is the class that provides the &apos;glue&apos; between your custom application logic and the QGIS application. You will see that a number of methods are already implemented for you - including some examples of how to add a raster or vector layer to the main application map canvas. This class is a concrete instance of the QgisPlugin interface which defines required behaviour for a plugin. In particular, a plugin has a number of static methods and members so that the QgsPluginManager and plugin loader logic can identify each plugin, create an appropriate menu entry for it etc. Note there is nothing stopping you creating multiple toolbar icons and menu entries for a single plugin. By default though a single menu entry and toolbar button is created and its pre-configured to call the run() method in this class when selected. This default implementation provided for you by the plugin builder is well documented, so please refer to the code for further advice.</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapturegui.cpp" line="66"/>
        <source>This is a Qt designer &apos;ui&apos; file. It defines the look of the default plugin dialog without implementing any application logic. You can modify this form to suite your needs or completely remove it if your plugin does not need to display a user form (e.g. for custom MapTools).</source>
        <translation>This is a Qt designer &apos;ui&apos; file. It defines the look of the default plugin dialog without implementing any application logic. You can modify this form to suite your needs or completely remove it if your plugin does not need to display a user form (e.g. for custom MapTools).</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapturegui.cpp" line="67"/>
        <source>This is the concrete class where application logic for the above mentioned dialog should go. The world is your oyster here really....</source>
        <translation>This is the concrete class where application logic for the above mentioned dialog should go. The world is your oyster here really....</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapturegui.cpp" line="68"/>
        <source>This is the Qt4 resources file for your plugin. The Makefile generated for your plugin is all set up to compile the resource file so all you need to do is add your additional icons etc using the simple xml file format. Note the namespace used for all your resources e.g. (&apos;:/Homann/&apos;). It is important to use this prefix for all your resources. We suggest you include any other images and run time data in this resurce file too.</source>
        <translation>This is the Qt4 resources file for your plugin. The Makefile generated for your plugin is all set up to compile the resource file so all you need to do is add your additional icons etc using the simple xml file format. Note the namespace used for all your resources e.g. (&apos;:/Homann/&apos;). It is important to use this prefix for all your resources. We suggest you include any other images and run time data in this resurce file too.</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapturegui.cpp" line="69"/>
        <source>This is the icon that will be used for your plugin menu entry and toolbar icon. Simply replace this icon with your own icon to make your plugin disctinctive from the rest.</source>
        <translation>This is the icon that will be used for your plugin menu entry and toolbar icon. Simply replace this icon with your own icon to make your plugin disctinctive from the rest.</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapturegui.cpp" line="70"/>
        <source>This file contains the documentation you are reading now!</source>
        <translation>This file contains the documentation you are reading now!</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapturegui.cpp" line="71"/>
        <source>Getting developer help:</source>
        <translation>Getting developer help:</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapturegui.cpp" line="72"/>
        <source>For Questions and Comments regarding the plugin builder template and creating your features in QGIS using the plugin interface please contact us via:</source>
        <translation>For Questions and Comments regarding the plugin builder template and creating your features in QGIS using the plugin interface please contact us via:</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapturegui.cpp" line="73"/>
        <source>&lt;li&gt; the QGIS developers mailing list, or &lt;/li&gt;&lt;li&gt; IRC (#qgis on freenode.net)&lt;/li&gt;</source>
        <translation>&lt;li&gt; the QGIS developers mailing list, or &lt;/li&gt;&lt;li&gt; IRC (#qgis on freenode.net)&lt;/li&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapturegui.cpp" line="74"/>
        <source>QGIS is distributed under the Gnu Public License. If you create a useful plugin please consider contributing it back to the community.</source>
        <translation>QGIS is distributed under the Gnu Public License. If you create a useful plugin please consider contributing it back to the community.</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapturegui.cpp" line="75"/>
        <source>Have fun and thank you for choosing QGIS.</source>
        <translation>Have fun and thank you for choosing QGIS.</translation>
    </message>
</context>
<context>
    <name>CoordinateCaptureGuiBase</name>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecaptureguibase.ui" line="13"/>
        <source>QGIS Plugin Template</source>
        <translation>Шаблон модуля QGIS</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecaptureguibase.ui" line="47"/>
        <source>Plugin Template</source>
        <translation>Шаблон модуля</translation>
    </message>
</context>
<context>
    <name>Dialog</name>
    <message>
        <location filename="../src/plugins/ogr_converter/dialog.cpp" line="115"/>
        <source>Connect</source>
        <translation>Подключиться</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/dialog.cpp" line="119"/>
        <source>Browse</source>
        <translation>Обзор</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/dialog.cpp" line="424"/>
        <source>OGR Converter</source>
        <translation>Преобразователь OGR</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/dialog.cpp" line="421"/>
        <source>Could not establish connection to: &apos;</source>
        <translation>Не удалось установить соединение с:&apos;</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/dialog.cpp" line="216"/>
        <source>Open OGR file</source>
        <translation>Открыть файл в формате OGR</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/dialog.cpp" line="435"/>
        <source>OGR File Data Source (*.*)</source>
        <translation>Файловый источник данных OGR (*.*)</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/dialog.cpp" line="226"/>
        <source>Open Directory</source>
        <translation>Открыть каталог</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/dialog.cpp" line="241"/>
        <source>Input OGR dataset is missing!</source>
        <translation>Не указан исходный набор данных OGR!</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/dialog.cpp" line="248"/>
        <source>Input OGR layer name is missing!</source>
        <translation>Не указано имя слоя OGR!</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/dialog.cpp" line="262"/>
        <source>Target OGR format not selected!</source>
        <translation>Не выбран целевой формат OGR!</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/dialog.cpp" line="269"/>
        <source>Output OGR dataset is missing!</source>
        <translation>Не указан целевой набор данных OGR!</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/dialog.cpp" line="276"/>
        <source>Output OGR layer name is missing!</source>
        <translation>Не указано имя целевого слоя OGR!</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/dialog.cpp" line="298"/>
        <source>Successfully translated layer &apos;</source>
        <translation>Успешно преобразован слой &apos;</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/dialog.cpp" line="303"/>
        <source>Failed to translate layer &apos;</source>
        <translation>Не удалось преобразовать слой &apos;</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/dialog.cpp" line="417"/>
        <source>Successfully connected to: &apos;</source>
        <translation>Успешное соединение с &apos;</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/dialog.cpp" line="434"/>
        <source>Choose a file name to save to</source>
        <translation>Выберите имя сохраняемого файла</translation>
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
    <name>OgrConverterGuiBase</name>
    <message>
        <location filename="../src/plugins/ogr_converter/ogrconverterguibase.ui" line="25"/>
        <source>OGR Layer Converter</source>
        <translation>Преобразователь слоёв OGR</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/ogrconverterguibase.ui" line="40"/>
        <source>Source</source>
        <translation>Источник</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/ogrconverterguibase.ui" line="183"/>
        <source>Format</source>
        <translation>Формат</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/ogrconverterguibase.ui" line="88"/>
        <source>File</source>
        <translation>Файл</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/ogrconverterguibase.ui" line="95"/>
        <source>Directory</source>
        <translation>Каталог</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/ogrconverterguibase.ui" line="102"/>
        <source>Remote source</source>
        <translation>Удалённый источник</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/ogrconverterguibase.ui" line="206"/>
        <source>Dataset</source>
        <translation>Набор данных</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/ogrconverterguibase.ui" line="226"/>
        <source>Browse</source>
        <translation>Обзор</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/ogrconverterguibase.ui" line="239"/>
        <source>Layer</source>
        <translation>Слой</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/ogrconverterguibase.ui" line="171"/>
        <source>Target</source>
        <translation>Приёмник</translation>
    </message>
</context>
<context>
    <name>OgrPlugin</name>
    <message>
        <location filename="../src/plugins/ogr_converter/plugin.cpp" line="57"/>
        <source>Run OGR Layer Converter</source>
        <translation>Преобразователь слоёв OGR</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/plugin.cpp" line="67"/>
        <source>OG&amp;R Converter</source>
        <translation>Преобразователь OG&amp;R</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/plugin.cpp" line="60"/>
        <source>Translates vector layers between formats supported by OGR library</source>
        <translation>Преобразует векторные слои в форматы, поддерживаемые библиотекой OGR</translation>
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
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="935"/>
        <source>Load layer properties from style file (.qml)</source>
        <translation>Загрузить свойства слоя из файла стиля (.qml)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="1000"/>
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
        <location filename="../src/core/qgsproviderregistry.cpp" line="253"/>
        <source>No data provider plugins are available. No vector layers can be loaded</source>
        <translation>Недоступны модули источников данных. Загрузка векторных слоёв невозможна</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3076"/>
        <source>QGis files (*.qgs)</source>
        <translation>Файлы QGIS (*.qgs)</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="770"/>
        <source> at line </source>
        <translation> в строке </translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="771"/>
        <source> column </source>
        <translation>, столбец </translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="777"/>
        <source> for file </source>
        <translation> в файле </translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="931"/>
        <source>Unable to save to file </source>
        <translation>Не удалось сохранить в файл  </translation>
    </message>
    <message>
        <location filename="../src/core/qgssearchtreenode.cpp" line="287"/>
        <source>Referenced column wasn&apos;t found: </source>
        <translation>Упоминаемое поле не найдено: </translation>
    </message>
    <message>
        <location filename="../src/core/qgssearchtreenode.cpp" line="291"/>
        <source>Division by zero.</source>
        <translation>Деление на ноль.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolselect.cpp" line="71"/>
        <source>No active layer</source>
        <translation>Нет активного слоя</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="154"/>
        <source>Band</source>
        <translation>Канал</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="381"/>
        <source>action</source>
        <translation>действие</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="389"/>
        <source> features found</source>
        <translation> объектов найдено</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="393"/>
        <source> 1 feature found</source>
        <translation> найден 1 объект</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="399"/>
        <source>No features found</source>
        <translation>Объектов не найдено</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="399"/>
        <source>No features were found in the active layer at the point you clicked</source>
        <translation>В активном слое не найдено объектов в точке, на которой был произведён щелчок</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="424"/>
        <source>Could not identify objects on</source>
        <translation>Не удалось определить объекты на</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="424"/>
        <source>because</source>
        <translation>потому что</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="71"/>
        <source>New centroid</source>
        <translation>Новый центроид</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="238"/>
        <source>New point</source>
        <translation>Новая точка</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="135"/>
        <source>New vertex</source>
        <translation>Новая вершина</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="238"/>
        <source>Undo last point</source>
        <translation>Отменить последнюю точку</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="238"/>
        <source>Close line</source>
        <translation>Завершить линию</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="601"/>
        <source>Select vertex</source>
        <translation>Выбрать вершину</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="317"/>
        <source>Select new position</source>
        <translation>Выбрать новую позицию</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="471"/>
        <source>Select line segment</source>
        <translation>Выбрать сегмент линии</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="456"/>
        <source>New vertex position</source>
        <translation>Новая позиция вершины</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="456"/>
        <source>Release</source>
        <translation>Освободить</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="586"/>
        <source>Delete vertex</source>
        <translation>Удалить вершину</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="586"/>
        <source>Release vertex</source>
        <translation>Освободить вершину</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="870"/>
        <source>Select element</source>
        <translation>Выбрать элемент</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="662"/>
        <source>New location</source>
        <translation>Новое положение</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="747"/>
        <source>Release selected</source>
        <translation>Освободить выделение</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="747"/>
        <source>Delete selected / select next</source>
        <translation>Удалить выделение / выбрать следующий</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="816"/>
        <source>Select position on line</source>
        <translation>Выбрать позицию на линии</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="837"/>
        <source>Split the line</source>
        <translation>Разделить линию</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="837"/>
        <source>Release the line</source>
        <translation>Освободить линию</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedittools.cpp" line="853"/>
        <source>Select point on line</source>
        <translation>Выбрать точку на линии</translation>
    </message>
    <message>
        <location filename="../src/core/qgslabelattributes.cpp" line="61"/>
        <source>Label</source>
        <translation>Подпись</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="347"/>
        <source>Length</source>
        <translation>Длина</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="365"/>
        <source>Area</source>
        <translation>Площадь</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="770"/>
        <source>Project file read error: </source>
        <translation>Ошибка чтения файла проекта: </translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsleastsquares.cpp" line="34"/>
        <source>Fit to a linear transform requires at least 2 points.</source>
        <translation>Для привязки с линейным преобразованием необходимо как минимум 2 точки.</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsleastsquares.cpp" line="76"/>
        <source>Fit to a Helmert transform requires at least 2 points.</source>
        <translation>Для привязки с преобразованием Гельмерта необходимо как минимум 2 точки.</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgsleastsquares.cpp" line="132"/>
        <source>Fit to an affine transform requires at least 4 points.</source>
        <translation>Для привязки с аффинным преобразованием необходимо как минимум 4 точки.</translation>
    </message>
    <message>
        <location filename="../src/providers/gpx/gpsdata.cpp" line="375"/>
        <source>Couldn&apos;t open the data source: </source>
        <translation>Не удалось открыть источник данных: </translation>
    </message>
    <message>
        <location filename="../src/providers/gpx/gpsdata.cpp" line="399"/>
        <source>Parse error at line </source>
        <translation>Ошибка разбора в строке </translation>
    </message>
    <message>
        <location filename="../src/providers/gpx/qgsgpxprovider.cpp" line="56"/>
        <source>GPS eXchange format provider</source>
        <translation>Источник данных GPS eXchange</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="310"/>
        <source>Caught a coordinate system exception while trying to transform a point. Unable to calculate line length.</source>
        <translation>Перехвачено исключение системы координат при попытке преобразования точки. Не удалось определить длину линии.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="401"/>
        <source>Caught a coordinate system exception while trying to transform a point. Unable to calculate polygon area.</source>
        <translation>Перехвачено исключение системы координат при попытке преобразования точки. Не удалось определить площадь полигона.</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="162"/>
        <source>GRASS plugin</source>
        <translation>GRASS</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="132"/>
        <source>QGIS couldn&apos;t find your GRASS installation.
Would you like to specify path (GISBASE) to your GRASS installation?</source>
        <translation>Не удалось найти установленную систему GRASS.
Вы хотите указать путь установки GRASS (GISBASE)?</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="146"/>
        <source>Choose GRASS installation path (GISBASE)</source>
        <translation>Выберите путь установки GRASS (GISBASE)</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="163"/>
        <source>GRASS data won&apos;t be available if GISBASE is not specified.</source>
        <translation>Данные GRASS будут недоступны, если значение GISBASE не задано.</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="49"/>
        <source>CopyrightLabel</source>
        <translation>Знак авторского права</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="50"/>
        <source>Draws copyright information</source>
        <translation>Вывод знака авторского права</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfsplugin.cpp" line="31"/>
        <source>Version 0.1</source>
        <translation>Версия 0.1</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="42"/>
        <source>Version 0.2</source>
        <translation>Версия 0.2</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="43"/>
        <source>Loads and displays delimited text files containing x,y coordinates</source>
        <translation>Загружает и выводит текстовые файлы, содержащие координаты X,Y</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="159"/>
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
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="52"/>
        <source>GPS Tools</source>
        <translation>Инструменты GPS</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="54"/>
        <source>Tools for loading and importing GPS data</source>
        <translation>Инструменты для загрузки и импорта данных GPS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="830"/>
        <source>GRASS</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="836"/>
        <source>GRASS layer</source>
        <translation>Поддержка GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugin.cpp" line="41"/>
        <source>Graticule Creator</source>
        <translation>Конструктор сетки</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugin.cpp" line="42"/>
        <source>Builds a graticule</source>
        <translation>Конструктор сетки</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="58"/>
        <source>NorthArrow</source>
        <translation>Указатель «север-юг»</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="59"/>
        <source>Displays a north arrow overlayed onto the map</source>
        <translation>Вывод указателя «север-юг»</translation>
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
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="60"/>
        <source>ScaleBar</source>
        <translation>Масштабная линейка</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="61"/>
        <source>Draws a scale bar</source>
        <translation>Вывод масштабной линейки</translation>
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
        <translation>Добавляет возможность загрузки слоёв WFS</translation>
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
        <location filename="../src/app/qgsmaptoolselect.cpp" line="74"/>
        <source>To select features, you must choose a vector layer by clicking on its name in the legend</source>
        <translation>Для выделения объектов необходимо выбрать векторный слой щелчком мыши на его имени в легенде</translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="199"/>
        <source>Python error</source>
        <translation>Ошибка Python</translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="497"/>
        <source>Couldn&apos;t load plugin </source>
        <translation>Не удалось загрузить модуль </translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="501"/>
        <source> due an error when calling its classFactory() method</source>
        <translation> из-за ошибки вызова его метода classFactory()</translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="505"/>
        <source> due an error when calling its initGui() method</source>
        <translation> из-за ошибки вызова его метода initGui()</translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="517"/>
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
        <translation>Не удалось применить инструмент «захватить точку» в этом векторном слое</translation>
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
        <translation>Не удалось применить инструмент «захватить линию» в этом векторном слое</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="208"/>
        <source>Cannot apply the &apos;capture polygon&apos; tool on this vector layer</source>
        <translation>Не удалось применить инструмент «захватить полигон» в этом векторном слое</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="435"/>
        <source>Error</source>
        <translation>Ошибка</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="420"/>
        <source>Cannot add feature. Unknown WKB type</source>
        <translation>Не удалось добавить объект, неизвестный тип WKB</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptooladdisland.cpp" line="134"/>
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
        <translation>Вставляемое кольцо не является допустимой геометрией</translation>
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
        <location filename="../src/core/qgsdistancearea.cpp" line="647"/>
        <source> km2</source>
        <translation> км2</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="652"/>
        <source> ha</source>
        <translation> га</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="657"/>
        <source> m2</source>
        <translation> м2</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="685"/>
        <source> m</source>
        <translation> м</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="670"/>
        <source> km</source>
        <translation> км</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="675"/>
        <source> mm</source>
        <translation> мм</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="680"/>
        <source> cm</source>
        <translation> см</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="694"/>
        <source> sq mile</source>
        <translation> кв. миль</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="699"/>
        <source> sq ft</source>
        <translation> кв. футов</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="706"/>
        <source> mile</source>
        <translation> миль</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="712"/>
        <source> foot</source>
        <translation> фут</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="714"/>
        <source> feet</source>
        <translation> футов</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="721"/>
        <source> sq.deg.</source>
        <translation> кв. град.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="726"/>
        <source> degree</source>
        <translation> градус</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="728"/>
        <source> degrees</source>
        <translation> градусов</translation>
    </message>
    <message>
        <location filename="../src/core/qgsdistancearea.cpp" line="732"/>
        <source> unknown</source>
        <translation> неизв</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="287"/>
        <source>Received %1 of %2 bytes</source>
        <translation>Получено %1 из %2 байт</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="293"/>
        <source>Received %1 bytes (total unknown)</source>
        <translation>Получено %1 байт (размер неизвестен)</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="405"/>
        <source>Not connected</source>
        <translation>Нет соединения</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="411"/>
        <source>Looking up &apos;%1&apos;</source>
        <translation>Поиск «%1»</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="418"/>
        <source>Connecting to &apos;%1&apos;</source>
        <translation>Соединение с «%1»</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="425"/>
        <source>Sending request &apos;%1&apos;</source>
        <translation>Отправка запроса «%1»</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="432"/>
        <source>Receiving reply</source>
        <translation>Получение ответа</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="438"/>
        <source>Response is complete</source>
        <translation>Ответ получен</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="444"/>
        <source>Closing down connection</source>
        <translation>Соединение закрыто</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="755"/>
        <source>Unable to open </source>
        <translation>Не удалось открыть </translation>
    </message>
    <message>
        <location filename="../src/core/qgssearchtreenode.cpp" line="253"/>
        <source>Regular expressions on numeric values don&apos;t make sense. Use comparison instead.</source>
        <translation>Регулярные выражения не имеют смысла для числовых значений. Используйте сравнение.</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="46"/>
        <source>Geoprocessing functions for working with PostgreSQL/PostGIS layers</source>
        <translation>Функции пространственной обработки для слоёв PostgreSQL/PostGIS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="117"/>
        <source>Location: </source>
        <comment>Metadata in GRASS Browser</comment>
        <translation>Район: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="117"/>
        <source>&lt;br&gt;Mapset: </source>
        <comment>Metadata in GRASS Browser</comment>
        <translation>&lt;br&gt;Набор: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="121"/>
        <source>Location: </source>
        <translation>Район: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="121"/>
        <source>&lt;br&gt;Mapset: </source>
        <translation>&lt;br&gt;Набор: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="126"/>
        <source>&lt;b&gt;Raster&lt;/b&gt;</source>
        <translation>&lt;b&gt;Растр&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="135"/>
        <source>Cannot open raster header</source>
        <translation>Не удалось открыть заголовок растра</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="139"/>
        <source>Rows</source>
        <translation>Строк</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="140"/>
        <source>Columns</source>
        <translation>Столбцов</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="141"/>
        <source>N-S resolution</source>
        <translation>Вертикальное разрешение</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="142"/>
        <source>E-W resolution</source>
        <translation>Горизонтальное разрешение</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="280"/>
        <source>North</source>
        <translation>Север</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="282"/>
        <source>South</source>
        <translation>Юг</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="284"/>
        <source>East</source>
        <translation>Восток</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="286"/>
        <source>West</source>
        <translation>Запад</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="169"/>
        <source>Format</source>
        <translation>Формат</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="180"/>
        <source>Minimum value</source>
        <translation>Мин. значение</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="181"/>
        <source>Maximum value</source>
        <translation>Макс. значение</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="192"/>
        <source>Data source</source>
        <translation>Источник данных</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="197"/>
        <source>Data description</source>
        <translation>Описание данных</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="206"/>
        <source>Comments</source>
        <translation>Комментарии</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="221"/>
        <source>Categories</source>
        <translation>Категории</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="328"/>
        <source>&lt;b&gt;Vector&lt;/b&gt;</source>
        <translation>&lt;b&gt;Вектор&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="254"/>
        <source>Points</source>
        <translation>Точки</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="255"/>
        <source>Lines</source>
        <translation>Линии</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="256"/>
        <source>Boundaries</source>
        <translation>Границы</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="257"/>
        <source>Centroids</source>
        <translation>Центроиды</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="260"/>
        <source>Faces</source>
        <translation>Грани</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="261"/>
        <source>Kernels</source>
        <translation>Ядра</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="264"/>
        <source>Areas</source>
        <translation>Площади</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="265"/>
        <source>Islands</source>
        <translation>Острова</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="289"/>
        <source>Top</source>
        <translation>Верх</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="290"/>
        <source>Bottom</source>
        <translation>Низ</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="293"/>
        <source>yes</source>
        <translation>да</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="293"/>
        <source>no</source>
        <translation>нет</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="300"/>
        <source>History&lt;br&gt;</source>
        <translation>История&lt;br&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="329"/>
        <source>&lt;b&gt;Layer&lt;/b&gt;</source>
        <translation>&lt;b&gt;Слой&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="348"/>
        <source>Features</source>
        <translation>Объектов</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="357"/>
        <source>Driver</source>
        <translation>Драйвер</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="358"/>
        <source>Database</source>
        <translation>База данных</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="359"/>
        <source>Table</source>
        <translation>Таблица</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodel.cpp" line="360"/>
        <source>Key column</source>
        <translation>Ключевое поле</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="436"/>
        <source>GISBASE is not set.</source>
        <translation>GISBASE не задана.</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="441"/>
        <source> is not a GRASS mapset.</source>
        <translation> не является набором GRASS.</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="475"/>
        <source>Cannot start </source>
        <translation>Не удалось запустить </translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="485"/>
        <source>Mapset is already in use.</source>
        <translation>Набор уже используется.</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="500"/>
        <source>Temporary directory </source>
        <translation>Временный каталог </translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="500"/>
        <source> exist but is not writable</source>
        <translation> существует, но закрыт для записи</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="506"/>
        <source>Cannot create temporary directory </source>
        <translation>Не удаётся создать временный каталог </translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="520"/>
        <source>Cannot create </source>
        <translation>Не удаётся создать </translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="595"/>
        <source>Cannot remove mapset lock: </source>
        <translation>Не удалось снять блокировку набора: </translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="1016"/>
        <source>Warning</source>
        <translation>Внимание</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="964"/>
        <source>Cannot read raster map region</source>
        <translation>Не удаётся прочесть границы растровой карты</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="981"/>
        <source>Cannot read vector map region</source>
        <translation>Не удаётся прочесть границы векторной карты</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="1017"/>
        <source>Cannot read region</source>
        <translation>Не удаётся прочесть регион</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2496"/>
        <source>Where is &apos;</source>
        <translation>Где искать &apos;</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2496"/>
        <source>original location: </source>
        <translation>оригинальное местоположение: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="121"/>
        <source>To identify features, you must choose an active layer by clicking on its name in the legend</source>
        <translation>Чтобы определить объекты, следует выбрать активный слой щелчком на имени слоя в легенде</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="45"/>
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
        <location filename="../src/app/qgsmaptooladdfeature.cpp" line="435"/>
        <source>Could not remove polygon intersection</source>
        <translation>Не удалось удалить пересечение полигонов</translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="585"/>
        <source>The directory containing your dataset needs to be writeable!</source>
        <translation>Необходимы права на запись в каталог, содержащий ваши данные!</translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="599"/>
        <source>Created default style file as </source>
        <translation>Файл стиля по умолчанию создан в </translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="604"/>
        <source>ERROR: Failed to created default style file as %1 Check file permissions and retry.</source>
        <translation>ОШИБКА: Не удалось создать файл стиля по умолчанию в %1. Проверьте права доступа к файлу и попробуйте ещё раз.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="940"/>
        <source> is not writeable.</source>
        <translation> не является записываемым файлом.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="940"/>
        <source>Please adjust permissions (if possible) and try again.</source>
        <translation>Пожалуйста, исправьте права доступа (если возможно) и попробуйте ещё раз.</translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsgridfilewriter.cpp" line="65"/>
        <source>Abort</source>
        <translation>Отменить</translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsinterpolationplugin.cpp" line="24"/>
        <source>Version 0.001</source>
        <translation>Версия 0.001</translation>
    </message>
    <message>
        <location filename="../src/providers/grass/qgsgrass.cpp" line="386"/>
        <source>Uncatched fatal GRASS error</source>
        <translation>Необработанная фатальная ошибка GRASS</translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="64"/>
        <source>Couldn&apos;t load SIP module.</source>
        <translation>Не удалось загрузить модуль SIP.</translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="79"/>
        <source>Python support will be disabled.</source>
        <translation>Поддержка Python будет выключена.</translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="72"/>
        <source>Couldn&apos;t load PyQt4.</source>
        <translation>Не удалось загрузить PyQt4.</translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="79"/>
        <source>Couldn&apos;t load PyQGIS.</source>
        <translation>Не удалось загрузить PyQGIS.</translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="90"/>
        <source>An error has occured while executing Python code:</source>
        <translation>При выполнении Python-кода возникла ошибка:</translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="194"/>
        <source>Python version:</source>
        <translation>Версия Python:</translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="195"/>
        <source>Python path:</source>
        <translation>Путь поиска Python:</translation>
    </message>
    <message>
        <location filename="../src/python/qgspythonutilsimpl.cpp" line="185"/>
        <source>An error occured during execution of following code:</source>
        <translation>Возникла ошибка при выполнении следующего кода:</translation>
    </message>
    <message>
        <location filename="../src/core/composer/qgscomposerlegend.cpp" line="27"/>
        <source>Legend</source>
        <translation>Легенда</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapture.cpp" line="50"/>
        <source>Coordinate Capture</source>
        <translation>Захват координат</translation>
    </message>
    <message>
        <location filename="../src/plugins/coordinate_capture/coordinatecapture.cpp" line="51"/>
        <source>Capture mouse coordinates in different CRS</source>
        <translation>Захват координат курсора в различных системах координат</translation>
    </message>
    <message>
        <location filename="../src/plugins/dxf2shp_converter/dxf2shpconverter.cpp" line="37"/>
        <source>Dxf2Shp Converter</source>
        <translation>Преобразователь Dxf2Shp</translation>
    </message>
    <message>
        <location filename="../src/plugins/dxf2shp_converter/dxf2shpconverter.cpp" line="39"/>
        <source>Converts from dxf to shp file format</source>
        <translation>Преобразование файлов формата dxf в shape-файлы</translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsgridfilewriter.cpp" line="65"/>
        <source>Interpolating...</source>
        <translation>Интерполяция...</translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsinterpolationplugin.cpp" line="22"/>
        <source>Interpolation plugin</source>
        <translation>Модуль интерполяции</translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsinterpolationplugin.cpp" line="23"/>
        <source>A plugin for interpolation based on vertices of a vector layer</source>
        <translation>Модуль интерполяции данных по вершинам в векторном слое</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/plugin.cpp" line="33"/>
        <source>OGR Layer Converter</source>
        <translation>Преобразователь слоёв OGR</translation>
    </message>
    <message>
        <location filename="../src/plugins/ogr_converter/plugin.cpp" line="34"/>
        <source>Translates vector layers between formats supported by OGR library</source>
        <translation>Преобразует векторные слои в форматы, поддерживаемые библиотекой OGR</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolselect.cpp" line="133"/>
        <source>CRS Exception</source>
        <translation>CRS-исключение</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolselect.cpp" line="134"/>
        <source>Selection extends beyond layer&apos;s coordinate system.</source>
        <translation>Выделение выходит за границы координатной системы слоя.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="545"/>
        <source>Loading style file </source>
        <translation>Не удалось загрузить файл стиля</translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="545"/>
        <source> failed because:</source>
        <translation>по причине:</translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="573"/>
        <source>Could not save symbology because:</source>
        <translation>Не удалось сохранить символику по причине:</translation>
    </message>
    <message>
        <location filename="../src/core/qgsproject.cpp" line="1027"/>
        <source>Unable to save to file. Your project may be corrupted on disk. Try clearing some space on the volume and check file permissions before pressing save again.</source>
        <translation>Не удалось сохранить файл. Файл проекта на диске может быть испорчен. Попробуйте освободить дисковое пространство и проверить права доступа, прежде чем вы попытаетесь сохранить проект повторно.</translation>
    </message>
    <message>
        <location filename="../src/app/qgspluginregistry.cpp" line="289"/>
        <source>Error Loading Plugin</source>
        <translation>Ошибка загрузки модуля</translation>
    </message>
    <message>
        <location filename="../src/app/qgspluginregistry.cpp" line="291"/>
        <source>There was an error loading a plugin.The following diagnostic information may help the QGIS developers resolve the issue:
%1.</source>
        <translation>При загрузке модуля возникла ошибка. Нижеследующая информация может помочь разработчикам QGIS решить эту проблему:
%1.</translation>
    </message>
    <message>
        <location filename="../src/app/qgspluginregistry.cpp" line="411"/>
        <source>Error when reading metadata of plugin </source>
        <translation>Ошибка чтения метаданных модуля </translation>
    </message>
</context>
<context>
    <name>QgisApp</name>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="370"/>
        <source>Quantum GIS - </source>
        <translation>Quantum GIS — </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1801"/>
        <source>Version</source>
        <translation>Версия</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2154"/>
        <source>is not a valid or recognized data source</source>
        <translation>не является действительным источником данных</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5160"/>
        <source>Invalid Data Source</source>
        <translation>Недопустимый источник данных</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3628"/>
        <source>No Layer Selected</source>
        <translation>Слой не выбран</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4167"/>
        <source>There is a new version of QGIS available</source>
        <translation>Доступна новая версия QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4173"/>
        <source>You are running a development version of QGIS</source>
        <translation>Вы используете разрабатываемую версию QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4177"/>
        <source>You are running the current version of QGIS</source>
        <translation>Вы используете последнюю версию QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4182"/>
        <source>Would you like more information?</source>
        <translation>Вы хотите получить дополнительную информацию?</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4231"/>
        <source>QGIS Version Information</source>
        <translation>Информация о версии QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4203"/>
        <source>Unable to get current version information from server</source>
        <translation>Не удалось получить информацию о версии с сервера</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4217"/>
        <source>Connection refused - server may be down</source>
        <translation>В соединении отказано — вероятно, сервер недоступен</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4220"/>
        <source>QGIS server was not found</source>
        <translation>Сервер QGIS не найден</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2247"/>
        <source>Invalid Layer</source>
        <translation>Недействительный слой</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2247"/>
        <source>%1 is an invalid layer and cannot be loaded.</source>
        <translation>Слой %1 не является действительным и не может быть загружен.</translation>
    </message>
    <message>
        <source>Error Loading Plugin</source>
        <translation type="obsolete">Ошибка загрузки модуля</translation>
    </message>
    <message>
        <source>There was an error loading %1.</source>
        <translation type="obsolete">При загрузке модуля %1 возникла ошибка.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3358"/>
        <source>Saved map image to</source>
        <translation>Сохранить снимок карты в</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4669"/>
        <source>Extents: </source>
        <translation>Границы: </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3657"/>
        <source>Problem deleting features</source>
        <translation>Ошибка удаления объектов</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3658"/>
        <source>A problem occured during deletion of features</source>
        <translation>При удалении объектов возникла ошибка</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3636"/>
        <source>No Vector Layer Selected</source>
        <translation>Не выбран векторный слой</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3637"/>
        <source>Deleting features only works on vector layers</source>
        <translation>Удаление объектов работает только для векторных слоёв</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3629"/>
        <source>To delete features, you must select a vector layer in the legend</source>
        <translation>Для удаления объектов, следует выбрать в легенде векторный слой</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1644"/>
        <source>Map legend that displays all the layers currently on the map canvas. Click on the check box to turn a layer on or off. Double click on a layer in the legend to customize its appearance and set other properties.</source>
        <translation>Легенда карты, в которой перечислены все слои отображаемой карты. Щёлкните на флажке, чтобы переключить видимость соответствующего слоя. Дважды щёлкните на имени слоя, чтобы задать его отображение и другие свойства.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1584"/>
        <source>Map overview canvas. This canvas can be used to display a locator map that shows the current extent of the map canvas. The current extent is shown as a red rectangle. Any layer on the map can be added to the overview canvas.</source>
        <translation>Область обзора карты. Данная область используется для вывода обзорной карты, на которой виден текущий охват области карты QGIS в виде красного прямоугольника. Любой слой карты может быть добавлен в область обзора.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1536"/>
        <source>Map canvas. This is where raster and vector layers are displayed when added to the map</source>
        <translation>Область карты. Вывод растровых и векторных слоёв осуществляется в данную область </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1155"/>
        <source>&amp;Plugins</source>
        <translation>&amp;Модули</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1300"/>
        <source>Progress bar that displays the status of rendering layers and other time-intensive operations</source>
        <translation>Индикатор хода процесса отрисовки слоёв и других долговременных операций</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1350"/>
        <source>Displays the current map scale</source>
        <translation>Показывает текущий масштаб карты</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1363"/>
        <source>Render</source>
        <translation>Отрисовка</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1369"/>
        <source>When checked, the map layers are rendered in response to map navigation commands and other events. When not checked, no rendering is done. This allows you to add a large number of layers and symbolize them before rendering.</source>
        <translation>Если включено, отрисовка слоёв карты выполняется сразу в ответ на команды навигации и другие события. Если выключено, отрисовка не выполняется. К примеру, это позволяет добавить большое количество слоёв и назначить им условные обозначения до их отображения.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3000"/>
        <source>Choose a QGIS project file</source>
        <translation>Выберите файл проекта QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3126"/>
        <source>Unable to save project</source>
        <translation>Не удалось сохранить проект</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3127"/>
        <source>Unable to save project to </source>
        <translation>Не удалось сохранить проект в </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1370"/>
        <source>Toggle map rendering</source>
        <translation>Переключить отрисовку карты</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2098"/>
        <source>Open an OGR Supported Vector Layer</source>
        <translation>Открыть OGR-совместимый векторный слой</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2725"/>
        <source>Save As</source>
        <translation>Сохранить как</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2825"/>
        <source>Choose a QGIS project file to open</source>
        <translation>Выберите открываемый файл проекта QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2943"/>
        <source>QGIS Project Read Error</source>
        <translation>Ошибка чтения проекта QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2945"/>
        <source>Try to find missing layers?</source>
        <translation>Попытаться найти недостающие слои?</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3119"/>
        <source>Saved project to:</source>
        <translation>Проект сохранён в:</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5066"/>
        <source>Open a GDAL Supported Raster Data Source</source>
        <translation>Открыть GDAL-совместимый источник растровых данных</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="331"/>
        <source>Reading settings</source>
        <translation>Загрузка параметров</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="334"/>
        <source>Setting up the GUI</source>
        <translation>Настройка пользовательского интерфейса</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="325"/>
        <source>Checking database</source>
        <translation>Проверка базы данных</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="394"/>
        <source>Restoring loaded plugins</source>
        <translation>Восстановление загруженных модулей</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="399"/>
        <source>Initializing file filters</source>
        <translation>Инициализация файловых фильтров</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="424"/>
        <source>Restoring window state</source>
        <translation>Восстановление состояния окна</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="428"/>
        <source>QGIS Ready!</source>
        <translation>QGIS Готова!</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="533"/>
        <source>&amp;New Project</source>
        <translation>&amp;Новый проект</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="534"/>
        <source>Ctrl+N</source>
        <comment>New Project</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="535"/>
        <source>New Project</source>
        <translation>Новый проект</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="538"/>
        <source>&amp;Open Project...</source>
        <translation>&amp;Открыть проект...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="539"/>
        <source>Ctrl+O</source>
        <comment>Open a Project</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="540"/>
        <source>Open a Project</source>
        <translation>Открыть проект</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="543"/>
        <source>&amp;Save Project</source>
        <translation>&amp;Сохранить проект</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="544"/>
        <source>Ctrl+S</source>
        <comment>Save Project</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="545"/>
        <source>Save Project</source>
        <translation>Сохранить проект</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="548"/>
        <source>Save Project &amp;As...</source>
        <translation>Сохранить проект &amp;как...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="550"/>
        <source>Save Project under a new name</source>
        <translation>Сохранить проект под другим именем</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="553"/>
        <source>Save as Image...</source>
        <translation>Сохранить как изображение...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="555"/>
        <source>Save map as image</source>
        <translation>Сохранить карту как изображение</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="563"/>
        <source>Exit</source>
        <translation>Выйти</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="564"/>
        <source>Ctrl+Q</source>
        <comment>Exit QGIS</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="565"/>
        <source>Exit QGIS</source>
        <translation>Выйти из QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="759"/>
        <source>V</source>
        <comment>Add a Vector Layer</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="760"/>
        <source>Add a Vector Layer</source>
        <translation>Добавить векторный слой</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="764"/>
        <source>R</source>
        <comment>Add a Raster Layer</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="765"/>
        <source>Add a Raster Layer</source>
        <translation>Добавить растровый слой</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="769"/>
        <source>D</source>
        <comment>Add a PostGIS Layer</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="770"/>
        <source>Add a PostGIS Layer</source>
        <translation>Добавить слой PostGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="753"/>
        <source>New Vector Layer...</source>
        <translation>Новый векторный слой...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="754"/>
        <source>N</source>
        <comment>Create a New Vector Layer</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="755"/>
        <source>Create a New Vector Layer</source>
        <translation>Создать новый векторный слой</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="807"/>
        <source>Remove Layer</source>
        <translation>Удалить слой</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="808"/>
        <source>Ctrl+D</source>
        <comment>Remove a Layer</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="809"/>
        <source>Remove a Layer</source>
        <translation>Удалить слой</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="825"/>
        <source>+</source>
        <comment>Show all layers in the overview map</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="826"/>
        <source>Show all layers in the overview map</source>
        <translation>Показать все слои на обзорной карте</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="829"/>
        <source>Remove All From Overview</source>
        <translation>Удалить все из обзора</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="830"/>
        <source>-</source>
        <comment>Remove all layers from overview map</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="831"/>
        <source>Remove all layers from overview map</source>
        <translation>Удалить все слои с обзорной карты</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="834"/>
        <source>Show All Layers</source>
        <translation>Показать все слои</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="835"/>
        <source>S</source>
        <comment>Show all layers</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="836"/>
        <source>Show all layers</source>
        <translation>Показать все слои</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="839"/>
        <source>Hide All Layers</source>
        <translation>Скрыть все слои</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="840"/>
        <source>H</source>
        <comment>Hide all layers</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="841"/>
        <source>Hide all layers</source>
        <translation>Скрыть все слои</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="858"/>
        <source>Project Properties...</source>
        <translation>Свойства проекта...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="859"/>
        <source>P</source>
        <comment>Set project properties</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="860"/>
        <source>Set project properties</source>
        <translation>Задать параметры проекта</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="863"/>
        <source>Options...</source>
        <translation>Параметры...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="865"/>
        <source>Change various QGIS options</source>
        <translation>Изменить параметры приложения QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="897"/>
        <source>Help Contents</source>
        <translation>Содержание</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="901"/>
        <source>F1</source>
        <comment>Help Documentation</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="903"/>
        <source>Help Documentation</source>
        <translation>Открыть руководство по программе</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="908"/>
        <source>Ctrl+H</source>
        <comment>QGIS Home Page</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="910"/>
        <source>QGIS Home Page</source>
        <translation>Веб-сайт QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="917"/>
        <source>About</source>
        <translation>О программе</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="918"/>
        <source>About QGIS</source>
        <translation>О программе QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="913"/>
        <source>Check Qgis Version</source>
        <translation>Проверить версию Qgis</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="914"/>
        <source>Check if your QGIS version is up to date (requires internet access)</source>
        <translation>Проверить, является ли ваша версия QGIS последней (требует доступ в Интернет)</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="746"/>
        <source>Refresh</source>
        <translation>Обновить</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="747"/>
        <source>Ctrl+R</source>
        <comment>Refresh Map</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="748"/>
        <source>Refresh Map</source>
        <translation>Обновить карту</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="677"/>
        <source>Zoom In</source>
        <translation>Увеличить</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="676"/>
        <source>Ctrl++</source>
        <comment>Zoom In</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="682"/>
        <source>Zoom Out</source>
        <translation>Уменьшить</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="681"/>
        <source>Ctrl+-</source>
        <comment>Zoom Out</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="706"/>
        <source>Zoom Full</source>
        <translation>Полный охват</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="707"/>
        <source>F</source>
        <comment>Zoom to Full Extents</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="708"/>
        <source>Zoom to Full Extents</source>
        <translation>Увеличить до полного охвата</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="671"/>
        <source>Pan Map</source>
        <translation>Прокрутка карты</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="672"/>
        <source>Pan the map</source>
        <translation>Прокрутка карты</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="721"/>
        <source>Zoom Last</source>
        <translation>Предыдущий охват</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="723"/>
        <source>Zoom to Last Extent</source>
        <translation>Увеличить до предыдущего охвата</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="713"/>
        <source>Zoom to Layer</source>
        <translation>Увеличить до слоя</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="690"/>
        <source>Identify Features</source>
        <translation>Определить объекты</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="691"/>
        <source>I</source>
        <comment>Click on features to identify them</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="692"/>
        <source>Click on features to identify them</source>
        <translation>Определить объекты по щелчку мыши</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="686"/>
        <source>Select Features</source>
        <translation>Выбрать объекты</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="696"/>
        <source>Measure Line </source>
        <translation>Измерить линию </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="698"/>
        <source>Measure a Line</source>
        <translation>Измерить линию</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="701"/>
        <source>Measure Area</source>
        <translation>Измерить площадь</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="703"/>
        <source>Measure an Area</source>
        <translation>Измерить площадь</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="743"/>
        <source>Show Bookmarks</source>
        <translation>Показать закладки</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="742"/>
        <source>B</source>
        <comment>Show Bookmarks</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="736"/>
        <source>New Bookmark...</source>
        <translation>Новая закладка...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="737"/>
        <source>Ctrl+B</source>
        <comment>New Bookmark</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5423"/>
        <source>New Bookmark</source>
        <translation>Новая закладка</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="780"/>
        <source>Add WMS Layer...</source>
        <translation>Добавить WMS-слой...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="819"/>
        <source>O</source>
        <comment>Add current layer to overview map</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="820"/>
        <source>Add current layer to overview map</source>
        <translation>Добавить текущий слой в обзорную карту</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="848"/>
        <source>Open the plugin manager</source>
        <translation>Открыть менеджер модулей</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="611"/>
        <source>Capture Point</source>
        <translation>Захватить точку</translation>
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
        <translation>Захватить точки</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="617"/>
        <source>Capture Line</source>
        <translation>Захватить линию</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="618"/>
        <source>/</source>
        <comment>Capture Lines</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="619"/>
        <source>Capture Lines</source>
        <translation>Захватить линии</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="623"/>
        <source>Capture Polygon</source>
        <translation>Захватить полигон</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="624"/>
        <source>Ctrl+/</source>
        <comment>Capture Polygons</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="625"/>
        <source>Capture Polygons</source>
        <translation>Захватить полигоны</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="640"/>
        <source>Delete Selected</source>
        <translation>Удалить выделенное</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="645"/>
        <source>Add Vertex</source>
        <translation>Добавить вершину</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="655"/>
        <source>Delete Vertex</source>
        <translation>Удалить вершину</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="650"/>
        <source>Move Vertex</source>
        <translation>Переместить вершину</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1006"/>
        <source>&amp;File</source>
        <translation>&amp;Файл</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1010"/>
        <source>&amp;Open Recent Projects</source>
        <translation>&amp;Открыть недавние проекты</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1073"/>
        <source>&amp;View</source>
        <translation>&amp;Вид</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1106"/>
        <source>&amp;Layer</source>
        <translation>С&amp;лой</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1138"/>
        <source>&amp;Settings</source>
        <translation>&amp;Установки</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1177"/>
        <source>&amp;Help</source>
        <translation>&amp;Справка</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1199"/>
        <source>File</source>
        <translation>Файл</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1216"/>
        <source>Manage Layers</source>
        <translation>Управление слоями</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1282"/>
        <source>Help</source>
        <translation>Справка</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1227"/>
        <source>Digitizing</source>
        <translation>Оцифровка</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1248"/>
        <source>Map Navigation</source>
        <translation>Навигация</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1262"/>
        <source>Attributes</source>
        <translation>Атрибуты</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1276"/>
        <source>Plugins</source>
        <translation>Модули</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1398"/>
        <source>Ready</source>
        <translation>Готово</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1803"/>
        <source>New features</source>
        <translation>Новые возможности</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2961"/>
        <source>Unable to open project</source>
        <translation>Не удалось открыть проект</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3133"/>
        <source>Unable to save project </source>
        <translation>Не удалось сохранить проект </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3191"/>
        <source>QGIS: Unable to load project</source>
        <translation>QGIS: не удалось загрузить проект</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3192"/>
        <source>Unable to load project </source>
        <translation>Не удалось загрузить проект </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4191"/>
        <source>QGIS - Changes in SVN Since Last Release</source>
        <translation>QGIS — Изменения в SVN со времени последнего релиза</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5240"/>
        <source>Layer is not valid</source>
        <translation>Неверный слой</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5241"/>
        <source>The layer is not a valid layer and can not be added to the map</source>
        <translation>Слой не является действительным и не может быть добавлен на карту</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4449"/>
        <source>Save?</source>
        <translation>Сохранить?</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5159"/>
        <source> is not a valid or recognized raster data source</source>
        <translation> не является допустимым (определяемым) источником растровых данных</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5322"/>
        <source> is not a supported raster data source</source>
        <translation> не является поддерживаемым источником растровых данных</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5327"/>
        <source>Unsupported Data Source</source>
        <translation>Неподдерживаемый источник данных</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5424"/>
        <source>Enter a name for the new bookmark:</source>
        <translation>Введите имя для этой закладки:</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5441"/>
        <source>Error</source>
        <translation>Ошибка</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5441"/>
        <source>Unable to create the bookmark. Your user database may be missing or corrupted</source>
        <translation>Не удалось создать закладку. Ваша пользовательская база данных отсутствует или повреждена</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="899"/>
        <source>Ctrl+?</source>
        <comment>Help Documentation (Mac)</comment>
        <translation>Ctrl+?</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="593"/>
        <source>Cut Features</source>
        <translation>Вырезать объекты</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="595"/>
        <source>Cut selected features</source>
        <translation>Вырезать выделенные объекты</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="599"/>
        <source>Copy Features</source>
        <translation>Копировать объекты</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="601"/>
        <source>Copy selected features</source>
        <translation>Копировать выделенные объекты</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="605"/>
        <source>Paste Features</source>
        <translation>Вставить объекты</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="607"/>
        <source>Paste selected features</source>
        <translation>Вставить выделенные объекты</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4223"/>
        <source>Network error while communicating with server</source>
        <translation>Ошибка сети во время соединения с сервером</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4226"/>
        <source>Unknown network socket error</source>
        <translation>Неизвестная ошибка сетевого соединения</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4231"/>
        <source>Unable to communicate with QGIS Version server</source>
        <translation>Не удалось связаться с сервером версии QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="383"/>
        <source>Checking provider plugins</source>
        <translation>Проверка источников данных</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="387"/>
        <source>Starting Python</source>
        <translation>Запуск Python</translation>
    </message>
    <message>
        <source>Python error</source>
        <translation type="obsolete">Ошибка Python</translation>
    </message>
    <message>
        <source>Error when reading metadata of plugin </source>
        <translation type="obsolete">Ошибка чтения метаданных модуля </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3643"/>
        <source>Provider does not support deletion</source>
        <translation>Источник не поддерживает удаление</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3644"/>
        <source>Data provider does not support deleting features</source>
        <translation>Источник данных не поддерживает удаление объектов</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3650"/>
        <source>Layer not editable</source>
        <translation>Нередактируемый слой</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3651"/>
        <source>The current layer is not editable. Choose &apos;Start editing&apos; in the digitizing toolbar.</source>
        <translation>Текущий слой нередактируем. Выберите «Режим редактирования» на панели инструментов оцифровки.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="791"/>
        <source>Toggle editing</source>
        <translation>Режим редактирования</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="792"/>
        <source>Toggles the editing state of the current layer</source>
        <translation>Переключить текущий слой в режим редактирования</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="660"/>
        <source>Add Ring</source>
        <translation>Добавить кольцо</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="664"/>
        <source>Add Island</source>
        <translation>Добавить остров</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="665"/>
        <source>Add Island to multipolygon</source>
        <translation>Добавить остров к мультиполигону</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1336"/>
        <source>Scale </source>
        <translation>Масштаб </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1351"/>
        <source>Current map scale (formatted as x:y)</source>
        <translation>Текущий масштаб карты (в формате x:y)</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4656"/>
        <source>Map coordinates at mouse cursor position</source>
        <translation>Координаты карты в позиции курсора мыши</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3981"/>
        <source>Invalid scale</source>
        <translation>Неверный масштаб</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4450"/>
        <source>Do you want to save the current project?</source>
        <translation>Вы хотите сохранить текущий проект?</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1326"/>
        <source>Shows the map coordinates at the current cursor position. The display is continuously updated as the mouse is moved.</source>
        <translation>Здесь показаны координаты карты в позиции курсора. Эти значения постоянно обновляются при движении мыши.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="630"/>
        <source>Move Feature</source>
        <translation>Переместить объект</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="635"/>
        <source>Split Features</source>
        <translation>Разделить объекты</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="731"/>
        <source>Map Tips</source>
        <translation>Всплывающие описания</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="732"/>
        <source>Show information about a feature when the mouse is hovered over it</source>
        <translation>Показать информацию об объекте при перемещении над ним курсора мыши</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1337"/>
        <source>Current map scale</source>
        <translation>Текущий масштаб карты</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5455"/>
        <source>Project file is older</source>
        <translation>Устаревший файл проекта</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5457"/>
        <source>&lt;p&gt;This project file was saved by an older version of QGIS.</source>
        <translation>&lt;p&gt;Этот файл проекта был создан старой версией QGIS.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5459"/>
        <source> When saving this project file, QGIS will update it to the latest version, possibly rendering it useless for older versions of QGIS.</source>
        <translation> При сохранении, этот файл будет обновлён, что может повлечь за собой несовместимость с предыдущими версиями QGIS.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5462"/>
        <source>&lt;p&gt;Even though QGIS developers try to maintain backwards compatibility, some of the information from the old project file might be lost.</source>
        <translation>&lt;p&gt;Несмотря на то, что разработчики QGIS стремятся к максимальной обратной совместимости, часть информации из старого проекта может быть потеряна.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5464"/>
        <source> To improve the quality of QGIS, we appreciate if you file a bug report at %3.</source>
        <translation> Вы могли бы помочь нам улучшить QGIS, отправив сообщение об ошибке по адресу: %3.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5466"/>
        <source> Be sure to include the old project file, and state the version of QGIS you used to discover the error.</source>
        <translation> Пожалуйста, приложите старый файл проекта и укажите версию QGIS, в которой была обнаружена ошибка.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5468"/>
        <source>&lt;p&gt;To remove this warning when opening an older project file, uncheck the box &apos;%5&apos; in the %4 menu.</source>
        <translation>&lt;p&gt;Если вы не хотите видеть это сообщение в дальнейшем, снимите флажок «%5» в меню «%4».</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5468"/>
        <source>&lt;p&gt;Version of the project file: %1&lt;br&gt;Current version of QGIS: %2</source>
        <translation>&lt;p&gt;Версия файла проекта: %1&lt;br&gt;Текущая версия QGIS: %2</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5472"/>
        <source>&lt;tt&gt;Settings:Options:General&lt;/tt&gt;</source>
        <comment>Menu path to setting options</comment>
        <translation>&lt;tt&gt;Правка:Параметры:Общие&lt;/tt&gt;</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="5473"/>
        <source>Warn me when opening a project file saved with an older version of QGIS</source>
        <translation>Предупреждать при попытке открытия файлов проекта старых версий QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="854"/>
        <source>Ctrl-F</source>
        <comment>Toggle fullscreen mode</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="855"/>
        <source>Toggle fullscreen mode</source>
        <translation>Полноэкранный режим</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1383"/>
        <source>Resource Location Error</source>
        <translation>Ошибка поиска ресурса</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1384"/>
        <source>Error reading icon resources from: 
 %1
 Quitting...</source>
        <translation>Ошибка загрузки значков из: 
 %1
 Выход...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1593"/>
        <source>Overview</source>
        <translation>Обзор</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1645"/>
        <source>Legend</source>
        <translation>Легенда</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1781"/>
        <source>You are using QGIS version %1 built against code revision %2.</source>
        <translation>Версия QGIS: %1, ревизия: %2.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1785"/>
        <source> This copy of QGIS has been built with PostgreSQL support.</source>
        <translation> Данная версия QGIS собрана с поддержкой PostgreSQL.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1788"/>
        <source> This copy of QGIS has been built without PostgreSQL support.</source>
        <translation> Данная версия QGIS собрана без поддержки PostgreSQL.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1792"/>
        <source>
This binary was compiled against Qt %1,and is currently running against Qt %2</source>
        <translation>
Версия Qt, используемая при сборке: %1. Текущая версия Qt: %2</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1360"/>
        <source>Stop map rendering</source>
        <translation>Остановить отрисовку</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="315"/>
        <source>Multiple Instances of QgisApp</source>
        <translation>Множество экземпляров QgisApp</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="316"/>
        <source>Multiple instances of Quantum GIS application object detected.
Please contact the developers.
</source>
        <translation>Обнаружено более одного экземпляра приложения Quantum GIS.
Пожалуйста, свяжитесь с разработчиками.
</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="869"/>
        <source>Custom CRS...</source>
        <translation>Ввод системы координат...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="871"/>
        <source>Manage custom coordinate reference systems</source>
        <translation>Управление пользовательскими системами координат</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1313"/>
        <source>Toggle extents and mouse position display</source>
        <translation>Переключить вывод границ или позиции курсора</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1392"/>
        <source>This icon shows whether on the fly coordinate reference system transformation is enabled or not. Click the icon to bring up the project properties dialog to alter this behaviour.</source>
        <translation>Этот значок показывает, было ли включено преобразование координат «на лету». Щёлкните по значку, чтобы открыть диалог свойств проекта и изменить данный режим.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1394"/>
        <source>CRS status - Click to open coordinate reference system dialog</source>
        <translation>Преобразование координат — щёлкните для открытия диалога свойств системы координат</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1806"/>
        <source>This release candidate includes over 60 bug fixes and enchancements over the QGIS 0.10.0 release. In addition we have added the following new features:</source>
        <translation>Эта версия включает более 60 исправлений ошибок и обновлений с момента выхода QGIS 0.10.0. Кроме того, мы добавили следующие возможности: </translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1809"/>
        <source>Revision of all dialogs for user interface consistancy</source>
        <translation>Все диалоги были переработаны по единому образцу</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1812"/>
        <source>Improvements to unique value renderer vector dialog</source>
        <translation>Улучшен диалог ввода параметров для отрисовки уникальных значений</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1815"/>
        <source>Symbol previews when defining vector classes</source>
        <translation>Добавлен предварительный просмотр условных знаков при определении векторных классов</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1818"/>
        <source>Separation of python support into its own library</source>
        <translation>Поддержка Python выделена в отдельную библиотеку</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1821"/>
        <source>List view and filter for GRASS toolbox to find tools more quickly</source>
        <translation>В диалоге инструментов GRASS реализован список и фильтр для быстрого поиска инструментов</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1824"/>
        <source>List view and filter for Plugin Manager to find plugins more easily</source>
        <translation>В менеджере модулей реализован список и фильтр для быстрого поиска модулей</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1827"/>
        <source>Updated Spatial Reference System definitions</source>
        <translation>Обновлены определения систем координат</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1830"/>
        <source>QML Style support for rasters and database layers</source>
        <translation>Добавлена поддержка QML-стилей для растровых и PostGIS-слоёв</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3890"/>
        <source>Start editing failed</source>
        <translation>Не удалось начать редактирование</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3890"/>
        <source>Provider cannot be opened for editing</source>
        <translation>Источник не может быть открыт для редактирования</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3898"/>
        <source>Stop editing</source>
        <translation>Прекратить редактирование</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3899"/>
        <source>Do you want to save the changes to layer %1?</source>
        <translation>Вы хотите сохранить изменения в слое %1?</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3909"/>
        <source>Could not commit changes to layer %1

Errors:  %2
</source>
        <translation>Не удалось внести изменения в слой %1

Ошибка:  %2
</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3920"/>
        <source>Problems during roll back</source>
        <translation>Ошибка в процессе отката</translation>
    </message>
    <message>
        <source>There was an error loading a plugin.The following diagnostic information may help the QGIS developers resolve the issue:
%1.</source>
        <translation type="obsolete">При загрузке модуля возникла ошибка. Информация ниже может помочь разработчикам QGIS решить эту проблему:
%1.</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4649"/>
        <source>Map coordinates for the current view extents</source>
        <translation>Границы текущего окна в координатах карты</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4741"/>
        <source>Maptips require an active layer</source>
        <translation>Для вывода всплывающих описаний необходим активный слой</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="549"/>
        <source>Shift+Ctrl+S</source>
        <comment>Save Project under a new name</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="558"/>
        <source>&amp;Print Composer</source>
        <translation>Ко&amp;мпоновка карты</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="559"/>
        <source>Ctrl+P</source>
        <comment>Print Composer</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="560"/>
        <source>Print Composer</source>
        <translation>Компоновка карты</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="572"/>
        <source>&amp;Undo</source>
        <translation>&amp;Отменить</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="573"/>
        <source>Ctrl+Z</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="574"/>
        <source>Undo the last operation</source>
        <translation>Отменить последнюю операцию</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="577"/>
        <source>Cu&amp;t</source>
        <translation>&amp;Вырезать</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="594"/>
        <source>Ctrl+X</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="579"/>
        <source>Cut the current selection&apos;s contents to the clipboard</source>
        <translation>Вырезать выделение в буфер обмена</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="582"/>
        <source>&amp;Copy</source>
        <translation>&amp;Копировать</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="600"/>
        <source>Ctrl+C</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="584"/>
        <source>Copy the current selection&apos;s contents to the clipboard</source>
        <translation>Копировать выделение в буфер обмена</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="587"/>
        <source>&amp;Paste</source>
        <translation>Вст&amp;авить</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="606"/>
        <source>Ctrl+V</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="589"/>
        <source>Paste the clipboard&apos;s contents into the current selection</source>
        <translation>Вставить содержимое буфера обмена в текущее выделение</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="697"/>
        <source>M</source>
        <comment>Measure a Line</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="702"/>
        <source>J</source>
        <comment>Measure an Area</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="718"/>
        <source>Zoom to Selection</source>
        <translation>Увеличить до выделенного</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="717"/>
        <source>Ctrl+J</source>
        <comment>Zoom to Selection</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="726"/>
        <source>Zoom Actual Size</source>
        <translation>Фактический размер</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="727"/>
        <source>Zoom to Actual Size</source>
        <translation>Увеличить до фактического размера</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="758"/>
        <source>Add Vector Layer...</source>
        <translation>Добавить векторный слой...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="763"/>
        <source>Add Raster Layer...</source>
        <translation>Добавить растровый слой...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="768"/>
        <source>Add PostGIS Layer...</source>
        <translation>Добавить слой PostGIS...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="781"/>
        <source>W</source>
        <comment>Add a Web Mapping Server Layer</comment>
        <translation>W</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="782"/>
        <source>Add a Web Mapping Server Layer</source>
        <translation>Добавить слой с картографического веб-сервера</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="787"/>
        <source>Open Attribute Table</source>
        <translation>Открыть таблицу атрибутов</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="797"/>
        <source>Save as Shapefile...</source>
        <translation>Сохранить как shape-файл...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="798"/>
        <source>Save the current layer as a shapefile</source>
        <translation>Сохранить текущий слой в shape-файл</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="802"/>
        <source>Save Selection as Shapefile...</source>
        <translation>Сохранить выделение как shape-файл...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="803"/>
        <source>Save the selection as a shapefile</source>
        <translation>Сохранить выделение в shape-файл</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="813"/>
        <source>Properties...</source>
        <translation>Свойства...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="814"/>
        <source>Set properties of the current layer</source>
        <translation>Изменить свойства текущего слоя</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="818"/>
        <source>Add to Overview</source>
        <translation>Добавить в обзор</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="824"/>
        <source>Add All to Overview</source>
        <translation>Добавить все в обзор</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="846"/>
        <source>Manage Plugins...</source>
        <translation>Управление модулями...</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="853"/>
        <source>Toggle Full Screen Mode</source>
        <translation>Полноэкранный режим</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="878"/>
        <source>Minimize</source>
        <translation>Свернуть</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="879"/>
        <source>Ctrl+M</source>
        <comment>Minimize Window</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="880"/>
        <source>Minimizes the active window to the dock</source>
        <translation>Свернуть активное окно в док</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="883"/>
        <source>Zoom</source>
        <translation>Увеличить</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="884"/>
        <source>Toggles between a predefined size and the window size set by the user</source>
        <translation>Переключение между предопределённым и заданным пользователем размером окна</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="887"/>
        <source>Bring All to Front</source>
        <translation>Все на передний план</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="888"/>
        <source>Bring forward all open windows</source>
        <translation>Выдвинуть на передний план все открытые окна</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1034"/>
        <source>&amp;Edit</source>
        <translation>&amp;Правка</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1068"/>
        <source>Panels</source>
        <translation>Панели</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1069"/>
        <source>Toolbars</source>
        <translation>Панели инструментов</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="1164"/>
        <source>&amp;Window</source>
        <translation>&amp;Окно</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3075"/>
        <source>Choose a file name to save the QGIS project file as</source>
        <translation>Выберите имя файла для сохранения проекта QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="3316"/>
        <source>Choose a file name to save the map image as</source>
        <translation>Выберите имя файла для сохранения снимка карты</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="4108"/>
        <source>Python Console</source>
        <translation>Консоль Python</translation>
    </message>
    <message>
        <location filename="../src/app/qgisapp.cpp" line="2944"/>
        <source></source>
        <translation></translation>
    </message>
</context>
<context>
    <name>QgisAppBase</name>
    <message>
        <location filename="../src/ui/qgisappbase.ui" line="13"/>
        <source>QGIS</source>
        <translation></translation>
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
        <location filename="../src/ui/qgsabout.ui" line="152"/>
        <source>What&apos;s New</source>
        <translation>Что нового</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="104"/>
        <source>http://www.gnu.org/licenses</source>
        <translation>http://www.gnu.org/licenses</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="91"/>
        <source>Quantum GIS is licensed under the GNU General Public License</source>
        <translation>Quantum GIS выпускается под Стандартной Общественной Лицензией GNU</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="129"/>
        <source>QGIS Home Page</source>
        <translation>Веб-сайт QGIS</translation>
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
        <location filename="../src/app/qgsabout.cpp" line="116"/>
        <source>Name</source>
        <translation>Имя</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="199"/>
        <source>Sponsors</source>
        <translation>Спонсоры</translation>
    </message>
    <message>
        <location filename="../src/app/qgsabout.cpp" line="117"/>
        <source>Website</source>
        <translation>Веб-сайт</translation>
    </message>
    <message>
        <location filename="../src/app/qgsabout.cpp" line="114"/>
        <source>&lt;p&gt;The following have sponsored QGIS by contributing money to fund development and other project costs&lt;/p&gt;</source>
        <translation>&lt;p&gt;Эти люди спонсировали QGIS, вкладывая деньги в разработку и покрытие иных расходов проекта&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/app/qgsabout.cpp" line="170"/>
        <source>Available QGIS Data Provider Plugins</source>
        <translation>Доступные модули источников данных QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgsabout.cpp" line="173"/>
        <source>Available Qt Database Plugins</source>
        <translation>Доступные модули источников данных Qt</translation>
    </message>
    <message>
        <location filename="../src/app/qgsabout.cpp" line="179"/>
        <source>Available Qt Image Plugins</source>
        <translation>Доступные модули Qt для загрузки изображений</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="50"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Lucida Grande&apos;; font-size:13pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:16px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:x-large; font-weight:600;&quot;&gt;&lt;span style=&quot; font-size:x-large;&quot;&gt;Quantum GIS (QGIS)&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsabout.ui" line="139"/>
        <source>Join our user mailing list</source>
        <translation>Список рассылки для пользователей</translation>
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
</context>
<context>
    <name>QgsApplication</name>
    <message>
        <location filename="../src/core/qgsapplication.cpp" line="82"/>
        <source>Exception</source>
        <translation>Исключение</translation>
    </message>
</context>
<context>
    <name>QgsAttributeActionDialog</name>
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
        <translation>Имя</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="162"/>
        <source>Action</source>
        <translation>Действие</translation>
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
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="79"/>
        <source>Enter the action command here</source>
        <translation>Введите команду действия</translation>
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
        <translation>Допустимые имена атрибутов для этого слоя</translation>
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
        <translation>Действия с атрибутами</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="37"/>
        <source>Action properties</source>
        <translation>Свойства действия</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="89"/>
        <source>Browse for action</source>
        <translation>Поиск действия</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="92"/>
        <source>Click to browse for an action</source>
        <translation>Щелкните для поиска команды действия</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="98"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="167"/>
        <source>Capture</source>
        <translation>Захватывать</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributeactiondialogbase.ui" line="95"/>
        <source>Clicking the button will let you select an application to use as the action</source>
        <translation>Эта кнопка позволяет найти приложение, используемое как действие</translation>
    </message>
</context>
<context>
    <name>QgsAttributeDialog</name>
    <message>
        <location filename="../src/app/qgsattributedialog.cpp" line="271"/>
        <source> (int)</source>
        <translation> (целое)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributedialog.cpp" line="275"/>
        <source> (dbl)</source>
        <translation> (действ.)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributedialog.cpp" line="280"/>
        <source> (txt)</source>
        <translation> (текст.)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributedialog.cpp" line="256"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributedialog.cpp" line="315"/>
        <source>Select a file</source>
        <translation>Выберите файл</translation>
    </message>
</context>
<context>
    <name>QgsAttributeDialogBase</name>
    <message>
        <location filename="../src/ui/qgsattributedialogbase.ui" line="13"/>
        <source>Enter Attribute Values</source>
        <translation>Введите значения атрибутов</translation>
    </message>
</context>
<context>
    <name>QgsAttributeTable</name>
    <message>
        <location filename="../src/app/qgsattributetable.cpp" line="356"/>
        <source>Run action</source>
        <translation>Выполнить действие</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetable.cpp" line="630"/>
        <source>Updating selection...</source>
        <translation>Обновление выделения...</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetable.cpp" line="630"/>
        <source>Abort</source>
        <translation>Отменить</translation>
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
        <location filename="../src/ui/qgsattributetablebase.ui" line="77"/>
        <source>Ctrl+S</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="67"/>
        <source>Invert selection</source>
        <translation>Обратить выделение</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="60"/>
        <source>Ctrl+T</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="50"/>
        <source>Move selected to top</source>
        <translation>Переместить выделенное в начало</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="33"/>
        <source>Remove selection</source>
        <translation>Удалить выделение</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="84"/>
        <source>Copy selected rows to clipboard (Ctrl+C)</source>
        <translation>Копировать выбранные строки в буфер обмена (Ctrl+C)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="87"/>
        <source>Copies the selected rows to the clipboard</source>
        <translation>Копирует выбранные строки в буфер обмена</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="97"/>
        <source>Ctrl+C</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="148"/>
        <source>in</source>
        <translation>в</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="168"/>
        <source>Search</source>
        <translation>Поиск</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="178"/>
        <source>Adva&amp;nced...</source>
        <translation>&amp;Дополнительно...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="181"/>
        <source>Alt+N</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="107"/>
        <source>Zoom map to the selected rows</source>
        <translation>Увеличить карту до выбранных строк</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="135"/>
        <source>Search for</source>
        <translation>Искать</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="207"/>
        <source>Toggle editing mode</source>
        <translation>Режим редактирования</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="210"/>
        <source>Click to toggle table editing</source>
        <translation>Переключить редактирование таблицы</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="104"/>
        <source>Zoom map to the selected rows (Ctrl-J)</source>
        <translation>Увеличить карту до выбранных строк (Ctrl-J)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsattributetablebase.ui" line="113"/>
        <source>Ctrl+J</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>QgsAttributeTableDisplay</name>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="117"/>
        <source>select</source>
        <translation>выбрать</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="118"/>
        <source>select and bring to top</source>
        <translation>выбрать и переместить в начало</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="119"/>
        <source>show only matching</source>
        <translation>только показать соответствия</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="358"/>
        <source>Search string parsing error</source>
        <translation>Ошибка разбора поискового запроса</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="406"/>
        <source>Search results</source>
        <translation>Результаты поиска</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="365"/>
        <source>You&apos;ve supplied an empty search string.</source>
        <translation>Вы ввели пустой поисковый запрос.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="394"/>
        <source>Error during search</source>
        <translation>Ошибка во время поиска</translation>
    </message>
    <message numerus="yes">
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="403"/>
        <source>Found %d matching features.</source>
        <translation>
            <numerusform>Найден %d подходящий объект.</numerusform>
            <numerusform>Найдено %d подходящих объекта.</numerusform>
            <numerusform>Найдено %d подходящих объектов.</numerusform>
        </translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="405"/>
        <source>No matching features found.</source>
        <translation>Подходящих объектов не найдено.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="132"/>
        <source>Attribute table - </source>
        <translation>Таблица атрибутов — </translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="472"/>
        <source>bad_alloc exception</source>
        <translation>Исключение bad_alloc</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="472"/>
        <source>Filling the attribute table has been stopped because there was no more virtual memory left</source>
        <translation>Заполнение таблицы атрибутов остановлено, поскольку закончилась виртуальная память</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="139"/>
        <source>QGIS</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="143"/>
        <source>File</source>
        <translation>Файл</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="144"/>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="144"/>
        <source>Ctrl+W</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="146"/>
        <source>Edit</source>
        <translation>Правка</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="147"/>
        <source>&amp;Undo</source>
        <translation>&amp;Отменить</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="147"/>
        <source>Ctrl+Z</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="150"/>
        <source>Cu&amp;t</source>
        <translation>&amp;Вырезать</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="150"/>
        <source>Ctrl+X</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="153"/>
        <source>&amp;Copy</source>
        <translation>&amp;Копировать</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="153"/>
        <source>Ctrl+C</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="154"/>
        <source>&amp;Paste</source>
        <translation>Вст&amp;авить</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="154"/>
        <source>Ctrl+V</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="157"/>
        <source>Delete</source>
        <translation>Удалить</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="159"/>
        <source>Layer</source>
        <translation>Слой</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="161"/>
        <source>Zoom to Selection</source>
        <translation>Увеличить до выделенного</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="161"/>
        <source>Ctrl+J</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="164"/>
        <source>Toggle Editing</source>
        <translation>Режим редактирования</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="170"/>
        <source>Table</source>
        <translation>Таблица</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="172"/>
        <source>Move to Top</source>
        <translation>Переместить в начало</translation>
    </message>
    <message>
        <location filename="../src/app/qgsattributetabledisplay.cpp" line="174"/>
        <source>Invert</source>
        <translation>Обратить</translation>
    </message>
</context>
<context>
    <name>QgsBookmarks</name>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="158"/>
        <source>Really Delete?</source>
        <translation>Действительно удалить?</translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="159"/>
        <source>Are you sure you want to delete the </source>
        <translation>Вы уверены, что хотите удалить закладку </translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="159"/>
        <source> bookmark?</source>
        <translation>?</translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="175"/>
        <source>Error deleting bookmark</source>
        <translation>Ошибка удаления закладки</translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="177"/>
        <source>Failed to delete the </source>
        <translation>Не удалось удалить закладку </translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="179"/>
        <source> bookmark from the database. The database said:
</source>
        <translation> из базы данных. Сообщение базы данных:
</translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="58"/>
        <source>&amp;Delete</source>
        <translation>&amp;Удалить</translation>
    </message>
    <message>
        <location filename="../src/app/qgsbookmarks.cpp" line="59"/>
        <source>&amp;Zoom to</source>
        <translation>&amp;Увеличить до</translation>
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
        <translation>Охват</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsbookmarksbase.ui" line="44"/>
        <source>Id</source>
        <translation>ID</translation>
    </message>
</context>
<context>
    <name>QgsComposer</name>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="473"/>
        <source>Big image</source>
        <translation>Большое изображение</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="474"/>
        <source>To create image </source>
        <translation>Для создания изображения </translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="477"/>
        <source> requires circa </source>
        <translation> требуется приблизительно </translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="477"/>
        <source> MB of memory</source>
        <translation> МБ памяти</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="83"/>
        <source>QGIS - print composer</source>
        <translation>QGIS — компоновка карты</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="181"/>
        <source>Map 1</source>
        <translation>Карта 1</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="504"/>
        <source>format</source>
        <translation>формат</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="597"/>
        <source>SVG warning</source>
        <translation>Предупреждение SVG</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="1094"/>
        <source>Don&apos;t show this message again</source>
        <translation>Не показывать это сообщение в дальнейшем</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="624"/>
        <source>SVG Format</source>
        <translation>Формат SVG</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="604"/>
        <source>&lt;p&gt;The SVG export function in Qgis has several problems due to bugs and deficiencies in the </source>
        <translation>&lt;p&gt;Функция SVG-экспорта в QGIS может работать неправильно из-за ошибок в </translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="82"/>
        <source>Move Content</source>
        <translation type="obsolete">Переместить содержимое элемента</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="83"/>
        <source>Move item content</source>
        <translation type="obsolete">Переместить содержимое элемента</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="90"/>
        <source>&amp;Group</source>
        <translation type="obsolete">С&amp;группировать</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="91"/>
        <source>Group items</source>
        <translation type="obsolete">Сгруппировать элементы</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="93"/>
        <source>&amp;Ungroup</source>
        <translation type="obsolete">&amp;Разгруппировать</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="94"/>
        <source>Ungroup items</source>
        <translation type="obsolete">Разгруппировать элементы</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="96"/>
        <source>Raise</source>
        <translation type="obsolete">Поднять</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="97"/>
        <source>Raise selected items</source>
        <translation type="obsolete">Поднять выбранные элементы</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="99"/>
        <source>Lower</source>
        <translation type="obsolete">Опустить</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="100"/>
        <source>Lower selected items</source>
        <translation type="obsolete">Опустить выбранные элементы</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="102"/>
        <source>Bring to Front</source>
        <translation type="obsolete">На передний план</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="103"/>
        <source>Move selected items to top</source>
        <translation type="obsolete">Поднять выделенные элементы на передний план</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="105"/>
        <source>Send to Back</source>
        <translation type="obsolete">На задний план</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="106"/>
        <source>Move selected items to bottom</source>
        <translation type="obsolete">Опустить выделенные элементы на задний план</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="94"/>
        <source>QGIS</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="98"/>
        <source>File</source>
        <translation>Файл</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="99"/>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="99"/>
        <source>Ctrl+W</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="105"/>
        <source>Edit</source>
        <translation>Правка</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="106"/>
        <source>&amp;Undo</source>
        <translation>&amp;Отменить</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="106"/>
        <source>Ctrl+Z</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="109"/>
        <source>Cu&amp;t</source>
        <translation>&amp;Вырезать</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="109"/>
        <source>Ctrl+X</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="111"/>
        <source>&amp;Copy</source>
        <translation>&amp;Копировать</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="111"/>
        <source>Ctrl+C</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="113"/>
        <source>&amp;Paste</source>
        <translation>Вст&amp;авить</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="113"/>
        <source>Ctrl+V</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="115"/>
        <source>Delete</source>
        <translation>Удалить</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="118"/>
        <source>View</source>
        <translation>Вид</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="125"/>
        <source>Layout</source>
        <translation>Компоновка</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="527"/>
        <source>Choose a file name to save the map image as</source>
        <translation>Выберите имя файла для сохранения снимка карты</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="623"/>
        <source>Choose a file name to save the map as</source>
        <translation>Выберите имя файла для сохранения карты</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="1092"/>
        <source>Project contains WMS layers</source>
        <translation>Проект содержит WMS-слои</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposer.cpp" line="1093"/>
        <source>Some WMS servers (e.g. UMN mapserver) have a limit for the WIDTH and HEIGHT parameter. Printing layers from such servers may exceed this limit. If this is the case, the WMS layer will not be printed</source>
        <translation>Некоторые WMS-сервера (например, UMN mapserver) имеют ограничения на значения параметров ширины и высоты (WIDTH и HEIGHT). Во время печати слоёв с этих серверов, эти лимиты могут быть превышены. В этом случае, WMS-слой не будет напечатан</translation>
    </message>
</context>
<context>
    <name>QgsComposerBase</name>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="81"/>
        <source>General</source>
        <translation>Общие</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="134"/>
        <source>Composition</source>
        <translation>Композиция</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="142"/>
        <source>Item</source>
        <translation>Элемент</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="260"/>
        <source>&amp;Print...</source>
        <translation>&amp;Печать...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="295"/>
        <source>Add new map</source>
        <translation>Добавить карту</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="306"/>
        <source>Add new label</source>
        <translation>Добавить текст</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="317"/>
        <source>Add new vect legend</source>
        <translation>Добавить легенду</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="328"/>
        <source>Select/Move item</source>
        <translation>Выбрать/переместить элемент</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="355"/>
        <source>Add new scalebar</source>
        <translation>Добавить масштабную линейку</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="366"/>
        <source>Refresh view</source>
        <translation>Обновить</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="16"/>
        <source>MainWindow</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="276"/>
        <source>Zoom In</source>
        <translation>Увеличить</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="284"/>
        <source>Zoom Out</source>
        <translation>Уменьшить</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="374"/>
        <source>Add Image</source>
        <translation>Добавить изображение</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="214"/>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="194"/>
        <source>Help</source>
        <translation>Справка</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="268"/>
        <source>Zoom Full</source>
        <translation>Полный охват</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="292"/>
        <source>Add Map</source>
        <translation>Добавить карту</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="303"/>
        <source>Add Label</source>
        <translation>Добавить текст</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="314"/>
        <source>Add Vector Legend</source>
        <translation>Добавить легенду</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="325"/>
        <source>Move Item</source>
        <translation>Переместить элемент</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="336"/>
        <source>Export as Image...</source>
        <translation>Экспорт в изображение...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="344"/>
        <source>Export as SVG...</source>
        <translation>Экспорт в SVG...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="352"/>
        <source>Add Scalebar</source>
        <translation>Добавить масштабную линейку</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="363"/>
        <source>Refresh</source>
        <translation>Обновить</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="379"/>
        <source>Move Content</source>
        <translation>Переместить содержимое</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="382"/>
        <source>Move item content</source>
        <translation>Переместить содержимое элемента</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="387"/>
        <source>Group</source>
        <translation>Сгруппировать</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="390"/>
        <source>Group items</source>
        <translation>Сгруппировать элементы</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="395"/>
        <source>Ungroup</source>
        <translation>Разгруппировать</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="398"/>
        <source>Ungroup items</source>
        <translation>Разгруппировать элементы</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="403"/>
        <source>Raise</source>
        <translation>Поднять</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="406"/>
        <source>Raise selected items</source>
        <translation>Поднять выбранные элементы</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="411"/>
        <source>Lower</source>
        <translation>Опустить</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="414"/>
        <source>Lower selected items</source>
        <translation>Опустить выбранные элементы</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="419"/>
        <source>Bring to Front</source>
        <translation>На передний план</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="422"/>
        <source>Move selected items to top</source>
        <translation>Поднять выделенные элементы на передний план</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="427"/>
        <source>Send to Back</source>
        <translation>На задний план</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerbase.ui" line="430"/>
        <source>Move selected items to bottom</source>
        <translation>Опустить выделенные элементы на задний план</translation>
    </message>
</context>
<context>
    <name>QgsComposerItemWidgetBase</name>
    <message>
        <location filename="../src/ui/qgscomposeritemwidgetbase.ui" line="13"/>
        <source>Form</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposeritemwidgetbase.ui" line="19"/>
        <source>Composer item properties</source>
        <translation>Свойства элемента</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposeritemwidgetbase.ui" line="25"/>
        <source>Color:</source>
        <translation>Цвет:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposeritemwidgetbase.ui" line="32"/>
        <source>Frame...</source>
        <translation>Рамки...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposeritemwidgetbase.ui" line="39"/>
        <source>Background...</source>
        <translation>Фона...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposeritemwidgetbase.ui" line="46"/>
        <source>Opacity:</source>
        <translation>Непрозрачность:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposeritemwidgetbase.ui" line="63"/>
        <source>Outline width: </source>
        <translation>Ширина контура: </translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposeritemwidgetbase.ui" line="73"/>
        <source>Frame</source>
        <translation>Рамка</translation>
    </message>
</context>
<context>
    <name>QgsComposerLabelWidgetBase</name>
    <message>
        <location filename="../src/ui/qgscomposerlabelwidgetbase.ui" line="19"/>
        <source>Label Options</source>
        <translation>Параметры текста</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlabelwidgetbase.ui" line="38"/>
        <source>Font</source>
        <translation>Шрифт</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlabelwidgetbase.ui" line="45"/>
        <source>Margin (mm):</source>
        <translation>Поле (мм):</translation>
    </message>
</context>
<context>
    <name>QgsComposerLegendItemDialogBase</name>
    <message>
        <location filename="../src/ui/qgscomposerlegenditemdialogbase.ui" line="13"/>
        <source>Legend item properties</source>
        <translation>Свойства элемента легенды</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlegenditemdialogbase.ui" line="19"/>
        <source>Item text:</source>
        <translation>Текст элемента:</translation>
    </message>
</context>
<context>
    <name>QgsComposerLegendWidgetBase</name>
    <message>
        <location filename="../src/ui/qgscomposerlegendwidgetbase.ui" line="19"/>
        <source>Barscale Options</source>
        <translation>Параметры масштабной линейки</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlegendwidgetbase.ui" line="25"/>
        <source>General</source>
        <translation>Общие</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlegendwidgetbase.ui" line="33"/>
        <source>Title:</source>
        <translation>Заголовок:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlegendwidgetbase.ui" line="47"/>
        <source>Font:</source>
        <translation>Шрифт:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlegendwidgetbase.ui" line="60"/>
        <source>Title...</source>
        <translation>Заглавия...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlegendwidgetbase.ui" line="67"/>
        <source>Layer...</source>
        <translation>Слоя...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlegendwidgetbase.ui" line="74"/>
        <source>Item...</source>
        <translation>Элемента...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlegendwidgetbase.ui" line="85"/>
        <source>Symbol width: </source>
        <translation>Ширина знака: </translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlegendwidgetbase.ui" line="95"/>
        <source>Symbol height:</source>
        <translation>Высота знака: </translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlegendwidgetbase.ui" line="105"/>
        <source>Layer space: </source>
        <translation>Отступ слоя: </translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlegendwidgetbase.ui" line="115"/>
        <source>Symbol space:</source>
        <translation>Отступ знака:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlegendwidgetbase.ui" line="129"/>
        <source>Icon label space:</source>
        <translation>Отступ текста:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlegendwidgetbase.ui" line="139"/>
        <source>Box space:</source>
        <translation>Отступ рамки:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlegendwidgetbase.ui" line="154"/>
        <source>Legend items</source>
        <translation>Элементы легенды</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlegendwidgetbase.ui" line="160"/>
        <source>down</source>
        <translation>вниз</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlegendwidgetbase.ui" line="167"/>
        <source>up</source>
        <translation>вверх</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlegendwidgetbase.ui" line="174"/>
        <source>remove</source>
        <translation>удалить</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlegendwidgetbase.ui" line="181"/>
        <source>edit...</source>
        <translation>правка...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlegendwidgetbase.ui" line="188"/>
        <source>update</source>
        <translation>обновить</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerlegendwidgetbase.ui" line="195"/>
        <source>update all</source>
        <translation>обновить все</translation>
    </message>
</context>
<context>
    <name>QgsComposerMap</name>
    <message>
        <location filename="../src/core/composer/qgscomposermap.cpp" line="83"/>
        <source>Map</source>
        <translation>Карта</translation>
    </message>
    <message>
        <location filename="../src/core/composer/qgscomposermap.cpp" line="191"/>
        <source>Map will be printed here</source>
        <translation>Место изображения карты</translation>
    </message>
</context>
<context>
    <name>QgsComposerMapWidget</name>
    <message>
        <location filename="../src/app/composer/qgscomposermapwidget.cpp" line="215"/>
        <source>Cache</source>
        <translation>Кэш</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposermapwidget.cpp" line="223"/>
        <source>Rectangle</source>
        <translation>Прямоугольник</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposermapwidget.cpp" line="219"/>
        <source>Render</source>
        <translation>Отрисовка</translation>
    </message>
</context>
<context>
    <name>QgsComposerMapWidgetBase</name>
    <message>
        <location filename="../src/ui/qgscomposermapwidgetbase.ui" line="19"/>
        <source>Map options</source>
        <translation>Параметры карты</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapwidgetbase.ui" line="25"/>
        <source>&lt;b&gt;Map&lt;/b&gt;</source>
        <translation>&lt;b&gt;Карта&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapwidgetbase.ui" line="45"/>
        <source>Width</source>
        <translation>Ширина</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapwidgetbase.ui" line="61"/>
        <source>Height</source>
        <translation>Высота</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapwidgetbase.ui" line="83"/>
        <source>Scale:</source>
        <translation>Масштаб:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapwidgetbase.ui" line="93"/>
        <source>1:</source>
        <translation>1:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapwidgetbase.ui" line="106"/>
        <source>Map extent</source>
        <translation>Границы карты</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapwidgetbase.ui" line="117"/>
        <source>X min:</source>
        <translation>Мин. X:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapwidgetbase.ui" line="124"/>
        <source>Y min:</source>
        <translation>Мин. Y:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapwidgetbase.ui" line="145"/>
        <source>X max:</source>
        <translation>Макс. X:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapwidgetbase.ui" line="155"/>
        <source>Y max:</source>
        <translation>Макс. Y:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapwidgetbase.ui" line="174"/>
        <source>set to map canvas extent</source>
        <translation>взять с экрана</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapwidgetbase.ui" line="197"/>
        <source>Preview</source>
        <translation>Предпросмотр</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposermapwidgetbase.ui" line="220"/>
        <source>Update preview</source>
        <translation>Обновить</translation>
    </message>
</context>
<context>
    <name>QgsComposerPictureWidget</name>
    <message>
        <location filename="../src/app/composer/qgscomposerpicturewidget.cpp" line="59"/>
        <source>Select svg or image file</source>
        <translation>Выберите файл SVG или изображение</translation>
    </message>
</context>
<context>
    <name>QgsComposerPictureWidgetBase</name>
    <message>
        <location filename="../src/ui/qgscomposerpicturewidgetbase.ui" line="19"/>
        <source>Picture Options</source>
        <translation>Параметры изображения</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerpicturewidgetbase.ui" line="57"/>
        <source>Browse...</source>
        <translation>Обзор...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerpicturewidgetbase.ui" line="74"/>
        <source>Width:</source>
        <translation>Ширина:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerpicturewidgetbase.ui" line="93"/>
        <source>Height:</source>
        <translation>Высота:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerpicturewidgetbase.ui" line="112"/>
        <source>Rotation:</source>
        <translation>Угол поворота:</translation>
    </message>
</context>
<context>
    <name>QgsComposerScaleBar</name>
    <message>
        <location filename="../src/core/composer/qgscomposerscalebar.cpp" line="200"/>
        <source>Single Box</source>
        <translation>Одинарная рамка</translation>
    </message>
    <message>
        <location filename="../src/core/composer/qgscomposerscalebar.cpp" line="204"/>
        <source>Double Box</source>
        <translation>Двойная рамка</translation>
    </message>
    <message>
        <location filename="../src/core/composer/qgscomposerscalebar.cpp" line="211"/>
        <source>Line Ticks Middle</source>
        <translation>Штрих вверх/вниз</translation>
    </message>
    <message>
        <location filename="../src/core/composer/qgscomposerscalebar.cpp" line="215"/>
        <source>Line Ticks Down</source>
        <translation>Штрих вниз</translation>
    </message>
    <message>
        <location filename="../src/core/composer/qgscomposerscalebar.cpp" line="219"/>
        <source>Line Ticks Up</source>
        <translation>Штрих вверх</translation>
    </message>
    <message>
        <location filename="../src/core/composer/qgscomposerscalebar.cpp" line="225"/>
        <source>Numeric</source>
        <translation>Числовой</translation>
    </message>
</context>
<context>
    <name>QgsComposerScaleBarWidget</name>
    <message>
        <location filename="../src/app/composer/qgscomposerscalebarwidget.cpp" line="34"/>
        <source>Single Box</source>
        <translation>Одинарная рамка</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposerscalebarwidget.cpp" line="35"/>
        <source>Double Box</source>
        <translation>Двойная рамка</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposerscalebarwidget.cpp" line="36"/>
        <source>Line Ticks Middle</source>
        <translation>Штрих вверх/вниз</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposerscalebarwidget.cpp" line="37"/>
        <source>Line Ticks Down</source>
        <translation>Штрих вниз</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposerscalebarwidget.cpp" line="38"/>
        <source>Line Ticks Up</source>
        <translation>Штрих вверх</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposerscalebarwidget.cpp" line="39"/>
        <source>Numeric</source>
        <translation>Числовой</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscomposerscalebarwidget.cpp" line="148"/>
        <source>Map </source>
        <translation>Карта </translation>
    </message>
</context>
<context>
    <name>QgsComposerScaleBarWidgetBase</name>
    <message>
        <location filename="../src/ui/qgscomposerscalebarwidgetbase.ui" line="19"/>
        <source>Barscale Options</source>
        <translation>Параметры масштабной линейки</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarwidgetbase.ui" line="25"/>
        <source>Segment size (map units):</source>
        <translation>Размер сегмента (единицы карты):</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarwidgetbase.ui" line="62"/>
        <source>Number of segments:</source>
        <translation>Количество сегментов:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarwidgetbase.ui" line="75"/>
        <source>Segments left:</source>
        <translation>Сегменты слева:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarwidgetbase.ui" line="85"/>
        <source>Style:</source>
        <translation>Стиль:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarwidgetbase.ui" line="189"/>
        <source>Unit label:</source>
        <translation>Обозначение единиц:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarwidgetbase.ui" line="45"/>
        <source>Map units per bar unit:</source>
        <translation>Единиц карты в делении:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarwidgetbase.ui" line="101"/>
        <source>Map:</source>
        <translation>Карта:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarwidgetbase.ui" line="127"/>
        <source>Height (mm):</source>
        <translation>Высота (мм):</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarwidgetbase.ui" line="137"/>
        <source>Line width:</source>
        <translation>Ширина линии:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarwidgetbase.ui" line="169"/>
        <source>Label space:</source>
        <translation>Отступ метки:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarwidgetbase.ui" line="179"/>
        <source>Box space:</source>
        <translation>Отступ рамки:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarwidgetbase.ui" line="211"/>
        <source>Font...</source>
        <translation>Шрифт...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposerscalebarwidgetbase.ui" line="218"/>
        <source>Color...</source>
        <translation>Цвет...</translation>
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
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="53"/>
        <source>Preview</source>
        <translation>Предпросмотр</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="149"/>
        <source>Layers</source>
        <translation>Слои</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="154"/>
        <source>Group</source>
        <translation>Группа</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscomposervectorlegendbase.ui" line="159"/>
        <source>ID</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>QgsCompositionBase</name>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="19"/>
        <source>Composition</source>
        <translation>Композиция</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="43"/>
        <source>Paper</source>
        <translation>Бумага</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="182"/>
        <source>Size</source>
        <translation>Размер</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="166"/>
        <source>Units</source>
        <translation>Единицы</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="150"/>
        <source>Width</source>
        <translation>Ширина</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="134"/>
        <source>Height</source>
        <translation>Высота</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionbase.ui" line="118"/>
        <source>Orientation</source>
        <translation>Ориентация</translation>
    </message>
</context>
<context>
    <name>QgsCompositionWidget</name>
    <message>
        <location filename="../src/app/composer/qgscompositionwidget.cpp" line="241"/>
        <source>Landscape</source>
        <translation>Альбом</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscompositionwidget.cpp" line="245"/>
        <source>Portrait</source>
        <translation>Портрет</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscompositionwidget.cpp" line="110"/>
        <source>Custom</source>
        <translation>Пользовательский</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscompositionwidget.cpp" line="63"/>
        <source>A5 (148x210 mm)</source>
        <translation>A5 (148x210 мм)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscompositionwidget.cpp" line="65"/>
        <source>A4 (210x297 mm)</source>
        <translation>A4 (210x297 мм)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscompositionwidget.cpp" line="67"/>
        <source>A3 (297x420 mm)</source>
        <translation>A3 (297x420 мм)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscompositionwidget.cpp" line="69"/>
        <source>A2 (420x594 mm)</source>
        <translation>A2 (420x594 мм)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscompositionwidget.cpp" line="71"/>
        <source>A1 (594x841 mm)</source>
        <translation>A1 (594x841 мм)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscompositionwidget.cpp" line="73"/>
        <source>A0 (841x1189 mm)</source>
        <translation>A0 (841x1189 мм)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscompositionwidget.cpp" line="75"/>
        <source>B5 (176 x 250 mm)</source>
        <translation>B5 (176 x 250 мм)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscompositionwidget.cpp" line="77"/>
        <source>B4 (250 x 353 mm)</source>
        <translation>B4 (250 x 353 мм)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscompositionwidget.cpp" line="79"/>
        <source>B3 (353 x 500 mm)</source>
        <translation>B3 (353 x 500 мм)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscompositionwidget.cpp" line="81"/>
        <source>B2 (500 x 707 mm)</source>
        <translation>B2 (500 x 707 мм)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscompositionwidget.cpp" line="83"/>
        <source>B1 (707 x 1000 mm)</source>
        <translation>B1 (707 x 1000 мм)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscompositionwidget.cpp" line="85"/>
        <source>B0 (1000 x 1414 mm)</source>
        <translation>B0 (1000 x 1414 мм)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscompositionwidget.cpp" line="87"/>
        <source>Letter (8.5x11 inches)</source>
        <translation>Letter (8.5x11 дюймов)</translation>
    </message>
    <message>
        <location filename="../src/app/composer/qgscompositionwidget.cpp" line="89"/>
        <source>Legal (8.5x14 inches)</source>
        <translation>Legal (8.5x14 дюймов)</translation>
    </message>
</context>
<context>
    <name>QgsCompositionWidgetBase</name>
    <message>
        <location filename="../src/ui/qgscompositionwidgetbase.ui" line="19"/>
        <source>Composition</source>
        <translation>Композиция</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionwidgetbase.ui" line="43"/>
        <source>Paper</source>
        <translation>Бумага</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionwidgetbase.ui" line="118"/>
        <source>Orientation</source>
        <translation>Ориентация</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionwidgetbase.ui" line="134"/>
        <source>Height</source>
        <translation>Высота</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionwidgetbase.ui" line="150"/>
        <source>Width</source>
        <translation>Ширина</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionwidgetbase.ui" line="166"/>
        <source>Units</source>
        <translation>Единицы</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionwidgetbase.ui" line="182"/>
        <source>Size</source>
        <translation>Размер</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscompositionwidgetbase.ui" line="217"/>
        <source>Print quality (dpi)</source>
        <translation>Качество печати (dpi)</translation>
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
        <source>The source spatial reference system (CRS) is not valid. </source>
        <translation>Неверная исходная система координат. </translation>
    </message>
    <message>
        <location filename="../src/core/qgscoordinatetransform.cpp" line="426"/>
        <source>The coordinates can not be reprojected. The CRS is: </source>
        <translation>Не удалось спроецировать координаты. Система координат: </translation>
    </message>
    <message>
        <location filename="../src/core/qgscoordinatetransform.cpp" line="425"/>
        <source>The destination spatial reference system (CRS) is not valid. </source>
        <translation>Неверная целевая система координат. </translation>
    </message>
</context>
<context>
    <name>QgsCopyrightLabelPlugin</name>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="64"/>
        <source>Bottom Left</source>
        <translation>Внизу слева</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="65"/>
        <source>Top Left</source>
        <translation>Вверху слева</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="65"/>
        <source>Top Right</source>
        <translation>Вверху справа</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="65"/>
        <source>Bottom Right</source>
        <translation>Внизу справа</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="77"/>
        <source>&amp;Copyright Label</source>
        <translation>&amp;Знак авторского права</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="78"/>
        <source>Creates a copyright label that is displayed on the map canvas.</source>
        <translation>Добавляет в область карты знак авторского права.</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/plugin.cpp" line="202"/>
        <source>&amp;Decorations</source>
        <translation>&amp;Оформление</translation>
    </message>
</context>
<context>
    <name>QgsCopyrightLabelPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="13"/>
        <source>Copyright Label Plugin</source>
        <translation>Модуль знака авторского права</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="185"/>
        <source>Placement</source>
        <translation>Размещение</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="193"/>
        <source>Bottom Left</source>
        <translation>Внизу слева</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="198"/>
        <source>Top Left</source>
        <translation>Вверху слева</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="203"/>
        <source>Bottom Right</source>
        <translation>Внизу справа</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="208"/>
        <source>Top Right</source>
        <translation>Вверху справа</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="216"/>
        <source>Orientation</source>
        <translation>Ориентация</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="224"/>
        <source>Horizontal</source>
        <translation>Горизонтальная</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="229"/>
        <source>Vertical</source>
        <translation>Вертикальная</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="146"/>
        <source>Enable Copyright Label</source>
        <translation>Включить знак авторского права</translation>
    </message>
    <message>
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="57"/>
        <source>Color</source>
        <translation>Цвет</translation>
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
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;Введите в следующем поле знак авторского права. Для форматирования знака разрешается использовать базовую HTML-разметку, например:&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;&lt;span style=&quot; font-weight:600;&quot;&gt;&amp;lt;B&amp;gt;Полужирный шрифт&amp;lt;/B&amp;gt; &lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:600;&quot;&gt;&lt;span style=&quot; font-weight:400; font-style:italic;&quot;&gt;&amp;lt;I&amp;gt;Курсив&amp;lt;/I&amp;gt;&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-style:italic;&quot;&gt;&lt;span style=&quot; font-style:normal;&quot;&gt;(примечание: знак &amp;copy; задаётся последовательностью «&amp;amp;copy;»)&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message encoding="UTF-8">
        <location filename="../src/plugins/copyright_label/pluginguibase.ui" line="158"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;© QGIS 2008&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>QgsCustomProjectionDialog</name>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="170"/>
        <source>Delete Projection Definition?</source>
        <translation>Удалить определение проекции?</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="171"/>
        <source>Deleting a projection definition is not reversable. Do you want to delete it?</source>
        <translation>Удаление определения проекции ‒ необратимая операция. Вы уверены, что хотите удалить его?</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="847"/>
        <source>Abort</source>
        <translation>Отменить</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="849"/>
        <source>New</source>
        <translation>Создать</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="903"/>
        <source>QGIS Custom Projection</source>
        <translation>Пользовательская проекция QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="720"/>
        <source>This proj4 projection definition is not valid. Please give the projection a name before pressing save.</source>
        <translation>Неверное определение проекции proj4. Пожалуйста, введите имя проекции перед сохранением.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="726"/>
        <source>This proj4 projection definition is not valid. Please add the parameters before pressing save.</source>
        <translation>Неверное определение проекции proj4. Пожалуйста, введите параметры проекции перед сохранением.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="741"/>
        <source>This proj4 projection definition is not valid. Please add a proj= clause before pressing save.</source>
        <translation>Неверное определение проекции proj4. Пожалуйста, введите условие proj= перед сохранением.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="772"/>
        <source>This proj4 projection definition is not valid. Please correct before pressing save.</source>
        <translation>Неверное определение проекции proj4. Пожалуйста, исправьте его перед сохранением.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="877"/>
        <source>This proj4 projection definition is not valid.</source>
        <translation>Неверное определение проекции proj4.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="892"/>
        <source>Northing and Easthing must be in decimal form.</source>
        <translation>Север и восток следует вводить в десятичной форме.</translation>
    </message>
    <message>
        <location filename="../src/app/qgscustomprojectiondialog.cpp" line="904"/>
        <source>Internal Error (source projection invalid?)</source>
        <translation>Внутренняя ошибка (неверная исходная проекция?)</translation>
    </message>
</context>
<context>
    <name>QgsCustomProjectionDialogBase</name>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="22"/>
        <source>Define</source>
        <translation>Определение</translation>
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
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="151"/>
        <source>Test</source>
        <translation>Проверка</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="253"/>
        <source>Calculate</source>
        <translation>Расчитать</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="180"/>
        <source>Geographic / WGS84</source>
        <translation>Географическая / WGS84</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="38"/>
        <source>Name</source>
        <translation>Имя</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="167"/>
        <source>Parameters</source>
        <translation>Параметры</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="116"/>
        <source>*</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="126"/>
        <source>S</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="136"/>
        <source>X</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="194"/>
        <source>North</source>
        <translation>Север</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="224"/>
        <source>East</source>
        <translation>Восток</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="13"/>
        <source>Custom Coordinate Reference System Definition</source>
        <translation>Определение пользовательской системы координат</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="28"/>
        <source>You can define your own custom Coordinate Reference System (CRS) here. The definition must conform to the proj4 format for specifying a CRS.</source>
        <translation>В этом диалоге вы можете определить вашу собственную систему координат. Определение должно быть задано в формате координатных систем PROJ4.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="157"/>
        <source>Use the text boxes below to test the CRS definition you are creating. Enter a coordinate where both the lat/long and the transformed result are known (for example by reading off a map). Then press the calculate button to see if the CRS definition you are creating is accurate.</source>
        <translation>Используйте данные поля для проверки вновь созданной системы координат. Введите точку для которой известны широта/долгота и прямоугольные координаты (например, с карты). После этого нажмите кнопку «Расчитать» и проверьте, верно ли задана ваша система координат.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgscustomprojectiondialogbase.ui" line="187"/>
        <source>Destination CRS        </source>
        <translation>Целевая        </translation>
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
        <location filename="../src/app/qgsdbsourceselect.cpp" line="434"/>
        <source>Connection failed</source>
        <translation>Не удалось соединиться</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="147"/>
        <source>Type</source>
        <translation>Тип</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="155"/>
        <source>Sql</source>
        <translation>SQL</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="437"/>
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
        <location filename="../src/app/qgsdbsourceselect.cpp" line="544"/>
        <source>Accessible tables could not be determined</source>
        <translation>Не удалось распознать доступные таблицы</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="546"/>
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
        <location filename="../src/app/qgsdbsourceselect.cpp" line="551"/>
        <source>No accessible tables found</source>
        <translation>Доступные таблицы не найдены</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbsourceselect.cpp" line="555"/>
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
        <location filename="../src/app/qgsdbtablemodel.cpp" line="25"/>
        <source>Schema</source>
        <translation>Схема</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="26"/>
        <source>Table</source>
        <translation>Таблица</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="27"/>
        <source>Type</source>
        <translation>Тип</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="28"/>
        <source>Geometry column</source>
        <translation>Поле геометрии</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="29"/>
        <source>Sql</source>
        <translation>SQL</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="237"/>
        <source>Point</source>
        <translation>Точка</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="241"/>
        <source>Multipoint</source>
        <translation>Мультиточка</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="245"/>
        <source>Line</source>
        <translation>Линия</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="249"/>
        <source>Multiline</source>
        <translation>Мультилиния</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="253"/>
        <source>Polygon</source>
        <translation>Полигон</translation>
    </message>
    <message>
        <location filename="../src/app/qgsdbtablemodel.cpp" line="257"/>
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
</context>
<context>
    <name>QgsDelimitedTextPlugin</name>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="99"/>
        <source>&amp;Add Delimited Text Layer</source>
        <translation>Добавить слой из &amp;текста с разделителями</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="102"/>
        <source>Add a delimited text file as a map layer. </source>
        <translation>Добавить текстовый файл с разделителями как слой карты.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="103"/>
        <source>The file must have a header row containing the field names. </source>
        <translation>Файл должен включать строку заголовка, содержащую имена полей.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="103"/>
        <source>X and Y fields are required and must contain coordinates in decimal units.</source>
        <translation>Поля X и Y обязательны и должны содержать координаты в десятичных единицах.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="140"/>
        <source>&amp;Delimited text</source>
        <translation>&amp;Текст с разделителями</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugin.cpp" line="54"/>
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
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="207"/>
        <source>No delimiter</source>
        <translation>Разделитель не указан</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="207"/>
        <source>Please specify a delimiter prior to parsing the file</source>
        <translation>Пожалуйста, укажите разделитель до начала загрузки файла</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextplugingui.cpp" line="241"/>
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
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="91"/>
        <source>&lt;p align=&quot;right&quot;&gt;X field&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;right&quot;&gt;X-поле&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="110"/>
        <source>Name of the field containing x values</source>
        <translation>Имя поля, содержащего X-значения</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="113"/>
        <source>Name of the field containing x values. Choose a field from the list. The list is generated by parsing the header row of the delimited text file.</source>
        <translation>Имя поля, содержащего X-значения. Выберите поле из списка, создаваемого анализом строки заголовка в текстовом файле.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="123"/>
        <source>&lt;p align=&quot;right&quot;&gt;Y field&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;right&quot;&gt;Y-поле&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="142"/>
        <source>Name of the field containing y values</source>
        <translation>Имя поля, содержащего Y-значения</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="145"/>
        <source>Name of the field containing y values. Choose a field from the list. The list is generated by parsing the header row of the delimited text file.</source>
        <translation>Имя поля, содержащего Y-значения. Выберите поле из списка, создаваемого анализом строки заголовка в текстовом файле.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="229"/>
        <source>Layer name</source>
        <translation>Имя слоя</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="236"/>
        <source>Name to display in the map legend</source>
        <translation>Имя для отображения в легенде карты</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="239"/>
        <source>Name displayed in the map legend</source>
        <translation>Имя, отображаемое в легенде карты</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="354"/>
        <source>Delimiter</source>
        <translation>Разделитель</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="373"/>
        <source>Delimiter to use when splitting fields in the text file. The delimiter can be more than one character.</source>
        <translation>Разделитель полей в текстовом файле. Разделитель может состоять из более чем одного символа.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="376"/>
        <source>Delimiter to use when splitting fields in the delimited text file. The delimiter can be 1 or more characters in length.</source>
        <translation>Разделитель полей в текстовом файле. Разделитель может состоять из одного и более символов.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="51"/>
        <source>Delimited Text Layer</source>
        <translation>Слой текста с разделителями</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="174"/>
        <source>Delimited text file</source>
        <translation>Текстовый файл</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="181"/>
        <source>Full path to the delimited text file</source>
        <translation>Полный путь к текстовому файлу</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="184"/>
        <source>Full path to the delimited text file. In order to properly parse the fields in the file, the delimiter must be defined prior to entering the file name. Use the Browse button to the right of this field to choose the input file.</source>
        <translation>Полный путь к текстовому файлу. Для обеспечения правильности анализа файла, разделитель следует указывать перед вводом имени файла. Нажмите кнопку «Обзор» для интерактивного выбора файла.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="197"/>
        <source>Browse to find the delimited text file to be processed</source>
        <translation>Выбор текстового файла для обработки</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="200"/>
        <source>Use this button to browse to the location of the delimited text file. This button will not be enabled until a delimiter has been entered in the &lt;i&gt;Delimiter&lt;/i&gt; box. Once a file is chosen, the X and Y field drop-down boxes will be populated with the fields from the delimited text file.</source>
        <translation>Используйте эту кнопку для выбора текстового файла. Кнопка не будет активирована, пока разделитель не будет введён в поле &lt;i&gt;Разделитель&lt;/i&gt;. После того, как файл будет выбран, списки X и Y-полей будут заполнены именами полей из текстового файла.</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="284"/>
        <source>Sample text</source>
        <translation>Образец</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="203"/>
        <source>Browse...</source>
        <translation>Обзор...</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="386"/>
        <source>The delimiter is taken as is</source>
        <translation>Разделитель используется как есть</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="389"/>
        <source>Plain characters</source>
        <translation>Простой текст</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="399"/>
        <source>The delimiter is a regular expression</source>
        <translation>Разделитель является регулярным выражением</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="402"/>
        <source>Regular expression</source>
        <translation>Регулярное выражение</translation>
    </message>
    <message>
        <location filename="../src/plugins/delimited_text/qgsdelimitedtextpluginguibase.ui" line="64"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation></translation>
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
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdetaileditemwidgetbase.ui" line="96"/>
        <source>Heading Label</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsdetaileditemwidgetbase.ui" line="117"/>
        <source>Detail label</source>
        <translation></translation>
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
    <name>QgsEncodingFileDialog</name>
    <message>
        <location filename="../src/gui/qgsencodingfiledialog.cpp" line="29"/>
        <source>Encoding:</source>
        <translation>Кодировка:</translation>
    </message>
</context>
<context>
    <name>QgsGPSDeviceDialog</name>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialog.cpp" line="45"/>
        <source>New device %1</source>
        <translation>Новое устройство %1</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialog.cpp" line="59"/>
        <source>Are you sure?</source>
        <translation>Вы уверены?</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialog.cpp" line="60"/>
        <source>Are you sure that you want to delete this device?</source>
        <translation>Вы уверены, что хотите удалить это устройство?</translation>
    </message>
</context>
<context>
    <name>QgsGPSDeviceDialogBase</name>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="19"/>
        <source>GPS Device Editor</source>
        <translation>Редактор GPS-устройств</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="154"/>
        <source>This is the name of the device as it will appear in the lists</source>
        <translation>Имя устройства, которое отображается в списке</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="92"/>
        <source>Update device</source>
        <translation>Обновить устройство</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="79"/>
        <source>Delete device</source>
        <translation>Удалить устройство</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="66"/>
        <source>New device</source>
        <translation>Новое устройство</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="163"/>
        <source>Commands</source>
        <translation>Команды</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="204"/>
        <source>Waypoint download:</source>
        <translation>Загрузка точек:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="267"/>
        <source>Waypoint upload:</source>
        <translation>Выгрузка точек:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="218"/>
        <source>Route download:</source>
        <translation>Загрузка маршрутов:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="197"/>
        <source>Route upload:</source>
        <translation>Выгрузка маршрутов:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="190"/>
        <source>Track download:</source>
        <translation>Загрузка треков:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="260"/>
        <source>The command that is used to upload tracks to the device</source>
        <translation>Команда, используемая для выгрузки треков в устройство</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="232"/>
        <source>Track upload:</source>
        <translation>Выгрузка треков:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="239"/>
        <source>The command that is used to download tracks from the device</source>
        <translation>Команда, используемая для загрузки треков из устройства</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="246"/>
        <source>The command that is used to upload routes to the device</source>
        <translation>Команда, используемая для выгрузки маршрутов в устройство</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="211"/>
        <source>The command that is used to download routes from the device</source>
        <translation>Команда, используемая для загрузки маршрутов из устройства</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="225"/>
        <source>The command that is used to upload waypoints to the device</source>
        <translation>Команда, используемая для выгрузки точек в устройство</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="253"/>
        <source>The command that is used to download waypoints from the device</source>
        <translation>Команда, используемая для загрузки точек из устройства</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="141"/>
        <source>Device name</source>
        <translation>Имя устройства</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsdevicedialogbase.ui" line="283"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;In the download and upload commands there can be special words that will be replaced by QGIS when the commands are used. These words are:&lt;span style=&quot; font-style:italic;&quot;&gt;%babel&lt;/span&gt; - the path to GPSBabel&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%in&lt;/span&gt; - the GPX filename when uploading or the port when downloading&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%out&lt;/span&gt; - the port when uploading or the GPX filename when downloading&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;В командах загрузки и выгрузки допускаются специальные слова, которые QGIS заменяет во время выполнения команд. Этими словами являются:&lt;br/&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%babel&lt;/span&gt; — путь к программе GPSBabel&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%in&lt;/span&gt; — GPX-файл при выгрузке или порт при загрузке&lt;br /&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;%out&lt;/span&gt; — порт при выгрузке или GPX-файл при загрузке&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
</context>
<context>
    <name>QgsGPSPlugin</name>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="90"/>
        <source>&amp;Gps Tools</source>
        <translation>Инструменты &amp;GPS</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="91"/>
        <source>&amp;Create new GPX layer</source>
        <translation>&amp;Создать новый GPX-слой</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="94"/>
        <source>Creates a new GPX layer and displays it on the map canvas</source>
        <translation>Создать новый GPX-слой и вывести его на карте</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="200"/>
        <source>&amp;Gps</source>
        <translation>&amp;GPS</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="160"/>
        <source>Save new GPX file as...</source>
        <translation>Сохранить новый GPX-файл как...</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="162"/>
        <source>GPS eXchange file (*.gpx)</source>
        <translation>Файлы GPS eXchange (*.gpx)</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="169"/>
        <source>Could not create file</source>
        <translation>Не удалось создать файл</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="171"/>
        <source>Unable to create a GPX file with the given name. </source>
        <translation>Не удалось создать GPX-файл с заданным именем. </translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="172"/>
        <source>Try again with another name or in another </source>
        <translation>Попробуйте ещё раз с другим именем или в другом </translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="172"/>
        <source>directory.</source>
        <translation>каталоге.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="213"/>
        <source>GPX Loader</source>
        <translation>Загрузчик GPX</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="215"/>
        <source>Unable to read the selected file.
</source>
        <translation>Не удалось прочитать выбранный файл.
</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="215"/>
        <source>Please reselect a valid file.</source>
        <translation>Пожалуйста, выберите правильный файл.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="516"/>
        <source>Could not start process</source>
        <translation>Не удалось запустить процесс</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="517"/>
        <source>Could not start GPSBabel!</source>
        <translation>Не удалось запустить GPSBabel!</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="342"/>
        <source>Importing data...</source>
        <translation>Импорт данных...</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="522"/>
        <source>Cancel</source>
        <translation>Отменить</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="284"/>
        <source>Could not import data from %1!

</source>
        <translation>Ошибка импорта данных из %1!

</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="286"/>
        <source>Error importing data</source>
        <translation>Ошибка импорта данных</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="504"/>
        <source>Not supported</source>
        <translation>Функция не поддерживается</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="413"/>
        <source>This device does not support downloading </source>
        <translation>Это устройство не поддерживает загрузку </translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="413"/>
        <source>of </source>
        <translation>данных типа </translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="430"/>
        <source>Downloading data...</source>
        <translation>Загрузка данных...</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="443"/>
        <source>Could not download data from GPS!

</source>
        <translation>Ошибка загрузки данных из GPS!

</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="445"/>
        <source>Error downloading data</source>
        <translation>Ошибка загрузки данных</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="505"/>
        <source>This device does not support uploading of </source>
        <translation>Это устройство не поддерживает выгрузку данных типа  </translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="522"/>
        <source>Uploading data...</source>
        <translation>Выгрузка данных...</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="535"/>
        <source>Error while uploading data to GPS!

</source>
        <translation>Ошибка выгрузки данных в GPS!

</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="537"/>
        <source>Error uploading data</source>
        <translation>Ошибка выгрузки данных</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="356"/>
        <source>Could not convert data from %1!

</source>
        <translation>Не удалось преобразовать данные из %1!</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugin.cpp" line="358"/>
        <source>Error converting data</source>
        <translation>Ошибка преобразования данных</translation>
    </message>
</context>
<context>
    <name>QgsGPSPluginGui</name>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="571"/>
        <source>GPS eXchange format (*.gpx)</source>
        <translation>Формат GPS eXchange (*.gpx)</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="557"/>
        <source>Select GPX file</source>
        <translation>Выберите GPX-файл</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="261"/>
        <source>Select file and format to import</source>
        <translation>Выберите файл и формат для импорта</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="532"/>
        <source>Waypoints</source>
        <translation>Маршрутные точки</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="532"/>
        <source>Routes</source>
        <translation>Маршруты</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="285"/>
        <source>Tracks</source>
        <translation>Треки</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="537"/>
        <source>QGIS can perform conversions of GPX files, by using GPSBabel (%1) to perform the conversions.</source>
        <translation>QGIS может выполнять преобразование GPX-файлов при помощи пакета GPSBabel (%1).</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="538"/>
        <source>This requires that you have GPSBabel installed where QGIS can find it.</source>
        <translation>Для этого требуется установить пакет GPSBabel так, чтобы он мог быть найден QGIS.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="464"/>
        <source>GPX is the %1, which is used to store information about waypoints, routes, and tracks.</source>
        <translation>GPX (%1) — это формат, используемый для хранения маршрутных точек, маршрутов и треков.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="464"/>
        <source>GPS eXchange file format</source>
        <translation>Формат GPS eXchange</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="465"/>
        <source>Select a GPX file and then select the feature types that you want to load.</source>
        <translation>Выберите GPX-файл и типы объектов, которые вы хотели бы загрузить.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="478"/>
        <source>This tool will help you download data from a GPS device.</source>
        <translation>Этот инструмент поможет вам загрузить данные с GPS-устройства.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="479"/>
        <source>Choose your GPS device, the port it is connected to, the feature type you want to download, a name for your new layer, and the GPX file where you want to store the data.</source>
        <translation>Выберите ваше GPS-устройство и порт, к которому оно подключено, а также тип объектов, которые вы хотите загрузить, имя нового слоя и GPX-файл для сохранения данных.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="499"/>
        <source>If your device isn&apos;t listed, or if you want to change some settings, you can also edit the devices.</source>
        <translation>Если вашего устройства нет в списке или вы хотите изменить его параметры, нажмите «Редактировать устройства».</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="501"/>
        <source>This tool uses the program GPSBabel (%1) to transfer the data.</source>
        <translation>Этот инструмент использует GPSBabel (%1) для передачи данных.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="497"/>
        <source>This tool will help you upload data from a GPX layer to a GPS device.</source>
        <translation>Этот инстумент поможет вам выгрузить данные в GPS-устройство из существующего GPX-слоя.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="498"/>
        <source>Choose the layer you want to upload, the device you want to upload it to, and the port your device is connected to.</source>
        <translation>Выберите слой, который вы желаете выгрузить, устройство для выгрузки и порт, к которому оно подключено.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="517"/>
        <source>QGIS can only load GPX files by itself, but many other formats can be converted to GPX using GPSBabel (%1).</source>
        <translation>QGIS может загружать только GPX-файлы, но прочие форматы могут быть преобразованы в GPX при помощи GPSBabel (%1).</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="520"/>
        <source>All file formats can not store waypoints, routes, and tracks, so some feature types may be disabled for some file formats.</source>
        <translation>Не все форматы могут содержать маршрутные точки, маршруты и треки, поэтому для некоторых форматов часть типов данных будет выключена.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="569"/>
        <source>Choose a file name to save under</source>
        <translation>Выберите имя сохраняемого файла</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="519"/>
        <source>Select a GPS file format and the file that you want to import, the feature type that you want to use, a GPX file name that you want to save the converted file as, and a name for the new layer.</source>
        <translation>Выберите формат GPS-данных и файл для импорта, а также тип загружаемых объектов, имя GPX-файла, в который будет сохранён результат и имя нового слоя.</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpsplugingui.cpp" line="539"/>
        <source>Select a GPX input file name, the type of conversion you want to perform, a GPX file name that you want to save the converted file as, and a name for the new layer created from the result.</source>
        <translation>Выберите исходный GPX-файл, тип преобразования, которое вы хотели бы осуществить, а также имя файла, в котором будет сохранён результат и имя нового слоя.</translation>
    </message>
</context>
<context>
    <name>QgsGPSPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="13"/>
        <source>GPS Tools</source>
        <translation>Инструменты GPS</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="70"/>
        <source>Load GPX file</source>
        <translation>GPX-файлы</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="116"/>
        <source>File:</source>
        <translation>Файл:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="133"/>
        <source>Feature types:</source>
        <translation>Типы объектов:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="419"/>
        <source>Waypoints</source>
        <translation>Маршрутные точки</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="424"/>
        <source>Routes</source>
        <translation>Маршруты</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="429"/>
        <source>Tracks</source>
        <translation>Треки</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="180"/>
        <source>Import other file</source>
        <translation>Прочие файлы</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="298"/>
        <source>File to import:</source>
        <translation>Импортируемый файл:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="405"/>
        <source>Feature type:</source>
        <translation>Тип объектов:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="641"/>
        <source>GPX output file:</source>
        <translation>Выходной GPX-файл:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="617"/>
        <source>Layer name:</source>
        <translation>Имя слоя:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="306"/>
        <source>Download from GPS</source>
        <translation>Загрузка с GPS</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="555"/>
        <source>Edit devices</source>
        <translation>Редактировать устройства</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="562"/>
        <source>GPS device:</source>
        <translation>GPS-устройство:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="437"/>
        <source>Output file:</source>
        <translation>Файл вывода:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="528"/>
        <source>Port:</source>
        <translation>Порт:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="465"/>
        <source>Upload to GPS</source>
        <translation>Выгрузка в GPS</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="569"/>
        <source>Data layer:</source>
        <translation>Слой данных:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="631"/>
        <source>Browse...</source>
        <translation>Обзор...</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="624"/>
        <source>Save As...</source>
        <translation>Сохранить как...</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="284"/>
        <source>(Note: Selecting correct file type in browser dialog important!)</source>
        <translation>(Внимание: важно выбрать правильный тип файла в диалоге выбора файлов!)</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="577"/>
        <source>GPX Conversions</source>
        <translation>Конвертеры GPX</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="674"/>
        <source>Conversion:</source>
        <translation>Преобразование:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="688"/>
        <source>GPX input file:</source>
        <translation>Исходный GPX-файл:</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="604"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="342"/>
        <source>Edit devices...</source>
        <translation>Редактировать устройства...</translation>
    </message>
    <message>
        <location filename="../src/plugins/gps_importer/qgsgpspluginguibase.ui" line="382"/>
        <source>Refresh</source>
        <translation>Обновить</translation>
    </message>
</context>
<context>
    <name>QgsGPXProvider</name>
    <message>
        <location filename="../src/providers/gpx/qgsgpxprovider.cpp" line="72"/>
        <source>Bad URI - you need to specify the feature type.</source>
        <translation>Неверный URI — необходимо указать тип объектов.</translation>
    </message>
    <message>
        <location filename="../src/providers/gpx/qgsgpxprovider.cpp" line="120"/>
        <source>GPS eXchange file</source>
        <translation>Файлы GPS eXchange (*.gpx)</translation>
    </message>
    <message>
        <location filename="../src/providers/gpx/qgsgpxprovider.cpp" line="815"/>
        <source>Digitized in QGIS</source>
        <translation>Оцифрован в QGIS</translation>
    </message>
</context>
<context>
    <name>QgsGenericProjectionSelector</name>
    <message>
        <location filename="../src/gui/qgsgenericprojectionselector.cpp" line="43"/>
        <source>Define this layer&apos;s projection:</source>
        <translation>Укажите проекцию слоя:</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsgenericprojectionselector.cpp" line="44"/>
        <source>This layer appears to have no projection specification.</source>
        <translation>Этот слой не содержит сведений о проекции.</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsgenericprojectionselector.cpp" line="46"/>
        <source>By default, this layer will now have its projection set to that of the project, but you may override this by selecting a different projection below.</source>
        <translation>По умолчанию, для этого слоя будет выбрана проекция текущего проекта, но вы можете переопределить её, выбрав другую проекцию ниже.</translation>
    </message>
</context>
<context>
    <name>QgsGenericProjectionSelectorBase</name>
    <message>
        <location filename="../src/ui/qgsgenericprojectionselectorbase.ui" line="13"/>
        <source>Projection Selector</source>
        <translation>Выбор проекции</translation>
    </message>
</context>
<context>
    <name>QgsGeomTypeDialog</name>
    <message>
        <location filename="../src/app/qgsgeomtypedialog.cpp" line="30"/>
        <source>Real</source>
        <translation>Действительное число</translation>
    </message>
    <message>
        <location filename="../src/app/qgsgeomtypedialog.cpp" line="31"/>
        <source>Integer</source>
        <translation>Целое число</translation>
    </message>
    <message>
        <location filename="../src/app/qgsgeomtypedialog.cpp" line="32"/>
        <source>String</source>
        <translation>Строка</translation>
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
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="22"/>
        <source>File format</source>
        <translation>Формат файла</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="65"/>
        <source>Attributes</source>
        <translation>Атрибуты</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="150"/>
        <source>Name</source>
        <translation>Имя</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="127"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="124"/>
        <source>Add attribute</source>
        <translation>Добавить атрибут</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgeomtypedialogbase.ui" line="111"/>
        <source>Delete selected attribute</source>
        <translation>Удалить выбранный атрибут</translation>
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
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="86"/>
        <source>Choose a raster file</source>
        <translation>Выберите растровый файл</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="88"/>
        <source>Raster files (*.*)</source>
        <translation>Растровые файлы (*.*)</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="99"/>
        <source>Error</source>
        <translation>Ошибка</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="100"/>
        <source>The selected file is not a valid raster file.</source>
        <translation>Выбранный файл не является действительным растровым файлом.</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="127"/>
        <source>World file exists</source>
        <translation>Файл привязки уже существует</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="129"/>
        <source>&lt;p&gt;The selected file already seems to have a </source>
        <translation>&lt;p&gt;Судя по всему выбранный файл уже имеет </translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="130"/>
        <source>world file! Do you want to replace it with the </source>
        <translation>файл привязки! Вы хотите заменить его </translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/plugingui.cpp" line="130"/>
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
        <location filename="../src/plugins/georeferencer/pluginguibase.ui" line="99"/>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/pluginguibase.ui" line="22"/>
        <source>Raster file:</source>
        <translation>Растровый файл:</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/pluginguibase.ui" line="76"/>
        <source>Arrange plugin windows</source>
        <translation>Выровнять окна модуля</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/pluginguibase.ui" line="45"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/pluginguibase.ui" line="69"/>
        <source>Description...</source>
        <translation>Описание...</translation>
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
        <location filename="../src/app/qgsgraduatedsymboldialog.cpp" line="368"/>
        <source>Equal Interval</source>
        <translation>Равные интервалы</translation>
    </message>
    <message>
        <location filename="../src/app/qgsgraduatedsymboldialog.cpp" line="343"/>
        <source>Quantiles</source>
        <translation>Квантили</translation>
    </message>
    <message>
        <location filename="../src/app/qgsgraduatedsymboldialog.cpp" line="394"/>
        <source>Empty</source>
        <translation>Пустые значения</translation>
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
        <translation>Поле классификации</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="93"/>
        <source>Mode</source>
        <translation>Режим</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsgraduatedsymboldialogbase.ui" line="131"/>
        <source>Number of classes</source>
        <translation>Количество классов</translation>
    </message>
</context>
<context>
    <name>QgsGrassAttributes</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="266"/>
        <source>Warning</source>
        <translation>Внимание</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="122"/>
        <source>Column</source>
        <translation>Поле</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="123"/>
        <source>Value</source>
        <translation>Значение</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="123"/>
        <source>Type</source>
        <translation>Тип</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="267"/>
        <source>ERROR</source>
        <translation>ОШИБКА</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="271"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributes.cpp" line="157"/>
        <source>Layer</source>
        <translation>Слой</translation>
    </message>
</context>
<context>
    <name>QgsGrassAttributesBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="43"/>
        <source>GRASS Attributes</source>
        <translation>Атрибуты GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="80"/>
        <source>Tab 1</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="112"/>
        <source>result</source>
        <translation>результат</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="185"/>
        <source>Update</source>
        <translation>Обновить</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="182"/>
        <source>Update database record</source>
        <translation>Обновить запись базы данных</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="213"/>
        <source>New</source>
        <translation>Новая</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="210"/>
        <source>Add new category using settings in GRASS Edit toolbox</source>
        <translation>Добавить новую категорию, используя параметры редактора GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="241"/>
        <source>Delete</source>
        <translation>Удалить</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassattributesbase.ui" line="238"/>
        <source>Delete selected category</source>
        <translation>Удалить выбранную категорию</translation>
    </message>
</context>
<context>
    <name>QgsGrassBrowser</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="43"/>
        <source>Tools</source>
        <translation>Инструменты</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="47"/>
        <source>Add selected map to canvas</source>
        <translation>Добавить выбранную карту в область QGIS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="55"/>
        <source>Copy selected map</source>
        <translation>Копировать выбранную карту</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="63"/>
        <source>Rename selected map</source>
        <translation>Переименовать выбранную карту</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="71"/>
        <source>Delete selected map</source>
        <translation>Удалить выбранную карту</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="79"/>
        <source>Set current region to selected map</source>
        <translation>Установить регион по границам выбранной карты</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="87"/>
        <source>Refresh</source>
        <translation>Обновить</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="412"/>
        <source>Warning</source>
        <translation>Внимание</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="256"/>
        <source>Cannot copy map </source>
        <translation>Не удалось скопировать карту </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="374"/>
        <source>&lt;br&gt;command: </source>
        <translation>&lt;br&gt;команда: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="320"/>
        <source>Cannot rename map </source>
        <translation>Не удалось переименовать карту </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="356"/>
        <source>Delete map &lt;b&gt;</source>
        <translation>Удалить карту &lt;b&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="373"/>
        <source>Cannot delete map </source>
        <translation>Не удалось удалить карту </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="413"/>
        <source>Cannot write new region</source>
        <translation>Не удалось сохранить новый регион</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassbrowser.cpp" line="304"/>
        <source>New name</source>
        <translation>Новое имя</translation>
    </message>
</context>
<context>
    <name>QgsGrassEdit</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="259"/>
        <source>New point</source>
        <translation>Новая точка</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="277"/>
        <source>New centroid</source>
        <translation>Новый центроид</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="295"/>
        <source>Delete vertex</source>
        <translation>Удалить вершину</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1885"/>
        <source>Left: </source>
        <translation>Левая: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1886"/>
        <source>Middle: </source>
        <translation>Средняя: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="256"/>
        <source>Edit tools</source>
        <translation>Инструменты редактора</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="265"/>
        <source>New line</source>
        <translation>Новая линия</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="271"/>
        <source>New boundary</source>
        <translation>Новая граница</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="283"/>
        <source>Move vertex</source>
        <translation>Переместить вершину</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="289"/>
        <source>Add vertex</source>
        <translation>Добавить вершину</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="301"/>
        <source>Move element</source>
        <translation>Переместить элемент</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="307"/>
        <source>Split line</source>
        <translation>Разделить линию</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="313"/>
        <source>Delete element</source>
        <translation>Удалить элемент</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="319"/>
        <source>Edit attributes</source>
        <translation>Изменить атрибуты</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="324"/>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1522"/>
        <source>Warning</source>
        <translation>Внимание</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="238"/>
        <source>You are not owner of the mapset, cannot open the vector for editing.</source>
        <translation>Вы не являетесь владельцем набора, невозможно изменить векторный слой.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="244"/>
        <source>Cannot open vector for update.</source>
        <translation>Не удалось открыть векторный слой для обновления.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="699"/>
        <source>Info</source>
        <translation>Информация</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="699"/>
        <source>The table was created</source>
        <translation>Таблица была создана</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1382"/>
        <source>Tool not yet implemented.</source>
        <translation>Инструмент не реализован.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1404"/>
        <source>Cannot check orphan record: </source>
        <translation>Не удаётся проверить изолированную запись: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1411"/>
        <source>Orphan record was left in attribute table. &lt;br&gt;Delete the record?</source>
        <translation>В таблице атрибутов обнаружена изолированная запись. &lt;br/&gt;Удалить запись?</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1421"/>
        <source>Cannot delete orphan record: </source>
        <translation>Не удаётся удалить изолированную запись: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1454"/>
        <source>Cannot describe table for field </source>
        <translation>Не удаётся описать таблицу для поля </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="407"/>
        <source>Background</source>
        <translation>Фон</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="408"/>
        <source>Highlight</source>
        <translation>Подсветка</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="409"/>
        <source>Dynamic</source>
        <translation>Изменяемое</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="410"/>
        <source>Point</source>
        <translation>Точка</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="411"/>
        <source>Line</source>
        <translation>Линия</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="412"/>
        <source>Boundary (no area)</source>
        <translation>Граница (нет площади)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="413"/>
        <source>Boundary (1 area)</source>
        <translation>Граница (1 площадь)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="414"/>
        <source>Boundary (2 areas)</source>
        <translation>Граница (2 площади)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="415"/>
        <source>Centroid (in area)</source>
        <translation>Центроид (в площади)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="416"/>
        <source>Centroid (outside area)</source>
        <translation>Центроид (за площадью)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="417"/>
        <source>Centroid (duplicate in area)</source>
        <translation>Центроид (дублированный)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="418"/>
        <source>Node (1 line)</source>
        <translation>Узел (1 линия)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="419"/>
        <source>Node (2 lines)</source>
        <translation>Узел (2 линии)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="548"/>
        <source>Next not used</source>
        <translation>Следующая неиспользуемая</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="549"/>
        <source>Manual entry</source>
        <translation>Ручной ввод</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="550"/>
        <source>No category</source>
        <translation>Без категории</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassedit.cpp" line="1887"/>
        <source>Right: </source>
        <translation>Правая: </translation>
    </message>
</context>
<context>
    <name>QgsGrassEditBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="13"/>
        <source>GRASS Edit</source>
        <translation>Редактор GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="84"/>
        <source>Category</source>
        <translation>Категории</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="54"/>
        <source>Mode</source>
        <translation>Режим</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="154"/>
        <source>Settings</source>
        <translation>Параметры</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="162"/>
        <source>Snapping in screen pixels</source>
        <translation>Прилипание в пикселях экрана</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="213"/>
        <source>Symbology</source>
        <translation>Символика</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="327"/>
        <source>Table</source>
        <translation>Таблица</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="416"/>
        <source>Add Column</source>
        <translation>Добавить столбец</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="423"/>
        <source>Create / Alter Table</source>
        <translation>Создать / обновить таблицу</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="227"/>
        <source>Line width</source>
        <translation>Ширина линии</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="250"/>
        <source>Marker size</source>
        <translation>Размер маркера</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="335"/>
        <source>Layer</source>
        <translation>Слой</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="295"/>
        <source>Disp</source>
        <translation>Видимость</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="300"/>
        <source>Color</source>
        <translation>Цвет</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="388"/>
        <source>Type</source>
        <translation>Тип</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="310"/>
        <source>Index</source>
        <translation>Индекс</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="383"/>
        <source>Column</source>
        <translation>Поле</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasseditbase.ui" line="393"/>
        <source>Length</source>
        <translation>Длина</translation>
    </message>
</context>
<context>
    <name>QgsGrassElementDialog</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="114"/>
        <source>Cancel</source>
        <translation>Отменить</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="143"/>
        <source>Ok</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="148"/>
        <source>&lt;font color=&apos;red&apos;&gt;Enter a name!&lt;/font&gt;</source>
        <translation>&lt;font color=&apos;red&apos;&gt;Введите имя!&lt;/font&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="159"/>
        <source>&lt;font color=&apos;red&apos;&gt;This is name of the source!&lt;/font&gt;</source>
        <translation>&lt;font color=&apos;red&apos;&gt;Это имя источника!&lt;/font&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="165"/>
        <source>&lt;font color=&apos;red&apos;&gt;Exists!&lt;/font&gt;</source>
        <translation>&lt;font color=&apos;red&apos;&gt;Уже существует!&lt;/font&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassutils.cpp" line="166"/>
        <source>Overwrite</source>
        <translation>Перезаписать</translation>
    </message>
</context>
<context>
    <name>QgsGrassMapcalc</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="77"/>
        <source>Mapcalc tools</source>
        <translation>Инструменты Mapcalc</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="80"/>
        <source>Add map</source>
        <translation>Добавить карту</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="87"/>
        <source>Add constant value</source>
        <translation>Добавить постоянное значение</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="94"/>
        <source>Add operator or function</source>
        <translation>Добавить оператор или функцию</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="101"/>
        <source>Add connection</source>
        <translation>Добавить соединение</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="108"/>
        <source>Select item</source>
        <translation>Выбрать элемент</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="115"/>
        <source>Delete selected item</source>
        <translation>Удалить выбранный элемент</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="125"/>
        <source>Open</source>
        <translation>Открыть</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="130"/>
        <source>Save</source>
        <translation>Сохранить</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="136"/>
        <source>Save as</source>
        <translation>Сохранить как</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="144"/>
        <source>Addition</source>
        <translation>Сложение</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="145"/>
        <source>Subtraction</source>
        <translation>Вычитание</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="146"/>
        <source>Multiplication</source>
        <translation>Умножение</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="147"/>
        <source>Division</source>
        <translation>Деление</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="148"/>
        <source>Modulus</source>
        <translation>Остаток</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="149"/>
        <source>Exponentiation</source>
        <translation>Возведение в степень</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="152"/>
        <source>Equal</source>
        <translation>Равно</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="153"/>
        <source>Not equal</source>
        <translation>Не равно</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="154"/>
        <source>Greater than</source>
        <translation>Больше чем</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="155"/>
        <source>Greater than or equal</source>
        <translation>Больше или равно</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="156"/>
        <source>Less than</source>
        <translation>Меньше чем</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="157"/>
        <source>Less than or equal</source>
        <translation>Меньше или равно</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="158"/>
        <source>And</source>
        <translation>И</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="159"/>
        <source>Or</source>
        <translation>Или</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="162"/>
        <source>Absolute value of x</source>
        <translation>Абсолютное значение x</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="163"/>
        <source>Inverse tangent of x (result is in degrees)</source>
        <translation>Арктангенс x (результат в градусах)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="164"/>
        <source>Inverse tangent of y/x (result is in degrees)</source>
        <translation>Арктангенс у/x (результат в градусах)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="165"/>
        <source>Current column of moving window (starts with 1)</source>
        <translation>Текущий столбец подвижного окна (начиная с 1)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="166"/>
        <source>Cosine of x (x is in degrees)</source>
        <translation>Косинус x (x в градусах)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="167"/>
        <source>Convert x to double-precision floating point</source>
        <translation>Преобразование x в число с двойной точностью</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="168"/>
        <source>Current east-west resolution</source>
        <translation>Текущее горизонтальное разрешение</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="169"/>
        <source>Exponential function of x</source>
        <translation>Экспонента от x</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="170"/>
        <source>x to the power y</source>
        <translation>x в степени y</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="171"/>
        <source>Convert x to single-precision floating point</source>
        <translation>Преобразование x в число с одинарной точностью</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="172"/>
        <source>Decision: 1 if x not zero, 0 otherwise</source>
        <translation>Решение: 1 если x не равно нулю, иначе 0</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="173"/>
        <source>Decision: a if x not zero, 0 otherwise</source>
        <translation>Решение: a если x не равно нулю, иначе 0</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="174"/>
        <source>Decision: a if x not zero, b otherwise</source>
        <translation>Решение: a если x не равно нулю, иначе b</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="175"/>
        <source>Decision: a if x &gt; 0, b if x is zero, c if x &lt; 0</source>
        <translation>Решение: a если x &gt; 0, b если x = 0, c если x &lt; 0</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="176"/>
        <source>Convert x to integer [ truncates ]</source>
        <translation>Преобразование x в целое [отсечение]</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="177"/>
        <source>Check if x = NULL</source>
        <translation>Проверка, равен ли x значению NULL</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="178"/>
        <source>Natural log of x</source>
        <translation>Натуральный логарифм x</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="179"/>
        <source>Log of x base b</source>
        <translation>Логарифм x по основанию b</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="181"/>
        <source>Largest value</source>
        <translation>Наибольшее значение</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="183"/>
        <source>Median value</source>
        <translation>Медиана</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="185"/>
        <source>Smallest value</source>
        <translation>Наименьшее значение</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="187"/>
        <source>Mode value</source>
        <translation>Мода</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="188"/>
        <source>1 if x is zero, 0 otherwise</source>
        <translation>1, если x равен нулю, иначе 0</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="189"/>
        <source>Current north-south resolution</source>
        <translation>Текущее вертикальное разрешение</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="190"/>
        <source>NULL value</source>
        <translation>Значение NULL</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="191"/>
        <source>Random value between a and b</source>
        <translation>Случайное значение между a и b</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="192"/>
        <source>Round x to nearest integer</source>
        <translation>Округление x до ближайшего целого</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="193"/>
        <source>Current row of moving window (Starts with 1)</source>
        <translation>Текущая строка подвижного окна (начиная с 1)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="194"/>
        <source>Sine of x (x is in degrees)</source>
        <comment>sin(x)</comment>
        <translation>Синус x (x в градусах)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="195"/>
        <source>Square root of x</source>
        <comment>sqrt(x)</comment>
        <translation>Квадратный корень из x</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="196"/>
        <source>Tangent of x (x is in degrees)</source>
        <comment>tan(x)</comment>
        <translation>Тангенс x (x в градусах)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="197"/>
        <source>Current x-coordinate of moving window</source>
        <translation>Текущая x-координата подвижного окна</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="198"/>
        <source>Current y-coordinate of moving window</source>
        <translation>Текущая y-координата подвижного окна</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1259"/>
        <source>Warning</source>
        <translation>Внимание</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="538"/>
        <source>Cannot get current region</source>
        <translation>Не удалось получить регион</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="517"/>
        <source>Cannot check region of map </source>
        <translation>Не удалось проверить регион карты </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="572"/>
        <source>Cannot get region of map </source>
        <translation>Не удалось получить регион карты </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="769"/>
        <source>No GRASS raster maps currently in QGIS</source>
        <translation>Не найдено доступных для QGIS растровых карт GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1045"/>
        <source>Cannot create &apos;mapcalc&apos; directory in current mapset.</source>
        <translation>Не удалось создать каталог «mapcalc» в текущем наборе.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1055"/>
        <source>New mapcalc</source>
        <translation>Новая схема mapcalc</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1056"/>
        <source>Enter new mapcalc name:</source>
        <translation>Введите имя новой схемы mapcalc:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1062"/>
        <source>Enter vector name</source>
        <translation>Введите имя файла</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1070"/>
        <source>The file already exists. Overwrite? </source>
        <translation>Файл уже существует. Перезаписать? </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1106"/>
        <source>Save mapcalc</source>
        <translation>Сохранить схему mapcalc</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1088"/>
        <source>File name empty</source>
        <translation>Пустое имя файла</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1107"/>
        <source>Cannot open mapcalc file</source>
        <translation>Не удаётся открыть файл mapcalc</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1236"/>
        <source>The mapcalc schema (</source>
        <translation>Схема mapcalc (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1236"/>
        <source>) not found.</source>
        <translation>) не найдена.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1243"/>
        <source>Cannot open mapcalc schema (</source>
        <translation>Не удаётся открыть схему mapcalc (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1255"/>
        <source>Cannot read mapcalc schema (</source>
        <translation>Не удаётся прочесть схему mapcalc (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1256"/>
        <source>
at line </source>
        <translation>
в строке </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1257"/>
        <source> column </source>
        <translation>, столбец </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalc.cpp" line="1330"/>
        <source>Output</source>
        <translation>Вывод</translation>
    </message>
</context>
<context>
    <name>QgsGrassMapcalcBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalcbase.ui" line="13"/>
        <source>MainWindow</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmapcalcbase.ui" line="22"/>
        <source>Output</source>
        <translation>Вывод</translation>
    </message>
</context>
<context>
    <name>QgsGrassModule</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1384"/>
        <source>Run</source>
        <translation>Выполнить</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1356"/>
        <source>Stop</source>
        <translation>Остановить</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="163"/>
        <source>Module</source>
        <translation>Модуль</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1350"/>
        <source>Warning</source>
        <translation>Внимание</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="177"/>
        <source>The module file (</source>
        <translation>Файл модуля (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="177"/>
        <source>) not found.</source>
        <translation>) не найден.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="182"/>
        <source>Cannot open module file (</source>
        <translation>Не удалось открыть файл модуля (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="986"/>
        <source>)</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="981"/>
        <source>Cannot read module file (</source>
        <translation>Не удалось прочесть файл модуля (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="981"/>
        <source>):
</source>
        <translation>):
</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="982"/>
        <source>
at line </source>
        <translation>
в строке </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="219"/>
        <source>Module </source>
        <translation>Модуль </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="219"/>
        <source> not found</source>
        <translation> не найден</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="263"/>
        <source>Cannot find man page </source>
        <translation>Не удалось найти страницу руководства </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="974"/>
        <source>Not available, cannot open description (</source>
        <translation>Модуль недоступен, не удалось открыть описание (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="982"/>
        <source> column </source>
        <translation>, столбец </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="986"/>
        <source>Not available, incorrect description (</source>
        <translation>Модуль недоступен, неверное описание (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1176"/>
        <source>Cannot get input region</source>
        <translation>Не удаётся получить исходный регион</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1164"/>
        <source>Use Input Region</source>
        <translation>Использовать исходный регион</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1279"/>
        <source>Cannot find module </source>
        <translation>Не удалось найти модуль </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1351"/>
        <source>Cannot start module: </source>
        <translation>Не удалось запустить модуль: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1369"/>
        <source>&lt;B&gt;Successfully finished&lt;/B&gt;</source>
        <translation>&lt;B&gt;Успешное завершение&lt;/B&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1377"/>
        <source>&lt;B&gt;Finished with error&lt;/B&gt;</source>
        <translation>&lt;B&gt;Завершено с ошибкой&lt;/B&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1382"/>
        <source>&lt;B&gt;Module crashed or killed&lt;/B&gt;</source>
        <translation>&lt;B&gt;Модуль рухнул или был убит&lt;/B&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="970"/>
        <source>Not available, description not found (</source>
        <translation>Модуль недоступен, описание не найдено (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="264"/>
        <source>Please ensure you have the GRASS documentation installed.</source>
        <translation>Пожалуйста, убедитесь, что документация GRASS была установлена.</translation>
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
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="20"/>
        <source>Options</source>
        <translation>Параметры</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="25"/>
        <source>Output</source>
        <translation>Вывод</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="47"/>
        <source>Manual</source>
        <translation>Справка</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="116"/>
        <source>Run</source>
        <translation>Выполнить</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="159"/>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="139"/>
        <source>View output</source>
        <translation>Открыть вывод</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodulebase.ui" line="78"/>
        <source>TextLabel</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleField</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2814"/>
        <source>Attribute field</source>
        <translation>Поле атрибута</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleFile</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="3022"/>
        <source>File</source>
        <translation>Файл</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="3138"/>
        <source>:&amp;nbsp;missing value</source>
        <translation>:&amp;nbsp;значение не задано</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="3145"/>
        <source>:&amp;nbsp;directory does not exist</source>
        <translation>:&amp;nbsp;каталог не существует</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleGdalInput</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2764"/>
        <source>Warning</source>
        <translation>Внимание</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2619"/>
        <source>Cannot find layeroption </source>
        <translation>Не удаётся найти параметр слоя </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2768"/>
        <source>PostGIS driver in OGR does not support schemas!&lt;br&gt;Only the table name will be used.&lt;br&gt;It can result in wrong input if more tables of the same name&lt;br&gt;are present in the database.</source>
        <translation>OGR-драйвер PostGIS не поддерживает схемы!&lt;br&gt;Будет использоваться только имя таблицы.&lt;br&gt;Это может повлиять на правильность ввода,&lt;br&gt;если в базе данных есть более одной таблицы&lt;br&gt;с одинаковыми именами.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2792"/>
        <source>:&amp;nbsp;no input</source>
        <translation>:&amp;nbsp;параметр не задан</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2634"/>
        <source>Cannot find whereoption </source>
        <translation>Не удаётся найти параметр where </translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleInput</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2146"/>
        <source>Warning</source>
        <translation>Внимание</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2047"/>
        <source>Cannot find typeoption </source>
        <translation>Не удаётся найти параметр type </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2056"/>
        <source>Cannot find values for typeoption </source>
        <translation>Не удаётся найти значения для параметра type </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2129"/>
        <source>Cannot find layeroption </source>
        <translation>Не удаётся найти параметр слоя </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2146"/>
        <source>GRASS element </source>
        <translation>Элемент GRASS </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2146"/>
        <source> not supported</source>
        <translation> не поддерживается</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2170"/>
        <source>Use region of this map</source>
        <translation>Использовать регион этой карты</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2516"/>
        <source>:&amp;nbsp;no input</source>
        <translation>:&amp;nbsp;параметр не задан</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleOption</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="1955"/>
        <source>:&amp;nbsp;missing value</source>
        <translation>:&amp;nbsp;значение не задано</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleSelection</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="2906"/>
        <source>Attribute field</source>
        <translation>Поле атрибута</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleStandardOptions</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="876"/>
        <source>Warning</source>
        <translation>Внимание</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="329"/>
        <source>Cannot find module </source>
        <translation>Не удалось найти модуль </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="346"/>
        <source>Cannot start module </source>
        <translation>Не удалось запустить модуль </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="360"/>
        <source>Cannot read module description (</source>
        <translation>Не удалось прочесть описание модуля (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="360"/>
        <source>):
</source>
        <translation>):
</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="361"/>
        <source>
at line </source>
        <translation>
в строке </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="361"/>
        <source> column </source>
        <translation>, столбец </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="403"/>
        <source>Cannot find key </source>
        <translation>Не удаётся найти ключ </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="543"/>
        <source>Item with id </source>
        <translation>Элемент с ID </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="543"/>
        <source> not found</source>
        <translation> не найден</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="836"/>
        <source>Cannot get current region</source>
        <translation>Не удалось получить текущий регион</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="813"/>
        <source>Cannot check region of map </source>
        <translation>Не удалось проверить регион карты </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassmodule.cpp" line="877"/>
        <source>Cannot set region of map </source>
        <translation>Не удалось задать регион карты </translation>
    </message>
</context>
<context>
    <name>QgsGrassNewMapset</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="137"/>
        <source>Database</source>
        <translation>База данных</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="149"/>
        <source>Location 2</source>
        <translation>Район 2</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="153"/>
        <source>User&apos;s mapset</source>
        <translation>Пользовательский набор</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="151"/>
        <source>System mapset</source>
        <translation>Системный набор</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="140"/>
        <source>Location 1</source>
        <translation>Район 1</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="202"/>
        <source>Enter path to GRASS database</source>
        <translation>Введите путь к базе данных GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="210"/>
        <source>The directory doesn&apos;t exist!</source>
        <translation>Каталог не существует!</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="240"/>
        <source>No writable locations, the database not writable!</source>
        <translation>Не найдено доступных для записи районов, нет прав доступа к базе данных!</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="356"/>
        <source>Enter location name!</source>
        <translation>Введите имя района!</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="363"/>
        <source>The location exists!</source>
        <translation>Район уже существует!</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="494"/>
        <source>Selected projection is not supported by GRASS!</source>
        <translation>Выбранная проекция не поддерживается GRASS!</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1086"/>
        <source>Warning</source>
        <translation>Внимание</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="530"/>
        <source>Cannot create projection.</source>
        <translation>Не удалось создать проекцию.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="579"/>
        <source>Cannot reproject previously set region, default region set.</source>
        <translation>Не удалось спроецировать ранее заданный регион, выбран регион по умолчанию.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="706"/>
        <source>North must be greater than south</source>
        <translation>Значение севера должно быть больше значения юга</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="711"/>
        <source>East must be greater than west</source>
        <translation>Значение востока должно быть больше значения запада</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="755"/>
        <source>Regions file (</source>
        <translation>Файл областей (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="755"/>
        <source>) not found.</source>
        <translation>) не найден.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="761"/>
        <source>Cannot open locations file (</source>
        <translation>Не удаётся открыть файл районов (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="761"/>
        <source>)</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="771"/>
        <source>Cannot read locations file (</source>
        <translation>Не удаётся прочесть файл районов (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="772"/>
        <source>):
</source>
        <translation>):
</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="772"/>
        <source>
at line </source>
        <translation>
в строке </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="773"/>
        <source> column </source>
        <translation>, столбец </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="904"/>
        <source>Cannot reproject selected region.</source>
        <translation>Не удаётся спроецировать выбранный регион.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="989"/>
        <source>Cannot reproject region</source>
        <translation>Не удаётся спроецировать регион</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1207"/>
        <source>Enter mapset name.</source>
        <translation>Введите имя набора.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1217"/>
        <source>The mapset already exists</source>
        <translation>Этот набор уже существует</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1235"/>
        <source>Database: </source>
        <translation>База данных:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1246"/>
        <source>Location: </source>
        <translation>Район: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1248"/>
        <source>Mapset: </source>
        <translation>Набор:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1279"/>
        <source>Create location</source>
        <translation>Создать район</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1281"/>
        <source>Cannot create new location: </source>
        <translation>Не удалось создать новый район: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1328"/>
        <source>Create mapset</source>
        <translation>Создать набор</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1321"/>
        <source>Cannot open DEFAULT_WIND</source>
        <translation>Не удалось открыть DEFAULT_WIND</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1328"/>
        <source>Cannot open WIND</source>
        <translation>Не удалось открыть WIND</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1356"/>
        <source>New mapset</source>
        <translation>Новый набор</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1352"/>
        <source>New mapset successfully created, but cannot be opened: </source>
        <translation>Новый набор успешно создан, но не может быть открыт: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1358"/>
        <source>New mapset successfully created and set as current working mapset.</source>
        <translation>Новый набор успешно создан и открыт как текущий рабочий набор.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1311"/>
        <source>Cannot create new mapset directory</source>
        <translation>Не удалось создать каталог для нового набора</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapset.cpp" line="1087"/>
        <source>Cannot create QgsCoordinateReferenceSystem</source>
        <translation>Не удаётся создать объект QgsCoordinateReferenceSystem</translation>
    </message>
</context>
<context>
    <name>QgsGrassNewMapsetBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="52"/>
        <source>Example directory tree:</source>
        <translation>Пример структуры каталогов:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="561"/>
        <source>Database Error</source>
        <translation>Ошибка базы данных</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2668"/>
        <source>Database:</source>
        <translation>База данных:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="607"/>
        <source>Select existing directory or create a new one:</source>
        <translation>Выберите существующий каталог или создайте новый:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="639"/>
        <source>Location</source>
        <translation>Район</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="654"/>
        <source>Select location</source>
        <translation>Выбрать район</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="671"/>
        <source>Create new location</source>
        <translation>Создать новый район</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1147"/>
        <source>Location Error</source>
        <translation>Ошибка района</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1669"/>
        <source>Projection Error</source>
        <translation>Ошибка проекции</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1676"/>
        <source>Coordinate system</source>
        <translation>Система координат</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1688"/>
        <source>Projection</source>
        <translation>Проекция</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1695"/>
        <source>Not defined</source>
        <translation>Не определена</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1798"/>
        <source>Set current QGIS extent</source>
        <translation>Установить текущие границы QGIS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1827"/>
        <source>Set</source>
        <translation>Установить</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1839"/>
        <source>Region Error</source>
        <translation>Ошибка области</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1870"/>
        <source>S</source>
        <translation>Ю</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1906"/>
        <source>W</source>
        <translation>З</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1945"/>
        <source>E</source>
        <translation>В</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1981"/>
        <source>N</source>
        <translation>С</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2041"/>
        <source>New mapset:</source>
        <translation>Новый набор:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2526"/>
        <source>Mapset Error</source>
        <translation>Ошибка набора</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2567"/>
        <source>&lt;p align=&quot;center&quot;&gt;Existing masets&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;center&quot;&gt;Существующие наборы&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2687"/>
        <source>Location:</source>
        <translation>Район: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2706"/>
        <source>Mapset:</source>
        <translation>Набор:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="13"/>
        <source>New Mapset</source>
        <translation>Новый набор</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="17"/>
        <source>GRASS Database</source>
        <translation>База данных GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="39"/>
        <source>Tree</source>
        <translation>Дерево</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="44"/>
        <source>Comment</source>
        <translation>Комментарий</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="59"/>
        <source>&lt;!DOCTYPE HTML PUBLIC &quot;-//W3C//DTD HTML 4.0//EN&quot; &quot;http://www.w3.org/TR/REC-html40/strict.dtd&quot;&gt;
&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Lucida Grande&apos;; font-size:13pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;GRASS data are stored in tree directory structure. The GRASS database is the top-level directory in this tree structure.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;!DOCTYPE HTML PUBLIC &quot;-//W3C//DTD HTML 4.0//EN&quot; &quot;http://www.w3.org/TR/REC-html40/strict.dtd&quot;&gt;
&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Lucida Grande&apos;; font-size:13pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;Данные GRASS хранятся в виде дерева каталогов. Базой данных GRASS называется верхний каталог в этой структуре.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="596"/>
        <source>Browse...</source>
        <translation>Обзор...</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="633"/>
        <source>GRASS Location</source>
        <translation>Район GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1163"/>
        <source>&lt;!DOCTYPE HTML PUBLIC &quot;-//W3C//DTD HTML 4.0//EN&quot; &quot;http://www.w3.org/TR/REC-html40/strict.dtd&quot;&gt;
&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Lucida Grande&apos;; font-size:13pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;The GRASS location is a collection of maps for a particular territory or project.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;!DOCTYPE HTML PUBLIC &quot;-//W3C//DTD HTML 4.0//EN&quot; &quot;http://www.w3.org/TR/REC-html40/strict.dtd&quot;&gt;
&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Lucida Grande&apos;; font-size:13pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;Районом GRASS называется коллекция карт для определённой территории или проекта.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1716"/>
        <source>Default GRASS Region</source>
        <translation>Регион GRASS по умолчанию</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="1747"/>
        <source>&lt;!DOCTYPE HTML PUBLIC &quot;-//W3C//DTD HTML 4.0//EN&quot; &quot;http://www.w3.org/TR/REC-html40/strict.dtd&quot;&gt;
&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Lucida Grande&apos;; font-size:13pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;The GRASS region defines a workspace for raster modules. The default region is valid for one location. It is possible to set a different region in each mapset. It is possible to change the default location region later.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;!DOCTYPE HTML PUBLIC &quot;-//W3C//DTD HTML 4.0//EN&quot; &quot;http://www.w3.org/TR/REC-html40/strict.dtd&quot;&gt;
&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Lucida Grande&apos;; font-size:13pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;Регион GRASS определяет область работы для растровых модулей. Для каждого района существует регион по умолчанию, и в каждом наборе может быть определён собственный регион. Регион по умолчанию также может быть изменён позднее.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2587"/>
        <source>Mapset</source>
        <translation>Набор</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2592"/>
        <source>Owner</source>
        <translation>Владелец</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2625"/>
        <source>&lt;!DOCTYPE HTML PUBLIC &quot;-//W3C//DTD HTML 4.0//EN&quot; &quot;http://www.w3.org/TR/REC-html40/strict.dtd&quot;&gt;
&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Lucida Grande&apos;; font-size:13pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;The GRASS mapset is a collection of maps used by one user. A user can read maps from all mapsets in the location but he can open for writing only his mapset (owned by user).&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;!DOCTYPE HTML PUBLIC &quot;-//W3C//DTD HTML 4.0//EN&quot; &quot;http://www.w3.org/TR/REC-html40/strict.dtd&quot;&gt;
&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Lucida Grande&apos;; font-size:13pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;Набором GRASS называется коллекция карт, с которыми работает один пользователь. Каждый пользователь может читать карты из всех наборов района, но запись карт разрешена только в пределах его собственного района (владельцем которого он является).&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassnewmapsetbase.ui" line="2650"/>
        <source>Create New Mapset</source>
        <translation>Создать новый набор</translation>
    </message>
</context>
<context>
    <name>QgsGrassPlugin</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="179"/>
        <source>GRASS</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="770"/>
        <source>&amp;GRASS</source>
        <translation>&amp;GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="126"/>
        <source>Open mapset</source>
        <translation>Открыть набор</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="127"/>
        <source>New mapset</source>
        <translation>Новый набор</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="128"/>
        <source>Close mapset</source>
        <translation>Закрыть набор</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="131"/>
        <source>Add GRASS vector layer</source>
        <translation>Добавить векторный слой GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="133"/>
        <source>Add GRASS raster layer</source>
        <translation>Добавить растровый слой GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="149"/>
        <source>Open GRASS tools</source>
        <translation>Открыть инструменты GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="138"/>
        <source>Display Current Grass Region</source>
        <translation>Показать текущий регион GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="142"/>
        <source>Edit Current Grass Region</source>
        <translation>Изменить текущий регион GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="144"/>
        <source>Edit Grass Vector layer</source>
        <translation>Редактировать векторный слой GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="147"/>
        <source>Adds a GRASS vector layer to the map canvas</source>
        <translation>Добавить на карту векторный слой GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="148"/>
        <source>Adds a GRASS raster layer to the map canvas</source>
        <translation>Добавить на карту растровый слой GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="150"/>
        <source>Displays the current GRASS region as a rectangle on the map canvas</source>
        <translation>Показать текущий регион GRASS в виде прямоугольника на карте</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="151"/>
        <source>Edit the current GRASS region</source>
        <translation>Изменить текущий регион GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="152"/>
        <source>Edit the currently selected GRASS vector layer.</source>
        <translation>Изменить выбранный векторный слой GRASS.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="63"/>
        <source>GrassVector</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="64"/>
        <source>0.1</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="65"/>
        <source>GRASS layer</source>
        <translation>Слой GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="145"/>
        <source>Create new Grass Vector</source>
        <translation>Создать новый векторный слой GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="742"/>
        <source>Warning</source>
        <translation>Внимание</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="454"/>
        <source>GRASS Edit is already running.</source>
        <translation>Редактор GRASS уже запущен.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="463"/>
        <source>New vector name</source>
        <translation>Имя нового слоя</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="479"/>
        <source>Cannot create new vector: </source>
        <translation>Не удалось создать новый векторный слой: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="504"/>
        <source>New vector created but cannot be opened by data provider.</source>
        <translation>Новый слой создан, но не может быть открыт поставщиком данных.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="518"/>
        <source>Cannot start editing.</source>
        <translation>Не удалось начать редактирование.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="550"/>
        <source>GISDBASE, LOCATION_NAME or MAPSET is not set, cannot display current region.</source>
        <translation>Не заданы GISDBASE, LOCATION_NAME или MAPSET, невозможно вывести текущий регион.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="561"/>
        <source>Cannot read current region: </source>
        <translation>Не удалось прочесть текущий регион: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="662"/>
        <source>Cannot open the mapset. </source>
        <translation>Не удаётся открыть набор. </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="678"/>
        <source>Cannot close mapset. </source>
        <translation>Не удаётся закрыть набор. </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="733"/>
        <source>Cannot close current mapset. </source>
        <translation>Не удаётся закрыть текущий набор. </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassplugin.cpp" line="742"/>
        <source>Cannot open GRASS mapset. </source>
        <translation>Не удаётся открыть набор GRASS. </translation>
    </message>
</context>
<context>
    <name>QgsGrassRegion</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregion.cpp" line="433"/>
        <source>Warning</source>
        <translation>Внимание</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregion.cpp" line="164"/>
        <source>GISDBASE, LOCATION_NAME or MAPSET is not set, cannot display current region.</source>
        <translation>Не заданы GISDBASE, LOCATION_NAME или MAPSET, невозможно вывести текущий регион.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregion.cpp" line="172"/>
        <source>Cannot read current region: </source>
        <translation>Не удаётся прочесть текущий регион: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregion.cpp" line="433"/>
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
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="92"/>
        <source>N</source>
        <translation>С</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="169"/>
        <source>W</source>
        <translation>З</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="195"/>
        <source>E</source>
        <translation>В</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="266"/>
        <source>S</source>
        <translation>Ю</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="316"/>
        <source>N-S Res</source>
        <translation>Вертикальное разрешение</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="329"/>
        <source>Rows</source>
        <translation>Строк</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="339"/>
        <source>Cols</source>
        <translation>Столбцов</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="352"/>
        <source>E-W Res</source>
        <translation>Горизонтальное разрешение</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="407"/>
        <source>Color</source>
        <translation>Цвет</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="427"/>
        <source>Width</source>
        <translation>Ширина</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="514"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassregionbase.ui" line="537"/>
        <source>Cancel</source>
        <translation>Отменить</translation>
    </message>
</context>
<context>
    <name>QgsGrassSelect</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="64"/>
        <source>Select GRASS Vector Layer</source>
        <translation>Выберите векторный слой GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="71"/>
        <source>Select GRASS Raster Layer</source>
        <translation>Выберите растровый слой GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="78"/>
        <source>Select GRASS mapcalc schema</source>
        <translation>Выберите схему GRASS mapcalc</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="86"/>
        <source>Select GRASS Mapset</source>
        <translation>Выберите набор GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="428"/>
        <source>Warning</source>
        <translation>Внимание</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="428"/>
        <source>Cannot open vector on level 2 (topology not available).</source>
        <translation>Не удаётся открыть вектор уровня 2 (топология недоступна).</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="485"/>
        <source>Choose existing GISDBASE</source>
        <translation>Выберите существующую GISDBASE</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="502"/>
        <source>Wrong GISDBASE, no locations available.</source>
        <translation>Неверная GISDBASE, доступных районов не найдено.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="503"/>
        <source>Wrong GISDBASE</source>
        <translation>Неверная GISDBASE</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="521"/>
        <source>Select a map.</source>
        <translation>Выберите карту.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="522"/>
        <source>No map</source>
        <translation>Нет карты</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="530"/>
        <source>No layer</source>
        <translation>Нет слоя</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselect.cpp" line="531"/>
        <source>No layers available in this map</source>
        <translation>В этой карте нет доступных слоёв</translation>
    </message>
</context>
<context>
    <name>QgsGrassSelectBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="82"/>
        <source>Gisdbase</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="95"/>
        <source>Location</source>
        <translation>Район</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="185"/>
        <source>Browse</source>
        <translation>Обзор</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="102"/>
        <source>Mapset</source>
        <translation>Набор</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="135"/>
        <source>Map name</source>
        <translation>Имя карты</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="142"/>
        <source>Layer</source>
        <translation>Слой</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="199"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="119"/>
        <source>Select or type map name (wildcards &apos;*&apos; and &apos;?&apos; accepted for rasters)</source>
        <translation>Выберите или введите имя карты (шаблоны * и ? принимаются для растров)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="19"/>
        <source>Add GRASS Layer</source>
        <translation>Добавить слой GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrassselectbase.ui" line="192"/>
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
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="125"/>
        <source>Browser</source>
        <translation>Браузер</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="66"/>
        <source>GRASS Tools</source>
        <translation>Инструменты GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="383"/>
        <source>GRASS Tools: </source>
        <translation>Инструменты GRASS: </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="272"/>
        <source>Warning</source>
        <translation>Внимание</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="185"/>
        <source>Cannot find MSYS (</source>
        <translation>Не удаётся найти MSYS (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="207"/>
        <source>GRASS Shell is not compiled.</source>
        <translation>Оболочка GRASS не была скомпилирована.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="255"/>
        <source>The config file (</source>
        <translation>Файл настроек (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="255"/>
        <source>) not found.</source>
        <translation>) не найден.</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="260"/>
        <source>Cannot open config file (</source>
        <translation>Не удаётся открыть файл конфигурации (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="260"/>
        <source>)</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="269"/>
        <source>Cannot read config file (</source>
        <translation>Не удаётся прочесть файл конфигурации (</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="270"/>
        <source>
at line </source>
        <translation>
в строке </translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstools.cpp" line="270"/>
        <source> column </source>
        <translation>, столбец </translation>
    </message>
</context>
<context>
    <name>QgsGrassToolsBase</name>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstoolsbase.ui" line="13"/>
        <source>Grass Tools</source>
        <translation>Инструменты GRASS</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstoolsbase.ui" line="23"/>
        <source>Modules Tree</source>
        <translation>Дерево модулей</translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstoolsbase.ui" line="42"/>
        <source>1</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugins/grass/qgsgrasstoolsbase.ui" line="51"/>
        <source>Modules List</source>
        <translation>Список модулей</translation>
    </message>
</context>
<context>
    <name>QgsGridMakerPlugin</name>
    <message>
        <location filename="../src/plugins/grid_maker/plugin.cpp" line="91"/>
        <source>&amp;Graticule Creator</source>
        <translation>&amp;Конструктор сетки</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugin.cpp" line="92"/>
        <source>Creates a graticule (grid) and stores the result as a shapefile</source>
        <translation>Построить сетку и сохранить результат в shape-файл</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugin.cpp" line="134"/>
        <source>&amp;Graticules</source>
        <translation>Се&amp;тка</translation>
    </message>
</context>
<context>
    <name>QgsGridMakerPluginGui</name>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="62"/>
        <source>QGIS - Grid Maker</source>
        <translation>QGIS — Конструктор сетки</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="52"/>
        <source>Please enter the file name before pressing OK!</source>
        <translation>Пожалуйста, введите имя файла прежде чем нажимать OK!</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="118"/>
        <source>ESRI Shapefile (*.shp)</source>
        <translation>Shape-файлы (*.shp)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="63"/>
        <source>Please enter intervals before pressing OK!</source>
        <translation>Пожалуйста, введите интервалы прежде чем нажимать OK!</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/plugingui.cpp" line="116"/>
        <source>Choose a file name to save under</source>
        <translation>Выберите имя сохраняемого файла</translation>
    </message>
</context>
<context>
    <name>QgsGridMakerPluginGuiBase</name>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="322"/>
        <source>Graticule Builder</source>
        <translation>Конструктор сетки</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="243"/>
        <source>Type</source>
        <translation>Тип</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="264"/>
        <source>Point</source>
        <translation>Точка</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="274"/>
        <source>Polygon</source>
        <translation>Полигон</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="146"/>
        <source>Origin (lower left)</source>
        <translation>Начальная точка (нижняя левая)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="93"/>
        <source>End point (upper right)</source>
        <translation>Конечная точка (верхняя правая)</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="209"/>
        <source>Output (shape) file</source>
        <translation>Выходной (shape) файл</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="233"/>
        <source>Save As...</source>
        <translation>Сохранить как...</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="186"/>
        <source>X</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="176"/>
        <source>Y</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="40"/>
        <source>Graticle size</source>
        <translation>Размеры сетки</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="80"/>
        <source>X Interval:</source>
        <translation>Интервал по X:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="70"/>
        <source>Y Interval:</source>
        <translation>Интервал по Y:</translation>
    </message>
    <message>
        <location filename="../src/plugins/grid_maker/pluginguibase.ui" line="13"/>
        <source>QGIS Graticule Creator</source>
        <translation>Конструктор сетки QGIS</translation>
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
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:11pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;Этот модуль предназначен для создания shape-файла с картографической сеткой, которую можно использовать для наложения на карту.&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;;&quot;&gt;Пожалуйста, вводите все значения в десятичных градусах&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
</context>
<context>
    <name>QgsHelpViewer</name>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="187"/>
        <source>Quantum GIS Help - </source>
        <translation>Справка Quantum GIS — </translation>
    </message>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="193"/>
        <source>Failed to get the help text from the database</source>
        <translation>Не удалось получить текст справки из базы данных</translation>
    </message>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="217"/>
        <source>Error</source>
        <translation>Ошибка</translation>
    </message>
    <message>
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="218"/>
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
        <location filename="../src/helpviewer/qgshelpviewer.cpp" line="158"/>
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
        <location filename="../src/core/qgshttptransaction.cpp" line="248"/>
        <source>WMS Server responded unexpectedly with HTTP Status Code %1 (%2)</source>
        <translation>Неожиданный ответ WMS-сервера с HTTP-кодом %1 (%2)</translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="327"/>
        <source>HTTP response completed, however there was an error: %1</source>
        <translation>HTTP-ответ получен  с ошибкой: %1</translation>
    </message>
    <message numerus="yes">
        <location filename="../src/core/qgshttptransaction.cpp" line="456"/>
        <source>Network timed out after %1 seconds of inactivity.
This may be a problem in your network connection or at the WMS server.</source>
        <translation>
            <numerusform>Соединение сброшено после %1 секунды бездействия.
Возможно существует проблема в подключении к сети или на WMS-сервере.</numerusform>
            <numerusform>Соединение сброшено после %1 секунд бездействия.Возможно существует проблема в подключении к сети или на WMS-сервере.</numerusform>
            <numerusform>Соединение сброшено после %1 секунд бездействия.Возможно существует проблема в подключении к сети или на WMS-сервере.</numerusform>
        </translation>
    </message>
    <message>
        <location filename="../src/core/qgshttptransaction.cpp" line="377"/>
        <source>HTTP transaction completed, however there was an error: %1</source>
        <translation>HTTP-транзакция завершена с ошибкой: %1</translation>
    </message>
</context>
<context>
    <name>QgsIDWInterpolatorDialogBase</name>
    <message>
        <location filename="../src/plugins/interpolation/qgsidwinterpolatordialogbase.ui" line="13"/>
        <source>Dialog</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsidwinterpolatordialogbase.ui" line="19"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:12pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-weight:600;&quot;&gt;Inverse Distance Weighting&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-weight:600;&quot;&gt;&lt;span style=&quot; font-weight:400;&quot;&gt;The only parameter for the IDW interpolation method is the coefficient that describes the decrease of weights with distance.&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:12pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-weight:600;&quot;&gt;Обратное взвешивание расстояний&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-weight:600;&quot;&gt;&lt;span style=&quot; font-weight:400;&quot;&gt;Единственный параметр для метода IDW-интерполяции — это коэффициент, характеризующий уменьшение веса в зависимости от расстояния.&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsidwinterpolatordialogbase.ui" line="32"/>
        <source>Distance coefficient P:</source>
        <translation>Коэффициент расстояния P:</translation>
    </message>
</context>
<context>
    <name>QgsIdentifyResults</name>
    <message>
        <location filename="../src/app/qgsidentifyresults.cpp" line="214"/>
        <source>Identify Results - </source>
        <translation>Результат определения — </translation>
    </message>
    <message>
        <location filename="../src/app/qgsidentifyresults.cpp" line="45"/>
        <source>Feature</source>
        <translation>Объект</translation>
    </message>
    <message>
        <location filename="../src/app/qgsidentifyresults.cpp" line="46"/>
        <source>Value</source>
        <translation>Значение</translation>
    </message>
    <message>
        <location filename="../src/app/qgsidentifyresults.cpp" line="110"/>
        <source>Run action</source>
        <translation>Выполнить действие</translation>
    </message>
    <message>
        <location filename="../src/app/qgsidentifyresults.cpp" line="59"/>
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
        <location filename="../src/plugins/interpolation/qgsinterpolationdialog.cpp" line="210"/>
        <source>Triangular interpolation (TIN)</source>
        <translation>Триангуляция (TIN)</translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsinterpolationdialog.cpp" line="206"/>
        <source>Inverse Distance Weighting (IDW)</source>
        <translation>Обратное взвешивание расстояний (IDW)</translation>
    </message>
</context>
<context>
    <name>QgsInterpolationDialogBase</name>
    <message>
        <location filename="../src/plugins/interpolation/qgsinterpolationdialogbase.ui" line="100"/>
        <source>Output</source>
        <translation>Вывод</translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsinterpolationdialogbase.ui" line="176"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsinterpolationdialogbase.ui" line="31"/>
        <source>Input</source>
        <translation>Исходные данные</translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsinterpolationdialogbase.ui" line="64"/>
        <source>Use z-Coordinate for interpolation</source>
        <translation>Использовать для интерполяции Z-координату</translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsinterpolationdialogbase.ui" line="19"/>
        <source>Interpolation plugin</source>
        <translation>Модуль интерполяции</translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsinterpolationdialogbase.ui" line="37"/>
        <source>Input vector layer</source>
        <translation>Исходный векторный слой</translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsinterpolationdialogbase.ui" line="71"/>
        <source>Interpolation attribute </source>
        <translation>Атрибут для интерполяции </translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsinterpolationdialogbase.ui" line="106"/>
        <source>Interpolation method</source>
        <translation>Метод интерполяции</translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsinterpolationdialogbase.ui" line="129"/>
        <source>Number of columns</source>
        <translation>Столбцов</translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsinterpolationdialogbase.ui" line="146"/>
        <source>Number of rows</source>
        <translation>Строк</translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgsinterpolationdialogbase.ui" line="163"/>
        <source>Output file </source>
        <translation>Файл вывода</translation>
    </message>
</context>
<context>
    <name>QgsInterpolationPlugin</name>
    <message>
        <location filename="../src/plugins/interpolation/qgsinterpolationplugin.cpp" line="49"/>
        <source>&amp;Interpolation</source>
        <translation>&amp;Интерполяция</translation>
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
        <location filename="../src/ui/qgsludialogbase.ui" line="66"/>
        <source>Upper value</source>
        <translation>Верхнее значение</translation>
    </message>
</context>
<context>
    <name>QgsLabelDialog</name>
    <message>
        <location filename="../src/app/qgslabeldialog.cpp" line="206"/>
        <source>Auto</source>
        <translation>Авто</translation>
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
        <location filename="../src/ui/qgslabeldialogbase.ui" line="999"/>
        <source>Preview:</source>
        <translation>Предпросмотр:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="1008"/>
        <source>QGIS Rocks!</source>
        <translation>QGIS работает!</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="152"/>
        <source>Font</source>
        <translation>Шрифт</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="582"/>
        <source>Points</source>
        <translation>Пункты</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="589"/>
        <source>Map units</source>
        <translation>Единицы карты</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="480"/>
        <source>%</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="877"/>
        <source>Transparency:</source>
        <translation>Прозрачность:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="95"/>
        <source>Position</source>
        <translation>Позиция</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="900"/>
        <source>Size:</source>
        <translation>Размер:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="513"/>
        <source>Size is in map units</source>
        <translation>Единицы карты</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="506"/>
        <source>Size is in points</source>
        <translation>Пункты</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="275"/>
        <source>Above</source>
        <translation>Сверху</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="265"/>
        <source>Over</source>
        <translation>Поверх</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="282"/>
        <source>Left</source>
        <translation>Слева</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="258"/>
        <source>Below</source>
        <translation>Внизу</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="251"/>
        <source>Right</source>
        <translation>Справа</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="296"/>
        <source>Above Right</source>
        <translation>Сверху справа</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="244"/>
        <source>Below Right</source>
        <translation>Внизу справа</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="303"/>
        <source>Above Left</source>
        <translation>Сверху слева</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="289"/>
        <source>Below Left</source>
        <translation>Внизу слева</translation>
    </message>
    <message encoding="UTF-8">
        <location filename="../src/ui/qgslabeldialogbase.ui" line="220"/>
        <source>°</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="313"/>
        <source>Font size units</source>
        <translation>Единицы размера шрифта</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="824"/>
        <source>Placement</source>
        <translation>Размещение</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="90"/>
        <source>Buffer</source>
        <translation>Буферизация</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="500"/>
        <source>Buffer size units</source>
        <translation>Единицы размера буфера</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="576"/>
        <source>Offset units</source>
        <translation>Единицы смещения</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="30"/>
        <source>Field containing label</source>
        <translation>Поле, содержащее подпись</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="53"/>
        <source>Default label</source>
        <translation>Подпись по умолчанию</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="100"/>
        <source>Data defined style</source>
        <translation>Данные стиля</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="105"/>
        <source>Data defined alignment</source>
        <translation>Данные выравнивания</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="110"/>
        <source>Data defined buffer</source>
        <translation>Данные буферизации</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="115"/>
        <source>Data defined position</source>
        <translation>Данные положения</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="162"/>
        <source>Font transparency</source>
        <translation>Прозрачность шрифта</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="493"/>
        <source>Color</source>
        <translation>Цвет</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="840"/>
        <source>Angle (deg)</source>
        <translation>Угол (град)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="432"/>
        <source>Buffer labels?</source>
        <translation>Буферизовать подписи?</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="442"/>
        <source>Buffer size</source>
        <translation>Размер буфера</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="786"/>
        <source>Transparency</source>
        <translation>Прозрачность</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="963"/>
        <source>X Offset (pts)</source>
        <translation>Смещение по X (пункты)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="979"/>
        <source>Y Offset (pts)</source>
        <translation>Смещение по Y (пункты)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="630"/>
        <source>&amp;Font family</source>
        <translation>&amp;Шрифт</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="656"/>
        <source>&amp;Bold</source>
        <translation>&amp;Полужирный</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="682"/>
        <source>&amp;Italic</source>
        <translation>&amp;Курсив</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="708"/>
        <source>&amp;Underline</source>
        <translation>&amp;Подчёркивание</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="734"/>
        <source>&amp;Size</source>
        <translation>&amp;Размер</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="760"/>
        <source>Size units</source>
        <translation>Единицы размера</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="931"/>
        <source>X Coordinate</source>
        <translation>X-координата</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="947"/>
        <source>Y Coordinate</source>
        <translation>Y-координата</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="410"/>
        <source>Multiline labels?</source>
        <translation>Разбивать подписи на строки?</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="85"/>
        <source>General</source>
        <translation>Общие</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="349"/>
        <source>Use scale dependent rendering</source>
        <translation>Видимость в пределах масштаба</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="361"/>
        <source>Maximum</source>
        <translation>Максимальный</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="371"/>
        <source>Minimum</source>
        <translation>Минимальный</translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="381"/>
        <source>Minimum scale at which this layer will be displayed. </source>
        <translation>Минимальный масштаб, при котором виден данный слой. </translation>
    </message>
    <message>
        <location filename="../src/ui/qgslabeldialogbase.ui" line="394"/>
        <source>Maximum scale at which this layer will be displayed. </source>
        <translation>Максимальный масштаб, при котором виден данный слой. </translation>
    </message>
</context>
<context>
    <name>QgsLegend</name>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="114"/>
        <source>group</source>
        <translation>группа</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="452"/>
        <source>&amp;Remove</source>
        <translation>&amp;Удалить</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="445"/>
        <source>&amp;Make to toplevel item</source>
        <translation>Сделать элементом &amp;первого уровня</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="457"/>
        <source>Re&amp;name</source>
        <translation>Переи&amp;меновать</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="462"/>
        <source>&amp;Add group</source>
        <translation>&amp;Добавить группу</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="463"/>
        <source>&amp;Expand all</source>
        <translation>&amp;Развернуть все</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="464"/>
        <source>&amp;Collapse all</source>
        <translation>&amp;Свернуть все</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="466"/>
        <source>Show file groups</source>
        <translation>Показать группы файлов</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="1868"/>
        <source>No Layer Selected</source>
        <translation>Слой не выбран</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegend.cpp" line="1869"/>
        <source>To open an attribute table, you must select a vector layer in the legend</source>
        <translation>Для открытия таблицы атрибутов, следует выбрать в легенде векторный слой</translation>
    </message>
</context>
<context>
    <name>QgsLegendLayer</name>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="481"/>
        <source>&amp;Zoom to layer extent</source>
        <translation>&amp;Увеличить до границ слоя</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="484"/>
        <source>&amp;Zoom to best scale (100%)</source>
        <translation>&amp;Увеличить до наилучшего масштаба (100%)</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="488"/>
        <source>&amp;Show in overview</source>
        <translation>&amp;Показать в обзоре</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="494"/>
        <source>&amp;Remove</source>
        <translation>&amp;Удалить</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="501"/>
        <source>&amp;Open attribute table</source>
        <translation>&amp;Открыть таблицу атрибутов</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="525"/>
        <source>Save as shapefile...</source>
        <translation>Сохранить как shape-файл...</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="532"/>
        <source>Save selection as shapefile...</source>
        <translation>Сохранить выделение как shape-файл...</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="542"/>
        <source>&amp;Properties</source>
        <translation>&amp;Свойства</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="591"/>
        <source>Multiple layers</source>
        <translation>Множество слоёв</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayer.cpp" line="592"/>
        <source>This item contains multiple layers. Displaying multiple layers in the table is not supported.</source>
        <translation>Этот элемент содержит более одного слоя. Отображение нескольких слоёв в таблице не поддерживается.</translation>
    </message>
</context>
<context>
    <name>QgsLegendLayerFile</name>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="244"/>
        <source>Save layer as...</source>
        <translation>Сохранить слой как...</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="335"/>
        <source>Error</source>
        <translation>Ошибка</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="319"/>
        <source>Saving done</source>
        <translation>Сохранение выполнено</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="319"/>
        <source>Export to Shapefile has been completed</source>
        <translation>Экспорт в shape-файл завершён</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="323"/>
        <source>Driver not found</source>
        <translation>Драйвер не найден</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="323"/>
        <source>ESRI Shapefile driver is not available</source>
        <translation>Драйвер shape-файлов ESRI не доступен</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="327"/>
        <source>Error creating shapefile</source>
        <translation>Ошибка создания shape-файла</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="328"/>
        <source>The shapefile could not be created (</source>
        <translation>Не удалось создать shape-файл (</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="332"/>
        <source>Layer creation failed</source>
        <translation>Не удалось создать слой</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="362"/>
        <source>&amp;Zoom to layer extent</source>
        <translation>&amp;Увеличить до границ слоя</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="365"/>
        <source>&amp;Show in overview</source>
        <translation>&amp;Показать в обзоре</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="373"/>
        <source>&amp;Remove</source>
        <translation>&amp;Удалить</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="382"/>
        <source>&amp;Open attribute table</source>
        <translation>&amp;Открыть таблицу атрибутов</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="395"/>
        <source>Save as shapefile...</source>
        <translation>Сохранить как shape-файл...</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="397"/>
        <source>Save selection as shapefile...</source>
        <translation>Сохранить выделение как shape-файл...</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="414"/>
        <source>&amp;Properties</source>
        <translation>&amp;Свойства</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="336"/>
        <source>Layer attribute table contains unsupported datatype(s)</source>
        <translation>Таблица атрибутов слоя включает неподдерживаемые типы данных</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="281"/>
        <source>Select the coordinate reference system for the saved shapefile.</source>
        <translation>Выберите систему координат для вновь создаваемого shape-файла.</translation>
    </message>
    <message>
        <location filename="../src/app/legend/qgslegendlayerfile.cpp" line="281"/>
        <source>The data points will be transformed from the layer coordinate reference system.</source>
        <translation>Данные в исходной системе координат слоя будут преобразованы.</translation>
    </message>
    <message>
        <source>Not a vector layer</source>
        <translation type="obsolete">Слой не является векторным</translation>
    </message>
    <message>
        <source>To open an attribute table, you must select a vector layer in the legend</source>
        <translation type="obsolete">Для открытия таблицы атрибутов, следует выбрать в легенде векторный слой</translation>
    </message>
    <message>
        <source>bad_alloc exception</source>
        <translation type="obsolete">Исключение bad_alloc</translation>
    </message>
    <message>
        <source>Filling the attribute table has been stopped because there was no more virtual memory left</source>
        <translation type="obsolete">Заполнение таблицы атрибутов остановлено, поскольку закончилась виртуальная память</translation>
    </message>
    <message>
        <source>Attribute table - </source>
        <translation type="obsolete">Таблица атрибутов — </translation>
    </message>
    <message>
        <source>Start editing failed</source>
        <translation type="obsolete">Не удалось начать редактирование</translation>
    </message>
    <message>
        <source>Provider cannot be opened for editing</source>
        <translation type="obsolete">Источник не может быть открыт для редактирования</translation>
    </message>
    <message>
        <source>Stop editing</source>
        <translation type="obsolete">Прекратить редактирование</translation>
    </message>
    <message>
        <source>Problems during roll back</source>
        <translation type="obsolete">Ошибка в процессе отката</translation>
    </message>
</context>
<context>
    <name>QgsMapCanvas</name>
    <message>
        <location filename="../src/gui/qgsmapcanvas.cpp" line="1223"/>
        <source>Could not draw</source>
        <translation>Ошибка отрисовки</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsmapcanvas.cpp" line="1223"/>
        <source>because</source>
        <translation>по причине</translation>
    </message>
</context>
<context>
    <name>QgsMapLayer</name>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="517"/>
        <source>%1 at line %2 column %3</source>
        <translation>%1 в строке %2, столбце %3</translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="620"/>
        <source>User database could not be opened.</source>
        <translation>Не удалось открыть базу данных пользователя.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="635"/>
        <source>The style table could not be created.</source>
        <translation>Не удалось создать таблицу стилей.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="650"/>
        <source>The style %1 was saved to database</source>
        <translation>Стиль %1 был сохранён в базе данных</translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="667"/>
        <source>The style %1 was updated in the database.</source>
        <translation>Стиль %1 был обновлён в базе данных.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="672"/>
        <source>The style %1 could not be updated in the database.</source>
        <translation>Не удалось обновить в базе данных стиль %1.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="679"/>
        <source>The style %1 could not be inserted into database.</source>
        <translation>Не удалось вставить в базу данных стиль %1.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsmaplayer.cpp" line="522"/>
        <source>style not found in database</source>
        <translation>стиль не найден в базе данных</translation>
    </message>
</context>
<context>
    <name>QgsMapToolIdentify</name>
    <message numerus="yes">
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="404"/>
        <source>- %1 features found</source>
        <comment>Identify results window title</comment>
        <translation>
            <numerusform>— найден %1 объект</numerusform>
            <numerusform>— найдено %1 объекта</numerusform>
            <numerusform>— найдено %1 объектов</numerusform>
        </translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="162"/>
        <source>(clicked coordinate)</source>
        <translation>(координаты щелчка)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolidentify.cpp" line="224"/>
        <source>WMS identify result for %1
%2</source>
        <translation>Результат WMS-определения для %1
%2</translation>
    </message>
</context>
<context>
    <name>QgsMapToolSplitFeatures</name>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="90"/>
        <source>Split error</source>
        <translation>Ошибка разделения</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="90"/>
        <source>An error occured during feature splitting</source>
        <translation>При разделении объектов возникла ошибка</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="85"/>
        <source>No feature split done</source>
        <translation>Разделение объектов не выполнено</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmaptoolsplitfeatures.cpp" line="85"/>
        <source>If there are selected features, the split tool only applies to the selected ones. If you like to split all features under the split line, clear the selection</source>
        <translation>Если есть выбранные объекты, инструмент разделения применяется только к ним. Если вы хотите разделить все объекты по линии разделения, следует очистить выделение</translation>
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
        <location filename="../tools/mapserver_export/qgsmapserverexport.cpp" line="201"/>
        <source>Overwrite File?</source>
        <translation>Перезаписать файл?</translation>
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
    <message>
        <location filename="../tools/mapserver_export/qgsmapserverexport.cpp" line="203"/>
        <source> exists. 
Do you want to overwrite it?</source>
        <comment>a fileName is prepended to this text, and appears in a dialog box</comment>
        <translation> уже существует. Вы хотите перезаписать этот файл?</translation>
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
        <translation>Мин. масштаб</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsmapserverexportbase.ui" line="236"/>
        <source>MaxScale</source>
        <translation>Макс. масштаб</translation>
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
        <translation>Сбросить</translation>
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
        <translation>Сегменты</translation>
    </message>
</context>
<context>
    <name>QgsMeasureDialog</name>
    <message>
        <location filename="../src/app/qgsmeasuredialog.cpp" line="196"/>
        <source>Segments (in meters)</source>
        <translation>Сегменты (в метрах)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmeasuredialog.cpp" line="199"/>
        <source>Segments (in feet)</source>
        <translation>Сегменты (в футах)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmeasuredialog.cpp" line="202"/>
        <source>Segments (in degrees)</source>
        <translation>Сегменты (в градусах)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsmeasuredialog.cpp" line="205"/>
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
    <name>QgsNewConnection</name>
    <message>
        <location filename="../src/app/qgsnewconnection.cpp" line="123"/>
        <source>Test connection</source>
        <translation>Проверка соединения</translation>
    </message>
    <message>
        <location filename="../src/app/qgsnewconnection.cpp" line="123"/>
        <source>Connection failed - Check settings and try again.

Extended error information:
</source>
        <translation>Не удалось соединиться — проверьте параметры и попробуйте ещё раз.

Дополнительная информация:
</translation>
    </message>
    <message>
        <location filename="../src/app/qgsnewconnection.cpp" line="119"/>
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
        <translation>Ограничить поиск пространственных таблиц, не включенных в geometry_columns, схемой «public»</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="64"/>
        <source>When searching for spatial tables that are not in the geometry_columns tables, restrict the search to tables that are in the public schema (for some databases this can save lots of time)</source>
        <translation>При поиске пространственных таблиц, не включенных в таблицу geometry_columns, сократить поиск до таблиц, содержащихся в схеме «public» (позволяет существенно сокращатить время поиска в некоторых БД)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="84"/>
        <source>Restrict the displayed tables to those that are in the geometry_columns table</source>
        <translation>Вывести только таблицы, содержащиеся в таблице geometry_columns</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewconnectionbase.ui" line="87"/>
        <source>Restricts the displayed tables to those that are in the geometry_columns table. This can speed up the initial display of spatial tables.</source>
        <translation>Вывести только таблицы, содержащиеся в таблице geometry_columns. Это может ускорить начальный поиск пространственных таблиц.</translation>
    </message>
</context>
<context>
    <name>QgsNewHttpConnectionBase</name>
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
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="13"/>
        <source>Create a new WMS connection</source>
        <translation>Создание нового WMS-соединения</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsnewhttpconnectionbase.ui" line="25"/>
        <source>Connection details</source>
        <translation>Параметры соединения</translation>
    </message>
</context>
<context>
    <name>QgsNorthArrowPlugin</name>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="80"/>
        <source>Bottom Left</source>
        <translation>Внизу слева</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="81"/>
        <source>Top Right</source>
        <translation>Вверху справа</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="81"/>
        <source>Bottom Right</source>
        <translation>Внизу справа</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="81"/>
        <source>Top Left</source>
        <translation>Вверху слева</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="94"/>
        <source>&amp;North Arrow</source>
        <translation>Указатель «&amp;север-юг»</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="95"/>
        <source>Creates a north arrow that is displayed on the map canvas</source>
        <translation>Вывод указателя «север-юг»</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="254"/>
        <source>&amp;Decorations</source>
        <translation>&amp;Оформление</translation>
    </message>
    <message>
        <location filename="../src/plugins/north_arrow/plugin.cpp" line="245"/>
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
        <location filename="../src/plugins/north_arrow/pluginguibase.ui" line="198"/>
        <source>Browse...</source>
        <translation>Обзор...</translation>
    </message>
</context>
<context>
    <name>QgsOptions</name>
    <message>
        <location filename="../src/app/qgsoptions.cpp" line="156"/>
        <source>Detected active locale on your system: </source>
        <translation>Обнаруженный системный язык: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsoptions.cpp" line="176"/>
        <source>to vertex</source>
        <translation>к вершинам</translation>
    </message>
    <message>
        <location filename="../src/app/qgsoptions.cpp" line="177"/>
        <source>to segment</source>
        <translation>к сегментам</translation>
    </message>
    <message>
        <location filename="../src/app/qgsoptions.cpp" line="178"/>
        <source>to vertex and segment</source>
        <translation>к вершинам и сегментам</translation>
    </message>
    <message>
        <location filename="../src/app/qgsoptions.cpp" line="345"/>
        <source>Semi transparent circle</source>
        <translation>Полупрозрачный круг</translation>
    </message>
    <message>
        <location filename="../src/app/qgsoptions.cpp" line="349"/>
        <source>Cross</source>
        <translation>Перекрестие</translation>
    </message>
    <message>
        <location filename="../src/app/qgsoptions.cpp" line="70"/>
        <source>Show all features</source>
        <translation>Показывать все объекты</translation>
    </message>
    <message>
        <location filename="../src/app/qgsoptions.cpp" line="71"/>
        <source>Show selected features</source>
        <translation>Показывать выбранные объекты</translation>
    </message>
    <message>
        <location filename="../src/app/qgsoptions.cpp" line="72"/>
        <source>Show features in current canvas</source>
        <translation>Показывать объекты из видимой области</translation>
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
        <location filename="../src/ui/qgsoptionsbase.ui" line="212"/>
        <source>Hide splash screen at startup</source>
        <translation>Не показывать заставку при запуске</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="188"/>
        <source>&lt;b&gt;Note: &lt;/b&gt;Theme changes take effect the next time QGIS is started</source>
        <translation>&lt;b&gt;Внимание: &lt;/b&gt;Изменение темы вступит в силу при следующем запуске QGIS</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="271"/>
        <source>&amp;Rendering</source>
        <translation>От&amp;рисовка</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="300"/>
        <source>Map display will be updated (drawn) after this many features have been read from the data source</source>
        <translation>Изображение карты будет обновлено (перерисовано) после того, как это количество объектов загружено из источника данных</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="854"/>
        <source>Select Global Default ...</source>
        <translation>Выбрать глобальную систему координат...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="341"/>
        <source>Make lines appear less jagged at the expense of some drawing performance</source>
        <translation>Рисовать сглаженные линии (снижает скорость отрисовки)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="283"/>
        <source>By default new la&amp;yers added to the map should be displayed</source>
        <translation>Добавляемые на карту слои &amp;видимы по умолчанию</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="472"/>
        <source>Measure tool</source>
        <translation>Инструмент измерений</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="542"/>
        <source>Search radius</source>
        <translation>Радиус поиска</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="351"/>
        <source>Fix problems with incorrectly filled polygons</source>
        <translation>Исправлять ошибки заливки полигонов</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="358"/>
        <source>Continuously redraw the map when dragging the legend/map divider</source>
        <translation>Обновлять карту при перемещении разделителя карты/легенды</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="382"/>
        <source>&amp;Map tools</source>
        <translation>&amp;Инструменты</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="580"/>
        <source>%</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="400"/>
        <source>Panning and zooming</source>
        <translation>Прокрутка и масштабирование</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="419"/>
        <source>Zoom</source>
        <translation>Увеличить</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="424"/>
        <source>Zoom and recenter</source>
        <translation>Увеличить и центрировать</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="434"/>
        <source>Nothing</source>
        <translation>Ничего</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="32"/>
        <source>&amp;General</source>
        <translation>&amp;Общие</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="907"/>
        <source>Locale</source>
        <translation>Язык</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="922"/>
        <source>Locale to use instead</source>
        <translation>Язык, используемый вместо системного</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="961"/>
        <source>Additional Info</source>
        <translation>Дополнительная информация</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="967"/>
        <source>Detected active locale on your system:</source>
        <translation>Обнаруженный системный язык:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="348"/>
        <source>Selecting this will unselect the &apos;make lines less&apos; jagged toggle</source>
        <translation>Активация этого параметра выключит флажок «Рисовать сглаженные линии»</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="613"/>
        <source>Digitizing</source>
        <translation>Оцифровка</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="619"/>
        <source>Rubberband</source>
        <translation>Резиновая нить</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="635"/>
        <source>Line width in pixels</source>
        <translation>Ширина линии в пикселях</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="671"/>
        <source>Snapping</source>
        <translation>Прилипание</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="429"/>
        <source>Zoom to mouse cursor</source>
        <translation>Увеличить в положении курсора</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="38"/>
        <source>Project files</source>
        <translation>Файлы проектов</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="56"/>
        <source>Prompt to save project changes when required</source>
        <translation>Запрашивать сохранение изменений в проекте, когда это необходимо</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="63"/>
        <source>Warn when opening a project file saved with an older version of QGIS</source>
        <translation>Предупреждать при попытке открытия файлов проекта старых версий QGIS</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="73"/>
        <source>Default Map Appearance (overridden by project properties)</source>
        <translation>Вид карты по умолчанию (заменяется свойствами проекта)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="79"/>
        <source>Selection color</source>
        <translation>Цвет выделения</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="115"/>
        <source>Background color</source>
        <translation>Цвет фона</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="154"/>
        <source>&amp;Application</source>
        <translation>&amp;Приложение</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="166"/>
        <source>Icon theme</source>
        <translation>Тема значков</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="198"/>
        <source>Capitalise layer names in legend</source>
        <translation>Выводить имя слоя с заглавной буквы</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="205"/>
        <source>Display classification attribute names in legend</source>
        <translation>Показывать в легенде атрибуты классификации</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="277"/>
        <source>Rendering behavior</source>
        <translation>Параметры отрисовки</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="290"/>
        <source>Number of features to draw before updating the display</source>
        <translation>Количество объектов для отрисовки между обновлениями экрана</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="313"/>
        <source>&lt;b&gt;Note:&lt;/b&gt; Use zero to prevent display updates until all features have been rendered</source>
        <translation>&lt;b&gt;Внимание:&lt;/b&gt; введите 0, чтобы запретить обновление экрана до отрисовки всех объектов</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="323"/>
        <source>Rendering quality</source>
        <translation>Качество отрисовки</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="442"/>
        <source>Zoom factor</source>
        <translation>Фактор увеличения</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="449"/>
        <source>Mouse wheel action</source>
        <translation>Действие при прокрутке колеса мыши</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="519"/>
        <source>Rubberband color</source>
        <translation>Цвет линии</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="529"/>
        <source>Ellipsoid for distance calculations</source>
        <translation>Эллипсоид для вычисления расстояний</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="560"/>
        <source>&lt;b&gt;Note:&lt;/b&gt; Specify the search radius as a percentage of the map width</source>
        <translation>&lt;b&gt;Внимание:&lt;/b&gt; радиус поиска задаётся в процентах от ширины видимой карты</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="570"/>
        <source>Search radius for identifying features and displaying map tips</source>
        <translation>Радиус поиска для определения объектов и всплывающих описаний</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="625"/>
        <source>Line width</source>
        <translation>Ширина линии</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="645"/>
        <source>Line colour</source>
        <translation>Цвет линии</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="677"/>
        <source>Default snap mode</source>
        <translation>Режим прилипания по умолчанию</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="707"/>
        <source>Default snapping tolerance in layer units</source>
        <translation>Порог прилипания по умолчанию в единицах слоя</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="737"/>
        <source>Search radius for vertex edits in layer units</source>
        <translation>Радиус поиска для редактирования вершин в единицах слоя</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="770"/>
        <source>Vertex markers</source>
        <translation>Маркеры вершин</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="776"/>
        <source>Marker style</source>
        <translation>Стиль маркера</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="913"/>
        <source>Override system locale</source>
        <translation>Переопределить системный язык</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="935"/>
        <source>&lt;b&gt;Note:&lt;/b&gt; Enabling / changing overide on local requires an application restart</source>
        <translation>&lt;b&gt;Внимание:&lt;/b&gt; для переопределения параметров языка необходимо перезапустить приложение</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="978"/>
        <source>Proxy</source>
        <translation>Прокси-сервер</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="984"/>
        <source>Use proxy for web access</source>
        <translation>Использовать прокси-сервер для внешних соединений</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="996"/>
        <source>Host</source>
        <translation>Узел</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="1009"/>
        <source>Port</source>
        <translation>Порт</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="1022"/>
        <source>User</source>
        <translation>Пользователь</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="1049"/>
        <source>Leave this blank if no proxy username / password are required</source>
        <translation>Оставьте это поле пустым, если для прокси-сервера не требуется имя пользователя и пароль</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="1039"/>
        <source>Password</source>
        <translation>Пароль</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="219"/>
        <source>Open attribute table in a dock window</source>
        <translation>Открывать таблицу атрибутов во встраиваемом окне</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="823"/>
        <source>CRS</source>
        <translation>Система координат</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="864"/>
        <source>When layer is loaded that has no coordinate reference system (CRS)</source>
        <translation>При загрузке слоя, не содержащего сведений о системе координат</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="882"/>
        <source>Prompt for CRS</source>
        <translation>Запрашивать систему координат</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="889"/>
        <source>Project wide default CRS will be used</source>
        <translation>Использовать значение по умолчанию для данного проекта</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="896"/>
        <source>Global default CRS displa&amp;yed below will be used</source>
        <translation>Использовать ниж&amp;еприведённую глобальную систему координат</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsoptionsbase.ui" line="232"/>
        <source>Attribute table behaviour</source>
        <translation>Таблица атрибутов</translation>
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
    <name>QgsPgGeoprocessing</name>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="119"/>
        <source>Buffer features in layer %1</source>
        <translation>Буферизация объектов слоя %1</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="364"/>
        <source>Error connecting to the database</source>
        <translation>Ошибка подключения к базе данных</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="76"/>
        <source>&amp;Buffer features</source>
        <translation>&amp;Буферизация объектов</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="78"/>
        <source>A new layer is created in the database with the buffered features.</source>
        <translation>В базе данных создан новый слой буферных зон.</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="445"/>
        <source>&amp;Geoprocessing</source>
        <translation>&amp;Обработка данных</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="345"/>
        <source>Unable to add geometry column</source>
        <translation>Не удалось добавить поле геометрии</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="347"/>
        <source>Unable to add geometry column to the output table </source>
        <translation>Не удалось добавить поле геометрии в выходную таблицу </translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="353"/>
        <source>Unable to create table</source>
        <translation>Не удалось создать таблицу</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="355"/>
        <source>Failed to create the output table </source>
        <translation>Не удалось создать выходную таблицу </translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="370"/>
        <source>No GEOS support</source>
        <translation>Поддержка GEOS не установлена</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="371"/>
        <source>Buffer function requires GEOS support in PostGIS</source>
        <translation>Буферизация объектов PostGIS требует поддержки GEOS</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="384"/>
        <source>No Active Layer</source>
        <translation>Нет активного слоя</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="385"/>
        <source>You must select a layer in the legend to buffer</source>
        <translation>Для буферизации необходимо выбрать слой в легенде</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="376"/>
        <source>Not a PostgreSQL/PostGIS Layer</source>
        <translation>Слой не в формате PostgreSQL/PostGIS</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="379"/>
        <source> is not a PostgreSQL/PostGIS layer.
</source>
        <translation> не является слоем PostgreSQL/PostGIS.
</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="379"/>
        <source>Geoprocessing functions are only available for PostgreSQL/PostGIS Layers</source>
        <translation>Функции обработки данных доступны только для слоёв PostgreSQL/PostGIS</translation>
    </message>
    <message>
        <location filename="../src/plugins/geoprocessing/qgspggeoprocessing.cpp" line="78"/>
        <source>Create a buffer for a PostgreSQL layer. </source>
        <translation>Создание буферных зон для слоя PostgreSQL. </translation>
    </message>
</context>
<context>
    <name>QgsPgQueryBuilder</name>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="84"/>
        <source>Table &lt;b&gt;%1&lt;/b&gt; in database &lt;b&gt;%2&lt;/b&gt; on host &lt;b&gt;%3&lt;/b&gt;, user &lt;b&gt;%4&lt;/b&gt;</source>
        <translation>Таблица &lt;b&gt;%1&lt;/b&gt; в базе данных &lt;b&gt;%2&lt;/b&gt; на сервере &lt;b&gt;%3&lt;/b&gt;, пользователь &lt;b&gt;%4&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="68"/>
        <source>Connection Failed</source>
        <translation>Не удалось соединиться</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="68"/>
        <source>Connection to the database failed:</source>
        <translation>Не удалось подключиться к базе данных:</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="207"/>
        <source>Database error</source>
        <translation>Ошибка базы данных</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="282"/>
        <source>Query Result</source>
        <translation>Результат запроса</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="284"/>
        <source>The where clause returned </source>
        <translation>По условию WHERE получено </translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="284"/>
        <source> rows.</source>
        <translation> строк.</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="288"/>
        <source>Query Failed</source>
        <translation>Ошибка запроса</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="290"/>
        <source>An error occurred when executing the query:</source>
        <translation>При выполнении запроса возникла ошибка:</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="344"/>
        <source>No Records</source>
        <translation>Нет записей</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="344"/>
        <source>The query you specified results in zero records being returned. Valid PostgreSQL layers must have at least one feature.</source>
        <translation>По указанному запросу не было получено ни одной записи. Действительные слои PostgreSQL должны содержать как минимум один объект.</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="207"/>
        <source>&lt;p&gt;Failed to get sample of field values using SQL:&lt;/p&gt;&lt;p&gt;</source>
        <translation>&lt;p&gt;Ошибка получения образцов значений по SQL-запросу:&lt;/p&gt;&lt;p&gt;</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="271"/>
        <source>No Query</source>
        <translation>Нет запроса</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="271"/>
        <source>You must create a query before you can test it</source>
        <translation>Следует создать запрос, прежде чем он сможет быть проверен</translation>
    </message>
    <message>
        <location filename="../src/app/qgspgquerybuilder.cpp" line="337"/>
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
        <translation>Источник данных</translation>
    </message>
</context>
<context>
    <name>QgsPluginInstaller</name>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="33"/>
        <source>Couldn&apos;t parse output from the repository</source>
        <translation>Не удалось обработать ответ репозитория</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="34"/>
        <source>Couldn&apos;t open the system plugin directory</source>
        <translation>Не удалось открыть системный каталог модулей</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="39"/>
        <source>Couldn&apos;t open the local plugin directory</source>
        <translation>Не удалось открыть локальный каталог модулей</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="45"/>
        <source>Fetch Python Plugins...</source>
        <translation>Загрузить модули...</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="47"/>
        <source>Install more plugins from remote repositories</source>
        <translation>Установка дополнительных модулей из удалённых репозиториев</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="50"/>
        <source>Looking for new plugins...</source>
        <translation>Поиск новых модулей...</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="51"/>
        <source>There is a new plugin available</source>
        <translation>Доступен новый модуль</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="54"/>
        <source>There is a plugin update available</source>
        <translation>Доступна новая версия модуля</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="55"/>
        <source>QGIS Python Plugin Installer</source>
        <translation>Установка модулей QGIS</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="60"/>
        <source>Error reading repository:</source>
        <translation>Ошибка чтения из репозитория:</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="66"/>
        <source>Nothing to remove! Plugin directory doesn&apos;t exist:</source>
        <translation>Удаление невозможно! Каталог модулей не существует:</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="67"/>
        <source>Failed to remove the directory:</source>
        <translation>Не удалось удалить каталог:</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="69"/>
        <source>Check permissions or remove it manually</source>
        <translation>Проверьте права доступа или удалите его вручную</translation>
    </message>
</context>
<context>
    <name>QgsPluginInstallerDialog</name>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="221"/>
        <source>QGIS Python Plugin Installer</source>
        <translation>Установка модулей QGIS</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="117"/>
        <source>Error reading repository:</source>
        <translation>Ошибка чтения из репозитория:</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="118"/>
        <source>all repositories</source>
        <translation>все репозитории</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="119"/>
        <source>connected</source>
        <translation>подключен</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="120"/>
        <source>This repository is connected</source>
        <translation>Репозиторий подключен</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="121"/>
        <source>unavailable</source>
        <translation>недоступен</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="122"/>
        <source>This repository is enabled, but unavailable</source>
        <translation>Репозиторий активен, но недоступен</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="123"/>
        <source>disabled</source>
        <translation>выключен</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="124"/>
        <source>This repository is disabled</source>
        <translation>Репозиторий выключен</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="212"/>
        <source>This repository is blocked due to incompatibility with your Quantum GIS version</source>
        <translation>Репозиторий заблокирован по причине несовместимости с вашей версией Quantum GIS</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="138"/>
        <source>orphans</source>
        <translation>неподдерживаемые</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="127"/>
        <source>any status</source>
        <translation>все</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="128"/>
        <source>not installed</source>
        <comment>plural</comment>
        <translation>не установленные</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="129"/>
        <source>installed</source>
        <comment>plural</comment>
        <translation>установленные</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="134"/>
        <source>upgradeable and news</source>
        <translation>обновляемые и новые</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="139"/>
        <source>This plugin is not installed</source>
        <translation>Модуль не установлен</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="140"/>
        <source>This plugin is installed</source>
        <translation>Модуль установлен</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="141"/>
        <source>This plugin is installed, but there is an updated version available</source>
        <translation>Модуль установлен, но доступна более новая версия</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="142"/>
        <source>This plugin is installed, but I can&apos;t find it in any enabled repository</source>
        <translation>Модуль установлен, но не найден в активных репозиториях</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="143"/>
        <source>This plugin is not installed and is seen for the first time</source>
        <translation>Модуль не установлен и впервые зарегистрирован</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="144"/>
        <source>This plugin is installed and is newer than its version available in a repository</source>
        <translation>Модуль установлен, и его версия выше доступной в репозитории</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="148"/>
        <source>not installed</source>
        <comment>singular</comment>
        <translation>не установлен</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="153"/>
        <source>installed</source>
        <comment>singular</comment>
        <translation>установлен</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="150"/>
        <source>upgradeable</source>
        <comment>singular</comment>
        <translation>обновляем</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="152"/>
        <source>new!</source>
        <comment>singular</comment>
        <translation>новый!</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="154"/>
        <source>invalid</source>
        <comment>singular</comment>
        <translation>недействительный</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="161"/>
        <source>installed version</source>
        <translation>установленная версия</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="158"/>
        <source>available version</source>
        <translation>доступная версия</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="160"/>
        <source>That&apos;s the newest available version</source>
        <translation>Эта версия является самой последней</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="162"/>
        <source>There is no version available for download</source>
        <translation>Доступных для загрузки версий не найдено</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="168"/>
        <source>only locally available</source>
        <translation>доступен только локально</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="173"/>
        <source>Install plugin</source>
        <translation>Установить модуль</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="175"/>
        <source>Reinstall plugin</source>
        <translation>Переустановить модуль</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="171"/>
        <source>Upgrade plugin</source>
        <translation>Обновить модуль</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="178"/>
        <source>Install/upgrade plugin</source>
        <translation>Установить/обновить модуль</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="174"/>
        <source>Downgrade plugin</source>
        <translation>Понизить версию</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="180"/>
        <source>Are you sure you want to downgrade the plugin to the latest available version? The installed one is newer!</source>
        <translation>Вы уверены, что хотите понизить версию модуля до последней доступной? Установленная версия выше!</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="181"/>
        <source>Plugin installation failed</source>
        <translation>Установка модуля не выполнена</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="182"/>
        <source>Plugin has disappeared</source>
        <translation>Модуль потерян</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="183"/>
        <source>The plugin seems to have been installed but I don&apos;t know where. Probably the plugin package contained a wrong named directory.
Please search the list of installed plugins. I&apos;m nearly sure you&apos;ll find the plugin there, but I just can&apos;t determine which of them it is. It also means that I won&apos;t be able to determine if this plugin is installed and inform you about available updates. However the plugin may work. Please contact the plugin author and submit this issue.</source>
        <translation>Модуль был установлен, но не был обнаружен после установки. Вероятно, архив модуля содержал каталог с неверным именем.
Пожалуйста, просмотрите список установленных модулей. Скорее всего, модуль будет в этом списке, но Quantum GIS не сможет определить, который из них. Кроме того, это означает что будет невозможно определить состояние модуля и наличие обновлений. Тем не менее, модуль может работать. Пожалуйста, свяжитесь с его автором и сообщите об этой ошибке.</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="184"/>
        <source>Plugin installed successfully</source>
        <translation>Модуль успешно установлен</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="185"/>
        <source>Python plugin installed.
You have to enable it in the Plugin Manager.</source>
        <translation type="obsolete">Модуль установлен.
Вы можете включить его в менеджере модулей.</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="187"/>
        <source>Python plugin reinstalled.
You have to restart Quantum GIS to reload it.</source>
        <translation type="obsolete">Модуль переустановлен.
Для его перезагрузки следует перезапустить Quantum GIS.</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="197"/>
        <source>Plugin uninstall failed</source>
        <translation>Удаление модуля не выполнено</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="194"/>
        <source>Are you sure you want to uninstall the following plugin?</source>
        <translation>Вы уверены, что хотите удалить этот модуль?</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="195"/>
        <source>Warning: this plugin isn&apos;t available in any accessible repository!</source>
        <translation>Внимание: этот модуль не доступен ни в одном активном репозитории!</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="199"/>
        <source>Plugin uninstalled successfully</source>
        <translation>Модуль успешно удалён</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="204"/>
        <source>You are going to add some plugin repositories neither authorized nor supported by the Quantum GIS team, however provided by folks associated with us. Plugin authors generally make efforts to make their works useful and safe, but we can&apos;t assume any responsibility for them. FEEL WARNED!</source>
        <translation>ВНИМАНИЕ! Вы собираетесь добавить один или несколько репозиториев, которые не поддерживаются командой Quantum GIS. Авторы модулей, как правило, стараются сделать свои работы полезными и безопасными, но мы не можем нести за них никакую ответственность!</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="216"/>
        <source>Unable to add another repository with the same URL!</source>
        <translation>Невозможно добавить другой репозиторий с тем же URL!</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="217"/>
        <source>Are you sure you want to remove the following repository?</source>
        <translation>Вы уверены, что хотите удалить следующий репозиторий?</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="145"/>
        <source>This plugin is incompatible with your Quantum GIS version and probably won&apos;t work.</source>
        <translation>Модуль несовместим с вашей версией Quantum GIS и может работать неправильно.</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="146"/>
        <source>The required Python module is not installed.
For more information, please visit its homepage.</source>
        <translation type="obsolete">Требуемая библиотека Python не установлена.
Обратитесь к домашней странице модуля за дополнительной информацией.</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="147"/>
        <source>This plugin seems to be broken.
It has been installed but can&apos;t be loaded.
Here is the error message:</source>
        <translation>Модуль неисправен.
Установка прошла успешно, но он не может быть загружен.
Сообщение об ошибке:</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="155"/>
        <source>Note that it&apos;s an uninstallable core plugin</source>
        <translation>Обратите внимание, что модуль является базовым и не может быть удалён</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="163"/>
        <source>This plugin is broken</source>
        <translation>Модуль неисправен</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="164"/>
        <source>This plugin requires a newer version of Quantum GIS</source>
        <translation>Модуль требует более позднюю версию Quantum GIS</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="165"/>
        <source>This plugin requires a missing module</source>
        <translation>Требуются отстутствующие в системе библиотеки</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="186"/>
        <source>Plugin reinstalled successfully</source>
        <translation>Модуль успешно переустановлен</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="188"/>
        <source>The plugin is designed for a newer version of Quantum GIS. The minimum required version is:</source>
        <translation>Модуль написан для более новой версии Quantum GIS. Минимальная требуемая версия:</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="189"/>
        <source>The plugin depends on some components missing on your system. You need to install the following Python module in order to enable it:</source>
        <translation>Для работы модуля требуются компоненты, которые не найдены в вашей системе. Чтобы включить его, требуется установить следующие библиотеки:</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="190"/>
        <source>The plugin is broken. Python said:</source>
        <translation>Модуль неисправен. Сообщение Python:</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="146"/>
        <source>The required Python module is not installed.
For more information, please visit its homepage and Quantum GIS wiki.</source>
        <translation>Требуемая библиотека Python не установлена.
Обратитесь к домашней странице модуля или вики Quantum GIS за дополнительной информацией.</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="185"/>
        <source>Python plugin installed.
Now you need to enable it in Plugin Manager.</source>
        <translation>Модуль установлен.
Вы можете включить его в менеджере модулей.</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="187"/>
        <source>Python plugin reinstalled.
You need to restart Quantum GIS in order to reload it.</source>
        <translation>Модуль переустановлен.
Для его перезагрузки следует перезапустить Quantum GIS.</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="203"/>
        <source>Python plugin uninstalled. Note that tou may need to restart Quantum GIS in order to remove it completely.</source>
        <translation>Модуль удалён. Для завершения удаления может потребоваться перезапуск Quantum GIS.</translation>
    </message>
</context>
<context>
    <name>QgsPluginInstallerDialogBase</name>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="20"/>
        <source>QGIS Python Plugin Installer</source>
        <translation>Установка модулей QGIS</translation>
    </message>
    <message>
        <source>QGIS Plugin Installer</source>
        <translation type="obsolete">Установка модулей QGIS</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="36"/>
        <source>Plugins</source>
        <translation>Модули</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="39"/>
        <source>List of available and installed plugins</source>
        <translation>Список доступных и установленных модулей</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="50"/>
        <source>Filter:</source>
        <translation>Фильтр:</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="63"/>
        <source>Display only plugins containing this word in their metadata</source>
        <translation>Показывать только модули, содержащие в метаданных это слово</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="85"/>
        <source>Display only plugins from given repository</source>
        <translation>Показывать модули только из данного репозитория</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="89"/>
        <source>all repositories</source>
        <translation>все репозитории</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="109"/>
        <source>Display only plugins with matching status</source>
        <translation>Показывать модули с подходщяим состоянием</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="242"/>
        <source>Status</source>
        <translation>Состояние</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="247"/>
        <source>Name</source>
        <translation>Имя</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="144"/>
        <source>Version</source>
        <translation>Версия</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="149"/>
        <source>Description</source>
        <translation>Описание</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="154"/>
        <source>Author</source>
        <translation>Автор</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="159"/>
        <source>Repository</source>
        <translation>Репозиторий</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="197"/>
        <source>Install, reinstall or upgrade the selected plugin</source>
        <translation>Установить, переустановить или обновить выбранный модуль</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="200"/>
        <source>Install/upgrade plugin</source>
        <translation>Установить/обновить модуль</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="213"/>
        <source>Uninstall the selected plugin</source>
        <translation>Удалить выбранный модуль</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="216"/>
        <source>Uninstall plugin</source>
        <translation>Удалить модуль</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="226"/>
        <source>Repositories</source>
        <translation>Репозитории</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="229"/>
        <source>List of plugin repositories</source>
        <translation>Список репозиториев модулей</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="252"/>
        <source>URL</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="266"/>
        <source>Allow the Installer to look for updates and news in enabled repositories on QGIS startup</source>
        <translation>Разрешить поиск обновлений в активных репозиториях при запуске QGIS</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="269"/>
        <source>Check for updates on startup</source>
        <translation>Проверять обновления при запуске</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="298"/>
        <source>Add third party plugin repositories to the list</source>
        <translation>Добавить в список репозитории сторонних разработчиков</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="301"/>
        <source>Add 3rd party repositories</source>
        <translation>Добавить сторонние репозитории</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="324"/>
        <source>Add a new plugin repository</source>
        <translation>Добавить новый репозиторий</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="327"/>
        <source>Add...</source>
        <translation>Добавить...</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="337"/>
        <source>Edit the selected repository</source>
        <translation>Изменить выбранный репозиторий</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="340"/>
        <source>Edit...</source>
        <translation>Изменить...</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="350"/>
        <source>Remove the selected repository</source>
        <translation>Удалить выбранный репозиторий</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="353"/>
        <source>Delete</source>
        <translation>Удалить</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="369"/>
        <source>The plugins will be installed to ~/.qgis/python/plugins</source>
        <translation>Модули будут установлены в ~/.qgis/python/plugins</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="385"/>
        <source>Close the Installer window</source>
        <translation>Закрыть окно установки</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerbase.ui" line="388"/>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
</context>
<context>
    <name>QgsPluginInstallerFetchingDialog</name>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="73"/>
        <source>Success</source>
        <translation>Успешное завершение</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="74"/>
        <source>Resolving host name...</source>
        <translation>Поиск узла...</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="75"/>
        <source>Connecting...</source>
        <translation>Соединение...</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="76"/>
        <source>Host connected. Sending request...</source>
        <translation>Соединение установлено. Отправка запроса...</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="77"/>
        <source>Downloading data...</source>
        <translation>Загрузка данных...</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="78"/>
        <source>Idle</source>
        <translation>Бездействие</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="79"/>
        <source>Closing connection...</source>
        <translation>Закрытие соединения...</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="80"/>
        <source>Error</source>
        <translation>Ошибка</translation>
    </message>
</context>
<context>
    <name>QgsPluginInstallerFetchingDialogBase</name>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerfetchingbase.ui" line="14"/>
        <source>Fetching repositories</source>
        <translation>Получение списка репозиториев</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerfetchingbase.ui" line="39"/>
        <source>Overall progress:</source>
        <translation>Общий прогресс:</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerfetchingbase.ui" line="111"/>
        <source>Abort fetching</source>
        <translation>Прервать получение</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerfetchingbase.ui" line="167"/>
        <source>Repository</source>
        <translation>Репозиторий</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerfetchingbase.ui" line="172"/>
        <source>State</source>
        <translation>Состояние</translation>
    </message>
</context>
<context>
    <name>QgsPluginInstallerInstallingDialog</name>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="86"/>
        <source>Installing...</source>
        <translation>Установка...</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="87"/>
        <source>Resolving host name...</source>
        <translation>Поиск узла...</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="88"/>
        <source>Connecting...</source>
        <translation>Соединение...</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="89"/>
        <source>Host connected. Sending request...</source>
        <translation>Соединение установлено. Отправка запроса...</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="90"/>
        <source>Downloading data...</source>
        <translation>Загрузка данных...</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="91"/>
        <source>Idle</source>
        <translation>Бездействие</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="92"/>
        <source>Closing connection...</source>
        <translation>Закрытие соединения...</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="95"/>
        <source>Error</source>
        <translation>Ошибка</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="98"/>
        <source>Failed to unzip the plugin package. Probably it&apos;s broken or missing from the repository. You may also want to make sure that you have write permission to the plugin directory:</source>
        <translation>Не удалось распаковать архив модуля. Возможно, файл повреждён или отсутствует в репозитории. Данная ошибка может возникать по причине отсутствия прав на запись в каталог модулей:</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="99"/>
        <source>Aborted by user</source>
        <translation>Отменено пользователем</translation>
    </message>
</context>
<context>
    <name>QgsPluginInstallerInstallingDialogBase</name>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerinstallingbase.ui" line="14"/>
        <source>QGIS Python Plugin Installer</source>
        <translation>Установка модулей QGIS</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerinstallingbase.ui" line="41"/>
        <source>Installing plugin:</source>
        <translation>Установка модуля:</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerinstallingbase.ui" line="63"/>
        <source>Connecting...</source>
        <translation>Подключение...</translation>
    </message>
</context>
<context>
    <name>QgsPluginInstallerPluginErrorDialog</name>
    <message>
        <location filename="../python/plugins/plugin_installer/i18n.cpp" line="106"/>
        <source>no error message received</source>
        <translation>ссобщений об ошибках не зафиксировано</translation>
    </message>
</context>
<context>
    <name>QgsPluginInstallerPluginErrorDialogBase</name>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerpluginerrorbase.ui" line="20"/>
        <source>Error loading plugin</source>
        <translation>Ошибка загрузки модуля</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerpluginerrorbase.ui" line="35"/>
        <source>The plugin seems to be invalid or have unfulfilled dependencies. It has been installed, but can&apos;t be loaded. If you really need this plugin, you can contact its author or &lt;a href=&quot;http://lists.osgeo.org/mailman/listinfo/qgis-user&quot;&gt;QGIS users group&lt;/a&gt; and try to solve the problem. If not, you can just uninstall it. Here is the error message below:</source>
        <translation>Модуль недействителен или имеет неудовлетворённые зависимости. Установка прошла успешно, но модуль не может быть загружен. Если вам действительно необходим данный модуль, вы можете связаться с его автором через &lt;a href=&quot;http://lists.osgeo.org/mailman/listinfo/qgis-user&quot;&gt;группу пользователей QGIS&lt;/a&gt; и попытаться разрешить проблему. Если нет, вы можете просто его удалить. Подробная информация об ошибке приведена ниже:</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerpluginerrorbase.ui" line="83"/>
        <source>Do you want to uninstall this plugin now? If you&apos;re unsure, probably you would like to do this.</source>
        <translation>Вы хотите удалить этот модуль сейчас? Пожалуйста, ответьте утвердительно, если вы не уверены.</translation>
    </message>
</context>
<context>
    <name>QgsPluginInstallerRepositoryDetailsDialogBase</name>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerrepositorybase.ui" line="20"/>
        <source>Repository details</source>
        <translation>Данные репозитория</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerrepositorybase.ui" line="41"/>
        <source>Name:</source>
        <translation>Имя:</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerrepositorybase.ui" line="67"/>
        <source>Enter a name for the repository</source>
        <translation>Введите имя репозитория</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerrepositorybase.ui" line="74"/>
        <source>URL:</source>
        <translation>URL:</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerrepositorybase.ui" line="84"/>
        <source>Enter the repository URL, beginning with &quot;http://&quot;</source>
        <translation>Введите URL репозитория, начиная с http://</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerrepositorybase.ui" line="106"/>
        <source>Enable or disable the repository (disabled repositories will be omitted)</source>
        <translation>Включить или выключить репозиторий (выключенные репозитории будут пропущены)</translation>
    </message>
    <message>
        <location filename="../python/plugins/plugin_installer/qgsplugininstallerrepositorybase.ui" line="109"/>
        <source>Enabled</source>
        <translation>Активен</translation>
    </message>
</context>
<context>
    <name>QgsPluginManager</name>
    <message>
        <location filename="../src/app/qgspluginmanager.cpp" line="209"/>
        <source>No Plugins</source>
        <translation>Модулей не найдено</translation>
    </message>
    <message>
        <location filename="../src/app/qgspluginmanager.cpp" line="209"/>
        <source>No QGIS plugins found in </source>
        <translation>Модули QGIS не найдены в </translation>
    </message>
    <message>
        <location filename="../src/app/qgspluginmanager.cpp" line="85"/>
        <source>&amp;Select All</source>
        <translation>В&amp;ыбрать все</translation>
    </message>
    <message>
        <location filename="../src/app/qgspluginmanager.cpp" line="86"/>
        <source>&amp;Clear All</source>
        <translation>&amp;Отключить все</translation>
    </message>
    <message>
        <location filename="../src/app/qgspluginmanager.cpp" line="154"/>
        <source>[ incompatible ]</source>
        <translation>[ несовместимый ]</translation>
    </message>
</context>
<context>
    <name>QgsPluginManagerBase</name>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="16"/>
        <source>QGIS Plugin Manager</source>
        <translation>Менеджер модулей QGIS</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="25"/>
        <source>To enable / disable a plugin, click its checkbox or description</source>
        <translation>Для включения или выключения модуля, щёлкните на соответствующем ему флажке или описании</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="45"/>
        <source>&amp;Filter</source>
        <translation>&amp;Фильтр</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="58"/>
        <source>Plugin Directory:</source>
        <translation>Каталог модулей:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgspluginmanagerbase.ui" line="71"/>
        <source>Directory</source>
        <translation>Каталог</translation>
    </message>
</context>
<context>
    <name>QgsPointDialog</name>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="497"/>
        <source>Zoom In</source>
        <translation>Увеличить</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="496"/>
        <source>z</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="502"/>
        <source>Zoom Out</source>
        <translation>Уменьшить</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="501"/>
        <source>Z</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="505"/>
        <source>Zoom To Layer</source>
        <translation>Увеличить до слоя</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="507"/>
        <source>Zoom to Layer</source>
        <translation>Увеличить до слоя</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="510"/>
        <source>Pan Map</source>
        <translation>Прокрутка карты</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="511"/>
        <source>Pan the map</source>
        <translation>Прокрутка карты</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="514"/>
        <source>Add Point</source>
        <translation>Добавить точку</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="515"/>
        <source>.</source>
        <translation>.</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="516"/>
        <source>Capture Points</source>
        <translation>Захватить точки</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="519"/>
        <source>Delete Point</source>
        <translation>Удалить точку</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="520"/>
        <source>Delete Selected</source>
        <translation>Удалить выбранное</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="573"/>
        <source>Linear</source>
        <translation>Линейное</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="574"/>
        <source>Helmert</source>
        <translation>Гельмерта</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="206"/>
        <source>Choose a name for the world file</source>
        <translation>Выберите имя файла привязки</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="273"/>
        <source>Warning</source>
        <translation>Внимание</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="288"/>
        <source>Affine</source>
        <translation>Аффинное</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="298"/>
        <source>Not implemented!</source>
        <translation>Функция не реализована!</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="293"/>
        <source>&lt;p&gt;An affine transform requires changing the original raster file. This is not yet supported.&lt;/p&gt;</source>
        <translation>&lt;p&gt;Аффинное преобразование требует изменения оригинального растрового файла. В данный момент, эта функция не поддерживается.&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="300"/>
        <source>&lt;p&gt;The </source>
        <translation>&lt;p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="301"/>
        <source> transform is not yet supported.&lt;/p&gt;</source>
        <translation> преобразование в настоящий момент не поддерживается.&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="339"/>
        <source>Error</source>
        <translation>Ошибка</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="340"/>
        <source>Could not write to </source>
        <translation>Не удалось сохранить в </translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="280"/>
        <source>Currently all modified files will be written in TIFF format.</source>
        <translation>В данный момент все изменённые файлы сохраняются в формате TIFF.</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="279"/>
        <source>&lt;p&gt;A Helmert transform requires modifications in the raster layer.&lt;/p&gt;&lt;p&gt;The modified raster will be saved in a new file and a world file will be generated for this new file instead.&lt;/p&gt;&lt;p&gt;Are you sure that this is what you want?&lt;/p&gt;</source>
        <translation>&lt;p&gt;Преобразование Гельмерта требует изменения исходного слоя.&lt;/p&gt;&lt;p&gt;Изменённый растр будет сохранён в новом файле и файл привязки будет создан уже для нового файла.&lt;/p&gt;&lt;p&gt;Вы уверены, что хотите продолжить?&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/georeferencer/qgspointdialog.cpp" line="224"/>
        <source>-modified</source>
        <comment>Georeferencer:QgsPointDialog.cpp - used to modify a user given file name</comment>
        <translation></translation>
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
        <translation>Прокрутка</translation>
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
    <name>QgsPostgresProvider</name>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="125"/>
        <source>Unable to access relation</source>
        <translation>Не удаётся открыть реляцию</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="106"/>
        <source>Unable to access the </source>
        <translation>Не удаётся открыть реляцию </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="128"/>
        <source> relation.
The error message from the database was:
</source>
        <translation>.
Сообщение базы данных:
</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="920"/>
        <source>No suitable key column in table</source>
        <translation>В таблице нет подходящего ключевого поля</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="924"/>
        <source>The table has no column suitable for use as a key.

Qgis requires that the table either has a column of type
int4 with a unique constraint on it (which includes the
primary key) or has a PostgreSQL oid column.
</source>
        <translation>Таблица не имеет поля, подходящего в качестве ключевого.

Для успешной работы QGIS требуется, чтобы в таблице имелось
поле типа int4 с ограничением уникальности (которое включает
первичный ключ) или служебное поле oid.
</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="964"/>
        <source>The unique index on column</source>
        <translation>Уникальный индекс поля</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="966"/>
        <source>is unsuitable because Qgis does not currently support non-int4 type columns as a key into the table.
</source>
        <translation>непригоден, поскольку QGIS в настоящее время не поддерживает ключевые поля, типом которых не является int4.
</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="987"/>
        <source>and </source>
        <translation>и </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="993"/>
        <source>The unique index based on columns </source>
        <translation>Уникальный индекс, состоящий из полей </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="995"/>
        <source> is unsuitable because Qgis does not currently support multiple columns as a key into the table.
</source>
        <translation> непригоден, поскольку QGIS в настоящее время не поддерживает ключи из нескольких полей.
</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1037"/>
        <source>Unable to find a key column</source>
        <translation>Не удаётся найти ключевое поле</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1117"/>
        <source> derives from </source>
        <translation> производное от </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1121"/>
        <source>and is suitable.</source>
        <translation>пригодно для работы.</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1125"/>
        <source>and is not suitable </source>
        <translation>не является пригодным </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1126"/>
        <source>type is </source>
        <translation>тип поля </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1128"/>
        <source> and has a suitable constraint)</source>
        <translation> и есть подходящее ограничение)</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1130"/>
        <source> and does not have a suitable constraint)</source>
        <translation> и нет подходящего ограничения)</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1228"/>
        <source>The view you selected has the following columns, none of which satisfy the above conditions:</source>
        <translation>Вид, который вы выбрали включает следующие поля, ни одно из которых не соответствует вышеприведённым условиям:</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1234"/>
        <source>Qgis requires that the view has a column that can be used as a unique key. Such a column should be derived from a table column of type int4 and be a primary key, have a unique constraint on it, or be a PostgreSQL oid column. To improve performance the column should also be indexed.
</source>
        <translation>QGIS требует, чтобы вид включал поле, которое можно использовать как уникальный ключ. Такое поле должно происходить от типа int4 и быть первичным ключом с ограничением уникальности или являться служебным полем oid. Для повышения производительности поле также следует проиндексировать.
</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1235"/>
        <source>The view </source>
        <translation>Вид </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1236"/>
        <source>has no column suitable for use as a unique key.
</source>
        <translation>не имеет поля, подходящего в качестве уникального ключа.
</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1237"/>
        <source>No suitable key column in view</source>
        <translation>Подходящий ключ не найден в виде</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2553"/>
        <source>Unknown geometry type</source>
        <translation>Неизвестный тип геометрии</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2554"/>
        <source>Column </source>
        <translation>Поле </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2564"/>
        <source> in </source>
        <translation> в </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2556"/>
        <source> has a geometry type of </source>
        <translation> имеет тип геометрии </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2556"/>
        <source>, which Qgis does not currently support.</source>
        <translation>, который QGIS не поддерживает в данный момент.</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2565"/>
        <source>. The database communication log was:
</source>
        <translation>. История операций с базой данных:
</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2566"/>
        <source>Unable to get feature type and srid</source>
        <translation>Не удалось получить тип объекта и SRID</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1214"/>
        <source>Note: </source>
        <translation>Внимание: </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1216"/>
        <source>initially appeared suitable but does not contain unique data, so is not suitable.
</source>
        <translation>изначально определилось как пригодное, но оказалось непригодным, поскольку не содержит уникальных данных.
</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2563"/>
        <source>Qgis was unable to determine the type and srid of column </source>
        <translation>Не удалось определить тип и SRID поля </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="126"/>
        <source>Unable to determine table access privileges for the </source>
        <translation>Не удаётся определить привилегии доступа к таблице для </translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1888"/>
        <source>Error while adding features</source>
        <translation>Ошибка при добавлении объектов</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1928"/>
        <source>Error while deleting features</source>
        <translation>Ошибка при удалении объектов</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="1966"/>
        <source>Error while adding attributes</source>
        <translation>Ошибка при добавлении атрибутов</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2011"/>
        <source>Error while deleting attributes</source>
        <translation>Ошибка при удалении атрибутов</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2081"/>
        <source>Error while changing attributes</source>
        <translation>Ошибка при изменении атрибутов</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="2160"/>
        <source>Error while changing geometry values</source>
        <translation>Ошибка при изменении значений геометрии</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.h" line="482"/>
        <source>unexpected PostgreSQL error</source>
        <translation>неожиданная ошибка PostgreSQL</translation>
    </message>
</context>
<context>
    <name>QgsPostgresProvider::Conn</name>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="323"/>
        <source>No GEOS Support!</source>
        <translation>Поддержка GEOS не установлена!</translation>
    </message>
    <message>
        <location filename="../src/providers/postgres/qgspostgresprovider.cpp" line="327"/>
        <source>Your PostGIS installation has no GEOS support.
Feature selection and identification will not work properly.
Please install PostGIS with GEOS support (http://geos.refractions.net)</source>
        <translation>Ваша версия PostGIS не поддерживает GEOS.
Выбор и определение объектов будут работать неверно.
Пожалуйста, установите PostGIS с поддержкой GEOS (http://geos.refractions.net)</translation>
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
        <translation>Заглавие и цвета</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="44"/>
        <source>Project title</source>
        <translation>Заглавие проекта</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="61"/>
        <source>Selection color</source>
        <translation>Цвет выделения</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="100"/>
        <source>Background color</source>
        <translation>Цвет фона</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="139"/>
        <source>Map units</source>
        <translation>Единицы карты</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="282"/>
        <source>Coordinate Reference System (CRS)</source>
        <translation>Система координат</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectpropertiesbase.ui" line="300"/>
        <source>Enable &apos;on the fly&apos; CRS transformation</source>
        <translation>Включить преобразование координат «на лету»</translation>
    </message>
</context>
<context>
    <name>QgsProjectionSelector</name>
    <message>
        <location filename="../src/gui/qgsprojectionselector.cpp" line="491"/>
        <source>User Defined Coordinate Systems</source>
        <translation>Пользовательские системы координат</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsprojectionselector.cpp" line="568"/>
        <source>Geographic Coordinate Systems</source>
        <translation>Географические системы координат</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsprojectionselector.cpp" line="577"/>
        <source>Projected Coordinate Systems</source>
        <translation>Прямоугольные системы координат</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsprojectionselector.cpp" line="932"/>
        <source>Resource Location Error</source>
        <translation>Ошибка поиска ресурса</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsprojectionselector.cpp" line="935"/>
        <source>Error reading database file from: 
 %1
Because of this the projection selector will not work...</source>
        <translation>Ошибка чтения файла данных: 
 %1
Выбор проекции невозможен...</translation>
    </message>
</context>
<context>
    <name>QgsProjectionSelectorBase</name>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="111"/>
        <source>Search</source>
        <translation>Поиск</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="155"/>
        <source>Find</source>
        <translation>Найти</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="117"/>
        <source>EPSG ID</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="133"/>
        <source>Name</source>
        <translation>Имя</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="19"/>
        <source>Coordinate Reference System Selector</source>
        <translation>Выбор системы координат</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="53"/>
        <source>Coordinate Reference System</source>
        <translation>Система координат</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="58"/>
        <source>EPSG</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsprojectionselectorbase.ui" line="63"/>
        <source>ID</source>
        <translation></translation>
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
        <location filename="../src/gui/qgsquickprint.cpp" line="832"/>
        <source> km</source>
        <translation> км</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="837"/>
        <source> mm</source>
        <translation> мм</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="842"/>
        <source> cm</source>
        <translation> см</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="846"/>
        <source> m</source>
        <translation> м</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="851"/>
        <source> miles</source>
        <translation> миль</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="856"/>
        <source> mile</source>
        <translation> миля</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="861"/>
        <source> inches</source>
        <translation>дюймов</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="866"/>
        <source> foot</source>
        <translation> фут</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="870"/>
        <source> feet</source>
        <translation> футов</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="875"/>
        <source> degree</source>
        <translation> градус</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="877"/>
        <source> degrees</source>
        <translation> градусов</translation>
    </message>
    <message>
        <location filename="../src/gui/qgsquickprint.cpp" line="880"/>
        <source> unknown</source>
        <translation> неизв</translation>
    </message>
</context>
<context>
    <name>QgsRasterLayer</name>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="3344"/>
        <source>Not Set</source>
        <translation>Не задано</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2424"/>
        <source>Driver:</source>
        <translation>Драйвер:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2501"/>
        <source>Dimensions:</source>
        <translation>Размеры:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2504"/>
        <source>X: </source>
        <translation>X: </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2505"/>
        <source> Y: </source>
        <translation> Y: </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2505"/>
        <source> Bands: </source>
        <translation> Каналы: </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2620"/>
        <source>Origin:</source>
        <translation>Базис:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2629"/>
        <source>Pixel Size:</source>
        <translation>Размер пикселя:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2571"/>
        <source>Pyramid overviews:</source>
        <translation>Обзор пирамид:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2647"/>
        <source>Band</source>
        <translation>Канал</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2654"/>
        <source>Band No</source>
        <translation>Канал №</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2666"/>
        <source>No Stats</source>
        <translation>Нет статистики</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2669"/>
        <source>No stats collected yet</source>
        <translation>Сбор статистики не производился</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2679"/>
        <source>Min Val</source>
        <translation>Мин. значение</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2687"/>
        <source>Max Val</source>
        <translation>Макс. значение</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2695"/>
        <source>Range</source>
        <translation>Диапазон</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2703"/>
        <source>Mean</source>
        <translation>Среднее</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2711"/>
        <source>Sum of squares</source>
        <translation>Сумма квадратов</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2719"/>
        <source>Standard Deviation</source>
        <translation>Стандартное отклонение</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2727"/>
        <source>Sum of all cells</source>
        <translation>Сумма всех ячеек</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2735"/>
        <source>Cell Count</source>
        <translation>Количество ячеек</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2527"/>
        <source>Data Type:</source>
        <translation>Тип данных:</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2533"/>
        <source>GDT_Byte - Eight bit unsigned integer</source>
        <translation>GDT_Byte — 8-битное беззнаковое целое</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2536"/>
        <source>GDT_UInt16 - Sixteen bit unsigned integer </source>
        <translation>GDT_UInt16 — 16-битное беззнаковое целое </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2539"/>
        <source>GDT_Int16 - Sixteen bit signed integer </source>
        <translation>GDT_Int16 — 16-битное целое со знаком</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2542"/>
        <source>GDT_UInt32 - Thirty two bit unsigned integer </source>
        <translation>GDT_UInt32 — 32-битное беззнаковое целое </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2545"/>
        <source>GDT_Int32 - Thirty two bit signed integer </source>
        <translation>GDT_Int32 — 32-битное целое со знаком </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2548"/>
        <source>GDT_Float32 - Thirty two bit floating point </source>
        <translation>GDT_Float32 — 32-битное с плавающей точкой</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2551"/>
        <source>GDT_Float64 - Sixty four bit floating point </source>
        <translation>GDT_Float64 — 64-битное с плавающей точкой </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2554"/>
        <source>GDT_CInt16 - Complex Int16 </source>
        <translation>GDT_CInt16 — Комплексное Int16 </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2557"/>
        <source>GDT_CInt32 - Complex Int32 </source>
        <translation>GDT_CInt32 — Комплексное Int32 </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2560"/>
        <source>GDT_CFloat32 - Complex Float32 </source>
        <translation>GDT_CFloat32 — Комплексное Float32 </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2563"/>
        <source>GDT_CFloat64 - Complex Float64 </source>
        <translation>GDT_CFloat64 — Комплексное Float64 </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2566"/>
        <source>Could not determine raster data type.</source>
        <translation>Не удалось установить тип растровых данных.</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="1045"/>
        <source>Average Magphase</source>
        <translation>Среднее отношение магн/фаза</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="1050"/>
        <source>Average</source>
        <translation>Среднее</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2592"/>
        <source>Layer Spatial Reference System: </source>
        <translation>Система координат слоя: </translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="1792"/>
        <source>out of extent</source>
        <translation>за границами</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="1830"/>
        <source>null (no data)</source>
        <translation>null (нет данных)</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2449"/>
        <source>Dataset Description</source>
        <translation>Описание набора данных</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2512"/>
        <source>No Data Value</source>
        <translation>Значение «нет данных»</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="380"/>
        <source>and all other files</source>
        <translation>и прочие файлы</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2521"/>
        <source>NoDataValue not set</source>
        <translation>Значение «нет данных» не задано</translation>
    </message>
    <message>
        <location filename="../src/core/raster/qgsrasterlayer.cpp" line="2470"/>
        <source>Band %1</source>
        <translation>Канал %1</translation>
    </message>
</context>
<context>
    <name>QgsRasterLayerProperties</name>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1001"/>
        <source>Grayscale</source>
        <translation>Градации серого</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2373"/>
        <source>Pseudocolor</source>
        <translation>Псевдоцвет</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2373"/>
        <source>Freak Out</source>
        <translation>Кислотная</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="56"/>
        <source>Not Set</source>
        <translation>Не задано</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="810"/>
        <source>Columns: </source>
        <translation>Столбцов: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="811"/>
        <source>Rows: </source>
        <translation>Строк: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="812"/>
        <source>No-Data Value: </source>
        <translation>Значение «нет данных»: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="812"/>
        <source>n/a</source>
        <translation>н/д</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2734"/>
        <source>Write access denied</source>
        <translation>Закрыт доступ на запись</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2734"/>
        <source>Write access denied. Adjust the file permissions and try again.

</source>
        <translation>Закрыт доступ на запись. Исправьте права доступа к файлу и попробуйте ещё раз.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1597"/>
        <source>Building pyramids failed.</source>
        <translation>Не удалось построить пирамиды.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1598"/>
        <source>Building pyramid overviews is not supported on this type of raster.</source>
        <translation>Построение пирамид не поддерживается для данного типа растра.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2912"/>
        <source>No Stretch</source>
        <translation>Без растяжения</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2917"/>
        <source>Stretch To MinMax</source>
        <translation>Растяжение до мин/макс</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2922"/>
        <source>Stretch And Clip To MinMax</source>
        <translation>Растяжение и отсечение по мин/макс</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2927"/>
        <source>Clip To MinMax</source>
        <translation>Отсечение по мин/макс</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2791"/>
        <source>Discrete</source>
        <translation>Дискретная</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2583"/>
        <source>Equal interval</source>
        <translation>Равные интервалы</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2606"/>
        <source>Quantiles</source>
        <translation>Квантили</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="306"/>
        <source>Description</source>
        <translation>Описание</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="307"/>
        <source>Large resolution raster layers can slow navigation in QGIS.</source>
        <translation>Растры высокого разрешения могут замедлить навигацию в QGIS.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="308"/>
        <source>By creating lower resolution copies of the data (pyramids) performance can be considerably improved as QGIS selects the most suitable resolution to use depending on the level of zoom.</source>
        <translation>Создание копий данных низкого разрешения (пирамид) позволяет существенно повысить скорость, поскольку QGIS будет автоматически выбирать оптимальное разрешение в зависимости от текущего масштаба.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="309"/>
        <source>You must have write access in the directory where the original data is stored to build pyramids.</source>
        <translation>Для сохранения пирамид необходимы права на запись в каталог, в котором хранятся оригинальные данные.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1826"/>
        <source>Red</source>
        <translation>Красный</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1826"/>
        <source>Green</source>
        <translation>Зелёный</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1826"/>
        <source>Blue</source>
        <translation>Синий</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1843"/>
        <source>Percent Transparent</source>
        <translation>Процент прозрачности</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1839"/>
        <source>Gray</source>
        <translation>Серый</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1843"/>
        <source>Indexed Value</source>
        <translation>Индексированное значение</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2910"/>
        <source>User Defined</source>
        <translation>Пользовательское</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="804"/>
        <source>No-Data Value: Not Set</source>
        <translation>Значение «нет данных»: не задано</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2680"/>
        <source>Save file</source>
        <translation>Сохранить файл</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2760"/>
        <source>Textfile (*.txt)</source>
        <translation>Текстовые файлы (*.txt)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1823"/>
        <source>QGIS Generated Transparent Pixel Value Export File</source>
        <translation>Файл экспорта значений прозрачности пикселей, созданный QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2760"/>
        <source>Open file</source>
        <translation>Открыть файл</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2828"/>
        <source>Import Error</source>
        <translation>Ошибка импорта</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2828"/>
        <source>The following lines contained errors

</source>
        <translation>Следующие строки содержат ошибки

</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2833"/>
        <source>Read access denied</source>
        <translation>Закрыт доступ на чтение</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2833"/>
        <source>Read access denied. Adjust the file permissions and try again.

</source>
        <translation>Закрыт доступ на чтение. Исправьте права доступа к файлу и попробуйте ещё раз.

</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2373"/>
        <source>Color Ramp</source>
        <translation>Градиент</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="3076"/>
        <source>Default Style</source>
        <translation>Стиль по умолчанию</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="3180"/>
        <source>QGIS Layer Style File (*.qml)</source>
        <translation>Файл стиля QGIS (*.qml)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="3206"/>
        <source>QGIS</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="3207"/>
        <source>Unknown style format: </source>
        <translation>Неизвестный формат стиля: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="310"/>
        <source>Please note that building internal pyramids may alter the original data file and once created they cannot be removed!</source>
        <translation>Обратите внимание, что операция построения встроенных пирамид может изменить оригинальный файл данных и их невозможно будет удалить после создания!</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="311"/>
        <source>Please note that building internal pyramids could corrupt your image - always make a backup of your data first!</source>
        <translation>Помните, что при построении пирамид ваши изображения могут быть повреждены — всегда создавайте резервные копии данных перед этой операцией!</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2905"/>
        <source>Default</source>
        <translation>По умолчанию</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1583"/>
        <source>The file was not writeable. Some formats do not support pyramid overviews. Consult the GDAL documentation if in doubt.</source>
        <translation>Запись в файл невозможна. Некоторые форматы не поддерживают обзорные пирамиды. Обратитесь к документации GDAL за дополнительной информацией.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="3199"/>
        <source>Saved Style</source>
        <translation>Сохранённый стиль</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1719"/>
        <source>Colormap</source>
        <translation>Цветовая карта</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2787"/>
        <source>Linear</source>
        <translation>Линейная</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2795"/>
        <source>Exact</source>
        <translation>Точная</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2675"/>
        <source>Custom color map entry</source>
        <translation>Пользовательское значение цветовой карты</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2692"/>
        <source>QGIS Generated Color Map Export File</source>
        <translation>Файл экспорта цветовой карты QGIS</translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2750"/>
        <source>Load Color Map</source>
        <translation>Загрузка цветовой карты</translation>
    </message>
    <message numerus="yes">
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="2750"/>
        <source>The color map for Band %n failed to load</source>
        <translation>
            <numerusform>Не удалось загрузить цветовую карту для канала %n</numerusform>
            <numerusform>Не удалось загрузить цветовую карту для канала %n</numerusform>
            <numerusform>Не удалось загрузить цветовую карту для канала %n</numerusform>
        </translation>
    </message>
    <message>
        <location filename="../src/app/qgsrasterlayerproperties.cpp" line="1593"/>
        <source>Building internal pyramid overviews is not supported on raster layers with JPEG compression.</source>
        <translation>Построение встроенных обзорных пирамид не поддерживается для растровых слоёв с JPEG-сжатием.</translation>
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
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1327"/>
        <source>General</source>
        <translation>Общие</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1397"/>
        <source>No Data:</source>
        <translation>Нет данных:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="34"/>
        <source>Symbology</source>
        <translation>Символика</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="930"/>
        <source>&lt;p align=&quot;right&quot;&gt;Full&lt;/p&gt;</source>
        <translation>Полная</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="884"/>
        <source>None</source>
        <translation>Нулевая</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1631"/>
        <source>Metadata</source>
        <translation>Метаданные</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1644"/>
        <source>Pyramids</source>
        <translation>Пирамиды</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1718"/>
        <source>Average</source>
        <translation>Среднее значение</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1723"/>
        <source>Nearest Neighbour</source>
        <translation>Ближайший сосед</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1512"/>
        <source>Thumbnail</source>
        <translation>Образец</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1371"/>
        <source>Columns:</source>
        <translation>Столбцы:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1384"/>
        <source>Rows:</source>
        <translation>Строки:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1418"/>
        <source>Maximum scale at which this layer will be displayed. </source>
        <translation>Максимальный масштаб, при котором виден данный слой. </translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1441"/>
        <source>Minimum scale at which this layer will be displayed. </source>
        <translation>Минимальный масштаб, при котором виден данный слой. </translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1746"/>
        <source>Histogram</source>
        <translation>Гистограмма</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1812"/>
        <source>Options</source>
        <translation>Параметры</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1783"/>
        <source>Chart Type</source>
        <translation>Тип диаграммы</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1894"/>
        <source>Refresh</source>
        <translation>Обновить</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="635"/>
        <source>Max</source>
        <translation>Макс</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="597"/>
        <source>Min</source>
        <translation>Мин</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="910"/>
        <source> 00%</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="40"/>
        <source>Render as</source>
        <translation>Отображать как</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1247"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1134"/>
        <source>Colormap</source>
        <translation>Цветовая карта</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1170"/>
        <source>Delete entry</source>
        <translation>Удалить значение</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1316"/>
        <source>Classify</source>
        <translation>Классифицировать</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1267"/>
        <source>1</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1272"/>
        <source>2</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="46"/>
        <source>Single band gray</source>
        <translation>Одноканальное серое</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="66"/>
        <source>Three band color</source>
        <translation>Трёхканальное цветное</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="118"/>
        <source>RGB mode band selection and scaling</source>
        <translation>Выбор каналов RGB и растяжения</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="124"/>
        <source>Red band</source>
        <translation>Красный канал</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="156"/>
        <source>Green band</source>
        <translation>Зелёный канал</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="188"/>
        <source>Blue band</source>
        <translation>Синий канал</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="584"/>
        <source>Custom min / max values</source>
        <translation>Пользовательские значения мин/макс</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="255"/>
        <source>Red min</source>
        <translation>Мин. красный</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="306"/>
        <source>Red max</source>
        <translation>Макс. красный</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="344"/>
        <source>Green min</source>
        <translation>Мин. зелёный</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="382"/>
        <source>Green max</source>
        <translation>Макс. зелёный</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="420"/>
        <source>Blue min</source>
        <translation>Мин. синий</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="458"/>
        <source>Blue max</source>
        <translation>Макс. синий</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="526"/>
        <source>Single band properties</source>
        <translation>Свойства канала</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="538"/>
        <source>Gray band</source>
        <translation>Канал серого</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="561"/>
        <source>Color map</source>
        <translation>Цветовая карта</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="92"/>
        <source>Invert color map</source>
        <translation>Обратить цветовую карту</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="667"/>
        <source>Use standard deviation</source>
        <translation>Использовать стандартное отклонение</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="707"/>
        <source>Note:</source>
        <translation>Внимание:</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="721"/>
        <source>Load min / max values from band</source>
        <translation>Загрузить мин./макс. значения канала</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="727"/>
        <source>Estimate (faster)</source>
        <translation>Расчётные (быстрее)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="747"/>
        <source>Actual (slower)</source>
        <translation>Фактические (медленнее)</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="767"/>
        <source>Load</source>
        <translation>Загрузить</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="777"/>
        <source>Contrast enhancement</source>
        <translation>Улучшение контраста</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="795"/>
        <source>Current</source>
        <translation>Текущее</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="818"/>
        <source>Save current contrast enhancement algorithm as default. This setting will be persistent between QGIS sessions.</source>
        <translation>Сохранить текущий алгоритм улучшения контраста по умолчанию. Этот параметр будет сохраняться между сеансами работы QGIS.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="821"/>
        <source>Saves current contrast enhancement algorithm as a default. This setting will be persistent between QGIS sessions.</source>
        <translation>Сохранить текущий алгоритм улучшения контраста по умолчанию. Этот параметр будет сохраняться между сеансами работы QGIS.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="835"/>
        <source>Default</source>
        <translation>По умолчанию</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="842"/>
        <source>TextLabel</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="853"/>
        <source>Transparency</source>
        <translation>Прозрачность</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="859"/>
        <source>Global transparency</source>
        <translation>Общая прозрачность</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="940"/>
        <source>No data value</source>
        <translation>Значение «нет данных»</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="949"/>
        <source>Reset no data value</source>
        <translation>Сбросить значение «нет данных»</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="959"/>
        <source>Custom transparency options</source>
        <translation>Параметры прозрачности</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="965"/>
        <source>Transparency band</source>
        <translation>Канал прозрачности</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="991"/>
        <source>Transparent pixel list</source>
        <translation>Перечень прозрачных пикселей</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1028"/>
        <source>Add values manually</source>
        <translation>Добавить значения вручную</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1045"/>
        <source>Add Values from display</source>
        <translation>Добавить значения с экрана</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1059"/>
        <source>Remove selected row</source>
        <translation>Удалить выбранную строку</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1073"/>
        <source>Default values</source>
        <translation>Значения по умолчанию</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1100"/>
        <source>Import from file</source>
        <translation>Импорт из файла</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1114"/>
        <source>Export to file</source>
        <translation>Экспорт в файл</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1286"/>
        <source>Number of entries</source>
        <translation>Количество значений</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1140"/>
        <source>Color interpolation</source>
        <translation>Интерполяция цветов</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1306"/>
        <source>Classification mode</source>
        <translation>Режим классификации</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1406"/>
        <source>Scale dependent visibility</source>
        <translation>Видимость в пределах масштаба</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1431"/>
        <source>Maximum</source>
        <translation>Максимальный</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1454"/>
        <source>Minimum</source>
        <translation>Минимальный</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1346"/>
        <source>Layer source</source>
        <translation>Источник слоя</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1333"/>
        <source>Display name</source>
        <translation>Имя в легенде</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1657"/>
        <source>Pyramid resolutions</source>
        <translation>Разрешения пирамид</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1707"/>
        <source>Resampling method</source>
        <translation>Метод интерполяции</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1738"/>
        <source>Build pyramids</source>
        <translation>Построить пирамиды</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1792"/>
        <source>Line graph</source>
        <translation>Линейная</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1802"/>
        <source>Bar chart</source>
        <translation>Столбчатая</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1818"/>
        <source>Column count</source>
        <translation>Количество столбцов</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1841"/>
        <source>Out of range OK?</source>
        <translation>Разрешить значения вне диапазона?</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1848"/>
        <source>Allow approximation</source>
        <translation>Разрешить аппроксимацию</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1905"/>
        <source>Restore Default Style</source>
        <translation>Восстановить по умолчанию</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1912"/>
        <source>Save As Default</source>
        <translation>Сохранить по умолчанию</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1919"/>
        <source>Load Style ...</source>
        <translation>Загрузить стиль...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1926"/>
        <source>Save Style ...</source>
        <translation>Сохранить стиль...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="233"/>
        <source>Default R:1 G:2 B:3</source>
        <translation>По умолчанию R:1 G:2 B:3</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1467"/>
        <source>Coordinate reference system</source>
        <translation>Система координат</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1650"/>
        <source>Notes</source>
        <translation>Замечания</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1700"/>
        <source>Build pyramids internally if possible</source>
        <translation>Создавать встроенные пирамиды, если возможно</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1163"/>
        <source>Add entry</source>
        <translation>Добавить значение</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1177"/>
        <source>Sort</source>
        <translation>Сортировать</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1200"/>
        <source>Load color map from band</source>
        <translation>Загрузить цветовую карту из канала</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1230"/>
        <source>Load color map from file</source>
        <translation>Загрузить цветовую карту из файла</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1244"/>
        <source>Export color map to file</source>
        <translation>Сохранить цветовую карту в файл</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1280"/>
        <source>Generate new color map</source>
        <translation>Создать новую цветовую карту</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1476"/>
        <source>Change ...</source>
        <translation>Изменить...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1549"/>
        <source>Legend</source>
        <translation>Легенда</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1586"/>
        <source>Palette</source>
        <translation>Палитра</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsrasterlayerpropertiesbase.ui" line="1670"/>
        <source>&lt;!DOCTYPE HTML PUBLIC &quot;-//W3C//DTD HTML 4.0//EN&quot; &quot;http://www.w3.org/TR/REC-html40/strict.dtd&quot;&gt;
&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;DejaVu Sans&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;;&quot;&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>QgsRunProcess</name>
    <message>
        <location filename="../src/core/qgsrunprocess.cpp" line="150"/>
        <source>Unable to run command</source>
        <translation>Не удалось выполнить команду</translation>
    </message>
    <message>
        <location filename="../src/core/qgsrunprocess.cpp" line="57"/>
        <source>Starting</source>
        <translation>Выполняется</translation>
    </message>
    <message>
        <location filename="../src/core/qgsrunprocess.cpp" line="119"/>
        <source>Done</source>
        <translation>Выполнено</translation>
    </message>
    <message>
        <location filename="../src/core/qgsrunprocess.cpp" line="72"/>
        <source>Action</source>
        <translation>Действие</translation>
    </message>
</context>
<context>
    <name>QgsScaleBarPlugin</name>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="160"/>
        <source> metres/km</source>
        <translation> метров/км</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="278"/>
        <source> feet</source>
        <translation> футов</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="285"/>
        <source> degrees</source>
        <translation> градусов</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="240"/>
        <source> km</source>
        <translation> км</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="245"/>
        <source> mm</source>
        <translation> мм</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="250"/>
        <source> cm</source>
        <translation> см</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="254"/>
        <source> m</source>
        <translation> м</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="274"/>
        <source> foot</source>
        <translation> фут</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="283"/>
        <source> degree</source>
        <translation> градус</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="288"/>
        <source> unknown</source>
        <translation> неизв</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="77"/>
        <source>Top Left</source>
        <translation>Вверху слева</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="76"/>
        <source>Bottom Left</source>
        <translation>Внизу слева</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="77"/>
        <source>Top Right</source>
        <translation>Вверху справа</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="77"/>
        <source>Bottom Right</source>
        <translation>Внизу справа</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="79"/>
        <source>Tick Down</source>
        <translation>Штрих вниз</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="80"/>
        <source>Tick Up</source>
        <translation>Штрих вверх</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="80"/>
        <source>Bar</source>
        <translation>Линия</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="80"/>
        <source>Box</source>
        <translation>Рамка</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="100"/>
        <source>&amp;Scale Bar</source>
        <translation>&amp;Масштабная линейка</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="101"/>
        <source>Creates a scale bar that is displayed on the map canvas</source>
        <translation>Создаёт масштабную линейку в области отображения карты</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="539"/>
        <source>&amp;Decorations</source>
        <translation>&amp;Оформление</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="161"/>
        <source> feet/miles</source>
        <translation> футов/миль</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="259"/>
        <source> miles</source>
        <translation> миль</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="264"/>
        <source> mile</source>
        <translation> миля</translation>
    </message>
    <message>
        <location filename="../src/plugins/scale_bar/plugin.cpp" line="269"/>
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
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="167"/>
        <source>Found %d matching features.</source>
        <translation>
            <numerusform>Найден %d подходящий объект.</numerusform>
            <numerusform>Найдено %d подходящих объекта.</numerusform>
            <numerusform>Найдено %d подходящих объектов.</numerusform>
        </translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="169"/>
        <source>No matching features found.</source>
        <translation>Подходящих объектов не найдено.</translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="170"/>
        <source>Search results</source>
        <translation>Результаты поиска</translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="179"/>
        <source>Search string parsing error</source>
        <translation>Ошибка обработки строки запроса</translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="235"/>
        <source>No Records</source>
        <translation>Нет записей</translation>
    </message>
    <message>
        <location filename="../src/app/qgssearchquerybuilder.cpp" line="235"/>
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
        <location filename="../src/app/qgsserversourceselect.cpp" line="172"/>
        <source>Are you sure you want to remove the </source>
        <translation>Вы уверены, что хотите удалить соединение </translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="172"/>
        <source> connection and all associated settings?</source>
        <translation> и все связанные с ним параметры?</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="173"/>
        <source>Confirm Delete</source>
        <translation>Потвердите удаление</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="352"/>
        <source>WMS Provider</source>
        <translation>WMS-источник</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="354"/>
        <source>Could not open the WMS Provider</source>
        <translation>Не удалось открыть WMS-источник</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="363"/>
        <source>Select Layer</source>
        <translation>Выберите слой</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="363"/>
        <source>You must select at least one layer first.</source>
        <translation>Для добавления следует выбрать хотя бы один слой.</translation>
    </message>
    <message numerus="yes">
        <location filename="../src/app/qgsserversourceselect.cpp" line="475"/>
        <source>Coordinate Reference System (%1 available)</source>
        <translation>
            <numerusform>Система координат (доступна %1)</numerusform>
            <numerusform>Система координат (доступно %1)</numerusform>
            <numerusform>Система координат (доступно %1)</numerusform>
        </translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="634"/>
        <source>Could not understand the response.  The</source>
        <translation>Ошибка обработки ответа. Источник</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="635"/>
        <source>provider said</source>
        <translation>сообщил</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="686"/>
        <source>WMS proxies</source>
        <translation>WMS-прокси</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="367"/>
        <source>Coordinate Reference System</source>
        <translation>Система координат</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="367"/>
        <source>There are no available coordinate reference system for the set of layers you&apos;ve selected.</source>
        <translation>Для выбранных слоёв не найдено доступных систем координат.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsserversourceselect.cpp" line="689"/>
        <source>Several WMS servers have been added to the server list. Note that if you access the internet via a web proxy, you will need to set the proxy settings in the QGIS options dialog.</source>
        <translation>Несколько WMS-серверов было добавлено в список. Обратите внимание, что если вы выходите в интернет через прокси-сервер, его необходимо указать в диалоге настроек QGIS.</translation>
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
        <translation>&amp;Создать</translation>
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
        <location filename="../src/plugins/spit/qgsshapefile.cpp" line="448"/>
        <source>The database gave an error while executing this SQL:</source>
        <translation>База данных вернула ошибку при выполнении SQL:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsshapefile.cpp" line="456"/>
        <source>The error was:</source>
        <translation>Сообщение об ошибке:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsshapefile.cpp" line="453"/>
        <source>... (rest of SQL trimmed)</source>
        <comment>is appended to a truncated SQL statement</comment>
        <translation>...(остаток SQL проигнорирован)</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsshapefile.cpp" line="89"/>
        <source>Scanning </source>
        <translation>Сканирование </translation>
    </message>
</context>
<context>
    <name>QgsSingleSymbolDialog</name>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="114"/>
        <source>Solid Line</source>
        <translation>Сплошная линия</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="115"/>
        <source>Dash Line</source>
        <translation>Штриховой пунктир</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="116"/>
        <source>Dot Line</source>
        <translation>Точечный пунктир</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="117"/>
        <source>Dash Dot Line</source>
        <translation>Штрихпунктир</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="118"/>
        <source>Dash Dot Dot Line</source>
        <translation>Штрихпунктир с двумя точками</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="119"/>
        <source>No Pen</source>
        <translation>Без линии</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="138"/>
        <source>No Brush</source>
        <translation>Без заливки</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="124"/>
        <source>Solid</source>
        <translation>Сплошная</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="125"/>
        <source>Horizontal</source>
        <translation>Горизонтальный шаблон</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="126"/>
        <source>Vertical</source>
        <translation>Вертикальный шаблон</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="127"/>
        <source>Cross</source>
        <translation>Перекрестие</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="128"/>
        <source>BDiagonal</source>
        <translation>Обратная диагональная</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="129"/>
        <source>FDiagonal</source>
        <translation>Прямая диагональная</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="130"/>
        <source>Diagonal X</source>
        <translation>Перекрестие по диагонали</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="131"/>
        <source>Dense1</source>
        <translation>Штриховка 1</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="132"/>
        <source>Dense2</source>
        <translation>Штриховка 2</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="133"/>
        <source>Dense3</source>
        <translation>Штриховка 3</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="134"/>
        <source>Dense4</source>
        <translation>Штриховка 4</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="135"/>
        <source>Dense5</source>
        <translation>Штриховка 5</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="136"/>
        <source>Dense6</source>
        <translation>Штриховка 6</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="137"/>
        <source>Dense7</source>
        <translation>Штриховка 7</translation>
    </message>
    <message>
        <location filename="../src/app/qgssinglesymboldialog.cpp" line="139"/>
        <source>Texture</source>
        <translation>Текстурой</translation>
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
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="130"/>
        <source>Size</source>
        <translation>Размер</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="76"/>
        <source>Point Symbol</source>
        <translation>Значок</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="120"/>
        <source>Area scale field</source>
        <translation>Поле масштаба</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="110"/>
        <source>Rotation field</source>
        <translation>Поле вращения</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="176"/>
        <source>Style Options</source>
        <translation>Параметры стиля</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="338"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="182"/>
        <source>Outline style</source>
        <translation>Стиль линии</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="214"/>
        <source>Outline color</source>
        <translation>Цвет линии</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="249"/>
        <source>Outline width</source>
        <translation>Ширина линии</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="275"/>
        <source>Fill color</source>
        <translation>Цвет заливки</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="310"/>
        <source>Fill style</source>
        <translation>Стиль заливки</translation>
    </message>
    <message>
        <location filename="../src/ui/qgssinglesymboldialogbase.ui" line="48"/>
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
        <translation>[Создать...] — создать новое соединение</translation>
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
        <translation>- для успешного импорта необходимо выбрать действительное (рабочее) соединение</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="362"/>
        <source>-when changing connections Global Schema also changes accordingly</source>
        <translation>- при изменении соединения общая схема также изменяется</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="363"/>
        <source>Shapefile List:</source>
        <translation>Список shape-файлов:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="365"/>
        <source>[Add ...] - open a File dialog and browse to the desired file(s) to import</source>
        <translation>[Добавить...]  —  выбрать файл(ы) для импорта в диалоге открытия файлов</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="366"/>
        <source>[Remove] - remove the currently selected file(s) from the list</source>
        <translation>[Удалить] — удалить выбранные файлы из списка</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="367"/>
        <source>[Remove All] - remove all the files in the list</source>
        <translation>[Удалить все] — удалить все файлы из списка</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="368"/>
        <source>[SRID] - Reference ID for the shapefiles to be imported</source>
        <translation>[SRID] — ID системы координат для загружаемых shape-файлов</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="369"/>
        <source>[Use Default (SRID)] - set SRID to -1</source>
        <translation>[SRID по умолчанию] — использовать значение -1 для SRID</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="370"/>
        <source>[Geometry Column Name] - name of the geometry column in the database</source>
        <translation>[Имя поля геометрии] — имя поля геометрии в базе данных</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="371"/>
        <source>[Use Default (Geometry Column Name)] - set column name to &apos;the_geom&apos;</source>
        <translation>[Поле геометрии по умолчанию] — использовать значение «the_geom» для поля геометрии</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="372"/>
        <source>[Glogal Schema] - set the schema for all files to be imported into</source>
        <translation>[Общая схема] — схема, в которую будут загружены все указанные файлы</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="374"/>
        <source>[Import] - import the current shapefiles in the list</source>
        <translation>[Импорт] — импортировать файлы, указанные в списке</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="375"/>
        <source>[Quit] - quit the program
</source>
        <translation>[Выйти] — выйти из программы
</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="376"/>
        <source>[Help] - display this help dialog</source>
        <translation>[Справка] — вывести этот диалог справки</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="835"/>
        <source>Import Shapefiles</source>
        <translation>Импорт shape-файлов</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="835"/>
        <source>You need to specify a Connection first</source>
        <translation>Необходимо указать соединение</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="428"/>
        <source>Connection failed - Check settings and try again</source>
        <translation>Не удалось соединиться — проверьте параметры и попробуйте ещё раз </translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="514"/>
        <source>You need to add shapefiles to the list first</source>
        <translation>Необходимо добавить shape-файлы в список</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="581"/>
        <source>Importing files</source>
        <translation>Импорт файлов</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="519"/>
        <source>Cancel</source>
        <translation>Отменить</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="523"/>
        <source>Progress</source>
        <translation>Прогресс</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="532"/>
        <source>Problem inserting features from file:</source>
        <translation>Проблема при вставке объектов из файла:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="539"/>
        <source>Invalid table name.</source>
        <translation>Неверное имя таблицы.</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="549"/>
        <source>No fields detected.</source>
        <translation>Поля не выбраны.</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="574"/>
        <source>The following fields are duplicates:</source>
        <translation>Следующие поля являются дубликатами:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="674"/>
        <source>Import Shapefiles - Relation Exists</source>
        <translation>Импорт shape-файлов — реляция существует</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="675"/>
        <source>The Shapefile:</source>
        <translation>Shape-файл:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="676"/>
        <source>will use [</source>
        <translation>будет загружен в реляцию [</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="676"/>
        <source>] relation for its data,</source>
        <translation>],</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="676"/>
        <source>which already exists and possibly contains data.</source>
        <translation>которая уже существует и возможно содержит данные.</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="677"/>
        <source>To avoid data loss change the &quot;DB Relation Name&quot;</source>
        <translation>Во избежание потери данных, измените «Имя реляции БД»</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="677"/>
        <source>for this Shapefile in the main dialog file list.</source>
        <translation>для этого shape-файла в списке файлов главного диалога.</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="678"/>
        <source>Do you want to overwrite the [</source>
        <translation>Вы хотите перезаписать реляцию [</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="678"/>
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
        <location filename="../src/plugins/spit/qgsspit.cpp" line="468"/>
        <source>PostGIS not available</source>
        <translation>PostGIS недоступна</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="470"/>
        <source>&lt;p&gt;The chosen database does not have PostGIS installed, but this is required for storage of spatial data.&lt;/p&gt;</source>
        <translation>&lt;p&gt;PostGIS не установлен в выбранной БД, что делает невозможным хранение пространственных данных.&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="816"/>
        <source>&lt;p&gt;Error while executing the SQL:&lt;/p&gt;&lt;p&gt;</source>
        <translation>&lt;p&gt;Ошибка при выполнении SQL:&lt;/p&gt;&lt;p&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="817"/>
        <source>&lt;/p&gt;&lt;p&gt;The database said:</source>
        <translation>&lt;/p&gt;&lt;p&gt;Сообщение БД:</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="831"/>
        <source>%1 of %2 shapefiles could not be imported.</source>
        <translation>Не удалось загрузить %1 из %2 shape-файлов.</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="408"/>
        <source>Password for </source>
        <translation>Пароль для </translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspit.cpp" line="409"/>
        <source>Please enter your password:</source>
        <translation>Пожалуйста, введите ваш пароль:</translation>
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
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="224"/>
        <source>Set the SRID to the default value</source>
        <translation>Заполнить SRID значением по умолчанию</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="237"/>
        <source>Set the geometry column name to the default value</source>
        <translation>Задать имя поля геометрии в соответствии со значением по умолчанию</translation>
    </message>
    <message>
        <location filename="../src/plugins/spit/qgsspitbase.ui" line="121"/>
        <source>New</source>
        <translation>Создать</translation>
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
        <translation>Подключиться</translation>
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
        <translation>Подключиться к PostGIS</translation>
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
        <translation>Линейная интерполяция</translation>
    </message>
</context>
<context>
    <name>QgsTINInterpolatorDialogBase</name>
    <message>
        <location filename="../src/plugins/interpolation/qgstininterpolatordialogbase.ui" line="13"/>
        <source>Triangle based interpolation</source>
        <translation>Триангуляция</translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgstininterpolatordialogbase.ui" line="19"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:12pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;This interpolator provides different methods for interpolation in a triangular irregular network (TIN).&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:12pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Этот интерполятор предоставляет различные методы для интерполяции в нерегулярной триангулированной сети (TIN).&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/interpolation/qgstininterpolatordialogbase.ui" line="31"/>
        <source>Interpolation method:</source>
        <translation>Метод интерполяции:</translation>
    </message>
</context>
<context>
    <name>QgsUniqueValueDialog</name>
    <message>
        <location filename="../src/app/qgsuniquevaluedialog.cpp" line="282"/>
        <source>Confirm Delete</source>
        <translation>Потвердите удаление</translation>
    </message>
    <message>
        <location filename="../src/app/qgsuniquevaluedialog.cpp" line="285"/>
        <source>The classification field was changed from &apos;%1&apos; to &apos;%2&apos;.
Should the existing classes be deleted before classification?</source>
        <translation>Поле классификации было изменено с «%1» на «%2».
Следует ли удалить существующие классы перед классификацией?</translation>
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
        <location filename="../src/ui/qgsuniquevaluedialogbase.ui" line="93"/>
        <source>Classify</source>
        <translation>Классифицировать</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsuniquevaluedialogbase.ui" line="49"/>
        <source>Classification field</source>
        <translation>Поле классификации</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsuniquevaluedialogbase.ui" line="100"/>
        <source>Add class</source>
        <translation>Добавить класс</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsuniquevaluedialogbase.ui" line="107"/>
        <source>Delete classes</source>
        <translation>Удалить классы</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsuniquevaluedialogbase.ui" line="114"/>
        <source>Randomize Colors</source>
        <translation>Перемешать цвета</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsuniquevaluedialogbase.ui" line="121"/>
        <source>Reset Colors</source>
        <translation>Сбросить цвета</translation>
    </message>
</context>
<context>
    <name>QgsVectorLayer</name>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2600"/>
        <source>ERROR: no provider</source>
        <translation>ОШИБКА: отсутствует источник</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2606"/>
        <source>ERROR: layer not editable</source>
        <translation>ОШИБКА: нередактируемый слой</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2624"/>
        <source>SUCCESS: %1 attributes added.</source>
        <translation>УСПЕХ: добавлено %1 атрибутов.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2630"/>
        <source>ERROR: %1 new attributes not added</source>
        <translation>ОШИБКА: не добавлено %1 новых атрибутов</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2642"/>
        <source>SUCCESS: %1 attributes deleted.</source>
        <translation>УСПЕХ: удалено %1 атрибутов.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2648"/>
        <source>ERROR: %1 attributes not deleted.</source>
        <translation>ОШИБКА: %1 атрибутов не было удалено.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2689"/>
        <source>SUCCESS: attribute %1 was added.</source>
        <translation>УСПЕХ: добавлен атрибут %1.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2696"/>
        <source>ERROR: attribute %1 not added</source>
        <translation>ОШИБКА: атрибут %1 не был добавлен</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2755"/>
        <source>SUCCESS: %1 attribute values changed.</source>
        <translation>УСПЕХ: изменено %1 значений атрибутов.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2760"/>
        <source>ERROR: %1 attribute value changes not applied.</source>
        <translation>ОШИБКА: не применено %1 изменений значений атрибутов.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2792"/>
        <source>SUCCESS: %1 features added.</source>
        <translation>УСПЕХ: добавлено %1 объектов.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2797"/>
        <source>ERROR: %1 features not added.</source>
        <translation>ОШИБКА: не добавлено %1 объектов.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2810"/>
        <source>SUCCESS: %1 geometries were changed.</source>
        <translation>УСПЕХ: изменено %1 геометрий.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2815"/>
        <source>ERROR: %1 geometries not changed.</source>
        <translation>ОШИБКА: не изменено %1 геометрий.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2827"/>
        <source>SUCCESS: %1 features deleted.</source>
        <translation>УСПЕХ: удалено %1 объектов.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2837"/>
        <source>ERROR: %1 features not deleted.</source>
        <translation>ОШИБКА: не удалено %1 объектов.</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2317"/>
        <source>No renderer object</source>
        <translation>Отсутствует объект отрисовки</translation>
    </message>
    <message>
        <location filename="../src/core/qgsvectorlayer.cpp" line="2321"/>
        <source>Classification field not found</source>
        <translation>Поле классификации не найдено</translation>
    </message>
</context>
<context>
    <name>QgsVectorLayerProperties</name>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="304"/>
        <source>Transparency: </source>
        <translation>Прозрачность: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="395"/>
        <source>Single Symbol</source>
        <translation>Обычный знак</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="398"/>
        <source>Graduated Symbol</source>
        <translation>Градуированный знак</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="399"/>
        <source>Continuous Color</source>
        <translation>Непрерывный цвет</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="400"/>
        <source>Unique Value</source>
        <translation>Уникальное значение</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="355"/>
        <source>This button opens the PostgreSQL query builder and allows you to create a subset of features to display on the map canvas rather than displaying all features in the layer</source>
        <translation>Эта кнопка открывает конструктор запросов PostgreSQL, при помощи которого можно выбрать подмножество объектов для отображения на карте, иначе все объекты будут видимы</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="358"/>
        <source>The query used to limit the features in the layer is shown here. This is currently only supported for PostgreSQL layers. To enter or modify the query, click on the Query Builder button</source>
        <translation>Этот запрос используется для ограничения доступа к объектам слоя. В данный момент поддерживаются только слои PostgreSQL. Для создания или изменения запроса кликните на кнопке «Конструктор запросов»</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="656"/>
        <source>Spatial Index</source>
        <translation>Пространственный индекс</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="656"/>
        <source>Creation of spatial index failed</source>
        <translation>Не удалось создать пространственный индекс</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="669"/>
        <source>General:</source>
        <translation>Общее:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="684"/>
        <source>Storage type of this layer : </source>
        <translation>Тип хранилища этого слоя: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="690"/>
        <source>Source for this layer : </source>
        <translation>Источник этого слоя: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="707"/>
        <source>Geometry type of the features in this layer : </source>
        <translation>Тип геометрии объектов в этом слое: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="715"/>
        <source>The number of features in this layer : </source>
        <translation>Количество объектов в слое: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="720"/>
        <source>Editing capabilities of this layer : </source>
        <translation>Возможности редактирования данного слоя: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="727"/>
        <source>Extents:</source>
        <translation>Границы: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="732"/>
        <source>In layer spatial reference system units : </source>
        <translation>В единицах координатной системы слоя: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="733"/>
        <source>xMin,yMin </source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="737"/>
        <source> : xMax,yMax </source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="792"/>
        <source>In project spatial reference system units : </source>
        <translation>В единицах координатной системы проекта: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="766"/>
        <source>Layer Spatial Reference System:</source>
        <translation>Система координат слоя:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="803"/>
        <source>Attribute field info:</source>
        <translation>Поля атрибутов:</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="810"/>
        <source>Field</source>
        <translation>Поле</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="813"/>
        <source>Type</source>
        <translation>Тип</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="816"/>
        <source>Length</source>
        <translation>Длина</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="819"/>
        <source>Precision</source>
        <translation>Точность</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="677"/>
        <source>Layer comment: </source>
        <translation>Комментарий слоя: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="822"/>
        <source>Comment</source>
        <translation>Комментарий</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="918"/>
        <source>Default Style</source>
        <translation>Стиль по умолчанию</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="1021"/>
        <source>QGIS Layer Style File (*.qml)</source>
        <translation>Файл стиля слоя QGIS (*.qml)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="1049"/>
        <source>QGIS</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="1050"/>
        <source>Unknown style format: </source>
        <translation>Неизвестный формат стиля: </translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="134"/>
        <source>id</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="135"/>
        <source>name</source>
        <translation>имя</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="136"/>
        <source>type</source>
        <translation>тип</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="137"/>
        <source>length</source>
        <translation>длина</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="138"/>
        <source>precision</source>
        <translation>точность</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="139"/>
        <source>comment</source>
        <translation>комментарий</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="140"/>
        <source>edit widget</source>
        <translation>элемент редактирования</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="141"/>
        <source>values</source>
        <translation>значения</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="166"/>
        <source>line edit</source>
        <translation>строчное редактирование</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="167"/>
        <source>unique values</source>
        <translation>уникальные значения</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="168"/>
        <source>unique values (editable)</source>
        <translation>уникальные значения (редактируемые)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="169"/>
        <source>value map</source>
        <translation>карта значений</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="170"/>
        <source>classification</source>
        <translation>классификация</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="249"/>
        <source>Name conflict</source>
        <translation>Конфликт имён</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="249"/>
        <source>The attribute could not be inserted. The name already exists in the table.</source>
        <translation>Не удалось вставить атрибут. Данное имя уже существует в таблице.</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="1042"/>
        <source>Saved Style</source>
        <translation>Сохранённый стиль</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="171"/>
        <source>range (editable)</source>
        <translation>диапазон (редактируемый)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="172"/>
        <source>range (slider)</source>
        <translation>диапазон (ползунок)</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="651"/>
        <source>Creation of spatial index successful</source>
        <translation> Пространственный индекс успешно создан</translation>
    </message>
    <message>
        <location filename="../src/app/qgsvectorlayerproperties.cpp" line="173"/>
        <source>file name</source>
        <translation>имя файла</translation>
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
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="304"/>
        <source>Symbology</source>
        <translation>Символика</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="108"/>
        <source>General</source>
        <translation>Общие</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="180"/>
        <source>Use scale dependent rendering</source>
        <translation>Видимость в пределах масштаба</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="212"/>
        <source>Minimum scale at which this layer will be displayed. </source>
        <translation>Минимальный масштаб, при котором виден данный слой. </translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="225"/>
        <source>Maximum scale at which this layer will be displayed. </source>
        <translation>Максимальный масштаб, при котором виден данный слой. </translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="120"/>
        <source>Display name</source>
        <translation>Имя в легенде</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="149"/>
        <source>Use this control to set which field is placed at the top level of the Identify Results dialog box.</source>
        <translation>Используйте этот список для выбора поля, помещаемого в верхний уровень дерева в диалоге результатов определения.</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="133"/>
        <source>Display field for the Identify Results dialog box</source>
        <translation>Отображаемое поле для диалога результатов определения</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="136"/>
        <source>This sets the display field for the Identify Results dialog box</source>
        <translation>Отображаемое поле для диалога результатов определения</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="139"/>
        <source>Display field</source>
        <translation>Отображаемое поле</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="241"/>
        <source>Subset</source>
        <translation>Подмножество</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="279"/>
        <source>Query Builder</source>
        <translation>Конструктор запросов</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="163"/>
        <source>Create Spatial Index</source>
        <translation>Создать пространственный индекс</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="437"/>
        <source>Metadata</source>
        <translation>Метаданные</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="465"/>
        <source>Labels</source>
        <translation>Подписи</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="477"/>
        <source>Display labels</source>
        <translation>Включить подписи</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="509"/>
        <source>Actions</source>
        <translation>Действия</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="41"/>
        <source>Restore Default Style</source>
        <translation>Восстановить по умолчанию</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="48"/>
        <source>Save As Default</source>
        <translation>Сохранить по умолчанию</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="55"/>
        <source>Load Style ...</source>
        <translation>Загрузить стиль...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="62"/>
        <source>Save Style ...</source>
        <translation>Сохранить стиль...</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="322"/>
        <source>Legend type</source>
        <translation>Тип легенды</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="360"/>
        <source>Transparency</source>
        <translation>Прозрачность</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="114"/>
        <source>Options</source>
        <translation>Параметры</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="192"/>
        <source>Maximum</source>
        <translation>Максимальный</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="202"/>
        <source>Minimum</source>
        <translation>Минимальный</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="170"/>
        <source>Change CRS</source>
        <translation>Система координат</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="540"/>
        <source>Attributes</source>
        <translation>Атрибуты</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="557"/>
        <source>New column</source>
        <translation>Новое поле</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="567"/>
        <source>Ctrl+N</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="574"/>
        <source>Delete column</source>
        <translation>Удалить поле</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="584"/>
        <source>Ctrl+X</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="591"/>
        <source>Toggle editing mode</source>
        <translation>Режим редактирования</translation>
    </message>
    <message>
        <location filename="../src/ui/qgsvectorlayerpropertiesbase.ui" line="594"/>
        <source>Click to toggle table editing</source>
        <translation>Переключить редактирование таблицы</translation>
    </message>
</context>
<context>
    <name>QgsWFSPlugin</name>
    <message>
        <location filename="../src/plugins/wfs/qgswfsplugin.cpp" line="59"/>
        <source>&amp;Add WFS layer</source>
        <translation>&amp;Добавить слой WFS</translation>
    </message>
</context>
<context>
    <name>QgsWFSProvider</name>
    <message>
        <location filename="../src/providers/wfs/qgswfsprovider.cpp" line="1371"/>
        <source>unknown</source>
        <translation> неизвестно</translation>
    </message>
    <message>
        <location filename="../src/providers/wfs/qgswfsprovider.cpp" line="1377"/>
        <source>received %1 bytes from %2</source>
        <translation>получено %1 из %2 байт</translation>
    </message>
</context>
<context>
    <name>QgsWFSSourceSelect</name>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselect.cpp" line="257"/>
        <source>Are you sure you want to remove the </source>
        <translation>Вы уверены, что хотите удалить соединение </translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselect.cpp" line="257"/>
        <source> connection and all associated settings?</source>
        <translation> и все связанные с ним параметры?</translation>
    </message>
    <message>
        <location filename="../src/plugins/wfs/qgswfssourceselect.cpp" line="258"/>
        <source>Confirm Delete</source>
        <translation>Потвердите удаление</translation>
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
        <translation>&amp;Создать</translation>
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
        <translation>Добавить слой WFS</translation>
    </message>
</context>
<context>
    <name>QgsWmsProvider</name>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="717"/>
        <source>Tried URL: </source>
        <translation>Используемый URL: </translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="697"/>
        <source>HTTP Exception</source>
        <translation>HTTP-исключение</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="660"/>
        <source>WMS Service Exception</source>
        <translation>Исключение WMS-службы</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="771"/>
        <source>Could not get WMS capabilities: %1 at line %2 column %3</source>
        <translation>Не удалось получить возможности WMS: %1 в строке %2, столбце %3</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="802"/>
        <source>This is probably due to an incorrect WMS Server URL.</source>
        <translation>Вероятнее всего, адрес WMS-сервера неверен.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="798"/>
        <source>Could not get WMS capabilities in the expected format (DTD): no %1 or %2 found</source>
        <translation>Не удалось получить возможности WMS в ожидаемом формате (DTD): %1 или %2 не найдены</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1546"/>
        <source>Could not get WMS Service Exception at %1: %2 at line %3 column %4</source>
        <translation>Не удалось получить ошибку WMS из %1: %2 в строке %3, столбец %4</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1598"/>
        <source>Request contains a Format not offered by the server.</source>
        <translation>Запрос требует формата, который не поддерживается сервером.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1602"/>
        <source>Request contains a CRS not offered by the server for one or more of the Layers in the request.</source>
        <translation>Запрос включает систему координат, которая не поддерживается сервером для одного или более слоёв.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1611"/>
        <source>GetMap request is for a Layer not offered by the server, or GetFeatureInfo request is for a Layer not shown on the map.</source>
        <translation>В запросе GetMap указан слой, не предлагаемый сервером, или в запросе GetFeatureInfo указан слой, не показанный на карте.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1615"/>
        <source>Request is for a Layer in a Style not offered by the server.</source>
        <translation>Запрос требует слой в стиле, который недоступен на сервере.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1619"/>
        <source>GetFeatureInfo request is applied to a Layer which is not declared queryable.</source>
        <translation>Попытка запроса GetFeatureInfo для слоя, который не поддерживает запросов.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1623"/>
        <source>GetFeatureInfo request contains invalid X or Y value.</source>
        <translation>Запрос GetFeatureInfo содержит недопустимые значения X или Y.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1628"/>
        <source>Value of (optional) UpdateSequence parameter in GetCapabilities request is equal to current value of service metadata update sequence number.</source>
        <translation>Значение необязательного параметра UpdateSequence в запросе GetCapabilities равно текущему номеру последовательности обновления в метаданных службы.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1633"/>
        <source>Value of (optional) UpdateSequence parameter in GetCapabilities request is greater than current value of service metadata update sequence number.</source>
        <translation>Значение необязательного параметра UpdateSequence в запросе GetCapabilities выше, чем текущий номер последовательности обновления в метаданных службы.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1638"/>
        <source>Request does not include a sample dimension value, and the server did not declare a default value for that dimension.</source>
        <translation>Запрос не включает образец величины, и значение этой величины по умолчанию не указано сервером.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1642"/>
        <source>Request contains an invalid sample dimension value.</source>
        <translation>Запрос включает недопустимый образец величины.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1646"/>
        <source>Request is for an optional operation that is not supported by the server.</source>
        <translation>Запрос необязательной операции, которая не поддерживается сервером.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1650"/>
        <source>(Unknown error code from a post-1.3 WMS server)</source>
        <translation>(Неизвестный код ошибки от WMS-сервера &gt; 1.3)</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1653"/>
        <source>The WMS vendor also reported: </source>
        <translation>Дополнительное сообщение WMS-провайдера: </translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1838"/>
        <source>Server Properties:</source>
        <translation>Свойства сервера:</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1969"/>
        <source>Property</source>
        <translation>Свойство</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1972"/>
        <source>Value</source>
        <translation>Значение</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1855"/>
        <source>WMS Version</source>
        <translation>Версия WMS</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2101"/>
        <source>Title</source>
        <translation>Заглавие</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2109"/>
        <source>Abstract</source>
        <translation>Описание</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1879"/>
        <source>Keywords</source>
        <translation>Ключевые слова</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1887"/>
        <source>Online Resource</source>
        <translation>Онлайн-ресурс</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1895"/>
        <source>Contact Person</source>
        <translation>Контактное лицо</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1907"/>
        <source>Fees</source>
        <translation>Плата</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1915"/>
        <source>Access Constraints</source>
        <translation>Ограничения доступа</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1923"/>
        <source>Image Formats</source>
        <translation>Форматы изображения</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1931"/>
        <source>Identify Formats</source>
        <translation>Форматы запроса</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1939"/>
        <source>Layer Count</source>
        <translation>Количество слоёв</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1959"/>
        <source>Layer Properties: </source>
        <translation>Свойства слоя: </translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1977"/>
        <source>Selected</source>
        <translation>Выбран</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2034"/>
        <source>Yes</source>
        <translation>Да</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2034"/>
        <source>No</source>
        <translation>Нет</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1986"/>
        <source>Visibility</source>
        <translation>Видимость</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1992"/>
        <source>Visible</source>
        <translation>Видимый</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1993"/>
        <source>Hidden</source>
        <translation>Скрытый</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1994"/>
        <source>n/a</source>
        <translation>н/д</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2015"/>
        <source>Can Identify</source>
        <translation>Можно определять</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2023"/>
        <source>Can be Transparent</source>
        <translation>Может быть прозрачным</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2031"/>
        <source>Can Zoom In</source>
        <translation>Можно увеличивать</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2039"/>
        <source>Cascade Count</source>
        <translation>Количество каскадов</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2047"/>
        <source>Fixed Width</source>
        <translation>Фикс. ширина</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2055"/>
        <source>Fixed Height</source>
        <translation>Фикс. высота</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2063"/>
        <source>WGS 84 Bounding Box</source>
        <translation>Рамка WGS 84</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2073"/>
        <source>Available in CRS</source>
        <translation>Доступен в CRS</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2084"/>
        <source>Available in style</source>
        <translation>Доступен в стиле</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2093"/>
        <source>Name</source>
        <translation>Имя</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="2194"/>
        <source>Layer cannot be queried.</source>
        <translation>Не удаётся опросить слой.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1544"/>
        <source>Dom Exception</source>
        <translation>DOM-исключение</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="1606"/>
        <source>Request contains a SRS not offered by the server for one or more of the Layers in the request.</source>
        <translation>Запрос включает систему координат, которая не поддерживается сервером для одного или более слоёв.</translation>
    </message>
    <message>
        <location filename="../src/providers/wms/qgswmsprovider.cpp" line="385"/>
        <source>Could not determine URL for GetMap from the WMS capabilities response</source>
        <translation>Не удалось определить URL запроса GetMap из ответа на запрос возможностей WMS</translation>
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
        <translation></translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintgui.cpp" line="155"/>
        <source>Unknown format: </source>
        <translation>Неизвестный формат: </translation>
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
        <location filename="../src/plugins/quick_print/quickprintplugin.cpp" line="82"/>
        <source>&amp;Quick Print</source>
        <translation>&amp;Быстрая печать</translation>
    </message>
    <message>
        <location filename="../src/plugins/quick_print/quickprintplugin.cpp" line="77"/>
        <source>Provides a way to quickly produce a map with minimal user input.</source>
        <translation>Модуль для быстрой печати карт с минимумом параметров.</translation>
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
    <name>dxf2shpConverter</name>
    <message>
        <location filename="../src/plugins/dxf2shp_converter/dxf2shpconverter.cpp" line="73"/>
        <source>Converts DXF files in Shapefile format</source>
        <translation>Преобразование файлов формата dxf в shape-файлы</translation>
    </message>
    <message>
        <location filename="../src/plugins/dxf2shp_converter/dxf2shpconverter.cpp" line="109"/>
        <source>&amp;Dxf2Shp</source>
        <translation>&amp;Dxf2Shp</translation>
    </message>
</context>
<context>
    <name>dxf2shpConverterGui</name>
    <message>
        <location filename="../src/plugins/dxf2shp_converter/dxf2shpconvertergui.cpp" line="152"/>
        <source>Choose a DXF file to open</source>
        <translation>Выберите загружаемый DXF-файл</translation>
    </message>
    <message>
        <location filename="../src/plugins/dxf2shp_converter/dxf2shpconvertergui.ui" line="25"/>
        <source>Dxf Importer</source>
        <translation>Импорт Dxf</translation>
    </message>
    <message>
        <location filename="../src/plugins/dxf2shp_converter/dxf2shpconvertergui.ui" line="34"/>
        <source>Input Dxf file</source>
        <translation>Исходный Dxf-файл</translation>
    </message>
    <message>
        <location filename="../src/plugins/dxf2shp_converter/dxf2shpconvertergui.ui" line="64"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="../src/plugins/dxf2shp_converter/dxf2shpconvertergui.ui" line="51"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;DejaVu Sans&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;MS Shell Dlg 2&apos;; font-size:8pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;Output file&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;DejaVu Sans&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;MS Shell Dlg 2&apos;; font-size:8pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;Файл вывода&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/plugins/dxf2shp_converter/dxf2shpconvertergui.ui" line="83"/>
        <source>Output file type</source>
        <translation>Тип выходного файла</translation>
    </message>
    <message>
        <location filename="../src/plugins/dxf2shp_converter/dxf2shpconvertergui.ui" line="89"/>
        <source>Polyline</source>
        <translation>Полилиния</translation>
    </message>
    <message>
        <location filename="../src/plugins/dxf2shp_converter/dxf2shpconvertergui.ui" line="99"/>
        <source>Polygon</source>
        <translation>Полигон</translation>
    </message>
    <message>
        <location filename="../src/plugins/dxf2shp_converter/dxf2shpconvertergui.ui" line="106"/>
        <source>Point</source>
        <translation>Точка</translation>
    </message>
    <message>
        <location filename="../src/plugins/dxf2shp_converter/dxf2shpconvertergui.ui" line="116"/>
        <source>Export text labels</source>
        <translation>Экспортировать текстовые подписи</translation>
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
        <translation>Описание полей:
* Исходный DXF-файл: путь к загружаемому DXF-файлу
* Выходной Shp-файл: желаемое имя создаваемого shape-файла
* Тип выходного файла: указывает тип создаваемого shape-файла
* Экспортировать текстовые подписи: если активировано, будет создан дополнительный точечный shape-файл, связанный с которым dbf-файл будет включать информацию о текстовых (TEXT) данных исходного файла и сами текстовые строки

---
Разработчики: Paolo L. Scala, Barbara Rita Barricelli, Marco Padula
CNR, Milan Unit (Information Technology), Construction Technologies Institute.
Поддержку можно получить по адресу scala@itc.cnr.it
</translation>
    </message>
    <message>
        <location filename="../src/plugins/dxf2shp_converter/dxf2shpconvertergui.cpp" line="162"/>
        <source>Choose a file name to save to</source>
        <translation>Выберите имя сохраняемого файла</translation>
    </message>
</context>
<context>
    <name>pluginname</name>
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
    <message>
        <location filename="../src/plugins/plugin_template/plugin.cpp" line="75"/>
        <source>Replace this with a short description of what the plugin does</source>
        <translation></translation>
    </message>
</context>
</TS>
