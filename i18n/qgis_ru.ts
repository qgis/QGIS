<!DOCTYPE TS><TS>
<defaultcodec></defaultcodec>
<context>
    <name>CoordinateCapture</name>
    <message>
        <source>Coordinate Capture</source>
        <translation>Захват координат</translation>
    </message>
    <message>
        <source>Click on the map to view coordinates and capture to clipboard.</source>
        <translation>Вывод координат в месте щелчка и копирование их в буфер обмена.</translation>
    </message>
    <message>
        <source>&amp;Coordinate Capture</source>
        <translation>&amp;Захват координат</translation>
    </message>
    <message>
        <source>Click to select the CRS to use for coordinate display</source>
        <translation>Щёлкните для выбора системы координат, используемой для вывода</translation>
    </message>
    <message>
        <source>Coordinate in your selected CRS</source>
        <translation>Координаты в выбранной системе</translation>
    </message>
    <message>
        <source>Coordinate in map canvas coordinate reference system</source>
        <translation>Координаты в системе координат проекта</translation>
    </message>
    <message>
        <source>Copy to clipboard</source>
        <translation>Копировать в буфер обмена</translation>
    </message>
    <message>
        <source>Click to enable mouse tracking. Click the canvas to stop</source>
        <translation>Щёлкните для активации слежения за курсором. Щёлкните на карте для прекращения слежения</translation>
    </message>
</context>
<context>
    <name>CoordinateCaptureGui</name>
    <message>
        <source>Welcome to your automatically generated plugin!</source>
        <translation>Welcome to your automatically generated plugin!</translation>
    </message>
    <message>
        <source>This is just a starting point. You now need to modify the code to make it do something useful....read on for a more information to get yourself started.</source>
        <translation>This is just a starting point. You now need to modify the code to make it do something useful....read on for a more information to get yourself started.</translation>
    </message>
    <message>
        <source>Documentation:</source>
        <translation>Documentation:</translation>
    </message>
    <message>
        <source>You really need to read the QGIS API Documentation now at:</source>
        <translation>You really need to read the QGIS API Documentation now at:</translation>
    </message>
    <message>
        <source>In particular look at the following classes:</source>
        <translation>In particular look at the following classes:</translation>
    </message>
    <message>
        <source>QgsPlugin is an ABC that defines required behaviour your plugin must provide. See below for more details.</source>
        <translation>QgsPlugin is an ABC that defines required behaviour your plugin must provide. See below for more details.</translation>
    </message>
    <message>
        <source>What are all the files in my generated plugin directory for?</source>
        <translation>What are all the files in my generated plugin directory for?</translation>
    </message>
    <message>
        <source>This is the generated CMake file that builds the plugin. You should add you application specific dependencies and source files to this file.</source>
        <translation>This is the generated CMake file that builds the plugin. You should add you application specific dependencies and source files to this file.</translation>
    </message>
    <message>
        <source>This is the class that provides the &apos;glue&apos; between your custom application logic and the QGIS application. You will see that a number of methods are already implemented for you - including some examples of how to add a raster or vector layer to the main application map canvas. This class is a concrete instance of the QgisPlugin interface which defines required behaviour for a plugin. In particular, a plugin has a number of static methods and members so that the QgsPluginManager and plugin loader logic can identify each plugin, create an appropriate menu entry for it etc. Note there is nothing stopping you creating multiple toolbar icons and menu entries for a single plugin. By default though a single menu entry and toolbar button is created and its pre-configured to call the run() method in this class when selected. This default implementation provided for you by the plugin builder is well documented, so please refer to the code for further advice.</source>
        <translation>This is the class that provides the &apos;glue&apos; between your custom application logic and the QGIS application. You will see that a number of methods are already implemented for you - including some examples of how to add a raster or vector layer to the main application map canvas. This class is a concrete instance of the QgisPlugin interface which defines required behaviour for a plugin. In particular, a plugin has a number of static methods and members so that the QgsPluginManager and plugin loader logic can identify each plugin, create an appropriate menu entry for it etc. Note there is nothing stopping you creating multiple toolbar icons and menu entries for a single plugin. By default though a single menu entry and toolbar button is created and its pre-configured to call the run() method in this class when selected. This default implementation provided for you by the plugin builder is well documented, so please refer to the code for further advice.</translation>
    </message>
    <message>
        <source>This is a Qt designer &apos;ui&apos; file. It defines the look of the default plugin dialog without implementing any application logic. You can modify this form to suite your needs or completely remove it if your plugin does not need to display a user form (e.g. for custom MapTools).</source>
        <translation>This is a Qt designer &apos;ui&apos; file. It defines the look of the default plugin dialog without implementing any application logic. You can modify this form to suite your needs or completely remove it if your plugin does not need to display a user form (e.g. for custom MapTools).</translation>
    </message>
    <message>
        <source>This is the concrete class where application logic for the above mentioned dialog should go. The world is your oyster here really....</source>
        <translation>This is the concrete class where application logic for the above mentioned dialog should go. The world is your oyster here really....</translation>
    </message>
    <message>
        <source>This is the Qt4 resources file for your plugin. The Makefile generated for your plugin is all set up to compile the resource file so all you need to do is add your additional icons etc using the simple xml file format. Note the namespace used for all your resources e.g. (&apos;:/Homann/&apos;). It is important to use this prefix for all your resources. We suggest you include any other images and run time data in this resurce file too.</source>
        <translation>This is the Qt4 resources file for your plugin. The Makefile generated for your plugin is all set up to compile the resource file so all you need to do is add your additional icons etc using the simple xml file format. Note the namespace used for all your resources e.g. (&apos;:/Homann/&apos;). It is important to use this prefix for all your resources. We suggest you include any other images and run time data in this resurce file too.</translation>
    </message>
    <message>
        <source>This is the icon that will be used for your plugin menu entry and toolbar icon. Simply replace this icon with your own icon to make your plugin disctinctive from the rest.</source>
        <translation>This is the icon that will be used for your plugin menu entry and toolbar icon. Simply replace this icon with your own icon to make your plugin disctinctive from the rest.</translation>
    </message>
    <message>
        <source>This file contains the documentation you are reading now!</source>
        <translation>This file contains the documentation you are reading now!</translation>
    </message>
    <message>
        <source>Getting developer help:</source>
        <translation>Getting developer help:</translation>
    </message>
    <message>
        <source>For Questions and Comments regarding the plugin builder template and creating your features in QGIS using the plugin interface please contact us via:</source>
        <translation>For Questions and Comments regarding the plugin builder template and creating your features in QGIS using the plugin interface please contact us via:</translation>
    </message>
    <message>
        <source>&lt;li&gt; the QGIS developers mailing list, or &lt;/li&gt;&lt;li&gt; IRC (#qgis on freenode.net)&lt;/li&gt;</source>
        <translation>&lt;li&gt; the QGIS developers mailing list, or &lt;/li&gt;&lt;li&gt; IRC (#qgis on freenode.net)&lt;/li&gt;</translation>
    </message>
    <message>
        <source>QGIS is distributed under the Gnu Public License. If you create a useful plugin please consider contributing it back to the community.</source>
        <translation>QGIS is distributed under the Gnu Public License. If you create a useful plugin please consider contributing it back to the community.</translation>
    </message>
    <message>
        <source>Have fun and thank you for choosing QGIS.</source>
        <translation>Have fun and thank you for choosing QGIS.</translation>
    </message>
</context>
<context>
    <name>CoordinateCaptureGuiBase</name>
    <message>
        <source>QGIS Plugin Template</source>
        <translation>Шаблон модуля QGIS</translation>
    </message>
    <message>
        <source>Plugin Template</source>
        <translation>Шаблон модуля</translation>
    </message>
</context>
<context>
    <name>Dialog</name>
    <message>
        <source>QGIS Plugin Installer</source>
        <translation>Установка модулей QGIS</translation>
    </message>
    <message>
        <source>Name of plugin to install</source>
        <translation>Имя модуля для установки</translation>
    </message>
    <message>
        <source>Get List</source>
        <translation>Получить список</translation>
    </message>
    <message>
        <source>Done</source>
        <translation>Готово</translation>
    </message>
    <message>
        <source>Install Plugin</source>
        <translation>Установить модуль</translation>
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
        <translation>Выберите репозиторий, загрузите список доступных модулей и выберите модуль для установки</translation>
    </message>
    <message>
        <source>Repository</source>
        <translation>Репозиторий</translation>
    </message>
    <message>
        <source>Active repository:</source>
        <translation>Активный репозиторий:</translation>
    </message>
    <message>
        <source>Add</source>
        <translation>Добавить</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation>Изменить</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>Удалить</translation>
    </message>
    <message>
        <source>Connect</source>
        <translation>Подключиться</translation>
    </message>
    <message>
        <source>Browse</source>
        <translation>Обзор</translation>
    </message>
    <message>
        <source>OGR Converter</source>
        <translation>Преобразователь OGR</translation>
    </message>
    <message>
        <source>Could not establish connection to: &apos;</source>
        <translation>Не удалось установить соединение с:&apos;</translation>
    </message>
    <message>
        <source>Open OGR file</source>
        <translation>Открыть файл в формате OGR</translation>
    </message>
    <message>
        <source>OGR File Data Source (*.*)</source>
        <translation>Файловый источник данных OGR (*.*)</translation>
    </message>
    <message>
        <source>Open Directory</source>
        <translation>Открыть каталог</translation>
    </message>
    <message>
        <source>Input OGR dataset is missing!</source>
        <translation>Не указан исходный набор данных OGR!</translation>
    </message>
    <message>
        <source>Input OGR layer name is missing!</source>
        <translation>Не указано имя слоя OGR!</translation>
    </message>
    <message>
        <source>Target OGR format not selected!</source>
        <translation>Не выбран целевой OGR-формат!</translation>
    </message>
    <message>
        <source>Output OGR dataset is missing!</source>
        <translation>Не указан целевой набор данных OGR!</translation>
    </message>
    <message>
        <source>Output OGR layer name is missing!</source>
        <translation>Не указано имя целевого слоя OGR!</translation>
    </message>
    <message>
        <source>Successfully translated layer &apos;</source>
        <translation>Успешно преобразован слой &apos;</translation>
    </message>
    <message>
        <source>Failed to translate layer &apos;</source>
        <translation>Не удалось преобразовать слой &apos;</translation>
    </message>
    <message>
        <source>Successfully connected to: &apos;</source>
        <translation>Успешное соединение с &apos;</translation>
    </message>
    <message>
        <source>Choose a file name to save to</source>
        <translation>Выберите имя сохраняемого файла</translation>
    </message>
    <message>
        <source>Dialog</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Point Symbol</source>
        <translation type="unfinished">Значок</translation>
    </message>
    <message>
        <source>Size</source>
        <translation type="unfinished">Размер</translation>
    </message>
    <message>
        <source>Area scale field</source>
        <translation type="unfinished">Поле масштаба</translation>
    </message>
    <message>
        <source>Rotation field</source>
        <translation type="unfinished">Поле вращения</translation>
    </message>
    <message>
        <source>Style Options</source>
        <translation type="unfinished">Параметры стиля</translation>
    </message>
    <message>
        <source>Outline style</source>
        <translation type="unfinished">Стиль линии</translation>
    </message>
    <message>
        <source>Outline color</source>
        <translation type="unfinished">Цвет линии</translation>
    </message>
    <message>
        <source>Outline width</source>
        <translation type="unfinished">Ширина линии</translation>
    </message>
    <message>
        <source>Fill color</source>
        <translation type="unfinished">Цвет заливки</translation>
    </message>
    <message>
        <source>Fill style</source>
        <translation type="unfinished">Стиль заливки</translation>
    </message>
    <message>
        <source>...</source>
        <translation type="unfinished">...</translation>
    </message>
</context>
<context>
    <name>Gui</name>
    <message>
        <source>Welcome to your automatically generated plugin!</source>
        <translation>Welcome to your automatically generated plugin!</translation>
    </message>
    <message>
        <source>This is just a starting point. You now need to modify the code to make it do something useful....read on for a more information to get yourself started.</source>
        <translation>This is just a starting point. You now need to modify the code to make it do something useful....read on for a more information to get yourself started.</translation>
    </message>
    <message>
        <source>Documentation:</source>
        <translation>Documentation:</translation>
    </message>
    <message>
        <source>You really need to read the QGIS API Documentation now at:</source>
        <translation>You really need to read the QGIS API Documentation now at:</translation>
    </message>
    <message>
        <source>In particular look at the following classes:</source>
        <translation>In particular look at the following classes:</translation>
    </message>
    <message>
        <source>QgsPlugin is an ABC that defines required behaviour your plugin must provide. See below for more details.</source>
        <translation>QgsPlugin is an ABC that defines required behaviour your plugin must provide. See below for more details.</translation>
    </message>
    <message>
        <source>What are all the files in my generated plugin directory for?</source>
        <translation>What are all the files in my generated plugin directory for?</translation>
    </message>
    <message>
        <source>This is the generated CMake file that builds the plugin. You should add you application specific dependencies and source files to this file.</source>
        <translation>This is the generated CMake file that builds the plugin. You should add you application specific dependencies and source files to this file.</translation>
    </message>
    <message>
        <source>This is the class that provides the &apos;glue&apos; between your custom application logic and the QGIS application. You will see that a number of methods are already implemented for you - including some examples of how to add a raster or vector layer to the main application map canvas. This class is a concrete instance of the QgisPlugin interface which defines required behaviour for a plugin. In particular, a plugin has a number of static methods and members so that the QgsPluginManager and plugin loader logic can identify each plugin, create an appropriate menu entry for it etc. Note there is nothing stopping you creating multiple toolbar icons and menu entries for a single plugin. By default though a single menu entry and toolbar button is created and its pre-configured to call the run() method in this class when selected. This default implementation provided for you by the plugin builder is well documented, so please refer to the code for further advice.</source>
        <translation>This is the class that provides the &apos;glue&apos; between your custom application logic and the QGIS application. You will see that a number of methods are already implemented for you - including some examples of how to add a raster or vector layer to the main application map canvas. This class is a concrete instance of the QgisPlugin interface which defines required behaviour for a plugin. In particular, a plugin has a number of static methods and members so that the QgsPluginManager and plugin loader logic can identify each plugin, create an appropriate menu entry for it etc. Note there is nothing stopping you creating multiple toolbar icons and menu entries for a single plugin. By default though a single menu entry and toolbar button is created and its pre-configured to call the run() method in this class when selected. This default implementation provided for you by the plugin builder is well documented, so please refer to the code for further advice.</translation>
    </message>
    <message>
        <source>This is a Qt designer &apos;ui&apos; file. It defines the look of the default plugin dialog without implementing any application logic. You can modify this form to suite your needs or completely remove it if your plugin does not need to display a user form (e.g. for custom MapTools).</source>
        <translation>This is a Qt designer &apos;ui&apos; file. It defines the look of the default plugin dialog without implementing any application logic. You can modify this form to suite your needs or completely remove it if your plugin does not need to display a user form (e.g. for custom MapTools).</translation>
    </message>
    <message>
        <source>This is the concrete class where application logic for the above mentioned dialog should go. The world is your oyster here really....</source>
        <translation>This is the concrete class where application logic for the above mentioned dialog should go. The world is your oyster here really....</translation>
    </message>
    <message>
        <source>This is the Qt4 resources file for your plugin. The Makefile generated for your plugin is all set up to compile the resource file so all you need to do is add your additional icons etc using the simple xml file format. Note the namespace used for all your resources e.g. (&apos;:/Homann/&apos;). It is important to use this prefix for all your resources. We suggest you include any other images and run time data in this resurce file too.</source>
        <translation>This is the Qt4 resources file for your plugin. The Makefile generated for your plugin is all set up to compile the resource file so all you need to do is add your additional icons etc using the simple xml file format. Note the namespace used for all your resources e.g. (&apos;:/Homann/&apos;). It is important to use this prefix for all your resources. We suggest you include any other images and run time data in this resurce file too.</translation>
    </message>
    <message>
        <source>This is the icon that will be used for your plugin menu entry and toolbar icon. Simply replace this icon with your own icon to make your plugin disctinctive from the rest.</source>
        <translation>This is the icon that will be used for your plugin menu entry and toolbar icon. Simply replace this icon with your own icon to make your plugin disctinctive from the rest.</translation>
    </message>
    <message>
        <source>This file contains the documentation you are reading now!</source>
        <translation>This file contains the documentation you are reading now!</translation>
    </message>
    <message>
        <source>Getting developer help:</source>
        <translation>Getting developer help:</translation>
    </message>
    <message>
        <source>For Questions and Comments regarding the plugin builder template and creating your features in QGIS using the plugin interface please contact us via:</source>
        <translation>For Questions and Comments regarding the plugin builder template and creating your features in QGIS using the plugin interface please contact us via:</translation>
    </message>
    <message>
        <source>&lt;li&gt; the QGIS developers mailing list, or &lt;/li&gt;&lt;li&gt; IRC (#qgis on freenode.net)&lt;/li&gt;</source>
        <translation>&lt;li&gt; the QGIS developers mailing list, or &lt;/li&gt;&lt;li&gt; IRC (#qgis on freenode.net)&lt;/li&gt;</translation>
    </message>
    <message>
        <source>QGIS is distributed under the Gnu Public License. If you create a useful plugin please consider contributing it back to the community.</source>
        <translation>QGIS is distributed under the Gnu Public License. If you create a useful plugin please consider contributing it back to the community.</translation>
    </message>
    <message>
        <source>Have fun and thank you for choosing QGIS.</source>
        <translation>Have fun and thank you for choosing QGIS.</translation>
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
        <translation>Введите XY-координаты, которые соответствуют выбранной точке на изображении. Либо нажмите на кнопке со значком карандаша и затем щёлкните на соответствующей точке в области карты QGIS для автоматического ввода координат этой точки.</translation>
    </message>
    <message>
        <source> from map canvas</source>
        <translation> с карты</translation>
    </message>
</context>
<context>
    <name>OgrConverterGuiBase</name>
    <message>
        <source>OGR Layer Converter</source>
        <translation>Преобразователь слоёв OGR</translation>
    </message>
    <message>
        <source>Source</source>
        <translation>Источник</translation>
    </message>
    <message>
        <source>Format</source>
        <translation>Формат</translation>
    </message>
    <message>
        <source>File</source>
        <translation>Файл</translation>
    </message>
    <message>
        <source>Directory</source>
        <translation>Каталог</translation>
    </message>
    <message>
        <source>Remote source</source>
        <translation>Удалённый источник</translation>
    </message>
    <message>
        <source>Dataset</source>
        <translation>Набор данных</translation>
    </message>
    <message>
        <source>Browse</source>
        <translation>Обзор</translation>
    </message>
    <message>
        <source>Layer</source>
        <translation>Слой</translation>
    </message>
    <message>
        <source>Target</source>
        <translation>Приёмник</translation>
    </message>
</context>
<context>
    <name>OgrPlugin</name>
    <message>
        <source>Run OGR Layer Converter</source>
        <translation>Преобразователь слоёв OGR</translation>
    </message>
    <message>
        <source>Replace this with a short description of the what the plugin does</source>
        <translation></translation>
    </message>
    <message>
        <source>OG&amp;R Converter</source>
        <translation>Преобразователь OG&amp;R</translation>
    </message>
</context>
<context>
    <name>QFileDialog</name>
    <message>
        <source>Save experiment report to portable document format (.pdf)</source>
        <translation>Сохранить вывод в формате PDF</translation>
    </message>
    <message>
        <source>Load layer properties from style file (.qml)</source>
        <translation>Загрузить свойства слоя из файла стиля (.qml)</translation>
    </message>
    <message>
        <source>Save layer properties as style file (.qml)</source>
        <translation>Сохранить свойства слоя в файл стиля (.qml)</translation>
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
        <source>QGis files (*.qgs)</source>
        <translation>Файлы QGIS (*.qgs)</translation>
    </message>
    <message>
        <source> at line </source>
        <translation> в строке </translation>
    </message>
    <message>
        <source> column </source>
        <translation>, столбец </translation>
    </message>
    <message>
        <source> for file </source>
        <translation> в файле </translation>
    </message>
    <message>
        <source>Unable to save to file </source>
        <translation>Не удалось сохранить в файл  </translation>
    </message>
    <message>
        <source>Referenced column wasn&apos;t found: </source>
        <translation>Упоминаемое поле не найдено: </translation>
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
        <translation>потому что</translation>
    </message>
    <message>
        <source>New centroid</source>
        <translation>Новый центроид</translation>
    </message>
    <message>
        <source>New point</source>
        <translation>Новая точка</translation>
    </message>
    <message>
        <source>New vertex</source>
        <translation>Новая вершина</translation>
    </message>
    <message>
        <source>Undo last point</source>
        <translation>Отменить последнюю точку</translation>
    </message>
    <message>
        <source>Close line</source>
        <translation>Закрыть линию</translation>
    </message>
    <message>
        <source>Select vertex</source>
        <translation>Выбрать вершину</translation>
    </message>
    <message>
        <source>Select new position</source>
        <translation>Выбрать новую позицию</translation>
    </message>
    <message>
        <source>Select line segment</source>
        <translation>Выбрать сегмент линии</translation>
    </message>
    <message>
        <source>New vertex position</source>
        <translation>Новая позиция вершины</translation>
    </message>
    <message>
        <source>Release</source>
        <translation>Освободить</translation>
    </message>
    <message>
        <source>Delete vertex</source>
        <translation>Удалить вершину</translation>
    </message>
    <message>
        <source>Release vertex</source>
        <translation>Освободить вершину</translation>
    </message>
    <message>
        <source>Select element</source>
        <translation>Выбрать элемент</translation>
    </message>
    <message>
        <source>New location</source>
        <translation>Новое положение</translation>
    </message>
    <message>
        <source>Release selected</source>
        <translation>Освободить выделение</translation>
    </message>
    <message>
        <source>Delete selected / select next</source>
        <translation>Удалить выделение / выбрать следующий</translation>
    </message>
    <message>
        <source>Select position on line</source>
        <translation>Выбрать позицию на линии</translation>
    </message>
    <message>
        <source>Split the line</source>
        <translation>Разделить линию</translation>
    </message>
    <message>
        <source>Release the line</source>
        <translation>Освободить линию</translation>
    </message>
    <message>
        <source>Select point on line</source>
        <translation>Выбрать точку на линии</translation>
    </message>
    <message>
        <source>Label</source>
        <translation>Подпись</translation>
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
        <translation>Для привязки с линейным преобразованием необходимо как минимум 2 точки.</translation>
    </message>
    <message>
        <source>Fit to a Helmert transform requires at least 2 points.</source>
        <translation>Для привязки с преобразованием Гельмерта необходимо как минимум 2 точки.</translation>
    </message>
    <message>
        <source>Fit to an affine transform requires at least 4 points.</source>
        <translation>Для привязки с аффинным преобразованием необходимо как минимум 4 точки.</translation>
    </message>
    <message>
        <source>Couldn&apos;t open the data source: </source>
        <translation>Не удалось открыть источник данных: </translation>
    </message>
    <message>
        <source>Parse error at line </source>
        <translation>Ошибка разбора в строке </translation>
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
        <translation>Модуль GRASS</translation>
    </message>
    <message>
        <source>QGIS couldn&apos;t find your GRASS installation.
Would you like to specify path (GISBASE) to your GRASS installation?</source>
        <translation>Не удалось найти установленную систему GRASS.
Вы хотите указать путь (GISBASE) к вашей установке GRASS?</translation>
    </message>
    <message>
        <source>Choose GRASS installation path (GISBASE)</source>
        <translation>Выберите путь установки GRASS (GISBASE)</translation>
    </message>
    <message>
        <source>GRASS data won&apos;t be available if GISBASE is not specified.</source>
        <translation>Данные GRASS будут недоступны, если не задано значение GISBASE.</translation>
    </message>
    <message>
        <source>CopyrightLabel</source>
        <translation>Метка авторского права</translation>
    </message>
    <message>
        <source>Draws copyright information</source>
        <translation>Вывод значка авторского права</translation>
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
        <translation>Загружает и показывает текстовые файлы, содержащие x,y-координаты</translation>
    </message>
    <message>
        <source>Add Delimited Text Layer</source>
        <translation>Текст с разделителями</translation>
    </message>
    <message>
        <source>Georeferencer</source>
        <translation>Привязка растров</translation>
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
        <translation></translation>
    </message>
    <message>
        <source>GRASS layer</source>
        <translation>Поддержка GRASS</translation>
    </message>
    <message>
        <source>Graticule Creator</source>
        <translation>Конструктор сетки</translation>
    </message>
    <message>
        <source>Builds a graticule</source>
        <translation>Модуль построения сетки</translation>
    </message>
    <message>
        <source>NorthArrow</source>
        <translation>Указатель «север-юг»</translation>
    </message>
    <message>
        <source>Displays a north arrow overlayed onto the map</source>
        <translation>Вывод указателя «север-юг»</translation>
    </message>
    <message>
        <source>[menuitemname]</source>
        <translation></translation>
    </message>
    <message>
        <source>[plugindescription]</source>
        <translation></translation>
    </message>
    <message>
        <source>ScaleBar</source>
        <translation>Масштабная линейка</translation>
    </message>
    <message>
        <source>Draws a scale bar</source>
        <translation>Вывод масштабной линейки</translation>
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
        <translation>Добавляет возможность загрузки WFS-слоёв</translation>
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
        <translation>Не удалось внести изменения в векторный слой. Для редактирования слоя, щёлкните на его имени в легенде правой кнопкой мыши и выберите «Режим редактирования».</translation>
    </message>
    <message>
        <source>To select features, you must choose a vector layer by clicking on its name in the legend</source>
        <translation>Для выделения объектов необходимо выбрать векторный слой щелчком мыши на его имени в легенде</translation>
    </message>
    <message>
        <source>Python error</source>
        <translation>Ошибка Python</translation>
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
        <translation>2.5-мерные данные не поддерживаются</translation>
    </message>
    <message>
        <source>Adding features to 2.5D shapetypes is not supported yet</source>
        <translation>Добавление 2.5-мерных объектов в данный момент не поддерживается</translation>
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
        <translation>Ошибка типа геометрии</translation>
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
        <translation>Неизвестная ошибка</translation>
    </message>
    <message>
        <source>Error, could not add ring</source>
        <translation>Ошибка, не удалось добавить кольцо</translation>
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
        <translation>Нет соединения</translation>
    </message>
    <message>
        <source>Looking up &apos;%1&apos;</source>
        <translation>Поиск «%1»</translation>
    </message>
    <message>
        <source>Connecting to &apos;%1&apos;</source>
        <translation>Соединение с «%1»</translation>
    </message>
    <message>
        <source>Sending request &apos;%1&apos;</source>
        <translation>Отправка запроса «%1»</translation>
    </message>
    <message>
        <source>Receiving reply</source>
        <translation>Получение ответа</translation>
    </message>
    <message>
        <source>Response is complete</source>
        <translation>Ответ получен</translation>
    </message>
    <message>
        <source>Closing down connection</source>
        <translation>Соединение закрыто</translation>
    </message>
    <message>
        <source>Unable to open </source>
        <translation>Не удалось открыть </translation>
    </message>
    <message>
        <source>Regular expressions on numeric values don&apos;t make sense. Use comparison instead.</source>
        <translation>Регулярные выражения не имеют смысла для числовых значений. Используйте сравнение.</translation>
    </message>
    <message>
        <source>Geoprocessing functions for working with PostgreSQL/PostGIS layers</source>
        <translation>Функции пространственной обработки для слоёв PostgreSQL/PostGIS</translation>
    </message>
    <message>
        <source>Location: </source>
        <comment>Metadata in GRASS Browser</comment>
        <translation>Район: </translation>
    </message>
    <message>
        <source>&lt;br&gt;Mapset: </source>
        <comment>Metadata in GRASS Browser</comment>
        <translation>&lt;br&gt;Набор: </translation>
    </message>
    <message>
        <source>Location: </source>
        <translation>Район: </translation>
    </message>
    <message>
        <source>&lt;br&gt;Mapset: </source>
        <translation>&lt;br&gt;Набор: </translation>
    </message>
    <message>
        <source>&lt;b&gt;Raster&lt;/b&gt;</source>
        <translation>&lt;b&gt;Растр&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Cannot open raster header</source>
        <translation>Не удалось открыть заголовок растра</translation>
    </message>
    <message>
        <source>Rows</source>
        <translation>Строк</translation>
    </message>
    <message>
        <source>Columns</source>
        <translation>Столбцов</translation>
    </message>
    <message>
        <source>N-S resolution</source>
        <translation>Вертикальное разрешение</translation>
    </message>
    <message>
        <source>E-W resolution</source>
        <translation>Горизонтальное разрешение</translation>
    </message>
    <message>
        <source>North</source>
        <translation>Север</translation>
    </message>
    <message>
        <source>South</source>
        <translation>Юг</translation>
    </message>
    <message>
        <source>East</source>
        <translation>Восток</translation>
    </message>
    <message>
        <source>West</source>
        <translation>Запад</translation>
    </message>
    <message>
        <source>Format</source>
        <translation>Формат</translation>
    </message>
    <message>
        <source>Minimum value</source>
        <translation>Мин. значение</translation>
    </message>
    <message>
        <source>Maximum value</source>
        <translation>Макс. значение</translation>
    </message>
    <message>
        <source>Data source</source>
        <translation>Источник данных</translation>
    </message>
    <message>
        <source>Data description</source>
        <translation>Описание данных</translation>
    </message>
    <message>
        <source>Comments</source>
        <translation>Комментарии</translation>
    </message>
    <message>
        <source>Categories</source>
        <translation>Категории</translation>
    </message>
    <message>
        <source>&lt;b&gt;Vector&lt;/b&gt;</source>
        <translation>&lt;b&gt;Вектор&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Points</source>
        <translation>Точки</translation>
    </message>
    <message>
        <source>Lines</source>
        <translation>Линии</translation>
    </message>
    <message>
        <source>Boundaries</source>
        <translation>Границы</translation>
    </message>
    <message>
        <source>Centroids</source>
        <translation>Центроиды</translation>
    </message>
    <message>
        <source>Faces</source>
        <translation>Грани</translation>
    </message>
    <message>
        <source>Kernels</source>
        <translation>Ядра</translation>
    </message>
    <message>
        <source>Areas</source>
        <translation>Площади</translation>
    </message>
    <message>
        <source>Islands</source>
        <translation>Острова</translation>
    </message>
    <message>
        <source>Top</source>
        <translation>Верх</translation>
    </message>
    <message>
        <source>Bottom</source>
        <translation>Низ</translation>
    </message>
    <message>
        <source>yes</source>
        <translation>да</translation>
    </message>
    <message>
        <source>no</source>
        <translation>нет</translation>
    </message>
    <message>
        <source>History&lt;br&gt;</source>
        <translation>История&lt;br&gt;</translation>
    </message>
    <message>
        <source>&lt;b&gt;Layer&lt;/b&gt;</source>
        <translation>&lt;b&gt;Слой&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Features</source>
        <translation>Объектов</translation>
    </message>
    <message>
        <source>Driver</source>
        <translation>Драйвер</translation>
    </message>
    <message>
        <source>Database</source>
        <translation>База данных</translation>
    </message>
    <message>
        <source>Table</source>
        <translation>Таблица</translation>
    </message>
    <message>
        <source>Key column</source>
        <translation>Ключевое поле</translation>
    </message>
    <message>
        <source>GISBASE is not set.</source>
        <translation>GISBASE не задана.</translation>
    </message>
    <message>
        <source> is not a GRASS mapset.</source>
        <translation> не является набором GRASS.</translation>
    </message>
    <message>
        <source>Cannot start </source>
        <translation>Не удалось запустить </translation>
    </message>
    <message>
        <source>Mapset is already in use.</source>
        <translation>Набор уже используется.</translation>
    </message>
    <message>
        <source>Temporary directory </source>
        <translation>Временный каталог </translation>
    </message>
    <message>
        <source> exist but is not writable</source>
        <translation> существует, но закрыт для записи</translation>
    </message>
    <message>
        <source>Cannot create temporary directory </source>
        <translation>Не удаётся создать временный каталог </translation>
    </message>
    <message>
        <source>Cannot create </source>
        <translation>Не удаётся создать </translation>
    </message>
    <message>
        <source>Cannot remove mapset lock: </source>
        <translation>Не удалось снять блокировку набора: </translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Внимание</translation>
    </message>
    <message>
        <source>Cannot read raster map region</source>
        <translation>Не удаётся прочесть границы растровой карты</translation>
    </message>
    <message>
        <source>Cannot read vector map region</source>
        <translation>Не удаётся прочесть границы векторной карты</translation>
    </message>
    <message>
        <source>Cannot read region</source>
        <translation>Не удаётся прочесть регион</translation>
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
        <translation>Пространственная обработка PostgreSQL</translation>
    </message>
    <message>
        <source>Quick Print</source>
        <translation>Быстрая печать</translation>
    </message>
    <message>
        <source>Quick Print is a plugin to quickly print a map with minimal effort.</source>
        <translation>Модуль для быстрой печати карт с минимумом параметров.</translation>
    </message>
    <message>
        <source>Could not remove polygon intersection</source>
        <translation>Не удалось удалить пересечение полигонов</translation>
    </message>
    <message>
        <source>Loaded default style file from </source>
        <translation>Стиль по умолчанию загружен из </translation>
    </message>
    <message>
        <source>The directory containing your dataset needs to be writeable!</source>
        <translation>Необходимы права на запись в каталог, содержащий ваши данные!</translation>
    </message>
    <message>
        <source>Created default style file as </source>
        <translation>Файл стиля по умолчанию создан в </translation>
    </message>
    <message>
        <source>ERROR: Failed to created default style file as %1 Check file permissions and retry.</source>
        <translation>ОШИБКА: Не удалось создать файл стиля по умолчанию в %1. Проверьте права доступа к файлу и попробуйте ещё раз.</translation>
    </message>
    <message>
        <source> is not writeable.</source>
        <translation> не является записываемым файлом.</translation>
    </message>
    <message>
        <source>Please adjust permissions (if possible) and try again.</source>
        <translation>Пожалуйста, исправьте права доступа (если возможно) и попробуйте ещё раз.</translation>
    </message>
    <message>
        <source>Abort</source>
        <translation>Отменить</translation>
    </message>
    <message>
        <source>Version 0.001</source>
        <translation>Версия 0.001</translation>
    </message>
    <message>
        <source>Uncatched fatal GRASS error</source>
        <translation>Необработанная фатальная ошибка GRASS</translation>
    </message>
    <message>
        <source>Couldn&apos;t load SIP module.</source>
        <translation>Не удалось загрузить модуль SIP.</translation>
    </message>
    <message>
        <source>Python support will be disabled.</source>
        <translation>Поддержка Python будет выключена.</translation>
    </message>
    <message>
        <source>Couldn&apos;t load PyQt4.</source>
        <translation>Не удалось загрузить PyQt4.</translation>
    </message>
    <message>
        <source>Couldn&apos;t load PyQGIS.</source>
        <translation>Не удалось загрузить PyQGIS.</translation>
    </message>
    <message>
        <source>An error has occured while executing Python code:</source>
        <translation>При выполнении Python-кода возникла ошибка:</translation>
    </message>
    <message>
        <source>Python version:</source>
        <translation>Версия Python:</translation>
    </message>
    <message>
        <source>Python path:</source>
        <translation>Путь поиска Python:</translation>
    </message>
    <message>
        <source>An error occured during execution of following code:</source>
        <translation>Возникла ошибка при выполнении следующего кода:</translation>
    </message>
    <message>
        <source>Legend</source>
        <translation>Легенда</translation>
    </message>
    <message>
        <source>Coordinate Capture</source>
        <translation>Захват координат</translation>
    </message>
    <message>
        <source>Capture mouse coordinates in different CRS</source>
        <translation>Захват координат курсора в различных системах координат</translation>
    </message>
    <message>
        <source>Dxf2Shp Converter</source>
        <translation>Преобразователь Dxf2Shp</translation>
    </message>
    <message>
        <source>Converts from dxf to shp file format</source>
        <translation>Преобразование файлов формата dxf в shape-файлы</translation>
    </message>
    <message>
        <source>Interpolating...</source>
        <translation>Интерполяция...</translation>
    </message>
    <message>
        <source>Interpolation plugin</source>
        <translation>Модуль интерполяции</translation>
    </message>
    <message>
        <source>A plugin for interpolation based on vertices of a vector layer</source>
        <translation>Модуль интерполяции данных по вершинам в векторном слое</translation>
    </message>
    <message>
        <source>OGR Layer Converter</source>
        <translation>Преобразователь слоёв OGR</translation>
    </message>
    <message>
        <source>Translates vector layers between formats supported by OGR library</source>
        <translation>Преобразует векторные слои в форматы, поддерживаемые библиотекой OGR</translation>
    </message>
</context>
<context>
    <name>QgisApp</name>
    <message>
        <source>Quantum GIS - </source>
        <translation>Quantum GIS — </translation>
    </message>
    <message>
        <source>Version</source>
        <translation>Версия</translation>
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
        <source>No Layer Selected</source>
        <translation>Слой не выбран</translation>
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
        <source>Unable to get current version information from server</source>
        <translation>Не удалось получить информацию о версии с сервера</translation>
    </message>
    <message>
        <source>Connection refused - server may be down</source>
        <translation>В соединении отказано — вероятно, сервер недоступен</translation>
    </message>
    <message>
        <source>QGIS server was not found</source>
        <translation>Сервер QGIS не найден</translation>
    </message>
    <message>
        <source>Invalid Layer</source>
        <translation>Недействительный слой</translation>
    </message>
    <message>
        <source>%1 is an invalid layer and cannot be loaded.</source>
        <translation>Слой %1 не является действительным и не может быть загружен.</translation>
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
        <source>Map legend that displays all the layers currently on the map canvas. Click on the check box to turn a layer on or off. Double click on a layer in the legend to customize its appearance and set other properties.</source>
        <translation>Легенда карты, в которой перечислены все слои отображаемой карты. Щёлкните на флажке, чтобы переключить видимость соответствующего слоя. Дважды щёлкните на имени слоя, чтобы задать его отображение и другие свойства.</translation>
    </message>
    <message>
        <source>Map overview canvas. This canvas can be used to display a locator map that shows the current extent of the map canvas. The current extent is shown as a red rectangle. Any layer on the map can be added to the overview canvas.</source>
        <translation>Область обзора карты. Данная область используется для вывода обзорной карты, на которой виден текущий экстент области карты QGIS. Текущий экстент нарисован в виде красного прямоугольника. Любой слой карты может быть добавлен в обзорную область.</translation>
    </message>
    <message>
        <source>Map canvas. This is where raster and vector layers are displayed when added to the map</source>
        <translation>Область карты. Вывод растровых и векторных слоёв осуществляется в данную область </translation>
    </message>
    <message>
        <source>&amp;Plugins</source>
        <translation>&amp;Модули</translation>
    </message>
    <message>
        <source>Progress bar that displays the status of rendering layers and other time-intensive operations</source>
        <translation>Индикатор хода процесса отрисовки слоёв и других долговременных операций</translation>
    </message>
    <message>
        <source>Displays the current map scale</source>
        <translation>Показывает текущий масштаб карты</translation>
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
        <translation>Открыть проект</translation>
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
        <source>Save Project under a new name</source>
        <translation>Сохранить проект под другим именем</translation>
    </message>
    <message>
        <source>Save as Image...</source>
        <translation>Сохранить как изображение...</translation>
    </message>
    <message>
        <source>Save map as image</source>
        <translation>Сохранить карту как изображение</translation>
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
        <source>V</source>
        <comment>Add a Vector Layer</comment>
        <translation></translation>
    </message>
    <message>
        <source>Add a Vector Layer</source>
        <translation>Добавить векторный слой</translation>
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
        <translation>Параметры...</translation>
    </message>
    <message>
        <source>Change various QGIS options</source>
        <translation>Изменить параметры приложения QGIS</translation>
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
        <source>Measure Line </source>
        <translation>Измерить линию </translation>
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
        <source>O</source>
        <comment>Add current layer to overview map</comment>
        <translation></translation>
    </message>
    <message>
        <source>Add current layer to overview map</source>
        <translation>Добавить текущий слой в обзорную карту</translation>
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
        <translation>Готово</translation>
    </message>
    <message>
        <source>New features</source>
        <translation>Новые возможности</translation>
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
        <source>QGIS: Unable to load project</source>
        <translation>QGIS: не удалось загрузить проект</translation>
    </message>
    <message>
        <source>Unable to load project </source>
        <translation>Не удалось загрузить проект </translation>
    </message>
    <message>
        <source>QGIS - Changes in SVN Since Last Release</source>
        <translation>QGIS — Изменения в SVN со времени последнего релиза</translation>
    </message>
    <message>
        <source>Layer is not valid</source>
        <translation>Неверный слой</translation>
    </message>
    <message>
        <source>The layer is not a valid layer and can not be added to the map</source>
        <translation>Слой не является действительным и не может быть добавлен на карту</translation>
    </message>
    <message>
        <source>Save?</source>
        <translation>Сохранить?</translation>
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
        <source>Network error while communicating with server</source>
        <translation>Ошибка сети во время соединения с сервером</translation>
    </message>
    <message>
        <source>Unknown network socket error</source>
        <translation>Неизвестная ошибка сетевого соединения</translation>
    </message>
    <message>
        <source>Unable to communicate with QGIS Version server</source>
        <translation>Не удалось связаться с сервером версии QGIS</translation>
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
        <translation>Текущий слой нередактируем. Выберите «Режим редактирования» на панели инструментов оцифровки.</translation>
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
        <source>Shows the map coordinates at the current cursor position. The display is continuously updated as the mouse is moved.</source>
        <translation>Здесь показаны координаты карты в позиции курсора. Эти значения постоянно обновляются при движении мыши.</translation>
    </message>
    <message>
        <source>Move Feature</source>
        <translation>Переместить объект</translation>
    </message>
    <message>
        <source>Split Features</source>
        <translation>Разделить объекты</translation>
    </message>
    <message>
        <source>Map Tips</source>
        <translation>Всплывающие описания</translation>
    </message>
    <message>
        <source>Show information about a feature when the mouse is hovered over it</source>
        <translation>Показать информацию об объекте при перемещении над ним курсора мыши</translation>
    </message>
    <message>
        <source>Current map scale</source>
        <translation>Текущий масштаб карты</translation>
    </message>
    <message>
        <source>Project file is older</source>
        <translation>Устаревший файл проекта</translation>
    </message>
    <message>
        <source>&lt;p&gt;This project file was saved by an older version of QGIS.</source>
        <translation>&lt;p&gt;Этот файл проекта был создан старой версией QGIS.</translation>
    </message>
    <message>
        <source> When saving this project file, QGIS will update it to the latest version, possibly rendering it useless for older versions of QGIS.</source>
        <translation> При сохранении, этот файл будет обновлён, что может повлечь за собой несовместимость с предыдущими версиями QGIS.</translation>
    </message>
    <message>
        <source>&lt;p&gt;Even though QGIS developers try to maintain backwards compatibility, some of the information from the old project file might be lost.</source>
        <translation>&lt;p&gt;Несмотря на то, что разработчики QGIS стремятся к максимальной обратной совместимости, часть информации из старого проекта может быть потеряна.</translation>
    </message>
    <message>
        <source> To improve the quality of QGIS, we appreciate if you file a bug report at %3.</source>
        <translation> Вы могли бы помочь нам улучшить QGIS, отправив сообщение об ошибке по адресу: %3.</translation>
    </message>
    <message>
        <source> Be sure to include the old project file, and state the version of QGIS you used to discover the error.</source>
        <translation> Пожалуйста, приложите старый файл проекта и укажите версию QGIS, в которой была обнаружена ошибка.</translation>
    </message>
    <message>
        <source>&lt;p&gt;To remove this warning when opening an older project file, uncheck the box &apos;%5&apos; in the %4 menu.</source>
        <translation>&lt;p&gt;Если вы не хотите видеть это сообщение в дальнейшем, снимите флажок «%5» в меню «%4».</translation>
    </message>
    <message>
        <source>&lt;p&gt;Version of the project file: %1&lt;br&gt;Current version of QGIS: %2</source>
        <translation>&lt;p&gt;Версия файла проекта: %1&lt;br&gt;Текущая версия QGIS: %2</translation>
    </message>
    <message>
        <source>&lt;tt&gt;Settings:Options:General&lt;/tt&gt;</source>
        <comment>Menu path to setting options</comment>
        <translation>&lt;tt&gt;Правка:Параметры:Общие&lt;/tt&gt;</translation>
    </message>
    <message>
        <source>Warn me when opening a project file saved with an older version of QGIS</source>
        <translation>Предупреждать при попытке открытия файлов проекта старых версий QGIS</translation>
    </message>
    <message>
        <source>Ctrl-F</source>
        <comment>Toggle fullscreen mode</comment>
        <translation></translation>
    </message>
    <message>
        <source>Toggle fullscreen mode</source>
        <translation>Полноэкранный режим</translation>
    </message>
    <message>
        <source>Resource Location Error</source>
        <translation>Ошибка поиска ресурса</translation>
    </message>
    <message>
        <source>Error reading icon resources from: 
 %1
 Quitting...</source>
        <translation>Ошибка загрузки значков из: 
 %1
 Выход...</translation>
    </message>
    <message>
        <source>Overview</source>
        <translation>Обзор</translation>
    </message>
    <message>
        <source>Legend</source>
        <translation>Легенда</translation>
    </message>
    <message>
        <source>You are using QGIS version %1 built against code revision %2.</source>
        <translation>Версия QGIS: %1, ревизия: %2.</translation>
    </message>
    <message>
        <source> This copy of QGIS has been built with PostgreSQL support.</source>
        <translation> Данная версия QGIS собрана с поддержкой PostgreSQL.</translation>
    </message>
    <message>
        <source> This copy of QGIS has been built without PostgreSQL support.</source>
        <translation> Данная версия QGIS собрана без поддержки PostgreSQL.</translation>
    </message>
    <message>
        <source>
This binary was compiled against Qt %1,and is currently running against Qt %2</source>
        <translation>
Версия Qt, используемая при сборке: %1. Текущая версия Qt: %2</translation>
    </message>
    <message>
        <source>Stop map rendering</source>
        <translation>Остановить отрисовку</translation>
    </message>
    <message>
        <source>Multiple Instances of QgisApp</source>
        <translation>Множество экземпляров QgisApp</translation>
    </message>
    <message>
        <source>Multiple instances of Quantum GIS application object detected.
Please contact the developers.
</source>
        <translation>Обнаружено более одного экземпляра приложения Quantum GIS.
Пожалуйста, свяжитесь с разработчиками.
</translation>
    </message>
    <message>
        <source>Custom CRS...</source>
        <translation>Ввод системы координат...</translation>
    </message>
    <message>
        <source>Manage custom coordinate reference systems</source>
        <translation>Управление пользовательскими системами координат</translation>
    </message>
    <message>
        <source>Toggle extents and mouse position display</source>
        <translation>Переключить вывод границ или позиции курсора</translation>
    </message>
    <message>
        <source>This icon shows whether on the fly coordinate reference system transformation is enabled or not. Click the icon to bring up the project properties dialog to alter this behaviour.</source>
        <translation>Этот значок показывает, было ли включено преобразование координат «на лету». Щёлкните по значку, чтобы открыть диалог свойств проекта и изменить данный режим.</translation>
    </message>
    <message>
        <source>CRS status - Click to open coordinate reference system dialog</source>
        <translation>Преобразование координат — щёлкните для открытия диалога свойств системы координат</translation>
    </message>
    <message>
        <source>This release candidate includes over 60 bug fixes and enchancements over the QGIS 0.10.0 release. In addition we have added the following new features:</source>
        <translation>Эта версия включает более 60 исправлений ошибок и обновлений с момента выхода QGIS 0.10.0. Кроме того, мы добавили следующие возможности: </translation>
    </message>
    <message>
        <source>Revision of all dialogs for user interface consistancy</source>
        <translation>Все диалоги были переработаны по единому образцу</translation>
    </message>
    <message>
        <source>Improvements to unique value renderer vector dialog</source>
        <translation>Улучшен диалог ввода параметров для отрисовки уникальных значений</translation>
    </message>
    <message>
        <source>Symbol previews when defining vector classes</source>
        <translation>Добавлен предварительный просмотр условных знаков при определении векторных классов</translation>
    </message>
    <message>
        <source>Separation of python support into its own library</source>
        <translation>Поддержка Python выделена в отдельную библиотеку</translation>
    </message>
    <message>
        <source>List view and filter for GRASS toolbox to find tools more quickly</source>
        <translation>В диалоге инструментов GRASS реализован список и фильтр для быстрого поиска инструментов</translation>
    </message>
    <message>
        <source>List view and filter for Plugin Manager to find plugins more easily</source>
        <translation>В менеджере модулей реализован список и фильтр для быстрого поиска модулей</translation>
    </message>
    <message>
        <source>Updated Spatial Reference System definitions</source>
        <translation>Обновлены определения систем координат</translation>
    </message>
    <message>
        <source>QML Style support for rasters and database layers</source>
        <translation>Добавлена поддержка QML-стилей для растровых и PostGIS-слоёв</translation>
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
        <translation>Прекратить редактирование</translation>
    </message>
    <message>
        <source>Do you want to save the changes to layer %1?</source>
        <translation>Вы хотите сохранить изменения в слое %1?</translation>
    </message>
    <message>
        <source>Could not commit changes to layer %1

Errors:  %2
</source>
        <translation>Не удалось внести изменения в слой %1

Ошибка:  %2
</translation>
    </message>
    <message>
        <source>Problems during roll back</source>
        <translation>Ошибка в процессе отката</translation>
    </message>
    <message>
        <source>There was an error loading a plugin.The following diagnostic information may help the QGIS developers resolve the issue:
%1.</source>
        <translation>При загрузке модуля возникла ошибка. Информация ниже может помочь разработчикам QGIS решить эту проблему:
%1.</translation>
    </message>
    <message>
        <source>Map coordinates for the current view extents</source>
        <translation>Границы текущего окна в координатах карты</translation>
    </message>
    <message>
        <source>Maptips require an active layer</source>
        <translation>Для вывода всплывающих описаний необходим активный слой</translation>
    </message>
    <message>
        <source>Shift+Ctrl+S</source>
        <comment>Save Project under a new name</comment>
        <translation></translation>
    </message>
    <message>
        <source>&amp;Print Composer</source>
        <translation>Ко&amp;мпоновка карты</translation>
    </message>
    <message>
        <source>Ctrl+P</source>
        <comment>Print Composer</comment>
        <translation></translation>
    </message>
    <message>
        <source>Print Composer</source>
        <translation>Компоновка карты</translation>
    </message>
    <message>
        <source>&amp;Undo</source>
        <translation>&amp;Отменить</translation>
    </message>
    <message>
        <source>Ctrl+Z</source>
        <translation></translation>
    </message>
    <message>
        <source>Undo the last operation</source>
        <translation>Отменить последнюю операцию</translation>
    </message>
    <message>
        <source>Cu&amp;t</source>
        <translation>&amp;Вырезать</translation>
    </message>
    <message>
        <source>Ctrl+X</source>
        <translation></translation>
    </message>
    <message>
        <source>Cut the current selection&apos;s contents to the clipboard</source>
        <translation>Вырезать выделение в буфер обмена</translation>
    </message>
    <message>
        <source>&amp;Copy</source>
        <translation>&amp;Копировать</translation>
    </message>
    <message>
        <source>Ctrl+C</source>
        <translation></translation>
    </message>
    <message>
        <source>Copy the current selection&apos;s contents to the clipboard</source>
        <translation>Копировать выделение в буфер обмена</translation>
    </message>
    <message>
        <source>&amp;Paste</source>
        <translation>Вст&amp;авить</translation>
    </message>
    <message>
        <source>Ctrl+V</source>
        <translation></translation>
    </message>
    <message>
        <source>Paste the clipboard&apos;s contents into the current selection</source>
        <translation>Вставить содержимое буфера обмена в текущее выделение</translation>
    </message>
    <message>
        <source>M</source>
        <comment>Measure a Line</comment>
        <translation></translation>
    </message>
    <message>
        <source>J</source>
        <comment>Measure an Area</comment>
        <translation></translation>
    </message>
    <message>
        <source>Zoom to Selection</source>
        <translation>Увеличить до выделенного</translation>
    </message>
    <message>
        <source>Ctrl+J</source>
        <comment>Zoom to Selection</comment>
        <translation></translation>
    </message>
    <message>
        <source>Zoom Actual Size</source>
        <translation>Фактический размер</translation>
    </message>
    <message>
        <source>Zoom to Actual Size</source>
        <translation>Увеличить до фактического размера</translation>
    </message>
    <message>
        <source>Add Vector Layer...</source>
        <translation>Добавить векторный слой...</translation>
    </message>
    <message>
        <source>Add Raster Layer...</source>
        <translation>Добавить растровый слой...</translation>
    </message>
    <message>
        <source>Add PostGIS Layer...</source>
        <translation>Добавить слой PostGIS...</translation>
    </message>
    <message>
        <source>W</source>
        <comment>Add a Web Mapping Server Layer</comment>
        <translation>W</translation>
    </message>
    <message>
        <source>Add a Web Mapping Server Layer</source>
        <translation>Добавить слой с картографического веб-сервера</translation>
    </message>
    <message>
        <source>Open Attribute Table</source>
        <translation>Открыть таблицу атрибутов</translation>
    </message>
    <message>
        <source>Save as Shapefile...</source>
        <translation>Сохранить как shape-файл...</translation>
    </message>
    <message>
        <source>Save the current layer as a shapefile</source>
        <translation>Сохранить текущий слой в shape-файл</translation>
    </message>
    <message>
        <source>Save Selection as Shapefile...</source>
        <translation>Сохранить выделение как shape-файл...</translation>
    </message>
    <message>
        <source>Save the selection as a shapefile</source>
        <translation>Сохранить выделение в shape-файл</translation>
    </message>
    <message>
        <source>Properties...</source>
        <translation>Свойства...</translation>
    </message>
    <message>
        <source>Set properties of the current layer</source>
        <translation>Изменить свойства текущего слоя</translation>
    </message>
    <message>
        <source>Add to Overview</source>
        <translation>Добавить в обзор</translation>
    </message>
    <message>
        <source>Add All to Overview</source>
        <translation>Добавить все в обзор</translation>
    </message>
    <message>
        <source>Manage Plugins...</source>
        <translation>Управление модулями...</translation>
    </message>
    <message>
        <source>Toggle Full Screen Mode</source>
        <translation>Полноэкранный режим</translation>
    </message>
    <message>
        <source>Minimize</source>
        <translation>Свернуть</translation>
    </message>
    <message>
        <source>Ctrl+M</source>
        <comment>Minimize Window</comment>
        <translation></translation>
    </message>
    <message>
        <source>Minimizes the active window to the dock</source>
        <translation>Свернуть активное окно в док</translation>
    </message>
    <message>
        <source>Zoom</source>
        <translation>Увеличить</translation>
    </message>
    <message>
        <source>Toggles between a predefined size and the window size set by the user</source>
        <translation>Переключение между предопределённым и заданным пользователем размером окна</translation>
    </message>
    <message>
        <source>Bring All to Front</source>
        <translation>Все на передний план</translation>
    </message>
    <message>
        <source>Bring forward all open windows</source>
        <translation>Выдвинуть на передний план все открытые окна</translation>
    </message>
    <message>
        <source>&amp;Edit</source>
        <translation>&amp;Правка</translation>
    </message>
    <message>
        <source>Panels</source>
        <translation>Панели</translation>
    </message>
    <message>
        <source>Toolbars</source>
        <translation>Панели инструментов</translation>
    </message>
    <message>
        <source>&amp;Window</source>
        <translation>&amp;Окно</translation>
    </message>
    <message>
        <source>Choose a file name to save the QGIS project file as</source>
        <translation>Выберите имя файла для сохранения проекта QGIS</translation>
    </message>
    <message>
        <source>Choose a file name to save the map image as</source>
        <translation>Выберите имя файла для сохранения снимка карты</translation>
    </message>
    <message>
        <source>Python Console</source>
        <translation>Консоль Python</translation>
    </message>
    <message>
        <source></source>
        <translation></translation>
    </message>
</context>
<context>
    <name>QgisAppBase</name>
    <message>
        <source>QGIS</source>
        <translation></translation>
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
        <source>What&apos;s New</source>
        <translation>Что нового</translation>
    </message>
    <message>
        <source>http://www.gnu.org/licenses</source>
        <translation>http://www.gnu.org/licenses</translation>
    </message>
    <message>
        <source>Quantum GIS is licensed under the GNU General Public License</source>
        <translation>Quantum GIS выпускается под Стандартной Общественной Лицензией GNU</translation>
    </message>
    <message>
        <source>QGIS Home Page</source>
        <translation>Веб-сайт QGIS</translation>
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
        <source>Name</source>
        <translation>Имя</translation>
    </message>
    <message>
        <source>Sponsors</source>
        <translation>Спонсоры</translation>
    </message>
    <message>
        <source>Website</source>
        <translation>Веб-сайт</translation>
    </message>
    <message>
        <source>&lt;p&gt;The following have sponsored QGIS by contributing money to fund development and other project costs&lt;/p&gt;</source>
        <translation>&lt;p&gt;Эти люди спонсировали QGIS, вкладывая деньги в разработку и покрытие иных расходов проекта&lt;/p&gt;</translation>
    </message>
    <message>
        <source>Available QGIS Data Provider Plugins</source>
        <translation>Доступные модули источников данных QGIS</translation>
    </message>
    <message>
        <source>Available Qt Database Plugins</source>
        <translation>Доступные модули источников данных Qt</translation>
    </message>
    <message>
        <source>Available Qt Image Plugins</source>
        <translation>Доступные модули Qt для загрузки изображений</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Lucida Grande&apos;; font-size:13pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:16px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:x-large; font-weight:600;&quot;&gt;&lt;span style=&quot; font-size:x-large;&quot;&gt;Quantum GIS (QGIS)&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation></translation>
    </message>
    <message>
        <source>Join our user mailing list</source>
        <translation>Список рассылки для пользователей</translation>
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
</context>
<context>
    <name>QgsAttributeActionDialog</name>
    <message>
        <source>Select an action</source>
        <comment>File dialog window title</comment>
        <translation>Выберите действие</translation>
    </message>
</context>
<context>
    <name>QgsAttributeActionDialogBase</name>
    <message>
        <source>Name</source>
        <translation>Имя</translation>
    </message>
    <message>
        <source>Action</source>
        <translation>Действие</translation>
    </message>
    <message>
        <source>This list contains all actions that have been defined for the current layer. Add actions by entering the details in the controls below and then pressing the Insert action button. Actions can be edited here by double clicking on the item.</source>
        <translation>Этот список содержит действия, определённые для текущего слоя. Чтобы добавить новое действие, заполните соответствующие поля и нажмите «Вставить действие». Чтобы изменить действие, дважды щёлкните на нём в этом списке.</translation>
    </message>
    <message>
        <source>Move up</source>
        <translation>Передвинуть вверх</translation>
    </message>
    <message>
        <source>Move the selected action up</source>
        <translation>Переместить выбранное действие выше</translation>
    </message>
    <message>
        <source>Move down</source>
        <translation>Передвинуть вниз</translation>
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
        <source>Enter the name of an action here. The name should be unique (qgis will make it unique if necessary).</source>
        <translation>Поле ввода имени действия. Имя должно быть уникальным (qgis сделает его уникальным при необходимости).</translation>
    </message>
    <message>
        <source>Enter the action name here</source>
        <translation>Введите имя действия</translation>
    </message>
    <message>
        <source>Enter the action command here</source>
        <translation>Введите команду действия</translation>
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
        <translation>Вставить в действие выбранное поле с предшествующим %</translation>
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
        <translation>Захватывать вывод команды действия</translation>
    </message>
    <message>
        <source>Captures the standard output or error generated by the action and displays it in a dialog box</source>
        <translation>Захватывать вывод или ошибки действия и выводить их в диалоговом окне</translation>
    </message>
    <message>
        <source>Enter the action here. This can be any program, script or command that is available on your system. When the action is invoked any set of characters that start with a % and then have the name of a field will be replaced by the value of that field. The special characters %% will be replaced by the value of the field that was selected. Double quote marks group text into single arguments to the program, script or command. Double quotes will be ignored if preceeded by a backslash</source>
        <translation>Данное поле предназначено для ввода действия. Действием может быть любая программа, сценарий или команда, доступная в вашей системе. Когда действие выполняется, любые последовательности, начинающиеся со знака % и следующим за ним именем поля, будут заменены на значение этого поля. Специальные символы %% будут заменены на значение выбранного поля. Двойные кавычки позволяют группировать текст в единый аргумент программы, сценария или команды. Двойные кавычки, перед которыми следует \, будут проигнорированы</translation>
    </message>
    <message>
        <source>Attribute Actions</source>
        <translation>Действия с атрибутами</translation>
    </message>
    <message>
        <source>Action properties</source>
        <translation>Свойства действия</translation>
    </message>
    <message>
        <source>Browse for action</source>
        <translation>Поиск действия</translation>
    </message>
    <message>
        <source>Click to browse for an action</source>
        <translation>Щелкните для поиска команды действия</translation>
    </message>
    <message>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <source>Capture</source>
        <translation>Захватывать</translation>
    </message>
    <message>
        <source>Clicking the button will let you select an application to use as the action</source>
        <translation>Эта кнопка позволяет найти приложение, используемое как действие</translation>
    </message>
</context>
<context>
    <name>QgsAttributeDialog</name>
    <message>
        <source> (int)</source>
        <translation> (целое)</translation>
    </message>
    <message>
        <source> (dbl)</source>
        <translation> (действ.)</translation>
    </message>
    <message>
        <source> (txt)</source>
        <translation> (текст.)</translation>
    </message>
    <message>
        <source>...</source>
        <translation type="unfinished">...</translation>
    </message>
    <message>
        <source>Select a file</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsAttributeDialogBase</name>
    <message>
        <source>Enter Attribute Values</source>
        <translation>Введите значения атрибутов</translation>
    </message>
</context>
<context>
    <name>QgsAttributeTable</name>
    <message>
        <source>Run action</source>
        <translation>Выполнить действие</translation>
    </message>
</context>
<context>
    <name>QgsAttributeTableBase</name>
    <message>
        <source>Attribute Table</source>
        <translation>Таблица атрибутов</translation>
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
        <translation>Переместить выделенное в начало</translation>
    </message>
    <message>
        <source>Remove selection</source>
        <translation>Удалить выделение</translation>
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
        <source>in</source>
        <translation>в</translation>
    </message>
    <message>
        <source>Search</source>
        <translation>Поиск</translation>
    </message>
    <message>
        <source>Adva&amp;nced...</source>
        <translation>&amp;Дополнительно...</translation>
    </message>
    <message>
        <source>Alt+N</source>
        <translation></translation>
    </message>
    <message>
        <source>Zoom map to the selected rows</source>
        <translation>Увеличить карту до выбранных строк</translation>
    </message>
    <message>
        <source>Search for</source>
        <translation>Искать</translation>
    </message>
    <message>
        <source>Toggle editing mode</source>
        <translation>Режим редактирования</translation>
    </message>
    <message>
        <source>Click to toggle table editing</source>
        <translation>Переключить редактирование таблицы</translation>
    </message>
    <message>
        <source>Zoom map to the selected rows (Ctrl-J)</source>
        <translation>Увеличить карту до выбранных строк (Ctrl-J)</translation>
    </message>
    <message>
        <source>Ctrl+J</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>QgsAttributeTableDisplay</name>
    <message>
        <source>select</source>
        <translation>выбрать</translation>
    </message>
    <message>
        <source>select and bring to top</source>
        <translation>выбрать и переместить в начало</translation>
    </message>
    <message>
        <source>show only matching</source>
        <translation>только показать соответствия</translation>
    </message>
    <message>
        <source>Search string parsing error</source>
        <translation>Ошибка разбора поискового запроса</translation>
    </message>
    <message>
        <source>Search results</source>
        <translation>Результаты поиска</translation>
    </message>
    <message>
        <source>You&apos;ve supplied an empty search string.</source>
        <translation>Вы ввели пустой поисковый запрос.</translation>
    </message>
    <message>
        <source>Error during search</source>
        <translation>Ошибка во время поиска</translation>
    </message>
    <message>
        <source>Found %d matching features.</source>
        <translation type="obsolete">Найдено %d подходящих объектов.
        </translation>
    </message>
    <message>
        <source>No matching features found.</source>
        <translation>Подходящих объектов не найдено.</translation>
    </message>
    <message>
        <source>Attribute table - </source>
        <translation>Таблица атрибутов — </translation>
    </message>
    <message>
        <source>bad_alloc exception</source>
        <translation>Исключение bad_alloc</translation>
    </message>
    <message>
        <source>Filling the attribute table has been stopped because there was no more virtual memory left</source>
        <translation>Заполнение таблицы атрибутов остановлено, поскольку закончилась виртуальная память</translation>
    </message>
    <message>
        <source>QGIS</source>
        <translation></translation>
    </message>
    <message>
        <source>File</source>
        <translation>Файл</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
    <message>
        <source>Ctrl+W</source>
        <translation></translation>
    </message>
    <message>
        <source>Edit</source>
        <translation>Правка</translation>
    </message>
    <message>
        <source>&amp;Undo</source>
        <translation>&amp;Отменить</translation>
    </message>
    <message>
        <source>Ctrl+Z</source>
        <translation></translation>
    </message>
    <message>
        <source>Cu&amp;t</source>
        <translation>&amp;Вырезать</translation>
    </message>
    <message>
        <source>Ctrl+X</source>
        <translation></translation>
    </message>
    <message>
        <source>&amp;Copy</source>
        <translation>&amp;Копировать</translation>
    </message>
    <message>
        <source>Ctrl+C</source>
        <translation></translation>
    </message>
    <message>
        <source>&amp;Paste</source>
        <translation>Вст&amp;авить</translation>
    </message>
    <message>
        <source>Ctrl+V</source>
        <translation></translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>Удалить</translation>
    </message>
    <message>
        <source>Layer</source>
        <translation>Слой</translation>
    </message>
    <message>
        <source>Zoom to Selection</source>
        <translation>Увеличить до выделенного</translation>
    </message>
    <message>
        <source>Ctrl+J</source>
        <translation></translation>
    </message>
    <message>
        <source>Toggle Editing</source>
        <translation>Режим редактирования</translation>
    </message>
    <message>
        <source>Table</source>
        <translation>Таблица</translation>
    </message>
    <message>
        <source>Move to Top</source>
        <translation>Переместить в начало</translation>
    </message>
    <message>
        <source>Invert</source>
        <translation>Обратить</translation>
    </message>
</context>
<context>
    <name>QgsBookmarks</name>
    <message>
        <source>Really Delete?</source>
        <translation>Действительно удалить?</translation>
    </message>
    <message>
        <source>Are you sure you want to delete the </source>
        <translation>Вы уверены, что хотите удалить закладку </translation>
    </message>
    <message>
        <source> bookmark?</source>
        <translation>?</translation>
    </message>
    <message>
        <source>Error deleting bookmark</source>
        <translation>Ошибка удаления закладки</translation>
    </message>
    <message>
        <source>Failed to delete the </source>
        <translation>Не удалось удалить закладку </translation>
    </message>
    <message>
        <source> bookmark from the database. The database said:
</source>
        <translation> из базы данных. Сообщение базы данных:
</translation>
    </message>
    <message>
        <source>&amp;Delete</source>
        <translation>&amp;Удалить</translation>
    </message>
    <message>
        <source>&amp;Zoom to</source>
        <translation>&amp;Увеличить до</translation>
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
        <translation>ID</translation>
    </message>
</context>
<context>
    <name>QgsComposer</name>
    <message>
        <source> for read/write</source>
        <translation> для чтения/записи</translation>
    </message>
    <message>
        <source>Error in Print</source>
        <translation>Ошибка печати</translation>
    </message>
    <message>
        <source>Cannot seek</source>
        <translation>Ошибка позиционирования</translation>
    </message>
    <message>
        <source>Cannot overwrite BoundingBox</source>
        <translation>Не удаётся перезаписать границы</translation>
    </message>
    <message>
        <source>Cannot find BoundingBox</source>
        <translation>Не удаётся найти границы</translation>
    </message>
    <message>
        <source>Cannot overwrite translate</source>
        <translation>Не удаётся перезаписать преобразование</translation>
    </message>
    <message>
        <source>Cannot find translate</source>
        <translation>Не удаётся найти преобразование</translation>
    </message>
    <message>
        <source>File IO Error</source>
        <translation>Ошибка чтения/записи файла</translation>
    </message>
    <message>
        <source>Paper does not match</source>
        <translation>Несовпадение размера бумаги</translation>
    </message>
    <message>
        <source>The selected paper size does not match the composition size</source>
        <translation>Выбранный размер бумаги не совпадает с размером композиции</translation>
    </message>
    <message>
        <source>Big image</source>
        <translation>Большое изображение</translation>
    </message>
    <message>
        <source>To create image </source>
        <translation>Для создания изображения </translation>
    </message>
    <message>
        <source> requires circa </source>
        <translation> требуется приблизительно </translation>
    </message>
    <message>
        <source> MB of memory</source>
        <translation> МБ памяти</translation>
    </message>
    <message>
        <source>QGIS - print composer</source>
        <translation>QGIS — компоновка карты</translation>
    </message>
    <message>
        <source>Map 1</source>
        <translation>Карта 1</translation>
    </message>
    <message>
        <source>Couldn&apos;t open </source>
        <translation>Не удалось открыть </translation>
    </message>
    <message>
        <source>format</source>
        <translation>формат</translation>
    </message>
    <message>
        <source>SVG warning</source>
        <translation>Предупреждение SVG</translation>
    </message>
    <message>
        <source>Don&apos;t show this message again</source>
        <translation>Не показывать это сообщение в дальнейшем</translation>
    </message>
    <message>
        <source>SVG Format</source>
        <translation>Формат SVG</translation>
    </message>
    <message>
        <source>&lt;p&gt;The SVG export function in Qgis has several problems due to bugs and deficiencies in the </source>
        <translation type="obsolete">&lt;p&gt;Функция SVG-экспорта в QGIS может работать неправильно из-за ошибок в </translation>
    </message>
    <message>
        <source>Move Content</source>
        <translation>Переместить содержимое элемента</translation>
    </message>
    <message>
        <source>Move item content</source>
        <translation>Переместить содержимое элемента</translation>
    </message>
    <message>
        <source>&amp;Group</source>
        <translation>С&amp;группировать</translation>
    </message>
    <message>
        <source>Group items</source>
        <translation>Сгруппировать элементы</translation>
    </message>
    <message>
        <source>&amp;Ungroup</source>
        <translation>&amp;Разгруппировать</translation>
    </message>
    <message>
        <source>Ungroup items</source>
        <translation>Разгруппировать элементы</translation>
    </message>
    <message>
        <source>Raise</source>
        <translation>Поднять</translation>
    </message>
    <message>
        <source>Raise selected items</source>
        <translation>Поднять выбранные элементы</translation>
    </message>
    <message>
        <source>Lower</source>
        <translation>Опустить</translation>
    </message>
    <message>
        <source>Lower selected items</source>
        <translation>Опустить выбранные элементы</translation>
    </message>
    <message>
        <source>Bring to Front</source>
        <translation>На передний план</translation>
    </message>
    <message>
        <source>Move selected items to top</source>
        <translation>Поднять выделенные элементы на передний план</translation>
    </message>
    <message>
        <source>Send to Back</source>
        <translation>На задний план</translation>
    </message>
    <message>
        <source>Move selected items to bottom</source>
        <translation>Опустить выделенные элементы на задний план</translation>
    </message>
    <message>
        <source>QGIS</source>
        <translation></translation>
    </message>
    <message>
        <source>File</source>
        <translation>Файл</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
    <message>
        <source>Ctrl+W</source>
        <translation></translation>
    </message>
    <message>
        <source>Edit</source>
        <translation>Правка</translation>
    </message>
    <message>
        <source>&amp;Undo</source>
        <translation>&amp;Отменить</translation>
    </message>
    <message>
        <source>Ctrl+Z</source>
        <translation></translation>
    </message>
    <message>
        <source>Cu&amp;t</source>
        <translation>&amp;Вырезать</translation>
    </message>
    <message>
        <source>Ctrl+X</source>
        <translation></translation>
    </message>
    <message>
        <source>&amp;Copy</source>
        <translation>&amp;Копировать</translation>
    </message>
    <message>
        <source>Ctrl+C</source>
        <translation></translation>
    </message>
    <message>
        <source>&amp;Paste</source>
        <translation>Вст&amp;авить</translation>
    </message>
    <message>
        <source>Ctrl+V</source>
        <translation></translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>Удалить</translation>
    </message>
    <message>
        <source>View</source>
        <translation>Вид</translation>
    </message>
    <message>
        <source>Layout</source>
        <translation>Компоновка</translation>
    </message>
    <message>
        <source>Choose a file name to save the map image as</source>
        <translation>Выберите имя файла для сохранения снимка карты</translation>
    </message>
    <message>
        <source>Choose a file name to save the map as</source>
        <translation>Выберите имя файла для сохранения карты</translation>
    </message>
</context>
<context>
    <name>QgsComposerBase</name>
    <message>
        <source>General</source>
        <translation>Общие</translation>
    </message>
    <message>
        <source>Composition</source>
        <translation>Композиция</translation>
    </message>
    <message>
        <source>Item</source>
        <translation>Элемент</translation>
    </message>
    <message>
        <source>Save Template &amp;As...</source>
        <translation>Сохранить шаблон &amp;как...</translation>
    </message>
    <message>
        <source>&amp;Print...</source>
        <translation>&amp;Печать...</translation>
    </message>
    <message>
        <source>Add new map</source>
        <translation>Добавить карту</translation>
    </message>
    <message>
        <source>Add new label</source>
        <translation>Добавить текст</translation>
    </message>
    <message>
        <source>Add new vect legend</source>
        <translation>Добавить легенду</translation>
    </message>
    <message>
        <source>Select/Move item</source>
        <translation>Выбрать/переместить элемент</translation>
    </message>
    <message>
        <source>Add new scalebar</source>
        <translation>Добавить масштабную линейку</translation>
    </message>
    <message>
        <source>Refresh view</source>
        <translation>Обновить</translation>
    </message>
    <message>
        <source>MainWindow</source>
        <translation></translation>
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
        <translation>Добавить изображение</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
    <message>
        <source>Help</source>
        <translation>Справка</translation>
    </message>
    <message>
        <source>&amp;Open Template...</source>
        <translation>&amp;Открыть шаблон...</translation>
    </message>
    <message>
        <source>Zoom Full</source>
        <translation>Полный экстент</translation>
    </message>
    <message>
        <source>Add Map</source>
        <translation>Добавить карту</translation>
    </message>
    <message>
        <source>Add Label</source>
        <translation>Добавить текст</translation>
    </message>
    <message>
        <source>Add Vector Legend</source>
        <translation>Добавить легенду</translation>
    </message>
    <message>
        <source>Move Item</source>
        <translation>Переместить элемент</translation>
    </message>
    <message>
        <source>Export as Image...</source>
        <translation>Экспорт в изображение...</translation>
    </message>
    <message>
        <source>Export as SVG...</source>
        <translation>Экспорт в SVG...</translation>
    </message>
    <message>
        <source>Add Scalebar</source>
        <translation>Добавить масштабную линейку</translation>
    </message>
    <message>
        <source>Refresh</source>
        <translation>Обновить</translation>
    </message>
</context>
<context>
    <name>QgsComposerItemWidgetBase</name>
    <message>
        <source>Form</source>
        <translation></translation>
    </message>
    <message>
        <source>Composer item properties</source>
        <translation>Свойства элемента</translation>
    </message>
    <message>
        <source>Color:</source>
        <translation>Цвет:</translation>
    </message>
    <message>
        <source>Frame...</source>
        <translation>Рамки...</translation>
    </message>
    <message>
        <source>Background...</source>
        <translation>Фона...</translation>
    </message>
    <message>
        <source>Opacity:</source>
        <translation>Непрозрачность:</translation>
    </message>
    <message>
        <source>Outline width: </source>
        <translation>Ширина контура: </translation>
    </message>
    <message>
        <source>Frame</source>
        <translation>Рамка</translation>
    </message>
</context>
<context>
    <name>QgsComposerLabelWidgetBase</name>
    <message>
        <source>Label Options</source>
        <translation>Параметры текста</translation>
    </message>
    <message>
        <source>Font</source>
        <translation>Шрифт</translation>
    </message>
    <message>
        <source>Margin (mm):</source>
        <translation>Поле (мм):</translation>
    </message>
</context>
<context>
    <name>QgsComposerLegendItemDialogBase</name>
    <message>
        <source>Legend item properties</source>
        <translation>Свойства элемента легенды</translation>
    </message>
    <message>
        <source>Item text:</source>
        <translation>Текст элемента:</translation>
    </message>
</context>
<context>
    <name>QgsComposerLegendWidgetBase</name>
    <message>
        <source>Barscale Options</source>
        <translation>Параметры масштабной линейки</translation>
    </message>
    <message>
        <source>General</source>
        <translation>Общие</translation>
    </message>
    <message>
        <source>Title:</source>
        <translation>Заголовок:</translation>
    </message>
    <message>
        <source>Font:</source>
        <translation>Шрифт:</translation>
    </message>
    <message>
        <source>Title...</source>
        <translation>Заглавия...</translation>
    </message>
    <message>
        <source>Layer...</source>
        <translation>Слоя...</translation>
    </message>
    <message>
        <source>Item...</source>
        <translation>Элемента...</translation>
    </message>
    <message>
        <source>Symbol width: </source>
        <translation>Ширина знака: </translation>
    </message>
    <message>
        <source>Symbol height:</source>
        <translation>Высота знака: </translation>
    </message>
    <message>
        <source>Layer space: </source>
        <translation>Отступ слоя: </translation>
    </message>
    <message>
        <source>Symbol space:</source>
        <translation>Отступ знака:</translation>
    </message>
    <message>
        <source>Icon label space:</source>
        <translation>Отступ текста:</translation>
    </message>
    <message>
        <source>Box space:</source>
        <translation>Отступ рамки:</translation>
    </message>
    <message>
        <source>Legend items</source>
        <translation>Элементы легенды</translation>
    </message>
    <message>
        <source>down</source>
        <translation>вниз</translation>
    </message>
    <message>
        <source>up</source>
        <translation>вверх</translation>
    </message>
    <message>
        <source>remove</source>
        <translation>удалить</translation>
    </message>
    <message>
        <source>edit...</source>
        <translation>правка...</translation>
    </message>
    <message>
        <source>update</source>
        <translation>обновить</translation>
    </message>
    <message>
        <source>update all</source>
        <translation>обновить все</translation>
    </message>
</context>
<context>
    <name>QgsComposerMap</name>
    <message>
        <source>Map</source>
        <translation>Карта</translation>
    </message>
    <message>
        <source>Map will be printed here</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsComposerMapWidget</name>
    <message>
        <source>Cache</source>
        <translation>Кэш</translation>
    </message>
    <message>
        <source>Rectangle</source>
        <translation>Прямоугольник</translation>
    </message>
    <message>
        <source>Render</source>
        <translation>Отрисовка</translation>
    </message>
</context>
<context>
    <name>QgsComposerMapWidgetBase</name>
    <message>
        <source>Map options</source>
        <translation>Параметры карты</translation>
    </message>
    <message>
        <source>&lt;b&gt;Map&lt;/b&gt;</source>
        <translation>&lt;b&gt;Карта&lt;/b&gt;</translation>
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
        <source>Scale:</source>
        <translation>Масштаб:</translation>
    </message>
    <message>
        <source>1:</source>
        <translation>1:</translation>
    </message>
    <message>
        <source>Map extent</source>
        <translation>Границы карты</translation>
    </message>
    <message>
        <source>X min:</source>
        <translation>Мин. X:</translation>
    </message>
    <message>
        <source>Y min:</source>
        <translation>Мин. Y:</translation>
    </message>
    <message>
        <source>X max:</source>
        <translation>Макс. X:</translation>
    </message>
    <message>
        <source>Y max:</source>
        <translation>Макс. Y:</translation>
    </message>
    <message>
        <source>set to map canvas extent</source>
        <translation>взять с экрана</translation>
    </message>
    <message>
        <source>Preview</source>
        <translation>Предпросмотр</translation>
    </message>
    <message>
        <source>Update preview</source>
        <translation>Обновить</translation>
    </message>
</context>
<context>
    <name>QgsComposerPictureWidget</name>
    <message>
        <source>Select svg or image file</source>
        <translation>Выберите файл SVG или изображение</translation>
    </message>
</context>
<context>
    <name>QgsComposerPictureWidgetBase</name>
    <message>
        <source>Picture Options</source>
        <translation>Параметры изображения</translation>
    </message>
    <message>
        <source>Browse...</source>
        <translation>Обзор...</translation>
    </message>
    <message>
        <source>Width:</source>
        <translation>Ширина:</translation>
    </message>
    <message>
        <source>Height:</source>
        <translation>Высота:</translation>
    </message>
    <message>
        <source>Rotation:</source>
        <translation>Угол поворота:</translation>
    </message>
</context>
<context>
    <name>QgsComposerScaleBar</name>
    <message>
        <source>Single Box</source>
        <translation>Одинарная рамка</translation>
    </message>
    <message>
        <source>Double Box</source>
        <translation>Двойная рамка</translation>
    </message>
    <message>
        <source>Line Ticks Middle</source>
        <translation>Штрих вверх/вниз</translation>
    </message>
    <message>
        <source>Line Ticks Down</source>
        <translation>Штрих вниз</translation>
    </message>
    <message>
        <source>Line Ticks Up</source>
        <translation>Штрих вверх</translation>
    </message>
    <message>
        <source>Numeric</source>
        <translation>Числовой</translation>
    </message>
</context>
<context>
    <name>QgsComposerScaleBarWidget</name>
    <message>
        <source>Single Box</source>
        <translation>Одинарная рамка</translation>
    </message>
    <message>
        <source>Double Box</source>
        <translation>Двойная рамка</translation>
    </message>
    <message>
        <source>Line Ticks Middle</source>
        <translation>Штрих вверх/вниз</translation>
    </message>
    <message>
        <source>Line Ticks Down</source>
        <translation>Штрих вниз</translation>
    </message>
    <message>
        <source>Line Ticks Up</source>
        <translation>Штрих вверх</translation>
    </message>
    <message>
        <source>Numeric</source>
        <translation>Числовой</translation>
    </message>
    <message>
        <source>Map </source>
        <translation>Карта </translation>
    </message>
</context>
<context>
    <name>QgsComposerScaleBarWidgetBase</name>
    <message>
        <source>Barscale Options</source>
        <translation>Параметры масштабной линейки</translation>
    </message>
    <message>
        <source>Segment size (map units):</source>
        <translation>Размер сегмента (единицы карты):</translation>
    </message>
    <message>
        <source>Number of segments:</source>
        <translation>Количество сегментов:</translation>
    </message>
    <message>
        <source>Segments left:</source>
        <translation>Сегменты слева:</translation>
    </message>
    <message>
        <source>Style:</source>
        <translation>Стиль:</translation>
    </message>
    <message>
        <source>Unit label:</source>
        <translation>Обозначение единиц:</translation>
    </message>
    <message>
        <source>Map units per bar unit:</source>
        <translation>Единиц карты в делении:</translation>
    </message>
    <message>
        <source>Map:</source>
        <translation>Карта:</translation>
    </message>
    <message>
        <source>Height (mm):</source>
        <translation>Высота (мм):</translation>
    </message>
    <message>
        <source>Line width:</source>
        <translation>Ширина линии:</translation>
    </message>
    <message>
        <source>Label space:</source>
        <translation>Отступ метки:</translation>
    </message>
    <message>
        <source>Box space:</source>
        <translation>Отступ рамки:</translation>
    </message>
    <message>
        <source>Font...</source>
        <translation>Шрифт...</translation>
    </message>
    <message>
        <source>Color...</source>
        <translation>Цвет...</translation>
    </message>
</context>
<context>
    <name>QgsComposerVectorLegendBase</name>
    <message>
        <source>Vector Legend Options</source>
        <translation>Параметры векторной легенды</translation>
    </message>
    <message>
        <source>Title</source>
        <translation>Заглавие</translation>
    </message>
    <message>
        <source>Map</source>
        <translation>Карта</translation>
    </message>
    <message>
        <source>Font</source>
        <translation>Шрифт</translation>
    </message>
    <message>
        <source>Box</source>
        <translation>Рамка</translation>
    </message>
    <message>
        <source>Preview</source>
        <translation>Предпросмотр</translation>
    </message>
    <message>
        <source>Layers</source>
        <translation>Слои</translation>
    </message>
    <message>
        <source>Group</source>
        <translation>Группа</translation>
    </message>
    <message>
        <source>ID</source>
        <translation></translation>
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
</context>
<context>
    <name>QgsCompositionWidget</name>
    <message>
        <source>Landscape</source>
        <translation>Альбом</translation>
    </message>
    <message>
        <source>Portrait</source>
        <translation>Портрет</translation>
    </message>
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
</context>
<context>
    <name>QgsCompositionWidgetBase</name>
    <message>
        <source>Composition</source>
        <translation>Композиция</translation>
    </message>
    <message>
        <source>Paper</source>
        <translation>Бумага</translation>
    </message>
    <message>
        <source>Orientation</source>
        <translation>Ориентация</translation>
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
        <source>Units</source>
        <translation>Единицы</translation>
    </message>
    <message>
        <source>Size</source>
        <translation>Размер</translation>
    </message>
    <message>
        <source>Print quality (dpi)</source>
        <translation>Качество печати (dpi)</translation>
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
        <translation>Не удалось выполнить</translation>
    </message>
    <message>
        <source>transform of</source>
        <translation>преобразование</translation>
    </message>
    <message>
        <source>with error: </source>
        <translation>по причине: </translation>
    </message>
    <message>
        <source>The source spatial reference system (CRS) is not valid. </source>
        <translation>Неверная исходная система координат. </translation>
    </message>
    <message>
        <source>The coordinates can not be reprojected. The CRS is: </source>
        <translation>Не удалось спроецировать координаты. Система координат: </translation>
    </message>
    <message>
        <source>The destination spatial reference system (CRS) is not valid. </source>
        <translation>Неверная целевая система координат. </translation>
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
        <translation>&amp;Метка авторского права</translation>
    </message>
    <message>
        <source>Creates a copyright label that is displayed on the map canvas.</source>
        <translation>Создаёт значок авторского права в области карты.</translation>
    </message>
    <message>
        <source>&amp;Decorations</source>
        <translation>&amp;Оформление</translation>
    </message>
</context>
<context>
    <name>QgsCopyrightLabelPluginGuiBase</name>
    <message>
        <source>Copyright Label Plugin</source>
        <translation>Модуль метки авторского права</translation>
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
        <translation>Включить метку авторского права</translation>
    </message>
    <message>
        <source>Color</source>
        <translation>Цвет</translation>
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
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;Введите вашу метку авторского права в следующем поле. Для форматирования метки разрешается использовать базовую HTML-разметку. Например:&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;&lt;span style=&quot; font-weight:600;&quot;&gt;&amp;lt;B&amp;gt;Полужирный шрифт&amp;lt;/B&amp;gt; &lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:600;&quot;&gt;&lt;span style=&quot; font-weight:400; font-style:italic;&quot;&gt;&amp;lt;I&amp;gt;Курсив&amp;lt;/I&amp;gt;&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-style:italic;&quot;&gt;&lt;span style=&quot; font-style:normal;&quot;&gt;(примечание: значок &amp;copy; задаётся последовательностью «&amp;amp;copy;»)&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message encoding="UTF-8">
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
        <source>Delete Projection Definition?</source>
        <translation>Удалить определение проекции?</translation>
    </message>
    <message>
        <source>Deleting a projection definition is not reversable. Do you want to delete it?</source>
        <translation>Удаление определения проекции ‒ необратимая операция. Вы уверены, что хотите удалить его?</translation>
    </message>
    <message>
        <source>Abort</source>
        <translation>Отменить</translation>
    </message>
    <message>
        <source>New</source>
        <translation>Создать</translation>
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
        <source>This proj4 projection definition is not valid. Please correct before pressing save.</source>
        <translation>Неверное определение проекции proj4. Пожалуйста, исправьте его перед сохранением.</translation>
    </message>
    <message>
        <source>This proj4 projection definition is not valid.</source>
        <translation>Неверное определение проекции proj4.</translation>
    </message>
    <message>
        <source>Northing and Easthing must be in decimal form.</source>
        <translation>Север и восток следует вводить в десятичной форме.</translation>
    </message>
    <message>
        <source>Internal Error (source projection invalid?)</source>
        <translation>Внутренняя ошибка (неверная исходная проекция?)</translation>
    </message>
</context>
<context>
    <name>QgsCustomProjectionDialogBase</name>
    <message>
        <source>Define</source>
        <translation>Определение</translation>
    </message>
    <message>
        <source>|&lt;</source>
        <translation></translation>
    </message>
    <message>
        <source>&lt;</source>
        <translation></translation>
    </message>
    <message>
        <source>1 of 1</source>
        <translation>1 из 1</translation>
    </message>
    <message>
        <source>&gt;</source>
        <translation></translation>
    </message>
    <message>
        <source>&gt;|</source>
        <translation></translation>
    </message>
    <message>
        <source>Test</source>
        <translation>Проверка</translation>
    </message>
    <message>
        <source>Calculate</source>
        <translation>Расчитать</translation>
    </message>
    <message>
        <source>Geographic / WGS84</source>
        <translation>Географическая / WGS84</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Имя</translation>
    </message>
    <message>
        <source>Parameters</source>
        <translation>Параметры</translation>
    </message>
    <message>
        <source>*</source>
        <translation></translation>
    </message>
    <message>
        <source>S</source>
        <translation></translation>
    </message>
    <message>
        <source>X</source>
        <translation></translation>
    </message>
    <message>
        <source>North</source>
        <translation>Север</translation>
    </message>
    <message>
        <source>East</source>
        <translation>Восток</translation>
    </message>
    <message>
        <source>Custom Coordinate Reference System Definition</source>
        <translation>Определение пользовательской системы координат</translation>
    </message>
    <message>
        <source>You can define your own custom Coordinate Reference System (CRS) here. The definition must conform to the proj4 format for specifying a CRS.</source>
        <translation>В этом диалоге вы можете определить вашу собственную систему координат. Определение должно быть задано в формате координатных систем PROJ4.</translation>
    </message>
    <message>
        <source>Use the text boxes below to test the CRS definition you are creating. Enter a coordinate where both the lat/long and the transformed result are known (for example by reading off a map). Then press the calculate button to see if the CRS definition you are creating is accurate.</source>
        <translation>Используйте данные поля для проверки вновь созданной системы координат. Введите точку для которой известны широта/долгота и прямоугольные координаты (например, с карты). После этого нажмите кнопку «Расчитать» и проверьте, верно ли задана ваша система координат.</translation>
    </message>
    <message>
        <source>Destination CRS        </source>
        <translation>Целевая        </translation>
    </message>
</context>
<context>
    <name>QgsDbSourceSelect</name>
    <message>
        <source>Are you sure you want to remove the </source>
        <translation>Вы уверены, что хотите удалить соединение </translation>
    </message>
    <message>
        <source> connection and all associated settings?</source>
        <translation> и все связанные с ним параметры?</translation>
    </message>
    <message>
        <source>Confirm Delete</source>
        <translation>Потвердите удаление</translation>
    </message>
    <message>
        <source>Select Table</source>
        <translation>Выберите таблицу</translation>
    </message>
    <message>
        <source>You must select a table in order to add a Layer.</source>
        <translation>Для добавления слоя необходимо выбрать таблицу.</translation>
    </message>
    <message>
        <source>Password for </source>
        <translation>Пароль для </translation>
    </message>
    <message>
        <source>Please enter your password:</source>
        <translation>Пожалуйста, введите ваш пароль:</translation>
    </message>
    <message>
        <source>Connection failed</source>
        <translation>Не удалось соединиться</translation>
    </message>
    <message>
        <source>Type</source>
        <translation>Тип</translation>
    </message>
    <message>
        <source>Sql</source>
        <translation>SQL</translation>
    </message>
    <message>
        <source>Connection to %1 on %2 failed. Either the database is down or your settings are incorrect.%3Check your username and password and try again.%4The database said:%5%6</source>
        <translation>Не удалось подключиться к %1 на %2. Вероятно, база данных выключена, или же вы указали неверные параметры.%3Проверьте имя пользователя и пароль и попытайтесь снова.%4Сообщение БД:%5%6</translation>
    </message>
    <message>
        <source>Wildcard</source>
        <translation>Шаблон</translation>
    </message>
    <message>
        <source>RegExp</source>
        <translation>Рег. выражение</translation>
    </message>
    <message>
        <source>All</source>
        <translation>Все</translation>
    </message>
    <message>
        <source>Schema</source>
        <translation>Схема</translation>
    </message>
    <message>
        <source>Table</source>
        <translation>Таблица</translation>
    </message>
    <message>
        <source>Geometry column</source>
        <translation>Поле геометрии</translation>
    </message>
    <message>
        <source>Accessible tables could not be determined</source>
        <translation>Не удалось распознать доступные таблицы</translation>
    </message>
    <message>
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
        <source>No accessible tables found</source>
        <translation>Доступные таблицы не найдены</translation>
    </message>
    <message>
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
        <translation>Изменить</translation>
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
        <source>Search:</source>
        <translation>Поиск:</translation>
    </message>
    <message>
        <source>Search mode:</source>
        <translation>Режим поиска:</translation>
    </message>
    <message>
        <source>Search in columns:</source>
        <translation>Искать в полях:</translation>
    </message>
    <message>
        <source>Search options...</source>
        <translation>Поиск...</translation>
    </message>
</context>
<context>
    <name>QgsDbTableModel</name>
    <message>
        <source>Schema</source>
        <translation>Схема</translation>
    </message>
    <message>
        <source>Table</source>
        <translation>Таблица</translation>
    </message>
    <message>
        <source>Type</source>
        <translation>Тип</translation>
    </message>
    <message>
        <source>Geometry column</source>
        <translation>Поле геометрии</translation>
    </message>
    <message>
        <source>Sql</source>
        <translation>SQL</translation>
    </message>
    <message>
        <source>Point</source>
        <translation>Точка</translation>
    </message>
    <message>
        <source>Multipoint</source>
        <translation>Мультиточка</translation>
    </message>
    <message>
        <source>Line</source>
        <translation>Линия</translation>
    </message>
    <message>
        <source>Multiline</source>
        <translation>Мультилиния</translation>
    </message>
    <message>
        <source>Polygon</source>
        <translation>Полигон</translation>
    </message>
    <message>
        <source>Multipolygon</source>
        <translation>Мультиполигон</translation>
    </message>
</context>
<context>
    <name>QgsDelAttrDialogBase</name>
    <message>
        <source>Delete Attributes</source>
        <translation>Удалить атрибуты</translation>
    </message>
</context>
<context>
    <name>QgsDelimitedTextPlugin</name>
    <message>
        <source>&amp;Add Delimited Text Layer</source>
        <translation>Добавить слой из &amp;текста с разделителями</translation>
    </message>
    <message>
        <source>Add a delimited text file as a map layer. </source>
        <translation>Добавить текстовый файл с разделителями как слой карты.</translation>
    </message>
    <message>
        <source>The file must have a header row containing the field names. </source>
        <translation>Файл должен включать строку заголовка, содержащую имена полей.</translation>
    </message>
    <message>
        <source>X and Y fields are required and must contain coordinates in decimal units.</source>
        <translation>Поля X и Y обязательны и должны содержать координаты в десятичных единицах.</translation>
    </message>
    <message>
        <source>&amp;Delimited text</source>
        <translation>&amp;Текст с разделителями</translation>
    </message>
    <message>
        <source>DelimitedTextLayer</source>
        <translation>Слой текста с разделителями</translation>
    </message>
</context>
<context>
    <name>QgsDelimitedTextPluginGui</name>
    <message>
        <source>No layer name</source>
        <translation>Не указано имя слоя</translation>
    </message>
    <message>
        <source>Please enter a layer name before adding the layer to the map</source>
        <translation>Пожалуйста, введите имя слоя перед его добавлением к карте</translation>
    </message>
    <message>
        <source>No delimiter</source>
        <translation>Разделитель не указан</translation>
    </message>
    <message>
        <source>Please specify a delimiter prior to parsing the file</source>
        <translation>Пожалуйста, укажите разделитель до начала загрузки файла</translation>
    </message>
    <message>
        <source>Choose a delimited text file to open</source>
        <translation>Выберите текстовый файл с разделителями</translation>
    </message>
    <message>
        <source>Parse</source>
        <translation>Анализ</translation>
    </message>
    <message>
        <source>Description</source>
        <translation>Описание</translation>
    </message>
    <message>
        <source>Select a delimited text file containing a header row and one or more rows of x and y coordinates that you would like to use as a point layer and this plugin will do the job for you!</source>
        <translation>Выберите текстовый файл с разделителями, содержащий строку заголовка и строки, содержащие XY-координаты и этот модуль преобразует его в точечный слой!</translation>
    </message>
    <message>
        <source>Use the layer name box to specify the legend name for the new layer. Use the delimiter box to specify what delimeter is used in your file (e.g. space, comma, tab or a regular expression in Perl style). After choosing a delimiter, press the parse button and select the columns containing the x and y values for the layer.</source>
        <translation>Введите имя слоя в легенде в поле «имя слоя». В поле «разделитель» введите разделитель, используемый в вашем файле (пробел, запятая, TAB или регулярное выражение в стиле Python). После выбора разделителя, нажмите кнопку «Анализ» и выберите поля, содержащие координаты X и Y.</translation>
    </message>
</context>
<context>
    <name>QgsDelimitedTextPluginGuiBase</name>
    <message>
        <source>Create a Layer from a Delimited Text File</source>
        <translation>Создать слой из текста с разделителями</translation>
    </message>
    <message>
        <source>&lt;p align=&quot;right&quot;&gt;X field&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;right&quot;&gt;X-поле&lt;/p&gt;</translation>
    </message>
    <message>
        <source>Name of the field containing x values</source>
        <translation>Имя поля, содержащего X-значения</translation>
    </message>
    <message>
        <source>Name of the field containing x values. Choose a field from the list. The list is generated by parsing the header row of the delimited text file.</source>
        <translation>Имя поля, содержащего X-значения. Выберите поле из списка, создаваемого анализом строки заголовка в текстовом файле.</translation>
    </message>
    <message>
        <source>&lt;p align=&quot;right&quot;&gt;Y field&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;right&quot;&gt;Y-поле&lt;/p&gt;</translation>
    </message>
    <message>
        <source>Name of the field containing y values</source>
        <translation>Имя поля, содержащего Y-значения</translation>
    </message>
    <message>
        <source>Name of the field containing y values. Choose a field from the list. The list is generated by parsing the header row of the delimited text file.</source>
        <translation>Имя поля, содержащего Y-значения. Выберите поле из списка, создаваемого анализом строки заголовка в текстовом файле.</translation>
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
        <translation>Разделитель полей в текстовом файле. Разделитель может состоять из более чем одного символа.</translation>
    </message>
    <message>
        <source>Delimiter to use when splitting fields in the delimited text file. The delimiter can be 1 or more characters in length.</source>
        <translation>Разделитель полей в текстовом файле. Разделитель может состоять из одного и более символов.</translation>
    </message>
    <message>
        <source>Delimited Text Layer</source>
        <translation>Слой текста с разделителями</translation>
    </message>
    <message>
        <source>Delimited text file</source>
        <translation>Текстовый файл</translation>
    </message>
    <message>
        <source>Full path to the delimited text file</source>
        <translation>Полный путь к текстовому файлу</translation>
    </message>
    <message>
        <source>Full path to the delimited text file. In order to properly parse the fields in the file, the delimiter must be defined prior to entering the file name. Use the Browse button to the right of this field to choose the input file.</source>
        <translation>Полный путь к текстовому файлу. Для обеспечения правильности анализа файла, разделитель следует указывать перед вводом имени файла. Нажмите кнопку «Обзор» для интерактивного выбора файла.</translation>
    </message>
    <message>
        <source>Browse to find the delimited text file to be processed</source>
        <translation>Выбор текстового файла для обработки</translation>
    </message>
    <message>
        <source>Use this button to browse to the location of the delimited text file. This button will not be enabled until a delimiter has been entered in the &lt;i&gt;Delimiter&lt;/i&gt; box. Once a file is chosen, the X and Y field drop-down boxes will be populated with the fields from the delimited text file.</source>
        <translation>Используйте эту кнопку для выбора текстового файла. Кнопка не будет активирована, пока разделитель не будет введён в поле &lt;i&gt;Разделитель&lt;/i&gt;. После того, как файл будет выбран, списки X и Y-полей будут заполнены именами полей из текстового файла.</translation>
    </message>
    <message>
        <source>Sample text</source>
        <translation>Образец</translation>
    </message>
    <message>
        <source>Browse...</source>
        <translation>Обзор...</translation>
    </message>
    <message>
        <source>The delimiter is taken as is</source>
        <translation>Разделитель используется как есть</translation>
    </message>
    <message>
        <source>Plain characters</source>
        <translation>Простой текст</translation>
    </message>
    <message>
        <source>The delimiter is a regular expression</source>
        <translation>Разделитель является регулярным выражением</translation>
    </message>
    <message>
        <source>Regular expression</source>
        <translation>Регулярное выражение</translation>
    </message>
    <message>
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
        <source>Note: the following lines were not loaded because Qgis was unable to determine values for the x and y coordinates:
</source>
        <translation>Внимание: следующие строки не были загружены, потому что не удалось определить значения XY-координат:
</translation>
    </message>
    <message>
        <source>Error</source>
        <translation>Ошибка</translation>
    </message>
</context>
<context>
    <name>QgsDetailedItemWidgetBase</name>
    <message>
        <source>Form</source>
        <translation></translation>
    </message>
    <message>
        <source>Heading Label</source>
        <translation></translation>
    </message>
    <message>
        <source>Detail label</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>QgsDlgPgBufferBase</name>
    <message>
        <source>Buffer features</source>
        <translation>Буферизация объектов</translation>
    </message>
    <message>
        <source>Buffer distance in map units:</source>
        <translation>Зона буфера в единицах карты:</translation>
    </message>
    <message>
        <source>Table name for the buffered layer:</source>
        <translation>Имя таблицы для слоя буферных зон:</translation>
    </message>
    <message>
        <source>Create unique object id</source>
        <translation>Создавать уникальные ID объектов</translation>
    </message>
    <message>
        <source>public</source>
        <translation></translation>
    </message>
    <message>
        <source>Geometry column:</source>
        <translation>Поле геометрии:</translation>
    </message>
    <message>
        <source>Spatial reference ID:</source>
        <translation>ID системы координат (SRID):</translation>
    </message>
    <message>
        <source>Unique field to use as feature id:</source>
        <translation>Уникальное поле для ID объектов:</translation>
    </message>
    <message>
        <source>Schema:</source>
        <translation>Схема:</translation>
    </message>
    <message>
        <source>Add the buffered layer to the map?</source>
        <translation>Добавить на карту слой буферных зон?</translation>
    </message>
    <message>
        <source>&lt;h2&gt;Buffer the features in layer: &lt;/h2&gt;</source>
        <translation>&lt;h2&gt;Буферизация объектов слоя: &lt;/h2&gt;</translation>
    </message>
    <message>
        <source>Parameters</source>
        <translation>Параметры</translation>
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
    <name>QgsGPSDeviceDialog</name>
    <message>
        <source>New device %1</source>
        <translation>Новое устройство %1</translation>
    </message>
    <message>
        <source>Are you sure?</source>
        <translation>Вы уверены?</translation>
    </message>
    <message>
        <source>Are you sure that you want to delete this device?</source>
        <translation>Вы уверены, что хотите удалить это устройство?</translation>
    </message>
</context>
<context>
    <name>QgsGPSDeviceDialogBase</name>
    <message>
        <source>GPS Device Editor</source>
        <translation>Редактор GPS-устройств</translation>
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
        <translation>Команда, используемая для выгрузки треков в устройство</translation>
    </message>
    <message>
        <source>Track upload:</source>
        <translation>Выгрузка треков:</translation>
    </message>
    <message>
        <source>The command that is used to download tracks from the device</source>
        <translation>Команда, используемая для загрузки треков из устройства</translation>
    </message>
    <message>
        <source>The command that is used to upload routes to the device</source>
        <translation>Команда, используемая для выгрузки маршрутов в устройство</translation>
    </message>
    <message>
        <source>The command that is used to download routes from the device</source>
        <translation>Команда, используемая для загрузки маршрутов из устройства</translation>
    </message>
    <message>
        <source>The command that is used to upload waypoints to the device</source>
        <translation>Команда, используемая для выгрузки точек в устройство</translation>
    </message>
    <message>
        <source>The command that is used to download waypoints from the device</source>
        <translation>Команда, используемая для загрузки точек из устройства</translation>
    </message>
    <message>
        <source>Device name</source>
        <translation>Имя устройства</translation>
    </message>
    <message>
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
        <translation>Попробуйте ещё раз с другим именем или в другом </translation>
    </message>
    <message>
        <source>directory.</source>
        <translation>каталоге.</translation>
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
        <translation>Пожалуйста, выберите правильный файл.</translation>
    </message>
    <message>
        <source>Could not start process</source>
        <translation>Не удалось запустить процесс</translation>
    </message>
    <message>
        <source>Could not start GPSBabel!</source>
        <translation>Не удалось запустить GPSBabel!</translation>
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
        <translation>Ошибка импорта данных из %1!

</translation>
    </message>
    <message>
        <source>Error importing data</source>
        <translation>Ошибка импорта данных</translation>
    </message>
    <message>
        <source>Not supported</source>
        <translation>Функция не поддерживается</translation>
    </message>
    <message>
        <source>This device does not support downloading </source>
        <translation>Это устройство не поддерживает загрузку </translation>
    </message>
    <message>
        <source>of </source>
        <translation>данных типа </translation>
    </message>
    <message>
        <source>Downloading data...</source>
        <translation>Загрузка данных...</translation>
    </message>
    <message>
        <source>Could not download data from GPS!

</source>
        <translation>Ошибка загрузки данных из GPS!

</translation>
    </message>
    <message>
        <source>Error downloading data</source>
        <translation>Ошибка загрузки данных</translation>
    </message>
    <message>
        <source>This device does not support uploading of </source>
        <translation>Это устройство не поддерживает выгрузку данных типа  </translation>
    </message>
    <message>
        <source>Uploading data...</source>
        <translation>Выгрузка данных...</translation>
    </message>
    <message>
        <source>Error while uploading data to GPS!

</source>
        <translation>Ошибка выгрузки данных в GPS!

</translation>
    </message>
    <message>
        <source>Error uploading data</source>
        <translation>Ошибка выгрузки данных</translation>
    </message>
    <message>
        <source>Could not convert data from %1!

</source>
        <translation>Не удалось преобразовать данные из %1!</translation>
    </message>
    <message>
        <source>Error converting data</source>
        <translation>Ошибка преобразования данных</translation>
    </message>
</context>
<context>
    <name>QgsGPSPluginGui</name>
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
        <translation>Выберите файл и формат для импорта</translation>
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
        <source>QGIS can perform conversions of GPX files, by using GPSBabel (%1) to perform the conversions.</source>
        <translation>QGIS может выполнять преобразование GPX-файлов при помощи пакета GPSBabel (%1).</translation>
    </message>
    <message>
        <source>This requires that you have GPSBabel installed where QGIS can find it.</source>
        <translation>Для этого требуется установить пакет GPSBabel так, чтобы он мог быть найден QGIS.</translation>
    </message>
    <message>
        <source>GPX is the %1, which is used to store information about waypoints, routes, and tracks.</source>
        <translation>GPX (%1) — это формат, используемый для хранения маршрутных точек, маршрутов и треков.</translation>
    </message>
    <message>
        <source>GPS eXchange file format</source>
        <translation>Формат GPS eXchange</translation>
    </message>
    <message>
        <source>Select a GPX file and then select the feature types that you want to load.</source>
        <translation>Выберите GPX-файл и типы объектов, которые вы хотели бы загрузить.</translation>
    </message>
    <message>
        <source>This tool will help you download data from a GPS device.</source>
        <translation>Этот инструмент поможет вам загрузить данные с GPS-устройства.</translation>
    </message>
    <message>
        <source>Choose your GPS device, the port it is connected to, the feature type you want to download, a name for your new layer, and the GPX file where you want to store the data.</source>
        <translation>Выберите ваше GPS-устройство и порт, к которому оно подключено, а также тип объектов, которые вы хотите загрузить, имя нового слоя и GPX-файл для сохранения данных.</translation>
    </message>
    <message>
        <source>If your device isn&apos;t listed, or if you want to change some settings, you can also edit the devices.</source>
        <translation>Если вашего устройства нет в списке или вы хотите изменить его параметры, нажмите «Редактировать устройства».</translation>
    </message>
    <message>
        <source>This tool uses the program GPSBabel (%1) to transfer the data.</source>
        <translation>Этот инструмент использует GPSBabel (%1) для передачи данных.</translation>
    </message>
    <message>
        <source>This tool will help you upload data from a GPX layer to a GPS device.</source>
        <translation>Этот инстумент поможет вам выгрузить данные в GPS-устройство из существующего GPX-слоя.</translation>
    </message>
    <message>
        <source>Choose the layer you want to upload, the device you want to upload it to, and the port your device is connected to.</source>
        <translation>Выберите слой, который вы желаете выгрузить, устройство для выгрузки и порт, к которому оно подключено.</translation>
    </message>
    <message>
        <source>QGIS can only load GPX files by itself, but many other formats can be converted to GPX using GPSBabel (%1).</source>
        <translation>QGIS может загружать только GPX-файлы, но прочие форматы могут быть преобразованы в GPX при помощи GPSBabel (%1).</translation>
    </message>
    <message>
        <source>All file formats can not store waypoints, routes, and tracks, so some feature types may be disabled for some file formats.</source>
        <translation>Не все форматы могут содержать маршрутные точки, маршруты и треки, поэтому для некоторых форматов часть типов данных будет выключена.</translation>
    </message>
    <message>
        <source>Choose a file name to save under</source>
        <translation>Выберите имя сохраняемого файла</translation>
    </message>
    <message>
        <source>Select a GPS file format and the file that you want to import, the feature type that you want to use, a GPX file name that you want to save the converted file as, and a name for the new layer.</source>
        <translation>Выберите формат GPS-данных и файл для импорта, а также тип загружаемых объектов, имя GPX-файла, в который будет сохранён результат и имя нового слоя.</translation>
    </message>
    <message>
        <source>Select a GPX input file name, the type of conversion you want to perform, a GPX file name that you want to save the converted file as, and a name for the new layer created from the result.</source>
        <translation>Выберите исходный GPX-файл, тип преобразования, которое вы хотели бы осуществить, а также имя файла, в котором будет сохранён результат и имя нового слоя.</translation>
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
        <source>Browse...</source>
        <translation>Обзор...</translation>
    </message>
    <message>
        <source>Save As...</source>
        <translation>Сохранить как...</translation>
    </message>
    <message>
        <source>(Note: Selecting correct file type in browser dialog important!)</source>
        <translation>(Внимание: важно выбрать правильный тип файла в диалоге выбора файлов!)</translation>
    </message>
    <message>
        <source>GPX Conversions</source>
        <translation>Конвертеры GPX</translation>
    </message>
    <message>
        <source>Conversion:</source>
        <translation>Преобразование:</translation>
    </message>
    <message>
        <source>GPX input file:</source>
        <translation>Исходный GPX-файл:</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation></translation>
    </message>
    <message>
        <source>Edit devices...</source>
        <translation>Редактировать устройства...</translation>
    </message>
    <message>
        <source>Refresh</source>
        <translation>Обновить</translation>
    </message>
</context>
<context>
    <name>QgsGPXProvider</name>
    <message>
        <source>Bad URI - you need to specify the feature type.</source>
        <translation>Неверный URI — необходимо указать тип объектов.</translation>
    </message>
    <message>
        <source>GPS eXchange file</source>
        <translation>Файлы GPS eXchange (*.gpx)</translation>
    </message>
    <message>
        <source>Digitized in QGIS</source>
        <translation>Оцифрован в QGIS</translation>
    </message>
</context>
<context>
    <name>QgsGenericProjectionSelector</name>
    <message>
        <source>Define this layer&apos;s projection:</source>
        <translation>Укажите проекцию слоя:</translation>
    </message>
    <message>
        <source>This layer appears to have no projection specification.</source>
        <translation>Этот слой не содержит сведений о проекции.</translation>
    </message>
    <message>
        <source>By default, this layer will now have its projection set to that of the project, but you may override this by selecting a different projection below.</source>
        <translation>По умолчанию, для этого слоя будет выбрана проекция текущего проекта, но вы можете переопределить её, выбрав другую проекцию ниже.</translation>
    </message>
</context>
<context>
    <name>QgsGenericProjectionSelectorBase</name>
    <message>
        <source>Projection Selector</source>
        <translation>Выбор проекции</translation>
    </message>
</context>
<context>
    <name>QgsGeomTypeDialog</name>
    <message>
        <source>Real</source>
        <translation>Действительное число</translation>
    </message>
    <message>
        <source>Integer</source>
        <translation>Целое число</translation>
    </message>
    <message>
        <source>String</source>
        <translation>Строка</translation>
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
        <source>File format</source>
        <translation>Формат файла</translation>
    </message>
    <message>
        <source>Attributes</source>
        <translation>Атрибуты</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Имя</translation>
    </message>
    <message>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <source>Add attribute</source>
        <translation>Добавить атрибут</translation>
    </message>
    <message>
        <source>Delete selected attribute</source>
        <translation>Удалить выбранный атрибут</translation>
    </message>
</context>
<context>
    <name>QgsGeorefDescriptionDialogBase</name>
    <message>
        <source>Description georeferencer</source>
        <translation>Описание модуля привязки</translation>
    </message>
    <message>
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
        <source>&amp;Georeferencer</source>
        <translation>&amp;Привязка растров</translation>
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
        <translation>Привязка растров</translation>
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
        <translation>Выровнять окна модуля</translation>
    </message>
    <message>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <source>Description...</source>
        <translation>Описание...</translation>
    </message>
</context>
<context>
    <name>QgsGeorefWarpOptionsDialogBase</name>
    <message>
        <source>Warp options</source>
        <translation>Параметры преобразования</translation>
    </message>
    <message>
        <source>Resampling method:</source>
        <translation>Метод интерполяции:</translation>
    </message>
    <message>
        <source>Nearest neighbour</source>
        <translation>Ближайший сосед</translation>
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
        <translation>Использовать 0 для прозрачности при необходимости</translation>
    </message>
    <message>
        <source>Compression:</source>
        <translation>Сжатие:</translation>
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
        <translation>Пустые значения</translation>
    </message>
</context>
<context>
    <name>QgsGraduatedSymbolDialogBase</name>
    <message>
        <source>graduated Symbol</source>
        <translation>градуированный знак</translation>
    </message>
    <message>
        <source>Delete class</source>
        <translation>Удалить класс</translation>
    </message>
    <message>
        <source>Classify</source>
        <translation>Классифицировать</translation>
    </message>
    <message>
        <source>Classification field</source>
        <translation>Поле классификации</translation>
    </message>
    <message>
        <source>Mode</source>
        <translation>Режим</translation>
    </message>
    <message>
        <source>Number of classes</source>
        <translation>Количество классов</translation>
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
        <translation>Поле</translation>
    </message>
    <message>
        <source>Value</source>
        <translation>Значение</translation>
    </message>
    <message>
        <source>Type</source>
        <translation>Тип</translation>
    </message>
    <message>
        <source>ERROR</source>
        <translation>ОШИБКА</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>Layer</source>
        <translation>Слой</translation>
    </message>
</context>
<context>
    <name>QgsGrassAttributesBase</name>
    <message>
        <source>GRASS Attributes</source>
        <translation>Атрибуты GRASS</translation>
    </message>
    <message>
        <source>Tab 1</source>
        <translation></translation>
    </message>
    <message>
        <source>result</source>
        <translation>результат</translation>
    </message>
    <message>
        <source>Update</source>
        <translation>Обновить</translation>
    </message>
    <message>
        <source>Update database record</source>
        <translation>Обновить запись базы данных</translation>
    </message>
    <message>
        <source>New</source>
        <translation>Новая</translation>
    </message>
    <message>
        <source>Add new category using settings in GRASS Edit toolbox</source>
        <translation>Добавить новую категорию, используя параметры редактора GRASS</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>Удалить</translation>
    </message>
    <message>
        <source>Delete selected category</source>
        <translation>Удалить выбранную категорию</translation>
    </message>
</context>
<context>
    <name>QgsGrassBrowser</name>
    <message>
        <source>Tools</source>
        <translation>Инструменты</translation>
    </message>
    <message>
        <source>Add selected map to canvas</source>
        <translation>Добавить выбранную карту в область QGIS</translation>
    </message>
    <message>
        <source>Copy selected map</source>
        <translation>Копировать выбранную карту</translation>
    </message>
    <message>
        <source>Rename selected map</source>
        <translation>Переименовать выбранную карту</translation>
    </message>
    <message>
        <source>Delete selected map</source>
        <translation>Удалить выбранную карту</translation>
    </message>
    <message>
        <source>Set current region to selected map</source>
        <translation>Установить регион по границам выбранной карты</translation>
    </message>
    <message>
        <source>Refresh</source>
        <translation>Обновить</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Внимание</translation>
    </message>
    <message>
        <source>Cannot copy map </source>
        <translation>Не удалось скопировать карту </translation>
    </message>
    <message>
        <source>&lt;br&gt;command: </source>
        <translation>&lt;br&gt;команда: </translation>
    </message>
    <message>
        <source>Cannot rename map </source>
        <translation>Не удалось переименовать карту </translation>
    </message>
    <message>
        <source>Delete map &lt;b&gt;</source>
        <translation>Удалить карту &lt;b&gt;</translation>
    </message>
    <message>
        <source>Cannot delete map </source>
        <translation>Не удалось удалить карту </translation>
    </message>
    <message>
        <source>Cannot write new region</source>
        <translation>Не удалось сохранить новый регион</translation>
    </message>
    <message>
        <source>New name</source>
        <translation>Новое имя</translation>
    </message>
</context>
<context>
    <name>QgsGrassEdit</name>
    <message>
        <source>New point</source>
        <translation>Новая точка</translation>
    </message>
    <message>
        <source>New centroid</source>
        <translation>Новый центроид</translation>
    </message>
    <message>
        <source>Delete vertex</source>
        <translation>Удалить вершину</translation>
    </message>
    <message>
        <source>Left: </source>
        <translation>Левая: </translation>
    </message>
    <message>
        <source>Middle: </source>
        <translation>Средняя: </translation>
    </message>
    <message>
        <source>Edit tools</source>
        <translation>Инструменты редактора</translation>
    </message>
    <message>
        <source>New line</source>
        <translation>Новая линия</translation>
    </message>
    <message>
        <source>New boundary</source>
        <translation>Новая граница</translation>
    </message>
    <message>
        <source>Move vertex</source>
        <translation>Переместить вершину</translation>
    </message>
    <message>
        <source>Add vertex</source>
        <translation>Добавить вершину</translation>
    </message>
    <message>
        <source>Move element</source>
        <translation>Переместить элемент</translation>
    </message>
    <message>
        <source>Split line</source>
        <translation>Разделить линию</translation>
    </message>
    <message>
        <source>Delete element</source>
        <translation>Удалить элемент</translation>
    </message>
    <message>
        <source>Edit attributes</source>
        <translation>Изменить атрибуты</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Внимание</translation>
    </message>
    <message>
        <source>You are not owner of the mapset, cannot open the vector for editing.</source>
        <translation>Вы не являетесь владельцем набора, невозможно изменить векторный слой.</translation>
    </message>
    <message>
        <source>Cannot open vector for update.</source>
        <translation>Не удалось открыть векторный слой для обновления.</translation>
    </message>
    <message>
        <source>Info</source>
        <translation>Информация</translation>
    </message>
    <message>
        <source>The table was created</source>
        <translation>Таблица была создана</translation>
    </message>
    <message>
        <source>Tool not yet implemented.</source>
        <translation>Инструмент не реализован.</translation>
    </message>
    <message>
        <source>Cannot check orphan record: </source>
        <translation>Не удаётся проверить изолированную запись: </translation>
    </message>
    <message>
        <source>Orphan record was left in attribute table. &lt;br&gt;Delete the record?</source>
        <translation>В таблице атрибутов обнаружена изолированная запись. &lt;br/&gt;Удалить запись?</translation>
    </message>
    <message>
        <source>Cannot delete orphan record: </source>
        <translation>Не удаётся удалить изолированную запись: </translation>
    </message>
    <message>
        <source>Cannot describe table for field </source>
        <translation>Не удаётся описать таблицу для поля </translation>
    </message>
    <message>
        <source>Background</source>
        <translation>Фон</translation>
    </message>
    <message>
        <source>Highlight</source>
        <translation>Подсветка</translation>
    </message>
    <message>
        <source>Dynamic</source>
        <translation>Изменяемое</translation>
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
        <source>Boundary (no area)</source>
        <translation>Граница (нет площади)</translation>
    </message>
    <message>
        <source>Boundary (1 area)</source>
        <translation>Граница (1 площадь)</translation>
    </message>
    <message>
        <source>Boundary (2 areas)</source>
        <translation>Граница (2 площади)</translation>
    </message>
    <message>
        <source>Centroid (in area)</source>
        <translation>Центроид (в площади)</translation>
    </message>
    <message>
        <source>Centroid (outside area)</source>
        <translation>Центроид (за площадью)</translation>
    </message>
    <message>
        <source>Centroid (duplicate in area)</source>
        <translation>Центроид (дублированный)</translation>
    </message>
    <message>
        <source>Node (1 line)</source>
        <translation>Узел (1 линия)</translation>
    </message>
    <message>
        <source>Node (2 lines)</source>
        <translation>Узел (2 линии)</translation>
    </message>
    <message>
        <source>Disp</source>
        <comment>Column title</comment>
        <translation type="obsolete">Видимость</translation>
    </message>
    <message>
        <source>Color</source>
        <comment>Column title</comment>
        <translation type="obsolete">Цвет</translation>
    </message>
    <message>
        <source>Type</source>
        <comment>Column title</comment>
        <translation type="obsolete">Тип</translation>
    </message>
    <message>
        <source>Index</source>
        <comment>Column title</comment>
        <translation type="obsolete">Индекс</translation>
    </message>
    <message>
        <source>Column</source>
        <translation type="obsolete">Поле</translation>
    </message>
    <message>
        <source>Type</source>
        <translation type="obsolete">Тип</translation>
    </message>
    <message>
        <source>Length</source>
        <translation type="obsolete">Длина</translation>
    </message>
    <message>
        <source>Next not used</source>
        <translation>Следующая неиспользуемая</translation>
    </message>
    <message>
        <source>Manual entry</source>
        <translation>Ручной ввод</translation>
    </message>
    <message>
        <source>No category</source>
        <translation>Без категории</translation>
    </message>
    <message>
        <source>Right: </source>
        <translation>Правая: </translation>
    </message>
</context>
<context>
    <name>QgsGrassEditBase</name>
    <message>
        <source>GRASS Edit</source>
        <translation>Редактор GRASS</translation>
    </message>
    <message>
        <source>Category</source>
        <translation>Категории</translation>
    </message>
    <message>
        <source>Mode</source>
        <translation>Режим</translation>
    </message>
    <message>
        <source>Settings</source>
        <translation>Параметры</translation>
    </message>
    <message>
        <source>Snapping in screen pixels</source>
        <translation>Прилипание в пикселях экрана</translation>
    </message>
    <message>
        <source>Symbology</source>
        <translation>Символика</translation>
    </message>
    <message>
        <source>Column 1</source>
        <translation type="obsolete">Столбец 1</translation>
    </message>
    <message>
        <source>Table</source>
        <translation>Таблица</translation>
    </message>
    <message>
        <source>Add Column</source>
        <translation>Добавить столбец</translation>
    </message>
    <message>
        <source>Create / Alter Table</source>
        <translation>Создать / обновить таблицу</translation>
    </message>
    <message>
        <source>Line width</source>
        <translation>Ширина линии</translation>
    </message>
    <message>
        <source>Marker size</source>
        <translation>Размер маркера</translation>
    </message>
    <message>
        <source>Layer</source>
        <translation>Слой</translation>
    </message>
    <message>
        <source>Disp</source>
        <translation type="unfinished">Видимость</translation>
    </message>
    <message>
        <source>Color</source>
        <translation type="unfinished">Цвет</translation>
    </message>
    <message>
        <source>Type</source>
        <translation type="unfinished">Тип</translation>
    </message>
    <message>
        <source>Index</source>
        <translation type="unfinished">Индекс</translation>
    </message>
    <message>
        <source>Column</source>
        <translation type="unfinished">Поле</translation>
    </message>
    <message>
        <source>Length</source>
        <translation type="unfinished">Длина</translation>
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
        <translation>&lt;font color=&apos;red&apos;&gt;Введите имя!&lt;/font&gt;</translation>
    </message>
    <message>
        <source>&lt;font color=&apos;red&apos;&gt;This is name of the source!&lt;/font&gt;</source>
        <translation>&lt;font color=&apos;red&apos;&gt;Это имя источника!&lt;/font&gt;</translation>
    </message>
    <message>
        <source>&lt;font color=&apos;red&apos;&gt;Exists!&lt;/font&gt;</source>
        <translation>&lt;font color=&apos;red&apos;&gt;Уже существует!&lt;/font&gt;</translation>
    </message>
    <message>
        <source>Overwrite</source>
        <translation>Перезаписать</translation>
    </message>
</context>
<context>
    <name>QgsGrassMapcalc</name>
    <message>
        <source>Mapcalc tools</source>
        <translation>Инструменты Mapcalc</translation>
    </message>
    <message>
        <source>Add map</source>
        <translation>Добавить карту</translation>
    </message>
    <message>
        <source>Add constant value</source>
        <translation>Добавить постоянное значение</translation>
    </message>
    <message>
        <source>Add operator or function</source>
        <translation>Добавить оператор или функцию</translation>
    </message>
    <message>
        <source>Add connection</source>
        <translation>Добавить соединение</translation>
    </message>
    <message>
        <source>Select item</source>
        <translation>Выбрать элемент</translation>
    </message>
    <message>
        <source>Delete selected item</source>
        <translation>Удалить выбранный элемент</translation>
    </message>
    <message>
        <source>Open</source>
        <translation>Открыть</translation>
    </message>
    <message>
        <source>Save</source>
        <translation>Сохранить</translation>
    </message>
    <message>
        <source>Save as</source>
        <translation>Сохранить как</translation>
    </message>
    <message>
        <source>Addition</source>
        <translation>Сложение</translation>
    </message>
    <message>
        <source>Subtraction</source>
        <translation>Вычитание</translation>
    </message>
    <message>
        <source>Multiplication</source>
        <translation>Умножение</translation>
    </message>
    <message>
        <source>Division</source>
        <translation>Деление</translation>
    </message>
    <message>
        <source>Modulus</source>
        <translation>Остаток</translation>
    </message>
    <message>
        <source>Exponentiation</source>
        <translation>Возведение в степень</translation>
    </message>
    <message>
        <source>Equal</source>
        <translation>Равно</translation>
    </message>
    <message>
        <source>Not equal</source>
        <translation>Не равно</translation>
    </message>
    <message>
        <source>Greater than</source>
        <translation>Больше чем</translation>
    </message>
    <message>
        <source>Greater than or equal</source>
        <translation>Больше или равно</translation>
    </message>
    <message>
        <source>Less than</source>
        <translation>Меньше чем</translation>
    </message>
    <message>
        <source>Less than or equal</source>
        <translation>Меньше или равно</translation>
    </message>
    <message>
        <source>And</source>
        <translation>И</translation>
    </message>
    <message>
        <source>Or</source>
        <translation>Или</translation>
    </message>
    <message>
        <source>Absolute value of x</source>
        <translation>Абсолютное значение x</translation>
    </message>
    <message>
        <source>Inverse tangent of x (result is in degrees)</source>
        <translation>Арктангенс x (результат в градусах)</translation>
    </message>
    <message>
        <source>Inverse tangent of y/x (result is in degrees)</source>
        <translation>Арктангенс у/x (результат в градусах)</translation>
    </message>
    <message>
        <source>Current column of moving window (starts with 1)</source>
        <translation>Текущий столбец подвижного окна (начиная с 1)</translation>
    </message>
    <message>
        <source>Cosine of x (x is in degrees)</source>
        <translation>Косинус x (x в градусах)</translation>
    </message>
    <message>
        <source>Convert x to double-precision floating point</source>
        <translation>Преобразование x в число с двойной точностью</translation>
    </message>
    <message>
        <source>Current east-west resolution</source>
        <translation>Текущее горизонтальное разрешение</translation>
    </message>
    <message>
        <source>Exponential function of x</source>
        <translation>Экспонента от x</translation>
    </message>
    <message>
        <source>x to the power y</source>
        <translation>x в степени y</translation>
    </message>
    <message>
        <source>Convert x to single-precision floating point</source>
        <translation>Преобразование x в число с одинарной точностью</translation>
    </message>
    <message>
        <source>Decision: 1 if x not zero, 0 otherwise</source>
        <translation>Решение: 1 если x не равно нулю, иначе 0</translation>
    </message>
    <message>
        <source>Decision: a if x not zero, 0 otherwise</source>
        <translation>Решение: a если x не равно нулю, иначе 0</translation>
    </message>
    <message>
        <source>Decision: a if x not zero, b otherwise</source>
        <translation>Решение: a если x не равно нулю, иначе b</translation>
    </message>
    <message>
        <source>Decision: a if x &gt; 0, b if x is zero, c if x &lt; 0</source>
        <translation>Решение: a если x &gt; 0, b если x = 0, c если x &lt; 0</translation>
    </message>
    <message>
        <source>Convert x to integer [ truncates ]</source>
        <translation>Преобразование x в целое [отсечение]</translation>
    </message>
    <message>
        <source>Check if x = NULL</source>
        <translation>Проверка, равен ли x значению NULL</translation>
    </message>
    <message>
        <source>Natural log of x</source>
        <translation>Натуральный логарифм x</translation>
    </message>
    <message>
        <source>Log of x base b</source>
        <translation>Логарифм x по основанию b</translation>
    </message>
    <message>
        <source>Largest value</source>
        <translation>Наибольшее значение</translation>
    </message>
    <message>
        <source>Median value</source>
        <translation>Медиана</translation>
    </message>
    <message>
        <source>Smallest value</source>
        <translation>Наименьшее значение</translation>
    </message>
    <message>
        <source>Mode value</source>
        <translation>Мода</translation>
    </message>
    <message>
        <source>1 if x is zero, 0 otherwise</source>
        <translation>1, если x равен нулю, иначе 0</translation>
    </message>
    <message>
        <source>Current north-south resolution</source>
        <translation>Текущее вертикальное разрешение</translation>
    </message>
    <message>
        <source>NULL value</source>
        <translation>Значение NULL</translation>
    </message>
    <message>
        <source>Random value between a and b</source>
        <translation>Случайное значение между a и b</translation>
    </message>
    <message>
        <source>Round x to nearest integer</source>
        <translation>Округление x до ближайшего целого</translation>
    </message>
    <message>
        <source>Current row of moving window (Starts with 1)</source>
        <translation>Текущая строка подвижного окна (начиная с 1)</translation>
    </message>
    <message>
        <source>Sine of x (x is in degrees)</source>
        <comment>sin(x)</comment>
        <translation>Синус x (x в градусах)</translation>
    </message>
    <message>
        <source>Square root of x</source>
        <comment>sqrt(x)</comment>
        <translation>Квадратный корень из x</translation>
    </message>
    <message>
        <source>Tangent of x (x is in degrees)</source>
        <comment>tan(x)</comment>
        <translation>Тангенс x (x в градусах)</translation>
    </message>
    <message>
        <source>Current x-coordinate of moving window</source>
        <translation>Текущая x-координата подвижного окна</translation>
    </message>
    <message>
        <source>Current y-coordinate of moving window</source>
        <translation>Текущая y-координата подвижного окна</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Внимание</translation>
    </message>
    <message>
        <source>Cannot get current region</source>
        <translation>Не удалось получить регион</translation>
    </message>
    <message>
        <source>Cannot check region of map </source>
        <translation>Не удалось проверить регион карты </translation>
    </message>
    <message>
        <source>Cannot get region of map </source>
        <translation>Не удалось получить регион карты </translation>
    </message>
    <message>
        <source>No GRASS raster maps currently in QGIS</source>
        <translation>Не найдено доступных для QGIS растровых карт GRASS</translation>
    </message>
    <message>
        <source>Cannot create &apos;mapcalc&apos; directory in current mapset.</source>
        <translation>Не удалось создать каталог «mapcalc» в текущем наборе.</translation>
    </message>
    <message>
        <source>New mapcalc</source>
        <translation>Новая схема mapcalc</translation>
    </message>
    <message>
        <source>Enter new mapcalc name:</source>
        <translation>Введите имя новой схемы mapcalc:</translation>
    </message>
    <message>
        <source>Enter vector name</source>
        <translation>Введите имя файла</translation>
    </message>
    <message>
        <source>The file already exists. Overwrite? </source>
        <translation>Файл уже существует. Перезаписать? </translation>
    </message>
    <message>
        <source>Save mapcalc</source>
        <translation>Сохранить схему mapcalc</translation>
    </message>
    <message>
        <source>File name empty</source>
        <translation>Пустое имя файла</translation>
    </message>
    <message>
        <source>Cannot open mapcalc file</source>
        <translation>Не удаётся открыть файл mapcalc</translation>
    </message>
    <message>
        <source>The mapcalc schema (</source>
        <translation>Схема mapcalc (</translation>
    </message>
    <message>
        <source>) not found.</source>
        <translation>) не найдена.</translation>
    </message>
    <message>
        <source>Cannot open mapcalc schema (</source>
        <translation>Не удаётся открыть схему mapcalc (</translation>
    </message>
    <message>
        <source>Cannot read mapcalc schema (</source>
        <translation>Не удаётся прочесть схему mapcalc (</translation>
    </message>
    <message>
        <source>
at line </source>
        <translation>
в строке </translation>
    </message>
    <message>
        <source> column </source>
        <translation>, столбец </translation>
    </message>
    <message>
        <source>Output</source>
        <translation>Вывод</translation>
    </message>
</context>
<context>
    <name>QgsGrassMapcalcBase</name>
    <message>
        <source>MainWindow</source>
        <translation></translation>
    </message>
    <message>
        <source>Output</source>
        <translation>Вывод</translation>
    </message>
</context>
<context>
    <name>QgsGrassModule</name>
    <message>
        <source>Run</source>
        <translation>Выполнить</translation>
    </message>
    <message>
        <source>Stop</source>
        <translation>Остановить</translation>
    </message>
    <message>
        <source>Module</source>
        <translation>Модуль</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Внимание</translation>
    </message>
    <message>
        <source>The module file (</source>
        <translation>Файл модуля (</translation>
    </message>
    <message>
        <source>) not found.</source>
        <translation>) не найден.</translation>
    </message>
    <message>
        <source>Cannot open module file (</source>
        <translation>Не удалось открыть файл модуля (</translation>
    </message>
    <message>
        <source>)</source>
        <translation></translation>
    </message>
    <message>
        <source>Cannot read module file (</source>
        <translation>Не удалось прочесть файл модуля (</translation>
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
в строке </translation>
    </message>
    <message>
        <source>Module </source>
        <translation>Модуль </translation>
    </message>
    <message>
        <source> not found</source>
        <translation> не найден</translation>
    </message>
    <message>
        <source>Cannot find man page </source>
        <translation>Не удалось найти страницу руководства </translation>
    </message>
    <message>
        <source>Not available, cannot open description (</source>
        <translation>Модуль недоступен, не удалось открыть описание (</translation>
    </message>
    <message>
        <source> column </source>
        <translation>, столбец </translation>
    </message>
    <message>
        <source>Not available, incorrect description (</source>
        <translation>Модуль недоступен, неверное описание (</translation>
    </message>
    <message>
        <source>Cannot get input region</source>
        <translation>Не удаётся получить исходный регион</translation>
    </message>
    <message>
        <source>Use Input Region</source>
        <translation>Использовать исходный регион</translation>
    </message>
    <message>
        <source>Cannot find module </source>
        <translation>Не удалось найти модуль </translation>
    </message>
    <message>
        <source>Cannot start module: </source>
        <translation>Не удалось запустить модуль: </translation>
    </message>
    <message>
        <source>&lt;B&gt;Successfully finished&lt;/B&gt;</source>
        <translation>&lt;B&gt;Успешное завершение&lt;/B&gt;</translation>
    </message>
    <message>
        <source>&lt;B&gt;Finished with error&lt;/B&gt;</source>
        <translation>&lt;B&gt;Завершено с ошибкой&lt;/B&gt;</translation>
    </message>
    <message>
        <source>&lt;B&gt;Module crashed or killed&lt;/B&gt;</source>
        <translation>&lt;B&gt;Модуль рухнул или был убит&lt;/B&gt;</translation>
    </message>
    <message>
        <source>Not available, description not found (</source>
        <translation>Модуль недоступен, описание не найдено (</translation>
    </message>
    <message>
        <source>Please ensure you have the GRASS documentation installed.</source>
        <translation>Пожалуйста, убедитесь, что документация GRASS была установлена.</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleBase</name>
    <message>
        <source>GRASS Module</source>
        <translation>Модуль GRASS</translation>
    </message>
    <message>
        <source>Options</source>
        <translation>Параметры</translation>
    </message>
    <message>
        <source>Output</source>
        <translation>Вывод</translation>
    </message>
    <message>
        <source>Manual</source>
        <translation>Справка</translation>
    </message>
    <message>
        <source>Run</source>
        <translation>Выполнить</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
    <message>
        <source>View output</source>
        <translation>Открыть вывод</translation>
    </message>
    <message>
        <source>TextLabel</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleField</name>
    <message>
        <source>Attribute field</source>
        <translation>Поле атрибута</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleFile</name>
    <message>
        <source>File</source>
        <translation>Файл</translation>
    </message>
    <message>
        <source>:&amp;nbsp;missing value</source>
        <translation>:&amp;nbsp;значение не задано</translation>
    </message>
    <message>
        <source>:&amp;nbsp;directory does not exist</source>
        <translation>:&amp;nbsp;каталог не существует</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleGdalInput</name>
    <message>
        <source>Warning</source>
        <translation>Внимание</translation>
    </message>
    <message>
        <source>Cannot find layeroption </source>
        <translation>Не удаётся найти параметр слоя </translation>
    </message>
    <message>
        <source>PostGIS driver in OGR does not support schemas!&lt;br&gt;Only the table name will be used.&lt;br&gt;It can result in wrong input if more tables of the same name&lt;br&gt;are present in the database.</source>
        <translation>OGR-драйвер PostGIS не поддерживает схемы!&lt;br&gt;Будет использоваться только имя таблицы.&lt;br&gt;Это может повлиять на правильность ввода,&lt;br&gt;если в базе данных есть более одной таблицы&lt;br&gt;с одинаковыми именами.</translation>
    </message>
    <message>
        <source>:&amp;nbsp;no input</source>
        <translation>:&amp;nbsp;параметр не задан</translation>
    </message>
    <message>
        <source>Cannot find whereoption </source>
        <translation>Не удаётся найти параметр where </translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleInput</name>
    <message>
        <source>Warning</source>
        <translation>Внимание</translation>
    </message>
    <message>
        <source>Cannot find typeoption </source>
        <translation>Не удаётся найти параметр type </translation>
    </message>
    <message>
        <source>Cannot find values for typeoption </source>
        <translation>Не удаётся найти значения для параметра type </translation>
    </message>
    <message>
        <source>Cannot find layeroption </source>
        <translation>Не удаётся найти параметр слоя </translation>
    </message>
    <message>
        <source>GRASS element </source>
        <translation>Элемент GRASS </translation>
    </message>
    <message>
        <source> not supported</source>
        <translation> не поддерживается</translation>
    </message>
    <message>
        <source>Use region of this map</source>
        <translation>Использовать регион этой карты</translation>
    </message>
    <message>
        <source>:&amp;nbsp;no input</source>
        <translation>:&amp;nbsp;параметр не задан</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleOption</name>
    <message>
        <source>:&amp;nbsp;missing value</source>
        <translation>:&amp;nbsp;значение не задано</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleSelection</name>
    <message>
        <source>Attribute field</source>
        <translation>Поле атрибута</translation>
    </message>
</context>
<context>
    <name>QgsGrassModuleStandardOptions</name>
    <message>
        <source>Warning</source>
        <translation>Внимание</translation>
    </message>
    <message>
        <source>Cannot find module </source>
        <translation>Не удалось найти модуль </translation>
    </message>
    <message>
        <source>Cannot start module </source>
        <translation>Не удалось запустить модуль </translation>
    </message>
    <message>
        <source>Cannot read module description (</source>
        <translation>Не удалось прочесть описание модуля (</translation>
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
в строке </translation>
    </message>
    <message>
        <source> column </source>
        <translation>, столбец </translation>
    </message>
    <message>
        <source>Cannot find key </source>
        <translation>Не удаётся найти ключ </translation>
    </message>
    <message>
        <source>Item with id </source>
        <translation>Элемент с ID </translation>
    </message>
    <message>
        <source> not found</source>
        <translation> не найден</translation>
    </message>
    <message>
        <source>Cannot get current region</source>
        <translation>Не удалось получить текущий регион</translation>
    </message>
    <message>
        <source>Cannot check region of map </source>
        <translation>Не удалось проверить регион карты </translation>
    </message>
    <message>
        <source>Cannot set region of map </source>
        <translation>Не удалось задать регион карты </translation>
    </message>
</context>
<context>
    <name>QgsGrassNewMapset</name>
    <message>
        <source>GRASS database</source>
        <translation type="obsolete">База данных GRASS</translation>
    </message>
    <message>
        <source>GRASS location</source>
        <translation type="obsolete">Район GRASS</translation>
    </message>
    <message>
        <source>Projection</source>
        <translation type="obsolete">Проекция</translation>
    </message>
    <message>
        <source>Default GRASS Region</source>
        <translation type="obsolete">Регион GRASS по умолчанию</translation>
    </message>
    <message>
        <source>Mapset</source>
        <translation type="obsolete">Набор</translation>
    </message>
    <message>
        <source>Create New Mapset</source>
        <translation type="obsolete">Создать новый набор</translation>
    </message>
    <message>
        <source>Tree</source>
        <translation type="obsolete">Дерево</translation>
    </message>
    <message>
        <source>Comment</source>
        <translation type="obsolete">Комментарий</translation>
    </message>
    <message>
        <source>Database</source>
        <translation>База данных</translation>
    </message>
    <message>
        <source>Location 2</source>
        <translation>Район 2</translation>
    </message>
    <message>
        <source>User&apos;s mapset</source>
        <translation>Пользовательский набор</translation>
    </message>
    <message>
        <source>System mapset</source>
        <translation>Системный набор</translation>
    </message>
    <message>
        <source>Location 1</source>
        <translation>Район 1</translation>
    </message>
    <message>
        <source>Owner</source>
        <translation type="obsolete">Владелец</translation>
    </message>
    <message>
        <source>Enter path to GRASS database</source>
        <translation>Введите путь к базе данных GRASS</translation>
    </message>
    <message>
        <source>The directory doesn&apos;t exist!</source>
        <translation>Каталог не существует!</translation>
    </message>
    <message>
        <source>No writable locations, the database not writable!</source>
        <translation>Не найдено доступных для записи районов, нет прав доступа к базе данных!</translation>
    </message>
    <message>
        <source>Enter location name!</source>
        <translation>Введите имя района!</translation>
    </message>
    <message>
        <source>The location exists!</source>
        <translation>Район уже существует!</translation>
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
        <translation>Не удалось спроецировать ранее заданный регион, выбран регион по умолчанию.</translation>
    </message>
    <message>
        <source>North must be greater than south</source>
        <translation>Значение севера должно быть больше значения юга</translation>
    </message>
    <message>
        <source>East must be greater than west</source>
        <translation>Значение востока должно быть больше значения запада</translation>
    </message>
    <message>
        <source>Regions file (</source>
        <translation>Файл областей (</translation>
    </message>
    <message>
        <source>) not found.</source>
        <translation>) не найден.</translation>
    </message>
    <message>
        <source>Cannot open locations file (</source>
        <translation>Не удаётся открыть файл районов (</translation>
    </message>
    <message>
        <source>)</source>
        <translation></translation>
    </message>
    <message>
        <source>Cannot read locations file (</source>
        <translation>Не удаётся прочесть файл районов (</translation>
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
в строке </translation>
    </message>
    <message>
        <source> column </source>
        <translation>, столбец </translation>
    </message>
    <message>
        <source>Cannot reproject selected region.</source>
        <translation>Не удаётся спроецировать выбранный регион.</translation>
    </message>
    <message>
        <source>Cannot reproject region</source>
        <translation>Не удаётся спроецировать регион</translation>
    </message>
    <message>
        <source>Enter mapset name.</source>
        <translation>Введите имя набора.</translation>
    </message>
    <message>
        <source>The mapset already exists</source>
        <translation>Этот набор уже существует</translation>
    </message>
    <message>
        <source>Database: </source>
        <translation>База данных:</translation>
    </message>
    <message>
        <source>Location: </source>
        <translation>Район: </translation>
    </message>
    <message>
        <source>Mapset: </source>
        <translation>Набор:</translation>
    </message>
    <message>
        <source>Create location</source>
        <translation>Создать район</translation>
    </message>
    <message>
        <source>Cannot create new location: </source>
        <translation>Не удалось создать новый район: </translation>
    </message>
    <message>
        <source>Create mapset</source>
        <translation>Создать набор</translation>
    </message>
    <message>
        <source>Cannot open DEFAULT_WIND</source>
        <translation>Не удалось открыть DEFAULT_WIND</translation>
    </message>
    <message>
        <source>Cannot open WIND</source>
        <translation>Не удалось открыть WIND</translation>
    </message>
    <message>
        <source>New mapset</source>
        <translation>Новый набор</translation>
    </message>
    <message>
        <source>New mapset successfully created, but cannot be opened: </source>
        <translation>Новый набор успешно создан, но не может быть открыт: </translation>
    </message>
    <message>
        <source>New mapset successfully created and set as current working mapset.</source>
        <translation>Новый набор успешно создан и открыт как текущий рабочий набор.</translation>
    </message>
    <message>
        <source>Cannot create new mapset directory</source>
        <translation>Не удалось создать каталог для нового набора</translation>
    </message>
    <message>
        <source>Cannot create QgsCoordinateReferenceSystem</source>
        <translation>Не удаётся создать объект QgsCoordinateReferenceSystem</translation>
    </message>
</context>
<context>
    <name>QgsGrassNewMapsetBase</name>
    <message>
        <source>Column 1</source>
        <translation type="obsolete">Столбец 1</translation>
    </message>
    <message>
        <source>Example directory tree:</source>
        <translation>Пример структуры каталогов:</translation>
    </message>
    <message>
        <source>Database Error</source>
        <translation>Ошибка базы данных</translation>
    </message>
    <message>
        <source>Database:</source>
        <translation>База данных:</translation>
    </message>
    <message>
        <source>...</source>
        <translation type="obsolete">...</translation>
    </message>
    <message>
        <source>Select existing directory or create a new one:</source>
        <translation>Выберите существующий каталог или создайте новый:</translation>
    </message>
    <message>
        <source>Location</source>
        <translation>Район</translation>
    </message>
    <message>
        <source>Select location</source>
        <translation>Выбрать район</translation>
    </message>
    <message>
        <source>Create new location</source>
        <translation>Создать новый район</translation>
    </message>
    <message>
        <source>Location Error</source>
        <translation>Ошибка района</translation>
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
        <translation>Не определена</translation>
    </message>
    <message>
        <source>Set current QGIS extent</source>
        <translation>Установить текущие границы QGIS</translation>
    </message>
    <message>
        <source>Set</source>
        <translation>Установить</translation>
    </message>
    <message>
        <source>Region Error</source>
        <translation>Ошибка области</translation>
    </message>
    <message>
        <source>S</source>
        <translation>Ю</translation>
    </message>
    <message>
        <source>W</source>
        <translation>З</translation>
    </message>
    <message>
        <source>E</source>
        <translation>В</translation>
    </message>
    <message>
        <source>N</source>
        <translation>С</translation>
    </message>
    <message>
        <source>New mapset:</source>
        <translation>Новый набор:</translation>
    </message>
    <message>
        <source>Mapset Error</source>
        <translation>Ошибка набора</translation>
    </message>
    <message>
        <source>&lt;p align=&quot;center&quot;&gt;Existing masets&lt;/p&gt;</source>
        <translation>&lt;p align=&quot;center&quot;&gt;Существующие наборы&lt;/p&gt;</translation>
    </message>
    <message>
        <source>Location:</source>
        <translation>Район: </translation>
    </message>
    <message>
        <source>Mapset:</source>
        <translation>Набор:</translation>
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
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;Данные GRASS хранятся в виде дерева каталогов.&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;Базой данных GRASS  называется верхний каталог в этой структуре.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;The GRASS location is a collection of maps for a particular territory or project.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="obsolete">&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;Районом GRASS называется коллекция карт для определённой территории или проекта.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
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
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;&gt;Регион GRASS определяет границы для обработки растровых данных. Для каждого района существует регион по умолчанию. Регион может быть задан отдельно для каждого набора.&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;Регион по умолчанию можно изменить впоследствии.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
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
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;Набором GRASS называется коллекция карт, используемых одним пользователем. &lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;Пользователь имеет право читать все наборы в районе, но &lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;он имеет право на запись только в своём собственном наборе.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>New Mapset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>GRASS Database</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Tree</source>
        <translation type="unfinished">Дерево</translation>
    </message>
    <message>
        <source>Comment</source>
        <translation type="unfinished">Комментарий</translation>
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
        <translation type="unfinished">Обзор...</translation>
    </message>
    <message>
        <source>GRASS Location</source>
        <translation type="unfinished"></translation>
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
        <translation type="unfinished">Регион GRASS по умолчанию</translation>
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
        <translation type="unfinished">Набор</translation>
    </message>
    <message>
        <source>Owner</source>
        <translation type="unfinished">Владелец</translation>
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
        <translation type="unfinished">Создать новый набор</translation>
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
        <translation>&amp;GRASS</translation>
    </message>
    <message>
        <source>Open mapset</source>
        <translation>Открыть набор</translation>
    </message>
    <message>
        <source>New mapset</source>
        <translation>Новый набор</translation>
    </message>
    <message>
        <source>Close mapset</source>
        <translation>Закрыть набор</translation>
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
        <translation>Открыть инструменты GRASS</translation>
    </message>
    <message>
        <source>Display Current Grass Region</source>
        <translation>Показать текущий регион GRASS</translation>
    </message>
    <message>
        <source>Edit Current Grass Region</source>
        <translation>Изменить текущий регион GRASS</translation>
    </message>
    <message>
        <source>Edit Grass Vector layer</source>
        <translation>Редактировать векторный слой GRASS</translation>
    </message>
    <message>
        <source>Adds a GRASS vector layer to the map canvas</source>
        <translation>Добавить на карту векторный слой GRASS</translation>
    </message>
    <message>
        <source>Adds a GRASS raster layer to the map canvas</source>
        <translation>Добавить на карту растровый слой GRASS</translation>
    </message>
    <message>
        <source>Displays the current GRASS region as a rectangle on the map canvas</source>
        <translation>Показать текущий регион GRASS в виде прямоугольника на карте</translation>
    </message>
    <message>
        <source>Edit the current GRASS region</source>
        <translation>Изменить текущий регион GRASS</translation>
    </message>
    <message>
        <source>Edit the currently selected GRASS vector layer.</source>
        <translation>Изменить выбранный векторный слой GRASS.</translation>
    </message>
    <message>
        <source>GrassVector</source>
        <translation></translation>
    </message>
    <message>
        <source>0.1</source>
        <translation></translation>
    </message>
    <message>
        <source>GRASS layer</source>
        <translation>Слой GRASS</translation>
    </message>
    <message>
        <source>Create new Grass Vector</source>
        <translation>Создать новый векторный слой GRASS</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Внимание</translation>
    </message>
    <message>
        <source>GRASS Edit is already running.</source>
        <translation>Редактор GRASS уже запущен.</translation>
    </message>
    <message>
        <source>New vector name</source>
        <translation>Имя нового слоя</translation>
    </message>
    <message>
        <source>Cannot create new vector: </source>
        <translation>Не удалось создать новый векторный слой: </translation>
    </message>
    <message>
        <source>New vector created but cannot be opened by data provider.</source>
        <translation>Новый слой создан, но не может быть открыт поставщиком данных.</translation>
    </message>
    <message>
        <source>Cannot start editing.</source>
        <translation>Не удалось начать редактирование.</translation>
    </message>
    <message>
        <source>GISDBASE, LOCATION_NAME or MAPSET is not set, cannot display current region.</source>
        <translation>Не заданы GISDBASE, LOCATION_NAME или MAPSET, невозможно вывести текущий регион.</translation>
    </message>
    <message>
        <source>Cannot read current region: </source>
        <translation>Не удалось прочесть текущий регион: </translation>
    </message>
    <message>
        <source>Cannot open the mapset. </source>
        <translation>Не удаётся открыть набор. </translation>
    </message>
    <message>
        <source>Cannot close mapset. </source>
        <translation>Не удаётся закрыть набор. </translation>
    </message>
    <message>
        <source>Cannot close current mapset. </source>
        <translation>Не удаётся закрыть текущий набор. </translation>
    </message>
    <message>
        <source>Cannot open GRASS mapset. </source>
        <translation>Не удаётся открыть набор GRASS. </translation>
    </message>
</context>
<context>
    <name>QgsGrassRegion</name>
    <message>
        <source>Warning</source>
        <translation>Внимание</translation>
    </message>
    <message>
        <source>GISDBASE, LOCATION_NAME or MAPSET is not set, cannot display current region.</source>
        <translation>Не заданы GISDBASE, LOCATION_NAME или MAPSET, невозможно вывести текущий регион.</translation>
    </message>
    <message>
        <source>Cannot read current region: </source>
        <translation>Не удаётся прочесть текущий регион: </translation>
    </message>
    <message>
        <source>Cannot write region</source>
        <translation>Не удаётся сохранить регион</translation>
    </message>
</context>
<context>
    <name>QgsGrassRegionBase</name>
    <message>
        <source>GRASS Region Settings</source>
        <translation>Параметры региона GRASS</translation>
    </message>
    <message>
        <source>N</source>
        <translation>С</translation>
    </message>
    <message>
        <source>W</source>
        <translation>З</translation>
    </message>
    <message>
        <source>E</source>
        <translation>В</translation>
    </message>
    <message>
        <source>S</source>
        <translation>Ю</translation>
    </message>
    <message>
        <source>N-S Res</source>
        <translation>Вертикальное разрешение</translation>
    </message>
    <message>
        <source>Rows</source>
        <translation>Строк</translation>
    </message>
    <message>
        <source>Cols</source>
        <translation>Столбцов</translation>
    </message>
    <message>
        <source>E-W Res</source>
        <translation>Горизонтальное разрешение</translation>
    </message>
    <message>
        <source>Color</source>
        <translation>Цвет</translation>
    </message>
    <message>
        <source>Width</source>
        <translation>Ширина</translation>
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
        <translation>Выберите векторный слой GRASS</translation>
    </message>
    <message>
        <source>Select GRASS Raster Layer</source>
        <translation>Выберите растровый слой GRASS</translation>
    </message>
    <message>
        <source>Select GRASS mapcalc schema</source>
        <translation>Выберите схему GRASS mapcalc</translation>
    </message>
    <message>
        <source>Select GRASS Mapset</source>
        <translation>Выберите набор GRASS</translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Внимание</translation>
    </message>
    <message>
        <source>Cannot open vector on level 2 (topology not available).</source>
        <translation>Не удаётся открыть вектор уровня 2 (топология недоступна).</translation>
    </message>
    <message>
        <source>Choose existing GISDBASE</source>
        <translation>Выберите существующую GISDBASE</translation>
    </message>
    <message>
        <source>Wrong GISDBASE, no locations available.</source>
        <translation>Неверная GISDBASE, доступных районов не найдено.</translation>
    </message>
    <message>
        <source>Wrong GISDBASE</source>
        <translation>Неверная GISDBASE</translation>
    </message>
    <message>
        <source>Select a map.</source>
        <translation>Выберите карту.</translation>
    </message>
    <message>
        <source>No map</source>
        <translation>Нет карты</translation>
    </message>
    <message>
        <source>No layer</source>
        <translation>Нет слоя</translation>
    </message>
    <message>
        <source>No layers available in this map</source>
        <translation>В этой карте нет доступных слоёв</translation>
    </message>
</context>
<context>
    <name>QgsGrassSelectBase</name>
    <message>
        <source>Gisdbase</source>
        <translation></translation>
    </message>
    <message>
        <source>Location</source>
        <translation>Район</translation>
    </message>
    <message>
        <source>Browse</source>
        <translation>Обзор</translation>
    </message>
    <message>
        <source>Mapset</source>
        <translation>Набор</translation>
    </message>
    <message>
        <source>Map name</source>
        <translation>Имя карты</translation>
    </message>
    <message>
        <source>Layer</source>
        <translation>Слой</translation>
    </message>
    <message>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <source>Select or type map name (wildcards &apos;*&apos; and &apos;?&apos; accepted for rasters)</source>
        <translation>Выберите или введите имя карты (шаблоны * и ? принимаются для растров)</translation>
    </message>
    <message>
        <source>Add GRASS Layer</source>
        <translation>Добавить слой GRASS</translation>
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
        <translation>Оболочка GRASS</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
</context>
<context>
    <name>QgsGrassTools</name>
    <message>
        <source>Browser</source>
        <translation>Браузер</translation>
    </message>
    <message>
        <source>GRASS Tools</source>
        <translation>Инструменты GRASS</translation>
    </message>
    <message>
        <source>GRASS Tools: </source>
        <translation>Инструменты GRASS: </translation>
    </message>
    <message>
        <source>Warning</source>
        <translation>Внимание</translation>
    </message>
    <message>
        <source>Cannot find MSYS (</source>
        <translation>Не удаётся найти MSYS (</translation>
    </message>
    <message>
        <source>GRASS Shell is not compiled.</source>
        <translation>Оболочка GRASS не была скомпилирована.</translation>
    </message>
    <message>
        <source>The config file (</source>
        <translation>Файл настроек (</translation>
    </message>
    <message>
        <source>) not found.</source>
        <translation>) не найден.</translation>
    </message>
    <message>
        <source>Cannot open config file (</source>
        <translation>Не удаётся открыть файл конфигурации (</translation>
    </message>
    <message>
        <source>)</source>
        <translation></translation>
    </message>
    <message>
        <source>Cannot read config file (</source>
        <translation>Не удаётся прочесть файл конфигурации (</translation>
    </message>
    <message>
        <source>
at line </source>
        <translation>
в строке </translation>
    </message>
    <message>
        <source> column </source>
        <translation>, столбец </translation>
    </message>
</context>
<context>
    <name>QgsGrassToolsBase</name>
    <message>
        <source>Grass Tools</source>
        <translation>Инструменты GRASS</translation>
    </message>
    <message>
        <source>Modules Tree</source>
        <translation>Дерево модулей</translation>
    </message>
    <message>
        <source>1</source>
        <translation></translation>
    </message>
    <message>
        <source>Modules List</source>
        <translation>Список модулей</translation>
    </message>
</context>
<context>
    <name>QgsGridMakerPlugin</name>
    <message>
        <source>&amp;Graticule Creator</source>
        <translation>&amp;Конструктор сетки</translation>
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
        <translation>QGIS — Конструктор сетки</translation>
    </message>
    <message>
        <source>Please enter the file name before pressing OK!</source>
        <translation>Пожалуйста, введите имя файла прежде чем нажимать OK!</translation>
    </message>
    <message>
        <source>ESRI Shapefile (*.shp)</source>
        <translation>Shape-файлы (*.shp)</translation>
    </message>
    <message>
        <source>Please enter intervals before pressing OK!</source>
        <translation>Пожалуйста, введите интервалы прежде чем нажимать OK!</translation>
    </message>
    <message>
        <source>Choose a file name to save under</source>
        <translation>Выберите имя сохраняемого файла</translation>
    </message>
</context>
<context>
    <name>QgsGridMakerPluginGuiBase</name>
    <message>
        <source>Graticule Builder</source>
        <translation>Конструктор сетки</translation>
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
        <source>Polygon</source>
        <translation>Полигон</translation>
    </message>
    <message>
        <source>Origin (lower left)</source>
        <translation>Начальная точка (нижняя левая)</translation>
    </message>
    <message>
        <source>End point (upper right)</source>
        <translation>Конечная точка (верхняя правая)</translation>
    </message>
    <message>
        <source>Output (shape) file</source>
        <translation>Выходной (shape) файл</translation>
    </message>
    <message>
        <source>Save As...</source>
        <translation>Сохранить как...</translation>
    </message>
    <message>
        <source>X</source>
        <translation></translation>
    </message>
    <message>
        <source>Y</source>
        <translation></translation>
    </message>
    <message>
        <source>Graticle size</source>
        <translation>Размеры сетки</translation>
    </message>
    <message>
        <source>X Interval:</source>
        <translation>Интервал по X:</translation>
    </message>
    <message>
        <source>Y Interval:</source>
        <translation>Интервал по Y:</translation>
    </message>
    <message>
        <source>QGIS Graticule Creator</source>
        <translation>Конструктор сетки QGIS</translation>
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
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;; font-size:11pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;Этот модуль предназначен для создания shape-файла с картографической сеткой, которую можно использовать для наложения на карту.&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Arial&apos;;&quot;&gt;Пожалуйста, вводите все значения в десятичных градусах&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
</context>
<context>
    <name>QgsHelpViewer</name>
    <message>
        <source>Quantum GIS Help - </source>
        <translation>Справка Quantum GIS — </translation>
    </message>
    <message>
        <source>Failed to get the help text from the database</source>
        <translation>Не удалось получить текст справки из базы данных</translation>
    </message>
    <message>
        <source>Error</source>
        <translation>Ошибка</translation>
    </message>
    <message>
        <source>The QGIS help database is not installed</source>
        <translation>База справки QGIS не установлена</translation>
    </message>
    <message>
        <source>This help file does not exist for your language</source>
        <translation>Данный файл справки недоступен на вашем языке</translation>
    </message>
    <message>
        <source>If you would like to create it, contact the QGIS development team</source>
        <translation>Если вы хотите создать его, свяжитесь с командой разработки QGIS</translation>
    </message>
    <message>
        <source>Quantum GIS Help</source>
        <translation>Справка Quantum GIS</translation>
    </message>
</context>
<context>
    <name>QgsHelpViewerBase</name>
    <message>
        <source>QGIS Help</source>
        <translation>Справка QGIS</translation>
    </message>
    <message>
        <source>&amp;Home</source>
        <translation>&amp;Главная</translation>
    </message>
    <message>
        <source>Alt+H</source>
        <translation>Alt+U</translation>
    </message>
    <message>
        <source>&amp;Forward</source>
        <translation>&amp;Вперёд</translation>
    </message>
    <message>
        <source>Alt+F</source>
        <translation>Alt+D</translation>
    </message>
    <message>
        <source>&amp;Back</source>
        <translation>&amp;Назад</translation>
    </message>
    <message>
        <source>Alt+B</source>
        <translation>Alt+Y</translation>
    </message>
    <message>
        <source>&amp;Close</source>
        <translation>&amp;Закрыть</translation>
    </message>
    <message>
        <source>Alt+C</source>
        <translation>Alt+P</translation>
    </message>
</context>
<context>
    <name>QgsHttpTransaction</name>
    <message>
        <source>WMS Server responded unexpectedly with HTTP Status Code %1 (%2)</source>
        <translation>Неожиданный ответ WMS-сервера с HTTP-кодом %1 (%2)</translation>
    </message>
    <message>
        <source>HTTP response completed, however there was an error: %1</source>
        <translation>HTTP-ответ получен  с ошибкой: %1</translation>
    </message>
    <message>
        <source>Network timed out after %1 seconds of inactivity.
This may be a problem in your network connection or at the WMS server.</source>
        <translation type="obsolete">Соединение сброшено после %1 секунд бездействия.
Возможно существует проблема в подключении к сети или на WMS-сервере.
        </translation>
    </message>
    <message>
        <source>HTTP transaction completed, however there was an error: %1</source>
        <translation>HTTP-транзакция завершена с ошибкой: %1</translation>
    </message>
</context>
<context>
    <name>QgsIDWInterpolatorDialogBase</name>
    <message>
        <source>Dialog</source>
        <translation></translation>
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
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-weight:600;&quot;&gt;Обратное взвешивание расстояний&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-weight:600;&quot;&gt;&lt;span style=&quot; font-weight:400;&quot;&gt;Единственный параметр для метода IDW-интерполяции — это коэффициент, характеризующий уменьшение веса в зависимости от расстояния.&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Distance coefficient P:</source>
        <translation>Коэффициент расстояния P:</translation>
    </message>
</context>
<context>
    <name>QgsIdentifyResults</name>
    <message>
        <source>Identify Results - </source>
        <translation>Результат определения — </translation>
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
    <name>QgsInterpolationDialog</name>
    <message>
        <source>Triangular interpolation (TIN)</source>
        <translation>Триангуляция (TIN)</translation>
    </message>
    <message>
        <source>Inverse Distance Weighting (IDW)</source>
        <translation>Обратное взвешивание расстояний (IDW)</translation>
    </message>
</context>
<context>
    <name>QgsInterpolationDialogBase</name>
    <message>
        <source>Output</source>
        <translation>Вывод</translation>
    </message>
    <message>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <source>Input</source>
        <translation>Исходные данные</translation>
    </message>
    <message>
        <source>Use z-Coordinate for interpolation</source>
        <translation>Использовать для интерполяции Z-координату</translation>
    </message>
    <message>
        <source>Interpolation plugin</source>
        <translation>Модуль интерполяции</translation>
    </message>
    <message>
        <source>Input vector layer</source>
        <translation>Исходный векторный слой</translation>
    </message>
    <message>
        <source>Interpolation attribute </source>
        <translation>Атрибут для интерполяции </translation>
    </message>
    <message>
        <source>Interpolation method</source>
        <translation>Метод интерполяции</translation>
    </message>
    <message>
        <source>Number of columns</source>
        <translation>Столбцов</translation>
    </message>
    <message>
        <source>Number of rows</source>
        <translation>Строк</translation>
    </message>
    <message>
        <source>Output file </source>
        <translation>Файл вывода</translation>
    </message>
</context>
<context>
    <name>QgsInterpolationPlugin</name>
    <message>
        <source>&amp;Interpolation</source>
        <translation>&amp;Интерполяция</translation>
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
        <source>Preview:</source>
        <translation>Предпросмотр:</translation>
    </message>
    <message>
        <source>QGIS Rocks!</source>
        <translation>QGIS работает!</translation>
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
        <source>Position</source>
        <translation>Позиция</translation>
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
    <message encoding="UTF-8">
        <source>°</source>
        <translation></translation>
    </message>
    <message>
        <source>Font size units</source>
        <translation>Единицы размера шрифта</translation>
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
        <translation>Единицы смещения</translation>
    </message>
    <message>
        <source>Field containing label</source>
        <translation>Поле, содержащее подпись</translation>
    </message>
    <message>
        <source>Default label</source>
        <translation>Подпись по умолчанию</translation>
    </message>
    <message>
        <source>Data defined style</source>
        <translation>Данные стиля</translation>
    </message>
    <message>
        <source>Data defined alignment</source>
        <translation>Данные выравнивания</translation>
    </message>
    <message>
        <source>Data defined buffer</source>
        <translation>Данные буферизации</translation>
    </message>
    <message>
        <source>Data defined position</source>
        <translation>Данные положения</translation>
    </message>
    <message>
        <source>Font transparency</source>
        <translation>Прозрачность шрифта</translation>
    </message>
    <message>
        <source>Color</source>
        <translation>Цвет</translation>
    </message>
    <message>
        <source>Angle (deg)</source>
        <translation>Угол (град)</translation>
    </message>
    <message>
        <source>Buffer labels?</source>
        <translation>Буферизовать подписи?</translation>
    </message>
    <message>
        <source>Buffer size</source>
        <translation>Размер буфера</translation>
    </message>
    <message>
        <source>Transparency</source>
        <translation>Прозрачность</translation>
    </message>
    <message>
        <source>X Offset (pts)</source>
        <translation>Смещение по X (пункты)</translation>
    </message>
    <message>
        <source>Y Offset (pts)</source>
        <translation>Смещение по Y (пункты)</translation>
    </message>
    <message>
        <source>&amp;Font family</source>
        <translation>&amp;Шрифт</translation>
    </message>
    <message>
        <source>&amp;Bold</source>
        <translation>&amp;Полужирный</translation>
    </message>
    <message>
        <source>&amp;Italic</source>
        <translation>&amp;Курсив</translation>
    </message>
    <message>
        <source>&amp;Underline</source>
        <translation>&amp;Подчёркивание</translation>
    </message>
    <message>
        <source>&amp;Size</source>
        <translation>&amp;Размер</translation>
    </message>
    <message>
        <source>Size units</source>
        <translation>Единицы размера</translation>
    </message>
    <message>
        <source>X Coordinate</source>
        <translation>X-координата</translation>
    </message>
    <message>
        <source>Y Coordinate</source>
        <translation>Y-координата</translation>
    </message>
    <message>
        <source>Multiline labels?</source>
        <translation>Разбивать подписи на строки?</translation>
    </message>
</context>
<context>
    <name>QgsLegend</name>
    <message>
        <source>group</source>
        <translation>группа</translation>
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
        <source>Multiple layers</source>
        <translation>Множество слоёв</translation>
    </message>
    <message>
        <source>This item contains multiple layers. Displaying multiple layers in the table is not supported.</source>
        <translation>Этот элемент содержит более одного слоя. Отображение нескольких слоёв в таблице не поддерживается.</translation>
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
        <source>Save layer as...</source>
        <translation>Сохранить слой как...</translation>
    </message>
    <message>
        <source>Error</source>
        <translation>Ошибка</translation>
    </message>
    <message>
        <source>Saving done</source>
        <translation>Сохранение выполнено</translation>
    </message>
    <message>
        <source>Export to Shapefile has been completed</source>
        <translation>Экспорт в shape-файл завершён</translation>
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
        <translation>Не удалось создать shape-файл (</translation>
    </message>
    <message>
        <source>Layer creation failed</source>
        <translation>Не удалось создать слой</translation>
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
        <source>Layer attribute table contains unsupported datatype(s)</source>
        <translation>Таблица атрибутов слоя включает неподдерживаемые типы данных</translation>
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
        <source>bad_alloc exception</source>
        <translation type="unfinished">Исключение bad_alloc</translation>
    </message>
    <message>
        <source>Filling the attribute table has been stopped because there was no more virtual memory left</source>
        <translation type="unfinished">Заполнение таблицы атрибутов остановлено, поскольку закончилась виртуальная память</translation>
    </message>
    <message>
        <source>Attribute table - </source>
        <translation type="unfinished">Таблица атрибутов — </translation>
    </message>
    <message>
        <source>Start editing failed</source>
        <translation type="unfinished">Не удалось начать редактирование</translation>
    </message>
    <message>
        <source>Provider cannot be opened for editing</source>
        <translation type="unfinished">Источник не может быть открыт для редактирования</translation>
    </message>
    <message>
        <source>Stop editing</source>
        <translation type="unfinished">Прекратить редактирование</translation>
    </message>
    <message>
        <source>Do you want to save the changes?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Could not commit changes</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Problems during roll back</source>
        <translation type="unfinished">Ошибка в процессе отката</translation>
    </message>
</context>
<context>
    <name>QgsMapCanvas</name>
    <message>
        <source>Could not draw</source>
        <translation>Ошибка отрисовки</translation>
    </message>
    <message>
        <source>because</source>
        <translation>по причине</translation>
    </message>
</context>
<context>
    <name>QgsMapLayer</name>
    <message>
        <source>%1 at line %2 column %3</source>
        <translation>%1 в строке %2, столбце %3</translation>
    </message>
    <message>
        <source>User database could not be opened.</source>
        <translation>Не удалось открыть базу данных пользователя.</translation>
    </message>
    <message>
        <source>The style table could not be created.</source>
        <translation>Не удалось создать таблицу стилей.</translation>
    </message>
    <message>
        <source>The style %1 was saved to database</source>
        <translation>Стиль %1 был сохранён в базе данных</translation>
    </message>
    <message>
        <source>The style %1 was updated in the database.</source>
        <translation>Стиль %1 был обновлён в базе данных.</translation>
    </message>
    <message>
        <source>The style %1 could not be updated in the database.</source>
        <translation>Не удалось обновить в базе данных стиль %1.</translation>
    </message>
    <message>
        <source>The style %1 could not be inserted into database.</source>
        <translation>Не удалось вставить в базу данных стиль %1.</translation>
    </message>
    <message>
        <source>style not found in database</source>
        <translation>стиль не найден в базе данных</translation>
    </message>
</context>
<context>
    <name>QgsMapToolIdentify</name>
    <message>
        <source>- %1 features found</source>
        <comment>Identify results window title</comment>
        <translation type="obsolete">— найдено %1 объектов
        </translation>
    </message>
    <message>
        <source>(clicked coordinate)</source>
        <translation>(координаты щелчка)</translation>
    </message>
    <message>
        <source>WMS identify result for %1
%2</source>
        <translation>Результат WMS-определения для %1
%2</translation>
    </message>
</context>
<context>
    <name>QgsMapToolSplitFeatures</name>
    <message>
        <source>Split error</source>
        <translation>Ошибка разделения</translation>
    </message>
    <message>
        <source>An error occured during feature splitting</source>
        <translation>При разделении объектов возникла ошибка</translation>
    </message>
</context>
<context>
    <name>QgsMapToolVertexEdit</name>
    <message>
        <source>Snap tolerance</source>
        <translation>Порог прилипания</translation>
    </message>
    <message>
        <source>Don&apos;t show this message again</source>
        <translation>Не показывать это сообщение в дальнейшем</translation>
    </message>
    <message>
        <source>Could not snap segment.</source>
        <translation>Ошибка выравнивания сегмента.</translation>
    </message>
    <message>
        <source>Have you set the tolerance in Settings &gt; Project Properties &gt; General?</source>
        <translation>Проверьте, был ли задан порог прилипания в меню Установки &gt; Свойства проекта &gt; Общие?</translation>
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
        <translation> уже существует. Вы хотите перезаписать этот файл?</translation>
    </message>
    <message>
        <source>MapServer map files (*.map);;All files (*.*)</source>
        <comment>Filter list for selecting files from a dialog box</comment>
        <translation>Файлы карт MapServer (*.map);;Все файлы (*.*)</translation>
    </message>
    <message>
        <source> exists. 
Do you want to overwrite it?</source>
        <comment>a fileName is prepended to this text, and appears in a dialog box</comment>
        <translation> уже существует. Вы хотите перезаписать этот файл?</translation>
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
        <translation>Экспортировать только данные слоёв (LAYER)</translation>
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
        <translation>пользовательский</translation>
    </message>
    <message>
        <source>wbmp</source>
        <translation></translation>
    </message>
    <message>
        <source>MinScale</source>
        <translation>Мин. масштаб</translation>
    </message>
    <message>
        <source>MaxScale</source>
        <translation>Макс. масштаб</translation>
    </message>
    <message>
        <source>Prefix attached to map, scalebar and legend GIF filenames created using this MapFile. It should be kept short.</source>
        <translation>Префикс для карты, масштабной линейки и GIF-файла легенды, созданных для этого map-файла. Имя должно быть кратким.</translation>
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
        <translation>...</translation>
    </message>
    <message>
        <source>Name for the map file to be created from the QGIS project file</source>
        <translation>Имя map-файла, создаваемого из файла проекта QGIS</translation>
    </message>
    <message>
        <source>If checked, only the layer information will be processed</source>
        <translation>Обрабатывать только сведения о слоях</translation>
    </message>
    <message>
        <source>Path to the MapServer template file</source>
        <translation>Путь к файлу шаблона MapServer</translation>
    </message>
    <message>
        <source>Prefix attached to map, scalebar and legend GIF filenames created using this MapFile</source>
        <translation>Префикс для карты, масштабной линейки и GIF-файла легенды, созданных для этого map-файла</translation>
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
        <translation>Сбросить</translation>
    </message>
    <message>
        <source>Cl&amp;ose</source>
        <translation>&amp;Закрыть</translation>
    </message>
    <message>
        <source>Total:</source>
        <translation>Всего:</translation>
    </message>
    <message>
        <source>Segments</source>
        <translation>Сегменты</translation>
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
    <name>QgsNewConnection</name>
    <message>
        <source>Test connection</source>
        <translation>Проверка соединения</translation>
    </message>
    <message>
        <source>Connection failed - Check settings and try again.

Extended error information:
</source>
        <translation>Не удалось соединиться — проверьте параметры и попробуйте ещё раз.

Дополнительная информация:
</translation>
    </message>
    <message>
        <source>Connection to %1 was successful</source>
        <translation>Успешное соединение с %1</translation>
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
        <translation>Узел</translation>
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
        <translation>Имя нового соединения</translation>
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
        <translation>Искать только в схеме «public»</translation>
    </message>
    <message>
        <source>Only look in the geometry_columns table</source>
        <translation>Искать только в таблице geometry_columns</translation>
    </message>
    <message>
        <source>Restrict the search to the public schema for spatial tables not in the geometry_columns table</source>
        <translation>Ограничить поиск пространственных таблиц, не включенных в geometry_columns, схемой «public»</translation>
    </message>
    <message>
        <source>When searching for spatial tables that are not in the geometry_columns tables, restrict the search to tables that are in the public schema (for some databases this can save lots of time)</source>
        <translation>При поиске пространственных таблиц, не включенных в таблицу geometry_columns, сократить поиск до таблиц, содержащихся в схеме «public» (позволяет существенно сокращатить время поиска в некоторых БД)</translation>
    </message>
    <message>
        <source>Restrict the displayed tables to those that are in the geometry_columns table</source>
        <translation>Вывести только таблицы, содержащиеся в таблице geometry_columns</translation>
    </message>
    <message>
        <source>Restricts the displayed tables to those that are in the geometry_columns table. This can speed up the initial display of spatial tables.</source>
        <translation>Вывести только таблицы, содержащиеся в таблице geometry_columns. Это может ускорить начальный поиск пространственных таблиц.</translation>
    </message>
</context>
<context>
    <name>QgsNewHttpConnectionBase</name>
    <message>
        <source>Name</source>
        <translation>Имя</translation>
    </message>
    <message>
        <source>URL</source>
        <translation></translation>
    </message>
    <message>
        <source>Name of the new connection</source>
        <translation>Имя нового соединения</translation>
    </message>
    <message>
        <source>HTTP address of the Web Map Server</source>
        <translation>HTTP-адрес WMS-сервера</translation>
    </message>
    <message>
        <source>Create a new WMS connection</source>
        <translation>Создание нового WMS-соединения</translation>
    </message>
    <message>
        <source>Connection details</source>
        <translation>Параметры соединения</translation>
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
        <translation>Вывод указателя «север-юг»</translation>
    </message>
    <message>
        <source>&amp;Decorations</source>
        <translation>&amp;Оформление</translation>
    </message>
    <message>
        <source>North arrow pixmap not found</source>
        <translation>Не найдено изображение указателя «север-юг»</translation>
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
        <translation>Указатель «север-юг»</translation>
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
        <translation>Выбирать направление автоматически</translation>
    </message>
    <message>
        <source>Enable North Arrow</source>
        <translation>Включить указатель «север-юг»</translation>
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
        <translation>Предпросмотр указателя</translation>
    </message>
    <message>
        <source>Icon</source>
        <translation>Значок</translation>
    </message>
    <message>
        <source>Browse...</source>
        <translation>Обзор...</translation>
    </message>
</context>
<context>
    <name>QgsOptions</name>
    <message>
        <source>Detected active locale on your system: </source>
        <translation>Обнаруженный системный язык: </translation>
    </message>
    <message>
        <source>to vertex</source>
        <translation>к вершинам</translation>
    </message>
    <message>
        <source>to segment</source>
        <translation>к сегментам</translation>
    </message>
    <message>
        <source>to vertex and segment</source>
        <translation>к вершинам и сегментам</translation>
    </message>
    <message>
        <source>Semi transparent circle</source>
        <translation>Полупрозрачный круг</translation>
    </message>
    <message>
        <source>Cross</source>
        <translation>Перекрестие</translation>
    </message>
    <message>
        <source>Show all features</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Show selected features</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Show features in current canvas</source>
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
        <source>Hide splash screen at startup</source>
        <translation>Не показывать заставку при запуске</translation>
    </message>
    <message>
        <source>&lt;b&gt;Note: &lt;/b&gt;Theme changes take effect the next time QGIS is started</source>
        <translation>&lt;b&gt;Внимание: &lt;/b&gt;Изменение темы вступит в силу при следующем запуске QGIS</translation>
    </message>
    <message>
        <source>&amp;Rendering</source>
        <translation>От&amp;рисовка</translation>
    </message>
    <message>
        <source>Map display will be updated (drawn) after this many features have been read from the data source</source>
        <translation>Изображение карты будет обновлено (перерисовано) после того, как это количество объектов загружено из источника данных</translation>
    </message>
    <message>
        <source>Select Global Default ...</source>
        <translation>Выбрать глобальную систему координат...</translation>
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
        <source>Measure tool</source>
        <translation>Инструмент измерений</translation>
    </message>
    <message>
        <source>Search radius</source>
        <translation>Радиус поиска</translation>
    </message>
    <message>
        <source>Fix problems with incorrectly filled polygons</source>
        <translation>Исправлять ошибки заливки полигонов</translation>
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
        <source>&amp;General</source>
        <translation>&amp;Общие</translation>
    </message>
    <message>
        <source>Locale</source>
        <translation>Язык</translation>
    </message>
    <message>
        <source>Locale to use instead</source>
        <translation>Язык, используемый вместо системного</translation>
    </message>
    <message>
        <source>Additional Info</source>
        <translation>Дополнительная информация</translation>
    </message>
    <message>
        <source>Detected active locale on your system:</source>
        <translation>Обнаруженный системный язык:</translation>
    </message>
    <message>
        <source>Selecting this will unselect the &apos;make lines less&apos; jagged toggle</source>
        <translation>Активация этого параметра выключит флажок «Рисовать сглаженные линии»</translation>
    </message>
    <message>
        <source>Digitizing</source>
        <translation>Оцифровка</translation>
    </message>
    <message>
        <source>Rubberband</source>
        <translation>Резиновая нить</translation>
    </message>
    <message>
        <source>Line width in pixels</source>
        <translation>Ширина линии в пикселях</translation>
    </message>
    <message>
        <source>Snapping</source>
        <translation>Прилипание</translation>
    </message>
    <message>
        <source>Zoom to mouse cursor</source>
        <translation>Увеличить в положении курсора</translation>
    </message>
    <message>
        <source>Project files</source>
        <translation>Файлы проектов</translation>
    </message>
    <message>
        <source>Prompt to save project changes when required</source>
        <translation>Запрашивать сохранение изменений в проекте, когда это необходимо</translation>
    </message>
    <message>
        <source>Warn when opening a project file saved with an older version of QGIS</source>
        <translation>Предупреждать при попытке открытия файлов проекта старых версий QGIS</translation>
    </message>
    <message>
        <source>Default Map Appearance (overridden by project properties)</source>
        <translation>Вид карты по умолчанию (заменяется свойствами проекта)</translation>
    </message>
    <message>
        <source>Selection color</source>
        <translation>Цвет выделения</translation>
    </message>
    <message>
        <source>Background color</source>
        <translation>Цвет фона</translation>
    </message>
    <message>
        <source>&amp;Application</source>
        <translation>&amp;Приложение</translation>
    </message>
    <message>
        <source>Icon theme</source>
        <translation>Тема значков</translation>
    </message>
    <message>
        <source>Capitalise layer names in legend</source>
        <translation>Выводить имя слоя с заглавной буквы</translation>
    </message>
    <message>
        <source>Display classification attribute names in legend</source>
        <translation>Показывать в легенде атрибуты классификации</translation>
    </message>
    <message>
        <source>Rendering behavior</source>
        <translation>Параметры отрисовки</translation>
    </message>
    <message>
        <source>Number of features to draw before updating the display</source>
        <translation>Количество объектов для отрисовки между обновлениями экрана</translation>
    </message>
    <message>
        <source>&lt;b&gt;Note:&lt;/b&gt; Use zero to prevent display updates until all features have been rendered</source>
        <translation>&lt;b&gt;Внимание:&lt;/b&gt; введите 0, чтобы запретить обновление экрана до отрисовки всех объектов</translation>
    </message>
    <message>
        <source>Rendering quality</source>
        <translation>Качество отрисовки</translation>
    </message>
    <message>
        <source>Zoom factor</source>
        <translation>Фактор увеличения</translation>
    </message>
    <message>
        <source>Mouse wheel action</source>
        <translation>Действие при прокрутке колеса мыши</translation>
    </message>
    <message>
        <source>Rubberband color</source>
        <translation>Цвет линии</translation>
    </message>
    <message>
        <source>Ellipsoid for distance calculations</source>
        <translation>Эллипсоид для вычисления расстояний</translation>
    </message>
    <message>
        <source>&lt;b&gt;Note:&lt;/b&gt; Specify the search radius as a percentage of the map width</source>
        <translation>&lt;b&gt;Внимание:&lt;/b&gt; радиус поиска задаётся в процентах от ширины видимой карты</translation>
    </message>
    <message>
        <source>Search radius for identifying features and displaying map tips</source>
        <translation>Радиус поиска для определения объектов и всплывающих описаний</translation>
    </message>
    <message>
        <source>Line width</source>
        <translation>Ширина линии</translation>
    </message>
    <message>
        <source>Line colour</source>
        <translation>Цвет линии</translation>
    </message>
    <message>
        <source>Default snap mode</source>
        <translation>Режим прилипания по умолчанию</translation>
    </message>
    <message>
        <source>Default snapping tolerance in layer units</source>
        <translation>Порог прилипания по умолчанию в единицах слоя</translation>
    </message>
    <message>
        <source>Search radius for vertex edits in layer units</source>
        <translation>Радиус поиска для редактирования вершин в единицах слоя</translation>
    </message>
    <message>
        <source>Vertex markers</source>
        <translation>Маркеры вершин</translation>
    </message>
    <message>
        <source>Marker style</source>
        <translation>Стиль маркера</translation>
    </message>
    <message>
        <source>Override system locale</source>
        <translation>Переопределить системный язык</translation>
    </message>
    <message>
        <source>&lt;b&gt;Note:&lt;/b&gt; Enabling / changing overide on local requires an application restart</source>
        <translation>&lt;b&gt;Внимание:&lt;/b&gt; для переопределения параметров языка необходимо перезапустить приложение</translation>
    </message>
    <message>
        <source>Proxy</source>
        <translation>Прокси-сервер</translation>
    </message>
    <message>
        <source>Use proxy for web access</source>
        <translation>Использовать прокси-сервер для внешних соединений</translation>
    </message>
    <message>
        <source>Host</source>
        <translation>Узел</translation>
    </message>
    <message>
        <source>Port</source>
        <translation>Порт</translation>
    </message>
    <message>
        <source>User</source>
        <translation>Пользователь</translation>
    </message>
    <message>
        <source>Leave this blank if no proxy username / password are required</source>
        <translation>Оставьте это поле пустым, если для прокси-сервера не требуется имя пользователя и пароль</translation>
    </message>
    <message>
        <source>Password</source>
        <translation>Пароль</translation>
    </message>
    <message>
        <source>Open attribute table in a dock window</source>
        <translation>Открывать таблицу атрибутов во встраиваемом окне</translation>
    </message>
    <message>
        <source>CRS</source>
        <translation>Система координат</translation>
    </message>
    <message>
        <source>When layer is loaded that has no coordinate reference system (CRS)</source>
        <translation>При загрузке слоя, не содержащего сведений о системе координат</translation>
    </message>
    <message>
        <source>Prompt for CRS</source>
        <translation>Запрашивать систему координат</translation>
    </message>
    <message>
        <source>Project wide default CRS will be used</source>
        <translation>Использовать значение по умолчанию для данного проекта</translation>
    </message>
    <message>
        <source>Global default CRS displa&amp;yed below will be used</source>
        <translation>Использовать ниж&amp;еприведённую глобальную систему координат</translation>
    </message>
    <message>
        <source>Attribute table behaviour</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsPasteTransformationsBase</name>
    <message>
        <source>Paste Transformations</source>
        <translation>Вставить преобразования</translation>
    </message>
    <message>
        <source>&lt;b&gt;Note: This function is not useful yet!&lt;/b&gt;</source>
        <translation>&lt;b&gt;Внимание: Эта функция пока бесполезна!&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Source</source>
        <translation>Источник</translation>
    </message>
    <message>
        <source>Destination</source>
        <translation>Приёмник</translation>
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
        <translation>Добавить новое преобразование</translation>
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
    <name>QgsPgGeoprocessing</name>
    <message>
        <source>Buffer features in layer %1</source>
        <translation>Буферизация объектов слоя %1</translation>
    </message>
    <message>
        <source>Error connecting to the database</source>
        <translation>Ошибка подключения к базе данных</translation>
    </message>
    <message>
        <source>&amp;Buffer features</source>
        <translation>&amp;Буферизация объектов</translation>
    </message>
    <message>
        <source>A new layer is created in the database with the buffered features.</source>
        <translation>В базе данных создан новый слой буферных зон.</translation>
    </message>
    <message>
        <source>&amp;Geoprocessing</source>
        <translation>&amp;Обработка данных</translation>
    </message>
    <message>
        <source>Unable to add geometry column</source>
        <translation>Не удалось добавить поле геометрии</translation>
    </message>
    <message>
        <source>Unable to add geometry column to the output table </source>
        <translation>Не удалось добавить поле геометрии в выходную таблицу </translation>
    </message>
    <message>
        <source>Unable to create table</source>
        <translation>Не удалось создать таблицу</translation>
    </message>
    <message>
        <source>Failed to create the output table </source>
        <translation>Не удалось создать выходную таблицу </translation>
    </message>
    <message>
        <source>No GEOS support</source>
        <translation>Поддержка GEOS не установлена</translation>
    </message>
    <message>
        <source>Buffer function requires GEOS support in PostGIS</source>
        <translation>Буферизация объектов PostGIS требует поддержки GEOS</translation>
    </message>
    <message>
        <source>No Active Layer</source>
        <translation>Нет активного слоя</translation>
    </message>
    <message>
        <source>You must select a layer in the legend to buffer</source>
        <translation>Для буферизации необходимо выбрать слой в легенде</translation>
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
    <message>
        <source>Create a buffer for a PostgreSQL layer. </source>
        <translation>Создание буферных зон для слоя PostgreSQL. </translation>
    </message>
</context>
<context>
    <name>QgsPgQueryBuilder</name>
    <message>
        <source>Table &lt;b&gt;%1&lt;/b&gt; in database &lt;b&gt;%2&lt;/b&gt; on host &lt;b&gt;%3&lt;/b&gt;, user &lt;b&gt;%4&lt;/b&gt;</source>
        <translation>Таблица &lt;b&gt;%1&lt;/b&gt; в базе данных &lt;b&gt;%2&lt;/b&gt; на сервере &lt;b&gt;%3&lt;/b&gt;, пользователь &lt;b&gt;%4&lt;/b&gt;</translation>
    </message>
    <message>
        <source>Connection Failed</source>
        <translation>Не удалось соединиться</translation>
    </message>
    <message>
        <source>Connection to the database failed:</source>
        <translation>Не удалось подключиться к базу данных:</translation>
    </message>
    <message>
        <source>Database error</source>
        <translation>Ошибка базы данных</translation>
    </message>
    <message>
        <source>Query Result</source>
        <translation>Результат запроса</translation>
    </message>
    <message>
        <source>The where clause returned </source>
        <translation>По условию WHERE получено </translation>
    </message>
    <message>
        <source> rows.</source>
        <translation> строк.</translation>
    </message>
    <message>
        <source>Query Failed</source>
        <translation>Ошибка запроса</translation>
    </message>
    <message>
        <source>An error occurred when executing the query:</source>
        <translation>При выполнении запроса возникла ошибка:</translation>
    </message>
    <message>
        <source>No Records</source>
        <translation>Нет записей</translation>
    </message>
    <message>
        <source>The query you specified results in zero records being returned. Valid PostgreSQL layers must have at least one feature.</source>
        <translation>По указанному запросу не было получено ни одной записи. Действительные слои PostgreSQL должны содержать как минимум один объект.</translation>
    </message>
    <message>
        <source>&lt;p&gt;Failed to get sample of field values using SQL:&lt;/p&gt;&lt;p&gt;</source>
        <translation>&lt;p&gt;Ошибка получения образцов значений по SQL-запросу:&lt;/p&gt;&lt;p&gt;</translation>
    </message>
    <message>
        <source>No Query</source>
        <translation>Нет запроса</translation>
    </message>
    <message>
        <source>You must create a query before you can test it</source>
        <translation>Следует создать запрос, прежде чем он сможет быть проверен</translation>
    </message>
    <message>
        <source>Error in Query</source>
        <translation>Ошибка запроса</translation>
    </message>
</context>
<context>
    <name>QgsPgQueryBuilderBase</name>
    <message>
        <source>PostgreSQL Query Builder</source>
        <translation>Конструктор запросов PostgreSQL</translation>
    </message>
    <message>
        <source>Clear</source>
        <translation>Очистить</translation>
    </message>
    <message>
        <source>Test</source>
        <translation>Проверка</translation>
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
        <translation>Значения</translation>
    </message>
    <message>
        <source>All</source>
        <translation>Все</translation>
    </message>
    <message>
        <source>Sample</source>
        <translation>Образец</translation>
    </message>
    <message>
        <source>Fields</source>
        <translation>Поля</translation>
    </message>
    <message>
        <source>Operators</source>
        <translation>Операторы</translation>
    </message>
    <message>
        <source>=</source>
        <translation></translation>
    </message>
    <message>
        <source>IN</source>
        <translation></translation>
    </message>
    <message>
        <source>NOT IN</source>
        <translation></translation>
    </message>
    <message>
        <source>&lt;</source>
        <translation></translation>
    </message>
    <message>
        <source>&gt;</source>
        <translation></translation>
    </message>
    <message>
        <source>%</source>
        <translation></translation>
    </message>
    <message>
        <source>&lt;=</source>
        <translation></translation>
    </message>
    <message>
        <source>&gt;=</source>
        <translation></translation>
    </message>
    <message>
        <source>!=</source>
        <translation></translation>
    </message>
    <message>
        <source>LIKE</source>
        <translation></translation>
    </message>
    <message>
        <source>AND</source>
        <translation></translation>
    </message>
    <message>
        <source>ILIKE</source>
        <translation></translation>
    </message>
    <message>
        <source>OR</source>
        <translation></translation>
    </message>
    <message>
        <source>NOT</source>
        <translation></translation>
    </message>
    <message>
        <source>SQL where clause</source>
        <translation>SQL-условие WHERE</translation>
    </message>
    <message>
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
        <source>Datasource</source>
        <translation>Источник данных</translation>
    </message>
</context>
<context>
    <name>QgsPluginInstallerDialog</name>
    <message>
        <source>QGIS Python Plugin Installer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>QGIS Plugin Installer</source>
        <translation type="unfinished">Установка модулей QGIS</translation>
    </message>
    <message>
        <source>Plugins</source>
        <translation type="unfinished">Модули</translation>
    </message>
    <message>
        <source>List of available and installed plugins</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Filter:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Display only plugins containing this word in their metadata</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Display only plugins from given repository</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>all repositories</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Display only plugins with matching status</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Status</source>
        <translation type="unfinished"></translation>
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
        <source>Author</source>
        <translation type="unfinished">Автор</translation>
    </message>
    <message>
        <source>Repository</source>
        <translation type="unfinished">Репозиторий</translation>
    </message>
    <message>
        <source>Install, reinstall or upgrade the selected plugin</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Install/upgrade plugin</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Uninstall the selected plugin</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Uninstall plugin</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Repositories</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>List of plugin repositories</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>URL</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Allow the Installer to look for updates and news in enabled repositories on QGIS startup</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Check for updates on startup</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Add third party plugin repositories to the list</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Add 3rd party repositories</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Add a new plugin repository</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Add...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Edit the selected repository</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Edit...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Remove the selected repository</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Delete</source>
        <translation type="unfinished">Удалить</translation>
    </message>
    <message>
        <source>The plugins will be installed to ~/.qgis/python/plugins</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Close the Installer window</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Close</source>
        <translation type="unfinished">Закрыть</translation>
    </message>
</context>
<context>
    <name>QgsPluginInstallerInstallingDialog</name>
    <message>
        <source>QGIS Python Plugin Installer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Installing plugin:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Connecting...</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsPluginInstallerPluginErrorDialog</name>
    <message>
        <source>Error loading plugin</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>The plugin seems to be invalid or have unfulfilled dependencies. It has been installed, but can&apos;t be loaded. If you really need this plugin, you can contact its author or &lt;a href=&quot;http://lists.osgeo.org/mailman/listinfo/qgis-user&quot;&gt;QGIS users group&lt;/a&gt; and try to solve the problem. If not, you can just uninstall it. Here is the error message below:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Do you want to uninstall this plugin now? If you&apos;re unsure, probably you would like to do this.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QgsPluginInstallerRepositoryDetailsDialog</name>
    <message>
        <source>Repository details</source>
        <translation type="unfinished">Данные репозитория</translation>
    </message>
    <message>
        <source>Name:</source>
        <translation type="unfinished">Имя:</translation>
    </message>
    <message>
        <source>Enter a name for the repository</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>URL:</source>
        <translation type="unfinished">URL:</translation>
    </message>
    <message>
        <source>Enter the repository URL, beginning with &quot;http://&quot;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Enable or disable the repository (disabled repositories will be omitted)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Enabled</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>[place for a warning message]</source>
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
        <source>&amp;Select All</source>
        <translation>В&amp;ыбрать все</translation>
    </message>
    <message>
        <source>&amp;Clear All</source>
        <translation>&amp;Отключить все</translation>
    </message>
</context>
<context>
    <name>QgsPluginManagerBase</name>
    <message>
        <source>QGIS Plugin Manager</source>
        <translation>Менеджер модулей QGIS</translation>
    </message>
    <message>
        <source>To enable / disable a plugin, click its checkbox or description</source>
        <translation>Для включения или выключения модуля, щёлкните на соответствующем ему флажке или описании</translation>
    </message>
    <message>
        <source>&amp;Filter</source>
        <translation>&amp;Фильтр</translation>
    </message>
    <message>
        <source>Plugin Directory:</source>
        <translation>Каталог модулей:</translation>
    </message>
    <message>
        <source>Directory</source>
        <translation>Каталог</translation>
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
        <translation>Панорамировать карту</translation>
    </message>
    <message>
        <source>Pan the map</source>
        <translation>Панорамировать карту</translation>
    </message>
    <message>
        <source>Add Point</source>
        <translation>Добавить точку</translation>
    </message>
    <message>
        <source>.</source>
        <translation>.</translation>
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
        <translation>Не удалось сохранить в </translation>
    </message>
    <message>
        <source>Currently all modified files will be written in TIFF format.</source>
        <translation>В данный момент все изменённые файлы сохраняются в формате TIFF.</translation>
    </message>
    <message>
        <source>&lt;p&gt;A Helmert transform requires modifications in the raster layer.&lt;/p&gt;&lt;p&gt;The modified raster will be saved in a new file and a world file will be generated for this new file instead.&lt;/p&gt;&lt;p&gt;Are you sure that this is what you want?&lt;/p&gt;</source>
        <translation>&lt;p&gt;Преобразование Гельмерта требует изменения исходного слоя.&lt;/p&gt;&lt;p&gt;Изменённый растр будет сохранён в новом файле и файл привязки будет создан уже для нового файла.&lt;/p&gt;&lt;p&gt;Вы уверены, что хотите продолжить?&lt;/p&gt;</translation>
    </message>
    <message>
        <source>-modified</source>
        <comment>Georeferencer:QgsPointDialog.cpp - used to modify a user given file name</comment>
        <translation></translation>
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
    <name>QgsPostgresProvider</name>
    <message>
        <source>Unable to access relation</source>
        <translation>Не удаётся открыть реляцию</translation>
    </message>
    <message>
        <source>Unable to access the </source>
        <translation>Не удаётся открыть реляцию </translation>
    </message>
    <message>
        <source> relation.
The error message from the database was:
</source>
        <translation>.
Сообщение базы данных:
</translation>
    </message>
    <message>
        <source>No suitable key column in table</source>
        <translation>В таблице нет подходящего ключевого поля</translation>
    </message>
    <message>
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
        <source>The unique index on column</source>
        <translation>Уникальный индекс поля</translation>
    </message>
    <message>
        <source>is unsuitable because Qgis does not currently support non-int4 type columns as a key into the table.
</source>
        <translation>непригоден, поскольку QGIS в настоящее время не поддерживает ключевые поля, типом которых не является int4.
</translation>
    </message>
    <message>
        <source>and </source>
        <translation>и </translation>
    </message>
    <message>
        <source>The unique index based on columns </source>
        <translation>Уникальный индекс, состоящий из полей </translation>
    </message>
    <message>
        <source> is unsuitable because Qgis does not currently support multiple columns as a key into the table.
</source>
        <translation> непригоден, поскольку QGIS в настоящее время не поддерживает ключи из нескольких полей.
</translation>
    </message>
    <message>
        <source>Unable to find a key column</source>
        <translation>Не удаётся найти ключевое поле</translation>
    </message>
    <message>
        <source> derives from </source>
        <translation> производное от </translation>
    </message>
    <message>
        <source>and is suitable.</source>
        <translation>пригодно для работы.</translation>
    </message>
    <message>
        <source>and is not suitable </source>
        <translation>не является пригодным </translation>
    </message>
    <message>
        <source>type is </source>
        <translation>тип поля </translation>
    </message>
    <message>
        <source> and has a suitable constraint)</source>
        <translation> и есть подходящее ограничение)</translation>
    </message>
    <message>
        <source> and does not have a suitable constraint)</source>
        <translation> и нет подходящего ограничения)</translation>
    </message>
    <message>
        <source>The view you selected has the following columns, none of which satisfy the above conditions:</source>
        <translation>Вид, который вы выбрали включает следующие поля, ни одно из которых не соответствует вышеприведённым условиям:</translation>
    </message>
    <message>
        <source>Qgis requires that the view has a column that can be used as a unique key. Such a column should be derived from a table column of type int4 and be a primary key, have a unique constraint on it, or be a PostgreSQL oid column. To improve performance the column should also be indexed.
</source>
        <translation>QGIS требует, чтобы вид включал поле, которое можно использовать как уникальный ключ. Такое поле должно происходить от типа int4 и быть первичным ключом с ограничением уникальности или являться служебным полем oid. Для повышения производительности поле также следует проиндексировать.
</translation>
    </message>
    <message>
        <source>The view </source>
        <translation>Вид </translation>
    </message>
    <message>
        <source>has no column suitable for use as a unique key.
</source>
        <translation>не имеет поля, подходящего в качестве уникального ключа.
</translation>
    </message>
    <message>
        <source>No suitable key column in view</source>
        <translation>Подходящий ключ не найден в виде</translation>
    </message>
    <message>
        <source>Unknown geometry type</source>
        <translation>Неизвестный тип геометрии</translation>
    </message>
    <message>
        <source>Column </source>
        <translation>Поле </translation>
    </message>
    <message>
        <source> in </source>
        <translation> в </translation>
    </message>
    <message>
        <source> has a geometry type of </source>
        <translation> имеет тип геометрии </translation>
    </message>
    <message>
        <source>, which Qgis does not currently support.</source>
        <translation>, который QGIS не поддерживает в данный момент.</translation>
    </message>
    <message>
        <source>. The database communication log was:
</source>
        <translation>. История операций с базой данных:
</translation>
    </message>
    <message>
        <source>Unable to get feature type and srid</source>
        <translation>Не удалось получить тип объекта и SRID</translation>
    </message>
    <message>
        <source>Note: </source>
        <translation>Внимание: </translation>
    </message>
    <message>
        <source>initially appeared suitable but does not contain unique data, so is not suitable.
</source>
        <translation>изначально определилось как пригодное, но оказалось непригодным, поскольку не содержит уникальных данных.
</translation>
    </message>
    <message>
        <source>Qgis was unable to determine the type and srid of column </source>
        <translation>Не удалось определить тип и SRID поля </translation>
    </message>
    <message>
        <source>Unable to determine table access privileges for the </source>
        <translation>Не удаётся определить привилегии доступа к таблице для </translation>
    </message>
    <message>
        <source>Error while adding features</source>
        <translation>Ошибка при добавлении объектов</translation>
    </message>
    <message>
        <source>Error while deleting features</source>
        <translation>Ошибка при удалении объектов</translation>
    </message>
    <message>
        <source>Error while adding attributes</source>
        <translation>Ошибка при добавлении атрибутов</translation>
    </message>
    <message>
        <source>Error while deleting attributes</source>
        <translation>Ошибка при удалении атрибутов</translation>
    </message>
    <message>
        <source>Error while changing attributes</source>
        <translation>Ошибка при изменении атрибутов</translation>
    </message>
    <message>
        <source>Error while changing geometry values</source>
        <translation>Ошибка при изменении значений геометрии</translation>
    </message>
    <message>
        <source>unexpected PostgreSQL error</source>
        <translation>неожиданная ошибка PostgreSQL</translation>
    </message>
</context>
<context>
    <name>QgsPostgresProvider::Conn</name>
    <message>
        <source>No GEOS Support!</source>
        <translation>Поддержка GEOS не установлена!</translation>
    </message>
    <message>
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
        <source>Project Properties</source>
        <translation>Свойства проекта</translation>
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
        <source>Default project title</source>
        <translation>Заглавие проекта по умолчанию</translation>
    </message>
    <message>
        <source>General</source>
        <translation>Общие</translation>
    </message>
    <message>
        <source>Automatic</source>
        <translation>Автоматически</translation>
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
        <translation>Вручную</translation>
    </message>
    <message>
        <source>Sets the number of decimal places to use for the mouse position display</source>
        <translation>Число десятичных знаков в поле вывода позиции курсора мыши</translation>
    </message>
    <message>
        <source>The number of decimal places for the manual option</source>
        <translation>Количество десятичных знаков для параметра «Вручную»</translation>
    </message>
    <message>
        <source>decimal places</source>
        <translation>десятичных знаков</translation>
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
        <source>Enable topological editing</source>
        <translation>Включить топологическое редактирование</translation>
    </message>
    <message>
        <source>Snapping options...</source>
        <translation>Параметры прилипания...</translation>
    </message>
    <message>
        <source>Avoid intersections of new polygons</source>
        <translation>Предотвращать пересечение новых полигонов</translation>
    </message>
    <message>
        <source>Title and colors</source>
        <translation>Заглавие и цвета</translation>
    </message>
    <message>
        <source>Project title</source>
        <translation>Заглавие проекта</translation>
    </message>
    <message>
        <source>Selection color</source>
        <translation>Цвет выделения</translation>
    </message>
    <message>
        <source>Background color</source>
        <translation>Цвет фона</translation>
    </message>
    <message>
        <source>Map units</source>
        <translation>Единицы карты</translation>
    </message>
    <message>
        <source>Coordinate Reference System (CRS)</source>
        <translation>Система координат</translation>
    </message>
    <message>
        <source>Enable &apos;on the fly&apos; CRS transformation</source>
        <translation>Включить преобразование координат «на лету»</translation>
    </message>
</context>
<context>
    <name>QgsProjectionSelector</name>
    <message>
        <source>User Defined Coordinate Systems</source>
        <translation>Пользовательские системы координат</translation>
    </message>
    <message>
        <source>Geographic Coordinate Systems</source>
        <translation>Географические системы координат</translation>
    </message>
    <message>
        <source>Projected Coordinate Systems</source>
        <translation>Прямоугольные системы координат</translation>
    </message>
    <message>
        <source>Resource Location Error</source>
        <translation>Ошибка поиска ресурса</translation>
    </message>
    <message>
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
        <source>Search</source>
        <translation>Поиск</translation>
    </message>
    <message>
        <source>Find</source>
        <translation>Найти</translation>
    </message>
    <message>
        <source>EPSG ID</source>
        <translation></translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Имя</translation>
    </message>
    <message>
        <source>Coordinate Reference System Selector</source>
        <translation>Выбор системы координат</translation>
    </message>
    <message>
        <source>Coordinate Reference System</source>
        <translation>Система координат</translation>
    </message>
    <message>
        <source>EPSG</source>
        <translation></translation>
    </message>
    <message>
        <source>ID</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>QgsPythonDialog</name>
    <message>
        <source>Python console</source>
        <translation>Консоль Python</translation>
    </message>
    <message>
        <source>&gt;&gt;&gt;</source>
        <translation></translation>
    </message>
    <message>
        <source>To access Quantum GIS environment from this python console use object from global scope which is an instance of QgisInterface class.&lt;br&gt;Usage e.g.: iface.zoomFull()</source>
        <translation>Для доступа к окружению Quantum GIS из консоли Python, используйте объект из глобального пространства имен, который является экземпляром класса QgisInterface.&lt;br&gt;Например: iface.zoomFull()</translation>
    </message>
</context>
<context>
    <name>QgsQuickPrint</name>
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
        <source> miles</source>
        <translation> миль</translation>
    </message>
    <message>
        <source> mile</source>
        <translation> миля</translation>
    </message>
    <message>
        <source> inches</source>
        <translation>дюймов</translation>
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
</context>
<context>
    <name>QgsRasterLayer</name>
    <message>
        <source>Not Set</source>
        <translation>Не задано</translation>
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
        <translation>Вырезанная область: </translation>
    </message>
    <message>
        <source>Pyramid overviews:</source>
        <translation>Обзор пирамид:</translation>
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
        <translation>Нет статистики</translation>
    </message>
    <message>
        <source>No stats collected yet</source>
        <translation>Сбор статистики не производился</translation>
    </message>
    <message>
        <source>Min Val</source>
        <translation>Мин. значение</translation>
    </message>
    <message>
        <source>Max Val</source>
        <translation>Макс. значение</translation>
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
        <translation>GDT_Byte — 8-битное беззнаковое целое</translation>
    </message>
    <message>
        <source>GDT_UInt16 - Sixteen bit unsigned integer </source>
        <translation>GDT_UInt16 — 16-битное беззнаковое целое </translation>
    </message>
    <message>
        <source>GDT_Int16 - Sixteen bit signed integer </source>
        <translation>GDT_Int16 — 16-битное целое со знаком</translation>
    </message>
    <message>
        <source>GDT_UInt32 - Thirty two bit unsigned integer </source>
        <translation>GDT_UInt32 — 32-битное беззнаковое целое </translation>
    </message>
    <message>
        <source>GDT_Int32 - Thirty two bit signed integer </source>
        <translation>GDT_Int32 — 32-битное целое со знаком </translation>
    </message>
    <message>
        <source>GDT_Float32 - Thirty two bit floating point </source>
        <translation>GDT_Float32 — 32-битное с плавающей точкой</translation>
    </message>
    <message>
        <source>GDT_Float64 - Sixty four bit floating point </source>
        <translation>GDT_Float64 — 64-битное с плавающей точкой </translation>
    </message>
    <message>
        <source>GDT_CInt16 - Complex Int16 </source>
        <translation>GDT_CInt16 — Комплексное Int16 </translation>
    </message>
    <message>
        <source>GDT_CInt32 - Complex Int32 </source>
        <translation>GDT_CInt32 — Комплексное Int32 </translation>
    </message>
    <message>
        <source>GDT_CFloat32 - Complex Float32 </source>
        <translation>GDT_CFloat32 — Комплексное Float32 </translation>
    </message>
    <message>
        <source>GDT_CFloat64 - Complex Float64 </source>
        <translation>GDT_CFloat64 — Комплексное Float64 </translation>
    </message>
    <message>
        <source>Could not determine raster data type.</source>
        <translation>Не удалось установить тип растровых данных.</translation>
    </message>
    <message>
        <source>Average Magphase</source>
        <translation>Среднее отношение магн/фаза</translation>
    </message>
    <message>
        <source>Average</source>
        <translation>Среднее</translation>
    </message>
    <message>
        <source>Layer Spatial Reference System: </source>
        <translation>Система координат слоя: </translation>
    </message>
    <message>
        <source>out of extent</source>
        <translation>за границами</translation>
    </message>
    <message>
        <source>null (no data)</source>
        <translation>null (нет данных)</translation>
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
        <translation>и прочие файлы</translation>
    </message>
    <message>
        <source>NoDataValue not set</source>
        <translation>Значение «нет данных» не задано</translation>
    </message>
    <message>
        <source>Band %1</source>
        <translation>Канал %1</translation>
    </message>
</context>
<context>
    <name>QgsRasterLayerProperties</name>
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
        <source>Not Set</source>
        <translation>Не задано</translation>
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
        <translation>Закрыт доступ на запись</translation>
    </message>
    <message>
        <source>Write access denied. Adjust the file permissions and try again.

</source>
        <translation>Закрыт доступ на запись. Исправьте права доступа к файлу и попробуйте ещё раз.</translation>
    </message>
    <message>
        <source>Building pyramids failed.</source>
        <translation>Не удалось построить пирамиды.</translation>
    </message>
    <message>
        <source>Building pyramid overviews is not supported on this type of raster.</source>
        <translation>Построение пирамид не поддерживается для данного типа растра.</translation>
    </message>
    <message>
        <source>No Stretch</source>
        <translation>Без растяжения</translation>
    </message>
    <message>
        <source>Stretch To MinMax</source>
        <translation>Растяжение до мин/макс</translation>
    </message>
    <message>
        <source>Stretch And Clip To MinMax</source>
        <translation>Растяжение и отсечение по мин/макс</translation>
    </message>
    <message>
        <source>Clip To MinMax</source>
        <translation>Отсечение по мин/макс</translation>
    </message>
    <message>
        <source>Discrete</source>
        <translation>Дискретная</translation>
    </message>
    <message>
        <source>Equal interval</source>
        <translation>Равные интервалы</translation>
    </message>
    <message>
        <source>Quantiles</source>
        <translation>Квантили</translation>
    </message>
    <message>
        <source>Description</source>
        <translation>Описание</translation>
    </message>
    <message>
        <source>Large resolution raster layers can slow navigation in QGIS.</source>
        <translation>Растры высокого разрешения могут замедлить навигацию в QGIS.</translation>
    </message>
    <message>
        <source>By creating lower resolution copies of the data (pyramids) performance can be considerably improved as QGIS selects the most suitable resolution to use depending on the level of zoom.</source>
        <translation>Создание копий данных низкого разрешения (пирамид) позволяет существенно повысить скорость, поскольку QGIS будет автоматически выбирать оптимальное разрешение в зависимости от текущего масштаба.</translation>
    </message>
    <message>
        <source>You must have write access in the directory where the original data is stored to build pyramids.</source>
        <translation>Для сохранения пирамид необходимы права на запись в каталог, в котором хранятся оригинальные данные.</translation>
    </message>
    <message>
        <source>Red</source>
        <translation>Красный</translation>
    </message>
    <message>
        <source>Green</source>
        <translation>Зелёный</translation>
    </message>
    <message>
        <source>Blue</source>
        <translation>Синий</translation>
    </message>
    <message>
        <source>Percent Transparent</source>
        <translation>Процент прозрачности</translation>
    </message>
    <message>
        <source>Gray</source>
        <translation>Серый</translation>
    </message>
    <message>
        <source>Indexed Value</source>
        <translation>Индексированное значение</translation>
    </message>
    <message>
        <source>User Defined</source>
        <translation>Пользовательское</translation>
    </message>
    <message>
        <source>No-Data Value: Not Set</source>
        <translation>Значение «нет данных»: не задано</translation>
    </message>
    <message>
        <source>Save file</source>
        <translation>Сохранить файл</translation>
    </message>
    <message>
        <source>Textfile (*.txt)</source>
        <translation>Текстовые файлы (*.txt)</translation>
    </message>
    <message>
        <source>QGIS Generated Transparent Pixel Value Export File</source>
        <translation>Файл экспорта значений прозрачности пикселей, созданный QGIS</translation>
    </message>
    <message>
        <source>Open file</source>
        <translation>Открыть файл</translation>
    </message>
    <message>
        <source>Import Error</source>
        <translation>Ошибка импорта</translation>
    </message>
    <message>
        <source>The following lines contained errors

</source>
        <translation>Следующие строки содержат ошибки

</translation>
    </message>
    <message>
        <source>Read access denied</source>
        <translation>Закрыт доступ на чтение</translation>
    </message>
    <message>
        <source>Read access denied. Adjust the file permissions and try again.

</source>
        <translation>Закрыт доступ на чтение. Исправьте права доступа к файлу и попробуйте ещё раз.

</translation>
    </message>
    <message>
        <source>Color Ramp</source>
        <translation>Градиент</translation>
    </message>
    <message>
        <source>Default Style</source>
        <translation>Стиль по умолчанию</translation>
    </message>
    <message>
        <source>QGIS Layer Style File (*.qml)</source>
        <translation>Файл стиля QGIS (*.qml)</translation>
    </message>
    <message>
        <source>QGIS</source>
        <translation></translation>
    </message>
    <message>
        <source>Unknown style format: </source>
        <translation>Неизвестный формат стиля: </translation>
    </message>
    <message>
        <source>Please note that building internal pyramids may alter the original data file and once created they cannot be removed!</source>
        <translation>Обратите внимание, что операция построения встроенных пирамид может изменить оригинальный файл данных и их невозможно будет удалить после создания!</translation>
    </message>
    <message>
        <source>Please note that building internal pyramids could corrupt your image - always make a backup of your data first!</source>
        <translation>Помните, что при построении пирамид ваши изображения могут быть повреждены — всегда создавайте резервные копии данных перед этой операцией!</translation>
    </message>
    <message>
        <source>Default</source>
        <translation>По умолчанию</translation>
    </message>
    <message>
        <source>The file was not writeable. Some formats do not support pyramid overviews. Consult the GDAL documentation if in doubt.</source>
        <translation>Запись в файл невозможна. Некоторые форматы не поддерживают обзорные пирамиды. Обратитесь к документации GDAL за дополнительной информацией.</translation>
    </message>
    <message>
        <source>Saved Style</source>
        <translation>Сохранённый стиль</translation>
    </message>
    <message>
        <source>Colormap</source>
        <translation>Цветовая карта</translation>
    </message>
    <message>
        <source>Linear</source>
        <translation>Линейная</translation>
    </message>
    <message>
        <source>Exact</source>
        <translation>Точная</translation>
    </message>
    <message>
        <source>Custom color map entry</source>
        <translation>Пользовательское значение цветовой карты</translation>
    </message>
    <message>
        <source>QGIS Generated Color Map Export File</source>
        <translation>Файл экспорта цветовой карты QGIS</translation>
    </message>
    <message>
        <source>Load Color Map</source>
        <translation>Загрузка цветовой карты</translation>
    </message>
    <message>
        <source>The color map for Band %n failed to load</source>
        <translation type="obsolete">Не удалось загрузить цветовую карту для канала %n
        </translation>
    </message>
</context>
<context>
    <name>QgsRasterLayerPropertiesBase</name>
    <message>
        <source>Raster Layer Properties</source>
        <translation>Свойства растрового слоя</translation>
    </message>
    <message>
        <source>General</source>
        <translation>Общие</translation>
    </message>
    <message>
        <source>No Data:</source>
        <translation>Нет данных:</translation>
    </message>
    <message>
        <source>Symbology</source>
        <translation>Символика</translation>
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
        <source>Metadata</source>
        <translation>Метаданные</translation>
    </message>
    <message>
        <source>Pyramids</source>
        <translation>Пирамиды</translation>
    </message>
    <message>
        <source>Average</source>
        <translation>Среднее значение</translation>
    </message>
    <message>
        <source>Nearest Neighbour</source>
        <translation>Ближайший сосед</translation>
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
        <source>Maximum scale at which this layer will be displayed. </source>
        <translation>Максимальный масштаб, при котором виден данный слой. </translation>
    </message>
    <message>
        <source>Minimum scale at which this layer will be displayed. </source>
        <translation>Минимальный масштаб, при котором виден данный слой. </translation>
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
        <source>Chart Type</source>
        <translation>Тип диаграммы</translation>
    </message>
    <message>
        <source>Refresh</source>
        <translation>Обновить</translation>
    </message>
    <message>
        <source>Max</source>
        <translation>Макс</translation>
    </message>
    <message>
        <source>Min</source>
        <translation>Мин</translation>
    </message>
    <message>
        <source> 00%</source>
        <translation></translation>
    </message>
    <message>
        <source>Render as</source>
        <translation>Отображать как</translation>
    </message>
    <message>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <source>Colormap</source>
        <translation>Цветовая карта</translation>
    </message>
    <message>
        <source>Delete entry</source>
        <translation>Удалить значение</translation>
    </message>
    <message>
        <source>Classify</source>
        <translation>Классифицировать</translation>
    </message>
    <message>
        <source>1</source>
        <translation></translation>
    </message>
    <message>
        <source>2</source>
        <translation></translation>
    </message>
    <message>
        <source>Single band gray</source>
        <translation>Одноканальное серое</translation>
    </message>
    <message>
        <source>Three band color</source>
        <translation>Трёхканальное цветное</translation>
    </message>
    <message>
        <source>RGB mode band selection and scaling</source>
        <translation>Выбор каналов RGB и растяжения</translation>
    </message>
    <message>
        <source>Red band</source>
        <translation>Красный канал</translation>
    </message>
    <message>
        <source>Green band</source>
        <translation>Зелёный канал</translation>
    </message>
    <message>
        <source>Blue band</source>
        <translation>Синий канал</translation>
    </message>
    <message>
        <source>Custom min / max values</source>
        <translation>Пользовательские значения мин/макс</translation>
    </message>
    <message>
        <source>Red min</source>
        <translation>Мин. красный</translation>
    </message>
    <message>
        <source>Red max</source>
        <translation>Макс. красный</translation>
    </message>
    <message>
        <source>Green min</source>
        <translation>Мин. зелёный</translation>
    </message>
    <message>
        <source>Green max</source>
        <translation>Макс. зелёный</translation>
    </message>
    <message>
        <source>Blue min</source>
        <translation>Мин. синий</translation>
    </message>
    <message>
        <source>Blue max</source>
        <translation>Макс. синий</translation>
    </message>
    <message>
        <source>Single band properties</source>
        <translation>Свойства канала</translation>
    </message>
    <message>
        <source>Gray band</source>
        <translation>Канал серого</translation>
    </message>
    <message>
        <source>Color map</source>
        <translation>Цветовая карта</translation>
    </message>
    <message>
        <source>Invert color map</source>
        <translation>Обратить цветовую карту</translation>
    </message>
    <message>
        <source>Use standard deviation</source>
        <translation>Использовать стандартное отклонение</translation>
    </message>
    <message>
        <source>Note:</source>
        <translation>Внимание:</translation>
    </message>
    <message>
        <source>Load min / max values from band</source>
        <translation>Загрузить мин./макс. значения канала</translation>
    </message>
    <message>
        <source>Estimate (faster)</source>
        <translation>Расчётные (быстрее)</translation>
    </message>
    <message>
        <source>Actual (slower)</source>
        <translation>Фактические (медленнее)</translation>
    </message>
    <message>
        <source>Load</source>
        <translation>Загрузить</translation>
    </message>
    <message>
        <source>Contrast enhancement</source>
        <translation>Улучшение контраста</translation>
    </message>
    <message>
        <source>Current</source>
        <translation>Текущее</translation>
    </message>
    <message>
        <source>Save current contrast enhancement algorithm as default. This setting will be persistent between QGIS sessions.</source>
        <translation>Сохранить текущий алгоритм улучшения контраста по умолчанию. Этот параметр будет сохраняться между сеансами работы QGIS.</translation>
    </message>
    <message>
        <source>Saves current contrast enhancement algorithm as a default. This setting will be persistent between QGIS sessions.</source>
        <translation>Сохранить текущий алгоритм улучшения контраста по умолчанию. Этот параметр будет сохраняться между сеансами работы QGIS.</translation>
    </message>
    <message>
        <source>Default</source>
        <translation>По умолчанию</translation>
    </message>
    <message>
        <source>TextLabel</source>
        <translation></translation>
    </message>
    <message>
        <source>Transparency</source>
        <translation>Прозрачность</translation>
    </message>
    <message>
        <source>Global transparency</source>
        <translation>Общая прозрачность</translation>
    </message>
    <message>
        <source>No data value</source>
        <translation>Значение «нет данных»</translation>
    </message>
    <message>
        <source>Reset no data value</source>
        <translation>Сбросить значение «нет данных»</translation>
    </message>
    <message>
        <source>Custom transparency options</source>
        <translation>Параметры прозрачности</translation>
    </message>
    <message>
        <source>Transparency band</source>
        <translation>Канал прозрачности</translation>
    </message>
    <message>
        <source>Transparent pixel list</source>
        <translation>Перечень прозрачных пикселей</translation>
    </message>
    <message>
        <source>Add values manually</source>
        <translation>Добавить значения вручную</translation>
    </message>
    <message>
        <source>Add Values from display</source>
        <translation>Добавить значения с экрана</translation>
    </message>
    <message>
        <source>Remove selected row</source>
        <translation>Удалить выбранную строку</translation>
    </message>
    <message>
        <source>Default values</source>
        <translation>Значения по умолчанию</translation>
    </message>
    <message>
        <source>Import from file</source>
        <translation>Импорт из файла</translation>
    </message>
    <message>
        <source>Export to file</source>
        <translation>Экспорт в файл</translation>
    </message>
    <message>
        <source>Number of entries</source>
        <translation>Количество значений</translation>
    </message>
    <message>
        <source>Color interpolation</source>
        <translation>Интерполяция цветов</translation>
    </message>
    <message>
        <source>Classification mode</source>
        <translation>Режим классификации</translation>
    </message>
    <message>
        <source>Scale dependent visibility</source>
        <translation>Видимость в пределах масштаба</translation>
    </message>
    <message>
        <source>Maximum</source>
        <translation>Максимальный</translation>
    </message>
    <message>
        <source>Minimum</source>
        <translation>Минимальный</translation>
    </message>
    <message>
        <source>Layer source</source>
        <translation>Источник слоя</translation>
    </message>
    <message>
        <source>Display name</source>
        <translation>Имя в легенде</translation>
    </message>
    <message>
        <source>Pyramid resolutions</source>
        <translation>Разрешения пирамид</translation>
    </message>
    <message>
        <source>Resampling method</source>
        <translation>Метод интерполяции</translation>
    </message>
    <message>
        <source>Build pyramids</source>
        <translation>Построить пирамиды</translation>
    </message>
    <message>
        <source>Line graph</source>
        <translation>Линейная</translation>
    </message>
    <message>
        <source>Bar chart</source>
        <translation>Столбчатая</translation>
    </message>
    <message>
        <source>Column count</source>
        <translation>Количество столбцов</translation>
    </message>
    <message>
        <source>Out of range OK?</source>
        <translation>Разрешить значения вне диапазона?</translation>
    </message>
    <message>
        <source>Allow approximation</source>
        <translation>Разрешить аппроксимацию</translation>
    </message>
    <message>
        <source>Restore Default Style</source>
        <translation>Восстановить по умолчанию</translation>
    </message>
    <message>
        <source>Save As Default</source>
        <translation>Сохранить по умолчанию</translation>
    </message>
    <message>
        <source>Load Style ...</source>
        <translation>Загрузить стиль...</translation>
    </message>
    <message>
        <source>Save Style ...</source>
        <translation>Сохранить стиль...</translation>
    </message>
    <message>
        <source>Default R:1 G:2 B:3</source>
        <translation>По умолчанию R:1 G:2 B:3</translation>
    </message>
    <message>
        <source>Coordinate reference system</source>
        <translation>Система координат</translation>
    </message>
    <message>
        <source>Notes</source>
        <translation>Замечания</translation>
    </message>
    <message>
        <source>Build pyramids internally if possible</source>
        <translation>Создавать встроенные пирамиды, если возможно</translation>
    </message>
    <message>
        <source>Add entry</source>
        <translation>Добавить значение</translation>
    </message>
    <message>
        <source>Sort</source>
        <translation>Сортировать</translation>
    </message>
    <message>
        <source>Load color map from band</source>
        <translation>Загрузить цветовую карту из канала</translation>
    </message>
    <message>
        <source>Load color map from file</source>
        <translation>Загрузить цветовую карту из файла</translation>
    </message>
    <message>
        <source>Export color map to file</source>
        <translation>Сохранить цветовую карту в файл</translation>
    </message>
    <message>
        <source>Generate new color map</source>
        <translation>Создать новую цветовую карту</translation>
    </message>
    <message>
        <source>Change ...</source>
        <translation>Изменить...</translation>
    </message>
    <message>
        <source>Legend</source>
        <translation>Легенда</translation>
    </message>
    <message>
        <source>Palette</source>
        <translation>Палитра</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Verdana&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;Sans Serif&apos;; font-size:9pt;&quot;&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation></translation>
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
    <message>
        <source>Action</source>
        <translation>Действие</translation>
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
        <translation>Штрих вниз</translation>
    </message>
    <message>
        <source>Tick Up</source>
        <translation>Штрих вверх</translation>
    </message>
    <message>
        <source>Bar</source>
        <translation>Линия</translation>
    </message>
    <message>
        <source>Box</source>
        <translation>Рамка</translation>
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
        <translation>&amp;Оформление</translation>
    </message>
    <message>
        <source> feet/miles</source>
        <translation> футов/миль</translation>
    </message>
    <message>
        <source> miles</source>
        <translation> миль</translation>
    </message>
    <message>
        <source> mile</source>
        <translation> миля</translation>
    </message>
    <message>
        <source> inches</source>
        <translation>дюймов</translation>
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
        <translation>Штрих вниз</translation>
    </message>
    <message>
        <source>Tick Up</source>
        <translation>Штрих вверх</translation>
    </message>
    <message>
        <source>Box</source>
        <translation>Рамка</translation>
    </message>
    <message>
        <source>Bar</source>
        <translation>Линия</translation>
    </message>
    <message>
        <source>Select the style of the scale bar</source>
        <translation>Выберите стиль масштабной линейки</translation>
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
        <translation>Включить масштабную линейку</translation>
    </message>
    <message>
        <source>Automatically snap to round number on resize</source>
        <translation>Автоматически изменять размер для округления показателя</translation>
    </message>
    <message>
        <source>Click to select the colour</source>
        <translation>Щелкните для выбора цвета</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;This plugin draws a scale bar on the map. Please note the size option below is a &apos;preferred&apos; size and may have to be altered by QGIS depending on the level of zoom.  The size is measured according to the map units specified in the project properties.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Sans Serif; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Этот модуль добавляет к карте масштабную линейку. Обратите внимание, что параметр размера ниже является «предпочтительным» и может быть изменён в зависимости от текущего масштаба.  Размер задаётся в соответствии с единицами карты, указанными в свойствах проекта.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
</context>
<context>
    <name>QgsSearchQueryBuilder</name>
    <message>
        <source>Found %d matching features.</source>
        <translation type="obsolete">Найдено %d подходящих объектов.
        </translation>
    </message>
    <message>
        <source>No matching features found.</source>
        <translation>Подходящих объектов не найдено.</translation>
    </message>
    <message>
        <source>Search results</source>
        <translation>Результаты поиска</translation>
    </message>
    <message>
        <source>Search string parsing error</source>
        <translation>Ошибка обработки строки запроса</translation>
    </message>
    <message>
        <source>No Records</source>
        <translation>Нет записей</translation>
    </message>
    <message>
        <source>The query you specified results in zero records being returned.</source>
        <translation>В результате указанного запроса найдено 0 записей.</translation>
    </message>
    <message>
        <source>Search query builder</source>
        <translation>Конструктор поисковых запросов</translation>
    </message>
</context>
<context>
    <name>QgsServerSourceSelect</name>
    <message>
        <source>Are you sure you want to remove the </source>
        <translation>Вы уверены, что хотите удалить соединение </translation>
    </message>
    <message>
        <source> connection and all associated settings?</source>
        <translation> и все связанные с ним параметры?</translation>
    </message>
    <message>
        <source>Confirm Delete</source>
        <translation>Потвердите удаление</translation>
    </message>
    <message>
        <source>WMS Provider</source>
        <translation>WMS-источник</translation>
    </message>
    <message>
        <source>Could not open the WMS Provider</source>
        <translation>Не удалось открыть WMS-источник</translation>
    </message>
    <message>
        <source>Select Layer</source>
        <translation>Выберите слой</translation>
    </message>
    <message>
        <source>You must select at least one layer first.</source>
        <translation>Для добавления следует выбрать хотя бы один слой.</translation>
    </message>
    <message>
        <source>Coordinate Reference System (%1 available)</source>
        <translation type="obsolete">Система координат (доступно %1)
        </translation>
    </message>
    <message>
        <source>Could not understand the response.  The</source>
        <translation>Ошибка обработки ответа. Источник</translation>
    </message>
    <message>
        <source>provider said</source>
        <translation>сообщил</translation>
    </message>
    <message>
        <source>WMS proxies</source>
        <translation>WMS-прокси</translation>
    </message>
    <message>
        <source>Coordinate Reference System</source>
        <translation>Система координат</translation>
    </message>
    <message>
        <source>There are no available coordinate reference system for the set of layers you&apos;ve selected.</source>
        <translation>Для выбранных слоёв не найдено доступных систем координат.</translation>
    </message>
    <message>
        <source>Several WMS servers have been added to the server list. Note that if you access the internet via a web proxy, you will need to set the proxy settings in the QGIS options dialog.</source>
        <translation>Несколько WMS-серверов было добавлено в список. Обратите внимание, что если вы выходите в интернет через прокси-сервер, его необходимо указать в диалоге настроек QGIS.</translation>
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
        <translation>&amp;Создать</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>Удалить</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation>Изменить</translation>
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
        <translation>База данных вернула ошибку при выполнении SQL:</translation>
    </message>
    <message>
        <source>The error was:</source>
        <translation>Сообщение об ошибке:</translation>
    </message>
    <message>
        <source>... (rest of SQL trimmed)</source>
        <comment>is appended to a truncated SQL statement</comment>
        <translation>...(остаток SQL проигнорирован)</translation>
    </message>
    <message>
        <source>Scanning </source>
        <translation>Сканирование </translation>
    </message>
</context>
<context>
    <name>QgsSingleSymbolDialog</name>
    <message>
        <source>Solid Line</source>
        <translation>Сплошная линия</translation>
    </message>
    <message>
        <source>Dash Line</source>
        <translation>Штриховой пунктир</translation>
    </message>
    <message>
        <source>Dot Line</source>
        <translation>Точечный пунктир</translation>
    </message>
    <message>
        <source>Dash Dot Line</source>
        <translation>Штрихпунктир</translation>
    </message>
    <message>
        <source>Dash Dot Dot Line</source>
        <translation>Штрихпунктир с двумя точками</translation>
    </message>
    <message>
        <source>No Pen</source>
        <translation>Без линии</translation>
    </message>
    <message>
        <source>No Brush</source>
        <translation>Без заливки</translation>
    </message>
    <message>
        <source>Solid</source>
        <translation>Сплошная</translation>
    </message>
    <message>
        <source>Horizontal</source>
        <translation>Горизонтальный шаблон</translation>
    </message>
    <message>
        <source>Vertical</source>
        <translation>Вертикальный шаблон</translation>
    </message>
    <message>
        <source>Cross</source>
        <translation>Перекрестие</translation>
    </message>
    <message>
        <source>BDiagonal</source>
        <translation>Обратная диагональная</translation>
    </message>
    <message>
        <source>FDiagonal</source>
        <translation>Прямая диагональная</translation>
    </message>
    <message>
        <source>Diagonal X</source>
        <translation>Перекрестие по диагонали</translation>
    </message>
    <message>
        <source>Dense1</source>
        <translation>Штриховка 1</translation>
    </message>
    <message>
        <source>Dense2</source>
        <translation>Штриховка 2</translation>
    </message>
    <message>
        <source>Dense3</source>
        <translation>Штриховка 3</translation>
    </message>
    <message>
        <source>Dense4</source>
        <translation>Штриховка 4</translation>
    </message>
    <message>
        <source>Dense5</source>
        <translation>Штриховка 5</translation>
    </message>
    <message>
        <source>Dense6</source>
        <translation>Штриховка 6</translation>
    </message>
    <message>
        <source>Dense7</source>
        <translation>Штриховка 7</translation>
    </message>
    <message>
        <source>Texture</source>
        <translation>Текстурой</translation>
    </message>
</context>
<context>
    <name>QgsSingleSymbolDialogBase</name>
    <message>
        <source>Single Symbol</source>
        <translation>Обычный знак</translation>
    </message>
    <message>
        <source>Size</source>
        <translation>Размер</translation>
    </message>
    <message>
        <source>Point Symbol</source>
        <translation>Значок</translation>
    </message>
    <message>
        <source>Area scale field</source>
        <translation>Поле масштаба</translation>
    </message>
    <message>
        <source>Rotation field</source>
        <translation>Поле вращения</translation>
    </message>
    <message>
        <source>Style Options</source>
        <translation>Параметры стиля</translation>
    </message>
    <message>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <source>Outline style</source>
        <translation>Стиль линии</translation>
    </message>
    <message>
        <source>Outline color</source>
        <translation>Цвет линии</translation>
    </message>
    <message>
        <source>Outline width</source>
        <translation>Ширина линии</translation>
    </message>
    <message>
        <source>Fill color</source>
        <translation>Цвет заливки</translation>
    </message>
    <message>
        <source>Fill style</source>
        <translation>Стиль заливки</translation>
    </message>
    <message>
        <source>Label</source>
        <translation>Метка</translation>
    </message>
</context>
<context>
    <name>QgsSnappingDialog</name>
    <message>
        <source>to vertex</source>
        <translation>к вершинам</translation>
    </message>
    <message>
        <source>to segment</source>
        <translation>к сегментам</translation>
    </message>
    <message>
        <source>to vertex and segment</source>
        <translation>к вершинам и сегментам</translation>
    </message>
</context>
<context>
    <name>QgsSnappingDialogBase</name>
    <message>
        <source>Snapping options</source>
        <translation>Параметры прилипания</translation>
    </message>
    <message>
        <source>Layer</source>
        <translation>Слой</translation>
    </message>
    <message>
        <source>Mode</source>
        <translation>Режим</translation>
    </message>
    <message>
        <source>Tolerance</source>
        <translation>Порог</translation>
    </message>
</context>
<context>
    <name>QgsSpit</name>
    <message>
        <source>Are you sure you want to remove the [</source>
        <translation>Вы уверены, что хотите удалить соединение [</translation>
    </message>
    <message>
        <source>] connection and all associated settings?</source>
        <translation>] и все связанные с ним параметры?</translation>
    </message>
    <message>
        <source>Confirm Delete</source>
        <translation>Потвердите удаление</translation>
    </message>
    <message>
        <source>The following Shapefile(s) could not be loaded:

</source>
        <translation>Не удалось загрузить следующие shape-файлы:

</translation>
    </message>
    <message>
        <source>REASON: File cannot be opened</source>
        <translation>ПРИЧИНА: Файл не может быть открыт</translation>
    </message>
    <message>
        <source>REASON: One or both of the Shapefile files (*.dbf, *.shx) missing</source>
        <translation>ПРИЧИНА: Один или оба дополнительных файла (*.dbf, *.shx) отсутствуют</translation>
    </message>
    <message>
        <source>General Interface Help:</source>
        <translation>Общая справка по интерфейсу:</translation>
    </message>
    <message>
        <source>PostgreSQL Connections:</source>
        <translation>Соединения PostgreSQL:</translation>
    </message>
    <message>
        <source>[New ...] - create a new connection</source>
        <translation>[Создать...] — создать новое соединение</translation>
    </message>
    <message>
        <source>[Edit ...] - edit the currently selected connection</source>
        <translation>[Изменить...] — редактировать выбранное соединение</translation>
    </message>
    <message>
        <source>[Remove] - remove the currently selected connection</source>
        <translation>[Удалить] — удалить выбранное соединение</translation>
    </message>
    <message>
        <source>-you need to select a connection that works (connects properly) in order to import files</source>
        <translation>- для успешного импорта необходимо выбрать действительное (рабочее) соединение</translation>
    </message>
    <message>
        <source>-when changing connections Global Schema also changes accordingly</source>
        <translation>- при изменении соединения общая схема также изменяется</translation>
    </message>
    <message>
        <source>Shapefile List:</source>
        <translation>Список shape-файлов:</translation>
    </message>
    <message>
        <source>[Add ...] - open a File dialog and browse to the desired file(s) to import</source>
        <translation>[Добавить...]  —  выбрать файл(ы) для импорта в диалоге открытия файлов</translation>
    </message>
    <message>
        <source>[Remove] - remove the currently selected file(s) from the list</source>
        <translation>[Удалить] — удалить выбранные файлы из списка</translation>
    </message>
    <message>
        <source>[Remove All] - remove all the files in the list</source>
        <translation>[Удалить все] — удалить все файлы из списка</translation>
    </message>
    <message>
        <source>[SRID] - Reference ID for the shapefiles to be imported</source>
        <translation>[SRID] — ID системы координат для загружаемых shape-файлов</translation>
    </message>
    <message>
        <source>[Use Default (SRID)] - set SRID to -1</source>
        <translation>[SRID по умолчанию] — использовать значение -1 для SRID</translation>
    </message>
    <message>
        <source>[Geometry Column Name] - name of the geometry column in the database</source>
        <translation>[Имя поля геометрии] — имя поля геометрии в базе данных</translation>
    </message>
    <message>
        <source>[Use Default (Geometry Column Name)] - set column name to &apos;the_geom&apos;</source>
        <translation>[Поле геометрии по умолчанию] — использовать значение «the_geom» для поля геометрии</translation>
    </message>
    <message>
        <source>[Glogal Schema] - set the schema for all files to be imported into</source>
        <translation>[Общая схема] — схема, в которую будут загружены все указанные файлы</translation>
    </message>
    <message>
        <source>[Import] - import the current shapefiles in the list</source>
        <translation>[Импорт] — импортировать файлы, указанные в списке</translation>
    </message>
    <message>
        <source>[Quit] - quit the program
</source>
        <translation>[Выйти] — выйти из программы
</translation>
    </message>
    <message>
        <source>[Help] - display this help dialog</source>
        <translation>[Справка] — вывести этот диалог справки</translation>
    </message>
    <message>
        <source>Import Shapefiles</source>
        <translation>Импорт shape-файлов</translation>
    </message>
    <message>
        <source>You need to specify a Connection first</source>
        <translation>Необходимо указать соединение</translation>
    </message>
    <message>
        <source>Connection failed - Check settings and try again</source>
        <translation>Не удалось соединиться — проверьте параметры и попробуйте ещё раз </translation>
    </message>
    <message>
        <source>You need to add shapefiles to the list first</source>
        <translation>Необходимо добавить shape-файлы в список</translation>
    </message>
    <message>
        <source>Importing files</source>
        <translation>Импорт файлов</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Отменить</translation>
    </message>
    <message>
        <source>Progress</source>
        <translation>Прогресс</translation>
    </message>
    <message>
        <source>Problem inserting features from file:</source>
        <translation>Проблема при вставке объектов из файла:</translation>
    </message>
    <message>
        <source>Invalid table name.</source>
        <translation>Неверное имя таблицы.</translation>
    </message>
    <message>
        <source>No fields detected.</source>
        <translation>Поля не выбраны.</translation>
    </message>
    <message>
        <source>The following fields are duplicates:</source>
        <translation>Следующие поля являются дубликатами:</translation>
    </message>
    <message>
        <source>Import Shapefiles - Relation Exists</source>
        <translation>Импорт shape-файлов — реляция существует</translation>
    </message>
    <message>
        <source>The Shapefile:</source>
        <translation>Shape-файл:</translation>
    </message>
    <message>
        <source>will use [</source>
        <translation>будет загружен в реляцию [</translation>
    </message>
    <message>
        <source>] relation for its data,</source>
        <translation>],</translation>
    </message>
    <message>
        <source>which already exists and possibly contains data.</source>
        <translation>которая уже существует и возможно содержит данные.</translation>
    </message>
    <message>
        <source>To avoid data loss change the &quot;DB Relation Name&quot;</source>
        <translation>Во избежание потери данных, измените «Имя реляции БД»</translation>
    </message>
    <message>
        <source>for this Shapefile in the main dialog file list.</source>
        <translation>для этого shape-файла в списке файлов главного диалога.</translation>
    </message>
    <message>
        <source>Do you want to overwrite the [</source>
        <translation>Вы хотите перезаписать реляцию [</translation>
    </message>
    <message>
        <source>] relation?</source>
        <translation>]?</translation>
    </message>
    <message>
        <source>File Name</source>
        <translation>Имя файла</translation>
    </message>
    <message>
        <source>Feature Class</source>
        <translation>Класс объектов</translation>
    </message>
    <message>
        <source>Features</source>
        <translation>Объекты</translation>
    </message>
    <message>
        <source>DB Relation Name</source>
        <translation>Имя реляции БД</translation>
    </message>
    <message>
        <source>Schema</source>
        <translation>Схема</translation>
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
        <translation>PostGIS недоступна</translation>
    </message>
    <message>
        <source>&lt;p&gt;The chosen database does not have PostGIS installed, but this is required for storage of spatial data.&lt;/p&gt;</source>
        <translation>&lt;p&gt;PostGIS не установлен в выбранной БД, что делает невозможным хранение пространственных данных.&lt;/p&gt;</translation>
    </message>
    <message>
        <source>&lt;p&gt;Error while executing the SQL:&lt;/p&gt;&lt;p&gt;</source>
        <translation>&lt;p&gt;Ошибка при выполнении SQL:&lt;/p&gt;&lt;p&gt;</translation>
    </message>
    <message>
        <source>&lt;/p&gt;&lt;p&gt;The database said:</source>
        <translation>&lt;/p&gt;&lt;p&gt;Сообщение БД:</translation>
    </message>
    <message>
        <source>%1 of %2 shapefiles could not be imported.</source>
        <translation>Не удалось загрузить %1 из %2 shape-файлов.</translation>
    </message>
    <message>
        <source>Password for </source>
        <translation>Пароль для </translation>
    </message>
    <message>
        <source>Please enter your password:</source>
        <translation>Пожалуйста, введите ваш пароль:</translation>
    </message>
</context>
<context>
    <name>QgsSpitBase</name>
    <message>
        <source>SPIT - Shapefile to PostGIS Import Tool</source>
        <translation>SPIT — инструмент импорта shape-файлов в PostGIS</translation>
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
        <source>Remove All</source>
        <translation>Удалить все</translation>
    </message>
    <message>
        <source>Global Schema</source>
        <translation>Общая схема</translation>
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
        <source>Set the SRID to the default value</source>
        <translation>Заполнить SRID значением по умолчанию</translation>
    </message>
    <message>
        <source>Set the geometry column name to the default value</source>
        <translation>Задать имя поля геометрии в соответствии со значением по умолчанию</translation>
    </message>
    <message>
        <source>New</source>
        <translation>Создать</translation>
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
        <translation>Подключиться</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation>Изменить</translation>
    </message>
    <message>
        <source>Edit the current PostGIS connection</source>
        <translation>Редактировать текущее PostGIS-соединение</translation>
    </message>
    <message>
        <source>Import options and shapefile list</source>
        <translation>Параметры импорта и список shape-файлов</translation>
    </message>
    <message>
        <source>Use Default SRID or specify here</source>
        <translation>Использовать SRID по умолчанию или указанный</translation>
    </message>
    <message>
        <source>Use Default Geometry Column Name or specify here</source>
        <translation>Использовать поле геометрии по умолчанию или указанное</translation>
    </message>
    <message>
        <source>Primary Key Column Name</source>
        <translation>Имя первичного ключевого поля</translation>
    </message>
    <message>
        <source>Connect to PostGIS</source>
        <translation>Подключиться к PostGIS</translation>
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
        <translation>&amp;SPIT</translation>
    </message>
</context>
<context>
    <name>QgsTINInterpolatorDialog</name>
    <message>
        <source>Linear interpolation</source>
        <translation>Линейная интерполяция</translation>
    </message>
</context>
<context>
    <name>QgsTINInterpolatorDialogBase</name>
    <message>
        <source>Triangle based interpolation</source>
        <translation>Триангуляция</translation>
    </message>
    <message>
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
        <source>Interpolation method:</source>
        <translation>Метод интерполяции:</translation>
    </message>
</context>
<context>
    <name>QgsUniqueValueDialog</name>
    <message>
        <source>Confirm Delete</source>
        <translation>Потвердите удаление</translation>
    </message>
    <message>
        <source>The classification field was changed from &apos;%1&apos; to &apos;%2&apos;.
Should the existing classes be deleted before classification?</source>
        <translation>Поле классификации было изменено с «%1» на «%2».
Следует ли удалить существующие классы перед классификацией?</translation>
    </message>
</context>
<context>
    <name>QgsUniqueValueDialogBase</name>
    <message>
        <source>Form1</source>
        <translation></translation>
    </message>
    <message>
        <source>Classify</source>
        <translation>Классифицировать</translation>
    </message>
    <message>
        <source>Classification field</source>
        <translation>Поле классификации</translation>
    </message>
    <message>
        <source>Add class</source>
        <translation>Добавить класс</translation>
    </message>
    <message>
        <source>Delete classes</source>
        <translation>Удалить классы</translation>
    </message>
    <message>
        <source>Randomize Colors</source>
        <translation>Перемешать цвета</translation>
    </message>
    <message>
        <source>Reset Colors</source>
        <translation>Сбросить цвета</translation>
    </message>
</context>
<context>
    <name>QgsVectorLayer</name>
    <message>
        <source>ERROR: no provider</source>
        <translation>ОШИБКА: отсутствует источник</translation>
    </message>
    <message>
        <source>ERROR: layer not editable</source>
        <translation>ОШИБКА: нередактируемый слой</translation>
    </message>
    <message>
        <source>SUCCESS: %1 attributes added.</source>
        <translation>УСПЕХ: добавлено %1 атрибутов.</translation>
    </message>
    <message>
        <source>ERROR: %1 new attributes not added</source>
        <translation>ОШИБКА: не добавлено %1 новых атрибутов</translation>
    </message>
    <message>
        <source>SUCCESS: %1 attributes deleted.</source>
        <translation>УСПЕХ: удалено %1 атрибутов.</translation>
    </message>
    <message>
        <source>ERROR: %1 attributes not deleted.</source>
        <translation>ОШИБКА: %1 атрибутов не было удалено.</translation>
    </message>
    <message>
        <source>SUCCESS: attribute %1 was added.</source>
        <translation>УСПЕХ: добавлен атрибут %1.</translation>
    </message>
    <message>
        <source>ERROR: attribute %1 not added</source>
        <translation>ОШИБКА: атрибут %1 не был добавлен</translation>
    </message>
    <message>
        <source>SUCCESS: %1 attribute values changed.</source>
        <translation>УСПЕХ: изменено %1 значений атрибутов.</translation>
    </message>
    <message>
        <source>ERROR: %1 attribute value changes not applied.</source>
        <translation>ОШИБКА: не применено %1 изменений значений атрибутов.</translation>
    </message>
    <message>
        <source>SUCCESS: %1 features added.</source>
        <translation>УСПЕХ: добавлено %1 объектов.</translation>
    </message>
    <message>
        <source>ERROR: %1 features not added.</source>
        <translation>ОШИБКА: не добавлено %1 объектов.</translation>
    </message>
    <message>
        <source>SUCCESS: %1 geometries were changed.</source>
        <translation>УСПЕХ: изменено %1 геометрий.</translation>
    </message>
    <message>
        <source>ERROR: %1 geometries not changed.</source>
        <translation>ОШИБКА: не изменено %1 геометрий.</translation>
    </message>
    <message>
        <source>SUCCESS: %1 features deleted.</source>
        <translation>УСПЕХ: удалено %1 объектов.</translation>
    </message>
    <message>
        <source>ERROR: %1 features not deleted.</source>
        <translation>ОШИБКА: не удалено %1 объектов.</translation>
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
        <translation>Эта кнопка открывает конструктор запросов PostgreSQL, при помощи которого можно выбрать подмножество объектов для отображения на карте, иначе все объекты будут видимы</translation>
    </message>
    <message>
        <source>The query used to limit the features in the layer is shown here. This is currently only supported for PostgreSQL layers. To enter or modify the query, click on the Query Builder button</source>
        <translation>Этот запрос используется для ограничения доступа к объектам слоя. В данный момент поддерживаются только слои PostgreSQL. Для создания или изменения запроса кликните на кнопке «Конструктор запросов»</translation>
    </message>
    <message>
        <source>Spatial Index</source>
        <translation>Пространственный индекс</translation>
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
        <translation>Стиль по умолчанию</translation>
    </message>
    <message>
        <source>QGIS Layer Style File (*.qml)</source>
        <translation>Файл стиля слоя QGIS (*.qml)</translation>
    </message>
    <message>
        <source>QGIS</source>
        <translation></translation>
    </message>
    <message>
        <source>Unknown style format: </source>
        <translation>Неизвестный формат стиля: </translation>
    </message>
    <message>
        <source>id</source>
        <translation></translation>
    </message>
    <message>
        <source>name</source>
        <translation>имя</translation>
    </message>
    <message>
        <source>type</source>
        <translation>тип</translation>
    </message>
    <message>
        <source>length</source>
        <translation>длина</translation>
    </message>
    <message>
        <source>precision</source>
        <translation>точность</translation>
    </message>
    <message>
        <source>comment</source>
        <translation>комментарий</translation>
    </message>
    <message>
        <source>edit widget</source>
        <translation>элемент редактирования</translation>
    </message>
    <message>
        <source>values</source>
        <translation>значения</translation>
    </message>
    <message>
        <source>line edit</source>
        <translation>строчное редактирование</translation>
    </message>
    <message>
        <source>unique values</source>
        <translation>уникальные значения</translation>
    </message>
    <message>
        <source>unique values (editable)</source>
        <translation>уникальные значения (редактируемые)</translation>
    </message>
    <message>
        <source>value map</source>
        <translation>карта значений</translation>
    </message>
    <message>
        <source>classification</source>
        <translation>классификация</translation>
    </message>
    <message>
        <source>Name conflict</source>
        <translation>Конфликт имён</translation>
    </message>
    <message>
        <source>The attribute could not be inserted. The name already exists in the table.</source>
        <translation>Не удалось вставить атрибут. Данное имя уже существует в таблице.</translation>
    </message>
    <message>
        <source>Saved Style</source>
        <translation>Сохранённый стиль</translation>
    </message>
    <message>
        <source>range (editable)</source>
        <translation>диапазон (редактируемый)</translation>
    </message>
    <message>
        <source>range (slider)</source>
        <translation>диапазон (ползунок)</translation>
    </message>
    <message>
        <source>Creation of spatial index successful</source>
        <translation> Пространственный индекс успешно создан</translation>
    </message>
    <message>
        <source>file name</source>
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
        <source>Symbology</source>
        <translation>Символика</translation>
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
        <translation>Используйте этот список для выбора поля, помещаемого в верхний уровень дерева в диалоге результатов определения.</translation>
    </message>
    <message>
        <source>Display field for the Identify Results dialog box</source>
        <translation>Отображаемое поле для диалога результатов определения</translation>
    </message>
    <message>
        <source>This sets the display field for the Identify Results dialog box</source>
        <translation>Отображаемое поле для диалога результатов определения</translation>
    </message>
    <message>
        <source>Display field</source>
        <translation>Отображаемое поле</translation>
    </message>
    <message>
        <source>Subset</source>
        <translation>Подмножество</translation>
    </message>
    <message>
        <source>Query Builder</source>
        <translation>Конструктор запросов</translation>
    </message>
    <message>
        <source>Create Spatial Index</source>
        <translation>Создать пространственный индекс</translation>
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
        <translation>Восстановить по умолчанию</translation>
    </message>
    <message>
        <source>Save As Default</source>
        <translation>Сохранить по умолчанию</translation>
    </message>
    <message>
        <source>Load Style ...</source>
        <translation>Загрузить стиль...</translation>
    </message>
    <message>
        <source>Save Style ...</source>
        <translation>Сохранить стиль...</translation>
    </message>
    <message>
        <source>Legend type</source>
        <translation>Тип легенды</translation>
    </message>
    <message>
        <source>Transparency</source>
        <translation>Прозрачность</translation>
    </message>
    <message>
        <source>Options</source>
        <translation>Параметры</translation>
    </message>
    <message>
        <source>Maximum</source>
        <translation>Максимальный</translation>
    </message>
    <message>
        <source>Minimum</source>
        <translation>Минимальный</translation>
    </message>
    <message>
        <source>Change CRS</source>
        <translation>Система координат</translation>
    </message>
    <message>
        <source>Attributes</source>
        <translation>Атрибуты</translation>
    </message>
    <message>
        <source>New column</source>
        <translation>Новое поле</translation>
    </message>
    <message>
        <source>Ctrl+N</source>
        <translation></translation>
    </message>
    <message>
        <source>Delete column</source>
        <translation>Удалить поле</translation>
    </message>
    <message>
        <source>Ctrl+X</source>
        <translation></translation>
    </message>
    <message>
        <source>Toggle editing mode</source>
        <translation>Режим редактирования</translation>
    </message>
    <message>
        <source>Click to toggle table editing</source>
        <translation>Переключить редактирование таблицы</translation>
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
        <translation> неизвестно</translation>
    </message>
    <message>
        <source>received %1 bytes from %2</source>
        <translation>получено %1 из %2 байт</translation>
    </message>
</context>
<context>
    <name>QgsWFSSourceSelect</name>
    <message>
        <source>Are you sure you want to remove the </source>
        <translation>Вы уверены, что хотите удалить соединение </translation>
    </message>
    <message>
        <source> connection and all associated settings?</source>
        <translation> и все связанные с ним параметры?</translation>
    </message>
    <message>
        <source>Confirm Delete</source>
        <translation>Потвердите удаление</translation>
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
        <translation>&amp;Создать</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>Удалить</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation>Изменить</translation>
    </message>
    <message>
        <source>C&amp;onnect</source>
        <translation>&amp;Подключить</translation>
    </message>
    <message>
        <source>Add WFS Layer from a Server</source>
        <translation>Добавить WFS-слой</translation>
    </message>
</context>
<context>
    <name>QgsWmsProvider</name>
    <message>
        <source>Tried URL: </source>
        <translation>Используемый URL: </translation>
    </message>
    <message>
        <source>HTTP Exception</source>
        <translation>HTTP-исключение</translation>
    </message>
    <message>
        <source>WMS Service Exception</source>
        <translation>Исключение WMS-службы</translation>
    </message>
    <message>
        <source>Could not get WMS capabilities: %1 at line %2 column %3</source>
        <translation>Не удалось получить возможности WMS: %1 в строке %2, столбце %3</translation>
    </message>
    <message>
        <source>This is probably due to an incorrect WMS Server URL.</source>
        <translation>Вероятнее всего, адрес WMS-сервера неверен.</translation>
    </message>
    <message>
        <source>Could not get WMS capabilities in the expected format (DTD): no %1 or %2 found</source>
        <translation>Не удалось получить возможности WMS в ожидаемом формате (DTD): %1 или %2 не найдены</translation>
    </message>
    <message>
        <source>Could not get WMS Service Exception at %1: %2 at line %3 column %4</source>
        <translation>Не удалось получить ошибку WMS из %1: %2 в строке %3, столбец %4</translation>
    </message>
    <message>
        <source>Request contains a Format not offered by the server.</source>
        <translation>Запрос требует формата, который не поддерживается сервером.</translation>
    </message>
    <message>
        <source>Request contains a CRS not offered by the server for one or more of the Layers in the request.</source>
        <translation>Запрос включает систему координат, которая не поддерживается сервером для одного или более слоёв.</translation>
    </message>
    <message>
        <source>GetMap request is for a Layer not offered by the server, or GetFeatureInfo request is for a Layer not shown on the map.</source>
        <translation>В запросе GetMap указан слой, не предлагаемый сервером, или в запросе GetFeatureInfo указан слой, не показанный на карте.</translation>
    </message>
    <message>
        <source>Request is for a Layer in a Style not offered by the server.</source>
        <translation>Запрос требует слой в стиле, который недоступен на сервере.</translation>
    </message>
    <message>
        <source>GetFeatureInfo request is applied to a Layer which is not declared queryable.</source>
        <translation>Попытка запроса GetFeatureInfo для слоя, который не поддерживает запросов.</translation>
    </message>
    <message>
        <source>GetFeatureInfo request contains invalid X or Y value.</source>
        <translation>Запрос GetFeatureInfo содержит недействительные значения X или Y.</translation>
    </message>
    <message>
        <source>Value of (optional) UpdateSequence parameter in GetCapabilities request is equal to current value of service metadata update sequence number.</source>
        <translation>Значение необязательного параметра UpdateSequence в запросе GetCapabilities равно текущему номеру последовательности обновления в метаданных службы.</translation>
    </message>
    <message>
        <source>Value of (optional) UpdateSequence parameter in GetCapabilities request is greater than current value of service metadata update sequence number.</source>
        <translation>Значение необязательного параметра UpdateSequence в запросе GetCapabilities выше, чем текущий номер последовательности обновления в метаданных службы.</translation>
    </message>
    <message>
        <source>Request does not include a sample dimension value, and the server did not declare a default value for that dimension.</source>
        <translation>Запрос не включает образец величины, и значение этой величины по умолчанию не указано сервером.</translation>
    </message>
    <message>
        <source>Request contains an invalid sample dimension value.</source>
        <translation>Запрос включает недействительный образец величины.</translation>
    </message>
    <message>
        <source>Request is for an optional operation that is not supported by the server.</source>
        <translation>Запрос необязательной операции, которая не поддерживается сервером.</translation>
    </message>
    <message>
        <source>(Unknown error code from a post-1.3 WMS server)</source>
        <translation>(Неизвестный код ошибки от WMS-сервера &gt; 1.3)</translation>
    </message>
    <message>
        <source>The WMS vendor also reported: </source>
        <translation>Дополнительное сообщение WMS-провайдера: </translation>
    </message>
    <message>
        <source>Server Properties:</source>
        <translation>Свойства сервера:</translation>
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
        <source>WMS Version</source>
        <translation>Версия WMS</translation>
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
        <source>Keywords</source>
        <translation>Ключевые слова</translation>
    </message>
    <message>
        <source>Online Resource</source>
        <translation>Онлайн-ресурс</translation>
    </message>
    <message>
        <source>Contact Person</source>
        <translation>Контактное лицо</translation>
    </message>
    <message>
        <source>Fees</source>
        <translation>Плата</translation>
    </message>
    <message>
        <source>Access Constraints</source>
        <translation>Ограничения доступа</translation>
    </message>
    <message>
        <source>Image Formats</source>
        <translation>Форматы изображения</translation>
    </message>
    <message>
        <source>Identify Formats</source>
        <translation>Форматы запроса</translation>
    </message>
    <message>
        <source>Layer Count</source>
        <translation>Количество слоёв</translation>
    </message>
    <message>
        <source>Layer Properties: </source>
        <translation>Свойства слоя: </translation>
    </message>
    <message>
        <source>Selected</source>
        <translation>Выбран</translation>
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
        <translation>Видимость</translation>
    </message>
    <message>
        <source>Visible</source>
        <translation>Видимый</translation>
    </message>
    <message>
        <source>Hidden</source>
        <translation>Скрытый</translation>
    </message>
    <message>
        <source>n/a</source>
        <translation>н/д</translation>
    </message>
    <message>
        <source>Can Identify</source>
        <translation>Можно определять</translation>
    </message>
    <message>
        <source>Can be Transparent</source>
        <translation>Может быть прозрачным</translation>
    </message>
    <message>
        <source>Can Zoom In</source>
        <translation>Можно увеличивать</translation>
    </message>
    <message>
        <source>Cascade Count</source>
        <translation>Количество каскадов</translation>
    </message>
    <message>
        <source>Fixed Width</source>
        <translation>Фикс. ширина</translation>
    </message>
    <message>
        <source>Fixed Height</source>
        <translation>Фикс. высота</translation>
    </message>
    <message>
        <source>WGS 84 Bounding Box</source>
        <translation>Рамка WGS 84</translation>
    </message>
    <message>
        <source>Available in CRS</source>
        <translation>Доступен в CRS</translation>
    </message>
    <message>
        <source>Available in style</source>
        <translation>Доступен в стиле</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>Имя</translation>
    </message>
    <message>
        <source>Layer cannot be queried.</source>
        <translation>Не удаётся опросить слой.</translation>
    </message>
    <message>
        <source>Dom Exception</source>
        <translation>DOM-исключение</translation>
    </message>
    <message>
        <source>Request contains a SRS not offered by the server for one or more of the Layers in the request.</source>
        <translation>Запрос включает систему координат, которая не поддерживается сервером для одного или более слоёв.</translation>
    </message>
</context>
<context>
    <name>QuickPrintGui</name>
    <message>
        <source>Portable Document Format (*.pdf)</source>
        <translation></translation>
    </message>
    <message>
        <source>quickprint</source>
        <translation></translation>
    </message>
    <message>
        <source>Unknown format: </source>
        <translation>Неизвестный формат: </translation>
    </message>
</context>
<context>
    <name>QuickPrintGuiBase</name>
    <message>
        <source>QGIS Quick Print Plugin</source>
        <translation>Модуль быстрой печати QGIS</translation>
    </message>
    <message>
        <source>Quick Print</source>
        <translation>Быстрая печать</translation>
    </message>
    <message>
        <source>Map Title e.g. ACME inc.</source>
        <translation>Заголовок карты (напр. «ACME inc.»).</translation>
    </message>
    <message>
        <source>Map Name e.g. Water Features</source>
        <translation>Имя карты (напр. «Водные объекты»)</translation>
    </message>
    <message>
        <source>Copyright</source>
        <translation>Авторское право</translation>
    </message>
    <message>
        <source>Output</source>
        <translation>Вывод</translation>
    </message>
    <message>
        <source>Use last filename but incremented.</source>
        <translation>Использовать предыдущее имя файла с приращением.</translation>
    </message>
    <message>
        <source>last used filename but incremented will be shown here</source>
        <translation>Здесь будет выведено предыдущее имя файла с приращением</translation>
    </message>
    <message>
        <source>Prompt for file name</source>
        <translation>Запрашивать имя файла</translation>
    </message>
    <message>
        <source>Note: If you want more control over the map layout please use the map composer function in QGIS.</source>
        <translation>Внимание: для полного контроля над макетом карты рекомендуется использовать компоновщик карт.</translation>
    </message>
    <message>
        <source>Page Size</source>
        <translation>Размер страницы</translation>
    </message>
</context>
<context>
    <name>QuickPrintPlugin</name>
    <message>
        <source>Quick Print</source>
        <translation>Быстрая печать</translation>
    </message>
    <message>
        <source>&amp;Quick Print</source>
        <translation>&amp;Быстрая печать</translation>
    </message>
    <message>
        <source>Provides a way to quickly produce a map with minimal user input.</source>
        <translation>Модуль для быстрой печати карт с минимумом параметров.</translation>
    </message>
</context>
<context>
    <name>RepositoryDetailsDialog</name>
    <message>
        <source>Repository details</source>
        <translation>Данные репозитория</translation>
    </message>
    <message>
        <source>Name:</source>
        <translation>Имя:</translation>
    </message>
    <message>
        <source>URL:</source>
        <translation>URL:</translation>
    </message>
    <message>
        <source>http://</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>[pluginname]GuiBase</name>
    <message>
        <source>QGIS Plugin Template</source>
        <translation></translation>
    </message>
    <message>
        <source>Plugin Template</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>dxf2shpConverter</name>
    <message>
        <source>Converts DXF files in Shapefile format</source>
        <translation>Преобразование файлов формата dxf в shape-файлы</translation>
    </message>
    <message>
        <source>&amp;Dxf2Shp</source>
        <translation>&amp;Dxf2Shp</translation>
    </message>
</context>
<context>
    <name>dxf2shpConverterGui</name>
    <message>
        <source>Choose a DXF file to open</source>
        <translation>Выберите DXF-файл для открытия</translation>
    </message>
    <message>
        <source>Dxf Importer</source>
        <translation>Импорт Dxf</translation>
    </message>
    <message>
        <source>Input Dxf file</source>
        <translation>Исходный Dxf-файл</translation>
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
        <translation>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;DejaVu Sans&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:&apos;MS Shell Dlg 2&apos;; font-size:8pt;&quot;&gt;&lt;span style=&quot; font-size:10pt;&quot;&gt;Файл вывода&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Output file type</source>
        <translation>Тип выходного файла</translation>
    </message>
    <message>
        <source>Polyline</source>
        <translation>Полилиния</translation>
    </message>
    <message>
        <source>Polygon</source>
        <translation>Полигон</translation>
    </message>
    <message>
        <source>Point</source>
        <translation>Точка</translation>
    </message>
    <message>
        <source>Export text labels</source>
        <translation>Экспортировать текстовые подписи</translation>
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
        <source>Choose a file name to save to</source>
        <translation>Выберите имя сохраняемого файла</translation>
    </message>
</context>
<context>
    <name>pluginname</name>
    <message>
        <source>Replace this with a short description of the what the plugin does</source>
        <translation></translation>
    </message>
    <message>
        <source>[menuitemname]</source>
        <translation></translation>
    </message>
    <message>
        <source>&amp;[menuname]</source>
        <translation>&amp;[menuname]</translation>
    </message>
</context>
</TS>
